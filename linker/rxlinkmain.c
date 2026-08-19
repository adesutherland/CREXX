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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "rxbin.h"
#include "rxsignature.h"

typedef struct string_list {
    char **items;
    size_t count;
    size_t capacity;
} string_list;

typedef struct link_module_info {
    module_file *module;
    char *input_path;
    size_t input_index;
    size_t member_index;
    char *selector_name;
    string_list exports;
    string_list imports;
    string_list defined_interfaces;
    string_list implemented_interfaces;
    string_list referenced_interfaces;
    string_list provided_methods;
    string_list referenced_methods;
    string_list unresolved_imports;
    int has_main;
    int selected;
    int queued;
    int omitted;
} link_module_info;

typedef struct module_list {
    link_module_info *items;
    size_t count;
    size_t capacity;
} module_list;

typedef struct link_config {
    string_list inputs;
    string_list roots;
    string_list includes;
    string_list omits;
    char *output_path;
    char *map_path;
    char *provider_requirements_path;
    char *location;
    int strip_source_metadata;
    int strip_inline_metadata;
    int debug_mode;
} link_config;

typedef struct provider_requirement {
    char *provider_id;
    char *symbol;
    char *type;
    char *args;
    char *module_name;
    uint32_t flags;
} provider_requirement;

typedef struct provider_requirement_list {
    provider_requirement *items;
    size_t count;
    size_t capacity;
} provider_requirement_list;

typedef struct const_map_entry {
    size_t old_offset;
    size_t new_offset;
} const_map_entry;

typedef struct rxlink_output_module {
    module_file *module;
    module_file *source_module;
    const_map_entry *maps;
    size_t map_count;
    size_t map_capacity;
    size_t *map_hash_slots;
    size_t map_hash_capacity;
} rxlink_output_module;

typedef struct rxlink_output_list {
    rxlink_output_module *items;
    size_t count;
    size_t capacity;
} rxlink_output_list;

typedef struct leaf_dedupe_entry {
    enum const_pool_type type;
    size_t size_in_pool;
    size_t output_offset;
    size_t hash;
} leaf_dedupe_entry;

typedef struct rxlink_build_context {
    rxbin_byte_buffer shared_pool;
    leaf_dedupe_entry *leaf_entries;
    size_t leaf_count;
    size_t leaf_capacity;
    size_t *leaf_hash_slots;
    size_t leaf_hash_capacity;
    int strip_source_metadata;
    int strip_inline_metadata;
} rxlink_build_context;

static void string_list_init(string_list *list) {
    list->items = 0;
    list->count = 0;
    list->capacity = 0;
}

static void string_list_free(string_list *list) {
    size_t i;
    if (!list) return;
    for (i = 0; i < list->count; i++) {
        free(list->items[i]);
    }
    free(list->items);
    list->items = 0;
    list->count = 0;
    list->capacity = 0;
}

static int string_list_contains(const string_list *list, const char *value) {
    size_t i;
    for (i = 0; i < list->count; i++) {
        if (strcmp(list->items[i], value) == 0) return 1;
    }
    return 0;
}

static int string_list_append(string_list *list, const char *value) {
    char **new_items;
    size_t new_capacity;
    char *copy;

    copy = strdup(value);
    if (!copy) {
        RX_REPORT_OOM("strdup rxlink string list entry", strlen(value) + 1, value);
        return 0;
    }

    if (list->count == list->capacity) {
        new_capacity = list->capacity ? list->capacity * 2 : 8;
        new_items = realloc(list->items, sizeof(char *) * new_capacity);
        if (!new_items) {
            RX_REPORT_OOM("realloc rxlink string list",
                          sizeof(char *) * new_capacity, value);
            free(copy);
            return 0;
        }
        list->items = new_items;
        list->capacity = new_capacity;
    }

    list->items[list->count++] = copy;
    return 1;
}

static int string_list_append_unique(string_list *list, const char *value) {
    if (string_list_contains(list, value)) return 1;
    return string_list_append(list, value);
}

static int rxlink_symbol_equals(const char *left, const char *right) {
    if (!left || !right) return 0;
    while (*left && *right) {
        if (tolower((unsigned char)*left) != tolower((unsigned char)*right)) return 0;
        left++;
        right++;
    }
    return *left == '\0' && *right == '\0';
}

static int module_list_append(module_list *list, link_module_info *item) {
    link_module_info *new_items;
    size_t new_capacity;

    if (list->count == list->capacity) {
        new_capacity = list->capacity ? list->capacity * 2 : 16;
        new_items = realloc(list->items, sizeof(link_module_info) * new_capacity);
        if (!new_items) {
            RX_REPORT_OOM("realloc rxlink module list",
                          sizeof(link_module_info) * new_capacity, 0);
            return 0;
        }
        list->items = new_items;
        list->capacity = new_capacity;
    }

    list->items[list->count++] = *item;
    return 1;
}

static const char *module_string_constant(module_file *module, size_t offset) {
    string_constant *value;

    if (!module || offset >= module->header.constant_size) return 0;
    value = (string_constant *)((unsigned char *)module->constant + offset);
    if (value->base.type != STRING_CONST) return 0;
    return value->string;
}

static void provider_requirement_list_free(provider_requirement_list *list) {
    size_t i;

    if (!list) return;
    for (i = 0u; i < list->count; i++) {
        free(list->items[i].provider_id);
        free(list->items[i].symbol);
        free(list->items[i].type);
        free(list->items[i].args);
        free(list->items[i].module_name);
    }
    free(list->items);
    memset(list, 0, sizeof(*list));
}

static const meta_func_constant *module_function_metadata(
        module_file *module, const char *symbol) {
    int offset;

    if (!module || !symbol) return 0;
    offset = module->header.meta_head;
    while (offset != -1) {
        meta_entry *entry =
            (meta_entry *)((unsigned char *)module->constant + (size_t)offset);
        if (entry->base.type == META_FUNC) {
            const meta_func_constant *function =
                (const meta_func_constant *)entry;
            const char *candidate =
                module_string_constant(module, function->symbol);
            if (candidate && rxlink_symbol_equals(candidate, symbol)) {
                return function;
            }
        }
        offset = entry->next;
    }
    return 0;
}

static int append_provider_requirement(provider_requirement_list *list,
                                       const provider_requirement *requirement) {
    provider_requirement *items;
    size_t capacity;
    provider_requirement copy;

    if (list->count == list->capacity) {
        capacity = list->capacity ? list->capacity * 2u : 8u;
        items = (provider_requirement *)realloc(
                list->items, capacity * sizeof(*items));
        if (!items) {
            RX_REPORT_OOM("grow rxlink provider requirements",
                          capacity * sizeof(*items), requirement->provider_id);
            return 0;
        }
        list->items = items;
        list->capacity = capacity;
    }
    memset(&copy, 0, sizeof(copy));
    copy.provider_id = strdup(requirement->provider_id);
    copy.symbol = strdup(requirement->symbol);
    copy.type = strdup(requirement->type);
    copy.args = strdup(requirement->args);
    copy.module_name = strdup(requirement->module_name);
    copy.flags = requirement->flags;
    if (!copy.provider_id || !copy.symbol || !copy.type || !copy.args ||
        !copy.module_name) {
        free(copy.provider_id);
        free(copy.symbol);
        free(copy.type);
        free(copy.args);
        free(copy.module_name);
        RX_REPORT_OOM("copy rxlink provider requirement",
                      RX_OOM_UNKNOWN_SIZE, requirement->provider_id);
        return 0;
    }
    list->items[list->count++] = copy;
    return 1;
}

