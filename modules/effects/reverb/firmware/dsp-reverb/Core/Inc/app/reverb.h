//
// Created by Lennart Querter on 02.02.25.
//

#ifndef REVERB_H
#define REVERB_H

#include "main.h"

typedef struct
{
    uint16_t size;
    GPIO_TypeDef* size_port;
    uint16_t size_pin;

    uint16_t texture;
    GPIO_TypeDef* texture_port;
    uint16_t texture_pin;

    uint16_t pitch;
    GPIO_TypeDef* pitch_port;
    uint16_t pitch_pin;

    uint16_t blend;
    GPIO_TypeDef* blend_port;
    uint16_t blend_pin;

    uint16_t size_cv;
    GPIO_TypeDef* size_cv_port;
    uint16_t size_cv_pin;

    uint16_t texture_cv;
    GPIO_TypeDef* texture_cv_port;
    uint16_t texture_cv_pin;

    uint16_t v_oct;
    GPIO_TypeDef* v_oct_port;
    uint16_t v_oct_pin;

    uint16_t blend_cv;
    GPIO_TypeDef* blend_cv_port;
    uint16_t blend_cv_pin;

    uint16_t position;
    GPIO_TypeDef* position_port;
    uint16_t position_pin;

    uint16_t density;
    GPIO_TypeDef* density_port;
    uint16_t density_pin;
} REVERB_config;

void REVERB_init(
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
);

void REVERB_run(REVERB_config* config);

void REVERB_process(int32_t* input, int32_t* output, uint16_t offset, uint16_t size);

#endif //REVERB_H
