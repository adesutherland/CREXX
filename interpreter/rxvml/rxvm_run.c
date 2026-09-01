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

struct rxvm_context *rxvm_context_create_in_runtime(rxvm_runtime *runtime) {
    struct rxvm_context *ctx;

    if (!runtime) return NULL;
    /* Match rxvm_create(): worker contexts see the mandatory static plugin. */
    CALL_PLUGIN_INITIALIZER(decnumber);
    ctx = calloc(1, sizeof(*ctx));
    if (!ctx) return NULL;
    if (!rxinimod_runtime(ctx, runtime)) {
        free(ctx);
        return NULL;
    }
    return ctx;
}

int rxvm_set_provider_path(struct rxvm_context* ctx, const char* path) {
    char *copy = 0;
    if (!ctx) return -1;
    if (path && *path) {
        copy = strdup(path);
        if (!copy) return -1;
    }
    free(ctx->provider_location);
    ctx->provider_location = copy;
    ctx->link_dirty = 1;
    return 0;
}

int rxvm_set_autoload(struct rxvm_context* ctx, int enabled) {
    if (!ctx) return -1;
    ctx->autoload_enabled = enabled ? 1u : 0u;
    ctx->link_dirty = 1;
    return 0;
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
    int autoloaded;
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
    do {
        if (rxvm_resolve_provider_dependencies(ctx) != 0) {
            rxvm_memory_leave(previous);
            return -1;
        }
        for (i = 0; i < ctx->num_modules; i++) {
            if (ctx->modules[i]->state < RXVM_MOD_LINKED) {
                rxvm_link_module(ctx, i);
                ctx->modules[i]->state = RXVM_MOD_LINKED;
                linked_any = 1;
            }
        }
        autoloaded = ctx->autoload_enabled
                ? rxvm_resolve_autoload_dependencies(ctx) : 0;
        if (autoloaded < 0) {
            rxvm_memory_leave(previous);
            return -1;
        }
    } while (autoloaded > 0);

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

static const char *rxvm_initializer_symbol(module *mod, size_t offset) {
    string_constant *symbol;

    if (!mod || offset >= mod->segment.const_size ||
        mod->segment.const_size - offset < sizeof(string_constant)) return 0;
    symbol = (string_constant *)(mod->segment.const_pool + offset);
    if (symbol->base.type != STRING_CONST ||
        symbol->base.size_in_pool < offsetof(string_constant, string) + 1u ||
        symbol->base.size_in_pool > mod->segment.const_size - offset ||
        symbol->string_len >= symbol->base.size_in_pool -
                              offsetof(string_constant, string) ||
        symbol->string[symbol->string_len] != 0) return 0;
    return symbol->string;
}

static int rxvm_execute_initializer(struct rxvm_context *ctx,
                                    module *mod,
                                    proc_runtime *procedure,
                                    const char *symbol) {
    proc_runtime *saved_proc;
    int saved_argc;
    value **saved_args;
    value *saved_ret;
    value *ret;
    int rc;

    saved_proc = ctx->ext_proc;
    saved_argc = ctx->ext_argc;
    saved_args = ctx->ext_args;
    saved_ret = ctx->ext_ret;
    ret = value_f_in(ctx->worker.memory_worker);
    if (!ret) {
        fprintf(stderr,
                "ERROR: unable to allocate return storage for initializer %s in module %s\n",
                symbol ? symbol : "<unknown>", mod->name);
        return -1;
    }

    ctx->ext_proc = procedure;
    ctx->ext_argc = 0;
    ctx->ext_args = 0;
    ctx->ext_ret = ret;
    rc = run(ctx, 0, 0);
    ctx->ext_proc = saved_proc;
    ctx->ext_argc = saved_argc;
    ctx->ext_args = saved_args;
    ctx->ext_ret = saved_ret;
    value_free(ret);

    if (rc != 0) {
        fprintf(stderr,
                "ERROR: initializer %s failed in module %s (rc=%d)\n",
                symbol ? symbol : "<unknown>", mod->name, rc);
        return -1;
    }
    return 0;
}

static int rxvm_initialize_module(struct rxvm_context *ctx, module *mod) {
    int offset;
    size_t visited;
    module *previous_initializer;

    if (!ctx || !mod) return -1;
    if (mod->initializer_state == RXVM_INIT_READY) return 0;
    if (mod->initializer_state == RXVM_INIT_FAILED) {
        fprintf(stderr, "ERROR: module %s initialization previously failed\n",
                mod->name);
        return -1;
    }
    if (mod->initializer_state == RXVM_INIT_INITIALIZING) {
        fprintf(stderr, "ERROR: module initializer cycle reaches %s\n",
                mod->name);
        return -1;
    }
    if (mod->state < RXVM_MOD_THREADED) {
        fprintf(stderr, "ERROR: module %s is not prepared for initialization\n",
                mod->name);
        return -1;
    }

    mod->initializer_state = RXVM_INIT_INITIALIZING;
    previous_initializer = ctx->current_initializer_module;
    ctx->current_initializer_module = mod;
    ctx->initializer_depth++;
    offset = mod->meta_head;
    visited = 0u;
    while (offset != -1 && visited++ <= mod->segment.const_size / 8u + 1u) {
        meta_entry *entry;

        if (offset < 0 || (size_t)offset >= mod->segment.const_size ||
            mod->segment.const_size - (size_t)offset < sizeof(meta_entry)) {
            fprintf(stderr, "ERROR: invalid metadata chain in module %s\n",
                    mod->name);
            goto fail;
        }
        entry = (meta_entry *)(mod->segment.const_pool + (size_t)offset);
        if (entry->base.type == META_INITIALIZER) {
            meta_initializer_constant *initializer;
            proc_runtime *procedure;
            const char *symbol;

            if (entry->base.size_in_pool < sizeof(meta_initializer_constant)) {
                fprintf(stderr,
                        "ERROR: invalid initializer metadata in module %s\n",
                        mod->name);
                goto fail;
            }
            initializer = (meta_initializer_constant *)entry;
            symbol = rxvm_initializer_symbol(mod, initializer->symbol);
            procedure = rxvm_get_module_runtime_procedure(
                    mod, initializer->function);
            if (!symbol || !procedure || procedure->binarySpace != &mod->segment ||
                procedure->start == SIZE_MAX) {
                fprintf(stderr,
                        "ERROR: initializer metadata does not name a local bytecode procedure in module %s\n",
                        mod->name);
                goto fail;
            }
            if (rxvm_execute_initializer(ctx, mod, procedure, symbol) != 0) {
                goto fail;
            }
        }
        offset = entry->next;
    }
    if (offset != -1) {
        fprintf(stderr, "ERROR: cyclic metadata chain in module %s\n", mod->name);
        goto fail;
    }

    ctx->initializer_depth--;
    ctx->current_initializer_module = previous_initializer;
    mod->initializer_state = RXVM_INIT_READY;
    return 0;

fail:
    ctx->initializer_depth--;
    ctx->current_initializer_module = previous_initializer;
    mod->initializer_state = RXVM_INIT_FAILED;
    return -1;
}

int rxvm_initialize(struct rxvm_context* ctx) {
    rxvm_memory_worker *previous;
    size_t index;
    int rc;

    if (!ctx) return -1;
    if (ctx->initializer_depth != 0u) return 0;
    if (rxvm_link(ctx) != 0 || rxvm_prepare(ctx) != 0) return -1;
    previous = rxvm_memory_enter(ctx->worker.memory_worker);
    rc = 0;
    for (index = 0u; index < ctx->num_modules; index++) {
        if (rxvm_initialize_module(ctx, ctx->modules[index]) != 0) {
            rc = -1;
            break;
        }
    }
    if (rc == 0) ctx->initialized_module_count = ctx->num_modules;
    rxvm_memory_leave(previous);
    return rc;
}

int rxvm_ensure_callee_initialized(struct rxvm_context *ctx,
                                   module *caller,
                                   proc_runtime *callee) {
    module *target;

    if (!ctx || !callee || !callee->binarySpace) return 0;
    target = callee->binarySpace->module;
    if (!target) return 0;
    if (ctx->initializer_depth == 0u) {
        return target->initializer_state == RXVM_INIT_READY &&
                       target->module_number <= ctx->initialized_module_count
                ? 0 : -1;
    }
    if (target == caller || target->initializer_state == RXVM_INIT_READY) {
        return 0;
    }
    return rxvm_initialize_module(ctx, target);
}

int rxvm_call(struct rxvm_context* ctx, char* proc_name, int argc, char** argv) {
    int initialization_rc;
    int rc;
    if (!ctx) return -1;
    initialization_rc = rxvm_initialize(ctx);
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
            if (initialization_rc != 0 &&
                rxvm_ensure_callee_initialized(ctx, 0, p) != 0) {
                value_free(ret_val);
                ctx->ext_ret = NULL;
                rxvm_memory_leave(previous);
                return -1;
            }
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
    } else if (initialization_rc != 0) {
        value_free(ret_val);
        ctx->ext_ret = NULL;
        rxvm_memory_leave(previous);
        return -1;
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
    if (rxvm_link(ctx) != 0) return -1;
    if (rxvm_prepare(ctx) != 0) return -1;
    if (rxvm_initialize(ctx) != 0) return -1;
    return rxvm_call(ctx, "main", argc, argv);
}
