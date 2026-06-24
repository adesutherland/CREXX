/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXCP_REMAP_BUILD_H
#define CREXX_RXCP_REMAP_BUILD_H

#include <stddef.h>
#include "rxcp_ast.h"
#include "rxcp_types.h"

typedef enum {
    RXCP_REMAP_GENERATED_BLOCK_NONE = 0,
    RXCP_REMAP_GENERATED_BLOCK_PRIMARY_REPORTING = 1
} RxcpRemapGeneratedBlockFlags;

typedef ASTNode *(*RxcpRemapExprMaterializer)(Context *context,
                                              ASTNode *source_node,
                                              Scope *scope,
                                              void *user_data);

typedef struct {
    ASTNode *source_node;
    Symbol **captured_symbols;
    size_t captured_count;
} RxcpRemapCapturedLocator;

void rxcp_remap_anchor_synthetic(ASTNode *node, ASTNode *source_node);
void rxcp_remap_mark_generated_block(ASTNode *node, int primary_reporting_anchor);
void rxcp_remap_copy_numeric_context(Scope *target, const Scope *source);
void rxcp_remap_copy_node_semantics(ASTNode *target, const ASTNode *source);
void rxcp_remap_disconnect_subtree_symbols(ASTNode *node);
int rxcp_remap_replace_node(ASTNode *old_node, ASTNode *new_node);

int rxcp_remap_copy_node_value_shape_to_symbol(Symbol *target, const ASTNode *source);
int rxcp_remap_copy_symbol_value_shape(Symbol *target, const Symbol *source);

Scope *rxcp_remap_create_scope(Context *context,
                               Scope *parent,
                               ASTNode *defining_node,
                               ScopeType type,
                               const Scope *numeric_context_source,
                               const char *name);
Scope *rxcp_remap_create_local_scope(Context *context,
                                     Scope *parent,
                                     ASTNode *defining_node,
                                     const Scope *numeric_context_source);

Symbol *rxcp_remap_create_local_symbol_from_node(Context *context,
                                                 Scope *scope,
                                                 const char *name,
                                                 ASTNode *shape_node,
                                                 int default_array_storage);
Symbol *rxcp_remap_create_temp_symbol(Context *context,
                                      Scope *scope,
                                      ASTNode *source_node,
                                      const char *prefix,
                                      size_t suffix);

ASTNode *rxcp_remap_create_symbol_node(Context *context,
                                       Scope *scope,
                                       ASTNode *source_node,
                                       Symbol *symbol,
                                       NodeType node_type,
                                       unsigned int read_usage,
                                       unsigned int write_usage);
ASTNode *rxcp_remap_create_integer_constant(Context *context,
                                            ASTNode *source_node,
                                            int value,
                                            ValueType type);
ASTNode *rxcp_remap_create_string_constant(Context *context,
                                           ASTNode *source_node,
                                           const char *value);
ASTNode *rxcp_remap_create_literal(Context *context,
                                   ASTNode *source_node,
                                   const char *value);
ASTNode *rxcp_remap_create_import(Context *context,
                                  ASTNode *source_node,
                                  const char *namespace_name);
ASTNode *rxcp_remap_create_noval(Context *context,
                                 ASTNode *source_node);
ASTNode *rxcp_remap_create_named_ref(Context *context,
                                     ASTNode *source_node,
                                     NodeType node_type,
                                     const char *name);
ASTNode *rxcp_remap_create_indexed_ref(Context *context,
                                       ASTNode *source_node,
                                       NodeType node_type,
                                       const char *name,
                                       int index);
ASTNode *rxcp_remap_create_class_type(Context *context,
                                      ASTNode *source_node,
                                      const char *class_name);
ASTNode *rxcp_remap_create_array_define(Context *context,
                                        ASTNode *source_node,
                                        const char *name,
                                        const char *class_name);
ASTNode *rxcp_remap_create_unary_keyword_expr(Context *context,
                                              ASTNode *source_node,
                                              NodeType node_type,
                                              const char *keyword,
                                              ASTNode *operand);
ASTNode *rxcp_remap_create_reference_expr(Context *context,
                                          ASTNode *source_node,
                                          ASTNode *operand);
ASTNode *rxcp_remap_create_dereference_expr(Context *context,
                                            ASTNode *source_node,
                                            ASTNode *operand);
ASTNode *rxcp_remap_create_factory_call(Context *context,
                                        ASTNode *source_node,
                                        const char *class_name,
                                        ASTNode **args,
                                        size_t arg_count);
ASTNode *rxcp_remap_create_function_call(Context *context,
                                         ASTNode *source_node,
                                         const char *function_name,
                                         ASTNode **args,
                                         size_t arg_count);
ASTNode *rxcp_remap_create_member_call(Context *context,
                                       ASTNode *source_node,
                                       ASTNode *receiver,
                                       const char *method_name,
                                       ASTNode **args,
                                       size_t arg_count);
ASTNode *rxcp_remap_create_call_statement(Context *context,
                                          ASTNode *source_node,
                                          ASTNode *call_expr);
ASTNode *rxcp_remap_create_member_call_statement(Context *context,
                                                 ASTNode *source_node,
                                                 ASTNode *receiver,
                                                 const char *method_name,
                                                 ASTNode **args,
                                                 size_t arg_count);
ASTNode *rxcp_remap_create_return_statement(Context *context,
                                            ASTNode *source_node);
