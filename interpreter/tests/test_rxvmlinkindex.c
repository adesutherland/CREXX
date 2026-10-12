/* cREXX License (MIT), Copyright (c) 2026 the cREXX contributors. */

#include "rxvmlinkindex.h"
#include "rxvm.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t allocation_calls;
static size_t fail_allocation;
static void *live_index_allocations[16];
static size_t live_index_count;
static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: line %d: %s\n", __LINE__, #condition); \
        failures++; \
    } \
} while (0)

static void *test_index_calloc(rxvm_memory_worker *worker, size_t count, size_t size) {
    void *result;

    allocation_calls++;
    if (allocation_calls == fail_allocation) return 0;
    result = rxvm_memory_calloc_bytes(worker, count, size);
    if (result) {
        CHECK(live_index_count < sizeof(live_index_allocations) / sizeof(*live_index_allocations));
        if (live_index_count >= sizeof(live_index_allocations) / sizeof(*live_index_allocations)) abort();
        live_index_allocations[live_index_count++] = result;
        CHECK(rxvm_memory_owner(result) == worker);
    }
    return result;
}

static rxvm_memory_result test_index_release(void *pointer) {
    size_t i;
    rxvm_memory_result result;

    if (pointer) {
        for (i = 0; i < live_index_count; i++) {
            if (live_index_allocations[i] == pointer) break;
        }
        CHECK(i < live_index_count);
    }
    result = rxvm_memory_release(pointer);
    CHECK(result == RXVM_MEMORY_OK);
    if (pointer && result == RXVM_MEMORY_OK && i < live_index_count) {
        live_index_allocations[i] = live_index_allocations[--live_index_count];
    }
    return result;
}

/* Only this private helper's allocations are intercepted. The VM, registry
 * entries and signature parser keep their real allocation/failure contracts.
 * Defining its entry points here avoids adding a production fault-injection API.
 */
#define rxvm_memory_calloc_bytes test_index_calloc
#define rxvm_memory_release test_index_release
#include "../rxvmlinkindex.c"
#undef rxvm_memory_calloc_bytes
#undef rxvm_memory_release

typedef struct expected_method {
    char class_name[128];
    char descriptor[512];
    proc_runtime *procedure;
} expected_method;

typedef struct expected_factory {
    char interface_name[128];
    char factory_name[128];
    char class_name[128];
    char descriptor[512];
    proc_runtime *procedure;
    proc_runtime *match;
} expected_factory;

static void copy_expected(char *target, size_t capacity, const char *text) {
    CHECK(strlen(text) < capacity);
    if (strlen(text) >= capacity) abort();
    memcpy(target, text, strlen(text) + 1u);
}

static rxvm_context *load_context(const char *image) {
    rxvm_context *context = rxvm_create();

    CHECK(context != 0);
    if (!context) return 0;
    if (!rxvm_load_file(context, (char *)image) || rxvm_link(context) != 0) {
        CHECK(0);
        rxvm_destroy(context);
        return 0;
    }
    return context;
}

