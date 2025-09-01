#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h> // for memset_

// begin for test only.
#include <assert.h>
// end for test only.

#include "opaque_counter/opaque_counter.h"

#include "internal/opaque_counter_history.h"

struct opaque_counter {
    // これらはconfigオブジェクトと共通だが、labelをconst char*からchar*に変更している。
    // const char*の場合は静的領域にメモリが確保されている場合があるため、その際にfreeすると未定義動作となる。
    // configオブジェクトを持たせても良いが、config.labelをconst char*にするのを優先するためこうした
    // また、const char*が現状なくても、将来的に追加される可能性を見越してconfigをそのまま持たせない方が良い
    int32_t min;
    int32_t initial;
    int32_t max;
    char* label;
};

static void test_opaque_counter_create(void);
static void test_opaque_counter_destroy(void);

void test_opaque_counter(void) {
    test_opaque_counter_create();
    test_opaque_counter_destroy();
}

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
    tmp = malloc(sizeof(*tmp));
    if(NULL == tmp) {
        fprintf(stderr, "[ERROR](NO_MEMORY): opaque_counter_create - Failed to allocate opaque_counter_t memory.\n");
        ret = OPAQUE_COUNTER_NO_MEMORY;
        goto cleanup;
    }
    memset(tmp, 0, sizeof(*tmp));

    tmp->label = NULL;
    tmp->label = strdup(config_->label);
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
        if(NULL != tmp && NULL != tmp->label) {
            free(tmp->label);
            tmp->label = NULL;
        }
        if(NULL != tmp) {
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

// TODO: test
bool opaque_counter_config_valid_check(const opaque_counter_config_t* const config_) {
    bool ret = false;
    if(config_->min > config_->max) {
        ret = false;
    } else if(config_->min > config_->initial) {
        ret = false;
    } else if(config_->initial > config_->max) {
        ret = false;
    } else {
        ret = true;
    }
    return ret;
}

static void test_opaque_counter_create(void) {
    opaque_counter_error_t ret = OPAQUE_COUNTER_INVALID_ARGUMENT;

    {
        opaque_counter_config_t config = { 0 };
        config.initial = 1;
        config.min = 0;
        config.max = 2;
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
        config.label = "test";
        opaque_counter_t* counter = NULL;
        ret = opaque_counter_create(&counter, &config);
        assert(OPAQUE_COUNTER_SUCCESS == ret);
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
