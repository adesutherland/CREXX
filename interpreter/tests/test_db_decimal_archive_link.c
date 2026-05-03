#include <stdio.h>
#include "rxvml.h"

int main(void) {
    rxvml_context* ctx = rxvml_create(NULL, 0);

    if (!ctx) {
        fprintf(stderr, "Failed to create rxvml context\n");
        return 1;
    }

    rxvml_destroy(ctx);
    return 0;
}
