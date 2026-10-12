/*
 * cREXX License (MIT)
 * Copyright (c) 2026 the cREXX contributors
 *
 * Temporary registry lookup indexes, based on Mike Beer's startup patch.
 * This is lookup machinery only; signature and dispatch policy remain in RXVM.
 */

#include "rxvmlinkindex.h"

#include <ctype.h>
#include <stdint.h>
#include <string.h>

typedef struct rxvm_link_function {
    module *mod;
    meta_func_constant *meta;
    proc_runtime *procedure;
    const char *name;
    size_t name_length;
} rxvm_link_function;

typedef struct rxvm_link_member {
    module *mod;
    meta_entry *meta;
    size_t next;
} rxvm_link_member;

typedef struct rxvm_link_owner {
    const char *name;
    size_t name_length;
    size_t first;
    size_t last;
} rxvm_link_owner;

typedef struct rxvm_link_implements {
    string_constant *class_name;
    string_constant *interface_name;
} rxvm_link_implements;

struct rxvm_link_index {
    rxvm_context *context;
    rxvm_link_function *functions;
    size_t num_functions;
    size_t *name_slots;
    size_t *procedure_slots;
    size_t function_capacity;
    rxvm_link_member *members;
    size_t num_members;
    rxvm_link_owner *owners;
    size_t owner_capacity;
    rxvm_link_implements *implements;
    size_t num_implements;
    size_t *implements_slots;
    size_t implements_capacity;
};

static string_constant *rxvm_link_string(module *mod, size_t offset) {
    string_constant *entry;

    if (offset >= mod->segment.const_size) return 0;
    entry = (string_constant *)(mod->segment.const_pool + offset);
    return entry->base.type == STRING_CONST ? entry : 0;
}

static int rxvm_link_name_equals(const char *left, size_t left_length,
                                 const char *right, size_t right_length,
                                 int fold_case) {
    size_t i;

    if (!left || !right || left_length != right_length) return 0;
    if (!fold_case) return memcmp(left, right, left_length) == 0;
    for (i = 0; i < left_length; i++) {
        if (tolower((unsigned char)left[i]) != tolower((unsigned char)right[i])) return 0;
    }
    return 1;
}

static size_t rxvm_link_name_hash(const char *name, size_t length, int fold_case) {
    size_t hash = (size_t)UINT64_C(1469598103934665603);
    size_t i;

    for (i = 0; i < length; i++) {
        unsigned char ch = (unsigned char)name[i];
        if (fold_case) ch = (unsigned char)tolower(ch);
        hash ^= ch;
        hash *= (size_t)UINT64_C(1099511628211);
    }
    return hash;
}

static size_t rxvm_link_procedure_hash(const proc_runtime *procedure) {
    size_t hash = (size_t)(uintptr_t)procedure;

    hash ^= hash >> 17;
    hash *= (size_t)UINT64_C(0x9E3779B97F4A7C15);
    hash ^= hash >> 29;
    return hash;
}

static size_t rxvm_link_implements_hash(const char *class_name, size_t class_length,
                                       const char *interface_name, size_t interface_length) {
    return rxvm_link_name_hash(class_name, class_length, 0) * 31u +
           rxvm_link_name_hash(interface_name, interface_length, 0);
}

/* Keep at least half the slots empty, including for a missing-key search. */
static int rxvm_link_capacity(size_t count, size_t element_size, size_t *capacity) {
    size_t wanted;
    size_t result = 16u;

    if (count > (SIZE_MAX - 2u) / 2u) return 0;
    wanted = count * 2u + 2u;
    while (result < wanted) {
        if (result > SIZE_MAX / 2u) return 0;
        result *= 2u;
    }
    if (result > SIZE_MAX / element_size) return 0;
    *capacity = result;
    return 1;
}

void rxvm_link_index_free(rxvm_link_index *index) {
    rxvm_memory_worker *previous;

    if (!index) return;
    /* Release requires the owning worker even when a nested context is active. */
    previous = rxvm_memory_enter(index->context->worker.memory_worker);
    (void)rxvm_memory_release(index->functions);
    (void)rxvm_memory_release(index->name_slots);
    (void)rxvm_memory_release(index->procedure_slots);
    (void)rxvm_memory_release(index->members);
    (void)rxvm_memory_release(index->owners);
    (void)rxvm_memory_release(index->implements);
    (void)rxvm_memory_release(index->implements_slots);
    (void)rxvm_memory_release(index);
    rxvm_memory_leave(previous);
}

