/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "rxcp_val.h"
#include "rxcp_ast.h"
#include "rxcpbgmr.h"
#include "rxcpdary.h"
#include "rxcp_sym.h"
#include "rxcp_source_tree.h"

/*
 * Production inlining size policy.  This is a profitability/metadata-size
 * cutoff, not a semantic safety boundary; semantic gates below still decide
 * whether a body is safe to inline.
 */
#define INLINE_MAX_NODES 300
#define INLINE_META_MAX_SOURCE_SPAN 512

typedef struct {
    Symbol *old_symbol;
    Symbol *new_symbol;
} InlineSymbolMapEntry;

typedef struct {
    Scope *old_scope;
    Scope *new_scope;
} InlineScopeMapEntry;

typedef struct {
    ASTNode *old_node;
    ASTNode *new_node;
} InlineNodeMapEntry;

typedef struct {
    Symbol *formal_symbol;
    ASTNode *actual_source;
    Symbol **captured_symbols;
    size_t captured_count;
} InlineRefActualEntry;

typedef struct {
    Scope *callee_scope;
    Scope *inline_scope;
    InlineSymbolMapEntry *symbol_entries;
    size_t symbol_count;
    InlineScopeMapEntry *scope_entries;
    size_t scope_count;
    InlineNodeMapEntry *node_entries;
    size_t node_count;
    InlineRefActualEntry *ref_entries;
    size_t ref_count;
    InlineRefActualEntry *varg_ref_entries;
    Symbol **varg_symbols;
    size_t varg_count;
    Symbol *varg_array_symbol;
    Symbol *method_receiver_source_symbol;
    Symbol *method_receiver_local_symbol;
    int method_receiver_needs_copyback;
} InlineCloneState;

typedef struct {
    ASTNode *root_proc;
    Context *context;
    int node_count;
    int return_count;
    int has_unsupported_varg_access;
    int has_unsupported_assembler_alias;
    int has_unsupported_assembler_effect;
    int has_unsupported_reference;
    int has_class_attribute_write;
    int has_unportable_class_attribute_shape;
    size_t max_required_varg_index;
    int ref_varg_mode;
} InlinableCheck;

typedef struct {
    ASTNode *return_target;
    Symbol *return_sink_symbol;
} InlineReturnPlan;

typedef struct {
    int return_count;
    int top_level_return_count;
    int final_is_return;
} InlineReturnShape;

typedef enum {
    INLINE_EXPR_CONTEXT_NONE = 0,
    INLINE_EXPR_CONTEXT_EAGER_VALUE_CONSUMER,
    INLINE_EXPR_CONTEXT_EAGER_OPERATOR,
    INLINE_EXPR_CONTEXT_EAGER_CALL_ARGUMENT,
    INLINE_EXPR_CONTEXT_SHORT_CIRCUIT_OPERATOR,
    INLINE_EXPR_CONTEXT_CONTROL_CONSUMER
} InlineExprContext;

typedef struct {
    Context *context;
    int changed;
} InlineWalkerPayload;

static size_t inline_count_siblings(ASTNode *node);
static ASTNode *inline_clone_subtree(Context *context, ASTNode *node, InlineCloneState *state);
static ASTNode *inline_clone_subtree_in_scope(Context *context,
                                              ASTNode *node,
                                              InlineCloneState *state,
                                              Scope *current_scope);
static ASTNode *inline_create_symbol_node(Context *context,
                                          Scope *scope,
                                          ASTNode *source_node,
                                          Symbol *symbol,
                                          NodeType node_type,
                                          unsigned int read_usage,
                                          unsigned int write_usage);
static ASTNode *inline_create_integer_constant(Context *context, ASTNode *source_node, int value, ValueType type);
static void inline_copy_numeric_context(Scope *target, const Scope *source);
static Symbol *inline_create_temp_symbol(Context *context,
                                         Scope *inline_scope,
                                         ASTNode *source_node,
                                         const char *prefix,
                                         size_t suffix);
static ASTNode *inline_create_temp_value_ref(Context *context,
                                             ASTNode *instr_list,
                                             Scope *inline_scope,
                                             ASTNode *source_node,
                                             InlineCloneState *state,
                                             const char *prefix,
                                             size_t suffix);
static int inline_capture_scoped_call_actuals(Context *context,
                                              ASTNode *instr_list,
                                              Scope *inline_scope,
                                              ASTNode *proc_def,
                                              ASTNode *call_node,
                                              InlineCloneState *clone_state,
                                              Symbol **captured_receiver_out,
                                              Symbol ***captured_symbols_out,
                                              size_t *captured_count_out);
static int inline_capture_varg_captured_actuals(Context *context,
                                                ASTNode *instr_list,
                                                Scope *inline_scope,
                                                ASTNode *varg_arg,
                                                ASTNode *actual_arg,
                                                InlineCloneState *state,
                                                Symbol **captured_symbols,
                                                size_t captured_count,
                                                size_t first_actual_index);
static int inline_initialise_varg_array(Context *context,
                                        ASTNode *instr_list,
                                        Scope *inline_scope,
                                        ASTNode *varg_arg,
                                        ASTNode *source_node,
                                        InlineCloneState *state);
static ASTNode *inline_find_varg_arg(ASTNode *proc_def);
static int inline_node_is_callable_def(ASTNode *node);
static int inline_node_is_inlineable_call(ASTNode *node, Symbol **proc_sym_out);
static ASTNode *inline_call_first_user_actual(ASTNode *call_node);
static ASTNode *inline_call_receiver(ASTNode *call_node);
static int inline_call_arity_matches(ASTNode *call_node, Symbol *proc_sym, size_t *varg_count_out);
static int inline_analyse_varg_usage(ASTNode *proc_def, int *unsupported_out, size_t *max_required_index_out);
static int inline_call_is_recursive(ASTNode *call_node, Symbol *proc_sym);
static int inline_analyse_return_shape(ASTNode *proc_def, InlineReturnShape *shape_out);
static int inline_method_writes_class_attribute(ASTNode *proc_def);
static int inline_symbol_writes_class_attribute(Symbol *symbol);
static int inline_subtree_reads_class_attribute(ASTNode *node);
static int inline_sibling_list_reads_class_attribute(ASTNode *node);
static int inline_assembler_has_unsupported_aliasing(ASTNode *node);
static int inline_assembler_has_unsupported_effect(ASTNode *node);
static int inline_proc_has_procedure_expose(ASTNode *node);
static int inline_count_return_nodes(ASTNode *node);
static int inline_rewrite_return_nodes(Context *context,
                                       ASTNode **node_ref,
                                       ASTNode *block_expr,
                                       Scope *inline_scope,
                                       int allow_dummy_return,
                                       ValueType proc_type,
                                       InlineCloneState *clone_state);

