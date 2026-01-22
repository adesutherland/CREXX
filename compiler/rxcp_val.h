#ifndef CREXX_RXCP_VAL_H
#define CREXX_RXCP_VAL_H

#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxvminst.h"

/* Internal validation module header */

/* Common Helpers (to be in orch or check) */
int is_node_string(ASTNode* node, const char* value);
int node_to_integer(ASTNode* node);
void node_to_dims(ASTNode* node, size_t *dims, int** dim_base, int** dim_elements);
ValueType node_to_type(ASTNode *node, size_t *dims, int **dim_base, int **dim_elements, char **class_name);
void validate_node_promotion(ASTNode* node);
void validate_node_promotion_for_ref(ASTNode* node);

/* orch */
void validate_ast(Context *context);
void rxcp_val(Context *context); /* legacy name */
void rxcp_bvl(Context *context);

/* sym */
walker_result build_symbols_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result resolve_functions_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result exposed_symbols_walker(walker_direction direction, ASTNode* node, void *payload);
void validate_symbols(Scope *scope);

/* type */
walker_result set_node_types_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result type_safety_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result func_type_safety_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result float2decimal_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result decimal2float_walker(walker_direction direction, ASTNode* node, void *payload);

/* trans */
walker_result rewrite_exit_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result rewrite_address_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result needs_rxsysb_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result add_rxsysb_walker(walker_direction direction, ASTNode* node, void *payload);

/* check */
walker_result initial_checks_walker(walker_direction direction, ASTNode* node, void *payload);
walker_result decimal_parameters_walker(walker_direction direction, ASTNode* node, void *payload);

/* misc */
walker_result set_node_ordinals_walker(walker_direction direction, ASTNode* node, void *payload);

#endif
