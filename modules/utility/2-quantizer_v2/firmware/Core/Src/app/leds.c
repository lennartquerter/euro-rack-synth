/*
******************************************************************************
* @file           : leds.c
* @author         : Lennart Querter
* @brief          : 74HC595 LED frame buffer, blink patterns, RCLK latch
******************************************************************************
*/

#include "app/leds.h"

#include "main.h"

/*
 * Which 595 chain bit each logical LED drives. Bit 23 is shifted out first
 * (ends up in the register furthest down the chain). Identity until
 * bring-up says otherwise — fix the panel order here.
 */
static const uint8_t led_map[LEDS_COUNT] = {0,  1,  2,  3,  4,  5,  6,
                                            7,  8,  9,  10, 11, 12, 13,
                                            14, 15, 16, 17, 18, 19};

static SPI_HandleTypeDef* led_spi;
static enum LEDS_mode modes[LEDS_COUNT];
static uint16_t pulse_ms[LEDS_COUNT];
static uint32_t tick_ms;
static uint32_t last_frame;
static uint8_t frame_sent;

void LEDS_init(SPI_HandleTypeDef* hspi)
{
    led_spi = hspi;
    tick_ms = 0;
    last_frame = 0;
    frame_sent = 0;
    for (uint8_t i = 0; i < LEDS_COUNT; i++)
    {
        modes[i] = LEDS_OFF;
        pulse_ms[i] = 0;
    }
    HAL_GPIO_WritePin(LED_LATCH_GPIO_Port, LED_LATCH_Pin, GPIO_PIN_RESET);
}

void LEDS_set(uint8_t index, enum LEDS_mode mode)
{
    if (index < LEDS_COUNT)
    {
        modes[index] = mode;
    }
}

enum LEDS_mode LEDS_get(uint8_t index)
{
    return index < LEDS_COUNT ? modes[index] : LEDS_OFF;
}

void LEDS_pulse(uint8_t index, uint16_t ms)
{
    if (index < LEDS_COUNT)
    {
        pulse_ms[index] = ms;
    }
}

static uint8_t led_lit(uint8_t index)
{
    if (pulse_ms[index] > 0)
    {
        return 1;
    }
    switch (modes[index])
    {
        case LEDS_ON:
            return 1;
        case LEDS_BLINK_SLOW:
            return (tick_ms >> 8) & 1; // 256 ms half-period
        case LEDS_BLINK_FAST:
            return (tick_ms >> 6) & 1; // 64 ms half-period
        default:
            return 0;
    }
}

static void transmit_frame(uint32_t frame)
{
#if !LEDS_ACTIVE_HIGH
    frame = ~frame;
#endif
    uint8_t bytes[3] = {(uint8_t)(frame >> 16), (uint8_t)(frame >> 8),
                        (uint8_t)frame};

    HAL_SPI_Transmit(led_spi, bytes, 3, 5);

    // RCLK rising edge moves the shifted bits to the output latches
    HAL_GPIO_WritePin(LED_LATCH_GPIO_Port, LED_LATCH_Pin, GPIO_PIN_SET);
    for (volatile uint8_t i = 0; i < 10; i++)
    {
    }
    HAL_GPIO_WritePin(LED_LATCH_GPIO_Port, LED_LATCH_Pin, GPIO_PIN_RESET);
}

void LEDS_tick(void)
{
    tick_ms++;

    uint32_t frame = 0;
    for (uint8_t i = 0; i < LEDS_COUNT; i++)
    {
        if (led_lit(i))
        {
            frame |= 1UL << led_map[i];
        }
        if (pulse_ms[i] > 0)
        {
            pulse_ms[i]--;
        }
    }

    if (!frame_sent || frame != last_frame)
    {
        transmit_frame(frame);
        last_frame = frame;
        frame_sent = 1;
    }
}