static void inline_debug_log(Context *context,
                             ASTNode *site,
                             Symbol *proc_sym,
                             const char *prefix,
                             const char *format,
                             ...) {
    Context *root;
    va_list args;

    root = context && context->master_context ? context->master_context : context;
    if (!root || root->debug_mode < 1 || !prefix || !format) return;

    fprintf(stderr, "%s", prefix);
    if (proc_sym && proc_sym->name) fprintf(stderr, " %s", proc_sym->name);
    if (site && site->file_name) {
        fprintf(stderr, " @ %s", site->file_name);
        if (site->line > 0) fprintf(stderr, ":%d", site->line);
        if (site->column > 0) fprintf(stderr, ":%d", site->column);
    }
    fprintf(stderr, " - ");

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

static void inline_debug_fail_closed(Context *context,
                                     ASTNode *site,
                                     Symbol *proc_sym,
                                     const char *format,
                                     ...) {
    Context *root;
    va_list args;

    root = context && context->master_context ? context->master_context : context;
    if (!root || root->debug_mode < 1 || !format) return;

    fprintf(stderr, "DEBUG_INLINE_FAILCLOSED");
    if (proc_sym && proc_sym->name) fprintf(stderr, " %s", proc_sym->name);
    if (site && site->file_name) {
        fprintf(stderr, " @ %s", site->file_name);
        if (site->line > 0) fprintf(stderr, ":%d", site->line);
        if (site->column > 0) fprintf(stderr, ":%d", site->column);
    }
    fprintf(stderr, " - ");

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

static void inline_export_debug_reject(Context *context,
                                       ASTNode *site,
                                       Symbol *proc_sym,
                                       const char *format,
                                       ...) {
    Context *root;
    va_list args;

    root = context && context->master_context ? context->master_context : context;
    if (!root || root->debug_mode < 2 || !format) return;

    fprintf(stderr, "DEBUG_INLINE_EXPORT");
    if (proc_sym && proc_sym->name) fprintf(stderr, " %s", proc_sym->name);
    if (site && site->file_name) {
        fprintf(stderr, " @ %s", site->file_name);
        if (site->line > 0) fprintf(stderr, ":%d", site->line);
        if (site->column > 0) fprintf(stderr, ":%d", site->column);
    }
    fprintf(stderr, " - reject: ");

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

static Symbol *inline_find_mapped_symbol(InlineCloneState *state, Symbol *old_symbol) {
    size_t i;

    if (!state || !old_symbol) return NULL;

    for (i = 0; i < state->symbol_count; i++) {
        if (state->symbol_entries[i].old_symbol == old_symbol) return state->symbol_entries[i].new_symbol;
    }

    return NULL;
}

static Scope *inline_find_mapped_scope(InlineCloneState *state, Scope *old_scope) {
    size_t i;

    if (!state || !old_scope) return NULL;

    for (i = 0; i < state->scope_count; i++) {
        if (state->scope_entries[i].old_scope == old_scope) return state->scope_entries[i].new_scope;
    }

    return NULL;
}

static ASTNode *inline_find_mapped_node(InlineCloneState *state, ASTNode *old_node) {
    size_t i;

    if (!state || !old_node) return NULL;

    for (i = 0; i < state->node_count; i++) {
        if (state->node_entries[i].old_node == old_node) return state->node_entries[i].new_node;
    }

    return NULL;
}

static int inline_append_symbol_map_entry(InlineCloneState *state, Symbol *old_symbol, Symbol *new_symbol) {
    InlineSymbolMapEntry *new_entries;

    if (!state || !old_symbol || !new_symbol) return 0;

    new_entries = realloc(state->symbol_entries,
                          sizeof(InlineSymbolMapEntry) * (state->symbol_count + 1));
    if (!new_entries) return 0;

    state->symbol_entries = new_entries;
    state->symbol_entries[state->symbol_count].old_symbol = old_symbol;
    state->symbol_entries[state->symbol_count].new_symbol = new_symbol;
    state->symbol_count++;
    return 1;
}

static ASTNode *inline_formal_target(ASTNode *param_arg) {
    return param_arg ? ast_chdn(param_arg, 0) : NULL;
}

static void inline_copy_numeric_context(Scope *target, const Scope *source) {
    if (!target || !source) return;

    target->num_context.digits = source->num_context.digits;
    target->num_context.fuzz = source->num_context.fuzz;
    target->num_context.form = source->num_context.form;
    target->num_context.casetype = source->num_context.casetype;
    target->num_context.standard = source->num_context.standard;
}

static int inline_node_is_callable_def(ASTNode *node) {
    return node &&
           (node->node_type == PROCEDURE ||
            node->node_type == METHOD ||
            node->node_type == FACTORY);
}

static int inline_symbol_has_callable_template(Symbol *symbol) {
    return symbol &&
           symbol->ast_template &&
           inline_node_is_callable_def(symbol->ast_template);
}

static int inline_symbol_uses_imported_template(Symbol *symbol) {
    ASTNode *def_node;

    if (!symbol || !symbol->ast_template) return 0;
    def_node = sym_proc(symbol);
    return def_node && symbol->ast_template != def_node;
}

static int inline_node_is_inlineable_call(ASTNode *node, Symbol **proc_sym_out) {
    Symbol *proc_sym;

    if (proc_sym_out) *proc_sym_out = NULL;
    if (!node ||
        (node->node_type != FUNCTION &&
         node->node_type != MEMBER_CALL &&
         node->node_type != FACTORY_CALL)) {
        return 0;
    }

    proc_sym = node->symbolNode ? node->symbolNode->symbol : NULL;
    if (!proc_sym || !proc_sym->is_inlinable || !inline_symbol_has_callable_template(proc_sym)) return 0;

    if (proc_sym_out) *proc_sym_out = proc_sym;
    return 1;
}

static ASTNode *inline_call_receiver(ASTNode *call_node) {
    if (!call_node || call_node->node_type != MEMBER_CALL) return NULL;
    return call_node->child;
}

static ASTNode *inline_call_first_user_actual(ASTNode *call_node) {
    if (!call_node) return NULL;
    if (call_node->node_type == MEMBER_CALL) {
        return call_node->child ? call_node->child->sibling : NULL;
    }
    return call_node->child;
}

static ASTNode *inline_formal_default(ASTNode *param_arg) {
    ASTNode *formal_target;

    formal_target = inline_formal_target(param_arg);
    return formal_target ? formal_target->sibling : NULL;
}

static int inline_is_missing_actual(ASTNode *actual_arg) {
    return actual_arg && actual_arg->node_type == NOVAL;
}

static int inline_is_supported_ref_actual(ASTNode *actual_arg) {
    if (!actual_arg) return 0;
    if (!actual_arg->symbolNode || !actual_arg->symbolNode->symbol) return 0;
    if (actual_arg->node_type != VAR_SYMBOL &&
        actual_arg->node_type != VAR_TARGET &&
        actual_arg->node_type != VAR_REFERENCE) return 0;

    return 1;
}

static InlineRefActualEntry *inline_find_ref_actual(InlineCloneState *state, Symbol *formal_symbol) {
    size_t i;

    if (!state || !formal_symbol) return NULL;

    for (i = 0; i < state->ref_count; i++) {
        if (state->ref_entries[i].formal_symbol == formal_symbol) return &state->ref_entries[i];
    }

    return NULL;
}

static InlineRefActualEntry *inline_find_ref_varg_actual(InlineCloneState *state, size_t index) {
    if (!state || !state->varg_ref_entries) return NULL;
    if (index < 1 || index > state->varg_count) return NULL;

    return &state->varg_ref_entries[index - 1];
}

static int inline_copy_node_shape(Symbol *target, ASTNode *source) {
    if (!target || !source) return 0;

    target->type = source->value_type;
    target->value_dims = source->value_dims;

    if (source->value_dims && source->value_dim_base) {
        target->dim_base = malloc(sizeof(int) * source->value_dims);
        if (!target->dim_base) return 0;
        memcpy(target->dim_base, source->value_dim_base, sizeof(int) * source->value_dims);
    } else {
        target->dim_base = NULL;
    }

    if (source->value_dims && source->value_dim_elements) {
        target->dim_elements = malloc(sizeof(int) * source->value_dims);
        if (!target->dim_elements) return 0;
        memcpy(target->dim_elements, source->value_dim_elements, sizeof(int) * source->value_dims);
    } else {
        target->dim_elements = NULL;
    }

    if (source->value_class) {
        target->value_class = strdup(source->value_class);
        if (!target->value_class) return 0;
    } else {
        target->value_class = NULL;
    }

    return 1;
}

static void inline_copy_replacement_semantics(ASTNode *replacement, ASTNode *replaced_node) {
    if (!replacement || !replaced_node) return;

    replacement->is_ref_arg = replaced_node->is_ref_arg;
    replacement->is_opt_arg = replaced_node->is_opt_arg;
    replacement->is_const_arg = replaced_node->is_const_arg;
    replacement->is_varg = replaced_node->is_varg;
    replacement->inherit_parent_reg_scope = replaced_node->inherit_parent_reg_scope;
    if (replacement->is_ref_arg && replacement->symbolNode) replacement->symbolNode->writeUsage = 1;

    ast_set_value_type(0,
                       replacement,
                       replaced_node->value_type,
                       replaced_node->value_dims,
                       replaced_node->value_dim_base,
                       replaced_node->value_dim_elements,
                       replaced_node->value_class);
    ast_set_target_type(0,
                        replacement,
                        replaced_node->target_type,
                        replaced_node->target_dims,
                        replaced_node->target_dim_base,
                        replaced_node->target_dim_elements,
                        replaced_node->target_class);
}

static int inline_node_has_array_shape(ASTNode *node) {
    if (!node) return 0;
    return node->value_dims > 0 || node->target_dims > 0;
}

static int inline_node_requires_local_scope(ASTNode *node) {
    if (!node) return 0;

    if (node->node_type == BLOCK_EXPR || node->node_type == DO) return 1;
    if (node->node_type != INSTRUCTIONS) return 0;
    if (node->force_local_scope) return 1;

    return node->parent &&
           (node->parent->node_type == IF || node->parent->node_type == INSTRUCTIONS);
}

static int inline_node_is_plain_object(ASTNode *node) {
    if (!node) return 0;
    return node->value_type == TP_OBJECT &&
           node->target_type == TP_OBJECT &&
           !inline_node_has_array_shape(node);
}

static int inline_is_direct_symbol_actual(ASTNode *node) {
    if (!node || !node->symbolNode || !node->symbolNode->symbol || node->child) return 0;

    return node->node_type == VAR_SYMBOL ||
           node->node_type == VAR_TARGET ||
           node->node_type == VAR_REFERENCE;
}

static int inline_symbol_is_class_attribute(Symbol *symbol) {
    return symbol &&
           symbol->symbol_type == VARIABLE_SYMBOL &&
           symbol->scope &&
           (symbol->scope->type == SCOPE_CLASS ||
            (symbol->scope->defining_node &&
             symbol->scope->defining_node->node_type == CLASS_DEF));
}

static Symbol *inline_resolve_class_symbol(Context *context, Scope *scope, const char *class_name) {
    Symbol *class_symbol;
    const char *lookup_name;
    Scope *namespace_scope;
    char *fq_name;

    if (!context || !context->ast || !class_name || !*class_name) return NULL;

    lookup_name = class_name;
    while (*lookup_name == '.') lookup_name++;

    class_symbol = sym_rfqn(context->ast, lookup_name);
    if (class_symbol) return class_symbol;

    if (strchr(lookup_name, '.')) return NULL;

    namespace_scope = scope;
    while (namespace_scope && namespace_scope->type != SCOPE_NAMESPACE) {
        namespace_scope = namespace_scope->parent;
    }
    if (!namespace_scope || !namespace_scope->name || !*namespace_scope->name) return NULL;

    fq_name = mprintf("%s.%s", namespace_scope->name, lookup_name);
    if (!fq_name) return NULL;
    class_symbol = sym_rfqn(context->ast, fq_name);
    free(fq_name);
    return class_symbol;
}

static int inline_class_has_reference_attribute(Context *context, Scope *scope, const char *class_name) {
    Symbol *class_symbol;
    Symbol **symbols;
    size_t i;
    int result = 0;

    class_symbol = inline_resolve_class_symbol(context, scope, class_name);
    if (!class_symbol || !class_symbol->defines_scope) return 0;

    symbols = scp_syms(class_symbol->defines_scope);
    if (!symbols) return 0;

    for (i = 0; symbols[i]; i++) {
        if (inline_symbol_is_class_attribute(symbols[i]) &&
            symbols[i]->type == TP_REFERENCE) {
            result = 1;
            break;
        }
    }

    free(symbols);
    return result;
}

static int inline_class_attribute_shape_is_portable(Symbol *symbol) {
    if (!inline_symbol_is_class_attribute(symbol)) return 1;
    if (symbol->is_this || symbol->is_factory) return 1;
    if (symbol->value_dims > 0) return 0;

    switch (symbol->type) {
        case TP_INTEGER:
        case TP_BOOLEAN:
        case TP_FLOAT:
        case TP_STRING:
            return 1;
        default:
            return 0;
    }
}

static int inline_class_attribute_register_num(Symbol *symbol) {
    int i;

    if (!inline_symbol_is_class_attribute(symbol)) return symbol ? symbol->register_num : UNSET_REGISTER;

    for (i = 0; i < (int)sym_nond(symbol); i++) {
        ASTNode *node = sym_trnd(symbol, i)->node;
        ASTNode *reg_node;
        ASTNode *idx;

        if (!node || !node->parent || node->parent->node_type != DEFINE) continue;
        reg_node = ast_chld(node->parent, NODE_REGISTER, 0);
        if (!reg_node) continue;

        idx = ast_chld(reg_node, INTEGER, 0);
        if (idx) return node_to_integer(idx);
        if (reg_node->int_value) return (int)reg_node->int_value;
    }

    return symbol->register_num;
}

static int inline_is_direct_receiver_copyback_target(ASTNode *node) {
    if (!inline_is_direct_symbol_actual(node)) return 0;
    return !inline_symbol_is_class_attribute(node->symbolNode->symbol);
}

static int inline_is_direct_single_value_consumer(ASTNode *node) {
    ASTNode *parent;

    if (!node) return 0;

    parent = node->parent;
    if (!parent || parent->child != node) return 0;

    switch (parent->node_type) {
        case SAY:
        case RETURN:
        case IF:
        case WHILE:
        case UNTIL:
        case FOR:
        case TO:
        case BY:
            return 1;

        default:
            return 0;
    }
}

static int inline_parent_is_eager_operator(ASTNode *parent) {
    if (!parent) return 0;

    switch (parent->node_type) {
        case OP_ADD:
        case OP_MINUS:
        case OP_MULT:
        case OP_DIV:
        case OP_IDIV:
        case OP_MOD:
        case OP_POWER:
        case OP_CONCAT:
        case OP_SCONCAT:
        case OP_COMPARE_EQUAL:
        case OP_COMPARE_NEQ:
        case OP_COMPARE_GT:
        case OP_COMPARE_LT:
        case OP_COMPARE_GTE:
        case OP_COMPARE_LTE:
        case OP_COMPARE_S_EQ:
        case OP_COMPARE_S_NEQ:
        case OP_COMPARE_S_GT:
        case OP_COMPARE_S_LT:
        case OP_COMPARE_S_GTE:
        case OP_COMPARE_S_LTE:
        case OP_NEG:
        case OP_PLUS:
        case OP_NOT:
            return 1;

        default:
            return 0;
    }
}

static int inline_node_is_constant_literal(ASTNode *node) {
    if (!node) return 0;

    switch (node->node_type) {
        case CONSTANT:
        case CONST_SYMBOL:
        case STRING:
        case FLOAT:
        case DECIMAL:
        case BINARY:
        case INTEGER:
        case CLASS:
            return node->value_type == node->target_type;

        default:
            return 0;
    }
}

static int inline_node_is_plain_stable_value(ASTNode *node) {
    Symbol *symbol;

    if (!node || node->node_type != VAR_SYMBOL || node->child) return 0;
    if (!node->symbolNode || !node->symbolNode->symbol) return 0;

    symbol = node->symbolNode->symbol;
    if (symbol->symbol_type == FUNCTION_SYMBOL) return 0;
    return !inline_symbol_is_class_attribute(symbol);
}

static int inline_eager_operator_context_is_safe(ASTNode *node) {
    ASTNode *parent;

    if (!node) return 0;

    parent = node->parent;
    if (!inline_parent_is_eager_operator(parent)) return 0;

    if (parent->child == node) return 1;

    return inline_node_is_constant_literal(parent->child) ||
           inline_node_is_plain_stable_value(parent->child);
}

static int inline_rhs_eager_operator_needs_left_capture(ASTNode *node) {
    ASTNode *parent;
    ASTNode *left;

    if (!node) return 0;

    parent = node->parent;
    if (!inline_parent_is_eager_operator(parent)) return 0;

    left = parent->child;
    if (!left || left == node) return 0;
    if (left->sibling != node || node->sibling) return 0;

    return !inline_eager_operator_context_is_safe(node);
}

static int inline_parent_is_short_circuit_operator(ASTNode *parent) {
    return parent &&
           (parent->node_type == OP_AND ||
            parent->node_type == OP_OR);
}

static int inline_node_is_call_argument(ASTNode *node) {
    ASTNode *parent;

    if (!node || !node->parent) return 0;

    parent = node->parent;
    switch (parent->node_type) {
        case FUNCTION:
        case FACTORY_CALL:
            return 1;

        case MEMBER_CALL:
            return parent->child != node;

        default:
            return 0;
    }
}

static int inline_node_needs_attr_copy(ASTNode *node) {
    if (!node) return 0;

    if (inline_node_has_array_shape(node)) return 1;

    return node->value_type == TP_OBJECT ||
           node->value_type == TP_BINARY ||
           node->value_type == TP_REFERENCE ||
           node->target_type == TP_OBJECT ||
           node->target_type == TP_BINARY ||
           node->target_type == TP_REFERENCE;
}

static int inline_formal_needs_isolated_copy(ASTNode *formal_target, ASTNode *param_arg) {
    if (!formal_target) return 0;
    if (inline_node_has_array_shape(formal_target)) return 1;
    if (formal_target->value_type == TP_BINARY || formal_target->target_type == TP_BINARY ||
        formal_target->value_type == TP_REFERENCE || formal_target->target_type == TP_REFERENCE) return 1;

    return inline_node_is_plain_object(formal_target) && !(param_arg && param_arg->is_const_arg);
}

static ASTNode *inline_create_register_copy_instr(Context *context,
                                                  Scope *scope,
                                                  const char *opcode,
                                                  ASTNode *lhs_node,
                                                  ASTNode *rhs_node) {
    ASTNode *instr;
    ASTNode *lhs_copy;
    ASTNode *rhs_copy;
    Symbol *lhs_symbol;
    Symbol *rhs_symbol;

    if (!context || !scope || !lhs_node || !rhs_node) return NULL;
    if (!lhs_node->symbolNode || !rhs_node->symbolNode) return NULL;

    lhs_symbol = lhs_node->symbolNode->symbol;
    rhs_symbol = rhs_node->symbolNode->symbol;
    if (!lhs_symbol || !rhs_symbol) return NULL;

    if (!opcode) return NULL;

    instr = ast_ftt(context, ASSEMBLER, strdup(opcode));
    if (!instr) return NULL;

    instr->free_node_string = 1;
    instr->scope = scope;
    ast_copy_source_anchor(instr, lhs_node, AST_SOURCE_SYNTHETIC);

    lhs_copy = inline_create_symbol_node(context, scope, lhs_node, lhs_symbol, VAR_TARGET, 0, 1);
    rhs_copy = inline_create_symbol_node(context, scope, rhs_node, rhs_symbol, VAR_SYMBOL, 1, 0);
    if (!lhs_copy || !rhs_copy) return NULL;

    add_ast(instr, lhs_copy);
    add_ast(instr, rhs_copy);
    return instr;
}

static ASTNode *inline_create_integer_constant(Context *context, ASTNode *source_node, int value, ValueType type) {
    ASTNode *node;
    char buffer[32];

    if (!context || !source_node) return NULL;

    node = ast_ft(context, INTEGER);
    if (!node) return NULL;

    snprintf(buffer, sizeof(buffer), "%d", value);
    ast_copy_str(node, buffer);
    ast_copy_source_anchor(node, source_node, AST_SOURCE_SYNTHETIC);
    node->int_value = value;
    node->bool_value = value ? 1 : 0;
    ast_set_value_type(0, node, type, 0, 0, 0, 0);
    ast_set_target_type(0, node, type, 0, 0, 0, 0);

    return node;
}

static ASTNode *inline_create_temp_value_ref(Context *context,
                                             ASTNode *instr_list,
                                             Scope *inline_scope,
                                             ASTNode *source_node,
                                             InlineCloneState *state,
                                             const char *prefix,
                                             size_t suffix) {
    Symbol *temp_symbol;
    ASTNode *capture_assign;
    ASTNode *capture_lhs;
    ASTNode *capture_rhs;
    ASTNode *temp_ref;

    if (!context || !instr_list || !inline_scope || !source_node || !state || !prefix) return NULL;

    temp_symbol = inline_create_temp_symbol(context, inline_scope, source_node, prefix, suffix);
    if (!temp_symbol) return NULL;

    capture_assign = ast_f(context, ASSIGN, source_node->token);
    if (!capture_assign) return NULL;
    capture_assign->scope = inline_scope;
    capture_assign->value_type = source_node->value_type;
    capture_assign->target_type = source_node->target_type;

    capture_lhs = inline_create_symbol_node(context,
                                            inline_scope,
                                            source_node,
                                            temp_symbol,
                                            VAR_TARGET,
                                            0,
                                            1);
    capture_rhs = inline_clone_subtree(context, source_node, state);
    if (!capture_lhs || !capture_rhs) return NULL;

    add_ast(capture_assign, capture_lhs);
    add_ast(capture_assign, capture_rhs);
    add_ast(instr_list, capture_assign);

    temp_ref = inline_create_symbol_node(context,
                                         inline_scope,
                                         source_node,
                                         temp_symbol,
                                         VAR_SYMBOL,
                                         1,
                                         0);
    if (!temp_ref) return NULL;

    ast_set_value_type(0,
                       temp_ref,
                       source_node->value_type,
                       source_node->value_dims,
                       source_node->value_dim_base,
                       source_node->value_dim_elements,
                       source_node->value_class);
    ast_set_target_type(0,
                        temp_ref,
                        source_node->target_type,
                        source_node->target_dims,
                        source_node->target_dim_base,
                        source_node->target_dim_elements,
                        source_node->target_class);

    return temp_ref;
}

static int inline_should_capture_scoped_actual(ASTNode *param_arg, ASTNode *actual_arg) {
    ASTNode *formal_target;

    if (!param_arg || !actual_arg) return 0;
    if (inline_is_missing_actual(actual_arg)) return 0;
    if (param_arg->is_ref_arg) return 0;
    if (!inline_subtree_reads_class_attribute(actual_arg)) return 0;

    formal_target = inline_formal_target(param_arg);
    if (inline_node_is_plain_object(formal_target) && param_arg->is_const_arg) return 0;

    return 1;
}

static int inline_subtree_reads_class_attribute(ASTNode *node) {
    ASTNode *child;

    if (!node) return 0;
    if (inline_node_is_callable_def(node)) return 0;

    if (node->node_type == VAR_SYMBOL &&
        node->symbolNode &&
        inline_symbol_is_class_attribute(node->symbolNode->symbol)) {
        return 1;
    }

    child = node->child;
    while (child) {
        if (inline_subtree_reads_class_attribute(child)) return 1;
        child = child->sibling;
    }

    return 0;
}

static int inline_sibling_list_reads_class_attribute(ASTNode *node) {
    while (node) {
        if (inline_subtree_reads_class_attribute(node)) return 1;
        node = node->sibling;
    }

    return 0;
}

static Scope *inline_find_callsite_instance_scope(ASTNode *call_node) {
    ASTNode *node;

    node = call_node;
    while (node) {
        ASTNode *association;

        association = node->association;
        if (association &&
            (association->node_type == METHOD || association->node_type == FACTORY) &&
            node->scope) {
            ASTNode lookup_node;
            const char *name;

            name = association->node_type == FACTORY ? "\xc2\xa7" "factory" : "\xc2\xa7" "this";
            memset(&lookup_node, 0, sizeof(lookup_node));
            lookup_node.node_string = (char *)name;
            lookup_node.node_string_length = strlen(name);

            if (sym_lrsv(node->scope, &lookup_node)) return node->scope;
        }

        node = node->parent;
    }

    return call_node ? call_node->scope : NULL;
}

static ASTNode *inline_scope_callable_association(Scope *scope) {
    while (scope) {
        ASTNode *defining_node;

        defining_node = scope->defining_node;
        if (inline_node_is_callable_def(defining_node)) return defining_node;
        if (defining_node && inline_node_is_callable_def(defining_node->association)) {
            return defining_node->association;
        }

        scope = scope->parent;
    }

    return NULL;
}

static int inline_scoped_call_needs_actual_capture(ASTNode *proc_def, ASTNode *call_node) {
    ASTNode *param_list;
    ASTNode *param_arg;
    ASTNode *actual_arg;
    ASTNode *varg_arg;

    if (!proc_def || !call_node) return 0;
    if (proc_def->node_type != FACTORY && proc_def->node_type != METHOD) return 0;

    param_list = ast_chld(proc_def, ARGS, 0);
    param_arg = param_list ? param_list->child : NULL;
    actual_arg = inline_call_first_user_actual(call_node);
    varg_arg = inline_find_varg_arg(proc_def);

    while (param_arg && actual_arg) {
        if (param_arg == varg_arg) {
            if (param_arg->is_ref_arg) return 0;
            return inline_sibling_list_reads_class_attribute(actual_arg);
        }

        if (inline_should_capture_scoped_actual(param_arg, actual_arg)) return 1;

        param_arg = param_arg->sibling;
        actual_arg = actual_arg->sibling;
    }

    return 0;
}

static Symbol *inline_capture_method_receiver_for_scoped_args(Context *context,
                                                              ASTNode *instr_list,
                                                              Scope *caller_scope,
                                                              ASTNode *proc_def,
                                                              ASTNode *call_node,
                                                              InlineCloneState *clone_state) {
    ASTNode *receiver;
    Symbol *temp_symbol;
    ASTNode *capture_assign;
    ASTNode *capture_lhs;
    ASTNode *capture_rhs;

    if (!context || !instr_list || !caller_scope || !proc_def || !call_node || !clone_state) return NULL;
    if (proc_def->node_type != METHOD) return NULL;

    receiver = inline_call_receiver(call_node);
    if (!receiver) return NULL;

    temp_symbol = inline_create_temp_symbol(context,
                                            caller_scope,
                                            receiver,
                                            "__inline_method_receiver",
                                            0);
    if (!temp_symbol) return NULL;

    capture_assign = ast_f(context, ASSIGN, receiver->token);
    if (!capture_assign) return NULL;
    capture_assign->association = inline_scope_callable_association(caller_scope);
    capture_assign->scope = caller_scope;
    capture_assign->inherit_parent_scope = 1;
    capture_assign->value_type = receiver->value_type;
    capture_assign->target_type = receiver->target_type;

    capture_lhs = inline_create_symbol_node(context,
                                            caller_scope,
                                            receiver,
                                            temp_symbol,
                                            VAR_TARGET,
                                            0,
                                            1);
    capture_rhs = inline_clone_subtree_in_scope(context, receiver, clone_state, caller_scope);
    if (!capture_lhs || !capture_rhs) return NULL;

    add_ast(capture_assign, capture_lhs);
    add_ast(capture_assign, capture_rhs);
    add_ast(instr_list, capture_assign);

    return temp_symbol;
}

static int inline_capture_scoped_call_actuals(Context *context,
                                              ASTNode *instr_list,
                                              Scope *inline_scope,
                                              ASTNode *proc_def,
                                              ASTNode *call_node,
                                              InlineCloneState *clone_state,
                                              Symbol **captured_receiver_out,
                                              Symbol ***captured_symbols_out,
                                              size_t *captured_count_out) {
    Scope *caller_scope;
    ASTNode *param_list;
    ASTNode *param_arg;
    ASTNode *actual_arg;
    ASTNode *varg_arg;
    Symbol **captured_symbols;
    size_t actual_count;
    size_t actual_index;
    int capture_varg_actuals;

    if (captured_receiver_out) *captured_receiver_out = NULL;
    if (captured_symbols_out) *captured_symbols_out = NULL;
    if (captured_count_out) *captured_count_out = 0;

    if (!context || !instr_list || !inline_scope || !proc_def || !call_node || !clone_state ||
        !captured_receiver_out || !captured_symbols_out || !captured_count_out) {
        return 0;
    }
    if (proc_def->node_type != FACTORY && proc_def->node_type != METHOD) return 1;
    if (!inline_scoped_call_needs_actual_capture(proc_def, call_node)) return 1;

    actual_arg = inline_call_first_user_actual(call_node);
    actual_count = inline_count_siblings(actual_arg);
    if (actual_count == 0) return 1;

    caller_scope = inline_find_callsite_instance_scope(call_node);
    if (!caller_scope) caller_scope = call_node->scope ? call_node->scope : inline_scope->parent;
    if (!caller_scope) return 0;

    if (proc_def->node_type == METHOD) {
        *captured_receiver_out = inline_capture_method_receiver_for_scoped_args(context,
                                                                               instr_list,
                                                                               caller_scope,
                                                                               proc_def,
                                                                               call_node,
                                                                               clone_state);
        if (!*captured_receiver_out) return 0;
    }

    captured_symbols = calloc(actual_count, sizeof(Symbol *));
    if (!captured_symbols) return 0;

    param_list = ast_chld(proc_def, ARGS, 0);
    param_arg = param_list ? param_list->child : NULL;
    varg_arg = inline_find_varg_arg(proc_def);
    actual_index = 0;
    capture_varg_actuals = 0;

    while (param_arg && actual_arg) {
        ASTNode *capture_assign;
        ASTNode *capture_lhs;
        ASTNode *capture_rhs;
        Symbol *temp_symbol;

        if (param_arg == varg_arg && param_arg->is_ref_arg) break;

        if (param_arg != varg_arg && !inline_should_capture_scoped_actual(param_arg, actual_arg)) {
            param_arg = param_arg->sibling;
            actual_arg = actual_arg->sibling;
            actual_index++;
            continue;
        }
        if (param_arg == varg_arg && !capture_varg_actuals) {
            capture_varg_actuals = inline_sibling_list_reads_class_attribute(actual_arg);
        }
        if (param_arg == varg_arg &&
            (inline_is_missing_actual(actual_arg) || !capture_varg_actuals)) {
            actual_arg = actual_arg->sibling;
            actual_index++;
            continue;
        }

        temp_symbol = inline_create_temp_symbol(context,
                                                caller_scope,
                                                actual_arg,
                                                "__inline_scoped_arg",
                                                actual_index);
        if (!temp_symbol) {
            free(captured_symbols);
            return 0;
        }

        capture_assign = ast_f(context, ASSIGN, actual_arg->token);
        if (!capture_assign) {
            free(captured_symbols);
            return 0;
        }
        capture_assign->association = inline_scope_callable_association(caller_scope);
        capture_assign->scope = caller_scope;
        capture_assign->inherit_parent_scope = 1;
        capture_assign->value_type = actual_arg->value_type;
        capture_assign->target_type = actual_arg->target_type;

        capture_lhs = inline_create_symbol_node(context,
                                                caller_scope,
                                                actual_arg,
                                                temp_symbol,
                                                VAR_TARGET,
                                                0,
                                                1);
        capture_rhs = inline_clone_subtree_in_scope(context, actual_arg, clone_state, caller_scope);
        if (!capture_lhs || !capture_rhs) {
            free(captured_symbols);
            return 0;
        }

        add_ast(capture_assign, capture_lhs);
        add_ast(capture_assign, capture_rhs);
        add_ast(instr_list, capture_assign);

        captured_symbols[actual_index] = temp_symbol;
        if (param_arg != varg_arg) param_arg = param_arg->sibling;
        actual_arg = actual_arg->sibling;
        actual_index++;
    }

    *captured_symbols_out = captured_symbols;
    *captured_count_out = actual_count;
    return 1;
}

static int inline_capture_varg_captured_actuals(Context *context,
                                                ASTNode *instr_list,
                                                Scope *inline_scope,
                                                ASTNode *varg_arg,
                                                ASTNode *actual_arg,
                                                InlineCloneState *state,
                                                Symbol **captured_symbols,
                                                size_t captured_count,
                                                size_t first_actual_index) {
    ASTNode *varg_type;
    ASTNode *source_template;
    size_t child_index;

    if (!context || !instr_list || !inline_scope || !varg_arg || !state) return 0;

    varg_type = inline_formal_default(varg_arg);
    source_template = varg_type ? varg_type : varg_arg;

    if (!actual_arg) {
        state->varg_symbols = NULL;
        state->varg_count = 0;
        return inline_initialise_varg_array(context, instr_list, inline_scope, varg_arg, source_template, state);
    }

    source_template = varg_type ? varg_type : actual_arg;
    state->varg_count = inline_count_siblings(actual_arg);
    state->varg_symbols = calloc(state->varg_count, sizeof(Symbol *));
    if (!state->varg_symbols) return 0;

    child_index = 0;
    while (actual_arg) {
        Symbol *captured_symbol;

        if (actual_arg->node_type == NOVAL) return 0;
        if (first_actual_index + child_index >= captured_count) return 0;

        captured_symbol = captured_symbols ? captured_symbols[first_actual_index + child_index] : NULL;
        if (!captured_symbol) return 0;

        state->varg_symbols[child_index] = captured_symbol;
        child_index++;
        actual_arg = actual_arg->sibling;
    }

    return inline_initialise_varg_array(context, instr_list, inline_scope, varg_arg, source_template, state);
}

static int inline_varg_actuals_are_captured(Symbol **captured_symbols,
                                            size_t captured_count,
                                            size_t first_actual_index,
                                            ASTNode *actual_arg) {
    size_t actual_index;

    if (!captured_symbols || !actual_arg) return 0;

    actual_index = first_actual_index;
    while (actual_arg) {
        if (actual_arg->node_type == NOVAL) return 0;
        if (actual_index >= captured_count || !captured_symbols[actual_index]) return 0;
        actual_arg = actual_arg->sibling;
        actual_index++;
    }

    return 1;
}

static ASTNode *inline_find_varg_arg(ASTNode *proc_def) {
    ASTNode *args;
    ASTNode *arg;

    if (!proc_def) return NULL;

    args = ast_chld(proc_def, ARGS, 0);
    arg = args ? args->child : NULL;
    while (arg) {
        if (arg->is_varg) return arg;
        arg = arg->sibling;
    }

    return NULL;
}

static int inline_call_arity_matches(ASTNode *call_node, Symbol *proc_sym, size_t *varg_count_out) {
    size_t actual_count;

    if (varg_count_out) *varg_count_out = 0;
    if (!call_node || !proc_sym) return 0;

    actual_count = inline_count_siblings(inline_call_first_user_actual(call_node));
    if (actual_count < proc_sym->fixed_args) return 0;
    if (!proc_sym->has_vargs && actual_count != proc_sym->fixed_args) return 0;

    if (varg_count_out && actual_count >= proc_sym->fixed_args) {
        *varg_count_out = actual_count - proc_sym->fixed_args;
    }

    return 1;
}

static int inline_varg_index_from_node(ASTNode *node, size_t *index_out) {
    int value;

    if (index_out) *index_out = 0;
    if (!node) return 0;

    switch (node->node_type) {
        case INTEGER:
        case CONSTANT:
            break;

        default:
            return 0;
    }

    value = node_to_integer(node);
    if (value < 1) return 0;

    if (index_out) *index_out = (size_t)value;
    return 1;
}

static walker_result inline_varg_usage_walker(walker_direction direction, ASTNode *node, void *payload) {
    InlinableCheck *check;
    size_t index;

    check = (InlinableCheck *)payload;
    if (!check || direction == in) return result_normal;

    if (node->node_type == OP_ARG_VALUE) {
        if (!node->child) {
            check->has_unsupported_varg_access = 1;
            return result_normal;
        }

        if (inline_varg_index_from_node(node->child, &index)) {
            if (index > check->max_required_varg_index) check->max_required_varg_index = index;
        } else {
            check->has_unsupported_varg_access = 1;
        }
    } else if (node->node_type == OP_ARG_IX_EXISTS) {
        if (!node->child) {
            check->has_unsupported_varg_access = 1;
        } else if (!inline_varg_index_from_node(node->child, &index)) {
            check->has_unsupported_varg_access = 1;
        }
    }

    return result_normal;
}

static int inline_analyse_varg_usage(ASTNode *proc_def, int *unsupported_out, size_t *max_required_index_out) {
    InlinableCheck check;
    ASTNode *varg_arg;

    if (unsupported_out) *unsupported_out = 0;
    if (max_required_index_out) *max_required_index_out = 0;
    if (!proc_def) return 0;

    memset(&check, 0, sizeof(check));
    varg_arg = inline_find_varg_arg(proc_def);
    check.ref_varg_mode = varg_arg && varg_arg->is_ref_arg;
    ast_wlkr(proc_def, inline_varg_usage_walker, &check);

    if (unsupported_out) *unsupported_out = check.has_unsupported_varg_access;
    if (max_required_index_out) *max_required_index_out = check.max_required_varg_index;

    return 1;
}

static Symbol *inline_create_temp_symbol(Context *context,
                                         Scope *inline_scope,
                                         ASTNode *source_node,
                                         const char *prefix,
                                         size_t suffix) {
    char temp_name[80];
    Symbol *temp_symbol;

    if (!context || !inline_scope || !source_node || !prefix) return NULL;

    snprintf(temp_name, sizeof(temp_name), "%s_%d_%zu", prefix, source_node->node_number, suffix);

    temp_symbol = sym_fn(inline_scope, temp_name, strlen(temp_name));
    if (!temp_symbol) return NULL;

    temp_symbol->symbol_type = VARIABLE_SYMBOL;
    temp_symbol->status = SYM_STATUS_LOCAL_VAR;
    temp_symbol->register_num = UNSET_REGISTER;
    temp_symbol->register_type = 'r';
    temp_symbol->meta_emitted = 0;
    temp_symbol->init_emitted = 0;

    if (!inline_copy_node_shape(temp_symbol, source_node)) return NULL;
    if (temp_symbol->value_dims > 0) temp_symbol->needs_default_initiation = 1;

    return temp_symbol;
}

static ASTNode *inline_create_symbol_node(Context *context,
                                          Scope *scope,
                                          ASTNode *source_node,
                                          Symbol *symbol,
                                          NodeType node_type,
                                          unsigned int read_usage,
                                          unsigned int write_usage) {
    ASTNode *node;

    if (!context || !scope || !source_node || !symbol) return NULL;

    node = ast_ftt(context, node_type, strdup(symbol->name));
    if (!node) return NULL;

    node->free_node_string = 1;
    node->scope = scope;
    ast_copy_source_anchor(node, source_node, AST_SOURCE_SYNTHETIC);
    sym_adnd(symbol, node, read_usage, write_usage);
    ast_svtp(node, symbol);

    return node;
}

static ASTNode *inline_clone_ref_actual(Context *context,
                                        ASTNode *formal_node,
                                        Scope *current_scope,
                                        InlineRefActualEntry *ref_entry,
                                        InlineCloneState *state) {
    ASTNode *replacement;
    ASTNode *source_child;
    ASTNode *formal_child;
    size_t child_index;

    if (!context || !formal_node || !current_scope || !ref_entry || !ref_entry->actual_source) return NULL;

    replacement = ast_dup(context, ref_entry->actual_source);
    if (!replacement) return NULL;

    replacement->node_type = formal_node->node_type;
    replacement->scope = current_scope;

    if (ref_entry->actual_source->symbolNode && ref_entry->actual_source->symbolNode->symbol) {
        sym_adnd(ref_entry->actual_source->symbolNode->symbol,
                 replacement,
                 formal_node->symbolNode ? formal_node->symbolNode->readUsage : 0,
                 formal_node->symbolNode ? formal_node->symbolNode->writeUsage : 0);
    }

    source_child = ref_entry->actual_source->child;
    child_index = 0;
    while (source_child) {
        ASTNode *captured_ref;

        if (child_index >= ref_entry->captured_count || !ref_entry->captured_symbols[child_index]) return NULL;

        captured_ref = inline_create_symbol_node(context,
                                                 current_scope,
                                                 source_child,
                                                 ref_entry->captured_symbols[child_index],
                                                 VAR_SYMBOL,
                                                 1,
                                                 0);
        if (!captured_ref) return NULL;

        add_ast(replacement, captured_ref);
        source_child = source_child->sibling;
        child_index++;
    }

    formal_child = formal_node->child;
    while (formal_child) {
        ASTNode *cloned_child;

        cloned_child = inline_clone_subtree_in_scope(context, formal_child, state, current_scope);
        if (!cloned_child) return NULL;

        add_ast(replacement, cloned_child);
        formal_child = formal_child->sibling;
    }

    inline_copy_replacement_semantics(replacement, formal_node);
    return replacement;
}

static ASTNode *inline_clone_ref_varg_actual(Context *context,
                                             ASTNode *source_node,
                                             Scope *current_scope,
                                             InlineRefActualEntry *ref_entry,
                                             InlineCloneState *state) {
    ASTNode *replacement;
    ASTNode *source_child;
    size_t child_index;

    if (!context || !source_node || !current_scope || !ref_entry || !ref_entry->actual_source) return NULL;

    replacement = ast_dup(context, ref_entry->actual_source);
    if (!replacement) return NULL;
    replacement->scope = current_scope;

    if (ref_entry->actual_source->symbolNode && ref_entry->actual_source->symbolNode->symbol) {
        sym_adnd(ref_entry->actual_source->symbolNode->symbol, replacement, 1, 0);
    }

    source_child = ref_entry->actual_source->child;
    child_index = 0;
    while (source_child) {
        ASTNode *captured_ref;

        if (child_index >= ref_entry->captured_count || !ref_entry->captured_symbols[child_index]) return NULL;

        captured_ref = inline_create_symbol_node(context,
                                                 current_scope,
                                                 source_child,
                                                 ref_entry->captured_symbols[child_index],
                                                 VAR_SYMBOL,
                                                 1,
                                                 0);
        if (!captured_ref) return NULL;

        add_ast(replacement, captured_ref);
        source_child = source_child->sibling;
        child_index++;
    }

    inline_copy_replacement_semantics(replacement, source_node);
    /* Mark forwarded `.ref` vararg actuals so later call-site checks can keep
     * them as normal calls rather than recursively inlining aliasing through a
     * synthetic locator model. */
    replacement->is_varg = 1;
    replacement->is_compiler_added = 1;
    return replacement;
}

static int inline_capture_ref_entry(Context *context,
                                    ASTNode *instr_list,
                                    Scope *inline_scope,
                                    ASTNode *actual_arg,
                                    InlineCloneState *state,
                                    InlineRefActualEntry *entry,
                                    const char *prefix) {
    ASTNode *child;
    size_t child_index;
    Symbol *formal_symbol;

    if (!context || !instr_list || !inline_scope || !actual_arg || !state || !entry || !prefix) return 0;
    if (!inline_is_supported_ref_actual(actual_arg)) return 0;

    formal_symbol = entry->formal_symbol;
    memset(entry, 0, sizeof(*entry));
    entry->formal_symbol = formal_symbol;
    entry->actual_source = actual_arg;
    entry->captured_count = inline_count_siblings(actual_arg->child);

    if (entry->captured_count) {
        entry->captured_symbols = calloc(entry->captured_count, sizeof(Symbol *));
        if (!entry->captured_symbols) return 0;
    }

    child = actual_arg->child;
    child_index = 0;
    while (child) {
        Symbol *temp_symbol;
        ASTNode *capture_assign;
        ASTNode *capture_lhs;
        ASTNode *capture_rhs;

        temp_symbol = inline_create_temp_symbol(context, inline_scope, child, prefix, child_index);
        if (!temp_symbol) return 0;

        capture_assign = ast_f(context, ASSIGN, child->token);
        if (!capture_assign) return 0;
        capture_assign->scope = inline_scope;
        capture_assign->value_type = child->value_type;
        capture_assign->target_type = child->value_type;

        capture_lhs = inline_create_symbol_node(context,
                                                inline_scope,
                                                child,
                                                temp_symbol,
                                                VAR_TARGET,
                                                0,
                                                1);
        capture_rhs = inline_clone_subtree(context, child, state);
        if (!capture_lhs || !capture_rhs) return 0;

        add_ast(capture_assign, capture_lhs);
        add_ast(capture_assign, capture_rhs);
        add_ast(instr_list, capture_assign);

        entry->captured_symbols[child_index] = temp_symbol;
        child = child->sibling;
        child_index++;
    }

    return 1;
}

static int inline_register_ref_actual(Context *context,
                                      ASTNode *instr_list,
                                      Scope *inline_scope,
                                      ASTNode *formal_target,
                                      ASTNode *actual_arg,
                                      InlineCloneState *state) {
    InlineRefActualEntry *new_entries;
    InlineRefActualEntry *entry;
    if (!context || !instr_list || !inline_scope || !formal_target || !actual_arg || !state) return 0;
    if (!formal_target->symbolNode || !formal_target->symbolNode->symbol) return 0;
    if (actual_arg->is_varg && actual_arg->is_compiler_added) return 0;
    if (!inline_is_supported_ref_actual(actual_arg)) return 0;

    entry = inline_find_ref_actual(state, formal_target->symbolNode->symbol);
    if (entry) return 1;

    new_entries = realloc(state->ref_entries, sizeof(InlineRefActualEntry) * (state->ref_count + 1));
    if (!new_entries) return 0;

    state->ref_entries = new_entries;
    entry = &state->ref_entries[state->ref_count];
    entry->formal_symbol = formal_target->symbolNode->symbol;
    if (!inline_capture_ref_entry(context,
                                  instr_list,
                                  inline_scope,
                                  actual_arg,
                                  state,
                                  entry,
                                  "__inline_ref")) {
        return 0;
    }

    state->ref_count++;
    return 1;
}

static int inline_capture_varg_actuals(Context *context,
                                       ASTNode *instr_list,
                                       Scope *inline_scope,
                                       ASTNode *varg_arg,
                                       ASTNode *actual_arg,
                                       InlineCloneState *state) {
    ASTNode *varg_type;
    ASTNode *source_template;
    size_t child_index;

    if (!context || !instr_list || !inline_scope || !varg_arg || !state) return 0;

    varg_type = inline_formal_default(varg_arg);
    source_template = varg_type ? varg_type : varg_arg;

    if (!actual_arg) {
        state->varg_symbols = NULL;
        state->varg_count = 0;
        return inline_initialise_varg_array(context, instr_list, inline_scope, varg_arg, source_template, state);
    }

    source_template = varg_type ? varg_type : actual_arg;
    state->varg_count = inline_count_siblings(actual_arg);
    state->varg_symbols = calloc(state->varg_count, sizeof(Symbol *));
    if (!state->varg_symbols) return 0;

    child_index = 0;
    while (actual_arg) {
        Symbol *temp_symbol;
        ASTNode *capture_assign;
        ASTNode *capture_lhs;
        ASTNode *capture_rhs;

        if (actual_arg->node_type == NOVAL) return 0;

        temp_symbol = inline_create_temp_symbol(context,
                                                inline_scope,
                                                varg_type ? varg_type : actual_arg,
                                                "__inline_varg",
                                                child_index);
        if (!temp_symbol) return 0;

        capture_assign = ast_f(context, ASSIGN, actual_arg->token);
        if (!capture_assign) return 0;
        capture_assign->scope = inline_scope;
        capture_assign->value_type = temp_symbol->type;
        capture_assign->target_type = temp_symbol->type;

        capture_lhs = inline_create_symbol_node(context,
                                                inline_scope,
                                                actual_arg,
                                                temp_symbol,
                                                VAR_TARGET,
                                                0,
                                                1);
        capture_rhs = inline_clone_subtree(context, actual_arg, state);
        if (!capture_lhs || !capture_rhs) return 0;

        add_ast(capture_assign, capture_lhs);
        add_ast(capture_assign, capture_rhs);
        add_ast(instr_list, capture_assign);

        state->varg_symbols[child_index] = temp_symbol;
        child_index++;
        actual_arg = actual_arg->sibling;
    }

    return inline_initialise_varg_array(context, instr_list, inline_scope, varg_arg, source_template, state);
}

static int inline_capture_ref_varg_actuals(Context *context,
                                           ASTNode *instr_list,
                                           Scope *inline_scope,
                                           ASTNode *actual_arg,
                                           InlineCloneState *state) {
    size_t child_index;

    if (!context || !instr_list || !inline_scope || !state) return 0;

    if (!actual_arg) {
        state->varg_count = 0;
        state->varg_ref_entries = NULL;
        return 1;
    }

    state->varg_count = inline_count_siblings(actual_arg);
    state->varg_ref_entries = calloc(state->varg_count, sizeof(InlineRefActualEntry));
    if (!state->varg_ref_entries) return 0;

    child_index = 0;
    while (actual_arg) {
        if (actual_arg->node_type == NOVAL) return 0;
        if (actual_arg->symbolNode) actual_arg->symbolNode->writeUsage = 1;
        if (!inline_capture_ref_entry(context,
                                      instr_list,
                                      inline_scope,
                                      actual_arg,
                                      state,
                                      &state->varg_ref_entries[child_index],
                                      "__inline_ref_varg")) {
            return 0;
        }
        child_index++;
        actual_arg = actual_arg->sibling;
    }

    return 1;
}

static Symbol *inline_create_varg_array_symbol(Context *context,
                                               Scope *inline_scope,
                                               ASTNode *template_node,
                                               ASTNode *source_node,
                                               size_t count) {
    Symbol *array_symbol;
    int *old_base;
    int *old_elements;
    int old_dims;
    int new_dims;
    int i;
    char temp_name[80];

    if (!context || !inline_scope || !source_node) return NULL;

    snprintf(temp_name, sizeof(temp_name), "__inline_varg_array_%d", source_node->node_number);
    array_symbol = sym_fn(inline_scope, temp_name, strlen(temp_name));
    if (!array_symbol) return NULL;

    array_symbol->symbol_type = VARIABLE_SYMBOL;
    array_symbol->status = SYM_STATUS_LOCAL_VAR;
    array_symbol->register_num = UNSET_REGISTER;
    array_symbol->register_type = 'r';
    array_symbol->meta_emitted = 0;
    array_symbol->init_emitted = 0;
    array_symbol->needs_default_initiation = 1;

    if (!inline_copy_node_shape(array_symbol, template_node ? template_node : source_node)) return NULL;

    old_base = array_symbol->dim_base;
    old_elements = array_symbol->dim_elements;
    old_dims = array_symbol->value_dims;
    new_dims = old_dims + 1;

    array_symbol->dim_base = calloc((size_t)new_dims, sizeof(int));
    array_symbol->dim_elements = calloc((size_t)new_dims, sizeof(int));
    if (!array_symbol->dim_base || !array_symbol->dim_elements) return NULL;

    array_symbol->value_dims = new_dims;
    array_symbol->dim_base[0] = 1;
    array_symbol->dim_elements[0] = (int)(count > 0 ? count : 1);

    for (i = 0; i < old_dims; i++) {
        array_symbol->dim_base[i + 1] = old_base ? old_base[i] : 1;
        array_symbol->dim_elements[i + 1] = old_elements ? old_elements[i] : 1;
    }

    free(old_base);
    free(old_elements);

    return array_symbol;
}

static ASTNode *inline_create_varg_array_slot(Context *context,
                                              Scope *scope,
                                              ASTNode *source_node,
                                              Symbol *array_symbol,
                                              ASTNode *index_node,
                                              NodeType node_type,
                                              ValueType element_type,
                                              int element_dims,
                                              int *element_base,
                                              int *element_elements,
                                              char *element_class) {
    ASTNode *slot_node;

    if (!context || !scope || !source_node || !array_symbol || !index_node) return NULL;

    slot_node = inline_create_symbol_node(context, scope, source_node, array_symbol, node_type, 1, node_type == VAR_TARGET ? 1 : 0);
    if (!slot_node) return NULL;

    add_ast(slot_node, index_node);
    ast_set_value_type(0, slot_node, element_type, element_dims, element_base, element_elements, element_class);
    ast_set_target_type(0, slot_node, element_type, element_dims, element_base, element_elements, element_class);

    return slot_node;
}

static int inline_initialise_varg_array(Context *context,
                                        ASTNode *instr_list,
                                        Scope *inline_scope,
                                        ASTNode *varg_arg,
                                        ASTNode *source_node,
                                        InlineCloneState *state) {
    ASTNode *template_node;
    size_t i;

    if (!context || !instr_list || !inline_scope || !source_node || !state) return 0;
    if (state->varg_array_symbol) return 1;

    template_node = inline_formal_default(varg_arg);
    if (!template_node) template_node = source_node;

    state->varg_array_symbol = inline_create_varg_array_symbol(context,
                                                               inline_scope,
                                                               template_node,
                                                               source_node,
                                                               state->varg_count);
    if (!state->varg_array_symbol) return 0;

    for (i = 0; i < state->varg_count; i++) {
        ASTNode *assign_node;
        ASTNode *lhs;
        ASTNode *rhs;
        ASTNode *index_node;

        if (!state->varg_symbols || !state->varg_symbols[i]) return 0;

        assign_node = ast_f(context, ASSIGN, source_node->token);
        if (!assign_node) return 0;
        assign_node->scope = inline_scope;
        assign_node->value_type = state->varg_symbols[i]->type;
        assign_node->target_type = state->varg_symbols[i]->type;

        index_node = inline_create_integer_constant(context, source_node, (int)(i + 1), TP_INTEGER);
        rhs = inline_create_symbol_node(context,
                                        inline_scope,
                                        source_node,
                                        state->varg_symbols[i],
                                        VAR_SYMBOL,
                                        1,
                                        0);
        if (!index_node || !rhs) return 0;
        index_node->scope = inline_scope;

        lhs = inline_create_varg_array_slot(context,
                                            inline_scope,
                                            source_node,
                                            state->varg_array_symbol,
                                            index_node,
                                            VAR_TARGET,
                                            state->varg_symbols[i]->type,
                                            state->varg_symbols[i]->value_dims,
                                            state->varg_symbols[i]->dim_base,
                                            state->varg_symbols[i]->dim_elements,
                                            state->varg_symbols[i]->value_class);
        if (!lhs) return 0;

        add_ast(assign_node, lhs);
        add_ast(assign_node, rhs);
        add_ast(instr_list, assign_node);
    }

    return 1;
}

static ASTNode *inline_create_assembler_instr(Context *context,
                                              Scope *scope,
                                              ASTNode *source_node,
                                              const char *opcode,
                                              ASTNode *arg1,
                                              ASTNode *arg2,
                                              ASTNode *arg3) {
    ASTNode *instr;

    if (!context || !scope || !source_node || !opcode) return NULL;

    instr = ast_ftt(context, ASSEMBLER, strdup(opcode));
    if (!instr) return NULL;

    instr->free_node_string = 1;
    instr->scope = scope;
    ast_copy_source_anchor(instr, source_node, AST_SOURCE_SYNTHETIC);

    if (arg1) add_ast(instr, arg1);
    if (arg2) add_ast(instr, arg2);
    if (arg3) add_ast(instr, arg3);

    return instr;
}

static ASTNode *inline_create_string_constant(Context *context, ASTNode *source_node, const char *value) {
    ASTNode *node;

    if (!context || !source_node || !value) return NULL;

    node = ast_ft(context, STRING);
    if (!node) return NULL;

    ast_copy_str(node, (char *)value);
    ast_copy_source_anchor(node, source_node, AST_SOURCE_SYNTHETIC);
    ast_set_value_type(0, node, TP_STRING, 0, 0, 0, 0);
    ast_set_target_type(0, node, TP_STRING, 0, 0, 0, 0);

    return node;
}

static Symbol *inline_find_instance_symbol(ASTNode *proc_def,
                                           InlineCloneState *state) {
    ASTNode lookup_node;
    const char *name;
    size_t name_len;
    Symbol *old_symbol;
    Symbol *new_symbol;

    if (!proc_def || !proc_def->scope || !state) return NULL;

    if (proc_def->node_type == METHOD) {
        name = "\xc2\xa7" "this";
    } else if (proc_def->node_type == FACTORY) {
        name = "\xc2\xa7" "factory";
    } else {
        return NULL;
    }

    name_len = strlen(name);
    memset(&lookup_node, 0, sizeof(lookup_node));
    lookup_node.node_string = (char *)name;
    lookup_node.node_string_length = name_len;

    old_symbol = sym_lrsv(proc_def->scope, &lookup_node);
    if (!old_symbol) return NULL;

    new_symbol = inline_find_mapped_symbol(state, old_symbol);
    if (!new_symbol) return NULL;

    return new_symbol;
}

static int inline_count_factory_attributes(ASTNode *factory_def) {
    ASTNode *class_node;
    ASTNode *attr;
    int count;

    if (!factory_def) return 0;

    class_node = factory_def->parent;
    while (class_node && class_node->node_type != CLASS_DEF) class_node = class_node->parent;
    if (!class_node) return 0;

    count = 0;
    attr = class_node->child;
    while (attr) {
        if (attr->node_type == DEFINE) {
            int index;
            ASTNode *nr;

            index = -1;
            nr = ast_chld(attr, NODE_REGISTER, 0);
            if (nr) {
                ASTNode *idx;

                idx = ast_chld(nr, INTEGER, 0);
                if (idx) index = node_to_integer(idx);
                else if (nr->int_value) index = (int)nr->int_value;
            }

            if (index >= count) count = index + 1;
            else if (index == -1) count++;
        }
        attr = attr->sibling;
    }

    return count;
}

static int inline_bind_method_receiver(Context *context,
                                       ASTNode *instr_list,
                                       Scope *inline_scope,
                                       ASTNode *proc_def,
                                       ASTNode *call_node,
                                       InlineCloneState *clone_state,
                                       Symbol *captured_receiver_symbol) {
    Symbol *this_symbol;
    ASTNode *receiver;
    ASTNode *assign_node;
    ASTNode *assign_lhs;
    ASTNode *assign_rhs;

    if (!context || !instr_list || !inline_scope || !proc_def || !call_node || !clone_state) return 0;
    if (proc_def->node_type != METHOD) return 1;

    receiver = inline_call_receiver(call_node);
    if (!receiver) return 0;

    this_symbol = inline_find_instance_symbol(proc_def, clone_state);
    if (!this_symbol) return 0;

    assign_node = ast_f(context, ASSIGN, receiver->token);
    if (!assign_node) return 0;
    assign_node->scope = inline_scope;
    assign_node->value_type = receiver->value_type;
    assign_node->target_type = receiver->target_type;

    assign_lhs = inline_create_symbol_node(context,
                                           inline_scope,
                                           receiver,
                                           this_symbol,
                                           VAR_TARGET,
                                           0,
                                           1);
    if (captured_receiver_symbol) {
        assign_rhs = inline_create_symbol_node(context,
                                               inline_scope,
                                               receiver,
                                               captured_receiver_symbol,
                                               VAR_SYMBOL,
                                               1,
                                               0);
    } else {
        assign_rhs = inline_clone_subtree(context, receiver, clone_state);
    }
    if (!assign_lhs || !assign_rhs) return 0;

    add_ast(assign_node, assign_lhs);
    add_ast(assign_node, assign_rhs);
    add_ast(instr_list, assign_node);

    if (inline_method_writes_class_attribute(proc_def) &&
        inline_is_direct_receiver_copyback_target(receiver) &&
        receiver->symbolNode &&
        receiver->symbolNode->symbol) {
        clone_state->method_receiver_source_symbol = receiver->symbolNode->symbol;
        clone_state->method_receiver_local_symbol = this_symbol;
        clone_state->method_receiver_needs_copyback = 1;
    }

    return 1;
}

static int inline_initialise_factory_instance(Context *context,
                                              ASTNode *instr_list,
                                              Scope *inline_scope,
                                              ASTNode *proc_def,
                                              InlineCloneState *clone_state) {
    Symbol *factory_symbol;
    ASTNode *class_node;
    ASTNode *factory_target;
    ASTNode *attrs_count;
    ASTNode *setattrs_instr;
    ASTNode *setobjtype_target;
    ASTNode *class_name_node;
    ASTNode *setobjtype_instr;
    char *class_fq;

    if (!context || !instr_list || !inline_scope || !proc_def || !clone_state) return 0;
    if (proc_def->node_type != FACTORY) return 1;

    factory_symbol = inline_find_instance_symbol(proc_def, clone_state);
    if (!factory_symbol) return 0;

    factory_target = inline_create_symbol_node(context,
                                               inline_scope,
                                               proc_def,
                                               factory_symbol,
                                               VAR_TARGET,
                                               0,
                                               1);
    attrs_count = inline_create_integer_constant(context,
                                                 proc_def,
                                                 inline_count_factory_attributes(proc_def),
                                                 TP_INTEGER);
    if (!factory_target || !attrs_count) return 0;
    attrs_count->scope = inline_scope;

    setattrs_instr = inline_create_assembler_instr(context,
                                                   inline_scope,
                                                   proc_def,
                                                   "setattrs",
                                                   factory_target,
                                                   attrs_count,
                                                   NULL);
    if (!setattrs_instr) return 0;
    add_ast(instr_list, setattrs_instr);

    class_node = proc_def->parent;
    while (class_node && class_node->node_type != CLASS_DEF) class_node = class_node->parent;
    if (!class_node || !class_node->symbolNode || !class_node->symbolNode->symbol) return 1;

    class_fq = sym_frnm(class_node->symbolNode->symbol);
    if (!class_fq) return 0;

    setobjtype_target = inline_create_symbol_node(context,
                                                  inline_scope,
                                                  proc_def,
                                                  factory_symbol,
                                                  VAR_TARGET,
                                                  0,
                                                  1);
    class_name_node = inline_create_string_constant(context, proc_def, class_fq);
    free(class_fq);
    if (!setobjtype_target || !class_name_node) return 0;
    class_name_node->scope = inline_scope;

    setobjtype_instr = inline_create_assembler_instr(context,
                                                     inline_scope,
                                                     proc_def,
                                                     "setobjtype",
                                                     setobjtype_target,
                                                     class_name_node,
                                                     NULL);
    if (!setobjtype_instr) return 0;
    add_ast(instr_list, setobjtype_instr);

    return 1;
}

static int inline_append_method_receiver_copyback(Context *context,
                                                  ASTNode *instr_list,
                                                  Scope *inline_scope,
                                                  ASTNode *source_node,
                                                  InlineCloneState *clone_state) {
    ASTNode *copy_lhs;
    ASTNode *copy_rhs;
    ASTNode *value_copy;
    ASTNode *attr_copy;

    if (!context || !instr_list || !inline_scope || !source_node || !clone_state) return 0;
    if (!clone_state->method_receiver_needs_copyback) return 1;
    if (!clone_state->method_receiver_source_symbol || !clone_state->method_receiver_local_symbol) return 0;

    copy_lhs = inline_create_symbol_node(context,
                                         inline_scope,
                                         source_node,
                                         clone_state->method_receiver_source_symbol,
                                         VAR_TARGET,
                                         0,
                                         1);
    copy_rhs = inline_create_symbol_node(context,
                                         inline_scope,
                                         source_node,
                                         clone_state->method_receiver_local_symbol,
                                         VAR_SYMBOL,
                                         1,
                                         0);
    if (!copy_lhs || !copy_rhs) return 0;

    value_copy = inline_create_register_copy_instr(context, inline_scope, "copy", copy_lhs, copy_rhs);
    attr_copy = inline_create_register_copy_instr(context, inline_scope, "acopy", copy_lhs, copy_rhs);
    if (!value_copy || !attr_copy) return 0;

    add_ast(instr_list, value_copy);
    add_ast(instr_list, attr_copy);
    return 1;
}

static ASTNode *inline_build_dynamic_varg_value(Context *context,
                                                ASTNode *node,
                                                Scope *current_scope,
                                                InlineCloneState *state) {
    ASTNode *block_expr;
    ASTNode *instr_list;
    Scope *inline_scope;
    Symbol *index_symbol;
    ASTNode *assign_node;
    ASTNode *assign_lhs;
    ASTNode *assign_rhs;
    ASTNode *leave_node;
    ASTNode *slot_node;
    ASTNode *index_ref;

    if (!context || !node || !current_scope || !state || !state->varg_array_symbol || !node->child) return NULL;

    block_expr = ast_dup(context, node);
    if (!block_expr) return NULL;
    block_expr->node_type = BLOCK_EXPR;
    block_expr->node_string = "do";
    block_expr->node_string_length = 2;
    block_expr->free_node_string = 0;

    inline_scope = scp_f(context, current_scope, block_expr, NULL, SCOPE_LOCAL);
    if (!inline_scope) return NULL;
    block_expr->scope = inline_scope;

    instr_list = ast_f(context, INSTRUCTIONS, node->token);
    if (!instr_list) return NULL;
    instr_list->scope = inline_scope;
    instr_list->value_type = TP_VOID;
    instr_list->target_type = TP_VOID;
    add_ast(block_expr, instr_list);

    index_symbol = inline_create_temp_symbol(context, inline_scope, node->child, "__inline_arg_ix", 0);
    if (!index_symbol) return NULL;

    assign_node = ast_f(context, ASSIGN, node->token);
    if (!assign_node) return NULL;
    assign_node->scope = inline_scope;
    assign_node->value_type = TP_INTEGER;
    assign_node->target_type = TP_INTEGER;

    assign_lhs = inline_create_symbol_node(context, inline_scope, node->child, index_symbol, VAR_TARGET, 0, 1);
    assign_rhs = inline_clone_subtree_in_scope(context, node->child, state, inline_scope);
    if (!assign_lhs || !assign_rhs) return NULL;
    add_ast(assign_node, assign_lhs);
    add_ast(assign_node, assign_rhs);
    add_ast(instr_list, assign_node);

    leave_node = ast_f(context, LEAVE_WITH, node->token);
    if (!leave_node) return NULL;
    leave_node->scope = inline_scope;
    leave_node->association = block_expr;
    leave_node->value_type = node->value_type;
    leave_node->target_type = node->target_type;

    index_ref = inline_create_symbol_node(context, inline_scope, node, index_symbol, VAR_SYMBOL, 1, 0);
    if (!index_ref) return NULL;

    slot_node = inline_create_varg_array_slot(context,
                                              inline_scope,
                                              node,
                                              state->varg_array_symbol,
                                              index_ref,
                                              VAR_SYMBOL,
                                              node->value_type,
                                              node->value_dims,
                                              node->value_dim_base,
                                              node->value_dim_elements,
                                              node->value_class);
    if (!slot_node) return NULL;

    add_ast(leave_node, slot_node);
    add_ast(instr_list, leave_node);
    return block_expr;
}

static ASTNode *inline_build_dynamic_varg_exists(Context *context,
                                                 ASTNode *node,
                                                 Scope *current_scope,
                                                 InlineCloneState *state) {
    ASTNode *block_expr;
    ASTNode *instr_list;
    Scope *inline_scope;
    Symbol *index_symbol;
    ASTNode *assign_node;
    ASTNode *assign_lhs;
    ASTNode *assign_rhs;
    ASTNode *leave_node;
    ASTNode *index_ref;
    ASTNode *const_one;
    ASTNode *const_max;
    ASTNode *gte_node;
    ASTNode *lte_node;
    ASTNode *and_node;

    if (!context || !node || !current_scope || !state || !node->child) return NULL;

    block_expr = ast_dup(context, node);
    if (!block_expr) return NULL;
    block_expr->node_type = BLOCK_EXPR;
    block_expr->node_string = "do";
    block_expr->node_string_length = 2;
    block_expr->free_node_string = 0;

    inline_scope = scp_f(context, current_scope, block_expr, NULL, SCOPE_LOCAL);
    if (!inline_scope) return NULL;
    block_expr->scope = inline_scope;

    instr_list = ast_f(context, INSTRUCTIONS, node->token);
    if (!instr_list) return NULL;
    instr_list->scope = inline_scope;
    instr_list->value_type = TP_VOID;
    instr_list->target_type = TP_VOID;
    add_ast(block_expr, instr_list);

    index_symbol = inline_create_temp_symbol(context, inline_scope, node->child, "__inline_arg_ix", 1);
    if (!index_symbol) return NULL;

    assign_node = ast_f(context, ASSIGN, node->token);
    if (!assign_node) return NULL;
    assign_node->scope = inline_scope;
    assign_node->value_type = TP_INTEGER;
    assign_node->target_type = TP_INTEGER;

    assign_lhs = inline_create_symbol_node(context, inline_scope, node->child, index_symbol, VAR_TARGET, 0, 1);
    assign_rhs = inline_clone_subtree_in_scope(context, node->child, state, inline_scope);
    if (!assign_lhs || !assign_rhs) return NULL;
    add_ast(assign_node, assign_lhs);
    add_ast(assign_node, assign_rhs);
    add_ast(instr_list, assign_node);

    index_ref = inline_create_symbol_node(context, inline_scope, node, index_symbol, VAR_SYMBOL, 1, 0);
    const_one = inline_create_integer_constant(context, node, 1, TP_INTEGER);
    const_max = inline_create_integer_constant(context, node, (int)state->varg_count, TP_INTEGER);
    if (!index_ref || !const_one || !const_max) return NULL;
    index_ref->scope = inline_scope;
    const_one->scope = inline_scope;
    const_max->scope = inline_scope;

    gte_node = ast_f(context, OP_COMPARE_GTE, node->token);
    lte_node = ast_f(context, OP_COMPARE_LTE, node->token);
    and_node = ast_f(context, OP_AND, node->token);
    leave_node = ast_f(context, LEAVE_WITH, node->token);
    if (!gte_node || !lte_node || !and_node || !leave_node) return NULL;

    gte_node->scope = inline_scope;
    lte_node->scope = inline_scope;
    and_node->scope = inline_scope;
    leave_node->scope = inline_scope;
    leave_node->association = block_expr;

    ast_set_value_type(0, gte_node, TP_BOOLEAN, 0, 0, 0, 0);
    ast_set_target_type(0, gte_node, TP_BOOLEAN, 0, 0, 0, 0);
    ast_set_value_type(0, lte_node, TP_BOOLEAN, 0, 0, 0, 0);
    ast_set_target_type(0, lte_node, TP_BOOLEAN, 0, 0, 0, 0);
    ast_set_value_type(0, and_node, TP_BOOLEAN, 0, 0, 0, 0);
    ast_set_target_type(0, and_node, TP_BOOLEAN, 0, 0, 0, 0);
    leave_node->value_type = TP_BOOLEAN;
    leave_node->target_type = TP_BOOLEAN;

    add_ast(gte_node, index_ref);
    add_ast(gte_node, const_one);

    index_ref = inline_create_symbol_node(context, inline_scope, node, index_symbol, VAR_SYMBOL, 1, 0);
    if (!index_ref) return NULL;
    add_ast(lte_node, index_ref);
    add_ast(lte_node, const_max);

    add_ast(and_node, gte_node);
    add_ast(and_node, lte_node);
    add_ast(leave_node, and_node);
    add_ast(instr_list, leave_node);

    return block_expr;
}

static int inline_bind_call_arguments(Context *context,
                                      ASTNode *instr_list,
                                      Scope *inline_scope,
                                      ASTNode *proc_def,
                                      ASTNode *call_node,
                                      Symbol *proc_sym,
                                      InlineCloneState *clone_state) {
    ASTNode *param_list;
    ASTNode *param_arg;
    ASTNode *actual_arg;
    ASTNode *varg_arg;
    Symbol *captured_method_receiver;
    Symbol **captured_scoped_actuals;
    size_t captured_scoped_actual_count;
    size_t actual_index;

#define INLINE_BIND_RETURN(value) do { free(captured_scoped_actuals); return (value); } while (0)

    if (!context || !instr_list || !inline_scope || !proc_def || !call_node || !proc_sym || !clone_state) return 0;

    captured_scoped_actuals = NULL;
    captured_method_receiver = NULL;
    captured_scoped_actual_count = 0;
    actual_index = 0;

    if (!inline_call_arity_matches(call_node, proc_sym, NULL)) INLINE_BIND_RETURN(0);
    if (!inline_capture_scoped_call_actuals(context,
                                            instr_list,
                                            inline_scope,
                                            proc_def,
                                            call_node,
                                            clone_state,
                                            &captured_method_receiver,
                                            &captured_scoped_actuals,
                                            &captured_scoped_actual_count)) {
        INLINE_BIND_RETURN(0);
    }
    if (!inline_initialise_factory_instance(context, instr_list, inline_scope, proc_def, clone_state)) INLINE_BIND_RETURN(0);
    if (!inline_bind_method_receiver(context,
                                     instr_list,
                                     inline_scope,
                                     proc_def,
                                     call_node,
                                     clone_state,
                                     captured_method_receiver)) {
        INLINE_BIND_RETURN(0);
    }

    param_list = ast_chld(proc_def, ARGS, 0);
    param_arg = param_list ? param_list->child : NULL;
    actual_arg = inline_call_first_user_actual(call_node);
    varg_arg = inline_find_varg_arg(proc_def);

    while (param_arg) {
        ASTNode *formal_target;
        ASTNode *formal_default;
        ASTNode *bind_assign;
        ASTNode *bind_lhs;
        ASTNode *bind_rhs;
        ASTNode *bind_source;
        Symbol *captured_actual_symbol;

        if (param_arg == varg_arg) break;

        formal_target = inline_formal_target(param_arg);
        formal_default = inline_formal_default(param_arg);
        captured_actual_symbol = actual_index < captured_scoped_actual_count ?
                                 captured_scoped_actuals[actual_index] : NULL;
        if (!formal_target || !actual_arg) INLINE_BIND_RETURN(0);

        if (param_arg->is_ref_arg && !inline_is_missing_actual(actual_arg)) {
            if (!inline_register_ref_actual(context, instr_list, inline_scope, formal_target, actual_arg, clone_state)) {
                INLINE_BIND_RETURN(0);
            }
            param_arg = param_arg->sibling;
            actual_arg = actual_arg->sibling;
            actual_index++;
            continue;
        }

        if (inline_is_missing_actual(actual_arg)) {
            if (!param_arg->is_opt_arg || !formal_default) INLINE_BIND_RETURN(0);
        }

        if (inline_node_is_plain_object(formal_target) &&
            param_arg->is_const_arg &&
            !inline_is_missing_actual(actual_arg)) {
            if (!inline_register_ref_actual(context, instr_list, inline_scope, formal_target, actual_arg, clone_state)) {
                INLINE_BIND_RETURN(0);
            }
            param_arg = param_arg->sibling;
            actual_arg = actual_arg->sibling;
            actual_index++;
            continue;
        }

        bind_assign = ast_f(context, ASSIGN, formal_target->token);
        bind_assign->scope = inline_scope;
        bind_assign->value_type = formal_target->value_type;
        bind_assign->target_type = formal_target->target_type;

        bind_lhs = inline_clone_subtree(context, formal_target, clone_state);
        if (inline_is_missing_actual(actual_arg)) {
            bind_source = formal_default;
        } else if (captured_actual_symbol) {
            bind_source = actual_arg;
        } else {
            bind_source = actual_arg;
        }

        if (captured_actual_symbol) {
            bind_rhs = inline_create_symbol_node(context,
                                                 inline_scope,
                                                 actual_arg,
                                                 captured_actual_symbol,
                                                 VAR_SYMBOL,
                                                 1,
                                                 0);
        } else if (inline_formal_needs_isolated_copy(formal_target, param_arg) &&
            !inline_is_direct_symbol_actual(bind_source)) {
            bind_rhs = inline_create_temp_value_ref(context,
                                                    instr_list,
                                                    inline_scope,
                                                    bind_source,
                                                    clone_state,
                                                    "__inline_bind",
                                                    0);
        } else if (inline_is_missing_actual(actual_arg)) {
            bind_rhs = inline_clone_subtree(context, formal_default, clone_state);
            bind_source = formal_default;
        } else {
            bind_rhs = inline_clone_subtree(context, actual_arg, clone_state);
            bind_source = actual_arg;
        }

        if (!bind_lhs || !bind_rhs) INLINE_BIND_RETURN(0);

        if (inline_formal_needs_isolated_copy(formal_target, param_arg)) {
            ASTNode *bind_copy;
            ASTNode *attr_copy;

            bind_copy = inline_create_register_copy_instr(context, inline_scope, "copy", bind_lhs, bind_rhs);
            attr_copy = inline_create_register_copy_instr(context, inline_scope, "acopy", bind_lhs, bind_rhs);
            if (!bind_copy || !attr_copy) INLINE_BIND_RETURN(0);

            add_ast(instr_list, bind_copy);
            add_ast(instr_list, attr_copy);
        } else {
            add_ast(bind_assign, bind_lhs);
            add_ast(bind_assign, bind_rhs);
            add_ast(instr_list, bind_assign);
        }

        param_arg = param_arg->sibling;
        actual_arg = actual_arg->sibling;
        actual_index++;
    }

    if (varg_arg) {
        if (varg_arg->is_ref_arg) {
            if (!inline_capture_ref_varg_actuals(context, instr_list, inline_scope, actual_arg, clone_state)) {
                INLINE_BIND_RETURN(0);
            }
        } else if ((proc_def->node_type == FACTORY || proc_def->node_type == METHOD) &&
                   inline_varg_actuals_are_captured(captured_scoped_actuals,
                                                    captured_scoped_actual_count,
                                                    actual_index,
                                                    actual_arg)) {
            if (!inline_capture_varg_captured_actuals(context,
                                                      instr_list,
                                                      inline_scope,
                                                      varg_arg,
                                                      actual_arg,
                                                      clone_state,
                                                      captured_scoped_actuals,
                                                      captured_scoped_actual_count,
                                                      actual_index)) {
                INLINE_BIND_RETURN(0);
            }
        } else {
            if (!inline_capture_varg_actuals(context, instr_list, inline_scope, varg_arg, actual_arg, clone_state)) {
                INLINE_BIND_RETURN(0);
            }
        }
        actual_arg = NULL;
        param_arg = varg_arg ? varg_arg->sibling : param_arg;
    }

    if (actual_arg || param_arg) INLINE_BIND_RETURN(0);

    INLINE_BIND_RETURN(1);
#undef INLINE_BIND_RETURN
}

static int inline_append_scope_map_entry(InlineCloneState *state, Scope *old_scope, Scope *new_scope) {
    InlineScopeMapEntry *new_entries;

    if (!state || !old_scope || !new_scope) return 0;

    new_entries = realloc(state->scope_entries,
                          sizeof(InlineScopeMapEntry) * (state->scope_count + 1));
    if (!new_entries) return 0;

    state->scope_entries = new_entries;
    state->scope_entries[state->scope_count].old_scope = old_scope;
    state->scope_entries[state->scope_count].new_scope = new_scope;
    state->scope_count++;
    return 1;
}

static int inline_append_node_map_entry(InlineCloneState *state, ASTNode *old_node, ASTNode *new_node) {
    InlineNodeMapEntry *new_entries;

    if (!state || !old_node || !new_node) return 0;

    new_entries = realloc(state->node_entries,
                          sizeof(InlineNodeMapEntry) * (state->node_count + 1));
    if (!new_entries) return 0;

    state->node_entries = new_entries;
    state->node_entries[state->node_count].old_node = old_node;
    state->node_entries[state->node_count].new_node = new_node;
    state->node_count++;
    return 1;
}

static int inline_duplicate_scope_symbols(Scope *old_scope,
                                          Scope *new_scope,
                                          InlineCloneState *state) {
    Symbol **symbols;
    size_t i;

    if (!old_scope || !new_scope || !state) return 0;

    symbols = scp_syms(old_scope);
    if (!symbols) return 1;

    for (i = 0; symbols[i]; i++) {
        Symbol *old_symbol;
        Symbol *new_symbol;

        old_symbol = symbols[i];
        if (!old_symbol || old_symbol->symbol_type == FUNCTION_SYMBOL) continue;
        if (inline_find_mapped_symbol(state, old_symbol)) continue;

        new_symbol = sym_dup(new_scope, old_symbol);
        if (!new_symbol) {
            free(symbols);
            return 0;
        }

        new_symbol->register_num = UNSET_REGISTER;
        new_symbol->register_type = 'r';
        new_symbol->meta_emitted = 0;
        new_symbol->init_emitted = 0;
        new_symbol->needs_default_initiation = old_symbol->needs_default_initiation;
        new_symbol->defines_scope = NULL;
        new_symbol->ast_template = NULL;
        new_symbol->is_inlinable = 0;

        if (!inline_append_symbol_map_entry(state, old_symbol, new_symbol)) {
            free(symbols);
            return 0;
        }
    }

    free(symbols);
    return 1;
}

static Scope *inline_clone_scope(Context *context,
                                 Scope *old_scope,
                                 Scope *new_parent,
                                 ASTNode *new_defining_node,
                                 InlineCloneState *state) {
    Scope *new_scope;

    if (!context || !old_scope || !state) return NULL;

    new_scope = scp_f(context, new_parent, new_defining_node, NULL, old_scope->type);
    if (!new_scope) return NULL;

    if (old_scope->name) new_scope->name = strdup(old_scope->name);
    inline_copy_numeric_context(new_scope, old_scope);

    if (!inline_append_scope_map_entry(state, old_scope, new_scope)) return NULL;
    if (!inline_duplicate_scope_symbols(old_scope, new_scope, state)) return NULL;

    return new_scope;
}

static Scope *inline_prepare_cloned_node_scope(Context *context,
                                               ASTNode *old_node,
                                               ASTNode *new_node,
                                               Scope *current_scope,
                                               InlineCloneState *state) {
    Scope *node_scope;

    if (!new_node) return NULL;

    node_scope = current_scope ? current_scope : old_node->scope;

    if (old_node->inherit_parent_scope && old_node->scope) {
        Scope *mapped_scope;

        mapped_scope = inline_find_mapped_scope(state, old_node->scope);
        if (mapped_scope) node_scope = mapped_scope;
    }

    if (old_node->scope && old_node->scope->defining_node == old_node) {
        node_scope = inline_find_mapped_scope(state, old_node->scope);
        if (!node_scope) {
            node_scope = inline_clone_scope(context, old_node->scope, current_scope, new_node, state);
            if (!node_scope) return NULL;
        }
        new_node->scope = node_scope;
        if (!inline_append_node_map_entry(state, old_node, new_node)) return NULL;
        return node_scope;
    }

    if (inline_node_requires_local_scope(old_node)) {
        node_scope = scp_f(context, current_scope, new_node, NULL, SCOPE_LOCAL);
        if (!node_scope) return NULL;
    }

    new_node->scope = node_scope;
    return node_scope;
}

static ASTNode *inline_clone_factory_selector_association(Context *context,
                                                          ASTNode *node,
                                                          Scope *node_scope) {
    ASTNode *selector;

    if (!context || !node || node->node_type != FACTORY_CALL || !node->association) return NULL;

    selector = ast_dup(context, node->association);
    if (!selector) return NULL;

    selector->scope = node_scope;
    return selector;
}

static ASTNode *inline_clone_subtree_in_scope(Context *context,
                                              ASTNode *node,
                                              InlineCloneState *state,
                                              Scope *current_scope) {
    ASTNode *new_node;
    ASTNode *child;
    Symbol *mapped_symbol;
    Scope *node_scope;
    ASTNode *mapped_association;

    if (!node) return NULL;

    if (state && node->symbolNode && node->symbolNode->symbol &&
        (node->node_type == VAR_SYMBOL || node->node_type == VAR_TARGET || node->node_type == VAR_REFERENCE)) {
        InlineRefActualEntry *ref_entry;

        ref_entry = inline_find_ref_actual(state, node->symbolNode->symbol);
        if (ref_entry) return inline_clone_ref_actual(context, node, current_scope, ref_entry, state);
    }

    if (state && node->node_type == OP_ARGS) {
        ASTNode *count_node;

        count_node = inline_create_integer_constant(context, node, (int)state->varg_count, TP_INTEGER);
        if (count_node) count_node->scope = current_scope;
        return count_node;
    }

    if (state && node->node_type == OP_ARG_VALUE) {
        size_t index;
        ASTNode *replacement;
        InlineRefActualEntry *ref_varg_entry;

        if (!node->child) return NULL;
        if (!inline_varg_index_from_node(node->child, &index)) {
            return inline_build_dynamic_varg_value(context, node, current_scope, state);
        }
        ref_varg_entry = inline_find_ref_varg_actual(state, index);
        if (ref_varg_entry) {
            return inline_clone_ref_varg_actual(context, node, current_scope, ref_varg_entry, state);
        }
        if (index < 1 || index > state->varg_count || !state->varg_symbols || !state->varg_symbols[index - 1]) return NULL;

        replacement = inline_create_symbol_node(context,
                                                current_scope,
                                                node,
                                                state->varg_symbols[index - 1],
                                                VAR_SYMBOL,
                                                1,
                                                0);
        if (!replacement) return NULL;
        ast_set_target_type(0,
                            replacement,
                            node->target_type,
                            node->target_dims,
                            node->target_dim_base,
                            node->target_dim_elements,
                            node->target_class);
        replacement->is_ref_arg = node->is_ref_arg;
        replacement->is_opt_arg = node->is_opt_arg;
        replacement->is_const_arg = node->is_const_arg;
        return replacement;
    }

    if (state && node->node_type == OP_ARG_IX_EXISTS) {
        size_t index;
        ASTNode *exists_node;

        if (!node->child) return NULL;
        if (!inline_varg_index_from_node(node->child, &index)) {
            return inline_build_dynamic_varg_exists(context, node, current_scope, state);
        }
        exists_node = inline_create_integer_constant(context,
                                                     node,
                                                     index <= state->varg_count ? 1 : 0,
                                                     TP_BOOLEAN);
        if (exists_node) exists_node->scope = current_scope;
        return exists_node;
    }

    new_node = ast_dup(context, node);
    if (!new_node) return NULL;

    node_scope = inline_prepare_cloned_node_scope(context, node, new_node, current_scope, state);
    if (!node_scope && inline_node_requires_local_scope(node)) return NULL;

    if (node->association) {
        mapped_association = inline_find_mapped_node(state, node->association);
        if (mapped_association) new_node->association = mapped_association;
        else if (node->node_type == FACTORY_CALL) {
            new_node->association = inline_clone_factory_selector_association(context, node, node_scope);
            if (!new_node->association) return NULL;
        } else {
            new_node->association = node->association;
        }
    }

    if (node->symbolNode && node->symbolNode->symbol) {
        mapped_symbol = inline_find_mapped_symbol(state, node->symbolNode->symbol);
        if (!mapped_symbol) mapped_symbol = node->symbolNode->symbol;
        sym_adnd(mapped_symbol, new_node, node->symbolNode->readUsage, node->symbolNode->writeUsage);
    }

    child = node->child;
    while (child) {
        ASTNode *cloned_child;

        cloned_child = inline_clone_subtree_in_scope(context, child, state, node_scope);
        if (!cloned_child) return NULL;
        add_ast(new_node, cloned_child);
        child = child->sibling;
    }

    return new_node;
}

static ASTNode *inline_clone_subtree(Context *context, ASTNode *node, InlineCloneState *state) {
    if (!state) return ast_dup_subtree(context, node);
    return inline_clone_subtree_in_scope(context, node, state, state->inline_scope);
}

static void inline_disconnect_subtree_symbols(ASTNode *node) {
    ASTNode *child;

    if (!node) return;

    child = node->child;
    while (child) {
        inline_disconnect_subtree_symbols(child);
        child = child->sibling;
    }

    if (node->symbolNode && node->symbolNode->symbol) {
        sym_dno(node->symbolNode->symbol, node);
    }
}

static size_t inline_count_siblings(ASTNode *node) {
    size_t count;

    count = 0;
    while (node) {
        count++;
        node = node->sibling;
    }

    return count;
}

static int inline_build_symbol_map(Scope *callee_scope,
                                   Scope *inline_scope,
                                   InlineCloneState *state) {
    if (!callee_scope || !inline_scope || !state) return 0;

    if (!state->symbol_entries) state->symbol_count = 0;
    if (!state->scope_entries) state->scope_count = 0;
    if (!state->node_entries) state->node_count = 0;
    state->callee_scope = callee_scope;
    state->inline_scope = inline_scope;
    inline_copy_numeric_context(inline_scope, callee_scope);

    if (!inline_append_scope_map_entry(state, callee_scope, inline_scope)) return 0;
    return inline_duplicate_scope_symbols(callee_scope, inline_scope, state);
}

static void inline_free_symbol_map(InlineCloneState *state) {
    size_t i;

    if (!state) return;
    if (state->symbol_entries) free(state->symbol_entries);
    if (state->scope_entries) free(state->scope_entries);
    if (state->node_entries) free(state->node_entries);
    if (state->ref_entries) {
        for (i = 0; i < state->ref_count; i++) {
            if (state->ref_entries[i].captured_symbols) free(state->ref_entries[i].captured_symbols);
        }
        free(state->ref_entries);
    }
    if (state->varg_ref_entries) {
        for (i = 0; i < state->varg_count; i++) {
            if (state->varg_ref_entries[i].captured_symbols) free(state->varg_ref_entries[i].captured_symbols);
        }
        free(state->varg_ref_entries);
    }
    if (state->varg_symbols) free(state->varg_symbols);
    state->symbol_entries = NULL;
    state->symbol_count = 0;
    state->scope_entries = NULL;
    state->scope_count = 0;
    state->node_entries = NULL;
    state->node_count = 0;
    state->ref_entries = NULL;
    state->ref_count = 0;
    state->varg_ref_entries = NULL;
    state->varg_symbols = NULL;
    state->varg_count = 0;
}

static void inline_copy_symbol_shape(Symbol *target, Symbol *source) {
    if (!target || !source) return;

    target->type = source->type;
    target->value_dims = source->value_dims;

    if (source->dim_base && source->value_dims) {
        target->dim_base = malloc(sizeof(int) * source->value_dims);
        memcpy(target->dim_base, source->dim_base, sizeof(int) * source->value_dims);
    }

    if (source->dim_elements && source->value_dims) {
        target->dim_elements = malloc(sizeof(int) * source->value_dims);
        memcpy(target->dim_elements, source->dim_elements, sizeof(int) * source->value_dims);
    }

    if (source->value_class) target->value_class = strdup(source->value_class);
}

static ASTNode *inline_create_sink_target(Context *context,
                                          Scope *inline_scope,
                                          ASTNode *source_node,
                                          ASTNode *shape_node) {
    char sink_name[64];
    ASTNode *sink_target;
    Symbol *sink_symbol;

    if (!context || !inline_scope || !source_node || !shape_node) return NULL;

    snprintf(sink_name, sizeof(sink_name), "__inline_unused_%d", source_node->node_number);

    sink_symbol = sym_fn(inline_scope, sink_name, strlen(sink_name));
    if (!sink_symbol) return NULL;

    sink_symbol->symbol_type = VARIABLE_SYMBOL;
    sink_symbol->status = SYM_STATUS_LOCAL_VAR;
    sink_symbol->register_num = UNSET_REGISTER;
    sink_symbol->register_type = 'r';
    sink_symbol->meta_emitted = 0;
    sink_symbol->init_emitted = 0;
    if (!inline_copy_node_shape(sink_symbol, shape_node)) return NULL;

    sink_target = ast_ftt(context, VAR_TARGET, strdup(sink_symbol->name));
    sink_target->free_node_string = 1;
    sink_target->scope = inline_scope;
    ast_copy_source_anchor(sink_target, source_node, AST_SOURCE_SYNTHETIC);
    sym_adnd(sink_symbol, sink_target, 0, 1);
    ast_svtp(sink_target, sink_symbol);

    return sink_target;
}

static InlineExprContext inline_classify_expr_context(ASTNode *node) {
    ASTNode *parent;

    if (!node) return 0;

    parent = node->parent;
    if (!parent) return 0;

    switch (parent->node_type) {
        case ASSIGN:
        case CALL:
            /* These use dedicated statement-position rewrites instead of
             * replacing the child call with a BLOCK_EXPR. */
            return INLINE_EXPR_CONTEXT_NONE;

        case FUNCTION:
        case FACTORY_CALL:
        case MEMBER_CALL:
            return inline_node_is_call_argument(node) ?
                   INLINE_EXPR_CONTEXT_EAGER_CALL_ARGUMENT :
                   INLINE_EXPR_CONTEXT_NONE;

        case IF:
        case WHILE:
        case UNTIL:
        case FOR:
        case TO:
        case BY:
            return inline_is_direct_single_value_consumer(node) ?
                   INLINE_EXPR_CONTEXT_CONTROL_CONSUMER :
                   INLINE_EXPR_CONTEXT_NONE;

        case OP_AND:
        case OP_OR:
            return inline_parent_is_short_circuit_operator(parent) ?
                   INLINE_EXPR_CONTEXT_SHORT_CIRCUIT_OPERATOR :
                   INLINE_EXPR_CONTEXT_NONE;

        case OP_TYPE_CAST:
        case OP_TYPE_IS:
        case OP_TYPEOF:
            /*
             * Type operators are direct value consumers: the first child is
             * evaluated before the cast/test/typeof operation, while any type
             * descriptor child is metadata-only for register purposes.
             */
            return parent->child == node ?
                   INLINE_EXPR_CONTEXT_EAGER_VALUE_CONSUMER :
                   INLINE_EXPR_CONTEXT_NONE;

        default:
            if (inline_parent_is_eager_operator(parent)) {
                return inline_eager_operator_context_is_safe(node) ?
                       INLINE_EXPR_CONTEXT_EAGER_OPERATOR :
                       INLINE_EXPR_CONTEXT_NONE;
            }
            return inline_is_direct_single_value_consumer(node) ?
                   INLINE_EXPR_CONTEXT_EAGER_VALUE_CONSUMER :
                   INLINE_EXPR_CONTEXT_NONE;
    }
}

static Symbol *inline_symbol_from_proc_def(ASTNode *proc_def) {
    return proc_def && proc_def->symbolNode ? proc_def->symbolNode->symbol : NULL;
}

static int inline_symbol_in_list(Symbol **symbols, size_t count, Symbol *symbol) {
    size_t i;

    if (!symbols || !symbol) return 0;

    for (i = 0; i < count; i++) {
        if (symbols[i] == symbol) return 1;
    }

    return 0;
}

static int inline_append_symbol(Symbol ***symbols_out, size_t *count_out, Symbol *symbol) {
    Symbol **new_symbols;

    if (!symbols_out || !count_out) return 0;
    if (!symbol) return 1;
    if (inline_symbol_in_list(*symbols_out, *count_out, symbol)) return 1;

    new_symbols = realloc(*symbols_out, sizeof(Symbol *) * (*count_out + 1));
    if (!new_symbols) return 0;

    *symbols_out = new_symbols;
    (*symbols_out)[*count_out] = symbol;
    (*count_out)++;
    return 1;
}

static int inline_subtree_reaches_targets(ASTNode *node,
                                          Symbol **targets,
                                          size_t target_count,
                                          Symbol ***visited,
                                          size_t *visited_count);

static int inline_symbol_reaches_targets(Symbol *start,
                                         Symbol **targets,
                                         size_t target_count,
                                         Symbol ***visited,
                                         size_t *visited_count) {
    ASTNode *instrs;

    if (inline_symbol_in_list(targets, target_count, start)) return 1;
    if (!start || !inline_symbol_has_callable_template(start)) return 1;
    if (inline_symbol_in_list(*visited, *visited_count, start)) return 0;
    if (!inline_append_symbol(visited, visited_count, start)) return 0;

    instrs = ast_chld(start->ast_template, INSTRUCTIONS, 0);
    return inline_subtree_reaches_targets(instrs, targets, target_count, visited, visited_count);
}

static int inline_subtree_reaches_targets(ASTNode *node,
                                          Symbol **targets,
                                          size_t target_count,
                                          Symbol ***visited,
                                          size_t *visited_count) {
    ASTNode *child;
    Symbol *callee_symbol;

    if (!node) return 0;
    if (inline_node_is_callable_def(node)) return 0;

    if ((node->node_type == FUNCTION ||
         node->node_type == MEMBER_CALL ||
         node->node_type == FACTORY_CALL) &&
        node->symbolNode &&
        node->symbolNode->symbol &&
        node->symbolNode->symbol->is_inlinable &&
        inline_symbol_has_callable_template(node->symbolNode->symbol)) {
        callee_symbol = node->symbolNode->symbol;
        if (inline_symbol_reaches_targets(callee_symbol, targets, target_count, visited, visited_count)) {
            return 1;
        }
    }

    child = node->child;
    while (child) {
        if (inline_subtree_reaches_targets(child, targets, target_count, visited, visited_count)) return 1;
        child = child->sibling;
    }

    return 0;
}

static int inline_call_is_recursive(ASTNode *call_node, Symbol *proc_sym) {
    ASTNode *enclosing_proc;
    Scope *scope;
    Symbol **targets;
    Symbol **visited;
    size_t target_count;
    size_t visited_count;
    int is_recursive;

    if (!call_node || !proc_sym || !proc_sym->ast_template) return 1;

    targets = NULL;
    visited = NULL;
    target_count = 0;
    visited_count = 0;
    is_recursive = 0;

    enclosing_proc = ast_proc(call_node);
    if (!inline_append_symbol(&targets, &target_count, inline_symbol_from_proc_def(enclosing_proc))) {
        free(targets);
        return 1;
    }

    scope = call_node->scope;
    while (scope) {
        ASTNode *scope_node;
        Symbol *scope_symbol;

        scope_node = scope->defining_node;
        scope_symbol = NULL;
        if (scope_node &&
            scope_node->association &&
            inline_node_is_callable_def(scope_node->association)) {
            scope_symbol = inline_symbol_from_proc_def(scope_node->association);
        }

        if (scope_symbol && !inline_append_symbol(&targets, &target_count, scope_symbol)) {
            free(targets);
            free(visited);
            return 1;
        }

        scope = scope->parent;
    }

    if (target_count > 0) {
        is_recursive = inline_symbol_reaches_targets(proc_sym, targets, target_count, &visited, &visited_count);
    }

    free(targets);
    free(visited);
    return is_recursive;
}

static int inline_callable_writes_class_attribute(Symbol *start,
                                                  Symbol ***visited,
                                                  size_t *visited_count);

static int inline_subtree_writes_class_attribute(ASTNode *node,
                                                 Symbol ***visited,
                                                 size_t *visited_count) {
    ASTNode *child;

    if (!node) return 0;
    if (inline_node_is_callable_def(node)) return 0;

    if ((node->node_type == VAR_TARGET || node->node_type == VAR_REFERENCE) &&
        node->symbolNode &&
        inline_symbol_is_class_attribute(node->symbolNode->symbol)) {
        return 1;
    }

    if ((node->node_type == FUNCTION ||
         node->node_type == MEMBER_CALL ||
         node->node_type == FACTORY_CALL) &&
        node->symbolNode &&
        node->symbolNode->symbol &&
        node->symbolNode->symbol->ast_template &&
        inline_callable_writes_class_attribute(node->symbolNode->symbol, visited, visited_count)) {
        return 1;
    }

    child = node->child;
    while (child) {
        if (inline_subtree_writes_class_attribute(child, visited, visited_count)) return 1;
        child = child->sibling;
    }

    return 0;
}

static int inline_callable_writes_class_attribute(Symbol *start,
                                                  Symbol ***visited,
                                                  size_t *visited_count) {
    ASTNode *instrs;

    if (!start) return 0;
    if (!inline_symbol_has_callable_template(start)) return 1;
    if (inline_symbol_in_list(*visited, *visited_count, start)) return 0;
    if (!inline_append_symbol(visited, visited_count, start)) return 1;

    instrs = ast_chld(start->ast_template, INSTRUCTIONS, 0);
    return inline_subtree_writes_class_attribute(instrs, visited, visited_count);
}

static int inline_method_writes_class_attribute(ASTNode *proc_def) {
    Symbol *proc_symbol;

    if (!proc_def || proc_def->node_type != METHOD) return 0;

    proc_symbol = inline_symbol_from_proc_def(proc_def);
    if (!proc_symbol) return 0;

    return inline_symbol_writes_class_attribute(proc_symbol);
}

static int inline_symbol_writes_class_attribute(Symbol *symbol) {
    Symbol **visited;
    size_t visited_count;
    int result;

    if (!symbol) return 0;
    visited = NULL;
    visited_count = 0;
    result = inline_callable_writes_class_attribute(symbol, &visited, &visited_count);
    free(visited);
    return result;
}

static int inline_validate_call_site(Context *context,
                                     ASTNode *proc_def,
                                     ASTNode *call_node,
                                     Symbol *proc_sym) {
    int unsupported_varg_access;
    size_t max_required_varg_index;
    size_t varg_count;

    if (!proc_def || !call_node || !proc_sym) return 0;
    if (inline_call_is_recursive(call_node, proc_sym)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "recursive inline cycle detected");
        return 0;
    }
    if (!inline_call_arity_matches(call_node, proc_sym, &varg_count)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "call arity does not match formal arguments");
        return 0;
    }
    if (!proc_sym->has_vargs) return 1;

    if (!inline_analyse_varg_usage(proc_def, &unsupported_varg_access, &max_required_varg_index)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to analyse vararg usage");
        return 0;
    }
    if (unsupported_varg_access) {
        inline_debug_fail_closed(context, call_node, proc_sym, "unsupported vararg access in callee");
        return 0;
    }
    if (varg_count < max_required_varg_index) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "call provides %zu varargs but callee requires index %zu",
                                 varg_count, max_required_varg_index);
        return 0;
    }

    return 1;
}

