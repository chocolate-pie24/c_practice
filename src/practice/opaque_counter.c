#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "practice/opaque_counter.h"

#include "internal/opaque_counter_internal_data.h"

opaque_counter_error_t opaque_counter_create(opaque_counter_t** counter_) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    opaque_counter_t* tmp = NULL;
    if(NULL == counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create - Argument counter_ requires a valid pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL != *counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create - Argument *counter_ requires a null pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    tmp = malloc(sizeof(*tmp));
    if(NULL == tmp) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create - Failed to allocate opaque counter memory.\n");
        ret = OPAQUE_COUNTER_NO_MEMORY;
        goto cleanup;
    }
    tmp->counter = OPAQUE_COUNTER_MIN;
    *counter_ = tmp;
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

opaque_counter_error_t opaque_counter_inc(opaque_counter_t* const counter_) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    if(NULL == counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_inc - Argument counter_ requires a valid pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    int64_t tmp = (int64_t)counter_->counter;
    tmp++;
    if(tmp > (int64_t)OPAQUE_COUNTER_MAX) {
        fprintf(stderr, "[ERROR](OVERFLOW): opaque_counter_inc - Opaque counter overflow.\n");
        ret = OPAQUE_COUNTER_OVERFLOW;
        goto cleanup;
    }
    counter_->counter = (int32_t)tmp;
    ret = OPAQUE_COUNTER_SUCCESS;

cleanup:
    return ret;
}

opaque_counter_error_t opaque_counter_dec(opaque_counter_t* const counter_) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    if(NULL == counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_dec - Argument counter_ requires a valid pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    int64_t tmp = (int64_t)counter_->counter;
    tmp--;
    if(tmp < (int64_t)OPAQUE_COUNTER_MIN) {
        fprintf(stderr, "[ERROR](UNDERFLOW): opaque_counter_dec - Opaque counter underflow.\n");
        ret = OPAQUE_COUNTER_UNDERFLOW;
        goto cleanup;
    }
    counter_->counter = (int32_t)tmp;
    ret = OPAQUE_COUNTER_SUCCESS;

cleanup:
    return ret;
}

opaque_counter_error_t opaque_counter_add(opaque_counter_t* const counter_, int32_t delta_) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    if(NULL == counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_add - Argument counter_ requires a valid pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(0 == delta_) {
        ret = OPAQUE_COUNTER_SUCCESS;
        goto cleanup;
    }
    int64_t tmp = (int64_t)counter_->counter;
    tmp += delta_;
    if(tmp > (int64_t)OPAQUE_COUNTER_MAX) {
        fprintf(stderr, "[ERROR](OVERFLOW): opaque_counter_add - Opaque counter overflow.\n");
        ret = OPAQUE_COUNTER_OVERFLOW;
        goto cleanup;
    }
    if(tmp < (int64_t)OPAQUE_COUNTER_MIN) {
        fprintf(stderr, "[ERROR](UNDERFLOW): opaque_counter_add - Opaque counter underflow.\n");
        ret = OPAQUE_COUNTER_UNDERFLOW;
        goto cleanup;
    }
    counter_->counter = (int32_t)tmp;
    ret = OPAQUE_COUNTER_SUCCESS;

cleanup:
    return ret;
}

opaque_counter_error_t opaque_counter_get(const opaque_counter_t* const counter_, int32_t* const out_value_) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    if(NULL == counter_ || NULL == out_value_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_get - Arguments counter_ and out_value_ require valid pointers.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    *out_value_ = counter_->counter;
    ret = OPAQUE_COUNTER_SUCCESS;

cleanup:
    return ret;
}
