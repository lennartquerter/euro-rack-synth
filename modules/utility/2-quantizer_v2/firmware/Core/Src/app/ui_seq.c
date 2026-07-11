/*
******************************************************************************
* @file           : ui_seq.c
* @author         : Lennart Querter
* @brief          : Sequencer panel UI: binary load flow, shift layer, LEDs
******************************************************************************
*/

#include "app/ui_seq.h"

#include "stm32f4xx_hal.h"

#include "app/app.h"
#include "app/app_seq.h"
#include "app/cli.h"
#include "app/encoder.h"
#include "app/leds.h"
#include "app/settings.h"
#include "app/switches.h"

enum UI_SEQ_mode
{
    UI_SEQ_PLAY = 0,
    UI_SEQ_LOAD, // note buttons enter a sequence number in binary
};

static enum UI_SEQ_mode mode;
static uint8_t load_lane;
static uint16_t load_value;
static uint32_t load_activity_ms;
static uint8_t shift_held; // button D

static const char* const direction_names[] = {"forward", "reverse",
                                              "ping-pong", "random"};

static const char* lane_name(uint8_t lane)
{
    return lane == SEQ_LANE_A ? "A" : "B";
}

// how many note buttons carry value bits for this lane's bank
static uint8_t load_bits(uint8_t lane)
{
    uint16_t count = SEQ_APP_bank_count(lane);
    uint8_t bits = 1;
    while ((1u << bits) < count && bits < 12)
    {
        bits++;
    }
    return bits;
}

void UI_SEQ_init(void)
{
    mode = UI_SEQ_PLAY;
    shift_held = 0;
}

static void enter_load_mode(uint8_t lane)
{
    mode = UI_SEQ_LOAD;
    load_lane = lane;
    load_value = SETTINGS_get()->seq_loaded[lane];
    load_activity_ms = HAL_GetTick();
    CLI_printf("load %s: enter bits on the note buttons, %s confirms\r\n",
               lane_name(lane), lane_name(lane));
}

static void cancel_load_mode(void)
{
    mode = UI_SEQ_PLAY;
}

static void confirm_load(void)
{
    if (!SEQ_APP_load(load_lane, load_value, SEQ_QUANT_PHRASE))
    {
        // out of range: stay in load mode, the LEDs are already blinking
        CLI_printf("load %s: #%u is outside the bank (0-%u)\r\n",
                   lane_name(load_lane), load_value,
                   SEQ_APP_bank_count(load_lane) - 1);
        return;
    }
    CLI_printf("load %s: #%u \"%s\" armed for the next phrase\r\n",
               lane_name(load_lane), load_value,
               SEQ_APP_bank_sequence(load_lane, load_value)->name);
    mode = UI_SEQ_PLAY;
}

static void toggle_mute(uint8_t lane)
{
    struct SEQ_engine* engine = SEQ_APP_engine();
    uint8_t mute = !engine->lanes[lane].mute;
    SEQ_arm_mute(engine, lane, mute, SEQ_QUANT_NOW);
    CLI_printf("lane %s %s\r\n", lane_name(lane), mute ? "muted" : "unmuted");
}

static void cycle_direction(void)
{
    struct SEQ_engine* engine = SEQ_APP_engine();
    uint8_t direction = (uint8_t)((engine->direction + 1) & 3);
    SEQ_arm_direction(engine, (enum SEQ_direction)direction, SEQ_QUANT_NOW);
    CLI_printf("direction = %s\r\n", direction_names[direction]);
}

static void handle_play_button(uint8_t index)
{
    if (index == SW_BTN_A || index == SW_BTN_B)
    {
        uint8_t lane = index == SW_BTN_A ? SEQ_LANE_A : SEQ_LANE_B;
        if (shift_held)
        {
            toggle_mute(lane);
        }
        else
        {
            enter_load_mode(lane);
        }
    }
    else if (index == SW_BTN_C)
    {
        if (shift_held)
        {
            cycle_direction();
        }
        else if (SEQ_APP_external_clock())
        {
            toggle_mute(APP_active_channel()); // clock comes from outside
        }
        else
        {
            SEQ_APP_set_running(!SEQ_APP_running());
            CLI_printf("%s\r\n", SEQ_APP_running() ? "run" : "stop");
        }
    }
    else if (index == SW_ENC)
    {
        APP_set_active_channel(APP_active_channel() == SEQ_LANE_A
                                   ? SEQ_LANE_B
                                   : SEQ_LANE_A);
        CLI_printf("focus lane %s\r\n", lane_name(APP_active_channel()));
    }
}

static void handle_load_button(uint8_t index)
{
    load_activity_ms = HAL_GetTick();

    if (index < 12)
    {
        if (index < load_bits(load_lane))
        {
            load_value ^= (uint16_t)(1u << index);
        }
    }
    else if (index == SW_BTN_A || index == SW_BTN_B)
    {
        uint8_t lane = index == SW_BTN_A ? SEQ_LANE_A : SEQ_LANE_B;
        if (lane == load_lane)
        {
            confirm_load();
        }
        else
        {
            enter_load_mode(lane); // switch target
        }
    }
    else if (index == SW_ENC)
    {
        confirm_load();
    }
    else if (index == SW_BTN_C)
    {
        cancel_load_mode();
    }
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

        if (mode == UI_SEQ_LOAD)
        {
            handle_load_button(event.index);
        }
        else
        {
            handle_play_button(event.index);
        }
    }
}

