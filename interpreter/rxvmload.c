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

/* CREXX Module Loader */

#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#define RXPA_CATALOGUE_LOCK() AcquireSRWLockExclusive(&rxpa_catalogue_lock)
#define RXPA_CATALOGUE_UNLOCK() ReleaseSRWLockExclusive(&rxpa_catalogue_lock)
#else
#include <pthread.h>
#define RXPA_CATALOGUE_LOCK() ((void)pthread_mutex_lock(&rxpa_catalogue_lock))
#define RXPA_CATALOGUE_UNLOCK() ((void)pthread_mutex_unlock(&rxpa_catalogue_lock))
#endif
#include "rxvmintp.h"
#include "rxvmchannel.h"
#include "rxastree.h"
#include "rxvmvars.h"
#include "rxvmsock.h"
#include "rxpa.h"
#include "rxcrexxcmd.h"

// RXPA (Plugin Architecture) Support declarations  for this file

/* Context for RXPA Functions / callbacks */
void rxvm_addfunc(rxpa_libfunc func, char* name, char* option, char* type, char* args);
void rxvm_addclass(char* name, char* option, char* type);
void rxvm_addinterface(char* name, char* option, char* type);
void rxvm_addimplements(char* name, char* interface_name);
void rxvm_addmember(char* owner, char* kind, char* member, char* type, char* args);
char* rxvm_getstring(rxpa_attribute_value attributeValue);
void rxvm_setstring(rxpa_attribute_value attributeValue, const char* string);
void rxvm_setint(rxpa_attribute_value attributeValue, rxinteger int_value);
rxinteger rxvm_getint(rxpa_attribute_value attributeValue);
void rxvm_setfloat(rxpa_attribute_value attributeValue, double double_value);
double rxvm_getfloat(rxpa_attribute_value attributeValue);
int rxvm_setnativepayload(rxpa_attribute_value attributeValue, const void *payload, size_t length,
                          const rxpa_native_payload_ops *ops, unsigned int flags);
void* rxvm_getnativepayload(rxpa_attribute_value attributeValue, size_t *out_length,
                            const rxpa_native_payload_ops **out_ops, unsigned int *out_flags);
int rxvm_isinitialized(rxpa_attribute_value attributeValue);
rxinteger rxvm_getnumattrs(rxpa_attribute_value attributeValue);
void rxvm_setnumattrs(rxpa_attribute_value attributeValue, rxinteger numAttrs);
rxpa_attribute_value rxvm_getattr(rxpa_attribute_value attributeValue, rxinteger index);
rxpa_attribute_value rxvm_insertattr(rxpa_attribute_value attributeValue, rxinteger index);
void rxvm_removeattr(rxpa_attribute_value attributeValue, rxinteger index);
void rxvm_swapattrs(rxpa_attribute_value attributeValue, rxinteger index1, rxinteger index2);
void rxvm_setsayexit(say_exit_func sayExitFunc);
void rxvm_resetsayexit();

typedef struct rxpa_proc_policy {
    size_t procedure_offset;
    uint32_t capabilities;
    rxpa_session_instance *session_instance;
} rxpa_proc_policy;

typedef struct rxpa_context {
    rxvm_context *rxvm_context;
    module_file *plugin_being_loaded;
    size_t const_buffer_size;
    size_t const_buffer_top;
    int meta_tail;
    uint32_t plugin_capabilities;
    const rxpa_loaded_plugin *dynamic_plugin;
    rxpa_session_instance *session_instances;
    rxpa_proc_policy *proc_policies;
    size_t proc_policy_count;
    size_t proc_policy_capacity;
} rxpa_context;

struct rxpa_library_reference {
    rxpa_loaded_plugin plugin;
    struct rxpa_library_reference *next;
};

/* Statically Linked Plugin List */
struct static_linked_function {
    char *name;
    char *plugin_id;
    char *option;
    char *type;
    char *args;
    rxpa_libfunc func;
    struct static_linked_function *next;
};
static struct static_linked_function *static_linked_functions = 0;

struct static_linked_metadata {
    char *kind;
    char *symbol;
    char *option;
    char *type;
    char *interface_symbol;
    char *owner;
    char *member_kind;
    char *member;
    char *args;
    struct static_linked_metadata *next;
};
static struct static_linked_metadata *static_linked_metadata = 0;

struct static_plugin_capability {
    char *plugin_id;
    uint32_t capabilities;
    struct static_plugin_capability *next;
};
static struct static_plugin_capability *static_plugin_capabilities = 0;

struct static_plugin_manifest_v2 {
    char *plugin_id;
    rxpa_plugin_manifest_v2 manifest;
    unsigned char valid;
    struct static_plugin_manifest_v2 *next;
};
static struct static_plugin_manifest_v2 *static_plugin_manifests_v2 = 0;

typedef struct static_function_snapshot {
    const char *name;
    const char *plugin_id;
    const char *option;
    const char *type;
    const char *args;
    rxpa_libfunc func;
    uint32_t v1_capabilities;
    unsigned char has_manifest_v2;
    unsigned char valid_manifest_v2;
    rxpa_plugin_manifest_v2 manifest_v2;
} static_function_snapshot;

typedef struct static_metadata_snapshot {
    const char *kind;
    const char *symbol;
    const char *option;
    const char *type;
    const char *interface_symbol;
    const char *owner;
    const char *member_kind;
    const char *member;
    const char *args;
} static_metadata_snapshot;

#ifdef _WIN32
static SRWLOCK rxpa_catalogue_lock = SRWLOCK_INIT;
#else
static pthread_mutex_t rxpa_catalogue_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

#if defined(_MSC_VER)
#define RXPA_LOADER_THREAD_LOCAL __declspec(thread)
#else
#define RXPA_LOADER_THREAD_LOCAL __thread
#endif

/* The compatibility slot remains NULL outside a context-owned load. */
static RXPA_LOADER_THREAD_LOCAL rxpa_context *rxpa_loader_compat_context;

static rxpa_context **rxpa_current_context_slot(void) {
    rxvm_context *context = rxvm_active_context_current();
    if (context) return (rxpa_context **)&context->active.rxpa_context;
    return &rxpa_loader_compat_context;
}

#define current_rxpa_context (*rxpa_current_context_slot())

// Create a new RXPA context for a module
static rxpa_context *rxpa_context_f(rxvm_context *rxvm_context);

// Free statically linked functions list
static void free_rxpa_context(rxpa_context *context);

static void free_interface_factory_registry(rxvm_context *context);
static void free_interface_method_registry(rxvm_context *context);
static void build_module_runtime_procedures(module *mod);
static void build_module_dynamic_site_caches(module *mod);
static void apply_rxpa_proc_policies(rxpa_context *context,
                                     size_t module_number);
static void bind_rxpa_runtime_policy(rxvm_context *context,
                                     proc_runtime *runtime,
                                     uint32_t capabilities,
                                     rxpa_session_instance *session_instance);
static int rxpa_context_create_session(
        rxpa_context *context, const rxpa_plugin_manifest_v2 *manifest);
static rxpa_session_instance *rxpa_context_find_session(
        const rxpa_context *context, const char *plugin_id);
static void rxpa_context_publish_sessions(rxpa_context *context);
static void rxpa_context_destroy_sessions(rxpa_context *context);

static void *rxvm_load_memory_alloc(rxvm_memory_worker *worker, size_t size) {
    return rxvm_memory_alloc_bytes(worker, size);
}

static char *rxvm_load_memory_strdup(rxvm_memory_worker *worker,
                                     const char *text) {
    size_t length;
    char *copy;
    if (!text) text = "";
    length = strlen(text);
    copy = rxvm_load_memory_alloc(worker, length + 1u);
    if (copy) memcpy(copy, text, length + 1u);
    return copy;
}

static void rxvm_load_memory_free(void *pointer) {
    rxvm_memory_worker *previous;
    if (!pointer) return;
    previous = rxvm_memory_enter(rxvm_memory_owner(pointer));
    (void)rxvm_memory_release(pointer);
    rxvm_memory_leave(previous);
}

static char *rxpa_catalogue_strdup(const char *text) {
    size_t length;
    char *copy;
    if (!text) return 0;
    length = strlen(text);
    copy = rxvm_memory_alloc_unowned_bytes(length + 1u);
    if (!copy) RX_PANIC_OOM("copy static rxpa catalogue string",
                            length + 1u, text);
    memcpy(copy, text, length + 1u);
    return copy;
}

static int rxpa_strings_equal(const char *left, const char *right) {
    if (!left || !right) return left == right;
    return strcmp(left, right) == 0;
}

static void rxvm_memory_report_if_requested(rxvm_memory_context *context) {
    static const char *class_names[RXVM_MEMORY_CLASS_COUNT] = {
        "byte16", "byte32", "byte64", "byte128", "byte256", "byte512",
        "byte1024", "byte2048", "byte4096", "byte8192", "byte16384",
        "value1", "value2", "value4", "value8", "value16", "value32",
        "value64", "reference"
    };
    rxvm_memory_stats stats;
    const char *enabled = getenv("CREXX_RXVM_MEMORY_STATS");
    size_t retained_slab_bytes;
    uint64_t internal_fragmentation_bytes;
    size_t class_id;

    if (!enabled || !*enabled || strcmp(enabled, "0") == 0) return;
    rxvm_memory_get_stats(context, &stats);
    retained_slab_bytes = (size_t)(stats.standard_slabs_owned +
                                   stats.standard_slabs_reserved) *
                          RXVM_MEMORY_SLAB_SIZE;
    internal_fragmentation_bytes =
            stats.cumulative_capacity_bytes >= stats.cumulative_requested_bytes
            ? stats.cumulative_capacity_bytes -
              stats.cumulative_requested_bytes
            : 0u;

    fprintf(stderr,
            "RXVM_MEMORY_STATS version=1 allocations=%" PRIu64
            " frees=%" PRIu64 " reallocations=%" PRIu64
            " failures=%" PRIu64 " invalid_frees=%" PRIu64
            " wrong_owner_frees=%" PRIu64
            " requested_bytes=%" PRIu64 " capacity_bytes=%" PRIu64
            " internal_fragmentation_bytes=%" PRIu64
            " live_allocations=%" PRIu64
            " peak_live_allocations=%" PRIu64
            " live_capacity_bytes=%" PRIu64
            " peak_live_capacity_bytes=%" PRIu64
            " oversized_live_allocations=%" PRIu64
            " oversized_live_bytes=%" PRIu64
            " slabs_owned=%" PRIu64 " slabs_reserved=%" PRIu64
            " retained_slab_bytes=%zu slabs_from_system=%" PRIu64
            " slabs_to_system=%" PRIu64 " depot_refills=%" PRIu64
            " depot_returns=%" PRIu64 " depot_hits=%" PRIu64
            " trim_calls=%" PRIu64 "\n",
            stats.allocation_calls, stats.free_calls,
            stats.reallocation_calls, stats.allocation_failures,
            stats.invalid_frees, stats.wrong_owner_frees,
            stats.cumulative_requested_bytes,
            stats.cumulative_capacity_bytes, internal_fragmentation_bytes,
            stats.live_allocations, stats.peak_live_allocations,
            stats.live_capacity_bytes, stats.peak_live_capacity_bytes,
            stats.oversized_live_allocations, stats.oversized_live_bytes,
            stats.standard_slabs_owned, stats.standard_slabs_reserved,
            retained_slab_bytes, stats.standard_slabs_from_system,
            stats.standard_slabs_to_system, stats.depot_refills,
            stats.depot_returns, stats.depot_hits, stats.trim_calls);

    fputs("RXVM_MEMORY_CLASS_STATS version=1 allocation_calls=", stderr);
    for (class_id = 0; class_id < RXVM_MEMORY_CLASS_COUNT; class_id++) {
        fprintf(stderr, "%s%s:%" PRIu64,
                class_id ? "," : "", class_names[class_id],
                stats.class_allocation_calls[class_id]);
    }
    fputs(" peak_live=", stderr);
    for (class_id = 0; class_id < RXVM_MEMORY_CLASS_COUNT; class_id++) {
        fprintf(stderr, "%s%s:%" PRIu64,
                class_id ? "," : "", class_names[class_id],
                stats.class_peak_live_allocations[class_id]);
    }
    fputc('\n', stderr);
#ifdef CREXX_VM_MEMORY_CENSUS
    {
        int first_histogram_entry = 1;
        fprintf(stderr,
                "RXVM_MEMORY_OVERSIZED_STATS version=1 allocations=%" PRIu64
                " requested_bytes=%" PRIu64 " max_request_bytes=%" PRIu64
                " peak_live_allocations=%" PRIu64
                " peak_live_bytes=%" PRIu64
                " resize_standard_to_oversized=%" PRIu64
                " resize_oversized_to_standard=%" PRIu64
                " resize_oversized_to_oversized=%" PRIu64
                " request_histogram=",
                stats.oversized_allocation_calls,
                stats.cumulative_oversized_requested_bytes,
                stats.maximum_oversized_request_bytes,
                stats.peak_oversized_live_allocations,
                stats.peak_oversized_live_bytes,
                stats.resize_standard_to_oversized,
                stats.resize_oversized_to_standard,
                stats.resize_oversized_to_oversized);
        for (class_id = 0;
             class_id < RXVM_MEMORY_SIZE_HISTOGRAM_BUCKETS; class_id++) {
            uint64_t count = stats.oversized_request_histogram[class_id];
            uint64_t upper;
            if (!count) continue;
            if (!class_id) upper = 0;
            else if (class_id >= 65u) upper = UINT64_MAX;
            else upper = UINT64_C(1) << (class_id - 1u);
            fprintf(stderr, "%s%" PRIu64 ":%" PRIu64,
                    first_histogram_entry ? "" : ",", upper, count);
            first_histogram_entry = 0;
        }
        fputc('\n', stderr);
    }
#endif
}

// End of RXPA Declarations for this file

