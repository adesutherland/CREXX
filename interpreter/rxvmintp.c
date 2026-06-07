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

/*
 * Created by adrian on 29/03/2021.
 */

/*
 * ===================================================================
 * LEVEL C COMPATIBILITY & DESIGN NOTES (StrictAnsiArithmetic Flag)
 * ===================================================================
 * This section documents the arithmetic behaviour that must change when
 * the `StrictAnsiArithmetic` flag is enabled for Level C compatibility.
 *
 * -------------------------------------------------------------------
 * 1. Decimal Arithmetic (% and // operators on `.decimal` type)
 * -------------------------------------------------------------------
 * For Level C, the ANSI "Integer Magnitude-Precision Constraint"
 * must be enforced.
 *
 * CHECK: After calculating the intermediate division result, but
 * before finalizing it, the following check is required:
 *
 * IF LENGTH(TRUNC(intermediate_result)) > DIGITS() THEN
 * RAISE SYNTAX Error 26.11
 *
 * -------------------------------------------------------------------
 * 2. Float Arithmetic (% and // operators on `.float` and `.decimal` types)
 * -------------------------------------------------------------------
 * The current design implements a "Float-First" model:
 *
 * 1. Perform the division using native floating-point arithmetic.
 * 2. Truncate the final result to its integer part.
 *
 * This was chosen over an "Integer-First" model where operands
 * would be truncated to integers *before* the division.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <fcntl.h>
#endif
#ifdef __APPLE__
#include <mach/thread_policy.h>
#include <dlfcn.h>
#endif
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include "platform.h"
#include "rxas.h"
#include "../binutils/include/rxdefs.h"
#include "../binutils/include/rxflags.h"
#include "rxastree.h"
#include "rxvmintp.h"
/* #include <complex.h> */
#include <signal.h>


#include "rxvmvars.h"
#include "rxvmplugin_framework.h"
#include "rxvmsock.h"

int rxvm_link(rxvm_context *ctx);

static int rxvm_insert_attributes_checked(value *array, rxinteger index, rxinteger count) {
    size_t insert_index;
    size_t insert_count;

    if (!array || index < 0 || count < 0) return -1;

    insert_index = (size_t)index;
    insert_count = (size_t)count;
    if (insert_index > array->num_attributes) return -1;
    if (insert_count > SIZE_MAX - array->num_attributes) return -1;

    insert_attributes(array, insert_index, insert_count);
    return 0;
}

static int rxvm_insert_attributes1_checked(value *array, rxinteger index, rxinteger count) {
    if (index <= 0) return -1;
    return rxvm_insert_attributes_checked(array, index - 1, count);
}

static int rxvm_delete_attributes_checked(value *array, rxinteger index, rxinteger count) {
    size_t delete_index;
    size_t delete_count;

    if (!array || index < 0 || count < 0) return -1;

    delete_index = (size_t)index;
    delete_count = (size_t)count;
    if (delete_count == 0) {
        if (delete_index > array->num_attributes) return -1;
        return 0;
    }
    if (delete_index >= array->num_attributes) return -1;
    if (delete_count > array->num_attributes - delete_index) return -1;

    delete_attributes(array, delete_index, delete_count);
    return 0;
}

static int rxvm_delete_attributes1_checked(value *array, rxinteger index, rxinteger count) {
    if (index <= 0) return -1;
    return rxvm_delete_attributes_checked(array, index - 1, count);
}

static int rxvm_reference_storage_in_value_tree(value *root, value *storage) {
    size_t i;

    if (!root || !storage) return 0;
    if (root == storage) return 1;

    if (root->unlinked_attributes) {
        for (i = 0; i < root->max_num_attributes; i++) {
            if (rxvm_reference_storage_in_value_tree(root->unlinked_attributes[i], storage)) return 1;
        }
    }

    return 0;
}

static stack_frame *rxvm_reference_lifetime_owner_frame(stack_frame *frame,
                                                        value *storage) {
    size_t i;

    while (frame) {
        if (frame->procedure) {
            size_t locals = (size_t)frame->procedure->locals;
            size_t globals = frame->procedure->binarySpace
                             ? (size_t)frame->procedure->binarySpace->globals
                             : 0;
            size_t a0_index = locals + globals;

            if (locals > frame->number_locals) locals = frame->number_locals;
            for (i = 0; i < locals; i++) {
                if (rxvm_reference_storage_in_value_tree(frame->baselocals[i], storage)) return frame;
            }

            if (a0_index < frame->number_locals &&
                rxvm_reference_storage_in_value_tree(frame->baselocals[a0_index], storage)) {
                return frame;
            }
        }

        frame = frame->parent;
    }

    return 0;
}

static void rxvm_mark_reference_lifetime_owner(stack_frame *current_frame,
                                               value *storage) {
    stack_frame *owner_frame = rxvm_reference_lifetime_owner_frame(current_frame,
                                                                   storage);
    if (owner_frame) owner_frame->has_reference_lifetimes = 1;
}

static rxvm_ref_owner_kind rxvm_reference_owner_kind_for_storage(stack_frame *frame,
                                                                 size_t register_index,
                                                                 value *storage,
                                                                 void **owner) {
    size_t i;
    size_t locals;
    size_t globals;
    int found_base_storage = 0;

    if (owner) *owner = frame;
    if (!frame || !storage || !frame->procedure) return RXVM_REF_OWNER_NONE;

    locals = (size_t)frame->procedure->locals;
    globals = frame->procedure->binarySpace ? (size_t)frame->procedure->binarySpace->globals : 0;

    for (i = 0; i < frame->number_locals; i++) {
        if (frame->baselocals[i] == storage) {
            register_index = i;
            found_base_storage = 1;
            break;
        }
    }

    if (!found_base_storage) {
        if (owner) *owner = 0;
        return RXVM_REF_ATTRIBUTE;
    }

    if (register_index < locals) return RXVM_REF_LOCAL;
    if (register_index < locals + globals) {
        if (owner) *owner = frame->procedure->binarySpace ? frame->procedure->binarySpace->module : 0;
        return RXVM_REF_GLOBAL;
    }
    if (register_index < frame->number_locals) return RXVM_REF_ARGUMENT;

    return RXVM_REF_ATTRIBUTE;
}

static rxvm_reference_cell *rxvm_reference_payload_cell(value *reference_value) {
    if (!reference_value) return 0;
    return reference_value->reference_payload;
}

static void rxvm_release_frame_reference_lifetimes(stack_frame *frame) {
    size_t i;
    size_t globals;
    size_t a0_index;

    if (!frame || !frame->procedure || !frame->has_reference_lifetimes) return;

    for (i = 0; i < (size_t)frame->procedure->locals; i++) {
        release_value_reference_lifetime(frame->baselocals[i]);
    }

    globals = frame->procedure->binarySpace ? (size_t)frame->procedure->binarySpace->globals : 0;
    a0_index = (size_t)frame->procedure->locals + globals;
    if (a0_index < frame->number_locals) {
        release_value_reference_lifetime(frame->baselocals[a0_index]);
    }

    frame->has_reference_lifetimes = 0;
}

/* This defines the expected max number of args - if a call has more args than
 * this then an oversized block will be malloced
 * In terms of memory usage / waste each one is only 2 x pointer size */
#define NOMINAL_NUM_ARGS 20

/* Define this to use a safe stack frame recycling mechanism - zeros registers in the stack frame */
/*#define SAFE_RECYCLED_STACKFRAMES*/
#undef SAFE_RECYCLED_STACKFRAMES

/* Misc. Utilities here */
static string_constant *get_runtime_string_constant(module *mod, size_t offset);

static value *decimal_literal_value(decplugin *decimal, const char *literal) {
    value *literal_value = value_f();
    decimal->decimalFromString(decimal, literal_value, literal);
    return literal_value;
}

static void free_decimal_literal_value(value *literal_value) {
    clear_value(literal_value);
    free(literal_value);
}

static char *build_runtime_member_name(const char *class_name, size_t class_name_length,
                                       const char *member_name, size_t member_name_length) {
    char *proc_name;

    proc_name = malloc(class_name_length + member_name_length + 2);
    if (!proc_name) return 0;

    memcpy(proc_name, class_name, class_name_length);
    proc_name[class_name_length] = '.';
    memcpy(proc_name + class_name_length + 1, member_name, member_name_length);
    proc_name[class_name_length + member_name_length + 1] = 0;

    return proc_name;
}

static proc_runtime *resolve_runtime_procedure(rxvm_context *context, const char *proc_name, size_t proc_name_length) {
    size_t mod_index;

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod = context->modules[mod_index];
        int meta_ix = mod->meta_head;

        while (meta_ix != -1) {
            meta_entry *meta = (meta_entry *) (mod->segment.const_pool + meta_ix);

            if (meta->base.type == META_FUNC) {
                meta_func_constant *meta_func = (meta_func_constant *) meta;
                string_constant *symbol_name =
                        (string_constant *) (mod->segment.const_pool + meta_func->symbol);

                if (symbol_name->base.type == STRING_CONST &&
                    symbol_name->string_len == proc_name_length &&
                    memcmp(symbol_name->string, proc_name, proc_name_length) == 0) {
                    return rxvm_get_module_runtime_procedure(mod, meta_func->func);
                }
            }

            meta_ix = meta->next;
        }
    }

    return 0;
}

static int compare_runtime_name(const char *left, size_t left_length,
                                const char *right, size_t right_length) {
    size_t min_length;
    int cmp;

    if (!left && !right) return 0;
    if (!left) return -1;
    if (!right) return 1;

    min_length = left_length < right_length ? left_length : right_length;
    cmp = memcmp(left, right, min_length);
    if (cmp) return cmp;
    if (left_length < right_length) return -1;
    if (left_length > right_length) return 1;
    return 0;
}

static char *dup_runtime_name(const char *name, size_t name_length) {
    char *copy;

    copy = malloc(name_length + 1);
    if (!copy) return 0;

    memcpy(copy, name, name_length);
    copy[name_length] = 0;
    return copy;
}

static char *build_interface_factory_error(const char *prefix,
                                           const char *interface_name,
                                           size_t interface_name_length) {
    size_t prefix_length;
    char *buffer;

    prefix_length = strlen(prefix);
    buffer = malloc(prefix_length + interface_name_length + 1);
    if (!buffer) return 0;

    memcpy(buffer, prefix, prefix_length);
    memcpy(buffer + prefix_length, interface_name, interface_name_length);
    buffer[prefix_length + interface_name_length] = 0;
    return buffer;
}

static int runtime_type_name_is_builtin(const char *type_name, size_t type_name_length) {
    static const char *builtins[] = {
            ".boolean", ".int", ".float", ".decimal", ".string",
            ".binary", ".object", ".void", ".unknown", 0
    };
    size_t i;

    if (!type_name) return 0;

    for (i = 0; builtins[i]; i++) {
        if (strlen(builtins[i]) == type_name_length &&
            memcmp(builtins[i], type_name, type_name_length) == 0) {
            return 1;
        }
    }

    return 0;
}

static char *runtime_normalize_type_name(const char *type_name, size_t type_name_length) {
    size_t i;
    size_t out_length;
    char *normalized;
    size_t out_index;
    size_t start;

    if (!type_name) return 0;
    if (runtime_type_name_is_builtin(type_name, type_name_length)) {
        return dup_runtime_name(type_name, type_name_length);
    }

    start = (type_name_length > 0 && type_name[0] == '.') ? 1 : 0;
    normalized = malloc(type_name_length + 1);
    if (!normalized) return 0;

    out_index = 0;
    for (i = start; i < type_name_length; i++) {
        if (i + 1 < type_name_length &&
            ((type_name[i] == ':' && type_name[i + 1] == ':') ||
             (type_name[i] == '.' && type_name[i + 1] == '.'))) {
            normalized[out_index++] = '.';
            i++;
        } else {
            normalized[out_index++] = type_name[i];
        }
    }

    out_length = out_index;
    normalized[out_length] = 0;
    return normalized;
}

static char *runtime_internal_type_to_source_name(const char *type_name, size_t type_name_length) {
    size_t dots = 0;
    size_t i;
    char *source_name;
    size_t out_index;

    if (!type_name) return 0;
    if (runtime_type_name_is_builtin(type_name, type_name_length)) {
        return dup_runtime_name(type_name, type_name_length);
    }

    for (i = 0; i < type_name_length; i++) {
        if (type_name[i] == '.') dots++;
    }

    source_name = malloc(type_name_length + dots + 2);
    if (!source_name) return 0;

    out_index = 0;
    source_name[out_index++] = '.';
    for (i = 0; i < type_name_length; i++) {
        if (type_name[i] == '.') {
            source_name[out_index++] = '.';
            source_name[out_index++] = '.';
        } else {
            source_name[out_index++] = type_name[i];
        }
    }
    source_name[out_index] = 0;

    return source_name;
}

typedef enum runtime_contract_kind {
    RUNTIME_CONTRACT_UNKNOWN = 0,
    RUNTIME_CONTRACT_CLASS,
    RUNTIME_CONTRACT_INTERFACE
} runtime_contract_kind;

static runtime_contract_kind runtime_lookup_contract_kind(rxvm_context *context,
                                                          const char *type_name,
                                                          size_t type_name_length) {
    size_t mod_index;

    if (!context || !type_name || !type_name_length) return RUNTIME_CONTRACT_UNKNOWN;

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod = context->modules[mod_index];
        int meta_ix = mod->meta_head;

        while (meta_ix != -1) {
            meta_entry *meta = (meta_entry *) (mod->segment.const_pool + meta_ix);

            if (meta->base.type == META_CLASS || meta->base.type == META_INTERFACE) {
                size_t symbol_index = 0;
                string_constant *symbol_name;

                if (meta->base.type == META_CLASS) {
                    symbol_index = ((meta_class_constant *) meta)->symbol;
                } else {
                    symbol_index = ((meta_interface_constant *) meta)->symbol;
                }

                symbol_name = get_runtime_string_constant(mod, symbol_index);
                if (symbol_name &&
                    symbol_name->string_len == type_name_length &&
                    memcmp(symbol_name->string, type_name, type_name_length) == 0) {
                    return meta->base.type == META_CLASS ? RUNTIME_CONTRACT_CLASS : RUNTIME_CONTRACT_INTERFACE;
                }
            }

            meta_ix = meta->next;
        }
    }

    return RUNTIME_CONTRACT_UNKNOWN;
}

static int runtime_class_implements_interface(rxvm_context *context,
                                              const char *class_name,
                                              size_t class_name_length,
                                              const char *interface_name,
                                              size_t interface_name_length) {
    size_t mod_index;

    if (!context || !class_name || !interface_name) return 0;

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod = context->modules[mod_index];
        int meta_ix = mod->meta_head;

        while (meta_ix != -1) {
            meta_entry *meta = (meta_entry *) (mod->segment.const_pool + meta_ix);

            if (meta->base.type == META_IMPLEMENTS) {
                meta_implements_constant *impl_meta = (meta_implements_constant *) meta;
                string_constant *class_symbol = get_runtime_string_constant(mod, impl_meta->symbol);
                string_constant *interface_symbol = get_runtime_string_constant(mod, impl_meta->interface_symbol);

                if (class_symbol && interface_symbol &&
                    class_symbol->string_len == class_name_length &&
                    interface_symbol->string_len == interface_name_length &&
                    memcmp(class_symbol->string, class_name, class_name_length) == 0 &&
                    memcmp(interface_symbol->string, interface_name, interface_name_length) == 0) {
                    return 1;
                }
            }

            meta_ix = meta->next;
        }
    }

    return 0;
}

static int runtime_value_matches_object_type(rxvm_context *context,
                                             value *object_value,
                                             const char *type_name,
                                             size_t type_name_length) {
    char *normalized_type_name;
    runtime_contract_kind kind;
    int matches;

    if (!object_value || !type_name || !type_name_length) return 0;
    if (context->link_dirty || context->interface_method_registry_dirty || context->interface_factory_registry_dirty) {
        rxvm_link(context);
    }

    normalized_type_name = runtime_normalize_type_name(type_name, type_name_length);
    if (!normalized_type_name) return 0;

    if (strcmp(normalized_type_name, ".object") == 0) {
        free(normalized_type_name);
        return object_value->object_type_name && object_value->object_type_name_length > 0;
    }

    if (runtime_type_name_is_builtin(normalized_type_name, strlen(normalized_type_name))) {
        free(normalized_type_name);
        return 0;
    }

    matches = 0;
    if (object_value->object_type_name && object_value->object_type_name_length > 0) {
        kind = runtime_lookup_contract_kind(context, normalized_type_name, strlen(normalized_type_name));
        if (kind == RUNTIME_CONTRACT_INTERFACE) {
            matches = runtime_class_implements_interface(context,
                                                         object_value->object_type_name,
                                                         object_value->object_type_name_length,
                                                         normalized_type_name,
                                                         strlen(normalized_type_name));
        } else {
            matches = (object_value->object_type_name_length == strlen(normalized_type_name) &&
                       memcmp(object_value->object_type_name, normalized_type_name,
                              object_value->object_type_name_length) == 0);
        }
    }

    free(normalized_type_name);
    return matches;
}

static char *build_runtime_cast_error(value *object_value,
                                      const char *target_type_name,
                                      size_t target_type_name_length) {
    char *target_source_name;
    char *source_type_name;
    char *buffer;
    size_t buffer_length;

    target_source_name = runtime_internal_type_to_source_name(target_type_name, target_type_name_length);
    if (!target_source_name) return 0;

    if (object_value && object_value->object_type_name && object_value->object_type_name_length > 0) {
        source_type_name = runtime_internal_type_to_source_name(object_value->object_type_name,
                                                                object_value->object_type_name_length);
    } else {
        source_type_name = strdup(".object");
    }

    if (!source_type_name) {
        free(target_source_name);
        return 0;
    }

    buffer_length = strlen("Cannot cast ") + strlen(source_type_name) + strlen(" to ") + strlen(target_source_name) + 1;
    buffer = malloc(buffer_length);
    if (buffer) {
        snprintf(buffer, buffer_length, "Cannot cast %s to %s", source_type_name, target_source_name);
    }
    free(source_type_name);
    free(target_source_name);
    return buffer;
}

static void clear_runtime_interface_factories(rxvm_context *context) {
    size_t i;

    if (!context || !context->interface_factories) {
        if (context) {
            context->num_interface_factories = 0;
            context->interface_factory_capacity = 0;
        }
        return;
    }

    for (i = 0; i < context->num_interface_factories; i++) {
        free(context->interface_factories[i].interface_name);
        free(context->interface_factories[i].factory_name);
        free(context->interface_factories[i].class_name);
    }

    free(context->interface_factories);
    context->interface_factories = 0;
    context->num_interface_factories = 0;
    context->interface_factory_capacity = 0;
}

static void clear_runtime_interface_methods(rxvm_context *context) {
    size_t i;

    if (!context || !context->interface_methods) {
        if (context) {
            context->num_interface_methods = 0;
            context->interface_method_capacity = 0;
        }
        return;
    }

    for (i = 0; i < context->num_interface_methods; i++) {
        free(context->interface_methods[i].class_name);
        free(context->interface_methods[i].member_name);
    }

    free(context->interface_methods);
    context->interface_methods = 0;
    context->num_interface_methods = 0;
    context->interface_method_capacity = 0;
}

static int runtime_member_kind_is_method(const string_constant *kind_symbol) {
    if (!kind_symbol || kind_symbol->string_len < 6) return 0;
    return memcmp(kind_symbol->string, "method", 6) == 0;
}

static int runtime_member_kind_is_final(const string_constant *kind_symbol) {
    static const char final_flag[] = "final";
    size_t i;

    if (!runtime_member_kind_is_method(kind_symbol)) return 0;
    if (kind_symbol->string_len < sizeof(final_flag) - 1) return 0;

    for (i = 0; i + (sizeof(final_flag) - 1) <= kind_symbol->string_len; i++) {
        if (memcmp(kind_symbol->string + i, final_flag, sizeof(final_flag) - 1) == 0) {
            return 1;
        }
    }

    return 0;
}

static string_constant *get_runtime_string_constant(module *mod, size_t offset) {
    string_constant *entry;

    if (!mod || offset >= mod->segment.const_size) return 0;
    entry = (string_constant *) (mod->segment.const_pool + offset);
    if (entry->base.type != STRING_CONST) return 0;
    return entry;
}

typedef struct rxvm_source_context {
    string_constant *file;
    string_constant *source;
    size_t line;
    size_t column;
    size_t active_end_column;
    uint32_t step_id;
    uint32_t clause_id;
    uint32_t flags;
} rxvm_source_context;

static void resolve_runtime_source_context(module *mod, size_t address, rxvm_source_context *source_context) {
    int meta_ix;

    source_context->file = 0;
    source_context->source = 0;
    source_context->line = 0;
    source_context->column = 0;
    source_context->active_end_column = 0;
    source_context->step_id = 0;
    source_context->clause_id = 0;
    source_context->flags = 0;

    if (!mod) return;

    meta_ix = mod->meta_head;
    while (meta_ix != -1) {
        meta_entry *meta = (meta_entry *) (mod->segment.const_pool + meta_ix);
        if (meta->address > address) break;

        if (meta->base.type == META_SOURCE_STEP) {
            meta_source_step_constant *step_meta = (meta_source_step_constant *) meta;
            source_context->file = get_runtime_string_constant(mod, step_meta->file);
            source_context->source = get_runtime_string_constant(mod, step_meta->source_line);
            source_context->line = step_meta->line;
            source_context->column = step_meta->active_start_column;
            source_context->active_end_column = step_meta->active_end_column;
            source_context->step_id = step_meta->step_id;
            source_context->clause_id = step_meta->clause_id;
            source_context->flags = step_meta->flags;
        }

        meta_ix = meta->next;
    }
}

static void print_runtime_panic_location(rxvm_context *context, rxinteger module_number, rxinteger address) {
    module *mod;
    rxvm_source_context source_context;
    size_t module_index;
    size_t instruction_address;

    if (!context || module_number <= 0 || address < 0) return;

    module_index = (size_t) module_number - 1;
    if (module_index >= context->num_modules) return;

    mod = context->modules[module_index];
    instruction_address = (size_t) address;

    fprintf(stderr, "  at module %zu", (size_t) module_number);
    if (mod && mod->name) fprintf(stderr, " (%s)", mod->name);
    fprintf(stderr, " address %zu (0x%zx)\n", instruction_address, instruction_address);

    resolve_runtime_source_context(mod, instruction_address, &source_context);
    if (source_context.source) {
        fprintf(stderr, "  source: ");
        if (source_context.file) {
            fprintf(stderr, "%.*s:", (int) source_context.file->string_len, source_context.file->string);
        }
        fprintf(stderr, "%zu:%zu: %.*s\n",
                source_context.line,
                source_context.column,
                (int) source_context.source->string_len,
                source_context.source->string);
    }
}

static char *build_runtime_factory_proc_name(const char *class_name,
                                             size_t class_name_length,
                                             const char *factory_name,
                                             size_t factory_name_length) {
    static const char default_factory_name[] = "\xc2\xa7" "factory";
    static const char factory_prefix[] = "\xc2\xa7" "factory.";
    size_t prefix_length;
    char *proc_name;

    if (!class_name || !class_name_length || !factory_name || !factory_name_length) return 0;

    if (factory_name_length == 1 && factory_name[0] == '*') {
        return build_runtime_member_name(class_name, class_name_length,
                                         default_factory_name, sizeof(default_factory_name) - 1);
    }

    prefix_length = sizeof(factory_prefix) - 1;
    proc_name = malloc(class_name_length + 1 + prefix_length + factory_name_length + 1);
    if (!proc_name) return 0;

    memcpy(proc_name, class_name, class_name_length);
    proc_name[class_name_length] = '.';
    memcpy(proc_name + class_name_length + 1, factory_prefix, prefix_length);
    memcpy(proc_name + class_name_length + 1 + prefix_length, factory_name, factory_name_length);
    proc_name[class_name_length + 1 + prefix_length + factory_name_length] = 0;

    return proc_name;
}

static char *build_runtime_match_proc_name(const char *class_name,
                                           size_t class_name_length,
                                           const char *factory_name,
                                           size_t factory_name_length) {
    static const char default_match_name[] = "\xc2\xa7" "match";
    static const char match_prefix[] = "\xc2\xa7" "match.";
    size_t prefix_length;
    char *proc_name;

    if (!class_name || !class_name_length || !factory_name || !factory_name_length) return 0;

    if (factory_name_length == 1 && factory_name[0] == '*') {
        return build_runtime_member_name(class_name, class_name_length,
                                         default_match_name, sizeof(default_match_name) - 1);
    }

    prefix_length = sizeof(match_prefix) - 1;
    proc_name = malloc(class_name_length + 1 + prefix_length + factory_name_length + 1);
    if (!proc_name) return 0;

    memcpy(proc_name, class_name, class_name_length);
    proc_name[class_name_length] = '.';
    memcpy(proc_name + class_name_length + 1, match_prefix, prefix_length);
    memcpy(proc_name + class_name_length + 1 + prefix_length, factory_name, factory_name_length);
    proc_name[class_name_length + 1 + prefix_length + factory_name_length] = 0;

    return proc_name;
}

static int invoke_runtime_factory_match(rxvm_context *context,
                                        proc_runtime *match_proc,
                                        rxinteger argc,
                                        value **args,
                                        rxinteger *score_out) {
    proc_runtime *saved_ext_proc;
    int saved_ext_argc;
    value **saved_ext_args;
    value *saved_ext_ret;
    value *match_ret;
    char *dummy_argv[] = {"rxvm_factory_match"};

    if (score_out) *score_out = 1;
    if (!context || !match_proc) return 1;

    saved_ext_proc = context->ext_proc;
    saved_ext_argc = context->ext_argc;
    saved_ext_args = context->ext_args;
    saved_ext_ret = context->ext_ret;

    context->ext_proc = match_proc;
    context->ext_argc = (int) argc;
    context->ext_args = args;
    context->ext_ret = value_f();
    match_ret = context->ext_ret;
    if (!match_ret) {
        context->ext_proc = saved_ext_proc;
        context->ext_argc = saved_ext_argc;
        context->ext_args = saved_ext_args;
        context->ext_ret = saved_ext_ret;
        return 0;
    }

    run(context, 0, dummy_argv);
    if (score_out) *score_out = match_ret->int_value;

    clear_value(match_ret);
    free(match_ret);

    context->ext_proc = saved_ext_proc;
    context->ext_argc = saved_ext_argc;
    context->ext_args = saved_ext_args;
    context->ext_ret = saved_ext_ret;

    return 1;
}

static int add_runtime_interface_factory_entry(rxvm_context *context,
                                               const char *interface_name,
                                               size_t interface_name_length,
                                               const char *factory_name,
                                               size_t factory_name_length,
                                               const char *class_name,
                                               size_t class_name_length,
                                               proc_runtime *match_proc,
                                               proc_runtime *factory_proc) {
    rxvm_interface_factory_entry *entry;

    if (!context || !interface_name || !factory_name || !class_name || !factory_proc) return 0;

    if (context->num_interface_factories >= context->interface_factory_capacity) {
        size_t new_capacity;
        rxvm_interface_factory_entry *new_entries;

        new_capacity = context->interface_factory_capacity ? context->interface_factory_capacity * 2 : 16;
        new_entries = realloc(context->interface_factories,
                              sizeof(rxvm_interface_factory_entry) * new_capacity);
        if (!new_entries) return 0;

        context->interface_factories = new_entries;
        context->interface_factory_capacity = new_capacity;
    }

    entry = &context->interface_factories[context->num_interface_factories];
    memset(entry, 0, sizeof(*entry));

    entry->interface_name = dup_runtime_name(interface_name, interface_name_length);
    entry->factory_name = dup_runtime_name(factory_name, factory_name_length);
    entry->class_name = dup_runtime_name(class_name, class_name_length);
    if (!entry->interface_name || !entry->factory_name || !entry->class_name) {
        free(entry->interface_name);
        free(entry->factory_name);
        free(entry->class_name);
        memset(entry, 0, sizeof(*entry));
        return 0;
    }

    entry->interface_name_length = interface_name_length;
    entry->factory_name_length = factory_name_length;
    entry->class_name_length = class_name_length;
    entry->match_proc = match_proc;
    entry->factory_proc = factory_proc;
    context->num_interface_factories++;

    return 1;
}

static int add_runtime_interface_method_entry(rxvm_context *context,
                                              const char *class_name,
                                              size_t class_name_length,
                                              const char *member_name,
                                              size_t member_name_length,
                                              proc_runtime *method_proc) {
    rxvm_interface_method_entry *entry;
    size_t i;

    if (!context || !class_name || !member_name || !method_proc) return 0;

    for (i = 0; i < context->num_interface_methods; i++) {
        entry = &context->interface_methods[i];
        if (entry->class_name_length == class_name_length &&
            entry->member_name_length == member_name_length &&
            memcmp(entry->class_name, class_name, class_name_length) == 0 &&
            memcmp(entry->member_name, member_name, member_name_length) == 0) {
            entry->method_proc = method_proc;
            return 1;
        }
    }

    if (context->num_interface_methods >= context->interface_method_capacity) {
        size_t new_capacity;
        rxvm_interface_method_entry *new_entries;

        new_capacity = context->interface_method_capacity ? context->interface_method_capacity * 2 : 32;
        new_entries = realloc(context->interface_methods,
                              sizeof(rxvm_interface_method_entry) * new_capacity);
        if (!new_entries) return 0;

        context->interface_methods = new_entries;
        context->interface_method_capacity = new_capacity;
    }

    entry = &context->interface_methods[context->num_interface_methods];
    memset(entry, 0, sizeof(*entry));

    entry->class_name = dup_runtime_name(class_name, class_name_length);
    entry->member_name = dup_runtime_name(member_name, member_name_length);
    if (!entry->class_name || !entry->member_name) {
        free(entry->class_name);
        free(entry->member_name);
        memset(entry, 0, sizeof(*entry));
        return 0;
    }

    entry->class_name_length = class_name_length;
    entry->member_name_length = member_name_length;
    entry->method_proc = method_proc;
    context->num_interface_methods++;

    return 1;
}

void rxvm_rebuild_interface_method_registry(rxvm_context *context) {
    size_t mod_index;

    if (!context) return;

    clear_runtime_interface_methods(context);

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod;
        int meta_ix;

        mod = context->modules[mod_index];
        meta_ix = mod->meta_head;

        while (meta_ix != -1) {
            meta_entry *meta;

            meta = (meta_entry *) (mod->segment.const_pool + meta_ix);
            if (meta->base.type == META_IMPLEMENTS) {
                meta_implements_constant *impl_meta;
                string_constant *class_symbol;
                string_constant *interface_symbol;
                size_t iface_mod_index;

                impl_meta = (meta_implements_constant *) meta;
                class_symbol = get_runtime_string_constant(mod, impl_meta->symbol);
                interface_symbol = get_runtime_string_constant(mod, impl_meta->interface_symbol);

                if (!class_symbol || !interface_symbol) {
                    meta_ix = meta->next;
                    continue;
                }

                for (iface_mod_index = 0; iface_mod_index < context->num_modules; iface_mod_index++) {
                    module *iface_mod;
                    int iface_meta_ix;

                    iface_mod = context->modules[iface_mod_index];
                    iface_meta_ix = iface_mod->meta_head;

                    while (iface_meta_ix != -1) {
                        meta_entry *iface_meta;

                        iface_meta = (meta_entry *) (iface_mod->segment.const_pool + iface_meta_ix);
                        if (iface_meta->base.type == META_MEMBER) {
                            meta_member_constant *member_meta;
                            string_constant *owner_symbol;
                            string_constant *kind_symbol;
                            string_constant *member_symbol;

                            member_meta = (meta_member_constant *) iface_meta;
                            owner_symbol = get_runtime_string_constant(iface_mod, member_meta->owner);
                            kind_symbol = get_runtime_string_constant(iface_mod, member_meta->kind);
                            member_symbol = get_runtime_string_constant(iface_mod, member_meta->member);

                            if (owner_symbol && kind_symbol && member_symbol &&
                                runtime_member_kind_is_method(kind_symbol) &&
                                owner_symbol->string_len == interface_symbol->string_len &&
                                memcmp(owner_symbol->string, interface_symbol->string, interface_symbol->string_len) == 0) {
                                char *class_proc_name;
                                char *interface_proc_name;
                                proc_runtime *class_proc;
                                proc_runtime *interface_proc;
                                proc_runtime *effective_proc;

                                class_proc_name = build_runtime_member_name(class_symbol->string,
                                                                            class_symbol->string_len,
                                                                            member_symbol->string,
                                                                            member_symbol->string_len);
                                interface_proc_name = build_runtime_member_name(interface_symbol->string,
                                                                                interface_symbol->string_len,
                                                                                member_symbol->string,
                                                                                member_symbol->string_len);
                                if (!class_proc_name || !interface_proc_name) {
                                    if (class_proc_name) free(class_proc_name);
                                    if (interface_proc_name) free(interface_proc_name);
                                    iface_meta_ix = iface_meta->next;
                                    continue;
                                }

                                class_proc = resolve_runtime_procedure(context, class_proc_name, strlen(class_proc_name));
                                interface_proc = resolve_runtime_procedure(context, interface_proc_name, strlen(interface_proc_name));
                                free(class_proc_name);
                                free(interface_proc_name);

                                effective_proc = class_proc;
                                if (!effective_proc && runtime_member_kind_is_final(kind_symbol)) {
                                    effective_proc = interface_proc;
                                }

                                if (effective_proc) {
                                    add_runtime_interface_method_entry(context,
                                                                       class_symbol->string,
                                                                       class_symbol->string_len,
                                                                       member_symbol->string,
                                                                       member_symbol->string_len,
                                                                       effective_proc);
                                }
                            }
                        }

                        iface_meta_ix = iface_meta->next;
                    }
                }
            }

            meta_ix = meta->next;
        }
    }
}

static proc_runtime *resolve_runtime_method(rxvm_context *context,
                                            const char *class_name,
                                            size_t class_name_length,
                                            const char *member_name,
                                            size_t member_name_length) {
    size_t entry_index;
    char *proc_name;
    proc_runtime *called_function;

    if (!context || !class_name || !class_name_length || !member_name || !member_name_length) return 0;

    if (context->link_dirty || context->interface_method_registry_dirty || context->interface_factory_registry_dirty) {
        rxvm_link(context);
    }

    for (entry_index = 0; entry_index < context->num_interface_methods; entry_index++) {
        rxvm_interface_method_entry *entry;

        entry = &context->interface_methods[entry_index];
        if (entry->class_name_length == class_name_length &&
            entry->member_name_length == member_name_length &&
            memcmp(entry->class_name, class_name, class_name_length) == 0 &&
            memcmp(entry->member_name, member_name, member_name_length) == 0) {
            return entry->method_proc;
        }
    }

    proc_name = build_runtime_member_name(class_name, class_name_length, member_name, member_name_length);
    if (!proc_name) return 0;
    called_function = resolve_runtime_procedure(context, proc_name, strlen(proc_name));
    free(proc_name);
    return called_function;
}

