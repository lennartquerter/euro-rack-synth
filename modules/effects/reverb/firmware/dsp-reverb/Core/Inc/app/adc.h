//
// Created by Lennart Querter on 02.02.25.
//

#ifndef ADC_H
#define ADC_H

#include "stm32g4xx_hal_adc.h"
#include "reverb.h"

// Different smoothing factors for different parameters
#define SMOOTH_FACTOR_FAST   0.1f    // For direct controls
#define SMOOTH_FACTOR_MED    0.05f   // For moderate response
#define SMOOTH_FACTOR_SLOW   0.01f   // For CV inputs

typedef struct {
    ADC_HandleTypeDef* adc1_handler;
    ADC_HandleTypeDef* adc2_handler;
} ADC_config;

typedef enum {
   PARAM_V_OCT = 0,            // ADC1 Channel 1 -- PA0
   PARAM_POSITION,             // ADC1 Channel 2 -- PA1
   PARAM_DENSITY,              // ADC1 Channel 3 -- PA2
   PARAM_PITCH,                // ADC1 Channel 4 -- PA3
   PARAM_BLEND_CV,             // ADC1 Channel 5 -- PB14
} ADC1_Params;


typedef enum {
    PARAM_TEXTURE = 0,         // ADC2 Channel 3 -- PA6
    PARAM_TEXTURE_CV,          // ADC2 Channel 4 -- PA7
    PARAM_SIZE,                // ADC2 Channel 5 -- PC0
    PARAM_SIZE_CV,             // ADC2 Channel 6 -- PC1
    PARAM_BLEND,               // ADC2 Channel 7 -- PC4
} ADC2_Params;

void ADC_init(ADC_config* config);

REVERB_parameters ADC_fetch_parameters();

void ADC_smooth_parameters(REVERB_parameters parameters, REVERB_parameters* reverb_parameters);

#endif //ADC_H
