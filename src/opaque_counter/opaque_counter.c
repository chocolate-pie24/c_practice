#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "opaque_counter/opaque_counter.h"
#include "opaque_counter/opaque_counter_config.h"

#include "internal/oc_hist.h"
#include "internal/oc_ring_hist.h"

#ifdef __clang__
  #define NO_COVERAGE __attribute__((no_profile_instrument_function))
#else
  #define NO_COVERAGE
#endif

/*
TODO: opaque_counter_create()
- [x] test_oc_malloc()
- [x] oc_malloc実装
- [x] oc_config_valid_check()実装(opaque_counter_config.c)
- [] [WIP]test_opaque_counter_create()
- [x] oc_strdup()
- [] oc_ring_hist_createエラーを発生させる@test_opaque_counter_create

TODO: その他
- [] TEST_BUILDではなく、TEST_ENABLEにしてRELEASEとDEBUGでもテストできるようにする
- [] docsにカバレッジ関連マニュアルを置く
*/

#ifdef TEST_BUILD
#include <assert.h>
typedef struct oc_test_param {
    bool strdup_fail;
} oc_test_param_t;

typedef struct oc_malloc_test {
    uint32_t oc_malloc_counter;    // oc_malloc呼び出し回数(malloc failでもインクリメントされる)
    uint32_t oc_malloc_fail_n;
    bool fail_enable;
} oc_malloc_test_t;

static oc_test_param_t s_strdup_test_param;
static oc_malloc_test_t s_malloc_test_param;

static void test_oc_malloc(void);
static void test_opaque_counter_create(void);
static void test_opaque_counter_destroy(void);
static void test_opaque_counter_inc(void);
static void test_opaque_counter_dec(void);
static void test_opaque_counter_add(void);

void test_opaque_counter(void) {
    fprintf(stdout, "\t=== Testing test_oc_malloc\n");
    test_oc_malloc();
    fprintf(stdout, "\t=== Testing test_opaque_counter_create\n");
    test_opaque_counter_create();
    fprintf(stdout, "\t=== Testing test_opaque_counter_destroy\n");
    test_opaque_counter_destroy();
    fprintf(stdout, "\t=== Testing test_opaque_counter_inc\n");
    test_opaque_counter_inc();
    fprintf(stdout, "\t=== Testing test_opaque_counter_dec\n");
    test_opaque_counter_dec();
    fprintf(stdout, "\t=== Testing test_opaque_counter_add\n");
    test_opaque_counter_add();
}
#endif

typedef struct opaque_counter {
    char* name_label;
    int32_t min;
    int32_t max;
    int32_t initial;
    int32_t counter;
    oc_ring_hist_t* history;
};

static void* oc_malloc(size_t size_);
static char* oc_strdup(const char* str_);

