//
// Created by Lennart Querter on 05.10.24.
//

#ifndef ENCODER_H
#define ENCODER_H

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

#endif //ENCODER_H