static int rxinimod_common(rxvm_context *context,
                           rxvm_runtime *runtime,
                           unsigned char owns_runtime) {
    if (!context || !runtime) return 0;
    context->worker.runtime = 0;
    context->worker.memory_worker = 0;
    context->worker.owner_thread_token = 0;
    context->worker.execution_depth = 0u;
    context->worker.state = RXVM_WORKER_UNINITIALIZED;
    context->owns_runtime = owns_runtime;
    context->program_generation = 0;
    if (!rxvm_worker_initialize(&context->worker, runtime)) {
        context->owns_runtime = 0u;
        return 0;
    }
    context->num_modules = 0;
    context->exposed_proc_tree = 0;
    context->exposed_reg_tree = 0;
    context->debug_mode = 0;
#ifdef CREXX_VM_PROFILING
    context->profile_mode = 0;
    context->profile_output = 0;
    context->sequence_count = 0;
    context->sequence_output = 0;
#endif
    context->location = 0;
    context->provider_location = 0;
    context->autoload_enabled = 1u;
    context->autoloaded_artifacts = 0;
    context->autoloaded_artifact_count = 0u;
    context->autoloaded_artifact_capacity = 0u;
    context->ext_proc = 0;
    context->ext_argc = 0;
    context->ext_args = 0;
    context->ext_ret = 0;
    context->interface_factories = 0;
    context->num_interface_factories = 0;
    context->interface_factory_capacity = 0;
    context->interface_methods = 0;
    context->num_interface_methods = 0;
    context->interface_method_capacity = 0;
    context->graph_bindings = 0;
    context->graph_binding_count = 0;
    context->graph_binding_capacity = 0;
    context->semantic_generation = 1u;
    context->socket_registry = 0;
    rxvm_reference_context_init(&context->references);
    context->link_dirty = 0;
    context->interface_method_registry_dirty = 0;
    context->interface_factory_registry_dirty = 0;
    context->initializer_depth = 0u;
    context->current_initializer_module = 0;
    context->initialized_module_count = 0u;
    context->active.rxvml_context = 0;
    context->active.rxpa_context = 0;
    context->active.rxpa_pool_head = 0;
    context->active.crexx_command_state = 0;
    context->active.say_exit = 0;
    context->active.pending_interrupts = 0;
    context->active.compatibility_interrupts = 0;
    context->active.external_mailbox_owner = 0;
    context->active.external_mailbox_claim = 0;
    rxvmplugin_instance_set_init(&context->plugin_instances);
    context->rxpa_libraries = 0;
    context->rxpa_sessions = 0;
    context->rxpa_session_bindings = 0;
    rxpa_compatibility_context_init(&context->rxpa_compatibility,
                                    context->worker.memory_worker);
    context->channel_context = 0;

    /* Support 128 modules initially - this grows automatically */
    context->module_buffer_size = 128;
    context->modules = rxvm_memory_alloc_bytes(
            context->worker.memory_worker,
            sizeof(module*) * context->module_buffer_size);
    if (!context->modules) {
        RX_PANIC_OOM("malloc rxvm module table",
                     sizeof(module*) * context->module_buffer_size, 0);
    }
    return 1;
}

/* Initialise a compatibility context with its own one-worker runtime. */
void rxinimod(rxvm_context *context) {
    rxvm_runtime *runtime = rxvm_runtime_create();
    if (!runtime) {
        RX_PANIC_OOM("create rxvm runtime", 1, 0);
    }
    if (!rxinimod_common(context, runtime, 1u)) {
        (void)rxvm_runtime_destroy(runtime);
        RX_PANIC_OOM("create rxvm memory worker", 1, 0);
    }
}

/* Initialise an internal worker VM inside an existing runtime domain. */
int rxinimod_runtime(rxvm_context *context, rxvm_runtime *runtime) {
    return rxinimod_common(context, runtime, 0u);
}

/* Free Module Context */
void rxfremod(rxvm_context *context) {
    int j, k;
    size_t memory_leaks;
    rxvm_memory_worker *previous_memory_worker;
    rxvm_runtime *runtime;
    rxvm_worker_transition_result transition;

    if (!context) return;
    transition = rxvm_worker_begin_draining(&context->worker);
    if (transition != RXVM_WORKER_TRANSITION_OK) {
        fprintf(stderr,
                "RXVM worker teardown rejected: %s (%s)\n",
                transition == RXVM_WORKER_TRANSITION_WRONG_THREAD
                    ? "wrong owner thread" : "invalid lifecycle state",
                rxvm_worker_state_name(
                        rxvm_worker_get_state(&context->worker)));
        abort();
    }
    runtime = context->worker.runtime;
    previous_memory_worker =
            rxvm_memory_enter(context->worker.memory_worker);

    if (context->active.rxvml_context || context->active.rxpa_context ||
        context->active.rxpa_pool_head ||
        context->active.pending_interrupts) {
        fprintf(stderr, "RXVM teardown detected live active execution/callback state\n");
        abort();
    }

    rxvm_channel_context_destroy(context);

    /* Remove cold coordinator references before procedure storage is freed. */
    rxpa_compatibility_context_destroy(&context->rxpa_compatibility);

    free_interface_factory_registry(context);
    free_interface_method_registry(context);
    rxvm_free_graph_bindings(context);
    rxvm_socket_free_registry(context);
    rxcrexxcmd_context_state_free(context);
    context->active.say_exit = 0;

    /* Free Symbol Search Trees */
    DEBUG("Free Symbol Search Trees\n");
    free_tree(&context->exposed_proc_tree);
    context->exposed_proc_tree = 0;
    free_tree(&context->exposed_reg_tree);
    context->exposed_reg_tree = 0;

    /* Free Program Modules */
    for (j=0; j<context->num_modules; j++) {
        size_t i;
        /* Drain procedure stack frame free lists */
        for (i = 0; i < context->modules[j]->procedure_count; i++) {
            proc_runtime *p_entry = &context->modules[j]->procedures[i];
            /* Only drain if this module owns the procedure (it's not imported) */
            if (p_entry->frame_free_list == &(p_entry->frame_free_list_head)) {
                stack_frame *f = *p_entry->frame_free_list;
                while (f) {
                    stack_frame *next = f->prev_free;
                    completely_free_frame(f);
                    f = next;
                }
                *p_entry->frame_free_list = 0;
            }
        }

        if (!rxvm_program_generation_owns_module(context, (size_t)j)) {
            free_module(context->modules[j]->file);
        }
        if (context->modules[j]->globals) {
            value **temp_globals = context->modules[j]->globals;
            char *temp_dont_free = context->modules[j]->globals_dont_free;
            int temp_count = context->modules[j]->segment.globals;
            context->modules[j]->globals = 0;
            context->modules[j]->globals_dont_free = 0;
            for (k = 0; k < temp_count; k++) {
                if (temp_globals[k] && (!temp_dont_free || !temp_dont_free[k])) {
                    value_free(temp_globals[k]);
                }
            }
            (void)rxvm_memory_release(temp_globals);
            if (temp_dont_free) (void)rxvm_memory_release(temp_dont_free);
        }
        if (context->modules[j]->execution_image)
            (void)rxvm_memory_release(context->modules[j]->execution_image);
        (void)rxvm_memory_release(context->modules[j]->dynamic_site_cache_slots);
        (void)rxvm_memory_release(context->modules[j]->dynamic_site_caches);
        if (context->modules[j]->proc_runtime_lookup)
            (void)rxvm_memory_release(context->modules[j]->proc_runtime_lookup);
        if (context->modules[j]->procedures)
            (void)rxvm_memory_release(context->modules[j]->procedures);
        (void)rxvm_memory_release(context->modules[j]);
    }
    (void)rxvm_memory_release(context->modules);
    rxvm_reference_context_free(&context->references);
    if (context->location) free(context->location);
    if (context->provider_location) free(context->provider_location);
    for (j = 0; (size_t)j < context->autoloaded_artifact_count; j++) {
        free(context->autoloaded_artifacts[j]);
    }
    free(context->autoloaded_artifacts);

    /* Provider instances are context-owned and must die while the worker is
     * idle and its allocator family is still available. */
    rxvmplugin_instance_set_destroy(&context->plugin_instances);

    /* Session call bindings no longer have procedure owners.  Destroy the
     * per-VM plugin sessions next, while their dynamic libraries are still
     * resident, then release the libraries themselves. */
    while (context->rxpa_session_bindings) {
        rxpa_session_call_binding *next =
                context->rxpa_session_bindings->next;
        rxvm_load_memory_free(context->rxpa_session_bindings);
        context->rxpa_session_bindings = next;
    }
    while (context->rxpa_sessions) {
        rxpa_session_instance *next = context->rxpa_sessions->next;
        context->rxpa_sessions->destroy(context->rxpa_sessions->session);
        rxvm_load_memory_free((void *)context->rxpa_sessions->plugin_id);
        rxvm_load_memory_free(context->rxpa_sessions);
        context->rxpa_sessions = next;
    }

    /* All values, references, modules and native payload callbacks are now
     * unreachable; dynamic RXPA code may finally be unloaded. */
    while (context->rxpa_libraries) {
        struct rxpa_library_reference *next = context->rxpa_libraries->next;
        rxpa_close_plugin(&context->rxpa_libraries->plugin);
        rxvm_load_memory_free(context->rxpa_libraries);
        context->rxpa_libraries = next;
    }

    /* Release immutable program storage only after all worker-owned values,
     * references, providers, sessions and native code are unreachable. */
    rxvm_program_generation_release_context(context);

    rxvm_memory_report_if_requested(rxvm_runtime_memory_context(runtime));

    rxvm_memory_leave(previous_memory_worker == context->worker.memory_worker ?
                      0 : previous_memory_worker);
    memory_leaks = rxvm_worker_destroy(&context->worker);
    if (context->owns_runtime) {
        memory_leaks += rxvm_runtime_destroy(runtime);
    }
    context->worker.runtime = 0;
    context->owns_runtime = 0u;
#ifndef NDEBUG
    if (memory_leaks) {
        fprintf(stderr,
                "RXVM memory teardown detected %zu live allocation(s)\n",
                memory_leaks);
        abort();
    }
#else
    (void)memory_leaks;
#endif
}

static void free_interface_factory_registry(rxvm_context *context) {
    size_t i;

    if (!context || !context->interface_factories) {
        if (context) {
            context->num_interface_factories = 0;
            context->interface_factory_capacity = 0;
        }
        return;
    }

    for (i = 0; i < context->num_interface_factories; i++) {
        (void)rxvm_memory_release(context->interface_factories[i].interface_name);
        (void)rxvm_memory_release(context->interface_factories[i].factory_name);
        (void)rxvm_memory_release(context->interface_factories[i].descriptor);
        rx_sig_free(&context->interface_factories[i].signature);
        (void)rxvm_memory_release(context->interface_factories[i].class_name);
    }

    (void)rxvm_memory_release(context->interface_factories);
    context->interface_factories = 0;
    context->num_interface_factories = 0;
    context->interface_factory_capacity = 0;
}

static void free_interface_method_registry(rxvm_context *context) {
    size_t i;

    if (!context || !context->interface_methods) {
        if (context) {
            context->num_interface_methods = 0;
            context->interface_method_capacity = 0;
        }
        return;
    }

    for (i = 0; i < context->num_interface_methods; i++) {
        (void)rxvm_memory_release(context->interface_methods[i].class_name);
        (void)rxvm_memory_release(context->interface_methods[i].descriptor);
    }

    (void)rxvm_memory_release(context->interface_methods);
    context->interface_methods = 0;
    context->num_interface_methods = 0;
    context->interface_method_capacity = 0;
}

static int compare_proc_runtime_lookup_entries(const void *left, const void *right) {
    const proc_runtime_lookup_entry *left_entry = (const proc_runtime_lookup_entry *)left;
    const proc_runtime_lookup_entry *right_entry = (const proc_runtime_lookup_entry *)right;

    if (left_entry->offset < right_entry->offset) return -1;
    if (left_entry->offset > right_entry->offset) return 1;
    return 0;
}

static int module_runtime_procedure_seen(const proc_runtime_lookup_entry *lookup, size_t count, size_t offset) {
    size_t i;

    for (i = 0; i < count; i++) {
        if (lookup[i].offset == offset) return 1;
    }
    return 0;
}

static void add_module_runtime_procedure_offset(module *mod, proc_runtime_lookup_entry **lookup,
                                                size_t *count, size_t *capacity, size_t offset) {
    proc_runtime_lookup_entry *new_lookup;
    size_t new_capacity;

    if (offset >= mod->segment.const_size) {
        fprintf(stderr, "PANIC: Procedure offset outside constant pool in module %s\n", mod->name);
        exit(-1);
    }
    if (((chameleon_constant *)(mod->segment.const_pool + offset))->type != PROC_CONST) {
        fprintf(stderr, "PANIC: Invalid runtime procedure in module %s\n", mod->name);
        exit(-1);
    }
    if (module_runtime_procedure_seen(*lookup, *count, offset)) return;

    if (*count == *capacity) {
        new_capacity = *capacity ? *capacity * 2 : 8;
        new_lookup = rxvm_memory_resize_bytes(
                mod->memory_worker, *lookup,
                sizeof(proc_runtime_lookup_entry) * *count,
                sizeof(proc_runtime_lookup_entry) * new_capacity);
        if (!new_lookup) {
            RX_PANIC_OOM("realloc rxvm procedure runtime lookup",
                         sizeof(proc_runtime_lookup_entry) * new_capacity,
                         mod->name);
        }
        *lookup = new_lookup;
        *capacity = new_capacity;
    }

    (*lookup)[*count].offset = offset;
    (*lookup)[*count].runtime = 0;
    (*count)++;
}

static void init_module_runtime_procedure(module *mod, proc_runtime *runtime, size_t offset) {
    proc_constant *definition = (proc_constant *)(mod->segment.const_pool + offset);

    runtime->definition = definition;
    runtime->locals = definition->locals;
    runtime->native_capabilities = 0u;
    runtime->binarySpace = (!mod->native && definition->start != SIZE_MAX) ? &mod->segment : 0;
    runtime->frame_free_list_head = 0;
    runtime->frame_free_list = &runtime->frame_free_list_head;
    runtime->start = definition->start;
    runtime->name = definition->name;
#ifdef CREXX_VM_PROFILING
    runtime->profile_id = SIZE_MAX;
#endif
    runtime->native_invoker = NULL;
}

