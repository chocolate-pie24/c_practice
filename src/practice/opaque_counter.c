#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <assert.h> // for test
#include <string.h> // for test

#include "pracitce/opaque_counter.h"

#include "internal/opaque_counter_internal_data.h"

static opaque_counter_error_t opaque_counter_push_history(opaque_counter_t* counter_, opaque_counter_operation_t operation_, int32_t value_before_, int32_t value_after_);

static void zero_memory(void* mem_, size_t size_);

static size_t string_length(const char* const str_);
static bool string_copy(const char* const src_, char* const dst_, size_t dst_buff_size_);
static char* string_duplicate(const char* const str_);

// 成功: nameに値がセットされている、max,min,initialに値が入っている,counter,head,tailが0,histories,scratchが非NULL
void test_opaque_counter_create_ex(void) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_SUCCESS;
    opaque_counter_t* counter = NULL;
    opaque_counter_config_t config;
    config.initial = 5;
    config.min = 1;
    config.max = 8;

    // counter_ == NULLでINVALID_ARGUMENT
    ret = opaque_counter_create_ex(0, "test", &config);
    assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);

    // name_ == NULLでINVALID_ARGUMENT
    ret = opaque_counter_create_ex(&counter, NULL, &config);
    assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);
    assert(counter == NULL);

    // config_ == NULLでINVALID_ARGUMENT
    ret = opaque_counter_create_ex(&counter, "test", NULL);
    assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);
    assert(counter == NULL);

    // counter_ != NULLでINVALID_ARGUMENT
    opaque_counter_t test_counter;
    opaque_counter_t* test_counter2 = &test_counter;
    ret = opaque_counter_create_ex(&test_counter2, "test", &config);
    assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);
    assert(counter == NULL);

    ret = opaque_counter_create_ex(&counter, "test", &config);
    assert(OPAQUE_COUNTER_SUCCESS == ret);
    assert(counter->config.initial == 5);
    assert(counter->config.max == 8);
    assert(counter->config.min == 1);
    assert(counter->buffer_head == 0);
    assert(counter->buffer_tail == 0);
    assert(counter->buffer_capacity == OPAQUE_COUNTER_HISTROY_COUNT);
    assert(counter->buffer_len == 0);
    assert(counter->counter == counter->config.initial);
    assert(counter->histories != NULL);
    assert(0 == strcmp("test", counter->name));
    assert(counter->scrach != NULL);
}

opaque_counter_error_t opaque_counter_create_ex(opaque_counter_t** counter_, const char* const name_, const opaque_counter_config_t* const config_) {
    opaque_counter_t* tmp = NULL;
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    if(NULL == counter_ || NULL == config_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create_ex - Arguments counter_ and config_ require valid pointers.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL != *counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create_ex - Argument *counter_ requires a null pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL == name_) {
        fprintf(stdout, "[ERROR](INVALID_ARGUMENT): opaque_counter_create_ex - Argument name_ requires a valid pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }

    tmp = malloc(sizeof(*tmp));
    if(NULL == tmp) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create_ex - Failed to allocate opaque counter memory.\n");
        ret = OPAQUE_COUNTER_NO_MEMORY;
        goto cleanup;
    }
    zero_memory(tmp, sizeof(*tmp));

    tmp->name = NULL;
    tmp->name = string_duplicate(name_);
    if(NULL == tmp->name) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create_ex - Failed to allocate name label memory.\n");
        ret = OPAQUE_COUNTER_NO_MEMORY;
        goto cleanup;
    }

    tmp->histories = NULL;
    tmp->histories = malloc(sizeof(opaque_counter_operation_history_t) * OPAQUE_COUNTER_HISTROY_COUNT);
    if(NULL == tmp->histories) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create_ex - Failed to allocate histories memory.\n");
        ret = OPAQUE_COUNTER_NO_MEMORY;
        goto cleanup;
    }
    zero_memory(tmp->histories, sizeof(*tmp->histories));

    tmp->scrach = NULL;
    tmp->scrach = malloc(sizeof(char) * OPAQUE_COUNTER_DEFAULT_SCRATCH_SIZE);
    if(NULL == tmp->scrach) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create_ex - Failed to allocate scratch memory.\n");
        ret = OPAQUE_COUNTER_NO_MEMORY;
        goto cleanup;
    }
    zero_memory(tmp->scrach, sizeof(*tmp->scrach));

    tmp->config.initial = config_->initial;
    tmp->config.max = config_->max;
    tmp->config.min = config_->min;
    tmp->buffer_head = 0;
    tmp->buffer_tail = 0;
    tmp->buffer_capacity = OPAQUE_COUNTER_HISTROY_COUNT;
    tmp->buffer_len = 0;
    tmp->counter = tmp->config.initial;

    *counter_ = tmp;
    ret = OPAQUE_COUNTER_SUCCESS;

cleanup:
    if(OPAQUE_COUNTER_SUCCESS != ret) {
        if(NULL != tmp) {
            if(NULL != tmp->scrach) {
                free(tmp->scrach);
                tmp->scrach = NULL;
            }
            if(NULL != tmp->histories) {
                free(tmp->histories);
                tmp->histories = NULL;
            }
            if(NULL != tmp->name) {
                free(tmp->name);
                tmp->name = NULL;
            }

            free(tmp);
            tmp = NULL;
        }
    }
    return ret;
}

void test_opaque_counter_destroy(void) {
    opaque_counter_destroy(0);

    opaque_counter_t* counter = NULL;
    opaque_counter_destroy(&counter);

    opaque_counter_config_t config;
    config.initial = 3;
    config.max = 5;
    config.min = 1;
    opaque_counter_create_ex(&counter, "destroy", &config);
    opaque_counter_destroy(&counter);
    assert(counter == NULL);
    opaque_counter_destroy(&counter);
}

// counter_ == NULL / *counter_ == NULLで何もしない
// 2重destroy OK
// 成功したら*counter_ = NULL
void opaque_counter_destroy(opaque_counter_t** counter_) {
    if(NULL == counter_ || NULL == *counter_) {
        goto cleanup;
    }
    if(NULL != (*counter_)->histories) {
        free((*counter_)->histories);
        (*counter_)->histories = NULL;
    }
    if(NULL != (*counter_)->scrach) {
        free((*counter_)->scrach);
        (*counter_)->scrach = NULL;
    }
    if(NULL != (*counter_)->name) {
        free((*counter_)->name);
        (*counter_)->name = NULL;
    }
    free(*counter_);
    *counter_ = 0;
cleanup:
    return;
}

static void opaque_counter_push_history(opaque_counter_t* counter_, opaque_counter_operation_t operation_, int32_t value_before_, int32_t value_after_) {
    // TODO: NULLチェック、capacity != 0チェック
    counter_->histories[counter_->buffer_tail].operation = operation_;
    counter_->histories[counter_->buffer_tail].value_before = value_before_;
    counter_->histories[counter_->buffer_tail].value_after = value_after_;

    if(counter_->buffer_len == counter_->buffer_capacity) { // 満杯
        counter_->buffer_head = (counter_->buffer_head + 1) % counter_->buffer_capacity;
    } else {
        counter_->buffer_len++;
    }
    counter_->buffer_tail = (counter_->buffer_tail + 1) % counter_->buffer_capacity;
}

static void opaque_counter_history_print() {

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