rxvm_link_index *rxvm_link_index_build(rxvm_context *context) {
    rxvm_link_index *index;
    rxvm_memory_worker *worker;
    size_t num_functions = 0;
    size_t num_members = 0;
    size_t num_implements = 0;
    size_t function_capacity;
    size_t owner_capacity;
    size_t implements_capacity;
    size_t mod_index;
    size_t i;

    if (!context) return 0;
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod = context->modules[mod_index];
        int meta_index = mod->meta_head;

        while (meta_index != -1) {
            meta_entry *meta = (meta_entry *)(mod->segment.const_pool + meta_index);
            size_t *count = 0;

            if (meta->base.type == META_FUNC) count = &num_functions;
            else if (meta->base.type == META_MEMBER) count = &num_members;
            else if (meta->base.type == META_IMPLEMENTS) count = &num_implements;
            if (count) {
                if (*count == SIZE_MAX) return 0;
                (*count)++;
            }
            meta_index = meta->next;
        }
    }
    if (!rxvm_link_capacity(num_functions, sizeof(size_t), &function_capacity) ||
        !rxvm_link_capacity(num_members, sizeof(rxvm_link_owner), &owner_capacity) ||
        !rxvm_link_capacity(num_implements, sizeof(size_t), &implements_capacity)) return 0;

    worker = context->worker.memory_worker;
    index = rxvm_memory_calloc_bytes(worker, 1, sizeof(*index));
    if (!index) return 0;
    index->context = context;
    index->function_capacity = function_capacity;
    index->owner_capacity = owner_capacity;
    index->implements_capacity = implements_capacity;
    index->functions = rxvm_memory_calloc_bytes(worker, num_functions ? num_functions : 1u,
                                                sizeof(*index->functions));
    index->name_slots = rxvm_memory_calloc_bytes(worker, function_capacity, sizeof(size_t));
    index->procedure_slots = rxvm_memory_calloc_bytes(worker, function_capacity, sizeof(size_t));
    index->members = rxvm_memory_calloc_bytes(worker, num_members ? num_members : 1u,
                                              sizeof(*index->members));
    index->owners = rxvm_memory_calloc_bytes(worker, owner_capacity, sizeof(*index->owners));
    index->implements = rxvm_memory_calloc_bytes(worker, num_implements ? num_implements : 1u,
                                                 sizeof(*index->implements));
    index->implements_slots = rxvm_memory_calloc_bytes(worker, implements_capacity, sizeof(size_t));
    if (!index->functions || !index->name_slots || !index->procedure_slots ||
        !index->members || !index->owners || !index->implements || !index->implements_slots) {
        rxvm_link_index_free(index);
        return 0;
    }
    for (i = 0; i < function_capacity; i++) {
        index->name_slots[i] = SIZE_MAX;
        index->procedure_slots[i] = SIZE_MAX;
    }
    for (i = 0; i < implements_capacity; i++) index->implements_slots[i] = SIZE_MAX;

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod = context->modules[mod_index];
        int meta_index = mod->meta_head;

        while (meta_index != -1) {
            meta_entry *meta = (meta_entry *)(mod->segment.const_pool + meta_index);

            if (meta->base.type == META_FUNC) {
                meta_func_constant *func = (meta_func_constant *)meta;
                string_constant *name = rxvm_link_string(mod, func->symbol);
                rxvm_link_function *entry = &index->functions[index->num_functions];
                size_t slot;

                entry->mod = mod;
                entry->meta = func;
                entry->procedure = rxvm_get_module_runtime_procedure(mod, func->func);
                if (name) {
                    entry->name = name->string;
                    entry->name_length = name->string_len;
                    slot = rxvm_link_name_hash(entry->name, entry->name_length, 1) & (function_capacity - 1u);
                    while (index->name_slots[slot] != SIZE_MAX) {
                        rxvm_link_function *prior = &index->functions[index->name_slots[slot]];
                        if (rxvm_link_name_equals(prior->name, prior->name_length,
                                                  entry->name, entry->name_length, 1)) break;
                        slot = (slot + 1u) & (function_capacity - 1u);
                    }
                    if (index->name_slots[slot] == SIZE_MAX) index->name_slots[slot] = index->num_functions;
                }
                if (entry->procedure) {
                    slot = rxvm_link_procedure_hash(entry->procedure) & (function_capacity - 1u);
                    while (index->procedure_slots[slot] != SIZE_MAX &&
                           index->functions[index->procedure_slots[slot]].procedure != entry->procedure) {
                        slot = (slot + 1u) & (function_capacity - 1u);
                    }
                    if (index->procedure_slots[slot] == SIZE_MAX) index->procedure_slots[slot] = index->num_functions;
                }
                index->num_functions++;
            } else if (meta->base.type == META_MEMBER) {
                meta_member_constant *member = (meta_member_constant *)meta;
                string_constant *owner = rxvm_link_string(mod, member->owner);

                if (owner) {
                    size_t slot = rxvm_link_name_hash(owner->string, owner->string_len, 0) & (owner_capacity - 1u);
                    rxvm_link_owner *entry;
                    size_t member_index = index->num_members;

                    while (index->owners[slot].name &&
                           !rxvm_link_name_equals(index->owners[slot].name, index->owners[slot].name_length,
                                                  owner->string, owner->string_len, 0)) {
                        slot = (slot + 1u) & (owner_capacity - 1u);
                    }
                    entry = &index->owners[slot];
                    if (!entry->name) {
                        entry->name = owner->string;
                        entry->name_length = owner->string_len;
                        entry->first = SIZE_MAX;
                        entry->last = SIZE_MAX;
                    }
                    index->members[member_index].mod = mod;
                    index->members[member_index].meta = meta;
                    index->members[member_index].next = SIZE_MAX;
                    if (entry->last != SIZE_MAX) index->members[entry->last].next = member_index;
                    else entry->first = member_index;
                    entry->last = member_index;
                    index->num_members++;
                }
            } else if (meta->base.type == META_IMPLEMENTS) {
                meta_implements_constant *implements = (meta_implements_constant *)meta;
                string_constant *class_name = rxvm_link_string(mod, implements->symbol);
                string_constant *interface_name = rxvm_link_string(mod, implements->interface_symbol);

                if (class_name && interface_name) {
                    size_t slot = rxvm_link_implements_hash(class_name->string, class_name->string_len,
                                                           interface_name->string, interface_name->string_len) &
                                  (implements_capacity - 1u);
                    size_t implements_index = index->num_implements++;

                    index->implements[implements_index].class_name = class_name;
                    index->implements[implements_index].interface_name = interface_name;
                    while (index->implements_slots[slot] != SIZE_MAX) slot = (slot + 1u) & (implements_capacity - 1u);
                    index->implements_slots[slot] = implements_index;
                }
            }
            meta_index = meta->next;
        }
    }
    return index;
}