static int inline_analyse_return_shape(ASTNode *proc_def, InlineReturnShape *shape_out) {
    ASTNode *instrs;
    ASTNode *instr;
    ASTNode *last_instr;
    InlineReturnShape shape;

    if (!proc_def) return 0;

    instrs = ast_chld(proc_def, INSTRUCTIONS, 0);
    if (!instrs) return 0;

    shape.return_count = 0;
    shape.top_level_return_count = 0;
    shape.final_is_return = 0;

    instr = instrs->child;
    last_instr = NULL;
    while (instr) {
        if (instr->node_type == RETURN) shape.top_level_return_count++;
        last_instr = instr;
        instr = instr->sibling;
    }

    shape.return_count = inline_count_return_nodes(instrs->child);
    shape.final_is_return = last_instr && last_instr->node_type == RETURN;

    if (shape_out) *shape_out = shape;
    return 1;
}

static int inline_count_return_nodes(ASTNode *node) {
    int count;

    count = 0;
    while (node) {
        if (node->node_type == RETURN) count++;
        if (node->child) count += inline_count_return_nodes(node->child);
        node = node->sibling;
    }

    return count;
}

static ASTNode *inline_create_receiver_copyback_leave_wrapper(Context *context,
                                                              ASTNode *leave_node,
                                                              ASTNode *block_expr,
                                                              Scope *inline_scope,
                                                              InlineCloneState *clone_state) {
    ASTNode *wrapper;
    ASTNode *leave_expr;
    ASTNode *assign_node;
    ASTNode *assign_lhs;
    ASTNode *temp_ref;
    Symbol *temp_symbol;

    if (!context || !leave_node || !block_expr || !inline_scope || !clone_state) return NULL;
    if (leave_node->node_type != LEAVE_WITH) return NULL;
    if (!clone_state->method_receiver_needs_copyback) return NULL;

    wrapper = ast_f(context, INSTRUCTIONS, leave_node->token);
    if (!wrapper) return NULL;
    ast_copy_source_anchor(wrapper, leave_node, AST_SOURCE_SYNTHETIC);
    ast_mark_compiler_generated_block(wrapper);
    wrapper->association = block_expr;
    wrapper->scope = inline_scope;
    wrapper->value_type = TP_VOID;
    wrapper->target_type = TP_VOID;

    leave_expr = leave_node->child;
    if (!leave_expr) return NULL;

    temp_symbol = inline_create_temp_symbol(context,
                                            inline_scope,
                                            leave_expr,
                                            "__inline_leave",
                                            (size_t)leave_node->node_number);
    if (!temp_symbol) return NULL;

    assign_node = ast_f(context, ASSIGN, leave_expr->token ? leave_expr->token : leave_node->token);
    if (!assign_node) return NULL;
    assign_node->scope = inline_scope;
    assign_node->value_type = leave_expr->value_type;
    assign_node->target_type = leave_expr->target_type;

    assign_lhs = inline_create_symbol_node(context,
                                           inline_scope,
                                           leave_expr,
                                           temp_symbol,
                                           VAR_TARGET,
                                           0,
                                           1);
    if (!assign_lhs) return NULL;

    leave_node->child = NULL;
    leave_expr->parent = NULL;
    leave_expr->sibling = NULL;

    add_ast(assign_node, assign_lhs);
    add_ast(assign_node, leave_expr);
    add_ast(wrapper, assign_node);

    if (!inline_append_method_receiver_copyback(context,
                                                wrapper,
                                                inline_scope,
                                                leave_node,
                                                clone_state)) {
        return NULL;
    }

    temp_ref = inline_create_symbol_node(context,
                                         inline_scope,
                                         leave_node,
                                         temp_symbol,
                                         VAR_SYMBOL,
                                         1,
                                         0);
    if (!temp_ref) return NULL;

    add_ast(leave_node, temp_ref);
    return wrapper;
}

