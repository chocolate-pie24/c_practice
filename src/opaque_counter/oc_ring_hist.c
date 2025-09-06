#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memset

#include "internal/oc_hist.h"
#include "internal/oc_ring_hist.h"

/*
TODO: 2025/09/06...09/07
- [x] oc_malloc
- [x] test_oc_malloc
- [x] oc_ring_hist_create
- [x] oc_ring_hist_destroy
- [x] test_oc_ring_hist_create
- [x] test_oc_ring_hist_destroy
- [x] test_oc_ring_hist_createにoc_ring_hist_destroyを追加
- [x] #ifdef TEST_BUILD
- [x] testディレクトリ,テスト実行
- [] カバレッジ計測スクリプト
  - [] カバレッジ100%
  - [] スクリプト
  - [] カバレッジGitHub表示方法
*/

/*
TODO:
- [] oc_ring_hist_test_param_set
- [] test_oc_ring_hist_test_param_set
*/

#ifdef TEST_BUILD
#include <assert.h>
static oc_ring_hist_malloc_test_t s_test_param;
static void test_oc_malloc(void);
// static void test_oc_ring_hist_create(void);
// static void test_oc_ring_hist_destroy(void);

void test_oc_ring_hist(void) {
    test_oc_malloc();
    // test_oc_ring_hist_destroy();
    // test_oc_ring_hist_create();
}
#endif

static void* oc_malloc(size_t size_);

// ring_history_ == NULLでOC_RING_HIST_INVALID_ARGUMENT
// capacity_ == 0でOC_RING_HIST_INVALID_ARGUMENT
// ring_history_->histories != NULLでOC_RING_HIST_INVALID_ARGUMENT
// oc_hist_t配列メモリ確保失敗でOC_RING_HIST_NO_MEMORY
// oc_ring_hist_err_t oc_ring_hist_create(oc_ring_hist_t* const ring_history_, size_t capacity_) {
//     oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
//     oc_hist_t* tmp = NULL;
//     if(NULL == ring_history_) {
//         fprintf(stderr, "[ERROR](INVALID_ARGUMENTI): oc_ring_hist_create - Argument ring_history_ requires a valid pointer.\n");
//         ret = OC_RING_HIST_INVALID_ARGUMENT;
//         goto cleanup;
//     }
//     if(0 == capacity_) {
//         fprintf(stderr, "[ERROR](INVALID_ARGUMENT): oc_ring_hist_create - Provided capacity_ is not valid.\n");
//         ret = OC_RING_HIST_INVALID_ARGUMENT;
//         goto cleanup;
//     }
//     if(NULL != ring_history_->histories) {
//         fprintf(stderr, "[ERROR](INVALID_ARGUMENT): oc_ring_hist_create - Provided ring_history_ is not valid.\n");
//         ret = OC_RING_HIST_INVALID_ARGUMENT;
//         goto cleanup;
//     }
//     tmp = oc_malloc(sizeof(*tmp));
//     if(NULL == tmp) {
//         fprintf(stderr, "[ERROR](NO_MEMORY): oc_ring_hist_create - Failed to allocate oc_hist_t memory.\n");
//         ret = OC_RING_HIST_NO_MEMORY;
//         goto cleanup;
//     }
//     memset(tmp, 0, sizeof(*tmp));
//     ring_history_->capacity = capacity_;
//     ring_history_->head = 0;
//     ring_history_->tail = 0;
//     ring_history_->len = 0;
//     ring_history_->histories = tmp;
//     ret = OC_RING_HIST_SUCCESS;
// cleanup:
//     if(OC_RING_HIST_SUCCESS != ret) {
//         if(NULL != tmp) {
//             free(tmp);
//             tmp = NULL;
//         }
//     }
//     return ret;
// }

// ring_history_ == NULLでno-op
// void oc_ring_hist_destroy(oc_ring_hist_t* const ring_history_) {
//     if(NULL == ring_history_) {
//         goto cleanup;
//     }
//     if(NULL != ring_history_->histories) {
//         free(ring_history_->histories);
//         ring_history_->histories = NULL;
//     }
//     ring_history_->capacity = 0;
//     ring_history_->head = 0;
//     ring_history_->len = 0;
//     ring_history_->tail = 0;
// cleanup:
//     return;
// }

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
#endif
#ifndef TEST_BUILD
    ret = malloc(size_);
#endif
    return ret;
}

