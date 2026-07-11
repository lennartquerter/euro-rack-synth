/*
******************************************************************************
* @file           : cli.c
* @author         : Lennart Querter
* @brief          : USART1 debug shell: inspect ADC/DAC, force outputs,
*                   edit masks/scales, run two-point calibration, save/load
******************************************************************************
*/

#include "app/cli.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

#include "app/app.h"
#include "app/app_quant.h"
#include "app/app_seq.h"
#include "app/cv_in.h"
#include "app/dac.h"
#include "app/encoder.h"
#include "app/i2c_slave.h"
#include "app/leds.h"
#include "app/quantize.h"
#include "app/settings.h"
#include "app/switches.h"
#include "app/triggers.h"

#define CLI_RX_RING_SIZE 128
#define CLI_LINE_SIZE 96
#define CLI_TX_TIMEOUT_MS 100

static UART_HandleTypeDef* cli_uart;

static uint8_t rx_byte;
static volatile uint8_t rx_ring[CLI_RX_RING_SIZE];
static volatile uint16_t rx_head;
static uint16_t rx_tail;

static char line[CLI_LINE_SIZE];
static uint16_t line_length;

// pending two-point calibration measurements, per channel
struct CLI_cal_points
{
    uint8_t count;
    float code[2];
    float volts[2];
};
static struct CLI_cal_points cal_adc_points[2];
static struct CLI_cal_points cal_dac_points[2];

void CLI_init(UART_HandleTypeDef* huart)
{
    cli_uart = huart;
    rx_head = 0;
    rx_tail = 0;
    line_length = 0;
    memset(cal_adc_points, 0, sizeof(cal_adc_points));
    memset(cal_dac_points, 0, sizeof(cal_dac_points));

    HAL_UART_Receive_IT(cli_uart, &rx_byte, 1);
}

void CLI_on_rx_isr(void)
{
    uint16_t next = (uint16_t)((rx_head + 1) % CLI_RX_RING_SIZE);
    if (next != rx_tail)
    {
        rx_ring[rx_head] = rx_byte;
        rx_head = next;
    }
    HAL_UART_Receive_IT(cli_uart, &rx_byte, 1);
}

void CLI_printf(const char* format, ...)
{
    char buffer[192];
    va_list args;

    va_start(args, format);
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length > 0)
    {
        if (length > (int)sizeof(buffer))
        {
            length = sizeof(buffer);
        }
        HAL_UART_Transmit(cli_uart, (uint8_t*)buffer, (uint16_t)length,
                          CLI_TX_TIMEOUT_MS);
    }
}

// prints a float as a fixed-point decimal without pulling in printf-float
static void print_fixed(const char* label, float value, const char* unit)
{
    int32_t scaled = (int32_t)(value >= 0 ? value * 100.0f + 0.5f
                                          : value * 100.0f - 0.5f);
    int32_t whole = scaled / 100;
    int32_t frac = scaled % 100;
    if (frac < 0)
    {
        frac = -frac;
    }
    if (scaled < 0 && whole == 0)
    {
        CLI_printf("%s-0.%02ld%s", label, (long)frac, unit);
    }
    else
    {
        CLI_printf("%s%ld.%02ld%s", label, (long)whole, (long)frac, unit);
    }
}

static int parse_channel(const char* token)
{
    if (token == NULL)
    {
        return -1;
    }
    if (token[1] == '\0')
    {
        char c = (char)tolower((unsigned char)token[0]);
        if (c == 'a' || c == '0')
        {
            return 0;
        }
        if (c == 'b' || c == '1')
        {
            return 1;
        }
    }
    return -1;
}

static const char* channel_name(int channel)
{
    return channel == 0 ? "A" : "B";
}

// commands that edit quantizer state only make sense in that personality
static uint8_t require_quantizer(void)
{
    if (APP_personality() == APP_PERSONALITY_QUANTIZER)
    {
        return 1;
    }
    CLI_printf("quantizer personality not active (see 'mode')\r\n");
    return 0;
}

static uint8_t require_sequencer(void)
{
    if (APP_personality() == APP_PERSONALITY_SEQUENCER)
    {
        return 1;
    }
    CLI_printf("sequencer personality not active (see 'mode')\r\n");
    return 0;
}

