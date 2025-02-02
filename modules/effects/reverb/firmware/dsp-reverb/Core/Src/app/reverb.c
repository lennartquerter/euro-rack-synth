//
// Created by Lennart Querter on 02.02.25.
//

#include "app/reverb.h"

void REVERB_Init(
    REVERB_config* config,
    GPIO_TypeDef* size_port,
    uint16_t size_pin,
    GPIO_TypeDef* texture_port,
    uint16_t texture_pin,
    GPIO_TypeDef* pitch_port,
    uint16_t pitch_pin,
    GPIO_TypeDef* blend_port,
    uint16_t blend_pin,
    GPIO_TypeDef* v_oct_port,
    uint16_t v_oct_pin,
    GPIO_TypeDef* size_cv_port,
    uint16_t size_cv_pin,
    GPIO_TypeDef* blend_cv_port,
    uint16_t blend_cv_pin,
    GPIO_TypeDef* texture_cv_port,
    uint16_t texture_cv_pin,
    GPIO_TypeDef* position_port,
    uint16_t position_pin,
    GPIO_TypeDef* density_port,
    uint16_t density_pin
) {
    config->size = 0;
    config->size_port = size_port;
    config->size_pin = size_pin;

    config->texture = 0;
    config->texture_port = texture_port;
    config->texture_pin = texture_pin;

    config->pitch = 0;
    config->pitch_port = pitch_port;
    config->pitch_pin = pitch_pin;

    config->blend = 0;
    config->blend_port = blend_port;
    config->blend_pin = blend_pin;

    config->v_oct = 0;
    config->v_oct_port = v_oct_port;
    config->v_oct_pin = v_oct_pin;

    config->size_cv = 0;
    config->size_cv_port = size_cv_port;
    config->size_cv_pin = size_cv_pin;

    config->blend_cv = 0;
    config->blend_cv_port = blend_cv_port;
    config->blend_cv_pin = blend_cv_pin;

    config->texture_cv = 0;
    config->texture_cv_port = texture_cv_port;
    config->texture_cv_pin = texture_cv_pin;

    config->position = 0;
    config->position_port = position_port;
    config->position_pin = position_pin;

    config->density = 0;
    config->density_port = density_port;
    config->density_pin = density_pin;
}


void REVERB_run(REVERB_config* config) {

}

// Process the audio channels
void REVERB_process(int32_t* input, int32_t* output, uint16_t offset, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        // Get left and right channels
        int32_t leftIn = input[offset + i * 2];
        int32_t rightIn = input[offset + i * 2 + 1];

        // Your reverb processing here
        int32_t leftOut = leftIn; // Replace with reverb
        int32_t rightOut = rightIn; // Replace with reverb

        // Write to output buffer
        output[offset + i * 2] = leftOut;
        output[offset + i * 2 + 1] = rightOut;
    }
}