oc_error_t opaque_counter_create(opaque_counter_t** counter_, const oc_config_t* const config_) {
    oc_error_t ret = OC_INVALID_ARGUMENT;
    oc_ring_hist_err_t ret_hist_create = OC_RING_HIST_INVALID_ARGUMENT;
    opaque_counter_t* tmp = NULL;
    if(NULL == counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create - Argument counter_ requires a valid pointer.\n");
        ret = OC_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL != *counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create - Argument *counter_ requires a null pointer.\n");
        ret = OC_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(!oc_config_valid_check(config_)) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create - Provided config_ is not valid.\n");
        ret = OC_INVALID_ARGUMENT;
        goto cleanup;
    }
    tmp = (opaque_counter_t*)oc_malloc(sizeof(*tmp));
    if(NULL == tmp) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create - Failed to allocate opaque_counter memory.\n");
        ret = OC_NO_MEMORY;
        goto cleanup;
    }
    tmp->min = config_->min;
    tmp->max = config_->max;
    tmp->initial = config_->initial;
    tmp->counter = tmp->initial;
    tmp->name_label = oc_strdup(config_->name_label);
    if(NULL == tmp->name_label) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create - Failed to allocate name_label memory.\n");
        ret = OC_NO_MEMORY;
        goto cleanup;
    }

    tmp->history = oc_malloc(sizeof(*tmp->history));
    if(NULL == tmp->history) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create - Failed to allocate history memory.\n");
        ret = OC_NO_MEMORY;
        goto cleanup;
    }
    ret_hist_create = oc_ring_hist_create(tmp->history, config_->history_capacity);
    if(OC_RING_HIST_INVALID_ARGUMENT == ret_hist_create) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_create - Failed to create opaque counter history.\n");
        ret = OC_INVALID_ARGUMENT;
        goto cleanup;
    } else if(OC_RING_HIST_NO_MEMORY == ret_hist_create) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create - Failed to create opaque counter history.\n");
        ret = OC_NO_MEMORY;
        goto cleanup;
    } else if(OC_RING_HIST_SUCCESS != ret_hist_create) {
        fprintf(stderr, "[ERROR](UNDEFINED): opaque_counter_create - Failed to create opaque counter history.\n");
        ret = OC_UNDEFINED;
        goto cleanup;
    }

    *counter_ = tmp;
    ret = OC_SUCCESS;

cleanup:
    if(OC_SUCCESS != ret) {
        if(NULL != tmp) {
            if(NULL != tmp->history) {
                oc_ring_hist_destroy(tmp->history);
                free(tmp->history);
                tmp->history = NULL;
            }
            if(NULL != tmp->name_label) {
                free(tmp->name_label);
                tmp->name_label = NULL;
            }
            free(tmp);
            tmp = NULL;
        }
    }
    return ret;
}

void opaque_counter_destroy(opaque_counter_t** counter_) {
    if(NULL == counter_) {
        goto cleanup;
    }
    if(NULL == *counter_) {
        goto cleanup;
    }

    oc_ring_hist_destroy((*counter_)->history);
    if(NULL != (*counter_)->history) {
        free((*counter_)->history);
        (*counter_)->history = NULL;
    }
    if(NULL != (*counter_)->name_label) {
        free((*counter_)->name_label);
        (*counter_)->name_label = NULL;
    }
    (*counter_)->initial = 0;
    (*counter_)->max = 0;
    (*counter_)->min = 0;
    (*counter_)->counter = 0;

    free(*counter_);
    *counter_ = NULL;
cleanup:
    return;
}

oc_error_t opaque_counter_inc(opaque_counter_t* const counter_) {
    oc_error_t ret = OC_INVALID_ARGUMENT;
    if(NULL == counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_inc - Argument counter_ requires a valid pointer.\n");
        ret = OC_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL == counter_->history) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_inc - Provided counter_->history is not valid.\n");
        ret = OC_INVALID_ARGUMENT;
        goto cleanup;
    }
    int64_t tmp = (int64_t)counter_->counter + (int64_t)1;
    if(tmp > (int64_t)counter_->max) {
        fprintf(stderr, "[ERROR](OVERFLOW): opaque_counter_inc - Opaque counter overflow.\n");
        ret = OC_OVERFLOW;
        goto cleanup;
    }
    counter_->counter = (int32_t)tmp;
    ret = OC_SUCCESS;
cleanup:
    return ret;
}

oc_error_t opaque_counter_dec(opaque_counter_t* const counter_) {
    oc_error_t ret = OC_INVALID_ARGUMENT;
    if(NULL == counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_dec - Argument counter_ requires a valid pointer.\n");
        ret = OC_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL == counter_->history) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_dec - Provided counter_->history is not valid.\n");
        ret = OC_INVALID_ARGUMENT;
        goto cleanup;
    }
    int64_t tmp = (int64_t)counter_->counter - (int64_t)1;
    if(tmp < (int64_t)counter_->min) {
        fprintf(stderr, "[ERROR](UNDERFLOW): opaque_counter_dec - Opaque counter underflow.\n");
        ret = OC_UNDERFLOW;
        goto cleanup;
    }
    counter_->counter = (int32_t)tmp;
    ret = OC_SUCCESS;
