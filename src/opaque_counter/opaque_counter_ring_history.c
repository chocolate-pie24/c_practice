#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> // for strcpy, memset

#include "internal/opaque_counter_ring_history.h"
#include "internal/opaque_counter_history.h"

#ifdef TEST_BUILD
#include <assert.h>

typedef struct test_param {
    uint16_t malloc_counter;    // malloc実行回数
    uint16_t malloc_fail_n;     // n回目のmallocで失敗
    bool fail_enable;
} test_param_t;

static test_param_t s_test_param;

static void test_opaque_counter_ring_history_create(void);
static void test_opaque_counter_ring_history_destroy(void);
static void test_opaque_counter_ring_history_push(void);

void test_opaque_counter_ring_history(void) {
    test_opaque_counter_ring_history_create();
    test_opaque_counter_ring_history_destroy();
    test_opaque_counter_ring_history_push();
}
#endif

static void* oc_malloc(size_t size_);
static void history_op_string(opaque_counter_op_t op_, char* const out_str_);

oc_ring_history_error_t opaque_counter_ring_history_create(opaque_counter_ring_history_t* history_, size_t capacity_) {
    oc_ring_history_error_t ret = OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT;
    opaque_counter_history_t* tmp = NULL;
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
    tmp = oc_malloc(sizeof(opaque_counter_history_t) * capacity_);
    if(NULL == tmp) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_ring_history_create - Failed to allocate hisotry buffer memory.\n");
        ret = OPAQUE_COUNTER_RING_HISTORY_NO_MEMORY;
        goto cleanup;
    }
    memset(tmp, 0, sizeof(opaque_counter_history_t) * capacity_);
    history_->capacity = capacity_;
    history_->head = 0;
    history_->tail = 0;
    history_->len = 0;
    history_->histories = tmp;
    ret = OPAQUE_COUNTER_RING_HISTORY_SUCCESS;

cleanup:
    if(OPAQUE_COUNTER_RING_HISTORY_SUCCESS != ret) {
        if(NULL != tmp) {
            free(tmp);
            tmp = NULL;
        }
    }
    return ret;
}

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
    void* ret = NULL;
#ifdef TEST_BUILD
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
#endif
#ifndef TEST_BUILD
    ret = malloc(size_);
#endif
    return ret;
}

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

#ifdef TEST_BUILD
static void test_opaque_counter_ring_history_create(void) {
    {
        s_test_param.fail_enable = true;
        s_test_param.malloc_counter = 0;
        s_test_param.malloc_fail_n = 0;
        opaque_counter_ring_history_t history = { 0 };
        // 初回のmallocで失敗
        oc_ring_history_error_t ret = opaque_counter_ring_history_create(&history, 128);
        assert(OPAQUE_COUNTER_RING_HISTORY_NO_MEMORY == ret);
        opaque_counter_ring_history_destroy(&history);
        assert(0 == history.capacity);
        assert(0 == history.head);
        assert(0 == history.len);
        assert(0 == history.tail);
        assert(NULL == history.histories);
    }
    {
        s_test_param.fail_enable = false;
        opaque_counter_ring_history_t history = { 0 };
        // capacity == 0で失敗
        oc_ring_history_error_t ret = opaque_counter_ring_history_create(&history, 0);
        assert(OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT == ret);
        opaque_counter_ring_history_destroy(&history);
        assert(0 == history.capacity);
        assert(0 == history.head);
        assert(0 == history.len);
        assert(0 == history.tail);
        assert(NULL == history.histories);
    }
    {
        s_test_param.fail_enable = false;
        // history == NULLで失敗
        oc_ring_history_error_t ret = opaque_counter_ring_history_create(NULL, 128);
        assert(OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT == ret);
    }
    {
        s_test_param.fail_enable = false;
        opaque_counter_ring_history_t history = { 0 };
        oc_ring_history_error_t ret = opaque_counter_ring_history_create(&history, 128);
        assert(OPAQUE_COUNTER_RING_HISTORY_SUCCESS == ret);
        opaque_counter_ring_history_destroy(&history);
        assert(0 == history.capacity);
        assert(0 == history.head);
        assert(0 == history.len);
        assert(0 == history.tail);
        assert(NULL == history.histories);
    }
    {
        s_test_param.fail_enable = false;
        opaque_counter_ring_history_t history = { 0 };
        oc_ring_history_error_t ret = opaque_counter_ring_history_create(&history, 128);
        assert(OPAQUE_COUNTER_RING_HISTORY_SUCCESS == ret);
        // history->histories != NULLで失敗
        ret = opaque_counter_ring_history_create(&history, 128);
        assert(OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT == ret);
        opaque_counter_ring_history_destroy(&history);
        assert(0 == history.capacity);
        assert(0 == history.head);
        assert(0 == history.len);
        assert(0 == history.tail);
        assert(NULL == history.histories);
    }
}

