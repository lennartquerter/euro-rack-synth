/*
******************************************************************************
* @file           : i2c_slave.c
* @author         : Lennart Querter
* @brief          : Conductor I2C protocol slave (frame RX, dedup, apply)
******************************************************************************
*/

#include "app/i2c_slave.h"

#include <string.h>

#include "app/app_seq.h"
#include "app/leds.h"
#include "app/quantize.h"
#include "app/seq.h"
#include "app/settings.h"

// common commands
#define CMD_NOOP 0x00
#define CMD_RESET 0x01
#define CMD_MUTE 0x02
#define CMD_IDENTIFY 0x3F
// sequencer commands
#define CMD_SET_PATTERN 0x10
#define CMD_SET_LENGTH 0x11
#define CMD_TRANSPOSE 0x12
#define CMD_SET_SCALE 0x13
#define CMD_SET_DIR 0x14
#define CMD_MUTATE 0x15
#define CMD_SET_DENSITY 0x16
#define CMD_SET_OCTAVE 0x17

#define FRAME_OVERHEAD 4 // CMD + LEN + SEQ + CHK
#define FRAME_MAX (FRAME_OVERHEAD + I2C_SLAVE_MAX_PAYLOAD)

#define IDENTIFY_BLINK_MS 600

static I2C_HandleTypeDef* slave_i2c;
static uint8_t own_address;

// ISR side: byte collector
static volatile uint8_t rx_buffer[FRAME_MAX];
static volatile uint8_t rx_count;
static volatile uint8_t frame_ready;
static volatile uint8_t frame_length;
static uint8_t frame[FRAME_MAX]; // main-loop copy
static uint8_t tx_zero;          // protocol is write-only: reads get 0x00

static uint16_t last_seq[64]; // per command code, 0xFFFF = none yet
static struct I2C_SLAVE_stats stats;

void I2C_SLAVE_init(I2C_HandleTypeDef* hi2c)
{
    slave_i2c = hi2c;
    own_address = (uint8_t)(I2C_SLAVE_TYPE_SEQUENCER +
                            (SETTINGS_get()->i2c_instance & 3));

    HAL_I2C_DeInit(slave_i2c);
    slave_i2c->Init.OwnAddress1 = (uint32_t)own_address << 1;
    slave_i2c->Init.DualAddressMode = I2C_DUALADDRESS_ENABLE;
    slave_i2c->Init.OwnAddress2 = (uint32_t)I2C_SLAVE_BROADCAST << 1;
    HAL_I2C_Init(slave_i2c);

    rx_count = 0;
    frame_ready = 0;
    for (uint8_t i = 0; i < 64; i++)
    {
        last_seq[i] = 0xFFFF;
    }

    HAL_I2C_EnableListen_IT(slave_i2c);
}

uint8_t I2C_SLAVE_address(void)
{
    return own_address;
}

const struct I2C_SLAVE_stats* I2C_SLAVE_get_stats(void)
{
    return &stats;
}

/*
 * ISR side: collect bytes until the master's STOP.
 */

void I2C_SLAVE_on_addr(uint8_t direction, uint16_t match_code)
{
    (void)match_code; // broadcast and own address behave the same

    if (slave_i2c == NULL)
    {
        return;
    }
    if (direction == I2C_DIRECTION_TRANSMIT) // master writes to us
    {
        rx_count = 0;
        HAL_I2C_Slave_Seq_Receive_IT(slave_i2c, (uint8_t*)&rx_buffer[0], 1,
                                     I2C_NEXT_FRAME);
    }
    else // master reads: keep the bus alive with zeros
    {
        tx_zero = 0;
        HAL_I2C_Slave_Seq_Transmit_IT(slave_i2c, &tx_zero, 1, I2C_NEXT_FRAME);
    }
}

void I2C_SLAVE_on_rx_complete(void)
{
    if (slave_i2c == NULL)
    {
        return;
    }
    if (rx_count < FRAME_MAX - 1)
    {
        rx_count++;
    }
    // an oversized frame keeps overwriting the last slot and fails the
    // length check later
    HAL_I2C_Slave_Seq_Receive_IT(slave_i2c, (uint8_t*)&rx_buffer[rx_count], 1,
                                 I2C_NEXT_FRAME);
}

void I2C_SLAVE_on_tx_complete(void)
{
    if (slave_i2c == NULL)
    {
        return;
    }
    tx_zero = 0;
    HAL_I2C_Slave_Seq_Transmit_IT(slave_i2c, &tx_zero, 1, I2C_NEXT_FRAME);
}

