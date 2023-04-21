/*
 * Symbol and Scope Infrastructure
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
Scope *scp_f(Scope *parent, ASTNode *node, Symbol* symbol) {
    char* name;
    Scope *scope = (Scope*) malloc(sizeof(Scope));
    scope->defining_node = node;
    if (symbol) {
        name = symbol->name;
        scope->name = malloc(strlen(name) + 1);
        strcpy(scope->name, name);
        /* Update Symbol */
        symbol->defines_scope = scope;
    }
    else scope->name = 0;
    if (node) node->scope = scope;
    scope->parent = parent;
    scope->symbols_tree = 0;
    scope->num_registers = 1; /* r0 is always available as a temp register - TODO get rid of this! */
    scope->free_registers_array = dpa_f();
    scope->child_array  = dpa_f();
    if (scope->parent) dpa_add((dpa*)(parent->child_array), scope);

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

    free_array = (dpa*)(scope->free_registers_array);
//    printf("get a reg");
//    printf(" - free array is ");
//    {int ii; for (ii=0; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);printf("\n");}

    /* Check the free list */
    if (free_array->size) {
        free_array->size--;
        reg = (int)(size_t)(free_array->pointers[free_array->size]);
    }
    else {
        reg = (int)((scope->num_registers)++);
    }

//    printf("  returned %d", reg);
//    printf(" - free array is now ");
//    {int ii; for (ii=0; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);}
//    printf("\n");

    return reg;
}