static void test_opaque_counter_ring_history_destroy(void) {
    {
        opaque_counter_ring_history_destroy(NULL);

        opaque_counter_ring_history_t history = { 0 };
        opaque_counter_ring_history_destroy(&history);
    }
    {
        s_test_param.fail_enable = false;
        opaque_counter_ring_history_t history = { 0 };
        oc_ring_history_error_t ret = opaque_counter_ring_history_create(&history, 128);
        assert(OPAQUE_COUNTER_RING_HISTORY_SUCCESS == ret);
        opaque_counter_ring_history_destroy(&history);
        assert(0 == history.capacity);
        assert(0 == history.head);
        assert(0 == history.len);
        assert(0 == history.tail);
        assert(NULL == history.histories);
        opaque_counter_ring_history_destroy(&history);
    }
}

static void test_opaque_counter_ring_history_push(void) {
    {
        opaque_counter_history_t history = { 0 };
        history.op = OPAQUE_COUNTER_OP_ADD;
        history.value_after = 10;
        history.value_before = 5;
        // ring_history == NULLで失敗
        oc_ring_history_error_t ret = opaque_counter_ring_history_push(NULL, &history);
        assert(OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT == ret);
    }
    {
        s_test_param.fail_enable = false;
        opaque_counter_ring_history_t ring_history = { 0 };
        oc_ring_history_error_t ret = opaque_counter_ring_history_create(&ring_history, 128);
        assert(OPAQUE_COUNTER_RING_HISTORY_SUCCESS == ret);
        // history == NULLで失敗
        ret = opaque_counter_ring_history_push(&ring_history, NULL);
        assert(OPAQUE_COUNTER_RING_HISTORY_INVALID_ARGUMENT == ret);
    }
    {
        s_test_param.fail_enable = false;
        opaque_counter_ring_history_t ring_history = { 0 };
        oc_ring_history_error_t ret = opaque_counter_ring_history_create(&ring_history, 128);
        assert(OPAQUE_COUNTER_RING_HISTORY_SUCCESS == ret);
        opaque_counter_history_t history = { 0 };
        history.op = OPAQUE_COUNTER_OP_ADD;
        history.value_after = 10;
        history.value_before = 5;
        ret = opaque_counter_ring_history_push(&ring_history, &history);
        assert(OPAQUE_COUNTER_RING_HISTORY_SUCCESS == ret);
    }
    {
        fprintf(stdout, "====================\n");
        opaque_counter_ring_history_t history = { 0 };
        oc_ring_history_error_t history_error = opaque_counter_ring_history_create(&history, 4);
        for(size_t i = 0; i != 16; ++i) {
            opaque_counter_history_t tmp;
            tmp.op = OPAQUE_COUNTER_OP_INC;
            tmp.value_before = i;
            tmp.value_after = i + 1;
            opaque_counter_ring_history_push(&history, &tmp);
            fprintf(stdout, "====================\n");
            opaque_counter_ring_history_print(&history);
        }
    }
}
#endif
