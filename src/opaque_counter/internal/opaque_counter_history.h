#ifndef OPAQUE_COUNTER_HISTORY_H
#define OPAQUE_COUNTER_HISTORY_H

#include <stdint.h>

typedef enum {
    OPAQUE_COUNTER_OP_INC,
    OPAQUE_COUNTER_OP_DEC,
    OPAQUE_COUNTER_OP_ADD,
} opaque_counter_op_t;

typedef struct opaque_counter_history {
    opaque_counter_op_t op;
    int32_t value_before;
    int32_t value_after;
} opaque_counter_history_t;

#endif
