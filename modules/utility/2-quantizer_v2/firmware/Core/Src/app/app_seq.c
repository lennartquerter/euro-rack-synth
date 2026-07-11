/*
******************************************************************************
* @file           : app_seq.c
* @author         : Lennart Querter
* @brief          : Sequencer personality: clock sources, engine outputs
*                   to DAC/gates, reset input. Panel UI beyond the
*                   beat/bar display lands in Phase S4 (ui_seq.c).
******************************************************************************
*/

#include "app/app_seq.h"

#include "stm32f4xx_hal.h"

#include "app/app.h"
#include "app/cli.h"
#include "app/i2c_slave.h"
#include "app/leds.h"
#include "app/personality.h"
#include "app/sequences.h"
#include "app/settings.h"
#include "app/triggers.h"
#include "app/ui_seq.h"

#define SEQ_APP_ACTIVITY_PULSE_MS 30

static struct SEQ_engine engine;
static uint8_t running;
static uint32_t clock_accumulator; // BPM * 4 per ms, one pulse per 60000
static uint8_t retrig_wait[SEQ_LANES];
static struct SEQ_lane_output last_out[SEQ_LANES];

uint16_t SEQ_APP_bank_count(uint8_t lane)
{
    return (lane & 1) == SEQ_LANE_A ? SEQ_bank_a_count : SEQ_bank_b_count;
}

const struct SEQ_sequence* SEQ_APP_bank_sequence(uint8_t lane, uint16_t index)
{
    const struct SEQ_sequence* bank =
        (lane & 1) == SEQ_LANE_A ? SEQ_bank_a : SEQ_bank_b;
    return &bank[index];
}

struct SEQ_engine* SEQ_APP_engine(void)
{
    return &engine;
}

uint8_t SEQ_APP_running(void)
{
    return running;
}

void SEQ_APP_set_running(uint8_t on)
{
    running = on ? 1 : 0;
}

uint8_t SEQ_APP_external_clock(void)
{
    return TRIGGERS_jack_present(TRIGGERS_CHANNEL_A);
}

uint8_t SEQ_APP_load(uint8_t lane, uint16_t index, enum SEQ_quant quant)
{
    lane &= 1;
    if (index >= SEQ_APP_bank_count(lane))
    {
        return 0;
    }
    SEQ_arm_sequence(&engine, lane, SEQ_APP_bank_sequence(lane, index), quant);
    SETTINGS_get()->seq_loaded[lane] = index;
    SETTINGS_mark_dirty();
    return 1;
}

/*
 * Engine output -> hardware. Pitch goes to the DAC through the shared
 * calibration; the gate jack follows the step, with a short low gap on
 * retriggers so envelopes refire (ties glide through without a gap).
 */
static void apply_lane_output(uint8_t lane, const struct SEQ_lane_output* out)
{
    if (!out->gate)
    {
        TRIGGERS_out_level(lane, 0);
        retrig_wait[lane] = 0;
    }
    else
    {
        APP_dac_write_volts(lane, (float)out->note / 12.0f);
        if (out->retrigger)
        {
            TRIGGERS_out_level(lane, 0);
            retrig_wait[lane] = SEQ_APP_RETRIG_GAP_MS;
            LEDS_pulse(LED_STATUS_BASE + 2 + lane, SEQ_APP_ACTIVITY_PULSE_MS);
        }
        else if (retrig_wait[lane] == 0)
        {
            TRIGGERS_out_level(lane, 1);
        }
    }
    last_out[lane] = *out;
}

static void do_pulse(void)
{
    struct SEQ_lane_output out[SEQ_LANES];
    SEQ_advance(&engine, out);
    for (uint8_t lane = 0; lane < SEQ_LANES; lane++)
    {
        apply_lane_output(lane, &out[lane]);
    }
}

static void seq_init(void)
{
    struct SETTINGS_data* settings = SETTINGS_get();

    for (uint8_t lane = 0; lane < SEQ_LANES; lane++)
    {
        if (settings->seq_loaded[lane] >= SEQ_APP_bank_count(lane))
        {
            settings->seq_loaded[lane] = 0;
        }
        retrig_wait[lane] = 0;
        TRIGGERS_out_level(lane, 0);
    }

    SEQ_init(&engine, SEQ_APP_bank_sequence(SEQ_LANE_A, settings->seq_loaded[0]),
             SEQ_APP_bank_sequence(SEQ_LANE_B, settings->seq_loaded[1]),
             HAL_GetTick() ^ 0x5EED5EEDUL);
    running = 1;
    clock_accumulator = 0;
    UI_SEQ_init();
    I2C_SLAVE_init(APP_i2c());

    CLI_printf("\r\nsequencer: bank A %u lead, bank B %u bass, %u steps/phrase\r\n",
               SEQ_bank_a_count, SEQ_bank_b_count, (unsigned)SEQ_PHRASE_STEPS);
    CLI_printf("loaded: A#%u \"%s\", B#%u \"%s\"\r\n", settings->seq_loaded[0],
               SEQ_APP_bank_sequence(SEQ_LANE_A, settings->seq_loaded[0])->name,
               settings->seq_loaded[1],
               SEQ_APP_bank_sequence(SEQ_LANE_B, settings->seq_loaded[1])->name);
    CLI_printf("clock: %s, bpm %u ('run'/'stop', 'seq' commands)\r\n",
               SEQ_APP_external_clock() ? "external 4 PPQN" : "internal",
               settings->seq_bpm);
    CLI_printf("conductor: listening on 0x%02X (+broadcast)\r\n",
               I2C_SLAVE_address());
}

static void seq_fast_loop(void)
{
    // reset first: a simultaneous reset+clock edge restarts cleanly
    if (TRIGGERS_take_edge(TRIGGERS_CHANNEL_B))
    {
        SEQ_reset(&engine);
    }

    if (SEQ_APP_external_clock())
    {
        if (TRIGGERS_take_edge(TRIGGERS_CHANNEL_A))
        {
            do_pulse();
        }
    }
    else
    {
        TRIGGERS_take_edge(TRIGGERS_CHANNEL_A); // drop stale edges
    }

    I2C_SLAVE_poll();
}

static void seq_tick(void)
{
    if (!SEQ_APP_external_clock() && running)
    {
        struct SETTINGS_data* settings = SETTINGS_get();
        clock_accumulator +=
            (uint32_t)settings->seq_bpm * SEQ_SUBSTEPS_PER_BEAT;
        while (clock_accumulator >= 60000UL)
        {
            clock_accumulator -= 60000UL;
            do_pulse();
        }
    }

    for (uint8_t lane = 0; lane < SEQ_LANES; lane++)
    {
        if (retrig_wait[lane] > 0 && --retrig_wait[lane] == 0 &&
            last_out[lane].gate)
        {
            TRIGGERS_out_level(lane, 1);
        }
    }

    UI_SEQ_tick();
}

const struct PERSONALITY PERSONALITY_sequencer = {
    .name = "sequencer",
    .init = seq_init,
    .fast_loop = seq_fast_loop,
    .tick = seq_tick,
};
