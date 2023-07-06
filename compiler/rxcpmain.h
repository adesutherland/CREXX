/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

#ifndef CREXX_RXCPMAIN_H
#define CREXX_RXCPMAIN_H

#define rxversion "crexx f0019"

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
void *Opts_Alloc(void *(*mallocProc)(size_t));
void Opts_();
void Opts_Free();
void Opts_Trace(FILE *stream, char *zPrefix);

/* Level B Parser */
void *RexxBAlloc(void *(*mallocProc)(size_t));
void RexxB();
void RexxBFree();
void RexxBTrace(FILE *stream, char *zPrefix);

typedef enum RexxLevel {
    UNKNOWN, LEVELA, LEVELB, LEVELC, LEVELD, LEVELG, LEVELL
} RexxLevel;

typedef enum ValueType {
    TP_UNKNOWN, TP_VOID, TP_BOOLEAN, TP_INTEGER, TP_FLOAT, TP_STRING, TP_BINARY, TP_OBJECT
} ValueType;

/*  Importable Functions */
typedef struct importable_file importable_file;

/* Compiler Context Object */
typedef struct Context {
    struct Context *master_context; /* This points to the context of the file being compilkes (rather than imported files* */
    int debug_mode;
    char* location;
    char* file_name;
    char** import_locations;
    importable_file **importable_file_list;
    FILE *file_pointer;
    FILE *traceFile;
    char *buff_start;
    char *buff_end;
    char *top, *cursor, *marker, *ctxmarker, *linestart, *prev_linestart;
    char lexer_stem_mode; /* 1 if lexing a stem */
    int line;
    int token_counter;
    Token* token_head;
    Token* token_tail;
    ASTNode* ast;
    ASTNode* free_list;
    ASTNode* namespace;
    ASTNode* temp_node; /* Temporary node store to pass node between functions */
    Scope *current_scope;
    void* importable_function_tree;
    char after_rewrite; /* To avoid duplicate processing / warnings after the compiler rewrites */
    char changed; /* Flag Used to see if a walker has made a change */
    /* Do we need to import _rxsysb */
    char need_rxsysb;
    char has_rxsysb;
    /* Source Options */
    char processedComments;
    RexxLevel level;
    char hashcomments;
    char dashcomments;
    char slashcomments;
    /* Optimiser Options */
    int optimise;
} Context;

/* Context Factory */
Context *cntx_f();
/* Set Context Buffer */
void cntx_buf(Context *context, char* buff_start, size_t bytes);
/* Free Context */
void fre_cntx(Context *context);

int rexbscan(Context* s);
int rexbpars(Context *context);
int opt_scan(Context* s);
int opt_pars(Context *context);
/* Encodes a string into a malloced buffer */
char* encdstrg(const char* string, size_t length);

/* Try and import an external function - return its symbol if successful */
Symbol *sym_imfn(Context *context, ASTNode *node);

/* Set the type of a symbol from imported modules */
void sym_imva(Context *context, Symbol *symbol);

