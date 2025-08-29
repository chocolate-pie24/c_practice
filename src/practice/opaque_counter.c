#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pracitce/opaque_counter.h"

#include "internal/opaque_counter_internal_data.h"

static void zero_memory(void* mem_, size_t size_);

static size_t string_length(const char* const str_);
static bool string_copy(const char* const src_, char* const dst_, size_t dst_buff_size_);
static char* string_duplicate(const char* const str_);

// TODO: ring_buffer
// TODO: opaque_counter_create_ex
opaque_counter_error_t opaque_counter_create_ex(opaque_counter_t** counter_, const char* const name_, const opaque_counter_config_t* const config_) {
    opaque_counter_t* tmp = NULL;
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    if(NULL == counter_ || NULL == config_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create_ex - Arguments counter_ and config_ require valid pointers.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL != counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create_ex - Argument *counter_ requires a null pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL == name_) {
        fprintf(stdout, "[WARNING]: opaque_counter_create_ex - Provided name_ is null pointer.\n");
    }
    tmp = malloc(sizeof(*tmp));
    if(NULL == tmp) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create_ex - Failed to allocate opaque counter memory.\n");
        ret = OPAQUE_COUNTER_NO_MEMORY;
        goto cleanup;
    }
    zero_memory(tmp, sizeof(*tmp));

    tmp->config.initial = config_->initial;
    tmp->config.max = config_->max;
    tmp->config.min = config_->min;
    if(NULL != name_) {
        tmp->name = string_duplicate(name_);
    } else {
        tmp->name = NULL;
    }
    // TODO: 初期化

cleanup:
    // TODO: cleanup
    return ret;
}

static void zero_memory(void* mem_, size_t size_) {
    char* tmp = (char*)mem_;
    for(size_t i = 0; i != size_; ++i) {
        tmp[i] = 0;
    }
}

static size_t string_length(const char* const str_) {
    size_t ret = 0;
    if(NULL == str_) {
        ret = 0;
        goto cleanup;
    }
    size_t i = 0;
    while(str_[i] != '\0') {
        i++;
    }
    ret = i;

cleanup:
    return ret;
}

static bool string_copy(const char* const src_, char* const dst_, size_t dst_buff_size_) {
    bool ret = false;
    if(NULL == src_ || NULL == dst_) {
        fprintf(stderr, "[ERROR]: string_copy - Arguments src_ and dst_ require valid pointers.\n");
        ret = false;
        goto cleanup;
    }
    const size_t src_len = string_length(src_) + 1;
    if(src_len > dst_buff_size_) {
        fprintf(stderr, "[ERROR]: string_copy - Destination buffer size is too small.\n");
        ret = false;
        goto cleanup;
    }
    for(size_t i = 0; i != src_len; ++i) {
        dst_[i] = src_[i];
    }
    ret = true;

cleanup:
    return ret;
}

static char* string_duplicate(const char* const str_) {
    bool result = false;
    char* mem = NULL;

    const size_t len = string_length(str_) + 1;
    mem = malloc(len);
    if(NULL == mem) {
        fprintf(stderr, "[ERROR]: string_duplicate - Failed to allocate string memory.\n");
        result = false;
        goto cleanup;
    }
    for(size_t i = 0; i != len; ++i) {
        mem[i] = 0;
    }
    if(!string_copy(str_, mem, len)) {
        fprintf(stderr, "[ERROR]: string_duplicate - Failed to copy string.\n");
        result = false;
        goto cleanup;
    }
    result = true;

cleanup:
    if(!result) {
        if(NULL != mem) {
            free(mem);
            mem = NULL;
        }
    }
    return mem;
}