cleanup:
    return ret;
}

oc_error_t opaque_counter_add(opaque_counter_t* const counter_, int32_t delta_) {
    oc_error_t ret = OC_INVALID_ARGUMENT;
    if(NULL == counter_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_add - Argument counter_ requires a valid pointer.\n");
        ret = OC_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL == counter_->history) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): opaque_counter_add - Provided counter_->history is not valid.\n");
        ret = OC_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(0 == delta_) {
        ret = OC_SUCCESS;
        goto cleanup;
    }
    int64_t tmp = (int64_t)counter_->counter + (int64_t)delta_;
    if(tmp > counter_->max) {
        fprintf(stderr, "[ERROR](OVERFLOW): opaque_counter_add - Opaque counter overflow.\n");
        ret = OC_OVERFLOW;
        goto cleanup;
    } else if(tmp < counter_->min) {
        fprintf(stderr, "[ERROR](UNDERFLOW): opaque_counter_add - Opaque counter underflow.\n");
        ret = OC_UNDERFLOW;
        goto cleanup;
    }
    counter_->counter = (int32_t)tmp;
    ret = OC_SUCCESS;
cleanup:
    return ret;
}

static void* oc_malloc(size_t size_) {
    void* ret = NULL;
#ifdef TEST_BUILD
    if(!s_malloc_test_param.fail_enable) {
        ret = malloc(size_);
    } else if(s_malloc_test_param.oc_malloc_counter != s_malloc_test_param.oc_malloc_fail_n) {
        ret = malloc(size_);
    } else {
        ret = NULL;
    }
    s_malloc_test_param.oc_malloc_counter++;
#else
    ret = malloc(size_);
#endif
    return ret;
}

static char* oc_strdup(const char* str_) {
    char* ret = NULL;
#ifdef TEST_BUILD
    if(s_strdup_test_param.strdup_fail) {
        ret = NULL;
    } else {
        ret = strdup(str_);
    }
#else
    ret = strdup(str_);
#endif

    return ret;
}

#ifdef TEST_BUILD
static NO_COVERAGE void test_oc_malloc(void) {
    int* tmp = NULL;

    // テストケース1: テスト無効でmalloc成功
    s_malloc_test_param.fail_enable = false;
    s_malloc_test_param.oc_malloc_counter = 0;
    s_malloc_test_param.oc_malloc_fail_n = 0;
    tmp = NULL;
    tmp = (int*)oc_malloc(sizeof(*tmp));
    assert(NULL != tmp);
    free(tmp);
    tmp = NULL;

    // テストケース2: テスト有効、かつ、失敗するmalloc実行回数未達でmalloc成功
    s_malloc_test_param.fail_enable = true;
    s_malloc_test_param.oc_malloc_counter = 0;
    s_malloc_test_param.oc_malloc_fail_n = 1;
    tmp = NULL;
    tmp = (int*)oc_malloc(sizeof(*tmp));
    assert(NULL != tmp);
    free(tmp);
    tmp = NULL;

    // テストケース3: テスト有効かつ、指定したmalloc実行回数でmalloc失敗
    s_malloc_test_param.fail_enable = true;
    s_malloc_test_param.oc_malloc_counter = 0;
    s_malloc_test_param.oc_malloc_fail_n = 1;

    // 1回目はmalloc成功
    tmp = NULL;
    tmp = (int*)oc_malloc(sizeof(*tmp));
    assert(NULL != tmp);
    free(tmp);
    tmp = NULL;

    // 2回目で失敗
    tmp = (int*)oc_malloc(sizeof(*tmp));
    assert(NULL == tmp);

    s_malloc_test_param.fail_enable = false;
    s_malloc_test_param.oc_malloc_counter = 0;
    s_malloc_test_param.oc_malloc_fail_n = 0;
}

