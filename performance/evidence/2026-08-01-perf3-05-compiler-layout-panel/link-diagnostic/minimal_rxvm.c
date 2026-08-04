#include "rxvm.h"

int main(void) {
    struct rxvm_context *ctx = rxvm_create();
    if (ctx == NULL) {
        return 1;
    }
    rxvm_destroy(ctx);
    return 0;
}
