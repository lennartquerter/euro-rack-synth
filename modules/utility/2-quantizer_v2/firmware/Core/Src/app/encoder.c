/*
******************************************************************************
* @file           : encoder.c
* @author         : Lennart Querter
* @brief          : TIM3 quadrature decoding, detent conversion, acceleration
******************************************************************************
*/

#include "app/encoder.h"

static TIM_HandleTypeDef* enc_tim;
static uint16_t last_count;
static int32_t count_remainder;
static int32_t position;
static uint32_t last_detent_ms;

void ENCODER_init(TIM_HandleTypeDef* htim)
{
    enc_tim = htim;
    count_remainder = 0;
    position = 0;
    last_detent_ms = 0;

    HAL_TIM_Encoder_Start(enc_tim, TIM_CHANNEL_ALL);
    last_count = (uint16_t)__HAL_TIM_GET_COUNTER(enc_tim);
}

int32_t ENCODER_poll(void)
{
    uint16_t count = (uint16_t)__HAL_TIM_GET_COUNTER(enc_tim);
    // int16 wrap-around diff handles the 16-bit counter rollover
    int16_t diff = (int16_t)(count - last_count);
    last_count = count;

    count_remainder += diff;
    int32_t detents = count_remainder / ENCODER_COUNTS_PER_DETENT;
    count_remainder -= detents * ENCODER_COUNTS_PER_DETENT;

    if (detents == 0)
    {
        return 0;
    }
    position += detents;

    uint32_t now = HAL_GetTick();
    uint32_t interval = now - last_detent_ms;
    last_detent_ms = now;

    if (interval < ENCODER_ACCEL_FAST_MS)
    {
        detents *= 4;
    }
    else if (interval < ENCODER_ACCEL_MED_MS)
    {
        detents *= 2;
    }
    return detents;
}

int32_t ENCODER_position(void)
{
    return position;
}