proc_runtime *rxvm_link_index_resolve_procedure(rxvm_context *context,
                                               const rxvm_link_index *index,
                                               const char *name,
                                               size_t name_length) {
    size_t mod_index;

    if (!context || !name) return 0;
    if (index && index->context == context) {
        size_t slot = rxvm_link_name_hash(name, name_length, 1) & (index->function_capacity - 1u);

        while (index->name_slots[slot] != SIZE_MAX) {
            const rxvm_link_function *entry = &index->functions[index->name_slots[slot]];
            if (rxvm_link_name_equals(entry->name, entry->name_length, name, name_length, 1)) return entry->procedure;
            slot = (slot + 1u) & (index->function_capacity - 1u);
        }
        return 0;
    }
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod = context->modules[mod_index];
        int meta_index = mod->meta_head;

        while (meta_index != -1) {
            meta_entry *meta = (meta_entry *)(mod->segment.const_pool + meta_index);
            if (meta->base.type == META_FUNC) {
                meta_func_constant *func = (meta_func_constant *)meta;
                string_constant *symbol = rxvm_link_string(mod, func->symbol);
                if (symbol && rxvm_link_name_equals(symbol->string, symbol->string_len, name, name_length, 1)) {
                    return rxvm_get_module_runtime_procedure(mod, func->func);
                }
            }
            meta_index = meta->next;
        }
    }
    return 0;
}

meta_func_constant *rxvm_link_index_find_function(rxvm_context *context,
                                                 const rxvm_link_index *index,
                                                 proc_runtime *procedure,
                                                 module **function_module) {
    size_t mod_index;

    if (!context || !procedure || !function_module) return 0;
    if (index && index->context == context) {
        size_t slot = rxvm_link_procedure_hash(procedure) & (index->function_capacity - 1u);

        while (index->procedure_slots[slot] != SIZE_MAX) {
            const rxvm_link_function *entry = &index->functions[index->procedure_slots[slot]];
            if (entry->procedure == procedure) {
                *function_module = entry->mod;
                return entry->meta;
            }
            slot = (slot + 1u) & (index->function_capacity - 1u);
        }
        return 0;
    }
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod = context->modules[mod_index];
        int meta_index = mod->meta_head;

        while (meta_index != -1) {
            meta_entry *meta = (meta_entry *)(mod->segment.const_pool + meta_index);
            if (meta->base.type == META_FUNC) {
                meta_func_constant *func = (meta_func_constant *)meta;
                if (rxvm_get_module_runtime_procedure(mod, func->func) == procedure) {
                    *function_module = mod;
                    return func;
                }
            }
            meta_index = meta->next;
        }
    }
    return 0;
}

