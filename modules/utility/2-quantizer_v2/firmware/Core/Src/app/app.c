/*
******************************************************************************
* @file           : app.c
* @author         : Lennart Querter
* @brief          : Shared application core: personality select + scheduler
******************************************************************************
*/

#include "app/app.h"

#include "main.h"

#include "app/app_quant.h"
#include "app/cli.h"
#include "app/cv_in.h"
#include "app/dac.h"
#include "app/encoder.h"
#include "app/leds.h"
#include "app/personality.h"
#include "app/settings.h"
#include "app/switches.h"
#include "app/triggers.h"

#define APP_AUTOSAVE_DEBOUNCE_MS 2000
#define APP_BOOT_COMBO_FEEDBACK_MS 800
#define APP_BOOT_COMBO_RELEASE_TIMEOUT_MS 5000

static struct APP_config* app;
static const struct PERSONALITY* personality;
static uint8_t personality_id;
static uint8_t mode_strap;

static int32_t dac_override[APP_CHANNELS];
static uint8_t active_channel;
static uint8_t led_test;

static volatile uint32_t tick_pending;

static uint8_t encoder_switch_pressed(void)
{
    // external pull-up, switch to GND
    return HAL_GPIO_ReadPin(ENC_SW_GPIO_Port, ENC_SW_Pin) == GPIO_PIN_RESET;
}

// left half of the note circle = quantizer, right half = sequencer
static void show_personality_leds(uint8_t id)
{
    for (uint8_t note = 0; note < 12; note++)
    {
        uint8_t lit = (id == APP_PERSONALITY_QUANTIZER) ? note < 6 : note >= 6;
        LEDS_set(LED_NOTE_BASE + note, lit ? LEDS_ON : LEDS_OFF);
    }
    LEDS_tick();
    HAL_Delay(APP_BOOT_COMBO_FEEDBACK_MS);
    for (uint8_t note = 0; note < 12; note++)
    {
        LEDS_set(LED_NOTE_BASE + note, LEDS_OFF);
    }
    LEDS_tick();
}

/*
 * Personality resolution, highest priority first:
 *   1. flash override (CLI `mode`), 2. boot combo (encoder held at
 *   power-up flips + persists), 3. PB2 MODE strap (low = quantizer).
 */
static uint8_t select_personality(void)
{
    mode_strap =
        HAL_GPIO_ReadPin(MODE_GPIO_Port, MODE_Pin) == GPIO_PIN_SET;

    uint8_t override = SETTINGS_get()->mode_override;
    uint8_t id;
    if (override == SETTINGS_MODE_QUANTIZER)
    {
        id = APP_PERSONALITY_QUANTIZER;
    }
    else if (override == SETTINGS_MODE_SEQUENCER)
    {
        id = APP_PERSONALITY_SEQUENCER;
    }
    else
    {
        id = mode_strap ? APP_PERSONALITY_SEQUENCER : APP_PERSONALITY_QUANTIZER;
    }

    if (encoder_switch_pressed())
    {
        id ^= 1;
        SETTINGS_get()->mode_override = (id == APP_PERSONALITY_SEQUENCER)
                                            ? SETTINGS_MODE_SEQUENCER
                                            : SETTINGS_MODE_QUANTIZER;
        SETTINGS_save();
        show_personality_leds(id);

        // don't let the UI see the release as a normal press cycle
        uint32_t start = HAL_GetTick();
        while (encoder_switch_pressed() &&
               HAL_GetTick() - start < APP_BOOT_COMBO_RELEASE_TIMEOUT_MS)
        {
            HAL_Delay(10);
        }
        HAL_Delay(20);
    }
    return id;
}

static void do_tick(void)
{
    TRIGGERS_tick();
    personality->tick();
    LEDS_tick();

    if (SETTINGS_poll_autosave(APP_AUTOSAVE_DEBOUNCE_MS))
    {
        CLI_printf("settings saved\r\n");
    }
}

void APP_init(struct APP_config* config)
{
    app = config;
    active_channel = APP_CHANNEL_A;
    led_test = 0;
    tick_pending = 0;
    for (uint8_t ch = 0; ch < APP_CHANNELS; ch++)
    {
        dac_override[ch] = -1;
    }

    SETTINGS_init();

    DAC_init(app->hspi_dac);
    CV_IN_init(app->hadc);
    TRIGGERS_init();
    SWITCHES_init(app->hspi_sw);
    LEDS_init(app->hspi_led);
    ENCODER_init(app->htim_enc);
    CLI_init(app->huart_cli);

    personality_id = select_personality();
    personality = (personality_id == APP_PERSONALITY_SEQUENCER)
                      ? &PERSONALITY_sequencer
                      : &PERSONALITY_quantizer;

    personality->init();

    HAL_TIM_Base_Start_IT(app->htim_tick);

    CLI_printf("\r\nquantizer-v2 %s — personality: %s\r\n", APP_VERSION,
               personality->name);
    CLI_printf("(strap=%s, override=%s, settings: %s)\r\n",
               mode_strap ? "sequencer" : "quantizer",
               SETTINGS_get()->mode_override == SETTINGS_MODE_AUTO ? "auto"
               : SETTINGS_get()->mode_override == SETTINGS_MODE_SEQUENCER
                   ? "sequencer"
                   : "quantizer",
               SETTINGS_loaded_from_flash() ? "flash" : "defaults");
    CLI_printf("type 'help' for commands\r\n> ");
}

void APP_loop(void)
{
    TRIGGERS_poll();
    personality->fast_loop();

    if (tick_pending)
    {
        tick_pending = 0;
        do_tick();
    }

    CLI_tick();
}

void APP_on_tick(void)
{
    tick_pending++;
}

uint8_t APP_personality(void)
{
    return personality_id;
}

uint8_t APP_mode_strap(void)
{
    return mode_strap;
}

I2C_HandleTypeDef* APP_i2c(void)
{
    return app->hi2c;
}

uint8_t APP_active_channel(void)
{
    return active_channel;
}

void APP_set_active_channel(uint8_t channel)
{
    active_channel = channel & 1;
}

void APP_set_dac_override(uint8_t channel, int32_t code)
{
    channel &= 1;
    dac_override[channel] = code;
    if (code >= 0)
    {
        DAC_write_code(channel, (uint16_t)code);
    }
    else if (personality_id == APP_PERSONALITY_QUANTIZER)
    {
        // hand the DAC back to the quantizer's fast path
        QUANT_APP_reload_from_settings();
    }
}

int32_t APP_dac_override(uint8_t channel)
{
    return dac_override[channel & 1];
}

void APP_dac_write_volts(uint8_t channel, float volts)
{
    channel &= 1;
    if (dac_override[channel] >= 0)
    {
        return;
    }

    const struct SETTINGS_cal* cal = &SETTINGS_get()->cal[channel];
    float scaled = cal->dac_offset_code + volts * cal->dac_code_per_v;
    int32_t code = (int32_t)(scaled >= 0 ? scaled + 0.5f : scaled - 0.5f);
    if (code < 0)
    {
        code = 0;
    }
    if (code > DAC_CODE_MAX)
    {
        code = DAC_CODE_MAX;
    }
    DAC_write_code(channel, (uint16_t)code);
}

void APP_set_led_test(uint8_t on)
{
    led_test = on ? 1 : 0;
}

uint8_t APP_led_test(void)
{
    return led_test;
}
