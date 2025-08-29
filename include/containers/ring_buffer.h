#ifndef RING_BUFFER_H
#define RING_BUFFER_H

typedef struct ring_buffer ring_buffer_t;

typedef enum {
    RING_BUFFER_SUCCESS,
    RING_BUFFER_NO_MEMORY,
    RING_BUFFER_FULL,
    RING_BUFFER_EMPTY,
    RING_BUFFER_INVALID_ARGUMENT,
    RING_BUFFER_RUNTIME_ERROR,
} ring_buffer_error_t;

ring_buffer_error_t ring_buffer_create(void);

ring_buffer_error_t ring_buffer_destroy(void);

ring_buffer_error_t ring_buffer_push(void);

ring_buffer_error_t ring_buffer_pop(void);

void ring_buffer_debug_print(void);

#endif
