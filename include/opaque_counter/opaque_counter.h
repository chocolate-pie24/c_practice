#ifndef OPAQUE_COUNTER_H
#define OPAQUE_COUNTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "opaque_counter_config.h"

typedef struct opaque_counter opaque_counter_t;

typedef enum {
    OC_SUCCESS,
    OC_INVALID_ARGUMENT,
    OC_RUNTIME_ERROR,
    OC_NO_MEMORY,
    OC_OVERFLOW,
    OC_UNDERFLOW,
    OC_UNDEFINED,
} oc_error_t;

oc_error_t opaque_counter_create(opaque_counter_t** counter_, const oc_config_t* const config_);

void opaque_counter_destroy(opaque_counter_t** counter_);

oc_error_t opaque_counter_inc(opaque_counter_t* const counter_);

oc_error_t opaque_counter_dec(opaque_counter_t* const counter_);

oc_error_t opaque_counter_add(opaque_counter_t* const counter_, int32_t delta_);

#ifdef __cplusplus
}
#endif
#endif
