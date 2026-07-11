/*
******************************************************************************
* @file           : ui_quant.c
* @author         : Lennart Querter
* @brief          : Panel UI: modes, mask/root/transpose editing, LEDs
******************************************************************************
*/

#include "app/ui_quant.h"

#include "stm32f4xx_hal.h"

#include "app/app.h"
#include "app/app_quant.h"
#include "app/cli.h"
#include "app/encoder.h"
#include "app/leds.h"
#include "app/quantize.h"
#include "app/settings.h"
#include "app/switches.h"

enum UI_mode
{
    UI_MODE_PLAY = 0,
    UI_MODE_SCALE, // encoder browses presets for the active channel
};

static enum UI_mode mode;
static uint32_t scale_activity_ms;
static int8_t scale_index;       // -1 = the custom mask captured on entry
static uint16_t scale_entry_mask;
static uint8_t shift_held;       // button D

static const char* const note_names[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                           "F#", "G",  "G#", "A",  "A#", "B"};

void UI_QUANT_init(void)
{
    mode = UI_MODE_PLAY;
    scale_index = -1;
    shift_held = 0;
}

static const char* scale_name(int8_t index)
{
    return index < 0 ? "custom" : QUANTIZE_scales[index].name;
}

static void apply_scale_index(void)
{
    uint8_t channel = APP_active_channel();
    uint16_t mask = scale_index < 0 ? scale_entry_mask
                                    : QUANTIZE_scales[scale_index].mask;
    QUANT_APP_apply_mask(channel, mask);
    SETTINGS_mark_dirty();
    CLI_printf("scale %c = %s\r\n", channel == 0 ? 'A' : 'B',
               scale_name(scale_index));
}

static void enter_scale_mode(void)
{
    mode = UI_MODE_SCALE;
    scale_activity_ms = HAL_GetTick();

    // start browsing at the matching preset; slot -1 keeps the current
    // (possibly custom) mask reachable while turning back
    scale_entry_mask = SETTINGS_get()->note_mask[APP_active_channel()];
    scale_index = -1;
    for (uint8_t i = 0; i < QUANTIZE_SCALES_COUNT; i++)
    {
        if (QUANTIZE_scales[i].mask == scale_entry_mask)
        {
            scale_index = (int8_t)i;
            break;
        }
    }
    CLI_printf("scale browse %c: %s (turn encoder, C exits)\r\n",
               APP_active_channel() == 0 ? 'A' : 'B', scale_name(scale_index));
}

static void exit_scale_mode(void)
{
    mode = UI_MODE_PLAY;
}

static void select_channel(uint8_t channel)
{
    if (mode == UI_MODE_SCALE)
    {
        exit_scale_mode(); // browsing state belongs to the old channel
    }
    APP_set_active_channel(channel);
}

static void toggle_note(uint8_t note)
{
    uint8_t channel = APP_active_channel();
    struct QUANTIZE_state* quant = QUANT_APP_state(channel);

    // buttons are the absolute chromatic circle; the mask is degree-based,
    // so compensate for the root rotation
    int16_t degree = (int16_t)((note - quant->root) % 12);
    if (degree < 0)
    {
        degree += 12;
    }
    uint16_t mask =
        SETTINGS_get()->note_mask[channel] ^ (uint16_t)(1u << degree);
    QUANT_APP_apply_mask(channel, mask);
    SETTINGS_mark_dirty();
}

static void set_root(uint8_t note)
{
    uint8_t channel = APP_active_channel();
    QUANT_APP_apply_root(channel, note);
    SETTINGS_mark_dirty();
    CLI_printf("root %c = %s\r\n", channel == 0 ? 'A' : 'B', note_names[note]);
}