void rxvm_rebuild_interface_factory_registry(rxvm_context *context) {
    size_t mod_index;

    if (!context) return;

    clear_runtime_interface_factories(context);

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod;
        int meta_ix;

        mod = context->modules[mod_index];
        meta_ix = mod->meta_head;

        while (meta_ix != -1) {
            meta_entry *meta;

            meta = (meta_entry *) (mod->segment.const_pool + meta_ix);
            if (meta->base.type == META_IMPLEMENTS) {
                meta_implements_constant *impl_meta;
                string_constant *class_symbol;
                string_constant *interface_symbol;
                size_t iface_mod_index;

                impl_meta = (meta_implements_constant *) meta;
                class_symbol = get_runtime_string_constant(mod, impl_meta->symbol);
                interface_symbol = get_runtime_string_constant(mod, impl_meta->interface_symbol);

                if (!class_symbol || !interface_symbol) {
                    meta_ix = meta->next;
                    continue;
                }

                for (iface_mod_index = 0; iface_mod_index < context->num_modules; iface_mod_index++) {
                    module *iface_mod;
                    int iface_meta_ix;

                    iface_mod = context->modules[iface_mod_index];
                    iface_meta_ix = iface_mod->meta_head;

                    while (iface_meta_ix != -1) {
                        meta_entry *iface_meta;

                        iface_meta = (meta_entry *) (iface_mod->segment.const_pool + iface_meta_ix);
                        if (iface_meta->base.type == META_MEMBER) {
                            meta_member_constant *member_meta;
                            string_constant *owner_symbol;
                            string_constant *kind_symbol;
                            string_constant *member_symbol;

                            member_meta = (meta_member_constant *) iface_meta;
                            owner_symbol = get_runtime_string_constant(iface_mod, member_meta->owner);
                            kind_symbol = get_runtime_string_constant(iface_mod, member_meta->kind);
                            member_symbol = get_runtime_string_constant(iface_mod, member_meta->member);

                            if (owner_symbol && kind_symbol && member_symbol &&
                                kind_symbol->string_len == 7 &&
                                memcmp(kind_symbol->string, "factory", 7) == 0 &&
                                owner_symbol->string_len == interface_symbol->string_len &&
                                memcmp(owner_symbol->string, interface_symbol->string, interface_symbol->string_len) == 0) {
                                char *factory_proc_name;
                                char *match_proc_name;
                                proc_runtime *factory_proc;
                                proc_runtime *match_proc;

                                factory_proc_name = build_runtime_factory_proc_name(class_symbol->string,
                                                                                    class_symbol->string_len,
                                                                                    member_symbol->string,
                                                                                    member_symbol->string_len);
                                match_proc_name = build_runtime_match_proc_name(class_symbol->string,
                                                                               class_symbol->string_len,
                                                                               member_symbol->string,
                                                                               member_symbol->string_len);
                                if (!factory_proc_name) {
                                    if (match_proc_name) free(match_proc_name);
                                    iface_meta_ix = iface_meta->next;
                                    continue;
                                }

                                factory_proc = resolve_runtime_procedure(context, factory_proc_name, strlen(factory_proc_name));
                                match_proc = match_proc_name ?
                                             resolve_runtime_procedure(context, match_proc_name, strlen(match_proc_name)) :
                                             0;
                                free(factory_proc_name);
                                if (match_proc_name) free(match_proc_name);

                                if (factory_proc) {
                                    add_runtime_interface_factory_entry(context,
                                                                        interface_symbol->string,
                                                                        interface_symbol->string_len,
                                                                        member_symbol->string,
                                                                        member_symbol->string_len,
                                                                        class_symbol->string,
                                                                        class_symbol->string_len,
                                                                        match_proc,
                                                                        factory_proc);
                                }
                            }
                        }

                        iface_meta_ix = iface_meta->next;
                    }
                }
            }

            meta_ix = meta->next;
        }
    }
}

static void parse_runtime_factory_selector(const char *selector,
                                           size_t selector_length,
                                           const char **interface_name,
                                           size_t *interface_name_length,
                                           const char **factory_name,
                                           size_t *factory_name_length) {
    const char *selector_start;
    const char *selector_end;
    const char *sep;

    if (interface_name) *interface_name = selector;
    if (interface_name_length) *interface_name_length = selector_length;
    if (factory_name) *factory_name = "*";
    if (factory_name_length) *factory_name_length = 1;

    if (!selector || !selector_length) return;

    selector_start = selector;
    selector_end = selector_start + selector_length;
    sep = 0;
    while (!sep && selector + 1 < selector_end) {
        if ((selector[0] == ':' && selector[1] == ':') ||
            (selector[0] == '.' && selector[1] == '.')) {
            sep = selector;
            break;
        }
        selector++;
    }
    if (!sep) return;

    if (interface_name) *interface_name = selector_start;
    if (interface_name_length) *interface_name_length = (size_t) (sep - selector_start);
    if (factory_name) *factory_name = sep + 2;
    if (factory_name_length) *factory_name_length = selector_length - ((size_t) (sep - selector_start) + 2);
}

static int resolve_runtime_factory(rxvm_context *context,
                                   const char *selector,
                                   size_t selector_length,
                                   rxinteger argc,
                                   value **args,
                                   proc_runtime **factory_out,
                                   char **error_out) {
    const char *interface_name;
    size_t interface_name_length;
    const char *factory_name;
    size_t factory_name_length;
    size_t entry_index;
    proc_runtime *best_factory;
    rxinteger best_score;
    char *best_class_name;
    size_t best_class_name_length;
    int saw_candidate;
    int saw_positive_score;

    if (factory_out) *factory_out = 0;
    if (error_out) *error_out = 0;
    if (!context || !selector || !selector_length || !factory_out) return 0;

    if (context->link_dirty || context->interface_factory_registry_dirty) {
        rxvm_link(context);
    }

    parse_runtime_factory_selector(selector, selector_length,
                                   &interface_name, &interface_name_length,
                                   &factory_name, &factory_name_length);
    if (!interface_name_length || !factory_name_length) {
        if (error_out) *error_out = build_interface_factory_error("No interface factory providers for ", selector, selector_length);
        return 0;
    }

    best_factory = 0;
    best_score = 0;
    best_class_name = 0;
    best_class_name_length = 0;
    saw_candidate = 0;
    saw_positive_score = 0;

    for (entry_index = 0; entry_index < context->num_interface_factories; entry_index++) {
        rxvm_interface_factory_entry *entry;
        rxinteger score;

        entry = &context->interface_factories[entry_index];
        if (entry->interface_name_length != interface_name_length ||
            memcmp(entry->interface_name, interface_name, interface_name_length) != 0 ||
            entry->factory_name_length != factory_name_length ||
            memcmp(entry->factory_name, factory_name, factory_name_length) != 0) {
            continue;
        }

        saw_candidate = 1;
        score = 1;
        if (entry->match_proc &&
            !invoke_runtime_factory_match(context, entry->match_proc, argc, args, &score)) {
            if (error_out) *error_out = build_interface_factory_error("Failed to evaluate interface factory match for ", selector, selector_length);
            if (best_class_name) free(best_class_name);
            return 0;
        }
        if (score > 0) {
            saw_positive_score = 1;
            if (!best_factory ||
                score > best_score ||
                (score == best_score &&
                 compare_runtime_name(entry->class_name, entry->class_name_length,
                                      best_class_name, best_class_name_length) < 0)) {
                char *new_best_name;

                new_best_name = dup_runtime_name(entry->class_name, entry->class_name_length);
                if (!new_best_name) {
                    if (error_out) *error_out = build_interface_factory_error("Failed to resolve factory provider for ", selector, selector_length);
                    if (best_class_name) free(best_class_name);
                    return 0;
                }

                if (best_class_name) free(best_class_name);
                best_class_name = new_best_name;
                best_class_name_length = entry->class_name_length;
                best_score = score;
                best_factory = entry->factory_proc;
            }
        }
    }

    if (best_class_name) free(best_class_name);

    if (!saw_candidate) {
        if (error_out) *error_out = build_interface_factory_error("No interface factory providers for ", selector, selector_length);
        return 0;
    }

    if (!saw_positive_score || !best_factory) {
        if (error_out) *error_out = build_interface_factory_error("No matching interface factory provider for ", selector, selector_length);
        return 0;
    }

    *factory_out = best_factory;
    return 1;
}

/* Constant to get create the compile time data in ta "iso" like format */
/* __DATE__ format "Mmm dd yyyy" -> Convert to yyyymmdd */
const char compile_date[8+1] =
        {
                /* yyyy year */
                __DATE__[7], __DATE__[8],
                __DATE__[9], __DATE__[10],

                /* First month letter, Oct Nov Dec = '1' otherwise '0' */
                (__DATE__[0] == 'O' || __DATE__[0] == 'N' || __DATE__[0] == 'D') ? '1' : '0',

                /* Second month letter */
                (__DATE__[0] == 'J') ? ( (__DATE__[1] == 'a') ? '1' :       /* Jan, Jun or Jul */
                                         ((__DATE__[2] == 'n') ? '6' : '7') ) :
                (__DATE__[0] == 'F') ? '2' :                                /* Feb */
                (__DATE__[0] == 'M') ? (__DATE__[2] == 'r') ? '3' : '5' :   /* Mar or May */
                (__DATE__[0] == 'A') ? (__DATE__[1] == 'p') ? '4' : '8' :   /* Apr or Aug */
                (__DATE__[0] == 'S') ? '9' :                                /* Sep */
                (__DATE__[0] == 'O') ? '0' :                                /* Oct */
                (__DATE__[0] == 'N') ? '1' :                                /* Nov */
                (__DATE__[0] == 'D') ? '2' :                                /* Dec */
                0,

                /* First day letter, replace space with digit */
                __DATE__[4]==' ' ? '0' : __DATE__[4],

                /* Second day letter */
                __DATE__[5],

                '\0'
        };

/* Fast integer pow calculation - loop unwound - based / from https://gist.github.com/orlp/3551590
 * by Orson Peters / orlp / Leiden, Netherlands / orsonpeters@gmail.com
 * Returns the result or 0 for overflow / underflow
 * todo can overflow on 32 bit (update the table) */
RX_INLINE rxinteger ipow(rxinteger base, rxinteger exp_int) {
    static const uint8_t highest_bit_set[] = {
            0, 1, 2, 2, 3, 3, 3, 3,
            4, 4, 4, 4, 4, 4, 4, 4,
            5, 5, 5, 5, 5, 5, 5, 5,
            5, 5, 5, 5, 5, 5, 5, 5,
            6, 6, 6, 6, 6, 6, 6, 6,
            6, 6, 6, 6, 6, 6, 6, 6,
            6, 6, 6, 6, 6, 6, 6, 6,
            6, 6, 6, 6, 6, 6, 6, 255, // anything past 63 is a guaranteed overflow with base > 1
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255,
    };

    /* Adrian added this bit to check for negative / oversize exp
     * the original used uint8 for exp so not needed */
    if (exp_int > 255 || exp_int < 0) {
        if (base == 1) {
            return 1;
        }

        if (base == -1) {
            return 1 - 2 * (exp_int & 1);
        }

        return 0; /* Overflow / underflow */
    }

    rxinteger result = 1;
    uint8_t exp = (uint8_t) exp_int;

    switch (highest_bit_set[exp]) {
        case 255: // we use 255 as an overflow marker and return 0 on overflow/underflow
            if (base == 1) {
                return 1;
            }

            if (base == -1) {
                return 1 - 2 * (exp & 1);
            }

            return 0; /* Overflow / underflow */
        case 6:
            if (exp & 1) result *= base;
            exp >>= 1;
            base *= base;
        case 5:
            if (exp & 1) result *= base;
            exp >>= 1;
            base *= base;
        case 4:
            if (exp & 1) result *= base;
            exp >>= 1;
            base *= base;
        case 3:
            if (exp & 1) result *= base;
            exp >>= 1;
            base *= base;
        case 2:
            if (exp & 1) result *= base;
            exp >>= 1;
            base *= base;
        case 1:
            if (exp & 1) result *= base;
        default:
            return result;
    }
}

/* Function to convert an interrupt to a string: interrupt_entry -> Code Description Massage */
const char *interrupt_to_string(unsigned char interrupt) {
    switch (interrupt) {
        case RXSIGNAL_KILL:
            return "KILL";
        case RXSIGNAL_ERROR:
            return "ERROR";
        case RXSIGNAL_OVERFLOW_UNDERFLOW:
            return"OVERFLOW_UNDERFLOW";
        case RXSIGNAL_CONVERSION_ERROR:
            return "CONVERSION_ERROR";
        case RXSIGNAL_UNKNOWN_INSTRUCTION:
            return "UNKNOWN_INSTRUCTION";
        case RXSIGNAL_FUNCTION_NOT_FOUND:
            return "FUNCTION_NOT_FOUND";
        case RXSIGNAL_NOT_IMPLEMENTED:
            return "NOT_IMPLEMENTED";
        case RXSIGNAL_INVALID_SIGNAL_CODE:
            return "INVALID_SIGNAL_CODE";
        case RXSIGNAL_REFERENCE_INVALID:
            return "REFERENCE_INVALID";
        case RXSIGNAL_OUT_OF_RANGE:
            return "OUT_OF_RANGE";
        case RXSIGNAL_FAILURE:
            return "FAILURE";
        case RXSIGNAL_QUIT:
            return "QUIT";
        case RXSIGNAL_TERM:
            return "TERM";
        case RXSIGNAL_NOTREADY:
            return "NOTREADY";
        case RXSIGNAL_INVALID_ARGUMENTS:
            return "INVALID_ARGUMENTS";
        case RXSIGNAL_DIVISION_BY_ZERO:
            return "DIVISION_BY_ZERO";
        case RXSIGNAL_UNICODE_ERROR:
            return "UNICODE_ERROR";
        case RXSIGNAL_POSIX_HUP:
            return "POSIX_HUP";
        case RXSIGNAL_POSIX_INT:
            return "POSIX_INT";
        case RXSIGNAL_POSIX_USR1:
            return "POSIX_USR1";
        case RXSIGNAL_POSIX_USR2:
            return "POSIX_USR2";;
        case RXSIGNAL_POSIX_CHLD:
            return "POSIX_CHLD";
        case RXSIGNAL_BREAKPOINT:
            return "BREAKPOINT";
        case RXSIGNAL_OTHER:
            return "OTHER";
        default:
            return 0; // Invalid Signal Code
    }
}

/* Function to convert a string to an interrupt: Code Description Massage -> interrupt_entry */
unsigned char string_to_interrupt(const char *interrupt) {
    if (strcmp(interrupt, "KILL") == 0) return RXSIGNAL_KILL;
    if (strcmp(interrupt, "ERROR") == 0) return RXSIGNAL_ERROR;
    if (strcmp(interrupt, "OVERFLOW_UNDERFLOW") == 0) return RXSIGNAL_OVERFLOW_UNDERFLOW;
    if (strcmp(interrupt, "CONVERSION_ERROR") == 0) return RXSIGNAL_CONVERSION_ERROR;
    if (strcmp(interrupt, "UNKNOWN_INSTRUCTION") == 0) return RXSIGNAL_UNKNOWN_INSTRUCTION;
    if (strcmp(interrupt, "FUNCTION_NOT_FOUND") == 0) return RXSIGNAL_FUNCTION_NOT_FOUND;
    if (strcmp(interrupt, "NOT_IMPLEMENTED") == 0) return RXSIGNAL_NOT_IMPLEMENTED;
    if (strcmp(interrupt, "INVALID_SIGNAL_CODE") == 0) return RXSIGNAL_INVALID_SIGNAL_CODE;
    if (strcmp(interrupt, "REFERENCE_INVALID") == 0) return RXSIGNAL_REFERENCE_INVALID;
    if (strcmp(interrupt, "OUT_OF_RANGE") == 0) return RXSIGNAL_OUT_OF_RANGE;
    if (strcmp(interrupt, "FAILURE") == 0) return RXSIGNAL_FAILURE;
    if (strcmp(interrupt, "QUIT") == 0) return RXSIGNAL_QUIT;
    if (strcmp(interrupt, "TERM") == 0) return RXSIGNAL_TERM;
    if (strcmp(interrupt, "NOTREADY") == 0) return RXSIGNAL_NOTREADY;
    if (strcmp(interrupt, "INVALID_ARGUMENTS") == 0) return RXSIGNAL_INVALID_ARGUMENTS;
    if (strcmp(interrupt, "DIVISION_BY_ZERO") == 0) return RXSIGNAL_DIVISION_BY_ZERO;
    if (strcmp(interrupt, "UNICODE_ERROR") == 0) return RXSIGNAL_UNICODE_ERROR;
    if (strcmp(interrupt, "POSIX_HUP") == 0) return RXSIGNAL_POSIX_HUP;
    if (strcmp(interrupt, "POSIX_INT") == 0) return RXSIGNAL_POSIX_INT;
    if (strcmp(interrupt, "POSIX_USR1") == 0) return RXSIGNAL_POSIX_USR1;
    if (strcmp(interrupt, "POSIX_USR2") == 0) return RXSIGNAL_POSIX_USR2;
    if (strcmp(interrupt, "POSIX_CHLD") == 0) return RXSIGNAL_POSIX_CHLD;
    if (strcmp(interrupt, "BREAKPOINT") == 0) return RXSIGNAL_BREAKPOINT;
    if (strcmp(interrupt, "OTHER") == 0) return RXSIGNAL_OTHER;
    return RXSIGNAL_MAX; // Invalid Signal Code
}

typedef enum rxsignal_handler_action {
    RXSIGNAL_HANDLER_ACTION_NONE = 0,
    RXSIGNAL_HANDLER_ACTION_SKIP,
    RXSIGNAL_HANDLER_ACTION_FAIL,
    RXSIGNAL_HANDLER_ACTION_RETRY
} rxsignal_handler_action;

static rxsignal_handler_action rxsignal_read_handler_action(value *action) {
    if (!action || action->string_length <= 0) return RXSIGNAL_HANDLER_ACTION_NONE;

    null_terminate_string_buffer(action);
    if (strcmp(action->string_value, "__rxsignal_skip") == 0) return RXSIGNAL_HANDLER_ACTION_SKIP;
    if (strcmp(action->string_value, "__rxsignal_fail") == 0) return RXSIGNAL_HANDLER_ACTION_FAIL;
    if (strcmp(action->string_value, "__rxsignal_retry") == 0) return RXSIGNAL_HANDLER_ACTION_RETRY;
    return RXSIGNAL_HANDLER_ACTION_NONE;
}

static value *rxsignal_handler_payload(stack_frame *handler_frame) {
    size_t arg_index;
    value *arg;

    if (!handler_frame || !handler_frame->procedure || !handler_frame->procedure->binarySpace) return NULL;
    arg_index = handler_frame->procedure->binarySpace->globals + handler_frame->procedure->locals + 1;
    if (arg_index >= handler_frame->number_locals) return NULL;
    arg = handler_frame->baselocals[arg_index];
    if (!arg || arg->num_attributes < 5 || !arg->attributes) return NULL;
    return arg->attributes[4];
}

static void rxsignal_apply_native_interrupt_mode(unsigned char sig, interrupt_entry *entry) {
    if (!entry || sig == 0 || sig >= RXSIGNAL_MAX) return;
    if (entry->response == RXSIGNAL_RESPONSE_IGNORE) {
        if (sig != RXSIGNAL_KILL) ignore_interrupt((int)sig);
    } else {
        enable_interrupt((int)sig);
    }
}

static void rxsignal_push_handler(stack_frame *frame, unsigned char sig) {
    interrupt_saved_entry *saved;

    if (!frame || sig == 0 || sig >= RXSIGNAL_MAX) return;

    saved = malloc(sizeof(interrupt_saved_entry));
    if (!saved) return;
    saved->signal = sig;
    saved->entry = frame->interrupt_table[sig - 1];
    saved->next = frame->interrupt_stack;
    frame->interrupt_stack = saved;
}

static int rxsignal_pop_handler(stack_frame *frame, unsigned char sig) {
    interrupt_saved_entry *saved;
    interrupt_saved_entry *previous;

    if (!frame || sig == 0 || sig >= RXSIGNAL_MAX) return 0;

    previous = 0;
    saved = frame->interrupt_stack;
    while (saved) {
        if (saved->signal == sig) {
            frame->interrupt_table[sig - 1] = saved->entry;
            rxsignal_apply_native_interrupt_mode(sig, &frame->interrupt_table[sig - 1]);
            if (previous) previous->next = saved->next;
            else frame->interrupt_stack = saved->next;
            free(saved);
            return 1;
        }
        previous = saved;
        saved = saved->next;
    }

    return 0;
}

static void rxsignal_clear_handler_stack(stack_frame *frame) {
    if (!frame) return;
    while (frame->interrupt_stack) {
        rxsignal_pop_handler(frame, frame->interrupt_stack->signal);
    }
}

/* Stack Frame Factory */
RX_INLINE stack_frame *frame_f(
                    proc_runtime *procedure,
                    int no_args,
                    stack_frame *parent,
                    bin_code *return_pc,
                    value *return_reg) {
    stack_frame *this;
    int num_locals;
    int nominal_num_locals;
    int i, j;
    size_t frame_size;
    value *value_buffer;

    if (procedure->binarySpace == 0) {
        num_locals = procedure->locals + no_args + 1;
        nominal_num_locals = procedure->locals + NOMINAL_NUM_ARGS + 1;
    } else {
        num_locals = procedure->locals + procedure->binarySpace->globals + no_args + 1;
        nominal_num_locals = procedure->locals + procedure->binarySpace->globals + NOMINAL_NUM_ARGS + 1;
    }

    /* Do we need an oversized block */
    if (num_locals > nominal_num_locals) nominal_num_locals = num_locals;

    if (*procedure->frame_free_list &&
        (*procedure->frame_free_list)->nominal_number_locals >= num_locals) {

        /* We can reuse this stack frame */
        this = *procedure->frame_free_list;
        *procedure->frame_free_list = this->prev_free;
        this->prev_free = 0;

        /* Reset Local Registers */
        for (i = 0; i < procedure->locals; i++) {
            this->locals[i] = this->baselocals[i];
#ifdef SAFE_RECYCLED_STACKFRAMES
            value_zero(this->locals[i]);
#endif
        }
        /* Make sure global registers are linked correctly */
        if (procedure->binarySpace) {
            for (j = 0; j < procedure->binarySpace->globals; i++, j++) {
                this->locals[i] = this->baselocals[i];
            }
        }
        /* Reset register a0 - number of arguments */
        this->locals[i] = this->baselocals[i];
        value_zero(this->locals[i]);
        this->locals[i]->int_value = no_args;
    }
    else {
        /* Need a new stack frame - allocate all the memory in one go */
        frame_size = sizeof(stack_frame) +
                     ( sizeof(value*) * nominal_num_locals * 2 ) +
                     ( sizeof(value) * (procedure->locals + 1)); /* +1 is for a0 */

        this = (stack_frame *) malloc( frame_size );
        this->prev_free = 0;

        this->baselocals = (value**)(this + 1);
        this->locals = this->baselocals + nominal_num_locals;
        value_buffer = (value*)(this->locals + nominal_num_locals);

        /* Link Locals */
        for (i = 0; i < procedure->locals; i++, value_buffer++) {
            value_init(value_buffer);
            this->locals[i] = value_buffer;
            this->baselocals[i] = value_buffer;
        }

        /* Link Globals */
        if (procedure->binarySpace) {
            for (j = 0; j < procedure->binarySpace->globals; i++, j++) {
                this->baselocals[i] =  procedure->binarySpace->module->globals[j];
                this->locals[i] = procedure->binarySpace->module->globals[j];
            }
        }

        /* Link a0 */
        value_init(value_buffer);
        this->locals[i] = value_buffer;
        this->baselocals[i] = value_buffer;
        this->locals[i]->int_value = no_args;

        this->nominal_number_locals = nominal_num_locals;
    }
    this->parent = parent;
    if (parent) {
        /* Set the interrupt mask based on parent settings */
        memcpy(this->interrupt_table, parent->interrupt_table, sizeof(interrupt_entry) * (RXSIGNAL_MAX));
        this->is_interrupt = parent->is_interrupt;

        /* VM Plugins */
        this->unicode = parent->unicode;
        this->decimal = parent->decimal;

        /* Copy the numeric context */
        this->num_context = parent->num_context;

        // Set the numeric context of the decimal plugin
        if (this->decimal) {
            this->decimal->num_context = &this->num_context;
            this->decimal->syncNumericContext(this->decimal);
        }
    }
    else {
        for (i = 0; i < RXSIGNAL_MAX; i++) {
            this->interrupt_table[i].function = 0;
            this->interrupt_table[i].jump = 0;
            this->interrupt_table[i].frame = 0;
            this->interrupt_table[i].value_register = 0;
        }

        /* Set up the default interrupt mask */
        this->interrupt_table[RXSIGNAL_KILL-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_ERROR-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_OVERFLOW_UNDERFLOW-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_CONVERSION_ERROR-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_UNKNOWN_INSTRUCTION-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_NOT_IMPLEMENTED-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_FUNCTION_NOT_FOUND-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_REFERENCE_INVALID-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_OUT_OF_RANGE-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_FAILURE-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_QUIT-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_TERM-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_NOTREADY-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_INVALID_ARGUMENTS-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_DIVISION_BY_ZERO-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_UNICODE_ERROR-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_POSIX_HUP-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_POSIX_INT-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_POSIX_USR1-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_POSIX_USR2-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_POSIX_CHLD-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_BREAKPOINT-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_OTHER-1].response = RXSIGNAL_RESPONSE_HALT;
        this->is_interrupt = 0; // No signals pending

        /* VM Plugins */
        this->unicode = 0;
        this->decimal = 0;

        /* Default numeric context */
        this->num_context.digits = DIGITS_STRIKE_POINT;
        this->num_context.fuzz = 0;
        this->num_context.form = NUMERIC_FORM_SCIENTIFIC;
        this->num_context.casetype = CASE_LOWER;
        this->num_context.standard = NUMERIC_STANDARD_COMMON;
    }
    this->interrupt_stack = 0;
    this->decimal_loaded_here = 0;
    this->unicode_loaded_here = 0;
    this->return_pc = return_pc;
    this->number_locals = num_locals;
    this->number_args = no_args;
    this->return_reg = return_reg;
    this->procedure = procedure;
    this->has_reference_lifetimes = 0;
    this->is_interrupt_action = 0;

    return this;
}

/* Clear Stack Frame - deallocating register contents and plugins */
RX_INLINE void clear_frame(stack_frame *frame) {
    int i, offset;
    rxsignal_clear_handler_stack(frame);
    /* Reset Local Registers and a0 */
    for (i = 0; i < frame->procedure->locals; i++) {
        clear_value(frame->baselocals[i]);
    }
    offset = frame->procedure->locals;
    if (frame->procedure->binarySpace) offset += frame->procedure->binarySpace->globals;
    clear_value(frame->baselocals[offset]);
    if (frame->decimal_loaded_here) {
        frame->decimal->base.free((rxvm_plugin*)frame->decimal);
        frame->decimal_loaded_here = 0;
    }
    if (frame->unicode_loaded_here) {
        frame->unicode->base.free((rxvm_plugin*)frame->unicode);
        frame->unicode_loaded_here = 0;
    }
}

/* Free Stack Frame */
RX_INLINE void free_frame(stack_frame *frame) {
    rxsignal_clear_handler_stack(frame);
    rxvm_release_frame_reference_lifetimes(frame);
    /* Add to free list */
    frame->prev_free = *(frame->procedure->frame_free_list);
    *(frame->procedure->frame_free_list) = frame;
}

static stack_frame *rxsignal_unwind_to_frame(stack_frame *current, stack_frame *target) {
    stack_frame *discard;

    if (!target) return current;

    while (current && current != target) {
        discard = current;
        current = current->parent;
        free_frame(discard);
    }

    return current ? current : target;
}

static void rxsignal_populate_raw_interrupt(value *raw,
                                            unsigned char interrupt,
                                            rxinteger module,
                                            rxinteger address,
                                            value *payload) {
    value_zero(raw);
    set_num_attributes(raw, 5);
    raw->attributes[0]->int_value = (rxinteger)interrupt;
    raw->attributes[1]->int_value = module;
    raw->attributes[2]->int_value = address;
    set_null_string(raw->attributes[3], interrupt_to_string(interrupt));
    move_value(raw->attributes[4], payload);
}

static void rxsignal_populate_runtime_signal(value *dest, value *raw) {
    static char runtime_signal_type[] = "rxfnsb.runtime_signal";

    value_zero(dest);
    set_num_attributes(dest, 6);
    dest->object_type_name = runtime_signal_type;
    dest->object_type_name_length = strlen(runtime_signal_type);
    dest->attributes[0]->int_value = 1;
    copy_value(dest->attributes[4], raw);
}

void completely_free_frame(stack_frame *frame) {
    clear_frame(frame);
    free(frame);
}

// Bit field of raised VM interrupts (checked by the interpreter)
static volatile sig_atomic_t interrupts = 0;

// Function to set an interrupt
void raise_signal(unsigned char signal) {
    interrupts |= 1 << (signal - 1);
}

// Function to clear an interrupt
void clear_signal(unsigned char signal) {
    interrupts &= ~(1 << (signal - 1));
}

// Macro to detect and throw a signal if a RXVM plugin-raised error is present
#define RXSIGNAL_IF_RXVM_PLUGIN_ERROR(signal) \
if ((signal)->base.signal_number  && (signal)->base.signal_number < RXSIGNAL_MAX) { \
if (!current_frame->is_interrupt) interrupted_pc = pc; \
interrupts |= 1 << ((signal)->base.signal_number - 1); \
value_zero(interrupt_object[(signal)->base.signal_number]); \
set_null_string(interrupt_object[(signal)->base.signal_number], (signal)->base.signal_string); \
}

// Macro to throw a signal
#define SET_SIGNAL(signal) \
{if (!current_frame->is_interrupt) interrupted_pc = pc; \
interrupts |= 1 << ((signal) - 1); \
value_zero(interrupt_object[(signal)]);}

// Macro to throw a signal with a message
#define SET_SIGNAL_MSG(signal, message) \
{if (!current_frame->is_interrupt) interrupted_pc = pc; \
interrupts |= 1 << ((signal) - 1); \
value_zero(interrupt_object[(signal)]); \
set_null_string(interrupt_object[(signal)], (message));}

// Macro to throw a signal with a payload
#define SET_SIGNAL_PAYLOAD(signal, payload) \
{if (!current_frame->is_interrupt) interrupted_pc = pc; \
interrupts |= 1 << ((signal) - 1); \
copy_value(interrupt_object[(signal)], (payload));}

#define SET_SIGNAL_FROM_NAME(name) \
{ unsigned char signal__ = string_to_interrupt((name)); \
if (signal__ == RXSIGNAL_MAX) { SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE); } \
else { SET_SIGNAL(signal__); } }

#define SET_SIGNAL_MSG_FROM_NAME(name, message) \
{ unsigned char signal__ = string_to_interrupt((name)); \
if (signal__ == RXSIGNAL_MAX) { SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE); } \
else { SET_SIGNAL_MSG(signal__, (message)); } }

#define SET_SIGNAL_PAYLOAD_FROM_NAME(name, payload) \
{ unsigned char signal__ = string_to_interrupt((name)); \
if (signal__ == RXSIGNAL_MAX) { SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE); } \
else { SET_SIGNAL_PAYLOAD(signal__, (payload)); } }

#ifndef NUTF8
#define REQUIRE_VALID_UTF8_REGISTER(reg) \
do { \
    if (!has_utf8_valid_count_or_empty((reg))) refresh_utf8_flags((reg)); \
    if (!has_utf8_valid_count_or_empty((reg))) { \
        SET_SIGNAL_MSG(RXSIGNAL_UNICODE_ERROR, "Invalid UTF-8 string operand"); \
        DISPATCH; \
    } \
} while (0)
#else
#define REQUIRE_VALID_UTF8_REGISTER(reg) do { } while (0)
#endif

// Macro and function to detect and throw a signal if a RXPA plugin-raised error is present
#define INTERRUPT_FROM_RXPA_SIGNAL(signal) if ((signal)->int_value || (signal)->string_length) { if (!current_frame->is_interrupt) interrupted_pc = pc; interrupt_from_rxpa_signal(signal,interrupt_object); }

void interrupt_from_rxpa_signal(value *signal, value* interrupt_object[RXSIGNAL_MAX]) {
    size_t int_num;

    if (signal->int_value < 1 || signal->int_value >= RXSIGNAL_MAX) {
        null_terminate_string_buffer(signal);
        int_num = string_to_interrupt(signal->string_value);
        if (int_num == RXSIGNAL_MAX) {
            int_num = RXSIGNAL_OTHER;
            value_zero(interrupt_object[int_num]);
            set_null_string(interrupt_object[int_num], signal->string_value);
        }
        else {
            value_zero(interrupt_object[int_num]);
        }
    } else {
        int_num = signal->int_value;
        value_zero(interrupt_object[int_num]);
    }

    // Set the interrupt
    interrupts |= 1 << (int_num - 1);
}

#define HANDLE_INTERRUPT_ACTION_RETURN() \
if (is_interrupt && temp_frame->is_interrupt_action) { \
    rxsignal_handler_action action__ = rxsignal_read_handler_action(interrupt_action_value); \
    if (action__ == RXSIGNAL_HANDLER_ACTION_RETRY) { \
        if (current_frame && current_frame->procedure && current_frame->procedure->binarySpace) { \
            next_pc = current_frame->procedure->binarySpace->binary + last_interrupted_address[is_interrupt]; \
        } \
    } else if (action__ != RXSIGNAL_HANDLER_ACTION_SKIP) { \
        value *payload__ = rxsignal_handler_payload(temp_frame); \
        if (payload__ && payload__->string_length) { \
            fprintf(stderr, "PANIC: %.*s (SIGNAL %s)\n", (int)(payload__->string_length), payload__->string_value, interrupt_to_string(is_interrupt)); \
        } else { \
            fprintf(stderr, "PANIC: (SIGNAL %s)\n", interrupt_to_string(is_interrupt)); \
        } \
        print_runtime_panic_location(context, last_interrupted_module[is_interrupt], last_interrupted_address[is_interrupt]); \
        value_zero(interrupt_action_value); \
        rc = (int)is_interrupt; \
        free_frame(temp_frame); \
        goto interprt_finished; \
    } \
    value_zero(interrupt_action_value); \
}