static int rxlink_provider_id_valid(const char *provider_id) {
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

static int collect_provider_requirements(const module_list *modules,
                                         provider_requirement_list *requirements) {
    size_t module_index;

    for (module_index = 0u; module_index < modules->count; module_index++) {
        const link_module_info *info = &modules->items[module_index];
        module_file *module;
        int offset;

        if (!info->selected || info->omitted) continue;
        module = info->module;
        offset = module->header.meta_head;
        while (offset != -1) {
            meta_entry *entry =
                (meta_entry *)((unsigned char *)module->constant +
                               (size_t)offset);
            if (entry->base.type == META_PROVIDER) {
                const meta_provider_constant *provider =
                    (const meta_provider_constant *)entry;
                const char *symbol =
                    module_string_constant(module, provider->symbol);
                const char *provider_id =
                    module_string_constant(module, provider->provider);
                const meta_func_constant *function =
                    module_function_metadata(module, symbol);
                provider_requirement requirement;
                size_t prior;

                if (!symbol || !*symbol ||
                    !rxlink_provider_id_valid(provider_id) ||
                    !function) {
                    fprintf(stderr,
                            "ERROR: module %s has provider metadata without matching callable signature\n",
                            info->selector_name ? info->selector_name : "<unnamed>");
                    return 0;
                }
                requirement.provider_id = (char *)provider_id;
                requirement.symbol = (char *)symbol;
                requirement.type = (char *)module_string_constant(
                        module, function->type);
                requirement.args = (char *)module_string_constant(
                        module, function->args);
                requirement.module_name = info->selector_name
                        ? info->selector_name : "<unnamed>";
                requirement.flags = provider->flags;
                if (!requirement.type || !requirement.args) return 0;

                for (prior = 0u; prior < requirements->count; prior++) {
                    provider_requirement *existing =
                        &requirements->items[prior];
                    if (!rxlink_symbol_equals(existing->symbol, symbol)) continue;
                    if (strcmp(existing->provider_id, provider_id) != 0 ||
                        strcmp(existing->type, requirement.type) != 0 ||
                        strcmp(existing->args, requirement.args) != 0) {
                        fprintf(stderr,
                                "ERROR: native callable %s has incompatible provider requirements: %s in %s versus %s in %s\n",
                                symbol,
                                existing->provider_id,
                                existing->module_name,
                                provider_id,
                                requirement.module_name);
                        return 0;
                    }
                    if (strcmp(existing->module_name,
                               requirement.module_name) == 0) {
                        /* Duplicate metadata in one module is redundant, but
                         * required dominates optional independent of order. */
                        existing->flags |= requirement.flags;
                        goto next_metadata;
                    }
                }
                if (!append_provider_requirement(requirements, &requirement)) {
                    return 0;
                }
            }
next_metadata:
            offset = entry->next;
        }
    }
    return 1;
}

static char *module_instruction_string(module_file *module,
                                       int opcode,
                                       unsigned int operand_index,
                                       size_t value) {
    const char *text;

    if (module && module->graph_operands &&
        rx_graph_operand_kind(opcode, operand_index) != RX_GRAPH_OPERAND_NONE) {
        return rx_graph_operand_text(module->semantic_graph,
                                     opcode,
                                     operand_index,
                                     (uint32_t)value);
    }
    text = module_string_constant(module, value);
    return text ? strdup(text) : 0;
}

static int append_method_name_from_symbol(string_list *list, const char *symbol) {
    const char *last_dot;
    const char *method_name;

    if (!list || !symbol) return 1;
    last_dot = strrchr(symbol, '.');
    if (!last_dot || !last_dot[1]) return 1;
    method_name = last_dot + 1;
    if (strcmp(method_name, "\xC2\xA7" "factory") == 0) return 1;
    return string_list_append_unique(list, method_name);
}

static int rxlink_module_is_active(const link_module_info *module) {
    return module && module->selected && !module->omitted;
}

static int rxlink_kind_is_method(const char *kind) {
    return kind && strncmp(kind, "method", 6) == 0;
}

static int rxlink_kind_is_final_method(const char *kind) {
    return rxlink_kind_is_method(kind) && strstr(kind, "final") != 0;
}

static int rxlink_kind_is_factory(const char *kind) {
    return kind && strcmp(kind, "factory") == 0;
}

static char *rxlink_metadata_type_to_contract_name(const char *type_name) {
    size_t in_index;
    size_t out_index;
    size_t length;
    char *normalized;

    if (!type_name || !*type_name || type_name[0] != '.') return 0;

    length = strlen(type_name);
    normalized = malloc(length + 1);
    if (!normalized) return 0;

    out_index = 0;
    for (in_index = 1; in_index < length; in_index++) {
        if (type_name[in_index] == '.' &&
            in_index + 1 < length &&
            type_name[in_index + 1] == '.') {
            normalized[out_index++] = '.';
            in_index++;
        } else {
            normalized[out_index++] = type_name[in_index];
        }
    }
    normalized[out_index] = 0;

    return normalized;
}

static int rxlink_type_assignable(void *userdata,
                                  const char *actual_type,
                                  const char *expected_type) {
    const module_list *modules = (const module_list *)userdata;
    char *actual_contract;
    char *expected_contract;
    size_t i;
    int result = 0;

    if (!modules) return 0;

    actual_contract = rxlink_metadata_type_to_contract_name(actual_type);
    expected_contract = rxlink_metadata_type_to_contract_name(expected_type);
    if (!actual_contract || !expected_contract) {
        if (actual_contract) free(actual_contract);
        if (expected_contract) free(expected_contract);
        return 0;
    }

    if (strcmp(actual_contract, expected_contract) == 0) {
        result = 1;
    }

    for (i = 0; i < modules->count && !result; i++) {
        module_file *module;
        int meta_ix;

        if (!rxlink_module_is_active(&modules->items[i])) continue;
        module = modules->items[i].module;
        meta_ix = module->header.meta_head;
        while (meta_ix != -1) {
            meta_entry *entry = (meta_entry *)((unsigned char *)module->constant + (size_t)meta_ix);
            if (entry->base.type == META_IMPLEMENTS) {
                meta_implements_constant *impl = (meta_implements_constant *)entry;
                const char *class_name = module_string_constant(module, impl->symbol);
                const char *interface_name = module_string_constant(module, impl->interface_symbol);
                if (class_name && interface_name &&
                    strcmp(class_name, actual_contract) == 0 &&
                    strcmp(interface_name, expected_contract) == 0) {
                    result = 1;
                    break;
                }
            }
            meta_ix = entry->next;
        }
    }

    free(actual_contract);
    free(expected_contract);
    return result;
}

static char *rxlink_build_member_name(const char *owner, const char *member) {
    size_t owner_length;
    size_t member_length;
    char *name;

    if (!owner || !member) return 0;
    owner_length = strlen(owner);
    member_length = strlen(member);
    name = malloc(owner_length + 1 + member_length + 1);
    if (!name) return 0;
    memcpy(name, owner, owner_length);
    name[owner_length] = '.';
    memcpy(name + owner_length + 1, member, member_length);
    name[owner_length + 1 + member_length] = 0;
    return name;
}

static char *rxlink_build_factory_proc_name(const char *owner, const char *factory) {
    const char *prefix = "\xC2\xA7" "factory";
    char *factory_member;
    char *name;

    if (!owner || !factory) return 0;
    if (strcmp(factory, "*") == 0) return rxlink_build_member_name(owner, prefix);
    factory_member = malloc(strlen(prefix) + 1 + strlen(factory) + 1);
    if (!factory_member) return 0;
    sprintf(factory_member, "%s.%s", prefix, factory);
    name = rxlink_build_member_name(owner, factory_member);
    free(factory_member);
    return name;
}

static char *rxlink_build_match_proc_name(const char *owner, const char *factory) {
    const char *prefix = "\xC2\xA7" "match";
    char *match_member;
    char *name;

    if (!owner || !factory) return 0;
    if (strcmp(factory, "*") == 0) return rxlink_build_member_name(owner, prefix);
    match_member = malloc(strlen(prefix) + 1 + strlen(factory) + 1);
    if (!match_member) return 0;
    sprintf(match_member, "%s.%s", prefix, factory);
    name = rxlink_build_member_name(owner, match_member);
    free(match_member);
    return name;
}

static meta_func_constant *rxlink_find_meta_func(const module_list *modules,
                                                 const char *symbol,
                                                 module_file **module_out) {
    size_t i;

    if (module_out) *module_out = 0;
    if (!modules || !symbol) return 0;

    for (i = 0; i < modules->count; i++) {
        module_file *module;
        int meta_ix;

        if (!rxlink_module_is_active(&modules->items[i])) continue;
        module = modules->items[i].module;
        meta_ix = module->header.meta_head;
        while (meta_ix != -1) {
            meta_entry *entry = (meta_entry *)((unsigned char *)module->constant + (size_t)meta_ix);
            if (entry->base.type == META_FUNC) {
                meta_func_constant *func = (meta_func_constant *)entry;
                const char *func_symbol = module_string_constant(module, func->symbol);
                if (func_symbol && rxlink_symbol_equals(func_symbol, symbol)) {
                    if (module_out) *module_out = module;
                    return func;
                }
            }
            meta_ix = entry->next;
        }
    }

    return 0;
}

static int rxlink_meta_func_matches_signature(const module_list *modules,
                                              module_file *module,
                                              meta_func_constant *func,
                                              const rx_callable_signature *expected) {
    const char *type;
    const char *args;
    rx_callable_signature actual;
    rx_callable_compare_options options;
    int matches;

    if (!module || !func || !expected) return 0;

    type = module_string_constant(module, func->type);
    args = module_string_constant(module, func->args);
    if (!type || !args) return 0;

    if (!rx_sig_init_from_parts(&actual, expected->name ? expected->name : "", type, args)) {
        return 0;
    }
    memset(&options, 0, sizeof(options));
    options.allow_return_covariance = 1;
    options.type_assignable = rxlink_type_assignable;
    options.userdata = (void *)modules;

    matches = rx_sig_matches_contract(expected, &actual, &options);
    rx_sig_free(&actual);
    return matches;
}

static int rxlink_validate_contracts(const module_list *modules) {
    size_t i;

    for (i = 0; i < modules->count; i++) {
        module_file *module;
        int meta_ix;

        if (!rxlink_module_is_active(&modules->items[i])) continue;
        module = modules->items[i].module;
        meta_ix = module->header.meta_head;

        while (meta_ix != -1) {
            meta_entry *entry = (meta_entry *)((unsigned char *)module->constant + (size_t)meta_ix);

            if (entry->base.type == META_IMPLEMENTS) {
                meta_implements_constant *impl = (meta_implements_constant *)entry;
                const char *class_name = module_string_constant(module, impl->symbol);
                const char *interface_name = module_string_constant(module, impl->interface_symbol);
                size_t j;

                if (!class_name || !interface_name) return 0;

                for (j = 0; j < modules->count; j++) {
                    module_file *iface_module;
                    int iface_meta_ix;

                    if (!rxlink_module_is_active(&modules->items[j])) continue;
                    iface_module = modules->items[j].module;
                    iface_meta_ix = iface_module->header.meta_head;

                    while (iface_meta_ix != -1) {
                        meta_entry *iface_entry = (meta_entry *)((unsigned char *)iface_module->constant + (size_t)iface_meta_ix);
                        if (iface_entry->base.type == META_MEMBER) {
                            meta_member_constant *member = (meta_member_constant *)iface_entry;
                            const char *owner = module_string_constant(iface_module, member->owner);
                            const char *kind = module_string_constant(iface_module, member->kind);
                            const char *member_name = module_string_constant(iface_module, member->member);
                            const char *type = module_string_constant(iface_module, member->type);
                            const char *args = module_string_constant(iface_module, member->args);

                            if (owner && kind && member_name && type && args &&
                                strcmp(owner, interface_name) == 0) {
                                if (rxlink_kind_is_method(kind)) {
                                    char *proc_name = rxlink_build_member_name(class_name, member_name);
                                    module_file *func_module = 0;
                                    meta_func_constant *func = proc_name ? rxlink_find_meta_func(modules, proc_name, &func_module) : 0;
                                    rx_callable_signature expected;
                                    int ok;
                                    int signature_ready;

                                    if (!func && rxlink_kind_is_final_method(kind)) {
                                        if (proc_name) free(proc_name);
                                        proc_name = rxlink_build_member_name(interface_name, member_name);
                                        func = proc_name ? rxlink_find_meta_func(modules, proc_name, &func_module) : 0;
                                    }

                                    rx_sig_init_empty(&expected);
                                    signature_ready = func &&
                                                      rx_sig_init_from_parts(&expected, member_name, type, args);
                                    ok = signature_ready &&
                                         rxlink_meta_func_matches_signature(modules, func_module, func, &expected);
                                    rx_sig_free(&expected);
                                    if (proc_name) free(proc_name);
                                    if (!ok) {
                                        fprintf(stderr,
                                                "ERROR: class %s does not satisfy interface member %s.%s signature\n",
                                                class_name, interface_name, member_name);
                                        return 0;
                                    }
                                } else if (rxlink_kind_is_factory(kind)) {
                                    char *factory_proc_name = rxlink_build_factory_proc_name(class_name, member_name);
                                    char *match_proc_name = rxlink_build_match_proc_name(class_name, member_name);
                                    module_file *factory_module = 0;
                                    module_file *match_module = 0;
                                    meta_func_constant *factory_func = factory_proc_name ? rxlink_find_meta_func(modules, factory_proc_name, &factory_module) : 0;
                                    meta_func_constant *match_func = match_proc_name ? rxlink_find_meta_func(modules, match_proc_name, &match_module) : 0;
                                    rx_callable_signature expected_factory;
                                    rx_callable_signature expected_match;
                                    int ok;
                                    int factory_signature_ready;
                                    int match_signature_ready;

                                    rx_sig_init_empty(&expected_factory);
                                    rx_sig_init_empty(&expected_match);
                                    factory_signature_ready = factory_func &&
                                                              rx_sig_init_from_parts(&expected_factory, member_name, type, args);
                                    ok = factory_signature_ready &&
                                         rxlink_meta_func_matches_signature(modules, factory_module, factory_func, &expected_factory);
                                    if (ok && match_func) {
                                        match_signature_ready = rx_sig_init_from_parts(&expected_match, "", ".int", args);
                                        ok = match_signature_ready &&
                                             rxlink_meta_func_matches_signature(modules, match_module, match_func, &expected_match);
                                    }
                                    rx_sig_free(&expected_factory);
                                    rx_sig_free(&expected_match);

                                    if (factory_proc_name) free(factory_proc_name);
                                    if (match_proc_name) free(match_proc_name);
                                    if (!ok) {
                                        fprintf(stderr,
                                                "ERROR: class %s does not satisfy interface factory %s.%s signature\n",
                                                class_name, interface_name, member_name);
                                        return 0;
                                    }
                                }
                            }
                        }
                        iface_meta_ix = iface_entry->next;
                    }
                }
            }

            meta_ix = entry->next;
        }
    }

    return 1;
}

static void module_list_free(module_list *list) {
    size_t i;

    if (!list) return;
    for (i = 0; i < list->count; i++) {
        free_module(list->items[i].module);
        free(list->items[i].input_path);
        free(list->items[i].selector_name);
        string_list_free(&list->items[i].exports);
        string_list_free(&list->items[i].imports);
        string_list_free(&list->items[i].defined_interfaces);
        string_list_free(&list->items[i].implemented_interfaces);
        string_list_free(&list->items[i].referenced_interfaces);
        string_list_free(&list->items[i].provided_methods);
        string_list_free(&list->items[i].referenced_methods);
        string_list_free(&list->items[i].unresolved_imports);
    }
    free(list->items);
    list->items = 0;
    list->count = 0;
    list->capacity = 0;
}

static int output_list_append(rxlink_output_list *list, rxlink_output_module *item) {
    rxlink_output_module *new_items;
    size_t new_capacity;

    if (list->count == list->capacity) {
        new_capacity = list->capacity ? list->capacity * 2 : 8;
        new_items = realloc(list->items, sizeof(rxlink_output_module) * new_capacity);
        if (!new_items) {
            RX_REPORT_OOM("realloc rxlink output list",
                          sizeof(rxlink_output_module) * new_capacity, 0);
            return 0;
        }
        list->items = new_items;
        list->capacity = new_capacity;
    }

    list->items[list->count++] = *item;
    return 1;
}

static void output_list_free(rxlink_output_list *list) {
    size_t i;

    if (!list) return;
    for (i = 0; i < list->count; i++) {
        if (list->items[i].module) free_module(list->items[i].module);
        free(list->items[i].maps);
        free(list->items[i].map_hash_slots);
    }
    free(list->items);
    list->items = 0;
    list->count = 0;
    list->capacity = 0;
}

static void build_context_init(rxlink_build_context *context) {
    rxbin_byte_buffer_init(&context->shared_pool);
    context->leaf_entries = 0;
    context->leaf_count = 0;
    context->leaf_capacity = 0;
    context->leaf_hash_slots = 0;
    context->leaf_hash_capacity = 0;
    context->strip_source_metadata = 0;
    context->strip_inline_metadata = 1;
}

static void build_context_free(rxlink_build_context *context) {
    rxbin_byte_buffer_free(&context->shared_pool);
    free(context->leaf_entries);
    free(context->leaf_hash_slots);
    context->leaf_entries = 0;
    context->leaf_count = 0;
    context->leaf_capacity = 0;
    context->leaf_hash_slots = 0;
    context->leaf_hash_capacity = 0;
}

static int keyword_equals(const char *left, const char *right) {
    while (*left && *right) {
        if (toupper((unsigned char)*left) != toupper((unsigned char)*right)) return 0;
        left++;
        right++;
    }
    return *left == '\0' && *right == '\0';
}

static char *trim_whitespace(char *text) {
    char *end;

    while (*text && isspace((unsigned char)*text)) text++;
    if (!*text) return text;

    end = text + strlen(text) - 1;
    while (end >= text && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    return text;
}

static char *unquote_in_place(char *text) {
    size_t len;

    text = trim_whitespace(text);
    len = strlen(text);
    if (len >= 2 && ((text[0] == '"' && text[len - 1] == '"') || (text[0] == '\'' && text[len - 1] == '\''))) {
        text[len - 1] = '\0';
        text++;
    }
    return text;
}

static void init_link_config(link_config *config) {
    string_list_init(&config->inputs);
    string_list_init(&config->roots);
    string_list_init(&config->includes);
    string_list_init(&config->omits);
    config->output_path = 0;
    config->map_path = 0;
    config->provider_requirements_path = 0;
    config->location = 0;
    config->strip_source_metadata = 0;
    config->strip_inline_metadata = 1;
    config->debug_mode = 0;
}

static void free_link_config(link_config *config) {
    string_list_free(&config->inputs);
    string_list_free(&config->roots);
    string_list_free(&config->includes);
    string_list_free(&config->omits);
    free(config->output_path);
    free(config->map_path);
    free(config->provider_requirements_path);
}

static int set_single_path(char **target, const char *value) {
    char *copy = strdup(value);
    if (!copy) {
        RX_REPORT_OOM("strdup rxlink path", strlen(value) + 1, value);
        return 0;
    }
    if (*target) free(*target);
    *target = copy;
    return 1;
}

static int parse_control_file(link_config *config, const char *path) {
    FILE *fp;
    char line[4096];
    size_t line_number = 0;

    fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "ERROR: opening control file %s\n", path);
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        char *cursor;
        char *keyword;
        char *value;

        line_number++;
        cursor = trim_whitespace(line);
        if (!*cursor || *cursor == '*' || *cursor == '#') continue;

        keyword = cursor;
        while (*cursor && !isspace((unsigned char)*cursor)) cursor++;
        if (*cursor) {
            *cursor = '\0';
            cursor++;
        }

        value = unquote_in_place(cursor);
        if (!*value) {
            fprintf(stderr, "ERROR: control file %s line %lu missing value for %s\n",
                    path, (unsigned long)line_number, keyword);
            fclose(fp);
            return 0;
        }

        if (keyword_equals(keyword, "INPUT")) {
            if (!string_list_append(&config->inputs, value)) goto oom;
        } else if (keyword_equals(keyword, "ROOT")) {
            if (!string_list_append(&config->roots, value)) goto oom;
        } else if (keyword_equals(keyword, "INCLUDE")) {
            if (!string_list_append(&config->includes, value)) goto oom;
        } else if (keyword_equals(keyword, "OMIT")) {
            if (!string_list_append(&config->omits, value)) goto oom;
        } else if (keyword_equals(keyword, "OUTPUT")) {
            if (!set_single_path(&config->output_path, value)) goto oom;
        } else if (keyword_equals(keyword, "MAP")) {
            if (!set_single_path(&config->map_path, value)) goto oom;
        } else if (keyword_equals(keyword, "PROVIDERS")) {
            if (!set_single_path(&config->provider_requirements_path, value)) goto oom;
        } else if (keyword_equals(keyword, "STRIP")) {
            if (keyword_equals(value, "SOURCE")) {
                config->strip_source_metadata = 1;
            } else if (keyword_equals(value, "INLINE")) {
                config->strip_inline_metadata = 1;
            } else {
                fprintf(stderr, "ERROR: unknown STRIP mode %s on line %lu\n",
                        value, (unsigned long)line_number);
                fclose(fp);
                return 0;
            }
        } else if (keyword_equals(keyword, "PRESERVE")) {
            if (keyword_equals(value, "INLINE")) {
                config->strip_inline_metadata = 0;
            } else {
                fprintf(stderr, "ERROR: unknown PRESERVE mode %s on line %lu\n",
                        value, (unsigned long)line_number);
                fclose(fp);
                return 0;
            }
        } else {
            fprintf(stderr, "ERROR: unknown control directive %s on line %lu\n",
                    keyword, (unsigned long)line_number);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 1;

oom:
    RX_REPORT_OOM("reading rxlink control file", RX_OOM_UNKNOWN_SIZE, path);
    fclose(fp);
    return 0;
}

static int module_selector_matches(const link_module_info *module, const char *selector) {
    const char *separator = strstr(selector, "::");

    if (separator) {
        size_t input_len = (size_t)(separator - selector);
        const char *member = separator + 2;
        const char *module_basename = filename(module->module->name);
        size_t module_input_len = strlen(module->input_path);

        if (module_input_len != input_len) return 0;
        if (memcmp(selector, module->input_path, input_len) != 0) return 0;
        if (!*member) return 0;

        return strcmp(member, module->module->name) == 0 ||
               strcmp(member, module_basename) == 0 ||
               strcmp(member, module->selector_name) == 0;
    }

    return strcmp(selector, module->module->name) == 0 ||
           strcmp(selector, filename(module->module->name)) == 0 ||
           strcmp(selector, module->selector_name) == 0;
}

static int load_module_metadata(link_module_info *info) {
    int ix;
    module_file *module = info->module;
    size_t code_index;

    ix = module->header.proc_head;
    while (ix != -1) {
        proc_constant *proc = (proc_constant *)((unsigned char *)module->constant + (size_t)ix);
        if (proc->base.type != PROC_CONST) return 0;
        if (strcmp(proc->name, "main") == 0) info->has_main = 1;
        ix = proc->next;
    }

    ix = module->header.expose_head;
    while (ix != -1) {
        chameleon_constant *entry = (chameleon_constant *)((unsigned char *)module->constant + (size_t)ix);
        if (entry->type == EXPOSE_PROC_CONST) {
            expose_proc_constant *exposed = (expose_proc_constant *)entry;
            if (exposed->imported) {
                if (!string_list_append_unique(&info->imports, exposed->index)) return 0;
            } else {
                if (!string_list_append_unique(&info->exports, exposed->index)) return 0;
                if (!append_method_name_from_symbol(&info->provided_methods, exposed->index)) return 0;
            }
            ix = exposed->next;
        } else if (entry->type == EXPOSE_REG_CONST) {
            ix = ((expose_reg_constant *)entry)->next;
        } else {
            return 0;
        }
    }

    ix = module->header.meta_head;
    while (ix != -1) {
        meta_entry *entry = (meta_entry *)((unsigned char *)module->constant + (size_t)ix);
        switch (entry->base.type) {
            case META_FUNC: {
                meta_func_constant *func = (meta_func_constant *)entry;
                const char *symbol_name = module_string_constant(module, func->symbol);
                if (!append_method_name_from_symbol(&info->provided_methods, symbol_name)) return 0;
                break;
            }
            case META_INTERFACE: {
                meta_interface_constant *iface = (meta_interface_constant *)entry;
                string_constant *symbol = (string_constant *)((unsigned char *)module->constant + iface->symbol);
                if (!string_list_append_unique(&info->defined_interfaces, symbol->string)) return 0;
                break;
            }
            case META_IMPLEMENTS: {
                meta_implements_constant *impl = (meta_implements_constant *)entry;
                string_constant *iface = (string_constant *)((unsigned char *)module->constant + impl->interface_symbol);
                if (!string_list_append_unique(&info->implemented_interfaces, iface->string)) return 0;
                break;
            }
            case META_MEMBER: {
                meta_member_constant *member = (meta_member_constant *)entry;
                const char *member_name = module_string_constant(module, member->member);
                if (member_name &&
                    strcmp(member_name, "*") != 0 &&
                    !string_list_append_unique(&info->provided_methods, member_name)) {
                    return 0;
                }
                break;
            }
            default:
                break;
        }
        ix = entry->next;
    }

    code_index = 0;
    while (code_index < module->header.instruction_size) {
        size_t operand_count;
        size_t operand_index;
        int opcode;

        opcode = ((bin_code *)module->instructions)[code_index].instruction.opcode;
        operand_count = rxop_format_operand_count(rxbin_opcode_format(opcode));
        if (opcode == OP_SRCFPROCSEL_REG_STRING_REG) {
            for (operand_index = 0; operand_index < operand_count; operand_index++) {
                if (rxop_format_operand_type(rxbin_opcode_format(opcode), operand_index) == OP_STRING) {
                    size_t descriptor_offset;
                    char *descriptor;
                    const char *separator;
                    size_t interface_length;
                    char *interface_name;
                    int ok;
                    rx_callable_signature signature;

                    descriptor_offset = ((bin_code *)module->instructions)[code_index + (size_t)operand_index + 1].index;
                    descriptor = module_instruction_string(module,
                                                           opcode,
                                                           (unsigned int)operand_index,
                                                           descriptor_offset);
                    if (!descriptor) return 0;
                    if (!rx_sig_parse_descriptor(descriptor, &signature)) {
                        free(descriptor);
                        return 0;
                    }
                    free(descriptor);

                    separator = strstr(signature.name, "..");
                    interface_length = separator ? (size_t)(separator - signature.name) : strlen(signature.name);
                    if (!interface_length) {
                        rx_sig_free(&signature);
                        continue;
                    }

                    interface_name = malloc(interface_length + 1);
                    if (!interface_name) {
                        RX_REPORT_OOM("malloc rxlink interface reference name",
                                      interface_length + 1, info->input_path);
                        rx_sig_free(&signature);
                        return 0;
                    }
                    memcpy(interface_name, signature.name, interface_length);
                    interface_name[interface_length] = '\0';
                    ok = string_list_append_unique(&info->referenced_interfaces, interface_name);
                    free(interface_name);
                    rx_sig_free(&signature);
                    if (!ok) return 0;
                }
            }
        } else if (opcode == OP_SRCMETHODSEL_REG_REG_STRING) {
            for (operand_index = 0; operand_index < operand_count; operand_index++) {
                if (rxop_format_operand_type(rxbin_opcode_format(opcode), operand_index) == OP_STRING) {
                    size_t descriptor_offset;
                    char *descriptor;
                    rx_callable_signature signature;
                    int ok;

                    descriptor_offset = ((bin_code *)module->instructions)[code_index + (size_t)operand_index + 1].index;
                    descriptor = module_instruction_string(module,
                                                           opcode,
                                                           (unsigned int)operand_index,
                                                           descriptor_offset);
                    if (!descriptor) return 0;
                    if (!rx_sig_parse_descriptor(descriptor, &signature)) {
                        free(descriptor);
                        return 0;
                    }
                    free(descriptor);
                    ok = string_list_append_unique(&info->referenced_methods, signature.name);
                    rx_sig_free(&signature);
                    if (!ok) {
                        return 0;
                    }
                }
            }
        }
        code_index += (size_t)operand_count + 1;
    }

    return 1;
}

static int load_input_modules(module_list *modules, const link_config *config) {
    size_t input_index;

    for (input_index = 0; input_index < config->inputs.count; input_index++) {
        const char *input_path = config->inputs.items[input_index];
        const char *type_bin = has_any_extension(input_path) ? "" : "rxbin";
        FILE *fp = openfile((char *)input_path, (char *)type_bin, config->location, "rb");
        int rc = 0;
        size_t member_index = 0;

        if (!fp) {
            fprintf(stderr, "ERROR: opening input %s\n", input_path);
            return 0;
        }

        while (rc == 0) {
            module_file *module = 0;
            link_module_info info;

            rc = read_module(&module, fp);
            if (rc == 0) {
                memset(&info, 0, sizeof(info));
                string_list_init(&info.exports);
                string_list_init(&info.imports);
                string_list_init(&info.defined_interfaces);
                string_list_init(&info.implemented_interfaces);
                string_list_init(&info.referenced_interfaces);
                string_list_init(&info.provided_methods);
                string_list_init(&info.referenced_methods);
                string_list_init(&info.unresolved_imports);
                info.module = module;
                info.input_path = strdup(input_path);
                if (!info.input_path) {
                    RX_REPORT_OOM("strdup rxlink input path", strlen(input_path) + 1, input_path);
                    free_module(module);
                    fclose(fp);
                    return 0;
                }
                info.input_index = input_index;
                info.member_index = member_index++;
                info.selector_name = strip_rightmost_extension_if(filename(module->name), "rxas");
                if (!info.input_path || !info.selector_name || !load_module_metadata(&info) ||
                    !module_list_append(modules, &info)) {
                    free_module(module);
                    free(info.input_path);
                    free(info.selector_name);
                    string_list_free(&info.exports);
                    string_list_free(&info.imports);
                    string_list_free(&info.defined_interfaces);
                    string_list_free(&info.implemented_interfaces);
                    string_list_free(&info.referenced_interfaces);
                    string_list_free(&info.provided_methods);
                    string_list_free(&info.referenced_methods);
                    string_list_free(&info.unresolved_imports);
                    fclose(fp);
                    return 0;
                }
            } else if (rc != 1) {
                fprintf(stderr, "ERROR: reading input %s\n", input_path);
                fclose(fp);
                return 0;
            }
        }

        fclose(fp);
    }

    return 1;
}

static int add_to_queue(size_t **queue_ref, size_t *queue_count, size_t *queue_capacity, size_t index) {
    size_t *new_queue;
    size_t new_capacity;

    if (*queue_count == *queue_capacity) {
        new_capacity = *queue_capacity ? *queue_capacity * 2 : 16;
        new_queue = realloc(*queue_ref, sizeof(size_t) * new_capacity);
        if (!new_queue) {
            RX_REPORT_OOM("realloc rxlink selection queue",
                          sizeof(size_t) * new_capacity, 0);
            return 0;
        }
        *queue_ref = new_queue;
        *queue_capacity = new_capacity;
    }
    (*queue_ref)[(*queue_count)++] = index;
    return 1;
}

static int select_module_by_index(module_list *modules, size_t **queue_ref, size_t *queue_count,
                                  size_t *queue_capacity, size_t index) {
    if (modules->items[index].queued) return 1;
    modules->items[index].queued = 1;
    modules->items[index].selected = 1;
    return add_to_queue(queue_ref, queue_count, queue_capacity, index);
}

static int selected_export_conflicts(const module_list *modules, const char *export_name, size_t *first_index,
                                     size_t *second_index) {
    size_t i;
    int found = 0;

    for (i = 0; i < modules->count; i++) {
        if (!modules->items[i].selected || modules->items[i].omitted) continue;
        if (!string_list_contains(&modules->items[i].exports, export_name)) continue;
        if (!found) {
            *first_index = i;
            found = 1;
        } else {
            *second_index = i;
            return 1;
        }
    }

    return 0;
}

static int select_modules(module_list *modules, const link_config *config) {
    size_t i;
    size_t *queue = 0;
    size_t queue_count = 0;
    size_t queue_capacity = 0;
    size_t queue_pos = 0;
    int roots_applied = 0;

    for (i = 0; i < config->omits.count; i++) {
        size_t j;
        int matched = 0;
        for (j = 0; j < modules->count; j++) {
            if (module_selector_matches(&modules->items[j], config->omits.items[i])) {
                modules->items[j].omitted = 1;
                matched = 1;
            }
        }
        if (!matched) {
            fprintf(stderr, "ERROR: OMIT selector %s matched no modules\n", config->omits.items[i]);
            free(queue);
            return 0;
        }
    }

    for (i = 0; i < config->includes.count; i++) {
        size_t j;
        int matched = 0;
        for (j = 0; j < modules->count; j++) {
            if (module_selector_matches(&modules->items[j], config->includes.items[i])) {
                if (modules->items[j].omitted) {
                    fprintf(stderr, "ERROR: module %s is both included and omitted\n", config->includes.items[i]);
                    free(queue);
                    return 0;
                }
                if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, j)) {
                    free(queue);
                    return 0;
                }
                matched = 1;
                roots_applied = 1;
            }
        }
        if (!matched) {
            fprintf(stderr, "ERROR: INCLUDE selector %s matched no modules\n", config->includes.items[i]);
            free(queue);
            return 0;
        }
    }

    for (i = 0; i < config->roots.count; i++) {
        size_t j;
        int matched = 0;
        for (j = 0; j < modules->count; j++) {
            if (module_selector_matches(&modules->items[j], config->roots.items[i])) {
                if (modules->items[j].omitted) {
                    fprintf(stderr, "ERROR: root module %s is omitted\n", config->roots.items[i]);
                    free(queue);
                    return 0;
                }
                if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, j)) {
                    free(queue);
                    return 0;
                }
                matched = 1;
                roots_applied = 1;
            }
        }
        if (!matched) {
            fprintf(stderr, "ERROR: ROOT selector %s matched no modules\n", config->roots.items[i]);
            free(queue);
            return 0;
        }
    }

    if (!roots_applied) {
        int found_main = 0;
        for (i = 0; i < modules->count; i++) {
            if (!modules->items[i].omitted && modules->items[i].has_main) {
                if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, i)) {
                    free(queue);
                    return 0;
                }
                found_main = 1;
            }
        }

        if (!found_main) {
            for (i = 0; i < modules->count; i++) {
                if (!modules->items[i].omitted && modules->items[i].input_index == 0) {
                    if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, i)) {
                        free(queue);
                        return 0;
                    }
                }
            }
        }
    }

    if (!queue_count) {
        fprintf(stderr, "ERROR: no root modules selected\n");
        free(queue);
        return 0;
    }

    while (queue_pos < queue_count) {
        link_module_info *module = &modules->items[queue[queue_pos++]];
        size_t import_index;
        size_t iface_index;
        size_t method_index;

        for (import_index = 0; import_index < module->imports.count; import_index++) {
            size_t provider_index = 0;
            size_t provider_count = 0;
            size_t candidate;

            for (candidate = 0; candidate < modules->count; candidate++) {
                if (modules->items[candidate].omitted) continue;
                if (string_list_contains(&modules->items[candidate].exports, module->imports.items[import_index])) {
                    provider_index = candidate;
                    provider_count++;
                }
            }

            if (provider_count == 1) {
                if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, provider_index)) {
                    free(queue);
                    return 0;
                }
            } else if (provider_count > 1) {
                fprintf(stderr, "ERROR: Duplicate providers for import %s\n", module->imports.items[import_index]);
                free(queue);
                return 0;
            } else {
                if (!string_list_append_unique(&module->unresolved_imports, module->imports.items[import_index])) {
                    free(queue);
                    return 0;
                }
            }
        }

        for (iface_index = 0; iface_index < module->implemented_interfaces.count; iface_index++) {
            size_t candidate;
            for (candidate = 0; candidate < modules->count; candidate++) {
                if (modules->items[candidate].omitted) continue;
                if (string_list_contains(&modules->items[candidate].defined_interfaces,
                                         module->implemented_interfaces.items[iface_index])) {
                    if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, candidate)) {
                        free(queue);
                        return 0;
                    }
                }
            }
        }

        for (iface_index = 0; iface_index < module->referenced_interfaces.count; iface_index++) {
            size_t candidate;
            for (candidate = 0; candidate < modules->count; candidate++) {
                if (modules->items[candidate].omitted) continue;
                if (string_list_contains(&modules->items[candidate].defined_interfaces,
                                         module->referenced_interfaces.items[iface_index]) ||
                    string_list_contains(&modules->items[candidate].implemented_interfaces,
                                         module->referenced_interfaces.items[iface_index])) {
                    if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, candidate)) {
                        free(queue);
                        return 0;
                    }
                }
            }
        }

        for (iface_index = 0; iface_index < module->defined_interfaces.count; iface_index++) {
            size_t candidate;
            for (candidate = 0; candidate < modules->count; candidate++) {
                if (modules->items[candidate].omitted) continue;
                if (string_list_contains(&modules->items[candidate].implemented_interfaces,
                                         module->defined_interfaces.items[iface_index])) {
                    if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, candidate)) {
                        free(queue);
                        return 0;
                    }
                }
            }
        }

        for (method_index = 0; method_index < module->referenced_methods.count; method_index++) {
            size_t candidate;
            for (candidate = 0; candidate < modules->count; candidate++) {
                if (modules->items[candidate].omitted) continue;
                if (string_list_contains(&modules->items[candidate].provided_methods,
                                         module->referenced_methods.items[method_index])) {
                    if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, candidate)) {
                        free(queue);
                        return 0;
                    }
                }
            }
        }
    }

    for (i = 0; i < modules->count; i++) {
        size_t export_index;
        if (!modules->items[i].selected || modules->items[i].omitted) continue;
        for (export_index = 0; export_index < modules->items[i].exports.count; export_index++) {
            size_t first_index = 0;
            size_t second_index = 0;
            if (selected_export_conflicts(modules, modules->items[i].exports.items[export_index],
                                          &first_index, &second_index) &&
                first_index != i) {
                fprintf(stderr, "ERROR: Duplicate exported symbol %s in %s and %s\n",
                        modules->items[i].exports.items[export_index],
                        modules->items[first_index].selector_name,
                        modules->items[second_index].selector_name);
                free(queue);
                return 0;
            }
        }
    }

    free(queue);
    return 1;
}

