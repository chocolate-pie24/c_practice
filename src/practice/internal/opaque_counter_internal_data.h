#ifndef OPAQUE_COUNTER_INTERNAL_DATA_H
#define OPAQUE_COUNTER_INTERNAL_DATA_H

#include <stdint.h>
#include <stddef.h> // for size_t

#include "pracitce/opaque_counter.h"

enum {
    OPAQUE_COUNTER_DEFAULT_SCRATCH_SIZE = 1024,
    OPAQUE_COUNTER_HISTROY_COUNT = 128,
};

typedef enum {
    OPAQUE_COUNTER_OPP_INC, /**< 操作: インクリメント */
    OPAQUE_COUNTER_OPP_DEC, /**< 操作: デクリメント */
    OPAQUE_COUNTER_OPP_ADD, /**< 操作: 加算/減算 */
} opaque_counter_operation_t;

typedef struct opaque_counter_operation_history {
    opaque_counter_operation_t operation;   /**< オペレーション種別(成功したら追加) */
    int32_t value_before;                   /**< 各オペレーション実行前のカウント値 */
    int32_t value_after;                    /**< 各オペレーション実行後のカウント値 */
} opaque_counter_operation_history_t;

struct opaque_counter {
    char* name;                                     /**< オブジェクトネームラベル */
    void* scrach;                                   /**< 作業用データ格納領域 */
    opaque_counter_operation_history_t* histories;  /**< 履歴用リングバッファ TODO: 型をring_buffer_tに変更 */
    opaque_counter_config_t config;                 /**< 設定値オブジェクト */
    size_t scratch_size;                            /**< 作業用データ格納領域メモリサイズ */
    size_t history_size;                            /**< 履歴記憶件数 */
    int32_t counter;                                /**< 現在のカウント値 */
};

#endif
