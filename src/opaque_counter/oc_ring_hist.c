#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memset
#include <limits.h> // for SIZE_MAX

#include "opaque_counter/opaque_counter.h"

#include "opaque_counter/oc_hist.h"
#include "opaque_counter/oc_ring_hist.h"

#include "define.h"

#ifdef TEST_BUILD
#include <assert.h>
static oc_ring_hist_malloc_test_t s_test_param;
static void test_oc_malloc(void);
static void test_oc_ring_hist_create(void);
static void test_oc_ring_hist_destroy(void);
static void test_oc_ring_hist_push(void);

void test_oc_ring_hist(void) {
    test_oc_malloc();
    test_oc_ring_hist_destroy();
    test_oc_ring_hist_create();
    test_oc_ring_hist_push();
}
#endif

/*
TODO:
- [x] エラー処理マクロ化
- [] pushをvoidに変更
*/

#define CHECK_ARG_NULL_GOTO_CLEANUP(ptr_, function_name_, val_name_) \
    if(NULL == ptr_) { \
        fprintf(stderr, "[ERROR](OC_RING_HIST::INVALID_ARGUMENT): %s - Argument %s requires a valid pointer.\n", function_name_, val_name_); \
        ret = OC_RING_HIST_INVALID_ARGUMENT; \
        goto cleanup; \
    } \

#define CHECK_ARG_NOT_NULL_GOTO_CLEANUP(ptr_, function_name_, val_name_) \
    if(NULL != ptr_) { \
        fprintf(stderr, "[ERROR](OC_RING_HIST::INVALID_ARGUMENT): %s - Argument %s requires a null pointer.\n", function_name_, val_name_); \
        ret = OC_RING_HIST_INVALID_ARGUMENT; \
        goto cleanup; \
    } \

#define CHECK_ARG_NOT_VALID_GOTO_CLEANUP(is_valid_, function_name_, val_name_) \
    if(!(is_valid_)) { \
        fprintf(stderr, "[ERROR](OC_RING_HIST::INVALID_ARGUMENT): %s - Argument %s is not valid.\n", function_name_, val_name_); \
        ret = OC_RING_HIST_INVALID_ARGUMENT; \
        goto cleanup; \
    } \

#define CHECK_ALLOCATE_FAIL_GOTO_CLEANUP(ptr_, function_name_, val_name_) \
    if(NULL == ptr_) { \
        fprintf(stderr, "[ERROR](OC_RING_HIST::NO_MEMORY): %s - Failed to allocate %s memory.\n", function_name_, val_name_); \
        ret = OC_RING_HIST_NO_MEMORY; \
        goto cleanup; \
    } \

static void* oc_malloc(size_t size_);

// ring_history_ == NULLでOC_RING_HIST_INVALID_ARGUMENT
// capacity_ == 0でOC_RING_HIST_INVALID_ARGUMENT
// ring_history_->histories != NULLでOC_RING_HIST_INVALID_ARGUMENT
// oc_hist_t配列メモリ確保失敗でOC_RING_HIST_NO_MEMORY
oc_ring_hist_err_t oc_ring_hist_create(oc_ring_hist_t* const ring_history_, size_t capacity_) {
    oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
    oc_hist_t* tmp_ring_history = NULL;

    // Preconditions
    CHECK_ARG_NULL_GOTO_CLEANUP(ring_history_, "oc_ring_hist_create", "ring_history_");
    CHECK_ARG_NOT_NULL_GOTO_CLEANUP(ring_history_->histories, "oc_ring_hist_create", "ring_history_->histories");
    CHECK_ARG_NOT_VALID_GOTO_CLEANUP(0 != capacity_, "oc_ring_hist_create", "capacity_");
    CHECK_ARG_NOT_VALID_GOTO_CLEANUP(capacity_ < SIZE_MAX / sizeof(oc_hist_t), "oc_ring_hist_create", "capacity_");

    // Simulation
    const size_t alloc_size = sizeof(oc_hist_t) * capacity_;
    tmp_ring_history = (oc_hist_t*)oc_malloc(alloc_size);
    CHECK_ALLOCATE_FAIL_GOTO_CLEANUP(tmp_ring_history, "oc_ring_hist_create", "tmp_ring_history");
    memset(tmp_ring_history, 0, alloc_size);

    // Commit
    ring_history_->capacity = capacity_;
    ring_history_->head = 0;
    ring_history_->tail = 0;
    ring_history_->len = 0;
    ring_history_->histories = tmp_ring_history;
    ret = OC_RING_HIST_SUCCESS;

cleanup:
    // 現状ではtmp_ring_history = malloc成功した後にエラーとなることはないので、tmp_ring_historyをfreeする必要はなし
    // ただ、将来的に仕様変更になった時のためにコメントとして残しておく
    // if(OC_RING_HIST_SUCCESS != ret) {
        // if(NULL != tmp_ring_history) {
        //     free(tmp_ring_history);
        //     tmp_ring_history = NULL;
        // }
    // }
    return ret;
}

