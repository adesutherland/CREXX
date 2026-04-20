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
 * Symbol Table Management
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "platform.h"
#include "rxcpmain.h"
#include "../avl_tree/avl_tree.h"
#include "rxcpdary.h"

#ifndef NUTF8
#include "utf.h"
#endif

/* Internal Tree node structure */
struct symbol_wrapper {
    char *index;
    Symbol *value;
    struct avl_tree_node index_node;
};

#define SYMBOL_WRAPPER(i) avl_tree_entry((i), struct symbol_wrapper, index_node)

#define GET_INDEX(i) avl_tree_entry((i), struct symbol_wrapper, index_node)->index

#define GET_VALUE(i) avl_tree_entry((i), struct symbol_wrapper, index_node)->value

static int compare_node_node(const struct avl_tree_node *node1,
                             const struct avl_tree_node *node2)
{
    char* n1 = GET_INDEX(node1);
    char* n2 = GET_INDEX(node2);
    return strcmp(n1,n2);
}

static int compare_node_value(const void *value,
                              const struct avl_tree_node *nodeptr) {
    char* node = GET_INDEX(nodeptr);
    return strcmp((char*)value,node);
}

/* Adds a symbol to tree */
/* Returns 0 on success, 1 on duplicate */
static int add_symbol_to_tree(struct avl_tree_node **root, Symbol *value) {

    struct symbol_wrapper *i = malloc(sizeof(struct symbol_wrapper));
    i->index = value->name;
    i->value = value;
    if (avl_tree_insert(root, &i->index_node, compare_node_node)) {
        /* Duplicate! */
        free(i);
        return 1;
    }
    return 0;
}

// Search for a symbol
// Returns Symbol if found or null
static Symbol* src_symbol(struct avl_tree_node *root, const char* index) {
    struct avl_tree_node *result;

    result = avl_tree_lookup(root, index, compare_node_value);

    if (result) return GET_VALUE(result);
    else return NULL;
}

/* Scope Factory */
Scope *scp_f(Context* context, Scope *parent, ASTNode *node, Symbol* symbol, ScopeType type) {
    char* name;
    Scope *scope = (Scope*) malloc(sizeof(Scope));
    scope->defining_node = node;
    scope->type = type;
    if (symbol) {
        name = symbol->name;
        scope->name = malloc(strlen(name) + 1);
        strcpy(scope->name, name);
        /* Update Symbol */
        if (!symbol->defines_scope) symbol->defines_scope = scope;
    }
    else scope->name = 0;
    if (node) node->scope = scope;
    scope->parent = parent;
    scope->symbols_tree = 0;
    if (parent) {
        scope->num_context.digits = parent->num_context.digits;
        scope->num_context.fuzz = parent->num_context.fuzz;
        scope->num_context.form = parent->num_context.form;
        scope->num_context.casetype = parent->num_context.casetype;
        scope->num_context.standard = parent->num_context.standard;
    }
    else {
        scope->num_context.digits = DEFAULT_NUMERIC_DIGITS;
        scope->num_context.fuzz = DEFAULT_NUMERIC_FUZZ;
        scope->num_context.form = DEFAULT_NUMERIC_FORM;
        scope->num_context.casetype = DEFAULT_NUMERIC_CASE;
        if (context->numeric_standard)
            scope->num_context.standard = NUMERIC_STANDARD_CLASSIC;
        else
            scope->num_context.standard = NUMERIC_STANDARD_COMMON;
    }

    /* Force inherited context for PROCEDURE nodes - REMOVED */
    /* Procedures now default to standard numeric context unless explicitly set */
    scope->num_registers = 0; /* Changed from 1 - r0 is no longer a hardcoded temp register */
    scope->free_registers_array = dpa_f();
    scope->deferred_registers_array = dpa_f();
    scope->child_array  = dpa_f();
    scope->temp_flag = 0;
    if (parent) dpa_add((dpa*)(parent->child_array), scope);

    if (type == SCOPE_LOCAL || (node && node->inherit_parent_reg_scope)) {
        scope->reg_scope = parent ? parent->reg_scope : scope;
    } else {
        scope->reg_scope = scope;
    }

    return scope;
}

/* Calls the handler for each symbol in scope */
void scp_4all(Scope *scope, symbol_worker worker, void *payload) {
    struct symbol_wrapper *i;

    if (scope && scope->symbols_tree) {
        /* This walks the tree in sort order - do not alter list! */
        avl_tree_for_each_in_order(i, scope->symbols_tree, struct symbol_wrapper,
                                       index_node) {
            worker(i->value, payload);
        }
    }
}

/* Returns all the symbols in a scope as a null terminated malloced array (must be freed) */
Symbol **scp_syms(Scope *scope) {
    Symbol **symbols;
    size_t num;
    struct symbol_wrapper *i;

    /* Null - return an empty array */
    if (!scope || !scope->symbols_tree) {
        symbols = malloc(sizeof(Symbol*));
        symbols[0] = 0;
        return symbols;
    }

    /* How many symbols? */
    num = 0;
    {
        avl_tree_for_each_in_order(i, scope->symbols_tree, struct symbol_wrapper, index_node) num++;
    }

    /* malloc the array */
    symbols = malloc(sizeof(Symbol*) * (num + 1));
    symbols[num] = 0;

    /* Populate the array */
    num = 0;
    {
        avl_tree_for_each_in_order(i, scope->symbols_tree, struct symbol_wrapper, index_node) symbols[num++] = i->value;
    }

    return symbols;
}

