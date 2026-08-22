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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxcp_emit.h"
#include "rxcp_util.h"

static const char *signal_block_catch_all_names[] = {
        "FAILURE", "ERROR", "OVERFLOW_UNDERFLOW", "DIVISION_BY_ZERO",
        "CONVERSION_ERROR", "INVALID_ARGUMENTS", "OUT_OF_RANGE", "UNICODE_ERROR",
        "REFERENCE_INVALID", "OBJECT_NOT_INITIALIZED", "RXBIN_CORRUPTION", "UNKNOWN_INSTRUCTION", "FUNCTION_NOT_FOUND", "NOT_IMPLEMENTED",
        "INVALID_SIGNAL_CODE", "NOTREADY", "QUIT", "TERM", "POSIX_INT",
        "POSIX_HUP", "POSIX_USR1", "POSIX_USR2", "POSIX_CHLD",
        "CHANNEL_ERROR", "TASK_FAILURE", "OTHER", 0
};

/* A direct final BLOCK_EXPR exit is immediately followed by the block-end
 * label. Cleanup and signal unwind are still emitted; only the branch to the
 * next instruction is redundant. */
static int leave_with_falls_through_to_block_end(ASTNode *node) {
    ASTNode *instrs;

    if (!node || node->node_type != LEAVE_WITH || node->sibling ||
        !(instrs = node->parent) || instrs->node_type != INSTRUCTIONS ||
        !instrs->parent || instrs->parent->node_type != BLOCK_EXPR) {
        return 0;
    }
    return node->association == instrs->parent;
}

typedef struct signal_emit_names {
    char **items;
    size_t count;
    size_t capacity;
} signal_emit_names;

static void signal_emit_names_add(signal_emit_names *names, const char *name) {
    if (!names || !name) return;
    if (names->count == names->capacity) {
        size_t new_capacity = names->capacity ? names->capacity * 2 : 8;
        char **new_items = realloc(names->items, sizeof(char*) * new_capacity);
        if (!new_items) return;
        names->items = new_items;
        names->capacity = new_capacity;
    }
    names->items[names->count++] = strdup(name);
}

static void signal_emit_names_free(signal_emit_names *names) {
    size_t i;
    if (!names) return;
    for (i = 0; i < names->count; i++) free(names->items[i]);
    free(names->items);
    names->items = 0;
    names->count = 0;
    names->capacity = 0;
}

static int flow_scope_owns_cleanup(ASTNode *node) {
    return node && node->scope && node->scope->defining_node == node;
}

static int flow_scope_owns_recyclable_registers(ASTNode *node) {
    if (!flow_scope_owns_cleanup(node)) return 0;
    if (node->inherit_parent_reg_scope) return 0;
    if (node->scope->type != SCOPE_LOCAL) return 0;
    return 1;
}

/* Return the literal held by an ordinary write-once decimal symbol without
 * replacing the symbol use.  Decimal immediates are parsed by the VM at each
 * instruction dispatch, so arithmetic must retain the prepared register.
 * Counted-loop emission still benefits from the narrower fact that a fixed BY
 * value has a known sign.  This mirrors the mature constant-propagation
 * definition/write checks while leaving storage and TRACE-visible reads intact. */
static const char *flow_readonly_decimal_literal(ASTNode *node) {
    static const char zero[] = "0.0";
    Symbol *symbol;
    ASTNode *first;
    ASTNode *assignment;
    ASTNode *value;
    size_t i;
    size_t writes_start;

    if (!node || !node->symbolNode) return 0;
    symbol = node->symbolNode->symbol;
    if (!symbol || symbol->type != TP_DECIMAL) return 0;
    if (symbol->scope && symbol->scope->type == SCOPE_CLASS) return 0;
    if (symbol->has_reference_target || symbol->value_dims) return 0;
    if (!sym_nond(symbol)) return 0;

    first = sym_trnd(symbol, 0)->node;
    value = 0;
    writes_start = 0;

    if (first && first->node_type == VAR_TARGET && first->parent &&
        first->parent->node_type == ASSIGN && first->sibling) {
        value = first->sibling;
        writes_start = 1;
    }
    else if (first && first->parent && first->parent->node_type == DEFINE) {
        if (sym_nond(symbol) < 2) return zero;
        assignment = sym_trnd(symbol, 1)->node;
        if (assignment && assignment->node_type == VAR_TARGET &&
            assignment->parent && assignment->parent->node_type == ASSIGN &&
            assignment->sibling) {
            value = assignment->sibling;
            writes_start = 2;
        }
        else {
            for (i = 1; i < sym_nond(symbol); i++) {
                if (sym_trnd(symbol, i)->writeUsage) return 0;
            }
            return zero;
        }
    }
    else return 0;

    for (i = writes_start; i < sym_nond(symbol); i++) {
        if (sym_trnd(symbol, i)->writeUsage) return 0;
    }
    if (!is_constant(value) || value->value_type != TP_DECIMAL ||
        value->target_type != TP_DECIMAL || !value->decimal_value) return 0;
    return value->decimal_value;
}

