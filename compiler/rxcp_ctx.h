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
 * Compiler Context Definitions
 */

#ifndef CREXX_RXCP_CTX_H
#define CREXX_RXCP_CTX_H

#include "rxcp_types.h"
#include "rxcp_token.h"
#include "rxcp_ast.h"
#include "rxcp_sym.h"

/* Compiler Context Object */
struct Context {
    struct Context *master_context; /* This points to the context of the file being compiled (rather than imported files* */
    int debug_mode;
    int stop_after_parse;
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
    char processedOptions;
    RexxLevel level;
    char comments_hash;
    char comments_dash;
    char comments_slash;
    char numeric_standard; /* 0 = common, 1 = classic - this is the mode in use */
    /* Language Options */
    char floats_decimal;
    char floats_binary;
    /* Options validation */
    char numeric_common;   /* Numeric options - used to check process numeric options specified */
    char numeric_classic;  /*                   these are set if explicitly specified           */
    char comments_slash_specified;   /* 1 - slash/no slash specified - for checking inconsistent options  */
    char comments_dash_specified;    /* 1 - dash/no dash specified - for checking inconsistent options   */
    char comments_hash_specified;    /* 1 - hash/no hash specified - for checking inconsistent options   */
    /* Plugins */
    void *decimal_plugin; /* Pointer to the decimal plugin */
    /* Optimiser Options */
    int optimise;
    int iterations;

    /* Recursion Guard for Imports */
    char** loading_files;
    size_t loading_files_count;
};

#include "rxcp_emit.h"

/*  Importable Functions and Variables */
struct imported_func {
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
};

/*  Importable Files */
struct importable_file {
    char *name;
    file_type type;
    char *location;
    char imported;
};

/* RXC Main Function */
int rxcmain(int argc, char *argv[]);

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

/* Print Unescaped*/
void prt_unex(FILE* output, const char *ptr, int len);

/* Validator AST Tree */
void rxcp_val(Context *context);
void rxcp_bvl(Context *context);

/* Diagnostic Stages */
typedef enum {
    STAGE_RAW,
    STAGE_FIXUP,
    STAGE_SYMBOLS,
    STAGE_FINAL
} DebugStage;

void rxcp_debug_header(const char *stage_name, int iteration);
void rxcp_print_ast_recursive(ASTNode *node, int indent);
void rxcp_print_symbol_table(Scope *scope, int depth);

/* Optimise AST Tree */
void optimise(Context *context);

/* Prints errors and returns the number of errors in the AST Tree */
int prnterrs(Context *context);

/* Prints warnings and returns the number of warnings in the AST Tree */
int prntwars(Context *context);

/* Emit Assembler */
void emit(Context *context, FILE *output_file);


/* printf - but returns a malloced buffer with the result */
char* mprintf(const char* format, ...);

/* imported_func factory - returns null if the function is not in an applicable namespace */
imported_func *rximpf_f(Context*  context, char* file_name, char *fqname, char *options, char *type, char *args,
                        char *implementation, char is_variable);

/* Free Func Tree and functions */
void fre_ftre(Context *context);

/* Free an imported_func */
void freimpfc(imported_func *func);

/* Get the list of importable files as a null terminated malloced array */
importable_file **rxfl_lst(Context *context);

/* free the list of importable files */
void rxfl_fre(importable_file **file_list);

/* Returns Argument definition from the ARG Node as a malloced string to be used in meta-data */
/* Node should be an ARG node else the program aborts */
char *meta_narg(ASTNode *node);

/* Static Linked Plugin Support */
struct static_linked_function {
    char *name;
    char *option;
    char *type;
    char *args;
    struct static_linked_function *next;
};
extern struct static_linked_function *static_linked_functions;
void free_static_linked_functions();

#endif //CREXX_RXCP_CTX_H