/* Frees scope and all its symbols and sub-scopes */
void scp_free(Scope *scope) {
    struct symbol_wrapper *i;
    size_t j;

    if (scope->name) free(scope->name);

    if (scope->symbols_tree) {
        /* This walks the tree in post order which allows each node be freed */
        avl_tree_for_each_in_postorder(i, scope->symbols_tree,
                                       struct symbol_wrapper,
                                       index_node) {
            free_sym(i->value);
            free(i);
        }
        scope->symbols_tree = 0; /* Pedantic ... */
    }

    /* Free sub-scopes */
    for (j=0; j < ((dpa*)(scope->child_array))->size; j++) {
        scp_free( ((dpa*)(scope->child_array))->pointers[j] );
    }

    free_dpa(scope->child_array);
    free_dpa(scope->free_registers_array);
    free_dpa(scope->deferred_registers_array);
    free(scope);
}

/* Removes a Symbol from a scope - does not free symbol, see free_sym() */
void scp_rmsy(Scope *scope, Symbol *symbol) {
    struct avl_tree_node *result;

    /* Find the symbol */
    result = avl_tree_lookup((struct avl_tree_node *)scope->symbols_tree, symbol->name, compare_node_value);

    if (result)  {
        /* If found ... */

        /* Remove from tree */
        avl_remv((struct avl_tree_node **)&(scope->symbols_tree), result);

        /* Free the symbol_wrapper */
        free(SYMBOL_WRAPPER(result));
    }
}


/* Set the temp_flag for the scope and all its sub-scopes */
void scp_stmp(Scope *scope, size_t temp_flag) {
    size_t j;

    /* Set the temp_flag for this scope */
    scope->temp_flag = temp_flag;

    /* Set the temp_flag for child sub-scopes */
    for (j = 0; j < ((dpa *) (scope->child_array))->size; j++) {
        scp_stmp(((dpa *) (scope->child_array))->pointers[j], temp_flag);
    }
}

/* Get a free register from scope */
int get_reg(Scope *scope) {
    dpa *free_array;
    int reg;
    Scope *rs = scope->reg_scope ? scope->reg_scope : scope;

    free_array = (dpa*)(rs->free_registers_array);

    /* Check the free list */
    if (free_array->size) {
        free_array->size--;
        reg = (int)(size_t)(free_array->pointers[free_array->size]);
    }
    else {
        reg = (int)((rs->num_registers)++);
    }

    return reg;
}

/* Get a permanent register from scope (not reused) */
int get_reg_perm(Scope *scope) {
    Scope *rs = scope->reg_scope ? scope->reg_scope : scope;
    return (int)((rs->num_registers)++);
}

/* Return a no longer used register to the scope */
void ret_reg(Scope *scope, int reg) {
    size_t i;
    dpa *free_array;
    Scope *rs = scope->reg_scope ? scope->reg_scope : scope;
    free_array = (dpa*)(rs->free_registers_array);

    if (reg < 0) {
        return;
    }

    for (i=0; i<free_array->size; i++) {
        if (reg == (size_t)free_array->pointers[i]) {
//            printf("Reg %d already freed - free array remains ", reg);
//            {int ii; for (ii=0; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);printf("\n");}
            return;
        }
    }
    dpa_ado(free_array, (void*)(size_t)reg);

//    printf(" - free array is now ");
//    {int ii; for (ii=0; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);}
//    printf("\n");
}

/* Return a linked register later (end of statement) */
void ret_reg_later(Scope *scope, int reg) {
    size_t i;
    dpa *deferred_array;
    Scope *rs = scope->reg_scope ? scope->reg_scope : scope;
    deferred_array = (dpa*)(rs->deferred_registers_array);

    if (reg < 0) {
        return;
    }

    for (i=0; i<deferred_array->size; i++) {
        if (reg == (size_t)deferred_array->pointers[i]) {
            return;
        }
    }
    dpa_add(deferred_array, (void*)(size_t)reg);
}

/* Return all deferred registers */
void ret_reg_all_deferred(Scope *scope) {
    size_t i;
    dpa *deferred_array;
    Scope *rs = scope->reg_scope ? scope->reg_scope : scope;
    deferred_array = (dpa*)(rs->deferred_registers_array);

    for (i=0; i<deferred_array->size; i++) {
        ret_reg(rs, (int)(size_t)deferred_array->pointers[i]);
    }
    deferred_array->size = 0;
}

/* Get number of free register from scope - returns the start of a sequence
 * n, n+1, n+2, ... n+number */
