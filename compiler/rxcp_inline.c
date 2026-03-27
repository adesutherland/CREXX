/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcp_val.h"
#include "rxcp_ast.h"
#include "rxcpbgmr.h"
#include "rxcpdary.h"
#include "rxcp_sym.h"

#define INLINE_MAX_NODES 25

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
    Symbol **varg_symbols;
    size_t varg_count;
    Symbol *varg_array_symbol;
} InlineCloneState;

typedef struct {
    ASTNode *root_proc;
    int node_count;
    int return_count;
    int has_unsupported_varg_access;
    size_t max_required_varg_index;
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
static int inline_initialise_varg_array(Context *context,
                                        ASTNode *instr_list,
                                        Scope *inline_scope,
                                        ASTNode *varg_arg,
                                        ASTNode *source_node,
                                        InlineCloneState *state);
static ASTNode *inline_find_varg_arg(ASTNode *proc_def);
static int inline_call_arity_matches(ASTNode *call_node, Symbol *proc_sym, size_t *varg_count_out);
static int inline_analyse_varg_usage(ASTNode *proc_def, int *unsupported_out, size_t *max_required_index_out);
static int inline_call_is_recursive(ASTNode *call_node, Symbol *proc_sym);
static int inline_analyse_return_shape(ASTNode *proc_def, InlineReturnShape *shape_out);
static int inline_count_return_nodes(ASTNode *node);
static int inline_rewrite_return_nodes(Context *context,
                                       ASTNode *node,
                                       ASTNode *block_expr,
                                       Scope *inline_scope,
                                       int allow_dummy_return,
                                       ValueType proc_type);

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

static int inline_node_has_array_shape(ASTNode *node) {
    if (!node) return 0;
    return node->value_dims > 0 || node->target_dims > 0;
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

static int inline_node_needs_attr_copy(ASTNode *node) {
    if (!node) return 0;

    if (inline_node_has_array_shape(node)) return 1;

    return node->value_type == TP_OBJECT ||
           node->value_type == TP_BINARY ||
           node->target_type == TP_OBJECT ||
           node->target_type == TP_BINARY;
}

static int inline_formal_needs_isolated_copy(ASTNode *formal_target, ASTNode *param_arg) {
    if (!formal_target) return 0;
    if (inline_node_has_array_shape(formal_target)) return 1;
    if (formal_target->value_type == TP_BINARY || formal_target->target_type == TP_BINARY) return 1;

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
    instr->token = lhs_node->token;
    instr->line = lhs_node->line;
    instr->column = lhs_node->column;
    instr->source_start = lhs_node->source_start;
    instr->source_end = lhs_node->source_end;
    instr->token_start = lhs_node->token_start;
    instr->token_end = lhs_node->token_end;

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
    node->token = source_node->token;
    node->line = source_node->line;
    node->column = source_node->column;
    node->source_start = source_node->source_start;
    node->source_end = source_node->source_end;
    node->token_start = source_node->token_start;
    node->token_end = source_node->token_end;
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

    actual_count = inline_count_siblings(call_node->child);
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
        }
    } else if (node->node_type == OP_ARG_IX_EXISTS) {
        if (!node->child) {
            check->has_unsupported_varg_access = 1;
        }
    }

    return result_normal;
}

static int inline_analyse_varg_usage(ASTNode *proc_def, int *unsupported_out, size_t *max_required_index_out) {
    InlinableCheck check;

    if (unsupported_out) *unsupported_out = 0;
    if (max_required_index_out) *max_required_index_out = 0;
    if (!proc_def) return 0;

    memset(&check, 0, sizeof(check));
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
    node->token = source_node->token;
    node->line = source_node->line;
    node->column = source_node->column;
    node->source_start = source_node->source_start;
    node->source_end = source_node->source_end;
    node->token_start = source_node->token_start;
    node->token_end = source_node->token_end;
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

    return replacement;
}

