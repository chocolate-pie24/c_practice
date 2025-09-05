#include <stdio.h>

#include "opaque_counter/opaque_counter.h"

#ifdef TEST_BUILD
#include "../test/opaque_counter/test_opaque_counter.h"
#endif

int main(int argc_, char** argv_) {
    (void)argc_;
    (void)argv_;

#ifdef RELEASE_BUILD
    fprintf(stdout, "build mode = release.\n");
#endif
#ifdef DEBUG_BUILD
    fprintf(stdout, "build mode = debug.\n");
#endif
#ifdef TEST_BUILD
    fprintf(stdout, "build mode = test.\n");
#endif

#ifdef TEST_BUILD
    test_opaque_counter_ring_history();
    test_opaque_counter();
#endif

    return 0;
}