int get_regs(Scope *scope, size_t number) {
    dpa *free_array;
    int reg, r, top, i;
    size_t seq;
    Scope *rs = scope->reg_scope ? scope->reg_scope : scope;

    if (number == 1) return get_reg(rs);

    free_array = (dpa*)(rs->free_registers_array);

    /* Check the free list - how many could be used */
    if (free_array->size) {
        i = (int)free_array->size - 1;
        top = (int)(size_t)(free_array->pointers[i]);
        for (seq=1, i--; i>=0; i--) {
            r = (int)(size_t)(free_array->pointers[i]);
            if (r == top - 1) {
                /* Part of the sequence */
                top--;
                seq++;
                if (seq >= number) {
                    /* We have enough registers to reuse */
                    reg = top; /* Result is the beginning of the sequence */
                    /* Now remove them from the free list */
                    free_array->size -= number;
//                    printf("  a-returned %d-%d - free array is now ", reg, reg+(int)number - 1);
//                    {int ii; for (ii=0; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);printf("\n");}
                    return reg;
                }
            }
            else break;
        }
        /* seq is now the number of registers which could be used in the free list */
        /* top is the first register which may be useful */
        /* Can we use these plus some new ones */
        r = (int)(size_t)(free_array->pointers[free_array->size - 1]) + 1;
        if (r == (int)(rs->num_registers)) {
            /* Yes we can because the next unused register adds to the sequence */
            reg = top; /* Result is the beginning of the sequence */
            /* Now remove them from the free list */
            free_array->size -= seq;
            /* Now assign some brand ne ones */
            rs->num_registers += number - seq;
//            printf("  b-returned %d-%d - free array is now ", reg, reg+(int)number - 1);
//            {int ii; for (ii=0; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);printf("\n");}
            return reg;
        }
        /* No we can't so just assign new ones */
    }

    reg = (int)(rs->num_registers); /* Assign brand-new registers */
    rs->num_registers += number;
//    printf("  c-returned %d-%d - free array is now ", reg, reg+(int)number - 1);
//    {int ii; for (ii=0; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);printf("\n");}
    return reg;
}

/* Return no longer used registers to the scope, starting from reg
 * reg, reg+1, ... reg+number */
/*
void ret_regs(Scope *scope, int reg, size_t number) {
    dpa *free_array;
    size_t j, i;
    free_array = (dpa*)(scope->free_registers_array);

//    printf("free %d-%d - ", reg, reg + (int)number - 1);
//    {int ii; for (ii=1; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);printf("\n");}

    for (j=0; j<number; j++) {
        for (i=0; i<free_array->size; i++)
            if (reg == (size_t)free_array->pointers[i]) {
//                printf(" ... %d already freed\n",reg);
                break;
            }
        dpa_ado(free_array, (void*)(size_t)reg);
        reg++;
    }
}
*/

/* Returns string name of a Value type */
char* type_nm(ValueType type) {
    switch (type) {
        case TP_BOOLEAN: return ".boolean";
        case TP_INTEGER: return ".int";
        case TP_FLOAT: return ".float";
        case TP_DECIMAL: return ".decimal";
        case TP_STRING: return ".string";
        case TP_BINARY: return ".binary";
        case TP_OBJECT: return ".object";
        case TP_VOID: return ".void";
        default: return ".unknown";
    }
}


/* Returns the type of a symbol as a text string in a malloced buffer */
char* sym_2tp(Symbol *symbol) {
    char *buffer = 0;
    char *array;
    char *result;
    int free_buffer = 0;

    if (symbol->value_class) {
        buffer = mprintf(".%s", symbol->value_class);
        free_buffer = 1;
    }
    else buffer = type_nm(symbol->type);

    array = ast_astr(symbol->value_dims, symbol->dim_base, symbol->dim_elements);

    result = malloc(strlen(buffer) + strlen(array) + 1);
    strcpy(result, buffer);
    strcat(result, array);

    free(array);
    if (free_buffer) free(buffer);

    return result;
}

/* Returns string name of a SymbolValue type */
char* stype_nm(SymbolType type) {
    switch (type) {
        case CONSTANT_SYMBOL: return "Constant";
        case VARIABLE_SYMBOL: return "Variable";
        case FUNCTION_SYMBOL: return "Function";
        case CLASS_SYMBOL: return "Class";
        case NAMESPACE_SYMBOL: return "Namespace";
        default: return "UnknownSymbolType";
    }
}

Scope* scp_chd(Scope *scope, size_t index) {
    return (Scope*)((dpa*)(scope->child_array))->pointers[index];
}

size_t scp_noch(Scope *scope) {
    return ((dpa*)(scope->child_array))->size;
}

/* Symbol Factory - define a symbol with a name */
/* Returns NULL if the symbol is a duplicate */
Symbol *sym_fn(Scope *scope, const char* name, size_t name_length) {
    char *c;
    Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));

    /* Normalise stem variables by stripping the trailing dot */
    if (name_length > 0 && name[name_length - 1] == '.') {
        name_length--;
    }

    symbol->scope = scope;
    symbol->ast_node_array = 0;
    symbol->defines_scope = 0;
    symbol->type = TP_UNKNOWN;
    symbol->value_dims = 0;
    symbol->dim_base = 0;
    symbol->dim_elements = 0;
    symbol->value_class = 0;
    symbol->needs_default_initiation = 0;
    symbol->register_num = -1;
    symbol->name = (char*)malloc(name_length + 1);
    memcpy(symbol->name, name, name_length);
    symbol->name[name_length] = 0;
    symbol->register_type = 'r';
    symbol->symbol_type = UNKNOWN_SYMBOL;
    symbol->status = SYM_STATUS_UNRESOLVED;
    symbol->meta_emitted = 0;
    symbol->init_emitted = 0;
    symbol->fixed_args = 0;
    symbol->has_vargs = 0;
    symbol->exposed = 0;
    symbol->is_arg = 0;
    symbol->is_ref_arg = 0;
    symbol->is_const_arg = 0;
    symbol->is_opt_arg = 0;
    symbol->is_main = 0;
    symbol->is_implicit_main = 0;
    symbol->is_rc = 0;
    symbol->is_this = 0;
    symbol->is_factory = 0;
    symbol->is_shadowing = 0;
    symbol->shadowed_symbol = 0;
    symbol->is_global_var = 0;
    symbol->creation_ordinal = -1;
    symbol->creation_node = 0;

    /* Lowercase symbol name */