static void emit_dispatch(ASTNode *node) {
    ASTNode *selector = ast_chdn(node, 0);
    ASTNode *entry;
    ASTNode *default_entry = 0;
    char *text;
    char *comment_meta;
    size_t case_index = 0;
    const char *instruction;
    ValueType key_type;

    if (!selector) return;
    switch ((DispatchKind)node->dispatch_kind) {
        case DISPATCH_INTEGER:
            instruction = "jumpi";
            key_type = TP_INTEGER;
            break;
        case DISPATCH_STRING_EXACT:
            instruction = "jumps";
            key_type = TP_STRING;
            break;
        case DISPATCH_STRING_PADDED:
            instruction = "jumpr";
            key_type = TP_STRING;
            break;
        case DISPATCH_STRING_NUMERIC:
            instruction = "jumpn";
            key_type = TP_STRING;
            break;
        case DISPATCH_BINARY_EXACT:
            instruction = "jumpb";
            key_type = TP_BINARY;
            break;
        default:
            return;
    }
    if (!node->output) node->output = output_f();

    comment_meta = get_metaline(node);
    output_prepend_text(comment_meta, node->output);
    free(comment_meta);

    if (selector->output) output_concat(node->output, selector->output);
    text = mprintf("   .jtable jtable%d auto\n"
                   "   %s %c%d,jtable%d\n",
                   node->node_number,
                   instruction,
                   selector->register_type,
                   selector->register_num,
                   node->node_number);
    output_append_text(node->output, text);
    free(text);

    for (entry = selector->sibling; entry; entry = entry->sibling) {
        if (entry->node_type == OPT_DISPATCH_DEFAULT) {
            default_entry = entry;
            break;
        }
    }

    text = mprintf("   br l%ddispatch%s\n",
                   node->node_number,
                   default_entry ? "default" : "end");
    output_append_text(node->output, text);
    free(text);

    for (entry = selector->sibling; entry; entry = entry->sibling) {
        ASTNode *key;
        ASTNode *body;
        char *key_text;

        if (entry->node_type != OPT_DISPATCH_CASE) continue;
        key = ast_chdn(entry, 0);
        body = ast_chdn(entry, 1);
        if (!key || !body) continue;

        key_text = format_constant(key_type, key);
        text = mprintf("l%ddispatchcase%zu: .jcase jtable%d %s\n",
                       node->node_number,
                       case_index,
                       node->node_number,
                       key_text);
        output_append_text(node->output, text);
        free(text);
        free(key_text);

        if (body->output) output_concat(node->output, body->output);
        if (body->cleanup) output_concat(node->output, body->cleanup);
        text = mprintf("   br l%ddispatchend\n", node->node_number);
        output_append_text(node->output, text);
        free(text);
        case_index++;
    }

    if (default_entry) {
        ASTNode *body = ast_chdn(default_entry, 0);
        text = mprintf("l%ddispatchdefault:\n", node->node_number);
        output_append_text(node->output, text);
        free(text);
        if (body && body->output) output_concat(node->output, body->output);
        if (body && body->cleanup) output_concat(node->output, body->cleanup);
    }

    text = mprintf("l%ddispatchend:\n", node->node_number);
    output_append_text(node->output, text);
    free(text);
    if (selector->cleanup) output_concat(node->output, selector->cleanup);
}

static void flow_emit_scope_dereference_unlinks(OutputFragment *output, Scope *scope) {
    size_t i;
    Symbol *pending = 0;

    if (!output || !scope) return;
    for (i = scp_dereference_symbol_count(scope); i > 0; i--) {
        Symbol *symbol = scp_dereference_symbol_at(scope, i - 1);
        char *line;

        if (!symbol || symbol->register_num < 0 || symbol->register_type != 'r') continue;
        if (!pending) {
            pending = symbol;
        } else {
            line = mprintf("   unlinkn %c%d,%c%d\n",
                           pending->register_type, pending->register_num,
                           symbol->register_type, symbol->register_num);
            output_append_text(output, line);
            free(line);
            pending = 0;
        }
    }
    if (pending) {
        char *line = mprintf("   unlink %c%d\n",
                             pending->register_type, pending->register_num);
        output_append_text(output, line);
        free(line);
    }
}

static int flow_symbol_owns_scope_lifetime(Symbol *symbol) {
    if (!symbol || symbol->symbol_type != VARIABLE_SYMBOL) return 0;
    if (symbol->inline_value_alias) return 0;
    if (symbol->exposed || symbol->is_arg || symbol->is_ref_arg ||
        symbol->is_this || symbol->is_factory) return 0;
    if (symbol->register_type != 'r' || symbol->register_num < 0) return 0;
    if (symbol->name && strncmp(symbol->name, "__inline", 8) == 0) return 0;
    return 1;
}

static int flow_scope_is_generated_lifetime_boundary(Scope *scope) {
    Scope *current;

    for (current = scope; current; current = current->parent) {
        if (current->type != SCOPE_PROCEDURE) continue;
        if (!current->name) return 0;
        return strncmp(current->name, "__inline", 8) == 0 ||
               strncmp(current->name, "__rxtrace", 9) == 0;
    }
    return 0;
}