static int inline_rewrite_return_nodes(Context *context,
                                       ASTNode **node_ref,
                                       ASTNode *block_expr,
                                       Scope *inline_scope,
                                       int allow_dummy_return,
                                       ValueType proc_type,
                                       InlineCloneState *clone_state) {
    ASTNode *node;
    ASTNode *child;
    ASTNode *next_child;
    ASTNode *leave_expr;

    if (!node_ref || !*node_ref) return 1;

    node = *node_ref;

    child = node->child;
    while (child) {
        next_child = child->sibling;
        if (!inline_rewrite_return_nodes(context,
                                         &child,
                                         block_expr,
                                         inline_scope,
                                         allow_dummy_return,
                                         proc_type,
                                         clone_state)) {
            return 0;
        }
        child = next_child;
    }

    if (node->node_type != RETURN) return 1;

    if (!node->child) {
        if (!(allow_dummy_return && proc_type == TP_VOID)) return 0;

        leave_expr = inline_create_integer_constant(context, node, 0, TP_INTEGER);
        if (!leave_expr) return 0;
        leave_expr->scope = node->scope ? node->scope : inline_scope;
        add_ast(node, leave_expr);
    }

    node->node_type = LEAVE_WITH;
    node->node_string = "leave";
    node->node_string_length = 5;
    node->free_node_string = 0;
    node->association = block_expr;
    node->value_type = node->child ? node->child->value_type : TP_VOID;
    node->target_type = node->child ? node->child->target_type : TP_VOID;

    if (clone_state && clone_state->method_receiver_needs_copyback) {
        ASTNode *wrapper;

        wrapper = inline_create_receiver_copyback_leave_wrapper(context,
                                                                node,
                                                                block_expr,
                                                                inline_scope,
                                                                clone_state);
        if (!wrapper) return 0;

        if (node->parent) ast_rpl(node, wrapper);
        else *node_ref = wrapper;
        add_ast(wrapper, node);
    }

    return 1;
}

static ASTNode *inline_build_block_expr(Context *context,
                                        ASTNode *call_node,
                                        Symbol *proc_sym,
                                        Scope *parent_scope,
                                        int allow_dummy_return) {
    ASTNode *proc_def;
    ASTNode *proc_instrs;
    ASTNode *block_expr;
    ASTNode *instr_list;
    ASTNode *proc_instr;
    Scope *inline_scope;
    InlineCloneState clone_state;
    InlineReturnShape return_shape;

    if (!context || !call_node || !proc_sym || !proc_sym->ast_template || !parent_scope) return NULL;

    proc_def = proc_sym->ast_template;
    if (!proc_def || !proc_def->scope) {
        inline_debug_fail_closed(context, call_node, proc_sym, "callee has no inlineable procedure scope");
        return NULL;
    }

    if (!inline_validate_call_site(context, proc_def, call_node, proc_sym)) return NULL;

    block_expr = ast_dup(context, call_node);
    if (!block_expr) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to duplicate call node for BLOCK_EXPR");
        return NULL;
    }

    block_expr->node_type = BLOCK_EXPR;
    block_expr->node_string = "do";
    block_expr->node_string_length = 2;
    block_expr->free_node_string = 0;
    block_expr->association = proc_def;

    if (allow_dummy_return && proc_sym->type == TP_VOID) {
        ast_set_value_type(0, block_expr, TP_INTEGER, 0, 0, 0, 0);
        ast_set_target_type(0, block_expr, TP_INTEGER, 0, 0, 0, 0);
    }

    inline_scope = scp_f(context, parent_scope, block_expr, NULL, SCOPE_LOCAL);
    if (!inline_scope) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to create BLOCK_EXPR inline scope");
        return NULL;
    }
    block_expr->scope = inline_scope;

    instr_list = ast_f(context, INSTRUCTIONS, call_node->token);
    if (!instr_list) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to create inline instruction list");
        return NULL;
    }
    instr_list->scope = inline_scope;
    instr_list->value_type = TP_VOID;
    instr_list->target_type = TP_VOID;
    add_ast(block_expr, instr_list);

    memset(&clone_state, 0, sizeof(clone_state));

    if (!inline_build_symbol_map(proc_def->scope, inline_scope, &clone_state)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to build inline symbol/scope map");
        inline_free_symbol_map(&clone_state);
        return NULL;
    }

    if (!inline_bind_call_arguments(context, instr_list, inline_scope, proc_def, call_node, proc_sym, &clone_state)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to bind inline call arguments");
        inline_free_symbol_map(&clone_state);
        return NULL;
    }

    proc_instrs = ast_chld(proc_def, INSTRUCTIONS, 0);
    if (!proc_instrs) {
        inline_debug_fail_closed(context, call_node, proc_sym, "callee has no instruction list");
        inline_free_symbol_map(&clone_state);
        return NULL;
    }

    proc_instr = proc_instrs->child;
    while (proc_instr) {
        ASTNode *cloned_instr;

        cloned_instr = inline_clone_subtree(context, proc_instr, &clone_state);
        if (!cloned_instr) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to clone callee instruction subtree");
            inline_free_symbol_map(&clone_state);
            return NULL;
        }
        if (!inline_rewrite_return_nodes(context,
                                         &cloned_instr,
                                         block_expr,
                                         inline_scope,
                                         allow_dummy_return,
                                         proc_sym->type,
                                         &clone_state)) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to rewrite return nodes for BLOCK_EXPR inline");
            inline_free_symbol_map(&clone_state);
            return NULL;
        }
        add_ast(instr_list, cloned_instr);

        proc_instr = proc_instr->sibling;
    }

    if (allow_dummy_return && proc_sym->type == TP_VOID &&
        inline_analyse_return_shape(proc_def, &return_shape) &&
        !return_shape.final_is_return) {
        ASTNode *leave_with;
        ASTNode *leave_expr;

        leave_expr = inline_create_integer_constant(context, call_node, 0, TP_INTEGER);
        if (!leave_expr) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create dummy LEAVE_WITH expression");
            inline_free_symbol_map(&clone_state);
            return NULL;
        }
        leave_expr->scope = inline_scope;

        leave_with = ast_f(context, LEAVE_WITH, call_node->token);
        if (!leave_with) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create dummy LEAVE_WITH node");
            inline_free_symbol_map(&clone_state);
            return NULL;
        }
        leave_with->scope = inline_scope;
        leave_with->association = block_expr;
        leave_with->value_type = leave_expr->value_type;
        leave_with->target_type = leave_expr->target_type;

        add_ast(leave_with, leave_expr);
        add_ast(instr_list, leave_with);
    }

    inline_free_symbol_map(&clone_state);
    return block_expr;
}