#define IS_UNICODE_WHITESPACE(cp) ( \
    (cp) == 0x0009 || /* Tab */       \
    (cp) == 0x000A || /* Line Feed */ \
    (cp) == 0x000B || /* Vertical Tab */ \
    (cp) == 0x000C || /* Form Feed */ \
    (cp) == 0x000D || /* Carriage Return */ \
    (cp) == 0x0020 || /* Space */     \
    (cp) == 0x0085 || /* Next Line */ \
    (cp) == 0x00A0 || /* No-Break Space */ \
    (cp) == 0x1680 || \
    (cp) == 0x180E || \
    ((cp) >= 0x2000 && (cp) <= 0x200A) || \
    (cp) == 0x2028 || \
    (cp) == 0x2029 || \
    (cp) == 0x202F || \
    (cp) == 0x205F || \
    (cp) == 0x3000 )

/* -------------------------------------------------------------------------
 * Inline helper: forward ASCII non-blank scan.
 * Returns index (0-based) of first non-blank, or -len if none found.
 * -------------------------------------------------------------------------
 */
#define ASCII_FAST_PATH 1    // 1. activate ASCII fast path, 0: run normal mode
#if  ASCII_FAST_PATH
RX_INLINE rxinteger ascii_fwd_nonblank(const unsigned char *s, rxinteger start, rxinteger len) {
    rxinteger i;
    int ch;
    for (i = start; i < len; i++) {
        ch = (unsigned char)s[i];
       if (!IS_UNICODE_WHITESPACE(ch)) return i;
    }
    return -1;  /* Not found in forward scan */
}
RX_INLINE rxinteger ascii_back_nonblank( unsigned char *s, rxinteger start, rxinteger len) {
    rxinteger i;
    int ch;
    if (len <= 0) return -1;
    for (i = start; i >= 0; --i) {
        ch = (unsigned char)s[i];
        if (!IS_UNICODE_WHITESPACE(ch)) return i;
    }
    return -1;  /* Not found in reverse scan */
}

RX_INLINE rxinteger ascii_fwd_blank(const unsigned char *s, rxinteger start, rxinteger len) {
    rxinteger i;
    int ch;
    for (i = start; i < len; i++) {
        ch = (unsigned char)s[i];
        if (IS_UNICODE_WHITESPACE(ch)) return i;
    }
    return -1;  /* Not found in forward scan */
}
RX_INLINE rxinteger ascii_back_blank( unsigned char *s, rxinteger start, rxinteger len) {
    rxinteger i;
    int ch;
    if (len <= 0) return -1;
    for (i = start; i >= 0; --i) {
        ch = (unsigned char)s[i];
        if (IS_UNICODE_WHITESPACE(ch)) return i;
    }
    return -1;  /* Not found in reverse scan */
}
#endif

/* Interpreter */
RX_FLATTEN int run(rxvm_context *context, int argc, char *argv[]) {
    proc_runtime *procedure;
    proc_runtime *step_handler = 0;
    int rc = 0;
    unsigned int initSeed = 0;   /* keep last seed for Random function within REXX run */
    char hasSeed = 0; /* no seed set */
    bin_code *pc, *next_pc;
    bin_code *interrupted_pc = 0;
    int mod_index;
    value *interrupt_arg;
    value *signal_value = value_f();
    value *interrupt_action_value = value_f();
    unsigned char signal_code = 0;
    value *arguments_array = 0;                /* note that the needs mallocing / freeing */
    unsigned char last_interrupt = 0; /* Interrupt being handled */
    /* Array of objects attached to raised interrupts */
    value *interrupt_object[RXSIGNAL_MAX];
    /* Array of addresses that were last interrupted by interrupt number */
    rxinteger last_interrupted_address[RXSIGNAL_MAX] = {0};
    /* Array of modules that were last interrupted by interrupt number */
    rxinteger last_interrupted_module[RXSIGNAL_MAX] = {0};
    stack_frame *current_frame = 0, *temp_frame;
    /* 3 Work Registers */
    value *work1 = value_f();
    value *work2 = value_f();
    value *work3 = value_f();
#ifdef NTHREADED
    void *next_inst = 0;
#else
    module *current_module = 0;
    void *next_inst = &&IUNKNOWN;
#endif

    /* Set up the interrupt object array */
    {
        size_t i;
        for (i = 0; i < RXSIGNAL_MAX; i++) {
            interrupt_object[i] = value_f();
        }
    }
    /* Initialize the native signal handler system */
    initialize_vm_signals();

    /*
     * Instruction database - loaded from a generated header file
     */
#define FMT_EMPTY_MAP 0, OP_NONE, OP_NONE, OP_NONE
#define FMT_B_MAP 1, OP_BINARY, OP_NONE, OP_NONE
#define FMT_C_MAP 1, OP_CHAR, OP_NONE, OP_NONE
#define FMT_F_MAP 1, OP_FLOAT, OP_NONE, OP_NONE
#define FMT_I_MAP 1, OP_INT, OP_NONE, OP_NONE
#define FMT_I_I_MAP 2, OP_INT, OP_INT, OP_NONE
#define FMT_I_I_I_MAP 3, OP_INT, OP_INT, OP_INT
#define FMT_I_I_R_MAP 3, OP_INT, OP_INT, OP_REG
#define FMT_I_R_MAP 2, OP_INT, OP_REG, OP_NONE
#define FMT_I_R_R_MAP 3, OP_INT, OP_REG, OP_REG
#define FMT_L_MAP 1, OP_ID, OP_NONE, OP_NONE
#define FMT_L_L_R_MAP 3, OP_ID, OP_ID, OP_REG
#define FMT_L_P_S_MAP 3, OP_ID, OP_FUNC, OP_STRING
#define FMT_L_R_MAP 2, OP_ID, OP_REG, OP_NONE
#define FMT_L_R_I_MAP 3, OP_ID, OP_REG, OP_INT
#define FMT_L_R_R_MAP 3, OP_ID, OP_REG, OP_REG
#define FMT_L_R_S_MAP 3, OP_ID, OP_REG, OP_STRING
#define FMT_L_S_MAP 2, OP_ID, OP_STRING, OP_NONE
#define FMT_P_MAP 1, OP_FUNC, OP_NONE, OP_NONE
#define FMT_P_S_MAP 2, OP_FUNC, OP_STRING, OP_NONE
#define FMT_R_MAP 1, OP_REG, OP_NONE, OP_NONE
#define FMT_R_B_MAP 2, OP_REG, OP_BINARY, OP_NONE
#define FMT_R_C_MAP 2, OP_REG, OP_CHAR, OP_NONE
#define FMT_R_D_MAP 2, OP_REG, OP_DECIMAL, OP_NONE
#define FMT_R_D_R_MAP 3, OP_REG, OP_DECIMAL, OP_REG
#define FMT_R_F_MAP 2, OP_REG, OP_FLOAT, OP_NONE
#define FMT_R_F_I_MAP 3, OP_REG, OP_FLOAT, OP_INT
#define FMT_R_F_R_MAP 3, OP_REG, OP_FLOAT, OP_REG
#define FMT_R_I_MAP 2, OP_REG, OP_INT, OP_NONE
#define FMT_R_I_I_MAP 3, OP_REG, OP_INT, OP_INT
#define FMT_R_I_R_MAP 3, OP_REG, OP_INT, OP_REG
#define FMT_R_P_MAP 2, OP_REG, OP_FUNC, OP_NONE
#define FMT_R_P_R_MAP 3, OP_REG, OP_FUNC, OP_REG
#define FMT_R_R_MAP 2, OP_REG, OP_REG, OP_NONE
#define FMT_R_R_D_MAP 3, OP_REG, OP_REG, OP_DECIMAL
#define FMT_R_R_F_MAP 3, OP_REG, OP_REG, OP_FLOAT
#define FMT_R_R_I_MAP 3, OP_REG, OP_REG, OP_INT
#define FMT_R_R_R_MAP 3, OP_REG, OP_REG, OP_REG
#define FMT_R_R_S_MAP 3, OP_REG, OP_REG, OP_STRING
#define FMT_R_S_MAP 2, OP_REG, OP_STRING, OP_NONE
#define FMT_R_S_I_MAP 3, OP_REG, OP_STRING, OP_INT
#define FMT_R_S_R_MAP 3, OP_REG, OP_STRING, OP_REG
#define FMT_R_S_S_MAP 3, OP_REG, OP_STRING, OP_STRING
#define FMT_S_MAP 1, OP_STRING, OP_NONE, OP_NONE
#define FMT_S_R_MAP 2, OP_STRING, OP_REG, OP_NONE
#define FMT_S_S_MAP 2, OP_STRING, OP_STRING, OP_NONE
#define FMT_S_S_R_MAP 3, OP_STRING, OP_STRING, OP_REG

const Instruction meta_map[OP_MAX_INSTRUCTIONS] = {
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) \
    { OPCODE, #NAME, DESC, FMT##_MAP },
#include "../binutils/include/rxops.h"
#undef X
};

typedef Opcode instructions;

#ifdef NTHREADED
/* already typedefed */
#else
const void *address_map[OP_MAX_INSTRUCTIONS] = {
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) \
    &&NAME,
#include "../binutils/include/rxops.h"
#undef X
};
#endif

    /* Allocate Interrupt Arg */
    interrupt_arg = value_f();

    /* Thread code - we need to do it here because address_map is only valid
     * in this run() function */
    DEBUG("Threading/Preparing\n");
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        /* Idempotent check */
        if (context->modules[mod_index]->state >= RXVM_MOD_THREADED) continue;

#ifndef NTHREADED
        {
            module *mod = context->modules[mod_index];
            size_t i = 0, j;
            if (!mod->prepared_dispatch && mod->segment.inst_size) {
                mod->prepared_dispatch = malloc(sizeof(void *) * mod->segment.inst_size);
                if (!mod->prepared_dispatch) {
                    RX_PANIC_OOM("malloc rxvm prepared dispatch table",
                                 sizeof(void *) * mod->segment.inst_size,
                                 mod->name);
                }
            }
            while (i < context->modules[mod_index]->segment.inst_size) {
                j = i;
                i += context->modules[mod_index]->segment.binary[i].instruction.no_ops + 1;
                mod->prepared_dispatch[j] =
                        (void *)address_map[context->modules[mod_index]->segment
                                .binary[j].instruction.opcode];
            }
        }
#endif
        context->modules[mod_index]->state = RXVM_MOD_THREADED;
    }

    if (context->prepare_only) {
        /* We are only here to thread, return success */
        rc = 0;
        goto interprt_finished;
    }

    /* Find the program's entry point */
    DEBUG("Find program entry point\n");
    if (context->ext_proc) {
        procedure = context->ext_proc;
    } else {
        for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
            int i = context->modules[mod_index]->proc_head;
            while (i != -1) {
                proc_constant *definition =
                        (proc_constant *) (context->modules[mod_index]->segment.const_pool +
                                           i);
                if (definition->base.type == PROC_CONST &&
                    strcmp(definition->name, "main") == 0) {
                    procedure = rxvm_get_module_runtime_procedure(context->modules[mod_index], (size_t)i);
                    break;
                }
                i = definition->next;
                procedure = 0;
            }
            if (procedure) break;
        }
    }

    if (!procedure) {
        DEBUG("main() not found\n");
        goto interprt_finished;
    }

    DEBUG("Create first Stack Frame\n");
    if (context->ext_proc) {
        current_frame = frame_f(procedure, context->ext_argc, 0, 0, context->ext_ret);
        /* Arguments (passed as individual objects) */
        {
            int i;
            int a1 = procedure->binarySpace->globals + procedure->locals + 1;
            for (i = 0; i < context->ext_argc; i++) {
                current_frame->baselocals[a1 + i] = value_f();
                current_frame->locals[a1 + i] = current_frame->baselocals[a1 + i];
                copy_value(current_frame->baselocals[a1 + i], context->ext_args[i]);
            }
        }
    } else {
        current_frame = frame_f(procedure, 1, 0, 0, 0);
        /* Arguments (passed in an array) */
        /* a0 is already set by frame_f() */
        /* a1 is the array  */
        {
            int i;
            int a1 = procedure->binarySpace->globals + procedure->locals + 1;
            arguments_array = value_f();
            current_frame->baselocals[a1] = arguments_array;
            current_frame->locals[a1] = current_frame->baselocals[a1];
            set_num_attributes(current_frame->baselocals[a1], argc);

            for (i = 0; i < argc; i++) {
                set_null_string(current_frame->baselocals[a1]->attributes[i], argv[i]);
            }
        }
    }

    /* Load VM Plugins */
    DEBUG("Load VM Plugins\n");
    current_frame->decimal = (decplugin*)get_rxvmplugin(RXVM_PLUGIN_DECIMAL);
    if (!current_frame->decimal) {
        printf("PANIC - No default decimal plugin\n");
        exit(255); // Documented 255 is for missing decimal plugin
    }
    current_frame->decimal_loaded_here = 0;

    // Set the numeric context of the decimal plugin
    current_frame->decimal->num_context = &current_frame->num_context;
    current_frame->decimal->syncNumericContext(current_frame->decimal);

    /* Start */
    DEBUG("Starting inst# %s-0x%x\n", procedure->binarySpace->module->name, (int) procedure->start);
#ifndef NTHREADED
    current_module = current_frame->procedure->binarySpace->module;
#endif
    next_pc = &(current_frame->procedure->binarySpace->binary[procedure->start]);

    CALC_DISPATCH_MANUAL
    DISPATCH

    /* Instruction implementations */
    /* ----------------------------------------------------------------------------
     * The following shortcut macros are used in the instruction implementation
     *      op1R   address the first register operand
     *      op2R   address the second register operand
     *      op3R   address the third  register operand
     *
     *      op1RI  integer of first register operand
     *      op2RI  integer of second register operand
     *      op3RI  integer of third register operand
     *
     *      op1RF  float of first register operand
     *      op2RF  float of second register operand
     *      op3RF  float of third register operand
     *
     *      op1I   integer value of first operand (non-register value)
     *      op2I   integer value of second operand (non-register value)
     *      op3I   integer value of third  operand (non-register value)
     *
     *      op1F   float value of first operand (non-register value)
     *      op2F   float value of second operand (non-register value)
     *      op3F   float value of third  operand (non-register value)
     *
     *      CONV2INT(integer-result-variable,value-to-be-converted)
     *      CONV2FLOAT(float-result-variable,value-to-be-converted)
     * ----------------------------------------------------------------------------
     */

#define RESERVED_IMPL(name) START_INSTRUCTION(name) SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION); DISPATCH;

    /* Signal Interrupt Support - this is only used/called when interrupts are pending */
    START_INTERRUPT;
    DEBUG("TRACE - SIGNAL FIRED - CHECK HANDLER\n");

    /* Also clear any pending signals that are ignored and also find the first signal which */
    /* is masked and pending - the first one is the highest priority */
    last_interrupt = 0;
    for (signal_code = 0; signal_code < RXSIGNAL_MAX; signal_code++) {
        if (interrupts & (1 << signal_code)) {
            bin_code *signal_pc = (interrupted_pc && signal_code + 1 != RXSIGNAL_BREAKPOINT) ? interrupted_pc : pc;
            last_interrupted_module[signal_code + 1] = (rxinteger) current_frame->procedure->binarySpace->module->module_number;
            last_interrupted_address[signal_code + 1] = (rxinteger) (signal_pc - current_frame->procedure->binarySpace->binary);
            if (current_frame->interrupt_table[signal_code].response == RXSIGNAL_RESPONSE_IGNORE) {
                DEBUG("TRACE - INTR IGNORE %s\n", interrupt_to_string(signal_code + 1));
                interrupts &= ~(1 << signal_code);
            } else {
                last_interrupt = signal_code + 1;
                break;
            }
        }
    }
    interrupted_pc = 0;

    if (!last_interrupt || last_interrupt >= RXSIGNAL_MAX) {
        /* No un-ignored interrupts pending */
        END_INTERRUPT
    }

    // Clear the interrupt
    if (last_interrupt != RXSIGNAL_BREAKPOINT) {
        // Breakpoints are not cleared
        interrupts &= ~(1 << (last_interrupt - 1));
    }

    // Handle the interrupt
    interrupt_entry signal_handler = current_frame->interrupt_table[last_interrupt - 1];
    switch (signal_handler.response) {

        case RXSIGNAL_RESPONSE_HALT:
            /* Halt */
            DEBUG("TRACE - INTR HANDLER -> HALT %s\n", interrupt_to_string(last_interrupt));
            /* Print error message to stderr */
            if (interrupt_object[last_interrupt]->string_length) {
                fprintf(stderr, "PANIC: %.*s (SIGNAL %s)\n", (int)(interrupt_object[last_interrupt]->string_length), interrupt_object[last_interrupt]->string_value, interrupt_to_string(last_interrupt));
            } else {
                fprintf(stderr, "PANIC: (SIGNAL %s)\n", interrupt_to_string(last_interrupt));
                print_runtime_panic_location(context,
                                             last_interrupted_module[last_interrupt],
                                             last_interrupted_address[last_interrupt]);
            }
            rc = (int)last_interrupt;
            goto interprt_finished;

        case RXSIGNAL_RESPONSE_SILENT_HALT:
            /* Silent Halt */
            DEBUG("TRACE - INTR HANDLER -> SILENT HALT %s\n", interrupt_to_string(last_interrupt));
            rc = 0;
            goto interprt_finished;

        case RXSIGNAL_RESPONSE_CALL_BRANCH:
            DEBUG("TRACE - INTR HANDLER -> SET BRANCH FOR CALL RETURN ");
            current_frame = rxsignal_unwind_to_frame(current_frame, signal_handler.frame);
#ifndef NTHREADED
            current_module = current_frame->procedure->binarySpace->module;
#endif
            next_pc = current_frame->procedure->binarySpace->binary + signal_handler.jump;
            pc = next_pc;
            // Fall through to CALL

        case RXSIGNAL_RESPONSE_CALL_ACTION:
        case RXSIGNAL_RESPONSE_CALL: {
            /* Call */
            proc_runtime *intr_function = signal_handler.function;
            char action_aware = signal_handler.response == RXSIGNAL_RESPONSE_CALL_ACTION;
            DEBUG("TRACE - INTR HANDLER -> CALL %s->%s()\n", interrupt_to_string(last_interrupt), intr_function->name);

            if (intr_function->start == SIZE_MAX) {
                SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, "Exception handler not exposed/linked")
                DISPATCH
            }

            /* Populate the interrupt argument object */
            rxsignal_populate_raw_interrupt(interrupt_arg,
                                            last_interrupt,
                                            last_interrupted_module[last_interrupt],
                                            last_interrupted_address[last_interrupt],
                                            interrupt_object[last_interrupt]);

            if (intr_function->binarySpace == 0) {
                /* This is a native plugin function */
                rxvm_callfunc((void *) (intr_function->start), 1, &interrupt_arg, 0, signal_value);
                if (signal_value->int_value && signal_value->int_value < RXSIGNAL_MAX) {
                    if (signal_value->string_length) {
                        SET_SIGNAL_MSG(signal_value->int_value, signal_value->string_value)
                    } else {
                        SET_SIGNAL(signal_value->int_value)
                    }
                }
                DISPATCH
            } else {
                /* A CREXX Procedure */
                if (action_aware) value_zero(interrupt_action_value);
                current_frame = frame_f(intr_function, 1, current_frame, pc, action_aware ? interrupt_action_value : 0);

                /* Prepare dispatch to procedure as early as possible */
#ifndef NTHREADED
                current_module = current_frame->procedure->binarySpace->module;
#endif
                next_pc = &(current_frame->procedure->binarySpace->binary[intr_function->start]);
                CALC_DISPATCH_MANUAL

                /* Interrupt being handled */
                current_frame->is_interrupt = last_interrupt;
                current_frame->is_interrupt_action = action_aware;

                /* Argument */
                size_t arg_index = intr_function->binarySpace->globals + intr_function->locals + 1;
                current_frame->baselocals[arg_index] = current_frame->locals[arg_index] = interrupt_arg;

                /* DISPATCH goes the interrupt handler */
                DISPATCH
            }
        }

        case RXSIGNAL_RESPONSE_BRANCH:
            DEBUG("TRACE - INTR HANDLER -> BRANCH %s\n", interrupt_to_string(last_interrupt));
            current_frame = rxsignal_unwind_to_frame(current_frame, signal_handler.frame);
#ifndef NTHREADED
            current_module = current_frame->procedure->binarySpace->module;
#endif
            next_pc = current_frame->procedure->binarySpace->binary + signal_handler.jump;
            CALC_DISPATCH_MANUAL
            DISPATCH

        case RXSIGNAL_RESPONSE_BRANCH_VALUE:
            DEBUG("TRACE - INTR HANDLER -> BRANCH VALUE %s\n", interrupt_to_string(last_interrupt));
            current_frame = rxsignal_unwind_to_frame(current_frame, signal_handler.frame);
#ifndef NTHREADED
            current_module = current_frame->procedure->binarySpace->module;
#endif
            rxsignal_populate_raw_interrupt(interrupt_arg,
                                            last_interrupt,
                                            last_interrupted_module[last_interrupt],
                                            last_interrupted_address[last_interrupt],
                                            interrupt_object[last_interrupt]);
            rxsignal_populate_runtime_signal(current_frame->locals[signal_handler.value_register], interrupt_arg);
            next_pc = current_frame->procedure->binarySpace->binary + signal_handler.jump;
            CALC_DISPATCH_MANUAL
            DISPATCH

        case RXSIGNAL_RESPONSE_RETURN:
            DEBUG("TRACE - INTR HANDLER -> RET %s\n", interrupt_to_string(last_interrupt));
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                // Note that current_frame->is_interrupt cannot be set as a signal triggers us
                /* back to the parent's stack frame */
                temp_frame = current_frame;
                current_frame = current_frame->parent;
                if (!current_frame) {
                    DEBUG("TRACE - INTR RETURNING FROM MAIN()\n");
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                         i < num_args;
                         i++, j++) {
                        clear_value(temp_frame->baselocals[j]);
                        free(temp_frame->baselocals[j]);
                         }
                    rc = 0;
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    goto interprt_finished;
                }
                free_frame(temp_frame);
#ifndef NTHREADED
                current_module = current_frame->procedure->binarySpace->module;
#endif
                CALC_DISPATCH_MANUAL
                DISPATCH
            }

        case RXSIGNAL_RESPONSE_IGNORE:
            /* Ignore - Should never get here */
            DEBUG("*ERROR* TRACE INTR HANDLER -> IGNORE (SHOULD NOT GET HERE) %s\n", interrupt_to_string(last_interrupt));
            END_INTERRUPT
    }

    /* Should never get here */
    END_INTERRUPT

