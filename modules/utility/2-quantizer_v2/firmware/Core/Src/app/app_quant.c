/*
******************************************************************************
* @file           : app_quant.c
* @author         : Lennart Querter
* @brief          : Quantizer personality: fast CV->quantize->DAC path
******************************************************************************
*/

#include "app/app_quant.h"

#include "app/app.h"
#include "app/cv_in.h"
#include "app/dac.h"
#include "app/leds.h"
#include "app/personality.h"
#include "app/settings.h"
#include "app/triggers.h"
#include "app/ui_quant.h"

#define QUANT_NOTE_NONE INT32_MIN
#define QUANT_ACTIVITY_PULSE_MS 30

static struct QUANTIZE_state quant[APP_CHANNELS];
static int32_t last_out_note[APP_CHANNELS];

static int32_t round_f(float value)
{
    return (int32_t)(value >= 0 ? value + 0.5f : value - 0.5f);
}

static void force_refresh(uint8_t channel)
{
    last_out_note[channel] = QUANT_NOTE_NONE;
}

/*
 * Fast path: track the input, quantize, update the DAC when the output
 * note changes. Runs every main-loop iteration (way above the ~2 kHz the
 * spec asks for), so input→output latency stays well under 1 ms.
 *
 * With a cable in the trig-in jack the channel switches to triggered
 * mode: sample-and-hold, quantizing only on a trigger rising edge.
 */
static void process_channel(uint8_t channel)
{
    struct SETTINGS_data* settings = SETTINGS_get();

    if (TRIGGERS_jack_present(channel) && !TRIGGERS_take_edge(channel))
    {
        return; // triggered mode: hold the output until the next edge
    }

    float volts_in = CV_IN_volts(channel, &settings->cal[channel]);
    int32_t cents = round_f(volts_in * 1200.0f);
    int16_t note = QUANTIZE_process(&quant[channel], cents);
    int32_t out_note = note + settings->transpose[channel];

    if (out_note == last_out_note[channel])
    {
        return;
    }
    last_out_note[channel] = out_note;

    APP_dac_write_volts(channel, (float)out_note / 12.0f);
    TRIGGERS_pulse(channel);
    LEDS_pulse(LED_STATUS_BASE + 2 + channel, QUANT_ACTIVITY_PULSE_MS);
}

struct QUANTIZE_state* QUANT_APP_state(uint8_t channel)
{
    return &quant[channel & 1];
}

void QUANT_APP_apply_mask(uint8_t channel, uint16_t mask)
{
    channel &= 1;
    SETTINGS_get()->note_mask[channel] = mask & 0x0FFF;
    QUANTIZE_set_mask(&quant[channel], mask);
    force_refresh(channel);
}

void QUANT_APP_apply_root(uint8_t channel, int16_t root)
{
    channel &= 1;
    SETTINGS_get()->root[channel] = (int8_t)(root % 12);
    QUANTIZE_set_root(&quant[channel], root);
    force_refresh(channel);
}

void QUANT_APP_apply_transpose(uint8_t channel, int8_t transpose)
{
    channel &= 1;
    SETTINGS_get()->transpose[channel] = transpose;
    force_refresh(channel);
}

int16_t QUANT_APP_last_note(uint8_t channel)
{
    return quant[channel & 1].last_note;
}

void QUANT_APP_reload_from_settings(void)
{
    struct SETTINGS_data* settings = SETTINGS_get();
    for (uint8_t ch = 0; ch < APP_CHANNELS; ch++)
    {
        QUANTIZE_init(&quant[ch], settings->note_mask[ch], settings->root[ch]);
        force_refresh(ch);
    }
}

static void quant_init(void)
{
    QUANT_APP_reload_from_settings();
    UI_QUANT_init();
}

static void quant_fast_loop(void)
{
    process_channel(APP_CHANNEL_A);
    process_channel(APP_CHANNEL_B);
}

static void quant_tick(void)
{
    UI_QUANT_tick();
}

const struct PERSONALITY PERSONALITY_quantizer = {
    .name = "quantizer",
    .init = quant_init,
    .fast_loop = quant_fast_loop,
    .tick = quant_tick,
};
