/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

#ifndef CREXX_RXCPMAIN_H
#define CREXX_RXCPMAIN_H

#define rxversion "cREXX-Phase-0 v0.1.6 + F0026"

#include <stdio.h>
#include "platform.h"

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
    char *buff_start;
    char *buff_end;
    char *top, *cursor, *marker, *ctxmarker, *linestart;
    int line;
    int token_counter;
    Token* token_head;
    Token* token_tail;
    ASTNode* ast;
    ASTNode* free_list;
    RexxLevel level;
    int optimise;
} Context;

int rexbscan(Context* s);
int rexbpars(Context *context);
int opt_scan(Context* s);
int opt_pars(Context *context);

typedef enum NodeType {
    ABS_POS=1, ADDRESS, ARG, ASSIGN, BY, CALL, CONST_SYMBOL,
    DO, ENVIRONMENT, ERROR, FOR, FUNCTION, IF, INSTRUCTIONS, ITERATE, LABEL, LEAVE,
    FLOAT, INTEGER, NOP, OP_ADD, OP_MINUS, OP_AND, OP_CONCAT, OP_MULT, OP_DIV, OP_IDIV,
    OP_MOD, OP_OR, OP_POWER, OP_PREFIX,
    OP_COMPARE_EQUAL, OP_COMPARE_NEQ, OP_COMPARE_GT, OP_COMPARE_LT,
    OP_COMPARE_GTE, OP_COMPARE_LTE, OP_COMPARE_S_EQ, OP_COMPARE_S_NEQ,
    OP_COMPARE_S_GT, OP_COMPARE_S_LT, OP_COMPARE_S_GTE, OP_COMPARE_S_LTE,
    OP_SCONCAT, OPTIONS, PARSE, PATTERN, PROCEDURE, PROGRAM_FILE, PULL, REL_POS, REPEAT,
    RETURN, REXX_OPTIONS, SAY, SIGN, STRING, TARGET, TEMPLATES, TO, TOKEN, UPPER,
    VAR_SYMBOL, VAR_TARGET, CONSTANT
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
    char *node_string;
    size_t node_string_length;
    char free_node_string;
    rxinteger int_value;
    int bool_value;
    double float_value;
    /* These are only valid after the set_source_location walker has run */
    Token *token_start, *token_end;
    char *source_start, *source_end;
    int line, column;
    /* These are used by the code emitters */
    OutputFragment *output;
    OutputFragment *output2;
    OutputFragment *output3;
    OutputFragment *output4;
};

/* Print Unescaped*/
void prt_unex(FILE* output, const char *ptr, int len);

/* Token Functions */
Token* token_f(Context* context, int type);
void free_tok(Context* context);
void prnt_tok(Token* token);
const char* tk_tp_nm(int type); /* Get Token Type Name */

/* AST Functions */
ASTNode* ast_f(Context* context, NodeType type, Token *token);
/* ASTNode Factory - adds a STRING token removing the leading & trailing speech marks */
ASTNode *ast_fstr(Context* context, Token *token);
/* ASTNode Factory - With node type*/
ASTNode *ast_ft(Context* context, NodeType type);
/* ASTNode Factory - With node type and string value */
ASTNode *ast_ftt(Context* context, NodeType type, char *string);
/* ASTNode Factory - Error Node */
ASTNode *ast_err(Context* context, char *error_string, Token *token);
/* ASTNode Factory - Error at last Node */
ASTNode *ast_errh(Context* context, char *error_string);
const char *ast_ndtp(NodeType type);
void prnt_ast(ASTNode* node);
void pdot_ast(FILE* output, ASTNode* node, int parent, int *counter);
ASTNode* add_ast(ASTNode* parent, ASTNode* child); /* Add Child - Returns child for chaining */
ASTNode *add_sbtr(ASTNode *older, ASTNode *younger); /* Add sibling - Returns younger for chaining */
/* Turn a node to an ERROR */
void mknd_err(ASTNode* node, char *error_string);
void free_ast(Context* context);
void pdot_tree(ASTNode *tree, char* output_file);
/* Set the string value of an ASTNode. string must be malloced. memory is
 * then managed by the AST Library (the caller must not free it) */
void ast_sstr(ASTNode *node, char* string, size_t length);
/* Set a node string to a static value (i.e. the node isn't responsible for
 * freeing it). See also ast_sstr() */
void ast_str(ASTNode* node, char *string);
/* Replace replaced_node with new_node in the tree
 * note that replaced_node should not be a descendant or direct relation of
 * new_node (else we will get a loop in the tree)! */
void ast_rpl(ASTNode* replaced_node, ASTNode* new_node);
/* Delete / Remove node (i.e. the whole subtree) from the tree */
void ast_del(ASTNode* node);

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
walker_result ast_wlkr(ASTNode *tree, walker_handler handler, void *payload);

/* Validator AST Tree */
void validate(Context *context);

/* Optimise AST Tree */
void optimise(Context *context);

/* Prints errors and returns the number of errors in the AST Tree */
int prnterrs(Context *context);

/* Scope and Symbols */
struct Scope {
    ASTNode *defining_node;
    Scope *parent;
    void *child_array;
    void *symbols_tree;
    size_t num_registers;
    void *free_registers_array;
};

char* type_nm(ValueType type);

struct Symbol {
    char *name;
    void *ast_node_array;
    Scope *scope;
    ValueType type;
    int register_num;
    char is_constant;
};

/* Scope Factory */
Scope *scp_f(Scope *parent, ASTNode *node);

/* Calls the handler for each symbol in scope */
typedef void (*symbol_worker)(Symbol *symbol, void *payload);
void scp_4all(Scope *scope, symbol_worker worker, void *payload);

/* To get sub-scopes */
Scope* scp_chd(Scope *scope, size_t index);
size_t scp_noch(Scope *scope);

/* Get a free register from scope */
int get_reg(Scope *scope);

/* Return a no longer used register to the scope */
void ret_reg(Scope *scope, int reg);

/* Frees scope and all its symbols */
void scp_free(Scope *scope);

/* Symbol Factory - define a symbol */
Symbol *sym_f(Scope *scope, ASTNode *node);

/* Resolve a Symbol */
Symbol *sym_rslv(Scope *scope, ASTNode *node);

/* Get a the index'th ASTNode using the symbol */
ASTNode* sym_trnd(Symbol *symbol, size_t index);

/* Add a AST Node using the symbol */
void sym_adnd(Symbol *symbol, ASTNode* node);

/* Get number of ASTNodes using the symbol */
size_t sym_nond(Symbol *symbol);

/* Emit Assembler */
void emit(Context *context, FILE *output_file);

/* Output Marshalling */
struct OutputFragment {
    OutputFragment *before;
    OutputFragment *after;
    char *output;
};
void f_output(OutputFragment *output);

#endif //CREXX_RXCPMAIN_H