static const char* mode_override_name(uint8_t override)
{
    switch (override)
    {
        case SETTINGS_MODE_QUANTIZER:
            return "quantizer";
        case SETTINGS_MODE_SEQUENCER:
            return "sequencer";
        default:
            return "auto";
    }
}

static void cmd_help(void)
{
    CLI_printf(
        "commands (ch = a|b):\r\n"
        "  status               overview of both channels\r\n"
        "  adc                  averaged ADC codes + input volts\r\n"
        "  dac <ch> <code|off>  force a raw DAC code / release\r\n"
        "  mask <ch> <hex>      12-bit note mask, bit0 = C\r\n"
        "  scale <ch> <name>    preset mask (scale <ch> ? lists)\r\n"
        "  root <ch> <0-11>     rotate mask to a root note\r\n"
        "  trans <ch> <n>       transpose output, semitones (+-24)\r\n"
        "  trig <a|b>           fire a manual trigger-out pulse\r\n"
        "  ctl                  dump switches/buttons/encoder\r\n"
        "  led test <on|off>    take/release LED control\r\n"
        "  led <idx> <mode>     mode = on|off|slow|fast (test mode)\r\n"
        "  cal show             calibration constants\r\n"
        "  cal adc <ch> <mv>    capture ADC point at known input\r\n"
        "  cal dac <ch> <code> <mv> capture DAC point (measured)\r\n"
        "  cal reset            drop pending calibration points\r\n"
        "  mode [quantizer|sequencer|auto] personality override\r\n"
        "  reset                reboot (applies a mode change)\r\n"
        "sequencer personality:\r\n"
        "  bpm [30-300]         internal clock tempo\r\n"
        "  run | stop           internal clock transport\r\n"
        "  seq status           engine state\r\n"
        "  seq load <ch> <n> [now]  arm a sequence (default: next phrase)\r\n"
        "  seq reset            restart the phrase on the next pulse\r\n"
        "  i2c [instance <0-3>] conductor bus address + stats\r\n"
        "  save | load | defaults | ver\r\n");
}

static void cmd_status(void)
{
    struct SETTINGS_data* settings = SETTINGS_get();

    CLI_printf("quantizer-v2 %s — personality: %s (strap=%s, override=%s)\r\n",
               APP_VERSION,
               APP_personality() == APP_PERSONALITY_SEQUENCER ? "sequencer"
                                                              : "quantizer",
               APP_mode_strap() ? "sequencer" : "quantizer",
               mode_override_name(settings->mode_override));

    if (APP_personality() == APP_PERSONALITY_QUANTIZER)
    {
        CLI_printf("active channel %s\r\n", channel_name(APP_active_channel()));
        for (int ch = 0; ch < 2; ch++)
        {
            int16_t note = QUANT_APP_last_note((uint8_t)ch);
            int32_t override = APP_dac_override((uint8_t)ch);

            CLI_printf("%s: mode=%s mask=%03X root=%d trans=%+d note=",
                       channel_name(ch),
                       TRIGGERS_jack_present((uint8_t)ch) ? "trig" : "cont",
                       settings->note_mask[ch], settings->root[ch],
                       settings->transpose[ch]);
            if (note == QUANTIZE_NO_NOTE)
            {
                CLI_printf("-");
            }
            else
            {
                CLI_printf("%d", note);
            }
            CLI_printf(" dac=%u", DAC_get_code((uint8_t)ch));
            if (override >= 0)
            {
                CLI_printf(" (override)");
            }
            CLI_printf("\r\n");
        }
    }
    else
    {
        static const char* const directions[] = {"fwd", "rev", "pp", "rand"};
        struct SEQ_engine* engine = SEQ_APP_engine();

        CLI_printf("clock=%s run=%u bpm=%u bar=%u beat=%u dir=%s\r\n",
                   SEQ_APP_external_clock() ? "ext" : "int", SEQ_APP_running(),
                   settings->seq_bpm, SEQ_bar(engine) + 1, SEQ_beat(engine) + 1,
                   directions[engine->direction & 3]);
        for (int lane = 0; lane < SEQ_LANES; lane++)
        {
            const struct SEQ_lane* state = &engine->lanes[lane];
            CLI_printf("%s: #%u \"%s\"%s mute=%u trans=%+d oct=%+d dac=%u\r\n",
                       channel_name(lane), settings->seq_loaded[lane],
                       state->seq != NULL ? state->seq->name : "-",
                       SEQ_sequence_armed(engine, (uint8_t)lane) ? " (armed)"
                                                                 : "",
                       state->mute, state->transpose, state->octave,
                       DAC_get_code((uint8_t)lane));
        }
    }

    CLI_printf("jack A=%d B=%d, trig-in gate A=%d B=%d\r\n",
               TRIGGERS_jack_present(0), TRIGGERS_jack_present(1),
               TRIGGERS_gate(0), TRIGGERS_gate(1));
    CLI_printf("settings: %s%s\r\n",
               SETTINGS_loaded_from_flash() ? "flash" : "defaults",
               SETTINGS_is_dirty() ? " (dirty)" : "");
}