ASTNode *rxcp_remap_create_simple_assignment(Context *context,
                                             ASTNode *source_node,
                                             ASTNode *lhs,
                                             ASTNode *rhs);
ASTNode *rxcp_remap_create_named_assignment(Context *context,
                                            ASTNode *source_node,
                                            const char *target_name,
                                            ASTNode *rhs);
ASTNode *rxcp_remap_create_instruction_builder(Context *context,
                                               ASTNode *source_node);
void rxcp_remap_append_builder_children(ASTNode *instructions,
                                        ASTNode *builder);
ASTNode *rxcp_remap_create_do_block(Context *context,
                                    ASTNode *source_node,
                                    ASTNode *instructions);
ASTNode *rxcp_remap_create_if_statement(Context *context,
                                        ASTNode *source_node,
                                        ASTNode *condition,
                                        ASTNode *then_statement,
                                        ASTNode *else_statement);
ASTNode *rxcp_remap_create_assembler_instr(Context *context,
                                           Scope *scope,
                                           ASTNode *source_node,
                                           const char *opcode,
                                           ASTNode *arg1,
                                           ASTNode *arg2,
                                           ASTNode *arg3);
ASTNode *rxcp_remap_create_register_copy_instr(Context *context,
                                               Scope *scope,
                                               const char *opcode,
                                               ASTNode *lhs_node,
                                               ASTNode *rhs_node);
ASTNode *rxcp_remap_create_assignment_node(Context *context,
                                           Scope *scope,
                                           ASTNode *source_node,
                                           ASTNode *shape_node);
ASTNode *rxcp_remap_create_assignment(Context *context,
                                      Scope *scope,
                                      ASTNode *source_node,
                                      ASTNode *shape_node,
                                      ASTNode *lhs,
                                      ASTNode *rhs);
ASTNode *rxcp_remap_create_assignment_to_symbol(Context *context,
                                                Scope *scope,
                                                ASTNode *source_node,
                                                ASTNode *shape_node,
                                                Symbol *target_symbol,
                                                ASTNode *rhs);
ASTNode *rxcp_remap_create_leave_with(Context *context,
                                      Scope *scope,
                                      ASTNode *source_node,
                                      ASTNode *block_expr,
                                      ASTNode *expr);
ASTNode *rxcp_remap_create_sink_target(Context *context,
                                       Scope *scope,
                                       ASTNode *source_node,
                                       ASTNode *shape_node,
                                       const char *prefix);
ASTNode *rxcp_remap_append_assignment_node(ASTNode *instr_list,
                                           ASTNode *assign_node,
                                           ASTNode *lhs,
                                           ASTNode *rhs);
ASTNode *rxcp_remap_append_leave_with(Context *context,
                                      ASTNode *instr_list,
                                      Scope *scope,
                                      ASTNode *source_node,
                                      ASTNode *block_expr,
                                      ASTNode *expr);
ASTNode *rxcp_remap_build_capture_assignment(Context *context,
                                             Scope *scope,
                                             ASTNode *source_node,
                                             Symbol *temp_symbol,
                                             RxcpRemapExprMaterializer materializer,
                                             void *user_data);
ASTNode *rxcp_remap_create_captured_value_ref(Context *context,
                                              Scope *scope,
                                              ASTNode *source_node,
                                              Symbol *temp_symbol);
ASTNode *rxcp_remap_capture_once(Context *context,
                                 ASTNode *instr_list,
                                 Scope *scope,
                                 ASTNode *source_node,
                                 const char *prefix,
                                 size_t suffix,
                                 RxcpRemapExprMaterializer materializer,
                                 void *user_data,
                                 Symbol **temp_symbol_out,
                                 ASTNode **capture_assign_out);
void rxcp_remap_init_captured_locator(RxcpRemapCapturedLocator *locator);
void rxcp_remap_free_captured_locator(RxcpRemapCapturedLocator *locator);
int rxcp_remap_capture_locator_once(Context *context,
                                    ASTNode *instr_list,
                                    Scope *scope,
                                    ASTNode *locator_node,
                                    const char *prefix,
                                    RxcpRemapExprMaterializer materializer,
                                    void *user_data,
                                    RxcpRemapCapturedLocator *locator_out);
ASTNode *rxcp_remap_materialise_selected_value(Context *context,
                                               Scope *scope,
                                               const RxcpRemapCapturedLocator *locator,
                                               ASTNode *shape_node,
                                               NodeType node_type,
                                               unsigned int read_usage,
                                               unsigned int write_usage);
ASTNode *rxcp_remap_writeback_through_captured_locator(Context *context,
                                                       ASTNode *instr_list,
                                                       Scope *scope,
                                                       const RxcpRemapCapturedLocator *locator,
                                                       ASTNode *source_node,
                                                       ASTNode *rhs);

ASTNode *rxcp_remap_create_generated_instruction_block(Context *context,
                                                       Scope *parent_scope,
                                                       ASTNode *token_node,
                                                       ASTNode *anchor_node,
                                                       ASTNode *association,
                                                       int flags,
                                                       Scope **scope_out);
ASTNode *rxcp_remap_create_block_expr(Context *context,
                                      Scope *parent_scope,
                                      ASTNode *shape_node,
                                      ASTNode *association,
                                      Scope **scope_out,
                                      ASTNode **instr_list_out);

#endif
