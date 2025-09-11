#ifndef OPAQUE_COUNTER_CONFIG_H
#define OPAQUE_COUNTER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct oc_config {
    const char* name_label;
    size_t history_capacity;
    int32_t min;
    int32_t max;
    int32_t initial;
} oc_config_t;

bool oc_config_valid_check(const oc_config_t* const config_);

#ifdef __cplusplus
}
#endif
#endif