static int ast_inline_statement(Context *context,
                                ASTNode *statement_node,
                                ASTNode *call_node,
                                Symbol *proc_sym,
                                InlineReturnPlan *return_plan) {
    ASTNode *proc_def;
    ASTNode *proc_instrs;
    ASTNode *block;
    ASTNode *instr_list;
    ASTNode *proc_instr;
    Scope *inline_scope;
    InlineCloneState clone_state;

    if (!context || !statement_node || !call_node || !proc_sym || !proc_sym->ast_template) return 0;

    proc_def = proc_sym->ast_template;
    if (!proc_def || !proc_def->scope) {
        inline_debug_fail_closed(context, call_node, proc_sym, "callee has no inlineable procedure scope");
        return 0;
    }

    if (!inline_validate_call_site(context, proc_def, call_node, proc_sym)) return 0;

    block = ast_f(context, INSTRUCTIONS, call_node->token);
    if (!block) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to create compiler-generated statement block");
        return 0;
    }
    ast_copy_source_anchor(block, statement_node, AST_SOURCE_SYNTHETIC);
    ast_mark_compiler_generated_block(block);
    ast_enable_primary_reporting_anchor(block);
    block->association = proc_def;
    block->value_type = TP_VOID;
    block->target_type = TP_VOID;

    inline_scope = scp_f(context, statement_node->scope, block, NULL, SCOPE_LOCAL);
    if (!inline_scope) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to create compiler-generated statement scope");
        return 0;
    }
    instr_list = block;

    memset(&clone_state, 0, sizeof(clone_state));

    if (!inline_build_symbol_map(proc_def->scope, inline_scope, &clone_state)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to build inline symbol/scope map");
        return 0;
    }

    if (!inline_bind_call_arguments(context, instr_list, inline_scope, proc_def, call_node, proc_sym, &clone_state)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to bind inline call arguments");
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    proc_instrs = ast_chld(proc_def, INSTRUCTIONS, 0);
    if (!proc_instrs) {
        inline_debug_fail_closed(context, call_node, proc_sym, "callee has no instruction list");
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    proc_instr = proc_instrs->child;
    while (proc_instr) {
        if (proc_instr->node_type == RETURN) {
            ASTNode *ret_expr;
            ASTNode *ret_assign;
            ASTNode *ret_lhs;
            ASTNode *ret_rhs;

            ret_expr = proc_instr->child;
            if (!ret_expr) {
                proc_instr = proc_instr->sibling;
                continue;
            }

            ret_assign = ast_f(context, ASSIGN, proc_instr->token);

            if (return_plan && return_plan->return_target) {
                Scope *caller_scope;

                caller_scope = call_node->scope ? call_node->scope :
                               (return_plan->return_target->scope ?
                                return_plan->return_target->scope :
                                inline_scope);
                ret_assign->scope = caller_scope;
                ret_lhs = inline_clone_subtree_in_scope(context,
                                                        return_plan->return_target,
                                                        &clone_state,
                                                        caller_scope);
            } else if (return_plan && return_plan->return_sink_symbol) {
                ret_assign->scope = inline_scope;
                ret_lhs = inline_create_sink_target(context, inline_scope, proc_instr, proc_instr->child);
            } else {
                inline_debug_fail_closed(context, call_node, proc_sym, "missing return target/sink during statement inline");
                inline_free_symbol_map(&clone_state);
                return 0;
            }

            if (!ret_lhs) {
                inline_debug_fail_closed(context, call_node, proc_sym, "failed to build return assignment target");
                inline_free_symbol_map(&clone_state);
                return 0;
            }

            if (inline_node_has_array_shape(ret_expr) ||
                (inline_node_needs_attr_copy(ret_expr) &&
                 (ret_expr->value_type == TP_BINARY || ret_expr->target_type == TP_BINARY))) {
                ASTNode *ret_copy;
                ASTNode *attr_copy;

                ret_rhs = inline_clone_subtree(context, ret_expr, &clone_state);
                if (!ret_rhs) {
                    inline_debug_fail_closed(context, call_node, proc_sym, "failed to clone aggregate return expression");
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }

                if (!inline_is_direct_symbol_actual(ret_expr)) {
                    ret_rhs = inline_create_temp_value_ref(context,
                                                           instr_list,
                                                           inline_scope,
                                                           ret_expr,
                                                           &clone_state,
                                                           "__inline_ret",
                                                           0);
                }

                if (!ret_rhs) {
                    inline_debug_fail_closed(context, call_node, proc_sym, "failed to materialise aggregate return temp");
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }

                ret_copy = inline_create_register_copy_instr(context, inline_scope, "copy", ret_lhs, ret_rhs);
                attr_copy = inline_create_register_copy_instr(context, inline_scope, "acopy", ret_lhs, ret_rhs);
                if (!ret_copy || !attr_copy) {
                    inline_debug_fail_closed(context, call_node, proc_sym, "failed to create aggregate return copy instructions");
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }
                add_ast(instr_list, ret_copy);
                add_ast(instr_list, attr_copy);
            } else {
                ret_rhs = inline_clone_subtree(context, ret_expr, &clone_state);
                if (!ret_rhs) {
                    inline_debug_fail_closed(context, call_node, proc_sym, "failed to clone scalar return expression");
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }
                add_ast(ret_assign, ret_lhs);
                add_ast(ret_assign, ret_rhs);
                add_ast(instr_list, ret_assign);
            }
        } else {
            ASTNode *cloned_instr;

            cloned_instr = inline_clone_subtree(context, proc_instr, &clone_state);
            if (!cloned_instr) {
                inline_debug_fail_closed(context, call_node, proc_sym, "failed to clone statement instruction subtree");
                inline_free_symbol_map(&clone_state);
                return 0;
            }
            add_ast(instr_list, cloned_instr);
        }

        proc_instr = proc_instr->sibling;
    }

    if (!inline_append_method_receiver_copyback(context,
                                                instr_list,
                                                inline_scope,
                                                call_node,
                                                &clone_state)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to append method receiver copyback");
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    ast_rpl(statement_node, block);
    inline_disconnect_subtree_symbols(statement_node);
    inline_free_symbol_map(&clone_state);

    return 1;
}

static int ast_inline_assignment(Context *context, ASTNode *assign_node, ASTNode *call_node, Symbol *proc_sym) {
    ASTNode *lhs;
    ASTNode *block_expr;
    ASTNode *proc_def;
    InlineReturnShape return_shape;
    InlineReturnPlan return_plan;
    int method_needs_receiver_copyback;

    if (!assign_node || !call_node) return 0;

    lhs = assign_node->child;
    if (!lhs || lhs->node_type != VAR_TARGET) {
        inline_debug_fail_closed(context, call_node, proc_sym, "assignment inline requires a plain VAR_TARGET lhs");
        return 0;
    }

    memset(&return_plan, 0, sizeof(return_plan));
    return_plan.return_target = lhs;

    proc_def = proc_sym ? proc_sym->ast_template : NULL;
    if (!proc_def || !inline_analyse_return_shape(proc_def, &return_shape)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to analyse callee return shape for assignment inline");
        return 0;
    }
    method_needs_receiver_copyback = inline_method_writes_class_attribute(proc_def);
    if (proc_def->node_type == METHOD &&
        inline_symbol_uses_imported_template(proc_sym) &&
        !inline_is_direct_symbol_actual(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "imported method assignment inline requires a direct receiver");
        return 0;
    }
    if (method_needs_receiver_copyback &&
        proc_def->node_type == METHOD &&
        !inline_is_direct_receiver_copyback_target(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "mutating method assignment inline requires a direct receiver copyback target");
        return 0;
    }
    if (!return_shape.final_is_return || return_shape.return_count == 0) {
        inline_debug_fail_closed(context, call_node, proc_sym, "assignment inline requires a final value RETURN");
        return 0;
    }
    if ((assign_node->parent && assign_node->parent->node_type == REPEAT) ||
        lhs->child ||
        (proc_sym && proc_sym->value_dims > 0)) {
        if (method_needs_receiver_copyback) {
            inline_debug_fail_closed(context, call_node, proc_sym,
                                     "mutating method assignment inline requires statement-position copyback");
            return 0;
        }
        block_expr = inline_build_block_expr(context, call_node, proc_sym, assign_node->scope, 0);
        if (!block_expr) return 0;
        ast_rpl(call_node, block_expr);
        inline_disconnect_subtree_symbols(call_node);
        return 1;
    }
    if (return_shape.return_count != 1) {
        if (method_needs_receiver_copyback) {
            inline_debug_fail_closed(context, call_node, proc_sym,
                                     "mutating method multi-return assignment inline requires statement-position copyback");
            return 0;
        }
        block_expr = inline_build_block_expr(context, call_node, proc_sym, assign_node->scope, 0);
        if (!block_expr) return 0;
        ast_rpl(call_node, block_expr);
        inline_disconnect_subtree_symbols(call_node);
        return 1;
    }

    return ast_inline_statement(context, assign_node, call_node, proc_sym, &return_plan);
}

static int ast_inline_call(Context *context, ASTNode *call_stmt, ASTNode *call_node, Symbol *proc_sym) {
    ASTNode *proc_def;
    ASTNode *block;
    Scope *block_scope;
    ASTNode *block_expr;
    ASTNode *sink_assign;
    ASTNode *sink_lhs;
    InlineReturnShape return_shape;
    InlineReturnPlan return_plan;
    int method_needs_receiver_copyback;

    proc_def = proc_sym ? proc_sym->ast_template : NULL;
    if (!proc_def || !inline_analyse_return_shape(proc_def, &return_shape)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to analyse callee return shape for call inline");
        return 0;
    }
    if (return_shape.return_count == 0) {
        if (proc_sym->type != TP_VOID) {
            inline_debug_fail_closed(context, call_node, proc_sym, "value-returning callee has no RETURN");
            return 0;
        }
    } else if (!return_shape.final_is_return) {
        if (proc_sym->type != TP_VOID) {
            inline_debug_fail_closed(context, call_node, proc_sym, "call inline requires a final RETURN for value-producing callees");
            return 0;
        }
    }

    method_needs_receiver_copyback = inline_method_writes_class_attribute(proc_def);
    if (proc_def->node_type == METHOD &&
        inline_symbol_uses_imported_template(proc_sym) &&
        !inline_is_direct_symbol_actual(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "imported method call inline requires a direct receiver");
        return 0;
    }
    if (method_needs_receiver_copyback &&
        proc_def->node_type == METHOD &&
        !inline_is_direct_receiver_copyback_target(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "mutating method call inline requires a direct receiver copyback target");
        return 0;
    }
    if ((proc_sym->type == TP_VOID && (return_shape.return_count != 1 || !return_shape.final_is_return)) ||
        (proc_sym->type != TP_VOID && return_shape.return_count != 1)) {
        if (method_needs_receiver_copyback) {
            inline_debug_fail_closed(context, call_node, proc_sym,
                                     "mutating method call inline requires statement-position copyback");
            return 0;
        }
        block = ast_f(context, INSTRUCTIONS, call_node->token);
        if (!block) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create compiler-generated sink block");
            return 0;
        }
        ast_copy_source_anchor(block, call_stmt, AST_SOURCE_SYNTHETIC);
        ast_mark_compiler_generated_block(block);
        ast_enable_primary_reporting_anchor(block);
        block->association = proc_def;
        block->value_type = TP_VOID;
        block->target_type = TP_VOID;

        block_scope = scp_f(context, call_stmt->scope, block, NULL, SCOPE_LOCAL);
        if (!block_scope) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create sink block scope");
            return 0;
        }

        block_expr = inline_build_block_expr(context, call_node, proc_sym, block_scope, 1);
        if (!block_expr) return 0;

        sink_assign = ast_f(context, ASSIGN, call_node->token);
        if (!sink_assign) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create sink assignment");
            return 0;
        }
        sink_assign->scope = block_scope;
        sink_assign->value_type = block_expr->value_type;
        sink_assign->target_type = block_expr->target_type;

        sink_lhs = inline_create_sink_target(context, block_scope, call_node, block_expr);
        if (!sink_lhs) {
            inline_debug_fail_closed(context, call_node, proc_sym, "failed to create unused return sink target");
            return 0;
        }

        add_ast(sink_assign, sink_lhs);
        add_ast(sink_assign, block_expr);
        add_ast(block, sink_assign);

        ast_rpl(call_stmt, block);
        inline_disconnect_subtree_symbols(call_stmt);
        return 1;
    }

    memset(&return_plan, 0, sizeof(return_plan));
    return_plan.return_sink_symbol = proc_sym;

    return ast_inline_statement(context, call_stmt, call_node, proc_sym, &return_plan);
}

static int ast_inline_expression(Context *context, ASTNode *call_node, Symbol *proc_sym) {
    ASTNode *block_expr;
    InlineExprContext expr_context;
    InlineReturnShape return_shape;

    if (!context || !call_node || !proc_sym || !proc_sym->ast_template) return 0;

    expr_context = inline_classify_expr_context(call_node);
    if (expr_context == INLINE_EXPR_CONTEXT_NONE) {
        ASTNode *parent;

        parent = call_node->parent;
        if (parent && (parent->node_type == ASSIGN || parent->node_type == CALL)) {
            inline_debug_fail_closed(context, call_node, proc_sym,
                                     "expression context belongs to a dedicated statement rewrite");
        } else {
            inline_debug_fail_closed(context, call_node, proc_sym,
                                     "BLOCK_EXPR expression inline requires a direct single-value consumer");
        }
        return 0;
    }
    if (!inline_analyse_return_shape(proc_sym->ast_template, &return_shape)) {
        inline_debug_fail_closed(context, call_node, proc_sym, "failed to analyse callee return shape for expression inline");
        return 0;
    }
    if (inline_method_writes_class_attribute(proc_sym->ast_template) &&
        proc_sym->ast_template->node_type == METHOD &&
        !inline_is_direct_receiver_copyback_target(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "mutating method expression inline requires a direct receiver copyback target");
        return 0;
    }
    if (proc_sym->ast_template->node_type == METHOD &&
        inline_symbol_uses_imported_template(proc_sym) &&
        !inline_is_direct_symbol_actual(inline_call_receiver(call_node))) {
        inline_debug_fail_closed(context, call_node, proc_sym,
                                 "imported method expression inline requires a direct receiver");
        return 0;
    }
    if (!return_shape.final_is_return || return_shape.return_count == 0) {
        inline_debug_fail_closed(context, call_node, proc_sym, "expression inline requires a final value RETURN");
        return 0;
    }

    block_expr = inline_build_block_expr(context, call_node, proc_sym, call_node->scope, 0);
    if (!block_expr) return 0;

    ast_rpl(call_node, block_expr);
    inline_disconnect_subtree_symbols(call_node);

    return 1;
}

static int ast_inline_rhs_eager_operator(Context *context,
                                         ASTNode *op_node,
                                         ASTNode *rhs_call,
                                         Symbol *proc_sym) {
    ASTNode *left;
    ASTNode *block_expr;
    ASTNode *instr_list;
    ASTNode *assign_node;
    ASTNode *assign_lhs;
    ASTNode *assign_rhs;
    ASTNode *leave_node;
    ASTNode *op_expr;
    ASTNode *temp_ref;
    ASTNode *rhs_expr;
    Scope *parent_scope;
    Scope *inline_scope;
    Symbol *left_symbol;
    InlineCloneState clone_state;

    if (!context || !op_node || !rhs_call) return 0;
    if (!inline_parent_is_eager_operator(op_node)) return 0;

    left = op_node->child;
    if (!left || left->sibling != rhs_call || rhs_call->sibling) return 0;

    parent_scope = op_node->scope ? op_node->scope :
                   (rhs_call->scope ? rhs_call->scope : left->scope);
    if (!parent_scope) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "RHS eager-operator inline requires a parent scope");
        return 0;
    }

    block_expr = ast_dup(context, op_node);
    if (!block_expr) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to create RHS eager-operator BLOCK_EXPR");
        return 0;
    }
    block_expr->node_type = BLOCK_EXPR;
    block_expr->node_string = "do";
    block_expr->node_string_length = 2;
    block_expr->free_node_string = 0;

    inline_scope = scp_f(context, parent_scope, block_expr, NULL, SCOPE_LOCAL);
    if (!inline_scope) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to create RHS eager-operator inline scope");
        return 0;
    }
    block_expr->scope = inline_scope;

    instr_list = ast_f(context, INSTRUCTIONS, op_node->token);
    if (!instr_list) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to create RHS eager-operator instruction list");
        return 0;
    }
    instr_list->scope = inline_scope;
    instr_list->value_type = TP_VOID;
    instr_list->target_type = TP_VOID;
    add_ast(block_expr, instr_list);

    left_symbol = inline_create_temp_symbol(context,
                                            inline_scope,
                                            left,
                                            "__inline_lhs",
                                            (size_t)op_node->node_number);
    if (!left_symbol) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to create RHS eager-operator left temp");
        return 0;
    }

    memset(&clone_state, 0, sizeof(clone_state));
    clone_state.inline_scope = inline_scope;

    assign_rhs = inline_clone_subtree_in_scope(context, left, &clone_state, inline_scope);
    if (!assign_rhs) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to clone RHS eager-operator left operand");
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    rhs_expr = inline_clone_subtree_in_scope(context, rhs_call, &clone_state, inline_scope);
    if (!rhs_expr) {
        inline_debug_fail_closed(context, rhs_call, proc_sym,
                                 "failed to clone RHS eager-operator right operand");
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    assign_node = ast_f(context, ASSIGN, left->token ? left->token : op_node->token);
    if (!assign_node) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }
    assign_node->scope = inline_scope;
    assign_node->value_type = assign_rhs->value_type;
    assign_node->target_type = assign_rhs->target_type;

    assign_lhs = inline_create_symbol_node(context,
                                           inline_scope,
                                           left,
                                           left_symbol,
                                           VAR_TARGET,
                                           0,
                                           1);
    if (!assign_lhs) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    add_ast(assign_node, assign_lhs);
    add_ast(assign_node, assign_rhs);
    add_ast(instr_list, assign_node);

    op_expr = ast_dup(context, op_node);
    if (!op_expr) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }
    op_expr->scope = inline_scope;

    temp_ref = inline_create_symbol_node(context,
                                         inline_scope,
                                         left,
                                         left_symbol,
                                         VAR_SYMBOL,
                                         1,
                                         0);
    if (!temp_ref) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    add_ast(op_expr, temp_ref);
    add_ast(op_expr, rhs_expr);

    leave_node = ast_f(context, LEAVE_WITH, op_node->token);
    if (!leave_node) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }
    leave_node->scope = inline_scope;
    leave_node->association = block_expr;
    leave_node->value_type = op_expr->value_type;
    leave_node->target_type = op_expr->target_type;

    add_ast(leave_node, op_expr);
    add_ast(instr_list, leave_node);

    ast_rpl(op_node, block_expr);
    inline_disconnect_subtree_symbols(op_node);
    inline_free_symbol_map(&clone_state);

    return 1;
}

/* Walker to find statement-shaped call sites and inline them */
walker_result inline_procedure_walker(walker_direction direction, ASTNode *node, void *payload) {
    InlineWalkerPayload *inline_payload;
    Context *context;
    ASTNode *call_node;
    ASTNode *lhs;
    ASTNode *rhs;
    Symbol *proc_sym;

    inline_payload = (InlineWalkerPayload *)payload;
    context = inline_payload ? inline_payload->context : NULL;

    if (direction == in) return result_normal;

    if (inline_parent_is_eager_operator(node)) {
        lhs = node->child;
        rhs = lhs ? lhs->sibling : NULL;

        if (rhs && !rhs->sibling &&
            inline_node_is_inlineable_call(rhs, &proc_sym) &&
            inline_rhs_eager_operator_needs_left_capture(rhs)) {
            if (ast_inline_rhs_eager_operator(context, node, rhs, proc_sym) && inline_payload) {
                inline_payload->changed = 1;
            }
        }
        return result_normal;
    }

    if (node->node_type == FUNCTION ||
        node->node_type == MEMBER_CALL ||
        node->node_type == FACTORY_CALL) {
        if (inline_node_is_inlineable_call(node, &proc_sym)) {
            if (!inline_rhs_eager_operator_needs_left_capture(node) &&
                ast_inline_expression(context, node, proc_sym) && inline_payload) {
                inline_payload->changed = 1;
            }
        }
        return result_normal;
    }

    if (node->node_type == ASSIGN) {
        lhs = node->child;
        rhs = lhs ? lhs->sibling : NULL;

        if (!lhs || !rhs) return result_normal;

        if (inline_node_is_inlineable_call(rhs, &proc_sym)) {
            if (ast_inline_assignment(context, node, rhs, proc_sym) && inline_payload) inline_payload->changed = 1;
        }
        return result_normal;
    }

    if (node->node_type != CALL) return result_normal;

    call_node = node->child;

    if (inline_node_is_inlineable_call(call_node, &proc_sym)) {
        if (ast_inline_call(context, node, call_node, proc_sym) && inline_payload) inline_payload->changed = 1;
    }

    return result_normal;
}

static walker_result inlinable_check_walker(walker_direction direction, ASTNode *node, void *payload) {
    InlinableCheck *check;

    check = (InlinableCheck *)payload;

    if (direction == in) {
        check->node_count++;

        if (node->node_type == RETURN) {
            check->return_count++;
        }

        if (inline_assembler_has_unsupported_aliasing(node)) {
            check->has_unsupported_assembler_alias = 1;
        }
        if (inline_assembler_has_unsupported_effect(node)) {
            check->has_unsupported_assembler_effect = 1;
        }

        if (node->node_type == OP_REFERENCE ||
            node->node_type == OP_DEREFERENCE ||
            node->node_type == OP_REFVALID ||
            node->node_type == TYPE_REFERENCE ||
            node->value_type == TP_REFERENCE ||
            node->target_type == TP_REFERENCE ||
            (node->value_type == TP_OBJECT &&
             inline_class_has_reference_attribute(check->context, node->scope, node->value_class)) ||
            (node->target_type == TP_OBJECT &&
             inline_class_has_reference_attribute(check->context, node->scope, node->target_class)) ||
            (node->symbolNode && node->symbolNode->symbol &&
             (node->symbolNode->symbol->type == TP_REFERENCE ||
              (node->symbolNode->symbol->type == TP_OBJECT &&
               inline_class_has_reference_attribute(check->context,
                                                    node->symbolNode->symbol->scope,
                                                    node->symbolNode->symbol->value_class))))) {
            check->has_unsupported_reference = 1;
        }

        if (node->symbolNode &&
            node->symbolNode->symbol &&
            (node->node_type == VAR_TARGET || node->node_type == VAR_REFERENCE) &&
            inline_symbol_is_class_attribute(node->symbolNode->symbol)) {
            check->has_class_attribute_write = 1;
        }

        if (node->symbolNode &&
            node->symbolNode->symbol &&
            !inline_class_attribute_shape_is_portable(node->symbolNode->symbol)) {
            check->has_unportable_class_attribute_shape = 1;
        }

        if (node->node_type == OP_ARG_VALUE) {
            size_t index;

            if (!node->child) {
                check->has_unsupported_varg_access = 1;
            } else if (inline_varg_index_from_node(node->child, &index)) {
                if (index > check->max_required_varg_index) check->max_required_varg_index = index;
            } else {
                check->has_unsupported_varg_access = 1;
            }
        } else if (node->node_type == OP_ARG_IX_EXISTS) {
            if (!node->child) {
                check->has_unsupported_varg_access = 1;
            } else if (!inline_varg_index_from_node(node->child, NULL)) {
                check->has_unsupported_varg_access = 1;
            }
        }
    }
    return result_normal;
}

/* Walker to identify inlinable procedures */
walker_result identify_inlinable_walker(walker_direction direction, ASTNode *node, void *payload) {
    Context *context = (Context *)payload;

    if (direction == in) return result_normal;

    if (node->node_type == PROCEDURE ||
        node->node_type == METHOD ||
        node->node_type == FACTORY) {
        Symbol *sym;
        ASTNode *args;
        ASTNode *arg;
        ASTNode *formal_target;
        Symbol *formal_symbol;
        ASTNode *instrs;
        InlineReturnShape return_shape;
        InlinableCheck check;

        sym = node->symbolNode ? node->symbolNode->symbol : NULL;
        if (sym && sym->is_inlinable && inline_symbol_has_callable_template(sym) &&
            inline_symbol_uses_imported_template(sym)) {
            return result_normal;
        }

        if (!sym || sym->is_main || !sym->scope ||
            (node->node_type == PROCEDURE && sym->scope->type == SCOPE_CLASS) ||
            ((node->node_type == METHOD || node->node_type == FACTORY) &&
             (!node->parent || node->parent->node_type != CLASS_DEF))) {
            if (sym) sym->is_inlinable = 0;
            return result_normal;
        }

        if (inline_proc_has_procedure_expose(node)) {
            inline_debug_log(context, node, sym, "DEBUG_INLINE",
                             "reject: procedure-level EXPOSE is not inlineable");
            sym->is_inlinable = 0;
            return result_normal;
        }

        if (sym->type == TP_OBJECT &&
            inline_class_has_reference_attribute(context, sym->scope, sym->value_class)) {
            inline_debug_log(context, node, sym, "DEBUG_INLINE",
                             "reject: returns reference-bearing class");
            sym->is_inlinable = 0;
            return result_normal;
        }

        args = ast_chld(node, ARGS, 0);
        if (args) {
            arg = args->child;
            while (arg) {
                if (arg->is_varg) {
                    if (arg->sibling) {
                        inline_debug_log(context, node, sym, "DEBUG_INLINE",
                                         "reject: vararg formal is followed by additional formals");
                        sym->is_inlinable = 0;
                        return result_normal;
                    }
                }

                formal_target = ast_chdn(arg, 0);
                if (arg->is_varg) {
                    formal_target = arg->sibling;
                }

                formal_symbol = formal_target && formal_target->symbolNode ? formal_target->symbolNode->symbol : NULL;
                (void)formal_symbol;
                arg = arg->sibling;
            }
        }

        instrs = ast_chld(node, INSTRUCTIONS, 0);
        if (!instrs) {
            inline_debug_log(context, node, sym, "DEBUG_INLINE", "reject: procedure has no instruction list");
            sym->is_inlinable = 0;
            return result_normal;
        }

        if (!inline_analyse_return_shape(node, &return_shape)) {
            inline_debug_log(context, node, sym, "DEBUG_INLINE", "reject: failed to analyse return shape");
            sym->is_inlinable = 0;
            return result_normal;
        }
        if (!return_shape.final_is_return && sym->type != TP_VOID) {
            inline_debug_log(context, node, sym, "DEBUG_INLINE",
                             "reject: value-returning procedure does not end in RETURN");
            sym->is_inlinable = 0;
            return result_normal;
        }
        if (sym->type != TP_VOID && return_shape.return_count == 0) {
            inline_debug_log(context, node, sym, "DEBUG_INLINE",
                             "reject: value-returning procedure has no RETURN");
            sym->is_inlinable = 0;
            return result_normal;
        }

        memset(&check, 0, sizeof(check));
        check.root_proc = node;
        check.context = context;
        check.ref_varg_mode = args && inline_find_varg_arg(node) && inline_find_varg_arg(node)->is_ref_arg;
        ast_wlkr(node, inlinable_check_walker, &check);

        if (check.node_count > INLINE_MAX_NODES ||
            check.return_count != return_shape.return_count ||
            check.has_unsupported_assembler_alias ||
            check.has_unsupported_assembler_effect ||
            check.has_unsupported_reference ||
            check.has_unsupported_varg_access) {
            inline_debug_log(context, node, sym, "DEBUG_INLINE",
                             "reject: nodes=%d returns=%d final_return=%d assembler_alias=%d assembler_effect=%d unsupported_reference=%d class_attribute_write=%d unsupported_varg=%d cutoff=%d",
                             check.node_count,
                             check.return_count,
                             return_shape.final_is_return,
                             check.has_unsupported_assembler_alias,
                             check.has_unsupported_assembler_effect,
                             check.has_unsupported_reference,
                             check.has_class_attribute_write,
                             check.has_unsupported_varg_access,
                             INLINE_MAX_NODES);
            sym->is_inlinable = 0;
            return result_normal;
        }

        inline_debug_log(context, node, sym, "DEBUG_INLINE",
                         "accept: nodes=%d returns=%d final_return=%d cutoff=%d",
                         check.node_count,
                         check.return_count,
                         return_shape.final_is_return,
                         INLINE_MAX_NODES);
        sym->is_inlinable = 1;
        sym->ast_template = node;
    }
    return result_normal;
}

int rxcp_inline_pass(Context *context) {
    InlineWalkerPayload payload;

    if (!context || !context->ast) return 0;

    context->current_scope = 0;
    ast_wlkr(context->ast, identify_inlinable_walker, (void *)context);

    memset(&payload, 0, sizeof(payload));
    payload.context = context;

    context->current_scope = 0;
    ast_wlkr(context->ast, inline_procedure_walker, (void *)&payload);

    return payload.changed;
}

static int inline_proc_has_body(ASTNode *node) {
    ASTNode *instrs;

    if (!node) return 0;

    instrs = ast_chld(node, INSTRUCTIONS, NOP);
    return instrs && instrs->node_type != NOP;
}

static int inline_ascii_starts_with_ci(const char *text, size_t text_len, const char *prefix) {
    size_t i;
    size_t prefix_len;

    if (!text || !prefix) return 0;
    prefix_len = strlen(prefix);
    if (text_len < prefix_len) return 0;

    for (i = 0; i < prefix_len; i++) {
        char a = text[i];
        char b = prefix[i];

        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
        if (a != b) return 0;
    }

    return 1;
}

static int inline_assembler_has_unsupported_aliasing(ASTNode *node) {
    if (!node || node->node_type != ASSEMBLER || !node->node_string) return 0;

    return inline_ascii_starts_with_ci(node->node_string, node->node_string_length, "link") ||
           inline_ascii_starts_with_ci(node->node_string, node->node_string_length, "unlink");
}

static int inline_assembler_has_unsupported_effect(ASTNode *node) {
    /*
     * Reserved for non-aliasing assembler effects that are not represented by
     * ordinary AST read/write links. Stateful string helpers are intentionally
     * not rejected here: assembler operands are marked read/write by symbol
     * validation, so writable by-value formals are materialised through the same
     * copy semantics as normal calls. SCOPY resets copied string cursor state,
     * which preserves the call prologue boundary for setstrpos/substcut/dropchar
     * style helpers.
     */
    (void)node;
    return 0;
}

static int inline_proc_has_procedure_expose(ASTNode *node) {
    return node && ast_chld(node, EXPOSED, 0) != NULL;
}

static int inline_prune_candidate(ASTNode *node) {
    Symbol *symbol;

    if (!node || node->node_type != PROCEDURE) return 0;
    if (!inline_proc_has_body(node)) return 0;
    if (inline_proc_has_procedure_expose(node)) return 0;

    symbol = inline_symbol_from_proc_def(node);
    if (!symbol) return 0;
    if (!symbol->is_inlinable) return 0;
    if (symbol->is_main || symbol->is_implicit_main || symbol->exposed) return 0;

    return 1;
}