#ifdef TEST_BUILD
static void test_oc_malloc(void) {
    s_test_param.fail_enable = false;
    s_test_param.oc_malloc_counter = 0;
    s_test_param.oc_malloc_fail_n = 0;
    { (void)0;
        s_test_param.fail_enable = false;
        s_test_param.oc_malloc_counter = 0;
        s_test_param.oc_malloc_fail_n = 0;
        int* tmp = NULL;
        tmp = oc_malloc(sizeof(*tmp));
        assert(NULL != tmp);
        free(tmp);
        tmp = NULL;
    }
    { (void)0;
        s_test_param.fail_enable = true;
        s_test_param.oc_malloc_counter = 0;
        s_test_param.oc_malloc_fail_n = 1;
        int* tmp = NULL;
        tmp = oc_malloc(sizeof(*tmp));
        assert(NULL != tmp);
        free(tmp);
        tmp = NULL;
    }
    { (void)0;
        s_test_param.fail_enable = true;
        s_test_param.oc_malloc_counter = 0;
        s_test_param.oc_malloc_fail_n = 1;

        // 1回目はmalloc成功
        int* tmp = NULL;
        tmp = oc_malloc(sizeof(*tmp));
        assert(NULL != tmp);
        free(tmp);
        tmp = NULL;

        // 2回目で失敗
        tmp = oc_malloc(sizeof(*tmp));
        assert(NULL == tmp);
    }
    s_test_param.fail_enable = false;
    s_test_param.oc_malloc_counter = 0;
    s_test_param.oc_malloc_fail_n = 0;
}

// static void test_oc_ring_hist_create(void) {
//     s_test_param.fail_enable = false;
//     s_test_param.oc_malloc_counter = 0;
//     s_test_param.oc_malloc_fail_n = 0;
//     {
//         oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
//         ret = oc_ring_hist_create(NULL, 128);   // ring_history_ == NULLでOC_RING_HIST_INVALID_ARGUMENT
//         assert(OC_RING_HIST_INVALID_ARGUMENT == ret);
//     }
//     {
//         oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
//         oc_ring_hist_t ring_history = { 0 };
//         ret = oc_ring_hist_create(&ring_history, 0);    // capacity_ == 0でOC_RING_HIST_INVALID_ARGUMENT
//         assert(OC_RING_HIST_INVALID_ARGUMENT == ret);
//         oc_ring_hist_destroy(&ring_history);
//     }
//     {
//         oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
//         oc_ring_hist_t ring_history = { 0 };
//         ret = oc_ring_hist_create(&ring_history, 128);
//         assert(OC_RING_HIST_SUCCESS == ret);

//         ret = oc_ring_hist_create(&ring_history, 128);  // ring_history_->histories != NULLでOC_RING_HIST_INVALID_ARGUMENT
//         assert(OC_RING_HIST_INVALID_ARGUMENT == ret);
//         oc_ring_hist_destroy(&ring_history);
//     }
//     {
//         s_test_param.fail_enable = true;
//         s_test_param.oc_malloc_counter = 0;
//         s_test_param.oc_malloc_fail_n = 0;
//         oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
//         oc_ring_hist_t ring_history = { 0 };
//         ret = oc_ring_hist_create(&ring_history, 128);  // 初回のmallocで失敗し、OC_RING_HIST_NO_MEMORY
//         assert(OC_RING_HIST_NO_MEMORY == ret);
//         oc_ring_hist_destroy(&ring_history);
//     }
//     s_test_param.fail_enable = false;
//     s_test_param.oc_malloc_counter = 0;
//     s_test_param.oc_malloc_fail_n = 0;
// }

// static void test_oc_ring_hist_destroy(void) {
//     {
//         oc_ring_hist_destroy(NULL);
//     }
//     {
//         oc_ring_hist_t ring_history = { 0 };
//         oc_ring_hist_destroy(&ring_history);
//         assert(NULL == ring_history.histories);
//         assert(0 == ring_history.capacity);
//         assert(0 == ring_history.head);
//         assert(0 == ring_history.len);
//         assert(0 == ring_history.tail);
//     }
//     {
//         oc_ring_hist_err_t ret = OC_RING_HIST_INVALID_ARGUMENT;
//         oc_ring_hist_t ring_history = { 0 };
//         ret = oc_ring_hist_create(&ring_history, 128);
//         assert(OC_RING_HIST_SUCCESS == ret);
//         oc_ring_hist_destroy(&ring_history);
//         assert(NULL == ring_history.histories);
//         assert(0 == ring_history.capacity);
//         assert(0 == ring_history.head);
//         assert(0 == ring_history.len);
//         assert(0 == ring_history.tail);
//         oc_ring_hist_destroy(&ring_history);
//     }
//     {
//         oc_ring_hist_t ring_history = { 0 };
//         ring_history.capacity = 100;
//         ring_history.head = 200;
//         ring_history.len = 300;
//         ring_history.tail = 400;
//         oc_ring_hist_destroy(&ring_history);
//         assert(NULL == ring_history.histories);
//         assert(0 == ring_history.capacity);
//         assert(0 == ring_history.head);
//         assert(0 == ring_history.len);
//         assert(0 == ring_history.tail);
//     }
// }
#endif
