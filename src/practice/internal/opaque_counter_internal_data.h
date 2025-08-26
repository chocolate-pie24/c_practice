#ifndef OPAQUE_COUNTER_INTERNAL_DATA_H
#define OPAQUE_COUNTER_INTERNAL_DATA_H

#include <stdint.h>

struct opaque_counter {
    uint16_t value;
};

enum {
    OPAQUE_COUNTER_MIN = 0,
    OPAQUE_COUNTER_MAX = 10,
};

#endif