static int flow_symbol_is_reference_lifetime_free_scalar(Symbol *symbol) {
    size_t i;

    if (!symbol || symbol->symbol_type != VARIABLE_SYMBOL) return 0;
    if (!symbol->scope || symbol->scope->type != SCOPE_LOCAL) return 0;
    if (symbol->exposed || symbol->is_arg || symbol->is_ref_arg ||
        symbol->is_this || symbol->is_factory) return 0;
    if (symbol->register_type != 'r' || symbol->register_num < 0) return 0;
    if (symbol->has_reference_target || symbol->value_dims != 0) return 0;
    if (flow_scope_is_generated_lifetime_boundary(symbol->scope)) return 0;
    if (symbol->name && (strncmp(symbol->name, "__inline", 8) == 0 ||
                         strncmp(symbol->name, "__rxtrace", 9) == 0)) return 0;

    /* A dereference local is an alias even when its resolved value type is scalar. */
    for (i = 0; i < scp_dereference_symbol_count(symbol->scope); i++) {
        if (scp_dereference_symbol_at(symbol->scope, i) == symbol) return 0;
    }

    switch (symbol->type) {
        case TP_BOOLEAN:
        case TP_INTEGER:
        case TP_FLOAT:
        case TP_DECIMAL:
        case TP_STRING:
            return 1;
        default:
            return 0;
    }
}

void emit_scope_reference_lifetimes(OutputFragment *output, ASTNode *node) {
    Symbol **symbols;
    size_t i;

    if (!output || !node || !flow_scope_owns_recyclable_registers(node)) return;

    symbols = scp_syms(node->scope);
    if (!symbols) return;

    for (i = 0; symbols[i]; i++) {
        Symbol *symbol = symbols[i];
        char *line;

        if (!flow_symbol_owns_scope_lifetime(symbol)) continue;
        if (flow_symbol_is_reference_lifetime_free_scalar(symbol)) continue;
        line = mprintf("   endlife %c%d\n", symbol->register_type, symbol->register_num);
        output_append_text(output, line);
        free(line);
    }

    free(symbols);
}

static char *signal_emit_canonical_name(ASTNode *node) {
    char *name;
    size_t i;

    if (!node || !node->node_string || node->node_string_length == 0) return 0;
    name = malloc(node->node_string_length + 1);
    if (!name) return 0;
    for (i = 0; i < node->node_string_length; i++) {
        name[i] = (char)toupper((unsigned char)node->node_string[i]);
    }
    name[node->node_string_length] = 0;
    if (strcmp(name, "SYNTAX") == 0) {
        free(name);
        return strdup("ERROR");
    }
    return name;
}

static void signal_emit_collect_names(ASTNode *names_node, signal_emit_names *names) {
    ASTNode *name_node;
    const char **catch_all;

    if (!names_node || !names) return;
    if (!names_node->child) {
        for (catch_all = signal_block_catch_all_names; *catch_all; catch_all++) {
            signal_emit_names_add(names, *catch_all);
        }
        return;
    }

    name_node = names_node->child;
    while (name_node) {
        char *name = signal_emit_canonical_name(name_node);
        if (name) {
            signal_emit_names_add(names, name);
            free(name);
        }
        name_node = ast_nsib(name_node);
    }
}

static ASTNode *signal_handler_binding_target(ASTNode *handler) {
    ASTNode *binding;

    if (!handler) return 0;
    binding = ast_chdn(handler, 1);
    if (!binding || binding->node_type != DEFINE) return 0;
    return binding->child;
}

static void signal_emit_pop_names(OutputFragment *output, signal_emit_names *installed) {
    size_t i;
    char *line;

    if (!output || !installed) return;
    for (i = installed->count; i > 0; i--) {
        line = mprintf("   sigpop \"%s\"\n", installed->items[i - 1]);
        output_append_text(output, line);
        free(line);
    }
}

/* Control transfers that leave a protected block must restore every branch
 * handler installed by that block.  Normal SIGNAL_BLOCK fall-through already
 * emits the same pops; this path covers RETURN and LEAVE without relying on a
 * runtime frame teardown to repair block-scoped handler policy. */
static void signal_emit_unwind_for_control(OutputFragment *output,
                                           ASTNode *node,
                                           ASTNode *stop_before) {
    ASTNode *ancestor;
    ASTNode *handler_owner = 0;

    if (!output || !node) return;
    for (ancestor = node->parent;
         ancestor && ancestor != stop_before;
         ancestor = ancestor->parent) {
        if (ancestor->node_type == SIGNAL_HANDLER) {
            handler_owner = ancestor->parent;
        } else if (ancestor->node_type == SIGNAL_BLOCK) {
            ASTNode *handler;
            signal_emit_names installed = {0};

            if (ancestor == handler_owner) {
                handler_owner = 0;
                continue;
            }
            for (handler = ast_chdn(ancestor, 1);
                 handler;
                 handler = ast_nsib(handler)) {
                signal_emit_names handler_names = {0};
                size_t i;

                signal_emit_collect_names(handler->child, &handler_names);
                for (i = 0; i < handler_names.count; i++) {
                    signal_emit_names_add(&installed, handler_names.items[i]);
                }
                signal_emit_names_free(&handler_names);
            }
            signal_emit_pop_names(output, &installed);
            signal_emit_names_free(&installed);
        }
    }
}