#ifdef NUTF8
    for (c = symbol->name ; *c; ++c) *c = (char)tolower(*c);
#else
    utf8lwr(symbol->name);
#endif

    /* Set special symbol flags based on lowercased name */
    if (strcmp(symbol->name, "main") == 0) symbol->is_main = 1;
    else if (strcmp(symbol->name, "rc") == 0) symbol->is_rc = 1;
    else if (strcmp(symbol->name, "\xc2\xa7" "this") == 0) symbol->is_this = 1;
    else if (strcmp(symbol->name, "\xc2\xa7" "factory") == 0) symbol->is_factory = 1;

    symbol->ast_node_array = dpa_f();

    /* Returns 1 on duplicate */
    if (add_symbol_to_tree((struct avl_tree_node **)&(scope->symbols_tree),
                           symbol)) {
        Symbol *existing = src_symbol((struct avl_tree_node *)(scope->symbols_tree), symbol->name);
        free_sym(symbol);
        return existing;
    }

    return symbol;
}

/* Symbol Factory - define a symbol */
/* Returns NULL if the symbol is a duplicate */
Symbol *sym_f(Scope *scope, ASTNode *node) {
    return sym_fn(scope, node->node_string, node->node_string_length);
}

/* Resolve a Function Symbol via Name
 * the root parameter should the AST root - the function checks the root of all the PROGRAM_FILE and IMPORTED_FILE
 */
Symbol *sym_rvfn(ASTNode *root, char* name) {
    Symbol *result;
    size_t i;
    Scope *s;

    if (!root || !root->scope) return 0;

    /* Process top layer - files and imported namespaces */
    for (i = 0; i < scp_noch(root->scope); i++) {
        s = scp_chd(root->scope, i);

        /* Search symbols directly under the file or namespace scope */
        result = src_symbol((struct avl_tree_node *)(s->symbols_tree), name);
        if (result) return result;
    }
    return 0;
}

/* Resolve a Function Symbol recursively through child namespaces */
Symbol *sym_rvfn_deep(ASTNode *root, char* name) {
    Symbol *result;
    size_t i, j;
    Scope *s, *ns;
    Symbol **syms;

    if (!root || !root->scope) return 0;

    /* Process top layer - files and imported namespaces */
    for (i = 0; i < scp_noch(root->scope); i++) {
        s = scp_chd(root->scope, i);

        /* Search symbols directly under the file or namespace scope */
        result = src_symbol((struct avl_tree_node *)(s->symbols_tree), name);
        if (result) return result;

        /* Search child namespaces if this is an imported file */
        syms = scp_syms(s);
        for (j = 0; syms && syms[j]; j++) {
            if (syms[j]->symbol_type == NAMESPACE_SYMBOL && syms[j]->defines_scope) {
                ns = syms[j]->defines_scope;
                result = src_symbol((struct avl_tree_node *)(ns->symbols_tree), name);
                if (result) {
                    free(syms);
                    return result;
                }
            }
        }
        if (syms) free(syms);
    }
    return 0;
}

/*
 * Resolve a Symbol via a fully qualified Name
 * the root parameter should the AST root
 */
Symbol *sym_rfqn(ASTNode *root, const char* fqname) {
   const char *name = fqname;
   const char *c;
   Symbol *result = 0;
   if (!root) return 0;
   Scope *scope = root->scope;
   char* search_name;
   size_t len;

   if (!scope) return 0;

   while (*name) {
       for (c = name; 1; c++) {
           if (*c == '.') {
               /* Search namespace */
               len = c - name;
               search_name = malloc(len + 1);
               memcpy(search_name, name, len);
               search_name[len] = 0;
               result = src_symbol((struct avl_tree_node *)(scope->symbols_tree), search_name);
               free(search_name);
               if (!result) return 0;
               if (!result->defines_scope) return 0;
               scope = result->defines_scope;
               if (!scope) return 0;
               name = c + 1;
               break;
           }
           else if (!*c) {
               /* Search for final symbol */
               if (!scope) return 0;
               return src_symbol((struct avl_tree_node *)(scope->symbols_tree), name);
           }
       }
   }

   /* Empty parameter */
   return 0;
}

/*
 * Resolve or add a Symbol via a fully qualified Name
 * the root parameter should the AST root
 * Note: Symbols / Scopes are not linked to nodes
 * Returns the existing or new symbol with the fqname
 *         or 0 if there is an error (a namespace in the path corresponds to a non-namespace symbol)
 */
Symbol *sym_afqn(ASTNode *root, const char* fqname) {
    const char *name = fqname;
    const char *c;
    Symbol *result = 0;
    if (!root) return 0;
    Scope *scope = root->scope;
    char* search_name;
    size_t len;

    if (!scope) return 0;

    while (*name) {
        for (c = name; 1; c++) {
            if (*c == '.') {
                /* Search or add namespace */
                len = c - name;
                search_name = malloc(len + 1);
                memcpy(search_name, name, len);
                search_name[len] = 0;
                result = src_symbol((struct avl_tree_node *)(scope->symbols_tree), search_name);
                if (!result) {
                    /* Create scope */
                    result = sym_fn(scope, search_name, len);
                    scope = scp_f(root->context, scope, 0, result, SCOPE_NAMESPACE);
                    result->symbol_type = NAMESPACE_SYMBOL;
                    result->status = SYM_STATUS_LOCAL_DEF;
                    result->defines_scope = scope;
                }
                else if (!result->defines_scope) {
                    free(search_name);
                    return 0;
                }
                scope = result->defines_scope;
                if (!scope) {
                    free(search_name);
                    return 0;
                }
                name = c + 1;
                free(search_name);
                break;
            }
            else if (!*c) {
                /* Search for final symbol */
                if (!scope) return 0;
                result = src_symbol((struct avl_tree_node *)(scope->symbols_tree), name);
                if (!result) {
                    result = sym_fn(scope, name, strlen(name));
                }
                return result;
            }
        }
    }

    /* Empty parameter */
    return 0;
}

