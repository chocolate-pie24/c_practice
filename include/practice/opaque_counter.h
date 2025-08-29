#ifndef OPAQUE_COUNTER_H
#define OPAQUE_COUNTER_H

#include <stdint.h>

typedef struct opaque_counter opaque_counter_t;

typedef enum {
    OPAQUE_COUNTER_SUCCESS,
    OPAQUE_COUNTER_INVALID_ARGUMENT,
    OPAQUE_COUNTER_RUNTIME_ERROR,
    OPAQUE_COUNTER_OVERFLOW,
    OPAQUE_COUNTER_UNDERFLOW,
    OPAQUE_COUNTER_NO_MEMORY,
} opaque_counter_error_t;

opaque_counter_error_t opaque_counter_create(opaque_counter_t** counter_);

void opaque_counter_destroy(opaque_counter_t** counter_);

opaque_counter_error_t opaque_counter_inc(opaque_counter_t* const counter_);

opaque_counter_error_t opaque_counter_dec(opaque_counter_t* const counter_);

opaque_counter_error_t opaque_counter_add(opaque_counter_t* const counter_, int32_t delta_);

opaque_counter_error_t opaque_counter_get(const opaque_counter_t* const counter_, int32_t* const out_value_);

#endif
