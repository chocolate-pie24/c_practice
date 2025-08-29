#include <stdio.h>

#include "pracitce/opaque_counter.h"

int main(int argc_, char** argv_) {
    (void)argc_;
    (void)argv_;

    opaque_counter_create_ex(0, 0, 0);

    return 0;
}
