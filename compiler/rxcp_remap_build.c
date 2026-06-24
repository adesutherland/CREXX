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

void rxcp_remap_copy_node_semantics(ASTNode *target, const ASTNode *source) {
    if (!target || !source) return;

    target->is_ref_arg = source->is_ref_arg;
    target->is_opt_arg = source->is_opt_arg;
    target->is_const_arg = source->is_const_arg;
    target->is_varg = source->is_varg;
    target->inherit_parent_reg_scope = source->inherit_parent_reg_scope;
    if (target->is_ref_arg && target->symbolNode) target->symbolNode->writeUsage = 1;

    ast_set_value_type(0,
                       target,
                       source->value_type,
                       source->value_dims,
                       source->value_dim_base,
                       source->value_dim_elements,
                       source->value_class);
    ast_set_target_type(0,
                        target,
                        source->target_type,
                        source->target_dims,
                        source->target_dim_base,
                        source->target_dim_elements,
                        source->target_class);
}

void rxcp_remap_disconnect_subtree_symbols(ASTNode *node) {
    ASTNode *child;

    if (!node) return;

    child = node->child;
    while (child) {
        rxcp_remap_disconnect_subtree_symbols(child);
        child = child->sibling;
    }

    if (node->symbolNode && node->symbolNode->symbol) {
        sym_dno(node->symbolNode->symbol, node);
    }
}

