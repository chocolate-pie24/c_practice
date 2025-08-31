#ifndef OPAQUE_COUNTER_H
#define OPAQUE_COUNTER_H

#include <stdint.h>

typedef struct opaque_counter opaque_counter_t;

typedef struct opaque_counter_config {
    int32_t min;
    int32_t max;
    int32_t initial;
} opaque_counter_config_t;

typedef enum {
    OPAQUE_COUNTER_SUCCESS,
    OPAQUE_COUNTER_INVALID_ARGUMENT,
    OPAQUE_COUNTER_RUNTIME_ERROR,
    OPAQUE_COUNTER_NO_MEMORY,
    OPAQUE_COUNTER_OVERFLOW,
    OPAQUE_COUNTER_UNDERFLOW,
} opaque_counter_error_t;

opaque_counter_error_t opaque_counter_default_create(opaque_counter_t** counter_);

opaque_counter_error_t opaque_counter_create_ex(opaque_counter_t** counter_, const char* const name_, const opaque_counter_config_t* const config_);

void opaque_counter_destroy(opaque_counter_t** counter_);

opaque_counter_error_t opaque_counter_inc(opaque_counter_t* const counter_);

opaque_counter_error_t opaque_counter_dec(opaque_counter_t* const counter_);

opaque_counter_error_t opaque_counter_add(opaque_counter_t* const counter_, int32_t delta_);

opaque_counter_error_t opaque_counter_get(const opaque_counter_t* const counter_, int32_t* const out_value_);

opaque_counter_error_t opaque_counter_range_get(const opaque_counter_t* const counter_, opaque_counter_config_t* const out_config_);

opaque_counter_error_t opaque_counter_debug_print(const opaque_counter_t* const counter_);

// for test
void test_opaque_counter_create_ex(void);
void test_opaque_counter_destroy(void);

#endif