static void flow_append_output_copy(OutputFragment *output,
                                    OutputFragment *source) {
    if (!output || !source) return;
    while (source->before) source = source->before;
    for (; source; source = source->after) {
        if (source->output) output_append_text(output, source->output);
    }
}

/* A structured exit may bypass the normal end of one or more lexical scopes
 * and counted loops.  Their cleanup cannot be moved into the exit path because
 * the ordinary fall-through path still needs it, so reproduce it for every
 * crossed ancestor.  The target itself is excluded: LEAVE branches to its
 * normal end cleanup, ITERATE keeps the target loop active, and LEAVE_WITH
 * branches to the block-expression cleanup label. */
static void flow_emit_crossed_cleanups(OutputFragment *output,
                                       ASTNode *node,
                                       ASTNode *stop_before) {
    ASTNode *ancestor;

    if (!output || !node) return;
    for (ancestor = node->parent;
         ancestor && ancestor != stop_before;
         ancestor = ancestor->parent) {
        if (ancestor->node_type == DO) {
            ASTNode *repeat = ast_chdn(ancestor, 0);
            if (repeat && repeat->node_type == REPEAT && repeat->cleanup) {
                flow_append_output_copy(output, repeat->cleanup);
            }
        }

        if (flow_scope_owns_cleanup(ancestor)) {
            flow_emit_scope_dereference_unlinks(output, ancestor->scope);
            emit_scope_reference_lifetimes(output, ancestor);
            clear_scope_variable_metadata(output, ancestor);
        }
    }
}

/* A runtime SIGNAL can originate at any instruction in a protected body, so
 * its handler cannot use the single-ancestor path available to an explicit
 * LEAVE.  Restore every register that any descendant scope may have linked,
 * then end descendant reference lifetimes after all pointer mappings are back
 * at their base storage.  Repeating an unlink or lifetime release for an
 * inactive/reused sibling scope is intentionally harmless. */
static void flow_emit_descendant_unlinks(OutputFragment *output,
                                         ASTNode *node) {
    ASTNode *child;

    if (!output || !node) return;
    if (node->node_type == DO) {
        if (node->branch_cleanup) {
            flow_append_output_copy(output, node->branch_cleanup);
        }
    }
    if (flow_scope_owns_cleanup(node)) {
        flow_emit_scope_dereference_unlinks(output, node->scope);
    }
    for (child = node->child; child; child = child->sibling) {
        flow_emit_descendant_unlinks(output, child);
    }
}

static void flow_emit_descendant_lifetimes(OutputFragment *output,
                                           ASTNode *node) {
    ASTNode *child;

    if (!output || !node) return;
    if (flow_scope_owns_cleanup(node)) {
        emit_scope_reference_lifetimes(output, node);
    }
    for (child = node->child; child; child = child->sibling) {
        flow_emit_descendant_lifetimes(output, child);
    }
}

static void flow_emit_descendant_metadata_clear(OutputFragment *output,
                                                ASTNode *node) {
    ASTNode *child;

    if (!output || !node) return;
    if (flow_scope_owns_cleanup(node)) {
        clear_scope_variable_metadata(output, node);
    }
    for (child = node->child; child; child = child->sibling) {
        flow_emit_descendant_metadata_clear(output, child);
    }
}

