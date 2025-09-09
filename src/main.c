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
    for(int i = 0; i != 200; ++i ) {
        fprintf(stdout, "=== Testing test_oc_ring_hist()...\n");
        test_oc_ring_hist();
        fprintf(stdout, "=== Testing test_oc_config()...\n");
        test_oc_config();
        fprintf(stdout, "=== Testing test_opaque_counter()...\n");
        test_opaque_counter();
        fprintf(stdout, "=== %d success.\n", i + 1);
    }

#endif

    return 0;
}
