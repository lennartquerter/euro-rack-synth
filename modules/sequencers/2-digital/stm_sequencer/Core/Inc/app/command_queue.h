/*
 * Simple command_queue system for control events
 */

#ifndef _SEQ_COMMAND_QUEUE_
#define _SEQ_COMMAND_QUEUE_

#include <stdint.h>

typedef enum
{
    COMMAND_QUEUE_FAILURE,
    COMMAND_QUEUE_SUCCESS
} COMMAND_QUEUE_STATUS;

typedef enum
{
    ENCODER_CLICK = 0,
    ENCODER_UP = 1,
    ENCODER_DOWN = 2,
} COMMAND_QUEUE_EVENT;

typedef struct
{
    uint16_t idx_front;
    uint16_t idx_rear;
    uint16_t length;
    COMMAND_QUEUE_EVENT* queue;
} COMMAND_QUEUE_config;

COMMAND_QUEUE_STATUS command_queue_init(COMMAND_QUEUE_config* command_queue, uint16_t buffer_size);

COMMAND_QUEUE_STATUS command_queue_u8_free(COMMAND_QUEUE_config* command_queue);

COMMAND_QUEUE_STATUS command_queue_push(COMMAND_QUEUE_config* command_queue, COMMAND_QUEUE_EVENT input);

COMMAND_QUEUE_STATUS command_queue_pop(COMMAND_QUEUE_config* command_queue, COMMAND_QUEUE_EVENT* event);

#endif /* _SEQ_COMMAND_QUEUE_ */