oc_ring_hist_err_t oc_ring_hist_clone(const oc_ring_hist_t* const src_, oc_ring_hist_t** dst_) {
    oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
    oc_ring_hist_t* tmp = NULL;

    // Preconditions
    CHECK_ARG_NULL_GOTO_CLEANUP(src_, "oc_ring_hist_clone", "src_");
    CHECK_ARG_NULL_GOTO_CLEANUP(dst_, "oc_ring_hist_clone", "dst_");
    CHECK_ARG_NULL_GOTO_CLEANUP(src_->histories, "oc_ring_hist_clone", "src_->histories");
    CHECK_ARG_NOT_VALID_GOTO_CLEANUP(0 != src_->capacity, "oc_ring_hist_clone", "src_->capacity");
    CHECK_ARG_NOT_NULL_GOTO_CLEANUP(*dst_, "oc_ring_hist_clone", "*dst_");

    // Simulation
    tmp = oc_malloc(sizeof(*tmp));
    CHECK_ALLOCATE_FAIL_GOTO_CLEANUP(tmp, "oc_ring_hist_clone", "tmp");
    memset(tmp, 0, sizeof(*tmp));

    tmp->histories = oc_malloc(sizeof(oc_hist_t) * src_->capacity);
    CHECK_ALLOCATE_FAIL_GOTO_CLEANUP(tmp->histories, "oc_ring_hist_clone", "tmp->histories");
    memset(tmp->histories, 0, sizeof(*tmp->histories));

    memcpy(tmp->histories, src_->histories, sizeof(*tmp->histories));
    tmp->head = src_->head;
    tmp->tail = src_->tail;
    tmp->len = src_->len;
    tmp->capacity = src_->len;

    // Commit
    *dst_ = tmp;
    ret = OC_RING_HIST_SUCCESS;

cleanup:
    // Cleanup / return
    if(OC_RING_HIST_SUCCESS != ret) {
        if(NULL != tmp && NULL != tmp->histories) {
            free(tmp->histories);
            tmp->histories = NULL;
        }
        if(NULL != tmp) {
            free(tmp);
            tmp = NULL;
        }
    }

    return ret;
}

// ring_history_ == NULLでno-op
void oc_ring_hist_destroy(oc_ring_hist_t* const ring_history_) {
    if(NULL == ring_history_) {
        goto cleanup;
    }
    if(NULL != ring_history_->histories) {
        free(ring_history_->histories);
        ring_history_->histories = NULL;
    }
    ring_history_->capacity = 0;
    ring_history_->head = 0;
    ring_history_->len = 0;
    ring_history_->tail = 0;
cleanup:
    return;
}

