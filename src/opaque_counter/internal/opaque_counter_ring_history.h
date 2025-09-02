#ifndef OPAQUE_COUNTER_RING_HISTORY_H
#define OPAQUE_COUNTER_RING_HISTORY_H

#include <stddef.h>

#include "opaque_counter_history.h"

typedef enum {
    OPAQUE_COUNTER_RING_HISTORY_SUCCESS,
    OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT,
    OPAQUE_COUNTER_RING_HISTORY_RUNTIME_ERROR,
    OPAQUE_COUNTER_RING_HISTORY_NO_MEMORY,
} oc_ring_history_error_t;

typedef struct opaque_counter_ring_histroy {
    opaque_counter_history_t* histories;
    size_t head;
    size_t tail;
    size_t len;
    size_t capacity;
} opaque_counter_ring_history_t;

oc_ring_history_error_t opaque_counter_ring_history_create(opaque_counter_ring_history_t* history_, size_t capacity_);

void opaque_counter_ring_history_destroy(opaque_counter_ring_history_t* history_);

oc_ring_history_error_t opaque_counter_ring_history_push(opaque_counter_ring_history_t* ring_history_, const opaque_counter_history_t* const hitory_);

void opaque_counter_ring_history_print(const opaque_counter_ring_history_t* const history_);

#endif