static int inline_register_ref_actual(Context *context,
                                      ASTNode *instr_list,
                                      Scope *inline_scope,
                                      ASTNode *formal_target,
                                      ASTNode *actual_arg,
                                      InlineCloneState *state) {
    InlineRefActualEntry *new_entries;
    InlineRefActualEntry *entry;
    ASTNode *child;
    size_t child_count;
    size_t child_index;

    if (!context || !instr_list || !inline_scope || !formal_target || !actual_arg || !state) return 0;
    if (!formal_target->symbolNode || !formal_target->symbolNode->symbol) return 0;
    if (!inline_is_supported_ref_actual(actual_arg)) return 0;

    entry = inline_find_ref_actual(state, formal_target->symbolNode->symbol);
    if (entry) return 1;

    child_count = inline_count_siblings(actual_arg->child);
    new_entries = realloc(state->ref_entries, sizeof(InlineRefActualEntry) * (state->ref_count + 1));
    if (!new_entries) return 0;

    state->ref_entries = new_entries;
    entry = &state->ref_entries[state->ref_count];
    memset(entry, 0, sizeof(*entry));
    entry->formal_symbol = formal_target->symbolNode->symbol;
    entry->actual_source = actual_arg;
    entry->captured_count = child_count;

    if (child_count) {
        entry->captured_symbols = calloc(child_count, sizeof(Symbol *));
        if (!entry->captured_symbols) return 0;
    }

    child = actual_arg->child;
    child_index = 0;
    while (child) {
        Symbol *temp_symbol;
        ASTNode *capture_assign;
        ASTNode *capture_lhs;
        ASTNode *capture_rhs;

        temp_symbol = inline_create_temp_symbol(context, inline_scope, child, "__inline_ref", child_index);
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
    instr->token = source_node->token;
    instr->line = source_node->line;
    instr->column = source_node->column;
    instr->source_start = source_node->source_start;
    instr->source_end = source_node->source_end;
    instr->token_start = source_node->token_start;
    instr->token_end = source_node->token_end;

    if (arg1) add_ast(instr, arg1);
    if (arg2) add_ast(instr, arg2);
    if (arg3) add_ast(instr, arg3);

    return instr;
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

    if (!context || !instr_list || !inline_scope || !proc_def || !call_node || !proc_sym || !clone_state) return 0;

    if (!inline_call_arity_matches(call_node, proc_sym, NULL)) return 0;

    param_list = ast_chld(proc_def, ARGS, 0);
    param_arg = param_list ? param_list->child : NULL;
    actual_arg = call_node->child;
    varg_arg = inline_find_varg_arg(proc_def);

    while (param_arg) {
        ASTNode *formal_target;
        ASTNode *formal_default;
        ASTNode *bind_assign;
        ASTNode *bind_lhs;
        ASTNode *bind_rhs;
        ASTNode *bind_source;

        if (param_arg == varg_arg) break;

        formal_target = inline_formal_target(param_arg);
        formal_default = inline_formal_default(param_arg);
        if (!formal_target || !actual_arg) return 0;

        if (param_arg->is_ref_arg && !inline_is_missing_actual(actual_arg)) {
            if (!inline_register_ref_actual(context, instr_list, inline_scope, formal_target, actual_arg, clone_state)) {
                return 0;
            }
            param_arg = param_arg->sibling;
            actual_arg = actual_arg->sibling;
            continue;
        }

        if (inline_is_missing_actual(actual_arg)) {
            if (!param_arg->is_opt_arg || !formal_default) return 0;
        }

        if (inline_node_is_plain_object(formal_target) &&
            param_arg->is_const_arg &&
            !inline_is_missing_actual(actual_arg)) {
            if (!inline_register_ref_actual(context, instr_list, inline_scope, formal_target, actual_arg, clone_state)) {
                return 0;
            }
            param_arg = param_arg->sibling;
            actual_arg = actual_arg->sibling;
            continue;
        }

        bind_assign = ast_f(context, ASSIGN, formal_target->token);
        bind_assign->scope = inline_scope;
        bind_assign->value_type = formal_target->value_type;
        bind_assign->target_type = formal_target->target_type;

        bind_lhs = inline_clone_subtree(context, formal_target, clone_state);
        if (inline_is_missing_actual(actual_arg)) {
            bind_source = formal_default;
        } else {
            bind_source = actual_arg;
        }

        if (inline_formal_needs_isolated_copy(formal_target, param_arg) &&
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

        if (!bind_lhs || !bind_rhs) return 0;

        if (inline_formal_needs_isolated_copy(formal_target, param_arg)) {
            ASTNode *bind_copy;
            ASTNode *attr_copy;

            bind_copy = inline_create_register_copy_instr(context, inline_scope, "copy", bind_lhs, bind_rhs);
            attr_copy = inline_create_register_copy_instr(context, inline_scope, "acopy", bind_lhs, bind_rhs);
            if (!bind_copy || !attr_copy) return 0;

            add_ast(instr_list, bind_copy);
            add_ast(instr_list, attr_copy);
        } else {
            add_ast(bind_assign, bind_lhs);
            add_ast(bind_assign, bind_rhs);
            add_ast(instr_list, bind_assign);
        }

        param_arg = param_arg->sibling;
        actual_arg = actual_arg->sibling;
    }

    if (varg_arg) {
        if (!inline_capture_varg_actuals(context, instr_list, inline_scope, varg_arg, actual_arg, clone_state)) {
            return 0;
        }
        actual_arg = NULL;
        param_arg = varg_arg ? varg_arg->sibling : param_arg;
    }

    if (actual_arg || param_arg) return 0;

    return 1;
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

    if (!inline_append_scope_map_entry(state, old_scope, new_scope)) return NULL;
    if (!inline_duplicate_scope_symbols(old_scope, new_scope, state)) return NULL;

    return new_scope;
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

        if (!node->child) return NULL;
        if (!inline_varg_index_from_node(node->child, &index)) {
            return inline_build_dynamic_varg_value(context, node, current_scope, state);
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

    node_scope = current_scope;
    if (node->scope && node->scope->defining_node == node) {
        node_scope = inline_find_mapped_scope(state, node->scope);
        if (!node_scope) {
            node_scope = inline_clone_scope(context, node->scope, current_scope, new_node, state);
            if (!node_scope) return NULL;
        }
        new_node->scope = node_scope;
        if (!inline_append_node_map_entry(state, node, new_node)) return NULL;
    } else {
        new_node->scope = current_scope ? current_scope : node->scope;
    }

    if (node->association) {
        mapped_association = inline_find_mapped_node(state, node->association);
        if (mapped_association) new_node->association = mapped_association;
    }

    if (node->symbolNode && node->symbolNode->symbol) {
        mapped_symbol = inline_find_mapped_symbol(state, node->symbolNode->symbol);
        if (!mapped_symbol) mapped_symbol = node->symbolNode->symbol;
        sym_adnd(mapped_symbol, new_node, node->symbolNode->readUsage, node->symbolNode->writeUsage);
    }

    child = node->child;
    while (child) {
        add_ast(new_node, inline_clone_subtree_in_scope(context, child, state, node_scope));
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
    if (state->varg_symbols) free(state->varg_symbols);
    state->symbol_entries = NULL;
    state->symbol_count = 0;
    state->scope_entries = NULL;
    state->scope_count = 0;
    state->node_entries = NULL;
    state->node_count = 0;
    state->ref_entries = NULL;
    state->ref_count = 0;
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
    sink_target->token = source_node->token;
    sink_target->line = source_node->line;
    sink_target->column = source_node->column;
    sink_target->source_start = source_node->source_start;
    sink_target->source_end = source_node->source_end;
    sink_target->token_start = source_node->token_start;
    sink_target->token_end = source_node->token_end;
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
            return INLINE_EXPR_CONTEXT_EAGER_CALL_ARGUMENT;

        case IF:
        case WHILE:
        case UNTIL:
        case FOR:
        case TO:
        case BY:
            return INLINE_EXPR_CONTEXT_CONTROL_CONSUMER;

        case OP_AND:
        case OP_OR:
        case OP_NOT:
            return INLINE_EXPR_CONTEXT_SHORT_CIRCUIT_OPERATOR;

        default:
            return INLINE_EXPR_CONTEXT_EAGER_VALUE_CONSUMER;
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

    if (!start || !start->ast_template) return 0;
    if (inline_symbol_in_list(targets, target_count, start)) return 1;
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
    if (node->node_type == PROCEDURE) return 0;

    if (node->node_type == FUNCTION &&
        node->symbolNode &&
        node->symbolNode->symbol &&
        node->symbolNode->symbol->is_inlinable &&
        node->symbolNode->symbol->ast_template) {
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
            scope_node->is_compiler_added &&
            scope_node->association &&
            scope_node->association->node_type == PROCEDURE) {
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

static int inline_validate_call_site(ASTNode *proc_def, ASTNode *call_node, Symbol *proc_sym) {
    int unsupported_varg_access;
    size_t max_required_varg_index;
    size_t varg_count;

    if (!proc_def || !call_node || !proc_sym) return 0;
    if (inline_call_is_recursive(call_node, proc_sym)) return 0;
    if (!inline_call_arity_matches(call_node, proc_sym, &varg_count)) return 0;
    if (!proc_sym->has_vargs) return 1;

    if (!inline_analyse_varg_usage(proc_def, &unsupported_varg_access, &max_required_varg_index)) return 0;
    if (unsupported_varg_access) return 0;
    if (varg_count < max_required_varg_index) return 0;

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

static int inline_rewrite_return_nodes(Context *context,
                                       ASTNode *node,
                                       ASTNode *block_expr,
                                       Scope *inline_scope,
                                       int allow_dummy_return,
                                       ValueType proc_type) {
    ASTNode *child;
    ASTNode *next_child;
    ASTNode *leave_expr;

    if (!node) return 1;

    child = node->child;
    while (child) {
        next_child = child->sibling;
        if (!inline_rewrite_return_nodes(context,
                                         child,
                                         block_expr,
                                         inline_scope,
                                         allow_dummy_return,
                                         proc_type)) {
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
    if (!proc_def || !proc_def->scope) return NULL;

    if (!inline_validate_call_site(proc_def, call_node, proc_sym)) return NULL;

    block_expr = ast_dup(context, call_node);
    if (!block_expr) return NULL;

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
    block_expr->scope = inline_scope;

    instr_list = ast_f(context, INSTRUCTIONS, call_node->token);
    if (!instr_list) return NULL;
    instr_list->scope = inline_scope;
    instr_list->value_type = TP_VOID;
    instr_list->target_type = TP_VOID;
    add_ast(block_expr, instr_list);

    memset(&clone_state, 0, sizeof(clone_state));

    if (!inline_build_symbol_map(proc_def->scope, inline_scope, &clone_state)) {
        inline_free_symbol_map(&clone_state);
        return NULL;
    }

    if (!inline_bind_call_arguments(context, instr_list, inline_scope, proc_def, call_node, proc_sym, &clone_state)) {
        inline_free_symbol_map(&clone_state);
        return NULL;
    }

    proc_instrs = ast_chld(proc_def, INSTRUCTIONS, 0);
    if (!proc_instrs) {
        inline_free_symbol_map(&clone_state);
        return NULL;
    }

    proc_instr = proc_instrs->child;
    while (proc_instr) {
        ASTNode *cloned_instr;

        cloned_instr = inline_clone_subtree(context, proc_instr, &clone_state);
        if (!cloned_instr) {
            inline_free_symbol_map(&clone_state);
            return NULL;
        }
        if (!inline_rewrite_return_nodes(context,
                                         cloned_instr,
                                         block_expr,
                                         inline_scope,
                                         allow_dummy_return,
                                         proc_sym->type)) {
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
            inline_free_symbol_map(&clone_state);
            return NULL;
        }
        leave_expr->scope = inline_scope;

        leave_with = ast_f(context, LEAVE_WITH, call_node->token);
        if (!leave_with) {
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
    if (!proc_def || !proc_def->scope) return 0;

    if (!inline_validate_call_site(proc_def, call_node, proc_sym)) return 0;

    block = ast_f(context, INSTRUCTIONS, call_node->token);
    ast_mark_compiler_generated_block(block);
    block->association = proc_def;
    block->value_type = TP_VOID;
    block->target_type = TP_VOID;

    inline_scope = scp_f(context, statement_node->scope, block, NULL, SCOPE_LOCAL);
    instr_list = block;

    memset(&clone_state, 0, sizeof(clone_state));

    if (!inline_build_symbol_map(proc_def->scope, inline_scope, &clone_state)) {
        return 0;
    }

    if (!inline_bind_call_arguments(context, instr_list, inline_scope, proc_def, call_node, proc_sym, &clone_state)) {
        inline_free_symbol_map(&clone_state);
        return 0;
    }

    proc_instrs = ast_chld(proc_def, INSTRUCTIONS, 0);
    if (!proc_instrs) {
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
            ret_assign->scope = inline_scope;

            if (return_plan && return_plan->return_target) {
                ret_lhs = inline_clone_subtree(context, return_plan->return_target, &clone_state);
            } else if (return_plan && return_plan->return_sink_symbol) {
                ret_lhs = inline_create_sink_target(context, inline_scope, proc_instr, proc_instr->child);
            } else {
                inline_free_symbol_map(&clone_state);
                return 0;
            }

            if (!ret_lhs) {
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
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }

                ret_copy = inline_create_register_copy_instr(context, inline_scope, "copy", ret_lhs, ret_rhs);
                attr_copy = inline_create_register_copy_instr(context, inline_scope, "acopy", ret_lhs, ret_rhs);
                if (!ret_copy || !attr_copy) {
                    inline_free_symbol_map(&clone_state);
                    return 0;
                }
                add_ast(instr_list, ret_copy);
                add_ast(instr_list, attr_copy);
            } else {
                ret_rhs = inline_clone_subtree(context, ret_expr, &clone_state);
                if (!ret_rhs) {
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
            add_ast(instr_list, cloned_instr);
        }

        proc_instr = proc_instr->sibling;
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

    if (!assign_node || !call_node) return 0;

    lhs = assign_node->child;
    if (!lhs || lhs->node_type != VAR_TARGET) return 0;

    memset(&return_plan, 0, sizeof(return_plan));
    return_plan.return_target = lhs;

    proc_def = proc_sym ? proc_sym->ast_template : NULL;
    if (!proc_def || !inline_analyse_return_shape(proc_def, &return_shape)) return 0;
    if (!return_shape.final_is_return || return_shape.return_count == 0) return 0;
    if ((assign_node->parent && assign_node->parent->node_type == REPEAT) || lhs->child) {
        block_expr = inline_build_block_expr(context, call_node, proc_sym, assign_node->scope, 0);
        if (!block_expr) return 0;
        ast_rpl(call_node, block_expr);
        inline_disconnect_subtree_symbols(call_node);
        return 1;
    }
    if (return_shape.return_count != 1) {
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

    proc_def = proc_sym ? proc_sym->ast_template : NULL;
    if (!proc_def || !inline_analyse_return_shape(proc_def, &return_shape)) return 0;
    if (return_shape.return_count == 0) {
        if (proc_sym->type != TP_VOID) return 0;
    } else if (!return_shape.final_is_return) {
        if (proc_sym->type != TP_VOID) return 0;
    }

    if ((proc_sym->type == TP_VOID && (return_shape.return_count != 1 || !return_shape.final_is_return)) ||
        (proc_sym->type != TP_VOID && return_shape.return_count != 1)) {
        block = ast_f(context, INSTRUCTIONS, call_node->token);
        if (!block) return 0;
        ast_mark_compiler_generated_block(block);
        block->association = proc_def;
        block->value_type = TP_VOID;
        block->target_type = TP_VOID;

        block_scope = scp_f(context, call_stmt->scope, block, NULL, SCOPE_LOCAL);
        if (!block_scope) return 0;

        block_expr = inline_build_block_expr(context, call_node, proc_sym, block_scope, 1);
        if (!block_expr) return 0;

        sink_assign = ast_f(context, ASSIGN, call_node->token);
        if (!sink_assign) return 0;
        sink_assign->scope = block_scope;
        sink_assign->value_type = block_expr->value_type;
        sink_assign->target_type = block_expr->target_type;

        sink_lhs = inline_create_sink_target(context, block_scope, call_node, block_expr);
        if (!sink_lhs) return 0;

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
    if (expr_context == INLINE_EXPR_CONTEXT_NONE) return 0;
    if (!inline_analyse_return_shape(proc_sym->ast_template, &return_shape)) return 0;
    if (!return_shape.final_is_return || return_shape.return_count == 0) return 0;

    block_expr = inline_build_block_expr(context, call_node, proc_sym, call_node->scope, 0);
    if (!block_expr) return 0;

    ast_rpl(call_node, block_expr);
    inline_disconnect_subtree_symbols(call_node);

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

    if (node->node_type == FUNCTION) {
        proc_sym = node->symbolNode ? node->symbolNode->symbol : NULL;
        if (proc_sym && proc_sym->is_inlinable && proc_sym->ast_template) {
            if (ast_inline_expression(context, node, proc_sym) && inline_payload) inline_payload->changed = 1;
        }
        return result_normal;
    }

    if (node->node_type == ASSIGN) {
        lhs = node->child;
        rhs = lhs ? lhs->sibling : NULL;

        if (!lhs || !rhs) return result_normal;
        if (rhs->node_type != FUNCTION) return result_normal;

        proc_sym = rhs->symbolNode ? rhs->symbolNode->symbol : NULL;
        if (proc_sym && proc_sym->is_inlinable && proc_sym->ast_template) {
            if (ast_inline_assignment(context, node, rhs, proc_sym) && inline_payload) inline_payload->changed = 1;
        }
        return result_normal;
    }

    if (node->node_type != CALL) return result_normal;

    call_node = node->child;
    if (!call_node || call_node->node_type != FUNCTION) return result_normal;

    proc_sym = call_node->symbolNode ? call_node->symbolNode->symbol : NULL;
    if (proc_sym && proc_sym->is_inlinable && proc_sym->ast_template) {
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

        if (node->node_type == OP_ARG_VALUE) {
            size_t index;

            if (!node->child) {
                check->has_unsupported_varg_access = 1;
            } else if (inline_varg_index_from_node(node->child, &index) &&
                       index > check->max_required_varg_index) {
                check->max_required_varg_index = index;
            }
        } else if (node->node_type == OP_ARG_IX_EXISTS) {
            if (!node->child) {
                check->has_unsupported_varg_access = 1;
            }
        }
    }
    return result_normal;
}

/* Walker to identify inlinable procedures */
walker_result identify_inlinable_walker(walker_direction direction, ASTNode *node, void *payload) {
    (void)payload;

    if (direction == in) return result_normal;

    if (node->node_type == PROCEDURE) {
        Symbol *sym;
        ASTNode *args;
        ASTNode *arg;
        ASTNode *formal_target;
        Symbol *formal_symbol;
        ASTNode *instrs;
        InlineReturnShape return_shape;
        InlinableCheck check;

        sym = node->symbolNode ? node->symbolNode->symbol : NULL;
        if (!sym || sym->is_main || !sym->scope || sym->scope->type == SCOPE_CLASS) {
            if (sym) sym->is_inlinable = 0;
            return result_normal;
        }

        args = ast_chld(node, ARGS, 0);
        if (args) {
            arg = args->child;
            while (arg) {
                if (arg->is_varg) {
                    if (arg->is_ref_arg || arg->sibling) {
                        sym->is_inlinable = 0;
                        return result_normal;
                    }
                }

                formal_target = ast_chdn(arg, 0);
                if (arg->is_varg) {
                    formal_target = arg->sibling;
                }

                formal_symbol = formal_target && formal_target->symbolNode ? formal_target->symbolNode->symbol : NULL;
                arg = arg->sibling;
            }
        }

        instrs = ast_chld(node, INSTRUCTIONS, 0);
        if (!instrs) {
            sym->is_inlinable = 0;
            return result_normal;
        }

        if (!inline_analyse_return_shape(node, &return_shape)) {
            sym->is_inlinable = 0;
            return result_normal;
        }
        if (!return_shape.final_is_return && sym->type != TP_VOID) {
            sym->is_inlinable = 0;
            return result_normal;
        }
        if (sym->type != TP_VOID && return_shape.return_count == 0) {
            sym->is_inlinable = 0;
            return result_normal;
        }

        memset(&check, 0, sizeof(check));
        check.root_proc = node;
        ast_wlkr(node, inlinable_check_walker, &check);

        if (check.node_count > INLINE_MAX_NODES ||
            check.return_count != return_shape.return_count ||
            check.has_unsupported_varg_access) {
            sym->is_inlinable = 0;
            return result_normal;
        }

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

void rxcp_inline_prune(Context *context, ASTNode *tree) {
    (void)context;
    (void)tree;
}
