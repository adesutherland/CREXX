/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcp_remap_build.h"
#include "rxcp_sym.h"
#include "rxcp_val.h"
#include "rxcp_emit.h"

void rxcp_remap_anchor_synthetic(ASTNode *node, ASTNode *source_node) {
    ast_copy_source_anchor(node, source_node, AST_SOURCE_SYNTHETIC);
}

void rxcp_remap_mark_generated_block(ASTNode *node, int primary_reporting_anchor) {
    if (!node) return;

    ast_mark_compiler_generated_block(node);
    if (primary_reporting_anchor) ast_enable_primary_reporting_anchor(node);
}

void rxcp_remap_copy_numeric_context(Scope *target, const Scope *source) {
    if (!target || !source) return;

    target->num_context.digits = source->num_context.digits;
    target->num_context.fuzz = source->num_context.fuzz;
    target->num_context.form = source->num_context.form;
    target->num_context.casetype = source->num_context.casetype;
    target->num_context.standard = source->num_context.standard;
}

static int rxcp_remap_copy_dims(int **target, const int *source, size_t dims) {
    if (!target) return 0;

    *target = NULL;
    if (!source || !dims) return 1;

    *target = malloc(sizeof(int) * dims);
    if (!*target) return 0;
    memcpy(*target, source, sizeof(int) * dims);
    return 1;
}

static int rxcp_remap_copy_class(char **target, const char *source) {
    if (!target) return 0;

    *target = NULL;
    if (!source) return 1;

    *target = strdup(source);
    return *target != NULL;
}

int rxcp_remap_copy_node_value_shape_to_symbol(Symbol *target, const ASTNode *source) {
    if (!target || !source) return 0;

    target->type = source->value_type;
    target->value_dims = source->value_dims;

    if (!rxcp_remap_copy_dims(&target->dim_base,
                              source->value_dim_base,
                              source->value_dims)) {
        return 0;
    }
    if (!rxcp_remap_copy_dims(&target->dim_elements,
                              source->value_dim_elements,
                              source->value_dims)) {
        return 0;
    }
    if (!rxcp_remap_copy_class(&target->value_class, source->value_class)) return 0;

    return 1;
}

int rxcp_remap_copy_symbol_value_shape(Symbol *target, const Symbol *source) {
    if (!target || !source) return 0;

    target->type = source->type;
    target->value_dims = source->value_dims;

    if (!rxcp_remap_copy_dims(&target->dim_base,
                              source->dim_base,
                              source->value_dims)) {
        return 0;
    }
    if (!rxcp_remap_copy_dims(&target->dim_elements,
                              source->dim_elements,
                              source->value_dims)) {
        return 0;
    }
    if (!rxcp_remap_copy_class(&target->value_class, source->value_class)) return 0;

    return 1;
}

Scope *rxcp_remap_create_scope(Context *context,
                               Scope *parent,
                               ASTNode *defining_node,
                               ScopeType type,
                               const Scope *numeric_context_source,
                               const char *name) {
    Scope *scope;

    if (!context) return NULL;

    scope = scp_f(context, parent, defining_node, NULL, type);
    if (!scope) return NULL;

    if (name) scope->name = strdup(name);
    if (numeric_context_source) rxcp_remap_copy_numeric_context(scope, numeric_context_source);

    return scope;
}

Scope *rxcp_remap_create_local_scope(Context *context,
                                     Scope *parent,
                                     ASTNode *defining_node,
                                     const Scope *numeric_context_source) {
    return rxcp_remap_create_scope(context,
                                   parent,
                                   defining_node,
                                   SCOPE_LOCAL,
                                   numeric_context_source,
                                   NULL);
}

static void rxcp_remap_init_generated_local_symbol(Symbol *symbol) {
    if (!symbol) return;

    symbol->symbol_type = VARIABLE_SYMBOL;
    symbol->status = SYM_STATUS_LOCAL_VAR;
    symbol->register_num = UNSET_REGISTER;
    symbol->register_type = 'r';
    symbol->meta_emitted = 0;
    symbol->init_emitted = 0;
}

Symbol *rxcp_remap_create_local_symbol_from_node(Context *context,
                                                 Scope *scope,
                                                 const char *name,
                                                 ASTNode *shape_node,
                                                 int default_array_storage) {
    Symbol *symbol;

    (void)context;
    if (!scope || !name || !shape_node) return NULL;

    symbol = sym_fn(scope, name, strlen(name));
    if (!symbol) return NULL;

    rxcp_remap_init_generated_local_symbol(symbol);
    if (!rxcp_remap_copy_node_value_shape_to_symbol(symbol, shape_node)) return NULL;
    if (default_array_storage && symbol->value_dims > 0) symbol->needs_default_initiation = 1;

    return symbol;
}