static int reserve_pool_entry(rxlink_build_context *context, size_t size_in_pool, enum const_pool_type type,
                              size_t *offset_out) {
    size_t offset = context->shared_pool.size;

    if (!rxbin_byte_buffer_reserve(&context->shared_pool, size_in_pool)) {
        RX_REPORT_OOM("reserve rxlink shared constant pool", size_in_pool, 0);
        return 0;
    }
    memset(context->shared_pool.data + context->shared_pool.size, 0, size_in_pool);
    context->shared_pool.size += size_in_pool;
    ((chameleon_constant *)(context->shared_pool.data + offset))->size_in_pool = size_in_pool;
    ((chameleon_constant *)(context->shared_pool.data + offset))->type = type;
    *offset_out = offset;
    return 1;
}

static size_t rxlink_next_hash_capacity(size_t needed) {
    size_t capacity = 16;
    size_t target;

    if (needed > SIZE_MAX / 2) return 0;
    target = needed * 2;
    while (capacity < target) {
        if (capacity > SIZE_MAX / 2) return 0;
        capacity *= 2;
    }
    return capacity;
}

static size_t rxlink_hash_size_value(size_t value) {
    value ^= value >> 16;
    value *= (size_t)0x7feb352dU;
    value ^= value >> 15;
    value *= (size_t)0x846ca68bU;
    value ^= value >> 16;
    return value;
}

