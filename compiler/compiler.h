/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

#ifndef CREXX_COMPILER_H
#define CREXX_COMPILER_H

#include "stdio.h"

/* functions to interface the lemon parser */
/* OPTIONS Parser */
void *Opts_Alloc();
void Opts_();
void Opts_Free();
void Opts_Trace(FILE *stream, char *zPrefix);

/* Level B Parser */
void *RexxBAlloc();
void RexxB();
void RexxBFree();
void RexxBTrace(FILE *stream, char *zPrefix);

typedef struct ASTNode ASTNode;
typedef struct Token Token;

typedef enum RexxLevel {
    UNKNOWN, LEVELA, LEVELB, LEVELC, LEVELD, LEVELG, LEVELL
} RexxLevel;

/* Scanner */
typedef struct Context {
    FILE *traceFile;
    char *buff_end;
    char *top, *cursor, *marker, *ctxmarker, *linestart;
    int line;
    int token_counter;
    Token* token_head;
    Token* token_tail;
    ASTNode* ast;
    ASTNode* free_list;
    RexxLevel level;
} Context;

int rexbscan(Context* s);
int rexbpars(Context *context);
int opt_scan(Context* s);
int opt_pars(Context *context);

typedef enum NodeType {
    ABS_POS=1, ADDRESS, ARG, ASSIGN, BY, CALL, CONST_SYMBOL,
    DO, ENVIRONMENT, ERROR, FOR, FUNCTION, IF, INSTRUCTIONS, ITERATE, LABEL, LEAVE,
    NUMBER, OP_ADD, OP_MINUS, OP_AND, OP_COMPARE, OP_CONCAT, OP_MULT, OP_DIV, OP_IDIV,
    OP_MOD, OP_OR, OP_POWER, OP_PREFIX,
    OP_SCONCAT, OPTIONS, PARSE, PATTERN, PROCEDURE, PROGRAM_FILE, PULL, REL_POS, REPEAT,
    RETURN, REXX_OPTIONS, SAY, SIGN, STRING, TARGET, TEMPLATES, TO, TOKEN, UPPER, VAR_SYMBOL
} NodeType;

struct Token {
    int token_type;
    int token_number;
    int token_subtype;
    Token *token_next;
    Token *token_prev;
    size_t line, column, length;
    char* token_string;
};

struct ASTNode {
    NodeType node_type;
    ASTNode *free_list;
    int node_number;
    ASTNode *parent, *child, *sibling;
    Token *token;
    const char *node_string;
    size_t node_string_length;
    /* These are only valid after the set_source_location walker has run */
    char *source_start, *source_end;
};

/* Utilities */
void print_unescaped(FILE* output, const char *ptr, int len);

/* Token Functions */
Token* token_f(Context* context, int type);
void free_tok(Context* context);
void prnt_tok(Token* token);
const char* token_type_name(int type); /* Get Token Type Name */

/* AST Function */
ASTNode* ast_f(Context* context, NodeType type, Token *token);
/* ASTNode Factory - With node type*/
ASTNode *ast_ft(Context* context, NodeType type);
/* ASTNode Factory - With node type and string value */
ASTNode *ast_ftt(Context* context, NodeType type, const char *string);
/* ASTNode Factory - Error Node */
ASTNode *ast_error(Context* context, const char *error_string, Token *token);
/* ASTNode Factory - Error at last Node */
ASTNode *ast_error_here(Context* context, const char *error_string);
const char *ast_nodetype(NodeType type);
void prnt_ast(ASTNode* node);
void pdot_ast(FILE* output, ASTNode* node, int parent, int *counter);
ASTNode* add_ast(ASTNode* parent, ASTNode* child); /* Returns child for chaining */
ASTNode *add_sibling_ast(ASTNode *older, ASTNode *younger); /* Returns younger for chaining */
void free_ast(Context* context);
void pdot_tree(ASTNode *tree, char* output_file);

/* AST Walker Infrastructure */
typedef enum walker_direction { in, out } walker_direction;
typedef enum walker_result {
    result_normal, result_abort, result_error, request_skip
} walker_result;

/* This is the handler signature for the walker - it is called twice for each
 * node; in (top down) and out (bottom up). The handler should return
 *   result_normal - All OK - normal processing
 *   result_abort - Abort walk
 *   result_error - Aborted with an error condition
 *   request_skip - skip this node (only for in - top down - direction
 */
typedef walker_result (*walker_handler)(walker_direction direction,
        ASTNode* visited_node, void *payload);

/* And the walker itself
 * It returns
 *     result_normal - All OK - normal processing
 *     result_abort - Walk Aborted by handler
 *     result_error - error condition
 */
walker_result ast_walker(ASTNode *tree, walker_handler handler, void *payload);

#endif //CREXX_COMPILER_H