Symbol *rxcp_remap_create_temp_symbol(Context *context,
                                      Scope *scope,
                                      ASTNode *source_node,
                                      const char *prefix,
                                      size_t suffix) {
    char temp_name[80];

    if (!context || !scope || !source_node || !prefix) return NULL;

    snprintf(temp_name, sizeof(temp_name), "%s_%d_%zu", prefix, source_node->node_number, suffix);
    return rxcp_remap_create_local_symbol_from_node(context,
                                                    scope,
                                                    temp_name,
                                                    source_node,
                                                    1);
}

ASTNode *rxcp_remap_create_symbol_node(Context *context,
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
    rxcp_remap_anchor_synthetic(node, source_node);
    sym_adnd(symbol, node, read_usage, write_usage);
    ast_svtp(node, symbol);

    return node;
}

ASTNode *rxcp_remap_create_integer_constant(Context *context,
                                            ASTNode *source_node,
                                            int value,
                                            ValueType type) {
    ASTNode *node;
    char buffer[32];

    if (!context || !source_node) return NULL;

    node = ast_ft(context, INTEGER);
    if (!node) return NULL;

    snprintf(buffer, sizeof(buffer), "%d", value);
    ast_copy_str(node, buffer);
    rxcp_remap_anchor_synthetic(node, source_node);
    node->int_value = value;
    node->bool_value = value ? 1 : 0;
    ast_set_value_type(0, node, type, 0, 0, 0, 0);
    ast_set_target_type(0, node, type, 0, 0, 0, 0);

    return node;
}

ASTNode *rxcp_remap_create_assignment_node(Context *context,
                                           Scope *scope,
                                           ASTNode *source_node,
                                           ASTNode *shape_node) {
    ASTNode *assign_node;
    ASTNode *type_shape;

    if (!context || !scope || !source_node) return NULL;

    assign_node = ast_f(context, ASSIGN, source_node->token);
    if (!assign_node) return NULL;
    assign_node->scope = scope;

    type_shape = shape_node;
    if (type_shape) {
        assign_node->value_type = type_shape->value_type;
        assign_node->target_type = type_shape->target_type;
    }

    return assign_node;
}

ASTNode *rxcp_remap_create_assignment(Context *context,
                                      Scope *scope,
                                      ASTNode *source_node,
                                      ASTNode *shape_node,
                                      ASTNode *lhs,
                                      ASTNode *rhs) {
    ASTNode *assign_node;

    if (!lhs || !rhs) return NULL;

    assign_node = rxcp_remap_create_assignment_node(context,
                                                    scope,
                                                    source_node,
                                                    shape_node ? shape_node : rhs);
    if (!assign_node) return NULL;

    add_ast(assign_node, lhs);
    add_ast(assign_node, rhs);
    return assign_node;
}

ASTNode *rxcp_remap_create_assignment_to_symbol(Context *context,
                                                Scope *scope,
                                                ASTNode *source_node,
                                                ASTNode *shape_node,
                                                Symbol *target_symbol,
                                                ASTNode *rhs) {
    ASTNode *lhs;

    if (!context || !scope || !source_node || !target_symbol || !rhs) return NULL;

    lhs = rxcp_remap_create_symbol_node(context,
                                        scope,
                                        source_node,
                                        target_symbol,
                                        VAR_TARGET,
                                        0,
                                        1);
    if (!lhs) return NULL;

    return rxcp_remap_create_assignment(context,
                                        scope,
                                        source_node,
                                        shape_node,
                                        lhs,
                                        rhs);
}

ASTNode *rxcp_remap_create_leave_with(Context *context,
                                      Scope *scope,
                                      ASTNode *source_node,
                                      ASTNode *block_expr,
                                      ASTNode *expr) {
    ASTNode *leave_node;

    if (!context || !scope || !source_node || !block_expr || !expr) return NULL;

    leave_node = ast_f(context, LEAVE_WITH, source_node->token);
    if (!leave_node) return NULL;

    leave_node->scope = scope;
    leave_node->association = block_expr;
    leave_node->value_type = expr->value_type;
    leave_node->target_type = expr->target_type;
    add_ast(leave_node, expr);

    return leave_node;
}

