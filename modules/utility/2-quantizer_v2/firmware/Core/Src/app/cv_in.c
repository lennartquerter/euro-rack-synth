/*
******************************************************************************
* @file           : cv_in.c
* @author         : Lennart Querter
* @brief          : ADC oversampling/averaging and volts conversion
******************************************************************************
*/

#include "app/cv_in.h"

/*
 * DMA ring, interleaved A,B,A,B,... The ADC free-runs (continuous scan),
 * so reads just average the ring; a sample being overwritten mid-average
 * only mixes in an adjacent conversion, which the averaging absorbs.
 */
static volatile uint16_t cv_dma_buffer[CV_IN_CHANNELS * CV_IN_OVERSAMPLE];

void CV_IN_init(ADC_HandleTypeDef* hadc)
{
    HAL_ADC_Start_DMA(hadc, (uint32_t*)cv_dma_buffer,
                      CV_IN_CHANNELS * CV_IN_OVERSAMPLE);
}

float CV_IN_code(uint8_t channel)
{
    uint32_t sum = 0;
    for (uint32_t i = 0; i < CV_IN_OVERSAMPLE; i++)
    {
        sum += cv_dma_buffer[i * CV_IN_CHANNELS + (channel & 1)];
    }
    return (float)sum / (float)CV_IN_OVERSAMPLE;
}

float CV_IN_volts(uint8_t channel, const struct SETTINGS_cal* cal)
{
    return cal->adc_offset_v + cal->adc_v_per_lsb * CV_IN_code(channel);
}