static void cmd_adc(void)
{
    struct SETTINGS_data* settings = SETTINGS_get();
    for (int ch = 0; ch < 2; ch++)
    {
        float code = CV_IN_code((uint8_t)ch);
        float volts = CV_IN_volts((uint8_t)ch, &settings->cal[ch]);
        int32_t mv = (int32_t)(volts >= 0 ? volts * 1000.0f + 0.5f
                                          : volts * 1000.0f - 0.5f);
        CLI_printf("%s: code=%ld in=%ldmV\r\n", channel_name(ch),
                   (long)(code + 0.5f), (long)mv);
    }
}

static void cmd_dac(const char* ch_token, const char* value_token)
{
    int channel = parse_channel(ch_token);
    if (channel < 0 || value_token == NULL)
    {
        CLI_printf("usage: dac <a|b> <0-4095|off>\r\n");
        return;
    }
    if (strcmp(value_token, "off") == 0)
    {
        APP_set_dac_override((uint8_t)channel, -1);
        CLI_printf("dac %s override released\r\n", channel_name(channel));
        return;
    }

    long code = strtol(value_token, NULL, 0);
    if (code < 0 || code > DAC_CODE_MAX)
    {
        CLI_printf("code out of range (0-4095)\r\n");
        return;
    }
    APP_set_dac_override((uint8_t)channel, code);
    CLI_printf("dac %s = %ld (override on, 'dac %s off' to release)\r\n",
               channel_name(channel), code, ch_token);
}

static void cmd_mask(const char* ch_token, const char* mask_token)
{
    int channel = parse_channel(ch_token);
    if (channel < 0 || mask_token == NULL)
    {
        CLI_printf("usage: mask <a|b> <hex, e.g. FFF or AB5>\r\n");
        return;
    }
    long mask = strtol(mask_token, NULL, 16);
    if (mask < 0 || mask > 0x0FFF)
    {
        CLI_printf("mask out of range (000-FFF)\r\n");
        return;
    }
    QUANT_APP_apply_mask((uint8_t)channel, (uint16_t)mask);
    SETTINGS_mark_dirty();
    CLI_printf("mask %s = %03lX\r\n", channel_name(channel), mask);
}

static void cmd_scale(const char* ch_token, const char* name_token)
{
    int channel = parse_channel(ch_token);
    if (channel < 0 || name_token == NULL || strcmp(name_token, "?") == 0)
    {
        CLI_printf("scales:");
        for (unsigned i = 0; i < QUANTIZE_SCALES_COUNT; i++)
        {
            CLI_printf(" %s", QUANTIZE_scales[i].name);
        }
        CLI_printf("\r\nusage: scale <a|b> <name>\r\n");
        return;
    }
    for (unsigned i = 0; i < QUANTIZE_SCALES_COUNT; i++)
    {
        if (strcmp(name_token, QUANTIZE_scales[i].name) == 0)
        {
            QUANT_APP_apply_mask((uint8_t)channel, QUANTIZE_scales[i].mask);
            SETTINGS_mark_dirty();
            CLI_printf("scale %s = %s (mask %03X)\r\n", channel_name(channel),
                       QUANTIZE_scales[i].name, QUANTIZE_scales[i].mask);
            return;
        }
    }
    CLI_printf("unknown scale '%s' (scale %s ? lists)\r\n", name_token,
               ch_token);
}