/* Resolve a Symbol - only in the current procedure (and nested local scopes) */
Symbol *sym_rslv_local(Scope *scope, ASTNode *node) {
    Symbol *result;
    char *c;
    size_t len = node->node_string_length;

    if (!scope) return 0;

    /* Normalise stem variables by stripping the trailing dot */
    if (len > 0 && node->node_string[len - 1] == '.') {
        len--;
    }

    /* Sadly we are making a null terminated string */
    char *name = (char*)malloc(len + 1);
    memcpy(name, node->node_string, len);
    name[len] = 0;

    /* Lowercase symbol name */
#ifdef NUTF8
    for (c = name ; *c; ++c) *c = (char)tolower(*c);
#else
    utf8lwr(name);
#endif

    /* Look for the symbol - looking up in each parent scope until we hit a PROCEDURE or CLASS */
    while (scope) {
        result = src_symbol((struct avl_tree_node *)(scope->symbols_tree), name);
        if (result) {
            free(name);
            return result;
        }
        if (scope->type == SCOPE_PROCEDURE || scope->type == SCOPE_CLASS) break;
        scope = scope->parent;
    }
    free(name);
    return 0;
}

/* Resolve a Symbol - search for class attributes */
Symbol *sym_rslv_attribute(Scope *scope, ASTNode *node) {
    Symbol *result;
    char *c;
    size_t len = node->node_string_length;

    if (!scope) return 0;

    /* Normalise stem variables by stripping the trailing dot */
    if (len > 0 && node->node_string[len - 1] == '.') {
        len--;
    }

    /* Sadly we are making a null terminated string */
    char *name = (char*)malloc(len + 1);
    memcpy(name, node->node_string, len);
    name[len] = 0;

    /* Lowercase symbol name */
#ifdef NUTF8
    for (c = name ; *c; ++c) *c = (char)tolower(*c);
#else
    utf8lwr(name);
#endif

    /* Find the nearest CLASS scope */
    while (scope && scope->type != SCOPE_CLASS) {
        scope = scope->parent;
    }

    if (scope) {
        result = src_symbol((struct avl_tree_node *)(scope->symbols_tree), name);
        free(name);
        return result;
    }

    free(name);
    return 0;
}

/* Resolve a Symbol - search for global symbols in namespaces */
Symbol *sym_rslv_global(Scope *scope, ASTNode *node) {
    Symbol *result;
    char *c;
    size_t len = node->node_string_length;

    if (!scope) return 0;

    /* Normalise stem variables by stripping the trailing dot */
    if (len > 0 && node->node_string[len - 1] == '.') {
        len--;
    }

    /* Sadly we are making a null terminated string */
    char *name = (char*)malloc(len + 1);
    memcpy(name, node->node_string, len);
    name[len] = 0;

    /* Lowercase symbol name */
#ifdef NUTF8
    for (c = name ; *c; ++c) *c = (char)tolower(*c);
#else
    utf8lwr(name);
#endif

    /* Find the nearest NAMESPACE or UNIVERSE scope */
    while (scope && scope->type != SCOPE_NAMESPACE && scope->type != SCOPE_UNIVERSE) {
        scope = scope->parent;
    }

    /* Look for the symbol - looking up in each parent scope (namespaces can be nested) */
    while (scope) {
        result = src_symbol((struct avl_tree_node *)(scope->symbols_tree), name);
        if (result) {
            free(name);
            return result;
        }
        scope = scope->parent;
    }

    free(name);
    return 0;
}

/* Resolve a Symbol - Tiered Search: Local -> Attribute -> Global */
Symbol *sym_rslv_tiered(Scope *scope, ASTNode *node) {
    Symbol *result;
    if (!scope) return 0;
    result = sym_rslv_local(scope, node);
    if (result) return result;
    result = sym_rslv_attribute(scope, node);
    if (result) return result;
    return sym_rslv_global(scope, node);
}

/* Resolve a Function Symbol
 * the root parameter should the AST root - the function checks the root of all the PROGRAM_FILE and IMPORTED_FILE
 */
Symbol *sym_rvfc(ASTNode *root, ASTNode *node) {
    Symbol *result;
    char *c;
    char *name;

    size_t start = 0;
    size_t len = node->node_string_length;

    /* Make a null terminated string */
    if (len > 0 && node->node_string[0] == '.') {
        start = 1;
        len--;
    } else if (len >= 2 && (node->node_string[0] == '\'' || node->node_string[0] == '\"') && node->node_string[len - 1] == node->node_string[0]) {
        start = 1;
        len -= 2;
    }
    name = (char*)malloc(len + 1);
    memcpy(name, node->node_string + start, len);
    name[len] = 0;

    /* Lowercase symbol name */
#ifdef NUTF8
    for (c = name ; *c; ++c) *c = (char)tolower(*c);
#else
    utf8lwr(name);
#endif

    result = sym_rvfn_deep(root, name);
    free(name);
    return result;
}

