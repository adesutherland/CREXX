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

/**
 * Validation Pipeline Header
 */

#ifndef CREXX_RXCP_VAL_H
#define CREXX_RXCP_VAL_H

#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "../binutils/include/rxdefs.h"
#include "rxcp_ast_rewrite.h"

/* Internal validation module header */

/* Common Helpers (to be in orch or check) */
int is_node_string(ASTNode* node, const char* value);
int node_to_integer(ASTNode* node);
void node_to_dims(Context *context, ASTNode* node, size_t *dims, int** dim_base, int** dim_elements);
ValueType node_to_type(Context* context, ASTNode *node, size_t *dims, int **dim_base, int **dim_elements, char **class_name);
void rxcp_set_symbol_reference_type_from_node(Symbol *symbol, ASTNode *type_node);
void rxcp_set_node_value_reference_type_from_node(ASTNode *node, ASTNode *type_node);
int rxcp_same_reference_value_and_target_type(ASTNode *node);
int rxcp_same_reference_value_type(ASTNode *left, ASTNode *right);
void promote_symbol_from_target(Context *context, ASTNode *node);
void validate_node_promotion(Context *context, ASTNode* node);
void validate_node_promotion_for_ref(Context *context, ASTNode* node);

/* Monotonic Gatekeepers */
void sym_promote_type(Context *context, Symbol *sym, ValueType type, size_t dims, int *dim_base, int *dim_elements, char *class_name);
void sym_promote_status(Context *context, Symbol *sym, SymbolStatus status);
void sym_promote_symtype(Context *context, Symbol *sym, SymbolType symbol_type);
void ast_promote_type(Context *context, ASTNode *node, ValueType type, size_t dims, int *dim_base, int *dim_elements, char *class_name);
void ast_promote_target_type(Context *context, ASTNode *node, ValueType type, size_t dims, int *dim_base, int *dim_elements, char *class_name);
void ast_set_value_type(Context *context, ASTNode *node, ValueType type, size_t dims, int *dim_base, int *dim_elements, char *class_name);
void ast_set_target_type(Context *context, ASTNode *node, ValueType type, size_t dims, int *dim_base, int *dim_elements, char *class_name);

/* orch */
void validate_ast(Context *context);
void rxcp_val(Context *context); /* legacy name */
void rxcp_bvl(Context *context);
void rxcp_prepare_source_ast(Context *context);
void rxcp_prepare_work_ast(Context *context);
Context* rxcp_parse_buffer(char* source_string, int options);

/* sym */
walker_result structure_symbols_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result build_symbols_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result resolve_functions_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result exposed_symbols_walker(walker_direction direction, ASTNode* node, void *payload);
void validate_symbols(Context *context, Scope *scope);
int ast_hoist_var(Context* ctx, ASTNode* current_node, const char* var_name, int levels);
int ast_hoist_var_typed(Context* ctx, ASTNode* current_node, const char* var_name, int levels, const char* type_name, size_t dims);

/* type */
walker_result clear_node_types_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result set_node_types_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result type_safety_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result func_type_safety_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result float2decimal_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result decimal2float_walker(walker_direction direction, ASTNode* node, void *payload);
void propagate_explicit_constants(Context *context);

/* trans */
walker_result rewrite_constructor_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result rewrite_exit_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result rewrite_implicit_cmd_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result syntax_sugar_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result control_flow_rewrite_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result tostring_rewrite_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result needs_rxsysb_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result add_rxsysb_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result rxcp_fixup_walker(walker_direction direction, ASTNode* node, void *payload);

/* check */
walker_result ast_source_structure_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result ast_work_structure_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result ast_structure_fixup_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result source_location_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result syntax_validation_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result assembler_validation_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result decimal_parameters_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result exit_dispatch_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result exit_plan_walker(walker_direction direction, ASTNode* node, void *payload);

/* inline */
walker_result identify_inlinable_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result inline_procedure_walker(walker_direction direction, ASTNode* node, void *payload);
int rxcp_inline_pass(Context *context);
void rxcp_inline_prune(Context *context, ASTNode *tree);
char *rxcp_inline_export_payload(Context *context, ASTNode *callable);
int rxcp_inline_payload_is_supported(const char *payload);
int rxcp_inline_attach_imported_body(Context *context, const char *payload);
int rxcp_inline_attach_imported_symbol(Context *context, Symbol *symbol, const char *payload);

/* misc */
walker_result set_node_ordinals_walker(walker_direction direction, ASTNode* node, void *payload);

#endif