static void cmd_root(const char* ch_token, const char* value_token)
{
    int channel = parse_channel(ch_token);
    if (channel < 0 || value_token == NULL)
    {
        CLI_printf("usage: root <a|b> <0-11>\r\n");
        return;
    }
    long root = strtol(value_token, NULL, 10);
    if (root < 0 || root > 11)
    {
        CLI_printf("root out of range (0-11)\r\n");
        return;
    }
    QUANT_APP_apply_root((uint8_t)channel, (int16_t)root);
    SETTINGS_mark_dirty();
    CLI_printf("root %s = %ld\r\n", channel_name(channel), root);
}

static void cmd_trans(const char* ch_token, const char* value_token)
{
    int channel = parse_channel(ch_token);
    if (channel < 0 || value_token == NULL)
    {
        CLI_printf("usage: trans <a|b> <-24..24>\r\n");
        return;
    }
    long transpose = strtol(value_token, NULL, 10);
    if (transpose < APP_TRANSPOSE_MIN || transpose > APP_TRANSPOSE_MAX)
    {
        CLI_printf("transpose out of range (%+d..%+d)\r\n", APP_TRANSPOSE_MIN,
                   APP_TRANSPOSE_MAX);
        return;
    }
    QUANT_APP_apply_transpose((uint8_t)channel, (int8_t)transpose);
    SETTINGS_mark_dirty();
    CLI_printf("trans %s = %+ld\r\n", channel_name(channel), transpose);
}

static void cmd_ctl(void)
{
    CLI_printf("raw=%06lX stable:", (unsigned long)SWITCHES_raw());
    for (uint8_t i = 0; i < SW_COUNT; i++)
    {
        if (SWITCHES_state(i))
        {
            if (i < 12)
            {
                CLI_printf(" note%u", i);
            }
            else if (i < 16)
            {
                CLI_printf(" spare%u", i - 12);
            }
            else if (i <= SW_BTN_D)
            {
                CLI_printf(" btn%c", 'A' + (i - SW_BTN_A));
            }
            else
            {
                CLI_printf(" enc");
            }
        }
    }
    CLI_printf("\r\nencoder position=%ld\r\n", (long)ENCODER_position());
}

static void cmd_led(const char* arg1, const char* arg2)
{
    if (arg1 != NULL && strcmp(arg1, "test") == 0)
    {
        if (arg2 != NULL && strcmp(arg2, "on") == 0)
        {
            APP_set_led_test(1);
            CLI_printf("led test mode on ('led test off' to release)\r\n");
        }
        else if (arg2 != NULL && strcmp(arg2, "off") == 0)
        {
            APP_set_led_test(0);
            CLI_printf("led test mode off\r\n");
        }
        else
        {
            CLI_printf("usage: led test <on|off>\r\n");
        }
        return;
    }

    if (arg1 == NULL || arg2 == NULL)
    {
        CLI_printf("usage: led test <on|off> | led <0-%d> <on|off|slow|fast>\r\n",
                   LEDS_COUNT - 1);
        return;
    }
    if (!APP_led_test())
    {
        CLI_printf("enable 'led test on' first\r\n");
        return;
    }

    long index = strtol(arg1, NULL, 10);
    if (index < 0 || index >= LEDS_COUNT)
    {
        CLI_printf("led index out of range (0-%d)\r\n", LEDS_COUNT - 1);
        return;
    }

    enum LEDS_mode mode;
    if (strcmp(arg2, "on") == 0)
    {
        mode = LEDS_ON;
    }
    else if (strcmp(arg2, "off") == 0)
    {
        mode = LEDS_OFF;
    }
    else if (strcmp(arg2, "slow") == 0)
    {
        mode = LEDS_BLINK_SLOW;
    }
    else if (strcmp(arg2, "fast") == 0)
    {
        mode = LEDS_BLINK_FAST;
    }
    else
    {
        CLI_printf("unknown mode '%s'\r\n", arg2);
        return;
    }
    LEDS_set((uint8_t)index, mode);
    CLI_printf("led %ld %s\r\n", index, arg2);
}

