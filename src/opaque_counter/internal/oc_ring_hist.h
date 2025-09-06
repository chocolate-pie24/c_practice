#ifndef OC_RING_HIST_H
#define OC_RING_HIST_H

#include <stddef.h>
#include <stdbool.h>

#include "oc_hist.h"

typedef enum {
    OC_RING_HIST_SUCCESS,
    OC_RING_HIST_INVALID_ARGUMENT,
    OC_RING_HIST_NO_MEMORY,
} oc_ring_hist_err_t;

typedef struct oc_ring_hist {
    oc_hist_t* histories;
    size_t head;
    size_t tail;
    size_t len;
    size_t capacity;
} oc_ring_hist_t;

#ifdef TEST_BUILD
typedef struct oc_ring_hist_malloc_test {
    uint32_t oc_malloc_counter;    // oc_malloc呼び出し回数(malloc failでもインクリメントされる)
    uint32_t oc_malloc_fail_n;
    bool fail_enable;
} oc_ring_hist_malloc_test_t;

void oc_ring_hist_test_param_set(const oc_ring_hist_malloc_test_t* const test_param_);
#endif

oc_ring_hist_err_t oc_ring_hist_create(oc_ring_hist_t* const ring_history_, size_t capacity_);

void oc_ring_hist_destroy(oc_ring_hist_t* const ring_history_);

#endif
