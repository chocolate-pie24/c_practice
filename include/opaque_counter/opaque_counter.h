#ifndef OPAQUE_COUNTER_H
#define OPAQUE_COUNTER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    OPAQUE_COUNTER_SUCCESS,
    OPAQUE_COUNTER_INVALID_ARGUMENT,
    OPAQUE_COUNTER_RUNTIME_ERROR,
    OPAQUE_COUNTER_OVERFLOW,
    OPAQUE_COUNTER_UNDERFLOW,
    OPAQUE_COUNTER_NO_MEMORY,
} opaque_counter_error_t;

typedef struct opaque_counter opaque_counter_t;

typedef struct opaque_counter_config {
    const char* label;  /**< メモリは借用、既存文字列バッファのアドレスを差し替え、freeの責務はユーザー側。
                             これを直接freeしない。借用元をfreeする(その際はlabelはダングリングポインタになる)。
                             char*としないことでPODオブジェクトになり、コピーが容易 */
    int32_t min;
    int32_t max;
    int32_t initial;
} opaque_counter_config_t;

opaque_counter_error_t opaque_counter_create(opaque_counter_t** counter_, const opaque_counter_config_t* const config_);

void opaque_counter_destroy(opaque_counter_t** counter_);

bool opaque_counter_config_valid_check(const opaque_counter_config_t* const config_);

// test用に一時的に置いておく(そのうち移動)
void test_opaque_counter(void);

#endif
