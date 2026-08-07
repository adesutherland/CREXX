/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
    int linked_any;
    rxvm_memory_worker *previous;

    if (!ctx) return 0;
    previous = rxvm_memory_enter(ctx->worker.memory_worker);
    if (!ctx->link_dirty &&
        !ctx->interface_method_registry_dirty &&
        !ctx->interface_factory_registry_dirty) {
        rxvm_memory_leave(previous);
        return 0;
    }

    linked_any = 0;
    for (i = 0; i < ctx->num_modules; i++) {
        if (ctx->modules[i]->state < RXVM_MOD_LINKED) {
            rxvm_link_module(ctx, i);
            ctx->modules[i]->state = RXVM_MOD_LINKED;
            linked_any = 1;
        }
    }

    ctx->link_dirty = 0;

    if (linked_any) {
        ctx->interface_method_registry_dirty = 1;
        ctx->interface_factory_registry_dirty = 1;
    }

    if (ctx->interface_method_registry_dirty) {
        rxvm_rebuild_interface_method_registry(ctx);
        ctx->interface_method_registry_dirty = 0;
    }

    if (ctx->interface_factory_registry_dirty) {
        rxvm_rebuild_interface_factory_registry(ctx);
        ctx->interface_factory_registry_dirty = 0;
    }
    if (linked_any) {
        /* Factory bindings aggregate providers from every loaded image, so
           build them after the process-wide compatibility registries. */
        rxvm_rebuild_graph_bindings(ctx);
    }
    rxvm_memory_leave(previous);
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
    rxvm_memory_worker *previous =
            rxvm_memory_enter(ctx->worker.memory_worker);
    value* ret_val = value_f_in(ctx->worker.memory_worker);
    if (!ret_val) {
        rxvm_memory_leave(previous);
        return -1;
    }
    ctx->ext_ret = ret_val;

    if (proc_name && strcmp(proc_name, "main") != 0) {
        proc_runtime* p = NULL;
        if (src_node(ctx->exposed_proc_tree, proc_name, (size_t*)&p)) {
            int i;
            ctx->ext_proc = p;
            ctx->ext_argc = argc;
            ctx->ext_args = argc > 0
                ? rxvm_memory_alloc_bytes(ctx->worker.memory_worker,
                                          sizeof(value*) * (size_t)argc)
                : NULL;
            if (argc > 0 && !ctx->ext_args) {
                ctx->ext_argc = 0;
                ctx->ext_proc = NULL;
                value_free(ret_val);
                ctx->ext_ret = NULL;
                rxvm_memory_leave(previous);
                return -1;
            }
            for (i = 0; i < argc; i++) {
                ctx->ext_args[i] = value_f_in(ctx->worker.memory_worker);
                if (set_null_string_validated(ctx->ext_args[i], argv[i] ? argv[i] : "") != 0) {
                    int j;
                    fprintf(stderr, "ERROR: Invalid UTF-8 argument\n");
                    value_free(ctx->ext_args[i]);
                    for (j = 0; j < i; j++) {
                        value_free(ctx->ext_args[j]);
                    }
                    (void)rxvm_memory_release(ctx->ext_args);
                    ctx->ext_args = NULL;
                    ctx->ext_argc = 0;
                    ctx->ext_proc = NULL;
                    value_free(ret_val);
                    ctx->ext_ret = NULL;
                    rxvm_memory_leave(previous);
                    return -1;
                }
            }
        } else {
            fprintf(stderr, "ERROR: Procedure %s not found\n", proc_name);
            value_free(ret_val);
            ctx->ext_ret = NULL;
            rxvm_memory_leave(previous);
            return -1;
        }
    }

    rc = run(ctx, argc, argv);

    if (ctx->ext_proc) {
        int i;
        for (i = 0; i < ctx->ext_argc; i++) {
            value_free(ctx->ext_args[i]);
        }
        (void)rxvm_memory_release(ctx->ext_args);
        ctx->ext_proc = NULL;
        ctx->ext_argc = 0;
        ctx->ext_args = NULL;
    }

    /*
     * Normal RETURN from main leaves the integer result in ext_ret. EXIT and
     * unhandled runtime signals finish the interpreter with a non-zero rc
     * without updating ext_ret, so preserve that failure status.
     */
    if (rc == 0 || ret_val->int_value != 0) rc = (int)ret_val->int_value;

    value_free(ret_val);
    ctx->ext_ret = NULL;

    rxvm_memory_leave(previous);
    return rc;
}

int rxvm_run(struct rxvm_context* ctx, int argc, char** argv) {
    rxvm_link(ctx);
    rxvm_prepare(ctx);
    return rxvm_call(ctx, "main", argc, argv);
}