ASTNode *rxcp_remap_create_sink_target(Context *context,
                                       Scope *scope,
                                       ASTNode *source_node,
                                       ASTNode *shape_node,
                                       const char *prefix) {
    char sink_name[80];
    Symbol *sink_symbol;

    if (!context || !scope || !source_node || !shape_node) return NULL;
    if (!prefix) prefix = "__remap_unused";

    snprintf(sink_name, sizeof(sink_name), "%s_%d", prefix, source_node->node_number);
    sink_symbol = rxcp_remap_create_local_symbol_from_node(context,
                                                           scope,
                                                           sink_name,
                                                           shape_node,
                                                           0);
    if (!sink_symbol) return NULL;

    return rxcp_remap_create_symbol_node(context,
                                         scope,
                                         source_node,
                                         sink_symbol,
                                         VAR_TARGET,
                                         0,
                                         1);
}

ASTNode *rxcp_remap_capture_once(Context *context,
                                 ASTNode *instr_list,
                                 Scope *scope,
                                 ASTNode *source_node,
                                 const char *prefix,
                                 size_t suffix,
                                 RxcpRemapExprMaterializer materializer,
                                 void *user_data,
                                 Symbol **temp_symbol_out,
                                 ASTNode **capture_assign_out) {
    Symbol *temp_symbol;
    ASTNode *capture_lhs;
    ASTNode *capture_rhs;
    ASTNode *capture_assign;
    ASTNode *temp_ref;

    if (temp_symbol_out) *temp_symbol_out = NULL;
    if (capture_assign_out) *capture_assign_out = NULL;
    if (!context || !instr_list || !scope || !source_node || !prefix || !materializer) return NULL;

    temp_symbol = rxcp_remap_create_temp_symbol(context, scope, source_node, prefix, suffix);
    if (!temp_symbol) return NULL;

    capture_assign = rxcp_remap_create_assignment_node(context, scope, source_node, source_node);
    if (!capture_assign) return NULL;

    capture_lhs = rxcp_remap_create_symbol_node(context,
                                                scope,
                                                source_node,
                                                temp_symbol,
                                                VAR_TARGET,
                                                0,
                                                1);
    capture_rhs = materializer(context, source_node, scope, user_data);
    if (!capture_lhs || !capture_rhs) return NULL;
    add_ast(capture_assign, capture_lhs);
    add_ast(capture_assign, capture_rhs);
    add_ast(instr_list, capture_assign);

    temp_ref = rxcp_remap_create_symbol_node(context,
                                             scope,
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

    if (temp_symbol_out) *temp_symbol_out = temp_symbol;
    if (capture_assign_out) *capture_assign_out = capture_assign;
    return temp_ref;
}

ASTNode *rxcp_remap_create_generated_instruction_block(Context *context,
                                                       Scope *parent_scope,
                                                       ASTNode *token_node,
                                                       ASTNode *anchor_node,
                                                       ASTNode *association,
                                                       int flags,
                                                       Scope **scope_out) {
    ASTNode *block;
    Scope *scope;

    if (scope_out) *scope_out = NULL;
    if (!context || !parent_scope || !token_node) return NULL;
    if (!anchor_node) anchor_node = token_node;

    block = ast_f(context, INSTRUCTIONS, token_node->token);
    if (!block) return NULL;

    rxcp_remap_anchor_synthetic(block, anchor_node);
    rxcp_remap_mark_generated_block(block,
                                    flags & RXCP_REMAP_GENERATED_BLOCK_PRIMARY_REPORTING);
    block->association = association;
    block->value_type = TP_VOID;
    block->target_type = TP_VOID;

    scope = rxcp_remap_create_local_scope(context, parent_scope, block, NULL);
    if (!scope) return NULL;

    if (scope_out) *scope_out = scope;
    return block;
}

ASTNode *rxcp_remap_create_block_expr(Context *context,
                                      Scope *parent_scope,
                                      ASTNode *shape_node,
                                      ASTNode *association,
                                      Scope **scope_out,
                                      ASTNode **instr_list_out) {
    ASTNode *block_expr;
    ASTNode *instr_list;
    Scope *scope;

    if (scope_out) *scope_out = NULL;
    if (instr_list_out) *instr_list_out = NULL;
    if (!context || !parent_scope || !shape_node) return NULL;

    block_expr = ast_dup(context, shape_node);
    if (!block_expr) return NULL;
    block_expr->node_type = BLOCK_EXPR;
    ast_str(block_expr, "do");
    block_expr->association = association;

    scope = rxcp_remap_create_local_scope(context, parent_scope, block_expr, NULL);
    if (!scope) return NULL;

    instr_list = ast_f(context, INSTRUCTIONS, shape_node->token);
    if (!instr_list) return NULL;
    instr_list->scope = scope;
    instr_list->value_type = TP_VOID;
    instr_list->target_type = TP_VOID;
    add_ast(block_expr, instr_list);

    if (scope_out) *scope_out = scope;
    if (instr_list_out) *instr_list_out = instr_list;
    return block_expr;
}