static size_t rxlink_hash_bytes(const void *data, size_t size) {
    const unsigned char *bytes = (const unsigned char *)data;
    size_t hash;
    size_t i;

#if SIZE_MAX > UINT32_MAX
    hash = (size_t)1469598103934665603ULL;
    for (i = 0; i < size; i++) {
        hash ^= (size_t)bytes[i];
        hash *= (size_t)1099511628211ULL;
    }
#else
    hash = (size_t)2166136261UL;
    for (i = 0; i < size; i++) {
        hash ^= (size_t)bytes[i];
        hash *= (size_t)16777619UL;
    }
#endif

    return hash ? hash : 1;
}

static void insert_const_map_hash_slot(rxlink_output_module *module, size_t map_index) {
    size_t mask = module->map_hash_capacity - 1;
    size_t slot = rxlink_hash_size_value(module->maps[map_index].old_offset >> 3) & mask;

    while (module->map_hash_slots[slot]) {
        slot = (slot + 1) & mask;
    }
    module->map_hash_slots[slot] = map_index + 1;
}

static int rebuild_const_map_hash(rxlink_output_module *module, size_t new_capacity) {
    size_t *new_slots;
    size_t i;

    new_slots = calloc(new_capacity, sizeof(size_t));
    if (!new_slots) {
        RX_REPORT_OOM("calloc rxlink constant map hash",
                      sizeof(size_t) * new_capacity, 0);
        return 0;
    }

    free(module->map_hash_slots);
    module->map_hash_slots = new_slots;
    module->map_hash_capacity = new_capacity;
    for (i = 0; i < module->map_count; i++) {
        insert_const_map_hash_slot(module, i);
    }
    return 1;
}