void I2C_SLAVE_on_listen_complete(void)
{
    if (slave_i2c == NULL)
    {
        return;
    }
    if (rx_count > 0 && !frame_ready)
    {
        memcpy(frame, (const void*)rx_buffer, rx_count);
        frame_length = rx_count;
        frame_ready = 1;
    }
    rx_count = 0;
    HAL_I2C_EnableListen_IT(slave_i2c);
}

void I2C_SLAVE_on_error(void)
{
    if (slave_i2c == NULL)
    {
        return;
    }
    stats.bus_errors++;
    rx_count = 0;
    HAL_I2C_EnableListen_IT(slave_i2c);
}

/*
 * Main-loop side: validate, dedup, apply through the engine arming API.
 */

static void identify_blink(void)
{
    for (uint8_t i = 0; i < LEDS_COUNT; i++)
    {
        LEDS_pulse(i, IDENTIFY_BLINK_MS);
    }
}

static void apply_command(uint8_t code, enum SEQ_quant quant,
                          const uint8_t* payload, uint8_t length)
{
    struct SEQ_engine* engine = SEQ_APP_engine();

    switch (code)
    {
        case CMD_NOOP:
            break;
        case CMD_RESET:
            SEQ_reset(engine);
            break;
        case CMD_MUTE:
            if (length >= 1)
            {
                SEQ_arm_mute(engine, SEQ_LANE_A, payload[0], quant);
                SEQ_arm_mute(engine, SEQ_LANE_B, payload[0], quant);
            }
            break;
        case CMD_IDENTIFY:
            identify_blink();
            break;
        case CMD_SET_PATTERN:
            if (length >= 1)
            {
                uint8_t id = payload[0] & 0x7F;
                uint8_t lane = (id >> 6) & 1; // bit 6 selects the lane
                SEQ_APP_load(lane, (uint16_t)(id & 0x3F), quant);
            }
            break;
        case CMD_SET_LENGTH:
            if (length >= 1)
            {
                // protocol "steps" are interpreted as beats (clamped 1-32)
                SEQ_arm_length_beats(engine, payload[0], quant);
            }
            break;
        case CMD_TRANSPOSE:
            if (length >= 1)
            {
                SEQ_arm_transpose(engine, SEQ_LANE_A, (int8_t)payload[0],
                                  quant);
                SEQ_arm_transpose(engine, SEQ_LANE_B, (int8_t)payload[0],
                                  quant);
            }
            break;
        case CMD_SET_SCALE:
            if (length >= 1 && payload[0] < QUANTIZE_SCALES_COUNT)
            {
                SEQ_set_scale(engine, SEQ_LANE_A,
                              QUANTIZE_scales[payload[0]].mask, 0);
                SEQ_set_scale(engine, SEQ_LANE_B,
                              QUANTIZE_scales[payload[0]].mask, 0);
            }
            break;
        case CMD_SET_DIR:
            if (length >= 1)
            {
                SEQ_arm_direction(engine, (enum SEQ_direction)(payload[0] & 3),
                                  quant);
            }
            break;
        case CMD_SET_OCTAVE:
            if (length >= 1)
            {
                SEQ_arm_octave(engine, SEQ_LANE_A, (int8_t)payload[0], quant);
                SEQ_arm_octave(engine, SEQ_LANE_B, (int8_t)payload[0], quant);
            }
            break;
        case CMD_MUTATE:
        case CMD_SET_DENSITY:
            break; // accepted but ignored in v1
        default:
            stats.unknown++;
            return;
    }
    stats.applied++;
}

void I2C_SLAVE_poll(void)
{
    if (!frame_ready)
    {
        return;
    }

    uint8_t length = frame_length;
    uint8_t ok = 0;

    if (length >= FRAME_OVERHEAD)
    {
        uint8_t payload_length = frame[1];
        if (payload_length <= I2C_SLAVE_MAX_PAYLOAD &&
            length == FRAME_OVERHEAD + payload_length)
        {
            uint8_t seq = frame[2 + payload_length];
            uint8_t chk = frame[3 + payload_length];
            uint8_t expected = 0;
            for (uint8_t i = 0; i < (uint8_t)(length - 1); i++)
            {
                expected ^= frame[i]; // CMD, LEN, PAYLOAD, SEQ
            }

            if (chk == expected)
            {
                uint8_t code = frame[0] & 0x3F;
                if (last_seq[code] == seq)
                {
                    stats.duplicates++; // conductor sends everything twice
                }
                else
                {
                    last_seq[code] = seq;
                    apply_command(code, (enum SEQ_quant)(frame[0] >> 6),
                                  &frame[2], payload_length);
                }
                ok = 1;
            }
        }
    }
    if (!ok)
    {
        stats.bad_frames++;
    }

    frame_ready = 0;
}
