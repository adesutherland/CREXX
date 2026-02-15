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
 * Abstract Syntax Tree Definitions
 */

#ifndef CREXX_RXCP_AST_H
#define CREXX_RXCP_AST_H

#include "rxcp_types.h"
#include "rxcp_token.h"

struct ASTNode {
    Context *context;
    int node_number;
    NodeType node_type;
    char* file_name;               /* This is a pointer to the file name in the context (which may be the original context for a duplicated node for imported functions) */
    ValueType value_type;    /* Value type */
    size_t value_dims;       /* Value dimensions */
    int *value_dim_base;     /* Array of starting element number for array dimension - malloced or zero */
    int *value_dim_elements; /* Array of max number of elements for array dimension (0=infinite) - malloced or zero */
    char* value_class;       /* Value class name - malloced or zero */
    int *target_dim_base;    /* Array of starting element number for target array dimension - malloced or zero */
    int *target_dim_elements;/* Array of max number of elements for target array dimension (0=infinite) - malloced or zero */
    ValueType target_type;   /* Target type */
    size_t target_dims;      /* Target dimensions */
    char* target_class;      /* Target class name - malloced or zero */
    int high_ordinal; /* Order of node after validation but before any optimisations / tree re-writing - highest in this tree root */
    int low_ordinal;  /* lowest in this tree root - makes a range for the subtree */
    int register_num;
    char register_type;
    int additional_registers; /* always type 'r' */
    int num_additional_registers;
    char is_ref_arg;
    char is_opt_arg;
    char is_const_arg;
    char is_varg;
    ASTNode *free_list;
    ASTNode *parent, *child, *sibling;
    ASTNode *association; /* E.g. for LEAVE / ITERATE TO relevant DO node */
    Token *token;
    Scope *scope;
    char *node_string;
    size_t node_string_length;
    char free_node_string;
    rxinteger int_value;
    int bool_value;
    double float_value;
    char* decimal_value; /* Decimal value as a string - malloced */
    int exit_obj_reg; /* VM register index of the attached Exit object, initialized to -1 */
    /* These are only valid after the set_source_location walker has run */
    Token *token_start, *token_end;
    char *source_start, *source_end;
    int line, column;
    SymbolNode *symbolNode;
    /* These are used by the code emitters */
    OutputFragment *output;          /* Primary node output or loop assign / init instruction */
    OutputFragment *cleanup;         /* Clean up logic (e.g. to unlink registers */
    OutputFragment *loopstartchecks; /* Begin Loop exit checks */
    OutputFragment *loopinc;         /* Loop increments */
    OutputFragment *loopendchecks;   /* End Loop exit checks */
};

/* AST Functions */
/* Utility to check is a node (typically an IDENTIFIER) is a certain value */
/* Case-insensitive and only checks the first 14 characters of the value */
int nodeis(ASTNode *node, const char* value);
/* Utility to check if a token (typically an IDENTIFIER) is a certain value */
/* Case-insensitive and only checks the first 14 characters of the value */
int tokenis(Token *token, const char* value);
ASTNode* ast_f(Context* context, NodeType type, Token *token); /* ASTNode Factory */
/* ASTNode Factory - adds a STRING token removing the leading & trailing speech marks */
ASTNode *ast_fstr(Context* context, Token *token);
/* ASTNode Factory - adds a DECIMAL token removing the trailing d if it exists */
ASTNode *ast_fdec(Context* context, Token *token);
/* ASTNode Factory - With node type*/
ASTNode *ast_ft(Context* context, NodeType type);
/* ASTNode Factory - With node type and string value */
ASTNode *ast_ftt(Context* context, NodeType type, char *string);
/* ASTNode Factory - With node type and string value copied from another node */
ASTNode *ast_fstk(Context* context, ASTNode *source_node);
/* Factory to create a duplicated AST node into a new context
 * - context is the target context
 * - node is the node to be duplicated
 *
 * A number of aspects are NOT copied
 * - Symbols
 * - Scope
 * - Associated Nodes
 * - Tree position (child, sibling, parent)
 * - Emitter output fragments
 * - Ordinals
 */
ASTNode *ast_dup(Context* new_context, ASTNode *node);
/* Add error node to parent node */
ASTNode *ast_err(Context* context, char *error_string, Token *token);
/* Add warning node to parent node */
ASTNode *ast_war(Context* context, char *warning_string, Token *token);
/* ASTNode Factory - Error at last Node */
ASTNode *ast_errh(Context* context, char *error_string);
/* Add a duplicate of the tree headed by the source node as a child to dest
 * This handles the nodes, scopes and symbols, and associated nodes
 * Note that symbols and associated nodes out of the scope of the original child tree are removed */
