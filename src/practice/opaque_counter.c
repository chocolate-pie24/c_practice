#include <stdio.h>  // for fprintf
#include <stdlib.h> // for malloc
#include <string.h> // for memset
#include <stdint.h>

#include "practice/opaque_counter.h"

#include "internal/opaque_counter_internal_data.h"

opaque_counter_error_t opaque_counter_create(opaque_counter_t** out_counter_) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_RUNTIME_ERROR;  // デフォルト値
    if(NULL == out_counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create - Argument out_counter_ requires a valid pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL != *out_counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create - Argument out_counter_ requires a null pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }

    opaque_counter_t* tmp = NULL;
    tmp = malloc(sizeof(*tmp));
    if(NULL == tmp) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create - Failed to allocate opaque_counter memory.\n");
        ret = OPAQUE_COUNTER_NO_MEMORY;
        goto cleanup;
    }
    memset(tmp, 0, sizeof(*tmp));
    *out_counter_ = tmp;
    ret = OPAQUE_COUNTER_SUCCESS;

cleanup:
    if(OPAQUE_COUNTER_SUCCESS != ret) {
        if(NULL != tmp) {
            free(tmp);
            tmp = NULL;
        }
    }
    return ret;
}

void opaque_counter_destroy(opaque_counter_t** counter_) {
    if(NULL == counter_ || NULL == *counter_) {
        goto cleanup;
    }
    free(*counter_);
    *counter_ = NULL;

cleanup:
    return;
}

opaque_counter_error_t opaque_counter_inc(opaque_counter_t* counter_) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_RUNTIME_ERROR;
    if(NULL == counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_inc - Argument counter_ requires a valid pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    uint32_t tmp_val = (uint32_t)counter_->value;
    tmp_val += 1;
    if(tmp_val > OPAQUE_COUNTER_MAX) {
        fprintf(stderr, "[ERROR](OVERFLOW): opaque_counter_inc - Opaque counter overflow.\n");
        ret = OPAQUE_COUNTER_OVERFLOW;
        goto cleanup;
    }
    counter_->value = (uint16_t)tmp_val;
    ret = OPAQUE_COUNTER_SUCCESS;

cleanup:
    return ret;
}

opaque_counter_error_t opaque_counter_dec(opaque_counter_t* counter_) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_RUNTIME_ERROR;
    if(NULL == counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_dec - Argument counter_ requires a valid pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    int32_t tmp_val = (int32_t)counter_->value;
    tmp_val -= 1;
    if(tmp_val < OPAQUE_COUNTER_MIN) {
        fprintf(stderr, "[ERROR](UNDERFLOW): opaque_counter_dec - Opaque counter underflow.\n");
        ret = OPAQUE_COUNTER_UNDERFLOW;
        goto cleanup;
    }
    counter_->value = (uint16_t)tmp_val;
    ret = OPAQUE_COUNTER_SUCCESS;

cleanup:
    return ret;
}

opaque_counter_error_t opaque_counter_add(opaque_counter_t* counter_, int32_t delta_) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_RUNTIME_ERROR;
    if(NULL == counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_add - Argument counter_ requires a valid pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(0 == delta_) {
        ret = OPAQUE_COUNTER_SUCCESS;
        goto cleanup;
    }
    int64_t tmp_value = (int64_t)counter_->value;
    tmp_value += (int64_t)delta_;
    if(tmp_value > (int64_t)OPAQUE_COUNTER_MAX) {
        fprintf(stderr, "[ERROR](OVERFLOW): opaque_counter_add - Opaque counter overflow.\n");
        ret = OPAQUE_COUNTER_OVERFLOW;
        goto cleanup;
    } else if(tmp_value < (int64_t)OPAQUE_COUNTER_MIN) {
        fprintf(stderr, "[ERROR](UNDERFLOW): opaque_counter_add - Opaque counter underflow.\n");
        ret = OPAQUE_COUNTER_UNDERFLOW;
        goto cleanup;
    }
    counter_->value = (uint16_t)tmp_value;
    ret = OPAQUE_COUNTER_SUCCESS;

cleanup:
    return ret;
}

opaque_counter_error_t opaque_counter_get(const opaque_counter_t* const counter_, uint16_t* out_value_) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_RUNTIME_ERROR;
    if(NULL == counter_ || NULL == out_value_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_get - Arguments opaque_counter_ and out_value_ require valid pointers.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    *out_value_ = counter_->value;
    ret = OPAQUE_COUNTER_SUCCESS;

cleanup:
    return ret;
}