static int ensure_const_map_hash_capacity(rxlink_output_module *module, size_t needed) {
    size_t new_capacity;

    if (module->map_hash_capacity && needed <= module->map_hash_capacity / 2) return 1;
    new_capacity = rxlink_next_hash_capacity(needed);
    if (!new_capacity) {
        RX_REPORT_OOM("grow rxlink constant map hash", RX_OOM_UNKNOWN_SIZE, 0);
        return 0;
    }
    return rebuild_const_map_hash(module, new_capacity);
}

static const_map_entry *find_const_map(rxlink_output_module *module, size_t old_offset) {
    size_t i;

    if (module->map_hash_slots && module->map_hash_capacity) {
        size_t mask = module->map_hash_capacity - 1;
        size_t slot = rxlink_hash_size_value(old_offset >> 3) & mask;
        while (module->map_hash_slots[slot]) {
            const_map_entry *entry = &module->maps[module->map_hash_slots[slot] - 1];
            if (entry->old_offset == old_offset) return entry;
            slot = (slot + 1) & mask;
        }
        return 0;
    }

    for (i = 0; i < module->map_count; i++) {
        if (module->maps[i].old_offset == old_offset) return &module->maps[i];
    }
    return 0;
}

static int add_const_map(rxlink_output_module *module, size_t old_offset, size_t new_offset) {
    const_map_entry *new_entries;
    size_t new_capacity;

    if (find_const_map(module, old_offset)) return 1;

    if (module->map_count == module->map_capacity) {
        new_capacity = module->map_capacity ? module->map_capacity * 2 : 32;
        new_entries = realloc(module->maps, sizeof(const_map_entry) * new_capacity);
        if (!new_entries) {
            RX_REPORT_OOM("realloc rxlink constant map",
                          sizeof(const_map_entry) * new_capacity, 0);
            return 0;
        }
        module->maps = new_entries;
        module->map_capacity = new_capacity;
    }

    if (!ensure_const_map_hash_capacity(module, module->map_count + 1)) return 0;
    module->maps[module->map_count].old_offset = old_offset;
    module->maps[module->map_count].new_offset = new_offset;
    insert_const_map_hash_slot(module, module->map_count);
    module->map_count++;
    return 1;
}

static void insert_leaf_hash_slot(rxlink_build_context *context, size_t leaf_index) {
    size_t mask = context->leaf_hash_capacity - 1;
    size_t slot = context->leaf_entries[leaf_index].hash & mask;

    while (context->leaf_hash_slots[slot]) {
        slot = (slot + 1) & mask;
    }
    context->leaf_hash_slots[slot] = leaf_index + 1;
}

static int rebuild_leaf_hash(rxlink_build_context *context, size_t new_capacity) {
    size_t *new_slots;
    size_t i;

    new_slots = calloc(new_capacity, sizeof(size_t));
    if (!new_slots) {
        RX_REPORT_OOM("calloc rxlink leaf dedupe hash",
                      sizeof(size_t) * new_capacity, 0);
        return 0;
    }

    free(context->leaf_hash_slots);
    context->leaf_hash_slots = new_slots;
    context->leaf_hash_capacity = new_capacity;
    for (i = 0; i < context->leaf_count; i++) {
        insert_leaf_hash_slot(context, i);
    }
    return 1;
}

