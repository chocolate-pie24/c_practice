#include <stdio.h>
#include <assert.h>
#include <stdlib.h> // for abs()

#include "practice/opaque_counter.h"

#include "practice/internal/opaque_counter_internal_data.h" // for opaque_counter test

static void test_create(void);
static void test_destroy(void);
static void test_inc(void);
static void test_dec(void);
static void test_add(void);
static void test_get(void);

int main(int argc_, char** argv_) {
    (void)argc_;
    (void)argv_;

    test_create();
    test_destroy();
    test_inc();
    test_dec();
    test_add();
    test_get();

    return 0;
}

static void test_create(void) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_SUCCESS;

    opaque_counter_t* tmp = NULL;
    ret = opaque_counter_create(NULL);
    assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);

    tmp = malloc(sizeof(int));
    ret = opaque_counter_create(&tmp);
    assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);
    free(tmp);
    tmp = NULL;

    ret = opaque_counter_create(&tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    opaque_counter_destroy(&tmp);
    assert(NULL == tmp);
}

static void test_destroy(void) {
    opaque_counter_destroy(NULL);

    opaque_counter_t* tmp = NULL;
    opaque_counter_destroy(&tmp);

    opaque_counter_error_t ret = opaque_counter_create(&tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);
    assert(NULL != tmp);
    opaque_counter_destroy(&tmp);
    assert(NULL == tmp);
    opaque_counter_destroy(&tmp);
}

static void test_inc(void) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    opaque_counter_t* tmp = NULL;
    ret = opaque_counter_inc(tmp);
    assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);

    ret = opaque_counter_create(&tmp);

    assert(NULL != tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    int32_t counter = 0;
    opaque_counter_error_t ret2 = OPAQUE_COUNTER_INVALID_ARGUMENT;
    for(int32_t i = OPAQUE_COUNTER_MIN; i != (OPAQUE_COUNTER_MAX + 1); ++i) {
        ret = opaque_counter_inc(tmp);
        int32_t val = 0;
        ret2 = opaque_counter_get(tmp, &val);
        if(OPAQUE_COUNTER_MAX != i) {
            assert((OPAQUE_COUNTER_MIN + counter + 1) == val);
            assert(OPAQUE_COUNTER_SUCCESS == ret);
            assert(OPAQUE_COUNTER_SUCCESS == ret2);
        } else {
            assert((OPAQUE_COUNTER_MIN + counter) == val);
            assert(OPAQUE_COUNTER_OVERFLOW == ret);
            assert(OPAQUE_COUNTER_SUCCESS == ret2);
        }
        counter++;
    }

    opaque_counter_destroy(&tmp);
    assert(NULL == tmp);
}

static void test_dec(void) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    opaque_counter_t* tmp = NULL;
    ret = opaque_counter_inc(tmp);
    assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);

    ret = opaque_counter_create(&tmp);

    assert(NULL != tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_inc(tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_inc(tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_dec(tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_dec(tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_dec(tmp);
    assert(OPAQUE_COUNTER_UNDERFLOW == ret);

    opaque_counter_destroy(&tmp);
    assert(NULL == tmp);
}

static void test_add(void) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    opaque_counter_t* tmp = NULL;

    ret = opaque_counter_create(&tmp);
    assert(NULL != tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_add(tmp, OPAQUE_COUNTER_MAX - OPAQUE_COUNTER_MIN);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    opaque_counter_destroy(&tmp);
    assert(NULL == tmp);

    ret = opaque_counter_create(&tmp);
    assert(NULL != tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_add(tmp, OPAQUE_COUNTER_MAX - OPAQUE_COUNTER_MIN + 1);
    assert(OPAQUE_COUNTER_OVERFLOW == ret);

    opaque_counter_destroy(&tmp);
    assert(NULL == tmp);

    ret = opaque_counter_create(&tmp);
    assert(NULL != tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_add(tmp, abs(OPAQUE_COUNTER_MIN));
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_add(tmp, -1 * abs(OPAQUE_COUNTER_MIN));
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    opaque_counter_destroy(&tmp);
    assert(NULL == tmp);

    ret = opaque_counter_create(&tmp);
    assert(NULL != tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_add(tmp, 0);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    opaque_counter_destroy(&tmp);
    assert(NULL == tmp);

    ret = opaque_counter_create(&tmp);
    assert(NULL != tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_add(tmp, abs(OPAQUE_COUNTER_MIN));
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_add(tmp, -1 * abs(OPAQUE_COUNTER_MIN));
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    opaque_counter_destroy(&tmp);
    assert(NULL == tmp);

    ret = opaque_counter_create(&tmp);
    assert(NULL != tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_add(tmp, -1 * (abs(OPAQUE_COUNTER_MIN) + 1));
    assert(OPAQUE_COUNTER_UNDERFLOW == ret);

    opaque_counter_destroy(&tmp);
    assert(NULL == tmp);
}

static void test_get(void) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    int32_t value = 0;
    opaque_counter_t* tmp = NULL;
    ret = opaque_counter_get(0, &value);
    assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);
    ret = opaque_counter_get(tmp, 0);
    assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);
    ret = opaque_counter_get(0, 0);
    assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);

    ret = opaque_counter_create(&tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);

    ret = opaque_counter_inc(tmp);
    assert(OPAQUE_COUNTER_SUCCESS == ret);
    ret = opaque_counter_get(tmp, &value);
    assert((1 + OPAQUE_COUNTER_MIN) == value);

    opaque_counter_destroy(&tmp);
    tmp = NULL;
}
