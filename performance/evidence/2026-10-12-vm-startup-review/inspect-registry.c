#include <stdio.h>
#include "rxvmintp.h"
#include "rxvm.h"

int main(int argc, char **argv) {
    rxvm_context *ctx = rxvm_create();
    int result;
    if (!ctx || argc != 2 || !rxvm_load_file(ctx, argv[1])) return 2;
    result = rxvm_link(ctx);
    printf("link=%d methods=%zu factories=%zu method_dirty=%d factory_dirty=%d\n",
           result, ctx->num_interface_methods, ctx->num_interface_factories,
           ctx->interface_method_registry_dirty, ctx->interface_factory_registry_dirty);
    rxvm_destroy(ctx);
    return result != 0;
}
