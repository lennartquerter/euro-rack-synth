/*
 * Simple buffer system for uint8 values.
 * Used by the midi system to push during interrupt, and read values during the loop
 */

#ifndef _M2C_BUFFER_
#define _M2C_BUFFER_

#include <stdint.h>

typedef enum {
    BUFFER_FAILURE,
    BUFFER_SUCCESS
} BUFFER_STATUS;

typedef struct {
    uint16_t idx_front;
    uint16_t idx_rear;
    uint16_t length;
    uint8_t *buffer;
} Buffer;

BUFFER_STATUS buffer_init(Buffer *buf, uint16_t buffer_size);

BUFFER_STATUS buffer_u8_free(Buffer *buf);

BUFFER_STATUS buffer_push(Buffer *buf, const uint8_t *input);

BUFFER_STATUS buffer_pop(Buffer *buf, uint8_t *ret);

#endif /* _M2C_BUFFER_ */