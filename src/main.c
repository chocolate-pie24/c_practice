#include <stdio.h>

#include "practice/opaque_counter.h"

int main(int argc_, char** argv_) {
    (void)argc_;
    (void)argv_;

    opaque_counter_t* oc = NULL;
    const opaque_counter_error_t ret_create = opaque_counter_create(&oc);
    if(OPAQUE_COUNTER_SUCCESS == ret_create) {
        fprintf(stdout, "[INFO] opaque_counter created successfully.\n");

        opaque_counter_inc(oc);
        uint16_t tmp = 0;
        opaque_counter_get(oc, &tmp);
        fprintf(stdout, "[INFO] current value: %d\n", tmp);

        opaque_counter_dec(oc);
        opaque_counter_get(oc, &tmp);
        fprintf(stdout, "[INFO] current value: %d\n", tmp);

        opaque_counter_dec(oc);
        opaque_counter_get(oc, &tmp);
        fprintf(stdout, "[INFO] current value: %d\n", tmp);

        opaque_counter_destroy(&oc);
        if(NULL == oc) {
            fprintf(stdout, "[INFO] opaque_counter destroyed successfully.\n");
        }
    } else {
        fprintf(stderr, "[ERROR] Failed to create opaque_counter.\n");
        goto cleanup;
    }

cleanup:
    return 0;
}