/* Local Resolve a Symbol - current scope only */
Symbol *sym_lrsv(Scope *scope, ASTNode *node) {
    Symbol *result = 0;
    char *c;
    size_t len = node->node_string_length;

    if (!scope) return 0;

    /* Normalise stem variables by stripping the trailing dot */
    if (len > 0 && node->node_string[len - 1] == '.') {
        len--;
    }

    /* Sadly we are making a null terminated string */
    char *name = (char*)malloc(len + 1);
    memcpy(name, node->node_string, len);
    name[len] = 0;

    /* Lowercase symbol name */
#ifdef NUTF8
    for (c = name ; *c; ++c) *c = (char)tolower(*c);
#else
    utf8lwr(name);
#endif

    result = src_symbol((struct avl_tree_node *)(scope->symbols_tree), name);
    free(name);
    return result;
}

/* Deep Resolve a Symbol - current scope and all its sub-scopes (e.g. nested DO blocks) */
Symbol *sym_drsv(Scope *scope, ASTNode *node) {
    Symbol *result = sym_lrsv(scope, node);
    if (result) return result;

    size_t i;
    for (i = 0; i < scp_noch(scope); i++) {
        result = sym_drsv(scp_chd(scope, i), node);
        if (result) return result;
    }
    return 0;
}

/* Move (via a merge) a Symbol into a new Scope - returns the target symbol */
Symbol *sym_merg(Scope *new_scope, Symbol *symbol) {
    size_t i;
    SymbolNode *connector;

    if (!new_scope) return symbol;

    /* Find or create symbol in new_scope */
    Symbol *new_symbol = src_symbol((struct avl_tree_node *)(new_scope->symbols_tree), symbol->name);
    if (!new_symbol) {
        new_symbol = sym_fn(new_scope, symbol->name, strlen(symbol->name));
        new_symbol->symbol_type = symbol->symbol_type;
        new_symbol->status = symbol->status;
        new_symbol->type = symbol->type;
        new_symbol->value_dims = symbol->value_dims;
        if (symbol->dim_base) {
            new_symbol->dim_base = malloc(sizeof(int) * symbol->value_dims);
            memcpy(new_symbol->dim_base, symbol->dim_base, sizeof(int) * symbol->value_dims);
        }
        if (symbol->dim_elements) {
            new_symbol->dim_elements = malloc(sizeof(int) * symbol->value_dims);
            memcpy(new_symbol->dim_elements, symbol->dim_elements, sizeof(int) * symbol->value_dims);
        }
        if (symbol->value_class) new_symbol->value_class = strdup(symbol->value_class);
    } else {
        /* Merge status and type if the incoming symbol has more info */
        if (new_symbol->status == SYM_STATUS_UNRESOLVED) new_symbol->status = symbol->status;
        if (new_symbol->type == TP_UNKNOWN) {
            new_symbol->type = symbol->type;
            new_symbol->value_dims = symbol->value_dims;
            if (symbol->dim_base && !new_symbol->dim_base) {
                new_symbol->dim_base = malloc(sizeof(int) * symbol->value_dims);
                memcpy(new_symbol->dim_base, symbol->dim_base, sizeof(int) * symbol->value_dims);
            }
            if (symbol->dim_elements && !new_symbol->dim_elements) {
                new_symbol->dim_elements = malloc(sizeof(int) * symbol->value_dims);
                memcpy(new_symbol->dim_elements, symbol->dim_elements, sizeof(int) * symbol->value_dims);
            }
            if (symbol->value_class && !new_symbol->value_class) new_symbol->value_class = strdup(symbol->value_class);
        }
    }

    /* Move all the node/symbol connectors */
    for (i=0; i < ((dpa*)(symbol->ast_node_array))->size; i++) {
        connector = (SymbolNode*)((dpa*)(symbol->ast_node_array))->pointers[i];
        connector->symbol = new_symbol;
        dpa_add((dpa*)(new_symbol->ast_node_array),  connector);
    }

    /* Remove the old symbol from the old scope */
    scp_rmsy(symbol->scope, symbol);

    /* Delete the old symbol */
    free(symbol->name);
    free_dpa(symbol->ast_node_array);
    free(symbol);

    return new_symbol;
}

/* Hoist a symbol to a namespace scope (EXPOSE) */
Symbol *sym_hoist_to_namespace(Symbol *symbol, Scope *target_namespace) {
    if (!symbol || !target_namespace) return symbol;

    /* Ensure target is a namespace or universe */
    while (target_namespace && target_namespace->type != SCOPE_NAMESPACE && target_namespace->type != SCOPE_UNIVERSE) {
        target_namespace = target_namespace->parent;
    }

    if (!target_namespace) return symbol;

    /* Precondition: symbol must not be an argument */
    if (symbol->is_arg) return symbol;

    /* If already in that scope or above, do nothing */
    Scope *s = target_namespace;
    while (s) {
        if (s == symbol->scope) return symbol;
        s = s->parent;
    }

    /* Merge into the target namespace */
    return sym_merg(target_namespace, symbol);
}