static void build_module_runtime_procedures(module *mod) {
    int i;
    size_t proc_index;
    size_t lookup_capacity;

    mod->procedures = 0;
    mod->procedure_count = 0;
    mod->proc_runtime_lookup = 0;
    mod->proc_runtime_lookup_size = 0;
    lookup_capacity = 0;

    i = mod->proc_head;
    while (i != -1) {
        proc_constant *definition = (proc_constant *)(mod->segment.const_pool + (size_t)i);
        if (definition->base.type != PROC_CONST) {
            fprintf(stderr, "PANIC: Invalid procedure chain in module %s\n", mod->name);
            exit(-1);
        }
        add_module_runtime_procedure_offset(mod, &mod->proc_runtime_lookup,
                                            &mod->proc_runtime_lookup_size,
                                            &lookup_capacity, (size_t)i);
        i = definition->next;
    }

    i = mod->expose_head;
    while (i != -1) {
        chameleon_constant *entry = (chameleon_constant *)(mod->segment.const_pool + (size_t)i);
        if (entry->type == EXPOSE_PROC_CONST) {
            expose_proc_constant *exposed = (expose_proc_constant *)entry;
            proc_constant *definition = (proc_constant *)(mod->segment.const_pool + exposed->procedure);
            if (definition->base.type != PROC_CONST) {
                fprintf(stderr, "PANIC: Invalid exposed procedure in module %s\n", mod->name);
                exit(-1);
            }
            add_module_runtime_procedure_offset(mod, &mod->proc_runtime_lookup,
                                                &mod->proc_runtime_lookup_size,
                                                &lookup_capacity, exposed->procedure);
            i = exposed->next;
        } else if (entry->type == EXPOSE_REG_CONST) {
            i = ((expose_reg_constant *)entry)->next;
        } else {
            fprintf(stderr, "PANIC: Invalid expose chain in module %s\n", mod->name);
            exit(-1);
        }
    }

    mod->procedure_count = mod->proc_runtime_lookup_size;
    if (mod->procedure_count) {
        mod->procedures = rxvm_memory_calloc_bytes(
                mod->memory_worker, mod->procedure_count, sizeof(proc_runtime));
        if (!mod->procedures) {
            RX_PANIC_OOM("calloc rxvm procedure runtime table",
                         mod->procedure_count * sizeof(proc_runtime),
                         mod->name);
        }
    }

    for (proc_index = 0; proc_index < mod->procedure_count; proc_index++) {
        proc_runtime *runtime = &mod->procedures[proc_index];
        init_module_runtime_procedure(mod, runtime, mod->proc_runtime_lookup[proc_index].offset);
        mod->proc_runtime_lookup[proc_index].runtime = runtime;
    }

    if (mod->proc_runtime_lookup_size > 1) {
        qsort(mod->proc_runtime_lookup, mod->proc_runtime_lookup_size,
              sizeof(proc_runtime_lookup_entry), compare_proc_runtime_lookup_entries);
    }
}

static void build_module_dynamic_site_caches(module *mod) {
    size_t instruction_index;
    size_t cache_count;
    size_t cache_slot;

    if (!mod || !mod->segment.binary || !mod->segment.inst_size) return;
    cache_count = 0u;
    instruction_index = 0u;
    while (instruction_index < mod->segment.inst_size) {
        unsigned int opcode;
        size_t operand_count;
        opcode = (unsigned int)mod->segment.binary[instruction_index].instruction.opcode;
        operand_count = (size_t)mod->segment.binary[instruction_index].instruction.no_ops;
        if (opcode == OP_SRCMETHODSEL_REG_REG_STRING ||
            opcode == OP_SRCFPROCSEL_REG_STRING_REG) cache_count++;
        if (operand_count >= mod->segment.inst_size - instruction_index) break;
        instruction_index += operand_count + 1u;
    }
    if (!cache_count) return;
    if (cache_count > UINT32_MAX ||
        mod->segment.inst_size > SIZE_MAX / sizeof(*mod->dynamic_site_cache_slots) ||
        cache_count > SIZE_MAX / sizeof(*mod->dynamic_site_caches)) {
        RX_PANIC_OOM("size rxvm dynamic-site caches", (size_t)-1, mod->name);
    }
    mod->dynamic_site_cache_slots = (uint32_t *)rxvm_memory_alloc_bytes(
        mod->memory_worker,
        mod->segment.inst_size * sizeof(*mod->dynamic_site_cache_slots));
    mod->dynamic_site_caches = (rxvm_dynamic_site_cache *)
        rxvm_memory_calloc_bytes(mod->memory_worker, cache_count,
                                 sizeof(*mod->dynamic_site_caches));
    if (!mod->dynamic_site_cache_slots || !mod->dynamic_site_caches) {
        RX_PANIC_OOM("allocate rxvm dynamic-site caches",
                     mod->segment.inst_size * sizeof(*mod->dynamic_site_cache_slots) +
                         cache_count * sizeof(*mod->dynamic_site_caches),
                     mod->name);
    }
    memset(mod->dynamic_site_cache_slots,
           0xff,
           mod->segment.inst_size * sizeof(*mod->dynamic_site_cache_slots));
    mod->dynamic_site_cache_count = cache_count;
    instruction_index = 0u;
    cache_slot = 0u;
    while (instruction_index < mod->segment.inst_size && cache_slot < cache_count) {
        unsigned int opcode;
        size_t operand_count;
        opcode = (unsigned int)mod->segment.binary[instruction_index].instruction.opcode;
        operand_count = (size_t)mod->segment.binary[instruction_index].instruction.no_ops;
        if (opcode == OP_SRCMETHODSEL_REG_REG_STRING ||
            opcode == OP_SRCFPROCSEL_REG_STRING_REG) {
            mod->dynamic_site_cache_slots[instruction_index] = (uint32_t)cache_slot++;
        }
        if (operand_count >= mod->segment.inst_size - instruction_index) break;
        instruction_index += operand_count + 1u;
    }
}

/* Link a loaded module */
void rxvm_link_module(rxvm_context *context, size_t module_number_to_link) {
    int i;
    size_t mod_index;
    chameleon_constant *c_entry;
    proc_runtime *p_entry, *p_entry_linked;
    value *g_reg;

    DEBUG("Add Module Symbols\n");
    i = context->modules[module_number_to_link]->expose_head;
    mod_index = module_number_to_link;
    while (i != -1) {
        c_entry = (chameleon_constant *)(context->modules[mod_index]->segment.const_pool + (size_t)i);
        switch (c_entry->type) {

            case EXPOSE_REG_CONST:
                /* Exposed Register */
                if (src_node(context->exposed_reg_tree,
                             ((expose_reg_constant *) c_entry)->index,
                             (size_t *) &g_reg)) {
                    /* Register already exposed / initialised */
                    context->modules[mod_index]
                            ->globals[((expose_reg_constant *) c_entry)
                            ->global_reg] =
                            g_reg;
                    context->modules[mod_index]
                            ->globals_dont_free[((expose_reg_constant *) c_entry)
                            ->global_reg] = 1;
                } else {
                    /* Need to initialise a register and expose it in the search tree */
                    context->modules[mod_index]
                            ->globals[((expose_reg_constant *) c_entry)
                            ->global_reg] =
                                value_f_in(context->worker.memory_worker);
                    add_node(&context->exposed_reg_tree, ((expose_reg_constant *)c_entry)->index,
                             (size_t)(context->modules[mod_index]
                                     ->globals[((expose_reg_constant *)c_entry)
                                     ->global_reg]));
                }
                break;

            case EXPOSE_PROC_CONST:
                /* Exposed Procedure */
                p_entry = rxvm_get_module_runtime_procedure(
                        context->modules[mod_index],
                        ((expose_proc_constant *) c_entry)->procedure);

                if (((expose_proc_constant *) c_entry)->imported) {
                    /* Imported - Add to the unresolved symbols count for later */
                    context->modules[mod_index]->unresolved_symbols++;
                }
                else {
                    /* Exported - check duplicate */
                    if (add_node(&context->exposed_proc_tree,
                                 ((expose_proc_constant *) c_entry)->index,
                                 (size_t) p_entry)) {
                        DEBUG("WARNING: Duplicate exposed symbol: %s\n",
                              ((expose_proc_constant *) c_entry)->index);
                        context->modules[mod_index]->duplicated_symbols++;
                    }
                }
                break;

            default:;
        }

        i = ((expose_reg_constant *)c_entry)->next;
    }

    DEBUG("Resolve Symbols\n");
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        /* Skip modules without any unresolved symbols */
        if (!context->modules[mod_index]->unresolved_symbols) continue;
        i = context->modules[mod_index]->expose_head;
        while (i != -1) {
            c_entry = (chameleon_constant *)(context->modules[mod_index]->segment.const_pool + (size_t)i);
            switch (c_entry->type) {
                case EXPOSE_PROC_CONST:
                    if (((expose_proc_constant *) c_entry)->imported) {
                        if (src_node(context->exposed_proc_tree,
                                     ((expose_proc_constant *) c_entry)->index,
                                     (size_t *) &p_entry_linked)) {

                            /* Patch the procedure entry with the linked one */
                            p_entry = rxvm_get_module_runtime_procedure(
                                    context->modules[mod_index],
                                    ((expose_proc_constant *) c_entry)->procedure);
                            if (p_entry->start == SIZE_MAX ) { /* If not already linked up */
                                p_entry->locals = p_entry_linked->locals;
                                p_entry->start = p_entry_linked->start;
                                p_entry->binarySpace =
                                        p_entry_linked->binarySpace;
                                p_entry->frame_free_list =
                                        p_entry_linked->frame_free_list;
                                if (p_entry_linked->native_invoker) {
                                    if (p_entry_linked->native_invoker ==
                                        rxvm_callfunc_session) {
                                        p_entry->native_capabilities =
                                                p_entry_linked->native_capabilities;
                                        p_entry->native_invoker =
                                                rxvm_callfunc_session;
                                    } else {
                                        bind_rxpa_runtime_policy(
                                                context, p_entry,
                                                p_entry_linked->native_capabilities,
                                                0);
                                    }
                                }

                                /* Reduce the number of unresolved symbols */
                                context->modules[mod_index]
                                        ->unresolved_symbols--;
                            }
                        }
                    }
                    break;

                default:;
            }

            i = ((expose_proc_constant *)c_entry)->next;
        }
    }

    /* Allocate Module Globals that have not already been allocated during linking */
    DEBUG("Allocate Globals\n");
    mod_index = module_number_to_link;
    for (i = 0; i < context->modules[mod_index]->segment.globals; i++) {
        if (!context->modules[mod_index]->globals[i]) {
            context->modules[mod_index]->globals[i] =
                    value_f_in(context->worker.memory_worker);
        }
    }

}

/* Common Functionality to prep and link a module */
/* Returns the new number of modules */
size_t rxvm_materialize_module_overlay(
        rxvm_context *context,
        module_file *file_module_section) {
    size_t n = context->num_modules;
    void *new_buffer;

    DEBUG("Loading Module %s\n", file_module_section->name);

    /* Grow the module buffer if need be */
    while (n + 1 > context->module_buffer_size) {
        context->module_buffer_size *= 2;
        new_buffer = rxvm_memory_resize_bytes(
                context->worker.memory_worker, context->modules,
                sizeof(module*) * (context->module_buffer_size / 2u),
                sizeof(module*) * context->module_buffer_size);
        if (!new_buffer) {
            RX_PANIC_OOM("realloc rxvm module table",
                         sizeof(module*) * context->module_buffer_size,
                         file_module_section->name);
        }
        context->modules = new_buffer;
    }
    context->modules[n] = rxvm_memory_alloc_bytes(
            context->worker.memory_worker,
                                                  sizeof(module));
    if (!context->modules[n]) {
        RX_PANIC_OOM("malloc rxvm module", sizeof(module), file_module_section->name);
    }
    context->modules[n]->memory_worker = context->worker.memory_worker;
    context->modules[n]->segment.globals = file_module_section->header.globals;
    context->modules[n]->segment.inst_size = file_module_section->header.instruction_size;
    context->modules[n]->segment.const_size = file_module_section->header.constant_size;
    context->modules[n]->segment.binary = file_module_section->instructions;
    context->modules[n]->segment.const_pool = file_module_section->constant;
    context->modules[n]->segment.module = context->modules[n];
    context->modules[n]->name = file_module_section->name;
    context->modules[n]->description = file_module_section->description;
    context->modules[n]->proc_head = file_module_section->header.proc_head;
    context->modules[n]->expose_head = file_module_section->header.expose_head;
    context->modules[n]->meta_head = file_module_section->header.meta_head;
    context->modules[n]->globals = rxvm_memory_calloc_bytes(
            context->worker.memory_worker,
            context->modules[n]->segment.globals,
            sizeof(value*));
    if (!context->modules[n]->globals && context->modules[n]->segment.globals) {
        RX_PANIC_OOM("calloc rxvm module globals",
                     (size_t)context->modules[n]->segment.globals * sizeof(value*),
                     file_module_section->name);
    }
    context->modules[n]->globals_dont_free = rxvm_memory_calloc_bytes(
            context->worker.memory_worker,
            context->modules[n]->segment.globals,
            sizeof(char));
    if (!context->modules[n]->globals_dont_free && context->modules[n]->segment.globals) {
        RX_PANIC_OOM("calloc rxvm module globals ownership map",
                     (size_t)context->modules[n]->segment.globals * sizeof(char),
                     file_module_section->name);
    }
    context->modules[n]->unresolved_symbols = 0;
    context->modules[n]->duplicated_symbols = 0;
    context->modules[n]->file = file_module_section;
    context->modules[n]->native = file_module_section->native;
    context->modules[n]->state = RXVM_MOD_LOADED;
    context->modules[n]->initializer_state = RXVM_INIT_UNINITIALIZED;
    context->modules[n]->procedures = 0;
    context->modules[n]->procedure_count = 0;
    context->modules[n]->proc_runtime_lookup = 0;
    context->modules[n]->proc_runtime_lookup_size = 0;
    context->modules[n]->execution_image = 0;
    context->modules[n]->graph_binding = 0;
    context->modules[n]->dynamic_site_cache_slots = 0;
    context->modules[n]->dynamic_site_caches = 0;
    context->modules[n]->dynamic_site_cache_count = 0u;
    build_module_runtime_procedures(context->modules[n]);
    build_module_dynamic_site_caches(context->modules[n]);
    context->link_dirty = 1;
    context->interface_method_registry_dirty = 1;
    context->interface_factory_registry_dirty = 1;

    context->num_modules = context->modules[n]->module_number = n + 1;

    return context->num_modules ;
}