static void edit_bpm(int32_t delta)
{
    struct SETTINGS_data* settings = SETTINGS_get();
    int32_t bpm = (int32_t)settings->seq_bpm + delta;
    if (bpm < SEQ_APP_BPM_MIN)
    {
        bpm = SEQ_APP_BPM_MIN;
    }
    if (bpm > SEQ_APP_BPM_MAX)
    {
        bpm = SEQ_APP_BPM_MAX;
    }
    if ((uint16_t)bpm == settings->seq_bpm)
    {
        return;
    }
    settings->seq_bpm = (uint16_t)bpm;
    SETTINGS_mark_dirty();
    CLI_printf("bpm = %u\r\n", settings->seq_bpm);
}

static void edit_transpose(int32_t delta)
{
    struct SEQ_engine* engine = SEQ_APP_engine();
    uint8_t lane = APP_active_channel();
    int16_t transpose = (int16_t)(engine->lanes[lane].transpose + delta);

    SEQ_arm_transpose(engine, lane, transpose, SEQ_QUANT_NOW); // clamps
    CLI_printf("transpose %s = %+d\r\n", lane_name(lane),
               engine->lanes[lane].transpose);
}

static void handle_encoder(void)
{
    int32_t delta = ENCODER_poll();
    if (delta == 0)
    {
        return;
    }
    if (mode == UI_SEQ_LOAD)
    {
        load_activity_ms = HAL_GetTick();
        return; // knob reserved while entering a number
    }
    if (shift_held)
    {
        edit_transpose(delta);
    }
    else
    {
        edit_bpm(delta);
    }
}

static void refresh_leds(void)
{
    struct SEQ_engine* engine = SEQ_APP_engine();

    if (mode == UI_SEQ_LOAD)
    {
        // note LEDs show the entered value; blinking = outside the bank
        uint8_t bits = load_bits(load_lane);
        uint8_t invalid = load_value >= SEQ_APP_bank_count(load_lane);
        for (uint8_t i = 0; i < 12; i++)
        {
            uint8_t set = i < bits && ((load_value >> i) & 1u);
            LEDS_set(LED_NOTE_BASE + i,
                     set ? (invalid ? LEDS_BLINK_FAST : LEDS_ON) : LEDS_OFF);
        }
    }
    else
    {
        // beat chase on 1-8, bar on 9-12
        uint8_t beat = SEQ_beat(engine);
        uint8_t bar = SEQ_bar(engine);
        for (uint8_t i = 0; i < SEQ_BEATS_PER_BAR; i++)
        {
            LEDS_set(LED_NOTE_BASE + i, i == beat ? LEDS_ON : LEDS_OFF);
        }
        for (uint8_t i = 0; i < SEQ_BARS_PER_PHRASE; i++)
        {
            LEDS_set(LED_NOTE_BASE + SEQ_BEATS_PER_BAR + i,
                     i == bar ? LEDS_ON : LEDS_OFF);
        }
    }

    // lane buttons: blink while loading, otherwise mark the focus lane
    LEDS_set(LED_BTN_A, (mode == UI_SEQ_LOAD && load_lane == SEQ_LANE_A)
                            ? LEDS_BLINK_FAST
                        : APP_active_channel() == SEQ_LANE_A ? LEDS_ON
                                                             : LEDS_OFF);
    LEDS_set(LED_BTN_B, (mode == UI_SEQ_LOAD && load_lane == SEQ_LANE_B)
                            ? LEDS_BLINK_FAST
                        : APP_active_channel() == SEQ_LANE_B ? LEDS_ON
                                                             : LEDS_OFF);

    // C = clock/transport, D = shift
    LEDS_set(LED_BTN_C, SEQ_APP_external_clock() ? LEDS_BLINK_SLOW
                        : SEQ_APP_running()      ? LEDS_ON
                                                 : LEDS_OFF);
    LEDS_set(LED_BTN_D, shift_held ? LEDS_ON : LEDS_OFF);

    // status 1/2: lane state (on = playing, blink = armed, off = muted)
    for (uint8_t lane = 0; lane < SEQ_LANES; lane++)
    {
        enum LEDS_mode state = LEDS_ON;
        if (SEQ_sequence_armed(engine, lane))
        {
            state = LEDS_BLINK_FAST;
        }
        else if (engine->lanes[lane].mute)
        {
            state = LEDS_OFF;
        }
        LEDS_set(LED_STATUS_BASE + lane, state);
    }
    // status 3/4: gate activity pulses (app_seq overlays them)
    LEDS_set(LED_STATUS_BASE + 2, LEDS_OFF);
    LEDS_set(LED_STATUS_BASE + 3, LEDS_OFF);
}

void UI_SEQ_tick(void)
{
    SWITCHES_scan();
    handle_switch_events();
    handle_encoder();

    if (mode == UI_SEQ_LOAD &&
        HAL_GetTick() - load_activity_ms > UI_SEQ_LOAD_TIMEOUT_MS)
    {
        cancel_load_mode();
    }

    if (!APP_led_test())
    {
        refresh_leds();
    }
}
