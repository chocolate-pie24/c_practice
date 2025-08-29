#ifndef OPAQUE_COUNTER_INTERNAL_DATA_H
#define OPAQUE_COUNTER_INTERNAL_DATA_H

#include <stdint.h>

struct opaque_counter {
    int32_t counter;
};

enum {
    OPAQUE_COUNTER_MIN = 2,
    OPAQUE_COUNTER_MAX = 10,
};

#endif