static int ensure_leaf_hash_capacity(rxlink_build_context *context, size_t needed) {
    size_t new_capacity;

    if (context->leaf_hash_capacity && needed <= context->leaf_hash_capacity / 2) return 1;
    new_capacity = rxlink_next_hash_capacity(needed);
    if (!new_capacity) {
        RX_REPORT_OOM("grow rxlink leaf dedupe hash", RX_OOM_UNKNOWN_SIZE, 0);
        return 0;
    }
    return rebuild_leaf_hash(context, new_capacity);
}

static leaf_dedupe_entry *find_leaf_dedupe(rxlink_build_context *context,
                                           const chameleon_constant *entry,
                                           size_t hash) {
    size_t i;

    if (context->leaf_hash_slots && context->leaf_hash_capacity) {
        size_t mask = context->leaf_hash_capacity - 1;
        size_t slot = hash & mask;
        while (context->leaf_hash_slots[slot]) {
            leaf_dedupe_entry *candidate = &context->leaf_entries[context->leaf_hash_slots[slot] - 1];
            if (candidate->hash == hash &&
                candidate->type == entry->type &&
                candidate->size_in_pool == entry->size_in_pool &&
                memcmp(context->shared_pool.data + candidate->output_offset,
                       entry, entry->size_in_pool) == 0) {
                return candidate;
            }
            slot = (slot + 1) & mask;
        }
        return 0;
    }

    for (i = 0; i < context->leaf_count; i++) {
        if (context->leaf_entries[i].type == entry->type &&
            context->leaf_entries[i].size_in_pool == entry->size_in_pool &&
            memcmp(context->shared_pool.data + context->leaf_entries[i].output_offset,
                   entry, entry->size_in_pool) == 0) {
            return &context->leaf_entries[i];
        }
    }
    return 0;
}

static size_t dedupe_leaf_constant(rxlink_build_context *context, const chameleon_constant *entry, int *ok) {
    size_t offset;
    size_t hash;
    leaf_dedupe_entry *existing;
    leaf_dedupe_entry *new_entries;
    size_t new_capacity;

    hash = rxlink_hash_bytes(entry, entry->size_in_pool);
    existing = find_leaf_dedupe(context, entry, hash);
    if (existing) return existing->output_offset;

    if (!reserve_pool_entry(context, entry->size_in_pool, entry->type, &offset)) {
        *ok = 0;
        return 0;
    }

    memcpy(context->shared_pool.data + offset, entry, entry->size_in_pool);

    if (context->leaf_count == context->leaf_capacity) {
        new_capacity = context->leaf_capacity ? context->leaf_capacity * 2 : 32;
        new_entries = realloc(context->leaf_entries, sizeof(leaf_dedupe_entry) * new_capacity);
        if (!new_entries) {
            RX_REPORT_OOM("realloc rxlink leaf dedupe table",
                          sizeof(leaf_dedupe_entry) * new_capacity, 0);
            *ok = 0;
            return 0;
        }
        context->leaf_entries = new_entries;
        context->leaf_capacity = new_capacity;
    }

    if (!ensure_leaf_hash_capacity(context, context->leaf_count + 1)) {
        *ok = 0;
        return 0;
    }
    context->leaf_entries[context->leaf_count].type = entry->type;
    context->leaf_entries[context->leaf_count].size_in_pool = entry->size_in_pool;
    context->leaf_entries[context->leaf_count].output_offset = offset;
    context->leaf_entries[context->leaf_count].hash = hash;
    insert_leaf_hash_slot(context, context->leaf_count);
    context->leaf_count++;
    return offset;
}

static size_t link_constant_offset(rxlink_build_context *context, rxlink_output_module *output_module,
                                   module_file *input_module, size_t old_offset, int *ok);

static int is_meta_constant_type(enum const_pool_type type) {
    switch (type) {
        case META_SOURCE_STEP:
        case META_TRACE_EVENT:
        case META_FUNC:
        case META_REG:
        case META_CONST:
        case META_CLEAR:
        case META_CLASS:
        case META_ATTR:
        case META_INTERFACE:
        case META_IMPLEMENTS:
        case META_MEMBER:
        case META_INLINE:
        case META_TASK_TARGET:
        case META_PROVIDER:
            return 1;
        default:
            return 0;
    }
}

static int should_strip_meta_constant(const rxlink_build_context *context, enum const_pool_type type) {
    if (context->strip_source_metadata && type == META_SOURCE_STEP) return 1;
    if (context->strip_source_metadata && type == META_TRACE_EVENT) return 1;
    if (context->strip_inline_metadata && type == META_INLINE) return 1;
    return 0;
}

