/*
******************************************************************************
* @file           : encoder.h
* @author         : Lennart Querter
* @brief          : Header for encoder.c file.
*                   Rotary encoder on TIM3 (encoder mode TI1+TI2). One
*                   detent = 4 quadrature counts. Turning faster than the
*                   acceleration thresholds multiplies the step.
*                   The encoder push switch is handled by switches.c.
******************************************************************************
*/

#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f4xx_hal.h"

#define ENCODER_COUNTS_PER_DETENT 4

// detent-to-detent intervals faster than this accelerate the step
#define ENCODER_ACCEL_FAST_MS 30 // x4
#define ENCODER_ACCEL_MED_MS 80  // x2

void ENCODER_init(TIM_HandleTypeDef* htim);

// detents turned since the last poll, acceleration applied; call at 1 kHz
int32_t ENCODER_poll(void);

// accumulated detent position since boot (for the CLI)
int32_t ENCODER_position(void);

#endif