oc_ring_hist_err_t oc_ring_hist_push(oc_ring_hist_t* const ring_history_, const oc_hist_t* const history_) {
    oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
    if(NULL == ring_history_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): oc_ring_hist_push - Argument ring_history_ requires a valid pointer.\n");
        ret = OC_RING_HIST_INVALID_ARGUMENT;
        goto cleanup;
    }
    if(NULL == history_) {
        fprintf(stderr, "[ERROR](INVALID_ARGUMENT): oc_ring_hist_push - Argument history_ requires a valid pointer.\n");
        ret = OC_RING_HIST_INVALID_ARGUMENT;
        goto cleanup;
    }
    ring_history_->histories[ring_history_->tail].opp = history_->opp;
    ring_history_->histories[ring_history_->tail].before_value = history_->before_value;
    ring_history_->histories[ring_history_->tail].after_value = history_->after_value;
    ring_history_->tail = (ring_history_->tail + 1) % ring_history_->capacity;
    if(ring_history_->len != ring_history_->capacity) {
        ring_history_->len++;
    } else {
        ring_history_->head = (ring_history_->head + 1) % ring_history_->capacity;
    }
    ret = OC_RING_HIST_SUCCESS;
cleanup:
    return ret;
}

static void* oc_malloc(size_t size_) {
    void* ret = NULL;
#ifdef TEST_BUILD
    if(!s_test_param.fail_enable) {
        ret = malloc(size_);
    } else if(s_test_param.oc_malloc_counter != s_test_param.oc_malloc_fail_n) {
        ret = malloc(size_);
    } else {
        ret = NULL;
    }
    s_test_param.oc_malloc_counter++;
#else
    ret = malloc(size_);
#endif
    return ret;
}

#ifdef TEST_BUILD
static NO_COVERAGE void test_oc_malloc(void) {
    int* tmp = NULL;

    // テストケース1: テスト無効でmalloc成功
    s_test_param.fail_enable = false;
    s_test_param.oc_malloc_counter = 0;
    s_test_param.oc_malloc_fail_n = 0;
    tmp = NULL;
    tmp = (int*)oc_malloc(sizeof(*tmp));
    assert(NULL != tmp);
    free(tmp);
    tmp = NULL;

    // テストケース2: テスト有効、かつ、失敗するmalloc実行回数未達でmalloc成功
    s_test_param.fail_enable = true;
    s_test_param.oc_malloc_counter = 0;
    s_test_param.oc_malloc_fail_n = 1;
    tmp = NULL;
    tmp = (int*)oc_malloc(sizeof(*tmp));
    assert(NULL != tmp);
    free(tmp);
    tmp = NULL;

    // テストケース3: テスト有効かつ、指定したmalloc実行回数でmalloc失敗
    s_test_param.fail_enable = true;
    s_test_param.oc_malloc_counter = 0;
    s_test_param.oc_malloc_fail_n = 1;

    // 1回目はmalloc成功
    tmp = NULL;
    tmp = (int*)oc_malloc(sizeof(*tmp));
    assert(NULL != tmp);
    free(tmp);
    tmp = NULL;

    // 2回目で失敗
    tmp = (int*)oc_malloc(sizeof(*tmp));
    assert(NULL == tmp);

    s_test_param.fail_enable = false;
    s_test_param.oc_malloc_counter = 0;
    s_test_param.oc_malloc_fail_n = 0;
}