static void handle_switch_events(void)
{
    struct SWITCHES_event event;

    while (SWITCHES_get_event(&event))
    {
        if (event.index == SW_BTN_D)
        {
            shift_held = event.pressed;
            continue;
        }
        if (!event.pressed)
        {
            continue;
        }

        if (event.index < 12)
        {
            if (shift_held)
            {
                set_root(event.index);
            }
            else
            {
                if (mode == UI_MODE_SCALE)
                {
                    exit_scale_mode(); // editing notes makes it custom
                }
                toggle_note(event.index);
            }
        }
        else if (event.index == SW_BTN_A)
        {
            select_channel(APP_CHANNEL_A);
        }
        else if (event.index == SW_BTN_B)
        {
            select_channel(APP_CHANNEL_B);
        }
        else if (event.index == SW_BTN_C)
        {
            if (mode == UI_MODE_SCALE)
            {
                exit_scale_mode();
            }
            else
            {
                enter_scale_mode();
            }
        }
        else if (event.index == SW_ENC)
        {
            if (mode == UI_MODE_SCALE)
            {
                exit_scale_mode();
            }
            else
            {
                select_channel(APP_active_channel() == APP_CHANNEL_A
                                   ? APP_CHANNEL_B
                                   : APP_CHANNEL_A);
            }
        }
    }
}

static void browse_scales(int32_t delta)
{
    scale_activity_ms = HAL_GetTick();

    // wrap over the 13 slots: -1 (custom) .. QUANTIZE_SCALES_COUNT-1
    int32_t slots = QUANTIZE_SCALES_COUNT + 1;
    int32_t index = scale_index + delta + 1;
    index = ((index % slots) + slots) % slots;
    scale_index = (int8_t)(index - 1);

    apply_scale_index();
}

static void edit_transpose(int32_t delta)
{
    uint8_t channel = APP_active_channel();
    struct SETTINGS_data* settings = SETTINGS_get();

    int32_t transpose = settings->transpose[channel] + delta;
    if (transpose < APP_TRANSPOSE_MIN)
    {
        transpose = APP_TRANSPOSE_MIN;
    }
    if (transpose > APP_TRANSPOSE_MAX)
    {
        transpose = APP_TRANSPOSE_MAX;
    }
    if (transpose == settings->transpose[channel])
    {
        return;
    }

    QUANT_APP_apply_transpose(channel, (int8_t)transpose);
    SETTINGS_mark_dirty();
    CLI_printf("transpose %c = %+d\r\n", channel == 0 ? 'A' : 'B',
               (int)transpose);
}

static void handle_encoder(void)
{
    int32_t delta = ENCODER_poll();
    if (delta == 0)
    {
        return;
    }
    if (mode == UI_MODE_SCALE)
    {
        browse_scales(delta);
    }
    else
    {
        edit_transpose(delta);
    }
}

static void refresh_leds(void)
{
    uint8_t channel = APP_active_channel();
    struct QUANTIZE_state* quant = QUANT_APP_state(channel);

    for (uint8_t note = 0; note < 12; note++)
    {
        enum LEDS_mode led =
            QUANTIZE_note_allowed(quant, note) ? LEDS_ON : LEDS_OFF;
        // shift shows where the root sits
        if (shift_held && note == (uint8_t)quant->root)
        {
            led = LEDS_BLINK_FAST;
        }
        LEDS_set(LED_NOTE_BASE + note, led);
    }

    LEDS_set(LED_BTN_A, channel == APP_CHANNEL_A ? LEDS_ON : LEDS_OFF);
    LEDS_set(LED_BTN_B, channel == APP_CHANNEL_B ? LEDS_ON : LEDS_OFF);
    LEDS_set(LED_BTN_C, mode == UI_MODE_SCALE ? LEDS_BLINK_FAST : LEDS_OFF);
    LEDS_set(LED_BTN_D, shift_held ? LEDS_ON : LEDS_OFF);

    LEDS_set(LED_STATUS_BASE + 0,
             channel == APP_CHANNEL_A ? LEDS_ON : LEDS_OFF);
    LEDS_set(LED_STATUS_BASE + 1,
             channel == APP_CHANNEL_B ? LEDS_ON : LEDS_OFF);
    // LED_STATUS_BASE + 2/3 pulse on note changes (app.c fast path)
}

void UI_QUANT_tick(void)
{
    SWITCHES_scan();
    handle_switch_events();
    handle_encoder();

    if (mode == UI_MODE_SCALE &&
        HAL_GetTick() - scale_activity_ms > UI_QUANT_SCALE_TIMEOUT_MS)
    {
        exit_scale_mode();
    }

    if (!APP_led_test())
    {
        refresh_leds();
    }
}
