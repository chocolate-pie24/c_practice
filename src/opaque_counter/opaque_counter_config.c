#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "opaque_counter/opaque_counter_config.h"

#ifdef __clang__
  #define NO_COVERAGE __attribute__((no_profile_instrument_function))
#else
  #define NO_COVERAGE
#endif

#ifdef TEST_BUILD
#include <assert.h>

static void test_oc_config_valid_check(void);

void test_oc_config(void) {
    test_oc_config_valid_check();
}
#endif

bool oc_config_valid_check(const oc_config_t* const config_) {
    bool ret = false;
    if(NULL == config_) {
        ret = false;
    } else {
        if(0 == config_->history_capacity) {
            ret = false;
        } else if(config_->min >= config_->max) {
            ret = false;
        } else if(config_->min > config_->initial) {
            ret = false;
        } else if(config_->initial > config_->max) {
            ret = false;
        } else {
            ret = true;
        }
    }
    return ret;
}

#ifdef TEST_BUILD
static NO_COVERAGE void test_oc_config_valid_check(void) {
    bool ret = false;
    ret = oc_config_valid_check(NULL);
    assert(!ret);

    oc_config_t config = { 0 };

    // min == initial -> true
    config.min = 1;
    config.initial = 1;
    config.max = 5;
    config.history_capacity = 128;
    ret = oc_config_valid_check(&config);
    assert(ret);

    // max == initial -> true
    config.min = 1;
    config.initial = 5;
    config.max = 5;
    config.history_capacity = 128;
    ret = oc_config_valid_check(&config);
    assert(ret);

    // min > max -> false
    config.min = 5;
    config.initial = 1;
    config.max = 1;
    config.history_capacity = 128;
    ret = oc_config_valid_check(&config);
    assert(!ret);

    // initial > max -> false
    config.min = 1;
    config.initial = 6;
    config.max = 5;
    config.history_capacity = 128;
    ret = oc_config_valid_check(&config);
    assert(!ret);

    // min > initial -> false
    config.min = 1;
    config.initial = 0;
    config.max = 5;
    config.history_capacity = 128;
    ret = oc_config_valid_check(&config);
    assert(!ret);

    // 0 == capacity -> false
    config.min = 1;
    config.initial = 2;
    config.max = 5;
    config.history_capacity = 0;
    ret = oc_config_valid_check(&config);
    assert(!ret);
}
#endif
