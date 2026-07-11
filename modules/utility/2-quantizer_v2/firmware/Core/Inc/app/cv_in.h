/*
******************************************************************************
* @file           : cv_in.h
* @author         : Lennart Querter
* @brief          : Header for cv_in.c file.
*                   CV inputs A/B via ADC1 scan + circular DMA with x16
*                   oversampling. The analog frontend inverts and attenuates:
*                   Vadc = 1.815V * 1.25 - 0.25 * Vin, so volts come out of
*                   a per-channel linear calibration (see settings.h).
******************************************************************************
*/

#ifndef __CV_IN_H
#define __CV_IN_H

#include "stm32f4xx_hal.h"

#include "app/settings.h"

#define CV_IN_CHANNELS 2
#define CV_IN_OVERSAMPLE 16

#define CV_IN_CHANNEL_A 0
#define CV_IN_CHANNEL_B 1

// starts the free-running ADC scan into the DMA ring buffer
void CV_IN_init(ADC_HandleTypeDef* hadc);

// averaged 12-bit code (0..4095, fractional resolution from oversampling)
float CV_IN_code(uint8_t channel);

// calibrated input voltage
float CV_IN_volts(uint8_t channel, const struct SETTINGS_cal* cal);

#endif