static int inline_collect_remaining_call_symbols(ASTNode *node,
                                                 Symbol ***symbols,
                                                 size_t *symbol_count) {
    ASTNode *child;

    if (!node) return 1;
    if (node->is_inline_pruned && inline_node_is_callable_def(node)) return 1;

    if ((node->node_type == FUNCTION ||
         node->node_type == FUNC_SYMBOL ||
         node->node_type == MEMBER_CALL ||
         node->node_type == FACTORY_CALL) &&
        node->symbolNode &&
        node->symbolNode->symbol) {
        if (!inline_append_symbol(symbols, symbol_count, node->symbolNode->symbol)) return 0;
    }

    child = node->child;
    while (child) {
        if (!inline_collect_remaining_call_symbols(child, symbols, symbol_count)) return 0;
        child = child->sibling;
    }

    return 1;
}

static int inline_prune_unreferenced_candidates(Context *context,
                                                ASTNode *node,
                                                Symbol **live_symbols,
                                                size_t live_count) {
    ASTNode *child;
    int changed;

    if (!node) return 0;

    changed = 0;

    if (!node->is_inline_pruned && inline_prune_candidate(node)) {
        Symbol *symbol;

        symbol = inline_symbol_from_proc_def(node);
        if (symbol && !inline_symbol_in_list(live_symbols, live_count, symbol)) {
            node->is_inline_pruned = 1;
            changed = 1;
            inline_debug_log(context, node, symbol, "DEBUG_INLINE",
                             "prune: private procedure has no remaining local call sites");
        }
    }

    if (node->is_inline_pruned && inline_node_is_callable_def(node)) return changed;

    child = node->child;
    while (child) {
        if (inline_prune_unreferenced_candidates(context, child, live_symbols, live_count)) {
            changed = 1;
        }
        child = child->sibling;
    }

    return changed;
}

void rxcp_inline_prune(Context *context, ASTNode *tree) {
    int changed;

    if (!context || !tree) return;

    do {
        Symbol **live_symbols;
        size_t live_count;

        live_symbols = NULL;
        live_count = 0;

        if (!inline_collect_remaining_call_symbols(tree, &live_symbols, &live_count)) {
            free(live_symbols);
            return;
        }
        changed = inline_prune_unreferenced_candidates(context, tree, live_symbols, live_count);
        free(live_symbols);
    } while (changed);
}

typedef struct {
    char *text;
    size_t length;
    size_t capacity;
    int ok;
} InlineMetaText;

typedef struct {
    Symbol *symbol;
    size_t id;
} InlineMetaSymbolEntry;

typedef struct {
    Scope *scope;
    size_t id;
    size_t parent_id;
} InlineMetaScopeEntry;

typedef struct {
    const char *file_name;
    size_t id;
} InlineMetaFileEntry;

typedef struct {
    const char *file_name;
    int line;
    int active_start_column;
    int active_end_column;
    char provenance;
    const char *source_start;
    size_t source_length;
    size_t file_id;
    size_t id;
} InlineMetaSourceEntry;

typedef struct {
    Symbol *symbol;
    size_t id;
} InlineMetaDependencyEntry;

typedef struct {
    Scope *root_scope;
    InlineMetaScopeEntry *scopes;
    size_t scope_count;
    InlineMetaSymbolEntry *symbols;
    size_t symbol_count;
    InlineMetaFileEntry *files;
    size_t file_count;
    InlineMetaSourceEntry *sources;
    size_t source_count;
    InlineMetaDependencyEntry *dependencies;
    size_t dependency_count;
} InlineMetaExport;

typedef struct {
    ASTNode *node;
    Symbol *symbol;
    Scope *scope;
    const char *reason;
} InlineMetaCollectFailure;

typedef struct {
    SourceNode *source_node;
    char provenance;
} InlineMetaImportedSource;

typedef struct {
    char *fqname;
    char *type;
    char *args;
    Symbol *symbol;
} InlineMetaImportedDependency;

typedef struct {
    Symbol **symbols;
    size_t symbol_count;
    Scope **scopes;
    size_t scope_count;
    char **files;
    size_t file_count;
    InlineMetaImportedSource *sources;
    size_t source_count;
    InlineMetaImportedDependency *dependencies;
    size_t dependency_count;
    ASTNode **stack;
    size_t stack_count;
    ASTNode *args_root;
    ASTNode *root;
    Scope *scope;
    int tree_section;
    int version;
    int ok;
} InlineMetaImport;

#define INLINE_META_NODE_SCOPE_DEF 4096u

static void inline_meta_text_init(InlineMetaText *text) {
    text->capacity = 128;
    text->length = 0;
    text->text = malloc(text->capacity);
    text->ok = text->text != NULL;
    if (text->text) text->text[0] = 0;
}

static int inline_meta_text_reserve(InlineMetaText *text, size_t extra) {
    char *new_text;
    size_t new_capacity;

    if (!text || !text->ok) return 0;
    if (text->length + extra + 1 <= text->capacity) return 1;

    new_capacity = text->capacity;
    while (text->length + extra + 1 > new_capacity) new_capacity *= 2;

    new_text = realloc(text->text, new_capacity);
    if (!new_text) {
        text->ok = 0;
        return 0;
    }

    text->text = new_text;
    text->capacity = new_capacity;
    return 1;
}

static int inline_meta_text_append(InlineMetaText *text, const char *fragment) {
    size_t length;

    if (!fragment) fragment = "";
    length = strlen(fragment);
    if (!inline_meta_text_reserve(text, length)) return 0;
    memcpy(text->text + text->length, fragment, length + 1);
    text->length += length;
    return 1;
}

static int inline_meta_text_appendf(InlineMetaText *text, const char *format, ...) {
    va_list args;
    size_t needed;
    char *buffer;

    if (!text || !text->ok || !format) return 0;

    va_start(args, format);
    needed = (size_t)vsnprintf(NULL, 0, format, args);
    va_end(args);

    buffer = malloc(needed + 1);
    if (!buffer) {
        text->ok = 0;
        return 0;
    }

    va_start(args, format);
    vsnprintf(buffer, needed + 1, format, args);
    va_end(args);

    if (!inline_meta_text_append(text, buffer)) {
        free(buffer);
        return 0;
    }

    free(buffer);
    return 1;
}

static char inline_meta_hex_digit(unsigned int value) {
    return (char)(value < 10 ? ('0' + value) : ('A' + (value - 10)));
}

static char *inline_meta_hex_encode(const char *text, size_t length) {
    char *encoded;
    size_t i;

    if (!text || !length) return strdup("-");

    encoded = malloc((length * 2) + 1);
    if (!encoded) return NULL;

    for (i = 0; i < length; i++) {
        unsigned char value = (unsigned char)text[i];
        encoded[i * 2] = inline_meta_hex_digit((unsigned int)(value >> 4));
        encoded[(i * 2) + 1] = inline_meta_hex_digit((unsigned int)(value & 0x0f));
    }
    encoded[length * 2] = 0;
    return encoded;
}

static int inline_meta_hex_value(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return -1;
}

static char *inline_meta_hex_decode(const char *encoded, size_t *length_out) {
    char *decoded;
    size_t encoded_length;
    size_t i;

    if (length_out) *length_out = 0;
    if (!encoded || strcmp(encoded, "-") == 0) return strdup("");

    encoded_length = strlen(encoded);
    if (encoded_length % 2 != 0) return NULL;

    decoded = malloc((encoded_length / 2) + 1);
    if (!decoded) return NULL;

    for (i = 0; i < encoded_length; i += 2) {
        int hi = inline_meta_hex_value(encoded[i]);
        int lo = inline_meta_hex_value(encoded[i + 1]);
        if (hi < 0 || lo < 0) {
            free(decoded);
            return NULL;
        }
        decoded[i / 2] = (char)((hi << 4) | lo);
    }
    decoded[encoded_length / 2] = 0;
    if (length_out) *length_out = encoded_length / 2;
    return decoded;
}

static char *inline_meta_int_list_encode(const int *values, size_t count, int default_value) {
    char *encoded;
    size_t capacity;
    size_t length;
    size_t i;

    if (!count) return strdup("-");

    capacity = (count * 16) + 1;
    encoded = malloc(capacity);
    if (!encoded) return NULL;

    length = 0;
    encoded[0] = 0;
    for (i = 0; i < count; i++) {
        int value = values ? values[i] : default_value;
        int written;

        written = snprintf(encoded + length, capacity - length, "%s%d", i ? ":" : "", value);
        if (written < 0 || (size_t)written >= capacity - length) {
            free(encoded);
            return NULL;
        }
        length += (size_t)written;
    }

    return encoded;
}

static int *inline_meta_int_list_decode(const char *encoded, size_t count, int default_value) {
    int *values;
    const char *cursor;
    char *end;
    size_t i;

    if (!count) return NULL;

    values = malloc(sizeof(int) * count);
    if (!values) return NULL;

    if (!encoded || strcmp(encoded, "-") == 0) {
        for (i = 0; i < count; i++) values[i] = default_value;
        return values;
    }

    cursor = encoded;
    for (i = 0; i < count; i++) {
        long value;

        value = strtol(cursor, &end, 10);
        if (end == cursor || (i + 1 < count && *end != ':') || (i + 1 == count && *end != 0)) {
            free(values);
            return NULL;
        }

        values[i] = (int)value;
        cursor = end + 1;
    }

    return values;
}

static char *inline_meta_decode_optional_hex(const char *encoded) {
    char *decoded;
    size_t length;

    decoded = inline_meta_hex_decode(encoded, &length);
    if (!decoded) return NULL;
    if (!length) {
        free(decoded);
        return NULL;
    }

    return decoded;
}

static int inline_meta_set_symbol_shape(Symbol *symbol,
                                        ValueType type,
                                        size_t dims,
                                        int *dim_base,
                                        int *dim_elements,
                                        const char *class_name) {
    size_t i;

    if (!symbol) return 0;

    symbol->type = type;
    symbol->value_dims = dims;

    if (symbol->dim_base) free(symbol->dim_base);
    if (symbol->dim_elements) free(symbol->dim_elements);
    symbol->dim_base = NULL;
    symbol->dim_elements = NULL;

    if (dims > 0) {
        symbol->dim_base = malloc(sizeof(int) * dims);
        symbol->dim_elements = malloc(sizeof(int) * dims);
        if (!symbol->dim_base || !symbol->dim_elements) {
            if (symbol->dim_base) free(symbol->dim_base);
            if (symbol->dim_elements) free(symbol->dim_elements);
            symbol->dim_base = NULL;
            symbol->dim_elements = NULL;
            return 0;
        }
        for (i = 0; i < dims; i++) {
            symbol->dim_base[i] = dim_base ? dim_base[i] : 1;
            symbol->dim_elements[i] = dim_elements ? dim_elements[i] : 0;
        }
    }

    if (symbol->value_class) {
        free(symbol->value_class);
        symbol->value_class = NULL;
    }
    if (class_name && *class_name) {
        symbol->value_class = strdup(class_name);
        if (!symbol->value_class) return 0;
    }

    return 1;
}

static unsigned int inline_meta_symbol_flags(Symbol *symbol) {
    unsigned int flags;

    flags = 0;
    if (!symbol) return flags;
    if (symbol->is_arg) flags |= 1u;
    if (symbol->is_ref_arg) flags |= 2u;
    if (symbol->is_opt_arg) flags |= 4u;
    if (symbol->is_const_arg) flags |= 8u;
    if (symbol->has_vargs) flags |= 16u;
    if (symbol->exposed) flags |= 32u;
    if (symbol->is_this) flags |= 64u;
    if (symbol->is_factory) flags |= 128u;
    if (symbol->needs_default_initiation) flags |= 256u;
    return flags;
}

static unsigned int inline_meta_node_flags(ASTNode *node) {
    unsigned int flags;

    flags = 0;
    if (!node) return flags;
    if (node->is_ref_arg) flags |= 1u;
    if (node->is_opt_arg) flags |= 2u;
    if (node->is_const_arg) flags |= 4u;
    if (node->is_varg) flags |= 8u;
    if (node->is_compiler_added) flags |= 16u;
    if (node->is_implicit_main) flags |= 32u;
    if (node->is_interface_default_method) flags |= 64u;
    if (node->force_local_scope) flags |= 128u;
    if (node->inherit_parent_scope) flags |= 256u;
    if (node->inherit_parent_reg_scope) flags |= 512u;
    if (node->suppress_shadow_warnings) flags |= 1024u;
    if (node->skip_exit_dispatch) flags |= 2048u;
    if (node->scope && node->scope->defining_node == node) flags |= INLINE_META_NODE_SCOPE_DEF;
    return flags;
}

static void inline_meta_apply_symbol_flags(Symbol *symbol, unsigned int flags) {
    if (!symbol) return;
    symbol->is_arg = (flags & 1u) != 0;
    symbol->is_ref_arg = (flags & 2u) != 0;
    symbol->is_opt_arg = (flags & 4u) != 0;
    symbol->is_const_arg = (flags & 8u) != 0;
    symbol->has_vargs = (flags & 16u) != 0;
    symbol->exposed = (flags & 32u) != 0;
    symbol->is_this = (flags & 64u) != 0;
    symbol->is_factory = (flags & 128u) != 0;
    symbol->needs_default_initiation = (flags & 256u) != 0;
}

static void inline_meta_apply_node_flags(ASTNode *node, unsigned int flags) {
    if (!node) return;
    node->is_ref_arg = (flags & 1u) != 0;
    node->is_opt_arg = (flags & 2u) != 0;
    node->is_const_arg = (flags & 4u) != 0;
    node->is_varg = (flags & 8u) != 0;
    node->is_compiler_added = (flags & 16u) != 0;
    node->is_implicit_main = (flags & 32u) != 0;
    node->is_interface_default_method = (flags & 64u) != 0;
    node->force_local_scope = (flags & 128u) != 0;
    node->inherit_parent_scope = (flags & 256u) != 0;
    node->inherit_parent_reg_scope = (flags & 512u) != 0;
    node->suppress_shadow_warnings = (flags & 1024u) != 0;
    node->skip_exit_dispatch = (flags & 2048u) != 0;
}

static int inline_meta_symbol_is_exportable(Symbol *symbol) {
    if (!symbol) return 1;
    return symbol->symbol_type == VARIABLE_SYMBOL || symbol->symbol_type == CONSTANT_SYMBOL;
}

static ASTNode *inline_meta_direct_dependency_proc(Symbol *symbol) {
    SymbolNode *defsn;

    if (!symbol || symbol->symbol_type != FUNCTION_SYMBOL || !symbol->exposed) return NULL;
    if (sym_nond(symbol) <= 0) return NULL;

    defsn = sym_trnd(symbol, 0);
    if (!defsn || !defsn->node || defsn->node->node_type != PROCEDURE) return NULL;

    return defsn->node;
}

static int inline_meta_function_uses_direct_dependency(ASTNode *node) {
    if (!node || node->node_type != FUNCTION || !node->symbolNode || !node->symbolNode->symbol) return 0;
    return inline_meta_direct_dependency_proc(node->symbolNode->symbol) != NULL;
}

static int inline_meta_node_is_exportable(ASTNode *node) {
    if (!node) return 1;
    if (node->association &&
        !(node->node_type == INSTRUCTIONS &&
          node->is_compiler_added &&
          inline_node_is_callable_def(node->association))) {
        return 0;
    }

    switch (node->node_type) {
        case ARGS:
        case ARG:
        case INSTRUCTIONS:
        case IF:
        case RETURN:
        case SAY:
        case DEFINE:
        case ASSIGN:
        case DO:
        case REPEAT:
        case TO:
        case BY:
        case FOR:
        case WHILE:
        case UNTIL:
        case CLASS:
        case VAR_TARGET:
        case VAR_SYMBOL:
        case VAR_REFERENCE:
        case VARG:
        case VARG_REFERENCE:
        case CONST_SYMBOL:
        case INTEGER:
        case FLOAT:
        case DECIMAL:
        case STRING:
        case BINARY:
        case CONSTANT:
        case RANGE:
        case NOVAL:
        case VOID:
        case LITERAL:
        case OP_ADD:
        case OP_MINUS:
        case OP_MULT:
        case OP_DIV:
        case OP_IDIV:
        case OP_MOD:
        case OP_POWER:
        case OP_NEG:
        case OP_PLUS:
        case OP_CONCAT:
        case OP_SCONCAT:
        case OP_AND:
        case OP_OR:
        case OP_NOT:
        case OP_COMPARE_EQUAL:
        case OP_COMPARE_NEQ:
        case OP_COMPARE_GT:
        case OP_COMPARE_LT:
        case OP_COMPARE_GTE:
        case OP_COMPARE_LTE:
        case OP_COMPARE_S_EQ:
        case OP_COMPARE_S_NEQ:
        case OP_COMPARE_S_GT:
        case OP_COMPARE_S_LT:
        case OP_COMPARE_S_GTE:
        case OP_COMPARE_S_LTE:
        case OP_TYPE_IS:
        case OP_TYPE_CAST:
        case OP_TYPEOF:
        case OP_ARGS:
        case OP_ARG_VALUE:
        case OP_ARG_EXISTS:
        case OP_ARG_IX_EXISTS:
        case CALL:
            return 1;
        case FUNCTION:
            return inline_meta_function_uses_direct_dependency(node);
        case ASSEMBLER:
            return !inline_assembler_has_unsupported_aliasing(node) &&
                   !inline_assembler_has_unsupported_effect(node);
        default:
            return 0;
    }
}

static int inline_meta_find_scope_id(InlineMetaExport *meta, Scope *scope, size_t *id_out) {
    size_t i;

    if (id_out) *id_out = 0;
    if (!meta || !scope) return 0;

    for (i = 0; i < meta->scope_count; i++) {
        if (meta->scopes[i].scope == scope) {
            if (id_out) *id_out = meta->scopes[i].id;
            return 1;
        }
    }

    return 0;
}

static int inline_meta_find_file_id(InlineMetaExport *meta, const char *file_name, size_t *id_out) {
    size_t i;

    if (id_out) *id_out = (size_t)-1;
    if (!meta || !file_name) return 0;

    for (i = 0; i < meta->file_count; i++) {
        if (meta->files[i].file_name && strcmp(meta->files[i].file_name, file_name) == 0) {
            if (id_out) *id_out = meta->files[i].id;
            return 1;
        }
    }

    return 0;
}

static int inline_meta_collect_file(InlineMetaExport *meta, const char *file_name, size_t *id_out) {
    InlineMetaFileEntry *new_files;

    if (id_out) *id_out = (size_t)-1;
    if (!file_name || !*file_name) return 1;
    if (inline_meta_find_file_id(meta, file_name, id_out)) return 1;

    new_files = realloc(meta->files, sizeof(InlineMetaFileEntry) * (meta->file_count + 1));
    if (!new_files) return 0;

    meta->files = new_files;
    meta->files[meta->file_count].file_name = file_name;
    meta->files[meta->file_count].id = meta->file_count;
    if (id_out) *id_out = meta->file_count;
    meta->file_count++;
    return 1;
}

static int inline_meta_pointer_in_range(const char *ptr, const char *range_start, const char *range_end) {
    return ptr && range_start && range_end && ptr >= range_start && ptr <= range_end;
}

static int inline_meta_context_range(Context *context, const char *ptr, const char **range_start, const char **range_end) {
    if (!context || !context->buff_start || !context->buff_end || !ptr) return 0;
    if (ptr < context->buff_start || ptr > context->buff_end) return 0;
    if (range_start) *range_start = context->buff_start;
    if (range_end) *range_end = context->buff_end;
    return 1;
}

static int inline_meta_owned_source_range(SourceNode *source_node,
                                          const char *ptr,
                                          const char **range_start,
                                          const char **range_end) {
    size_t length;

    if (!source_node || !source_node->owned_source_text || !ptr) return 0;
    length = strlen(source_node->owned_source_text);
    if (!inline_meta_pointer_in_range(ptr, source_node->owned_source_text, source_node->owned_source_text + length)) return 0;
    if (range_start) *range_start = source_node->owned_source_text;
    if (range_end) *range_end = source_node->owned_source_text + length;
    return 1;
}

static int inline_meta_source_range(ASTNode *node,
                                    const char *source_start,
                                    const char **range_start,
                                    const char **range_end) {
    if (!node || !source_start) return 0;
    if (inline_meta_owned_source_range(node->source_node, source_start, range_start, range_end)) return 1;
    if (inline_meta_context_range(node->context, source_start, range_start, range_end)) return 1;
    if (node->source_node && inline_meta_context_range(node->source_node->context, source_start, range_start, range_end)) return 1;
    return 0;
}

static int inline_meta_line_bounds(const char *ptr,
                                   const char *range_start,
                                   const char *range_end,
                                   const char **line_start_out,
                                   const char **line_end_out) {
    const char *line_start;
    const char *line_end;

    if (!ptr || !range_start || !range_end || ptr < range_start || ptr > range_end) return 0;
    if (ptr == range_end && ptr > range_start) ptr--;

    line_start = ptr;
    while (line_start > range_start &&
           line_start[-1] != '\n' &&
           line_start[-1] != '\r') {
        line_start--;
    }

    line_end = ptr;
    while (line_end < range_end &&
           *line_end != '\n' &&
           *line_end != '\r' &&
           *line_end != 0) {
        line_end++;
    }

    if (line_start_out) *line_start_out = line_start;
    if (line_end_out) *line_end_out = line_end;
    return 1;
}

static int inline_meta_node_source_data(ASTNode *node,
                                        const char **file_name_out,
                                        int *line_out,
                                        int *active_start_column_out,
                                        int *active_end_column_out,
                                        char *provenance_out,
                                        const char **source_start_out,
                                        size_t *source_length_out) {
    const char *source_start;
    const char *source_end;
    const char *range_start;
    const char *range_end;
    const char *line_start;
    const char *line_end;
    const char *active_end;
    const char *file_name;
    int line;
    int column;
    int active_start_column;
    int active_end_column;
    char provenance;

    if (file_name_out) *file_name_out = NULL;
    if (line_out) *line_out = -1;
    if (active_start_column_out) *active_start_column_out = -1;
    if (active_end_column_out) *active_end_column_out = -1;
    if (provenance_out) *provenance_out = AST_SOURCE_NONE;
    if (source_start_out) *source_start_out = NULL;
    if (source_length_out) *source_length_out = 0;
    if (!node) return 0;

    file_name = node->file_name;
    line = node->line;
    column = node->column;
    provenance = node->source_provenance;
    source_start = node->source_start;
    source_end = node->source_end;

    if (node->source_node) {
        if (!file_name) file_name = node->source_node->file_name;
        if (line < 0) line = node->source_node->line;
        if (column < 0) column = node->source_node->column;
        if (!source_start) source_start = node->source_node->source_start;
        if (!source_end) source_end = node->source_node->source_end;
    }

    if (!source_start || !source_end || source_end < source_start) return 0;

    range_start = 0;
    range_end = 0;
    line_start = source_start;
    line_end = source_end + 1;
    active_start_column = column >= 0 ? column + 1 : 1;
    active_end_column = active_start_column + (int)(source_end - source_start) + 1;
    if (inline_meta_source_range(node, source_start, &range_start, &range_end) &&
        inline_meta_line_bounds(source_start, range_start, range_end, &line_start, &line_end)) {
        active_end = source_end;
        if (active_end >= line_end && line_end > line_start) active_end = line_end - 1;
        active_start_column = (int)(source_start - line_start) + 1;
        active_end_column = (int)(active_end - line_start) + 2;
        if (active_end_column > (int)(line_end - line_start) + 1) {
            active_end_column = (int)(line_end - line_start) + 1;
        }
    }

    if (line_end < line_start) return 0;
    if ((size_t)(line_end - line_start) > INLINE_META_MAX_SOURCE_SPAN) return 0;
    if (memchr(line_start, 0, (size_t)(line_end - line_start))) return 0;

    if (file_name_out) *file_name_out = file_name;
    if (line_out) *line_out = line;
    if (active_start_column_out) *active_start_column_out = active_start_column;
    if (active_end_column_out) *active_end_column_out = active_end_column;
    if (provenance_out) *provenance_out = provenance;
    if (source_start_out) *source_start_out = line_start;
    if (source_length_out) *source_length_out = (size_t)(line_end - line_start);
    return 1;
}

static int inline_meta_find_source_id(InlineMetaExport *meta, ASTNode *node, size_t *id_out) {
    const char *file_name;
    const char *source_start;
    size_t source_length;
    int line;
    int active_start_column;
    int active_end_column;
    char provenance;
    size_t i;

    if (id_out) *id_out = (size_t)-1;
    if (!inline_meta_node_source_data(node,
                                      &file_name,
                                      &line,
                                      &active_start_column,
                                      &active_end_column,
                                      &provenance,
                                      &source_start,
                                      &source_length)) {
        return 0;
    }

    for (i = 0; i < meta->source_count; i++) {
        InlineMetaSourceEntry *source = &meta->sources[i];
        if (source->line == line &&
            source->active_start_column == active_start_column &&
            source->active_end_column == active_end_column &&
            source->provenance == provenance &&
            source->source_length == source_length &&
            ((source->file_name == file_name) ||
             (source->file_name && file_name && strcmp(source->file_name, file_name) == 0)) &&
            memcmp(source->source_start, source_start, source_length) == 0) {
            if (id_out) *id_out = source->id;
            return 1;
        }
    }

    return 0;
}

static int inline_meta_collect_source(InlineMetaExport *meta, ASTNode *node) {
    InlineMetaSourceEntry *new_sources;
    const char *file_name;
    const char *source_start;
    size_t source_length;
    size_t file_id;
    int line;
    int active_start_column;
    int active_end_column;
    char provenance;

    if (!meta || !node) return 0;
    if (inline_meta_find_source_id(meta, node, NULL)) return 1;
    if (!inline_meta_node_source_data(node,
                                      &file_name,
                                      &line,
                                      &active_start_column,
                                      &active_end_column,
                                      &provenance,
                                      &source_start,
                                      &source_length)) {
        return 1;
    }

    file_id = (size_t)-1;
    if (file_name && *file_name && !inline_meta_collect_file(meta, file_name, &file_id)) return 0;

    new_sources = realloc(meta->sources, sizeof(InlineMetaSourceEntry) * (meta->source_count + 1));
    if (!new_sources) return 0;

    meta->sources = new_sources;
    meta->sources[meta->source_count].file_name = file_name;
    meta->sources[meta->source_count].line = line;
    meta->sources[meta->source_count].active_start_column = active_start_column;
    meta->sources[meta->source_count].active_end_column = active_end_column;
    meta->sources[meta->source_count].provenance = provenance;
    meta->sources[meta->source_count].source_start = source_start;
    meta->sources[meta->source_count].source_length = source_length;
    meta->sources[meta->source_count].file_id = file_id;
    meta->sources[meta->source_count].id = meta->source_count;
    meta->source_count++;
    return 1;
}

static int inline_meta_scope_is_exportable(InlineMetaExport *meta, Scope *scope) {
    Scope *cursor;

    if (!meta || !scope) return 0;
    if (scope == meta->root_scope) return 1;
    if (meta->root_scope &&
        meta->root_scope->defining_node &&
        (meta->root_scope->defining_node->node_type == METHOD ||
         meta->root_scope->defining_node->node_type == FACTORY) &&
        scope == meta->root_scope->parent &&
        scope->type == SCOPE_CLASS) {
        return 1;
    }
    if (scope->type != SCOPE_LOCAL) return 0;

    cursor = scope->parent;
    while (cursor) {
        if (cursor == meta->root_scope) return 1;
        if (cursor->type != SCOPE_LOCAL) return 0;
        cursor = cursor->parent;
    }

    return 0;
}

static int inline_meta_collect_scope(InlineMetaExport *meta, Scope *scope) {
    InlineMetaScopeEntry *new_scopes;
    size_t parent_id;

    if (!scope) return 1;
    if (!meta || !meta->root_scope) return 0;
    if (!inline_meta_scope_is_exportable(meta, scope)) return 0;
    if (inline_meta_find_scope_id(meta, scope, NULL)) return 1;

    parent_id = (size_t)-1;
    if (scope != meta->root_scope) {
        if (meta->root_scope &&
            meta->root_scope->defining_node &&
            (meta->root_scope->defining_node->node_type == METHOD ||
             meta->root_scope->defining_node->node_type == FACTORY) &&
            scope == meta->root_scope->parent &&
            scope->type == SCOPE_CLASS) {
            parent_id = (size_t)-1;
        } else {
            if (!inline_meta_collect_scope(meta, scope->parent)) return 0;
            if (!inline_meta_find_scope_id(meta, scope->parent, &parent_id)) return 0;
        }
    }

    new_scopes = realloc(meta->scopes, sizeof(InlineMetaScopeEntry) * (meta->scope_count + 1));
    if (!new_scopes) return 0;

    meta->scopes = new_scopes;
    meta->scopes[meta->scope_count].scope = scope;
    meta->scopes[meta->scope_count].id = meta->scope_count;
    meta->scopes[meta->scope_count].parent_id = parent_id;
    meta->scope_count++;
    return 1;
}

static int inline_meta_find_symbol_id(InlineMetaExport *meta, Symbol *symbol, size_t *id_out) {
    size_t i;

    if (id_out) *id_out = (size_t)-1;
    if (!meta || !symbol) return 0;

    for (i = 0; i < meta->symbol_count; i++) {
        if (meta->symbols[i].symbol == symbol) {
            if (id_out) *id_out = meta->symbols[i].id;
            return 1;
        }
    }

    return 0;
}

static int inline_meta_find_dependency_id(InlineMetaExport *meta, Symbol *symbol, size_t *id_out) {
    size_t i;

    if (id_out) *id_out = (size_t)-1;
    if (!meta || !symbol) return 0;

    for (i = 0; i < meta->dependency_count; i++) {
        if (meta->dependencies[i].symbol == symbol) {
            if (id_out) *id_out = meta->dependencies[i].id;
            return 1;
        }
    }

    return 0;
}

