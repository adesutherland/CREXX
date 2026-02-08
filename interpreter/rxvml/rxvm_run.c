#include <stdio.h>
#include <string.h>
#include "../../inc/rxvm.h"
#include "../rxvmintp.h"
#include "rxastree.h"

struct module* rxvm_load(struct rxvm_context* ctx, char* filename) {
    int n = rxldmod(ctx, filename);
    if (n > 0) {
        return ctx->modules[n-1];
    }
    return NULL;
}

int rxvm_link(struct rxvm_context* ctx) {
    size_t i;
    for (i = 0; i < ctx->num_modules; i++) {
        if (ctx->modules[i]->state < RXVM_MOD_LINKED) {
            rxvm_link_module(ctx, i);
            ctx->modules[i]->state = RXVM_MOD_LINKED;
        }
    }
    return 0;
}

int rxvm_prepare(struct rxvm_context* ctx) {
    /* rxvm_prepare is idempotent because run() in rxvmintp.c checks the state of each module */
    ctx->prepare_only = 1;
    run(ctx, 0, NULL);
    ctx->prepare_only = 0;
    return 0;
}

int rxvm_call(struct rxvm_context* ctx, char* proc_name, int argc, char** argv) {
    if (proc_name && strcmp(proc_name, "main") != 0) {
        proc_constant* p = NULL;
        if (src_node(ctx->exposed_proc_tree, proc_name, (size_t*)&p)) {
            ctx->ext_proc = p;
        } else {
            fprintf(stderr, "ERROR: Procedure %s not found\n", proc_name);
            return -1;
        }
    }
    
    int rc = run(ctx, argc, argv);
    
    ctx->ext_proc = NULL;
    return rc;
}

int rxvm_run(struct rxvm_context* ctx, int argc, char** argv) {
    rxvm_link(ctx);
    rxvm_prepare(ctx);
    return rxvm_call(ctx, "main", argc, argv);
}
