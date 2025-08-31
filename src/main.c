#include <stdio.h>

#include "pracitce/opaque_counter.h"

int main(int argc_, char** argv_) {
    (void)argc_;
    (void)argv_;

    test_opaque_counter_create_ex();
    test_opaque_counter_destroy();

    return 0;
}
