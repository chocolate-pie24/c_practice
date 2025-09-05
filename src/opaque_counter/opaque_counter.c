#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h> // for memset_

#include "opaque_counter/opaque_counter.h"

#include "internal/opaque_counter_history.h"
#include "internal/opaque_counter_ring_history.h"

#ifdef TEST_BUILD
#include <assert.h>

typedef struct test_param {
    uint16_t malloc_counter;    // malloc実行回数
    uint16_t malloc_fail_n;     // n回目のmallocで失敗
    bool fail_enable;
} test_param_t;

static void test_opaque_counter_create(void);
static void test_opaque_counter_destroy(void);
static void test_opaque_counter_config_valid_check(void);

static test_param_t s_test_param;

void test_opaque_counter(void) {
    test_opaque_counter_create();
    test_opaque_counter_destroy();
    test_opaque_counter_config_valid_check();
}
#endif

struct opaque_counter {
    // これらはconfigオブジェクトと共通だが、labelをconst char*からchar*に変更している。
    // const char*の場合は静的領域にメモリが確保されている場合があるため、その際にfreeすると未定義動作となる。
    // configオブジェクトを持たせても良いが、config.labelをconst char*にするのを優先するためこうした
    // また、const char*が現状なくても、将来的に追加される可能性を見越してconfigをそのまま持たせない方が良い
    size_t history_cap;
    int32_t min;
    int32_t initial;
    int32_t max;
    char* label;
    opaque_counter_ring_history_t ring_history;
};

static void* oc_malloc(size_t size_);
static char* oc_strdup(const char* const str_);

// TODO: TEST_BUILDでカバレッジ計測
opaque_counter_error_t opaque_counter_create(opaque_counter_t** counter_, const opaque_counter_config_t* const config_) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    opaque_counter_t* tmp = NULL;
    if(NULL == counter_ || NULL == config_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create - Arguments counter_ and config_ require valid pointers.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL != *counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create - Argument *counter_ requires a null pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL == config_->label) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create - Provided config_.label is null pointer.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(!opaque_counter_config_valid_check(config_)) {
        fprintf(stderr, "[ERORR](INVALID_ARGUMENT): opaque_counter_create - Provided config_ is not valid.\n");
        ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
        goto cleanup;
    }
    tmp = oc_malloc(sizeof(*tmp));
    if(NULL == tmp) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create - Failed to allocate opaque_counter_t memory.\n");
        ret = OPAQUE_COUNTER_NO_MEMORY;
        goto cleanup;
    }
    memset(tmp, 0, sizeof(*tmp));

    tmp->ring_history.histories = NULL;
    oc_ring_history_error_t ring_history_result = opaque_counter_ring_history_create(&tmp->ring_history, config_->history_cap);
    if(OPAQUE_COUNTER_RING_HISTORY_SUCCESS != ring_history_result) {
        fprintf(stderr, "[ERROR](RUNTIME_ERROR): opaque_counter_create - Failed to create history ring buffer.\n");
        ret = OPAQUE_COUNTER_RUNTIME_ERROR;
        goto cleanup;
    }

    tmp->label = NULL;
    tmp->label = oc_strdup(config_->label);
    if(NULL == tmp->label) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create - Failed to allocate name label memory.\n");
        ret = OPAQUE_COUNTER_NO_MEMORY;
        goto cleanup;
    }
    tmp->initial = config_->initial;
    tmp->max = config_->max;
    tmp->min = config_->min;
    *counter_ = tmp;
    ret = OPAQUE_COUNTER_SUCCESS;