static void cal_show(void)
{
    struct SETTINGS_data* settings = SETTINGS_get();
    for (int ch = 0; ch < 2; ch++)
    {
        const struct SETTINGS_cal* cal = &settings->cal[ch];
        CLI_printf("%s adc: ", channel_name(ch));
        print_fixed("offset=", cal->adc_offset_v, "V ");
        print_fixed("slope=", cal->adc_v_per_lsb * 1000.0f, "mV/lsb");
        CLI_printf("\r\n%s dac: ", channel_name(ch));
        print_fixed("offset=", cal->dac_offset_code, " codes ");
        print_fixed("gain=", cal->dac_code_per_v, " codes/V");
        CLI_printf("\r\n");
    }
    for (int ch = 0; ch < 2; ch++)
    {
        if (cal_adc_points[ch].count == 1)
        {
            CLI_printf("pending: adc %s point 1 captured\r\n", channel_name(ch));
        }
        if (cal_dac_points[ch].count == 1)
        {
            CLI_printf("pending: dac %s point 1 captured\r\n", channel_name(ch));
        }
    }
}

static void cal_adc(const char* ch_token, const char* mv_token)
{
    int channel = parse_channel(ch_token);
    if (channel < 0 || mv_token == NULL)
    {
        CLI_printf("usage: cal adc <a|b> <input mV>  (twice, two voltages)\r\n");
        return;
    }

    struct CLI_cal_points* points = &cal_adc_points[channel];
    long mv = strtol(mv_token, NULL, 10);

    points->code[points->count] = CV_IN_code((uint8_t)channel);
    points->volts[points->count] = (float)mv / 1000.0f;
    points->count++;

    if (points->count < 2)
    {
        CLI_printf("adc %s point 1 @ %ldmV captured, apply a second voltage\r\n",
                   channel_name(channel), mv);
        return;
    }
    points->count = 0;

    float code_delta = points->code[1] - points->code[0];
    if (code_delta > -20.0f && code_delta < 20.0f)
    {
        CLI_printf("points too close together, calibration dropped\r\n");
        return;
    }

    struct SETTINGS_cal* cal = &SETTINGS_get()->cal[channel];
    cal->adc_v_per_lsb =
        (points->volts[1] - points->volts[0]) / code_delta;
    cal->adc_offset_v = points->volts[0] - cal->adc_v_per_lsb * points->code[0];
    SETTINGS_mark_dirty();
    QUANT_APP_reload_from_settings();

    CLI_printf("adc %s calibrated: ", channel_name(channel));
    print_fixed("offset=", cal->adc_offset_v, "V ");
    print_fixed("slope=", cal->adc_v_per_lsb * 1000.0f, "mV/lsb\r\n");
}

static void cal_dac(const char* ch_token, const char* code_token,
                    const char* mv_token)
{
    int channel = parse_channel(ch_token);
    if (channel < 0 || code_token == NULL || mv_token == NULL)
    {
        CLI_printf("usage: cal dac <a|b> <code> <measured mV>  (twice)\r\n");
        return;
    }

    struct CLI_cal_points* points = &cal_dac_points[channel];
    long code = strtol(code_token, NULL, 10);
    long mv = strtol(mv_token, NULL, 10);

    points->code[points->count] = (float)code;
    points->volts[points->count] = (float)mv / 1000.0f;
    points->count++;

    if (points->count < 2)
    {
        CLI_printf("dac %s point 1 (code %ld = %ldmV) captured\r\n",
                   channel_name(channel), code, mv);
        return;
    }
    points->count = 0;

    float volt_delta = points->volts[1] - points->volts[0];
    if (volt_delta > -0.1f && volt_delta < 0.1f)
    {
        CLI_printf("points too close together, calibration dropped\r\n");
        return;
    }

    struct SETTINGS_cal* cal = &SETTINGS_get()->cal[channel];
    cal->dac_code_per_v = (points->code[1] - points->code[0]) / volt_delta;
    cal->dac_offset_code =
        points->code[0] - cal->dac_code_per_v * points->volts[0];
    SETTINGS_mark_dirty();
    QUANT_APP_reload_from_settings();

    CLI_printf("dac %s calibrated: ", channel_name(channel));
    print_fixed("offset=", cal->dac_offset_code, " codes ");
    print_fixed("gain=", cal->dac_code_per_v, " codes/V\r\n");
}

