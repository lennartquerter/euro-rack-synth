//
// Created by Lennart Querter on 02.02.25.
//

#include "app/adc.h"

static uint16_t adc1_buffer[5];  // 5 channels as per NbrOfConversion
static uint16_t adc2_buffer[5];  // 5 channels as per NbrOfConversion

void ADC_init(ADC_config* config)
{
    // Start ADCs with DMA
    HAL_ADC_Start_DMA(&config->adc1_handler, adc1_buffer, 5);
    HAL_ADC_Start_DMA(&config->adc2_handler, adc2_buffer, 5);
}

float normalize_adc(uint16_t adc_value) {
    return (float)adc_value / 4095.0f;
}

REVERB_parameters ADC_fetch_parameters() {
    REVERB_parameters new_parameters = {0};

    // Apply calibration and scaling
    new_parameters.v_oct = normalize_adc(adc1_buffer[PARAM_V_OCT]);
    new_parameters.position = normalize_adc(adc1_buffer[PARAM_POSITION]);
    new_parameters.density = normalize_adc(adc1_buffer[PARAM_DENSITY]);
    new_parameters.pitch = normalize_adc(adc1_buffer[PARAM_PITCH]);
    new_parameters.blend_cv = normalize_adc(adc1_buffer[PARAM_BLEND_CV]);

    new_parameters.texture = normalize_adc(adc2_buffer[PARAM_TEXTURE]);
    new_parameters.texture_cv = normalize_adc(adc2_buffer[PARAM_TEXTURE_CV]);
    new_parameters.size = normalize_adc(adc2_buffer[PARAM_SIZE]);
    new_parameters.size_cv = normalize_adc(adc2_buffer[PARAM_SIZE_CV]);
    new_parameters.blend = normalize_adc(adc2_buffer[PARAM_BLEND]);
    
    return new_parameters;
}

static inline float smooth_parameter(float current, float new_value, float factor)
{
    return current * (1.0f - factor) + new_value * factor;
}

void ADC_smooth_parameters(REVERB_parameters new_params, REVERB_parameters* current_params)
{
    static REVERB_parameters smooth_buffer = {0};

    // Direct controls with faster response
    smooth_buffer.position = smooth_parameter(smooth_buffer.position,
                                            new_params.position,
                                            SMOOTH_FACTOR_FAST);
    smooth_buffer.density = smooth_parameter(smooth_buffer.density,
                                           new_params.density,
                                           SMOOTH_FACTOR_FAST);
    smooth_buffer.blend = smooth_parameter(smooth_buffer.blend,
                                         new_params.blend,
                                         SMOOTH_FACTOR_FAST);

    // CV inputs with slower response
    smooth_buffer.v_oct = smooth_parameter(smooth_buffer.v_oct,
                                         new_params.v_oct,
                                         SMOOTH_FACTOR_SLOW);
    smooth_buffer.texture_cv = smooth_parameter(smooth_buffer.texture_cv,
                                              new_params.texture_cv,
                                              SMOOTH_FACTOR_SLOW);
    smooth_buffer.blend_cv = smooth_parameter(smooth_buffer.blend_cv,
                                            new_params.blend_cv,
                                            SMOOTH_FACTOR_SLOW);

    // Medium response for others
    smooth_buffer.pitch = smooth_parameter(smooth_buffer.pitch,
                                         new_params.pitch,
                                         SMOOTH_FACTOR_MED);
    smooth_buffer.texture = smooth_parameter(smooth_buffer.texture,
                                           new_params.texture,
                                           SMOOTH_FACTOR_MED);
    smooth_buffer.size = smooth_parameter(smooth_buffer.size,
                                        new_params.size,
                                        SMOOTH_FACTOR_MED);
    smooth_buffer.size_cv = smooth_parameter(smooth_buffer.size_cv,
                                           new_params.size_cv,
                                           SMOOTH_FACTOR_MED);

    // Copy smoothed values to output parameters
    *current_params = smooth_buffer;
}