int rxcp_remap_replace_node(ASTNode *old_node, ASTNode *new_node) {
    if (!old_node || !new_node) return 0;

    ast_rpl(old_node, new_node);
    rxcp_remap_disconnect_subtree_symbols(old_node);
    return 1;
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

char *rxcp_remap_create_prefixed_name(const char *prefix,
                                      const char *name,
                                      const char *suffix) {
    size_t prefix_len;
    size_t name_len;
    size_t suffix_len;
    char *result;

    if (!prefix || !name) return NULL;

    prefix_len = strlen(prefix);
    name_len = strlen(name);
    suffix_len = suffix ? strlen(suffix) : 0;
    result = malloc(prefix_len + name_len + suffix_len + 1);
    if (!result) return NULL;

    memcpy(result, prefix, prefix_len);
    memcpy(result + prefix_len, name, name_len);
    if (suffix_len) memcpy(result + prefix_len + name_len, suffix, suffix_len);
    result[prefix_len + name_len + suffix_len] = '\0';
    return result;
}

char *rxcp_remap_create_generated_node_name(const char *prefix,
                                            ASTNode *source_node) {
    int node_number;
    int length;
    char *result;

    if (!prefix) return NULL;

    node_number = source_node ? source_node->node_number : 0;
    length = snprintf(NULL, 0, "%s%d", prefix, node_number);
    if (length < 0) return NULL;

    result = malloc((size_t)length + 1);
    if (!result) return NULL;

    snprintf(result, (size_t)length + 1, "%s%d", prefix, node_number);
    return result;
}

char *rxcp_remap_create_generated_indexed_name(const char *prefix,
                                               size_t index) {
    int length;
    char *result;

    if (!prefix) return NULL;

    length = snprintf(NULL, 0, "%s%zu", prefix, index);
    if (length < 0) return NULL;

    result = malloc((size_t)length + 1);
    if (!result) return NULL;

    snprintf(result, (size_t)length + 1, "%s%zu", prefix, index);
    return result;
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

ASTNode *rxcp_remap_create_string_constant(Context *context,
                                           ASTNode *source_node,
                                           const char *value) {
    ASTNode *node;

    if (!context || !source_node || !value) return NULL;

    node = ast_ft(context, STRING);
    if (!node) return NULL;

    ast_copy_str(node, (char *)value);
    rxcp_remap_anchor_synthetic(node, source_node);
    ast_set_value_type(0, node, TP_STRING, 0, 0, 0, 0);
    ast_set_target_type(0, node, TP_STRING, 0, 0, 0, 0);

    return node;
}

ASTNode *rxcp_remap_create_literal(Context *context,
                                   ASTNode *source_node,
                                   const char *value) {
    ASTNode *node;

    if (!context || !source_node || !value) return NULL;

    node = ast_ftt(context, LITERAL, strdup(value));
    if (!node) return NULL;

    node->free_node_string = 1;
    rxcp_remap_anchor_synthetic(node, source_node);
    return node;
}

ASTNode *rxcp_remap_create_import(Context *context,
                                  ASTNode *source_node,
                                  const char *namespace_name) {
    ASTNode *node;
    ASTNode *literal;

    if (!context || !source_node || !namespace_name) return NULL;

    node = ast_f(context, IMPORT, source_node->token);
    if (!node) return NULL;
    rxcp_remap_anchor_synthetic(node, source_node);

    literal = rxcp_remap_create_literal(context, source_node, namespace_name);
    if (!literal) return NULL;
    add_ast(node, literal);
    return node;
}

ASTNode *rxcp_remap_create_generated_import(Context *context,
                                            ASTNode *source_node,
                                            const char *namespace_name) {
    ASTNode *node;

    node = rxcp_remap_create_import(context, source_node, namespace_name);
    if (!node) return NULL;

    node->is_compiler_added = 1;
    if (node->child) node->child->is_compiler_added = 1;
    return node;
}

ASTNode *rxcp_remap_create_noval(Context *context,
                                 ASTNode *source_node) {
    ASTNode *node;

    if (!context || !source_node) return NULL;

    node = ast_ft(context, NOVAL);
    if (!node) return NULL;

    rxcp_remap_anchor_synthetic(node, source_node);
    return node;
}

ASTNode *rxcp_remap_create_named_ref(Context *context,
                                     ASTNode *source_node,
                                     NodeType node_type,
                                     const char *name) {
    ASTNode *node;

    if (!context || !source_node || !name) return NULL;

    node = ast_ftt(context, node_type, strdup(name));
    if (!node) return NULL;

    node->free_node_string = 1;
    rxcp_remap_anchor_synthetic(node, source_node);
    return node;
}

ASTNode *rxcp_remap_create_indexed_ref(Context *context,
                                       ASTNode *source_node,
                                       NodeType node_type,
                                       const char *name,
                                       int index) {
    ASTNode *node;
    ASTNode *index_node;

    node = rxcp_remap_create_named_ref(context, source_node, node_type, name);
    index_node = rxcp_remap_create_integer_constant(context,
                                                   source_node,
                                                   index,
                                                   TP_INTEGER);
    if (!node || !index_node) return NULL;

    add_ast(node, index_node);
    return node;
}

ASTNode *rxcp_remap_create_class_type(Context *context,
                                      ASTNode *source_node,
                                      const char *class_name) {
    ASTNode *class_node;

    if (!context || !source_node || !class_name) return NULL;

    class_node = ast_ftt(context, CLASS, strdup(class_name));
    if (!class_node) return NULL;

    class_node->free_node_string = 1;
    rxcp_remap_anchor_synthetic(class_node, source_node);
    return class_node;
}

ASTNode *rxcp_remap_create_array_define(Context *context,
                                        ASTNode *source_node,
                                        const char *name,
                                        const char *class_name) {
    ASTNode *define_node;
    ASTNode *target;
    ASTNode *class_node;
    ASTNode *range_node;
    ASTNode *lower_bound;
    ASTNode *upper_bound;

    if (!context || !source_node || !name || !class_name) return NULL;

    define_node = ast_f(context, DEFINE, source_node->token);
    target = rxcp_remap_create_named_ref(context, source_node, VAR_TARGET, name);
    class_node = rxcp_remap_create_class_type(context, source_node, class_name);
    range_node = ast_ft(context, RANGE);
    lower_bound = rxcp_remap_create_noval(context, source_node);
    upper_bound = rxcp_remap_create_noval(context, source_node);
    if (!define_node || !target || !class_node || !range_node ||
        !lower_bound || !upper_bound) {
        return NULL;
    }

    rxcp_remap_anchor_synthetic(define_node, source_node);
    rxcp_remap_anchor_synthetic(range_node, source_node);
    add_ast(range_node, lower_bound);
    add_ast(range_node, upper_bound);
    add_ast(class_node, range_node);
    add_ast(define_node, target);
    add_ast(define_node, class_node);
    return define_node;
}

ASTNode *rxcp_remap_create_unary_keyword_expr(Context *context,
                                              ASTNode *source_node,
                                              NodeType node_type,
                                              const char *keyword,
                                              ASTNode *operand) {
    ASTNode *node;

    if (!context || !source_node || !keyword || !operand) return NULL;

    node = rxcp_remap_create_named_ref(context, source_node, node_type, keyword);
    if (!node) return NULL;

    add_ast(node, operand);
    return node;
}

ASTNode *rxcp_remap_create_reference_expr(Context *context,
                                          ASTNode *source_node,
                                          ASTNode *operand) {
    return rxcp_remap_create_unary_keyword_expr(context,
                                               source_node,
                                               OP_REFERENCE,
                                               "reference",
                                               operand);
}

ASTNode *rxcp_remap_create_dereference_expr(Context *context,
                                            ASTNode *source_node,
                                            ASTNode *operand) {
    return rxcp_remap_create_unary_keyword_expr(context,
                                               source_node,
                                               OP_DEREFERENCE,
                                               "dereference",
                                               operand);
}

static int rxcp_remap_append_call_args(Context *context,
                                       ASTNode *node,
                                       ASTNode *source_node,
                                       ASTNode **args,
                                       size_t arg_count) {
    size_t i;

    if (!context || !node || !source_node) return 0;

    if (arg_count == 0) {
        ASTNode *noval;

        noval = rxcp_remap_create_noval(context, source_node);
        if (!noval) return 0;
        add_ast(node, noval);
        return 1;
    }

    if (!args) return 0;
    for (i = 0; i < arg_count; i++) {
        if (!args[i]) return 0;
        add_ast(node, args[i]);
    }
    return 1;
}

ASTNode *rxcp_remap_create_factory_call(Context *context,
                                        ASTNode *source_node,
                                        const char *class_name,
                                        ASTNode **args,
                                        size_t arg_count) {
    ASTNode *node;

    if (!context || !source_node || !class_name) return NULL;

    node = ast_ftt(context, FACTORY_CALL, strdup(class_name));
    if (!node) return NULL;

    node->free_node_string = 1;
    rxcp_remap_anchor_synthetic(node, source_node);

    if (!rxcp_remap_append_call_args(context, node, source_node, args, arg_count)) return NULL;

    return node;
}

ASTNode *rxcp_remap_create_function_call(Context *context,
                                         ASTNode *source_node,
                                         const char *function_name,
                                         ASTNode **args,
                                         size_t arg_count) {
    ASTNode *node;

    if (!context || !source_node || !function_name) return NULL;

    node = ast_ftt(context, FUNCTION, strdup(function_name));
    if (!node) return NULL;

    node->free_node_string = 1;
    rxcp_remap_anchor_synthetic(node, source_node);

    if (!rxcp_remap_append_call_args(context, node, source_node, args, arg_count)) return NULL;

    return node;
}

ASTNode *rxcp_remap_create_member_call(Context *context,
                                       ASTNode *source_node,
                                       ASTNode *receiver,
                                       const char *method_name,
                                       ASTNode **args,
                                       size_t arg_count) {
    ASTNode *node;

    if (!context || !source_node || !receiver || !method_name) return NULL;

    node = ast_ftt(context, MEMBER_CALL, strdup(method_name));
    if (!node) return NULL;

    node->free_node_string = 1;
    rxcp_remap_anchor_synthetic(node, source_node);
    add_ast(node, receiver);

    if (!rxcp_remap_append_call_args(context, node, source_node, args, arg_count)) return NULL;

    return node;
}

ASTNode *rxcp_remap_create_call_statement(Context *context,
                                          ASTNode *source_node,
                                          ASTNode *call_expr) {
    ASTNode *node;

    if (!context || !source_node || !call_expr) return NULL;

    node = ast_f(context, CALL, source_node->token);
    if (!node) return NULL;

    rxcp_remap_anchor_synthetic(node, source_node);
    add_ast(node, call_expr);
    return node;
}

ASTNode *rxcp_remap_create_member_call_statement(Context *context,
                                                 ASTNode *source_node,
                                                 ASTNode *receiver,
                                                 const char *method_name,
                                                 ASTNode **args,
                                                 size_t arg_count) {
    ASTNode *call_expr;

    call_expr = rxcp_remap_create_member_call(context,
                                             source_node,
                                             receiver,
                                             method_name,
                                             args,
                                             arg_count);
    if (!call_expr) return NULL;

    return rxcp_remap_create_call_statement(context, source_node, call_expr);
}

ASTNode *rxcp_remap_create_return_statement(Context *context,
                                            ASTNode *source_node) {
    ASTNode *node;

    if (!context || !source_node) return NULL;

    node = ast_f(context, RETURN, source_node->token);
    if (!node) return NULL;

    rxcp_remap_anchor_synthetic(node, source_node);
    return node;
}

ASTNode *rxcp_remap_create_simple_assignment(Context *context,
                                             ASTNode *source_node,
                                             ASTNode *lhs,
                                             ASTNode *rhs) {
    ASTNode *node;

    if (!context || !source_node || !lhs || !rhs) return NULL;

    node = ast_f(context, ASSIGN, source_node->token);
    if (!node) return NULL;

    rxcp_remap_anchor_synthetic(node, source_node);
    add_ast(node, lhs);
    add_ast(node, rhs);
    return node;
}

ASTNode *rxcp_remap_create_named_assignment(Context *context,
                                            ASTNode *source_node,
                                            const char *target_name,
                                            ASTNode *rhs) {
    ASTNode *lhs;

    if (!context || !source_node || !target_name || !rhs) return NULL;

    lhs = rxcp_remap_create_named_ref(context, source_node, VAR_TARGET, target_name);
    if (!lhs) return NULL;

    return rxcp_remap_create_simple_assignment(context, source_node, lhs, rhs);
}

ASTNode *rxcp_remap_create_indexed_assignment(Context *context,
                                              ASTNode *source_node,
                                              const char *target_name,
                                              int index,
                                              ASTNode *rhs) {
    ASTNode *lhs;

    if (!context || !source_node || !target_name || !rhs) return NULL;

    lhs = rxcp_remap_create_indexed_ref(context,
                                        source_node,
                                        VAR_TARGET,
                                        target_name,
                                        index);
    if (!lhs) return NULL;

    return rxcp_remap_create_simple_assignment(context, source_node, lhs, rhs);
}

ASTNode *rxcp_remap_create_instruction_builder(Context *context,
                                               ASTNode *source_node) {
    ASTNode *node;

    if (!context || !source_node) return NULL;

    node = ast_ft(context, INSTRUCTIONS);
    if (!node) return NULL;

    rxcp_remap_anchor_synthetic(node, source_node);
    return node;
}

void rxcp_remap_append_builder_children(ASTNode *instructions,
                                        ASTNode *builder) {
    if (!instructions || !builder || !builder->child) return;

    add_ast(instructions, builder->child);
    builder->child = NULL;
}

ASTNode *rxcp_remap_create_do_block(Context *context,
                                    ASTNode *source_node,
                                    ASTNode *instructions) {
    ASTNode *node;
    ASTNode *repeat;
    ASTNode *for_node;
    ASTNode *count;

    if (!context || !source_node || !instructions) return NULL;

    node = ast_f(context, DO, source_node->token);
    repeat = ast_ft(context, REPEAT);
    for_node = ast_ft(context, FOR);
    count = rxcp_remap_create_integer_constant(context, source_node, 1, TP_INTEGER);
    if (!node || !repeat || !for_node || !count) return NULL;

    rxcp_remap_anchor_synthetic(node, source_node);
    rxcp_remap_anchor_synthetic(repeat, source_node);
    rxcp_remap_anchor_synthetic(for_node, source_node);
    add_ast(for_node, count);
    add_ast(repeat, for_node);
    add_ast(node, repeat);
    add_ast(node, instructions);
    return node;
}

ASTNode *rxcp_remap_create_if_statement(Context *context,
                                        ASTNode *source_node,
                                        ASTNode *condition,
                                        ASTNode *then_statement,
                                        ASTNode *else_statement) {
    ASTNode *node;

    if (!context || !source_node || !condition || !then_statement) return NULL;

    node = ast_f(context, IF, source_node->token);
    if (!node) return NULL;

    rxcp_remap_anchor_synthetic(node, source_node);
    add_ast(node, condition);
    add_ast(node, then_statement);
    if (else_statement) add_ast(node, else_statement);
    return node;
}

ASTNode *rxcp_remap_create_void_type(Context *context,
                                     ASTNode *source_node) {
    ASTNode *node;

    if (!context || !source_node) return NULL;

    node = ast_ft(context, VOID);
    if (!node) return NULL;

    rxcp_remap_anchor_synthetic(node, source_node);
    return node;
}

ASTNode *rxcp_remap_create_reference_type(Context *context,
                                          ASTNode *source_node,
                                          const char *class_name) {
    ASTNode *type_ref;
    ASTNode *class_node;

    if (!context || !source_node || !class_name) return NULL;

    type_ref = ast_ftt(context, TYPE_REFERENCE, strdup("reference"));
    class_node = rxcp_remap_create_class_type(context, source_node, class_name);
    if (!type_ref || !class_node) return NULL;

    type_ref->free_node_string = 1;
    rxcp_remap_anchor_synthetic(type_ref, source_node);
    add_ast(type_ref, class_node);
    return type_ref;
}

ASTNode *rxcp_remap_create_arg(Context *context,
                               ASTNode *source_node,
                               ASTNode *target,
                               ASTNode *type_node) {
    ASTNode *node;

    if (!context || !source_node || !target || !type_node) return NULL;

    node = ast_ft(context, ARG);
    if (!node) return NULL;

    rxcp_remap_anchor_synthetic(node, source_node);
    add_ast(node, target);
    add_ast(node, type_node);
    return node;
}

ASTNode *rxcp_remap_create_args_builder(Context *context,
                                        ASTNode *source_node) {
    ASTNode *node;

    if (!context || !source_node) return NULL;

    node = ast_ft(context, ARGS);
    if (!node) return NULL;

    rxcp_remap_anchor_synthetic(node, source_node);
    return node;
}

ASTNode *rxcp_remap_create_procedure_header(Context *context,
                                            ASTNode *source_node,
                                            const char *procedure_name,
                                            ASTNode *return_type) {
    ASTNode *node;

    if (!context || !source_node || !procedure_name || !return_type) return NULL;

    node = ast_ftt(context, PROCEDURE, strdup(procedure_name));
    if (!node) return NULL;

    node->free_node_string = 1;
    rxcp_remap_anchor_synthetic(node, source_node);
    add_ast(node, return_type);
    return node;
}

int rxcp_remap_begin_argument_frame(Context *context,
                                    ASTNode *instructions,
                                    ASTNode *source_node,
                                    const RxcpRemapArgumentFrameSpec *spec) {
    ASTNode *statement;

    if (!context || !instructions || !source_node || !spec) return 0;

    if (spec->values_name) {
        if (!spec->values_class_name) return 0;
        statement = rxcp_remap_create_array_define(context,
                                                   source_node,
                                                   spec->values_name,
                                                   spec->values_class_name);
        if (!statement) return 0;
        add_ast(instructions, statement);
    }

    if (spec->provided_name) {
        if (!spec->provided_class_name) return 0;
        statement = rxcp_remap_create_array_define(context,
                                                   source_node,
                                                   spec->provided_name,
                                                   spec->provided_class_name);
        if (!statement) return 0;
        add_ast(instructions, statement);
    }

    return 1;
}

int rxcp_remap_append_argument_frame_slot(Context *context,
                                          ASTNode *instructions,
                                          ASTNode *source_node,
                                          const RxcpRemapArgumentFrameSpec *spec,
                                          int index,
                                          ASTNode *value,
                                          int provided) {
    ASTNode *statement;
    ASTNode *provided_value;

    if (!context || !instructions || !source_node || !spec || index < 1) return 0;

    if (spec->values_name) {
        if (!value) return 0;
        statement = rxcp_remap_create_indexed_assignment(context,
                                                         source_node,
                                                         spec->values_name,
                                                         index,
                                                         value);
        if (!statement) return 0;
        add_ast(instructions, statement);
    }

    if (spec->provided_name) {
        provided_value = rxcp_remap_create_integer_constant(context,
                                                           source_node,
                                                           provided ? 1 : 0,
                                                           TP_INTEGER);
        if (!provided_value) return 0;

        statement = rxcp_remap_create_indexed_assignment(context,
                                                         source_node,
                                                         spec->provided_name,
                                                         index,
                                                         provided_value);
        if (!statement) return 0;
        add_ast(instructions, statement);
    }

    return 1;
}

ASTNode *rxcp_remap_create_assembler_instr(Context *context,
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
    rxcp_remap_anchor_synthetic(instr, source_node);

    if (arg1) add_ast(instr, arg1);
    if (arg2) add_ast(instr, arg2);
    if (arg3) add_ast(instr, arg3);

    return instr;
}

ASTNode *rxcp_remap_create_register_copy_instr(Context *context,
                                               Scope *scope,
                                               const char *opcode,
                                               ASTNode *lhs_node,
                                               ASTNode *rhs_node) {
    ASTNode *instr;
    ASTNode *lhs_copy;
    ASTNode *rhs_copy;
    Symbol *lhs_symbol;
    Symbol *rhs_symbol;

    if (!context || !scope || !lhs_node || !rhs_node || !opcode) return NULL;
    if (!lhs_node->symbolNode || !rhs_node->symbolNode) return NULL;

    lhs_symbol = lhs_node->symbolNode->symbol;
    rhs_symbol = rhs_node->symbolNode->symbol;
    if (!lhs_symbol || !rhs_symbol) return NULL;

    instr = rxcp_remap_create_assembler_instr(context,
                                              scope,
                                              lhs_node,
                                              opcode,
                                              NULL,
                                              NULL,
                                              NULL);
    if (!instr) return NULL;

    lhs_copy = rxcp_remap_create_symbol_node(context, scope, lhs_node, lhs_symbol, VAR_TARGET, 0, 1);
    rhs_copy = rxcp_remap_create_symbol_node(context, scope, rhs_node, rhs_symbol, VAR_SYMBOL, 1, 0);
    if (!lhs_copy || !rhs_copy) return NULL;

    add_ast(instr, lhs_copy);
    add_ast(instr, rhs_copy);
    return instr;
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
    ASTNode *assign_node;
    ASTNode *lhs;

    if (!context || !scope || !source_node || !target_symbol || !rhs) return NULL;

    assign_node = rxcp_remap_create_assignment_node(context,
                                                    scope,
                                                    source_node,
                                                    shape_node ? shape_node : rhs);
    if (!assign_node) return NULL;

    lhs = rxcp_remap_create_symbol_node(context,
                                        scope,
                                        source_node,
                                        target_symbol,
                                        VAR_TARGET,
                                        0,
                                        1);
    if (!lhs) return NULL;

    add_ast(assign_node, lhs);
    add_ast(assign_node, rhs);
    return assign_node;
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

ASTNode *rxcp_remap_append_assignment_node(ASTNode *instr_list,
                                           ASTNode *assign_node,
                                           ASTNode *lhs,
                                           ASTNode *rhs) {
    if (!instr_list || !assign_node || !lhs || !rhs) return NULL;

    add_ast(assign_node, lhs);
    add_ast(assign_node, rhs);
    add_ast(instr_list, assign_node);
    return assign_node;
}

ASTNode *rxcp_remap_append_leave_with(Context *context,
                                      ASTNode *instr_list,
                                      Scope *scope,
                                      ASTNode *source_node,
                                      ASTNode *block_expr,
                                      ASTNode *expr) {
    ASTNode *leave_node;

    if (!instr_list) return NULL;

    leave_node = rxcp_remap_create_leave_with(context,
                                              scope,
                                              source_node,
                                              block_expr,
                                              expr);
    if (!leave_node) return NULL;

    add_ast(instr_list, leave_node);
    return leave_node;
}

ASTNode *rxcp_remap_build_capture_assignment(Context *context,
                                             Scope *scope,
                                             ASTNode *source_node,
                                             Symbol *temp_symbol,
                                             RxcpRemapExprMaterializer materializer,
                                             void *user_data) {
    ASTNode *capture_assign;
    ASTNode *capture_lhs;
    ASTNode *capture_rhs;

    if (!context || !scope || !source_node || !temp_symbol || !materializer) return NULL;

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
    return capture_assign;
}

ASTNode *rxcp_remap_create_captured_value_ref(Context *context,
                                              Scope *scope,
                                              ASTNode *source_node,
                                              Symbol *temp_symbol) {
    ASTNode *temp_ref;

    if (!context || !scope || !source_node || !temp_symbol) return NULL;

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

    return temp_ref;
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
    ASTNode *capture_assign;
    ASTNode *temp_ref;

    if (temp_symbol_out) *temp_symbol_out = NULL;
    if (capture_assign_out) *capture_assign_out = NULL;
    if (!context || !instr_list || !scope || !source_node || !prefix || !materializer) return NULL;

    temp_symbol = rxcp_remap_create_temp_symbol(context, scope, source_node, prefix, suffix);
    if (!temp_symbol) return NULL;

    capture_assign = rxcp_remap_build_capture_assignment(context,
                                                         scope,
                                                         source_node,
                                                         temp_symbol,
                                                         materializer,
                                                         user_data);
    if (!capture_assign) return NULL;
    add_ast(instr_list, capture_assign);

    temp_ref = rxcp_remap_create_captured_value_ref(context,
                                                    scope,
                                                    source_node,
                                                    temp_symbol);
    if (!temp_ref) return NULL;

    if (temp_symbol_out) *temp_symbol_out = temp_symbol;
    if (capture_assign_out) *capture_assign_out = capture_assign;
    return temp_ref;
}

static size_t rxcp_remap_count_child_nodes(ASTNode *node) {
    size_t count;

    count = 0;
    while (node) {
        count++;
        node = node->sibling;
    }

    return count;
}

void rxcp_remap_init_captured_locator(RxcpRemapCapturedLocator *locator) {
    if (!locator) return;

    locator->source_node = NULL;
    locator->captured_symbols = NULL;
    locator->captured_count = 0;
}

void rxcp_remap_free_captured_locator(RxcpRemapCapturedLocator *locator) {
    if (!locator) return;

    if (locator->captured_symbols) free(locator->captured_symbols);
    rxcp_remap_init_captured_locator(locator);
}

int rxcp_remap_capture_locator_once(Context *context,
                                    ASTNode *instr_list,
                                    Scope *scope,
                                    ASTNode *locator_node,
                                    const char *prefix,
                                    RxcpRemapExprMaterializer materializer,
                                    void *user_data,
                                    RxcpRemapCapturedLocator *locator_out) {
    ASTNode *child;
    size_t child_index;

    if (!context || !instr_list || !scope || !locator_node || !materializer || !locator_out) return 0;
    if (!prefix) prefix = "__remap_locator";

    rxcp_remap_init_captured_locator(locator_out);
    locator_out->source_node = locator_node;
    locator_out->captured_count = rxcp_remap_count_child_nodes(locator_node->child);

    if (locator_out->captured_count) {
        locator_out->captured_symbols = calloc(locator_out->captured_count, sizeof(Symbol *));
        if (!locator_out->captured_symbols) return 0;
    }

    child = locator_node->child;
    child_index = 0;
    while (child) {
        Symbol *temp_symbol;
        ASTNode *capture_assign;

        temp_symbol = rxcp_remap_create_temp_symbol(context, scope, child, prefix, child_index);
        if (!temp_symbol) return 0;

        capture_assign = rxcp_remap_build_capture_assignment(context,
                                                             scope,
                                                             child,
                                                             temp_symbol,
                                                             materializer,
                                                             user_data);
        if (!capture_assign) return 0;
        capture_assign->target_type = child->value_type;
        add_ast(instr_list, capture_assign);

        locator_out->captured_symbols[child_index] = temp_symbol;
        child = child->sibling;
        child_index++;
    }

    return 1;
}

ASTNode *rxcp_remap_materialise_selected_value(Context *context,
                                               Scope *scope,
                                               const RxcpRemapCapturedLocator *locator,
                                               ASTNode *shape_node,
                                               NodeType node_type,
                                               unsigned int read_usage,
                                               unsigned int write_usage) {
    ASTNode *replacement;
    ASTNode *source_child;
    ASTNode *shape_source;
    size_t child_index;

    if (!context || !scope || !locator || !locator->source_node) return NULL;

    shape_source = shape_node ? shape_node : locator->source_node;
    replacement = ast_dup(context, locator->source_node);
    if (!replacement) return NULL;

    replacement->node_type = node_type;
    replacement->scope = scope;

    if (locator->source_node->symbolNode && locator->source_node->symbolNode->symbol) {
        sym_adnd(locator->source_node->symbolNode->symbol,
                 replacement,
                 read_usage,
                 write_usage);
    }

    source_child = locator->source_node->child;
    child_index = 0;
    while (source_child) {
        ASTNode *captured_ref;

        if (child_index >= locator->captured_count || !locator->captured_symbols[child_index]) return NULL;

        captured_ref = rxcp_remap_create_symbol_node(context,
                                                     scope,
                                                     source_child,
                                                     locator->captured_symbols[child_index],
                                                     VAR_SYMBOL,
                                                     1,
                                                     0);
        if (!captured_ref) return NULL;

        add_ast(replacement, captured_ref);
        source_child = source_child->sibling;
        child_index++;
    }

    rxcp_remap_copy_node_semantics(replacement, shape_source);
    return replacement;
}

ASTNode *rxcp_remap_writeback_through_captured_locator(Context *context,
                                                       ASTNode *instr_list,
                                                       Scope *scope,
                                                       const RxcpRemapCapturedLocator *locator,
                                                       ASTNode *source_node,
                                                       ASTNode *rhs) {
    ASTNode *assign_node;
    ASTNode *assign_lhs;
    ASTNode *anchor_node;

    if (!context || !instr_list || !scope || !locator || !locator->source_node || !rhs) return NULL;

    anchor_node = source_node ? source_node : locator->source_node;
    assign_node = rxcp_remap_create_assignment_node(context,
                                                    scope,
                                                    anchor_node,
                                                    locator->source_node);
    if (!assign_node) return NULL;

    assign_lhs = rxcp_remap_materialise_selected_value(context,
                                                       scope,
                                                       locator,
                                                       locator->source_node,
                                                       VAR_TARGET,
                                                       0,
                                                       1);
    if (!assign_lhs) return NULL;

    return rxcp_remap_append_assignment_node(instr_list, assign_node, assign_lhs, rhs);
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