typedef enum NodeType {
    ABS_POS=1, ADDRESS, ARG, ARGS, ASSEMBLER, ASSIGN, BY, CALL, CLASS, LITERAL, CONST_SYMBOL, DEFINE,
    DO, ENVIRONMENT, ERROR, EXPOSED, EXIT, FOR, FUNCTION, IF, IMPORT, IMPORTED_FILE, INSTRUCTIONS, ITERATE, LABEL, LEAVE,
    FLOAT, INTEGER, OP_MAKE_ARRAY,
    NAMESPACE, NOP, NOVAL, OP_ADD, OP_MINUS, OP_AND, OP_ARGS, OP_ARG_VALUE, OP_ARG_EXISTS, OP_ARG_IX_EXISTS,
    OP_CONCAT, OP_MULT, OP_DIV, OP_IDIV,
    OP_MOD, OP_OR, OP_POWER, OP_NOT, OP_NEG, OP_PLUS,
    OP_COMPARE_EQUAL, OP_COMPARE_NEQ, OP_COMPARE_GT, OP_COMPARE_LT,
    OP_COMPARE_GTE, OP_COMPARE_LTE, OP_COMPARE_S_EQ, OP_COMPARE_S_NEQ,
    OP_COMPARE_S_GT, OP_COMPARE_S_LT, OP_COMPARE_S_GTE, OP_COMPARE_S_LTE,
    OP_SCONCAT, OPTIONS, PARSE, PATTERN, PROCEDURE, PROGRAM_FILE, PULL, REL_POS, RANGE, REPEAT,
    REDIRECT_IN, REDIRECT_OUT, REDIRECT_ERROR, REDIRECT_EXPOSE,
    RETURN, REXX_OPTIONS, REXX_UNIVERSE, SAY, SIGN, STRING, BINARY, TARGET, TEMPLATES, TO, TOKEN, UPPER,
    VAR_REFERENCE, VAR_SYMBOL, VAR_TARGET, VOID,
    VARG, VARG_REFERENCE, CONSTANT, WARNING, WHILE, UNTIL
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

typedef struct SymbolNode SymbolNode;

struct ASTNode {
    Context *context;
    int node_number;
    NodeType node_type;
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

/* Symbol-ASTNode Connector */
struct SymbolNode {
    ASTNode *node;
    Symbol *symbol;
    unsigned int readUsage : 1;
    unsigned int writeUsage : 1;
};

/* Print Unescaped*/
void prt_unex(FILE* output, const char *ptr, int len);

/* Token Functions */
Token* token_f(Context* context, int type);
/* Split a token - returns the first token (token->token_next) points to the next twin; */
/* the first token has len characters, the second twin as the remaining characters.       */
/* The caller can then change the tokens' types as needed.                              */
Token *tok_splt(Context *context, Token *token, int len);
/* Remove the last (tail) token */
void token_r(Context *context);
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
void pdot_ast(FILE* output, ASTNode* node, int parent, int *counter);
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
void rxcp_val(Context *context);
void rxcp_bvl(Context *context);

/* Optimise AST Tree */
void optimise(Context *context);

/* Prints errors and returns the number of errors in the AST Tree */
int prnterrs(Context *context);

/* Prints warnings and returns the number of warnings in the AST Tree */
int prntwars(Context *context);

/* Scope and Symbols */
struct Scope {
    ASTNode *defining_node;
    Scope *parent;
    char *name; /* Note that the name is free()'d by the destructor */
    void *child_array;
    void *symbols_tree;
    size_t num_registers;
    void *free_registers_array;
    size_t temp_flag;
};

/* Returns string name of a Value type */
char* type_nm(ValueType type);

typedef enum SymbolType {
    CONSTANT_SYMBOL, VARIABLE_SYMBOL, FUNCTION_SYMBOL, CLASS_SYMBOL, NAMESPACE_SYMBOL
} SymbolType;

/* Returns string name of a SymbolValue type */
char* stype_nm(SymbolType type);

struct Symbol {
    char *name;
    void *ast_node_array;
    Scope *scope;
    Scope *defines_scope;
    ValueType type;       /* Value Type */
    size_t value_dims;    /* Value dimensions */
    int *dim_base;        /* Array of starting element number for array dimension - malloced or zero */
    int *dim_elements;    /* Array of max number of elements for array dimension (0=infinite) - malloced or zero */
    char* value_class;    /* Value class name - malloced or zero */
    SymbolType symbol_type;
    int register_num;
    char register_type;
    size_t fixed_args; /* Number of fixed arguments (for a procedure) */
    char has_vargs;    /* If it has variable arguments (for a procedure) */
    char exposed;      /* Is the symbol exposed */
    char is_arg;       /* Is an argument */
    char is_ref_arg;   /* Is  reference arg */
    char is_opt_arg;   /* Is an optional arg */
    char meta_emitted; /* Has the emitter output the symbol's metadata yet */
};

/* Scope Factory */
Scope *scp_f(Scope *parent, ASTNode *node, Symbol* symbol);

/* Calls the handler for each symbol in scope */
typedef void (*symbol_worker)(Symbol *symbol, void *payload);
void scp_4all(Scope *scope, symbol_worker worker, void *payload);

/* Returns all the symbols in a scope as a null terminated malloced array (must be freed) */
Symbol **scp_syms(Scope *scope);

/* Get the index sub-scope */
Scope* scp_chd(Scope *scope, size_t index);

/* Get the number of sub-scopes */
size_t scp_noch(Scope *scope);

/* Returns the fully resolved scope name in a malloced buffer */
char* scp_frnm(Scope *scope);

/* Removes a Symbol from a scope - does not free symbol, see free_sym() */
void scp_rmsy(Scope *scope, Symbol *symbol);

/* Get a free register from scope */
int get_reg(Scope *scope);

/* Return a no longer used register to the scope */
void ret_reg(Scope *scope, int reg);

/* Get number of free register from scope - returns the start of a sequence
 * n, n+1, n+2, ... n+number */
int get_regs(Scope *scope, size_t number);

/* Return no longer used registers to the scope, starting from reg
 * reg, reg+1, ... reg+number */
void ret_regs(Scope *scope, int reg, size_t number);

/* Set the temp_flag for the scope and all its sub-scopes */
void scp_stmp(Scope *scope, size_t temp_flag);

/* Frees scope and all its symbols */
void scp_free(Scope *scope);

/* Symbol Factory - define a symbol */
/* Returns NULL if the symbol is a duplicate */
Symbol *sym_f(Scope *scope, ASTNode *node);

/* Symbol Factory - define a symbol with a name */
/* Returns NULL if the symbol is a duplicate */
Symbol *sym_fn(Scope *scope, const char* name, size_t name_length);

/* Frees a symbol - does not remove symbol from scope see scp_rmsy() which probably should be used as well */
void free_sym(Symbol *symbol);

/* Resolve a Symbol - including parent scopes */
Symbol *sym_rslv(Scope *scope, ASTNode *node);

/* Local Resolve a Symbol - current scope only */
Symbol *sym_lrsv(Scope *scope, ASTNode *node);

/* Resolve a Function Symbol
 * the root parameter should the AST root - the function checks the root of all the PROGRAM_FILE and IMPORTED_FILE
 */
Symbol *sym_rvfc(ASTNode *root, ASTNode *node);

/* Resolve a Function Symbol
 * the root parameter should the AST root - the function checks the root of all the PROGRAM_FILE and IMPORTED_FILE
 */
Symbol *sym_rvfn(ASTNode *root, char* name);

/* Move (via a merge) a Symbol into a new Scope - returns the target symbol */
Symbol *sym_merg(Scope *new_scope, Symbol *symbol);

/* Returns the index'th SymbolNode connector attached to a symbol */
SymbolNode* sym_trnd(Symbol *symbol, size_t index);

/* Returns the PROCEDURE ASTNode of a Symbol */
ASTNode* sym_proc(Symbol *symbol);

/* Returns 1 if node is linked to symbol, or 0 */
int symislnk(ASTNode *node, Symbol *symbol);

/* Add an ASTNode using the symbol */
void sym_adnd(Symbol *symbol, ASTNode* node, unsigned int readAccess,
              unsigned int writeAccess);

/* Get number of ASTNodes using the symbol */
size_t sym_nond(Symbol *symbol);

/* Disconnects a node (via index) from a symbol */
void sym_dnd(Symbol *symbol, size_t node_num);

/* Disconnects a node from a symbol */
void sym_dno(Symbol *symbol, ASTNode* node);

/* Returns the lowest ASTNode ordinal associated with the symbol */
int sym_lord(Symbol *symbol);

/*
 * Resolve a Symbol via a fully qualified Name
 * the root parameter should the AST root
 */
Symbol *sym_rfqn(ASTNode *root, const char* fqname);

/*
 * Resolve or add a Symbol via a fully qualified Name
 * the root parameter should the AST root
 * Note: Symbols / Scopes are not linked to nodes
 * Returns the existing or new symbol with the fqname
 *         or 0 if there is an error (a namespace in the path corresponds to a non-namespace symbol)
 */
Symbol *sym_afqn(ASTNode *root, const char* fqname);

/* Returns the fully resolved symbol name in a malloced buffer */
char* sym_frnm(Symbol *symbol);

/* Returns the type of a symbol as a text string in a malloced buffer */
char* sym_2tp(Symbol *symbol);

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

/* Emit Assembler */
void emit(Context *context, FILE *output_file);

/* Output Marshalling */
struct OutputFragment {
    OutputFragment *before;
    OutputFragment *after;
    char *output;
};
void f_output(OutputFragment *output);

/* printf - but returns a malloced buffer with the result */
char* mprintf(const char* format, ...);

/*  Importable Functions and Variables */
typedef struct imported_func {
    char *namespace;
    char *file_name;
    char *fqname;
    char *name;
    char *options;
    char *type;
    char *args;
    char *implementation;
    Context *context;
    char is_variable; /* 0=function, 1=global variable */
    char *error_state; /* Pointer to a constant string with error code (or null). Not malloced/freed */
    struct imported_func *duplicate;
} imported_func;

/* imported_func factory - returns null if the function is not in an applicable namespace */
imported_func *rximpf_f(Context*  context, char* file_name, char *fqname, char *options, char *type, char *args,
                        char *implementation, char is_variable);

/* Free Func Tree and functions */
void fre_ftre(Context *context);

/* Free an imported_func */
void freimpfc(imported_func *func);

/* Imported file types */
typedef enum file_type {
    REXX_FILE, RXBIN_FILE, RXAS_FILE
} file_type;

/*  Importable Functions */
struct importable_file {
    char *name;
    file_type type;
    char *location;
    char imported;
};

/* Get the list of importable files as a null terminated malloced array */
importable_file **rxfl_lst(Context *context);

/* free the list of importable files */
void rxfl_fre(importable_file **file_list);

/* Returns Argument definition from the ARG Node as a malloced string to be used in meta-data */
/* Node should be an ARG node else the program aborts */
char *meta_narg(ASTNode *node);

#endif //CREXX_RXCPMAIN_H