START_OF_INSTRUCTIONS

        /* Signal / Interrupt Instructions */

        /* Enable Breakpoints */
        START_INSTRUCTION(BPON) CALC_DISPATCH(0)
            DEBUG("TRACE - BPON\n");
            interrupts |= 1 << (RXSIGNAL_BREAKPOINT - 1);
            DISPATCH

        /* Enable Breakpoints with op1 handler */
        START_INSTRUCTION(BPON_FUNC) CALC_DISPATCH(1)
            {
                proc_runtime *signal_function = PROC_OP(1);
                DEBUG("TRACE - BPON %s()\n", signal_function->name);
                current_frame->interrupt_table[RXSIGNAL_BREAKPOINT-1].response = RXSIGNAL_RESPONSE_CALL;
                current_frame->interrupt_table[RXSIGNAL_BREAKPOINT-1].function = signal_function;
                interrupts |= 1 << (RXSIGNAL_BREAKPOINT - 1);
            }
            DISPATCH

        /* Disable Breakpoints */
        START_INSTRUCTION(BPOFF) CALC_DISPATCH(0)
            DEBUG("TRACE - BPOFF\n");
            interrupts &= ~(1 << (RXSIGNAL_BREAKPOINT - 1));
            DISPATCH

        /* Set Signal op1 Handle to Ignore */
        START_INSTRUCTION(SIGIGNORE_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGIGNORE \"%.*s\"\n", (int) op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_IGNORE;
                    current_frame->interrupt_table[sig-1].function = 0;
                    current_frame->interrupt_table[sig-1].jump = 0;
                    current_frame->interrupt_table[sig-1].frame = 0;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    ignore_interrupt((int)sig); // Set the corresponding native interrupt to ignore
                }
            }
            DISPATCH

        /* Set Signal op1 Handle to Halt */
        START_INSTRUCTION(SIGHALT_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGHALT \"%.*s\"\n", (int) op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX) { // Kill can be set to halt
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_HALT;
                    current_frame->interrupt_table[sig-1].function = 0;
                    current_frame->interrupt_table[sig-1].jump = 0;
                    current_frame->interrupt_table[sig-1].frame = 0;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Set Signal op1 Handle to Silent Halt */
        START_INSTRUCTION(SIGSHALT_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGSHALT \"%.*s\"\n", (int) op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX) { // KILL can be set to silent halt
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_SILENT_HALT;
                    current_frame->interrupt_table[sig-1].function = 0;
                    current_frame->interrupt_table[sig-1].jump = 0;
                    current_frame->interrupt_table[sig-1].frame = 0;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Set Signal op2 Handle to Branch to op1 */
        START_INSTRUCTION(SIGBR_ID_STRING) CALC_DISPATCH(2)
            DEBUG("TRACE - SIGBR 0x%x,\"%.*s\"\n", (unsigned int)REG_IDX(1), (int)op2S->string_len, op2S->string);
            {
                size_t sig = string_to_interrupt(op2S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_BRANCH;
                    current_frame->interrupt_table[sig-1].jump = REG_IDX(1);
                    current_frame->interrupt_table[sig-1].function = 0;
                    current_frame->interrupt_table[sig-1].frame = current_frame;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Set Signal op3 Handle to Branch to op1 and bind a .signal object to op2 */
        START_INSTRUCTION(SIGBRV_ID_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SIGBRV 0x%x,R%d,\"%.*s\"\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)op3S->string_len, op3S->string);
            {
                size_t sig = string_to_interrupt(op3S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_BRANCH_VALUE;
                    current_frame->interrupt_table[sig-1].jump = REG_IDX(1);
                    current_frame->interrupt_table[sig-1].function = 0;
                    current_frame->interrupt_table[sig-1].frame = current_frame;
                    current_frame->interrupt_table[sig-1].value_register = REG_IDX(2);
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Set Signal op2 Handle to Call op1 */
        START_INSTRUCTION(SIGCALL_FUNC_STRING) CALC_DISPATCH(2)
            {
                proc_runtime *signal_function = PROC_OP(1);
                DEBUG("TRACE - SIGCALL %s(),\"%.*s\"\n", signal_function->name, (int)op2S->string_len, op2S->string);

                size_t sig = string_to_interrupt(op2S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_CALL;
                    current_frame->interrupt_table[sig-1].function = signal_function;
                    current_frame->interrupt_table[sig-1].jump = 0;
                    current_frame->interrupt_table[sig-1].frame = 0;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Set Signal op2 Handle to action-aware Call op1 */
        START_INSTRUCTION(SIGCALLA_FUNC_STRING) CALC_DISPATCH(2)
            {
                proc_runtime *signal_function = PROC_OP(1);
                DEBUG("TRACE - SIGCALLA %s(),\"%.*s\"\n", signal_function->name, (int)op2S->string_len, op2S->string);

                size_t sig = string_to_interrupt(op2S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_CALL_ACTION;
                    current_frame->interrupt_table[sig-1].function = signal_function;
                    current_frame->interrupt_table[sig-1].jump = 0;
                    current_frame->interrupt_table[sig-1].frame = 0;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Set Signal op3 Handle to Call op2 returning to op1 */
        START_INSTRUCTION(SIGCALLBR_ID_FUNC_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SIGCALLBR 0x%x,%s(),\"%.*s\"\n", (unsigned int)REG_IDX(1), PROC_OP(2)->name, (int)op3S->string_len, op3S->string);
            {
                proc_runtime *signal_function = PROC_OP(2);
                size_t sig = string_to_interrupt(op3S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_CALL_BRANCH;
                    current_frame->interrupt_table[sig-1].function = signal_function;
                    current_frame->interrupt_table[sig-1].jump = REG_IDX(1);
                    current_frame->interrupt_table[sig-1].frame = current_frame;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Set Signal op1 Handle to Return */
        START_INSTRUCTION(SIGRET_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGRET \"%.*s\"\n", (int)op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) { // KILL cannot be masked
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
                else {
                    current_frame->interrupt_table[sig-1].response = RXSIGNAL_RESPONSE_RETURN;
                    current_frame->interrupt_table[sig-1].function = 0;
                    current_frame->interrupt_table[sig-1].jump = 0;
                    current_frame->interrupt_table[sig-1].frame = 0;
                    current_frame->interrupt_table[sig-1].value_register = 0;
                    enable_interrupt((int)sig); // Enable the corresponding native interrupt
                }
            }
            DISPATCH

        /* Push current Signal op1 handler on the current frame handler stack */
        START_INSTRUCTION(SIGPUSH_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGPUSH \"%.*s\"\n", (int)op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL) {
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                } else {
                    rxsignal_push_handler(current_frame, (unsigned char)sig);
                }
            }
            DISPATCH

        /* Pop and restore current Signal op1 handler from the current frame handler stack */
        START_INSTRUCTION(SIGPOP_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGPOP \"%.*s\"\n", (int)op1S->string_len, op1S->string);
            {
                size_t sig = string_to_interrupt(op1S->string);
                if (sig == RXSIGNAL_MAX || sig == RXSIGNAL_KILL || !rxsignal_pop_handler(current_frame, (unsigned char)sig)) {
                    SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE);
                }
            }
            DISPATCH

        /* RXSIGNAL_STRING Signal type op1 */
        START_INSTRUCTION(SIGNAL_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGNAL \"%.*s\"\n", (int)op1S->string_len, op1S->string);
            SET_SIGNAL_FROM_NAME(op1S->string);
            DISPATCH

        /* SIGNAL_REG Signal type op1 */
        START_INSTRUCTION(SIGNAL_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - SIGNAL R%d\n", (int)REG_IDX(1));
            null_terminate_string_buffer(op1R);
            SET_SIGNAL_FROM_NAME(op1R->string_value);
            DISPATCH

        /* SIGNALT_STRING_REG Signal type op1 if op2 true */
        START_INSTRUCTION(SIGNALT_STRING_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SIGNALT \"%.*s\",R%d\n", (int)op1S->string_len, op1S->string, (int)REG_IDX(2));
            if (op2RI) {
                SET_SIGNAL_FROM_NAME(op1S->string);
            }
            DISPATCH

        /* SIGNALF_STRING_REG Signal type op1 if op2 true */
        START_INSTRUCTION(SIGNALF_STRING_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SIGNALF \"%.*s\",R%d\n", (int)op1S->string_len, op1S->string, (int)REG_IDX(2));
            if (!op2RI) {
                SET_SIGNAL_FROM_NAME(op1S->string);
            }
            DISPATCH

        /* SIGNAL_STRING_STRING Signal type op1 (message op2) */
        START_INSTRUCTION(SIGNAL_STRING_STRING) CALC_DISPATCH(2)
            DEBUG("TRACE - SIGNAL \"%.*s\",\"%.*s\"\n", (int)op1S->string_len, op1S->string, (int)op2S->string_len, op2S->string);
            SET_SIGNAL_MSG_FROM_NAME(op1S->string, op2S->string);
            DISPATCH

        /* SIGNAL_STRING_REG Signal type op1 (payload op2) */
        START_INSTRUCTION(SIGNAL_STRING_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SIGNAL \"%.*s\",R%d\n", (int)op1S->string_len, op1S->string, (int)REG_IDX(2));
            SET_SIGNAL_PAYLOAD_FROM_NAME(op1S->string, op2R);
            DISPATCH

        /* SIGNAL_REG_REG Signal type op1 (payload op2) */
        START_INSTRUCTION(SIGNAL_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SIGNAL R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            null_terminate_string_buffer(op1R);
            SET_SIGNAL_PAYLOAD_FROM_NAME(op1R->string_value, op2R);
            DISPATCH

        /* SIGNALT_STRING_STRING_REG Signal type op1 (message op2) if op3 true */
        START_INSTRUCTION(SIGNALT_STRING_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SIGNALT \"%.*s\",\"%.*s\",R%d\n", (int)op1S->string_len, op1S->string, (int)op2S->string_len, op2S->string, (int)REG_IDX(2));
            if (op3RI) {
                SET_SIGNAL_MSG_FROM_NAME(op1S->string, op2S->string);
            }
            DISPATCH

        /* SIGNALF_STRING_STRING_REG Signal type op1 (message op2) if op3 true */
        START_INSTRUCTION(SIGNALF_STRING_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SIGNALF \"%.*s\",\"%.*s\"\",R%d\n", (int)op1S->string_len, op1S->string, (int)op2S->string_len, op2S->string, (int)REG_IDX(2));
            if (!op3RI) {
                SET_SIGNAL_MSG_FROM_NAME(op1S->string, op2S->string);
            }
            DISPATCH

        /* Meta Instructions */

        /* Load Module (op1 = module num of loaded op2) */
        START_INSTRUCTION(METALOADMODULE_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - METALOADMODULE R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2));
            {
                int num_modules_before;
                null_terminate_string_buffer(op2R);
                /* Load the module */
                num_modules_before = (int) context->num_modules;
                op1R->int_value = rxldmod(context, op2R->string_value);
                if (op1R->int_value > 0) {
                    /* If successfully loaded, thread the binary - must be done in run() */
#ifndef NTHREADED
                    int mod;
                    DEBUG("Threading\n");
                    for (mod = num_modules_before; mod < op1R->int_value; mod++) {
                        module *loaded_module = context->modules[mod];
                        size_t i = 0, j;
                        if (!loaded_module->prepared_dispatch && loaded_module->segment.inst_size) {
                            loaded_module->prepared_dispatch = malloc(sizeof(void *) * loaded_module->segment.inst_size);
                            if (!loaded_module->prepared_dispatch) {
                                RX_PANIC_OOM("malloc rxvm loaded-module prepared dispatch table",
                                             sizeof(void *) * loaded_module->segment.inst_size,
                                             loaded_module->name);
                            }
                        }
                        while (i < context->modules[mod]->segment.inst_size) {
                            j = i;
                            i += context->modules[mod]->segment.binary[i].instruction.no_ops + 1;
                            loaded_module->prepared_dispatch[j] =
                                (void *) address_map[context->modules[mod]->segment.binary[j].instruction.opcode];
                        }
                    }
#endif
                    rxvm_link(context);
                }
            }
            DISPATCH

            /* Load Instruction Code (op1 = (inst)op2[op3]) */
        START_INSTRUCTION(METALOADINST_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - METALOADINST R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            {
                bin_code inst = context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value];
                op1R->int_value = inst.instruction.opcode;
            }
            DISPATCH

            /* Loaded Modules (op1=array loaded modules) */
        START_INSTRUCTION(METALOADEDMODULES_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - METALOADEDMODULES R%d\n", (int) REG_IDX(1));
            /* op1R will become an array of module names */
            value_zero(op1R);
            set_num_attributes(op1R, context->num_modules);
            op1R->int_value = (rxinteger) context->num_modules; /* The cREXX convention for arrays */
            for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
                set_null_string(op1R->attributes[mod_index], context->modules[mod_index]->name);
            }
            DISPATCH

        /* Loaded Exposed Procedures (op1 = array procedures in module op2) */
        START_INSTRUCTION(METALOADEDEPROCS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - METALOADEDEPROCS R%d\n", (int) REG_IDX(1));
            {
                chameleon_constant *c_entry;
                proc_constant *p_entry;
                expose_proc_constant *e_entry;
                int i;
                size_t entries;
                value *entry;

                /* Module to be listed */
                size_t mod = op2R->int_value - 1;

                /* op1R will become an array of procedure names and pointers */
                value_zero(op1R);

                /* How many entries are needed? */
                i = context->modules[mod]->expose_head;
                entries = 0;
                while (i != -1) {
                    c_entry = (chameleon_constant *) (context->modules[mod]->segment.const_pool + i);
                    if (c_entry->type == EXPOSE_PROC_CONST) {
                        if (!((expose_proc_constant *) c_entry)->imported) entries++;
                    }
                    i = ((expose_proc_constant *) c_entry)->next;
                }

                /* Set up array */
                set_num_attributes(op1R, entries);
                op1R->int_value = (rxinteger) entries; /* The cREXX convention for arrays */

                /* Populate array */
                i = context->modules[mod]->expose_head;
                entries = 0;
                while (i != -1) {
                    c_entry = (chameleon_constant *) (context->modules[mod]->segment.const_pool + i);
                    if (c_entry->type == EXPOSE_PROC_CONST) {
                        /* Exposed Procedure */
                        e_entry = (expose_proc_constant *) c_entry;
                        p_entry = (proc_constant *) (
                                context->modules[mod]->segment.const_pool
                                + e_entry->procedure);
                        if (!e_entry->imported) {
                            /* Exported - Procedure - populate object and add to array  */
                            entry = op1R->attributes[entries];
                            set_num_attributes(entry, 2);
                            set_null_string(entry->attributes[0], e_entry->index);
                            entry->attributes[1]->int_value =
                                    (rxinteger) rxvm_get_module_runtime_procedure(context->modules[mod],
                                                                                  e_entry->procedure);

                            entries++;
                        }
                    }
                    i = e_entry->next;
                }
            }
            DISPATCH

        /* Loaded Procedures (op1 = array procedures in module op2) */
        START_INSTRUCTION(METALOADEDPROCS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - METALOADEDPROCS R%d\n", (int) REG_IDX(1));
            {
                chameleon_constant *c_entry;
                proc_constant *p_entry;
                int i;
                size_t entries;
                value *entry;

                /* Module to be listed */
                size_t mod = op2R->int_value - 1;

                /* op1R will become an array of procedure names and pointers */
                value_zero(op1R);

                /* How many entries are needed? */
                i = context->modules[mod]->proc_head;
                entries = 0;
                while (i != -1) {
                    c_entry = (chameleon_constant *) (context->modules[mod]->segment.const_pool + i);
                    if (c_entry->type == PROC_CONST) {
                        entries++;
                    }
                    i = ((proc_constant *) c_entry)->next;
                }

                /* Set up array */
                set_num_attributes(op1R, entries);
                op1R->int_value = (rxinteger) entries; /* The cREXX convention for arrays */

                /* Populate array */
                i = context->modules[mod]->proc_head;
                entries = 0;
                while (i != -1) {
                    c_entry = (chameleon_constant *) (context->modules[mod]->segment.const_pool + i);
                    if (c_entry->type == PROC_CONST) {
                        /* Exposed Procedure */
                        p_entry = (proc_constant *) c_entry;
                        entry = op1R->attributes[entries];
                        set_num_attributes(entry, 2);
                        set_null_string(entry->attributes[0], p_entry->name);
                        entry->attributes[1]->int_value =
                                (rxinteger) rxvm_get_module_runtime_procedure(context->modules[mod], (size_t)i);
                        entries++;
                    }
                    i = p_entry->next;
                }
            }
            DISPATCH

            /* Decode opcode (op1 decoded op2) */
        START_INSTRUCTION(METADECODEINST_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - METADECODEINST R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2));

            /* The target register is turned into an object with 7 attributes */
            value_zero(op1R);
            set_num_attributes(op1R, 7);

            /* Populate the object */
            op1R->attributes[0]->int_value = meta_map[op2R->int_value].opcode;
            set_null_string(op1R->attributes[1], meta_map[op2R->int_value].instruction);
            set_null_string(op1R->attributes[2], meta_map[op2R->int_value].desc);
            op1R->attributes[3]->int_value = meta_map[op2R->int_value].operands;
            op1R->attributes[4]->int_value = meta_map[op2R->int_value].op1_type;
            op1R->attributes[5]->int_value = meta_map[op2R->int_value].op2_type;
            op1R->attributes[6]->int_value = meta_map[op2R->int_value].op3_type;
            DISPATCH

            /* Load Integer/Index Operand (op1 = (int)op2[op3]) */
        START_INSTRUCTION(METALOADIOPERAND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - METALOADIOPERAND R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            op1R->int_value = context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value].iconst;
            DISPATCH

            /* Load Float Operand (op1 = (float)op2[op3]) */
        START_INSTRUCTION(METALOADFOPERAND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - METALOADFOPERAND R%d,R%dR%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            op1R->float_value =
                    FLOAT_CONST_VALUE(context->modules[op2R->int_value - 1]->segment.const_pool,
                                      context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value].index);
            DISPATCH

            /* Load String Operand (op1 = (string)op2[op3]) */
        START_INSTRUCTION(METALOADSOPERAND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - METALOADSOPERAND R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            set_const_string(op1R,
                             (string_constant *) (
                                     context->modules[op2R->int_value - 1]->segment.const_pool +
                                     context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value].index
                             ));

            DISPATCH

            /* Load Procedure Operand (op1 = (proc)op2[op3]) */
            /* TODO needs to do more that get the function name - a function object is needed */
        START_INSTRUCTION(METALOADPOPERAND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - METALOADPOPERAND R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            {
                proc_constant
                        *proc =
                        (proc_constant *) (
                                context->modules[op2R->int_value - 1]->segment.const_pool +
                                context->modules[op2R->int_value - 1]->segment.binary[op3R->int_value].index
                        );
                set_null_string(op1R, proc->name);
            }
            DISPATCH

            /* Load Metadata (op1 = (metadata)op2[op3]) */
        START_INSTRUCTION(METALOADDATA_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - METALOADDATA R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            {
                module *metadata_module = context->modules[op2R->int_value - 1];
                unsigned char *pool = metadata_module->segment.const_pool;
                int i = metadata_module->meta_head;
                int j;
                size_t x;
                meta_entry *meta = 0;
                int size = 0;
                string_constant *string_entry;

                /* Clear return object */
                value_zero(op1R);

                /* Find the start of the metadata @ address */
                while (i != -1) {
                    meta = (meta_entry *) (pool + i);
                    if (meta->address < op3R->int_value) i = meta->next;
                    else break;
                }
                if (i == -1 || meta->address > op3R->int_value) {
                    /* No metadata for the address */
                    DISPATCH
                }
                int start = i;

                /* How many entries */
                size = 0;
                while (i != -1) {
                    meta = (meta_entry *) (pool + i);
                    if (meta->address == op3R->int_value) {
                        i = meta->next;
                        size++;
                    } else break;
                }
                set_num_attributes(op1R, size);
                op1R->int_value = size; /* TODO Using int to store the size of an array ... approach tbc! */

                /* Populate output with the metadata */
                for (j = 0, i = start; j < size; j++, i = ((meta_entry *) (pool + i))->next) {
                    value_zero(op1R->attributes[j]);
                    switch (((meta_entry *) (pool + i))->base.type) {
                        case META_SOURCE_STEP:
                            set_null_string(op1R->attributes[j], ".meta_source_step");
                            set_num_attributes(op1R->attributes[j], 8);
                            op1R->attributes[j]->attributes[0]->int_value =
                                    (rxinteger) ((meta_source_step_constant *) (pool + i))->step_id;
                            op1R->attributes[j]->attributes[1]->int_value =
                                    (rxinteger) ((meta_source_step_constant *) (pool + i))->clause_id;
                            op1R->attributes[j]->attributes[2]->int_value =
                                    (rxinteger) ((meta_source_step_constant *) (pool + i))->flags;
                            x = (rxinteger) ((meta_source_step_constant *) (pool + i))->file;
                            set_const_string(op1R->attributes[j]->attributes[3], (string_constant *) (pool + x));
                            op1R->attributes[j]->attributes[4]->int_value =
                                    (rxinteger) ((meta_source_step_constant *) (pool + i))->line;
                            op1R->attributes[j]->attributes[5]->int_value =
                                    (rxinteger) ((meta_source_step_constant *) (pool + i))->active_start_column;
                            op1R->attributes[j]->attributes[6]->int_value =
                                    (rxinteger) ((meta_source_step_constant *) (pool + i))->active_end_column;
                            x = (rxinteger) ((meta_source_step_constant *) (pool + i))->source_line;
                            set_const_string(op1R->attributes[j]->attributes[7], (string_constant *) (pool + x));
                            break;
                        case META_TRACE_EVENT:
                            set_null_string(op1R->attributes[j], ".meta_trace_event");
                            set_num_attributes(op1R->attributes[j], 11);
                            op1R->attributes[j]->attributes[0]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->kind;
                            op1R->attributes[j]->attributes[1]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->mode_mask;
                            op1R->attributes[j]->attributes[2]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->value_source;
                            op1R->attributes[j]->attributes[3]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->value_type;
                            op1R->attributes[j]->attributes[4]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->register_type;
                            op1R->attributes[j]->attributes[5]->int_value =
                                    ((meta_trace_event_constant *) (pool + i))->value_ref == RXBIN_TRACE_REF_NONE ?
                                    (rxinteger)-1 : (rxinteger) ((meta_trace_event_constant *) (pool + i))->value_ref;
                            op1R->attributes[j]->attributes[6]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->source_step_id;
                            op1R->attributes[j]->attributes[7]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->clause_id;
                            op1R->attributes[j]->attributes[8]->int_value =
                                    (rxinteger) ((meta_trace_event_constant *) (pool + i))->flags;
                            x = (rxinteger) ((meta_trace_event_constant *) (pool + i))->symbol;
                            if ((size_t)x == RXBIN_TRACE_REF_NONE) set_null_string(op1R->attributes[j]->attributes[9], "");
                            else {
                                string_entry = get_runtime_string_constant(metadata_module, x);
                                if (string_entry) set_const_string(op1R->attributes[j]->attributes[9], string_entry);
                                else set_null_string(op1R->attributes[j]->attributes[9], "");
                            }
                            x = (rxinteger) ((meta_trace_event_constant *) (pool + i))->resolved_name;
                            if ((size_t)x == RXBIN_TRACE_REF_NONE) set_null_string(op1R->attributes[j]->attributes[10], "");
                            else {
                                string_entry = get_runtime_string_constant(metadata_module, x);
                                if (string_entry) set_const_string(op1R->attributes[j]->attributes[10], string_entry);
                                else set_null_string(op1R->attributes[j]->attributes[10], "");
                            }
                            break;
                        case META_FUNC:
                            set_null_string(op1R->attributes[j], ".meta_func");
                            set_num_attributes(op1R->attributes[j], 5);
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->option;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_func_constant *) (pool + i))->args;
                            set_const_string(op1R->attributes[j]->attributes[3], (string_constant *) (pool + x));
                            op1R->attributes[j]->attributes[4]->int_value = (rxinteger) ((meta_func_constant *) (pool +
                                                                                                                 i))->func;
                            break;
                        case META_REG:
                            set_null_string(op1R->attributes[j], ".meta_reg");
                            set_num_attributes(op1R->attributes[j], 4);
                            x = (rxinteger) ((meta_reg_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_reg_constant *) (pool + i))->option;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_reg_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            op1R->attributes[j]->attributes[3]->int_value = (rxinteger) ((meta_reg_constant *) (pool +
                                                                                                                i))->reg;
                            break;
                        case META_CONST:
                            set_null_string(op1R->attributes[j], ".meta_const");
                            set_num_attributes(op1R->attributes[j], 4);
                            x = (rxinteger) ((meta_const_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_const_constant *) (pool + i))->option;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_const_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_const_constant *) (pool + i))->constant;
                            set_const_string(op1R->attributes[j]->attributes[3], (string_constant *) (pool + x));
                            break;
                        case META_CLEAR:
                            set_null_string(op1R->attributes[j], ".meta_clear");
                            set_num_attributes(op1R->attributes[j], 1);
                            x = (rxinteger) ((meta_clear_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            break;
                        case META_CLASS:
                            /* TODO: Implement META_CLASS */
                            break;
                        case META_ATTR:
                            /* TODO: Implement META_ATTR */
                            break;
                        case META_INTERFACE:
                            set_null_string(op1R->attributes[j], ".meta_interface");
                            set_num_attributes(op1R->attributes[j], 3);
                            x = (rxinteger) ((meta_interface_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_interface_constant *) (pool + i))->option;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_interface_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            break;
                        case META_IMPLEMENTS:
                            set_null_string(op1R->attributes[j], ".meta_implements");
                            set_num_attributes(op1R->attributes[j], 2);
                            x = (rxinteger) ((meta_implements_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_implements_constant *) (pool + i))->interface_symbol;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            break;
                        case META_MEMBER:
                            set_null_string(op1R->attributes[j], ".meta_member");
                            set_num_attributes(op1R->attributes[j], 5);
                            x = (rxinteger) ((meta_member_constant *) (pool + i))->owner;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_member_constant *) (pool + i))->kind;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_member_constant *) (pool + i))->member;
                            set_const_string(op1R->attributes[j]->attributes[2], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_member_constant *) (pool + i))->type;
                            set_const_string(op1R->attributes[j]->attributes[3], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_member_constant *) (pool + i))->args;
                            set_const_string(op1R->attributes[j]->attributes[4], (string_constant *) (pool + x));
                            break;
                        case META_INLINE:
                            set_null_string(op1R->attributes[j], ".meta_inline");
                            set_num_attributes(op1R->attributes[j], 2);
                            x = (rxinteger) ((meta_inline_constant *) (pool + i))->symbol;
                            set_const_string(op1R->attributes[j]->attributes[0], (string_constant *) (pool + x));
                            x = (rxinteger) ((meta_inline_constant *) (pool + i))->payload;
                            set_const_string(op1R->attributes[j]->attributes[1], (string_constant *) (pool + x));
                            break;
                        case STRING_CONST:
                        case FLOAT_CONST:
                        case PROC_CONST:
                        case BINARY_CONST:
                        case DECIMAL_CONST:
                        case EXPOSE_REG_CONST:
                        case EXPOSE_PROC_CONST:
                            break;

                    }
                }
            }
            DISPATCH

            /* METALOADCALLERADDR - Load caller address object to op1 */
        START_INSTRUCTION(METALOADCALLERADDR_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - METALOADCALLERADDR R%d\n", (int) REG_IDX(1));
            {
                rxinteger mod_no = -1;
                rxinteger addr = -1;

                if (current_frame->parent != 0) {
                    mod_no = (rxinteger) current_frame->parent->procedure->binarySpace->module->module_number;
                    addr = (rxinteger) (current_frame->return_pc -
                                        current_frame->parent->procedure->binarySpace->binary);
                }

                /* Populate the result object */
                value_zero(op1R);
                set_num_attributes(op1R, 2);
                op1R->attributes[0]->int_value = mod_no;
                op1R->attributes[1]->int_value = addr;
            }
            DISPATCH

        /* Regular Instructions */

        /* NULL (Clear the register) */
        START_INSTRUCTION(NULL_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - NULL R%lu\n", REG_IDX(1));
            value_zero(op1R);
            DISPATCH

        /* LOAD */
        START_INSTRUCTION(LOAD_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - LOAD R%d,%d\n", (int) REG_IDX(1), (int) op2I);
            set_int(op1R, op2I);
            DISPATCH

        START_INSTRUCTION(LOAD_REG_STRING) CALC_DISPATCH(2)
            DEBUG("TRACE - LOAD R%lu,\"%.*s\"\n",
                  REG_IDX(1), (int) (CONSTSTRING_OP(2))->string_len,
                  (CONSTSTRING_OP(2))->string);
            set_const_string(op1R, CONSTSTRING_OP(2));
            DISPATCH

        START_INSTRUCTION(LOAD_REG_BINARY) CALC_DISPATCH(2)
            DEBUG("TRACE - LOAD R%lu,binary[%zu]\n",
                  REG_IDX(1), (CONSTSTRING_OP(2))->string_len);
            if (set_binary(op1R, (CONSTSTRING_OP(2))->string, (CONSTSTRING_OP(2))->string_len) != 0) {
                SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
            }
            DISPATCH

        START_INSTRUCTION(LOAD_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - LOAD R%lu,R%lu\n",
                  REG_IDX(1), REG_IDX(2));
            op1R=op2R;
            DISPATCH

        START_INSTRUCTION(LOAD_INT_INT) CALC_DISPATCH(2) // TODO - review instruction
            DEBUG("TRACE - LOAD R%lu,R%lu\n",
                  (long) op1I, (long) op2I);
            REG_OP(op1I)=REG_OP(op2I);
        DISPATCH

    START_INSTRUCTION(LOAD_INT_REG) CALC_DISPATCH(2) // TODO - review instruction
    DEBUG("TRACE - LOAD R%lu, R%d\n",
          REG_IDX(1), (int) op2I);
          REG_OP(op1I)=op2R;
    DISPATCH

            /* Readline - Read a line from stdin to a register */
        START_INSTRUCTION(READLINE_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - READLINE R%lu\n", REG_IDX(1));
            {
                size_t pos = 0;
                int ch;
                op1R->string_length = 0;
                while ((ch = getchar()) != EOF) {
                    if (ch == '\n') break;
                    extend_string_buffer(op1R, pos + 1);
                    op1R->string_value[pos] = (char) ch;
                    pos++;
                }
                op1R->string_pos = 0;
#ifndef NUTF8
                op1R->string_char_pos = 0;
                refresh_utf8_flags(op1R);
#else
                clear_vm_private_flags(op1R);
#endif
            }
            DISPATCH
	      
        START_INSTRUCTION(SAY_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - SAY R%lu\n", REG_IDX(1));
            rxvm_mprintf("%.*s\n", (int) op1R->string_length, op1R->string_value);
            DISPATCH

        START_INSTRUCTION(SAY_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SAY \"%.*s\"\n",
                  (int) op1S->string_len, op1S->string);
            rxvm_mprintf("%.*s\n", (int) op1S->string_len, op1S->string);
            DISPATCH
	      
        START_INSTRUCTION(SAYX_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - SAYX R%lu\n", REG_IDX(1));
            rxvm_mprintf("%.*s", (int) op1R->string_length, op1R->string_value);
            DISPATCH
	      
	START_INSTRUCTION(SAYX_STRING) CALC_DISPATCH(1)
            DEBUG("TRACE - SAYX \"%.*s\"\n",
                  (int) op1S->string_len, op1S->string);
            rxvm_mprintf("%.*s", (int) op1S->string_len, op1S->string);
            DISPATCH

        START_INSTRUCTION(SCONCAT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SCONCAT R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            string_sconcat(op1R, op2R, op3R);
            DISPATCH

        START_INSTRUCTION(CONCAT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - CONCAT R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            string_concat(op1R, op2R, op3R);
            DISPATCH

        START_INSTRUCTION(SCONCAT_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SCONCAT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                  REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                  (CONSTSTRING_OP(3))->string);
            string_sconcat_var_const(op1R, op2R, op3S);
            DISPATCH

        START_INSTRUCTION(CONCAT_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - CONCAT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                  REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                  (CONSTSTRING_OP(3))->string);
            string_concat_var_const(op1R, op2R, op3S);
            DISPATCH

        START_INSTRUCTION(SCONCAT_REG_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SCONCAT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                  (int) (CONSTSTRING_OP(2))->string_len,
                  (CONSTSTRING_OP(2))->string, REG_IDX(3));
            string_sconcat_const_var(op1R, op2S, op3R);
            DISPATCH

        START_INSTRUCTION(CONCAT_REG_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - CONCAT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                  (int) (CONSTSTRING_OP(2))->string_len,
                  (CONSTSTRING_OP(2))->string, REG_IDX(3));
            string_concat_const_var(op1R, op2S, op3R);
            DISPATCH

        START_INSTRUCTION(IMULT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IMULT R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            REG_RETURN_INT(op2RI * op3RI)
            DISPATCH

        START_INSTRUCTION(IMULT_REG_REG_INT) {
            CALC_DISPATCH(3)
            DEBUG("TRACE - IMULT R%lu,R%lu,%lu\n", (long)REG_IDX(1),
                  (long)REG_IDX(2), (long)op3I);
            REG_RETURN_INT(op2RI * op3I)
            DISPATCH
        }

        START_INSTRUCTION(IADD_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IADD R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            REG_RETURN_INT(op2RI + op3RI)
            DISPATCH

        START_INSTRUCTION(ISUB_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ISUB R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            REG_RETURN_INT(op2RI - op3RI)
            DISPATCH

        START_INSTRUCTION(IADD_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IADD R%lu,R%lu,%d\n", REG_IDX(1),
                  REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI + op3I)
            DISPATCH

        START_INSTRUCTION(ERASE_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - ERASE R%lu\n", REG_IDX(1));
            value_zero(op1R);
            DISPATCH

/* ====================================================================================
 * Numeric Mode instructions
 * ==================================================================================== */

/* ------------------------------------------------------------------------------------
 *  SETNUMDGTS_REG Set Numeric Digits digits=op1 (>4)
 *  ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(SETNUMDGTS_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - SETNUMDGTS R%lu\n", REG_IDX(1));
    if (op1R->int_value < 1) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Digits must be greater than 0");
        DISPATCH
    }
    current_frame->num_context.digits = op1R->int_value;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  SETNUMDGTS_INT Set Numeric Digits digits=op1 (>4)
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(SETNUMDGTS_INT) CALC_DISPATCH(1)
    DEBUG("TRACE - SETNUMDGTS %d\n", (int)op1I);
    if (op1I < 1) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Digits must be greater than 0");
        DISPATCH
    }
    current_frame->num_context.digits = op1I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  GETNUMDGTS_REG Get Numeric Digits op1=digits
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(GETNUMDGTS_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - GETNUMDGTS R%lu\n", REG_IDX(1));
    op1R->int_value = (rxinteger)current_frame->num_context.digits;
    DISPATCH

/* ------------------------------------------------------------------------------------
 * SETNUMFUZ_REG Set Numeric Fuzz digits=op1 (>=0)
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(SETNUMFUZ_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - SETNUMFUZ R%lu\n", REG_IDX(1));
    if (op1R->int_value < 0) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Fuzz must be zero or greater");
        DISPATCH
    }
    current_frame->num_context.fuzz = op1R->int_value;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 * SETNUMFUZ_INT Set Numeric Fuzz digits=op1 (>=0)
 * ----------------------------------------------------------------------------------- */
START_INSTRUCTION(SETNUMFUZ_INT) CALC_DISPATCH(1)
    DEBUG("TRACE - SETNUMFUZ %d\n", (int)op1I);
    if (op1I < 0) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Fuzz must be zero or greater");
    }
    current_frame->num_context.fuzz = op1I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 * GETNUMFUZ_REG Get Numeric Fuzz op1=digits
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(GETNUMFUZ_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - GETNUMFUZ R%lu\n", REG_IDX(1));
    op1R->int_value = (rxinteger)current_frame->num_context.fuzz;
    DISPATCH

/* ------------------------------------------------------------------------------------
 * SETNUMFRM_REG Set Numeric Form=op1 (1=sci,2=eng)
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(SETNUMFRM_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - SETNUMFRM R%lu\n", REG_IDX(1));
    if (op1R->int_value < 1 || op1R->int_value > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Form must be 1 (scientific) or 2 (engineering)");
        DISPATCH
    }
    current_frame->num_context.form = (int)op1R->int_value;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 * SETNUMFRM_INT Set Numeric Form=op1 (1=sci,2=eng)
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(SETNUMFRM_INT) CALC_DISPATCH(1)
    DEBUG("TRACE - SETNUMFRM %d\n", (int)op1I);
    if (op1I < 1 || op1I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Form must be 1 (scientific) or 2 (engineering)");
        DISPATCH
    }
    current_frame->num_context.form = (int)op1I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 * GETNUMFRM_REG Get Numeric Form=op1 (1=sci,2=eng)
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(GETNUMFRM_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - GETNUMFRM R%lu\n", REG_IDX(1));
    op1R->int_value = (rxinteger)current_frame->num_context.form;
    DISPATCH

/* ------------------------------------------------------------------------------------
 * SETNUMCAS_REG Set Numeric Case=op1 (1=lower,2=upper)
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(SETNUMCAS_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - SETNUMCAS R%lu\n", REG_IDX(1));
    if (op1R->int_value < 1 || op1R->int_value > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Case must be 1 (lower) or 2 (upper)");
        DISPATCH
    }
    current_frame->num_context.casetype = (int)op1R->int_value;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 * SETNUMCAS_INT Set Numeric Case=op1 (1=lower,2=upper)
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(SETNUMCAS_INT) CALC_DISPATCH(1)
    DEBUG("TRACE - SETNUMCAS %d\n", (int)op1I);
    if (op1I < 1 || op1I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Case must be 1 (lower) or 2 (upper)");
        DISPATCH
    }
    current_frame->num_context.casetype = (int)op1I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 * GETNUMCAS_REG Get Numeric Case=op1 (1=lower,2=upper)
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(GETNUMCAS_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - GETNUMCAS R%lu\n", REG_IDX(1));
    op1R->int_value = (rxinteger)current_frame->num_context.casetype;
    DISPATCH

/* ------------------------------------------------------------------------------------
 * SETNUMSTD_REG Set Numeric Standard=op1 (1=common,2=classic)
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(SETNUMSTD_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - SETNUMSTD R%lu\n", REG_IDX(1));
    if (op1R->int_value < 1 || op1R->int_value > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Standard must be 1 (common) or 2 (classic)");
        DISPATCH
    }
    current_frame->num_context.standard = (int)op1R->int_value;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 * SETNUMSTD_INT Set Numeric Standard=op1 (1=common,2=classic)
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(SETNUMSTD_INT) CALC_DISPATCH(1)
    DEBUG("TRACE - SETNUMSTD %d\n", (int)op1I);
    if (op1I < 1 || op1I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Standard must be 1 (common) or 2 (classic)");
        DISPATCH
    }
    current_frame->num_context.standard = (int)op1I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 * GETNUMSTD_REG Get Numeric Standard=op1 (1=common,2=classic)
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(GETNUMSTD_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - GETNUMSTD R%lu\n", REG_IDX(1));
    op1R->int_value = (rxinteger)current_frame->num_context.standard;
    DISPATCH

/* ------------------------------------------------------------------------------------
 * NUMSCI_INT_INT_INT Setup Scientific Numeric digits=op1, case=op2, std=op3, fuzz=0, form=sci
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(NUMSCI_INT_INT_INT) CALC_DISPATCH(3)
    DEBUG("TRACE - NUMSCI %d,%d,%d\n", (int)op1I, (int)op2I, (int)op3I);
    if (op1I < 5) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Digits must be greater than 4");
        DISPATCH
    }
    if (op2I < 1 || op2I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Case must be 1 (lower) or 2 (upper)");
        DISPATCH
    }
    if (op3I < 1 || op3I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Standard must be 1 (common) or 2 (classic)");
        DISPATCH
    }
    current_frame->num_context.digits = op1I;
    current_frame->num_context.fuzz = 0;
    current_frame->num_context.form = 1; // scientific
    current_frame->num_context.casetype = (int)op2I;
    current_frame->num_context.standard = (int)op3I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 * NUMENG_INT_INT_INT Setup Engineering Numeric digits=op1, case=op2, std=op3, fuzz=0, form=eng
 * ----------------------------------------------------------------------------------- */
    START_INSTRUCTION(NUMENG_INT_INT_INT) CALC_DISPATCH(3)
    DEBUG("TRACE - NUMENG %d,%d,%d\n", (int)op1I, (int)op2I, (int)op3I);
    if (op1I < 5) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Digits must be greater than 4");
        DISPATCH
    }
    if (op2I < 1 || op2I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Case must be 1 (lower) or 2 (upper)");
        DISPATCH
    }
    if (op3I < 1 || op3I > 2) {
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "Numeric Standard must be 1 (common) or 2 (classic)");
        DISPATCH
    }
    current_frame->num_context.digits = op1I;
    current_frame->num_context.fuzz = 0;
    current_frame->num_context.form = 2; // engineering
    current_frame->num_context.casetype = (int)op2I;
    current_frame->num_context.standard = (int)op3I;
    // Sync the numeric context of the decimal plugin
    if (current_frame->decimal) {
        current_frame->decimal->syncNumericContext(current_frame->decimal);
    }
    DISPATCH

/* ====================================================================================
 * Decimal Plugin instructions
 * ==================================================================================== */

/* ------------------------------------------------------------------------------------
 * DECPLNM_REG_REG_REG Get Decimal Plugin Name op1=name op2=description op3=version
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DECPLNM_REG_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - DECPLNM R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_null_string(op1R, current_frame->decimal->base.name);
    set_null_string(op2R, current_frame->decimal->base.description);
    set_null_string(op3R, current_frame->decimal->base.version);
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  LOAD_REG_DECIMAL Load op1 with op2
 *  -----------------------------------------------------------------------------------
*/
    START_INSTRUCTION(LOAD_REG_DECIMAL) CALC_DISPATCH(2)
    DEBUG("TRACE - LOAD R%d,%s\n",(int)REG_IDX(1),op2S->string);
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op2S->string);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Convert decimal string to Decimal                              added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(STOD_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - STOD R%lu\n", REG_IDX(1));
    // Ensure the string is null terminated
    null_terminate_string_buffer(op1R);
    // Convert
    current_frame->decimal->decimalFromString(current_frame->decimal, op1R, op1R->string_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Convert Integer to Decimal                                     added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ITOD_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - ITOD R%lu\n", REG_IDX(1));
    // Convert
    current_frame->decimal->decimalFromInt(current_frame->decimal, op1R, op1R->int_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Convert Boolean to Decimal                                     added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BTOD_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - BTOD R%lu\n", REG_IDX(1));
    // Convert
    current_frame->decimal->decimalFromInt(current_frame->decimal, op1R, op1R->int_value ? 1 : 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
    /* ------------------------------------------------------------------------------------
 * Convert Float to Decimal                                       added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FTOD_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - FTOD R%lu\n",REG_IDX(1));
    current_frame->decimal->decimalFromDouble(current_frame->decimal, op1R, op1R->float_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Convert Decimal to string                                        17. August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DTOS_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - DTOS R%lu\n", REG_IDX(1));
    /* Determine how long the string needs to be */
    size_t string_size = current_frame->decimal->getRequiredStringSize(current_frame->decimal);
    /* Ensure the string buffer is big enough */
    extend_string_buffer(op1R, string_size);
    /* Convert */
    current_frame->decimal->decimalToString(current_frame->decimal, op1R, op1R->string_value);
    op1R->string_length = strlen(op1R->string_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Convert Decimal to integer                                       17. August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DTOI_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - DTOI R%lu\n", REG_IDX(1));
    current_frame->decimal->decimalToInt(current_frame->decimal, op1R, &op1R->int_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Convert Decimal to Boolean
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DTOB_REG)
    CALC_DISPATCH(1)
    DEBUG("TRACE - DTOB R%lu\n", REG_IDX(1));
    op1R->int_value = current_frame->decimal->decimalIsZero(current_frame->decimal, op1R) ? 0 : 1; // Convert to boolean
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DTOF_REG  Convert Decimal Number to Float op1=f2dec(op2)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
*/
    START_INSTRUCTION(DTOF_REG) // label not yet defined
    CALC_DISPATCH(1);
    DEBUG("TRACE - DTOF_REG\n");
    current_frame->decimal->decimalToDouble(current_frame->decimal, op1R, &op1R->float_value);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Decimal addition                                               added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DADD_REG_REG_REG)
    CALC_DISPATCH(3)
    DEBUG("TRACE - DADD R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalAdd(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DADD_REG_REG_DECIMAL  Decimal Add (op1=op2+op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DADD_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DADD R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        current_frame->decimal->decimalAdd(current_frame->decimal, op1R, op2R, op);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Decimal subtraction                                            added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DSUB_REG_REG_REG)
    CALC_DISPATCH(3)
    DEBUG("TRACE - DSUB R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalSub(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DSUB_REG_REG_DECIMAL  Decimal Subtract (op1=op2-op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DSUB_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DSUB R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        current_frame->decimal->decimalSub(current_frame->decimal, op1R, op2R, op);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DSUB_REG_REG_DECIMAL  Decimal Subtract (op1=op2-op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DSUB_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DSUB R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    {
        value *op = decimal_literal_value(current_frame->decimal, op2S->string);
        current_frame->decimal->decimalSub(current_frame->decimal, op1R, op, op3R);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH

/* ------------------------------------------------------------------------------------
 * Decimal Multiply                                               added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DMULT_REG_REG_REG)
    CALC_DISPATCH(3)
    DEBUG("TRACE - DMULT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalMul(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DMULT_REG_REG_DECIMAL Decimal Multiply (op1=op2*op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DMULT_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DMULT R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        current_frame->decimal->decimalMul(current_frame->decimal, op1R, op2R, op);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * Decimal division                                               added August 2024 pej
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DDIV_REG_REG_REG)
    CALC_DISPATCH(3)
    DEBUG("TRACE - DDIV R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DDIV_REG_DECIMAL_REG  Decimal Divide  (op1=op2/op3)                   pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DDIV_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DDIV R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    {
        value *op = decimal_literal_value(current_frame->decimal, op2S->string);
        current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op, op3R);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DDIV_REG_REG_DECIMAL  Decimal Divide (op1=op2/op3)                   pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DDIV_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DDIV R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op2R, op);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * DIDIV_REG_REG_REG Decimal integer division
 * ------------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DIDIV_REG_REG_REG)
    CALC_DISPATCH(3)
    DEBUG("TRACE - DIDIV R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op2R, op3R);
    // Note: This is integer division, so the result is truncated
    current_frame->decimal->decimalTruncate(current_frame->decimal, op1R, op1R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DIDIV_REG_DECIMAL_REG  Decimal Integer Divide  (op1=op2/op3)
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DIDIV_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DIDIV R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    {
        value *op = decimal_literal_value(current_frame->decimal, op2S->string);
        current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op, op3R);
        // Note: This is integer division, so the result is truncated
        current_frame->decimal->decimalTruncate(current_frame->decimal, op1R, op1R);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DIDIV_REG_REG_DECIMAL  Decimal Integer Divide (op1=op2/op3)
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DIDIV_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DIDIV R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        current_frame->decimal->decimalDiv(current_frame->decimal, op1R, op2R, op);
        // Note: This is integer division, so the result is truncated
        current_frame->decimal->decimalTruncate(current_frame->decimal, op1R, op1R);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DEQ_REG_REG_REG  Decimal Equals op1=(op2==op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DEQ_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DEQ R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) == 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DNE_REG_REG_REG  Decimal Not equals op1=(op2!=op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DNE_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DNE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) != 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGT_REG_REG_REG  Decimal Greater than op1=(op2>op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGT_REG_REG_REG) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - DGT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) > 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGTE_REG_REG_REG  Decimal Greater than equals op1=(op2>=op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGTE_REG_REG_REG) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - DGTE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) >= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLT_REG_REG_REG  Decimal Less than op1=(op2<op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLT_REG_REG_REG) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) < 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLTE_REG_REG_REG  Decimal Less than equals op1=(op2<=op3)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLTE_REG_REG_REG) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLTE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) <= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DEQ_REG_REG_DECIMAL  Decimal Equals op1=(op2==op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DEQ_REG_REG_DECIMAL) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - DEQ R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) == 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLTBR_ID_REG_REG  Decimal Less than if (op2<op3) goto op1              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLTBR_ID_REG_REG)
    CALC_DISPATCH(3); // This branch prediction for the condition not being met
    DEBUG("TRACE - DLTBR 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) < 0) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGTBR_ID_REG_REG  Decimal Greater than if (op2>op3) goto op1              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGTBR_ID_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLTBR 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) > 0) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DEQBR_ID_REG_REG  Decimal Equal if (op2=op3) goto op1              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DEQBR_ID_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DEQBR 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (current_frame->decimal->decimalCompare(current_frame->decimal, op2R, op3R) == 0) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DNE_REG_REG_DECIMAL  Decimal Not equals op1=(op2!=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DNE_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DNE R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) != 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGT_REG_REG_DECIMAL  Decimal Greater than op1=(op2>op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGT_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DGT R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) > 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGT_REG_DECIMAL_REG  Decimal Greater than op1=(op2>op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGT_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DGT R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op3R, op2S->string) < 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGTE_REG_REG_DECIMAL  Decimal Greater than equals op1=(op2>=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGTE_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DGTE R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) >= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DGTE_REG_DECIMAL_REG  Decimal Greater than equals op1=(op2>=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DGTE_REG_DECIMAL_REG) // label not yet defined
    CALC_DISPATCH(3);
    DEBUG("TRACE - DGTE R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op3R, op2S->string) <= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLT_REG_REG_DECIMAL  Decimal Less than op1=(op2<op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLT_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLT R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) < 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLT_REG_DECIMAL_REG  Decimal Less than op1=(op2<op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLT_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLT R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op3R, op2S->string) > 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLTE_REG_REG_DECIMAL  Decimal Less than equals op1=(op2<=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLTE_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLTE R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op2R, op3S->string) <= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DLTE_REG_DECIMAL_REG  Decimal Less than equals op1=(op2<=op3)              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DLTE_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DLTE R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    set_int(op1R, current_frame->decimal->decimalCompareString(current_frame->decimal, op3R, op2S->string) >= 0);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DCOPY_REG_REG  Copy Decimal op2 to op1              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DCOPY_REG_REG) // label not yet defined
    CALC_DISPATCH(2);
    DEBUG("TRACE - DCOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
    if (op2R->decimal_value == NULL) {
        // Signal error
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "No Source Decimal Value")
        DISPATCH
    }
    if (op1R == op2R) {
        // NOP
        DISPATCH
    }
    if (op1R->decimal_value == NULL) {
        // Allocate storage for the decimal
        op1R->decimal_value = malloc(op2R->decimal_value_length);
        op1R->decimal_buffer_length = op2R->decimal_value_length;
    }
    else if (op1R->decimal_buffer_length < op2R->decimal_value_length) {
        // Reallocate storage for the decimal
        op1R->decimal_value = realloc(op1R->decimal_value, op2R->decimal_value_length);
        op1R->decimal_buffer_length = op2R->decimal_value_length;
    }
    memcpy(op1R->decimal_value, op2R->decimal_value, op2R->decimal_value_length);
    op1R->decimal_value_length = op2R->decimal_value_length;
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DSEX_REG  Decimal op1 = -op1 (sign change)              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DSEX_REG)
    CALC_DISPATCH(1);
    DEBUG("TRACE - DSEX R%lu\n", REG_IDX(1));
    if (op1R->decimal_value == NULL) {
        // Signal error
        SET_SIGNAL_MSG(RXSIGNAL_INVALID_ARGUMENTS, "No Source Decimal Value")
        DISPATCH
    }
    current_frame->decimal->decimalNeg(current_frame->decimal, op1R, op1R);
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DPOW_REG_REG_REG  op1=op2**op3              pej 17 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DPOW_REG_REG_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DPOW R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
    current_frame->decimal->decimalPow(current_frame->decimal, op1R, op2R, op3R);
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DPOW_REG_REG_DECIMAL  op1=op2**op3              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DPOW_REG_REG_DECIMAL)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DPOW R%lu,R%lu,%s\n", REG_IDX(1), REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        current_frame->decimal->decimalPow(current_frame->decimal, op1R, op2R, op);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  DPOW_REG_DECIMAL_REG  op1=op2**op3              pej 19 Aug 2024
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(DPOW_REG_DECIMAL_REG)
    CALC_DISPATCH(3);
    DEBUG("TRACE - DPOW R%lu,%s,R%lu\n", REG_IDX(1), op2S->string, REG_IDX(3));
    {
        value *op = decimal_literal_value(current_frame->decimal, op2S->string);
        current_frame->decimal->decimalPow(current_frame->decimal, op1R, op, op3R);
        free_decimal_literal_value(op);
    }
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
 * DEXTR_REG_REG_REG Extract decimal to string coefficient and decimal exponent integer
 * R3 contains the decimal value
 * R1 will store the coefficient as a string (or nan, inf, -inf)
 * R2 will store the decimal exponent as an integer
 * Output normalised, rounded to the digits setting decimal places, and
 * trimmed of trailing zeros
 * This instruction is designed to allow the user to format the float as they wish
 */
    START_INSTRUCTION(DEXTR_REG_REG_REG)
    CALC_DISPATCH(3)
    DEBUG("TRACE - DEXTR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    prep_string_buffer(op1R, current_frame->decimal->getRequiredStringSize(current_frame->decimal));
    current_frame->decimal->decimalExtract(current_frame->decimal, op1R->string_value, &(op2R->int_value), op3R);
    op1R->string_length = strlen(op1R->string_value);
    DISPATCH
/* ====================================================================================
 * End of Decimal Plugin instructions
 * ====================================================================================
 */
       START_INSTRUCTION(CALL_FUNC) CALC_DISPATCH(1)
            /* New stackframe - grabbing procedure object from the caller frame */
            {
                proc_runtime *called_function = PROC_OP(1);
                DEBUG("TRACE - CALL %s()\n", called_function->name);
                if (called_function->start == SIZE_MAX) {
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, called_function->name)
                    DISPATCH
                }
                if (called_function->binarySpace == 0) {
                    /* This is a native plugin function */
                    rxvm_callfunc((void *) (called_function->start), 0, NULL, NULL, signal_value);
                    INTERRUPT_FROM_RXPA_SIGNAL(signal_value);
                } else {
                    /* This is a CREXX Procedure */
                    current_frame = frame_f(called_function, 0, current_frame, next_pc, 0);
                    /* Prepare dispatch to procedure as early as possible */
#ifndef NTHREADED
                    current_module = current_frame->procedure->binarySpace->module;
#endif
                    next_pc = &(current_frame->procedure->binarySpace->binary[called_function->start]);
                    CALC_DISPATCH_MANUAL
                    /* No Arguments - so nothing to do */
                }
            }
            DISPATCH

        START_INSTRUCTION(CALL_REG_FUNC) CALC_DISPATCH(2)
            {
                /* Clear target return value register */
                value_zero(op1R);

                proc_runtime *called_function = PROC_OP(2);
                DEBUG("TRACE - CALL R%lu,%s()\n", REG_IDX(1), called_function->name);

                if (called_function->start == SIZE_MAX) {
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, called_function->name);
                    DISPATCH
                }

                if (called_function->binarySpace == 0) {
                    /* This is a native plugin function */
                    rxvm_callfunc((void *) (called_function->start), 0, NULL, op1R, signal_value);
                    INTERRUPT_FROM_RXPA_SIGNAL(signal_value);
                } else {
                    /* This is a CREXX Procedure */
                    /* New stackframe - grabbing a procedure object from the caller frame */
                    current_frame = frame_f(called_function, 0, current_frame, next_pc, op1R);

                    /* Prepare dispatch to procedure as early as possible */
#ifndef NTHREADED
                    current_module = current_frame->procedure->binarySpace->module;
#endif
                    next_pc = &(current_frame->procedure->binarySpace->binary[called_function->start]);
                    CALC_DISPATCH_MANUAL
                    /* No Arguments - so nothing to do */
                }
            }
            DISPATCH

        START_INSTRUCTION(CALL_REG_FUNC_REG) CALC_DISPATCH(3)
            {
                proc_runtime *called_function = PROC_OP(2);
                DEBUG("TRACE - CALL R%lu,%s,R%lu\n", REG_IDX(1), called_function->name, REG_IDX(3));
                if (called_function->start == SIZE_MAX) {
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, called_function->name);
                    DISPATCH
                }

                if (called_function->binarySpace == 0) {
                    /* This is a native plugin function */
                    rxvm_callfunc((void *) (called_function->start), op3R->int_value, (&(op3R)) + 1, op1R,
                                  signal_value);
                    INTERRUPT_FROM_RXPA_SIGNAL(signal_value);
                } else {
                    /* This is a CREXX Procedure */
                    /* New stackframe - grabbing a procedure object from the caller frame */
                    current_frame = frame_f(called_function, (int) op3R->int_value, current_frame, next_pc, op1R);
                    /* Prepare dispatch to procedure as early as possible */
#ifndef NTHREADED
                    current_module = current_frame->procedure->binarySpace->module;
#endif
                    next_pc = &(current_frame->procedure->binarySpace->binary[called_function->start]);
                    CALC_DISPATCH_MANUAL

                    /* Arguments - complex lets never have to change this code! */
                    size_t j =
                            current_frame->procedure->binarySpace->globals +
                            current_frame->procedure->locals + 1; /* Callee register index */
                    size_t k = (pc + 3)->index + 1; /* Caller register index */
                    size_t i;
                    for (i = 0;
                         i < (current_frame->parent->locals[(pc + 3)->index])->int_value;
                         i++, j++, k++) {
                        current_frame->locals[j] = current_frame->parent->locals[k];
                    }
                }
            }
            DISPATCH

        START_INSTRUCTION(DCALL_REG_REG_REG) CALC_DISPATCH(3)
            {
                /* Function pointer is in register 2 */
                proc_runtime *called_function = (proc_runtime *) op2R->int_value;
                DEBUG("TRACE - DCALL R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
                if (called_function->start == SIZE_MAX) {
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, called_function->name);
                    DISPATCH
                }

                if (called_function->binarySpace == 0) {
                    /* This is a native plugin function */
                    rxvm_callfunc((void *) (called_function->start), op3R->int_value, (&(op3R)) + 1, op1R,
                                  signal_value);
                    INTERRUPT_FROM_RXPA_SIGNAL(signal_value);
                } else {
                    /* This is a CREXX Procedure */
                    current_frame = frame_f(called_function, (int) op3R->int_value, current_frame, next_pc, op1R);

                    /* Prepare dispatch to procedure as early as possible */
#ifndef NTHREADED
                    current_module = current_frame->procedure->binarySpace->module;
#endif
                    next_pc = &(current_frame->procedure->binarySpace->binary[called_function->start]);
                    CALC_DISPATCH_MANUAL

                    /* Arguments - complex lets never have to change this code! */
                    size_t j =
                            current_frame->procedure->binarySpace->globals +
                            current_frame->procedure->locals + 1; /* Callee register index */
                    size_t k = (pc + 3)->index + 1; /* Caller register index */
                    size_t i;
                    for (i = 0;
                         i < (current_frame->parent->locals[(pc + 3)->index])->int_value;
                         i++, j++, k++) {
                        current_frame->locals[j] = current_frame->parent->locals[k];
                    }
                }
            }
            DISPATCH

        START_INSTRUCTION(RET)
            DEBUG("TRACE - RET\n");
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                /* back to the parent's stack frame */
                temp_frame = current_frame;
                current_frame = current_frame->parent;
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Copy back arguments for external calls */
                    if (context->ext_proc && context->ext_args) {
                        int k, m;
                        int nargs = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                                temp_frame->procedure->locals]->int_value;
                        for (k = 0, m = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                             k < nargs && k < context->ext_argc;
                             k++, m++) {
                            copy_value(context->ext_args[k], temp_frame->locals[m]);
                        }
                    }
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                         i < num_args;
                         i++, j++) {
                        clear_value(temp_frame->baselocals[j]);
                        free(temp_frame->baselocals[j]);
                    }
                    rc = 0;
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    goto interprt_finished;
                }
                HANDLE_INTERRUPT_ACTION_RETURN()
                free_frame(temp_frame);
#ifndef NTHREADED
                current_module = current_frame->procedure->binarySpace->module;
#endif
                CALC_DISPATCH_MANUAL
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    pc = next_pc;
                    END_INTERRUPT // Breakpoints are not cleared, so we bypass the interrupt check
                }
                DISPATCH
            }

        START_INSTRUCTION(RET_REG)
            DEBUG("TRACE - RET R%lu\n", REG_IDX(1));
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                int return_rc = (int)op1R->int_value;
                /* Set the result register */
                if (current_frame->return_reg) {
                    if (REG_IDX(1) >= current_frame->procedure->locals || /* Not a local */
                        current_frame->locals[REG_IDX(1)] != current_frame->baselocals[REG_IDX(1)]) /* swapped/linked so might not be a local really */
                        copy_value(current_frame->return_reg,
                                   op1R); /* Must do a copy if it could be an argument, object attribute or global because ... */
                    else
                        move_value(current_frame->return_reg,
                                   op1R); /* ... the faster move deletes the source which is ok for locals going out of scope */
                }
                /* back to the parents stack frame */
                temp_frame = current_frame;
                current_frame = current_frame->parent;
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Exiting - grab the int rc */
                    if (temp_frame->return_reg) rc = (int) (temp_frame->return_reg->int_value);
                    else rc = return_rc;

                    /* Copy back arguments for external calls */
                    if (context->ext_proc && context->ext_args) {
                        int k, m;
                        int nargs = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                                temp_frame->procedure->locals]->int_value;
                        for (k = 0, m = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                             k < nargs && k < context->ext_argc;
                             k++, m++) {
                            copy_value(context->ext_args[k], temp_frame->locals[m]);
                        }
                    }
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j =
                                        temp_frame->procedure->binarySpace
                                                ->globals +
                                        temp_frame->procedure->locals + 1;
                            i < num_args;
                            i++, j++) {
                        clear_value(temp_frame->baselocals[j]);
                        free(temp_frame->baselocals[j]);
                    }
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    goto interprt_finished;
                }
                // Set the numeric context of the decimal plugin
                if (current_frame->decimal) {
                    current_frame->decimal->num_context = &current_frame->num_context;
                    current_frame->decimal->syncNumericContext(current_frame->decimal);
                }
                HANDLE_INTERRUPT_ACTION_RETURN()
                free_frame(temp_frame);
#ifndef NTHREADED
                current_module = current_frame->procedure->binarySpace->module;
#endif
                CALC_DISPATCH_MANUAL
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    pc = next_pc;
                    END_INTERRUPT // Breakpoints are not cleared, so we bypass the interrupt check
                }
                DISPATCH
            }

        START_INSTRUCTION(RET_INT)
            DEBUG("TRACE - RET %d\n", (int)op1I);
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                /* Set the result register */
                if (current_frame->return_reg)
                    current_frame->return_reg->int_value = op1I;
                /* back to the parents stack frame */
                temp_frame = current_frame;
                current_frame = current_frame->parent;
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Copy back arguments for external calls */
                    if (context->ext_proc && context->ext_args) {
                        int k, m;
                        int nargs = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                                temp_frame->procedure->locals]->int_value;
                        for (k = 0, m = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                             k < nargs && k < context->ext_argc;
                             k++, m++) {
                            copy_value(context->ext_args[k], temp_frame->locals[m]);
                        }
                    }
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j =
                                        temp_frame->procedure->binarySpace
                                                ->globals +
                                        temp_frame->procedure->locals + 1;
                            i < num_args;
                            i++, j++) {
                        clear_value(temp_frame->baselocals[j]);
                        free(temp_frame->baselocals[j]);
                    }
                    rc = (int) op1I;
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    goto interprt_finished;
                }
                // Set the numeric context of the decimal plugin
                if (current_frame->decimal) {
                    current_frame->decimal->num_context = &current_frame->num_context;
                    current_frame->decimal->syncNumericContext(current_frame->decimal);
                }
                HANDLE_INTERRUPT_ACTION_RETURN()
                free_frame(temp_frame);
#ifndef NTHREADED
                current_module = current_frame->procedure->binarySpace->module;
#endif
                CALC_DISPATCH_MANUAL
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    pc = next_pc;
                    END_INTERRUPT // Breakpoints are not cleared, so we bypass the interrupt check
                }
                DISPATCH
            }
/* ------------------------------------------------------------------------------------
 *  RET_FLOAT                                                        pej 12. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(RET_FLOAT)
            DEBUG("TRACE - RET %.15g\n", op1F);
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                /* Set the result register */
                if (current_frame->return_reg)
                    current_frame->return_reg->float_value = op1F;
                /* back to the parents stack frame */
                temp_frame = current_frame;
                current_frame = current_frame->parent;
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Copy back arguments for external calls */
                    if (context->ext_proc && context->ext_args) {
                        int k, m;
                        int nargs = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                                temp_frame->procedure->locals]->int_value;
                        for (k = 0, m = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                             k < nargs && k < context->ext_argc;
                             k++, m++) {
                            copy_value(context->ext_args[k], temp_frame->locals[m]);
                        }
                    }
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j = temp_frame->procedure->binarySpace->globals +
                                    temp_frame->procedure->locals + 1;
                            i < num_args;
                            i++, j++) {
                        clear_value(temp_frame->baselocals[j]);
                        free(temp_frame->baselocals[j]);
                    }
                    rc = 0;
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    goto interprt_finished;
                }
                // Set the numeric context of the decimal plugin
                if (current_frame->decimal) {
                    current_frame->decimal->num_context = &current_frame->num_context;
                    current_frame->decimal->syncNumericContext(current_frame->decimal);
                }
                HANDLE_INTERRUPT_ACTION_RETURN()
                free_frame(temp_frame);
#ifndef NTHREADED
                current_module = current_frame->procedure->binarySpace->module;
#endif
                CALC_DISPATCH_MANUAL
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    pc = next_pc;
                    END_INTERRUPT // Breakpoints are not cleared, so we bypass the interrupt check
                }
                DISPATCH
            }

            /* ------------------------------------------------------------------------------------
            *  RET_STRING                                                        pej 12. April 2021
            *  -----------------------------------------------------------------------------------
            */
        START_INSTRUCTION(RET_STRING)
            DEBUG("TRACE - RET \"%.*s\"\n", (int)op1S->string_len, op1S->string);
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                unsigned char is_interrupt = current_frame->is_interrupt;
                /* Set the result register */
                if (current_frame->return_reg)
                    set_const_string(current_frame->return_reg, CONSTSTRING_OP(1));
                /* back to the parents stack frame */
                temp_frame = current_frame;
                current_frame = current_frame->parent;
                if (!current_frame) {
                    DEBUG("TRACE - RETURNING FROM MAIN()\n");
                    /* Copy back arguments for external calls */
                    if (context->ext_proc && context->ext_args) {
                        int k, m;
                        int nargs = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                                temp_frame->procedure->locals]->int_value;
                        for (k = 0, m = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                             k < nargs && k < context->ext_argc;
                             k++, m++) {
                            copy_value(context->ext_args[k], temp_frame->locals[m]);
                        }
                    }
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j = temp_frame->procedure->binarySpace->globals +
                            temp_frame->procedure->locals + 1;
                            i < num_args;
                            i++, j++) {
                        clear_value(temp_frame->baselocals[j]);
                        free(temp_frame->baselocals[j]);
                    }
                    rc = 0;
                    free_frame(temp_frame);
                    goto interprt_finished;
                }
                // Set the numeric context of the decimal plugin
                if (current_frame->decimal) {
                    current_frame->decimal->num_context = &current_frame->num_context;
                    current_frame->decimal->syncNumericContext(current_frame->decimal);
                }
                HANDLE_INTERRUPT_ACTION_RETURN()
                free_frame(temp_frame);