/* Duplicate a symbol into a new scope */
Symbol *sym_dup(Scope *new_scope, Symbol *symbol) {
    Symbol *new_symbol;
    if (!symbol) return NULL;
    new_symbol = sym_fn(new_scope, symbol->name, strlen(symbol->name));
    if (!new_symbol) return NULL;

    new_symbol->symbol_type = symbol->symbol_type;
    new_symbol->status = symbol->status;
    new_symbol->type = symbol->type;
    new_symbol->value_dims = symbol->value_dims;
    if (symbol->dim_base) {
        new_symbol->dim_base = malloc(sizeof(int) * symbol->value_dims);
        memcpy(new_symbol->dim_base, symbol->dim_base, sizeof(int) * symbol->value_dims);
    }
    if (symbol->dim_elements) {
        new_symbol->dim_elements = malloc(sizeof(int) * symbol->value_dims);
        memcpy(new_symbol->dim_elements, symbol->dim_elements, sizeof(int) * symbol->value_dims);
    }
    if (symbol->value_class) new_symbol->value_class = strdup(symbol->value_class);
    new_symbol->register_num = symbol->register_num;
    new_symbol->register_type = symbol->register_type;
    new_symbol->fixed_args = symbol->fixed_args;
    new_symbol->has_vargs = symbol->has_vargs;
    new_symbol->exposed = symbol->exposed;
    new_symbol->is_arg = symbol->is_arg;
    new_symbol->is_ref_arg = symbol->is_ref_arg;
    new_symbol->is_opt_arg = symbol->is_opt_arg;
    new_symbol->is_const_arg = symbol->is_const_arg;
    new_symbol->is_main = symbol->is_main;
    new_symbol->is_implicit_main = symbol->is_implicit_main;
    new_symbol->is_rc = symbol->is_rc;
    new_symbol->is_this = symbol->is_this;
    new_symbol->is_factory = symbol->is_factory;
    new_symbol->is_inlinable = symbol->is_inlinable;
    new_symbol->ast_template = symbol->ast_template;

    return new_symbol;
}

static void scp_dup_worker(Symbol *symbol, void *payload) {
    Scope *new_scope = (Scope *)payload;
    sym_dup(new_scope, symbol);
}

/* Duplicate a scope and its symbols into a new parent scope */
Scope *scp_dup(Context *context, Scope *old_scope, Scope *new_parent, ASTNode *new_defining_node) {
    Scope *new_scope;
    if (!old_scope) return NULL;
    new_scope = scp_f(context, new_parent, new_defining_node, NULL, old_scope->type);
    if (old_scope->name) new_scope->name = strdup(old_scope->name);
    
    /* Duplicate symbols */
    scp_4all(old_scope, scp_dup_worker, new_scope);
    
    /* Recursively duplicate child scopes? 
     * In Level B procedures, usually no nested scopes unless we have blocks.
     * scp_4all only does symbols. We need to do child scopes too.
     */
    size_t i;
    for (i = 0; i < scp_noch(old_scope); i++) {
        /* We don't have the new defining nodes for child scopes yet.
         * This suggests scp_dup should be integrated into the AST duplication walker.
         */
    }

    return new_scope;
}

/* Frees a symbol */
void free_sym(Symbol *symbol) {
    size_t i;
    free(symbol->name);
    if (symbol->value_class) free(symbol->value_class);
    if (symbol->dim_base) free(symbol->dim_base);
    if (symbol->dim_elements) free(symbol->dim_elements);

    /* Free SymbolNode Connectors */
    for (i=0; i < ((dpa*)(symbol->ast_node_array))->size; i++) {
        free(((dpa*)(symbol->ast_node_array))->pointers[i]);
    }
    free_dpa(symbol->ast_node_array);

    free(symbol);
}

/* Returns the index'th SymbolNode connector attached to a symbol */
SymbolNode* sym_trnd(Symbol *symbol, size_t index) {
    return (SymbolNode*)((dpa*)(symbol->ast_node_array))->pointers[index];
}

/* Returns the lowest ASTNode ordinal associated with the symbol */
int sym_lord(Symbol *symbol) {

    dpa* array = (dpa*)(symbol->ast_node_array);
    size_t i;
    int o;
    int ord = 0;
    if (array->size) {
        ord = ((SymbolNode *) (array->pointers[0]))->node->low_ordinal;

        for (i = 1; i < array->size; i++) {
            o = ((SymbolNode *) (array->pointers[0]))->node->low_ordinal;
            if (o < ord) ord = o;
        }
    }

    return ord;
}

/* Returns the PROCEDURE, METHOD or FACTORY ASTNode of a Symbol */
ASTNode* sym_proc(Symbol *symbol) {
    size_t i;
    SymbolNode* sn;
    for (i=0; i < sym_nond(symbol); i++) {
        sn = sym_trnd(symbol, i);
        if (sn->node->node_type == PROCEDURE || sn->node->node_type == METHOD || sn->node->node_type == FACTORY) return sn->node;
    }
    return 0;
}

/* Returns 1 if node is linked to symbol, or 0 */
int symislnk(ASTNode *node, Symbol *symbol) {
    size_t i;
    SymbolNode* sn;
    for (i=0; i < sym_nond(symbol); i++) {
        sn = sym_trnd(symbol, i);
        if (sn->node == node) return 1;
    }
    return 0;
}

