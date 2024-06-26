#include "./buffer.h"
#include <stdint.h>
#include <stdlib.h>

BUFFER_STATUS buffer_init(Buffer *buf, uint16_t buffer_size) {
    uint32_t i;
    buffer_u8_free(buf);

    buf->buffer = (uint8_t *) malloc(buffer_size * sizeof(uint8_t));

    if (NULL == buf->buffer) {
        return BUFFER_FAILURE;
    }
    for (i = 0; i < buffer_size; i++) {
        buf->buffer[i] = 0;
    }

    buf->length = buffer_size;

    return BUFFER_SUCCESS;
}

BUFFER_STATUS buffer_u8_free(Buffer *buf) {
    if (NULL != buf->buffer) {
        free(buf->buffer);
    }

    buf->idx_front = buf->idx_rear = 0;
    buf->length = 0;

    return BUFFER_SUCCESS;
}

BUFFER_STATUS buffer_push(Buffer *buf, const uint8_t *input) {
    if (((buf->idx_front + 1) & (buf->length - 1)) == buf->idx_rear) { //buffer over-run error occurs.
        return BUFFER_FAILURE;
    } else {

        buf->buffer[buf->idx_front] = *input;
        buf->idx_front++;
        buf->idx_front &= (buf->length - 1);
        return BUFFER_SUCCESS;
    }
}

BUFFER_STATUS buffer_pop(Buffer *buf, uint8_t *ret) {
    if (buf->idx_front == buf->idx_rear) { // if buffer under-run error occurs.
        return BUFFER_FAILURE;
    } else {
        *ret = (buf->buffer[buf->idx_rear]);
        buf->idx_rear++;
        buf->idx_rear &= (buf->length - 1);
        return BUFFER_SUCCESS;
    }
}