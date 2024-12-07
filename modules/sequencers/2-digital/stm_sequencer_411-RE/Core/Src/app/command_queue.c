#include "app/command_queue.h"

#include <stdint.h>
#include <stdlib.h>

COMMAND_QUEUE_STATUS command_queue_init(COMMAND_QUEUE_config* command_queue, uint16_t size)
{
    command_queue_u8_free(command_queue);

    command_queue->queue = (COMMAND_QUEUE_EVENT*)malloc(size * sizeof(COMMAND_QUEUE_EVENT));

    if (NULL == command_queue->queue)
    {
        return COMMAND_QUEUE_FAILURE;
    }
    for (uint32_t i = 0; i < size; i++)
    {
        command_queue->queue[i] = 0;
    }

    command_queue->length = size;

    return COMMAND_QUEUE_SUCCESS;
}

COMMAND_QUEUE_STATUS command_queue_u8_free(COMMAND_QUEUE_config* command_queue)
{
    if (NULL != command_queue->queue)
    {
        free(command_queue->queue);
    }

    command_queue->idx_front = command_queue->idx_rear = 0;
    command_queue->length = 0;

    return COMMAND_QUEUE_SUCCESS;
}

COMMAND_QUEUE_STATUS command_queue_push(COMMAND_QUEUE_config *command_queue, COMMAND_QUEUE_EVENT input)
{
    if (((command_queue->idx_front + 1) & (command_queue->length - 1)) == command_queue->idx_rear)
    {
        // buffer over-run error occurs.
        return COMMAND_QUEUE_FAILURE;
    }

    command_queue->queue[command_queue->idx_front] = input;
    command_queue->idx_front++;
    command_queue->idx_front &= (command_queue->length - 1);
    return COMMAND_QUEUE_SUCCESS;
}

COMMAND_QUEUE_STATUS command_queue_pop(COMMAND_QUEUE_config *command_queue, COMMAND_QUEUE_EVENT *event)
{
    if (command_queue->idx_front == command_queue->idx_rear)
    {
        // if buffer under-run error occurs.
        return COMMAND_QUEUE_FAILURE;
    }

    *event = (command_queue->queue[command_queue->idx_rear]);
    command_queue->idx_rear++;
    command_queue->idx_rear &= (command_queue->length - 1);
    return COMMAND_QUEUE_SUCCESS;
}
