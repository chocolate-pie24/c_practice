#include <stdio.h>

#ifdef TEST_BUILD
#include "oc_test.h"
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
    test_oc_ring_hist();
    test_oc_config();
    test_opaque_counter();
#endif

    return 0;
}
