#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../inc/rxvm.h"
#include "../rxvmintp.h"
#include "../rxvmvars.h"
#include "rxastree.h"
#include "rxvmplugin.h"

struct rxvm_context* rxvm_create() {
    struct rxvm_context* ctx;

    /* Initialize mandatory plugins */
    CALL_PLUGIN_INITIALIZER(decnumber);

    ctx = calloc(1, sizeof(struct rxvm_context));
    if (!ctx) return NULL;
    rxinimod(ctx);
    return ctx;
}

void rxvm_destroy(struct rxvm_context* ctx) {
    if (ctx) {
        rxfremod(ctx);
        free(ctx);
    }
}

struct module* rxvm_load(struct rxvm_context* ctx, char* filename) {
    int n = rxldmod(ctx, filename);
    if (n > 0) {
        return ctx->modules[n-1];
    }
    return NULL;
}

struct module* rxvm_load_file(struct rxvm_context* ctx, char* filename) {
    return rxvm_load(ctx, filename);
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
    int rc;
    value* ret_val = value_f();
    ctx->ext_ret = ret_val;

    if (proc_name && strcmp(proc_name, "main") != 0) {
        proc_constant* p = NULL;
        if (src_node(ctx->exposed_proc_tree, proc_name, (size_t*)&p)) {
            int i;
            ctx->ext_proc = p;
            ctx->ext_argc = argc;
            ctx->ext_args = malloc(sizeof(value*) * argc);
            for (i = 0; i < argc; i++) {
                ctx->ext_args[i] = value_f();
                set_null_string(ctx->ext_args[i], argv[i]);
            }
        } else {
            fprintf(stderr, "ERROR: Procedure %s not found\n", proc_name);
            clear_value(ret_val);
            free(ret_val);
            ctx->ext_ret = NULL;
            return -1;
        }
    }

    rc = run(ctx, argc, argv);

    if (ctx->ext_proc) {
        int i;
        for (i = 0; i < ctx->ext_argc; i++) {
            clear_value(ctx->ext_args[i]);
            free(ctx->ext_args[i]);
        }
        free(ctx->ext_args);
        ctx->ext_proc = NULL;
        ctx->ext_argc = 0;
        ctx->ext_args = NULL;
    }

    /* If it's an integer return, use it as rc */
    if (ret_val->status.type_int) {
        rc = (int)ret_val->int_value;
    }

    clear_value(ret_val);
    free(ret_val);
    ctx->ext_ret = NULL;

    return rc;
}

int rxvm_run(struct rxvm_context* ctx, int argc, char** argv) {
    rxvm_link(ctx);
    rxvm_prepare(ctx);
    return rxvm_call(ctx, "main", argc, argv);
}