ASTNode *add_dast(ASTNode *dest, ASTNode *source);
const char *ast_ndtp(NodeType type);
void prnt_ast(ASTNode* node);
void pdot_scope(Symbol *symbol, void *payload);
walker_result pdot_walker_handler(walker_direction direction,
                                  ASTNode* node, void *payload);
walker_result prnt_walker_handler(walker_direction direction,
                                  ASTNode* node,
                                  void *payload);
void print_error(ASTNode* node, FILE* stream, char* prefix);
ASTNode* add_ast(ASTNode* parent, ASTNode* child); /* Add Child - Returns child for chaining */
ASTNode *add_sbtr(ASTNode *older, ASTNode *younger); /* Add sibling - Returns younger for chaining */
/* Add an error child node  - returns node for chaining */
ASTNode *mknd_err(ASTNode* node, char *error_string, ...);
/* Add a warning child node  - returns node for chaining */
ASTNode *mknd_war(ASTNode* node, char *error_string, ...);
void free_ast(Context* context);
void pdot_tree(ASTNode *tree, char* output_file, char* prefix);
/* Set the string value of an ASTNode. string must be malloced. memory is
 * then managed by the AST Library (the caller must not free it) */
void ast_sstr(ASTNode *node, char* string, size_t length);
/* Set a node string to a static value (i.e. the node isn't responsible for
 * freeing it). See also ast_sstr() */
/* Set a node string by copying a string */
void ast_copy_str(ASTNode* node, char *string);
void ast_str(ASTNode* node, char *string);
/* Replace replaced_node with new_node in the tree
 * note that replaced_node should not be a descendant or direct relation of
 * new_node (else we will get a loop in the tree)! */
void ast_rpl(ASTNode* replaced_node, ASTNode* new_node);
/* Delete / Remove node (i.e. the whole subtree) from the tree */
void ast_del(ASTNode* node);
/* Returns the fully resolved node name in a malloced buffer */
char* ast_frnm(ASTNode* node);
/* Returns the PROCEDURE ASTNode procedure of an AST node */
ASTNode* ast_proc(ASTNode *node);
/* Get the child node of a certain type1 or type2 (or null) */
ASTNode * ast_chld(ASTNode *parent, NodeType type1, NodeType type2);
/* Returns 1 if the node is an error or warning node, or has any descendant error or warning node */
int ast_hase(ASTNode *node);
/* Prune all nodes except ERRORs and WARNINGs */
void ast_prun(ASTNode *node);
/* Prune all children nodes except ERRORs and WARNINGs */
void ast_prnc(ASTNode *node);
/* Returns the number of children of a node */
size_t ast_nchd(ASTNode* node);
/* Returns the index number of a child of its parent (or -1 on error) */
int ast_chdi(ASTNode* node);
/* Returns the nth child of a parent (or 0 on error), skipping ERROR/WARNING nodes */
ASTNode* ast_chdn(ASTNode* parent, size_t n);
/* Returns the next sibling a node (or 0 on error), skipping ERROR/WARNING nodes */
ASTNode* ast_nsib(ASTNode* node);
/* Returns the type of a node as a text string in a malloced buffer */
char* ast_n2tp(ASTNode *node);
/* Returns the source code of a node in a malloced buffer with formatting removed / cleaned */
char *ast_nsrc(ASTNode *node);
/* Returns a malloced string of the array part of a symbols/type
 * (it returns a null terminated string if there is no array part - still needs a free() */
char *ast_astr(size_t dims, int* base, int* num_elements);

/* And the walker itself
 * It returns
 *     result_normal - All OK - normal processing
 *     result_abort - Walk Aborted by handler
 *     result_error - error condition
 */
walker_result ast_wlkr(ASTNode *tree, walker_handler handler, void *payload);

/* Set Node Value and Target Type from Symbol */
void ast_svtp(ASTNode* node, Symbol* symbol);

/* Set Node Target Value Type from Symbol */
void ast_sttp(ASTNode* node, Symbol* symbol);

/* Reset Node Target Type to be the same as the node value type */
void ast_rttp(ASTNode* node);

/* Set Node Target Value Type from Target Type of from_node */
/* Note: Does not validate promotion */
void ast_sttn(ASTNode* node, ASTNode* from_node);

/* Set Node Value (and Target) Type from the from_node target type */
void ast_svtn(ASTNode* node, ASTNode* from_node);

void ast_set_file_name(Context *context, char *file_name);
#endif //CREXX_RXCP_AST_H