/* Connect a ASTNode to a Symbol */
void sym_adnd(Symbol *symbol, ASTNode* node, unsigned int readAccess,
              unsigned int writeAccess) {
    SymbolNode *connector;

    if (!symbol || !node) return;

    /* Check if already added */
    if (node->symbolNode && node->symbolNode->symbol == symbol) return;

    /* If it points to a DIFFERENT symbol, disconnect it first */
    if (node->symbolNode) {
        sym_dno(node->symbolNode->symbol, node);
    }

    connector = malloc(sizeof(SymbolNode));
    connector->symbol = symbol;
    connector->node = node;
    connector->readUsage = readAccess;
    connector->writeUsage = writeAccess;

    dpa_add((dpa*)(symbol->ast_node_array), connector);
    node->symbolNode = connector;
}

/* Returns the number of AST nodes connected to a symbol */
size_t sym_nond(Symbol *symbol) {
    return ((dpa*)(symbol->ast_node_array))->size;
}

/* Disconnects a node from a symbol */
void sym_dnd(Symbol *symbol, size_t node_num) {

    /* unlink and free the symbolnode */
    SymbolNode* sn = sym_trnd(symbol, node_num);
    sn->node->symbolNode = 0;
    free(sn);

    /* Remove from array */
    dpa_del((dpa*)(symbol->ast_node_array),node_num);
}

/* Disconnects a node from a symbol */
void sym_dno(Symbol *symbol, ASTNode* node) {
    size_t i;
    SymbolNode* sn;
    for (i=0; i < sym_nond(symbol); i++) {
        sn = sym_trnd(symbol, i);
        if (sn->node == node) {
            sym_dnd(symbol, i);
            return;
        }
    }
}

static void prepend_scope(char* buffer, const char* scope)
{
    size_t len = strlen(scope);
    memmove(buffer + len + 1, buffer, strlen(buffer) + 1);
    memcpy(buffer, scope, len);
    buffer[len] = '.';
}

/* Returns the fully resolved symbol name in a malloced buffer */
char* sym_frnm(Symbol *symbol) {
    Scope *s;
    size_t len;
    char *result;
    int has_namespace = 0;

    /* Calculate buffer len */
    len = strlen(symbol->name) + 1; /* +1 for null */
    s = symbol->scope;
    while (s) {
        if (s->defining_node && s->defining_node->node_type == NAMESPACE) {
            has_namespace = 1;
        }
        if (s->name) {
            /* If it's a file scope */
            if (s->defining_node && (s->defining_node->node_type == PROGRAM_FILE || s->defining_node->node_type == IMPORTED_FILE)) {
                if (!has_namespace) {
                    len += strlen(s->name) + 1;
                }
                break; /* File scope is the top */
            } else {
                len += strlen(s->name) + 1;
            }
        }
        s = s->parent;
    }
    result = malloc(len);

    /* Create name */
    strcpy(result, symbol->name);
    has_namespace = 0;
    s = symbol->scope;
    while (s) {
        if (s->defining_node && s->defining_node->node_type == NAMESPACE) {
            has_namespace = 1;
        }
        if (s->name) {
            /* If it's a file scope */
            if (s->defining_node && (s->defining_node->node_type == PROGRAM_FILE || s->defining_node->node_type == IMPORTED_FILE)) {
                if (!has_namespace) {
                    prepend_scope(result, s->name);
                }
                break; /* File scope is the top */
            } else {
                prepend_scope(result, s->name);
            }
        }
        s = s->parent;
    }
    return result;
}

/* Returns the fully resolved symbol name (mangled) in a malloced buffer */
char* sym_mngd_frnm(Symbol *symbol) {
    char *result = sym_frnm(symbol);

    /* Top level main is not mangled or prefixed */
    if (symbol->is_main && symbol->scope && symbol->scope->parent &&
        symbol->scope->parent->defining_node && symbol->scope->parent->defining_node->node_type == REXX_UNIVERSE)
        return result;

    /* Prefix with "§" to ensure it's a valid RXAS ID and doesn't collide */
    result = realloc(result, strlen(result) + 3);
    memmove(result + 2, result, strlen(result) + 1);
    memcpy(result, "\xc2\xa7", 2);

    return result;
}

/* Returns the fully resolved scope name in a malloced buffer */
char* scp_frnm(Scope *scope) {
    Scope *s;
    size_t len;
    char *result;

    /* Calculate buffer len */
    len = (scope->name ? strlen(scope->name) : 0) + 1; /* +1 for null */
    s = scope->parent;
    while (s) {
        if (s->name) {
            len += strlen(s->name) + 1; /* +1 for the "." */
        }
        s = s->parent;
    }
    result = malloc(len);

    /* Create name */
    strcpy(result, scope->name ? scope->name : "");
    s = scope->parent;
    while (s) {
        if (s->name) {
            prepend_scope(result, s->name);
        }
        s = s->parent;
    }
    return result;
}

/* Returns the fully resolved node name in a malloced buffer */
char* ast_frnm(ASTNode *node) {
    Scope *s;
    size_t len;
    char *result;

    /* Calculate buffer len */
    len = (node->node_string ? node->node_string_length : 0) + 1; /* +1 for null */
    s = node->scope;
    while (s) {
        if (s->name) {
            len += strlen(s->name) + 1; /* +1 for the "." */
        }
        s = s->parent;
    }
    result = malloc(len);

    /* Create name */
    if (node->node_string) {
        memcpy(result, node->node_string, node->node_string_length);
        result[node->node_string_length] = 0;
    } else {
        result[0] = 0;
    }
    s = node->scope;
    while (s) {
        if (s->name) {
            prepend_scope(result, s->name);
        }
        s = s->parent;
    }
    return result;
}
