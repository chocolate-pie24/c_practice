#ifndef OPAQUE_COUNTER_H
#define OPAQUE_COUNTER_H

#include <stdint.h>

// opaque_counterを前方宣言(内部データは隠蔽)し、opaque_counter_tを公開
// opaque_counterはタグ名前空間/opaque_counter_tは通常の名前空間
// 同一オブジェクトに複数ポインタ禁止
typedef struct opaque_counter opaque_counter_t;

typedef enum {
    OPAQUE_COUNTER_SUCCESS,
    OPAQUE_COUNTER_INVALID_ARGUMENT,
    OPAQUE_COUNTER_RUNTIME_ERROR,
    OPAQUE_COUNTER_NO_MEMORY,
    OPAQUE_COUNTER_OVERFLOW,
    OPAQUE_COUNTER_UNDERFLOW,
} opaque_counter_error_t;

// opaque_counter_t型のメモリをout_opaque_counter_に確保する(カウンタの値は0で初期化される)
// out_opaque_counter_ == NULL or *out_opaque_counter_ != NULLはOPAQUE_COUNTER_INVALID_ARGUMENT
// 失敗時は *out を一切変更しない
opaque_counter_error_t opaque_counter_create(opaque_counter_t** out_counter_);

// opaque_counter_が保持しているメモリを全て解放しopaque_counter_にNULLをセットする
// opaque_counter_ == NULL or *opaque_counter_ == NULLは何もしない(ログも出力せず成功扱い)
void opaque_counter_destroy(opaque_counter_t** opaque_counter_);

opaque_counter_error_t opaque_counter_inc(opaque_counter_t* counter_);

opaque_counter_error_t opaque_counter_dec(opaque_counter_t* counter_);

// delta_==0 は成功no-op／範囲外は OVERFLOW/UNDERFLOW／成功時のみ更新／counter==NULLはINVALID_ARGUMENT
opaque_counter_error_t opaque_counter_add(opaque_counter_t* counter_, int32_t delta_);

opaque_counter_error_t opaque_counter_get(const opaque_counter_t* const counter_, uint16_t* out_value_);

#endif
