//
// Created by Lennart Querter on 02.02.25.
//

#ifndef REVERB_H
#define REVERB_H

#include "main.h"

// Structure to hold processed parameters
typedef struct
{
    float v_oct;
    float position;
    float density;
    float pitch;
    float texture;
    float texture_cv;
    float blend;
    float blend_cv;
    float size;
    float size_cv;
} REVERB_parameters;

void REVERB_run(REVERB_parameters* parameters);

void REVERB_process(int32_t* input, int32_t* output, uint16_t offset, uint16_t size);

#endif //REVERB_H
