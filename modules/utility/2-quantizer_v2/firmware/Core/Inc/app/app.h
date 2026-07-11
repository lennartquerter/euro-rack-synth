/*
******************************************************************************
* @file           : app.h
* @author         : Lennart Querter
* @brief          : Header for app.c file.
*                   Shared application core: boot-time personality select
*                   (quantizer/sequencer), main loop scheduler (fast path
*                   every iteration, UI/control work on the 1 kHz TIM2
*                   tick) and the hardware-debug plumbing shared by both
*                   personalities.
******************************************************************************
*/

#ifndef __APP_H
#define __APP_H

#include "stm32f4xx_hal.h"

#define APP_VERSION "0.8.0-s5"

#define APP_CHANNEL_A 0
#define APP_CHANNEL_B 1
#define APP_CHANNELS 2

#define APP_TRANSPOSE_MIN (-24)
#define APP_TRANSPOSE_MAX 24

#define APP_PERSONALITY_QUANTIZER 0
#define APP_PERSONALITY_SEQUENCER 1

struct APP_config
{
    ADC_HandleTypeDef* hadc;
    SPI_HandleTypeDef* hspi_dac;
    SPI_HandleTypeDef* hspi_sw;
    SPI_HandleTypeDef* hspi_led;
    TIM_HandleTypeDef* htim_tick;
    TIM_HandleTypeDef* htim_enc;
    UART_HandleTypeDef* huart_cli;
    I2C_HandleTypeDef* hi2c; // conductor bus (sequencer personality)
};

// the conductor bus handle for personality init
I2C_HandleTypeDef* APP_i2c(void);

void APP_init(struct APP_config* config);

// one main-loop iteration (called forever from main)
void APP_loop(void);

// 1 kHz tick, called from the TIM2 period-elapsed interrupt
void APP_on_tick(void);

// active personality (APP_PERSONALITY_*) and its selection inputs
uint8_t APP_personality(void);
uint8_t APP_mode_strap(void); // raw PB2 read at boot: 1 = sequencer side

/*
 * Shared plumbing used by both personalities and the CLI.
 */
uint8_t APP_active_channel(void);
void APP_set_active_channel(uint8_t channel);

// manual DAC override for testing/calibration: code 0..4095, -1 releases
void APP_set_dac_override(uint8_t channel, int32_t code);
int32_t APP_dac_override(uint8_t channel);

// calibrated volts -> DAC code, clamped to 0-10V; no-op while overridden
void APP_dac_write_volts(uint8_t channel, float volts);

// while set, personalities stop refreshing LEDs so `led` CLI commands stick
void APP_set_led_test(uint8_t on);
uint8_t APP_led_test(void);

#endif