static NO_COVERAGE void test_oc_ring_hist_create(void) {
    s_test_param.fail_enable = false;
    s_test_param.oc_malloc_counter = 0;
    s_test_param.oc_malloc_fail_n = 0;
    {
        oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
        ret = oc_ring_hist_create(NULL, 128);   // ring_history_ == NULLでOC_RING_HIST_INVALID_ARGUMENT
        assert(OC_RING_HIST_INVALID_ARGUMENT == ret);
    }
    {
        oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
        oc_ring_hist_t ring_history = { 0 };
        ret = oc_ring_hist_create(&ring_history, 0);    // capacity_ == 0でOC_RING_HIST_INVALID_ARGUMENT
        assert(OC_RING_HIST_INVALID_ARGUMENT == ret);
        oc_ring_hist_destroy(&ring_history);
    }
    {
        oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
        oc_ring_hist_t ring_history = { 0 };
        ret = oc_ring_hist_create(&ring_history, 128);
        assert(OC_RING_HIST_SUCCESS == ret);

        ret = oc_ring_hist_create(&ring_history, 128);  // ring_history_->histories != NULLでOC_RING_HIST_INVALID_ARGUMENT
        assert(OC_RING_HIST_INVALID_ARGUMENT == ret);
        oc_ring_hist_destroy(&ring_history);
    }
    {
        oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
        oc_ring_hist_t ring_history = { 0 };
        ret = oc_ring_hist_create(&ring_history, SIZE_MAX);
        assert(OC_RING_HIST_INVALID_ARGUMENT == ret);
        oc_ring_hist_destroy(&ring_history);
    }
    {
        s_test_param.fail_enable = true;
        s_test_param.oc_malloc_counter = 0;
        s_test_param.oc_malloc_fail_n = 0;
        oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
        oc_ring_hist_t ring_history = { 0 };
        ret = oc_ring_hist_create(&ring_history, 128);  // 初回のmallocで失敗し、OC_RING_HIST_NO_MEMORY
        assert(OC_RING_HIST_NO_MEMORY == ret);
        oc_ring_hist_destroy(&ring_history);
    }
    s_test_param.fail_enable = false;
    s_test_param.oc_malloc_counter = 0;
    s_test_param.oc_malloc_fail_n = 0;
}

static NO_COVERAGE void test_oc_ring_hist_destroy(void) {
    {
        oc_ring_hist_destroy(NULL);
    }
    {
        oc_ring_hist_t ring_history = { 0 };
        oc_ring_hist_destroy(&ring_history);
        assert(NULL == ring_history.histories);
        assert(0 == ring_history.capacity);
        assert(0 == ring_history.head);
        assert(0 == ring_history.len);
        assert(0 == ring_history.tail);
    }
    {
        oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
        oc_ring_hist_t ring_history = { 0 };
        ret = oc_ring_hist_create(&ring_history, 128);
        assert(OC_RING_HIST_SUCCESS == ret);
        oc_ring_hist_destroy(&ring_history);
        assert(NULL == ring_history.histories);
        assert(0 == ring_history.capacity);
        assert(0 == ring_history.head);
        assert(0 == ring_history.len);
        assert(0 == ring_history.tail);
        oc_ring_hist_destroy(&ring_history);
    }
    {
        oc_ring_hist_t ring_history = { 0 };
        ring_history.capacity = 100;
        ring_history.head = 200;
        ring_history.len = 300;
        ring_history.tail = 400;
        oc_ring_hist_destroy(&ring_history);
        assert(NULL == ring_history.histories);
        assert(0 == ring_history.capacity);
        assert(0 == ring_history.head);
        assert(0 == ring_history.len);
        assert(0 == ring_history.tail);
    }
}