static void NO_COVERAGE test_opaque_counter_create(void) {
    oc_error_t ret = OC_INVALID_ARGUMENT;
    oc_config_t config = { 0 };

    // counter == NULLでinvalid
    config.history_capacity = 128;
    config.initial = 5;
    config.min = 1;
    config.max = 10;
    config.name_label = "test_label";
    ret = opaque_counter_create(NULL, &config);
    assert(OC_INVALID_ARGUMENT == ret);

    // config == NULLでinvalid
    opaque_counter_t* counter = NULL;
    ret = opaque_counter_create(&counter, NULL);
    assert(OC_INVALID_ARGUMENT == ret);

    // capacity == 0でinvalid
    config.history_capacity = 0;
    config.initial = 5;
    config.min = 1;
    config.max = 10;
    config.name_label = "test_label";
    ret = opaque_counter_create(&counter, &config);
    assert(OC_INVALID_ARGUMENT == ret);

    // mallocエラーでno_memory
    config.history_capacity = 128;
    config.initial = 5;
    config.min = 1;
    config.max = 10;
    config.name_label = "test_label";
    s_malloc_test_param.fail_enable = true;
    s_malloc_test_param.oc_malloc_counter = 0;
    s_malloc_test_param.oc_malloc_fail_n = 0;
    ret = opaque_counter_create(&counter, &config);
    assert(OC_NO_MEMORY == ret);
    s_malloc_test_param.fail_enable = false;
    s_malloc_test_param.oc_malloc_counter = 0;
    s_malloc_test_param.oc_malloc_fail_n = 0;

    // strdupエラーでno_memory
    s_strdup_test_param.strdup_fail = true;
    ret = opaque_counter_create(&counter, &config);
    assert(OC_NO_MEMORY == ret);
    s_strdup_test_param.strdup_fail = false;

    // 2回目のmalloc失敗でno_memory
    s_malloc_test_param.fail_enable = true;
    s_malloc_test_param.oc_malloc_counter = 0;
    s_malloc_test_param.oc_malloc_fail_n = 1;
    ret = opaque_counter_create(&counter, &config);
    assert(OC_NO_MEMORY == ret);
    s_malloc_test_param.fail_enable = false;
    s_malloc_test_param.oc_malloc_counter = 0;
    s_malloc_test_param.oc_malloc_fail_n = 0;

    // success
    ret = opaque_counter_create(&counter, &config);
/*
    config.history_capacity = 128;
    config.initial = 5;
    config.min = 1;
    config.max = 10;
    config.name_label = "test_label";
*/
    assert(5 == counter->initial);
    assert(1 == counter->min);
    assert(10 == counter->max);
    assert(5 == counter->counter);
    assert(0 == strcmp("test_label", counter->name_label));
    assert(OC_SUCCESS == ret);

    // counter->histories != NULLでinvalid
    ret = opaque_counter_create(&counter, &config);
    assert(OC_INVALID_ARGUMENT == ret);

    opaque_counter_destroy(&counter);
    assert(NULL == counter);
}

static NO_COVERAGE void test_opaque_counter_destroy(void) {
    opaque_counter_destroy(NULL);

    {
        opaque_counter_t* counter = NULL;
        opaque_counter_destroy(&counter);
    }

    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.initial = 5;
        config.min = 1;
        config.max = 10;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        oc_error_t ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);

        opaque_counter_destroy(&counter);
        assert(NULL == counter);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
}

