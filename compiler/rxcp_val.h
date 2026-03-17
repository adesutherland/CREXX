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
Context* rxcp_parse_buffer(char* source_string, int options);

/* sym */
walker_result structure_symbols_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result build_symbols_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result resolve_functions_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result exposed_symbols_walker(walker_direction direction, ASTNode* node, void *payload);
void validate_symbols(Context *context, Scope *scope);
int ast_hoist_var(Context* ctx, ASTNode* current_node, const char* var_name, int levels);

/* type */
walker_result clear_node_types_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result set_node_types_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result type_safety_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result func_type_safety_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result float2decimal_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result decimal2float_walker(walker_direction direction, ASTNode* node, void *payload);

/* trans */
walker_result rewrite_constructor_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result rewrite_exit_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result rewrite_implicit_cmd_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result syntax_sugar_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result rewrite_address_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result needs_rxsysb_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result add_rxsysb_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result rxcp_fixup_walker(walker_direction direction, ASTNode* node, void *payload);

/* check */
walker_result ast_structure_fixup_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result source_location_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result syntax_validation_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result decimal_parameters_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result exit_dispatch_walker(walker_direction direction, ASTNode* node, void *payload);

/* misc */
walker_result set_node_ordinals_walker(walker_direction direction, ASTNode* node, void *payload);

#endif