static NO_COVERAGE void test_oc_ring_hist_push(void) {
    oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
    oc_ring_hist_t ring_history = { 0 };
    ret = oc_ring_hist_create(&ring_history, 3);
    assert(OC_RING_HIST_SUCCESS == ret);

    oc_ring_hist_err_t push_ret = OC_RING_HIST_INVALID_ARGUMENT;
    oc_hist_t hist = { 0 };

    push_ret = oc_ring_hist_push(NULL, &hist);
    assert(OC_RING_HIST_INVALID_ARGUMENT == push_ret);

    push_ret = oc_ring_hist_push(&ring_history, NULL);
    assert(OC_RING_HIST_INVALID_ARGUMENT == push_ret);

    hist.opp = OC_OP_ADD;
    hist.before_value = 1;
    hist.after_value = 2;
    push_ret = oc_ring_hist_push(&ring_history, &hist);
    assert(OC_RING_HIST_SUCCESS == push_ret);
    assert(ring_history.head == 0);
    assert(ring_history.len == 1);
    assert(ring_history.tail == 1);
    assert(ring_history.histories[0].opp == OC_OP_ADD);
    assert(ring_history.histories[0].before_value == 1);
    assert(ring_history.histories[0].after_value == 2);
    assert(ring_history.histories[1].opp == 0);
    assert(ring_history.histories[1].before_value == 0);
    assert(ring_history.histories[1].after_value == 0);
    assert(ring_history.histories[2].opp == 0);
    assert(ring_history.histories[2].before_value == 0);
    assert(ring_history.histories[2].after_value == 0);

    hist.opp = OC_OP_INC;
    hist.before_value = 3;
    hist.after_value = 4;
    push_ret = oc_ring_hist_push(&ring_history, &hist);
    assert(OC_RING_HIST_SUCCESS == push_ret);
    assert(ring_history.head == 0);
    assert(ring_history.len == 2);
    assert(ring_history.tail == 2);
    assert(ring_history.histories[0].opp == OC_OP_ADD);
    assert(ring_history.histories[0].before_value == 1);
    assert(ring_history.histories[0].after_value == 2);
    assert(ring_history.histories[1].opp == OC_OP_INC);
    assert(ring_history.histories[1].before_value == 3);
    assert(ring_history.histories[1].after_value == 4);
    assert(ring_history.histories[2].opp == 0);
    assert(ring_history.histories[2].before_value == 0);
    assert(ring_history.histories[2].after_value == 0);

    hist.opp = OC_OP_DEC;
    hist.before_value = 5;
    hist.after_value = 6;
    push_ret = oc_ring_hist_push(&ring_history, &hist);
    assert(OC_RING_HIST_SUCCESS == push_ret);
    assert(ring_history.head == 0);
    assert(ring_history.len == 3);
    assert(ring_history.tail == 0);
    assert(ring_history.histories[0].opp == OC_OP_ADD);
    assert(ring_history.histories[0].before_value == 1);
    assert(ring_history.histories[0].after_value == 2);
    assert(ring_history.histories[1].opp == OC_OP_INC);
    assert(ring_history.histories[1].before_value == 3);
    assert(ring_history.histories[1].after_value == 4);
    assert(ring_history.histories[2].opp == OC_OP_DEC);
    assert(ring_history.histories[2].before_value == 5);
    assert(ring_history.histories[2].after_value == 6);

    hist.opp = OC_OP_ADD;
    hist.before_value = 7;
    hist.after_value = 8;
    push_ret = oc_ring_hist_push(&ring_history, &hist);
    assert(OC_RING_HIST_SUCCESS == push_ret);
    assert(ring_history.head == 1);
    assert(ring_history.len == 3);
    assert(ring_history.tail == 1);
    assert(ring_history.histories[0].opp == OC_OP_ADD);
    assert(ring_history.histories[0].before_value == 7);
    assert(ring_history.histories[0].after_value == 8);
    assert(ring_history.histories[1].opp == OC_OP_INC);
    assert(ring_history.histories[1].before_value == 3);
    assert(ring_history.histories[1].after_value == 4);
    assert(ring_history.histories[2].opp == OC_OP_DEC);
    assert(ring_history.histories[2].before_value == 5);
    assert(ring_history.histories[2].after_value == 6);

    hist.opp = OC_OP_INC;
    hist.before_value = 9;
    hist.after_value = 10;
    push_ret = oc_ring_hist_push(&ring_history, &hist);
    assert(OC_RING_HIST_SUCCESS == push_ret);
    assert(ring_history.head == 2);
    assert(ring_history.len == 3);
    assert(ring_history.tail == 2);
    assert(ring_history.histories[0].opp == OC_OP_ADD);
    assert(ring_history.histories[0].before_value == 7);
    assert(ring_history.histories[0].after_value == 8);
    assert(ring_history.histories[1].opp == OC_OP_INC);
    assert(ring_history.histories[1].before_value == 9);
    assert(ring_history.histories[1].after_value == 10);
    assert(ring_history.histories[2].opp == OC_OP_DEC);
    assert(ring_history.histories[2].before_value == 5);
    assert(ring_history.histories[2].after_value == 6);
}
#endif