int rxvm_link_index_class_implements(rxvm_context *context,
                                     const rxvm_link_index *index,
                                     const char *class_name,
                                     size_t class_name_length,
                                     const char *interface_name,
                                     size_t interface_name_length) {
    size_t mod_index;

    if (!context || !class_name || !interface_name) return 0;
    if (index && index->context == context) {
        size_t slot = rxvm_link_implements_hash(class_name, class_name_length,
                                               interface_name, interface_name_length) &
                      (index->implements_capacity - 1u);

        while (index->implements_slots[slot] != SIZE_MAX) {
            const rxvm_link_implements *entry = &index->implements[index->implements_slots[slot]];
            if (rxvm_link_name_equals(entry->class_name->string, entry->class_name->string_len,
                                       class_name, class_name_length, 0) &&
                rxvm_link_name_equals(entry->interface_name->string, entry->interface_name->string_len,
                                       interface_name, interface_name_length, 0)) return 1;
            slot = (slot + 1u) & (index->implements_capacity - 1u);
        }
        return 0;
    }
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod = context->modules[mod_index];
        int meta_index = mod->meta_head;

        while (meta_index != -1) {
            meta_entry *meta = (meta_entry *)(mod->segment.const_pool + meta_index);
            if (meta->base.type == META_IMPLEMENTS) {
                meta_implements_constant *implements = (meta_implements_constant *)meta;
                string_constant *class_symbol = rxvm_link_string(mod, implements->symbol);
                string_constant *interface_symbol = rxvm_link_string(mod, implements->interface_symbol);
                if (class_symbol && interface_symbol &&
                    rxvm_link_name_equals(class_symbol->string, class_symbol->string_len,
                                           class_name, class_name_length, 0) &&
                    rxvm_link_name_equals(interface_symbol->string, interface_symbol->string_len,
                                           interface_name, interface_name_length, 0)) return 1;
            }
            meta_index = meta->next;
        }
    }
    return 0;
}

void rxvm_link_member_cursor_init(rxvm_link_member_cursor *cursor,
                                  rxvm_context *context,
                                  const rxvm_link_index *index,
                                  const char *owner,
                                  size_t owner_length) {
    memset(cursor, 0, sizeof(*cursor));
    cursor->context = context;
    cursor->index = index && index->context == context ? index : 0;
    cursor->owner = owner;
    cursor->owner_length = owner_length;
    cursor->meta_index = context->num_modules ? context->modules[0]->meta_head : -1;
    cursor->member_index = SIZE_MAX;
    if (cursor->index) {
        size_t slot = rxvm_link_name_hash(owner, owner_length, 0) & (index->owner_capacity - 1u);

        while (index->owners[slot].name) {
            if (rxvm_link_name_equals(index->owners[slot].name, index->owners[slot].name_length,
                                       owner, owner_length, 0)) {
                cursor->member_index = index->owners[slot].first;
                break;
            }
            slot = (slot + 1u) & (index->owner_capacity - 1u);
        }
    }
}

meta_entry *rxvm_link_member_cursor_next(rxvm_link_member_cursor *cursor,
                                        module **member_module) {
    if (cursor->index) {
        const rxvm_link_member *entry;

        if (cursor->member_index == SIZE_MAX) return 0;
        entry = &cursor->index->members[cursor->member_index];
        cursor->member_index = entry->next;
        *member_module = entry->mod;
        return entry->meta;
    }
    while (cursor->module_index < cursor->context->num_modules) {
        module *mod = cursor->context->modules[cursor->module_index];
        meta_entry *meta;

        if (cursor->meta_index == -1) {
            cursor->module_index++;
            if (cursor->module_index < cursor->context->num_modules) {
                cursor->meta_index = cursor->context->modules[cursor->module_index]->meta_head;
            }
            continue;
        }
        meta = (meta_entry *)(mod->segment.const_pool + cursor->meta_index);
        cursor->meta_index = meta->next;
        if (meta->base.type == META_MEMBER) {
            meta_member_constant *member = (meta_member_constant *)meta;
            string_constant *owner = rxvm_link_string(mod, member->owner);
            if (owner && rxvm_link_name_equals(owner->string, owner->string_len,
                                               cursor->owner, cursor->owner_length, 0)) {
                *member_module = mod;
                return meta;
            }
        }
    }
    return 0;
}