static void check_lookup_equivalence(rxvm_context *context, const rxvm_link_index *index) {
    size_t mod_index;

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod = context->modules[mod_index];
        int meta_index = mod->meta_head;

        while (meta_index != -1) {
            meta_entry *meta = (meta_entry *)(mod->segment.const_pool + meta_index);

            if (meta->base.type == META_FUNC) {
                meta_func_constant *func = (meta_func_constant *)meta;
                string_constant *name = rxvm_link_string(mod, func->symbol);
                proc_runtime *procedure = rxvm_get_module_runtime_procedure(mod, func->func);
                module *indexed_module = 0;
                module *linear_module = 0;
                char upper_name[256];
                size_t i;

                CHECK(name != 0);
                if (!name) break;
                CHECK(rxvm_link_index_resolve_procedure(context, index, name->string, name->string_len) ==
                      rxvm_link_index_resolve_procedure(context, 0, name->string, name->string_len));
                CHECK(name->string_len < sizeof(upper_name));
                if (name->string_len >= sizeof(upper_name)) abort();
                for (i = 0; i < name->string_len; i++) upper_name[i] = (char)toupper((unsigned char)name->string[i]);
                CHECK(rxvm_link_index_resolve_procedure(context, index, upper_name, name->string_len) ==
                      rxvm_link_index_resolve_procedure(context, 0, name->string, name->string_len));
                CHECK(rxvm_link_index_find_function(context, index, procedure, &indexed_module) ==
                      rxvm_link_index_find_function(context, 0, procedure, &linear_module));
                CHECK(indexed_module == linear_module);
            } else if (meta->base.type == META_IMPLEMENTS) {
                meta_implements_constant *implements = (meta_implements_constant *)meta;
                string_constant *class_name = rxvm_link_string(mod, implements->symbol);
                string_constant *interface_name = rxvm_link_string(mod, implements->interface_symbol);

                CHECK(class_name && interface_name);
                if (!class_name || !interface_name) break;
                CHECK(rxvm_link_index_class_implements(context, index,
                                                       class_name->string, class_name->string_len,
                                                       interface_name->string, interface_name->string_len));
            } else if (meta->base.type == META_MEMBER) {
                meta_member_constant *member = (meta_member_constant *)meta;
                string_constant *owner = rxvm_link_string(mod, member->owner);
                rxvm_link_member_cursor indexed;
                rxvm_link_member_cursor linear;
                meta_entry *indexed_meta;
                meta_entry *linear_meta;
                module *indexed_module = 0;
                module *linear_module = 0;

                CHECK(owner != 0);
                if (!owner) break;
                rxvm_link_member_cursor_init(&indexed, context, index, owner->string, owner->string_len);
                rxvm_link_member_cursor_init(&linear, context, 0, owner->string, owner->string_len);
                do {
                    indexed_meta = rxvm_link_member_cursor_next(&indexed, &indexed_module);
                    linear_meta = rxvm_link_member_cursor_next(&linear, &linear_module);
                    CHECK(indexed_meta == linear_meta);
                    if (indexed_meta) CHECK(indexed_module == linear_module);
                } while (indexed_meta && linear_meta);
            }
            meta_index = meta->next;
        }
    }
    CHECK(rxvm_link_index_resolve_procedure(context, index, "missing", 7) == 0);
    CHECK(!rxvm_link_index_class_implements(context, index,
                                            "INTERFACE_FACTORY_TIEBREAK.CAR", strlen("INTERFACE_FACTORY_TIEBREAK.CAR"),
                                            "interface_factory_tiebreak.vehicle", strlen("interface_factory_tiebreak.vehicle")));
}

