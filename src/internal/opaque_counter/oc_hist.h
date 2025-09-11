#ifndef OC_HIST_H
#define OC_HIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    OC_OP_ADD,
    OC_OP_INC,
    OC_OP_DEC,
} oc_op_t;

typedef struct oc_hist {
    oc_op_t opp;
    int32_t before_value;
    int32_t after_value;
} oc_hist_t;

#ifdef __cplusplus
}
#endif
#endif