void emit_flow(ASTNode *node, void *pl) {
    walker_payload *payload = (walker_payload*) pl;
    ASTNode *child1, *child2, *child3, *n;
    char *tp_prefix;
    char *temp1;
    char *temp2;
    char *comment_meta;
    char *op;
    const char *constant_decimal;
    int j;
    unsigned int trace_step_id;
    unsigned int trace_clause_id;

    child1 = node->child;
    if (child1) child2 = child1->sibling;
    else child2 = NULL;
    if (child2) child3 = child2->sibling;
    else child3 = NULL;

    if (node->value_dims) tp_prefix = "";
    else tp_prefix = type_to_prefix(node->value_type);

    if (tp_prefix[0] == 's' && (node->node_type == TO || node->node_type == BY)) tp_prefix = "f";

    switch (node->node_type) {

        case ARGS:
            if (!node->output) node->output = output_f();
            n = child1;
            while (n) {
                if (n->output) output_concat(node->output, n->output);
                if (n->cleanup) output_concat(node->output, n->cleanup);
                n = ast_nsib(n);
            }
            break;

        case INSTRUCTIONS:
            if (!node->output) node->output = output_f();
            n = child1;
            while (n) {
                if (n->output) output_concat(node->output, n->output);
                if (n->cleanup) output_concat(node->output, n->cleanup);
                n = ast_nsib(n);
            }
            if (flow_scope_owns_cleanup(node)) {
                flow_emit_scope_dereference_unlinks(node->output, node->scope);
                emit_scope_reference_lifetimes(node->output, node);
                clear_scope_variable_metadata(node->output, node);
            }
            comment_meta = get_reporting_metalines(node);
            if (comment_meta[0]) output_prepend_text(comment_meta, node->output);
            free(comment_meta);
            break;

        case SIGNAL_BLOCK: {
            signal_emit_names installed = {0};
            ASTNode *handler;
            size_t handler_index;

            if (!node->output) node->output = output_f();

            comment_meta = get_metaline_token_at(node);
            output_append_text(node->output, comment_meta);
            free(comment_meta);

            handler = child2;
            handler_index = 0;
            while (handler) {
                signal_emit_names handler_names = {0};

                signal_emit_collect_names(handler->child, &handler_names);
                for (j = 0; j < (int)handler_names.count; j++) {
                    temp1 = mprintf("   sigpush \"%s\"\n", handler_names.items[j]);
                    output_append_text(node->output, temp1);
                    free(temp1);
                    signal_emit_names_add(&installed, handler_names.items[j]);
                }
                signal_emit_names_free(&handler_names);
                handler = ast_nsib(handler);
                handler_index++;
            }

            /* Register the branches only after every saved handler is on the
             * stack.  The VM records this common top as the block boundary, so
             * a runtime branch can discard nested registrations without also
             * discarding another name owned by this SIGNAL_BLOCK. */
            handler = child2;
            handler_index = 0;
            while (handler) {
                signal_emit_names handler_names = {0};
                ASTNode *binding = signal_handler_binding_target(handler);

                signal_emit_collect_names(handler->child, &handler_names);
                for (j = 0; j < (int)handler_names.count; j++) {
                    int binding_register = binding ? binding->register_num : UNSET_REGISTER;
                    char binding_register_type = binding ? binding->register_type : 'r';

                    if (binding && binding->symbolNode && binding->symbolNode->symbol) {
                        binding_register = binding->symbolNode->symbol->register_num;
                        binding_register_type = binding->symbolNode->symbol->register_type;
                    }

                    if (binding) {
                        temp1 = mprintf("   sigbrv l%dsignalhandler%zu,%c%d,\"%s\"\n",
                                        node->node_number,
                                        handler_index,
                                        binding_register_type,
                                        binding_register,
                                        handler_names.items[j]);
                    } else {
                        temp1 = mprintf("   sigbr l%dsignalhandler%zu,\"%s\"\n",
                                        node->node_number,
                                        handler_index,
                                        handler_names.items[j]);
                    }
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                signal_emit_names_free(&handler_names);
                handler = ast_nsib(handler);
                handler_index++;
            }

            if (child1 && child1->output) output_concat(node->output, child1->output);
            if (child1 && child1->cleanup) output_concat(node->output, child1->cleanup);

            signal_emit_pop_names(node->output, &installed);
            temp1 = mprintf("   br l%dsignalend\n", node->node_number);
            output_append_text(node->output, temp1);
            free(temp1);

            handler = child2;
            handler_index = 0;
            while (handler) {
                ASTNode *handler_body = ast_chdn(handler, 2);

                temp1 = mprintf("l%dsignalhandler%zu:\n", node->node_number, handler_index);
                output_append_text(node->output, temp1);
                free(temp1);

                signal_emit_pop_names(node->output, &installed);
                flow_emit_descendant_unlinks(node->output, child1);
                flow_emit_descendant_lifetimes(node->output, child1);
                flow_emit_descendant_metadata_clear(node->output, child1);
                if (handler_body && handler_body->output) output_concat(node->output, handler_body->output);
                if (handler_body && handler_body->cleanup) output_concat(node->output, handler_body->cleanup);

                temp1 = mprintf("   br l%dsignalend\n", node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);

                handler = ast_nsib(handler);
                handler_index++;
            }

            temp1 = mprintf("l%dsignalend:\n", node->node_number);
            output_append_text(node->output, temp1);
            free(temp1);
            if (flow_scope_owns_cleanup(node)) {
                flow_emit_scope_dereference_unlinks(node->output, node->scope);
                emit_scope_reference_lifetimes(node->output, node);
                clear_scope_variable_metadata(node->output, node);
            }
            signal_emit_names_free(&installed);
            break;
        }

        case NOP:
            if (!node->output) node->output = output_f();
            break;

        case SAY:
            /* Add source metadata */
            comment_meta = get_metaline(node);
            trace_step_id = trace_source_step_id_from_metaline(comment_meta);
            trace_clause_id = trace_clause_id_from_metaline(comment_meta);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            if (child1->register_num == DONT_ASSIGN_REGISTER) {
                /* If the register is not set then the child is a constant
                 * which we SAY directly. Get the constant string - target type */
                temp2 = format_constant(child1->target_type, child1);
                temp1 = mprintf("   say %s\n", temp2);
                free(temp2);
            }
            else {
                output_concat(node->output, child1->output);
                output_apply_trace_source_ids(node->output, trace_step_id, trace_clause_id);
                temp1 = mprintf("   say %c%d\n",
                                child1->register_type,
                                child1->register_num);
            }
            output_append_text(node->output, temp1);
            free(temp1);

            /* Cleanup child */
            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            break;

        case RETURN:
            /* Add source metadata */
            comment_meta = get_metaline(node);
            trace_step_id = trace_source_step_id_from_metaline(comment_meta);
            trace_clause_id = trace_clause_id_from_metaline(comment_meta);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            if (child1 == 0) {
                temp1 = mprintf("   ret\n");
            }
            else if (child1->register_num == DONT_ASSIGN_REGISTER) {
                /* If the register is not set then the child is a constant
                 * which we RET directly. Get the constant string - target type */
                temp2 = format_constant(child1->target_type, child1);
                temp1 = mprintf("   ret %s\n", temp2);
                free(temp2);
            }
            else {
                output_concat(node->output, child1->output);
                output_apply_trace_source_ids(node->output, trace_step_id, trace_clause_id);
                temp1 = mprintf("   ret %c%d\n",
                                child1->register_type,
                                child1->register_num);
                // TODO - Test array element as we have not unlinked
            }
            signal_emit_unwind_for_control(node->output, node, ast_proc(node));
            output_append_text(node->output, temp1);
            free(temp1);
            break;

        case IF:
            /* Add source metadata */
            comment_meta = get_metaline_range(node, child1);
            trace_step_id = trace_source_step_id_from_metaline(comment_meta);
            trace_clause_id = trace_clause_id_from_metaline(comment_meta);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            if (child1->output) output_concat(node->output, child1->output);
            output_apply_trace_source_ids(node->output, trace_step_id, trace_clause_id);
            comment_meta = get_metaline_token_after(child1);
            temp1 = mprintf("   brf l%diffalse,%c%d\n%s",
                            node->node_number,
                            node->register_type,
                            node->register_num,
                            comment_meta);
            output_append_text(node->output, temp1);
            free(temp1);
            free(comment_meta);
            output_concat(node->output, child2->output);
            if (child3) {
                comment_meta = get_metaline_token_after(child2);
                temp1 = mprintf("   br l%difend\n%sl%diffalse:\n",
                                node->node_number,
                                comment_meta,
                                node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                free(comment_meta);
                output_concat(node->output, child3->output);

                temp1 = mprintf("l%difend:\n",
                                node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
                if (child3->cleanup) output_concat(node->output, child3->cleanup);
            }
            else {
                temp1 = mprintf("l%diffalse:\n",
                                node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
            }
            break;

        case OPT_DISPATCH:
            emit_dispatch(node);
            break;

        case DO: /* DO LOOP */
            /* Loop Assignments REPEAT->output */

            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */

            /* child1 is the REPEAT node, child2 is the INSTRUCTIONS */

            comment_meta = get_metaline_token_at(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Init */
            output_concat(node->output, child1->output);

            /* Loop Start */
            temp1 = mprintf("l%ddostart:\n",
                            node->node_number);
            output_append_text(node->output, temp1);
            free(temp1);

            /* Loop Begin Checks REPEAT->loopstartchecks */
            output_concat(node->output, child1->loopstartchecks);

            /* Loop Body - instructions */
            output_concat(node->output, child2->output);

            /* Loop End Checks REPEAT->loopendchecks */
            temp1 = mprintf("l%ddoinc:\n",
                            node->node_number);
            output_append_text(node->output, temp1);
            free(temp1);
            output_concat(node->output, child1->loopendchecks);

            /* Loop increments REPEAT->loopinc */
            output_concat(node->output, child1->loopinc);

            /* Loop End */
            comment_meta = get_metaline_token_after(child2);

            output_append_text(node->output, comment_meta);
            temp1 = mprintf("   br l%ddostart\nl%ddoend:\n",
                            node->node_number, node->node_number);
            output_append_text(node->output, temp1);
            if (child1->cleanup) {
                /* Keep a detached copy for SIGNAL handlers emitted after this
                 * loop has already been joined into its parent's output chain. */
                if (!node->branch_cleanup) node->branch_cleanup = output_f();
                flow_append_output_copy(node->branch_cleanup, child1->cleanup);
                output_concat(node->output, child1->cleanup);
            }
            if (flow_scope_owns_cleanup(node)) {
                flow_emit_scope_dereference_unlinks(node->output, node->scope);
                emit_scope_reference_lifetimes(node->output, node);
                clear_scope_variable_metadata(node->output, node);
            }
            free(temp1);
            free(comment_meta);
            break;

        case REPEAT:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */
            if (!node->output) node->output = output_f(); /* Assign / init instruction */
            node->loopstartchecks = output_f(); /* Begin Loop exit checks */
            node->loopinc = output_f(); /* Loop increments */
            node->loopendchecks = output_f(); /* End Loop exit checks */
            while (child1) {
                if (child1->node_type == ASSIGN) {
                    /* Only output is valid - does not follow convention */
                    if (child1->output) output_concat(node->output, child1->output);
                }
                else {
                    if (child1->output)
                        output_concat(node->output, child1->output);
                    if (child1->loopstartchecks)
                        output_concat(node->loopstartchecks, child1->loopstartchecks);
                    if (child1->loopinc)
                        output_concat(node->loopinc, child1->loopinc);
                    if (child1->loopendchecks)
                        output_concat(node->loopendchecks, child1->loopendchecks);
                }
                child1 = child1->sibling;
            }
            /* Output Cleanups */
            child1 = node->child;
            while (child1) {
                if (child1->cleanup) {
                    if (!node->cleanup) node->cleanup = output_f();
                    output_concat(node->cleanup, child1->cleanup);
                }
                child1 = child1->sibling;
            }

            break;

        case FOR:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            output_concat(node->output, child1->output);
            if (child1->register_num != node->register_num ||
                child1->register_type != node->register_type) {
                temp1 = mprintf("   icopy %c%d,%c%d\n",
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
            }
            node->loopstartchecks = output_fs(comment_meta);
            temp1 = mprintf("   bcf l%ddoend,%c%d\n",
                            node->parent->parent->node_number,
                            node->register_type,
                            node->register_num);
            output_append_text(node->loopstartchecks, temp1);
            if (child1->cleanup) {
                if (!node->cleanup) node->cleanup = output_f();
                output_concat(node->cleanup, child1->cleanup);
            }
            free(comment_meta);
            free(temp1);
            break;

        case TO:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */

            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            output_concat(node->output, child1->output);

            /* Need to determine the sign of the BY */
            /* Find the BY */
            j = 1; /* J is the sign: 1 (default)=positive, -1=negative, 0=dynamic */
            n = node->parent->child; /* First sibling */
            while (n) {
                if (n->node_type == BY) {
                    if (ast_chdn(n, 0)) {
                        if (is_constant(ast_chdn(n, 0))) {
                            if (ast_chdn(n, 0)->value_type == ast_chdn(n, 0)->target_type) {
                                /* Is a constant */
                                if (ast_chdn(n, 0)->value_type == TP_INTEGER) {
                                    if (ast_chdn(n, 0)->int_value >= 0) j = 1;
                                    else j = -1;
                                }
                                else if (ast_chdn(n, 0)->value_type == TP_FLOAT) {
                                    if (ast_chdn(n, 0)->float_value >= 0.0) j = 1;
                                    else j = -1;
                                }
                                else if (ast_chdn(n, 0)->value_type == TP_DECIMAL) {
                                    if (ast_chdn(n, 0)->decimal_value[0] == '-') j = -1;
                                    else j = 1;
                                }
                                else j = 1;
                            }
                            else j = 0; /* Not a constant */
                        }
                        else if (payload->context->optimise &&
                                 ast_chdn(n, 0)->value_type == TP_DECIMAL &&
                                 (constant_decimal = flow_readonly_decimal_literal(ast_chdn(n, 0)))) {
                            /* Preserve the prepared register operand while
                             * recovering only the immutable step-sign fact. */
                            if (constant_decimal[0] == '-') j = -1;
                            else j = 1;
                        }
                        else j = 0; /* Not a constant */
                    }
                    else j = 1; /* Implicit by */
                    break;
                }
                n = n->sibling;
            }
            /* n is set by the BY node */

            /* If the REPEAT has a TO it has an ASSIGN and its register
             * number will have been set to the ASSIGN Variable */
            node->loopstartchecks = output_fs(comment_meta);
            switch (j) {
                case 1: /* Positive */
                    temp1 = mprintf("   %sgt r%d,%c%d,%c%d\n   brt l%ddoend,r%d\n",
                                    tp_prefix,
                                    node->additional_registers,
                                    node->parent->register_type,
                                    node->parent->register_num,
                                    child1->register_type,
                                    child1->register_num,
                                    node->parent->parent->node_number,
                                    node->additional_registers);
                    break;
                case -1: /* Negative */
                    temp1 = mprintf("   %slt r%d,%c%d,%c%d\n   brt l%ddoend,r%d\n",
                                    tp_prefix,
                                    node->additional_registers,
                                    node->parent->register_type,
                                    node->parent->register_num,
                                    child1->register_type,
                                    child1->register_num,
                                    node->parent->parent->node_number,
                                    node->additional_registers);
                    break;
                default: /* Dynamic by value */
                    /* We need a zero (int or flaot */
                    if (*tp_prefix == 'i') op = "0";
                    else op = "0.0";
                    temp1 = mprintf(
                            "   %slt r%d,%c%d,%s\n" /* Check the by value sign */
                            "   brt l%ddoneg1,r%d\n"    /* JMP to Negative BY */

                            "   %sgt r%d,%c%d,%c%d\n"   /* Pos BY */
                            "   brtf l%ddoend,l%ddoneg2,r%d\n"

                            "l%ddoneg1:\n"
                            "   %slt r%d,%c%d,%c%d\n"   /* Neg BY */
                            "   brt l%ddoend,r%d\n"

                            "l%ddoneg2:\n",

                            tp_prefix,
                            node->additional_registers,
                            ast_chdn(n, 0)->register_type,
                            ast_chdn(n, 0)->register_num,
                            op,
                            node->parent->parent->node_number,
                            node->additional_registers,

                            tp_prefix,
                            node->additional_registers,
                            node->parent->register_type,
                            node->parent->register_num,
                            child1->register_type,
                            child1->register_num,
                            node->parent->parent->node_number,
                            node->parent->parent->node_number,
                            node->additional_registers,

                            node->parent->parent->node_number,
                            tp_prefix,
                            node->additional_registers,
                            node->parent->register_type,
                            node->parent->register_num,
                            child1->register_type,
                            child1->register_num,
                            node->parent->parent->node_number,
                            node->additional_registers,

                            node->parent->parent->node_number);
            }
            output_append_text(node->loopstartchecks, temp1);
            free(temp1);
            free(comment_meta);
            if (child1->cleanup) {
                if (!node->cleanup) node->cleanup = output_f();
                output_concat(node->cleanup, child1->cleanup);
            }
            break;

        case BY:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */

            /* If the REPEAT has a BY it has an ASSIGN and its register
             * number will have been set to the ASSIGN Variable */

            if (child1) {
                /* BY explicitly stated */
                comment_meta = get_metaline(node);
                if (node->output) output_prepend_text(comment_meta, node->output);
                else node->output = output_fs(comment_meta);
                output_concat(node->output, child1->output);

                node->loopinc = output_fs(comment_meta);
                temp1 = mprintf(" %sadd %c%d,%c%d,%c%d\n",
                                tp_prefix,
                                node->parent->register_type,
                                node->parent->register_num,
                                node->child->register_type,
                                node->child->register_num,
                                node->parent->register_type,
                                node->parent->register_num);
                output_append_text(node->loopinc, temp1);
                free(comment_meta);
                free(temp1);
                if (child1->cleanup) {
                    if (!node->cleanup) node->cleanup = output_f();
                    output_concat(node->cleanup, child1->cleanup);
                }
            }
            else {
                /* BY Added implicitly - increment by 1 */
                /* For the source we can only reference the symbol in the loop assignment node */
//                    comment_meta = get_comment_line_number_only(node->parent, "{Implicit \"BY 1\"}");
                comment_meta = get_metaline_token_at(node->parent->child);
                node->loopinc = output_fs(comment_meta);
                free(comment_meta);

                if (*tp_prefix == 'i') {
                    temp1 = mprintf("   inc %c%d\n",
                                    node->parent->register_type,
                                    node->parent->register_num);
                }
                else {
                    temp1 = mprintf("   %sadd %c%d,%c%d,1.0\n",
                                    tp_prefix,
                                    node->parent->register_type,
                                    node->parent->register_num,
                                    node->parent->register_type,
                                    node->parent->register_num);
                }
                output_append_text(node->loopinc, temp1);
                free(temp1);
            }
            break;

        case WHILE:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */
            comment_meta = get_metaline(node);
            node->loopstartchecks = output_fs(comment_meta);
            free(comment_meta);
            output_concat(node->loopstartchecks, child1->output);
            temp1 = mprintf("   brf l%ddoend,%c%d\n",
                            node->parent->parent->node_number,
                            node->register_type,
                            node->register_num);
            output_append_text(node->loopstartchecks, temp1);
            free(temp1);
            if (child1->cleanup) {
                if (!node->cleanup) node->cleanup = output_f();
                output_concat(node->cleanup, child1->cleanup);
            }
            break;

        case UNTIL:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */
            comment_meta = get_metaline(node);
            node->loopendchecks = output_fs(comment_meta);
            free(comment_meta);
            output_concat(node->loopendchecks, child1->output);
            temp1 = mprintf("   brt l%ddoend,%c%d\n",
                            node->parent->parent->node_number,
                            node->register_type,
                            node->register_num);
            output_append_text(node->loopendchecks, temp1);
            free(temp1);
            if (child1->cleanup) {
                if (!node->cleanup) node->cleanup = output_f();
                output_concat(node->cleanup, child1->cleanup);
            }
            break;

        case LEAVE:
            /* Leave Loop */
            /* Add source metadata */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            flow_emit_crossed_cleanups(node->output, node,
                                       node->association);
            signal_emit_unwind_for_control(node->output, node,
                                           node->association);

            temp1 = mprintf("   br l%ddoend\n",
                            node->association->node_number);
            output_append_text(node->output, temp1);
            free(temp1);
            break;

        case LEAVE_WITH:
            /* Leave BLOCK_EXPR with value */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            if (child1) {
                char *result_prefix;

                if (child1->output) output_concat(node->output, child1->output);

                if (node->association) {
                    if (node->association->value_dims) result_prefix = "";
                    else result_prefix = type_to_prefix(node->association->value_type);

                    if (child1->register_num == DONT_ASSIGN_REGISTER) {
                        temp2 = format_constant(child1->target_type, child1);
                        temp1 = mprintf("   load %c%d,%s\n",
                                        node->association->register_type,
                                        node->association->register_num,
                                        temp2);
                        output_append_text(node->output, temp1);
                        free(temp1);
                        free(temp2);
                    }
                    else if (child1->register_num != node->association->register_num ||
                             child1->register_type != node->association->register_type) {
                        temp1 = mprintf("   %scopy %c%d,%c%d\n",
                                        result_prefix,
                                        node->association->register_type,
                                        node->association->register_num,
                                        child1->register_type,
                                        child1->register_num);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }
                }

                if (child1->cleanup) output_concat(node->output, child1->cleanup);
            }

            if (node->association) {
                flow_emit_crossed_cleanups(node->output, node,
                                           node->association);
                signal_emit_unwind_for_control(node->output, node,
                                               node->association);
                if (!leave_with_falls_through_to_block_end(node)) {
                    temp1 = mprintf("   br l%dbexprend\n",
                                    node->association->node_number);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
            }
            break;

        case ITERATE:
            /* Iterate Loop */
            /* Add source metadata */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            flow_emit_crossed_cleanups(node->output, node,
                                       node->association);
            temp1 = mprintf("   br l%ddoinc\n",
                            node->association->node_number);
            output_append_text(node->output, temp1);
            free(temp1);
            break;

        default:
            break;
    }
}
