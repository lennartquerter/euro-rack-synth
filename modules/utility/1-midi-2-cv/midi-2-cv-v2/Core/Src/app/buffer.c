#include "app/buffer.h"
#include <stdint.h>
#include <stdlib.h>

BUFFER_STATUS buffer_init(Buffer *buf, uint16_t buffer_size) {
	 if (!buf || buffer_size == 0) {
		return BUFFER_FAILURE;
	}

	// Initialize structure first
	buf->buffer = NULL;
	buf->idx_front = 0;
	buf->idx_rear = 0;
	buf->length = 0;

	// Now safe to free
	buffer_u8_free(buf);

	// Buffer size should be power of 2 for the bitwise operations to work
	if ((buffer_size & (buffer_size - 1)) != 0) {
		// Round up to next power of 2
		buffer_size--;
		buffer_size |= buffer_size >> 1;
		buffer_size |= buffer_size >> 2;
		buffer_size |= buffer_size >> 4;
		buffer_size |= buffer_size >> 8;
		buffer_size++;
	}

	buf->buffer = (uint8_t *)malloc(buffer_size * sizeof(uint8_t));
	if (NULL == buf->buffer) {
		return BUFFER_FAILURE;
	}

	for (uint32_t i = 0; i < buffer_size; i++) {
		buf->buffer[i] = 0;
	}

	buf->length = buffer_size;
	return BUFFER_SUCCESS;
}

BUFFER_STATUS buffer_u8_free(Buffer *buf) {
	 if (!buf) {
		return BUFFER_FAILURE;
	}

	if (buf->buffer != NULL) {
		free(buf->buffer);
		buf->buffer = NULL;  // Prevent use-after-free
	}

	buf->idx_front = buf->idx_rear = 0;
	buf->length = 0;

	return BUFFER_SUCCESS;
}

BUFFER_STATUS buffer_push(Buffer *buf, const uint8_t *input) {
	  if (!buf || !buf->buffer || !input) {
		return BUFFER_FAILURE;
	}

	uint16_t next_front = (buf->idx_front + 1) & (buf->length - 1);
	if (next_front == buf->idx_rear) {
		return BUFFER_FAILURE;  // Buffer full
	}

	buf->buffer[buf->idx_front] = *input;
	buf->idx_front = next_front;
	return BUFFER_SUCCESS;
}

BUFFER_STATUS buffer_pop(Buffer *buf, uint8_t *ret) {
	 if (!buf || !buf->buffer || !ret) {
		return BUFFER_FAILURE;
	}

	if (buf->idx_front == buf->idx_rear) {
		return BUFFER_FAILURE;  // Buffer empty
	}

	*ret = buf->buffer[buf->idx_rear];
	buf->idx_rear = (buf->idx_rear + 1) & (buf->length - 1);
	return BUFFER_SUCCESS;
}

uint16_t buffer_get_size(Buffer *buf) {
    if (!buf) {
    	return 0;
    }
    return (buf->idx_front - buf->idx_rear) & (buf->length - 1);
}

bool buffer_is_empty(Buffer *buf) {
    if (!buf) {
    	return true;
    }
    return buf->idx_front == buf->idx_rear;
}

bool buffer_is_full(Buffer *buf) {
    if (!buf) {
    	return true;
    }
    return ((buf->idx_front + 1) & (buf->length - 1)) == buf->idx_rear;
}