#ifndef NTHREADED
                current_module = current_frame->procedure->binarySpace->module;
#endif
                CALC_DISPATCH_MANUAL
                if (is_interrupt == RXSIGNAL_BREAKPOINT) {
                    pc = next_pc;
                    END_INTERRUPT // Breakpoints are not cleared, so we bypass the interrupt check
                }
                DISPATCH
            }

        START_INSTRUCTION(MOVE_REG_REG) CALC_DISPATCH(2) /* Deprecated */
            DEBUG("TRACE - MOVE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            move_value(op1R, op2R);
            DISPATCH

        START_INSTRUCTION(SWAP_REG_REG) CALC_DISPATCH(2) /* Deprecated */
            DEBUG("TRACE - SWAP R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                value *v_temp;
                v_temp = op1R;
                op1R = op2R;
                op2R = v_temp;
            }
            DISPATCH

        START_INSTRUCTION(MKREF_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - MKREF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                void *owner = 0;
                rxvm_ref_owner_kind owner_kind;
                rxvm_reference_cell *cell;

                owner_kind = rxvm_reference_owner_kind_for_storage(current_frame,
                                                                   REG_IDX(2),
                                                                   op2R,
                                                                   &owner);
                cell = rxvm_reference_identity_for_context(&context->references,
                                                           op2R,
                                                           owner_kind,
                                                           owner,
                                                           REG_IDX(2),
                                                           0);
                if (!cell) {
                    SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
                    DISPATCH
                }

                rxvm_mark_reference_lifetime_owner(current_frame, op2R);
                clear_value_contents(op1R);
                rxvm_reference_value_set_payload(op1R, cell);
            }
            DISPATCH

        START_INSTRUCTION(DEREF_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - DEREF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                rxvm_reference_cell *cell = rxvm_reference_payload_cell(op2R);
                value *target = rxvm_reference_cell_target(cell);
                if (!target) {
                    SET_SIGNAL_MSG(RXSIGNAL_REFERENCE_INVALID, "Reference is invalid");
                    DISPATCH
                }
                copy_value(op1R, target);
            }
            DISPATCH

        START_INSTRUCTION(LINKREF_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - LINKREF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                rxvm_reference_cell *cell = rxvm_reference_payload_cell(op2R);
                value *target = rxvm_reference_cell_target(cell);
                if (!target) {
                    SET_SIGNAL_MSG(RXSIGNAL_REFERENCE_INVALID, "Reference is invalid");
                    DISPATCH
                }
                op1R = target;
            }
            DISPATCH

        START_INSTRUCTION(SETREF_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SETREF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                rxvm_reference_cell *cell = rxvm_reference_payload_cell(op1R);
                value *target = rxvm_reference_cell_target(cell);
                if (!target) {
                    SET_SIGNAL_MSG(RXSIGNAL_REFERENCE_INVALID, "Reference is invalid");
                    DISPATCH
                }
                copy_value(target, op2R);
            }
            DISPATCH

        START_INSTRUCTION(REFVALID_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - REFVALID R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                int valid = rxvm_reference_cell_is_valid(rxvm_reference_payload_cell(op2R));
                value_zero(op1R);
                op1R->int_value = valid ? 1 : 0;
            }
            DISPATCH

        START_INSTRUCTION(UNREF_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - UNREF R%lu\n", REG_IDX(1));
            clear_value_contents(op1R);
            DISPATCH

        /* Link attribute op3 of op2 to op1 */
        START_INSTRUCTION(LINKATTR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKATTR R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            if (op3R->int_value < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if (op3R->int_value >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op1R = op2R->attributes[op3R->int_value];
            DISPATCH

        /* Link attribute op3 of op2 to op1 */
        START_INSTRUCTION(LINKATTR_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKATTR R%lu,R%lu,%d\n", REG_IDX(1), REG_IDX(2), (int)op3I);
            if ((int)op3I < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if ((int)op3I >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op1R = op2R->attributes[(int)op3I];
            DISPATCH

        /* Link attribute op3 (1 base) of op2 to op1 */
        START_INSTRUCTION(LINKATTR1_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKATTR1 R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            if (op3R->int_value - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if (op3R->int_value - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op1R = op2R->attributes[op3R->int_value - 1];
            DISPATCH

        /* Link attribute op3 (1 base) of op2 to op1 */
        START_INSTRUCTION(LINKATTR1_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKATTR1 R%lu,R%lu,%d\n", REG_IDX(1), REG_IDX(2), (int)op3I);
            if ((int)op3I - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if ((int)op3I - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op1R = op2R->attributes[(int)op3I - 1];
            DISPATCH

        /* Link op3 to attribute op1 of op2 */
        START_INSTRUCTION(LINKTOATTR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKTOATTR R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            if (op1R->int_value < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if (op1R->int_value >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[op1R->int_value] = op3R;
            DISPATCH

        /* Link op3 to attribute op1 of op2 */
        START_INSTRUCTION(LINKTOATTR_INT_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKTOATTR %d,R%lu,R%lu\n", (int)op1I, REG_IDX(2), REG_IDX(3));
            if ((int)op1I < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if ((int)op1I >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[(int)op1I] = op3R;
            DISPATCH

        /* Link op3 to attribute op1 (1 base) of op2 */
        START_INSTRUCTION(LINKTOATTR1_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKTOATTR1 R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            if (op1R->int_value - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if (op1R->int_value - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[op1R->int_value - 1] = op3R;
            DISPATCH

        /* Link op3 to attribute op1 (1 base) of op2 */
        START_INSTRUCTION(LINKTOATTR1_INT_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKTOATTR1 %d,R%lu,R%lu\n", (int)op1I, REG_IDX(2), REG_IDX(3));
            if ((int)op1I - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if ((int)op1I - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[(int)op1I -1] = op3R;
            DISPATCH

        /* Unlink attribute op1 of op2 */
        START_INSTRUCTION(UNLINKATTR_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - UNLINKATTR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            if (op1R->int_value < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if (op1R->int_value >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[op1R->int_value] = op2R->unlinked_attributes[op1R->int_value];
            DISPATCH

        /* Unlink attribute op1 of op2 */
        START_INSTRUCTION(UNLINKATTR_INT_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - UNLINKATTR %d,R%lu\n", (int)op1I, REG_IDX(2));
            if ((int)op1I < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if ((int)op1I >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[(int)op1I] = op2R->unlinked_attributes[(int)op1I];
            DISPATCH

        /* Unlink attribute op1 (1 base) of op2 */
        START_INSTRUCTION(UNLINKATTR1_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - UNLINKATTR1 R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            if (op1R->int_value - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if (op1R->int_value - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[op1R->int_value - 1] = op2R->unlinked_attributes[op1R->int_value - 1];
            DISPATCH

        /* Unlink attribute op1 (1 base) of op2 */
        START_INSTRUCTION(UNLINKATTR1_INT_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - UNLINKATTR1 %d,R%lu\n", (int)op1I, REG_IDX(2));
            if ((int)op1I - 1 < 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            if ((int)op1I - 1 >= op2R->num_attributes) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
                DISPATCH
            }
            op2R->attributes[(int)op1I - 1] = op2R->unlinked_attributes[(int)op1I - 1];
            DISPATCH

            /* Link op2 to op1 */
        START_INSTRUCTION(LINK_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - LINK R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            op1R = op2R;
            DISPATCH

        /* Link parent-frame-register[op2] to op1 */
        START_INSTRUCTION(METALINKPREG_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - METALINKPREG R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            op1R = current_frame->parent->locals[op2R->int_value];
            DISPATCH

        /* Unlink op1 */
        START_INSTRUCTION(UNLINK_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - UNLINK R%lu\n", REG_IDX(1));
            op1R = (current_frame->baselocals[(pc + 1)->index]);
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  GETATTRS_REG_REG Get Number Attributes op1 = op2.num_attributes
         *  ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(GETATTRS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - GETATTRS R%lu,R%lu\n", REG_IDX(1),REG_IDX(2));
            REG_RETURN_INT(op2R->num_attributes)
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  GETATTRS_REG_REG_INT Get Number Attributes op1 = op2.num_attributes + op3
         *  ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(GETATTRS_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - GETATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
            REG_RETURN_INT(op2R->num_attributes + op3I)
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  SETATTRS_REG_REG Set Number Attributes op1.num_attributes = op2
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(SETATTRS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SETATTRS R%lu,R%lu\n", REG_IDX(1),REG_IDX(2));
            set_num_attributes(op1R, op2RI);
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  SETATTRS_REG_REG Set Number Attributes op1.num_attributes = op2
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(SETATTRS_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - SETATTRS R%lu,%d\n", REG_IDX(1),(int)op2I);
            set_num_attributes(op1R, op2I);
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  SETATTRS_REG_REG_INT Set Number Attributes op1.num_attributes = op2 + op3
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(SETATTRS_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - SETATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2), (int)op3I);
            set_num_attributes(op1R, op2RI + op3I);
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  SETATTRS_REG_INT_INT Set Number Attributes op1.num_attributes = op2 + op3
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(SETATTRS_REG_INT_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - SETATTRS R%lu,%d,%d\n", REG_IDX(1),(int)op2I, (int)op3I);
            set_num_attributes(op1R, op2I + op3I);
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  MINATTRS_REG_REG Ensure min number attributes op1.num_attributes >= op2
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(MINATTRS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - MINATTRS R%lu,R%lu\n", REG_IDX(1),REG_IDX(2));
            if (op2RI > op1R->num_attributes) {
                set_num_attributes(op1R, op2RI);
            }
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  MINATTRS_REG_REG Ensure min number attributes op1.num_attributes >= op2
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(MINATTRS_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - MINATTRS R%lu,%d\n", REG_IDX(1),(int)op2I);
            if (op2I > op1R->num_attributes) {
                set_num_attributes(op1R, op2I);
            }
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  MINATTRS_REG_REG_INT Ensure min number attributes op1.num_attributes >= op2 + op3
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(MINATTRS_REG_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - MINATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
        if (op2RI + op3I > op1R->num_attributes) {
            /* Set the number of attributes to the requested number */
            set_num_attributes(op1R, op2RI + op3I);
        }
        DISPATCH

        /* ------------------------------------------------------------------------------------
         *  MINATTRS_REG_REG_INT Ensure min number attributes op1.num_attributes >= op2 + op3
         * ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(MINATTRS_REG_INT_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - MINATTRS R%lu,%d,%d\n", REG_IDX(1),(int)op2I,(int)op3I);
            if (op2I + op3I > op1R->num_attributes) {
                /* Set the number of attributes to the requested number */
                set_num_attributes(op1R, op2I + op3I);
            }
            DISPATCH

        /* ------------------------------------------------------------------------------------
         *  GETABUFS_REG_REG Get attribute buffer size op1 = op2.max_attributes
         *  ----------------------------------------------------------------------------------- */
        START_INSTRUCTION(GETABUFS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - GETABUFS R%lu,R%lu\n", REG_IDX(1),REG_IDX(1));
            REG_RETURN_INT(op2R->max_num_attributes)
            DISPATCH

        START_INSTRUCTION(INSATTRS_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - INSATTRS R%lu,R%lu,R%lu\n", REG_IDX(1),REG_IDX(2),REG_IDX(3));
            if (rxvm_insert_attributes_checked(op1R, op2RI, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(INSATTRS_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - INSATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
            if (rxvm_insert_attributes_checked(op1R, op2RI, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(INSATTRS_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - INSATTRS R%lu,%d,R%lu\n", REG_IDX(1),(int)op2I,REG_IDX(3));
            if (rxvm_insert_attributes_checked(op1R, op2I, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(INSATTRS_REG_INT_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - INSATTRS R%lu,%d,%d\n", REG_IDX(1),(int)op2I,(int)op3I);
            if (rxvm_insert_attributes_checked(op1R, op2I, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(INSATTRS1_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - INSATTRS1 R%lu,R%lu,R%lu\n", REG_IDX(1),REG_IDX(2),REG_IDX(3));
            if (rxvm_insert_attributes1_checked(op1R, op2RI, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(INSATTRS1_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - INSATTRS1 R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
            if (rxvm_insert_attributes1_checked(op1R, op2RI, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(INSATTRS1_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - INSATTRS1 R%lu,%d,R%lu\n", REG_IDX(1),(int)op2I,REG_IDX(3));
            if (rxvm_insert_attributes1_checked(op1R, op2I, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(INSATTRS1_REG_INT_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - INSATTRS1 R%lu,%d,%d\n", REG_IDX(1),(int)op2I,(int)op3I);
            if (rxvm_insert_attributes1_checked(op1R, op2I, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(DELATTRS_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - DELATTRS R%lu,R%lu,R%lu\n", REG_IDX(1),REG_IDX(2),REG_IDX(3));
            if (rxvm_delete_attributes_checked(op1R, op2RI, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(DELATTRS_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - DELATTRS R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
            if (rxvm_delete_attributes_checked(op1R, op2RI, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(DELATTRS_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - DELATTRS R%lu,%d,R%lu\n", REG_IDX(1),(int)op2I,REG_IDX(3));
            if (rxvm_delete_attributes_checked(op1R, op2I, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(DELATTRS_REG_INT_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - DELATTRS R%lu,%d,%d\n", REG_IDX(1),(int)op2I,(int)op3I);
            if (rxvm_delete_attributes_checked(op1R, op2I, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(DELATTRS1_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - DELATTRS1 R%lu,R%lu,R%lu\n", REG_IDX(1),REG_IDX(2),REG_IDX(3));
            if (rxvm_delete_attributes1_checked(op1R, op2RI, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(DELATTRS1_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - DELATTRS1 R%lu,R%lu,%d\n", REG_IDX(1),REG_IDX(2),(int)op3I);
            if (rxvm_delete_attributes1_checked(op1R, op2RI, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(DELATTRS1_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - DELATTRS1 R%lu,%d,R%lu\n", REG_IDX(1),(int)op2I,REG_IDX(3));
            if (rxvm_delete_attributes1_checked(op1R, op2I, op3RI) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(DELATTRS1_REG_INT_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - DELATTRS1 R%lu,%d,%d\n", REG_IDX(1),(int)op2I,(int)op3I);
            if (rxvm_delete_attributes1_checked(op1R, op2I, op3I) != 0) {
                SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
            }
            DISPATCH

        START_INSTRUCTION(DEC0) CALC_DISPATCH(0)
            /* TODO This is really idec0 - i.e. it does not prime the int */
            DEBUG("TRACE - DEC0\n");
            (current_frame->locals[0]->int_value)--;
            DISPATCH

            /* ------------------------------------------------------------------------------------
         *  DEC1   R1--                                                       pej 7. April 2021
         *  -----------------------------------------------------------------------------------
         */
        START_INSTRUCTION(DEC1) CALC_DISPATCH(0)
            /* TODO This is really idec1 - i.e. it does not prime the int */
            DEBUG("TRACE - DEC1\n");
            (current_frame->locals[1]->int_value)--;
            DISPATCH

            /* ------------------------------------------------------------------------------------
            *  DEC2   op2R--                                                       pej 7. April 2021
            *  -----------------------------------------------------------------------------------
            */
        START_INSTRUCTION(DEC2) CALC_DISPATCH(0)
            /* TODO This is really idec2 - i.e. it does not prime the int */
            DEBUG("TRACE - DEC2\n");
            (current_frame->locals[2]->int_value)--;
            DISPATCH

        START_INSTRUCTION(DEC_REG) CALC_DISPATCH(1)
            /* TODO This is really idec reg - i.e. it does not prime the int */
            DEBUG("TRACE - DEC R%lu\n", REG_IDX(1));
            (current_frame->locals[REG_IDX(1)]->int_value)--;
            DISPATCH

        START_INSTRUCTION(BR_ID)
            DEBUG("TRACE - BR 0x%x\n", (unsigned int)REG_IDX(1));
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
            DISPATCH

            /* For these we optimise for condition to NOT be met because in a loop
             * these ae used to jump out of the loop when the end condition it met
             * (and every little bit helps to improve performance!)
             */

        START_INSTRUCTION(BRT_ID_REG) CALC_DISPATCH(2) /* i.e. if the condition is not met - this helps the
                                the real CPUs branch prediction (in theory) */
            DEBUG("TRACE - BRT 0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2));
            if (op2RI) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
            DISPATCH

        START_INSTRUCTION(BRF_ID_REG) CALC_DISPATCH(2) /* i.e. if the condition is not met - this helps the
                                  the real CPUs branch prediction (in theory) */
            DEBUG("TRACE - BRF 0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2));
            if (!(op2RI)) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
            DISPATCH

        START_INSTRUCTION(BRTF_ID_ID_REG)
            DEBUG("TRACE - BRTF 0x%x,0x%x,R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (op3RI) next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            else next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(2);
            CALC_DISPATCH_MANUAL
            DISPATCH

        START_INSTRUCTION(TIME_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - TIME R%d\n", (int)REG_IDX(1));
            {
                struct timeval tv;
                tzset();
                gettimeofday(&tv, NULL);
                REG_RETURN_INT(tv.tv_sec - timezone)
            }
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  XTIME return time properties                                  pej 02. December 2021
 * ------------------------------------------------------------------------------------
 */
        START_INSTRUCTION(XTIME_REG_STRING) CALC_DISPATCH(2)
        DEBUG("TRACE - XTIME R%d,\"%s\"\n", (int)REG_IDX(1),(CONSTSTRING_OP(2))->string);

            tzset();
            switch ((CONSTSTRING_OP(2))->string[0]) {
                case 'Z':
                    tzset();
                    op1R->int_value  = timezone;
                    break;
                case 'T':  op1R->int_value  = (rxinteger)clock(); break;
                case 'C':  op1R->int_value  = CLOCKS_PER_SEC; break;
                case 'N':  {
                     prep_string_buffer(op1R,2*SMALLEST_STRING_BUFFER_LENGTH); // Large enough for both time zone names
                     op1R->string_length = snprintf(op1R->string_value,2*SMALLEST_STRING_BUFFER_LENGTH,"%s;%s",tzname[0],tzname[1]);
                     op1R->string_pos = 0;
                     break;  // time zone names
                }
                case 'U':  {
                     time_t ctime;
                     rxinteger tm;
                     struct timeval tv;
                     struct tm *tmdata;
                     ctime = time(NULL);
                     tmdata = localtime(&ctime);
                     tzset();
                     tm=((tmdata->tm_hour * 3600) + (tmdata->tm_min  * 60) + (tmdata->tm_sec))+timezone;
                     gettimeofday(&tv, NULL);
                     op1R->int_value = tm*1000000+tv.tv_usec;
                     break;  // UTC Time
                }
            }
            DISPATCH

/* ---------------------------------------------------------------------------------
 *  MTIME get time of the day in microseconds                      pej 31. October 2021
 * ------------------------------------------------------------------------------------
 */
        START_INSTRUCTION(MTIME_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - MTIME R%d\n", (int)REG_IDX(1));
            {
                rxinteger tm;
                struct timeval tv;
                //struct timezone tz;
                time_t	ctime;
                struct tm *tmdata;

                ctime = time(NULL);
                tmdata = localtime(&ctime);
                tm =
                        ((tmdata->tm_hour * 3600) + (tmdata->tm_min * 60) +
                        (tmdata->tm_sec));
                gettimeofday(&tv, NULL);
                REG_RETURN_INT(tm * 1000000 + tv.tv_usec)
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  TRIMR  Trim right                                                 pej 7. April 2021
 * ------------------------------------------------------------------------------------
 */
        START_INSTRUCTION(TRIMR_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - TRIMR (DEPRECATED) R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            {
                int i = op1R->string_length - 1;
                while (i >= 0 && op1R->string_value[i] == ' ') {
                    i--;
                }
                op1R->string_length = i + 1;
                null_terminate_string_buffer(op1R);
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  TRIML  Trim left                                                  pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(TRIML_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - TRIML (DEPRECATED) R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            /* TODO - UTF etc */
            {
                int j = op1R->string_length - 1;
                int i = 0;
                while (i <= j && op1R->string_value[i] == ' ') i++;

                if (i >= j) {
                    op1R->string_length = 0;
                    null_terminate_string_buffer(op1R);
                } else {
                    op1R->string_length = op1R->string_length - i;
                    memcpy(op1R->string_value, op1R->string_value + i,
                           op1R->string_length);
                    null_terminate_string_buffer(op1R);
                }
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  INC0   R0++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INC0) CALC_DISPATCH(0)
            DEBUG("TRACE - INC0\n");
            REG_VAL(0)->int_value++;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  INC1   R1++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INC1) CALC_DISPATCH(0)
            DEBUG("TRACE - INC1\n");
            REG_VAL(1)->int_value++;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  INC2   op2R++                                                       pej 7. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INC2) CALC_DISPATCH(0)
            DEBUG("TRACE - INC2\n");
            REG_VAL(2)->int_value++;
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  ISEX   op1 = -op1  decimal                                    pej 2. September 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISEX_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - INC R%lu\n", REG_IDX(1));
            (current_frame->locals[REG_IDX(1)]->int_value)=0-(current_frame->locals[REG_IDX(1)]->int_value);
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  FSEX   op1 = -op1  decimal                                    pej 2. September 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FSEX_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - INC R%lu\n", REG_IDX(1));
            (current_frame->locals[REG_IDX(1)]->float_value)=0-(current_frame->locals[REG_IDX(1)]->float_value);
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  ISUB_REG_REG_INT: Integer Subtract (op1=op2-op3)               pej 8. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISUB_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - ISUB R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI - op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ISUB_REG_INT_REG: Integer Subtract (op1=op2-op3)               pej 8. April 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISUB_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ISUB R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I - op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  AND_REG_REG_REG  Int Logical AND op1=(op2 && op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(AND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - AND R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI && op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  OR_REG_REG_REG  Int Logical OR op1=(op2 || op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(OR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - OR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI || op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  NOT_REG_REG  Int Logical NOT op1=!op2
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(NOT_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - NOT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            if (op2RI) REG_RETURN_INT(0)
            else REG_RETURN_INT(1)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IEQ_REG_REG_REG  Int Equals op1=(op2==op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IEQ_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI == op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IEQ_REG_REG_INT  Int Equals op1=(op2==op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IEQ_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IEQ R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI == op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  INE_REG_REG_REG  Int Equals op1=(op2!=op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - INE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI != op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  INE_REG_REG_INT  Int Equals op1=(op2!=op3)                           pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INE_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - INE R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI != op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGT_REG_REG_REG  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI > op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGT_REG_REG_INT  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGT_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IGT R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI > op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGT_REG_INT_REG  Int Greater than op1=(op2>op3)                      pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGT_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IGT R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I > op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ILT_REG_REG_REG  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ILT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI < op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ILT_REG_REG_INT  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILT_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - ILT R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI < op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ILT_REG_INT_REG  Int Less than op1=(op2<op3)                         pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILT_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ILT R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I < op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGTE_REG_REG_REG  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGTE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IGTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI >= op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGTE_REG_REG_INT  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGTE_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IGTE R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI >= op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGTE_REG_INT_REG  Int Greater Equal than op1=(op2>=op3)              pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IGTE_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IGTE R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I >= op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ILTE_REG_REG_REG  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILTE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ILTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI <= op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ILTE_REG_REG_INT  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILTE_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - ILTE R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI <= op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ILTE_REG_INT_REG  Int Less Equal than op1=(op2<=op3)                 pej 9 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ILTE_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ILTE R%d,%d,r%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I <= op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IGTBR_ID_REG_REG  if op2>op3 ; goto op1                             pej 12 June 2023
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IGTBR_ID_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - IGTBR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),(int)REG_IDX(3));
    if (op2RI > op3RI) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  ILTBR_ID_REG_REG  if op2<op3 ; goto op1                             pej 14 June 2023
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(ILTBR_ID_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - ILTBR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),(int)REG_IDX(3));
    if (op2RI < op3RI) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  FEQ_REG_REG_REG  Float Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FEQ_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF == op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FEQ_REG_REG_FLOAT  Float Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FEQ_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FEQ R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF == op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FNE_REG_REG_REG  Float Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FNE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FNE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF != op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FNE_REG_REG_FLOAT  Float Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FNE_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FNE R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF != op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FGT_REG_REG_REG  Float Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF > op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FGT_REG_REG_FLOAT  Float Greater than op1=(op2>op3)
 *
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGT_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FGT R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF > op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FGT_REG_FLOAT_REG  Float Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGT_REG_FLOAT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FGT R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F > op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FLT_REG_REG_REG  Float Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FLT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF < op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FLT_REG_REG_FLOAT  Float Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLT_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FLT R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF < op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FLT_REG_FLOAT_REG  Float Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLT_REG_FLOAT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FLT R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F < op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FGTE_REG_REG_REG  Float Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGTE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FGTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF >= op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FGTE_REG_REG_FLOAT  Float Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGTE_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FGTE R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF >= op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FGTE_REG_FLOAT_REG  Float Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FGTE_REG_FLOAT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FGTE R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F >= op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FLTE_REG_REG_REG  Float Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLTE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FLTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RF <= op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FLTE_REG_REG_FLOAT  Float Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLTE_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FLTE R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_INT(op2RF <= op3F)
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  FLTE_REG_FLOAT_REG  Float Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FLTE_REG_FLOAT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FLTE R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_INT(op2F <= op3RF)
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  FGTBR_ID_REG_REG  if op2>op3 ; goto op1                            pej 14 June 2023
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FGTBR_ID_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - FGTBR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),(int)REG_IDX(3));
    if (op2RF > op3RF) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  FLTBR_ID_REG_REG  if op2>op3 ; goto op1                            pej 14 June 2023
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FLTBR_ID_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - FLTBR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2),(int)REG_IDX(3));
    if (op2RF < op3RF) {
        next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
        CALC_DISPATCH_MANUAL
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  SEQ_REG_REG_REG  String Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SEQ_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(!string_cmp_value(op2R, op3R))
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SEQ_REG_REG_STRING String Equals op1=(op2==op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SEQ_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SEQ R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(!string_cmp_const(op2R, op3S))
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  RSEQ_REG_REG_REG  String Equals op1=(op2=op3) non strict REXX comparison  pej 29. Nov 2021
 *  -----------------------------------------------------------------------------------
*/
        START_INSTRUCTION(RSEQ_REG_REG_REG) CALC_DISPATCH(3)
        {
            DEBUG("TRACE - RSEQ R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            int ch;
            int p1, p2;
            int len1, len2;

            REQUIRE_VALID_UTF8_REGISTER(op2R);
            REQUIRE_VALID_UTF8_REGISTER(op3R);
            GETSTRLEN(len1, op2R)
            GETSTRLEN(len2, op3R)

            // step 1 find last not blank character
            for (p1 = len1 - 1; p1 >= 0; p1--, len1--) {
                GETSTRCHAR(ch, op2R, p1)
                if (ch != ' ') break;
            }
            for (p2 = len2 - 1; p2 >= 0; p2--, len2--) {
                GETSTRCHAR(ch, op3R, p2)
                if (ch != ' ') break;
            }

            // step 2 find first non blank
            for (p1 = 0; p1 < len1; p1++) {
                GETSTRCHAR(ch, op2R, p1)
                if (ch != ' ') break;
            }
            for (p2 = 0; p2 < len2; p2++) {
                GETSTRCHAR(ch, op3R, p2)
                if (ch != ' ') break;
            }
            if (len1 - p1 != len2 - p2) REG_RETURN_INT(0)
            else {
                if (string_cmp(op2R->string_value + p1, len1 - p1,
                           op3R->string_value + p2, len2 - p2) == 0)
                    REG_RETURN_INT(1)
                else REG_RETURN_INT(0)
            }
        }
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  RSEQ_REG_REG_STRING String Equals op1=(op2=op3)  non strict REXX comparison
 *  TODO !!! not yet implemented !!!
 *  -----------------------------------------------------------------------------------
*/
        START_INSTRUCTION(RSEQ_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - RSEQ R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                  REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                  (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SNE_REG_REG_REG  String Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SNE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SNE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) != 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SNE_REG_REG_STRING  String Not Equals op1=(op2!=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SNE_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SNE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(string_cmp_const(op2R, op3S) != 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SGT_REG_REG_REG  String Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) > 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SGT_REG_REG_STRING  String Greater than op1=(op2>op3)
 *
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGT_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SGT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);

            REG_RETURN_INT(string_cmp_const(op2R, op3S) > 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SGT_REG_STRING_REG  String Greater than op1=(op2>op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGT_REG_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SGT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) < 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SLT_REG_REG_REG  String Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SLT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) < 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SLT_REG_REG_STRING  String Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLT_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SLT R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);

            REG_RETURN_INT(string_cmp_const(op2R, op3S) < 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SLT_REG_STRING_REG  String Less than op1=(op2<op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLT_REG_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SLT R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) > 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SGTE_REG_REG_REG  String Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGTE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SGTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) >= 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SGTE_REG_REG_STRING  String Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGTE_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SGTE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);

            REG_RETURN_INT(string_cmp_const(op2R, op3S) >= 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SGTE_REG_STRING_REG  String Greater Equal than op1=(op2>=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SGTE_REG_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SGTE R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) <= 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SLTE_REG_REG_REG  String Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLTE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SLTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(string_cmp_value(op2R, op3R) <= 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SLTE_REG_REG_STRING  String Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLTE_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - SLTE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1),
                REG_IDX(2), (int) (CONSTSTRING_OP(3))->string_len,
                (CONSTSTRING_OP(3))->string);
            REG_RETURN_INT(string_cmp_const(op2R, op3S) <= 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SLTE_REG_STRING_REG  String Less Equal than op1=(op2<=op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SLTE_REG_STRING_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SLTE R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                (int) (CONSTSTRING_OP(2))->string_len,
                (CONSTSTRING_OP(2))->string, REG_IDX(3));
            REG_RETURN_INT(string_cmp_const(op3R, op2S) >= 0)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  COPY_REG_REG  Copy op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(COPY_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - COPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            copy_value(op1R, op2R);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SCOPY_REG_REG  Copy String op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SCOPY_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SCOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            copy_string_value(op1R, op2R);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ICOPY_REG_REG  Copy Integer op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ICOPY_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - ICOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            op1R->int_value = op2R->int_value;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FCOPY_REG_REG  Copy Float op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FCOPY_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - FCOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            op1R->float_value = op2R->float_value;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ACOPY_REG_REG Copy status Attributes op2 to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ACOPY_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - ACOPY R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            op1R->status.all_type_flags = op2R->status.all_type_flags;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  INC_REG  Increment Int (op1++)                                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INC_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - INC R%lu\n", REG_IDX(1));
            (current_frame->locals[REG_IDX(1)]->int_value)++;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IDIV_REG_REG_INT  Integer Divide (op1=op2/op3)                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IDIV_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IDIV R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI / op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IDIV_REG_INT_REG  Integer Divide (op1=op2/op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IDIV_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IDIV R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I / op3RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  IDIV_REG_REG_REG  Integer Divide (op1=op2/op3)                      pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IDIV_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IDIV R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI / op3RI)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IMOD_REG_REG_INT  Integer Modulo (op1=op2 & op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IMOD_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IMOD R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI % op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IMOD_REG_INT_REG  Integer Modulo (op1=op2 % op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IMOD_REG_INT_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IMOD R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));
            REG_RETURN_INT(op2I % op3RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  IMOD_REG_REG_REG  Integer Modulo (op1=op2 % op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IMOD_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IMOD R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI % op3RI)
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  FMOD_REG_REG_FLOAT Float Modulo (op1=op2 & op3)
 *  -----------------------------------------------------------------------------------
*/
    START_INSTRUCTION(FMOD_REG_REG_FLOAT) CALC_DISPATCH(3)
    DEBUG("TRACE - FMOD R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
    REG_RETURN_FLOAT(fmod(op2RF, op3F))
    DISPATCH
/* ------------------------------------------------------------------------------------
*  FMOD_REG_FLOAT_REG  Float Modulo (op1=op2 % op3)
*  -----------------------------------------------------------------------------------
*/
START_INSTRUCTION(FMOD_REG_FLOAT_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - FMOD R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
    REG_RETURN_FLOAT(fmod(op2F, op3RF))
    DISPATCH
/* -----------------------------------------------------------------------------------
*  FMOD_REG_REG_REG  Float Modulo (op1=op2 % op3)
*  -----------------------------------------------------------------------------------
*/
START_INSTRUCTION(FMOD_REG_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - FMOD R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    REG_RETURN_FLOAT(fmod(op2RF, op3RF))
    DISPATCH



/* ------------------------------------------------------------------------------------
 *  DMOD_REG_REG_DECIMAL Decimal Modulo (op1=op2 & op3)
 *  -----------------------------------------------------------------------------------
*/
    START_INSTRUCTION(DMOD_REG_REG_DECIMAL) CALC_DISPATCH(3)
    DEBUG("TRACE - DMOD R%d,R%d,%s\n", (int)REG_IDX(1), (int)REG_IDX(2), op3S->string);
    {
        value *op = decimal_literal_value(current_frame->decimal, op3S->string);
        value *work = value_f();
        // First, Calculate the decimal division (truncated)
        current_frame->decimal->decimalDiv(current_frame->decimal, work, op2R, op);
        current_frame->decimal->decimalTruncate(current_frame->decimal, work, work);
        // Now calculate the remainder
        current_frame->decimal->decimalMul(current_frame->decimal, work, work, op);
        current_frame->decimal->decimalSub(current_frame->decimal, op1R, op2R, work);
        // Clear the temporary value
        clear_value(work);
        free(work);
        free_decimal_literal_value(op);
    }
    // Check for errors
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
*  FMOD_REG_DECIMAL_REG  Decimal Modulo (op1=op2 % op3)
*  -----------------------------------------------------------------------------------
*/
START_INSTRUCTION(DMOD_REG_DECIMAL_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - FMOD R%d,%s,R%d\n", (int)REG_IDX(1), op2S->string, (int)REG_IDX(3));
    {
        value *op = decimal_literal_value(current_frame->decimal, op2S->string);
        value *work = value_f();
        // First, Calculate the decimal division (truncated)
        current_frame->decimal->decimalDiv(current_frame->decimal, work, op, op3R);
        current_frame->decimal->decimalTruncate(current_frame->decimal, work, work);
        // Now calculate the remainder
        current_frame->decimal->decimalMul(current_frame->decimal, work, work, op3R);
        current_frame->decimal->decimalSub(current_frame->decimal, op1R, op, work);
        // Clear the temporary value
        clear_value(work);
        free(work);
        free_decimal_literal_value(op);
    }
    // Check for errors
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* -----------------------------------------------------------------------------------
*  FMOD_REG_REG_REG Decimal Modulo (op1=op2 % op3)
*  -----------------------------------------------------------------------------------
*/
START_INSTRUCTION(DMOD_REG_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - DMOD R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    {
        value *work = value_f();
        // First, Calculate the decimal division (truncated)
        current_frame->decimal->decimalDiv(current_frame->decimal, work, op2R, op3R);
        current_frame->decimal->decimalTruncate(current_frame->decimal, work, work);
        // Now calculate the remainder
        current_frame->decimal->decimalMul(current_frame->decimal, work, work, op3R);
        current_frame->decimal->decimalSub(current_frame->decimal, op1R, op2R, work);
        // Clear the temporary value
        clear_value(work);
        free(work);
    }
    // Check for errors
    RXSIGNAL_IF_RXVM_PLUGIN_ERROR(current_frame->decimal)
    DISPATCH
/* ------------------------------------------------------------------------------------
  *  IOR_REG_REG_REG bitwise OR (op1=op2|op3)                           pej 17 Oct 2021
  *  -----------------------------------------------------------------------------------
  */
        START_INSTRUCTION(IOR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IOR R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI | op3RI)
            DISPATCH
 /* -----------------------------------------------------------------------------------
  *  IOR_REG_REG_INT  bitwise OR (op1=op2|op3)                          pej 17 Oct 2021
  *  ----------------------------------------------------------------------------------
  */
        START_INSTRUCTION(IOR_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IOR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI | op3I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  IAND_REG_REG_INT  bitwise AND (op1=op2&op3)                         pej 17 Oct 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IAND_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IAND R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI & op3I)
            DISPATCH
/* -----------------------------------------------------------------------------------
 *  IAND_REG_REG_REG  bitwise AND (op1=op2&op3)                        pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IAND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IAND R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI & op3RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  IXOR_REG_REG_REG  bitwise XOR (op1=op2^op3)                        pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IXOR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IXOR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI ^ op3RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  IXOR_REG_REG_INT  bitwise XOR (op1=op2^op3)                        pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IXOR_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IXOR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI ^ op3I)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  ISHL_REG_REG_REG  bitwise shift logical left (op1=op2<<op3)         pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISHL_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ISHL R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI << op3RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  ISHL_REG_REG_INT  bitwise shift logical left (op1=op2<<op3)         pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISHL_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - ISHL R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI << op3I)
            DISPATCH
/* -----------------------------------------------------------------------------------
 *  ISHR_REG_REG_REG  bitwise shift logical right (op1=op2>>op3)       pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISHR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - ISHR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_INT(op2RI >> op3RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  ISHR_REG_REG_INT  bitwise shift logical right (op1=op2>>op3)       pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(ISHR_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - IXSHL R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            REG_RETURN_INT(op2RI >> op3I)
            DISPATCH
/* -----------------------------------------------------------------------------------
 *  INOT_REG_REG  inverts all bits of an integer (op1=~op2)            pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INOT_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - INOT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            REG_RETURN_INT(~op2RI)
            DISPATCH

/* -----------------------------------------------------------------------------------
 *  INOT_REG_INT  inverts all bits of an integer (op1=~op2)            pej 17 Oct 2021
 *  ----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(INOT_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - INOT R%d,R%d\n", (int)REG_IDX(1), (int)op2I);
            REG_RETURN_INT(~op2I)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SAY_INT  Say op1                                                    pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SAY_INT) CALC_DISPATCH(1)
            DEBUG("TRACE - SAY %d\n", (int)op1I);
#ifdef __32BIT__
            rxvm_mprintf("%ld\n", op1I);
#else
            rxvm_mprintf("%lld\n", op1I);
#endif
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SAY_CHAR  Say op1                                                   pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SAY_CHAR) CALC_DISPATCH(1)
            DEBUG("TRACE - SAY \'%c\'\n", (pc + (1))->cconst);
            rxvm_mprintf("%c\n", (pc + (1))->cconst);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SAY_FLOAT  Say op1                                                  pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(SAY_FLOAT) CALC_DISPATCH(1)
            DEBUG("TRACE - SAY %.15g\n", op1F);
            rxvm_mprintf("%.15g\n", op1F);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  LOAD_REG_FLOAT  Load op1 with op2                                   pej 10 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(LOAD_REG_FLOAT) CALC_DISPATCH(2)
            DEBUG("TRACE - LOAD R%d,%.15g\n",(int)REG_IDX(1),op2F);
            REG_RETURN_FLOAT(op2F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FADD_REG_REG_REG  Float Add (op1=op2+op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FADD_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FADD R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF + op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FSUB_REG_REG_REG  Float Sub (op1=op2-op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */

        START_INSTRUCTION(FSUB_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FSUB R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF - op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FDIV_REG_REG_REG  Float Div (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FDIV_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FDIV R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF / op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FIDIV_REG_REG_REG  Integer Div for Float (op1=op2/op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FIDIV_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FIDIV R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(trunc(op2RF / op3RF))
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FMULT_REG_REG_REG  Float Mult (op1=op2/op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FMULT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FMULT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2RF * op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FADD_REG_REG_FLOAT  Float Add (op1=op2+op3)                          pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FADD_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FADD R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF + op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FSUB_REG_REG_FLOAT  Float Sub (op1=op2-op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FSUB_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FSUB R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF - op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FDIV_REG_REG_FLOAT  Float Div (op1=op2/op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FDIV_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FDIV R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF / op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FIDIV_REG_REG_FLOAT  Integer Div for Float(op1=op2/op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FIDIV_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FIDIV R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(trunc(op2RF / op3F))
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FMULT_REG_REG_FLOAT  Float Mult (op1=op2/op3)                       pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FMULT_REG_REG_FLOAT) CALC_DISPATCH(3)
            DEBUG("TRACE - FMULT R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
            REG_RETURN_FLOAT(op2RF * op3F)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FSUB_REG_FLOAT_REG  Float Sub (op1=op2-op3)                         pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FSUB_REG_FLOAT_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - FSUB R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2F - op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FDIV_REG_FLOAT_REG  Float Div (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FDIV_REG_FLOAT_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - FDIV R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
            REG_RETURN_FLOAT(op2F / op3RF)
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FIDIV_REG_FLOAT_REG  Integer Div for Float (op1=op2/op3)                           pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FIDIV_REG_FLOAT_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - FIDIV R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
        REG_RETURN_FLOAT(trunc(op2F / op3RF))
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  FPOW_REG_REG_FLOAT  op1=op2**op3 Float operationn                   pej 3 March 2022
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FPOW_REG_REG_FLOAT) CALC_DISPATCH(3)
    DEBUG("TRACE - DIVF R%d,R%d,%.15g\n", (int)REG_IDX(1), (int)REG_IDX(2), op3F);
    {
        REG_RETURN_FLOAT(pow(op2RF,op3F))
    }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  FPOW_REG_FLOAT_REG  op1=op2**op3 Float operation
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FPOW_REG_FLOAT_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - DIVF R%d,%.15g,R%d\n", (int)REG_IDX(1), op2F, (int)REG_IDX(3));
    {
        REG_RETURN_FLOAT(pow(op2F,op3RF))
    }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  FPOW_REG_REG_REG  op1=op2**op3 Float operation                     pej 3 March 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FPOW_REG_REG_REG) CALC_DISPATCH(3)
    {
        DEBUG("TRACE - FPOW R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2),
              (int) REG_IDX(3));
        REG_RETURN_FLOAT(pow(op2RF,op3RF))
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  IPOW_REG_REG_REG  op1=op2**op2w Integer operation
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IPOW_REG_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - IPOW R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));

        op1R->int_value = ipow(op2R->int_value, op3R->int_value);
        if (!op1R->int_value) SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  IPOW_REG_REG_INT  op1=op2**op2w Integer operationn
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IPOW_REG_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - IPOW R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);

        op1R->int_value = ipow(op2R->int_value, op3I);
        if (!op1R->int_value) SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  IPOW_REG_INT_REG  op1=op2**op2w Integer operationn
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(IPOW_REG_INT_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - IPOW R%d,%d,R%d\n", (int)REG_IDX(1), (int)op2I, (int)REG_IDX(3));

        op1R->int_value = ipow(op2I, op3R->int_value);
        if (!op1R->int_value) SET_SIGNAL(RXSIGNAL_OVERFLOW_UNDERFLOW);
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  LINKARG_REG_REG_INT  Link args[op2+op3] to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(LINKARG_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - LINKARG R%d,R%d,%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            op1R = current_frame->locals[op2RI + op3I +
                                         current_frame->procedure->binarySpace->globals +
                                         current_frame->procedure->locals];
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  LINKARG_REG_INT  Link args[op2] to op1
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(LINKARG_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - LINKARG R%d,%d\n", (int)REG_IDX(1), (int)op2I);
            op1R = current_frame->locals[op2I +
                                         current_frame->procedure->binarySpace->globals +
                                         current_frame->procedure->locals];
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ITOS_REG  Set register string value from its int value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(ITOS_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - ITOS R%lu\n", REG_IDX(1));
            int_to_string(&(current_frame->num_context), work1, op1R);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FTOS_REG  Set register string value from its float value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(FTOS_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - FTOS R%lu\n", REG_IDX(1));
            float_to_string(&(current_frame->num_context), work1, op1R);
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  ITOF_REG  Set register float value from its int value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(ITOF_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - ITOF R%lu\n", REG_IDX(1));
            op1R->float_value = op1R->int_value;
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  FTOI_REG  Set register int value from its float value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(FTOI_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - FTOI R%lu\n", REG_IDX(1));
            int_from_float(op1R);
            if (op1R->float_value != (double)op1R->int_value) {
                SET_SIGNAL(RXSIGNAL_CONVERSION_ERROR);
            }
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  FTOB_REG  Set register boolean (int set to 1 or 0) value from its float value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(FTOB_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - FTOB R%lu\n", REG_IDX(1));
            int_from_float(op1R);

            if (op1R->float_value) op1R->int_value = 1;
            else op1R->int_value = 0;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ITOB_REG  Set register boolean (int set to 1 or 0) value from its int value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(ITOB_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - ITOB R%lu\n", REG_IDX(1));

            if (op1R->int_value) op1R->int_value = 1;
            else op1R->int_value = 0;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  ITOB_REG  Set register boolean (int set to 1 or 0) value from its string value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(STOB_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - STOB R%lu\n", REG_IDX(1));

            if (op1R->string_length != 1) op1R->int_value = 0;
            else {
                if (op1R->string_value[0] == '1') {
                    op1R->int_value = 1;
                } else {
                    op1R->int_value = 0;
                }
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  BTOI_REG  Set register integer value from its boolean value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(BTOI_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - BTOI R%lu\n", REG_IDX(1));
            if (op1R->int_value) op1R->int_value = 1;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  BTOF_REG  Set register float value from its boolean value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(BTOF_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - BTOF R%lu\n", REG_IDX(1));
            if (op1R->int_value) op1R->float_value = 1.0;
            else op1R->float_value = 0.0;
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  BTOS_REG  Set register string value from its boolean value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(BTOS_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - BTOS R%lu\n", REG_IDX(1));
            if (op1R->int_value) set_null_string(op1R,"1");
            else set_null_string(op1R,"0");
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  STOI_REG  Set register int value from its string value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(STOI_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - STOI R%lu\n", REG_IDX(1));
            /* Convert a string to a integer - returns 1 on error */
            if (string2integer(&op1R->int_value, op1R->string_value, op1R->string_length)) {
                SET_SIGNAL(RXSIGNAL_CONVERSION_ERROR);
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  STOF_REG  Set register float value from its string value
 *  -----------------------------------------------------------------------------------*/
        START_INSTRUCTION(STOF_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - STOF R%lu\n", REG_IDX(1));
            /* Convert a string to a float - returns 1 on error */
            if (string2float(&op1R->float_value, op1R->string_value, op1R->string_length)) {
                SET_SIGNAL(RXSIGNAL_CONVERSION_ERROR);
            }
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  FFORMAT_REG_REG_REG  Set string from float use format string   pej 3. November 2021
 *  DEPRECATED - use FEXTR_REG_REG_REG
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FFORMAT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FFORMAT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            prep_string_buffer(op1R,SMALLEST_STRING_BUFFER_LENGTH); // Large enough for a float
            null_terminate_string_buffer(op3R);    // terminate format string explicitly, rexx vars aren't!
            op1R->string_length = snprintf(op1R->string_value,SMALLEST_STRING_BUFFER_LENGTH,op3R->string_value,op2R->float_value);
            op1R->string_pos = 0;
  #ifndef NUTF8
            op1R->string_char_pos = 0;
            op1R->string_chars = op1R->string_length;
  #endif
            DISPATCH
/* ------------------------------------------------------------------------------------
 * FEXTR_REG_REG_REG Extract float to string coefficient and decimal exponent integer
 * R3 contains the float value
 * R1 will store the coefficient as a string (or nan, inf, -inf)
 * R2 will store the decimal exponent as an integer
 * Output normalised, rounded to DBL_DIG (typically 15) decimal places (i.e. DBL_DIG-1 fractional digits), and
 * trimmed of trailing zeros
 * This instruction is designed to allow the user to format the float as they wish
 */
        START_INSTRUCTION(FEXTR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FEXTR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            extract_double_decimal(&(current_frame->num_context), op1R, op2R, op3R->float_value);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  STRLOWER_REG_REG  translate string into lower case string              pej 23.10.21
 *  -----------------------------------------------------------------------------------
 */
// TODO: what to do if there is a length change of chars during translation
            START_INSTRUCTION(STRLOWER_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - STRLOWER R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                set_value_string(op1R, op2R);
                null_terminate_string_buffer(op1R); /* the logic requires a null terminator */
#ifdef NUTF8
                char *c;
                for (c = op1R->string_value; *c; ++c) *c = (char)tolower(*c);
#else
                REQUIRE_VALID_UTF8_REGISTER(op1R);
                utf8lwr(op1R->string_value);
#endif
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  STRUPPER_REG_REG  translate string into upper case string              pej 23.10.21
 *  -----------------------------------------------------------------------------------
 */
// TODO: what to do if there is a length change of chars during translation
            START_INSTRUCTION(STRUPPER_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - STRUPPER R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                set_value_string(op1R, op2R);
                null_terminate_string_buffer(op1R); /* the logic requires a null terminator */
#ifdef NUTF8
                char *c;
                for (c = op1R->string_value ; *c; ++c) *c = (char)toupper(*c);
#else
                REQUIRE_VALID_UTF8_REGISTER(op1R);
                utf8upr(op1R->string_value);
#endif
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  STRCHAR_REG_REG_REG  String to Int op1 = op2[op3]                   pej 12 Apr 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(STRCHAR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - STRCHAR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
#ifndef NUTF8
                int result;
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                string_set_byte_pos(op2R, op3R->int_value);
                utf8codepoint(op2R->string_value + op2R->string_pos, &result);
                REG_RETURN_INT(result)
#else
                REG_RETURN_INT(op2R->string_value[op3R->int_value])
#endif
            }
            DISPATCH
// Lazy Peter approach
#define setCodePointETC()       \
   {string_set_byte_pos(op2R, op3R->int_value); \
    start = op2R->string_value + op2R->string_pos; \
    end = utf8codepoint(start, &ch); \
    bytelen = end - start;}
#define split2hex(chr,into)           \
    {op1R->string_value[into] = hexconst[(chr>> 4) & 0xF];   \
     op1R->string_value[into+1] = hexconst[chr & 0xF]  ; \
     op1R->string_value[into+2] = '\0';  }

/* ------------------------------------------------------------------------------------
 *  HEXCHAR_REG_REG_REG  op1 = hex(op2[op3])                       pej 04 November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(HEXCHAR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - HEXCHAR R%d,R%d,R%d\n", (int) REG_IDX(1), (int) REG_IDX(2), (int) REG_IDX(3));
            {
                static const char hexconst[] = "0123456789ABCDEF";
                int ch, i, bytelen, mode;
                unsigned char bytebuf[4] = {0, 0, 0, 0};

                null_terminate_string_buffer(op1R);
                if (strstr(op1R->string_value, "UTFV1") > 0) goto hexm2;
                if (strstr(op1R->string_value, "UTFV2") > 0) goto hexm3;

                char *start, *end;
#ifndef NUTF8
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                setCodePointETC()   //calculate Codepoint/length, etc. in a macro, character is in ch
#else
                ch=op2R->string_value[op3R->int_value];
#endif
                prep_string_buffer(op1R, 3);
                split2hex(ch, 0)                    // split the character into their 2 byte hex presentation
                PUTSTRLEN(op1R, 2)                  // hex length is 2
                goto hexdispatch;
   /* ----- create full UTF8 code -------------- */
                hexm2:
#ifndef NUTF8
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                setCodePointETC()   //calculate Codepoint/length, etc. in a macro, character is in ch
                for (i = 0; i < bytelen && i < 4; ++i) {
                    bytebuf[i] = (unsigned char) start[i];
                }
#else
                ch=op2R->string_value[op3R->int_value];
#endif
                prep_string_buffer(op1R, 9);
                for (i = 0; i < 4; ++i) {
                    unsigned char b = bytebuf[i];
                    split2hex(b, i*2)                     // split the character into their 2 byte hex presentation
                }
                PUTSTRLEN(op1R, 8);
                goto hexdispatch;
    /* ----- add UTF8 code depending on length -------------- */
            hexm3:
#ifndef NUTF8
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                setCodePointETC()   //calculate Codepoint/length, etc. in a macro
#else
                ch=op2R->string_value[op3R->int_value];
#endif
                prep_string_buffer(op1R, bytelen * 2 + 1);
                for (i = 0; i < bytelen; ++i) {
                    unsigned char b = start[i];
                    split2hex(b, i * 2)                     // split the character into their 2 byte hex presentation
                }
                PUTSTRLEN(op1R, bytelen * 2);
            }
        hexdispatch:
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  POSCHAR_REG_REG_REG  op1 position of op3 in op2                pej 05 November 2021
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(POSCHAR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - POSCHAR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                rxinteger result = -1, i;
                int ch;

                REQUIRE_VALID_UTF8_REGISTER(op2R);
                for (i = 0; i < op2R->string_length; i++) {
#ifndef NUTF8
                    string_set_byte_pos(op2R, i);
                    utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
#else
                    ch=op2R->string_value[i];
#endif
                    if (ch == op3R->int_value) {
                        result = i;
                        break;
                    }
                }
                REG_RETURN_INT(result)
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  BGT_ID_REG_REG  if op2>op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BGT_ID_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - BGT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (current_frame->locals[REG_IDX(2)]->int_value > current_frame->locals[REG_IDX(3)]->int_value) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  BGT_ID_REG_INT  if op2>op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BGT_ID_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - BGT 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (current_frame->locals[REG_IDX(2)]->int_value > op3I) {
               next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
               CALC_DISPATCH_MANUAL
            }
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  BGE_ID_REG_REG  if op2>=op3 goto op1                          pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BGE_ID_REG_REG) CALC_DISPATCH(3)
       DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
       if (current_frame->locals[REG_IDX(2)]->int_value >= current_frame->locals[REG_IDX(3)]->int_value) {
          next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
          CALC_DISPATCH_MANUAL
       }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  BGE_ID_REG_INT  if op2>=op3 goto op1                         pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BGE_ID_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value >= op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BLT_ID_REG_REG  if op2<op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLT_ID_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - BLT 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value < current_frame->locals[REG_IDX(3)]->int_value) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BLT_ID_REG_INT  if op2<op3 goto op1                           pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLT_ID_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - BGT 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value < op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BLE_ID_REG_REG  if op2<=op3 goto op1                          pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLE_ID_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value <= current_frame->locals[REG_IDX(3)]->int_value) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BLE_ID_REG_INT  if op2<=op3 goto op1                          pej 13 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLE_ID_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value <= op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BNE_ID_REG_INT  if op2!=op3 goto op1                          pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BNE_ID_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value != current_frame->locals[REG_IDX(3)]->int_value) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  BNE_ID_REG_INT  if op2!=op3 goto op1                          pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BNE_ID_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value != op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BEQ_ID_REG_INT  if op2=op3 goto op1                           pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BEQ_ID_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
        if (current_frame->locals[REG_IDX(2)]->int_value == current_frame->locals[REG_IDX(3)]->int_value) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
/* ------------------------------------------------------------------------------------
 *  BEQ_ID_REG_INT  if op2=op3 goto op1                           pej 14 September 2021
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BEQ_ID_REG_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - BGE 0x%x,R%d,%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
        if (current_frame->locals[REG_IDX(2)]->int_value == op3I) {
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        }
    DISPATCH
 /* ------------------------------------------------------------------------------------
 *  BCT_REG_ID  dec op2; if op2>0 goto op1                           pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCT_ID_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - BCT R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            (current_frame->locals[REG_IDX(2)]->int_value)--;
            if (current_frame->locals[REG_IDX(2)]->int_value > 0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  BCT_REG_REG_ID  dec op2, inc op3; if op2>0 goto op1              pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCT_ID_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - BCT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            (current_frame->locals[REG_IDX(2)]->int_value)--;
            (current_frame->locals[REG_IDX(3)]->int_value)++;
            if (current_frame->locals[REG_IDX(2)]->int_value>0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  BCF_ID_REG if op2=0 goto op1(if false) else dec op2
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCF_ID_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - BCF R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            if (current_frame->locals[REG_IDX(2)]->int_value == 0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
            else (current_frame->locals[REG_IDX(2)]->int_value)--;
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  BCF_ID_REG_REG  if op2=0 goto op1(if false) else dec op2 and inc op3
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCF_ID_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - BCF R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            if (current_frame->locals[REG_IDX(2)]->int_value == 0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
            else {
                (current_frame->locals[REG_IDX(2)]->int_value)--;
                (current_frame->locals[REG_IDX(3)]->int_value)++;
            }
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  BCTNM_REG_ID  dec op2; if op2>=0 goto op1                           pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCTNM_ID_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - BCTNM R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            (current_frame->locals[REG_IDX(2)]->int_value)--;
            if (current_frame->locals[REG_IDX(2)]->int_value>=0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  BCTNM_REG_REG_ID  dec op2, inc op3; if op2>=0 goto op1           pej 26 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCTNM_ID_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - BCTNM R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            (current_frame->locals[REG_IDX(2)]->int_value)--;
            (current_frame->locals[REG_IDX(3)]->int_value)++;
            if (current_frame->locals[REG_IDX(2)]->int_value>=0) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  BCTP_ID_REG  inc op2; goto op1                                     pej 11 June 2023
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(BCTP_ID_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - BCTP R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            (current_frame->locals[REG_IDX(2)]->int_value)++;
            next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
            CALC_DISPATCH_MANUAL
        DISPATCH
 /* ------------------------------------------------------------------------------------
 *  FndBlnk REG_REG_REG  return first blank after op2[op3]          pej 27 August 2021
 *  Blanks are all unicode white spaces
 *  returned is the offset which is 0 based
 *  a negative return value means nothing found: to distinguish from offset 0
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(FNDBLNK_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - FNDBLNK R%lu R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                rxinteger len;
                rxinteger result;
                int ch;
                REQUIRE_VALID_UTF8_REGISTER(op2R);
#ifndef NUTF8
                len = (rxinteger) op2R->string_chars;
#if ASCII_FAST_PATH
                if (len==op2R->string_length) {  /* it is plain ASCII */
                    if (op3R->int_value>=0) {
                        result = ascii_fwd_blank((const unsigned char *)op2R->string_value, op3R->int_value, len);
                        goto blankfound;
                    } else {
                        result = -op3R->int_value;   /* Convert to positive index */
                        if (result >= len) result = len - 1;  /* Clamp to valid range */
                        result = ascii_back_blank((unsigned char *)op2R->string_value, result, len);
                        goto blankfound;
                    }
                }
#endif

#else
                len = (rxinteger) op2R->string_length;
#endif
                for (result = op3R->int_value; result < len; result++) {
#ifndef NUTF8
                    string_set_byte_pos(op2R, result);
                    utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
#else
                    ch = op2R->string_value[result];
#endif
                    if (IS_UNICODE_WHITESPACE(ch)) goto blankfound;
                }
                result = -len;
            blankfound:
             // printf("FBlank %d %d\n",fresult,result);
                REG_RETURN_INT(result)
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  FndNBlnk REG_REG_REG  return first non blank after op2[op3]          pej 27 August 2021
 *  Blanks are all unicode white spaces
 *  returned is the offset which is 0 based
 *  a negative return value means nothing found: to distinguish from offset 0
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(FNDNBLNK_REG_REG_REG)  CALC_DISPATCH(3)
            DEBUG("TRACE - FNDNBLNK R%lu R%lu\n", REG_IDX(1), REG_IDX(2));
            {
                rxinteger result;
                rxinteger len;
                int ch;
                REQUIRE_VALID_UTF8_REGISTER(op2R);

#ifndef NUTF8
             len = (rxinteger)op2R->string_chars;
#if ASCII_FAST_PATH
             if (len==op2R->string_length) {  /* it is plain ASCII */
                   if (op3R->int_value>=0) {
                        result = ascii_fwd_nonblank((const unsigned char *)op2R->string_value, op3R->int_value, len);
                        goto nonblankfound;
                    } else {
                        result = -op3R->int_value;   /* Convert to positive index */
                        if (result >= len) result = len - 1;  /* Clamp to valid range */
                        result = ascii_back_nonblank((unsigned char *)op2R->string_value, result, len);
                        goto nonblankfound;
                    }
             }
#endif
#else
                len = (rxinteger)op2R->string_length;
#endif
                result = op3R->int_value;

                if (result >= 0) {  // FORWARD SEARCH
                    for (; result < len; result++) {
#ifndef NUTF8
                        string_set_byte_pos(op2R, result);
                        utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
#else
                        ch = op2R->string_value[result];
#endif
                        if (!IS_UNICODE_WHITESPACE(ch)) goto nonblankfound;
                    }
                    result = -len;  // Not found in forward scan
                } else { // REVERSE SEARCH
                    result = -result;  // Convert to positive index
                    if (result >= len) result = len - 1;  // Clamp to valid range
                    for (; result >= 0; result--) {
#ifndef NUTF8
                        string_set_byte_pos(op2R, result);
                        utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
#else
                        ch = op2R->string_value[result];
#endif
                        if (!IS_UNICODE_WHITESPACE(ch)) goto nonblankfound;
                    }
                    result = -1;  // Not found in reverse scan
                }
                nonblankfound:
         //       printf("FnBLANK %d %d\n",fresult,result);
                REG_RETURN_INT(result)
            }
            DISPATCH

 /* ------------------------------------------------------------------------------------
  *  GETBYTE_REG_REG_REG  Int op1 = op2[op3]                             pej 19 Oct 2021
  *  -----------------------------------------------------------------------------------
  */
    START_INSTRUCTION(GETBYTE_REG_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - GETBYTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));

    if (op3R->int_value < 0 || (size_t)op3R->int_value >= op2R->binary_length) {
        REG_RETURN_INT(-1)
    }
    else {
        REG_RETURN_INT((unsigned char)op2R->binary_value[op3R->int_value])
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  BLEN_REG_REG  Int op1 = byte length of op2
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BLEN_REG_REG) CALC_DISPATCH(2)
    DEBUG("TRACE - BLEN R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
    REG_RETURN_INT((rxinteger)op2R->binary_length)
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  SETBYTE_REG_REG_REG  op1[op2] = op3
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SETBYTE_REG_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - SETBYTE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (op2R->int_value < 0 || (size_t)op2R->int_value >= op1R->binary_length ||
        op3R->int_value < 0 || op3R->int_value > 255) {
        SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
    }
    else {
        op1R->binary_value[(size_t)op2R->int_value] = (char)(unsigned char)op3R->int_value;
        clear_vm_private_flags(op1R);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  BCONCAT_REG_REG_REG  op1 = op2 || op3
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BCONCAT_REG_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - BCONCAT R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (concat_binary(op1R, op2R, op3R) != 0) {
        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  BAPPEND_REG_REG  op1 = op1 || op2
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BAPPEND_REG_REG) CALC_DISPATCH(2)
    DEBUG("TRACE - BAPPEND R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
    if (append_binary_value(op1R, op2R) != 0) {
        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  SETBINPOS_REG_REG  op1 binary cursor = clamp(op2, 0..len)
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(SETBINPOS_REG_REG) CALC_DISPATCH(2)
    DEBUG("TRACE - SETBINPOS R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
    if (op2R->int_value < 0) {
        op1R->binary_pos = 0;
    }
    else if ((size_t)op2R->int_value > op1R->binary_length) {
        op1R->binary_pos = op1R->binary_length;
    }
    else {
        op1R->binary_pos = (size_t)op2R->int_value;
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  GETBINPOS_REG_REG  Int op1 = op2 binary cursor
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(GETBINPOS_REG_REG) CALC_DISPATCH(2)
    DEBUG("TRACE - GETBINPOS R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
    REG_RETURN_INT((rxinteger)op2R->binary_pos)
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  BSLICE_REG_REG_REG  op1 = op2[op2.binary_pos..op2.binary_pos + op3)
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BSLICE_REG_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - BSLICE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (op3R->int_value < 0) {
        SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
    }
    else if (slice_binary(op1R, op2R, op2R->binary_pos, (size_t)op3R->int_value) != 0) {
        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  BUPDATE_REG_REG_REG  overlay op3 into op1 at byte offset op2
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BUPDATE_REG_REG_REG) CALC_DISPATCH(3)
    DEBUG("TRACE - BUPDATE R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
    if (op2R->int_value < 0 || (size_t)op2R->int_value > op1R->binary_length) {
        SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
    }
    else {
        size_t offset = (size_t)op2R->int_value;
        if (op3R->binary_length > op1R->binary_length - offset) {
            SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE);
        }
        else {
            if (op3R->binary_length) {
                memmove(op1R->binary_value + offset, op3R->binary_value, op3R->binary_length);
            }
            clear_vm_private_flags(op1R);
        }
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  STOBIN_REG  op1.binary = op1.string bytes
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(STOBIN_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - STOBIN R%d\n", (int)REG_IDX(1));
    if (set_binary(op1R, op1R->string_value, op1R->string_length) != 0) {
        SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  BINTOS_REG  op1.string = op1.binary bytes, validated as UTF-8
 *  -----------------------------------------------------------------------------------
 */
    START_INSTRUCTION(BINTOS_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - BINTOS R%d\n", (int)REG_IDX(1));
#ifndef NUTF8
    {
        size_t chars = 0;
        if (utf8nvalid_count(op1R->binary_value, op1R->binary_length, &chars)) {
            SET_SIGNAL_MSG(RXSIGNAL_UNICODE_ERROR, "Invalid UTF-8 in binary-to-string conversion");
        } else {
            set_string(op1R,
                       op1R->binary_value ? op1R->binary_value : "",
                       op1R->binary_length);
            op1R->string_chars = chars;
            mark_utf8_valid_count(op1R);
        }
    }
#else
    set_string(op1R,
               op1R->binary_value ? op1R->binary_value : "",
               op1R->binary_length);
#endif
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  CONCCHAR_REG_REG_REG  op1=op2[op3]                                pej 27 August 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(CONCCHAR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - CONCCHAR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger temp = op3R->int_value;   // save offset, we misuse v3 later
#ifndef NUTF8
                int ch;
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                string_set_byte_pos(op2R, op3R->int_value);
                utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
                op3R->int_value = ch;
#else
                op3R->int_value=op2R->string_value[op3R->int_value - 1];
#endif
                string_concat_char(op1R, op3R);
                op3R->int_value = temp;   // restore original v3
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  TRANSCHAR_REG_REG_REG  replace op1 if it is in op3-list by char in op2-list pej 7 November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(TRANSCHAR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - TRANSCHAR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger val = op1R->int_value;
                rxinteger len, i;
                int ch;

                REQUIRE_VALID_UTF8_REGISTER(op2R);
                REQUIRE_VALID_UTF8_REGISTER(op3R);
                GETSTRLEN(len, op3R)

                for (i = 0; i < len; i++) {
                    GETSTRCHAR(ch, op3R, i)
                    if (val == ch) {
                        GETSTRCHAR(ch, op2R, i)
                        val = ch;
                        break;
                    }
                }
                REG_RETURN_INT(val)
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  DROPCHAR_REG_REG_REG  removes characters contained in op3-list pej 19 November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(DROPCHAR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - DROPCHAR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                rxinteger i, len1, len2;
                int found;
                int ch;
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                REQUIRE_VALID_UTF8_REGISTER(op3R);
                GETSTRLEN(len1, op2R)
                GETSTRLEN(len2, op3R)
                if (len2 == 0) len2 = (rxinteger) op3R->string_length;
                for (i = 0; i < len1; i++) {
                    rxinteger j;
                    GETSTRCHAR(ch, op2R, i)
                    op2R->int_value = ch;
                    found = 0;
                    for (j = 0; j < len2; j++) {
                        GETSTRCHAR(ch, op3R, j)
                        op3R->int_value = ch;
                        if (op2R->int_value == op3R->int_value) {
                            found = 1;  // found drop char
                            break;
                        }
                    }
                    if (found == 1) continue;
                    string_concat_char(op1R, op2R);
                }
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  SUBSTR_REG_REG_REG op1 = substr(op2, length)
 *  Extracts 'length' codepoints from op2, starting at the cursor previously set by
 *  SETSTRPOS. The source cursor is stored as byte + codepoint positions on the value.
 * ----------------------------------------------------------------------------------- */

        START_INSTRUCTION(SUBSTR_REG_REG_REG)
        START_INSTRUCTION(SUBSTRING_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SUBSTR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            {
                rxinteger length = op3R->int_value;
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                if (length <= 0) {
                    PUTSTRLEN(op1R, 0);
                } else {
                    string_slice_from_cursor(op1R, op2R, (size_t) length);
                }
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
*  SUBSTCUT_REG_REG_REG op1=substr(op1,,op2) cuts off after op2   pej 13 November 2021
*  -----------------------------------------------------------------------------------
*/
        START_INSTRUCTION(SUBSTCUT_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SUBSTCUT R%lu R%lu\n", REG_IDX(1), REG_IDX(2));

        /* input parm op2 defines how many leading codepoints remain in the string */
           REQUIRE_VALID_UTF8_REGISTER(op1R);
           string_truncate_chars(op1R, (size_t) op2R->int_value);
           DISPATCH

/* ------------------------------------------------------------------------------------
 *  PADSTR_REG_REG_REG op1=op2(repeated op3 times)                 pej 13 November 2021
 *  requires in op2R the codepoint of the character
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(PADSTR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - PADSTR R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                int i;
                for (i = 0; i < op3R->int_value; i++) {
                    string_concat_char(op1R, op2R);
                }
             }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  CNOP Dummy instruction for testing purposes                     pej 11 November 2021
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(CNOP) CALC_DISPATCH(0)
        DEBUG("TRACE - CNOP\n");
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  find substring in string                                           pej 27 June 2025
 *  the initial offset is in R1: it is 1-based
 *  returned is the offset 1-based both for better performance in calling REXX scripts
 *  In UTF builds, both the input and output positions are codepoint-based.
 *  0 means nothing found
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(STRPOS_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - STRPOS R%lu R%lu R%lu\n", REG_IDX(1), REG_IDX(2),REG_IDX(3));
            {
                char *charpos;
                rxinteger start_pos = op1RI;

                REQUIRE_VALID_UTF8_REGISTER(op2R);
                REQUIRE_VALID_UTF8_REGISTER(op3R);
                if (start_pos <= 0) {
                    REG_RETURN_INT(0);
                } else {
                    null_terminate_string_buffer(op2R);
                    null_terminate_string_buffer(op3R);
#ifndef NUTF8
                    {
                        size_t start_offset;
                        size_t saved_string_pos = op3R->string_pos;
                        size_t saved_string_char_pos = op3R->string_char_pos;
                        char *search_start;

                        start_offset = (size_t)(start_pos - 1);
                        if (start_offset >= op3R->string_chars) {
                            REG_RETURN_INT(0);
                        } else {
                            string_set_byte_pos(op3R, start_offset);
                            search_start = op3R->string_value + op3R->string_pos;
                            charpos = (char *)utf8str(search_start, op2R->string_value);
                            op3R->string_pos = saved_string_pos;
                            op3R->string_char_pos = saved_string_char_pos;

                            if (charpos) {
                                size_t byte_offset = (size_t)(charpos - op3R->string_value);
                                REG_RETURN_INT((rxinteger)(utf8nlen(op3R->string_value, byte_offset) + 1));
                            } else {
                                REG_RETURN_INT(0);
                            }
                        }
                    }
#else
                    {
                        rxinteger offset = start_pos - 1;    // make position an offset
                        if (offset >= op3R->string_length) {
                            REG_RETURN_INT(0);
                        } else {
                            charpos = strstr(op3R->string_value + offset, op2R->string_value);
                            if (charpos) offset = (rxinteger)(charpos - op3R->string_value);
                            else offset = -1;
                            REG_RETURN_INT(offset + 1);      // make offset a position
                        }
                    }
#endif
                }
            }
            DISPATCH

/*
 *   APPENDCHAR_REG_REG Append Concat Char op2 (as int) on op1
 */
        START_INSTRUCTION(APPENDCHAR_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - APPENDCHAR R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            string_concat_char(op1R, op2R);
            DISPATCH

/*
 *   APPEND_REG_REG Append string op2 on op1
 */
        START_INSTRUCTION(APPEND_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - APPEND R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            string_append(op1R, op2R);
            DISPATCH

/*
 *   SAPPEND_REG_REG Append with space string op2 on op1
 */
        START_INSTRUCTION(SAPPEND_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SAPPEND R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            string_sappend(op1R, op2R);
            DISPATCH

/*
 *   STRLEN_REG_REG String Length op1 = length(op2)
 */
        START_INSTRUCTION(STRLEN_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - STRLEN R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            REQUIRE_VALID_UTF8_REGISTER(op2R);
#ifndef NUTF8
            op1R->int_value = (rxinteger)op2R->string_chars;
#else
            op1R->int_value = (rxinteger)op2R->string_length;
#endif
            DISPATCH

/*
 * SETSTRPOS_REG_REG - Set String (op1) charpos set to op2
 */
        START_INSTRUCTION(SETSTRPOS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SETSTRPOS R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            REQUIRE_VALID_UTF8_REGISTER(op1R);
#ifndef NUTF8
            string_set_byte_pos(op1R, op2R->int_value);
#else
            op1R->string_pos = op2R->int_value;
#endif
            DISPATCH

/*
 * GETSTRPOS_REG_REG - Get String (op2) charpos into op1
 */
        START_INSTRUCTION(GETSTRPOS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - GETSTRPOS R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
#ifndef NUTF8
            op1R->int_value = (int) op2R->string_char_pos;
#else
            op1R->int_value = op2R->string_pos;
#endif
            DISPATCH

/*
 * STRCHAR_REG_REG - op1 (as int) = op2[charpos]
 */
        START_INSTRUCTION(STRCHAR_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - STRCHAR R%lu R%lu\n", REG_IDX(1),
                  REG_IDX(2));
            {
                int ch;
#ifndef NUTF8
                REQUIRE_VALID_UTF8_REGISTER(op2R);
                utf8codepoint(op2R->string_value + op2R->string_pos, &ch);
                op1R->int_value = ch;
#else
                op1R->int_value = op2R->string_value[op2R->string_pos];
#endif
            }
            DISPATCH

/*
 * GETTP_REG_REG gets readable register status flags (op1 = op2.flags)
 */
        START_INSTRUCTION(GETTP_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - GETTP R%d R%d\n", (int)REG_IDX(1), (int)REG_IDX(2));
            op1R->int_value = (rxinteger)(op2R->status.all_type_flags & RXFLAG_READABLE_MASK);
            DISPATCH

/*
 * SETTP_REG_INT sets externally writable register status flags
 */
        START_INSTRUCTION(SETTP_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - SETTP R%d %d\n", (int)REG_IDX(1), (int)op2I);
            op1R->status.all_type_flags = RXFLAGS_PUBLIC_WRITE(op1R->status.all_type_flags, (uint32_t)op2I);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  LOADSETTP_REG_INT load register and set externally writable status flags
 *   op1=op2 and (op1.flags = op3)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(LOADSETTP_REG_INT_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - LOADSETTP R%d %d %d\n", (int)REG_IDX(1),(int)op2I,(int)op3I);

            op1R->int_value = op2I;
            op1R->status.all_type_flags = RXFLAGS_PUBLIC_REPLACE((uint32_t)op3I);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  LOADSETTP_REG_string load string to register and set externally writable status flags
 *   op1=op2 and (op1.flags = op3)
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(LOADSETTP_REG_STRING_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - LOADSETTP R%d %s %d\n", (int)REG_IDX(1),op2S->string,(int) op3I);

            set_const_string(op1R, op2S);
            op1R->status.all_type_flags = RXFLAGS_PUBLIC_WRITE(op1R->status.all_type_flags, (uint32_t)op3I);
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  LOADSETTP_REG_FLOAT float to load register and set externally writable status flags
 *   op1=op2 and (op1.flags = op3)
 *  -----------------------------------------------------------------------------------
 */
            START_INSTRUCTION(LOADSETTP_REG_FLOAT_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - LOADSETTP R%d %.15g %d\n", (int)REG_IDX(1), op2F,(int) op3I);
            op1R->float_value = op2F;
            op1R->status.all_type_flags = RXFLAGS_PUBLIC_REPLACE((uint32_t)op3I);
            DISPATCH

/*
 * SETORTP_REG_INT or externally writable register status flags
 */
        START_INSTRUCTION(SETORTP_REG_INT) CALC_DISPATCH(2)
            DEBUG("TRACE - SETORTP R%d %d\n", (int)REG_IDX(1), (int)op2I);
            op1R->status.all_type_flags = RXFLAGS_PUBLIC_OR(op1R->status.all_type_flags, (uint32_t)op2I);
            DISPATCH

/*
 * GETANDTP_REG_REG_INT get readable register status flags with mask
 */
        START_INSTRUCTION(GETANDTP_REG_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - GETANDTP R%d R%d %d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)op3I);
            op1R->int_value = (rxinteger)((op2R->status.all_type_flags & RXFLAG_READABLE_MASK) & (uint32_t)op3I);
            DISPATCH

/*
 * BRTPT_ID_REG if op2 has public status flags then goto op1
 */
        START_INSTRUCTION(BRTPT_ID_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - BRTPT_ID_REG 0x%x R%d\n", (unsigned int)REG_IDX(1), (int)REG_IDX(2));
            if (op2R->status.all_type_flags & RXFLAG_PUBLIC_TEST_MASK) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
            DISPATCH

/*
 * BRTPANDT_ID_REG_INT if op2 readable status flags & op3 true then goto op1
*/
        START_INSTRUCTION(BRTPANDT_ID_REG_INT) CALC_DISPATCH(3)
            DEBUG("TRACE - BRTPANDT_ID_REG_INT 0x%x R%d %d\n",
                  (unsigned int)REG_IDX(1),
                  (int)REG_IDX(2),(int)op3I);
            if ((op2R->status.all_type_flags & RXFLAG_READABLE_MASK) & (uint32_t)op3I) {
                next_pc = current_frame->procedure->binarySpace->binary + REG_IDX(1);
                CALC_DISPATCH_MANUAL
            }
            DISPATCH
/* ------------------------------------------------------------------------------------
 *  IRAND_REG_REG Random Number with seed register                 pej 27 February 2022
 *   op1=irand(op2)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IRAND_REG_REG) CALC_DISPATCH(2)
             DEBUG("TRACE - IRAND R%d R%d \n", (int)REG_IDX(1), (int)REG_IDX(2));

            if (op2R->int_value<0) {                 // no seed set
                if (!hasSeed)  {       // seed still initial, set time based seed
                   initSeed = (time((time_t *) 0) % (3600 * 24)); // initial seed still active
                   srand((unsigned) initSeed);
                   hasSeed = 1;
                }
            } else {                                 // seed set re-init with new seed
                initSeed=op2R->int_value;
                srand((unsigned) initSeed);
                hasSeed = 1;
            }
            set_int(op1R, rand());   // receive new random value
        DISPATCH
/* ------------------------------------------------------------------------------------
 *  IRAND_REG_REG Random Number with seed register                 pej 27 February 2022
 *   op1=irand(op2)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(IRAND_REG_INT) CALC_DISPATCH(2)
             DEBUG("TRACE - IRAND R%d R%d \n", (int)REG_IDX(1), (int)op2I);
             if (op2I<0) {                                // no seed set
                if (!hasSeed)  {            // seed still initial, set time based seed
                    initSeed = (time((time_t *) 0) % (3600 * 24)); // initial seed still active
                    srand(initSeed);
                    hasSeed = 1;
                }
             } else {                                     // seed set and NE old seed, set it new
                initSeed=op2I;
                srand( initSeed);
                hasSeed = 1;
            }
            set_int(op1R, rand());   // receive new random value
        DISPATCH

/* ------------------------------------------------------------------------------------
 *  rxvers  returns os information                                    pej 20. June 2023
 *  -----------------------------------------------------------------------------------
 */

// MACRO

#define FDATE (char const[]){ __DATE__[7], __DATE__[8], ..., ' ', ... , '\0' }

    START_INSTRUCTION(RXVERS_REG) CALC_DISPATCH(1)
    DEBUG("TRACE - RXVERS R%d\n", (int) REG_IDX(1));
    {
        char vers[256];
        const char *platform;
        const char *bits;

#if defined(__linux__)
        platform = "linux";
#elif defined(_WIN32)
        platform = "windows";
#elif defined(__APPLE__)
        platform = "macOS";
#elif defined(__CMS__)
        platform = "cms";
#else
        platform = "unknown";
#endif

#ifdef __32BIT__
        bits = "32";
#else
        bits = "64";
#endif

        snprintf(vers, sizeof(vers), "%s %s %s %s", platform, bits, rxversion, compile_date);

        set_null_string(op1R, vers);
    }
    DISPATCH

/* ------------------------------------------------------------------------------------
 *  rxhash  returns hash of a string it runs in reverse order         pej 24. June 2023
 *  op1=hash(op2,op3)
 *      op2=string
 *      op3=length(string)
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(RXHASH_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - RXHASH R%d R%d R%d \n", (int)REG_IDX(1),(int)REG_IDX(1),(int)REG_IDX(3));

    {
#ifdef __32BIT__
        uint32_t hash=0;
#else
        uint64_t hash=0;
#endif
        int i1,len;
        REQUIRE_VALID_UTF8_REGISTER(op2R);
        GETSTRLEN(len, op2R);
        if(len<=0) len=op2R->string_length;
        else if(op2R->string_length<len) len=op2R->string_length;
        for (i1 = len - 1; i1 >= 0; i1--) {
            hash = (unsigned char)op2R->string_value[i1] + (hash << 6) + (hash << 16) - hash;
        }
        hash ^= (hash >> 16);
#ifdef __32BIT__
        hash = hash & 0x7FFFFFFF
#else
        hash = hash & 0x7FFFFFFFFFFFFFFF;
#endif
        op1R->int_value = hash;
     }
     DISPATCH
    /* Spawn - Spawn a process with io redirects - Spawn Process op1 = exec op2 redirect op3
     * reg 1 will be the return code of the process
     * reg 2 is the command (the path environment variable is used for search resolution)
     * reg 3 is an array of 3 opaque REDIRECT binary structures (e.g. generated by redrtoarr)
     *       reg3[1] = input, reg3[2] = output, reg3[3] = error
     * spawn generates a failure signal if the command is not found */
    START_INSTRUCTION(SPAWN_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SPAWN R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            {
                int command_rc = 0;
                int spawn_rc = 0;
                REDIRECT *pIn = 0;
                REDIRECT *pOut = 0;
                REDIRECT *pErr = 0;
                char *command;
                command = malloc(op2R->string_length + 1);
                memcpy(command,op2R->string_value, op2R->string_length);
                command[op2R->string_length] = 0;
                char* errorText = 0;

                if (op3R->num_attributes > 0) pIn = (REDIRECT*)(op3R->attributes[0])->binary_value;
                if (op3R->num_attributes > 1) pOut = (REDIRECT*)(op3R->attributes[1])->binary_value;
                if (op3R->num_attributes > 2) pErr = (REDIRECT*)(op3R->attributes[2])->binary_value;

                /* op3R->attributes[2] is the environment variables */
                spawn_rc = shellspawn(command, pIn, pOut, pErr, op3R->attributes[3], &command_rc, &errorText);
                if (spawn_rc == SHELLSPAWN_NOFOUND) {
                    SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Command Not Found");
                    free(command);
                    if (errorText) free(errorText);
                    DISPATCH
                }
                if (spawn_rc) {
                    SET_SIGNAL_MSG(RXSIGNAL_FAILURE, errorText);
                    free(command);
                    if (errorText) free(errorText);
                    DISPATCH
                }
                if (errorText) free(errorText);
                free(command);
                op1R->int_value = command_rc;
            }
            DISPATCH

    /* redir2str - Redirect op1 = to-string op2
     * reg 1 will be the redirect object
     * reg 2 is string that will have the redirected string appended to
     *       the redirect object MUST then be used in shellspawn() to cleanup/free memory
     */
    START_INSTRUCTION(REDIR2STR_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - REDIR2STR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        redr2str(op1R, op2R);
        DISPATCH

    /* redir2arr - Redirect op1 = to-array op2
     * reg 1 will be the redirect object
     * reg 2 is array that will have the redirected output appended to
     *       the redirect object MUST then be used in shellspawn() to cleanup/free memory
     */
    START_INSTRUCTION(REDIR2ARR_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - REDIR2ARR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        redr2arr(op1R, op2R);
        DISPATCH

    /* str2redir - Redirect op1 <- string op2
     * reg 1 will be the redirect object
     * reg 2 is string that will be redirected to the pipe
     *       the redirect object MUST then be used in shellspawn() to cleanup/free memory
     */
    START_INSTRUCTION(STR2REDIR_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - STR2REDIR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        str2redr(op1R, op2R);
        DISPATCH

    /* arr2redir - Redirect op1 <- array op2
     * reg 1 will be the redirect object
     * reg 2 is array that will be redirected to the pipe
     *       the redirect object MUST then be used in shellspawn() to cleanup/free memory
     */
    START_INSTRUCTION(ARR2REDIR_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - ARR2REDIR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        arr2redr(op1R, op2R);
        DISPATCH

    /* nullredir - Redirect op1 = to/from null
     * reg 1 will be the redirect object
    */
    START_INSTRUCTION(NULLREDIR_REG) CALC_DISPATCH(1)
        DEBUG("TRACE - NULLREDIR R%lu\n", REG_IDX(1));
        nullredr(op1R);
        DISPATCH

    /* File IO functions - mapped to C90 functions */

    /* fopen - op1 file*(int) = fopen filename op2(string) mode op3(string) */
    START_INSTRUCTION(FOPEN_REG_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - FOPEN R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
        {
            int fd;
            char* filename = reg2nullstring(op2R);
            char* mode = reg2nullstring(op3R);

            // If the filename is "stdout", "stderr", or "stdin" we use the C runtime's handle
            if (strcmp(filename, "stdout") == 0) {
                op1R->int_value = (rxinteger)stdout;
            } else if (strcmp(filename, "stderr") == 0) {
                op1R->int_value = (rxinteger)stderr;
            } else if (strcmp(filename, "stdin") == 0) {
                op1R->int_value = (rxinteger)stdin;
            } else {
                /* Otherwise we open the file normally */
                op1R->int_value = (rxinteger)fopen(filename, mode);

                /* If the open succeeds, add the FD_CLOEXEC so that the file is not available to an ADDRESSed command! */
                if (op1R->int_value) {
#ifdef _WIN32
                    // On Windows, get the underlying OS HANDLE from the C-runtime file descriptor.
                    intptr_t handle = _get_osfhandle(_fileno((FILE*)op1R->int_value));
                    if (handle != -1) {
                        // Set the handle to be non-inheritable. This is the equivalent
                        // of POSIX's FD_CLOEXEC flag.
                        SetHandleInformation((HANDLE)handle, HANDLE_FLAG_INHERIT, 0);
                    }
#else
                    fd = fileno((FILE*)op1R->int_value);
                    fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
                }
            }
            free(filename);
            free(mode);
        }
        DISPATCH

    /* fclose - op1 rc(int) = fclose op2 file*(int) */
    START_INSTRUCTION(FCLOSE_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FCLOSE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = fclose((FILE*)op2R->int_value);
        DISPATCH

    /* fflush - op1 rc(int) = fflush op2 file*(int) */
    START_INSTRUCTION(FFLUSH_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FFLUSH R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = fflush((FILE*)op1R->int_value);
        DISPATCH

    /* freadb - op1(binary) = fread op2 file*(int) op3 bytes(int) */
    START_INSTRUCTION(FREADB_REG_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - FREADB R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
        {
            if (op3R->int_value < 0) {
                SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Invalid binary read size");
            }
            else if (op3R->int_value == 0) {
                if (prep_binary_buffer(op1R, 0) != 0) SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
            }
            else if (reserve_binary_buffer(op1R, (size_t)op3R->int_value) != 0) {
                SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Out of memory");
            }
            else {
                op1R->binary_length = fread(op1R->binary_value, 1, (size_t)op3R->int_value, (FILE *) op2R->int_value);
                op1R->binary_pos = 0;
                clear_vm_private_flags(op1R);
            }
        }
        DISPATCH

    /* freadline - op1 (string) = read until newline op2 file*(int) */
    START_INSTRUCTION(FREADLINE_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FREADLINE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
    {
        int ch;

        op1R->string_length = 0;
        op1R->string_pos = 0;

        if (op2R->int_value == 0) {
            // no file - raise error
            SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "File not open");
        }

        else {
            /* Read until EOF or newline */
            while ((ch = fgetc((FILE*)op2R->int_value)) != EOF) {

                /* End of line detection (LF) */
                if (ch == '\n') break;

                /* End of line detection (CR LF, or CR (old macs) */
                if (ch == '\r') {
                    ch = fgetc((FILE*)op2R->int_value);\
                    if (ch != '\n') ungetc(ch, (FILE*)op2R->int_value);
                    break;
                }

                op1R->string_length++;
                extend_string_buffer(op1R, op1R->string_length);
                op1R->string_value[ op1R->string_length - 1] = (char)ch;
            }
        }
        }
#ifndef NUTF8
        {
            size_t chars = 0;
            if (validate_utf8_bytes(op1R->string_value, op1R->string_length, &chars) != 0) {
                SET_SIGNAL_MSG(RXSIGNAL_UNICODE_ERROR, "Invalid UTF-8 in text file read");
            } else {
                op1R->string_char_pos = 0;
                op1R->string_chars = chars;
                mark_utf8_valid_count(op1R);
            }
        }
#else
        clear_vm_private_flags(op1R);
#endif
        DISPATCH

    /* freadbyte - op1 (int) = read byte op2 file*(int) */
    START_INSTRUCTION(FREADBYTE_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FREADBYTE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = fgetc((FILE *)op2R->int_value);
        DISPATCH

    /* freadcdpt - op1 (string and int) = read codepoint op2 file*(int) */
    START_INSTRUCTION(FREADCDPT_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FREADCDPT R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        {
#ifndef NUTF8
            int codepoint;
            int first_byte;
            int next_byte;
            int invalid = 0;
            size_t chars = 0;
            op1R->string_pos = 0;
            prep_string_buffer(op1R, 4);

            /* Read the first byte - determines length */
            first_byte = fgetc((FILE *)op2R->int_value);
            if (first_byte == EOF) {
                op1R->int_value = -1;
                op1R->string_length = 0;
                op1R->string_char_pos = 0;
                op1R->string_chars = 0;
                mark_utf8_valid_count(op1R);
            } else {
                op1R->string_value[0] = (char)first_byte;

                /* Read the rest of the code point */
                if ((unsigned char)op1R->string_value[0] < 128) {
                    op1R->string_length = 1;
                } else if ((unsigned char)op1R->string_value[0] < 224) {
                    op1R->string_length = 2;
                } else if ((unsigned char)op1R->string_value[0] < 240) {
                    op1R->string_length = 3;
                } else {
                    op1R->string_length = 4;
                }

                if (op1R->string_length > 1) {
                    next_byte = fgetc((FILE *)op2R->int_value);
                    if (next_byte == EOF) invalid = 1;
                    else op1R->string_value[1] = (char)next_byte;
                }
                if (op1R->string_length > 2) {
                    next_byte = fgetc((FILE *)op2R->int_value);
                    if (next_byte == EOF) invalid = 1;
                    else op1R->string_value[2] = (char)next_byte;
                }
                if (op1R->string_length > 3) {
                    next_byte = fgetc((FILE *)op2R->int_value);
                    if (next_byte == EOF) invalid = 1;
                    else op1R->string_value[3] = (char)next_byte;
                }

                if (invalid || validate_utf8_bytes(op1R->string_value, op1R->string_length, &chars) != 0) {
                    SET_SIGNAL_MSG(RXSIGNAL_UNICODE_ERROR, "Invalid UTF-8 codepoint in text file read");
                } else {
                    utf8codepoint(op1R->string_value, &codepoint);
                    op1R->int_value = codepoint;
                    op1R->string_char_pos = 0;
                    op1R->string_chars = chars;
                    mark_utf8_valid_count(op1R);
                }
            }
#else
            {
                int byte = fgetc( (FILE*)op2R->int_value );
                if (byte == EOF) {
                    op1R->int_value = -1;
                    op1R->string_length = 0;
                } else {
                    prep_string_buffer(op1R, 1);
                    op1R->int_value = (unsigned char)byte;
                    op1R->string_value[0] = op1R->int_value;
                    op1R->string_length = 1;
                }
            }
            op1R->string_pos = 0;
            clear_vm_private_flags(op1R);
#endif
        }
        DISPATCH

    /* fwrite - fwrite to op1 file*(int) from op2(string) */
    START_INSTRUCTION(FWRITE_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FWRITE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        fwrite(op2R->string_value, op2R->string_length, 1, (FILE*)op1R->int_value);
        DISPATCH

    /* fwriteb - fwrite to op1 file*(int) from op2(binary) */
    START_INSTRUCTION(FWRITEB_REG_REG) CALC_DISPATCH(2)
    DEBUG("TRACE - FWRITEB R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
    fwrite(op2R->binary_value, op2R->binary_length, 1, (FILE*)op1R->int_value);
    DISPATCH

    /* fwritebyte - write byte to op1 file*(int) op2 source(int) */
    START_INSTRUCTION(FWRITEBYTE_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FWRITEBYTE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        fputc(op2R->int_value, (FILE*)op1R->int_value);
        DISPATCH

    /* fwritecdpt - write codepoint to op1 file*(int) op2 source(int) */
    START_INSTRUCTION(FWRITECDPT_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FWRITECDPT R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        {
            char codepoint[4];
            size_t length_of_codepoint;
#ifndef NUTF8
            char* end_of_codepoint;

            end_of_codepoint = utf8catcodepoint(codepoint, op2R->int_value, 4);
            length_of_codepoint = end_of_codepoint - codepoint;
#else
            length_of_codepoint = 1;
            codepoint[0] = op2R->int_value;
#endif
            fwrite(codepoint, length_of_codepoint, 1, (FILE*)op1R->int_value);
        }
        DISPATCH

    /* fclearerr - clearerr op1 file*(int) */
    START_INSTRUCTION(FCLEARERR_REG) CALC_DISPATCH(1)
        DEBUG("TRACE - FCLEARERR R%lu\n", REG_IDX(1));
        clearerr((FILE*)op1R->int_value);
        DISPATCH

    /* feof - op1 rc(int) = feof op2 file*(int) */
    START_INSTRUCTION(FEOF_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FEOF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = feof((FILE*)op2R->int_value);
        DISPATCH

    /* ferror - op1 rc(int) = ferror op2 file*(int) */
    START_INSTRUCTION(FERROR_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - FERROR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        op1R->int_value = ferror((FILE*)op2R->int_value);
        DISPATCH

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */
    START_INSTRUCTION(ICHKRNG_REG_INT_INT) CALC_DISPATCH(3)
        DEBUG("TRACE - ICHKRNG R%lu,%d,%d\n", REG_IDX(1), (int)op2I, (int)op3I);
        if (op1R->int_value < op2I || op1R->int_value > op3I) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */
    START_INSTRUCTION(ICHKRNG_REG_INT_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - ICHKRNG R%lu,%d,R%lu\n", REG_IDX(1), (int)op2I, REG_IDX(3));
        if (op1R->int_value < op2I || op1R->int_value > op3R->int_value) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */
    START_INSTRUCTION(ICHKRNG_REG_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - ICHKRNG R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
        if (op1R->int_value < op2R->int_value || op1R->int_value > op3R->int_value) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */
    START_INSTRUCTION(ICHKRNG_INT_INT_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - ICHKRNG %d,%d,R%lu\n", (int)op1I, (int)op2I, REG_IDX(3));
        if (op1I < op2I || op1I > op3R->int_value) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH

    /* ichkrng - if op1<op2 | op1>op3 signal OUT_OF_RANGE */
    START_INSTRUCTION(ICHKRNG_INT_REG_REG) CALC_DISPATCH(3)
        DEBUG("TRACE - ICHKRNG %d,R%lu,R%lu\n", (int)op1I, REG_IDX(2), REG_IDX(3));
        if (op1I < op2R->int_value || op1I > op3R->int_value) SET_SIGNAL(RXSIGNAL_OUT_OF_RANGE)
        DISPATCH

    /* getenv - get environment variable, op1=env[op2] */
    START_INSTRUCTION(GETENV_REG_REG) CALC_DISPATCH(2)
        DEBUG("TRACE - GETENV R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
        {
            char *value;
            int should_free = getEnvVal(&value, op2R->string_value, op2R->string_length);
            set_null_string(op1R, value);
            if (should_free) free(value);
        }
        DISPATCH

    /* getenv - get environment variable, op1=env[op2] */
    START_INSTRUCTION(GETENV_REG_STRING) CALC_DISPATCH(2)
        DEBUG("TRACE - GETENV R%lu,\"%.*s\"\n", REG_IDX(1),
              (int)(CONSTSTRING_OP(2))->string_len, (CONSTSTRING_OP(2))->string);
        {
            char *value;
            int should_free = getEnvVal(&value, (CONSTSTRING_OP(2))->string, (CONSTSTRING_OP(2))->string_len);
            set_null_string(op1R, value);
            if (should_free) free(value);
        }
        DISPATCH


    /* ------------------------------------------------------------------------------------
 *  TRIMR_REG_REG_REG  Trim right with char                          pej updated Jan 2026
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(TRIMR_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - TRIMR R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                rxinteger i;
                if (op1R != op2R) set_value_string(op1R, op2R);
                i = op1R->string_length - 1;
                while (i >= 0 && op1R->string_value[i] == op3R->string_value[0]) {
                    i--;
                }
                op1R->string_length = i + 1;
                null_terminate_string_buffer(op1R);
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  TRIML_REG_REG_REG  Trim left with char                           pej updated Jan 2026
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(TRIML_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - TRIML R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                rxinteger i = 0;
                rxinteger j;
                if (op1R != op2R) set_value_string(op1R, op2R);
                j = op1R->string_length - 1;
                while (i <= j && op1R->string_value[i] == op3R->string_value[0]) i++;

                if (i > j) {
                    op1R->string_length = 0;
                } else {
                    op1R->string_length = op1R->string_length - i;
                    if (i > 0) memmove(op1R->string_value, op1R->string_value + i, op1R->string_length);
                }
                null_terminate_string_buffer(op1R);
            }
            DISPATCH

/* ------------------------------------------------------------------------------------
 *  TRUNC_REG_REG_REG  Truncate string                              pej updated Jan 2026
 *  -----------------------------------------------------------------------------------
 */
        START_INSTRUCTION(TRUNC_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - TRUNC R%d,R%d,R%d\n", (int)REG_IDX(1), (int)REG_IDX(2), (int)REG_IDX(3));
            {
                rxinteger len = op3R->int_value;
                if (op1R != op2R) set_value_string(op1R, op2R);
                if (len < 0) len = 0;
                if (len < op1R->string_length) {
                    op1R->string_length = len;
                    null_terminate_string_buffer(op1R);
                }
            }
            DISPATCH

        START_INSTRUCTION(IUNKNOWN)
        START_INSTRUCTION(INULL)
            SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
            DISPATCH

        START_INSTRUCTION(EXIT)
            DEBUG("TRACE - EXIT");
            rc = 0;
            goto interprt_finished;

        START_INSTRUCTION(EXIT_INT)
            DEBUG("TRACE - EXIT %d\n", (int)op1I);
            rc = op1I;
            goto interprt_finished;

        START_INSTRUCTION(EXIT_REG)
            DEBUG("TRACE - EXIT R%lu\n", REG_IDX(1));
            rc = op1RI;
            goto interprt_finished;

        START_INSTRUCTION(SETOBJTYPE_REG_STRING) CALC_DISPATCH(2)
            DEBUG("TRACE - SETOBJTYPE R%lu,\"%.*s\"\n", REG_IDX(1), (int) op2S->string_len, op2S->string);
            op1R->object_type_name = op2S->string;
            op1R->object_type_name_length = op2S->string_len;
            DISPATCH

        START_INSTRUCTION(SRCMETHOD_REG_REG_STRING) CALC_DISPATCH(3)
            {
                proc_runtime *called_function;
                const char *class_name;
                size_t class_name_length;

                DEBUG("TRACE - SRCMETHOD R%lu,R%lu,\"%.*s\"\n", REG_IDX(1), REG_IDX(2),
                      (int) op3S->string_len, op3S->string);

                class_name = op2R->object_type_name;
                class_name_length = op2R->object_type_name_length;
                called_function = 0;

                if (!class_name || !class_name_length) {
                    char *proc_name;
                    proc_name = build_runtime_member_name("<untyped>", 9, op3S->string, op3S->string_len);
                    if (proc_name) {
                        SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, proc_name);
                        free(proc_name);
                    }
                    else {
                        SET_SIGNAL(RXSIGNAL_FAILURE);
                    }
                    DISPATCH
                }

                called_function = resolve_runtime_method(context,
                                                         class_name,
                                                         class_name_length,
                                                         op3S->string,
                                                         op3S->string_len);
                if (!called_function) {
                    char *proc_name;
                    proc_name = build_runtime_member_name(class_name, class_name_length, op3S->string, op3S->string_len);
                    if (!proc_name) {
                        SET_SIGNAL(RXSIGNAL_FAILURE);
                        DISPATCH
                    }
                    SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, proc_name);
                    free(proc_name);
                    DISPATCH
                }

                value_zero(op1R);
                op1R->int_value = (rxinteger) called_function;
            }
            DISPATCH

        START_INSTRUCTION(SRCFPROC_REG_STRING_REG) CALC_DISPATCH(3)
            {
                proc_runtime *called_function;
                char *error_message;

                DEBUG("TRACE - SRCFPROC R%lu,\"%.*s\",R%lu\n", REG_IDX(1),
                      (int) op2S->string_len, op2S->string, REG_IDX(3));

                called_function = 0;
                error_message = 0;

                if (!resolve_runtime_factory(context,
                                             op2S->string,
                                             op2S->string_len,
                                             (rxinteger) op3R->int_value,
                                             (&(op3R)) + 1,
                                             &called_function,
                                             &error_message)) {
                    if (error_message) {
                        SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, error_message);
                        free(error_message);
                    } else {
                        SET_SIGNAL(RXSIGNAL_FAILURE);
                    }
                    DISPATCH
                }

                value_zero(op1R);
                op1R->int_value = (rxinteger) called_function;
            }
            DISPATCH

        START_INSTRUCTION(TYPEOF_REG_REG) CALC_DISPATCH(2)
            {
                char *type_name;

                DEBUG("TRACE - TYPEOF R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));

                if (op2R->object_type_name && op2R->object_type_name_length > 0) {
                    type_name = runtime_internal_type_to_source_name(op2R->object_type_name,
                                                                     op2R->object_type_name_length);
                } else {
                    type_name = strdup(".object");
                }

                if (!type_name) {
                    SET_SIGNAL(RXSIGNAL_FAILURE);
                    DISPATCH
                }

                value_zero(op1R);
                set_null_string(op1R, type_name);
                free(type_name);
            }
            DISPATCH

        START_INSTRUCTION(ISTYPE_REG_REG_STRING) CALC_DISPATCH(3)
            DEBUG("TRACE - ISTYPE R%lu,R%lu,\"%.*s\"\n", REG_IDX(1), REG_IDX(2),
                  (int) op3S->string_len, op3S->string);
            value_zero(op1R);
            set_int(op1R, runtime_value_matches_object_type(context,
                                                            op2R,
                                                            op3S->string,
                                                            op3S->string_len));
            DISPATCH

        START_INSTRUCTION(ASSERTTYPE_REG_STRING) CALC_DISPATCH(2)
            DEBUG("TRACE - ASSERTTYPE R%lu,\"%.*s\"\n", REG_IDX(1),
                  (int) op2S->string_len, op2S->string);
            if (!runtime_value_matches_object_type(context, op1R, op2S->string, op2S->string_len)) {
                char *error_message = build_runtime_cast_error(op1R, op2S->string, op2S->string_len);
                if (error_message) {
                    SET_SIGNAL_MSG(RXSIGNAL_CONVERSION_ERROR, error_message);
                    free(error_message);
                } else {
                    SET_SIGNAL(RXSIGNAL_CONVERSION_ERROR);
                }
                DISPATCH
            }
            DISPATCH

        START_INSTRUCTION(SOCKNEW_REG) CALC_DISPATCH(1)
            DEBUG("TRACE - SOCKNEW R%lu\n", REG_IDX(1));
            set_int(op1R, rxvm_socket_new(context));
            DISPATCH

        START_INSTRUCTION(SOCKCLOSE_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SOCKCLOSE R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            set_int(op1R, rxvm_socket_close(context, op2R->int_value));
            DISPATCH

        START_INSTRUCTION(SOCKCONNECT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKCONNECT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            rxvm_socket_connect(context, op1R->int_value, op2R, op3R->int_value);
            DISPATCH

        START_INSTRUCTION(SOCKBIND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKBIND R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            rxvm_socket_bind(context, op1R->int_value, op2R, op3R->int_value);
            DISPATCH

        START_INSTRUCTION(SOCKLISTEN_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKLISTEN R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_listen(context, op2R->int_value, op3R->int_value));
            DISPATCH

        START_INSTRUCTION(SOCKACCEPT_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SOCKACCEPT R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            set_int(op1R, rxvm_socket_accept(context, op2R->int_value));
            DISPATCH

        START_INSTRUCTION(SOCKSHUTDOWN_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKSHUTDOWN R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_shutdown(context, op2R->int_value, op3R->int_value));
            DISPATCH

        START_INSTRUCTION(SOCKSEND_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKSEND R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_send_string(context, op2R->int_value, op3R));
            DISPATCH

        START_INSTRUCTION(SOCKSENDB_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKSENDB R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_send_binary(context, op2R->int_value, op3R));
            DISPATCH

        START_INSTRUCTION(SOCKRECV_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKRECV R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            rxvm_socket_recv_string(context, op1R, op2R->int_value, op3R->int_value);
            DISPATCH

        START_INSTRUCTION(SOCKRECVB_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKRECVB R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            rxvm_socket_recv_binary(context, op1R, op2R->int_value, op3R->int_value);
            DISPATCH

        START_INSTRUCTION(SOCKPENDING_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SOCKPENDING R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            set_int(op1R, rxvm_socket_pending(context, op2R->int_value));
            DISPATCH

        START_INSTRUCTION(SOCKTIMEOUT_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKTIMEOUT R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_timeout(context, op2R->int_value, op3R->int_value));
            DISPATCH

        START_INSTRUCTION(SOCKBLOCKING_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKBLOCKING R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_blocking(context, op2R->int_value, op3R->int_value));
            DISPATCH

        START_INSTRUCTION(SOCKNODELAY_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKNODELAY R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_nodelay(context, op2R->int_value, op3R->int_value));
            DISPATCH

        START_INSTRUCTION(SOCKKEEPALIVE_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKKEEPALIVE R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_keepalive(context, op2R->int_value, op3R->int_value));
            DISPATCH

        START_INSTRUCTION(SOCKPEER_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SOCKPEER R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            rxvm_socket_peer(context, op1R, op2R->int_value);
            DISPATCH

        START_INSTRUCTION(SOCKLOCAL_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SOCKLOCAL R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            rxvm_socket_local(context, op1R, op2R->int_value);
            DISPATCH

        START_INSTRUCTION(SOCKSTATUS_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SOCKSTATUS R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            set_int(op1R, rxvm_socket_status(context, op2R->int_value));
            DISPATCH

        START_INSTRUCTION(SOCKERROR_REG_REG) CALC_DISPATCH(2)
            DEBUG("TRACE - SOCKERROR R%lu,R%lu\n", REG_IDX(1), REG_IDX(2));
            rxvm_socket_error(context, op1R, op2R->int_value);
            DISPATCH

        START_INSTRUCTION(SOCKSTARTTLS_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKSTARTTLS R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            set_int(op1R, rxvm_socket_starttls(context, op2R->int_value, op3R));
            DISPATCH

        START_INSTRUCTION(SOCKCONNECTTLS_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - SOCKCONNECTTLS R%lu,R%lu,R%lu\n", REG_IDX(1), REG_IDX(2), REG_IDX(3));
            rxvm_socket_connect_tls(context, op1R->int_value, op2R, op3R->int_value);
            DISPATCH

        RESERVED_IMPL(RESERVED_084)
        RESERVED_IMPL(RESERVED_085)
        RESERVED_IMPL(RESERVED_086)
        RESERVED_IMPL(RESERVED_087)
        RESERVED_IMPL(RESERVED_088)
        RESERVED_IMPL(RESERVED_089)
        RESERVED_IMPL(RESERVED_090)
        RESERVED_IMPL(RESERVED_091)
        RESERVED_IMPL(RESERVED_092)
        RESERVED_IMPL(RESERVED_093)
        RESERVED_IMPL(RESERVED_094)
        RESERVED_IMPL(RESERVED_095)
        RESERVED_IMPL(RESERVED_096)
        RESERVED_IMPL(RESERVED_097)
        RESERVED_IMPL(RESERVED_098)
        RESERVED_IMPL(RESERVED_099)
        RESERVED_IMPL(RESERVED_263)
        RESERVED_IMPL(RESERVED_264)
        RESERVED_IMPL(RESERVED_265)
        RESERVED_IMPL(RESERVED_266)
        RESERVED_IMPL(RESERVED_267)
        RESERVED_IMPL(RESERVED_268)
        RESERVED_IMPL(RESERVED_269)
        RESERVED_IMPL(RESERVED_270)
        RESERVED_IMPL(RESERVED_271)
        RESERVED_IMPL(RESERVED_272)
        RESERVED_IMPL(RESERVED_273)
        RESERVED_IMPL(RESERVED_274)
        RESERVED_IMPL(RESERVED_275)
        RESERVED_IMPL(RESERVED_276)
        RESERVED_IMPL(RESERVED_277)
        RESERVED_IMPL(RESERVED_278)
        RESERVED_IMPL(RESERVED_279)
        RESERVED_IMPL(RESERVED_280)
        RESERVED_IMPL(RESERVED_281)
        RESERVED_IMPL(RESERVED_282)
        RESERVED_IMPL(RESERVED_283)
        RESERVED_IMPL(RESERVED_284)
        RESERVED_IMPL(RESERVED_285)
        RESERVED_IMPL(RESERVED_286)
        RESERVED_IMPL(RESERVED_287)
        RESERVED_IMPL(RESERVED_288)
        RESERVED_IMPL(RESERVED_289)
        RESERVED_IMPL(RESERVED_290)
        RESERVED_IMPL(RESERVED_291)
        RESERVED_IMPL(RESERVED_292)
        RESERVED_IMPL(RESERVED_293)
        RESERVED_IMPL(RESERVED_294)
        RESERVED_IMPL(RESERVED_295)
        RESERVED_IMPL(RESERVED_296)
        RESERVED_IMPL(RESERVED_297)
        RESERVED_IMPL(RESERVED_298)
        RESERVED_IMPL(RESERVED_299)
        RESERVED_IMPL(RESERVED_401)
        RESERVED_IMPL(RESERVED_402)
        RESERVED_IMPL(RESERVED_403)
        RESERVED_IMPL(RESERVED_404)
        RESERVED_IMPL(RESERVED_405)
        RESERVED_IMPL(RESERVED_406)
        RESERVED_IMPL(RESERVED_407)
        RESERVED_IMPL(RESERVED_408)
        RESERVED_IMPL(RESERVED_409)
        RESERVED_IMPL(RESERVED_410)
        RESERVED_IMPL(RESERVED_411)
        RESERVED_IMPL(RESERVED_412)
        RESERVED_IMPL(RESERVED_413)
        RESERVED_IMPL(RESERVED_414)
        RESERVED_IMPL(RESERVED_415)
        RESERVED_IMPL(RESERVED_416)
        RESERVED_IMPL(RESERVED_417)
        RESERVED_IMPL(RESERVED_418)
        RESERVED_IMPL(RESERVED_419)
        RESERVED_IMPL(RESERVED_420)
        RESERVED_IMPL(RESERVED_421)
        RESERVED_IMPL(RESERVED_422)
        RESERVED_IMPL(RESERVED_423)
        RESERVED_IMPL(RESERVED_424)
        RESERVED_IMPL(RESERVED_425)
        RESERVED_IMPL(RESERVED_426)
        RESERVED_IMPL(RESERVED_427)
        RESERVED_IMPL(RESERVED_428)
        RESERVED_IMPL(RESERVED_429)
        RESERVED_IMPL(RESERVED_430)
        RESERVED_IMPL(RESERVED_431)
        RESERVED_IMPL(RESERVED_432)
        RESERVED_IMPL(RESERVED_433)
        RESERVED_IMPL(RESERVED_434)
        RESERVED_IMPL(RESERVED_435)
        RESERVED_IMPL(RESERVED_436)
        RESERVED_IMPL(RESERVED_437)
        RESERVED_IMPL(RESERVED_438)
        RESERVED_IMPL(RESERVED_439)
        RESERVED_IMPL(RESERVED_440)
        RESERVED_IMPL(RESERVED_441)
        RESERVED_IMPL(RESERVED_442)
        RESERVED_IMPL(RESERVED_443)
        RESERVED_IMPL(RESERVED_444)
        RESERVED_IMPL(RESERVED_445)
        RESERVED_IMPL(RESERVED_446)
        RESERVED_IMPL(RESERVED_447)
        RESERVED_IMPL(RESERVED_448)
        RESERVED_IMPL(RESERVED_449)
        RESERVED_IMPL(RESERVED_514)
        RESERVED_IMPL(RESERVED_515)

    END_OF_INSTRUCTIONS

    interprt_finished:

    /* Cleanup / Remove OS Interrupt handlers */
    cleanup_vm_signals();

    /* Unwind any stack frames */
    while (current_frame) {
        temp_frame = current_frame->parent;
        free_frame(current_frame);
        current_frame = temp_frame;
    }

    /* Deallocate Frames */
    /* We need to loop through each procedure in each module */
    DEBUG("Deallocating Frames and Registers\n");
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        size_t i;
        for (i = 0; i < context->modules[mod_index]->procedure_count; i++) {
            proc_runtime *runtime_proc = &context->modules[mod_index]->procedures[i];
            if (runtime_proc->frame_free_list == &runtime_proc->frame_free_list_head) {
                /* Free frames in the procedures free list */
                while (*(runtime_proc->frame_free_list)) {
                    temp_frame = *(runtime_proc->frame_free_list);
                    *(runtime_proc->frame_free_list) = temp_frame->prev_free;
                    clear_frame(temp_frame);
                    free(temp_frame);
                }
            }
        }
    }

    /* Free signal value */
    clear_value(signal_value);
    free(signal_value);

    /* Free interrupt action value */
    clear_value(interrupt_action_value);
    free(interrupt_action_value);

    /* Free work registers */
    clear_value(work1);
    free(work1);
    clear_value(work2);
    free(work2);
    clear_value(work3);
    free(work3);

    /* Free interrupt argument */
    clear_value(interrupt_arg);
    free(interrupt_arg);

    /* Free array of interrupt objects - interrupt_object[] */
    {
        size_t i;
        for (i = 0; i < RXSIGNAL_MAX; i++) {
            clear_value(interrupt_object[i]);
            free(interrupt_object[i]);
        }
    }

    /* Free arguments array */
    if (arguments_array) {
        clear_value(arguments_array);
        free(arguments_array);
    }

#ifndef NDEBUG
    if (context->debug_mode) rxvm_mprintf("Interpreter Finished with rc=%d\n", rc);
#endif

    return rc;
}