static NO_COVERAGE void test_opaque_counter_inc(void) {
    oc_error_t ret = OC_INVALID_ARGUMENT;

    {
        ret = opaque_counter_inc(NULL);
        assert(OC_INVALID_ARGUMENT == ret);
    }
    {
        opaque_counter_t tmp = { 0 };
        ret = opaque_counter_inc(&tmp);
        assert(OC_INVALID_ARGUMENT == ret);
    }
    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.min = 1;
        config.max = 10;
        config.initial = config.max;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);
        ret = opaque_counter_inc(counter);
        assert(counter->counter == config.initial);
        assert(OC_OVERFLOW == ret);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.min = 1;
        config.max = 10;
        config.initial = config.max - 1;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);
        ret = opaque_counter_inc(counter);
        assert(counter->counter == (config.initial + 1));
        assert(OC_SUCCESS == ret);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.min = 1;
        config.max = 10;
        config.initial = config.min;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);
        ret = opaque_counter_inc(counter);
        assert(counter->counter == (config.initial + 1));
        assert(OC_SUCCESS == ret);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.min = 1;
        config.max = INT32_MAX;
        config.initial = config.max;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);
        ret = opaque_counter_inc(counter);
        assert(counter->counter == config.initial);
        assert(OC_OVERFLOW == ret);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
}

static void NO_COVERAGE test_opaque_counter_dec(void) {
    {
        oc_error_t ret = opaque_counter_dec(NULL);
        assert(OC_INVALID_ARGUMENT == ret);
    }
    {
        opaque_counter_t counter = { 0 };
        oc_error_t ret = opaque_counter_dec(&counter);
        assert(OC_INVALID_ARGUMENT == ret);
    }
    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.initial = INT32_MIN;
        config.min = INT32_MIN;
        config.max = 10;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        oc_error_t ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);

        ret = opaque_counter_dec(counter);
        assert(counter->counter == config.initial);
        assert(OC_UNDERFLOW == ret);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.initial = 0;
        config.min = 0;
        config.max = 10;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        oc_error_t ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);

        ret = opaque_counter_dec(counter);
        assert(counter->counter == config.initial);
        assert(OC_UNDERFLOW == ret);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.initial = 5;
        config.min = 0;
        config.max = 10;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        oc_error_t ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);

        ret = opaque_counter_dec(counter);
        assert(counter->counter == (config.initial - 1));
        assert(OC_SUCCESS == ret);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
}

static void NO_COVERAGE test_opaque_counter_add(void) {
    {
        oc_error_t ret = opaque_counter_add(NULL, 10);
        assert(OC_INVALID_ARGUMENT == ret);
    }
    {
        opaque_counter_t counter = { 0 };
        oc_error_t ret = opaque_counter_add(&counter, 10);
        assert(OC_INVALID_ARGUMENT == ret);
    }
    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.initial = INT32_MIN;
        config.min = INT32_MIN;
        config.max = 10;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        oc_error_t ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);

        ret = opaque_counter_add(counter, 0);
        assert(counter->counter == config.initial);
        assert(OC_SUCCESS == ret);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.initial = 0;
        config.min = 0;
        config.max = 10;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        oc_error_t ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);

        ret = opaque_counter_add(counter, -1);
        assert(counter->counter == config.initial);
        assert(OC_UNDERFLOW == ret);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.initial = 10;
        config.min = 0;
        config.max = 10;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        oc_error_t ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);

        ret = opaque_counter_add(counter, 1);
        assert(counter->counter == config.initial);
        assert(OC_OVERFLOW == ret);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.initial = 5;
        config.min = 0;
        config.max = 10;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        oc_error_t ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);

        ret = opaque_counter_add(counter, 2);
        assert(counter->counter == (config.initial + 2));
        assert(OC_SUCCESS == ret);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
    {
        oc_config_t config = { 0 };
        config.history_capacity = 128;
        config.initial = 5;
        config.min = 0;
        config.max = 10;
        config.name_label = "test_label";

        opaque_counter_t* counter = NULL;
        oc_error_t ret = opaque_counter_create(&counter, &config);
        assert(OC_SUCCESS == ret);

        ret = opaque_counter_add(counter, -2);
        assert(counter->counter == (config.initial - 2));
        assert(OC_SUCCESS == ret);
        opaque_counter_destroy(&counter);
        assert(NULL == counter);
    }
}
#endif
