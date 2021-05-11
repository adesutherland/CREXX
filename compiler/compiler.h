/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

#ifndef CREXX_COMPILER_H
#define CREXX_COMPILER_H

#include "stdio.h"

/* Typedefs */
typedef struct ASTNode ASTNode;
typedef struct Token Token;
typedef struct Scope Scope;
typedef struct Symbol Symbol;
typedef struct OutputFragment OutputFragment;

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

typedef enum RexxLevel {
    UNKNOWN, LEVELA, LEVELB, LEVELC, LEVELD, LEVELG, LEVELL
} RexxLevel;

typedef enum ValueType {
    TP_UNKNOWN, TP_BOOLEAN, TP_INTEGER, TP_FLOAT, TP_STRING, TP_OBJECT
} ValueType;

/* Compiler Context Object */
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
    FLOAT, INTEGER, OP_ADD, OP_MINUS, OP_AND, OP_CONCAT, OP_MULT, OP_DIV, OP_IDIV,
    OP_MOD, OP_OR, OP_POWER, OP_PREFIX,
    OP_COMPARE_EQUAL, OP_COMPARE_NEQ, OP_COMPARE_GT, OP_COMPARE_LT,
    OP_COMPARE_GTE, OP_COMPARE_LTE, OP_COMPARE_S_EQ, OP_COMPARE_S_NEQ,
    OP_COMPARE_S_GT, OP_COMPARE_S_LT, OP_COMPARE_S_GTE, OP_COMPARE_S_LTE,
    OP_SCONCAT, OPTIONS, PARSE, PATTERN, PROCEDURE, PROGRAM_FILE, PULL, REL_POS, REPEAT,
    RETURN, REXX_OPTIONS, SAY, SIGN, STRING, TARGET, TEMPLATES, TO, TOKEN, UPPER,
    VAR_SYMBOL, VAR_TARGET
} NodeType;

struct Token {
    int token_type;
    int token_number;
    int token_subtype;
    Token *token_next;
    Token *token_prev;
    int line, column, length;
    char* token_string;
};

struct ASTNode {
    NodeType node_type;
    ValueType value_type;
    ValueType target_type;
    int register_num;
    ASTNode *free_list;
    int node_number;
    ASTNode *parent, *child, *sibling;
    Token *token;
    Scope *scope;
    Symbol *symbol;
    const char *node_string;
    size_t node_string_length;
    /* These are only valid after the set_source_location walker has run */
    char *source_start, *source_end;
    int line, column;
    OutputFragment *output;
    OutputFragment *output2;
    OutputFragment *output3;
    OutputFragment *output4;
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
/* Turn a node to an ERROR */
void make_node_an_error(ASTNode* node, const char *error_string);
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

/* Validator - returns the number of errors */
int validate(Context *context);

/* Scope and Symbols */
struct Scope {
    ASTNode *defining_node;
    Scope *parent;
    void *child_array;
    void *symbols_tree;
    size_t num_registers;
    void *free_registers_array;
};

char* type_name(ValueType type);

struct Symbol {
    char *name;
    void *ast_node_array;
    Scope *scope;
    ValueType type;
    int register_num;
};

/* Scope Factory */
Scope *scp_f(Scope *parent, ASTNode *node);

/* Calls the handler for each symbol in scope */
typedef void (*symbol_worker)(Symbol *symbol, void *payload);
void scp_for_all(Scope *scope, symbol_worker worker, void *payload);

/* To get sub-scopes */
Scope* scope_child(Scope *scope, size_t index);
size_t scope_num_children(Scope *scope);

/* Get a free register from scope */
int get_reg(Scope *scope);

/* Return a no longer used register to the scope */
void return_reg(Scope *scope, int reg);

/* Frees scope and all its symbols */
void scp_free(Scope *scope);

/* Symbol Factory - define a symbol */
Symbol *sym_f(Scope *scope, ASTNode *node);

/* Resolve a Symbol */
Symbol *sym_rslv(Scope *scope, ASTNode *node);

/* Get a the index'th ASTNode using the symbol */
ASTNode* symbol_astnode(Symbol *symbol, size_t index);

/* Add a AST Node using the symbol */
void symbol_add_astnode(Symbol *symbol, ASTNode* node);

/* Get number of ASTNodes using the symbol */
size_t symbol_num_astnodes(Symbol *symbol);

/* Emit Assembler */
void emit(Context *context, char *output_file);

/* Output Marshalling */
struct OutputFragment {
    OutputFragment *before;
    OutputFragment *after;
    char *output;
};
void free_output(OutputFragment *output);

#endif //CREXX_COMPILER_H