/* Loads a module from a file
 * returns <= 0  - Error
 *         >0 - Last Module Number loaded (1 based) (more than one might have been loaded ...)  */
static int rxldmod_internal(rxvm_context *context, char *file_name,
                            const char *expected_provider_id) {
    FILE *fp;
    module_file *file_module_section;
    size_t modules_processed = 0;
    int loaded_rc;
    size_t n = 0;

    DEBUG("Loading Module(s) from file %s\n", file_name);

    // Check if the file is a crexx module
    // if context->location is not set we need to first check the file_name as an absolute path
    // then if not found try as a relative path from the current working directory ('.')
    char *location = context->location;
    char found_location[MAXFILEPATH];
    int file_exists = 0;

    found_location[0] = 0;

    /* Determine if provided file_name already contains an extension */
    int has_ext = 0;
    int has_plugin_ext = 0;
    if (file_name) {
        const char *last_slash = strrchr(file_name, '/');
#ifdef _WIN32
        const char *last_bsl = strrchr(file_name, '\\');
        if (!last_slash || (last_bsl && last_bsl > last_slash)) last_slash = last_bsl;
#endif
        const char *fname = last_slash ? last_slash + 1 : file_name;
        const char *extension = strrchr(fname, '.');
        if (extension != NULL) {
            has_ext = 1;
            has_plugin_ext = strcmp(extension, ".rxplugin") == 0;
        }
    }
    const char *type_bin = has_ext ? "" : "rxbin";

    // Check if the file exists as an absolute path
    if (!has_plugin_ext && fileexists(file_name, (char*)type_bin, 0)) {
        if (context->debug_mode) fprintf(stderr, "DEBUG_EXIT: Found module %s (as absolute path)\n", file_name);
        file_exists = 1;
    } else if (!has_plugin_ext && location) {
        char *loc_copy = rxvm_load_memory_strdup(
                context->worker.memory_worker,
                                                 location);
        if (!loc_copy) RX_PANIC_OOM("copy rxvm module location",
                                    strlen(location) + 1u, location);
        char *token = strtok(loc_copy, ";");
        while (token) {
            if (context->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Checking for module %s in location %s\n", file_name, token);
            if (fileexists(file_name, (char*)type_bin, token)) {
                strncpy(found_location, token, MAXFILEPATH - 1);
                found_location[MAXFILEPATH - 1] = 0;
                file_exists = 1;
                break;
            }
            token = strtok(NULL, ";");
        }
        rxvm_load_memory_free(loc_copy);
    } else if (!has_plugin_ext) {
        // Try as a relative path from the current working directory
        if (context->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Checking for module %s in current directory\n", file_name);
        if (fileexists(file_name, (char*)type_bin, ".")) {
            strncpy(found_location, ".", MAXFILEPATH - 1);
            found_location[MAXFILEPATH - 1] = 0;
            file_exists = 1;
        }
    }

    if (file_exists) {
        DEBUG("CREXX Module file\n");
        fp = openfile(file_name, (char*)type_bin, found_location[0] ? found_location : 0, "rb");
        if (!fp) return 0;

        loaded_rc = 0;
        while (loaded_rc == 0) {
            file_module_section = 0;
            switch (loaded_rc = read_module(&file_module_section, fp)) {
                case 0: /* Success */
                    DEBUG("Module Read\n");
                    n = rxvm_materialize_module_overlay(
                            context, file_module_section);
                    DEBUG("Module Prep and linked\n");
                    modules_processed++;
                    break;

                case 1: /* eof */
                    if (file_module_section) free_module(file_module_section);
                    if (!modules_processed) {
                        DEBUG("ERROR: empty file %s\n", file_name);
                        fclose(fp);
                        return 0;
                    }
                    break;

                default: /* error */
                    if (file_module_section) free_module(file_module_section);
                    DEBUG("ERROR: reading file %s\n", file_name);
                    fclose(fp);
                    return(-1);
            }
        }
        fclose(fp);
    }

    else {
        // Check if the file is a native plugin
        // if context->location is not set we need to first check the file_name as an absolute path
        // then if not found try as a relative path from the current working directory ('.')
        location = context->location;
        found_location[0] = 0;
        file_exists = 0;
        /* For plugins, also avoid appending extension if one is already provided */
        const char *type_plugin = has_ext ? "" : "rxplugin";

        // Check if the file exists as an absolute path
        if (fileexists(file_name, (char*)type_plugin, 0)) {
            if (context->debug_mode) fprintf(stderr, "DEBUG_EXIT: Found plugin %s (as absolute path)\n", file_name);
            file_exists = 1;
        } else if (location) {
            char *loc_copy = rxvm_load_memory_strdup(
                    context->worker.memory_worker, location);
            if (!loc_copy) RX_PANIC_OOM("copy rxvm plugin location",
                                        strlen(location) + 1u, location);
            char *token = strtok(loc_copy, ";");
            while (token) {
                if (context->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Checking for plugin %s in location %s\n", file_name, token);
                if (fileexists(file_name, (char*)type_plugin, token)) {
                    strncpy(found_location, token, MAXFILEPATH - 1);
                    found_location[MAXFILEPATH - 1] = 0;
                    file_exists = 1;
                    break;
                }
                token = strtok(NULL, ";");
            }
            rxvm_load_memory_free(loc_copy);
        } else {
            // Try as a relative path from the current working directory
            if (context->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Checking for plugin %s in current directory\n", file_name);
            if (fileexists(file_name, (char*)type_plugin, ".")) {
                strncpy(found_location, ".", MAXFILEPATH - 1);
                found_location[MAXFILEPATH - 1] = 0;
                file_exists = 1;
            }
        }

        if (file_exists) {
            rxvm_context *previous_active_context;
            rxpa_loaded_plugin loaded_plugin;
            DEBUG("CREXX Native Plugin file\n");
            // Check if the file exists
            fp = openfile(file_name, (char*)type_plugin, found_location[0] ? found_location : 0, "rb");
            if (!fp) return -1;
            fclose(fp);

            // Create the rxpa_initctxptr context
            struct rxpa_initctxptr rxpa_functions;
            rxpa_functions.addfunc = rxvm_addfunc;
            rxpa_functions.addclass = rxvm_addclass;
            rxpa_functions.addinterface = rxvm_addinterface;
            rxpa_functions.addimplements = rxvm_addimplements;
            rxpa_functions.addmember = rxvm_addmember;
            rxpa_functions.getstring = rxvm_getstring;
            rxpa_functions.setstring = rxvm_setstring;
            rxpa_functions.setint = rxvm_setint;
            rxpa_functions.getint = rxvm_getint;
            rxpa_functions.setfloat = rxvm_setfloat;
            rxpa_functions.getfloat = rxvm_getfloat;
            rxpa_functions.setnativepayload = rxvm_setnativepayload;
            rxpa_functions.getnativepayload = rxvm_getnativepayload;
            rxpa_functions.getnumattrs = rxvm_getnumattrs;
            rxpa_functions.setnumattrs = rxvm_setnumattrs;
            rxpa_functions.getattr = rxvm_getattr;
            rxpa_functions.insertattr = rxvm_insertattr;
            rxpa_functions.removeattr = rxvm_removeattr;
            rxpa_functions.swapattrs = rxvm_swapattrs;

            // Exit Function Management
            rxpa_functions.setsayexit = rxvm_setsayexit;
            rxpa_functions.resetsayexit = rxvm_resetsayexit;
            rxpa_functions.isinitialized = rxvm_isinitialized;

            // Load the plugin - and run the plugin initialization function.
            // Declarative provider lookup supplies the canonical filename with
            // its suffix; the legacy CLI also accepts an extensionless base.
            size_t full_file_name_size = strlen(file_name) +
                    (has_ext ? 1u : strlen(".rxplugin") + 1u);
            char *full_file_name = rxvm_load_memory_alloc(
                    context->worker.memory_worker, full_file_name_size);
            if (!full_file_name) RX_PANIC_OOM("build rxpa plugin filename",
                                               full_file_name_size, file_name);
            if (has_ext) strcpy(full_file_name, file_name);
            else sprintf(full_file_name, "%s.rxplugin", file_name);

            // Create rxpa_context and module for the addfunc callback
            previous_active_context = rxvm_active_context_enter(context);
            current_rxpa_context = rxpa_context_f(context);
            current_rxpa_context->plugin_being_loaded = malloc(sizeof(module_file));
            init_module(current_rxpa_context->plugin_being_loaded);
            current_rxpa_context->plugin_being_loaded->header.name_size = strlen(full_file_name) + 1;
            current_rxpa_context->plugin_being_loaded->name = malloc(
                    current_rxpa_context->plugin_being_loaded->header.name_size);
            strcpy(current_rxpa_context->plugin_being_loaded->name, full_file_name);
            current_rxpa_context->plugin_being_loaded->fromfile = 0;
            current_rxpa_context->plugin_being_loaded->native = 1;
            current_rxpa_context->plugin_being_loaded->header.expose_head = -1;
            current_rxpa_context->plugin_being_loaded->header.proc_head = -1;
            current_rxpa_context->plugin_being_loaded->header.meta_head = -1;

            // Open the plugin, validate its optional capability declaration,
            // then run the legacy initializer under the loader transaction.
            int rc = rxpa_open_plugin(
                    found_location[0] ? found_location : 0,
                    full_file_name, &loaded_plugin);
            if (!rc) {
                if (expected_provider_id &&
                    (!loaded_plugin.plugin_id ||
                     strcmp(loaded_plugin.plugin_id,
                            expected_provider_id) != 0)) {
                    fprintf(stderr,
                            "ERROR: RXPA provider %s resolved to manifest id %s in %s\n",
                            expected_provider_id,
                            loaded_plugin.plugin_id
                                ? loaded_plugin.plugin_id : "<missing>",
                            full_file_name);
                    rc = -1;
                }
            }
            if (!rc) {
                current_rxpa_context->dynamic_plugin = &loaded_plugin;
                current_rxpa_context->plugin_capabilities =
                        loaded_plugin.capabilities;
                if (loaded_plugin.has_manifest_v2 &&
                    loaded_plugin.manifest_v2.session_create) {
                    rc = rxpa_context_create_session(
                            current_rxpa_context,
                            &loaded_plugin.manifest_v2);
                }
                if (!rc) {
                    rc = rxpa_initialize_plugin(
                            &loaded_plugin, &rxpa_functions);
                }
            }
            rxvm_load_memory_free(full_file_name);

            // Check Result
            if (!rc) {
                DEBUG("CREXX Plugin %s loaded successfully\n", file_name);
                n = rxvm_materialize_module_overlay(
                        context, current_rxpa_context->plugin_being_loaded);
                apply_rxpa_proc_policies(current_rxpa_context, n);
                rxpa_context_publish_sessions(current_rxpa_context);
                current_rxpa_context->plugin_being_loaded = 0; // We are done with it! It will be freed eventually
                {
                    struct rxpa_library_reference *reference =
                            rxvm_load_memory_alloc(
                                    context->worker.memory_worker,
                                    sizeof(*reference));
                    if (!reference) RX_PANIC_OOM(
                            "allocate rxpa library reference",
                            sizeof(*reference), file_name);
                    reference->plugin = loaded_plugin;
                    reference->next = context->rxpa_libraries;
                    context->rxpa_libraries = reference;
                }
            } else {
                DEBUG("Failed to load plugin %s (rc=%d)\n", file_name, rc);
                free_rxpa_context(current_rxpa_context);
                current_rxpa_context = 0;
                rxpa_close_plugin(&loaded_plugin);
                rxvm_active_context_leave(previous_active_context);
                return(-1);
            }

            // Free the rxpa_context
            free_rxpa_context(current_rxpa_context);
            current_rxpa_context = 0;
            rxvm_active_context_leave(previous_active_context);
        }
        // Else an unrecognised file
        else {
            DEBUG("Unrecognised file type %s\n", file_name);
            return(-1);
        }
    }
    return (int)(n); /* Module Number */
}

int rxldmod(rxvm_context *context, char *file_name) {
    return rxldmod_internal(context, file_name, 0);
}

int rxldmod_provider(rxvm_context *context, char *provider_file,
                     const char *expected_provider_id) {
    if (!expected_provider_id || !*expected_provider_id) return -1;
    return rxldmod_internal(context, provider_file, expected_provider_id);
}

static const char *provider_module_string(const module *mod, size_t offset) {
    const string_constant *value;
    size_t minimum;

    if (!mod || !mod->segment.const_pool ||
        offset > mod->segment.const_size ||
        mod->segment.const_size - offset < sizeof(string_constant)) return 0;
    value = (const string_constant *)(mod->segment.const_pool + offset);
    if (value->base.type != STRING_CONST ||
        value->base.size_in_pool < sizeof(string_constant)) return 0;
    minimum = offsetof(string_constant, string) + value->string_len + 1u;
    if (value->base.size_in_pool < minimum ||
        value->base.size_in_pool > mod->segment.const_size - offset ||
        value->string[value->string_len] != 0) return 0;
    return value->string;
}

static const meta_func_constant *provider_module_function(
        const module *mod, const char *symbol) {
    int offset;
    size_t visited = 0u;

    if (!mod || !symbol) return 0;
    offset = mod->meta_head;
    while (offset != -1 && visited++ <= mod->segment.const_size / 8u + 1u) {
        const meta_entry *entry;
        if (offset < 0 || (size_t)offset > mod->segment.const_size ||
            mod->segment.const_size - (size_t)offset < sizeof(meta_entry)) {
            return 0;
        }
        entry = (const meta_entry *)(mod->segment.const_pool + (size_t)offset);
        if (entry->base.type == META_FUNC &&
            entry->base.size_in_pool >= sizeof(meta_func_constant)) {
            const meta_func_constant *function =
                    (const meta_func_constant *)entry;
            const char *candidate = provider_module_string(
                    mod, function->symbol);
            if (candidate && strcmp(candidate, symbol) == 0) return function;
        }
        offset = entry->next;
    }
    return 0;
}

static int provider_signature_punctuation(unsigned char ch) {
    return ch == '=' || ch == ',' || ch == '[' || ch == ']' || ch == '*';
}

/* RXPA declarations are source-like text, while rxc emits canonical RXAS
 * signature spelling.  Normalize only lexical trivia that has no signature
 * meaning: whitespace around punctuation, runs of other whitespace, and the
 * source shorthand [] for the canonical unbounded array shape [*]. */
static char *provider_signature_canonical(const char *text) {
    size_t input_length;
    size_t input_index = 0u;
    size_t output_index = 0u;
    char *result;

    if (!text) text = "";
    input_length = strlen(text);
    if (input_length > (SIZE_MAX - 1u) / 2u) {
        RX_PANIC_OOM("canonicalize RXPA signature", input_length, text);
    }
    result = malloc(input_length * 2u + 1u);
    if (!result) {
        RX_PANIC_OOM("canonicalize RXPA signature",
                     input_length * 2u + 1u, text);
    }
    while (input_index < input_length) {
        unsigned char ch = (unsigned char)text[input_index];
        if (isspace(ch)) {
            size_t next = input_index + 1u;
            while (next < input_length &&
                   isspace((unsigned char)text[next])) next++;
            if (output_index && next < input_length &&
                !provider_signature_punctuation(
                        (unsigned char)result[output_index - 1u]) &&
                !provider_signature_punctuation(
                        (unsigned char)text[next])) {
                result[output_index++] = ' ';
            }
            input_index = next;
            continue;
        }
        if (ch == '[') {
            size_t next = input_index + 1u;
            while (next < input_length &&
                   isspace((unsigned char)text[next])) next++;
            if (next < input_length && text[next] == ']') {
                result[output_index++] = '[';
                result[output_index++] = '*';
                result[output_index++] = ']';
                input_index = next + 1u;
                continue;
            }
        }
        result[output_index++] = (char)ch;
        input_index++;
    }
    if (output_index && result[output_index - 1u] == ' ') output_index--;
    result[output_index] = 0;
    return result;
}

static int provider_signature_text_equal(const char *left,
                                         const char *right) {
    char *canonical_left = provider_signature_canonical(left);
    char *canonical_right = provider_signature_canonical(right);
    int equal = strcmp(canonical_left, canonical_right) == 0;
    free(canonical_left);
    free(canonical_right);
    return equal;
}

static int provider_module_declares(const module *mod,
                                    const char *provider_id,
                                    const char *symbol) {
    int offset;
    size_t visited = 0u;

    if (!mod || !mod->native || !provider_id) return 0;
    offset = mod->meta_head;
    while (offset != -1 && visited++ <= mod->segment.const_size / 8u + 1u) {
        const meta_entry *entry;
        if (offset < 0 || (size_t)offset > mod->segment.const_size ||
            mod->segment.const_size - (size_t)offset < sizeof(meta_entry)) {
            return 0;
        }
        entry = (const meta_entry *)(mod->segment.const_pool + (size_t)offset);
        if (entry->base.type == META_PROVIDER &&
            entry->base.size_in_pool >= sizeof(meta_provider_constant)) {
            const meta_provider_constant *provider =
                    (const meta_provider_constant *)entry;
            const char *candidate_id = provider_module_string(
                    mod, provider->provider);
            const char *candidate_symbol = provider_module_string(
                    mod, provider->symbol);
            if (candidate_id && strcmp(candidate_id, provider_id) == 0 &&
                (!symbol || (candidate_symbol &&
                             strcmp(candidate_symbol, symbol) == 0))) {
                return 1;
            }
        }
        offset = entry->next;
    }
    return 0;
}

static int provider_signature_matches(const module *required_module,
                                      const meta_func_constant *required,
                                      const module *native_module,
                                      const meta_func_constant *native) {
    const char *required_option = provider_module_string(
            required_module, required->option);
    const char *required_type = provider_module_string(
            required_module, required->type);
    const char *required_args = provider_module_string(
            required_module, required->args);
    const char *native_option = provider_module_string(
            native_module, native->option);
    const char *native_type = provider_module_string(
            native_module, native->type);
    const char *native_args = provider_module_string(
            native_module, native->args);

    return required_option && required_type && required_args &&
           native_option && native_type && native_args &&
           provider_signature_text_equal(required_option, native_option) &&
           provider_signature_text_equal(required_type, native_type) &&
           provider_signature_text_equal(required_args, native_args);
}

/* Returns 1 for an exact provider/callable/signature match, 0 when the
 * provider or callable is absent, and -1 for a conflicting loaded signature. */
static int provider_callable_loaded(rxvm_context *context,
                                    const char *provider_id,
                                    const char *symbol,
                                    const module *required_module,
                                    const meta_func_constant *required) {
    size_t index;
    int provider_seen = 0;

    for (index = 0u; index < context->num_modules; index++) {
        const module *candidate = context->modules[index];
        const meta_func_constant *native;
        if (!provider_module_declares(candidate, provider_id, 0)) continue;
        provider_seen = 1;
        if (!provider_module_declares(candidate, provider_id, symbol)) continue;
        native = provider_module_function(candidate, symbol);
        if (!native) return -1;
        if (!provider_signature_matches(required_module, required,
                                        candidate, native)) return -1;
        return 1;
    }
    return provider_seen ? -1 : 0;
}

static int provider_id_valid(const char *provider_id) {
    const unsigned char *cursor = (const unsigned char *)provider_id;
    if (!provider_id ||
        !( (*cursor >= 'A' && *cursor <= 'Z') ||
           (*cursor >= 'a' && *cursor <= 'z') ||
           (*cursor >= '0' && *cursor <= '9'))) return 0;
    cursor++;
    while (*cursor) {
        if (!( (*cursor >= 'A' && *cursor <= 'Z') ||
               (*cursor >= 'a' && *cursor <= 'z') ||
               (*cursor >= '0' && *cursor <= '9') ||
               *cursor == '.' || *cursor == '_' || *cursor == '-')) return 0;
        cursor++;
    }
    return 1;
}

static int load_declared_provider(rxvm_context *context,
                                  const char *provider_id) {
    char *locations;
    char *cursor;
    char *directory;
    int candidate_seen = 0;

    if (!context->provider_location || !*context->provider_location) return 0;
    if (!provider_id_valid(provider_id)) return 0;
    locations = strdup(context->provider_location);
    if (!locations) RX_PANIC_OOM("copy RXPA provider path",
                                 strlen(context->provider_location) + 1u,
                                 provider_id);
    cursor = locations;
    while (cursor) {
        char artifact_path[MAXFILEPATH * 2u];
        char *next = strchr(cursor, ';');
        size_t length;
        directory = cursor;
        if (next) {
            *next = 0;
            cursor = next + 1;
        } else cursor = 0;
        if (!*directory) {
            continue;
        }
        length = strlen(directory);
        if (strlen(directory) + strlen(provider_id) +
            strlen("/.rxplugin") + 1u > sizeof(artifact_path)) continue;
        snprintf(artifact_path, sizeof(artifact_path), "%s%s%s.rxplugin",
                 directory,
                 length && (directory[length - 1u] == '/' ||
                            directory[length - 1u] == '\\') ? "" : "/",
                 provider_id);
        if (!fileexists(artifact_path, "", 0)) continue;
        candidate_seen = 1;
        if (rxldmod_provider(context, artifact_path, provider_id) > 0) {
            free(locations);
            return 1;
        }
        fprintf(stderr,
                "ERROR: rejected RXPA provider artifact %s for %s\n",
                artifact_path, provider_id);
    }
    if (candidate_seen) {
        fprintf(stderr, "ERROR: no valid RXPA provider artifact for %s\n",
                provider_id);
    }
    free(locations);
    return 0;
}

int rxvm_resolve_provider_dependencies(rxvm_context *context) {
    size_t module_index;

    if (!context) return -1;
    for (module_index = 0u; module_index < context->num_modules;
         module_index++) {
        module *required_module = context->modules[module_index];
        int offset;
        size_t visited = 0u;
        if (required_module->native) continue;
        offset = required_module->meta_head;
        while (offset != -1 &&
               visited++ <= required_module->segment.const_size / 8u + 1u) {
            const meta_entry *entry;
            if (offset < 0 ||
                (size_t)offset > required_module->segment.const_size ||
                required_module->segment.const_size - (size_t)offset <
                        sizeof(meta_entry)) return -1;
            entry = (const meta_entry *)(required_module->segment.const_pool +
                                         (size_t)offset);
            if (entry->base.type == META_PROVIDER &&
                entry->base.size_in_pool >= sizeof(meta_provider_constant)) {
                const meta_provider_constant *provider =
                        (const meta_provider_constant *)entry;
                const char *provider_id = provider_module_string(
                        required_module, provider->provider);
                const char *symbol = provider_module_string(
                        required_module, provider->symbol);
                const meta_func_constant *required =
                        provider_module_function(required_module, symbol);
                int resolved;
                if (!provider_id_valid(provider_id) ||
                    !symbol || !*symbol ||
                    !required) return -1;
                resolved = provider_callable_loaded(
                        context, provider_id, symbol,
                        required_module, required);
                if (resolved < 0) {
                    fprintf(stderr,
                            "ERROR: RXPA provider %s does not supply compatible callable %s required by %s\n",
                            provider_id, symbol, required_module->name);
                    return -1;
                }
                if (!resolved && load_declared_provider(context, provider_id)) {
                    resolved = provider_callable_loaded(
                            context, provider_id, symbol,
                            required_module, required);
                }
                if (resolved != 1 &&
                    (provider->flags & RXBIN_PROVIDER_REQUIRED)) {
                    fprintf(stderr,
                            "ERROR: required RXPA provider %s for %s (module %s) was not resolved; searched: %s\n",
                            provider_id, symbol, required_module->name,
                            context->provider_location
                                ? context->provider_location : "<none>");
                    return -1;
                }
            }
            offset = entry->next;
        }
    }
    return 0;
}

static int autoload_artifact_seen(const rxvm_context *context,
                                  const char *artifact) {
    size_t i;
    for (i = 0u; i < context->autoloaded_artifact_count; i++) {
        if (strcmp(context->autoloaded_artifacts[i], artifact) == 0) return 1;
    }
    return 0;
}

static void remember_autoload_artifact(rxvm_context *context,
                                       const char *artifact) {
    char **items;
    size_t capacity;
    char *copy;

    if (autoload_artifact_seen(context, artifact)) return;
    if (context->autoloaded_artifact_count ==
        context->autoloaded_artifact_capacity) {
        capacity = context->autoloaded_artifact_capacity
                ? context->autoloaded_artifact_capacity * 2u : 8u;
        items = (char **)realloc(context->autoloaded_artifacts,
                                capacity * sizeof(*items));
        if (!items) RX_PANIC_OOM("grow RXBIN autoload set",
                                 capacity * sizeof(*items), artifact);
        context->autoloaded_artifacts = items;
        context->autoloaded_artifact_capacity = capacity;
    }
    copy = strdup(artifact);
    if (!copy) RX_PANIC_OOM("copy RXBIN autoload stem",
                            strlen(artifact) + 1u, artifact);
    context->autoloaded_artifacts[context->autoloaded_artifact_count++] = copy;
}

static int module_symbol_unresolved(module *mod, const char *symbol) {
    int offset;

    if (!mod || !symbol || !mod->unresolved_symbols) return 0;
    offset = mod->expose_head;
    while (offset != -1) {
        chameleon_constant *entry =
                (chameleon_constant *)(mod->segment.const_pool + (size_t)offset);
        if (entry->type == EXPOSE_PROC_CONST) {
            expose_proc_constant *exposed = (expose_proc_constant *)entry;
            if (exposed->imported && strcmp(exposed->index, symbol) == 0) {
                proc_runtime *runtime = rxvm_get_module_runtime_procedure(
                        mod, exposed->procedure);
                return runtime && runtime->start == SIZE_MAX;
            }
        }
        offset = ((expose_proc_constant *)entry)->next;
    }
    return 0;
}

int rxvm_resolve_autoload_dependencies(rxvm_context *context) {
    size_t module_index;
    int loads = 0;

    if (!context || !context->autoload_enabled) return 0;
    for (module_index = 0u; module_index < context->num_modules;
         module_index++) {
        module *required_module = context->modules[module_index];
        int offset;
        size_t visited = 0u;

        if (!required_module || required_module->native ||
            !required_module->unresolved_symbols) continue;
        offset = required_module->meta_head;
        while (offset != -1 &&
               visited++ <= required_module->segment.const_size / 8u + 1u) {
            const meta_entry *entry;
            if (offset < 0 ||
                (size_t)offset > required_module->segment.const_size ||
                required_module->segment.const_size - (size_t)offset <
                        sizeof(meta_entry)) return -1;
            entry = (const meta_entry *)(required_module->segment.const_pool +
                                         (size_t)offset);
            if (entry->base.type == META_AUTOLOAD &&
                entry->base.size_in_pool >= sizeof(meta_autoload_constant)) {
                const meta_autoload_constant *autoload =
                        (const meta_autoload_constant *)entry;
                const char *symbol = provider_module_string(
                        required_module, autoload->symbol);
                const char *artifact = provider_module_string(
                        required_module, autoload->artifact);
                if (!symbol || !*symbol || !provider_id_valid(artifact)) {
                    fprintf(stderr,
                            "ERROR: invalid RXBIN autoload metadata in module %s\n",
                            required_module->name);
                    return -1;
                }
                if (module_symbol_unresolved(required_module, symbol) &&
                    !autoload_artifact_seen(context, artifact)) {
                    if (rxldmod(context, (char *)artifact) <= 0) {
                        fprintf(stderr,
                                "ERROR: RXBIN autoload could not resolve %s for %s; searched for %s.rxbin in %s\n",
                                symbol, required_module->name, artifact,
                                context->location ? context->location : ".");
                        return -1;
                    }
                    remember_autoload_artifact(context, artifact);
                    loads++;
                }
            }
            offset = entry->next;
        }
        if (offset != -1) {
            fprintf(stderr,
                    "ERROR: cyclic RXBIN metadata chain in module %s\n",
                    required_module->name);
            return -1;
        }
    }
    return loads;
}

/* Loads a module from a memory buffer
 * returns 0  - Error
 *         >0 - Last Module Number loaded (1 based) (more than one might have been loaded ...)  */
int rxldmodm(rxvm_context *context, char *buffer_start, size_t buffer_length) {
    module_file *file_module_section;
    size_t modules_processed = 0;
    int loaded_rc;
    size_t n;
    char *buffer_end = buffer_start + buffer_length;

    DEBUG("Loading Module(s) from memory\n");

    loaded_rc = 0;
    while (loaded_rc == 0) {
        file_module_section = 0;
        switch (loaded_rc = read_module_mem(&file_module_section, &buffer_start, buffer_end)) {
            case 0: /* Success */
                n = rxvm_materialize_module_overlay(
                        context, file_module_section);
                modules_processed++;
                break;

            case 1: /* eof */
                if (file_module_section) free_module(file_module_section);
                if (!modules_processed) {
                    DEBUG("ERROR: empty buffer\n");
                    return 0;
                }
                break;

            default: /* error */
                if (file_module_section) free_module(file_module_section);
                fprintf(stderr, "ERROR: reading buffer\n");
                exit(-1);
        }
    }

    return (int)(n); /* Module Number */
}

// RXPA (Plugin Architecture) Support Functions

// Free statically linked functions list
static void free_rxpa_context(rxpa_context *context)
{
    if (!context) return;
    rxpa_context_destroy_sessions(context);
    if (context->plugin_being_loaded) free_module(context->plugin_being_loaded);
    rxvm_load_memory_free(context->proc_policies);
    rxvm_load_memory_free(context);
}

// Create a new RXPA context for a module
static rxpa_context *rxpa_context_f(rxvm_context *rxvm_context) {
    rxpa_context *new_rxpa_context = rxvm_load_memory_alloc(
            rxvm_context->worker.memory_worker, sizeof(rxpa_context));
    if (!new_rxpa_context) {
        RX_PANIC_OOM("allocate rxpa context", sizeof(rxpa_context), 0);
    }
    new_rxpa_context->rxvm_context = rxvm_context;
    new_rxpa_context->plugin_being_loaded = 0;
    new_rxpa_context->const_buffer_size = 0;
    new_rxpa_context->const_buffer_top = 0;
    new_rxpa_context->meta_tail = -1;
    new_rxpa_context->plugin_capabilities = 0u;
    new_rxpa_context->dynamic_plugin = 0;
    new_rxpa_context->session_instances = 0;
    new_rxpa_context->proc_policies = 0;
    new_rxpa_context->proc_policy_count = 0u;
    new_rxpa_context->proc_policy_capacity = 0u;
    return new_rxpa_context;
}

static rxpa_session_instance *rxpa_context_find_session(
        const rxpa_context *context, const char *plugin_id) {
    rxpa_session_instance *instance;
    if (!context || !plugin_id) return 0;
    instance = context->session_instances;
    while (instance) {
        if (strcmp(instance->plugin_id, plugin_id) == 0) return instance;
        instance = instance->next;
    }
    return 0;
}

static int rxpa_context_create_session(
        rxpa_context *context, const rxpa_plugin_manifest_v2 *manifest) {
    rxpa_session_instance *instance;
    void *session;
    char *plugin_id;
    if (!context || !manifest || !manifest->plugin_id ||
        !manifest->session_create || !manifest->session_destroy ||
        !manifest->session_enter || !manifest->session_leave) {
        return -1;
    }
    if (rxpa_context_find_session(context, manifest->plugin_id)) return 0;
    session = manifest->session_create();
    if (!session) return -1;
    instance = rxvm_load_memory_alloc(
            context->rxvm_context->worker.memory_worker, sizeof(*instance));
    plugin_id = rxvm_load_memory_strdup(
            context->rxvm_context->worker.memory_worker,
            manifest->plugin_id);
    if (!instance || !plugin_id) {
        manifest->session_destroy(session);
        if (instance) rxvm_load_memory_free(instance);
        if (plugin_id) rxvm_load_memory_free(plugin_id);
        RX_PANIC_OOM("allocate rxpa plugin session",
                     sizeof(*instance) + strlen(manifest->plugin_id) + 1u,
                     manifest->plugin_id);
    }
    instance->plugin_id = plugin_id;
    instance->session = session;
    instance->destroy = manifest->session_destroy;
    instance->enter = manifest->session_enter;
    instance->leave = manifest->session_leave;
    instance->next = context->session_instances;
    context->session_instances = instance;
    return 0;
}

static void rxpa_context_publish_sessions(rxpa_context *context) {
    rxpa_session_instance *tail;
    if (!context || !context->session_instances) return;
    tail = context->session_instances;
    while (tail->next) tail = tail->next;
    tail->next = context->rxvm_context->rxpa_sessions;
    context->rxvm_context->rxpa_sessions = context->session_instances;
    context->session_instances = 0;
}

static void rxpa_context_destroy_sessions(rxpa_context *context) {
    while (context && context->session_instances) {
        rxpa_session_instance *next = context->session_instances->next;
        context->session_instances->destroy(
                context->session_instances->session);
        rxvm_load_memory_free((void *)context->session_instances->plugin_id);
        rxvm_load_memory_free(context->session_instances);
        context->session_instances = next;
    }
}

static unsigned char *rxpa_constant_pool_at(rxpa_context *context, size_t offset) {
    return (unsigned char *) context->plugin_being_loaded->constant + offset;
}

/* Reserves space in the constant pool for an entry and returns its index;
 * This is used to create a fake  constant pool for native functions with the function name and pointer
 * the caller can then populate the entry.
 * NOTE - THIS CALL MIGHT MOVE THE CONSTANT POOL - CHANGING ENTRY ADDRESSES (USE OFFSETS!)
 * The 'size' parameter is the size of the payload including
 * space for chameleon_constant etc.
 * Returns the index to the entry (from binary.const_pool)
 */
static size_t reserve_in_const_pool(rxpa_context *context, size_t size, enum const_pool_type type) {
    size_t index, new_size;
    chameleon_constant * entry;
    void* new_buffer;

    /* Create buffer if needed */
    if (!context->plugin_being_loaded->constant) {
        context->const_buffer_size = 1024;
        context->plugin_being_loaded->constant = malloc(context->const_buffer_size);
        if (!context->plugin_being_loaded->constant) {
            RX_PANIC_OOM("malloc rxpa plugin constant pool",
                         context->const_buffer_size,
                         context->plugin_being_loaded->name);
        }
        context->plugin_being_loaded->header.constant_size = 0;
    }

    /* Extend the buffer if we need to */
    while (size + 8 > context->const_buffer_size - context->plugin_being_loaded->header.constant_size) { // +8 for the 8 bit alignment
        new_size = context->const_buffer_size * 2;
        new_buffer = realloc(context->plugin_being_loaded->constant, new_size);
        if (!new_buffer) {
            RX_PANIC_OOM("realloc rxpa plugin constant pool",
                         new_size,
                         context->plugin_being_loaded->name);
        }
        context->plugin_being_loaded->constant = new_buffer;
        memset(rxpa_constant_pool_at(context, context->const_buffer_size),
               0, context->const_buffer_size);
        context->const_buffer_size = new_size;
    }

    /* We are putting the entry at the top of the pool */
    index = context->const_buffer_top;
    entry = (chameleon_constant *) rxpa_constant_pool_at(context, index);

    entry->type = type;

    /* Round up the size for alignment */
    size = (size + (size_t)7) & ~ (size_t)0x07; /* 8 byte alignment */

    /* Store the size */
    entry->size_in_pool = size;

    /* Move up the const_size "pointer" */
    context->plugin_being_loaded->header.constant_size += size;
    context->const_buffer_top += size;

    return index;
}

static size_t add_native_string_to_pool(rxpa_context *context, const char *value) {
    string_constant *sentry;
    size_t entry_index;
    size_t entry_size;
    size_t string_len;

    if (!value) value = "";
    string_len = strlen(value);
    entry_size = sizeof(string_constant) + string_len;
    entry_index = reserve_in_const_pool(context, entry_size, STRING_CONST);
    sentry = (string_constant *) rxpa_constant_pool_at(context, entry_index);
    sentry->string_len = string_len;
#ifndef NUTF8
    sentry->string_chars = string_len;
#endif
    memcpy(sentry->string, value, string_len + 1);
    return entry_index;
}

static size_t add_native_meta_entry(rxpa_context *context, size_t entry_size, enum const_pool_type type) {
    meta_entry *entry;
    size_t entry_index;

    entry_index = reserve_in_const_pool(context, entry_size, type);
    entry = (meta_entry *) rxpa_constant_pool_at(context, entry_index);

    if (context->plugin_being_loaded->header.meta_head != -1) {
        ((meta_entry *) rxpa_constant_pool_at(context, context->meta_tail))->next =
                (int) entry_index;
        entry->prev = context->meta_tail;
        context->meta_tail = (int)entry_index;
        entry->next = -1;
    }
    else {
        context->plugin_being_loaded->header.meta_head = (int)entry_index;
        context->meta_tail = (int)entry_index;
        entry->next = -1;
        entry->prev = -1;
    }

    entry->address = 0;
    return entry_index;
}

static void add_class_meta_to_module(rxpa_context *context, char *name, char *option, char *type) {
    size_t s_name = add_native_string_to_pool(context, name);
    size_t s_option = add_native_string_to_pool(context, option);
    size_t s_type = add_native_string_to_pool(context, type);
    size_t entry = add_native_meta_entry(context, sizeof(meta_class_constant), META_CLASS);
    meta_class_constant *meta;

    meta = (meta_class_constant *) rxpa_constant_pool_at(context, entry);
    meta->symbol = s_name;
    meta->option = s_option;
    meta->type = s_type;
}

static void add_interface_meta_to_module(rxpa_context *context, char *name, char *option, char *type) {
    size_t s_name = add_native_string_to_pool(context, name);
    size_t s_option = add_native_string_to_pool(context, option);
    size_t s_type = add_native_string_to_pool(context, type);
    size_t entry = add_native_meta_entry(context, sizeof(meta_interface_constant), META_INTERFACE);
    meta_interface_constant *meta;

    meta = (meta_interface_constant *) rxpa_constant_pool_at(context, entry);
    meta->symbol = s_name;
    meta->option = s_option;
    meta->type = s_type;
}

static void add_implements_meta_to_module(rxpa_context *context, char *name, char *interface_name) {
    size_t s_name = add_native_string_to_pool(context, name);
    size_t s_interface = add_native_string_to_pool(context, interface_name);
    size_t entry = add_native_meta_entry(context, sizeof(meta_implements_constant), META_IMPLEMENTS);
    meta_implements_constant *meta;

    meta = (meta_implements_constant *) rxpa_constant_pool_at(context, entry);
    meta->symbol = s_name;
    meta->interface_symbol = s_interface;
}

static void add_member_meta_to_module(rxpa_context *context, char *owner, char *kind, char *member, char *type, char *args) {
    size_t s_owner = add_native_string_to_pool(context, owner);
    size_t s_kind = add_native_string_to_pool(context, kind);
    size_t s_member = add_native_string_to_pool(context, member);
    size_t s_type = add_native_string_to_pool(context, type);
    size_t s_args = add_native_string_to_pool(context, args);
    size_t entry = add_native_meta_entry(context, sizeof(meta_member_constant), META_MEMBER);
    meta_member_constant *meta;

    meta = (meta_member_constant *) rxpa_constant_pool_at(context, entry);
    meta->owner = s_owner;
    meta->kind = s_kind;
    meta->member = s_member;
    meta->type = s_type;
    meta->args = s_args;
}

static void add_function_meta_to_module(rxpa_context *context,
                                        const char *name,
                                        const char *option,
                                        const char *type,
                                        const char *args,
                                        size_t procedure_offset) {
    meta_func_constant *meta;
    size_t entry;
    size_t symbol = add_native_string_to_pool(context, name);
    size_t meta_option = add_native_string_to_pool(context, option);
    size_t meta_type = add_native_string_to_pool(context, type);
    size_t meta_args = add_native_string_to_pool(context, args);

    entry = add_native_meta_entry(context, sizeof(*meta), META_FUNC);
    meta = (meta_func_constant *)rxpa_constant_pool_at(context, entry);
    meta->symbol = symbol;
    meta->option = meta_option;
    meta->type = meta_type;
    meta->args = meta_args;
    meta->func = procedure_offset;
}

static void add_provider_meta_to_module(rxpa_context *context,
                                        const char *name,
                                        const char *provider_id) {
    meta_provider_constant *meta;
    size_t entry;
    size_t symbol;
    size_t provider;

    if (!provider_id || !*provider_id) return;
    symbol = add_native_string_to_pool(context, name);
    provider = add_native_string_to_pool(context, provider_id);
    entry = add_native_meta_entry(context, sizeof(*meta), META_PROVIDER);
    meta = (meta_provider_constant *)rxpa_constant_pool_at(context, entry);
    meta->symbol = symbol;
    meta->provider = provider;
    meta->flags = RXBIN_PROVIDER_REQUIRED;
}

static void append_rxpa_proc_policy(rxpa_context *context,
                                    size_t procedure_offset,
                                    uint32_t capabilities,
                                    rxpa_session_instance *session_instance) {
    rxpa_proc_policy *policies;
    size_t new_capacity;

    if (context->proc_policy_count == context->proc_policy_capacity) {
        new_capacity = context->proc_policy_capacity
                ? context->proc_policy_capacity * 2u : 8u;
        policies = rxvm_memory_resize_bytes(
                context->rxvm_context->worker.memory_worker,
                context->proc_policies,
                context->proc_policy_count * sizeof(*policies),
                new_capacity * sizeof(*policies));
        if (!policies) RX_PANIC_OOM("grow rxpa procedure policy table",
                                    new_capacity * sizeof(*policies),
                                    context->plugin_being_loaded->name);
        context->proc_policies = policies;
        context->proc_policy_capacity = new_capacity;
    }

    context->proc_policies[context->proc_policy_count].procedure_offset =
            procedure_offset;
    context->proc_policies[context->proc_policy_count].capabilities =
            capabilities;
    context->proc_policies[context->proc_policy_count].session_instance =
            session_instance;
    context->proc_policy_count++;
}

static void apply_rxpa_proc_policies(rxpa_context *context,
                                     size_t module_number) {
    module *mod;
    size_t index;
    if (!context || !module_number ||
        module_number > context->rxvm_context->num_modules) return;
    mod = context->rxvm_context->modules[module_number - 1u];
    for (index = 0; index < context->proc_policy_count; index++) {
        proc_runtime *runtime = rxvm_get_module_runtime_procedure(
                mod, context->proc_policies[index].procedure_offset);
        if (!runtime) abort();
        size_t module_index;
        uint32_t capabilities = context->proc_policies[index].capabilities;
        rxpa_session_instance *session_instance =
                context->proc_policies[index].session_instance;
        bind_rxpa_runtime_policy(context->rxvm_context, runtime,
                                 capabilities, session_instance);

        /* Imports linked before the native policy was known share the owner's
         * frame-free-list identity. Bind each alias slot now as well. */
        for (module_index = 0u;
             module_index < context->rxvm_context->num_modules;
             module_index++) {
            module *alias_module =
                    context->rxvm_context->modules[module_index];
            size_t procedure_index;
            for (procedure_index = 0u;
                 procedure_index < alias_module->procedure_count;
                 procedure_index++) {
                proc_runtime *alias =
                        &alias_module->procedures[procedure_index];
                if (alias != runtime &&
                    alias->frame_free_list == runtime->frame_free_list) {
                    bind_rxpa_runtime_policy(context->rxvm_context,
                                             alias, capabilities,
                                             session_instance);
                }
            }
        }
    }
}

static void bind_rxpa_runtime_policy(rxvm_context *context,
                                     proc_runtime *runtime,
                                     uint32_t capabilities,
                                     rxpa_session_instance *session_instance) {
    runtime->native_capabilities = capabilities;
    if ((capabilities & RXPA_PLUGIN_CAP_PROCESS_REENTRANT) != 0u) {
        runtime->native_invoker = rxvm_callfunc_direct;
    } else if ((capabilities & RXPA_PROCEDURE_CAP_SESSION_AFFINE) != 0u &&
               session_instance) {
        rxpa_session_call_binding *binding;
        if (runtime->native_invoker == rxvm_callfunc_session) return;
        binding = rxvm_load_memory_alloc(
                context->worker.memory_worker, sizeof(*binding));
        if (!binding) RX_PANIC_OOM("bind session-aware rxpa procedure",
                                   sizeof(*binding), runtime->name);
        binding->function = (void *)runtime->start;
        binding->instance = session_instance;
        binding->procedure_capabilities = capabilities;
        binding->next = context->rxpa_session_bindings;
        context->rxpa_session_bindings = binding;
        runtime->start = (size_t)binding;
        runtime->native_invoker = rxvm_callfunc_session;
    } else if (!rxpa_compatibility_bind_legacy(
                       &context->rxpa_compatibility,
                       &runtime->native_invoker,
                       rxvm_callfunc_direct, rxvm_callfunc)) {
        RX_PANIC_OOM("bind legacy rxpa procedure",
                     sizeof(runtime->native_invoker), runtime->name);
    }
}

// Add a statically linked function to the constant pool being created
static size_t add_proc_to_module(rxpa_context* context, char* index,
                               rxpa_libfunc func, uint32_t capabilities,
                               rxpa_session_instance *session_instance) {
    size_t entry_size, proc_index, exposed_proc_index;
    proc_constant *proc;
    expose_proc_constant *exposed_proc;

    /* Create Procedure Entry */
    entry_size = sizeof(proc_constant) + strlen(index);
    proc_index = reserve_in_const_pool(context, entry_size, PROC_CONST);
    proc = (proc_constant *) rxpa_constant_pool_at(context, proc_index);

    /* Set structure data */
    memcpy(proc->name, index, strlen(index) + 1);
    proc->next = -1;
    proc->locals = 0;
    proc->start = (size_t)func;
    append_rxpa_proc_policy(context, proc_index, capabilities,
                            session_instance);

    /* Create Exposed Procedure Entry */
    entry_size = sizeof(expose_proc_constant) + strlen(index);
    exposed_proc_index = reserve_in_const_pool(context, entry_size,EXPOSE_PROC_CONST);
    exposed_proc = (expose_proc_constant *)
            rxpa_constant_pool_at(context, exposed_proc_index);

    /* Set structure data */
    memcpy(exposed_proc->index, index, strlen(index) + 1);
    exposed_proc->procedure = proc_index;
    exposed_proc->imported = 0;

    /* Chain the exposed constant entries */
    exposed_proc->next = context->plugin_being_loaded->header.expose_head;
    context->plugin_being_loaded->header.expose_head = (int)exposed_proc_index;

    proc = (proc_constant *) rxpa_constant_pool_at(context, proc_index);
    proc->exposed = exposed_proc_index;
    return proc_index;
}

static uint32_t static_plugin_capabilities_locked(const char *plugin_id) {
    struct static_plugin_capability *entry = static_plugin_capabilities;
    if (!plugin_id) return 0u;
    while (entry) {
        if (strcmp(entry->plugin_id, plugin_id) == 0) {
            return entry->capabilities;
        }
        entry = entry->next;
    }
    return 0u;
}

static struct static_plugin_manifest_v2 *
static_plugin_manifest_v2_locked(const char *plugin_id) {
    struct static_plugin_manifest_v2 *entry = static_plugin_manifests_v2;
    if (!plugin_id) return 0;
    while (entry) {
        if (strcmp(entry->plugin_id, plugin_id) == 0) return entry;
        entry = entry->next;
    }
    return 0;
}

static int validate_static_plugin_manifest_v2(
        const rxpa_plugin_manifest_v2 *manifest) {
    size_t minimum_size = offsetof(rxpa_plugin_manifest_v2, session_leave) +
                          sizeof(((rxpa_plugin_manifest_v2 *)0)->session_leave);
    unsigned int session_hook_count;
    if (!manifest || manifest->struct_size < minimum_size ||
        manifest->abi_version != RXPA_PLUGIN_MANIFEST_ABI_V2 ||
        !manifest->plugin_id || !*manifest->plugin_id ||
        !manifest->procedure_capabilities) {
        return 0;
    }
    session_hook_count = (manifest->session_create != NULL) +
                         (manifest->session_destroy != NULL) +
                         (manifest->session_enter != NULL) +
                         (manifest->session_leave != NULL);
    return session_hook_count == 0u || session_hook_count == 4u;
}

static uint32_t sanitize_manifest_procedure_capabilities(
        const rxpa_plugin_manifest_v2 *manifest,
        const char *procedure_name) {
    uint32_t capabilities;
    if (!manifest || !manifest->procedure_capabilities || !procedure_name) {
        return 0u;
    }
    capabilities = manifest->procedure_capabilities(procedure_name);
    if ((capabilities & ~RXPA_PROCEDURE_CAP_KNOWN_V2) != 0u ||
        capabilities == RXPA_PROCEDURE_CAP_KNOWN_V2) {
        return 0u;
    }
    if ((capabilities & RXPA_PROCEDURE_CAP_SESSION_AFFINE) != 0u &&
        !manifest->session_create) {
        return 0u;
    }
    return capabilities;
}

void rxvm_register_static_plugin_capability(const char *plugin_id,
                                            uint32_t capabilities) {
    struct static_plugin_capability *entry;
    if (!plugin_id || !*plugin_id) return;
    capabilities &= RXPA_PLUGIN_CAP_KNOWN_V1;
    RXPA_CATALOGUE_LOCK();
    entry = static_plugin_capabilities;
    while (entry) {
        if (strcmp(entry->plugin_id, plugin_id) == 0) {
            entry->capabilities |= capabilities;
            RXPA_CATALOGUE_UNLOCK();
            return;
        }
        entry = entry->next;
    }
    entry = rxvm_memory_alloc_unowned_bytes(sizeof(*entry));
    if (!entry) RX_PANIC_OOM("allocate static rxpa capability",
                             sizeof(*entry), plugin_id);
    entry->plugin_id = rxpa_catalogue_strdup(plugin_id);
    entry->capabilities = capabilities;
    entry->next = static_plugin_capabilities;
    static_plugin_capabilities = entry;
    RXPA_CATALOGUE_UNLOCK();
}

void rxvm_register_static_plugin_manifest_v2(
        const rxpa_plugin_manifest_v2 *manifest) {
    struct static_plugin_manifest_v2 *entry;
    const char *plugin_id;
    int valid;
    size_t plugin_id_minimum =
            offsetof(rxpa_plugin_manifest_v2, plugin_id) +
            sizeof(((rxpa_plugin_manifest_v2 *)0)->plugin_id);

    if (!manifest || manifest->struct_size < plugin_id_minimum ||
        !manifest->plugin_id || !*manifest->plugin_id) return;
    plugin_id = manifest->plugin_id;
    valid = validate_static_plugin_manifest_v2(manifest);
    RXPA_CATALOGUE_LOCK();
    entry = static_plugin_manifest_v2_locked(plugin_id);
    if (!entry) {
        entry = rxvm_memory_alloc_unowned_bytes(sizeof(*entry));
        if (!entry) RX_PANIC_OOM("allocate static rxpa V2 manifest",
                                 sizeof(*entry), plugin_id);
        memset(entry, 0, sizeof(*entry));
        entry->plugin_id = rxpa_catalogue_strdup(plugin_id);
        entry->next = static_plugin_manifests_v2;
        static_plugin_manifests_v2 = entry;
    }
    entry->valid = (unsigned char)valid;
    memset(&entry->manifest, 0, sizeof(entry->manifest));
    if (valid) {
        entry->manifest = *manifest;
        entry->manifest.plugin_id = entry->plugin_id;
    }
    RXPA_CATALOGUE_UNLOCK();
}

static void append_static_function(const char *plugin_id, rxpa_libfunc func,
                                   char *name, char *option, char *type,
                                   char *args) {
    struct static_linked_function *entry;
    RXPA_CATALOGUE_LOCK();
    entry = static_linked_functions;
    while (entry) {
        if (entry->func == func && strcmp(entry->name, name) == 0 &&
            rxpa_strings_equal(entry->plugin_id, plugin_id) &&
            rxpa_strings_equal(entry->option, option) &&
            rxpa_strings_equal(entry->type, type) &&
            rxpa_strings_equal(entry->args, args)) {
            RXPA_CATALOGUE_UNLOCK();
            return;
        }
        entry = entry->next;
    }
    entry = rxvm_memory_alloc_unowned_bytes(sizeof(*entry));
    if (!entry) RX_PANIC_OOM("allocate static rxpa function",
                             sizeof(*entry), name);
    entry->name = rxpa_catalogue_strdup(name);
    entry->plugin_id = rxpa_catalogue_strdup(plugin_id);
    entry->option = rxpa_catalogue_strdup(option);
    entry->type = rxpa_catalogue_strdup(type);
    entry->args = rxpa_catalogue_strdup(args);
    entry->func = func;
    entry->next = static_linked_functions;
    static_linked_functions = entry;
    RXPA_CATALOGUE_UNLOCK();
}

static void rxvm_addfunc_owned(const char *plugin_id, rxpa_libfunc func,
                               char* name, char* option, char* type,
                               char* args) {
    if (current_rxpa_context) {
        rxvm_context *context = current_rxpa_context->rxvm_context;
        rxpa_session_instance *session_instance = 0;
        const char *effective_plugin_id = plugin_id;
        size_t procedure_offset;
        DEBUG("Loading Procedure %s from plugin %s\n", name, context->modules[context->num_modules - 1]->name);

        // Add the procedure to the module
        uint32_t capabilities = current_rxpa_context->plugin_capabilities;
        if (current_rxpa_context->dynamic_plugin) {
            effective_plugin_id =
                    current_rxpa_context->dynamic_plugin->plugin_id;
            capabilities = rxpa_loaded_plugin_procedure_capabilities(
                    current_rxpa_context->dynamic_plugin, name);
            if ((capabilities & RXPA_PROCEDURE_CAP_SESSION_AFFINE) != 0u) {
                session_instance = rxpa_context_find_session(
                        current_rxpa_context,
                        current_rxpa_context->dynamic_plugin
                                ->manifest_v2.plugin_id);
            }
        } else if (plugin_id) {
            struct static_plugin_manifest_v2 *manifest_entry;
            RXPA_CATALOGUE_LOCK();
            manifest_entry = static_plugin_manifest_v2_locked(plugin_id);
            if (manifest_entry) {
                capabilities = manifest_entry->valid
                        ? sanitize_manifest_procedure_capabilities(
                                &manifest_entry->manifest, name) : 0u;
            } else {
                capabilities = static_plugin_capabilities_locked(plugin_id);
            }
            RXPA_CATALOGUE_UNLOCK();
            if ((capabilities & RXPA_PROCEDURE_CAP_SESSION_AFFINE) != 0u) {
                session_instance = rxpa_context_find_session(
                        current_rxpa_context, plugin_id);
            }
        }
        procedure_offset = add_proc_to_module(current_rxpa_context, name, func,
                                              capabilities, session_instance);
        add_function_meta_to_module(current_rxpa_context, name, option, type,
                                    args, procedure_offset);
        add_provider_meta_to_module(current_rxpa_context, name,
                                    effective_plugin_id);
    }
    else {
        append_static_function(plugin_id, func, name, option, type, args);
    }
}

// RXPA Add Function Implementation
// This is the callback function for rxldmod() when the plugin adds functions,
// or is called during initialising a statically linked plugin.
void rxvm_addfunc(rxpa_libfunc func, char* name, char* option, char* type,
                  char* args) {
    rxvm_addfunc_owned(0, func, name, option, type, args);
}

void rxvm_addfunc_for_plugin(const char *plugin_id, rxpa_libfunc func,
                             char* name, char* option, char* type,
                             char* args) {
    rxvm_addfunc_owned(plugin_id, func, name, option, type, args);
}

static void append_static_metadata(char *kind, char *symbol, char *option, char *type,
                                   char *interface_symbol, char *owner, char *member_kind,
                                   char *member, char *args) {
    struct static_linked_metadata *entry;
    struct static_linked_metadata *new_meta;
    RXPA_CATALOGUE_LOCK();
    entry = static_linked_metadata;
    while (entry) {
        if (rxpa_strings_equal(entry->kind, kind) &&
            rxpa_strings_equal(entry->symbol, symbol) &&
            rxpa_strings_equal(entry->option, option) &&
            rxpa_strings_equal(entry->type, type) &&
            rxpa_strings_equal(entry->interface_symbol, interface_symbol) &&
            rxpa_strings_equal(entry->owner, owner) &&
            rxpa_strings_equal(entry->member_kind, member_kind) &&
            rxpa_strings_equal(entry->member, member) &&
            rxpa_strings_equal(entry->args, args)) {
            RXPA_CATALOGUE_UNLOCK();
            return;
        }
        entry = entry->next;
    }
    new_meta = rxvm_memory_alloc_unowned_bytes(sizeof(*new_meta));
    if (!new_meta) {
        RX_PANIC_OOM("allocate static rxpa metadata",
                     sizeof(struct static_linked_metadata), symbol);
    }

    new_meta->kind = rxpa_catalogue_strdup(kind);
    new_meta->symbol = rxpa_catalogue_strdup(symbol);
    new_meta->option = rxpa_catalogue_strdup(option);
    new_meta->type = rxpa_catalogue_strdup(type);
    new_meta->interface_symbol = rxpa_catalogue_strdup(interface_symbol);
    new_meta->owner = rxpa_catalogue_strdup(owner);
    new_meta->member_kind = rxpa_catalogue_strdup(member_kind);
    new_meta->member = rxpa_catalogue_strdup(member);
    new_meta->args = rxpa_catalogue_strdup(args);
    new_meta->next = static_linked_metadata;
    static_linked_metadata = new_meta;
    RXPA_CATALOGUE_UNLOCK();
}

void rxvm_addclass(char* name, char* option, char* type) {
    if (current_rxpa_context) {
        add_class_meta_to_module(current_rxpa_context, name, option, type);
    }
    else {
        append_static_metadata("class", name, option, type, 0, 0, 0, 0, 0);
    }
}

void rxvm_addinterface(char* name, char* option, char* type) {
    if (current_rxpa_context) {
        add_interface_meta_to_module(current_rxpa_context, name, option, type);
    }
    else {
        append_static_metadata("interface", name, option, type, 0, 0, 0, 0, 0);
    }
}

void rxvm_addimplements(char* name, char* interface_name) {
    if (current_rxpa_context) {
        add_implements_meta_to_module(current_rxpa_context, name, interface_name);
    }
    else {
        append_static_metadata("implements", name, 0, 0, interface_name, 0, 0, 0, 0);
    }
}

void rxvm_addmember(char* owner, char* kind, char* member, char* type, char* args) {
    if (current_rxpa_context) {
        add_member_meta_to_module(current_rxpa_context, owner, kind, member, type, args);
    }
    else {
        append_static_metadata("member", 0, 0, type, 0, owner, kind, member, args);
    }
}

/* Loads statically loaded plugins
 * returns -1  - Error
 *         >=0 - Last Module Number loaded (1 based) (more than one (or none) might have been loaded ...)  */
int rxldmodp(rxvm_context *context) {
    size_t n;
    size_t function_count = 0u;
    size_t metadata_count = 0u;
    size_t function_index = 0u;
    size_t metadata_index = 0u;
    static_function_snapshot *functions = 0;
    static_metadata_snapshot *metadata = 0;
    struct static_linked_function *static_func;
    struct static_linked_metadata *static_meta;
    rxvm_context *previous_active_context;

    RXPA_CATALOGUE_LOCK();
    if (static_linked_functions == 0 && static_linked_metadata == 0) {
        RXPA_CATALOGUE_UNLOCK();
        return 0;
    }

    /* Snapshot the immutable call catalogue and current capability manifests.
     * Plugin callbacks (session factories and procedure queries) must never run
     * under the process catalogue lock. */
    for (static_func = static_linked_functions; static_func;
         static_func = static_func->next) function_count++;
    for (static_meta = static_linked_metadata; static_meta;
         static_meta = static_meta->next) metadata_count++;
    if (function_count) {
        functions = rxvm_load_memory_alloc(
                context->worker.memory_worker,
                function_count * sizeof(*functions));
        if (!functions) RX_PANIC_OOM("snapshot static rxpa functions",
                                     function_count * sizeof(*functions), 0);
    }
    if (metadata_count) {
        metadata = rxvm_load_memory_alloc(
                context->worker.memory_worker,
                metadata_count * sizeof(*metadata));
        if (!metadata) RX_PANIC_OOM("snapshot static rxpa metadata",
                                    metadata_count * sizeof(*metadata), 0);
    }
    for (static_func = static_linked_functions; static_func;
         static_func = static_func->next) {
        struct static_plugin_manifest_v2 *manifest_entry =
                static_plugin_manifest_v2_locked(static_func->plugin_id);
        functions[function_index].name = static_func->name;
        functions[function_index].plugin_id = static_func->plugin_id;
        functions[function_index].option = static_func->option;
        functions[function_index].type = static_func->type;
        functions[function_index].args = static_func->args;
        functions[function_index].func = static_func->func;
        functions[function_index].v1_capabilities =
                static_plugin_capabilities_locked(static_func->plugin_id);
        functions[function_index].has_manifest_v2 =
                manifest_entry != 0;
        functions[function_index].valid_manifest_v2 =
                manifest_entry && manifest_entry->valid;
        memset(&functions[function_index].manifest_v2, 0,
               sizeof(functions[function_index].manifest_v2));
        if (manifest_entry && manifest_entry->valid) {
            functions[function_index].manifest_v2 = manifest_entry->manifest;
        }
        function_index++;
    }
    for (static_meta = static_linked_metadata; static_meta;
         static_meta = static_meta->next) {
        metadata[metadata_index].kind = static_meta->kind;
        metadata[metadata_index].symbol = static_meta->symbol;
        metadata[metadata_index].option = static_meta->option;
        metadata[metadata_index].type = static_meta->type;
        metadata[metadata_index].interface_symbol =
                static_meta->interface_symbol;
        metadata[metadata_index].owner = static_meta->owner;
        metadata[metadata_index].member_kind = static_meta->member_kind;
        metadata[metadata_index].member = static_meta->member;
        metadata[metadata_index].args = static_meta->args;
        metadata_index++;
    }
    RXPA_CATALOGUE_UNLOCK();

    DEBUG("Loading Statically linked Plugins\n");

    // Create the rxpa_initctxptr context
    struct rxpa_initctxptr rxpa_functions;
    rxpa_functions.addfunc = rxvm_addfunc;
    rxpa_functions.addclass = rxvm_addclass;
    rxpa_functions.addinterface = rxvm_addinterface;
    rxpa_functions.addimplements = rxvm_addimplements;
    rxpa_functions.addmember = rxvm_addmember;
    rxpa_functions.getstring = rxvm_getstring;
    rxpa_functions.setstring = rxvm_setstring;
    rxpa_functions.setint = rxvm_setint;
    rxpa_functions.getint = rxvm_getint;
    rxpa_functions.setfloat = rxvm_setfloat;
    rxpa_functions.getfloat = rxvm_getfloat;
    rxpa_functions.setnativepayload = rxvm_setnativepayload;
    rxpa_functions.getnativepayload = rxvm_getnativepayload;
    rxpa_functions.getnumattrs = rxvm_getnumattrs;
    rxpa_functions.setnumattrs = rxvm_setnumattrs;
    rxpa_functions.getattr = rxvm_getattr;
    rxpa_functions.insertattr = rxvm_insertattr;
    rxpa_functions.removeattr = rxvm_removeattr;
    rxpa_functions.swapattrs = rxvm_swapattrs;

    // Exit Function Management
    rxpa_functions.setsayexit = rxvm_setsayexit;
    rxpa_functions.resetsayexit = rxvm_resetsayexit;
    rxpa_functions.isinitialized = rxvm_isinitialized;

    // Create rxpa_context and module
    char* dummy_file_name = "statically_linked_plugins";
    previous_active_context = rxvm_active_context_enter(context);
    current_rxpa_context = rxpa_context_f(context);
    current_rxpa_context->plugin_being_loaded = malloc(sizeof(module_file));
    init_module(current_rxpa_context->plugin_being_loaded);
    current_rxpa_context->plugin_being_loaded->header.name_size = strlen(dummy_file_name) + 1;
    current_rxpa_context->plugin_being_loaded->name = malloc(
            current_rxpa_context->plugin_being_loaded->header.name_size);
    strcpy(current_rxpa_context->plugin_being_loaded->name, dummy_file_name);
    current_rxpa_context->plugin_being_loaded->fromfile = 0;
    current_rxpa_context->plugin_being_loaded->native = 1;
    current_rxpa_context->plugin_being_loaded->header.expose_head = -1;
    current_rxpa_context->plugin_being_loaded->header.proc_head = -1;
    current_rxpa_context->plugin_being_loaded->header.meta_head = -1;

    /* Create exactly one session per plugin represented in this VM snapshot. */
    for (function_index = 0u; function_index < function_count;
         function_index++) {
        if (functions[function_index].valid_manifest_v2 &&
            functions[function_index].manifest_v2.session_create &&
            rxpa_context_create_session(
                    current_rxpa_context,
                    &functions[function_index].manifest_v2) != 0) {
            rxvm_load_memory_free(functions);
            rxvm_load_memory_free(metadata);
            free_rxpa_context(current_rxpa_context);
            current_rxpa_context = 0;
            rxvm_active_context_leave(previous_active_context);
            return -1;
        }
    }

    for (function_index = 0u; function_index < function_count;
         function_index++) {
        uint32_t capabilities;
        rxpa_session_instance *session_instance = 0;
        if (functions[function_index].has_manifest_v2) {
            capabilities = functions[function_index].valid_manifest_v2
                    ? sanitize_manifest_procedure_capabilities(
                            &functions[function_index].manifest_v2,
                            functions[function_index].name) : 0u;
        } else {
            capabilities = functions[function_index].v1_capabilities;
        }
        if ((capabilities & RXPA_PROCEDURE_CAP_SESSION_AFFINE) != 0u) {
            session_instance = rxpa_context_find_session(
                    current_rxpa_context,
                    functions[function_index].plugin_id);
            if (!session_instance) capabilities = 0u;
        }
        DEBUG("CREXX Statically Linked Native Plugin %s\n",
              functions[function_index].name);
        {
            size_t procedure_offset = add_proc_to_module(
                    current_rxpa_context,
                    (char *)functions[function_index].name,
                    functions[function_index].func, capabilities,
                    session_instance);
            add_function_meta_to_module(
                    current_rxpa_context, functions[function_index].name,
                    functions[function_index].option,
                    functions[function_index].type,
                    functions[function_index].args, procedure_offset);
            add_provider_meta_to_module(
                    current_rxpa_context, functions[function_index].name,
                    functions[function_index].plugin_id);
        }
    }

    for (metadata_index = 0u; metadata_index < metadata_count;
         metadata_index++) {
        if (strcmp(metadata[metadata_index].kind, "class") == 0) {
            rxvm_addclass((char *)metadata[metadata_index].symbol,
                          (char *)metadata[metadata_index].option,
                          (char *)metadata[metadata_index].type);
        }
        else if (strcmp(metadata[metadata_index].kind, "interface") == 0) {
            rxvm_addinterface((char *)metadata[metadata_index].symbol,
                              (char *)metadata[metadata_index].option,
                              (char *)metadata[metadata_index].type);
        }
        else if (strcmp(metadata[metadata_index].kind, "implements") == 0) {
            rxvm_addimplements((char *)metadata[metadata_index].symbol,
                               (char *)metadata[metadata_index]
                                       .interface_symbol);
        }
        else if (strcmp(metadata[metadata_index].kind, "member") == 0) {
            rxvm_addmember((char *)metadata[metadata_index].owner,
                           (char *)metadata[metadata_index].member_kind,
                           (char *)metadata[metadata_index].member,
                           (char *)metadata[metadata_index].type,
                           (char *)metadata[metadata_index].args);
        }
    }
    rxvm_load_memory_free(functions);
    rxvm_load_memory_free(metadata);

    // Link as a plugin
    n = rxvm_materialize_module_overlay(
            context, current_rxpa_context->plugin_being_loaded);
    apply_rxpa_proc_policies(current_rxpa_context, n);
    rxpa_context_publish_sessions(current_rxpa_context);

    // Free the rxpa_context
    current_rxpa_context->plugin_being_loaded = 0; // It's now owned by the module list
    free_rxpa_context(current_rxpa_context);
    current_rxpa_context = 0;
    rxvm_active_context_leave(previous_active_context);

    return (int)(n); /* Module Number */
}
