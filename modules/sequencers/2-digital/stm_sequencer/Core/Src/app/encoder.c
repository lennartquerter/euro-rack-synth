#include "app/encoder.h"

/*
 * Initialize the encoder
 * @input config Used to generate the initial values.
 * @input command_queue_config Used to generate the initial values.
 * @input timer
 */
void encoder_init(ENCODER_config* config, COMMAND_QUEUE_config* command_queue_config, TIM_HandleTypeDef* timer)
{
    config->timer = timer;
    config->current_position = 0;
    config->command_queue_config = command_queue_config;
}

void encoder_handle_timer_interrupt(ENCODER_config* config)
{
    // Check if we are scrolling
    uint32_t counter = __HAL_TIM_GET_COUNTER(config->timer);
    uint16_t position = ((int16_t)counter) / 4;

    if (position < config->current_position)
    {
        command_queue_push(config->command_queue_config, ENCODER_UP);
    }
    else if (position > config->current_position)
    {
        command_queue_push(config->command_queue_config, ENCODER_DOWN);
    }

    // set the current_position to where we are
    config->current_position = position;
}