cleanup:
    if(OPAQUE_COUNTER_SUCCESS != ret) {
        if(NULL != tmp) {
            opaque_counter_ring_history_destroy(&tmp->ring_history);
            if(NULL != tmp->label) {
                free(tmp->label);
                tmp->label = NULL;
            }
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
    opaque_counter_t* tmp = *counter_;
    if(NULL != tmp->label) {
        free(tmp->label);
        tmp->label = NULL;
    }
    free(tmp);
    tmp = NULL;
    *counter_ = NULL;
cleanup:
    return;
}

bool opaque_counter_config_valid_check(const opaque_counter_config_t* const config_) {
    bool ret = false;
    if(NULL == config_) {
        ret = false;
    } else if(config_->min > config_->max) {
        ret = false;
    } else if(config_->min > config_->initial) {
        ret = false;
    } else if(config_->initial > config_->max) {
        ret = false;
    } else if(0 == config_->history_cap) {
        ret = false;
    } else {
        ret = true;
    }
    return ret;
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

static char* oc_strdup(const char* const str_) {
    char* ret = NULL;
#ifdef TEST_BUILD
    if(!s_test_param.fail_enable) {
        ret = strdup(str_);
    } else {
        if(s_test_param.malloc_counter == s_test_param.malloc_fail_n) {
            ret = NULL;
            s_test_param.malloc_counter++;
        } else {
            ret = strdup(str_);
            s_test_param.malloc_counter++;
        }
    }
#endif
#ifndef TEST_BUILD
    ret = strdup(str_);
#endif
    return ret;
}

#ifdef TEST_BUILD
static void test_opaque_counter_create(void) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;
    s_test_param.fail_enable = false;
    s_test_param.malloc_counter = 0;
    s_test_param.malloc_fail_n = 0; // 初回のmallocで失敗させる

    {
        opaque_counter_config_t config = { 0 };
        config.initial = 1;
        config.min = 0;
        config.max = 2;
        config.history_cap = 4;
        config.label = "test";
        ret = opaque_counter_create(NULL, &config);
        assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);
    }
    {
        opaque_counter_t* counter = NULL;
        ret = opaque_counter_create(&counter, NULL);
        assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);
    }
    {
        opaque_counter_config_t config = { 0 };
        config.initial = 1;
        config.min = 0;
        config.max = 2;
        config.history_cap = 4;
        config.label = NULL;
        opaque_counter_t* counter = NULL;
        ret = opaque_counter_create(&counter, &config);
        assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);
    }
    {
        opaque_counter_config_t config = { 0 };
        config.initial = 1;
        config.min = 0;
        config.max = 2;
        config.history_cap = 4;
        config.label = "test";
        opaque_counter_t* counter = NULL;
        ret = opaque_counter_create(&counter, &config);
        assert(OPAQUE_COUNTER_SUCCESS == ret);

        ret = opaque_counter_create(&counter, &config);
        assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret); // *counter_ != NULL
    }
    {
        opaque_counter_config_t config = { 0 };
        config.initial = 1;
        config.min = 0;
        config.max = 2;
        config.history_cap = 0;
        config.label = "test";
        opaque_counter_t* counter = NULL;
        ret = opaque_counter_create(&counter, &config);
        assert(OPAQUE_COUNTER_INVALID_ARGUMENT == ret);
    }
    {
        opaque_counter_config_t config = { 0 };
        config.initial = 1;
        config.min = 0;
        config.max = 2;
        config.history_cap = 4;
        config.label = "test";
        opaque_counter_t* counter = NULL;
        ret = opaque_counter_create(&counter, &config);
        assert(OPAQUE_COUNTER_SUCCESS == ret);
    }
    {
        s_test_param.fail_enable = true;
        s_test_param.malloc_counter = 0;
        s_test_param.malloc_fail_n = 0; // 初回のmallocで失敗させる
        opaque_counter_config_t config = { 0 };
        config.initial = 1;
        config.min = 0;
        config.max = 2;
        config.history_cap = 4;
        config.label = "test";
        opaque_counter_t* counter = NULL;
        ret = opaque_counter_create(&counter, &config);
        assert(OPAQUE_COUNTER_NO_MEMORY == ret);
    }
    {
        s_test_param.fail_enable = true;
        s_test_param.malloc_counter = 0;
        s_test_param.malloc_fail_n = 1; // 2回目のmallocで失敗させる
        opaque_counter_config_t config = { 0 };
        config.initial = 1;
        config.min = 0;
        config.max = 2;
        config.history_cap = 4;
        config.label = "test";
        opaque_counter_t* counter = NULL;
        ret = opaque_counter_create(&counter, &config);
        assert(OPAQUE_COUNTER_NO_MEMORY == ret);
    }
}

static void test_opaque_counter_destroy(void) {
    {
        opaque_counter_destroy(NULL);
    }
    {
        opaque_counter_t* counter = NULL;
        opaque_counter_destroy(&counter);
    }
    {
        opaque_counter_config_t config = { 0 };
        config.initial = 1;
        config.min = 0;
        config.max = 2;
        config.history_cap = 4;
        config.label = "test";
        opaque_counter_t* counter = NULL;
        opaque_counter_error_t ret = opaque_counter_create(&counter, &config);
        assert(OPAQUE_COUNTER_SUCCESS == ret);
        assert(NULL != counter);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
}

static void test_opaque_counter_config_valid_check(void) {
    {
        // 正常
        opaque_counter_config_t config = { 0 };
        config.history_cap = 128;
        config.initial = 10;
        config.label = "test";
        config.max = 20;
        config.min = 10;
        bool ret = opaque_counter_config_valid_check(&config);
        assert(ret);
    }
    {
        // 正常
        opaque_counter_config_t config = { 0 };
        config.history_cap = 128;
        config.initial = 20;
        config.label = "test";
        config.max = 20;
        config.min = 10;
        bool ret = opaque_counter_config_valid_check(&config);
        assert(ret);
    }
    {
        // 正常
        opaque_counter_config_t config = { 0 };
        config.history_cap = 128;
        config.initial = 15;
        config.label = "test";
        config.max = 20;
        config.min = 10;
        bool ret = opaque_counter_config_valid_check(&config);
        assert(ret);
    }
    {
        // 異常(histroy_cap = 0)
        opaque_counter_config_t config = { 0 };
        config.history_cap = 0;
        config.initial = 15;
        config.label = "test";
        config.max = 20;
        config.min = 10;
        bool ret = opaque_counter_config_valid_check(&config);
        assert(!ret);
    }
    {
        // 異常(min > max)
        opaque_counter_config_t config = { 0 };
        config.history_cap = 128;
        config.initial = 15;
        config.label = "test";
        config.max = 10;
        config.min = 20;
        bool ret = opaque_counter_config_valid_check(&config);
        assert(!ret);
    }
    {
        // 異常(initial > max)
        opaque_counter_config_t config = { 0 };
        config.history_cap = 128;
        config.initial = 30;
        config.label = "test";
        config.max = 20;
        config.min = 10;
        bool ret = opaque_counter_config_valid_check(&config);
        assert(!ret);
    }
    {
        // 異常(min > initial)
        opaque_counter_config_t config = { 0 };
        config.history_cap = 128;
        config.initial = 0;
        config.label = "test";
        config.max = 20;
        config.min = 10;
        bool ret = opaque_counter_config_valid_check(&config);
        assert(!ret);
    }
    {
        // 異常(config == NULL)
        bool ret = opaque_counter_config_valid_check(NULL);
        assert(!ret);
    }
}
#endif
