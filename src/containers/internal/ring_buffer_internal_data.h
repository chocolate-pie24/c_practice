#ifndef RING_BUFFER_INTERNAL_DATA_H
#define RING_BUFFER_INTERNAL_DATA_H

#include <stddef.h>

struct ring_buffer {
    void* memory_pool;
    size_t head;
    size_t tail;
    size_t capacity;
};

#endif