static void check_allocation_failures(rxvm_context *context) {
    expected_method methods[16];
    expected_factory factories[16];
    rxvm_memory_stats before;
    rxvm_memory_stats after;
    rxvm_link_index *index;
    size_t count;
    size_t fail;
    size_t i;

    CHECK(context->num_interface_methods == 5);
    CHECK(context->num_interface_factories == 5);
    if (context->num_interface_methods != 5 || context->num_interface_factories != 5) return;
    for (i = 0; i < 5; i++) {
        rxvm_interface_method_entry *method = &context->interface_methods[i];
        rxvm_interface_factory_entry *factory = &context->interface_factories[i];

        copy_expected(methods[i].class_name, sizeof(methods[i].class_name), method->class_name);
        copy_expected(methods[i].descriptor, sizeof(methods[i].descriptor), method->descriptor);
        methods[i].procedure = method->method_proc;
        copy_expected(factories[i].interface_name, sizeof(factories[i].interface_name), factory->interface_name);
        copy_expected(factories[i].factory_name, sizeof(factories[i].factory_name), factory->factory_name);
        copy_expected(factories[i].class_name, sizeof(factories[i].class_name), factory->class_name);
        copy_expected(factories[i].descriptor, sizeof(factories[i].descriptor), factory->descriptor);
        factories[i].procedure = factory->factory_proc;
        factories[i].match = factory->match_proc;
    }
    allocation_calls = 0;
    index = rxvm_link_index_build(context);
    CHECK(index != 0);
    count = allocation_calls;
    CHECK(count > 1);
    rxvm_link_index_free(index);
    CHECK(live_index_count == 0);
    rxvm_memory_get_stats(rxvm_runtime_memory_context(context->worker.runtime), &before);

    for (fail = 1; fail <= count; fail++) {
        allocation_calls = 0;
        fail_allocation = fail;
        CHECK(rxvm_link_index_build(context) == 0);
        CHECK(live_index_count == 0);
        rxvm_memory_get_stats(rxvm_runtime_memory_context(context->worker.runtime), &after);
        CHECK(before.live_allocations == after.live_allocations);
        CHECK(before.wrong_owner_frees == after.wrong_owner_frees);
    }
    /* Both registries are dirtied exactly as the real link caller does. Fail
     * every allocation position in the method or factory index independently.
     */
    for (fail = 1; fail <= count * 2u; fail++) {
        allocation_calls = 0;
        fail_allocation = fail;
        context->interface_method_registry_dirty = 1;
        context->interface_factory_registry_dirty = 1;
        CHECK(rxvm_link(context) == 0);
        CHECK(!context->interface_method_registry_dirty && !context->interface_factory_registry_dirty);
        CHECK(live_index_count == 0);
        CHECK(context->num_interface_methods == 5 && context->num_interface_factories == 5);
        if (context->num_interface_methods != 5 || context->num_interface_factories != 5) break;
        for (i = 0; i < 5; i++) {
            rxvm_interface_method_entry *method = &context->interface_methods[i];
            rxvm_interface_factory_entry *factory = &context->interface_factories[i];

            CHECK(strcmp(methods[i].class_name, method->class_name) == 0);
            CHECK(strcmp(methods[i].descriptor, method->descriptor) == 0);
            CHECK(methods[i].procedure == method->method_proc);
            CHECK(strcmp(factories[i].interface_name, factory->interface_name) == 0);
            CHECK(strcmp(factories[i].factory_name, factory->factory_name) == 0);
            CHECK(strcmp(factories[i].class_name, factory->class_name) == 0);
            CHECK(strcmp(factories[i].descriptor, factory->descriptor) == 0);
            CHECK(factories[i].procedure == factory->factory_proc && factories[i].match == factory->match_proc);
        }
    }
    fail_allocation = 0;
}

int main(int argc, char **argv) {
    rxvm_context *outer;
    rxvm_context *inner;
    rxvm_link_index *outer_index;
    rxvm_link_index *inner_index;
    rxvm_memory_worker *previous;
    size_t capacity;

    if (argc != 2) return 2;
    outer = load_context(argv[1]);
    inner = load_context(argv[1]);
    if (!outer || !inner) return 1;
    previous = rxvm_memory_enter(inner->worker.memory_worker);
    check_allocation_failures(outer);
    CHECK(rxvm_memory_current_worker() == inner->worker.memory_worker);
    /* Explicit allocation ownership must work while another context is active. */
    outer_index = rxvm_link_index_build(outer);
    inner_index = rxvm_link_index_build(inner);
    CHECK(outer_index && inner_index);
    if (outer_index && inner_index) {
        check_lookup_equivalence(outer, outer_index);
        check_lookup_equivalence(inner, inner_index);
        check_lookup_equivalence(outer, inner_index);
        check_lookup_equivalence(inner, outer_index);
        CHECK(outer->interface_methods[0].method_proc != inner->interface_methods[0].method_proc);
    }
    rxvm_link_index_free(outer_index);
    CHECK(rxvm_memory_current_worker() == inner->worker.memory_worker);
    rxvm_destroy(outer);
    check_lookup_equivalence(inner, inner_index);
    rxvm_link_index_free(inner_index);
    CHECK(live_index_count == 0);
    rxvm_memory_leave(previous);
    rxvm_destroy(inner);
    CHECK(!rxvm_link_capacity(SIZE_MAX, sizeof(size_t), &capacity));
    CHECK(!rxvm_link_capacity(SIZE_MAX / 2u, sizeof(size_t), &capacity));
    CHECK(!rxvm_link_capacity(1, SIZE_MAX, &capacity));
    if (failures) return 1;
    puts("PASS: link index allocation fallback and context scope");
    return 0;
}
