#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> // for strcpy, memset

#include "internal/opaque_counter_ring_history.h"
#include "internal/opaque_counter_history.h"

// TODO: #ifdef TEST_BUILD
typedef struct test_param {
    uint16_t malloc_counter;    // malloc実行回数
    uint16_t malloc_fail_n;     // n回目のmallocで失敗
    bool fail_enable;
} test_param_t;

static void* oc_malloc(size_t size_);
static void history_op_string(opaque_counter_op_t op_, char* const out_str_);

// TODO: #ifdef TEST_BUILD
static test_param_t s_test_param;

// TODO: test
// TODO: oc_malloc
oc_ring_history_error_t opaque_counter_ring_history_create(opaque_counter_ring_history_t* history_, size_t capacity_) {
    oc_ring_history_error_t ret = OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT;
    if(NULL == history_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_ring_history_create - Argument history_ requires a valid pointer.\n");
        ret = OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(0 == capacity_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_ring_history_create - Argument capacity_ requires a non-zero value.\n");
        ret = OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL != history_->histories) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_ring_history_create - Argument history_->histories requires a null pointer.\n");
        ret = OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT;
        goto cleanup;
    }
    history_->histories = oc_malloc(sizeof(opaque_counter_history_t) * capacity_);
    if(NULL == history_->histories) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_ring_history_create - Failed to allocate hisotry buffer memory.\n");
        ret = OPAQUE_COUNTER_RING_HISTORY_NO_MEMORY;
        goto cleanup;
    }
    memset(history_->histories, 0, sizeof(opaque_counter_history_t) * capacity_);
    history_->capacity = capacity_;
    history_->head = 0;
    history_->tail = 0;
    history_->len = 0;
    ret = OPAQUE_COUNTER_RING_HISTORY_SUCCESS;

cleanup:
    if(OPAQUE_COUNTER_RING_HISTORY_SUCCESS != ret) {
        if(NULL != history_->histories) {
            free(history_->histories);
            history_->histories = NULL;
        }
    }
    return ret;
}

// TODO: test
void opaque_counter_ring_history_destroy(opaque_counter_ring_history_t* history_) {
    if(NULL == history_ || NULL == history_->histories) {
        goto cleanup;
    }
    free(history_->histories);
    history_->histories = NULL;
    history_->capacity = 0;
    history_->head = 0;
    history_->len = 0;
    history_->tail = 0;
cleanup:
    return;
}

// TODO: test
oc_ring_history_error_t opaque_counter_ring_history_push(opaque_counter_ring_history_t* ring_history_, const opaque_counter_history_t* const history_) {
    oc_ring_history_error_t ret = OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT;
    if(NULL == ring_history_ || NULL == history_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_ring_history_push - Arguments history_ and history_ require valid pointers.\n");
        ret = OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT;
        goto cleanup;
    }
    ring_history_->histories[ring_history_->tail].op = history_->op;
    ring_history_->histories[ring_history_->tail].value_after = history_->value_after;
    ring_history_->histories[ring_history_->tail].value_before = history_->value_before;

    ring_history_->tail = (ring_history_->tail + 1) % ring_history_->capacity;
    if(ring_history_->len == ring_history_->capacity) {
        ring_history_->head = (ring_history_->head + 1) % ring_history_->capacity;
    } else {
        ring_history_->len++;
    }
    ret = OPAQUE_COUNTER_RING_HISTORY_SUCCESS;
cleanup:
    return ret;
}

// TODO: test
void opaque_counter_ring_history_print(const opaque_counter_ring_history_t* const ring_history_) {
    if(NULL == ring_history_) {
        goto cleanup;
    }
    if(ring_history_->len == ring_history_->capacity) {
        for(size_t i = ring_history_->head; i != ring_history_->capacity; ++i) {
            char op[32] = { 0 };
            history_op_string(ring_history_->histories[i].op, op);
            fprintf(stdout, "ope - %s %d -> %d\n", op, ring_history_->histories[i].value_before, ring_history_->histories[i].value_after);
        }
        for(size_t i = 0; i != ring_history_->tail; ++i) {
            char op[32] = { 0 };
            history_op_string(ring_history_->histories[i].op, op);
            fprintf(stdout, "ope - %s %d -> %d\n", op, ring_history_->histories[i].value_before, ring_history_->histories[i].value_after);
        }
    } else {
        for(size_t i = 0; i != ring_history_->tail; ++i) {
            char op[32] = { 0 };
            history_op_string(ring_history_->histories[i].op, op);
            fprintf(stdout, "ope - %s %d -> %d\n", op, ring_history_->histories[i].value_before, ring_history_->histories[i].value_after);
        }
    }
cleanup:
    return;
}

static void* oc_malloc(size_t size_) {
    // TODO #ifdef TEST_BUILD
    void* ret = NULL;
    if(!s_test_param.fail_enable) {
        ret = malloc(size_);
    } else {
        if(s_test_param.malloc_counter == s_test_param.malloc_fail_n) {
            ret = NULL;
            s_test_param.malloc_counter++;
        } else {
            ret = malloc(size_);
            s_test_param.malloc_counter++;
        }
    }
    return ret;
}

// TODO: strcpyの返り値チェック
static void history_op_string(opaque_counter_op_t op_, char* const out_str_) {
    switch(op_) {
    case OPAQUE_COUNTER_OP_INC:
        strcpy(out_str_, "increment");
        break;
    case OPAQUE_COUNTER_OP_DEC:
        strcpy(out_str_, "decrement");
        break;
    case OPAQUE_COUNTER_OP_ADD:
        strcpy(out_str_, "add");
        break;
    default:
        strcpy(out_str_, "unknown");
        break;
    }
}
