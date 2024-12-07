/*
******************************************************************************
* @file           : encoder.h
* @author         : Lennart Querter
* @brief          : Header for encoder.c file.
*                   Handles the rotation of the encoder, pushes commands to the command_queue
******************************************************************************
*/

#ifndef __ENCODER_H
#define __ENCODER_H

#include "main.h"
#include "app/command_queue.h"

typedef struct
{
    TIM_HandleTypeDef* timer;
    COMMAND_QUEUE_config* command_queue_config;

    uint32_t current_position;
} ENCODER_config;

void encoder_init(ENCODER_config* config, COMMAND_QUEUE_config* command_queue_config, TIM_HandleTypeDef* timer);

void encoder_handle_timer_interrupt(ENCODER_config* config);

#endif //__ENCODER_H
