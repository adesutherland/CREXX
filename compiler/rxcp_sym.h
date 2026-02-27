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
 * Symbol Table Definitions
 */

#ifndef CREXX_RXCP_SYM_H
#define CREXX_RXCP_SYM_H

#include "rxcp_types.h"

/* Scope and Symbols */
struct Scope {
    ASTNode *defining_node;
    Scope *parent;
    ScopeType type;
    char *name; /* Note that the name is free()'d by the destructor */
    numeric_context num_context; /* Numeric context - digite, fuzz, form, case */
    void *child_array;
    void *symbols_tree;
    size_t num_registers;
    void *free_registers_array;
    void *deferred_registers_array;
    size_t temp_flag;
    struct Scope *reg_scope;
};

/* Symbol-ASTNode Connector */
struct SymbolNode {
    ASTNode *node;
    Symbol *symbol;
    unsigned int readUsage : 1;
    unsigned int writeUsage : 1;
};

struct Symbol {
    char *name;
    void *ast_node_array;
    Scope *scope;
    Scope *defines_scope;
    char needs_default_initiation; /* Set if the compiler should include a default initiator */
    ValueType type;       /* Value Type */
    size_t value_dims;    /* Value dimensions */
    int *dim_base;        /* Array of starting element number for array dimension - malloced or zero */
    int *dim_elements;    /* Array of max number of elements for array dimension (0=infinite) - malloced or zero */
    char* value_class;    /* Value class name - malloced or zero */
    SymbolType symbol_type;
    SymbolStatus status;
    int register_num;
    char register_type;
    size_t fixed_args; /* Number of fixed arguments (for a procedure) */
    char has_vargs;    /* If it has variable arguments (for a procedure) */
    char exposed;      /* Is the symbol exposed */
    char is_arg;       /* Is an argument */
    char is_ref_arg;   /* Is a reference arg */
    char is_opt_arg;   /* Is an optional arg */
    char is_const_arg; /* Is a constant arg */
    char meta_emitted; /* Has the emitter output the symbol's metadata */
    char init_emitted; /* Has the emitter output the symbol's default inititator */
};

/* Returns string name of a Value type */
char* type_nm(ValueType type);

/* Returns string name of a SymbolValue type */
char* stype_nm(SymbolType type);

/* Scope Factory */
Scope *scp_f(Context *context, Scope *parent, ASTNode *node, Symbol* symbol, ScopeType type);

/* Calls the handler for each symbol in scope */
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

/* Get a permanent register from scope (not reused) */
int get_reg_perm(Scope *scope);

/* Return a no longer used register to the scope */
void ret_reg(Scope *scope, int reg);

/* Return a linked register later (end of statement) */
void ret_reg_later(Scope *scope, int reg);

/* Return all deferred registers */
void ret_reg_all_deferred(Scope *scope);

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

/* Local Resolve a Symbol - current scope only */
Symbol *sym_lrsv(Scope *scope, ASTNode *node);

/* Resolve a Symbol - only in the current procedure (and nested local scopes) */
Symbol *sym_rslv_local(Scope *scope, ASTNode *node);

/* Resolve a Symbol - search for class attributes */
Symbol *sym_rslv_attribute(Scope *scope, ASTNode *node);

/* Resolve a Symbol - search for global symbols in namespaces */
Symbol *sym_rslv_global(Scope *scope, ASTNode *node);

/* Resolve a Symbol - Tiered Search: Local -> Attribute -> Global */
Symbol *sym_rslv_tiered(Scope *scope, ASTNode *node);

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

/* Hoist a symbol to a namespace scope (EXPOSE) */
Symbol *sym_hoist_to_namespace(Symbol *symbol, Scope *target_namespace);

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
char* sym_mngd_frnm(Symbol *symbol);

/* Returns the type of a symbol as a text string in a malloced buffer */
char* sym_2tp(Symbol *symbol);

#endif //CREXX_RXCP_SYM_H