/* Return a no longer used register to the scope */
void ret_reg(Scope *scope, int reg) {
    size_t i;
    dpa *free_array;
    free_array = (dpa*)(scope->free_registers_array);

    if (reg < 0) {
        return;
    }

//    printf("free %d", reg);

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

/* Get number of free register from scope - returns the start of a sequence
 * n, n+1, n+2, ... n+number */
int get_regs(Scope *scope, size_t number) {
    dpa *free_array;
    int reg, r, top, i;
    size_t seq;

    if (number == 1) return get_reg(scope);

    free_array = (dpa*)(scope->free_registers_array);

//    printf("get %d regs - free array is ", (int)number);
//    {int ii; for (ii=0; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);printf("\n");}

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
        if (r == (int)(scope->num_registers)) {
            /* Yes we can because the next unused register adds to the sequence */
            reg = top; /* Result is the beginning of the sequence */
            /* Now remove them from the free list */
            free_array->size -= seq;
            /* Now assign some brand ne ones */
            scope->num_registers += number - seq;
//            printf("  b-returned %d-%d - free array is now ", reg, reg+(int)number - 1);
//            {int ii; for (ii=0; ii<free_array->size; ii++) printf("%d ",(int)(size_t)free_array->pointers[ii]);printf("\n");}
            return reg;
        }
        /* No we can't so just assign new ones */
    }

    reg = (int)(scope->num_registers); /* Assign brand-new registers */
    scope->num_registers += number;
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
        case TP_STRING: return ".string";
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

    if (symbol->value_class) buffer = symbol->value_class;
    else buffer = type_nm(symbol->type);

    array = ast_astr(symbol->value_dims, symbol->dim_base, symbol->dim_elements);

    result = malloc(strlen(buffer) + strlen(array) + 1);
    strcpy(result, buffer);
    strcat(result, array);

    free(array);

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

    symbol->scope = scope;
    symbol->defines_scope = 0;
    symbol->type = TP_UNKNOWN;
    symbol->value_dims = 0;
    symbol->dim_base = 0;
    symbol->dim_elements = 0;
    symbol->value_class = 0;
    symbol->register_num = -1;
    symbol->name = (char*)malloc(name_length + 1);
    memcpy(symbol->name, name, name_length);
    symbol->name[name_length] = 0;
    symbol->register_type = 'r';
    symbol->symbol_type = VARIABLE_SYMBOL;
    symbol->meta_emitted = 0;
    symbol->exposed = 0;

    /* Lowercase symbol name */
#ifdef NUTF8
    for (c = symbol->name ; *c; ++c) *c = (char)tolower(*c);
#else
    utf8lwr(symbol->name);
#endif
    symbol->ast_node_array = dpa_f();

    /* Returns 1 on duplicate */
    if (add_symbol_to_tree((struct avl_tree_node **)&(scope->symbols_tree),
                           symbol)) {
        free_sym(symbol);
        return NULL;
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

    /* Process top layer - files */
    for (i = 0; i < scp_noch(root->scope); i++) {
        s = scp_chd(root->scope, i);

        result = src_symbol((struct avl_tree_node *)(s->symbols_tree), name);
        if (result) {
            return result;
        }
        /* TODO Process second level - Classes */
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
   Scope *scope = root->scope;
   char* search_name;
   size_t len;

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
               name = c + 1;
               break;
           }
           else if (!*c) {
               /* Search for final symbol */
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
    Scope *scope = root->scope;
    char* search_name;
    size_t len;

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
                    scope = scp_f(scope, 0, result);
                    result->symbol_type = NAMESPACE_SYMBOL;
                    result->defines_scope = scope;
                    free(search_name);
                }
                else if (!result->defines_scope) {
                    free(search_name);
                    return 0;
                }
                scope = result->defines_scope;
                name = c + 1;
                free(search_name);
                break;
            }
            else if (!*c) {
                /* Search for final symbol */
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

/* Resolve a Function Symbol
 * the root parameter should the AST root - the function checks the root of all the PROGRAM_FILE and IMPORTED_FILE
 */
Symbol *sym_rvfc(ASTNode *root, ASTNode *node) {
    Symbol *result;

    /* Make a null terminated string */
    char *name = (char*)malloc(node->node_string_length + 1);
    memcpy(name, node->node_string, node->node_string_length);
    name[node->node_string_length] = 0;

    /* Lowercase symbol name */
#ifdef NUTF8
    for (c = name ; *c; ++c) *c = (char)tolower(*c);
#else
    utf8lwr(name);
#endif

    result = sym_rvfn(root, name);
    free(name);
    return result;
}

/* Resolve a Symbol - including parent scope */
Symbol *sym_rslv(Scope *scope, ASTNode *node) {
    Symbol *result;
    char *c;
    /* Sadly we are making a null terminated string */
    char *name = (char*)malloc(node->node_string_length + 1);
    memcpy(name, node->node_string, node->node_string_length);
    name[node->node_string_length] = 0;

    /* Lowercase symbol name */
#ifdef NUTF8
    for (c = name ; *c; ++c) *c = (char)tolower(*c);
#else
    utf8lwr(name);
#endif

    /* Look for the symbol - looking up in each parent scope */
    do {
        result = src_symbol((struct avl_tree_node *)(scope->symbols_tree), name);
        if (result) {
            free(name);
            return result;
        }
        scope = scope->parent;
    } while (scope);
    free(name);
    return 0;
}

/* Local Resolve a Symbol - current scope only */
Symbol *sym_lrsv(Scope *scope, ASTNode *node) {
    Symbol *result = 0;
    char *c;
    /* Sadly we are making a null terminated string */
    char *name = (char*)malloc(node->node_string_length + 1);
    memcpy(name, node->node_string, node->node_string_length);
    name[node->node_string_length] = 0;

    /* Uppercase symbol name */
#ifdef NUTF8
    for (c = name ; *c; ++c) *c = (char)tolower(*c);
#else
    utf8lwr(name);
#endif

    result = src_symbol((struct avl_tree_node *)(scope->symbols_tree), name);
    free(name);
    return result;
}

/* Move (via a merge) a Symbol into a new Scope - returns the target symbol */
Symbol *sym_merg(Scope *new_scope, Symbol *symbol) {
    size_t i;
    SymbolNode *connector;

    /* Find or create symbol in new_scope */
    Symbol *new_symbol = src_symbol((struct avl_tree_node *)(new_scope->symbols_tree), symbol->name);
    if (!new_symbol) {
        new_symbol = sym_fn(new_scope, symbol->name, strlen(symbol->name));
        new_symbol->symbol_type = symbol->symbol_type;
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
    int ord = ((SymbolNode*)(array->pointers[0]))->node->low_ordinal;

    for (i = 1; i < array->size; i++) {
        o = ((SymbolNode*)(array->pointers[0]))->node->low_ordinal;
        if (o < ord) ord = o;
    }

    return ord;
}

/* Returns the PROCEDURE ASTNode of a Symbol */
ASTNode* sym_proc(Symbol *symbol) {
    size_t i;
    SymbolNode* sn;
    for (i=0; i < sym_nond(symbol); i++) {
        sn = sym_trnd(symbol, i);
        if (sn->node->node_type == PROCEDURE) return sn->node;
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
    SymbolNode *connector = malloc(sizeof(SymbolNode));
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

    /* Calculate buffer len */
    len = strlen(symbol->name) + 1; /* +1 for null */
    s = symbol->scope;
    while (s) {
        if (s->name) {
            len += strlen(s->name) + 1; /* +1 for the "." */
        }
        s = s->parent;
    }
    result = malloc(len);

    /* Create name */
    strcpy(result, symbol->name);
    s = symbol->scope;
    while (s) {
        if (s->name) {
            prepend_scope(result, s->name);
        }
        s = s->parent;
    }
    return result;
}

/* Returns the fully resolved scope name in a malloced buffer */
char* scp_frnm(Scope *scope) {
    Scope *s;
    size_t len;
    char *result;

    /* Calculate buffer len */
    len = strlen(scope->name) + 1; /* +1 for null */
    s = scope->parent;
    while (s) {
        if (s->name) {
            len += strlen(s->name) + 1; /* +1 for the "." */
        }
        s = s->parent;
    }
    result = malloc(len);

    /* Create name */
    strcpy(result, scope->name);
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
    len = node->node_string_length + 1; /* +1 for null */
    s = node->scope;
    while (s) {
        if (s->name) {
            len += strlen(s->name) + 1; /* +1 for the "." */
        }
        s = s->parent;
    }
    result = malloc(len);

    /* Create name */
    memcpy(result, node->node_string, node->node_string_length);
    result[node->node_string_length] = 0;
    s = node->scope;
    while (s) {
        if (s->name) {
            prepend_scope(result, s->name);
        }
        s = s->parent;
    }
    return result;
}