static int rewrite_meta_constant(rxlink_build_context *context, rxlink_output_module *output_module,
                                 module_file *input_module, chameleon_constant *entry,
                                 size_t new_offset, int prev_offset, int next_offset, int *ok) {
    switch (entry->type) {
        case META_SOURCE_STEP: {
            meta_source_step_constant *source = (meta_source_step_constant *)entry;
            size_t file_offset = link_constant_offset(context, output_module, input_module, source->file, ok);
            size_t source_line_offset = link_constant_offset(context, output_module, input_module, source->source_line, ok);
            meta_source_step_constant *meta = (meta_source_step_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->file = file_offset;
            meta->source_line = source_line_offset;
            return *ok;
        }
        case META_TRACE_EVENT: {
            meta_trace_event_constant *source = (meta_trace_event_constant *)entry;
            size_t value_ref = source->value_ref;
            size_t symbol = source->symbol;
            size_t resolved_name = source->resolved_name;
            if (source->value_source == RXBIN_TRACE_VALUE_CONSTANT &&
                source->value_ref != RXBIN_TRACE_REF_NONE) {
                value_ref = link_constant_offset(context, output_module, input_module, source->value_ref, ok);
            }
            if (source->symbol != RXBIN_TRACE_REF_NONE) {
                symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            }
            if (source->resolved_name != RXBIN_TRACE_REF_NONE) {
                resolved_name = link_constant_offset(context, output_module, input_module, source->resolved_name, ok);
            }
            {
                meta_trace_event_constant *meta = (meta_trace_event_constant *)(context->shared_pool.data + new_offset);
                meta->base.prev = prev_offset;
                meta->base.next = next_offset;
                meta->value_ref = value_ref;
                meta->symbol = symbol;
                meta->resolved_name = resolved_name;
            }
            return *ok;
        }
        case META_FUNC: {
            meta_func_constant *source = (meta_func_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t option = link_constant_offset(context, output_module, input_module, source->option, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            size_t func = link_constant_offset(context, output_module, input_module, source->func, ok);
            size_t args = link_constant_offset(context, output_module, input_module, source->args, ok);
            meta_func_constant *meta = (meta_func_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->option = option;
            meta->type = type;
            meta->func = func;
            meta->args = args;
            return *ok;
        }
        case META_REG: {
            meta_reg_constant *source = (meta_reg_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t option = link_constant_offset(context, output_module, input_module, source->option, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            meta_reg_constant *meta = (meta_reg_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->option = option;
            meta->type = type;
            return *ok;
        }
        case META_CONST: {
            meta_const_constant *source = (meta_const_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t option = link_constant_offset(context, output_module, input_module, source->option, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            size_t constant = link_constant_offset(context, output_module, input_module, source->constant, ok);
            meta_const_constant *meta = (meta_const_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->option = option;
            meta->type = type;
            meta->constant = constant;
            return *ok;
        }
        case META_CLEAR: {
            meta_clear_constant *source = (meta_clear_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            meta_clear_constant *meta = (meta_clear_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            return *ok;
        }
        case META_CLASS: {
            meta_class_constant *source = (meta_class_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t option = link_constant_offset(context, output_module, input_module, source->option, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            meta_class_constant *meta = (meta_class_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->option = option;
            meta->type = type;
            return *ok;
        }
        case META_ATTR: {
            meta_attr_constant *source = (meta_attr_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t option = link_constant_offset(context, output_module, input_module, source->option, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            meta_attr_constant *meta = (meta_attr_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->option = option;
            meta->type = type;
            return *ok;
        }
        case META_INTERFACE: {
            meta_interface_constant *source = (meta_interface_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t option = link_constant_offset(context, output_module, input_module, source->option, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            meta_interface_constant *meta = (meta_interface_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->option = option;
            meta->type = type;
            return *ok;
        }
        case META_IMPLEMENTS: {
            meta_implements_constant *source = (meta_implements_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t interface_symbol = link_constant_offset(context, output_module, input_module,
                                                           source->interface_symbol, ok);
            meta_implements_constant *meta = (meta_implements_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->interface_symbol = interface_symbol;
            return *ok;
        }
        case META_MEMBER: {
            meta_member_constant *source = (meta_member_constant *)entry;
            size_t owner = link_constant_offset(context, output_module, input_module, source->owner, ok);
            size_t kind = link_constant_offset(context, output_module, input_module, source->kind, ok);
            size_t member = link_constant_offset(context, output_module, input_module, source->member, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            size_t args = link_constant_offset(context, output_module, input_module, source->args, ok);
            meta_member_constant *meta = (meta_member_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->owner = owner;
            meta->kind = kind;
            meta->member = member;
            meta->type = type;
            meta->args = args;
            return *ok;
        }
        case META_INLINE: {
            meta_inline_constant *source = (meta_inline_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t payload = link_constant_offset(context, output_module, input_module, source->payload, ok);
            meta_inline_constant *meta = (meta_inline_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->payload = payload;
            return *ok;
        }
        case META_TASK_TARGET: {
            meta_task_target_constant *source = (meta_task_target_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module,
                                                 source->symbol, ok);
            size_t binding = link_constant_offset(context, output_module, input_module,
                                                  source->binding, ok);
            meta_task_target_constant *meta =
                (meta_task_target_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->binding = binding;
            meta->kind = source->kind;
            return *ok;
        }
        case META_PROVIDER: {
            meta_provider_constant *source =
                (meta_provider_constant *)entry;
            size_t symbol = link_constant_offset(
                    context, output_module, input_module, source->symbol, ok);
            size_t provider = link_constant_offset(
                    context, output_module, input_module, source->provider, ok);
            meta_provider_constant *meta =
                (meta_provider_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->provider = provider;
            meta->flags = source->flags;
            return *ok;
        }
        default:
            *ok = 0;
            return 0;
    }
}

static int link_meta_chain(rxlink_build_context *context, rxlink_output_module *output_module,
                           module_file *input_module, int old_head, int *ok) {
    int old_offset = old_head;
    int new_head = -1;
    int previous_new = -1;

    while (old_offset != -1) {
        chameleon_constant *entry;
        meta_entry *old_meta;
        size_t new_offset;

        if ((size_t)old_offset >= input_module->header.constant_size) {
            *ok = 0;
            return -1;
        }

        entry = (chameleon_constant *)((unsigned char *)input_module->constant + (size_t)old_offset);
        if (!is_meta_constant_type(entry->type)) {
            *ok = 0;
            return -1;
        }

        old_meta = (meta_entry *)entry;
        if (should_strip_meta_constant(context, entry->type)) {
            old_offset = old_meta->next;
            continue;
        }

        if (!reserve_pool_entry(context, entry->size_in_pool, entry->type, &new_offset)) {
            *ok = 0;
            return -1;
        }
        if (!add_const_map(output_module, (size_t)old_offset, new_offset)) {
            *ok = 0;
            return -1;
        }

        memcpy(context->shared_pool.data + new_offset, entry, entry->size_in_pool);
        if (!rewrite_meta_constant(context, output_module, input_module, entry, new_offset, previous_new, -1, ok)) {
            return -1;
        }

        if (previous_new != -1) {
            ((meta_entry *)(context->shared_pool.data + (size_t)previous_new))->next = (int)new_offset;
        } else {
            new_head = (int)new_offset;
        }
        previous_new = (int)new_offset;
        old_offset = old_meta->next;
    }

    return new_head;
}

static int link_constant_offset_int(rxlink_build_context *context, rxlink_output_module *output_module,
                                    module_file *input_module, int old_offset, int *ok) {
    size_t new_offset;

    if (old_offset == -1) return -1;
    new_offset = link_constant_offset(context, output_module, input_module, (size_t)old_offset, ok);
    if (!*ok || new_offset > (size_t)INT_MAX) {
        *ok = 0;
        return -1;
    }
    return (int)new_offset;
}

static size_t link_constant_offset(rxlink_build_context *context, rxlink_output_module *output_module,
                                   module_file *input_module, size_t old_offset, int *ok) {
    chameleon_constant *entry;
    const_map_entry *existing;
    size_t new_offset;

    if (old_offset == SIZE_MAX) return SIZE_MAX;
    if (old_offset >= input_module->header.constant_size) {
        *ok = 0;
        return 0;
    }

    existing = find_const_map(output_module, old_offset);
    if (existing) return existing->new_offset;

    entry = (chameleon_constant *)((unsigned char *)input_module->constant + old_offset);
    switch (entry->type) {
        case STRING_CONST:
        case BINARY_CONST:
        case DECIMAL_CONST:
        case FLOAT_CONST:
            new_offset = dedupe_leaf_constant(context, entry, ok);
            if (!*ok) return 0;
            if (!add_const_map(output_module, old_offset, new_offset)) {
                *ok = 0;
                return 0;
            }
            return new_offset;
        default:
            break;
    }

    if (!reserve_pool_entry(context, entry->size_in_pool, entry->type, &new_offset)) {
        *ok = 0;
        return 0;
    }
    if (!add_const_map(output_module, old_offset, new_offset)) {
        *ok = 0;
        return 0;
    }

    memcpy(context->shared_pool.data + new_offset, entry, entry->size_in_pool);

    switch (entry->type) {
        case PROC_CONST: {
            proc_constant *source = (proc_constant *)entry;
            int next = link_constant_offset_int(context, output_module, input_module, source->next, ok);
            size_t exposed = link_constant_offset(context, output_module, input_module, source->exposed, ok);
            proc_constant *proc = (proc_constant *)(context->shared_pool.data + new_offset);
            proc->next = next;
            proc->exposed = exposed;
            break;
        }
        case EXPOSE_REG_CONST: {
            expose_reg_constant *source = (expose_reg_constant *)entry;
            int next = link_constant_offset_int(context, output_module, input_module, source->next, ok);
            expose_reg_constant *reg = (expose_reg_constant *)(context->shared_pool.data + new_offset);
            reg->next = next;
            break;
        }
        case EXPOSE_PROC_CONST: {
            expose_proc_constant *source = (expose_proc_constant *)entry;
            int next = link_constant_offset_int(context, output_module, input_module, source->next, ok);
            size_t procedure = link_constant_offset(context, output_module, input_module, source->procedure, ok);
            expose_proc_constant *proc = (expose_proc_constant *)(context->shared_pool.data + new_offset);
            proc->next = next;
            proc->procedure = procedure;
            break;
        }
        case META_SOURCE_STEP:
        case META_TRACE_EVENT:
        case META_FUNC:
        case META_REG:
        case META_CONST:
        case META_CLEAR:
        case META_CLASS:
        case META_ATTR:
        case META_INTERFACE:
        case META_IMPLEMENTS:
        case META_MEMBER:
        case META_INLINE:
        case META_TASK_TARGET:
        case META_PROVIDER: {
            meta_entry *meta = (meta_entry *)entry;
            int prev = link_constant_offset_int(context, output_module, input_module, meta->prev, ok);
            int next = link_constant_offset_int(context, output_module, input_module, meta->next, ok);
            if (!rewrite_meta_constant(context, output_module, input_module, entry, new_offset, prev, next, ok)) {
                return 0;
            }
            break;
        }
        default:
            *ok = 0;
            return 0;
    }

    return new_offset;
}

static int rewrite_module_code(rxlink_build_context *context, rxlink_output_module *output_module,
                               const link_module_info *input_info) {
    module_file *input = input_info->module;
    bin_code *input_code;
    bin_code *output_code;
    size_t index;

    output_module->module = malloc(sizeof(module_file));
    if (!output_module->module) {
        RX_REPORT_OOM("malloc rxlink output module", sizeof(module_file),
                      input_info->input_path);
        return 0;
    }
    init_module(output_module->module);
    output_module->source_module = input;
    output_module->module->graph_operands = 1u;
    output_module->module->semantic_graph = input->semantic_graph;
    rx_graph_retain(output_module->module->semantic_graph);
    output_module->module->fromfile = 1;
    output_module->module->header.record_type = RXBIN_RECORD_MODULE_SHARED;
    output_module->module->name = strdup(input->name ? input->name : "");
    output_module->module->description = strdup(input->description ? input->description : "");
    if (!output_module->module->name) {
        RX_REPORT_OOM("strdup rxlink output module name",
                      strlen(input->name ? input->name : "") + 1,
                      input_info->input_path);
        return 0;
    }
    if (!output_module->module->description) {
        RX_REPORT_OOM("strdup rxlink output module description",
                      strlen(input->description ? input->description : "") + 1,
                      input_info->input_path);
        return 0;
    }
    output_module->module->header.name_size = strlen(output_module->module->name) + 1;
    output_module->module->header.description_size = strlen(output_module->module->description) + 1;
    output_module->module->header.instruction_size = input->header.instruction_size;
    output_module->module->header.constant_size = 0;
    output_module->module->header.constant_stored_size = 0;
    output_module->module->header.globals = input->header.globals;
    output_module->module->header.proc_head = -1;
    output_module->module->header.expose_head = -1;
    output_module->module->header.meta_head = -1;
    output_module->module->instructions = 0;
    output_module->module->constant = 0;
    output_module->module->shared_constant_pool = 0;

    if (input->header.instruction_size) {
        output_module->module->instructions = malloc(sizeof(bin_code) * input->header.instruction_size);
        if (!output_module->module->instructions) {
            RX_REPORT_OOM("malloc rxlink output instruction copy",
                          sizeof(bin_code) * input->header.instruction_size,
                          input_info->input_path);
            return 0;
        }
        memcpy(output_module->module->instructions, input->instructions, sizeof(bin_code) * input->header.instruction_size);
    }

    {
        int ok = 1;
        output_module->module->header.proc_head =
                link_constant_offset_int(context, output_module, input, input->header.proc_head, &ok);
        output_module->module->header.expose_head =
                link_constant_offset_int(context, output_module, input, input->header.expose_head, &ok);
        output_module->module->header.meta_head =
                link_meta_chain(context, output_module, input, input->header.meta_head, &ok);
        if (!ok) return 0;
    }

    input_code = (bin_code *)input->instructions;
    output_code = (bin_code *)output_module->module->instructions;
    index = 0;
    while (index < input->header.instruction_size) {
        OpFormat format;
        size_t operand_count;
        size_t operand_index;
        int ok = 1;

        format = rxbin_opcode_format(input_code[index].instruction.opcode);
        operand_count = rxop_format_operand_count(format);
        for (operand_index = 0; operand_index < operand_count; operand_index++) {
            bin_code *operand = &output_code[index + (size_t)operand_index + 1];
            if (rx_graph_operand_kind(input_code[index].instruction.opcode,
                                      (unsigned int)operand_index) != RX_GRAPH_OPERAND_NONE) {
                continue;
            }
            switch (rxop_format_operand_type(format, operand_index)) {
                case OP_FUNC:
                case OP_FLOAT:
                case OP_STRING:
                case OP_DECIMAL:
                case OP_BINARY:
                    operand->index = link_constant_offset(context, output_module, input,
                                                          input_code[index + (size_t)operand_index + 1].index, &ok);
                    break;
                default:
                    break;
            }
            if (!ok) return 0;
        }
        index += (size_t)operand_count + 1;
    }

    return 1;
}

static int build_linked_modules(rxlink_build_context *context, module_list *modules, rxlink_output_list *outputs) {
    size_t i;

    for (i = 0; i < modules->count; i++) {
        rxlink_output_module output_module;
        memset(&output_module, 0, sizeof(output_module));
        if (!modules->items[i].selected || modules->items[i].omitted) continue;
        if (!rewrite_module_code(context, &output_module, &modules->items[i]) ||
            !output_list_append(outputs, &output_module)) {
            if (output_module.module) free_module(output_module.module);
            free(output_module.maps);
            free(output_module.map_hash_slots);
            return 0;
        }
    }

    return 1;
}

static int write_map_file(const module_list *modules,
                          const provider_requirement_list *requirements,
                          const link_config *config) {
    FILE *fp;
    size_t i;

    if (!config->map_path) return 1;

    fp = openfile(config->map_path, "", config->location, "w");
    if (!fp) {
        fprintf(stderr, "ERROR: opening map output %s\n", config->map_path);
        return 0;
    }

    fprintf(fp, "cREXX Link Map\n");
    fprintf(fp, "Selected Modules:\n");
    for (i = 0; i < modules->count; i++) {
        if (!modules->items[i].selected || modules->items[i].omitted) continue;
        fprintf(fp, "  %s from %s\n", modules->items[i].selector_name, modules->items[i].input_path);
        if (modules->items[i].unresolved_imports.count) {
            size_t j;
            fprintf(fp, "    unresolved:\n");
            for (j = 0; j < modules->items[i].unresolved_imports.count; j++) {
                fprintf(fp, "      %s\n", modules->items[i].unresolved_imports.items[j]);
            }
        }
    }
    if (requirements->count) {
        fprintf(fp, "Native Providers:\n");
        for (i = 0u; i < requirements->count; i++) {
            const provider_requirement *requirement =
                &requirements->items[i];
            fprintf(fp, "  %s %s %s from %s\n",
                    (requirement->flags & RXBIN_PROVIDER_REQUIRED)
                        ? "required" : "optional",
                    requirement->provider_id,
                    requirement->symbol,
                    requirement->module_name);
        }
    }

    fclose(fp);
    return 1;
}

static int write_provider_requirements_file(
        const provider_requirement_list *requirements,
        const link_config *config) {
    FILE *fp;
    size_t i;

    if (!config->provider_requirements_path) return 1;
    fp = openfile(config->provider_requirements_path, "",
                  config->location, "w");
    if (!fp) {
        fprintf(stderr, "ERROR: opening provider requirements output %s\n",
                config->provider_requirements_path);
        return 0;
    }
    fprintf(fp, "CREXX-RXPA-REQUIREMENTS 1\n");
    for (i = 0u; i < requirements->count; i++) {
        const provider_requirement *requirement = &requirements->items[i];
        fprintf(fp, "%s\t%s\t%s\t%s\t%s\t%s\n",
                (requirement->flags & RXBIN_PROVIDER_REQUIRED)
                    ? "required" : "optional",
                requirement->provider_id,
                requirement->symbol,
                requirement->type,
                requirement->args,
                requirement->module_name);
    }
    fclose(fp);
    return 1;
}

static void detach_linked_pool(rxlink_output_list *outputs) {
    size_t i;
    for (i = 0; i < outputs->count; i++) {
        outputs->items[i].module->constant = 0;
        outputs->items[i].module->header.constant_size = 0;
        outputs->items[i].module->header.constant_stored_size = 0;
    }
}

static RxGraph *prepare_linked_graph(rxlink_build_context *context,
                                     rxlink_output_list *outputs) {
    module_file **modules;
    RxGraph *graph;
    char *graph_error;
    size_t i;

    if (!outputs->count) return 0;
    modules = (module_file **)calloc(outputs->count, sizeof(*modules));
    if (!modules) return 0;
    for (i = 0; i < outputs->count; i++) {
        modules[i] = outputs->items[i].module;
        modules[i]->constant = context->shared_pool.data;
        modules[i]->header.constant_size = context->shared_pool.size;
        modules[i]->header.constant_stored_size = context->shared_pool.size;
    }
    graph_error = 0;
    graph = rx_graph_build_crexx(modules, outputs->count, &graph_error);
    free(modules);
    if (!graph) {
        fprintf(stderr, "ERROR: rebuilding linked semantic graph: %s\n",
                graph_error ? graph_error : "unknown graph error");
        free(graph_error);
        return 0;
    }
    free(graph_error);
    for (i = 0; i < outputs->count; i++) {
        module_file *source;
        module_file *output;
        bin_code *source_code;
        bin_code *output_code;
        size_t code_index;

        source = outputs->items[i].source_module;
        output = outputs->items[i].module;
        source_code = (bin_code *)source->instructions;
        output_code = (bin_code *)output->instructions;
        code_index = 0u;
        while (code_index < output->header.instruction_size) {
            int opcode;
            size_t operand_count;
            size_t operand_index;

            opcode = output_code[code_index].instruction.opcode;
            operand_count = rxop_format_operand_count(rxbin_opcode_format(opcode));
            for (operand_index = 0; operand_index < operand_count; operand_index++) {
                uint32_t graph_id;
                char *text;
                char *resolve_error;

                if (rx_graph_operand_kind(opcode, (unsigned int)operand_index) ==
                    RX_GRAPH_OPERAND_NONE) continue;
                text = module_instruction_string(
                    source,
                    opcode,
                    (unsigned int)operand_index,
                    source_code[code_index + (size_t)operand_index + 1u].index);
                if (!text) {
                    fprintf(stderr,
                            "ERROR: reading graph operand %d:%zu from module %s\n",
                            opcode,
                            operand_index,
                            source->name ? source->name : "<unnamed>");
                    rx_graph_release(&graph);
                    return 0;
                }
                resolve_error = 0;
                if (!rx_graph_resolve_operand(graph,
                                              opcode,
                                              (unsigned int)operand_index,
                                              text,
                                              &graph_id,
                                              &resolve_error)) {
                    fprintf(stderr,
                            "ERROR: resolving linked graph operand %s: %s\n",
                            text,
                            resolve_error ? resolve_error : "unknown graph error");
                    free(resolve_error);
                    free(text);
                    rx_graph_release(&graph);
                    return 0;
                }
                free(resolve_error);
                free(text);
                output_code[code_index + (size_t)operand_index + 1u].index = graph_id;
            }
            code_index += (size_t)operand_count + 1u;
        }
        output->graph_operands = 1u;
    }
    return graph;
}

static int write_linked_image(const link_config *config, rxlink_build_context *context, rxlink_output_list *outputs) {
    FILE *fp;
    module_file **modules;
    RxGraph *graph;
    size_t i;
    int ok;

    graph = prepare_linked_graph(context, outputs);
    if (!graph) {
        detach_linked_pool(outputs);
        return 0;
    }
    modules = (module_file **)calloc(outputs->count, sizeof(*modules));
    if (!modules) {
        rx_graph_release(&graph);
        detach_linked_pool(outputs);
        return 0;
    }
    for (i = 0; i < outputs->count; i++) modules[i] = outputs->items[i].module;
    fp = openfile(config->output_path, "rxbin", config->location, "wb");
    if (!fp) {
        fprintf(stderr, "ERROR: opening output %s\n", config->output_path);
        free(modules);
        rx_graph_release(&graph);
        detach_linked_pool(outputs);
        return 0;
    }
    ok = write_modules(modules, outputs->count, graph, fp) == 0;
    if (!ok) {
        fprintf(stderr, "ERROR: writing linked RXBIN 007 image: %s\n",
                rxbin_last_error() ? rxbin_last_error() : "unknown RXBIN error");
    }
    fclose(fp);
    free(modules);
    rx_graph_release(&graph);
    detach_linked_pool(outputs);
    return ok;
}

static void print_help(void) {
    printf("cREXX Linker\n");
    printf("Usage: rxlink [options] input_file [input_file ...]\n");
    printf("Options:\n");
    printf("  -o output_stem  Linked output stem or .rxbin file\n");
    printf("  -c control_file Control file with INPUT/ROOT/INCLUDE/OMIT/OUTPUT/MAP/STRIP\n");
    printf("  -r root_member  Root module selector (may be repeated)\n");
    printf("  -m map_file     Write a simple link map\n");
    printf("  -p providers    Write native-provider requirements for packaging\n");
    printf("  -l location     Working location for input/output resolution\n");
    printf("  -s              Strip source/TRACE debug metadata from linked output\n");
    printf("  -i              Preserve inline-body metadata in linked output\n");
    printf("  -d              Debug mode\n");
    printf("  -h              Help\n");
}

int main(int argc, char *argv[]) {
    link_config config;
    module_list modules;
    rxlink_build_context build_context;
    rxlink_output_list outputs;
    provider_requirement_list provider_requirements;
    char *control_path = 0;
    int argi;

    init_link_config(&config);
    memset(&modules, 0, sizeof(modules));
    build_context_init(&build_context);
    memset(&outputs, 0, sizeof(outputs));
    memset(&provider_requirements, 0, sizeof(provider_requirements));

    for (argi = 1; argi < argc && argv[argi][0] == '-'; argi++) {
        if (strlen(argv[argi]) != 2) {
            fprintf(stderr, "ERROR: invalid argument %s\n", argv[argi]);
            goto fail;
        }
        switch (toupper((unsigned char)argv[argi][1])) {
            case 'O':
                if (++argi >= argc || !set_single_path(&config.output_path, argv[argi])) goto fail;
                break;
            case 'C':
                if (++argi >= argc) goto fail;
                control_path = argv[argi];
                break;
            case 'R':
                if (++argi >= argc || !string_list_append(&config.roots, argv[argi])) goto fail;
                break;
            case 'M':
                if (++argi >= argc || !set_single_path(&config.map_path, argv[argi])) goto fail;
                break;
            case 'P':
                if (++argi >= argc ||
                    !set_single_path(&config.provider_requirements_path,
                                     argv[argi])) goto fail;
                break;
            case 'S':
                config.strip_source_metadata = 1;
                break;
            case 'I':
                config.strip_inline_metadata = 0;
                break;
            case 'L':
                if (++argi >= argc) goto fail;
                config.location = argv[argi];
                break;
            case 'D':
                config.debug_mode = 1;
                break;
            case 'H':
            case '?':
                print_help();
                free_link_config(&config);
                build_context_free(&build_context);
                return 0;
            default:
                fprintf(stderr, "ERROR: invalid option %s\n", argv[argi]);
                goto fail;
        }
    }

    if (control_path && !parse_control_file(&config, control_path)) goto fail;
    build_context.strip_source_metadata = config.strip_source_metadata;
    build_context.strip_inline_metadata = config.strip_inline_metadata;

    while (argi < argc) {
        if (!string_list_append(&config.inputs, argv[argi++])) goto fail;
    }

    if (!config.inputs.count) {
        fprintf(stderr, "ERROR: no input files\n");
        goto fail;
    }

    if (!config.output_path) {
        fprintf(stderr, "ERROR: no output file specified\n");
        goto fail;
    }

    if (!load_input_modules(&modules, &config)) goto fail;
    if (!select_modules(&modules, &config)) goto fail;
    if (!rxlink_validate_contracts(&modules)) goto fail;
    if (!collect_provider_requirements(&modules, &provider_requirements)) goto fail;
    if (!build_linked_modules(&build_context, &modules, &outputs)) goto fail;
    if (!outputs.count) {
        fprintf(stderr, "ERROR: no modules selected for output\n");
        goto fail;
    }
    if (!write_linked_image(&config, &build_context, &outputs)) goto fail;
    if (!write_map_file(&modules, &provider_requirements, &config)) goto fail;
    if (!write_provider_requirements_file(&provider_requirements, &config)) goto fail;

    provider_requirement_list_free(&provider_requirements);
    output_list_free(&outputs);
    build_context_free(&build_context);
    module_list_free(&modules);
    free_link_config(&config);
    return 0;

fail:
    provider_requirement_list_free(&provider_requirements);
    output_list_free(&outputs);
    build_context_free(&build_context);
    module_list_free(&modules);
    free_link_config(&config);
    return 1;
}
