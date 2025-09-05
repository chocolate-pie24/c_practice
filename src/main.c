#include <stdio.h>

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
    return 0;
}
