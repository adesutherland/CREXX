#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxvm.h"

int main() {
    struct rxvm_context* ctx;
    int i;
    int rc;

    ctx = rxvm_create();
    if (!ctx) {
        fprintf(stderr, "Failed to create VM context\n");
        return 1;
    }

    if (rxvm_load_file(ctx, "test_reentrancy") == NULL) {
        fprintf(stderr, "Failed to load test_reentrancy\n");
        rxvm_destroy(ctx);
        return 1;
    }

    if (rxvm_link(ctx) != 0) {
        fprintf(stderr, "Failed to link modules\n");
        rxvm_destroy(ctx);
        return 1;
    }

    if (rxvm_prepare(ctx) != 0) {
        fprintf(stderr, "Failed to prepare context\n");
        rxvm_destroy(ctx);
        return 1;
    }

    for (i = 0; i < 10; i++) {
        char val1[32];
        char val2[32];
        char* args[2];
        int expected = i + (i * 2);

        sprintf(val1, "%d", i);
        sprintf(val2, "%d", i * 2);
        args[0] = val1;
        args[1] = val2;

        rc = rxvm_call(ctx, "reentrancy.add", 2, args);

        if (rc != expected) {
            fprintf(stderr, "Iteration %d: Expected %d, got %d\n", i, expected, rc);
            rxvm_destroy(ctx);
            return 1;
        }
        printf("Iteration %d: %d + %d = %d (OK)\n", i, i, i * 2, rc);
    }

    rxvm_destroy(ctx);
    printf("Re-entrancy test passed successfully\n");
    return 0;
}