static int inline_meta_collect_symbol(InlineMetaExport *meta, Symbol *symbol) {
    InlineMetaSymbolEntry *new_symbols;

    if (!symbol) return 1;
    if (!inline_meta_symbol_is_exportable(symbol)) return 0;
    if (!inline_meta_collect_scope(meta, symbol->scope)) return 0;
    if (inline_meta_find_symbol_id(meta, symbol, NULL)) return 1;

    new_symbols = realloc(meta->symbols, sizeof(InlineMetaSymbolEntry) * (meta->symbol_count + 1));
    if (!new_symbols) return 0;

    meta->symbols = new_symbols;
    meta->symbols[meta->symbol_count].symbol = symbol;
    meta->symbols[meta->symbol_count].id = meta->symbol_count;
    meta->symbol_count++;
    return 1;
}

static int inline_meta_collect_dependency(InlineMetaExport *meta, ASTNode *node) {
    InlineMetaDependencyEntry *new_dependencies;
    Symbol *symbol;
    ASTNode *dependency_proc;

    if (!meta || !node || node->node_type != FUNCTION || !node->symbolNode) return 0;

    symbol = node->symbolNode->symbol;
    dependency_proc = inline_meta_direct_dependency_proc(symbol);
    if (!dependency_proc) return 0;
    if (meta->root_scope && dependency_proc == meta->root_scope->defining_node) return 0;
    if (inline_meta_find_dependency_id(meta, symbol, NULL)) return 1;

    new_dependencies = realloc(meta->dependencies,
                               sizeof(InlineMetaDependencyEntry) * (meta->dependency_count + 1));
    if (!new_dependencies) return 0;

    meta->dependencies = new_dependencies;
    meta->dependencies[meta->dependency_count].symbol = symbol;
    meta->dependencies[meta->dependency_count].id = meta->dependency_count;
    meta->dependency_count++;
    return 1;
}

static int inline_meta_collect(ASTNode *node, InlineMetaExport *meta) {
    ASTNode *child;

    if (!node) return 1;
    if (!inline_meta_node_is_exportable(node)) return 0;
    if (!inline_meta_collect_scope(meta, node->scope)) return 0;
    if (inline_meta_function_uses_direct_dependency(node)) {
        if (!inline_meta_collect_dependency(meta, node)) return 0;
    } else if (node->symbolNode && !inline_meta_collect_symbol(meta, node->symbolNode->symbol)) {
        return 0;
    }
    if (!inline_meta_collect_source(meta, node)) return 0;

    child = node->child;
    while (child) {
        if (!inline_meta_collect(child, meta)) return 0;
        child = child->sibling;
    }

    return 1;
}

static int inline_meta_find_collect_failure(ASTNode *node,
                                            InlineMetaExport *meta,
                                            InlineMetaCollectFailure *failure) {
    ASTNode *child;

    if (!node || !meta || !failure) return 0;

    if (node->association &&
        !(node->node_type == INSTRUCTIONS &&
          node->is_compiler_added &&
          inline_node_is_callable_def(node->association))) {
        failure->node = node;
        failure->reason = "unsupported association in inline metadata";
        return 1;
    }
    if (node->node_type == ASSEMBLER &&
        inline_assembler_has_unsupported_aliasing(node)) {
        failure->node = node;
        failure->reason = "assembler aliasing instruction";
        return 1;
    }
    if (node->node_type == ASSEMBLER &&
        inline_assembler_has_unsupported_effect(node)) {
        failure->node = node;
        failure->reason = "assembler stateful instruction";
        return 1;
    }
    if (node->node_type == FUNCTION &&
        node->symbolNode &&
        node->symbolNode->symbol &&
        !inline_meta_function_uses_direct_dependency(node)) {
        failure->node = node;
        failure->symbol = node->symbolNode->symbol;
        failure->reason = "unsupported residual callable dependency in inline metadata";
        return 1;
    }
    if (!inline_meta_node_is_exportable(node)) {
        failure->node = node;
        failure->reason = "unsupported AST node in inline metadata";
        return 1;
    }
    if (node->scope && !inline_meta_scope_is_exportable(meta, node->scope)) {
        failure->node = node;
        failure->scope = node->scope;
        failure->reason = "unsupported scope in inline metadata";
        return 1;
    }
    if (node->symbolNode &&
        node->symbolNode->symbol &&
        !inline_meta_function_uses_direct_dependency(node) &&
        !inline_meta_symbol_is_exportable(node->symbolNode->symbol)) {
        failure->node = node;
        failure->symbol = node->symbolNode->symbol;
        failure->reason = "unsupported symbol in inline metadata";
        return 1;
    }

    child = node->child;
    while (child) {
        if (inline_meta_find_collect_failure(child, meta, failure)) return 1;
        child = child->sibling;
    }

    return 0;
}

static void inline_meta_debug_collect_failure(Context *context,
                                              ASTNode *callable,
                                              Symbol *symbol,
                                              InlineMetaExport *meta,
                                              ASTNode *args,
                                              ASTNode *instrs) {
    InlineMetaCollectFailure failure;
    ASTNode *node;
    const char *node_type;

    memset(&failure, 0, sizeof(failure));
    if (inline_meta_find_collect_failure(args, meta, &failure) ||
        inline_meta_find_collect_failure(instrs, meta, &failure)) {
        node = failure.node ? failure.node : callable;
        node_type = node ? ast_ndtp(node->node_type) : "UNKNOWN";
        if (failure.symbol && failure.symbol->name) {
            inline_export_debug_reject(context,
                                       node,
                                       symbol,
                                       "%s: %s symbol=%s",
                                       failure.reason,
                                       node_type,
                                       failure.symbol->name);
        } else {
            inline_export_debug_reject(context,
                                       node,
                                       symbol,
                                       "%s: %s",
                                       failure.reason,
                                       node_type);
        }
        return;
    }

    inline_export_debug_reject(context,
                               callable,
                               symbol,
                               "inline metadata collection failed");
}

static int inline_meta_emit_scopes(InlineMetaText *text, InlineMetaExport *meta) {
    size_t i;

    for (i = 0; i < meta->scope_count; i++) {
        Scope *scope = meta->scopes[i].scope;
        long parent_id = meta->scopes[i].parent_id == (size_t)-1 ? -1L : (long)meta->scopes[i].parent_id;

        if (!inline_meta_text_appendf(text,
                                      ";q,%zu,%ld,%d",
                                      meta->scopes[i].id,
                                      parent_id,
                                      scope ? (int)scope->type : (int)SCOPE_LOCAL)) {
            return 0;
        }
    }

    return 1;
}

static int inline_meta_emit_files(InlineMetaText *text, InlineMetaExport *meta) {
    size_t i;

    for (i = 0; i < meta->file_count; i++) {
        char *file_hex = inline_meta_hex_encode(meta->files[i].file_name,
                                                meta->files[i].file_name ? strlen(meta->files[i].file_name) : 0);
        if (!file_hex) return 0;
        if (!inline_meta_text_appendf(text, ";f,%zu,%s", meta->files[i].id, file_hex)) {
            free(file_hex);
            return 0;
        }
        free(file_hex);
    }

    return 1;
}

static int inline_meta_emit_sources(InlineMetaText *text, InlineMetaExport *meta) {
    size_t i;

    for (i = 0; i < meta->source_count; i++) {
        InlineMetaSourceEntry *source = &meta->sources[i];
        long file_id = source->file_id == (size_t)-1 ? -1L : (long)source->file_id;
        char *source_hex = inline_meta_hex_encode(source->source_start, source->source_length);
        if (!source_hex) return 0;
        if (!inline_meta_text_appendf(text,
                                      ";u,%zu,%ld,%d,%d,%d,%d,%s",
                                      source->id,
                                      file_id,
                                      source->line,
                                      source->active_start_column,
                                      source->active_end_column,
                                      (int)source->provenance,
                                      source_hex)) {
            free(source_hex);
            return 0;
        }
        free(source_hex);
    }

    return 1;
}

static int inline_meta_emit_symbols(InlineMetaText *text, InlineMetaExport *meta) {
    size_t i;

    for (i = 0; i < meta->symbol_count; i++) {
        Symbol *symbol = meta->symbols[i].symbol;
        size_t scope_id;
        char *name_hex = inline_meta_hex_encode(symbol->name, strlen(symbol->name));
        char *base_text = inline_meta_int_list_encode(symbol->dim_base, symbol->value_dims, 1);
        char *elements_text = inline_meta_int_list_encode(symbol->dim_elements, symbol->value_dims, 0);
        char *class_hex = inline_meta_hex_encode(symbol->value_class,
                                                 symbol->value_class ? strlen(symbol->value_class) : 0);
        if (!name_hex || !base_text || !elements_text || !class_hex) {
            if (name_hex) free(name_hex);
            if (base_text) free(base_text);
            if (elements_text) free(elements_text);
            if (class_hex) free(class_hex);
            return 0;
        }
        if (!inline_meta_find_scope_id(meta, symbol->scope, &scope_id)) {
            free(name_hex);
            free(base_text);
            free(elements_text);
            free(class_hex);
            return 0;
        }

        if (!inline_meta_text_appendf(text,
                                      ";s,%zu,%zu,%s,%d,%d,%zu,%s,%s,%s,%u,%c,%d",
                                      meta->symbols[i].id,
                                      scope_id,
                                      name_hex,
                                      (int)symbol->symbol_type,
                                      (int)symbol->type,
                                      symbol->value_dims,
                                      base_text,
                                      elements_text,
                                      class_hex,
                                      inline_meta_symbol_flags(symbol),
                                      symbol->register_type ? symbol->register_type : '-',
                                      inline_class_attribute_register_num(symbol))) {
            free(name_hex);
            free(base_text);
            free(elements_text);
            free(class_hex);
            return 0;
        }
        free(name_hex);
        free(base_text);
        free(elements_text);
        free(class_hex);
    }

    return 1;
}

static int inline_meta_emit_dependencies(InlineMetaText *text, InlineMetaExport *meta) {
    size_t i;

    for (i = 0; i < meta->dependency_count; i++) {
        Symbol *symbol = meta->dependencies[i].symbol;
        ASTNode *proc = inline_meta_direct_dependency_proc(symbol);
        ASTNode *args = proc ? ast_chld(proc, ARGS, 0) : NULL;
        char *fqname = symbol ? sym_frnm(symbol) : NULL;
        char *type = proc ? callable_effective_return_type(proc) : NULL;
        char *arg_text = meta_narg(args);
        char *fqname_hex;
        char *type_hex;
        char *args_hex;

        if (!fqname || !type || !arg_text) {
            if (fqname) free(fqname);
            if (type) free(type);
            if (arg_text) free(arg_text);
            return 0;
        }

        fqname_hex = inline_meta_hex_encode(fqname, strlen(fqname));
        type_hex = inline_meta_hex_encode(type, strlen(type));
        args_hex = inline_meta_hex_encode(arg_text, strlen(arg_text));
        free(fqname);
        free(type);
        free(arg_text);
        if (!fqname_hex || !type_hex || !args_hex) {
            if (fqname_hex) free(fqname_hex);
            if (type_hex) free(type_hex);
            if (args_hex) free(args_hex);
            return 0;
        }

        if (!inline_meta_text_appendf(text,
                                      ";d,%zu,%s,%s,%s",
                                      meta->dependencies[i].id,
                                      fqname_hex,
                                      type_hex,
                                      args_hex)) {
            free(fqname_hex);
            free(type_hex);
            free(args_hex);
            return 0;
        }

        free(fqname_hex);
        free(type_hex);
        free(args_hex);
    }

    return 1;
}

static int inline_meta_emit_node(InlineMetaText *text, InlineMetaExport *meta, ASTNode *node) {
    ASTNode *child;
    size_t symbol_id;
    size_t dependency_id;
    size_t scope_id;
    size_t source_id;
    unsigned int symbol_read_usage;
    unsigned int symbol_write_usage;
    char *node_hex;
    char *decimal_hex;
    char *value_base_text;
    char *value_elements_text;
    char *target_base_text;
    char *target_elements_text;
    char *value_class_hex;
    char *target_class_hex;
    char int_buffer[64];

    if (!node) return 1;

    symbol_id = (size_t)-1;
    dependency_id = (size_t)-1;
    symbol_read_usage = 0;
    symbol_write_usage = 0;
    if (inline_meta_function_uses_direct_dependency(node)) {
        if (!inline_meta_find_dependency_id(meta, node->symbolNode->symbol, &dependency_id)) return 0;
    } else if (node->symbolNode && node->symbolNode->symbol) {
        if (!inline_meta_find_symbol_id(meta, node->symbolNode->symbol, &symbol_id)) return 0;
        symbol_read_usage = node->symbolNode->readUsage;
        symbol_write_usage = node->symbolNode->writeUsage;
    }
    if (node->scope) {
        if (!inline_meta_find_scope_id(meta, node->scope, &scope_id)) return 0;
    } else {
        scope_id = 0;
    }
    if (!inline_meta_find_source_id(meta, node, &source_id)) source_id = (size_t)-1;

    node_hex = inline_meta_hex_encode(node->node_string, node->node_string_length);
    decimal_hex = inline_meta_hex_encode(node->decimal_value, node->decimal_value ? strlen(node->decimal_value) : 0);
    value_base_text = inline_meta_int_list_encode(node->value_dim_base, node->value_dims, 1);
    value_elements_text = inline_meta_int_list_encode(node->value_dim_elements, node->value_dims, 0);
    target_base_text = inline_meta_int_list_encode(node->target_dim_base, node->target_dims, 1);
    target_elements_text = inline_meta_int_list_encode(node->target_dim_elements, node->target_dims, 0);
    value_class_hex = inline_meta_hex_encode(node->value_class, node->value_class ? strlen(node->value_class) : 0);
    target_class_hex = inline_meta_hex_encode(node->target_class, node->target_class ? strlen(node->target_class) : 0);
    if (!node_hex || !decimal_hex || !value_base_text || !value_elements_text ||
        !target_base_text || !target_elements_text || !value_class_hex || !target_class_hex) {
        if (node_hex) free(node_hex);
        if (decimal_hex) free(decimal_hex);
        if (value_base_text) free(value_base_text);
        if (value_elements_text) free(value_elements_text);
        if (target_base_text) free(target_base_text);
        if (target_elements_text) free(target_elements_text);
        if (value_class_hex) free(value_class_hex);
        if (target_class_hex) free(target_class_hex);
        return 0;
    }

#ifdef __32BIT__
    snprintf(int_buffer, sizeof(int_buffer), "%ld", node->int_value);
#else
    snprintf(int_buffer, sizeof(int_buffer), "%lld", (long long)node->int_value);
#endif

    if (!inline_meta_text_appendf(text,
                                  ";>,%zu,%ld,%d,%d,%d,%zu,%zu,%s,%s,%s,%s,%s,%s,%u,%ld,%ld,%u,%u,%s,%d,%.17g,%s,%s",
                                  scope_id,
                                  source_id == (size_t)-1 ? -1L : (long)source_id,
                                  (int)node->node_type,
                                  (int)node->value_type,
                                  (int)node->target_type,
                                  node->value_dims,
                                  node->target_dims,
                                  value_base_text,
                                  value_elements_text,
                                  target_base_text,
                                  target_elements_text,
                                  value_class_hex,
                                  target_class_hex,
                                  inline_meta_node_flags(node),
                                  symbol_id == (size_t)-1 ? -1L : (long)symbol_id,
                                  dependency_id == (size_t)-1 ? -1L : (long)dependency_id,
                                  symbol_read_usage,
                                  symbol_write_usage,
                                  int_buffer,
                                  node->bool_value,
                                  node->float_value,
                                  node_hex,
                                  decimal_hex)) {
        free(node_hex);
        free(decimal_hex);
        free(value_base_text);
        free(value_elements_text);
        free(target_base_text);
        free(target_elements_text);
        free(value_class_hex);
        free(target_class_hex);
        return 0;
    }

    free(node_hex);
    free(decimal_hex);
    free(value_base_text);
    free(value_elements_text);
    free(target_base_text);
    free(target_elements_text);
    free(value_class_hex);
    free(target_class_hex);

    child = node->child;
    while (child) {
        if (!inline_meta_emit_node(text, meta, child)) return 0;
        child = child->sibling;
    }

    return inline_meta_text_append(text, ";<");
}

char *rxcp_inline_export_payload(Context *context, ASTNode *callable) {
    Symbol *symbol;
    ASTNode *args;
    ASTNode *instrs;
    InlineReturnShape return_shape;
    InlinableCheck check;
    InlineMetaExport meta;
    InlineMetaText text;

    if (!callable || !inline_node_is_callable_def(callable)) return strdup("");
    if (callable->is_inline_pruned) {
        inline_export_debug_reject(context, callable, NULL, "callable already pruned");
        return strdup("");
    }

    symbol = inline_symbol_from_proc_def(callable);
    if (inline_proc_has_procedure_expose(callable)) {
        inline_export_debug_reject(context, callable, symbol, "procedure-level expose");
        return strdup("");
    }

    if (!symbol || !symbol->exposed) {
        inline_export_debug_reject(context, callable, symbol, "callable is not namespace-exposed");
        return strdup("");
    }
    if (symbol->is_main || symbol->is_implicit_main) {
        inline_export_debug_reject(context, callable, symbol, "main is not inline-exportable");
        return strdup("");
    }

    args = ast_chld(callable, ARGS, 0);
    instrs = ast_chld(callable, INSTRUCTIONS, 0);
    if (!args || !instrs) {
        inline_export_debug_reject(context, callable, symbol, "missing args or instruction list");
        return strdup("");
    }

    if (!inline_analyse_return_shape(callable, &return_shape)) {
        inline_export_debug_reject(context, callable, symbol, "failed to analyse return shape");
        return strdup("");
    }
    if (!return_shape.final_is_return && symbol->type != TP_VOID) {
        inline_export_debug_reject(context, callable, symbol, "value procedure lacks final RETURN");
        return strdup("");
    }
    if (symbol->type != TP_VOID && return_shape.return_count == 0) {
        inline_export_debug_reject(context, callable, symbol, "value procedure has no RETURN");
        return strdup("");
    }

    memset(&check, 0, sizeof(check));
    check.root_proc = callable;
    check.context = context;
    check.ref_varg_mode = args && inline_find_varg_arg(callable) && inline_find_varg_arg(callable)->is_ref_arg;
    ast_wlkr(callable, inlinable_check_walker, &check);
    if (check.node_count > INLINE_MAX_NODES) {
        inline_export_debug_reject(context,
                                   callable,
                                   symbol,
                                   "node count %d exceeds cutoff %d",
                                   check.node_count,
                                   INLINE_MAX_NODES);
        return strdup("");
    }
    if (check.return_count != return_shape.return_count) {
        inline_export_debug_reject(context, callable, symbol, "return-shape mismatch");
        return strdup("");
    }
    if (check.has_unsupported_assembler_alias) {
        inline_export_debug_reject(context, callable, symbol, "assembler aliasing instruction");
        return strdup("");
    }
    if (check.has_unsupported_assembler_effect) {
        inline_export_debug_reject(context, callable, symbol, "assembler stateful instruction");
        return strdup("");
    }
    if (check.has_unsupported_reference) {
        inline_export_debug_reject(context, callable, symbol, "reference operation or type");
        return strdup("");
    }
    if (check.has_unsupported_varg_access) {
        inline_export_debug_reject(context, callable, symbol, "unsupported vararg access");
        return strdup("");
    }
    if ((callable->node_type == METHOD || callable->node_type == FACTORY) &&
        check.has_unportable_class_attribute_shape) {
        inline_export_debug_reject(context, callable, symbol, "unportable class attribute shape");
        return strdup("");
    }

    memset(&meta, 0, sizeof(meta));
    meta.root_scope = callable->scope;
    if (!inline_meta_collect_scope(&meta, callable->scope)) {
        inline_export_debug_reject(context, callable, symbol, "failed to collect callable scope");
        free(meta.scopes);
        return strdup("");
    }
    if (!inline_meta_collect(args, &meta) ||
        !inline_meta_collect(instrs, &meta)) {
        inline_meta_debug_collect_failure(context, callable, symbol, &meta, args, instrs);
        free(meta.scopes);
        free(meta.symbols);
        free(meta.files);
        free(meta.sources);
        free(meta.dependencies);
        return strdup("");
    }

    inline_meta_text_init(&text);
    if (!text.ok ||
        !inline_meta_text_append(&text, "I5") ||
        !inline_meta_emit_files(&text, &meta) ||
        !inline_meta_emit_sources(&text, &meta) ||
        !inline_meta_emit_scopes(&text, &meta) ||
        !inline_meta_emit_symbols(&text, &meta) ||
        !inline_meta_emit_dependencies(&text, &meta) ||
        !inline_meta_text_append(&text, ";a") ||
        !inline_meta_emit_node(&text, &meta, args) ||
        !inline_meta_text_append(&text, ";b") ||
        !inline_meta_emit_node(&text, &meta, instrs)) {
        inline_export_debug_reject(context, callable, symbol, "failed to emit inline metadata");
        free(meta.scopes);
        free(meta.symbols);
        free(meta.files);
        free(meta.sources);
        free(meta.dependencies);
        if (text.text) free(text.text);
        return strdup("");
    }

    free(meta.scopes);
    free(meta.symbols);
    free(meta.files);
    free(meta.sources);
    free(meta.dependencies);
    return text.text;
}

int rxcp_inline_payload_is_supported(const char *payload) {
    return payload && payload[0] == 'I' && (payload[1] == '4' || payload[1] == '5') &&
           (payload[2] == 0 || payload[2] == ';');
}

static ASTNode *inline_meta_find_first_procedure(ASTNode *node) {
    ASTNode *child;
    ASTNode *found;

    if (!node) return NULL;
    if (node->node_type == PROCEDURE) return node;

    child = node->child;
    while (child) {
        found = inline_meta_find_first_procedure(child);
        if (found) return found;
        child = child->sibling;
    }

    return NULL;
}

static char *inline_meta_next_field(char **cursor) {
    char *field;
    char *comma;

    if (!cursor || !*cursor) return NULL;
    field = *cursor;
    comma = strchr(field, ',');
    if (comma) {
        *comma = 0;
        *cursor = comma + 1;
    } else {
        *cursor = NULL;
    }
    return field;
}

static int inline_meta_ensure_symbol_slot(InlineMetaImport *meta, size_t id) {
    Symbol **new_symbols;
    size_t old_count;
    size_t i;

    if (!meta) return 0;
    if (id < meta->symbol_count) return 1;

    old_count = meta->symbol_count;
    new_symbols = realloc(meta->symbols, sizeof(Symbol *) * (id + 1));
    if (!new_symbols) return 0;

    meta->symbols = new_symbols;
    meta->symbol_count = id + 1;
    for (i = old_count; i < meta->symbol_count; i++) meta->symbols[i] = NULL;
    return 1;
}

static int inline_meta_ensure_scope_slot(InlineMetaImport *meta, size_t id) {
    Scope **new_scopes;
    size_t old_count;
    size_t i;

    if (!meta) return 0;
    if (id < meta->scope_count) return 1;

    old_count = meta->scope_count;
    new_scopes = realloc(meta->scopes, sizeof(Scope *) * (id + 1));
    if (!new_scopes) return 0;

    meta->scopes = new_scopes;
    meta->scope_count = id + 1;
    for (i = old_count; i < meta->scope_count; i++) meta->scopes[i] = NULL;
    return 1;
}

static int inline_meta_ensure_file_slot(InlineMetaImport *meta, size_t id) {
    char **new_files;
    size_t old_count;
    size_t i;

    if (!meta) return 0;
    if (id < meta->file_count) return 1;

    old_count = meta->file_count;
    new_files = realloc(meta->files, sizeof(char *) * (id + 1));
    if (!new_files) return 0;

    meta->files = new_files;
    meta->file_count = id + 1;
    for (i = old_count; i < meta->file_count; i++) meta->files[i] = NULL;
    return 1;
}

static int inline_meta_ensure_source_slot(InlineMetaImport *meta, size_t id) {
    InlineMetaImportedSource *new_sources;
    size_t old_count;
    size_t i;

    if (!meta) return 0;
    if (id < meta->source_count) return 1;

    old_count = meta->source_count;
    new_sources = realloc(meta->sources, sizeof(InlineMetaImportedSource) * (id + 1));
    if (!new_sources) return 0;

    meta->sources = new_sources;
    meta->source_count = id + 1;
    for (i = old_count; i < meta->source_count; i++) {
        meta->sources[i].source_node = NULL;
        meta->sources[i].provenance = AST_SOURCE_NONE;
    }
    return 1;
}

static int inline_meta_ensure_dependency_slot(InlineMetaImport *meta, size_t id) {
    InlineMetaImportedDependency *new_dependencies;
    size_t old_count;
    size_t i;

    if (!meta) return 0;
    if (id < meta->dependency_count) return 1;

    old_count = meta->dependency_count;
    new_dependencies = realloc(meta->dependencies, sizeof(InlineMetaImportedDependency) * (id + 1));
    if (!new_dependencies) return 0;

    meta->dependencies = new_dependencies;
    meta->dependency_count = id + 1;
    for (i = old_count; i < meta->dependency_count; i++) {
        meta->dependencies[i].fqname = NULL;
        meta->dependencies[i].type = NULL;
        meta->dependencies[i].args = NULL;
        meta->dependencies[i].symbol = NULL;
    }
    return 1;
}

static Scope *inline_meta_scope_by_id(InlineMetaImport *meta, size_t id) {
    if (!meta) return NULL;
    if (id >= meta->scope_count) return NULL;
    return meta->scopes[id];
}

static int inline_meta_import_file(Context *context, InlineMetaImport *meta, char *record) {
    char *cursor;
    char *id_field;
    char *name_field;
    char *name;
    size_t id;
    size_t name_length;

    (void)context;
    if (!meta || !record) return 0;

    cursor = record + 2;
    id_field = inline_meta_next_field(&cursor);
    name_field = inline_meta_next_field(&cursor);
    if (!id_field || !name_field) return 0;

    id = (size_t)strtoul(id_field, NULL, 10);
    if (!inline_meta_ensure_file_slot(meta, id)) return 0;

    name = inline_meta_hex_decode(name_field, &name_length);
    if (!name) return 0;

    if (meta->files[id]) free(meta->files[id]);
    meta->files[id] = name;
    return 1;
}

static int inline_meta_import_source(Context *context, InlineMetaImport *meta, char *record) {
    char *cursor;
    char *id_field;
    char *file_field;
    char *line_field;
    char *start_column_field;
    char *end_column_field;
    char *provenance_field;
    char *source_field;
    char *source_text;
    char *file_name;
    size_t id;
    size_t source_length;
    size_t start_offset;
    size_t end_offset;
    long file_id;
    int start_column;
    int end_column;
    SourceNode *source_node;

    if (!context || !meta || !record) return 0;

    cursor = record + 2;
    id_field = inline_meta_next_field(&cursor);
    file_field = inline_meta_next_field(&cursor);
    line_field = inline_meta_next_field(&cursor);
    start_column_field = inline_meta_next_field(&cursor);
    end_column_field = NULL;
    if (meta->version >= 5) end_column_field = inline_meta_next_field(&cursor);
    provenance_field = inline_meta_next_field(&cursor);
    source_field = inline_meta_next_field(&cursor);
    if (!id_field || !file_field || !line_field || !start_column_field || !provenance_field || !source_field) return 0;
    if (meta->version >= 5 && !end_column_field) return 0;

    id = (size_t)strtoul(id_field, NULL, 10);
    if (!inline_meta_ensure_source_slot(meta, id)) return 0;

    file_name = NULL;
    file_id = strtol(file_field, NULL, 10);
    if (file_id >= 0) {
        if ((size_t)file_id >= meta->file_count || !meta->files[file_id]) return 0;
        file_name = meta->files[file_id];
    }

    source_text = inline_meta_hex_decode(source_field, &source_length);
    if (!source_text) return 0;

    source_node = calloc(1, sizeof(SourceNode));
    if (!source_node) {
        free(source_text);
        return 0;
    }

    source_node->context = context;
    source_node->node_type = NOP;
    source_node->owned_file_name = file_name ? strdup(file_name) : NULL;
    source_node->file_name = source_node->owned_file_name;
    source_node->owned_source_text = source_text;
    source_node->line = atoi(line_field);
    if (meta->version >= 5) {
        start_column = atoi(start_column_field);
        end_column = atoi(end_column_field);
        if (start_column < 1) start_column = 1;
        if (end_column < start_column) end_column = start_column;
        start_offset = (size_t)(start_column - 1);
        end_offset = (size_t)(end_column - 1);
        if (start_offset > source_length) start_offset = source_length;
        if (end_offset > source_length) end_offset = source_length;
        if (end_offset < start_offset) end_offset = start_offset;
        source_node->source_start = source_text + start_offset;
        if (end_offset > start_offset) source_node->source_end = source_text + end_offset - 1;
        else source_node->source_end = source_node->source_start;
        source_node->column = start_column - 1;
    } else {
        source_node->source_start = source_text;
        source_node->source_end = source_length ? source_text + source_length - 1 : source_text;
        source_node->column = atoi(start_column_field);
    }
    source_node->free_list = context->source_free_list;
    if (source_node->free_list) source_node->node_number = source_node->free_list->node_number + 1;
    else source_node->node_number = 1;
    context->source_free_list = source_node;

    meta->sources[id].source_node = source_node;
    meta->sources[id].provenance = (char)atoi(provenance_field);
    return 1;
}