static void execute_line(void)
{
    char* saveptr = NULL;
    char* command = strtok_r(line, " ", &saveptr);

    if (command == NULL)
    {
        return;
    }
    for (char* c = command; *c; c++)
    {
        *c = (char)tolower((unsigned char)*c);
    }

    if (strcmp(command, "help") == 0)
    {
        cmd_help();
    }
    else if (strcmp(command, "ver") == 0)
    {
        CLI_printf("quantizer-v2 %s (built %s %s)\r\n", APP_VERSION, __DATE__,
                   __TIME__);
    }
    else if (strcmp(command, "status") == 0)
    {
        cmd_status();
    }
    else if (strcmp(command, "adc") == 0)
    {
        cmd_adc();
    }
    else if (strcmp(command, "dac") == 0)
    {
        cmd_dac(strtok_r(NULL, " ", &saveptr), strtok_r(NULL, " ", &saveptr));
    }
    else if (strcmp(command, "mask") == 0)
    {
        if (require_quantizer())
        {
            cmd_mask(strtok_r(NULL, " ", &saveptr),
                     strtok_r(NULL, " ", &saveptr));
        }
    }
    else if (strcmp(command, "scale") == 0)
    {
        if (require_quantizer())
        {
            cmd_scale(strtok_r(NULL, " ", &saveptr),
                      strtok_r(NULL, " ", &saveptr));
        }
    }
    else if (strcmp(command, "root") == 0)
    {
        if (require_quantizer())
        {
            cmd_root(strtok_r(NULL, " ", &saveptr),
                     strtok_r(NULL, " ", &saveptr));
        }
    }
    else if (strcmp(command, "trans") == 0)
    {
        if (require_quantizer())
        {
            cmd_trans(strtok_r(NULL, " ", &saveptr),
                      strtok_r(NULL, " ", &saveptr));
        }
    }
    else if (strcmp(command, "mode") == 0)
    {
        char* value = strtok_r(NULL, " ", &saveptr);
        struct SETTINGS_data* settings = SETTINGS_get();
        if (value == NULL)
        {
            CLI_printf("personality=%s strap=%s override=%s\r\n",
                       APP_personality() == APP_PERSONALITY_SEQUENCER
                           ? "sequencer"
                           : "quantizer",
                       APP_mode_strap() ? "sequencer" : "quantizer",
                       mode_override_name(settings->mode_override));
        }
        else if (strcmp(value, "quantizer") == 0 ||
                 strcmp(value, "sequencer") == 0 || strcmp(value, "auto") == 0)
        {
            settings->mode_override = value[0] == 'q' ? SETTINGS_MODE_QUANTIZER
                                      : value[0] == 's'
                                          ? SETTINGS_MODE_SEQUENCER
                                          : SETTINGS_MODE_AUTO;
            if (SETTINGS_save() == HAL_OK)
            {
                CLI_printf("override=%s saved — 'reset' or power-cycle to "
                           "apply\r\n",
                           mode_override_name(settings->mode_override));
            }
            else
            {
                CLI_printf("flash write failed\r\n");
            }
        }
        else
        {
            CLI_printf("usage: mode [quantizer|sequencer|auto]\r\n");
        }
    }
    else if (strcmp(command, "reset") == 0)
    {
        CLI_printf("rebooting...\r\n");
        HAL_Delay(20);
        NVIC_SystemReset();
    }
    else if (strcmp(command, "bpm") == 0)
    {
        if (require_sequencer())
        {
            char* value = strtok_r(NULL, " ", &saveptr);
            struct SETTINGS_data* settings = SETTINGS_get();
            if (value == NULL)
            {
                CLI_printf("bpm = %u (%s clock)\r\n", settings->seq_bpm,
                           SEQ_APP_external_clock() ? "external" : "internal");
            }
            else
            {
                long bpm = strtol(value, NULL, 10);
                if (bpm < SEQ_APP_BPM_MIN || bpm > SEQ_APP_BPM_MAX)
                {
                    CLI_printf("bpm out of range (%d-%d)\r\n", SEQ_APP_BPM_MIN,
                               SEQ_APP_BPM_MAX);
                }
                else
                {
                    settings->seq_bpm = (uint16_t)bpm;
                    SETTINGS_mark_dirty();
                    CLI_printf("bpm = %u\r\n", settings->seq_bpm);
                }
            }
        }
    }
    else if (strcmp(command, "run") == 0 || strcmp(command, "stop") == 0)
    {
        if (require_sequencer())
        {
            SEQ_APP_set_running(command[0] == 'r');
            if (SEQ_APP_external_clock())
            {
                CLI_printf("%s (note: external clock overrides run/stop)\r\n",
                           command);
            }
            else
            {
                CLI_printf("%s\r\n", command);
            }
        }
    }
    else if (strcmp(command, "seq") == 0)
    {
        if (require_sequencer())
        {
            char* sub = strtok_r(NULL, " ", &saveptr);
            if (sub != NULL && strcmp(sub, "status") == 0)
            {
                cmd_status();
            }
            else if (sub != NULL && strcmp(sub, "load") == 0)
            {
                int channel = parse_channel(strtok_r(NULL, " ", &saveptr));
                char* index_token = strtok_r(NULL, " ", &saveptr);
                char* now_token = strtok_r(NULL, " ", &saveptr);
                uint8_t now =
                    now_token != NULL && strcmp(now_token, "now") == 0;

                if (channel < 0 || index_token == NULL)
                {
                    CLI_printf("usage: seq load <a|b> <n> [now]\r\n");
                }
                else
                {
                    long index = strtol(index_token, NULL, 10);
                    if (index < 0 ||
                        !SEQ_APP_load((uint8_t)channel, (uint16_t)index,
                                      now ? SEQ_QUANT_NOW : SEQ_QUANT_PHRASE))
                    {
                        CLI_printf("bank %s has %u sequences (0-%u)\r\n",
                                   channel_name(channel),
                                   SEQ_APP_bank_count((uint8_t)channel),
                                   SEQ_APP_bank_count((uint8_t)channel) - 1);
                    }
                    else
                    {
                        CLI_printf(
                            "%s: #%ld \"%s\" %s\r\n", channel_name(channel),
                            index,
                            SEQ_APP_bank_sequence((uint8_t)channel,
                                                  (uint16_t)index)
                                ->name,
                            now ? "loaded" : "armed for the next phrase");
                    }
                }
            }
            else if (sub != NULL && strcmp(sub, "reset") == 0)
            {
                SEQ_reset(SEQ_APP_engine());
                CLI_printf("phrase restarts on the next pulse\r\n");
            }
            else
            {
                CLI_printf("usage: seq <status|load|reset>\r\n");
            }
        }
    }
    else if (strcmp(command, "i2c") == 0)
    {
        if (require_sequencer())
        {
            char* sub = strtok_r(NULL, " ", &saveptr);
            if (sub == NULL)
            {
                const struct I2C_SLAVE_stats* stats = I2C_SLAVE_get_stats();
                CLI_printf("addr=0x%02X (instance %u, +broadcast 0x%02X)\r\n",
                           I2C_SLAVE_address(), SETTINGS_get()->i2c_instance,
                           I2C_SLAVE_BROADCAST);
                CLI_printf(
                    "applied=%lu dup=%lu bad=%lu unknown=%lu buserr=%lu\r\n",
                    (unsigned long)stats->applied,
                    (unsigned long)stats->duplicates,
                    (unsigned long)stats->bad_frames,
                    (unsigned long)stats->unknown,
                    (unsigned long)stats->bus_errors);
            }
            else if (strcmp(sub, "instance") == 0)
            {
                char* value = strtok_r(NULL, " ", &saveptr);
                long instance = value != NULL ? strtol(value, NULL, 10) : -1;
                if (instance < 0 || instance > 3)
                {
                    CLI_printf("usage: i2c instance <0-3>\r\n");
                }
                else
                {
                    SETTINGS_get()->i2c_instance = (uint8_t)instance;
                    SETTINGS_mark_dirty();
                    I2C_SLAVE_init(APP_i2c()); // re-address live
                    CLI_printf("instance %ld: listening on 0x%02X\r\n",
                               instance, I2C_SLAVE_address());
                }
            }
            else
            {
                CLI_printf("usage: i2c [instance <0-3>]\r\n");
            }
        }
    }
    else if (strcmp(command, "trig") == 0)
    {
        int channel = parse_channel(strtok_r(NULL, " ", &saveptr));
        if (channel < 0)
        {
            CLI_printf("usage: trig <a|b>\r\n");
        }
        else
        {
            TRIGGERS_pulse((uint8_t)channel);
            CLI_printf("trig %s pulsed (%d ms, jack should read ~8V)\r\n",
                       channel_name(channel), TRIGGERS_PULSE_MS);
        }
    }
    else if (strcmp(command, "ctl") == 0)
    {
        cmd_ctl();
    }
    else if (strcmp(command, "led") == 0)
    {
        cmd_led(strtok_r(NULL, " ", &saveptr), strtok_r(NULL, " ", &saveptr));
    }
    else if (strcmp(command, "cal") == 0)
    {
        char* sub = strtok_r(NULL, " ", &saveptr);
        if (sub != NULL && strcmp(sub, "show") == 0)
        {
            cal_show();
        }
        else if (sub != NULL && strcmp(sub, "adc") == 0)
        {
            cal_adc(strtok_r(NULL, " ", &saveptr),
                    strtok_r(NULL, " ", &saveptr));
        }
        else if (sub != NULL && strcmp(sub, "dac") == 0)
        {
            cal_dac(strtok_r(NULL, " ", &saveptr),
                    strtok_r(NULL, " ", &saveptr),
                    strtok_r(NULL, " ", &saveptr));
        }
        else if (sub != NULL && strcmp(sub, "reset") == 0)
        {
            memset(cal_adc_points, 0, sizeof(cal_adc_points));
            memset(cal_dac_points, 0, sizeof(cal_dac_points));
            CLI_printf("pending calibration points dropped\r\n");
        }
        else
        {
            CLI_printf("usage: cal <show|adc|dac|reset>\r\n");
        }
    }
    else if (strcmp(command, "save") == 0)
    {
        CLI_printf(SETTINGS_save() == HAL_OK ? "settings saved\r\n"
                                             : "flash write failed\r\n");
    }
    else if (strcmp(command, "load") == 0)
    {
        SETTINGS_init();
        QUANT_APP_reload_from_settings();
        CLI_printf("settings reloaded from %s\r\n",
                   SETTINGS_loaded_from_flash() ? "flash" : "defaults");
    }
    else if (strcmp(command, "defaults") == 0)
    {
        SETTINGS_load_defaults();
        QUANT_APP_reload_from_settings();
        CLI_printf("defaults applied (RAM only, 'save' to persist)\r\n");
    }
    else
    {
        CLI_printf("unknown command '%s', try 'help'\r\n", command);
    }
}

void CLI_tick(void)
{
    while (rx_tail != rx_head)
    {
        char c = (char)rx_ring[rx_tail];
        rx_tail = (uint16_t)((rx_tail + 1) % CLI_RX_RING_SIZE);

        if (c == '\r' || c == '\n')
        {
            CLI_printf("\r\n");
            line[line_length] = '\0';
            if (line_length > 0)
            {
                execute_line();
            }
            line_length = 0;
            CLI_printf("> ");
        }
        else if (c == '\b' || c == 0x7F)
        {
            if (line_length > 0)
            {
                line_length--;
                CLI_printf("\b \b");
            }
        }
        else if (c >= 0x20 && c < 0x7F && line_length < CLI_LINE_SIZE - 1)
        {
            line[line_length++] = c;
            CLI_printf("%c", c);
        }
    }
}