static int inline_meta_import_scope(Context *context, InlineMetaImport *meta, char *record) {
    char *cursor;
    char *id_field;
    char *parent_field;
    char *type_field;
    size_t id;
    long parent_id;
    ScopeType type;
    Scope *parent;
    Scope *scope;

    if (!context || !meta || !record) return 0;

    cursor = record + 2;
    id_field = inline_meta_next_field(&cursor);
    parent_field = inline_meta_next_field(&cursor);
    type_field = inline_meta_next_field(&cursor);
    if (!id_field || !parent_field || !type_field) return 0;

    id = (size_t)strtoul(id_field, NULL, 10);
    parent_id = strtol(parent_field, NULL, 10);
    type = (ScopeType)atoi(type_field);

    if (!inline_meta_ensure_scope_slot(meta, id)) return 0;

    if (id == 0) {
        if (type != SCOPE_PROCEDURE) return 0;
        meta->scopes[0] = meta->scope;
        return 1;
    }

    if (type == SCOPE_CLASS && parent_id < 0) {
        if (!meta->scope || !meta->scope->parent || meta->scope->parent->type != SCOPE_CLASS) return 0;
        meta->scopes[id] = meta->scope->parent;
        return 1;
    }

    if (type != SCOPE_LOCAL || parent_id < 0) return 0;
    parent = inline_meta_scope_by_id(meta, (size_t)parent_id);
    if (!parent) return 0;

    if (meta->scopes[id]) return 1;

    scope = scp_f(context, parent, NULL, NULL, type);
    if (!scope) return 0;

    meta->scopes[id] = scope;
    return 1;
}

static Symbol *inline_meta_find_or_create_symbol(Context *context,
                                                 Scope *scope,
                                                 const char *name,
                                                 ValueType type,
                                                 size_t dims,
                                                 int *dim_base,
                                                 int *dim_elements,
                                                 const char *class_name,
                                                 unsigned int flags) {
    ASTNode lookup_node;
    Symbol *symbol;

    if (!context || !scope || !name) return NULL;

    memset(&lookup_node, 0, sizeof(lookup_node));
    lookup_node.node_string = (char *)name;
    lookup_node.node_string_length = strlen(name);

    symbol = sym_lrsv(scope, &lookup_node);
    if (!symbol) symbol = sym_fn(scope, name, strlen(name));
    if (!symbol) return NULL;

    if (!inline_meta_set_symbol_shape(symbol, type, dims, dim_base, dim_elements, class_name)) return NULL;
    if (symbol->symbol_type == UNKNOWN_SYMBOL) symbol->symbol_type = VARIABLE_SYMBOL;
    if (symbol->status == SYM_STATUS_UNRESOLVED) symbol->status = SYM_STATUS_LOCAL_VAR;
    inline_meta_apply_symbol_flags(symbol, flags);

    return symbol;
}

static int inline_meta_import_symbol(Context *context, InlineMetaImport *meta, char *record) {
    char *cursor;
    char *id_field;
    char *scope_field;
    char *name_field;
    char *symbol_kind_field;
    char *type_field;
    char *dims_field;
    char *base_field;
    char *elements_field;
    char *class_field;
    char *flags_field;
    char *register_type_field;
    char *register_num_field;
    size_t id;
    size_t scope_id;
    size_t name_length;
    size_t dims;
    int *dim_base;
    int *dim_elements;
    char *class_name;
    char *name;
    Scope *scope;
    Symbol *symbol;

    cursor = record + 2;
    id_field = inline_meta_next_field(&cursor);
    scope_field = NULL;
    if (meta->version >= 2) scope_field = inline_meta_next_field(&cursor);
    name_field = inline_meta_next_field(&cursor);
    symbol_kind_field = NULL;
    if (meta->version >= 4) symbol_kind_field = inline_meta_next_field(&cursor);
    type_field = inline_meta_next_field(&cursor);
    dims_field = inline_meta_next_field(&cursor);
    base_field = NULL;
    elements_field = NULL;
    class_field = NULL;
    register_type_field = NULL;
    register_num_field = NULL;
    if (meta->version >= 3) {
        base_field = inline_meta_next_field(&cursor);
        elements_field = inline_meta_next_field(&cursor);
        class_field = inline_meta_next_field(&cursor);
    }
    flags_field = inline_meta_next_field(&cursor);
    if (meta->version >= 3) {
        register_type_field = inline_meta_next_field(&cursor);
        register_num_field = inline_meta_next_field(&cursor);
    }
    if (!id_field || !name_field || !type_field || !dims_field || !flags_field) return 0;
    if (meta->version >= 3 && (!base_field || !elements_field || !class_field)) return 0;

    id = (size_t)strtoul(id_field, NULL, 10);
    if (!inline_meta_ensure_symbol_slot(meta, id)) return 0;

    scope_id = 0;
    if (scope_field) scope_id = (size_t)strtoul(scope_field, NULL, 10);
    scope = meta->version >= 2 ? inline_meta_scope_by_id(meta, scope_id) : meta->scope;
    if (!scope) return 0;

    name = inline_meta_hex_decode(name_field, &name_length);
    if (!name) return 0;

    dims = (size_t)strtoul(dims_field, NULL, 10);
    dim_base = meta->version >= 3 ? inline_meta_int_list_decode(base_field, dims, 1) : NULL;
    dim_elements = meta->version >= 3 ? inline_meta_int_list_decode(elements_field, dims, 0) : NULL;
    class_name = meta->version >= 3 ? inline_meta_decode_optional_hex(class_field) : NULL;
    if (dims && (!dim_base || !dim_elements)) {
        free(name);
        if (dim_base) free(dim_base);
        if (dim_elements) free(dim_elements);
        if (class_name) free(class_name);
        return 0;
    }

    symbol = inline_meta_find_or_create_symbol(context,
                                               scope,
                                               name,
                                               (ValueType)atoi(type_field),
                                               dims,
                                               dim_base,
                                               dim_elements,
                                               class_name,
                                               (unsigned int)strtoul(flags_field, NULL, 10));
    free(name);
    if (dim_base) free(dim_base);
    if (dim_elements) free(dim_elements);
    if (class_name) free(class_name);
    if (!symbol) return 0;
    if (symbol_kind_field) symbol->symbol_type = (SymbolType)atoi(symbol_kind_field);

    if (inline_symbol_is_class_attribute(symbol) &&
        register_type_field && register_type_field[0] &&
        register_num_field && register_num_field[0]) {
        symbol->register_type = register_type_field[0];
        symbol->register_num = atoi(register_num_field);
    }

    meta->symbols[id] = symbol;
    return 1;
}

static Symbol *inline_meta_resolve_dependency_symbol(Context *context,
                                                     InlineMetaImport *meta,
                                                     const char *fqname) {
    Symbol *symbol;
    ASTNode *lookup;
    char *lookup_name;

    if (!context || !context->ast || !fqname || !*fqname) return NULL;

    symbol = sym_rfqn(context->ast, fqname);
    if (symbol && symbol->symbol_type == FUNCTION_SYMBOL) return symbol;

    symbol = sym_rfqv(context->ast, fqname);
    if (symbol && symbol->symbol_type == FUNCTION_SYMBOL) return symbol;

    lookup_name = strdup(fqname);
    if (!lookup_name) return NULL;

    lookup = ast_ftt(context, FUNCTION, lookup_name);
    if (!lookup) {
        free(lookup_name);
        return NULL;
    }
    lookup->free_node_string = 1;
    lookup->scope = meta ? meta->scope : NULL;

    symbol = sym_imfn(context, lookup);
    if (symbol && symbol->symbol_type == FUNCTION_SYMBOL) return symbol;

    symbol = sym_rfqn(context->ast, fqname);
    if (symbol && symbol->symbol_type == FUNCTION_SYMBOL) return symbol;

    return NULL;
}

static int inline_meta_nullable_strings_equal(const char *left, const char *right) {
    if (!left || !right) return left == right || (!left && right && !*right) || (!right && left && !*left);
    return strcmp(left, right) == 0;
}

static int inline_meta_dependency_signature_matches(Context *context,
                                                    const char *fqname,
                                                    const char *type,
                                                    const char *args) {
    imported_func *func;
    char *lookup_name;
    int found;

    if (!context || !fqname) return 0;

    lookup_name = strdup(fqname);
    if (!lookup_name) return 0;
    func = NULL;
    found = src_fqfu(context, 0, lookup_name, &func);
    free(lookup_name);

    if (!found || !func) return 1;
    if (func->is_variable) return 0;
    if (!inline_meta_nullable_strings_equal(func->type, type)) return 0;
    if (!inline_meta_nullable_strings_equal(func->args, args)) return 0;
    return 1;
}

static int inline_meta_import_dependency(Context *context, InlineMetaImport *meta, char *record) {
    char *cursor;
    char *id_field;
    char *fqname_field;
    char *type_field;
    char *args_field;
    size_t id;
    size_t length;
    char *fqname;
    char *type;
    char *args;
    Symbol *symbol;

    if (!context || !meta || !record) return 0;

    cursor = record + 2;
    id_field = inline_meta_next_field(&cursor);
    fqname_field = inline_meta_next_field(&cursor);
    type_field = inline_meta_next_field(&cursor);
    args_field = inline_meta_next_field(&cursor);
    if (!id_field || !fqname_field || !type_field || !args_field) return 0;

    id = (size_t)strtoul(id_field, NULL, 10);
    if (!inline_meta_ensure_dependency_slot(meta, id)) return 0;

    fqname = inline_meta_hex_decode(fqname_field, &length);
    type = inline_meta_hex_decode(type_field, &length);
    args = inline_meta_hex_decode(args_field, &length);
    if (!fqname || !type || !args) {
        if (fqname) free(fqname);
        if (type) free(type);
        if (args) free(args);
        return 0;
    }

    symbol = inline_meta_resolve_dependency_symbol(context, meta, fqname);
    if (!symbol || !inline_meta_dependency_signature_matches(context, fqname, type, args)) {
        free(fqname);
        free(type);
        free(args);
        return 0;
    }

    if (meta->dependencies[id].fqname) free(meta->dependencies[id].fqname);
    if (meta->dependencies[id].type) free(meta->dependencies[id].type);
    if (meta->dependencies[id].args) free(meta->dependencies[id].args);
    meta->dependencies[id].fqname = fqname;
    meta->dependencies[id].type = type;
    meta->dependencies[id].args = args;
    meta->dependencies[id].symbol = symbol;
    return 1;
}

static int inline_meta_push_node(InlineMetaImport *meta, ASTNode *node) {
    ASTNode **new_stack;

    new_stack = realloc(meta->stack, sizeof(ASTNode *) * (meta->stack_count + 1));
    if (!new_stack) return 0;

    meta->stack = new_stack;
    meta->stack[meta->stack_count] = node;
    meta->stack_count++;
    return 1;
}

static int inline_meta_import_node(Context *context, InlineMetaImport *meta, char *record) {
    char *cursor;
    char *scope_field;
    char *source_field;
    char *node_type_field;
    char *value_type_field;
    char *target_type_field;
    char *value_dims_field;
    char *target_dims_field;
    char *value_base_field;
    char *value_elements_field;
    char *target_base_field;
    char *target_elements_field;
    char *value_class_field;
    char *target_class_field;
    char *flags_field;
    char *symbol_field;
    char *dependency_field;
    char *symbol_read_field;
    char *symbol_write_field;
    char *int_field;
    char *bool_field;
    char *float_field;
    char *node_string_field;
    char *decimal_field;
    ASTNode *node;
    char *node_string;
    char *decimal_string;
    size_t node_string_length;
    size_t decimal_length;
    size_t scope_id;
    size_t value_dims;
    size_t target_dims;
    int *value_dim_base;
    int *value_dim_elements;
    int *target_dim_base;
    int *target_dim_elements;
    char *value_class;
    char *target_class;
    long symbol_id;
    long source_id;
    long dependency_id;
    unsigned int symbol_read_usage;
    unsigned int symbol_write_usage;
    unsigned int flags;
    Scope *node_scope;

    cursor = record + 2;
    scope_field = NULL;
    if (meta->version >= 2) scope_field = inline_meta_next_field(&cursor);
    source_field = NULL;
    if (meta->version >= 4) source_field = inline_meta_next_field(&cursor);
    node_type_field = inline_meta_next_field(&cursor);
    value_type_field = inline_meta_next_field(&cursor);
    target_type_field = inline_meta_next_field(&cursor);
    value_dims_field = inline_meta_next_field(&cursor);
    target_dims_field = inline_meta_next_field(&cursor);
    value_base_field = NULL;
    value_elements_field = NULL;
    target_base_field = NULL;
    target_elements_field = NULL;
    value_class_field = NULL;
    target_class_field = NULL;
    if (meta->version >= 3) {
        value_base_field = inline_meta_next_field(&cursor);
        value_elements_field = inline_meta_next_field(&cursor);
        target_base_field = inline_meta_next_field(&cursor);
        target_elements_field = inline_meta_next_field(&cursor);
        value_class_field = inline_meta_next_field(&cursor);
        target_class_field = inline_meta_next_field(&cursor);
    }
    flags_field = inline_meta_next_field(&cursor);
    symbol_field = inline_meta_next_field(&cursor);
    dependency_field = NULL;
    if (meta->version >= 4) dependency_field = inline_meta_next_field(&cursor);
    symbol_read_field = NULL;
    symbol_write_field = NULL;
    if (meta->version >= 3) {
        symbol_read_field = inline_meta_next_field(&cursor);
        symbol_write_field = inline_meta_next_field(&cursor);
    }
    int_field = inline_meta_next_field(&cursor);
    bool_field = inline_meta_next_field(&cursor);
    float_field = inline_meta_next_field(&cursor);
    node_string_field = inline_meta_next_field(&cursor);
    decimal_field = inline_meta_next_field(&cursor);
    if (!node_type_field || !value_type_field || !target_type_field ||
        !value_dims_field || !target_dims_field || !flags_field ||
        !symbol_field || !int_field || !bool_field || !float_field ||
        !node_string_field || !decimal_field) {
        return 0;
    }
    if (meta->version >= 4 && !dependency_field) return 0;
    if (meta->version >= 3 &&
        (!value_base_field || !value_elements_field || !target_base_field ||
         !target_elements_field || !value_class_field || !target_class_field ||
         !symbol_read_field || !symbol_write_field)) {
        return 0;
    }
    dependency_id = -1;
    if (meta->version >= 4) {
        dependency_id = strtol(dependency_field, NULL, 10);
        if (dependency_id < -1) return 0;
    }

    scope_id = 0;
    if (scope_field) scope_id = (size_t)strtoul(scope_field, NULL, 10);
    node_scope = meta->version >= 2 ? inline_meta_scope_by_id(meta, scope_id) : meta->scope;
    if (!node_scope) return 0;

    node = ast_ft(context, (NodeType)atoi(node_type_field));
    if (!node) return 0;

    value_dims = (size_t)strtoul(value_dims_field, NULL, 10);
    target_dims = (size_t)strtoul(target_dims_field, NULL, 10);
    value_dim_base = meta->version >= 3 ? inline_meta_int_list_decode(value_base_field, value_dims, 1) : NULL;
    value_dim_elements = meta->version >= 3 ? inline_meta_int_list_decode(value_elements_field, value_dims, 0) : NULL;
    target_dim_base = meta->version >= 3 ? inline_meta_int_list_decode(target_base_field, target_dims, 1) : NULL;
    target_dim_elements = meta->version >= 3 ? inline_meta_int_list_decode(target_elements_field, target_dims, 0) : NULL;
    value_class = meta->version >= 3 ? inline_meta_decode_optional_hex(value_class_field) : NULL;
    target_class = meta->version >= 3 ? inline_meta_decode_optional_hex(target_class_field) : NULL;
    if ((value_dims && (!value_dim_base || !value_dim_elements)) ||
        (target_dims && (!target_dim_base || !target_dim_elements))) {
        if (value_dim_base) free(value_dim_base);
        if (value_dim_elements) free(value_dim_elements);
        if (target_dim_base) free(target_dim_base);
        if (target_dim_elements) free(target_dim_elements);
        if (value_class) free(value_class);
        if (target_class) free(target_class);
        return 0;
    }
    ast_set_value_type(0,
                       node,
                       (ValueType)atoi(value_type_field),
                       value_dims,
                       value_dim_base,
                       value_dim_elements,
                       value_class);
    ast_set_target_type(0,
                        node,
                        (ValueType)atoi(target_type_field),
                        target_dims,
                        target_dim_base,
                        target_dim_elements,
                        target_class);
    if (value_dim_base) free(value_dim_base);
    if (value_dim_elements) free(value_dim_elements);
    if (target_dim_base) free(target_dim_base);
    if (target_dim_elements) free(target_dim_elements);
    if (value_class) free(value_class);
    if (target_class) free(target_class);
    node->int_value = (rxinteger)atoll(int_field);
    node->bool_value = atoi(bool_field);
    node->float_value = atof(float_field);
    node->scope = node_scope;
    flags = (unsigned int)strtoul(flags_field, NULL, 10);
    inline_meta_apply_node_flags(node, flags);
    if ((flags & INLINE_META_NODE_SCOPE_DEF) && node_scope != meta->scope) node_scope->defining_node = node;

    source_id = source_field ? strtol(source_field, NULL, 10) : -1;
    if (source_id >= 0) {
        SourceNode *source_node;
        if ((size_t)source_id >= meta->source_count) return 0;
        source_node = meta->sources[source_id].source_node;
        if (!source_node) return 0;
        node->file_name = source_node->file_name;
        node->line = source_node->line;
        node->column = source_node->column;
        node->source_start = source_node->source_start;
        node->source_end = source_node->source_end;
        ast_set_primary_source_node(node,
                                    source_node,
                                    (ASTSourceProvenance)meta->sources[source_id].provenance);
    }

    node_string = inline_meta_hex_decode(node_string_field, &node_string_length);
    decimal_string = inline_meta_hex_decode(decimal_field, &decimal_length);
    if (!node_string || !decimal_string) {
        if (node_string) free(node_string);
        if (decimal_string) free(decimal_string);
        return 0;
    }

    ast_sstr(node, node_string, node_string_length);
    if (decimal_length) node->decimal_value = decimal_string;
    else free(decimal_string);

    symbol_id = strtol(symbol_field, NULL, 10);
    if (dependency_id >= 0) {
        Symbol *symbol;

        if (symbol_id >= 0 || node->node_type != FUNCTION) return 0;
        if ((size_t)dependency_id >= meta->dependency_count) return 0;
        symbol = meta->dependencies[dependency_id].symbol;
        if (!symbol || symbol->symbol_type != FUNCTION_SYMBOL) return 0;
        sym_adnd(symbol, node, 1, 0);
    }
    if (symbol_id >= 0) {
        Symbol *symbol;
        if ((size_t)symbol_id >= meta->symbol_count) return 0;
        symbol = meta->symbols[symbol_id];
        if (!symbol) return 0;

        if (meta->version >= 3) {
            symbol_read_usage = (unsigned int)strtoul(symbol_read_field, NULL, 10);
            symbol_write_usage = (unsigned int)strtoul(symbol_write_field, NULL, 10);
        } else if (node->node_type == VAR_TARGET) {
            symbol_read_usage = 0;
            symbol_write_usage = 1;
        } else {
            symbol_read_usage = 1;
            symbol_write_usage = 0;
        }
        sym_adnd(symbol, node, symbol_read_usage, symbol_write_usage);
    }

    if (meta->stack_count) add_ast(meta->stack[meta->stack_count - 1], node);
    else if (meta->version >= 3) {
        if (meta->tree_section == 1) meta->args_root = node;
        else if (meta->tree_section == 2) meta->root = node;
        else return 0;
    } else {
        meta->root = node;
    }

    return inline_meta_push_node(meta, node);
}

static Symbol *inline_meta_clone_signature_symbol(Context *context, Scope *scope, Symbol *old_symbol) {
    Symbol *symbol;

    if (!context || !scope || !old_symbol || !old_symbol->name) return NULL;

    symbol = inline_meta_find_or_create_symbol(context,
                                               scope,
                                               old_symbol->name,
                                               old_symbol->type,
                                               old_symbol->value_dims,
                                               old_symbol->dim_base,
                                               old_symbol->dim_elements,
                                               old_symbol->value_class,
                                               inline_meta_symbol_flags(old_symbol));
    if (!symbol) return NULL;

    symbol->symbol_type = old_symbol->symbol_type;
    symbol->status = old_symbol->status;
    symbol->fixed_args = old_symbol->fixed_args;
    symbol->has_vargs = old_symbol->has_vargs;
    symbol->needs_default_initiation = old_symbol->needs_default_initiation;
    symbol->register_num = UNSET_REGISTER;
    symbol->register_type = 'r';
    return symbol;
}

static ASTNode *inline_meta_clone_signature_with_symbols(Context *context, ASTNode *node, Scope *scope) {
    ASTNode *clone;
    ASTNode *child;

    if (!context || !node || !scope) return NULL;

    clone = ast_dup(context, node);
    if (!clone) return NULL;
    clone->scope = scope;
    if (node->symbolNode && node->symbolNode->symbol) {
        Symbol *symbol = inline_meta_clone_signature_symbol(context, scope, node->symbolNode->symbol);
        if (!symbol) return NULL;
        sym_adnd(symbol, clone, node->symbolNode->readUsage, node->symbolNode->writeUsage);
    }

    child = node->child;
    while (child) {
        ASTNode *child_clone = inline_meta_clone_signature_with_symbols(context, child, scope);
        if (!child_clone) return NULL;
        add_ast(clone, child_clone);
        child = child->sibling;
    }

    return clone;
}

static int inline_meta_clone_missing_scope_symbols(Scope *source, Scope *target) {
    Symbol **symbols;
    size_t i;

    if (!source || !target) return 0;

    symbols = scp_syms(source);
    if (!symbols) return 1;

    for (i = 0; symbols[i]; i++) {
        ASTNode lookup_node;
        Symbol *old_symbol;
        Symbol *new_symbol;

        old_symbol = symbols[i];
        if (!old_symbol || old_symbol->symbol_type == FUNCTION_SYMBOL || !old_symbol->name) continue;

        memset(&lookup_node, 0, sizeof(lookup_node));
        lookup_node.node_string = old_symbol->name;
        lookup_node.node_string_length = strlen(old_symbol->name);
        if (sym_lrsv(target, &lookup_node)) continue;

        new_symbol = sym_dup(target, old_symbol);
        if (!new_symbol) {
            free(symbols);
            return 0;
        }

        new_symbol->register_num = UNSET_REGISTER;
        new_symbol->register_type = 'r';
        new_symbol->meta_emitted = 0;
        new_symbol->init_emitted = 0;
        new_symbol->defines_scope = NULL;
        new_symbol->ast_template = NULL;
        new_symbol->is_inlinable = 0;
    }

    free(symbols);
    return 1;
}

static ASTNode *inline_meta_create_template_proc(Context *context,
                                                 ASTNode *proc,
                                                 Scope **scope_out,
                                                 int skip_args) {
    ASTNode *template_proc;
    ASTNode *child;
    Scope *template_scope;

    if (scope_out) *scope_out = NULL;
    if (!context || !proc || !proc->scope || !proc->symbolNode || !proc->symbolNode->symbol) return NULL;

    template_proc = ast_dup(context, proc);
    if (!template_proc) return NULL;

    template_scope = scp_f(context,
                           (proc->node_type == METHOD || proc->node_type == FACTORY) ? proc->scope->parent : NULL,
                           template_proc,
                           NULL,
                           SCOPE_PROCEDURE);
    if (!template_scope) return NULL;
    if (proc->node_type == METHOD || proc->node_type == FACTORY) template_proc->parent = proc->parent;
    if (proc->scope->name) template_scope->name = strdup(proc->scope->name);
    inline_copy_numeric_context(template_scope, proc->scope);

    template_proc->scope = template_scope;
    if (!inline_meta_clone_missing_scope_symbols(proc->scope, template_scope)) return NULL;
    sym_adnd(proc->symbolNode->symbol,
             template_proc,
             proc->symbolNode->readUsage,
             proc->symbolNode->writeUsage);

    child = proc->child;
    while (child) {
        if (child->node_type != INSTRUCTIONS && child->node_type != NOP &&
            (!skip_args || child->node_type != ARGS)) {
            ASTNode *child_clone = inline_meta_clone_signature_with_symbols(context, child, template_scope);
            if (!child_clone) return NULL;
            add_ast(template_proc, child_clone);
        }
        child = child->sibling;
    }

    if (scope_out) *scope_out = template_scope;
    return template_proc;
}

static int inline_meta_finish_template(ASTNode *proc,
                                       ASTNode *template_proc,
                                       ASTNode *args_root,
                                       ASTNode *body_root) {
    if (!proc || !template_proc || !proc->symbolNode || !proc->symbolNode->symbol || !body_root) return 0;

    if (args_root) add_ast(template_proc, args_root);
    add_ast(template_proc, body_root);

    proc->symbolNode->symbol->is_inlinable = 1;
    proc->symbolNode->symbol->ast_template = template_proc;
    return 1;
}

static char *inline_meta_next_record(char **cursor) {
    char *record;
    char *separator;

    if (!cursor || !*cursor) return NULL;

    record = *cursor;
    separator = strchr(record, ';');
    if (separator) {
        *separator = '\0';
        *cursor = separator + 1;
    } else {
        *cursor = NULL;
    }

    return record;
}

static int inline_meta_attach_to_proc(Context *context, ASTNode *proc, const char *payload) {
    InlineMetaImport meta;
    ASTNode *template_proc;
    Scope *template_scope;
    char *copy;
    char *cursor;
    char *record;

    if (!context || !rxcp_inline_payload_is_supported(payload)) return 0;
    if (!proc || !proc->scope || !proc->symbolNode || !proc->symbolNode->symbol) return 0;

    memset(&meta, 0, sizeof(meta));
    meta.ok = 1;

    copy = strdup(payload);
    if (!copy) return 0;

    cursor = copy;
    record = inline_meta_next_record(&cursor);
    if (!record) {
        free(copy);
        return 0;
    }
    if (strcmp(record, "I1") == 0) meta.version = 1;
    else if (strcmp(record, "I2") == 0) meta.version = 2;
    else if (strcmp(record, "I3") == 0) meta.version = 3;
    else if (strcmp(record, "I4") == 0) meta.version = 4;
    else if (strcmp(record, "I5") == 0) meta.version = 5;
    else {
        free(copy);
        return 0;
    }

    template_scope = NULL;
    template_proc = inline_meta_create_template_proc(context, proc, &template_scope, meta.version >= 3);
    if (!template_proc || !template_scope) {
        free(copy);
        return 0;
    }

    meta.scope = template_scope;
    meta.tree_section = meta.version >= 3 ? 0 : 2;

    if (!inline_meta_ensure_scope_slot(&meta, 0)) {
        free(copy);
        return 0;
    }
    meta.scopes[0] = meta.scope;

    while ((record = inline_meta_next_record(&cursor)) != NULL) {
        if (strcmp(record, "<") == 0) {
            if (!meta.stack_count) {
                meta.ok = 0;
                break;
            }
            meta.stack_count--;
        } else if (strcmp(record, "a") == 0) {
            if (meta.version < 3 || meta.stack_count || meta.args_root) {
                meta.ok = 0;
                break;
            }
            meta.tree_section = 1;
        } else if (strcmp(record, "b") == 0) {
            if (meta.version < 3 || meta.stack_count || meta.root) {
                meta.ok = 0;
                break;
            }
            meta.tree_section = 2;
        } else if (record[0] == 'q' && record[1] == ',') {
            if (meta.version < 2 || !inline_meta_import_scope(context, &meta, record)) {
                meta.ok = 0;
                break;
            }
        } else if (record[0] == 'f' && record[1] == ',') {
            if (meta.version < 4 || !inline_meta_import_file(context, &meta, record)) {
                meta.ok = 0;
                break;
            }
        } else if (record[0] == 'u' && record[1] == ',') {
            if (meta.version < 4 || !inline_meta_import_source(context, &meta, record)) {
                meta.ok = 0;
                break;
            }
        } else if (record[0] == 's' && record[1] == ',') {
            if (!inline_meta_import_symbol(context, &meta, record)) {
                meta.ok = 0;
                break;
            }
        } else if (record[0] == 'd' && record[1] == ',') {
            if (meta.version < 4 || !inline_meta_import_dependency(context, &meta, record)) {
                meta.ok = 0;
                break;
            }
        } else if (record[0] == '>' && record[1] == ',') {
            if (!inline_meta_import_node(context, &meta, record)) {
                meta.ok = 0;
                break;
            }
        } else {
            meta.ok = 0;
            break;
        }
    }

    if (meta.stack_count != 0 || !meta.root || meta.root->node_type != INSTRUCTIONS) meta.ok = 0;
    if (meta.version >= 3 && (!meta.args_root || meta.args_root->node_type != ARGS)) meta.ok = 0;

    free(copy);
    if (meta.files) {
        size_t i;
        for (i = 0; i < meta.file_count; i++) {
            if (meta.files[i]) free(meta.files[i]);
        }
    }
    free(meta.files);
    free(meta.sources);
    if (meta.dependencies) {
        size_t i;
        for (i = 0; i < meta.dependency_count; i++) {
            if (meta.dependencies[i].fqname) free(meta.dependencies[i].fqname);
            if (meta.dependencies[i].type) free(meta.dependencies[i].type);
            if (meta.dependencies[i].args) free(meta.dependencies[i].args);
        }
    }
    free(meta.dependencies);
    free(meta.symbols);
    free(meta.scopes);
    free(meta.stack);

    if (!meta.ok) return 0;

    return inline_meta_finish_template(proc,
                                       template_proc,
                                       meta.version >= 3 ? meta.args_root : NULL,
                                       meta.root);
}

int rxcp_inline_attach_imported_body(Context *context, const char *payload) {
    ASTNode *proc;

    if (!context || !context->ast || !rxcp_inline_payload_is_supported(payload)) return 0;

    proc = inline_meta_find_first_procedure(context->ast);
    if (!proc) return 0;

    return inline_meta_attach_to_proc(context, proc, payload);
}

int rxcp_inline_attach_imported_symbol(Context *context, Symbol *symbol, const char *payload) {
    ASTNode *proc;

    if (!context || !symbol || !rxcp_inline_payload_is_supported(payload)) return 0;

    proc = sym_proc(symbol);
    if (!proc) return 0;

    return inline_meta_attach_to_proc(context, proc, payload);
}
