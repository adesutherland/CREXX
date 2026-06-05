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
 * Built-in and External Function Management
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/stat.h>
#include "avl_tree.h"
#include "platform.h"
#include "rxcpmain.h"
#include "rxcp_emit.h"
#include "rxcp_source_ext.h"
#include "rxbin.h"
#include "rxas.h"
#include "rxpa.h"
#include "rxcp_val.h"
#ifndef NUTF8
#include "utf.h"
#endif


/* Internal Tree node structure for functions */
struct tree_wrapper {
    imported_func *func;
    struct avl_tree_node index_node;
};

/* Internal Tree node structure for classes */
struct class_tree_wrapper {
    struct imported_class *cls;
    struct avl_tree_node index_node;
};

typedef struct {
    Context *parent_context;
    Context *import_context;
} class_import_payload;

static int load_another_file(Context *context);
static size_t module_stem_length(const char *name);
static void mark_source_import_interface_default_methods(Context *stub_ctx, ASTNode *contract_node);

#define GET_INDEX(i) avl_tree_entry((i), struct tree_wrapper, index_node)->func->fqname
#define GET_VALUE(i) avl_tree_entry((i), struct tree_wrapper, index_node)->func

#define GET_CLASS_INDEX(i) avl_tree_entry((i), struct class_tree_wrapper, index_node)->cls->fqname
#define GET_CLASS_VALUE(i) avl_tree_entry((i), struct class_tree_wrapper, index_node)->cls

static int compare_node_node(const struct avl_tree_node *node1,
                             const struct avl_tree_node *node2)
{
    char* n1 = GET_INDEX(node1);
    char* n2 = GET_INDEX(node2);
    return strcmp(n1,n2);
}

static int compare_class_node_node(const struct avl_tree_node *node1,
                                   const struct avl_tree_node *node2)
{
    char* n1 = GET_CLASS_INDEX(node1);
    char* n2 = GET_CLASS_INDEX(node2);
    return strcmp(n1,n2);
}

static int compare_node_value(const void *value,
                              const struct avl_tree_node *nodeptr) {
    char* node = GET_INDEX(nodeptr);
    return strcmp((char*)value,node);
}

static int compare_class_node_value(const void *value,
                                    const struct avl_tree_node *nodeptr) {
    char* node = GET_CLASS_INDEX(nodeptr);
    return strcmp((char*)value,node);
}

// Search for a function / variable from fqname in the master context
// Returns 1 if found and sets value
// Returns 0 if not found
static int src_func(Context *context, char* fqname, imported_func **func) {
    struct avl_tree_node *result;
    struct avl_tree_node *root = context->master_context->importable_function_tree;

    result = avl_tree_lookup(root, fqname, compare_node_value);

    if (result) {
        *func = GET_VALUE(result);
        return 1;
    }
    else return 0;
}

// Search for a function / variable from namespace and name in the master context
// Returns 1 if found and sets value
// Returns 0 if not found
static int src_nsfu(Context *context, char* namespace, char* name, imported_func **func) {
    struct avl_tree_node *result;
    struct avl_tree_node *root;
    char *fqname;

    if (!context || !context->master_context) return 0;
    root = context->master_context->importable_function_tree;
    if (!root) return 0;

    if (!namespace || !name) return 0;

    fqname = malloc(strlen(namespace) + strlen(name) + 2);
    strcpy(fqname,namespace);
    strcat(fqname, ".");
    strcat(fqname, name);

    result = avl_tree_lookup(root, fqname, compare_node_value);

    if (result) {
        *func = GET_VALUE(result);
        free(fqname);
        return 1;
    }

    free(fqname);
    return 0;
}

// Search for a class from fqname in the master context
// Returns 1 if found and sets value
// Returns 0 if not found
static int src_class(Context *context, char* fqname, struct imported_class **cls) {
    struct avl_tree_node *result;
    struct avl_tree_node *root = context->master_context->importable_class_tree;

    if (!root) return 0;
    result = avl_tree_lookup(root, fqname, compare_class_node_value);

    if (result) {
        *cls = GET_CLASS_VALUE(result);
        return 1;
    }
    else return 0;
}

static int imported_class_metadata_implements(Context *context, const char *class_fqname, const char *interface_fqname) {
    struct imported_class *cls = 0;
    size_t i;

    if (!context || !context->master_context || !class_fqname || !interface_fqname) return 0;
    if (!src_class(context, (char *)class_fqname, &cls) || !cls) return 0;

    for (i = 0; i < cls->implements_count; i++) {
        if (cls->implements_fqnames[i] && strcmp(cls->implements_fqnames[i], interface_fqname) == 0) {
            return 1;
        }
    }

    return 0;
}

// Search for a class from namespace and name in the master context
// Returns 1 if found and sets value
// Returns 0 if not found
static int src_nscl(Context *context, char* namespace, char* name, struct imported_class **cls) {
    struct avl_tree_node *result;
    struct avl_tree_node *root;
    char *fqname;

    if (!context || !context->master_context) return 0;
    root = context->master_context->importable_class_tree;
    if (!root) return 0;

    if (!namespace || !name) return 0;

    fqname = malloc(strlen(namespace) + strlen(name) + 2);
    strcpy(fqname,namespace);
    strcat(fqname, ".");
    strcat(fqname, name);

    result = avl_tree_lookup(root, fqname, compare_class_node_value);

    if (result) {
        *cls = GET_CLASS_VALUE(result);
        free(fqname);
        return 1;
    }

    free(fqname);
    return 0;
}

static Scope *find_visible_namespace_scope(Context *context, int only_namespace, const char *namespace_name) {
    size_t i;
    Scope *scope;

    if (!context || !context->ast || !context->ast->scope || !namespace_name || !*namespace_name) return 0;
    scope = context->ast->scope;

    if (only_namespace) {
        Scope *module_scope = scp_noch(scope) > 0 ? scp_chd(scope, 0) : 0;
        if (module_scope && module_scope->name && strcmp(module_scope->name, namespace_name) == 0) {
            return module_scope;
        }
        return 0;
    }

    for (i = 0; i < scp_noch(scope); i++) {
        Scope *candidate = scp_chd(scope, i);
        if (candidate && candidate->name && strcmp(candidate->name, namespace_name) == 0) {
            return candidate;
        }
    }

    return 0;
}

static Symbol *lookup_loaded_symbol(Context *context, const char *name) {
    if (!context || !context->ast || !name || !*name) return 0;
    if (strchr(name, '.')) return sym_rfqv(context->ast, name);
    return sym_rvfn(context->ast, (char *)name);
}

// Search for a class from name (checking for imported namespaces in the context)
// Returns 1 if found and sets value
// Returns 0 if not found
static int src_fqcl(Context *context, char* name, struct imported_class **cls) {
    char *namespace;
    size_t i;
    Scope *scope;

    if (!context || !context->ast || !context->ast->scope) return 0;
    scope = context->ast->scope;

    if (rxcp_split_internal_symbol_name(name, &namespace, 0)) {
        int found = 0;
        found = src_class(context, name, cls);
        free(namespace);
        return found;
    }

    if (context->debug_mode >= 2) fprintf(stderr, "Searching for Class %s in namespaces (context file: %s):\n", name, context->file_name);

    /* Check the module namespace */
    if (scp_noch(scope) > 0) {
        Scope *s0 = scp_chd(scope, 0);
        if (s0) {
            namespace = s0->name;
            if (context->debug_mode >= 2) fprintf(stderr, " - namespace: %s\n", namespace);
            if (src_nscl(context, namespace, name, cls)) return 1;
        }
    }

    /* Check imported namespaces */
    for (i = 1; i < scp_noch(scope); i++) {
        Scope *si = scp_chd(scope, i);
        if (si) {
            namespace = si->name;
            if (context->debug_mode >= 2) fprintf(stderr, " - namespace: %s\n", namespace);
            if (src_nscl(context, namespace, name, cls)) return 1;
        }
    }

    return 0;
}

int sym_is_interface_symbol(Symbol *symbol) {
    if (!symbol || symbol->symbol_type != CLASS_SYMBOL || !symbol->defines_scope || !symbol->defines_scope->defining_node) {
        return 0;
    }
    return symbol->defines_scope->defining_node->node_type == INTERFACE_DEF;
}

int sym_is_class_contract_symbol(Symbol *symbol) {
    if (!symbol || symbol->symbol_type != CLASS_SYMBOL || !symbol->defines_scope || !symbol->defines_scope->defining_node) {
        return 0;
    }
    return symbol->defines_scope->defining_node->node_type == CLASS_DEF;
}

static Symbol *resolve_contract_symbol(Context *context, const char *name) {
    Symbol *symbol;

    if (!context || !name || !*name) return 0;

    symbol = lookup_loaded_symbol(context, name);
    if (symbol && symbol->symbol_type == CLASS_SYMBOL) return symbol;

    ensure_class_imported(context, name, strlen(name));
    symbol = lookup_loaded_symbol(context, name);
    if (symbol && symbol->symbol_type == CLASS_SYMBOL) return symbol;

    return 0;
}

static int loaded_class_implements_interface(Context *context, Symbol *class_symbol, Symbol *interface_symbol) {
    ASTNode *class_node;
    ASTNode *implements_node;
    ASTNode *iface_ref;
    char *class_fqname = 0;
    char *interface_fqname = 0;
    int matched = 0;

    if (!context || !sym_is_class_contract_symbol(class_symbol) || !sym_is_interface_symbol(interface_symbol)) {
        return 0;
    }

    class_node = class_symbol->defines_scope->defining_node;
    if (!class_node) return 0;

    class_fqname = sym_frnm(class_symbol);
    interface_fqname = sym_frnm(interface_symbol);
    if (class_fqname && interface_fqname &&
        imported_class_metadata_implements(context, class_fqname, interface_fqname)) {
        matched = 1;
        goto done;
    }

    implements_node = ast_chld(class_node, IMPLEMENTS, 0);
    if (!implements_node) goto done;

    iface_ref = implements_node->child;
    while (iface_ref) {
        Symbol *implemented_symbol = sym_rvfc(context->ast, iface_ref);
        if (!implemented_symbol) {
            ensure_class_imported(context, iface_ref->node_string, iface_ref->node_string_length);
            implemented_symbol = sym_rvfc(context->ast, iface_ref);
        }

        if (implemented_symbol == interface_symbol) {
            matched = 1;
            break;
        }

        if (implemented_symbol && interface_fqname) {
            char *implemented_fqname = sym_frnm(implemented_symbol);
            if (implemented_fqname && strcmp(implemented_fqname, interface_fqname) == 0) {
                matched = 1;
            }
            if (implemented_fqname) free(implemented_fqname);
            if (matched) break;
        }

        iface_ref = iface_ref->sibling;
    }

done:
    if (class_fqname) free(class_fqname);
    if (interface_fqname) free(interface_fqname);
    return matched;
}

int symbol_name_assignable_to(Context *context, const char *from_name, const char *to_name) {
    Symbol *from_symbol;
    Symbol *to_symbol;

    if (!from_name || !to_name) return 0;
    if (symbol_names_equivalent(context, from_name, to_name)) return 1;
    if (!context) return 0;
    if (imported_class_metadata_implements(context, from_name, to_name)) return 1;

    from_symbol = resolve_contract_symbol(context, from_name);
    to_symbol = resolve_contract_symbol(context, to_name);

    if (!from_symbol || !to_symbol) return 0;
    if (sym_is_class_contract_symbol(from_symbol) && sym_is_interface_symbol(to_symbol)) {
        return loaded_class_implements_interface(context, from_symbol, to_symbol);
    }

    return 0;
}

int symbol_names_equivalent(Context *context, const char *left_name, const char *right_name) {
    Symbol *left_symbol;
    Symbol *right_symbol;

    if (!left_name || !right_name) return left_name == 0 && right_name == 0;
    if (strcmp(left_name, right_name) == 0) return 1;
    if (!context) return 0;

    left_symbol = resolve_contract_symbol(context, left_name);
    right_symbol = resolve_contract_symbol(context, right_name);
    return left_symbol && right_symbol && left_symbol == right_symbol;
}

// Search for a function / variable from name (checking for imported namespaces in the context)
// if only_namespace is set then the search only covers the file namespace not imported ones
// (this is for global variables)
// Returns 1 if found and sets value
// Returns 0 if not found
int src_fqfu(Context *context, int only_namespace, char* name, imported_func **func) {
    char *namespace;
    size_t i;
    Scope *scope;

    if (!context || !context->ast || !context->ast->scope) return 0;
    scope = context->ast->scope;

    if (rxcp_split_internal_symbol_name(name, &namespace, 0)) {
        int found = 0;
        char *short_name = 0;
        rxcp_split_internal_symbol_name(name, 0, &short_name);
        if (short_name && find_visible_namespace_scope(context, only_namespace, namespace)) {
            found = src_nsfu(context, namespace, short_name, func);
        }
        free(namespace);
        if (short_name) free(short_name);
        return found;
    }

    if (context->debug_mode >= 2) fprintf(stderr, "Searching for Procedure %s in namespaces (context file: %s):\n", name, context->file_name);

    /* Check the module namespace */
    if (scp_noch(scope) > 0) {
        Scope *s0 = scp_chd(scope, 0);
        if (s0) {
            namespace = s0->name;
            if (context->debug_mode >= 2) fprintf(stderr, " - namespace: %s\n", namespace);
            if (src_nsfu(context, namespace, name, func)) return 1;
        }
    }

    /* Check imported namespaces */
    if (!only_namespace) {
        for (i = 1; i < scp_noch(scope); i++) {
            Scope *si = scp_chd(scope, i);
            if (si) {
                namespace = si->name;
                if (context->debug_mode >= 2) fprintf(stderr, " - namespace: %s\n", namespace);
                if (src_nsfu(context, namespace, name, func)) return 1;
            }
        }
    }

    return 0;
}

/* Add a [inconsistent] duplicate symbol to the end of the symbols duplicate list */
static void add_inconsistent_duplicate(imported_func *first, imported_func *duplicate) {
    /* Loop to the end of the list */
    while (first->duplicate) first=first->duplicate;

    /* Add the duplicate */
    first->duplicate = duplicate;
}

/* strcmp() that handles nulls */
static int safe_strcmp(const char *s1, const char* s2) {
    if (s1==0 && s2==0) return 0;
    if (s1==0) return -1;
    if (s2==0) return 1;
    return strcmp(s1,s2);
}

static int append_arg_text(char **buffer, size_t *capacity, size_t *length, const char *text, size_t text_len) {
    char *tmp;

    if (*length + text_len + 1 >= *capacity) {
        while (*length + text_len + 1 >= *capacity) *capacity = *capacity * 2 + text_len + 16;
        tmp = realloc(*buffer, *capacity);
        if (!tmp) {
            free(*buffer);
            *buffer = 0;
            return 0;
        }
        *buffer = tmp;
    }

    memcpy(*buffer + *length, text, text_len);
    *length += text_len;
    (*buffer)[*length] = 0;
    return 1;
}

static const char *default_source_for_metadata_type(const char *type_start, const char *type_end) {
    size_t len;

    while (type_start < type_end && isspace((unsigned char)*type_start)) type_start++;
    while (type_end > type_start && isspace((unsigned char)*(type_end - 1))) type_end--;
    len = (size_t)(type_end - type_start);

    if (len == 7 && memcmp(type_start, ".string", 7) == 0) return "\"\"";
    if (len == 4 && memcmp(type_start, ".int", 4) == 0) return "0";
    if (len == 6 && memcmp(type_start, ".float", 6) == 0) return "0";
    if (len == 5 && memcmp(type_start, ".bool", 5) == 0) return "0";
    return 0;
}

static char *metadata_args_to_source_args(const char *args) {
    const char *p;
    char *buffer;
    size_t capacity;
    size_t length;

    if (!args || !*args) return strdup("");

    capacity = strlen(args) + 32;
    buffer = malloc(capacity);
    if (!buffer) return 0;
    buffer[0] = 0;
    length = 0;
    p = args;

    while (*p) {
        const char *segment_start;
        const char *segment_end;
        const char *equals;
        int optional = 0;

        while (*p && isspace((unsigned char)*p)) p++;
        segment_start = p;
        while (*p && *p != ',') p++;
        segment_end = p;
        while (segment_end > segment_start && isspace((unsigned char)*(segment_end - 1))) segment_end--;

        if (length > 0 && !append_arg_text(&buffer, &capacity, &length, ", ", 2)) return 0;

        if ((size_t)(segment_end - segment_start) >= 7 &&
            strncasecmp(segment_start, "expose ", 7) == 0) {
            if (!append_arg_text(&buffer, &capacity, &length, "expose ", 7)) return 0;
            segment_start += 7;
            while (segment_start < segment_end && isspace((unsigned char)*segment_start)) segment_start++;
        }

        /*
         * RXAS metadata marks optional args with a leading '?'. Convert it to
         * valid Rexx source while preserving reference (`expose`) arguments.
         * Optional/default and `expose` are independent: this synthetic default
         * is only for the temporary import stub because RXAS metadata does not
         * currently carry the original source default expression.
         */
        if (segment_start < segment_end && *segment_start == '?') {
            optional = 1;
            segment_start++;
            while (segment_start < segment_end && isspace((unsigned char)*segment_start)) segment_start++;
        }

        equals = memchr(segment_start, '=', (size_t)(segment_end - segment_start));
        if (optional && equals) {
            const char *default_value = default_source_for_metadata_type(equals + 1, segment_end);
            if (default_value) {
                if (!append_arg_text(&buffer, &capacity, &length,
                                     segment_start, (size_t)(equals - segment_start + 1))) return 0;
                if (!append_arg_text(&buffer, &capacity, &length, default_value, strlen(default_value))) return 0;
            } else {
                if (!append_arg_text(&buffer, &capacity, &length,
                                     segment_start, (size_t)(segment_end - segment_start))) return 0;
            }
        } else {
            if (!append_arg_text(&buffer, &capacity, &length,
                                 segment_start, (size_t)(segment_end - segment_start))) return 0;
        }

        if (*p == ',') p++;
    }

    return buffer;
}

/* Adds a func / variable to the master context*/
/* Returns 0 on success, 1 on duplicate
 * If it is a duplicate this function either calls freimpfc(func) or stashes it in the duplicate list
 * (the caller does not need to worry (should not worry) about freeing it if it is a duplicate) */
static int add_func(Context *context, imported_func *func) {
    if (context->debug_mode >= 2) fprintf(stderr, "Importing Procedure %s from %s (into context file: %s)\n", func->fqname, func->file_name, context->file_name);
    struct avl_tree_node **root = (struct avl_tree_node **)&(context->master_context->importable_function_tree);
    imported_func *existing_func;
    /* Does the function already exist? */
    if ( src_func(context, func->fqname, &existing_func) ) {
        /* Yes a duplicate - so check it is consistent */

        /* Both should be a variable or a function */
        if (func->is_variable != existing_func->is_variable) {
            add_inconsistent_duplicate(existing_func, func);
            return 1;
        }

        /* Both should have the same type */
        if (safe_strcmp(func->type, existing_func->type) != 0) {
            add_inconsistent_duplicate(existing_func, func);
            return 1;
        }

        /* If a function both should have the same arguments */
        if (func->is_variable!=0 && safe_strcmp(func->args, existing_func->args) != 0) {
            add_inconsistent_duplicate(existing_func, func);
            return 1;
        }

        /* Otherwise, it is just a consistent duplicate - not an error */
        /* We are responsible for freeing it */
        freimpfc(func);
        return 1;
    }

    struct tree_wrapper *i = malloc(sizeof(struct tree_wrapper));
    i->func = func;
    if (avl_tree_insert(root, &i->index_node, compare_node_node)) {
        /* Duplicate */
        fprintf(stderr, "Internal Error: Unexpected duplicate symbol\n");
        free(i);
        /* We are responsible for freeing it */
        freimpfc(func);
        return 1;
    }
    if (func->is_variable) context->master_context->importable_variable_count++;
    else context->master_context->importable_function_count++;
    return 0;
}

static int imported_contract_member_count(Context *ctx) {
    ASTNode *program;
    ASTNode *contract;
    ASTNode *member;
    int count = 0;

    if (!ctx || !ctx->ast) return 0;
    program = ctx->ast->child;
    if (!program) return 0;
    contract = program->child;
    while (contract && contract->node_type == REXX_OPTIONS) contract = contract->sibling;
    if (!contract) return 0;

    for (member = contract->child; member; member = member->sibling) {
        if (member->node_type == METHOD || member->node_type == FACTORY) count++;
    }

    return count;
}

static void free_imported_class_payload(struct imported_class *cls, int free_identity) {
    if (!cls) return;
    if (free_identity) {
        free(cls->name);
        free(cls->fqname);
        free(cls->namespace);
    }
    free(cls->file_name);
    if (cls->implements_fqnames) {
        size_t k;
        for (k = 0; k < cls->implements_count; k++) free(cls->implements_fqnames[k]);
        free(cls->implements_fqnames);
    }
    if (cls->context) fre_cntx(cls->context);
}

/* Adds a class to the master context*/
/* Returns 0 on success, 1 on duplicate */
static int add_class(Context *context, struct imported_class *cls) {

    if (context->debug_mode >= 2) fprintf(stderr, "Importing Class %s from %s (into context file: %s)\n", cls->fqname, cls->file_name, context->file_name);
    struct avl_tree_node **root = (struct avl_tree_node **)&(context->master_context->importable_class_tree);
    struct imported_class *existing_cls;
    /* Does the class already exist? */
    if ( src_class(context, cls->fqname, &existing_cls) ) {
        if (existing_cls &&
            imported_contract_member_count(existing_cls->context) < imported_contract_member_count(cls->context)) {
            free(existing_cls->file_name);
            if (existing_cls->implements_fqnames) {
                size_t k;
                for (k = 0; k < existing_cls->implements_count; k++) free(existing_cls->implements_fqnames[k]);
                free(existing_cls->implements_fqnames);
            }
            if (existing_cls->context) fre_cntx(existing_cls->context);

            existing_cls->file_name = cls->file_name;
            existing_cls->context = cls->context;
            existing_cls->contract_type = cls->contract_type;
            existing_cls->implements_fqnames = cls->implements_fqnames;
            existing_cls->implements_count = cls->implements_count;

            free(cls->name);
            free(cls->fqname);
            free(cls->namespace);
            free(cls);
            return 1;
        }

        /* Yes a duplicate - we don't care if it's consistent for now, just free the new one */
        free_imported_class_payload(cls, 1);
        free(cls);
        return 1;
    }

    struct class_tree_wrapper *i = malloc(sizeof(struct class_tree_wrapper));
    i->cls = cls;
    if (avl_tree_insert(root, &i->index_node, compare_class_node_node)) {
        /* Duplicate */
        fprintf(stderr, "Internal Error: Unexpected duplicate class symbol\n");
        free(i);
        free_imported_class_payload(cls, 1);
        free(cls);
        return 1;
    }
    context->master_context->importable_class_count++;
    return 0;
}

/* Free Class Tree and classes */
void fre_ctre(Context *context) {
    struct class_tree_wrapper *i;
    struct avl_tree_node **root = (struct avl_tree_node **)&(context->importable_class_tree);

    if (!root || !*root) return;

    /* This walks the tree in post order which allows each node be freed */
    avl_tree_for_each_in_postorder(i, *root, struct class_tree_wrapper, index_node) {
        free_imported_class_payload(i->cls, 1);
        free(i->cls);
        free(i);
    }
    *root = 0;
}

/* imported_class factory */
static struct imported_class *rximpcl_f(Context* context, char* file_name, char *fqname, Context *stub_ctx,
                                        NodeType contract_type, char **implements_fqnames, size_t implements_count) {
    struct imported_class *cls;
    char *dot;

    if (!context || !fqname || !file_name) return 0;

    cls = malloc(sizeof(struct imported_class));
    cls->file_name = strdup(file_name);
    cls->fqname = strdup(fqname);
    cls->context = stub_ctx;
    if (stub_ctx) {
        stub_ctx->file_name = cls->file_name;
        ast_set_file_name(stub_ctx, cls->file_name);
    }
    cls->contract_node = 0;
    cls->contract_type = contract_type;
    cls->implements_fqnames = implements_fqnames;
    cls->implements_count = implements_count;

    dot = strrchr(cls->fqname, '.');
    if (dot) {
        cls->namespace = malloc(dot - cls->fqname + 1);
        memcpy(cls->namespace, cls->fqname, dot - cls->fqname);
        cls->namespace[dot - cls->fqname] = 0;
        cls->name = strdup(dot + 1);
    } else {
        cls->namespace = strdup("");
        cls->name = strdup(cls->fqname);
    }

    if (add_class(context, cls)) {
        /* Duplicate - add_class already freed it */
        return 0;
    }

    return cls;
}

static void attach_imported_member_inline_payloads(Context *context, ASTNode *node, int include_factories) {
    ASTNode *child;

    if (!context || !node) return;

    if ((node->node_type == METHOD || (include_factories && node->node_type == FACTORY)) &&
        node->symbolNode &&
        node->symbolNode->symbol) {
        char *fqname = sym_frnm(node->symbolNode->symbol);
        imported_func *func = 0;

        if (fqname &&
            src_fqfu(context, 0, fqname, &func) &&
            func &&
            rxcp_inline_payload_is_supported(func->implementation)) {
            rxcp_inline_attach_imported_symbol(context, node->symbolNode->symbol, func->implementation);
        }
        if (fqname) free(fqname);
    }

    child = node->child;
    while (child) {
        attach_imported_member_inline_payloads(context, child, include_factories);
        child = child->sibling;
    }
}

static int imported_class_has_source_contract(struct imported_class *cls) {
    const char *extension;

    if (!cls || !cls->file_name) return 0;

    extension = filenext(cls->file_name);
    return extension && extension[0] && !rxcp_source_extension_is_reserved(extension);
}

static Symbol *load_imported_contract(Context *context, struct imported_class *found_cls) {
    Symbol *found_symbol = 0;

    if (!context || !found_cls || !found_cls->context || !found_cls->context->ast || !found_cls->context->ast->child) {
        return 0;
    }

    found_symbol = sym_rfqv(context->ast, found_cls->fqname);
    if (found_symbol) return found_symbol;

    ASTNode *new_stub = add_dast(context->ast, found_cls->context->ast->child);
    context->changed_flags |= FLAG_FUNC;

    {
        Scope *old_scope = context->current_scope;
        context->current_scope = context->ast->scope;
        ast_wlkr(new_stub, build_symbols_walker, context);
        context->current_scope = old_scope;
    }

    found_symbol = sym_rfqv(context->ast, found_cls->fqname);
    if (found_symbol) {
        found_symbol->exposed = 1;
        found_symbol->status = SYM_STATUS_RESOLVED_GLOBAL;
    }

    attach_imported_member_inline_payloads(context, new_stub, imported_class_has_source_contract(found_cls));

    return found_symbol;
}

Symbol *find_unique_implementing_class(Context *context, Symbol *interface_symbol, int *candidate_count) {
    size_t i;
    int count = 0;
    Symbol *winner = 0;
    struct imported_class *winner_import = 0;
    char *iface_fq;
    Scope *universe;

    if (candidate_count) *candidate_count = 0;
    if (!context || !sym_is_interface_symbol(interface_symbol)) return 0;

    iface_fq = sym_frnm(interface_symbol);
    if (!iface_fq) return 0;

    while (load_another_file(context)) {
        /* Ensure the importable contract registry is complete before scanning implementations. */
    }

    universe = context->ast ? context->ast->scope : 0;
    if (universe) {
        for (i = 0; i < scp_noch(universe); i++) {
            Scope *ns_scope = scp_chd(universe, i);
            Symbol **symbols;
            size_t j;

            if (!ns_scope) continue;
            symbols = scp_syms(ns_scope);
            if (!symbols) continue;

            for (j = 0; symbols[j]; j++) {
                Symbol *symbol = symbols[j];
                if (sym_is_class_contract_symbol(symbol) &&
                    loaded_class_implements_interface(context, symbol, interface_symbol)) {
                    count++;
                    if (count == 1) winner = symbol;
                }
            }

            free(symbols);
        }
    }

    if (context->master_context && context->master_context->importable_class_tree) {
        struct class_tree_wrapper *it;
        avl_tree_for_each_in_order(it, context->master_context->importable_class_tree, struct class_tree_wrapper, index_node) {
            struct imported_class *candidate = it->cls;
            Symbol *candidate_symbol = 0;
            int matches = 0;
            size_t k;

            if (!candidate || candidate->contract_type != CLASS_DEF) continue;
            candidate_symbol = sym_rfqn(context->ast, candidate->fqname);
            if (candidate_symbol &&
                sym_is_class_contract_symbol(candidate_symbol) &&
                loaded_class_implements_interface(context, candidate_symbol, interface_symbol)) {
                continue;
            }

            if (candidate->implements_count) {
                for (k = 0; k < candidate->implements_count; k++) {
                    if (strcmp(candidate->implements_fqnames[k], iface_fq) == 0) {
                        matches = 1;
                        break;
                    }
                }
            }

            if (!matches) {
                if (!candidate_symbol) candidate_symbol = load_imported_contract(context, candidate);
                if (candidate_symbol &&
                    sym_is_class_contract_symbol(candidate_symbol) &&
                    loaded_class_implements_interface(context, candidate_symbol, interface_symbol)) {
                    matches = 1;
                }
            }

            if (!matches) continue;

            count++;
            if (count == 1) {
                if (candidate_symbol) winner = candidate_symbol;
                else winner_import = candidate;
            }
        }
    }

    if (candidate_count) *candidate_count = count;
    free(iface_fq);

    if (count != 1) return 0;
    if (winner) return winner;
    if (winner_import) return load_imported_contract(context, winner_import);

    return 0;
}

/* Free Func Tree and functions */
void fre_ftre(Context *context) {
    struct tree_wrapper *i;
    struct avl_tree_node **root = (struct avl_tree_node **)&(context->importable_function_tree);

    /* This walks the tree in post order which allows each node be freed */
    avl_tree_for_each_in_postorder(i, *root, struct tree_wrapper, index_node) {
        freimpfc(i->func);
        free(i);
    }
    *root = 0;
}

static void dump_error_ast(Context *context) {
#ifndef __CMS__
    if (context->debug_mode >= 2) {
//        pdot_tree(context->ast, "error", context->file_name);
    }
#endif
}

static char error_in_node(ASTNode* node) {  // NOLINT
    if (node->node_type == ERROR) return 1;
    node = node->child;
    while (node) {
        if (error_in_node(node)) return 1;
        node = node->sibling;
    }
    return 0;
}

/* Forward declaration for local parser used by import stubs */
static Context *parseRexx(Context* parent_context, char *location, char* file_name, RexxLevel level, int debug_mode, char* rexx_source, size_t bytes);

/* Returns Argument source from the ARGS Node as a malloced string */
static char *generate_arg_source(ASTNode *node) {
    size_t args;
    size_t i;
    char *buffer;
    ASTNode *a;

    if (!node || node->node_type != ARGS) return strdup("");
    args = ast_nchd(node);
    if (!args) return strdup("");

    buffer = strdup("");
    for (i=0; i<args; i++) {
        a = ast_chdn(node, i);
        char *type_str = 0;
        char *name_str = 0;

        if (a->child->node_type == VARG || a->child->node_type == VARG_REFERENCE) {
            type_str = ast_n2tp(a->child->sibling);
            name_str = strdup("...");
        }
        else {
            /* Try to get type from symbol */
            if (a->child->symbolNode && a->child->symbolNode->symbol) {
                type_str = sym_2tp(a->child->symbolNode->symbol);
                name_str = strdup(a->child->symbolNode->symbol->name);
            }
            /* Fallback to AST nodes if symbol resolution is incomplete or .unknown */
            if (!type_str || strcmp(type_str, ".unknown") == 0) {
                if (type_str) free(type_str);
                /* a is ARG, a->child is VAR_SYMBOL, a->child->sibling is CLASS */
                if (a->child && a->child->sibling && a->child->sibling->node_type == CLASS) {
                    ASTNode *type_node = a->child->sibling;
                    if (type_node->node_string) {
                        type_str = malloc(type_node->node_string_length + 1);
                        memcpy(type_str, type_node->node_string, type_node->node_string_length);
                        type_str[type_node->node_string_length] = 0;
                    } else type_str = strdup(".unknown");
                } else type_str = strdup(".unknown");
            }
            if (!name_str) {
                if (a->child && a->child->node_type == VAR_SYMBOL) {
                    name_str = malloc(a->child->node_string_length + 1);
                    memcpy(name_str, a->child->node_string, a->child->node_string_length);
                    name_str[a->child->node_string_length] = 0;
                } else name_str = strdup("unknown");
            }
        }

        char *tmp;
        if (i == 0) tmp = mprintf("%s%s = %s", buffer, name_str, type_str);
        else tmp = mprintf("%s, %s = %s", buffer, name_str, type_str);

        free(buffer);
        buffer = tmp;
        free(type_str);
        free(name_str);
    }

    return buffer;
}

static char *copy_member_label_from_node(ASTNode *node) {
    char *label;

    if (!node || !node->node_string) return strdup("*");

    label = malloc(node->node_string_length + 1);
    if (!label) return 0;

    memcpy(label, node->node_string, node->node_string_length);
    label[node->node_string_length] = 0;
    return label;
}

static int parse_class_factory_fqname(const char *fqname, char **class_fq_out, char **member_name_out) {
    const char *marker;
    size_t class_len;

    if (class_fq_out) *class_fq_out = 0;
    if (member_name_out) *member_name_out = 0;
    if (!fqname) return 0;

    marker = strstr(fqname, ".\xc2\xa7" "factory");
    if (!marker) return 0;

    class_len = (size_t) (marker - fqname);
    if (!class_len || !memchr(fqname, '.', class_len)) return 0;

    if (class_fq_out) {
        *class_fq_out = malloc(class_len + 1);
        if (!*class_fq_out) return 0;
        memcpy(*class_fq_out, fqname, class_len);
        (*class_fq_out)[class_len] = 0;
    }

    if (member_name_out) {
        const char *suffix = marker + 10;
        if (*suffix == 0) {
            *member_name_out = strdup("*");
        } else if (*suffix == '.') {
            *member_name_out = strdup(suffix + 1);
        }
        if (!*member_name_out) {
            if (class_fq_out && *class_fq_out) {
                free(*class_fq_out);
                *class_fq_out = 0;
            }
            return 0;
        }
    }

    return 1;
}

static char **collect_implemented_interface_fqnames(Context *context, ASTNode *class_node, size_t *out_count) {
    ASTNode *implements_node;
    ASTNode *iface_ref;
    char **result = 0;
    size_t count = 0;

    if (out_count) *out_count = 0;
    if (!class_node || class_node->node_type != CLASS_DEF) return 0;

    implements_node = ast_chld(class_node, IMPLEMENTS, 0);
    if (!implements_node) return 0;

    for (iface_ref = implements_node->child; iface_ref; iface_ref = iface_ref->sibling) {
        Symbol *iface_symbol = iface_ref->symbolNode ? iface_ref->symbolNode->symbol : 0;
        char *fqname;
        if (!iface_symbol) iface_symbol = sym_rvfc(context->ast, iface_ref);
        if (!iface_symbol) continue;
        fqname = sym_frnm(iface_symbol);
        if (!fqname) continue;
        result = realloc(result, sizeof(char *) * (count + 1));
        result[count++] = fqname;
    }

    if (out_count) *out_count = count;
    return result;
}

static const char *register_view_for_type(const char *type) {
    if (!type) return "object";
    if (strcmp(type, ".int") == 0 || strcmp(type, ".bool") == 0) return "int";
    if (strcmp(type, ".float") == 0) return "float";
    if (strcmp(type, ".string") == 0) return "string";
    return "object";
}

static int register_index_from_node(ASTNode *node) {
    ASTNode *idx;

    if (!node) return -1;
    idx = ast_chld(node, INTEGER, 0);
    if (idx) return node_to_integer(idx);
    if (node->int_value) return (int)node->int_value;
    return -1;
}

static char *register_view_from_node(ASTNode *node, const char *type) {
    ASTNode *idx;
    ASTNode *view;

    if (!node) return strdup(register_view_for_type(type));
    idx = ast_chld(node, INTEGER, 0);
    view = idx ? idx->sibling : 0;
    if (view && view->node_type == VAR_SYMBOL && view->node_string && view->node_string_length) {
        char *result = malloc(view->node_string_length + 1);
        memcpy(result, view->node_string, view->node_string_length);
        result[view->node_string_length] = 0;
        return result;
    }
    return strdup(register_view_for_type(type));
}

static void append_class_attribute_stub_lines(ASTNode *contract_node, char **buffer) {
    ASTNode *m;

    if (!contract_node || contract_node->node_type != CLASS_DEF || !buffer || !*buffer) return;

    for (m = contract_node->child; m; m = m->sibling) {
        ASTNode *target;
        ASTNode *type_node;
        ASTNode *reg_node;
        char *type;
        char *view;
        char *tmp;
        int reg_index;

        if (m->node_type != DEFINE) continue;
        target = ast_chld(m, VAR_TARGET, 0);
        type_node = ast_chld(m, CLASS, 0);
        if (!target || !target->node_string || !target->node_string_length || !type_node) continue;

        type = ast_n2tp(type_node);
        if (!type) continue;
        reg_node = ast_chld(m, NODE_REGISTER, 0);
        reg_index = register_index_from_node(reg_node);
        view = register_view_from_node(reg_node, type);

        if (reg_index > 0) {
            tmp = mprintf("%s  %.*s = %s with register.%d.%s\n",
                          *buffer,
                          (int)target->node_string_length,
                          target->node_string,
                          type,
                          reg_index,
                          view ? view : register_view_for_type(type));
        } else {
            tmp = mprintf("%s  %.*s = %s\n",
                          *buffer,
                          (int)target->node_string_length,
                          target->node_string,
                          type);
        }
        free(*buffer);
        *buffer = tmp;
        free(type);
        if (view) free(view);
    }
}

/* Build a minimal contract stub source for an exposed class or interface */
static char* generate_contract_stub_source(ASTNode *contract_node) {
    Symbol *cls_sym;
    char *fq = 0;
    char *ns = 0;
    char *buffer = 0;

    if (!contract_node || (contract_node->node_type != CLASS_DEF && contract_node->node_type != INTERFACE_DEF)) return 0;
    if (!contract_node->symbolNode || !contract_node->symbolNode->symbol) return 0;

    cls_sym = contract_node->symbolNode->symbol;

    /* Get namespace and short class name from FQ name */
    fq = sym_frnm(cls_sym);
    if (!fq) return 0;

    size_t len = strlen(fq);
    size_t dot = len;
    while (dot) {
        dot--;
        if (fq[dot] == '.') break;
    }
    if (dot == 0 || fq[dot] != '.') { /* No namespace - skip (not importable) */
        free(fq);
        return 0;
    }

    ns = (char*)malloc(dot + 1);
    memcpy(ns, fq, dot);
    ns[dot] = 0;

    const char *cls_name = fq + dot + 1;

    /* Start stub source */
    if (contract_node->node_type == CLASS_DEF) {
        ASTNode *implements_node = ast_chld(contract_node, IMPLEMENTS, 0);
        if (implements_node && implements_node->child) {
            ASTNode *iface_ref = implements_node->child;
            buffer = mprintf("options levelb\nnamespace %s\n%s: class", ns, cls_name);
            while (iface_ref) {
                char *tmp = mprintf("%s%s %.*s",
                                    buffer,
                                    iface_ref == implements_node->child ? " implements" : "",
                                    (int)iface_ref->node_string_length,
                                    iface_ref->node_string);
                free(buffer);
                buffer = tmp;
                iface_ref = iface_ref->sibling;
            }
            {
                char *tmp = mprintf("%s\n", buffer);
                free(buffer);
                buffer = tmp;
            }
        } else {
            buffer = mprintf("options levelb\nnamespace %s\n%s: class\n", ns, cls_name);
        }
    } else {
        buffer = mprintf("options levelb\nnamespace %s\n%s: interface\n", ns, cls_name);
    }

    append_class_attribute_stub_lines(contract_node, &buffer);

    /* Iterate contract members for FACTORY/METHOD signatures */
    ASTNode *m;
    for (m = contract_node->child; m; m = m->sibling) {
        if (m->node_type == FACTORY) {
            char *factory_name = copy_member_label_from_node(m);
            char *tmp;
            tmp = mprintf("%s  %s: factory\n", buffer, factory_name ? factory_name : "*");
            free(buffer);
            buffer = tmp;
            if (factory_name) free(factory_name);

            ASTNode *args_node = ast_chld(m, ARGS, 0);
            if (args_node) {
                char *args = generate_arg_source(args_node);
                if (args && args[0]) {
                    tmp = mprintf("%s  arg %s\n", buffer, args);
                    free(buffer);
                    buffer = tmp;
                }
                if (args) free(args);
            }
        }
        else if (m->node_type == METHOD) {
            /* Method name */
            char *mname = (char*)malloc(m->node_string_length + 1);
            memcpy(mname, m->node_string, m->node_string_length);
            mname[m->node_string_length] = 0;

            /* Return type (default .void added by grammar) */
            ASTNode *ret = ast_chld(m, CLASS, VOID);
            char *rtype = ast_n2tp(ret);

            char *tmp = mprintf("%s  %s: method = %s\n", buffer, mname, rtype);
            free(buffer);
            buffer = tmp;

            ASTNode *args_node = ast_chld(m, ARGS, 0);
            if (args_node) {
                char *args = generate_arg_source(args_node);
                if (args && args[0]) {
                    tmp = mprintf("%s  arg %s\n", buffer, args);
                    free(buffer);
                    buffer = tmp;
                }
                if (args) free(args);
            }
            free(mname);
            free(rtype);
        }
    }

    free(ns);
    free(fq);
    return buffer;
}

static void register_source_import_member_inline_payloads(Context *context, ASTNode *contract_node) {
    ASTNode *member;

    if (!context || !contract_node || contract_node->node_type != CLASS_DEF) return;

    for (member = contract_node->child; member; member = member->sibling) {
        ASTNode *body_marker;
        char *fqname;
        char *type;
        char *args;
        char *impl;

        if (member->node_type != METHOD && member->node_type != FACTORY) continue;
        if (!member->symbolNode || !member->symbolNode->symbol) continue;

        body_marker = ast_chld(member, INSTRUCTIONS, NOP);
        if (!body_marker || body_marker->node_type == NOP) continue;

        fqname = sym_frnm(member->symbolNode->symbol);
        if (!fqname) continue;

        type = callable_effective_return_type(member);
        args = meta_narg(ast_chld(member, ARGS, 0));
        impl = rxcp_inline_export_payload(context, member);

        rximpf_f(context,
                member->file_name ? member->file_name : context->file_name,
                fqname,
                "b",
                type,
                args,
                impl && *impl ? impl : 0,
                0);

        if (type) free(type);
        if (args) free(args);
        if (impl) free(impl);
        free(fqname);
    }
}

static walker_result class_signature_walker(walker_direction direction,
                                            ASTNode* node,
                                            void *pl) {
    /* Walk entire imported AST and register exposed contracts in the class tree */
    if (direction == out && (node->node_type == CLASS_DEF || node->node_type == INTERFACE_DEF)) {
        class_import_payload *p = (class_import_payload*)pl;
        if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->exposed) {
            char *fqname = sym_frnm(node->symbolNode->symbol);
            if (fqname) {
                size_t implements_count = 0;
                char **implements_fqnames = collect_implemented_interface_fqnames(p->import_context, node, &implements_count);
                char *stub_source = generate_contract_stub_source(node);
                register_source_import_member_inline_payloads(p->import_context, node);
                if (stub_source) {
                    Context *stub_ctx = parseRexx(p->parent_context, p->import_context->location, p->import_context->file_name,
                                                  LEVELB, p->parent_context->debug_mode, stub_source, strlen(stub_source));
                    if (stub_ctx && stub_ctx->ast && !error_in_node(stub_ctx->ast)) {
                        if (node->node_type == INTERFACE_DEF) {
                            mark_source_import_interface_default_methods(stub_ctx, node);
                        }
                        if (stub_ctx->ast->child) stub_ctx->ast->child->node_type = IMPORTED_FILE;
                        rximpcl_f(p->parent_context, p->import_context->file_name, fqname, stub_ctx,
                                  node->node_type, implements_fqnames, implements_count);
                    } else {
                        if (implements_fqnames) {
                            size_t i;
                            for (i = 0; i < implements_count; i++) free(implements_fqnames[i]);
                            free(implements_fqnames);
                        }
                        if (stub_ctx) fre_cntx(stub_ctx);
                        else free(stub_source);
                    }
                } else if (implements_fqnames) {
                    size_t i;
                    for (i = 0; i < implements_count; i++) free(implements_fqnames[i]);
                    free(implements_fqnames);
                }
                free(fqname);
            }
        }
    }
    return result_normal;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"
static walker_result procedure_signature_walker(walker_direction direction,
                                                ASTNode* node,
                                                void *pl) {

    Context *context = pl;
    ASTNode *type_node;
    ASTNode *args_node;
    ASTNode *impl_node;
    imported_func *func;
    char *type;
    char *args;
    char *impl;
    char *fqname;

    if (direction == out) {
        /* OUT - BOTTOM UP */
        if (node->node_type == PROCEDURE) {
            type_node = ast_chld(node, CLASS, VOID);
            args_node = ast_chld(node, ARGS, 0);
            impl_node = ast_chld(node, INSTRUCTIONS, NOP);
            fqname = sym_frnm(node->symbolNode->symbol);

            if (impl_node && impl_node->node_type != NOP && node->symbolNode->symbol->exposed) {

                if ( !type_node || !args_node || error_in_node(type_node) || error_in_node(args_node) ) {
                    /* Error in the syntax of the important function */
                    func = rximpf_f(context, node->file_name, fqname, "b", 0, 0, 0, 0);
                    if (func) func->error_state = "SYNTAX_ERROR_IN_IMPORT_DECL";
                }

                else {
                    /* Add the function to the "database" of functions */
                    type = ast_n2tp(type_node);
                    args = meta_narg(args_node);
                    impl = rxcp_inline_export_payload(context, node);

                    func = rximpf_f(context,
                                    node->file_name,
                                    fqname,
                                    "b",
                                    type,
                                    args,
                                    impl && *impl ? impl : 0,
                                    0);

                    /* Check Type <> unknown */
                    if ( !type || strcmp(type, ".unknown") == 0 ) {
                        if (func) func->error_state = "SYNTAX_ERROR_IN_IMPORT_DECL";
                    }

                    free(type);
                    free(args);
                    free(impl);
                }
            }
            free(fqname);
        }

    }
    return result_normal;
}
#pragma clang diagnostic pop

/* Get the constant string in a malloced buffer */
static char* get_const_string(void* constpool, size_t ix) {
    char *result;
    char *c;
    size_t sz;

    if (ix == -1) return 0;

    c = ((string_constant *)(constpool + ix))->string;
    sz = ((string_constant *)(constpool + ix))->string_len;

    result = malloc(sz + 1);
    if (result) {
        memcpy(result, c, sz);
        result[sz] = 0;
    }
    return result;
}

/* Simple aggregator for class stubs extracted from metadata */
typedef struct class_meta_agg {
    char *fq;        /* fully-qualified class name: namespace.class */
    char *ns;        /* namespace */
    char *name;      /* short class name */
    char *attributes;/* accumulated class attribute signature lines */
    char *methods;   /* accumulated method/factory signature lines */
    NodeType contract_type;
    char **implements_fqnames;
    size_t implements_count;
    char **default_method_names;
    size_t default_method_count;
    struct class_meta_agg *next;
} class_meta_agg;

static class_meta_agg* agg_find(class_meta_agg *head, const char *fq) {
    class_meta_agg *it = head;
    while (it) {
        if (strcmp(it->fq, fq) == 0) return it;
        it = it->next;
    }
    return 0;
}

static class_meta_agg* agg_find_or_add(class_meta_agg **head, const char *fq, NodeType contract_type) {
    class_meta_agg *it = *head;
    while (it) {
        if (strcmp(it->fq, fq) == 0) {
            if (contract_type != 0) it->contract_type = contract_type;
            return it;
        }
        it = it->next;
    }
    /* Create */
    class_meta_agg *n = calloc(1, sizeof(class_meta_agg));
    n->fq = strdup(fq);
    /* Split namespace and name by last '.' */
    const char *dot = strrchr(fq, '.');
    if (dot) {
        size_t nslen = (size_t)(dot - fq);
        n->ns = malloc(nslen + 1);
        memcpy(n->ns, fq, nslen);
        n->ns[nslen] = 0;
    n->name = strdup(dot + 1);
    } else {
        n->ns = strdup("");
        n->name = strdup(fq);
    }
    n->attributes = 0;
    n->methods = 0;
    n->contract_type = contract_type;
    n->implements_fqnames = 0;
    n->implements_count = 0;
    n->default_method_names = 0;
    n->default_method_count = 0;
    n->next = *head;
    *head = n;
    return n;
}

static void agg_append_line(class_meta_agg *agg, const char *line) {
    if (!line || !*line) return;
    if (!agg->methods) {
        agg->methods = strdup(line);
    } else {
        char *tmp = mprintf("%s%s", agg->methods, line);
        free(agg->methods);
        agg->methods = tmp;
    }
}

static void agg_append_attribute_line(class_meta_agg *agg, const char *line) {
    if (!line || !*line) return;
    if (!agg->attributes) {
        agg->attributes = strdup(line);
    } else {
        char *tmp = mprintf("%s%s", agg->attributes, line);
        free(agg->attributes);
        agg->attributes = tmp;
    }
}

static void agg_free_all(class_meta_agg *head) {
    class_meta_agg *n = head;
    while (n) {
        class_meta_agg *nx = n->next;
        if (n->fq) free(n->fq);
        if (n->ns) free(n->ns);
        if (n->name) free(n->name);
        if (n->attributes) free(n->attributes);
        if (n->methods) free(n->methods);
        if (n->implements_fqnames) {
            size_t i;
            for (i = 0; i < n->implements_count; i++) free(n->implements_fqnames[i]);
            free(n->implements_fqnames);
        }
        if (n->default_method_names) {
            size_t i;
            for (i = 0; i < n->default_method_count; i++) free(n->default_method_names[i]);
            free(n->default_method_names);
        }
        free(n);
        n = nx;
    }
}

static void agg_add_implements(class_meta_agg *agg, const char *iface_fq) {
    size_t i;

    if (!agg || !iface_fq) return;
    for (i = 0; i < agg->implements_count; i++) {
        if (strcmp(agg->implements_fqnames[i], iface_fq) == 0) return;
    }

    agg->implements_fqnames = realloc(agg->implements_fqnames, sizeof(char *) * (agg->implements_count + 1));
    agg->implements_fqnames[agg->implements_count++] = strdup(iface_fq);
}

static void agg_add_default_method(class_meta_agg *agg, const char *member_name) {
    size_t i;

    if (!agg || !member_name) return;
    for (i = 0; i < agg->default_method_count; i++) {
        if (strcmp(agg->default_method_names[i], member_name) == 0) return;
    }

    agg->default_method_names = realloc(agg->default_method_names,
                                        sizeof(char *) * (agg->default_method_count + 1));
    agg->default_method_names[agg->default_method_count++] = strdup(member_name);
}

static void mark_imported_interface_default_methods(Context *stub_ctx, class_meta_agg *agg) {
    ASTNode *file_node;
    ASTNode *contract_node;
    ASTNode *member;
    size_t i;

    if (!stub_ctx || !stub_ctx->ast || !stub_ctx->ast->child || !agg || !agg->default_method_count) return;

    file_node = stub_ctx->ast->child;
    contract_node = file_node ? file_node->child : 0;
    while (contract_node && contract_node->node_type != INTERFACE_DEF) contract_node = contract_node->sibling;
    if (!contract_node) return;

    for (member = contract_node->child; member; member = member->sibling) {
        if (member->node_type != METHOD || !member->node_string) continue;
        for (i = 0; i < agg->default_method_count; i++) {
            if (strlen(agg->default_method_names[i]) == member->node_string_length &&
                memcmp(agg->default_method_names[i], member->node_string, member->node_string_length) == 0) {
                member->is_interface_default_method = 1;
                break;
            }
        }
    }
}

static int contract_member_is_default_method(ASTNode *member) {
    ASTNode *instructions;

    if (!member || member->node_type != METHOD) return 0;
    if (member->is_interface_default_method) return 1;

    instructions = ast_chld(member, INSTRUCTIONS, 0);
    return instructions && instructions->child != 0;
}

static int ast_node_label_equals(ASTNode *node, const char *label) {
    size_t label_len;
    if (!node || !node->node_string || !label) return 0;
    label_len = strlen(label);
    return node->node_string_length == label_len &&
           memcmp(node->node_string, label, label_len) == 0;
}

static void remove_import_stub_implicit_main(Context *stub_ctx) {
    ASTNode *file_node;
    ASTNode *prev = 0;
    ASTNode *child;

    if (!stub_ctx || !stub_ctx->ast || !stub_ctx->ast->child) return;

    file_node = stub_ctx->ast->child;
    child = file_node->child;
    while (child) {
        ASTNode *next = child->sibling;
        if (child->node_type == PROCEDURE && ast_node_label_equals(child, "main")) {
            if (prev) prev->sibling = next;
            else file_node->child = next;
            child->sibling = 0;
        } else {
            prev = child;
        }
        child = next;
    }
}

static void mark_source_import_interface_default_methods(Context *stub_ctx, ASTNode *contract_node) {
    ASTNode *file_node;
    ASTNode *stub_contract;
    ASTNode *orig_member;

    if (!stub_ctx || !stub_ctx->ast || !stub_ctx->ast->child || !contract_node ||
        contract_node->node_type != INTERFACE_DEF) return;

    file_node = stub_ctx->ast->child;
    stub_contract = file_node ? file_node->child : 0;
    while (stub_contract && stub_contract->node_type != INTERFACE_DEF) stub_contract = stub_contract->sibling;
    if (!stub_contract) return;

    for (orig_member = contract_node->child; orig_member; orig_member = orig_member->sibling) {
        ASTNode *stub_member;

        if (!contract_member_is_default_method(orig_member) || !orig_member->node_string) continue;

        for (stub_member = stub_contract->child; stub_member; stub_member = stub_member->sibling) {
            if (stub_member->node_type == METHOD &&
                stub_member->node_string &&
                stub_member->node_string_length == orig_member->node_string_length &&
                memcmp(stub_member->node_string, orig_member->node_string, orig_member->node_string_length) == 0) {
                stub_member->is_interface_default_method = 1;
                break;
            }
        }
    }
}

static char **clone_fqname_array(char **names, size_t count) {
    char **copy;
    size_t i;

    if (!names || !count) return 0;
    copy = malloc(sizeof(char *) * count);
    for (i = 0; i < count; i++) copy[i] = strdup(names[i]);
    return copy;
}

static void import_class_meta_aggs(Context *context, char *full_file_name, class_meta_agg *class_aggs) {
    class_meta_agg *a;

    if (!class_aggs) return;

    a = class_aggs;
    while (a) {
        if (a->contract_type == CLASS_DEF || (a->methods && *a->methods)) {
            char *stub_source;
            if (a->contract_type == INTERFACE_DEF) {
                stub_source = mprintf("options levelb\nnamespace %s\n%s: interface\n%s",
                                      a->ns,
                                      a->name,
                                      a->methods ? a->methods : "");
            } else {
                /* Implements metadata is kept as FQNs on imported_class; source stubs only provide members. */
                stub_source = mprintf("options levelb\nnamespace %s\n%s: class\n%s%s",
                                      a->ns,
                                      a->name,
                                      a->attributes ? a->attributes : "",
                                      a->methods ? a->methods : "");
            }
            Context *stub_ctx = parseRexx(context, context->location, full_file_name, LEVELB, context->debug_mode, stub_source, strlen(stub_source));
            if (stub_ctx && stub_ctx->ast && !error_in_node(stub_ctx->ast)) {
                remove_import_stub_implicit_main(stub_ctx);
                if (a->contract_type == INTERFACE_DEF) {
                    mark_imported_interface_default_methods(stub_ctx, a);
                }
                if (stub_ctx->ast->child) stub_ctx->ast->child->node_type = IMPORTED_FILE;
                rximpcl_f(context, full_file_name, a->fq, stub_ctx, a->contract_type,
                          clone_fqname_array(a->implements_fqnames, a->implements_count), a->implements_count);
            } else {
                if (stub_ctx) fre_cntx(stub_ctx);
                else free(stub_source);
            }
        }
        a = a->next;
    }
}

/* Get the global variable type by reading metadata */
/* name is null terminated fqname of the variable */
/* returns found meta_reg_constant entry - or 0 if not found */
static meta_reg_constant* get_variable_type(char* name, void* constant, int meta_head) {
    meta_entry *entry;
    meta_reg_constant *mentry;
    size_t i;
    char* fqname;

    i = meta_head;
    while (i != -1) {
        entry = (meta_entry *) (constant + i);

        if (entry->base.type == META_REG) {
            mentry = (meta_reg_constant *) entry;
            fqname = get_const_string(constant, mentry->symbol);
            if (fqname) {
                if (strcmp(fqname, name) == 0) {
                    free(fqname);
                    return mentry;
                }
                free(fqname);
                fqname = 0;
            }
        }

        i = entry->next;
    }
    return 0;
}

static char *get_inline_payload_for_symbol(void *constant, int meta_head, const char *symbol) {
    meta_entry *entry;
    meta_inline_constant *mentry;
    int i;
    char *inline_symbol;
    char *payload;

    if (!constant || !symbol) return 0;

    i = meta_head;
    while (i != -1) {
        entry = (meta_entry *) (constant + (size_t)i);

        if (entry->base.type == META_INLINE) {
            mentry = (meta_inline_constant *) entry;
            inline_symbol = get_const_string(constant, mentry->symbol);
            if (inline_symbol) {
                if (strcmp(inline_symbol, symbol) == 0) {
                    free(inline_symbol);
                    payload = get_const_string(constant, mentry->payload);
                    return payload;
                }
                free(inline_symbol);
            }
        }

        i = entry->next;
    }

    return 0;
}

static void read_constant_pool_for_functions(Context *context, char *full_file_name, void* constant, size_t constant_size, int meta_head) {
    chameleon_constant *entry;
    int i;
    size_t exposed_ix;
    expose_proc_constant *exposed;
    char* fqname;
    char* option = 0;
    char* type = 0;
    char* args = 0;
    char* inline_payload = 0;
    char* meta_symbol = 0;

    /* Aggregator for class metadata to synthesize class stubs */
    class_meta_agg *class_aggs = 0;

    (void)constant_size;

    /* Walk only the module metadata chain so shared constant pools stay module-local in effect. */
    i = meta_head;
    while (i != -1) {
        entry = (chameleon_constant *) (constant + (size_t)i);

        /* A function/method definition */
        if (entry->type == META_FUNC) {
            meta_func_constant *mentry = (meta_func_constant *) entry;

            /* Check it is exposed */
            exposed_ix = ((proc_constant *) (constant + mentry->func))->exposed;
            if ((int) exposed_ix != -1) {
                exposed = (expose_proc_constant *) (constant + exposed_ix);

                /* Exported */
                if (!exposed->imported) {
                    fqname = exposed->index;
                    meta_symbol = get_const_string(constant, mentry->symbol);
                    option = get_const_string(constant, mentry->option);
                    type = get_const_string(constant, mentry->type);
                    args = get_const_string(constant, mentry->args);
                    inline_payload = get_inline_payload_for_symbol(constant, meta_head, meta_symbol ? meta_symbol : fqname);

                    /* Always register as importable function (methods too) */
                    rximpf_f(context, full_file_name, fqname, option, type, args, inline_payload, 0);

                    /* If this looks like a class method (fqname contains namespace.class.method) then
                     * accumulate a signature line for later class stub synthesis */
                    if (fqname) {
                        char *class_fq = 0;
                        char *factory_member = 0;
                        if (parse_class_factory_fqname(fqname, &class_fq, &factory_member)) {
                            class_meta_agg *agg = agg_find_or_add(&class_aggs, class_fq, CLASS_DEF);
                            if (args && *args) {
                                char *ln = mprintf("  %s: factory\n  arg %s\n", factory_member, args);
                                agg_append_line(agg, ln);
                                free(ln);
                            } else {
                                char *ln = mprintf("  %s: factory\n", factory_member);
                                agg_append_line(agg, ln);
                                free(ln);
                            }
                            free(factory_member);
                            free(class_fq);
                        } else {
                            const char *last_dot = strrchr(fqname, '.');
                            if (last_dot) {
                                /* Ensure there is at least another dot before last to separate namespace and class */
                                size_t class_len = (size_t)(last_dot - fqname);
                                if (memchr(fqname, '.', class_len) != 0) {
                                    class_fq = malloc(class_len + 1);
                                    memcpy(class_fq, fqname, class_len);
                                    class_fq[class_len] = 0;
                                    class_meta_agg *agg = agg_find(class_aggs, class_fq);

                                    if (!agg || agg->contract_type != INTERFACE_DEF) {
                                        if (!agg) agg = agg_find_or_add(&class_aggs, class_fq, CLASS_DEF);

                                        /* method name is after last dot */
                                        const char *mname = last_dot + 1;
                                        if (type && *type) {
                                            char *ln = mprintf("  %s: method = %s\n", mname, type);
                                            agg_append_line(agg, ln);
                                            free(ln);
                                        } else {
                                            char *ln = mprintf("  %s: method\n", mname);
                                            agg_append_line(agg, ln);
                                            free(ln);
                                        }
                                        if (args && *args) {
                                            char *ln2 = mprintf("  arg %s\n", args);
                                            agg_append_line(agg, ln2);
                                            free(ln2);
                                        }
                                    }
                                    free(class_fq);
                                }
                            }
                        }
                    }

                    if (option) {
                        free(option);
                        option = 0;
                    }
                    if (type) {
                        free(type);
                        type = 0;
                    }
                    if (args) {
                        free(args);
                        args = 0;
                    }
                    if (inline_payload) {
                        free(inline_payload);
                        inline_payload = 0;
                    }
                    if (meta_symbol) {
                        free(meta_symbol);
                        meta_symbol = 0;
                    }
                }
            }
        }
        else if (entry->type == META_CLASS) {
            /* Record class presence so that a stub is synthesized even if it has no methods */
            meta_class_constant *mentry = (meta_class_constant *) entry;
            char *cls_sym = get_const_string(constant, mentry->symbol);
            if (cls_sym) {
                agg_find_or_add(&class_aggs, cls_sym, CLASS_DEF);
                free(cls_sym);
            }
        }
        else if (entry->type == META_ATTR) {
            meta_attr_constant *mentry = (meta_attr_constant *) entry;
            char *attr_sym = get_const_string(constant, mentry->symbol);
            char *type_str = get_const_string(constant, mentry->type);

            if (attr_sym) {
                const char *last_dot = strrchr(attr_sym, '.');
                if (last_dot && last_dot != attr_sym) {
                    size_t owner_len = (size_t)(last_dot - attr_sym);
                    char *owner = malloc(owner_len + 1);

                    memcpy(owner, attr_sym, owner_len);
                    owner[owner_len] = 0;
                    {
                        class_meta_agg *agg = agg_find_or_add(&class_aggs, owner, CLASS_DEF);
                        const char *attr_name = last_dot + 1;
                        char *ln = mprintf("  %s = %s with register.%d.%s\n",
                                           attr_name,
                                           type_str ? type_str : ".object",
                                           (int)mentry->reg,
                                           register_view_for_type(type_str));
                        agg_append_attribute_line(agg, ln);
                        free(ln);
                    }
                    free(owner);
                }
                free(attr_sym);
            }
            if (type_str) free(type_str);
            (void)mentry;
        }
        else if (entry->type == META_INTERFACE) {
            meta_interface_constant *mentry = (meta_interface_constant *) entry;
            char *iface_sym = get_const_string(constant, mentry->symbol);
            if (iface_sym) {
                agg_find_or_add(&class_aggs, iface_sym, INTERFACE_DEF);
                free(iface_sym);
            }
        }
        else if (entry->type == META_IMPLEMENTS) {
            meta_implements_constant *mentry = (meta_implements_constant *) entry;
            char *class_sym = get_const_string(constant, mentry->symbol);
            char *iface_sym = get_const_string(constant, mentry->interface_symbol);
            if (class_sym && iface_sym) {
                class_meta_agg *agg = agg_find_or_add(&class_aggs, class_sym, CLASS_DEF);
                agg_add_implements(agg, iface_sym);
            }
            if (class_sym) free(class_sym);
            if (iface_sym) free(iface_sym);
        }
        else if (entry->type == META_MEMBER) {
            meta_member_constant *mentry = (meta_member_constant *) entry;
            char *owner = get_const_string(constant, mentry->owner);
            char *kind = get_const_string(constant, mentry->kind);
            char *member = get_const_string(constant, mentry->member);
            char *type_str = get_const_string(constant, mentry->type);
            char *args_str = get_const_string(constant, mentry->args);

            if (owner && kind && member) {
                class_meta_agg *agg = agg_find(class_aggs, owner);
                char *ln;

                if (!agg) agg = agg_find_or_add(&class_aggs, owner, INTERFACE_DEF);

                if (strcmp(kind, "factory") == 0) {
                    ln = mprintf("  %s: factory\n", member);
                } else {
                    ln = mprintf("  %s: method = %s\n", member, type_str ? type_str : ".void");
                    if (strstr(kind, "final")) {
                        agg_add_default_method(agg, member);
                    }
                }
                agg_append_line(agg, ln);
                free(ln);
                if (args_str && *args_str) {
                    ln = mprintf("  arg %s\n", args_str);
                    agg_append_line(agg, ln);
                    free(ln);
                }
            }

            if (owner) free(owner);
            if (kind) free(kind);
            if (member) free(member);
            if (type_str) free(type_str);
            if (args_str) free(args_str);
        }

        /* A exposed global variable */
        else if (entry->type == EXPOSE_REG_CONST) {
            fqname = ((expose_reg_constant *)entry)->index;
            meta_reg_constant *mentry = get_variable_type(fqname, constant, meta_head);
            if (mentry) {
                option = get_const_string(constant, mentry->option);
                type = get_const_string(constant, mentry->type);

                rximpf_f(context, full_file_name, fqname, option, type, "", "", 1);

                if (option) {
                    free(option);
                    option = 0;
                }
                if (type) {
                    free(type);
                    type = 0;
                }
            }
        }

        i = ((meta_entry *)entry)->next;
    }

    if (class_aggs) {
        import_class_meta_aggs(context, full_file_name, class_aggs);
        agg_free_all(class_aggs);
    }
}

// RXPA Disabler Function
static char* plugin_being_loaded = "statically linked";
static Context* plugin_being_loaded_context = 0;
static struct static_linked_metadata *plugin_being_loaded_metadata = 0;

static void append_rxpa_metadata(struct static_linked_metadata **head, char *kind, char *symbol,
                                 char *option, char *type, char *interface_symbol,
                                 char *owner, char *member_kind, char *member, char *args) {
    struct static_linked_metadata *new_meta = malloc(sizeof(struct static_linked_metadata));

    new_meta->kind = kind;
    new_meta->symbol = symbol;
    new_meta->option = option;
    new_meta->type = type;
    new_meta->interface_symbol = interface_symbol;
    new_meta->owner = owner;
    new_meta->member_kind = member_kind;
    new_meta->member = member;
    new_meta->args = args;
    new_meta->next = *head;
    *head = new_meta;
}

static void free_rxpa_metadata_list(struct static_linked_metadata **head) {
    while (*head) {
        struct static_linked_metadata *next = (*head)->next;
        free(*head);
        *head = next;
    }
}

static void import_rxpa_metadata_list(Context *context, char *file_name, struct static_linked_metadata *metadata) {
    class_meta_agg *class_aggs = 0;
    struct static_linked_metadata *entry = metadata;

    while (entry) {
        if (entry->kind && strcmp(entry->kind, "class") == 0 && entry->symbol) {
            agg_find_or_add(&class_aggs, entry->symbol, CLASS_DEF);
        }
        else if (entry->kind && strcmp(entry->kind, "interface") == 0 && entry->symbol) {
            agg_find_or_add(&class_aggs, entry->symbol, INTERFACE_DEF);
        }
        else if (entry->kind && strcmp(entry->kind, "implements") == 0 &&
                 entry->symbol && entry->interface_symbol) {
            class_meta_agg *agg = agg_find_or_add(&class_aggs, entry->symbol, CLASS_DEF);
            agg_add_implements(agg, entry->interface_symbol);
        }
        else if (entry->kind && strcmp(entry->kind, "member") == 0 &&
                 entry->owner && entry->member_kind && entry->member) {
            class_meta_agg *agg = agg_find(class_aggs, entry->owner);
            char *ln;

            if (!agg) agg = agg_find_or_add(&class_aggs, entry->owner, INTERFACE_DEF);

            if (strcmp(entry->member_kind, "factory") == 0) {
                ln = mprintf("  %s: factory\n", entry->member);
            } else {
                ln = mprintf("  %s: method = %s\n", entry->member, entry->type ? entry->type : ".void");
                if (strstr(entry->member_kind, "final")) {
                    agg_add_default_method(agg, entry->member);
                }
            }
            agg_append_line(agg, ln);
            free(ln);
            if (entry->args && *entry->args) {
                ln = mprintf("  arg %s\n", entry->args);
                agg_append_line(agg, ln);
                free(ln);
            }
        }

        entry = entry->next;
    }

    if (class_aggs) {
        import_class_meta_aggs(context, file_name, class_aggs);
        agg_free_all(class_aggs);
    }
}

static void disablerFunction(char* fname) {
    fprintf(stderr, "RXC Panic: When loading %s plugin, it called %s() illegally\n", plugin_being_loaded, fname);
    exit(EXIT_FAILURE);
}

// RXPA Helper functions required to be provided - dummy functions
char* rxpa_getstring(rxpa_attribute_value attributeValue)  /* Get a string from an attribute value */
    { disablerFunction("rxpa_getstring"); return NULL; }

void rxpa_setstring(rxpa_attribute_value attributeValue, char* string)  /* Set a string in an attribute value */
    { disablerFunction("rxpa_setstring"); }

void rxpa_setint(rxpa_attribute_value attributeValue, rxinteger value)  /* Set an integer in an attribute value */
    { disablerFunction("rxpa_setint"); }

rxinteger rxpa_getint(rxpa_attribute_value attributeValue)  /* Get an integer from an attribute value */
    { disablerFunction("rxpa_getint"); return 0; }

void rxpa_setfloat(rxpa_attribute_value attributeValue, double value)  /* Set a float in an attribute value */
    { disablerFunction("rxpa_setfloat"); }

double rxpa_getfloat(rxpa_attribute_value attributeValue)  /* Get a float from an attribute value */
    { disablerFunction("rxpa_getfloat"); return 0.0; }

int rxpa_setnativepayload(rxpa_attribute_value attributeValue, const void *payload, size_t length,
                          const rxpa_native_payload_ops *ops,
                          unsigned int flags)  /* Set a native binary payload */
    { disablerFunction("rxpa_setnativepayload"); return -1; }

void* rxpa_getnativepayload(rxpa_attribute_value attributeValue, size_t *out_length,
                            const rxpa_native_payload_ops **out_ops,
                            unsigned int *out_flags)  /* Get a native binary payload */
    { disablerFunction("rxpa_getnativepayload"); return NULL; }

rxinteger rxpa_getnumattrs(rxpa_attribute_value attributeValue)  /* Get the number of child attributes */
    { disablerFunction("rxpa_getnumattrs"); return 0; }

void rxpa_setnumattrs(rxpa_attribute_value attributeValue, rxinteger numAttrs)  /* Set the number of child attributes */
    { disablerFunction("rxpa_setnumattrs"); }

rxpa_attribute_value rxpa_getattr(rxpa_attribute_value attributeValue, rxinteger index)  /* Get the nth child attribute */
    { disablerFunction("rxpa_getattr"); return NULL; }

rxpa_attribute_value rxpa_insertattr(rxpa_attribute_value attributeValue, rxinteger index)  /* Insert a child attribute before the nth position */
    { disablerFunction("rxpa_insertattr"); return NULL; }

void rxpa_removeattr(rxpa_attribute_value attributeValue, rxinteger index)  /* Remove the nth child attribute */
    { disablerFunction("rxpa_removeattr"); }

void rxpa_swapattrs(rxpa_attribute_value attributeValue, rxinteger index1, rxinteger index2)  /* Swap the nth child attribute with the mth child attribute */
    { disablerFunction("rxpa_swapattrs"); }

// Exit Function Management
void rxpa_setsayexit(say_exit_func sayExitFunc)  /* Set Say exit function */
    { disablerFunction("rxpa_setsayexit"); }

void rxpa_resetsayexit()  /* Reset Say exit function */
    { disablerFunction("rxpa_resetsayexit"); }

// RXPA Add Function Implementation
// This is the callback function for loadPluginFileForFunctions() when the plugin adds functions,
// oir is called during initialising a statically linked plugin
void rxpa_addfunc(rxpa_libfunc func, char* name, char* option, char* type, char* args) {
    if (plugin_being_loaded_context) {
        if (plugin_being_loaded_metadata) {
            import_rxpa_metadata_list(plugin_being_loaded_context, plugin_being_loaded, plugin_being_loaded_metadata);
            free_rxpa_metadata_list(&plugin_being_loaded_metadata);
        }
        if (plugin_being_loaded_context->debug_mode >= 2) printf("Importing Procedures - Loading %s\n", name);
        rximpf_f(plugin_being_loaded_context, plugin_being_loaded, name, option, type, args, 0, 0);
    }
    else {
        // Add to the list of statically linked functions
        struct static_linked_function *new_static_func = malloc(sizeof(struct static_linked_function));
        new_static_func->name = name;
        new_static_func->option = option;
        new_static_func->type = type;
        new_static_func->args = args;
        new_static_func->next = static_linked_functions;
        static_linked_functions = new_static_func;
    }
}

void rxpa_addclass(char* name, char* option, char* type) {
    if (plugin_being_loaded_context) {
        append_rxpa_metadata(&plugin_being_loaded_metadata, "class", name, option, type, 0, 0, 0, 0, 0);
    }
    else {
        append_rxpa_metadata(&static_linked_metadata, "class", name, option, type, 0, 0, 0, 0, 0);
    }
}

void rxpa_addinterface(char* name, char* option, char* type) {
    if (plugin_being_loaded_context) {
        append_rxpa_metadata(&plugin_being_loaded_metadata, "interface", name, option, type, 0, 0, 0, 0, 0);
    }
    else {
        append_rxpa_metadata(&static_linked_metadata, "interface", name, option, type, 0, 0, 0, 0, 0);
    }
}

void rxpa_addimplements(char* name, char* interface_name) {
    if (plugin_being_loaded_context) {
        append_rxpa_metadata(&plugin_being_loaded_metadata, "implements", name, 0, 0, interface_name, 0, 0, 0, 0);
    }
    else {
        append_rxpa_metadata(&static_linked_metadata, "implements", name, 0, 0, interface_name, 0, 0, 0, 0);
    }
}

void rxpa_addmember(char* owner, char* kind, char* member, char* type, char* args) {
    if (plugin_being_loaded_context) {
        append_rxpa_metadata(&plugin_being_loaded_metadata, "member", 0, 0, type, 0, owner, kind, member, args);
    }
    else {
        append_rxpa_metadata(&static_linked_metadata, "member", 0, 0, type, 0, owner, kind, member, args);
    }
}

static void loadPluginFileForFunctions(Context *context, char* file_name, char* location) {

    /* Update context */
    plugin_being_loaded = file_name;
    plugin_being_loaded_context = context;
    plugin_being_loaded_metadata = 0;

    // Create the rxpa_initctxptr context
    struct rxpa_initctxptr rxpa_context;
    rxpa_context.addfunc = rxpa_addfunc;
    rxpa_context.addclass = rxpa_addclass;
    rxpa_context.addinterface = rxpa_addinterface;
    rxpa_context.addimplements = rxpa_addimplements;
    rxpa_context.addmember = rxpa_addmember;
    rxpa_context.getstring = rxpa_getstring;
    rxpa_context.setstring = rxpa_setstring;
    rxpa_context.setint = rxpa_setint;
    rxpa_context.getint = rxpa_getint;
    rxpa_context.setfloat = rxpa_setfloat;
    rxpa_context.getfloat = rxpa_getfloat;
    rxpa_context.setnativepayload = rxpa_setnativepayload;
    rxpa_context.getnativepayload = rxpa_getnativepayload;
    rxpa_context.getnumattrs = rxpa_getnumattrs;
    rxpa_context.setnumattrs = rxpa_setnumattrs;
    rxpa_context.getattr = rxpa_getattr;
    rxpa_context.insertattr = rxpa_insertattr;
    rxpa_context.removeattr = rxpa_removeattr;
    rxpa_context.swapattrs = rxpa_swapattrs;
    rxpa_context.setsayexit = rxpa_setsayexit;
    rxpa_context.resetsayexit = rxpa_resetsayexit;

    if (context->debug_mode >= 2) printf("Importing Procedures - Reading CREXX Plugin file %s for possible procedure imports\n", file_name);

    // Load the plugin - and run the plugin initialization function
    int rc = load_plugin(&rxpa_context, location, file_name);
    if (!rc) {
        if (context->debug_mode >= 2) printf("Importing Procedures - CREXX Plugin %s loaded successfully\n", file_name);
        import_rxpa_metadata_list(context, file_name, plugin_being_loaded_metadata);
    }
    else {
        fprintf(stderr, "Importing Procedures - Failed to load plugin %s\n", file_name);
    }

    free_rxpa_metadata_list(&plugin_being_loaded_metadata);
}

static void parseRxasFileForFunctions(Context *context, char* file_name, char* location) {
    FILE *outFile;
    int token_type;
    Assembler_Token *token;
    bin_space *pgm;
    module_file module;

    Assembler_Context scanner;
    int i;

    /* Zero Context */
    memset(&scanner, 0, sizeof(Assembler_Context));

    /* scanner context parameter */
    scanner.debug_mode = 0;
    scanner.quiet = 1;
    scanner.traceFile = 0;
    scanner.optimise = 0;
    scanner.file_name = file_name;
    scanner.output_file_name = 0;
    scanner.location = location;

    if (context->debug_mode >= 2) printf("Importing Procedures - Reading RXAS file %s for possible procedure imports\n", file_name);

    /* Opening an Assembler file */
    if (rxasinfl(&scanner, 1)) {
        rxasclrc(&scanner);
        return;
    }

    /* Parse & Process */
    rxaspars(&scanner);
    read_constant_pool_for_functions(context, file_name, scanner.binary.const_pool,
                                     scanner.binary.const_size, scanner.meta_head);

    rxasclrc(&scanner);
}

static void parseRxbinFileForFunctions(Context *context, char* file_name, char* location) {
    FILE *fp;
    module_file *file_module_section;
    size_t modules_processed = 0;
    int loaded_rc;
    char *full_file_name;

    if (context->debug_mode >= 2) printf("Importing Procedures - Reading RXBIN file %s for possible procedure imports\n", file_name);

    fp = openfile(file_name, "", location, "rb");
    if (!fp) {
        if (context->debug_mode >= 2) printf("Importing Procedures - Could not open file\n");
        return;
    }

    loaded_rc = 0;
    while (loaded_rc == 0) {
        file_module_section = 0;
        switch (loaded_rc = read_module(&file_module_section, fp)) {
            case 0: /* Success */

                full_file_name = mprintf("%s@%s", file_module_section->name, file_name);

                read_constant_pool_for_functions(context, full_file_name, file_module_section->constant,
                                                 file_module_section->header.constant_size, file_module_section->header.meta_head);
                free(full_file_name);
                free_module(file_module_section);
                modules_processed++;
                break;

            case 1: /* eof */
                if (file_module_section) free_module(file_module_section);
                if (!modules_processed) {
                    if (context->debug_mode >= 2) printf("Importing Procedures - Empty file\n");
                    fclose(fp);
                    return;
                }
                break;

            default: /* error */
                if (file_module_section) free_module(file_module_section);
                if (context->debug_mode >= 2) printf("Importing Procedures - Error reading file\n");
                fclose(fp);
                return;
        }
    }

    fclose(fp);
}

static void parseRexxFileForFunctions(Context *parent_context, char* file_name, char* location,
                                      RexxLevel source_default_level) {
    size_t bytes;
    Context *context;
    char *buff_start;
    imported_func *global;
    size_t i;
    RexxLevel cli_level_override;

    if (parent_context->debug_mode >= 2) printf("Importing Procedures - Reading REXX file %s for possible procedure imports\n", file_name);

    /* Context for parsing */
    context = cntx_f();

    /* Open input file */
    context->file_pointer = openfile(file_name, "", location, "r");
    if (context->file_pointer == NULL) {
        if (parent_context->debug_mode >= 2) fprintf(stderr, "Warning: Importing Procedures - Can't open input file: %s from %s\n", file_name, location ? location : ".");
        free(context);
        return;
    }

    /* Propagate trace file */
#ifndef NDEBUG
    context->traceFile = parent_context->traceFile;
#endif

    buff_start = file2buf(context->file_pointer, &bytes);
    /* Close file */
    fclose(context->file_pointer);
    context->file_pointer  = 0;

    if (buff_start == NULL) {
        fprintf(stderr, "Importing Procedures - Can't read input file\n");
        exit(-1);
    }

    /* Initialize context */
    cntx_buf(context, buff_start, bytes);
    context->debug_mode = parent_context->debug_mode;
    context->location = parent_context->location;
    context->file_name = (char*) filename(file_name);
    context->disable_exits = parent_context->disable_exits;
    context->decimal_plugin = parent_context->decimal_plugin;
    cli_level_override = parent_context->master_context ?
                         parent_context->master_context->cli_level_override :
                         parent_context->cli_level_override;
    context->cli_level_override = cli_level_override;
    context->cli_default_level = cli_level_override != UNKNOWN ?
                                 cli_level_override :
                                 (source_default_level != UNKNOWN ?
                                  source_default_level :
                                  rxcp_source_default_level_for_file(file_name));

    /* Propagate the master_context */
    context->master_context = parent_context->master_context;

    /* Create Options parser to work out required language level */
    opt_pars(context);

    /* Deallocate memory and reset context */
    free_ast(context);
    free_tok(context);
    cntx_buf(context, buff_start, bytes);

    /* Parse program for real */
    switch (context->level){
        case LEVELA:
        case LEVELC:
        case LEVELD:
            if (parent_context->debug_mode >= 2) fprintf(stderr,"Importing Procedures - REXX Level A/C/D (cREXX Classic) - Not supported yet\n");
            break;

        case LEVELB:
        case LEVELG:
        case LEVELL:
            rexbpars(context);
            break;

        default:
            if (parent_context->debug_mode >= 2) fprintf(stderr, "Importing Procedures - Failed to determine REXX Level of imported rexx file\n");
    }

    if (!context->ast) {
        if (parent_context->debug_mode >= 2) fprintf(stderr,"Importing Procedures - Failure to create AST of imported rexx file\n");
        goto finish;
    }

    /* Recursion Guard - Check if already loading */
    for (i = 0; i < parent_context->master_context->loading_files_count; i++) {
        if (strcmp(parent_context->master_context->loading_files[i], file_name) == 0) {
            if (parent_context->debug_mode >= 2) printf("Importing Procedures - File %s already being loaded - skipping to avoid recursion\n", file_name);
            goto finish;
        }
    }

    /* Recursion Guard - Add to loading list */
    parent_context->master_context->loading_files_count++;
    parent_context->master_context->loading_files = realloc(parent_context->master_context->loading_files, sizeof(char*) * parent_context->master_context->loading_files_count);
    parent_context->master_context->loading_files[parent_context->master_context->loading_files_count - 1] = strdup(file_name);

    /* Extract Function Definitions  */
    rxcp_val(context);
    if (parent_context->optimise && !error_in_node(context->ast)) {
        context->optimise = parent_context->optimise;
        optimise(context);
    }

    /* Recursion Guard - Remove from loading list */
    free(parent_context->master_context->loading_files[parent_context->master_context->loading_files_count - 1]);
    parent_context->master_context->loading_files_count--;
    /* No need to realloc down, it's fine */

#ifndef __CMS__
    if (parent_context->debug_mode >= 2) {
//        pdot_tree(context->ast, "astimport", context->file_name);
    }
#endif

    ast_wlkr(context->ast, procedure_signature_walker, context);

    /* Extract Exposed Classes (signatures only) and register in the on-demand registry via a full AST walk */
    {
        class_import_payload pl;
        pl.parent_context = parent_context;
        pl.import_context = context;
        ast_wlkr(context->ast, class_signature_walker, &pl);
    }

    /* Extract Global Variables */
    /* Returns all the symbols in the scope of the PROGRAM_FILE node */
    Symbol **symbols = scp_syms(context->ast->child->scope);
    for (i=0; symbols[i]; i++) {
        if (symbols[i]->symbol_type == VARIABLE_SYMBOL && symbols[i]->exposed) {
            /* import symbol */
            char* fqname = sym_frnm(symbols[i]);
            rximpf_f(parent_context, file_name, fqname, 0, type_nm(symbols[i]->type), 0, 0, 1);
            free(fqname);
        }
    }
    free(symbols);

    finish:

    /* Free context */
#ifndef NDEBUG
    context->traceFile =  0;
#endif
    fre_cntx(context);
}

static char *default_import_namespace_from_file_name(const char *file_name) {
    const char *base_name;
    size_t stem_len;

    if (!file_name) return 0;
    base_name = filename(file_name);
    stem_len = module_stem_length(base_name);
    return rxcp_normalize_source_symbol_name(base_name, stem_len, 0, 0);
}

static void ensure_importable_source_header(importable_file *file, RexxLevel cli_level_override) {
    char *raw_namespace = 0;
    RexxLevel default_level;

    if (!file || file->type != REXX_FILE || file->header_scanned) return;

    default_level = cli_level_override != UNKNOWN ? cli_level_override : file->source_default_level;
    if (default_level == UNKNOWN) default_level = rxcp_source_default_level_for_file(file->name);
    rxcp_scan_source_header(file->location, file->name, default_level, 0, &raw_namespace);
    if (raw_namespace) {
        file->namespace_name = rxcp_normalize_source_symbol_name(raw_namespace, strlen(raw_namespace), 0, 1);
        free(raw_namespace);
    }
    if (!file->namespace_name) {
        file->namespace_name = default_import_namespace_from_file_name(file->name);
    }
    file->header_scanned = 1;
}

static int source_import_file_is_visible(Context *context, importable_file *file) {
    if (!file || file->type != REXX_FILE) return 1;

    ensure_importable_source_header(file,
                                    context && context->master_context ?
                                    context->master_context->cli_level_override :
                                    UNKNOWN);
    if (!file->namespace_name || !file->namespace_name[0]) return 1;
    if (!context || !context->ast || !context->ast->scope) return 1;

    return find_visible_namespace_scope(context, 0, file->namespace_name) != 0;
}

/* Parse and rxcp_val a rexx program in a string and return the context */
/* Parse and rxcp_val a rexx program in a string and return the context */
Context *rxcp_parse_buffer(char* rexx_source, int debug_mode) {
    Context *context;
    size_t bytes = strlen(rexx_source);
    char *buff = malloc(bytes + 1);
    memcpy(buff, rexx_source, bytes);
    buff[bytes] = 0;

    context = cntx_f();
    cntx_buf(context, buff, bytes);

    /* Initialize context */
    context->level = LEVELB;
    context->debug_mode = debug_mode;
    context->master_context = context;
    context->file_name = "fragment";

    rexbpars(context);

    return context;
}

static Context *parseRexx(Context* parent_context, char *location, char* file_name, RexxLevel level, int debug_mode, char* rexx_source, size_t bytes) {
    Context *context;

    switch (level){
        case LEVELA:
        case LEVELC:
        case LEVELD:
            fprintf(stderr,"INTERNAL ERROR: Importing Procedures - REXX Level A/C/D (cREXX Classic) - Not supported yet\n");
            return 0;

        case LEVELB:
        case LEVELG:
        case LEVELL:
            context = cntx_f();
            cntx_buf(context, rexx_source, bytes);

            /* Initialize context */
            context->location = location;
            context->file_name = file_name;
            context->level = level;
            context->debug_mode = debug_mode;
            context->master_context = parent_context->master_context;
            context->disable_exits = parent_context->disable_exits;
            context->decimal_plugin = parent_context->decimal_plugin;

            rexbpars(context);
            break;

        default:
            fprintf(stderr, "INTERNAL ERROR: Importing Procedures - Failed to determine REXX Level\n");
            return 0;
    }

    if (!context->ast) {
        if (debug_mode >= 2) fprintf(stderr,"INTERNAL ERROR: Importing Procedures - Compiler Exiting - Failure to create AST\n");
        goto finish;
    }

    rxcp_bvl(context);

#ifndef __CMS__
//    if (debug_mode >= 2) {
//        pdot_tree(context->ast, "parserexx.dot");
//        /* Get dot from https://graphviz.org/download/ */
//        system("dot parserexx.dot -Tpng -o parserexx.png");
//    }
#endif

    finish:

    return context;
}

/* Load the next importable file */
/* return 1 if a file was loaded, or 0 if no more files are available */
static int load_another_file(Context *context) {
    size_t f;
    Context *master_context;

    if (!context) return 0;
    master_context = context->master_context;
    if (!master_context) return 0;

    /* Load files from the importable_file_list */
    if (!master_context->importable_file_list) master_context->importable_file_list = rxfl_lst(master_context);
    if (!master_context->importable_file_list) {
        /* No files to import */
        return 0;
    }

    if (context->debug_mode >= 2) {
        fprintf(stderr, "Importable file list state (context file: %s):\n", context->file_name);
        for (f = 0; master_context->importable_file_list[f]; f++) {
            fprintf(stderr, " - file %zu: %s (imported: %d)\n", f, master_context->importable_file_list[f]->name, master_context->importable_file_list[f]->imported);
        }
    }

    for (f = 0; master_context->importable_file_list[f]; f++) {
        /* Already imported? */
        if (!master_context->importable_file_list[f]->imported) {
            if (master_context->importable_file_list[f]->type == REXX_FILE &&
                !source_import_file_is_visible(context, master_context->importable_file_list[f])) {
                continue;
            }
            master_context->importable_file_list[f]->imported = 1;

            /* Import File */
            if (context->debug_mode >= 2) fprintf(stderr, "Importing Procedure/Classes - Loading file %s from %s\n",
                                                  master_context->importable_file_list[f]->name,
                                                  master_context->importable_file_list[f]->location);
            switch (master_context->importable_file_list[f]->type) {
                case REXX_FILE:
                    parseRexxFileForFunctions(context,
                                              master_context->importable_file_list[f]->name,
                                              master_context->importable_file_list[f]->location,
                                              master_context->importable_file_list[f]->source_default_level);
                    break;
                case RXBIN_FILE:
                    parseRxbinFileForFunctions(context,
                                               master_context->importable_file_list[f]->name,
                                               master_context->importable_file_list[f]->location);
                    break;
                case RXAS_FILE:
                    parseRxasFileForFunctions(context,
                                              master_context->importable_file_list[f]->name,
                                              master_context->importable_file_list[f]->location);
                    break;
                case NATIVE_FILE:
                    loadPluginFileForFunctions(context,
                                              master_context->importable_file_list[f]->name,
                                              master_context->importable_file_list[f]->location);
                    break;
            }
            return 1; /* File loaded - job done */
        }
    }

    // Process any statically linked functions
    if (static_linked_functions || static_linked_metadata) {
        struct static_linked_function *functions = static_linked_functions;
        struct static_linked_function *static_func = functions;
        struct static_linked_metadata *metadata = static_linked_metadata;

        static_linked_functions = 0;
        static_linked_metadata = 0;

        import_rxpa_metadata_list(context, "statically-linked", metadata);

        while (static_func) {
            rximpf_f(context, "statically-linked",
                     static_func->name, static_func->option, static_func->type,
                     static_func->args, 0, 0);
            static_func = static_func->next;
        }

        // Free the list of statically linked functions -  we only load these once
        static_linked_functions = functions;
        static_linked_metadata = metadata;
        free_static_linked_functions();
        return 1;
    }

    if (context->debug_mode >= 2) fprintf(stderr, "Importing Procedure/Classes - No more files to load\n");
    return 0; /* No file was loaded */
}

static ValueType type_from_string(char* type) {
    if (!type) return TP_UNKNOWN;
    if (strcmp(type, ".int") == 0) return TP_INTEGER;
    if (strcmp(type, ".float") == 0) return TP_FLOAT;
    if (strcmp(type, ".decimal") == 0) return TP_DECIMAL;
    if (strcmp(type, ".string") == 0) return TP_STRING;
    if (strcmp(type, ".binary") == 0) return TP_BINARY;
    if (strcmp(type, ".boolean") == 0) return TP_BOOLEAN;
    if (strcmp(type, ".void") == 0) return TP_VOID;
    if (strcmp(type, ".unknown") == 0) return TP_UNKNOWN;
    return TP_OBJECT;
}

static int contract_label_matches(const ASTNode *node, const char *short_name) {
    size_t start;
    size_t len;
    size_t i;

    if (!node || !node->node_string || !short_name) return 0;

    start = 0;
    len = node->node_string_length;

    if (len > 0 && node->node_string[len - 1] == ':') len--;

    while (start < len && isspace((unsigned char)node->node_string[start])) start++;
    while (len > start && isspace((unsigned char)node->node_string[len - 1])) len--;

    if (len - start != strlen(short_name)) return 0;

    for (i = 0; i < len - start; i++) {
        if (tolower((unsigned char)node->node_string[start + i]) !=
            tolower((unsigned char)short_name[i])) {
            return 0;
        }
    }

    return 1;
}

static int ast_declares_local_contract(ASTNode *node, const char *short_name) {
    for (; node; node = node->sibling) {
        if ((node->node_type == CLASS_DEF || node->node_type == INTERFACE_DEF) &&
            contract_label_matches(node, short_name)) {
            return 1;
        }

        if (node->child && ast_declares_local_contract(node->child, short_name)) return 1;
    }

    return 0;
}

static int current_source_declares_contract(Context *context, const char *short_name) {
    char *namespace_name = 0;
    char *local_name = 0;
    int result;

    if (!context || !context->ast || !short_name || !*short_name) return 0;

    if (rxcp_split_internal_symbol_name(short_name, &namespace_name, &local_name)) {
        if (!find_visible_namespace_scope(context, 1, namespace_name)) {
            free(namespace_name);
            free(local_name);
            return 0;
        }
        free(namespace_name);
        short_name = local_name;
    }

    /* Only scan the current source tree, not imported sibling modules. */
    result = ast_declares_local_contract(context->ast->child, short_name);
    if (local_name) free(local_name);
    return result;
}

/* Try and import an external class - return its symbol if successful */
Symbol *sym_imcls(Context *context, ASTNode *node) {
    struct imported_class *found_cls = 0;
    Symbol *found_symbol = 0;
    char *name;
    name = rxcp_normalize_source_symbol_name(node->node_string, node->node_string_length, 1, 1);
    if (!name) return 0;

    /* Do not import a stale external artifact for a contract declared later in this source file. */
    if (current_source_declares_contract(context, name)) {
        found_symbol = lookup_loaded_symbol(context, name);
        if (found_symbol && found_symbol->defines_scope && found_symbol->defines_scope->defining_node &&
            found_symbol->defines_scope->defining_node->context == context) {
            free(name);
            return found_symbol;
        }
        free(name);
        return 0;
    }

    /* Check if the class is already in the master AST */
    found_symbol = lookup_loaded_symbol(context, name);
    if (found_symbol && found_symbol->symbol_type == CLASS_SYMBOL && found_symbol->exposed) {
        free(name);
        return found_symbol;
    }

    if (context->debug_mode >= 2) fprintf(stderr, "Importing Class for file %s Looking for Class %s\n", context->file_name, name);

    /* Process all the unread files */
    do {
        /* Check if the class has been loaded */
        if (src_fqcl(context, name, &found_cls)) {
            if (context->debug_mode >= 2)
                fprintf(stderr, "Importing Classes - Found Class %s in file %s\n", found_cls->fqname, found_cls->file_name);
            break;
        }
    } while (load_another_file(context));

    if (context->debug_mode >= 2) fprintf(stderr, "Finished Importing files needed for file %s when Looking for Class %s\n", context->file_name, name);

    if (found_cls) {
        found_symbol = load_imported_contract(context, found_cls);
    }

    free(name);
    return found_symbol;
}

/* Try and import an external class by name - return its symbol if successful */
Symbol *ensure_class_imported(Context *context, const char *class_name, size_t class_name_length) {
    ASTNode lookup_node;

    if (!context || !class_name || !class_name_length) return 0;

    memset(&lookup_node, 0, sizeof(ASTNode));
    lookup_node.node_string = (char*)class_name;
    lookup_node.node_string_length = class_name_length;

    return sym_imcls(context, &lookup_node);
}

/* Check if a symbol is an importable Variable - return 1 if it is, 0 otherwise */
int sym_is_glob_var(Context *context, ASTNode *node) {
    imported_func *func;
    char *name;
    int found = 0;
    name = rxcp_normalize_source_symbol_name(node->node_string, node->node_string_length, 1, 1);
    if (!name) return 0;

    /* Check unread files */
    if (src_fqfu(context, 0, name, &func)) {
        if (func->is_variable) found = 1;
    }

    free(name);
    return found;
}

/* Check if a class is importable - return 1 if it is, 0 otherwise */
int sym_is_imcls(Context *context, ASTNode *node) {
    struct imported_class *found_cls = 0;
    char *name;
    int found = 0;
    name = rxcp_normalize_source_symbol_name(node->node_string, node->node_string_length, 1, 1);
    if (!name) return 0;

    if (current_source_declares_contract(context, name)) {
        free(name);
        return 0;
    }

    if (src_fqcl(context, name, &found_cls)) {
        found = 1;
    }

    free(name);
    return found;
}

/* Check if a function is importable - return 1 if it is a function, 0 otherwise */
int sym_is_imfn(Context *context, ASTNode *node) {
    imported_func *func;
    char *name;
    int found = 0;
    name = rxcp_normalize_source_symbol_name(node->node_string, node->node_string_length, 1, 1);
    if (!name) return 0;

    /* sym_rvfn check removed: locally defined functions should not trigger shadowing warnings for themselves */

    /* Check unread files */
    if (src_fqfu(context, 0, name, &func)) {
        if (!func->is_variable) found = 1;
    }

    free(name);
    return found;
}

/* Try and import an external function - return its symbol if successful */
Symbol *sym_imfn(Context *context, ASTNode *node) {
    Symbol *symbol;
    ASTNode *func_node;
    Symbol *func_symbol;
    Symbol *found_symbol = 0;
    Scope *namespace;
    imported_func *func;
    imported_func *found_func = 0;
    imported_func *inconsistent_func;
    ValueType tp;
    char error = 0;
    char* defining_file = context->file_name;
    char *name;
    name = rxcp_normalize_source_symbol_name(node->node_string, node->node_string_length, 1, 1);
    if (!name) return 0;

    /* Check if the function is already in the master AST */
    found_symbol = lookup_loaded_symbol(context, name);
    if (found_symbol && found_symbol->symbol_type == FUNCTION_SYMBOL && found_symbol->exposed) {
        free(name);
        return found_symbol;
    }

    if (context->debug_mode >= 2) printf("Importing Procedures for file %s Looking for Procedure %s\n", defining_file, name);

    /* Process all the unread files - but we just are interested in the first found variable - duplicates done next */
    do {
        /* Check if the function has been loaded */
        if (src_fqfu(context, 0, name, &func)) {
            if (context->debug_mode >= 2)
                printf("Importing Procedures - Found Procedure %s in file %s\n", func->fqname, func->file_name);
            found_func = func;
            break;
        }
    } while (load_another_file(context));

    if (context->debug_mode >= 2) printf("Finished Importing files needed for file %s when Looking for Procedure %s\n", defining_file, name);

    if (found_func) {
        /* Check if it's already in the master AST by its fully qualified name */
        found_symbol = sym_rfqn(context->ast, found_func->fqname);
        if (found_symbol && found_symbol->symbol_type == FUNCTION_SYMBOL && found_symbol->exposed) {
            free(name);
            return found_symbol;
        }

        /* Compare found variable with the type defined in the master file being compiled */
        tp = type_from_string(found_func->type);
        if (found_func->is_variable) {
            mknd_err(node,
                     "PROC_VAR_MISMATCH, \"%s\", \"%s\", \"%s\"",
                     name,found_func->file_name, defining_file);
            error = 1;
        }

        /* Has the func got an error_state? */
        if (found_func->error_state) {
            mknd_err(node,
                     "%s, \"%s\", \"%s\"",
                     found_func->error_state, name, found_func->file_name);
            error = 1;
        }

        /* Produce Errors for all import inconsistencies */
        inconsistent_func = found_func->duplicate;
        while (inconsistent_func) {
            if (inconsistent_func->is_variable) {
                mknd_err(node,
                         "PROC_VAR_MISMATCH, \"%s\", \"%s\", \"%s\"",
                         name,inconsistent_func->file_name, defining_file);
                error = 1;
            }

            if (safe_strcmp(found_func->type, inconsistent_func->type)) {
                mknd_err(node,
                         "TYPE_MISMATCH, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
                         name,found_func->type, found_func->file_name, inconsistent_func->type,
                         inconsistent_func->file_name);
                error = 1;
            }

            if (safe_strcmp(found_func->args, inconsistent_func->args)) {
                mknd_err(node,
                         "ARGS_MISMATCH, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
                         name,found_func->args, found_func->file_name, inconsistent_func->args,
                         inconsistent_func->file_name);
                error = 1;
            }

            inconsistent_func = inconsistent_func->duplicate;
        }
        if (!error) {

            if (context->debug_mode >= 2)
                printf("Importing Procedures - Found Procedure %s\n", found_func->fqname);

            /* Splice the ASTs together */
            ASTNode *new_stub = add_dast(context->ast, func->context->ast->child);
            context->changed_flags |= FLAG_FUNC;

            /* Build symbols for the new stub immediately so it can be resolved */
            Scope *old_scope = context->current_scope;
            context->current_scope = context->ast->scope;
            ast_wlkr(new_stub, build_symbols_walker, context);
            context->current_scope = old_scope;

            found_symbol = sym_rfqn(context->ast, found_func->fqname);
            if (found_symbol) {
                found_symbol->exposed = 1; /* Exposed by definition! */
                found_symbol->status = SYM_STATUS_RESOLVED_GLOBAL;
                found_symbol->is_arg = 0; /* Can't expose args */
                found_symbol->is_opt_arg = 0;
                found_symbol->is_ref_arg = 0;
                found_symbol->is_const_arg = 0;
                if (rxcp_inline_payload_is_supported(found_func->implementation)) {
                    rxcp_inline_attach_imported_symbol(context, found_symbol, found_func->implementation);
                }
            }
        }
    }

    free(name);
    return found_symbol;
}

/* Set the type of a symbol from imported modules */
void sym_imva(Context *context, Symbol *symbol) {
    imported_func *var;
    imported_func *found_var = 0;
    imported_func *inconsistent_var;
    ValueType tp;
    char error = 0;
    char* defining_file = 0;

    if (context->debug_mode >= 2) printf("Importing Globals - Looking for Global %s\n", symbol->name);

    defining_file = context->file_name;

    /* Process all the unread files - but we just are interested in the first found variable - duplicated done next */
    do {
        if (!found_var) {
            /* Check if the function has been loaded */
            if (src_fqfu(context, 0, symbol->name, &var)) {
                if (context->debug_mode >= 2) printf("Importing Globals - Found Global Symbol %s\n", var->fqname);
                found_var = var;
            }
        }
    } while (load_another_file(context));

    if (found_var) {
        /* Compare found variable with the type defined in the master file being compiled */
        tp = type_from_string(found_var->type);
        if (!found_var->is_variable) {
            mknd_err(sym_trnd(symbol, 0)->node, "PROC_VAR_MISMATCH, \"%s\", \"%s\", \"%s\"",
                     symbol->name, defining_file, found_var->file_name);
            error = 1;
        }
        else if (symbol->type != TP_UNKNOWN) {
            if ((tp != TP_UNKNOWN) && (tp != symbol->type)) {
                mknd_err(sym_trnd(symbol, 0)->node, "TYPE_MISMATCH, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
                         symbol->name,type_nm(symbol->type), defining_file, found_var->type, found_var->file_name);
                error = 1;
            }
        }

        /* Produce Errors for all import inconsistencies */
        inconsistent_var = found_var->duplicate;
        while (inconsistent_var) {
            if (!inconsistent_var->is_variable) {
                mknd_err(sym_trnd(symbol, 0)->node, "PROC_VAR_MISMATCH, \"%s\", \"%s\", \"%s\"",
                         inconsistent_var->name, defining_file, inconsistent_var->file_name);
                error = 1;
            }

            if (safe_strcmp(found_var->type, inconsistent_var->type)) {
                mknd_err(sym_trnd(symbol, 0)->node,
                         "TYPE_MISMATCH, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
                         found_var->name, found_var->type, found_var->file_name, inconsistent_var->type, inconsistent_var->file_name);
                error = 1;
            }
            inconsistent_var = inconsistent_var->duplicate;
        }
        if (!error) {
            symbol->type = tp;
            symbol->status = SYM_STATUS_RESOLVED_GLOBAL;
        }
    }
}

/* imported_func factory - returns null if the function is not in an applicable namespace or is a duplicate */
imported_func *rximpf_f(Context* context, char* file_name, char *fqname, char *options, char *type, char *args,
                        char *implementation, char is_variable)  {
    imported_func *func;
    char *buffer;
    char *name;

    if (!fqname) return 0;

    /* Get the namespace (string before the last ".") from the fqn */
    size_t len  = strlen(fqname);
    while (len) {
        len--;
        if (fqname[len] == '.') break;
    }

    name = fqname + len + 1;

    /* OK - Create func */
    func = malloc(sizeof(imported_func));
    func->context = 0;
    func->is_variable = is_variable; /* Is a function or a Variable */
    func->duplicate = 0;
    func->error_state = 0;

    /* Store the namespace */
    func->namespace = malloc(len + 1);
    memcpy(func->namespace, fqname, len);
    func->namespace[len] = 0;

    /* Store the file name */
    func->file_name = malloc(strlen(file_name) + 1);
    strcpy(func->file_name,file_name);

    /* Store fqname */
    func->fqname = malloc(strlen(fqname) + 1);
    strcpy(func->fqname,fqname);

    /* Store the rest of the fields */
    if (name) {
        func->name = malloc(strlen(name) + 1);
        strcpy(func->name,name);
    }
    else func->name = 0;

    if (options) {
        func->options = malloc(strlen(options) + 1);
        strcpy(func->options,options);
    }
    else func->options = 0;

    if (type) {
        func->type = malloc(strlen(type) + 1);
        strcpy(func->type,type);
    }
    else func->type = 0;

    if (args) {
        func->args = malloc(strlen(args) + 1);
        strcpy(func->args,args);
    }
    else func->args = 0;

    if (implementation) {
        func->implementation = malloc(strlen(implementation) + 1);
        strcpy(func->implementation,implementation);
    }
    else func->implementation = 0;

    if (func->is_variable == 0) {
        /* Generate Function Declaration AST - only if we are a function not a variable */
        // TODO This only does Level B
        char *source_args = metadata_args_to_source_args(func->args);
        if (!source_args) source_args = strdup("");
        buffer = mprintf("options levelb\nnamespace %s\n%s: procedure = %s\narg %s\n", func->namespace, func->name,
                         func->type, source_args);
        free(source_args);
        if (context->debug_mode >= 2) printf("Importing Procedures - Analysing procedure %s\n", func->fqname);
        func->context = parseRexx(context, context->location, func->file_name, LEVELB, context->debug_mode, buffer,
                                  strlen(buffer));

        if (!func->context || error_in_node(func->context->ast)) {
            func->error_state = "INTERNAL_ERROR_PARSING_IMPORT_AST";
        }

        /* Make sure the AST seems sane */
        else if (func->context->ast->child->node_type != PROGRAM_FILE) {
            func->error_state = "INTERNAL_ERROR_PARSING_IMPORT_AST";
        }

        /* Fixup the node type */
        func->context->ast->child->node_type = IMPORTED_FILE;

        if (func->implementation && rxcp_inline_payload_is_supported(func->implementation)) {
            rxcp_inline_attach_imported_body(func->context, func->implementation);
        }
    }

    if (add_func(context, func)) {
        /* Duplicate */
        func = 0;
    }

    return func;
}

/* Free an imported_func */
void freimpfc(imported_func *func) {
    if (func->namespace) free(func->namespace);
    if (func->file_name) free(func->file_name);
    if (func->fqname) free(func->fqname);
    if (func->name) free(func->name);
    if (func->options) free(func->options);
    if (func->type) free(func->type);
    if (func->args) free(func->args);
    if (func->implementation) free(func->implementation);
    if (func->context) fre_cntx(func->context);
    if (func->duplicate) freimpfc(func->duplicate);
    free(func);
}

/* free the list of importable files */
void rxfl_fre(importable_file **file_list) {
    importable_file **file;
    for (file = file_list; *file; file++) {
        free((*file)->name);
        if ((*file)->location) free((*file)->location);
        if ((*file)->namespace_name) free((*file)->namespace_name);
        free(*file);
    }
    free(file_list);
}

static void add_file_to_list(importable_file *file, size_t *number, importable_file ***list) {
    (*number)++;
    *list = (importable_file**)realloc(*list, ((*number) + 1) * sizeof(importable_file*));
    (*list)[(*number)-1] = file;
    (*list)[*number] = 0;
}

static size_t module_stem_length(const char *name) {
    const char *dot;

    if (!name) return 0;
    dot = strrchr(name, '.');
    if (!dot) return strlen(name);
    return (size_t)(dot - name);
}

static int module_name_equals(const char *left, const char *right) {
    size_t left_len;
    size_t right_len;
    size_t i;

    if (!left || !right) return 0;

    left_len = module_stem_length(left);
    right_len = module_stem_length(right);
    if (left_len != right_len) return 0;

    for (i = 0; i < left_len; i++) {
        if (tolower((unsigned char)left[i]) != tolower((unsigned char)right[i])) return 0;
    }
    return 1;
}

static int importable_file_stage(const importable_file *file) {
    if (!file) return 0;
    if (file->type == NATIVE_FILE) return 3;
    if (file->source_root) return 1;
    return 2;
}

static int importable_type_preference(file_type type) {
    switch (type) {
        case REXX_FILE: return 3;
        case RXBIN_FILE: return 2;
        case RXAS_FILE: return 1;
        default: return 0;
    }
}

static int list_has_module_for_stage(importable_file **list, int stage, const char *name) {
    size_t i;

    if (!list || !name) return 0;
    for (i = 0; list[i]; i++) {
        if (importable_file_stage(list[i]) != stage) continue;
        if (module_name_equals(list[i]->name, name)) return 1;
    }
    return 0;
}

static int should_replace_importable_candidate(const importable_file *existing, const importable_file *candidate) {
    if (!existing || !candidate) return 0;
    if (candidate->mtime != existing->mtime) return candidate->mtime > existing->mtime;
    return importable_type_preference(candidate->type) > importable_type_preference(existing->type);
}

static char *join_importable_path(const char *directory, const char *name) {
    size_t dir_len;
    size_t name_len;
    int needs_sep;
    char *full_path;

    if (!name || !name[0]) return 0;
    if (!directory || !directory[0]) return strdup(name);

    dir_len = strlen(directory);
    name_len = strlen(name);
    needs_sep = directory[dir_len - 1] != '/' && directory[dir_len - 1] != '\\';

    full_path = malloc(dir_len + needs_sep + name_len + 1);
    if (!full_path) return 0;
    memcpy(full_path, directory, dir_len);
    if (needs_sep) full_path[dir_len++] = '/';
    memcpy(full_path + dir_len, name, name_len);
    full_path[dir_len + name_len] = 0;
    return full_path;
}

static time_t read_importable_mtime(const char *directory, const char *name) {
    struct stat st;
    char *path;
    time_t mtime;

    path = join_importable_path(directory, name);
    if (!path) return 0;
    if (stat(path, &st) == 0) mtime = st.st_mtime;
    else mtime = 0;
    free(path);
    return mtime;
}

static importable_file* importable_file_f(char* name, file_type type, char *location) {
    importable_file *file;
    file = malloc(sizeof(importable_file));
    file->name = malloc(strlen(name) + 1);
    strcpy(file->name, name);
    if (location) {
        file->location = malloc(strlen(location) + 1);
        strcpy(file->location, location);
    }
    else file->location = 0;
    file->imported = 0;
    file->source_root = 0;
    file->source_default_level = UNKNOWN;
    file->mtime = read_importable_mtime(location, name);
    file->namespace_name = 0;
    file->header_scanned = 0;
    file->type = type;
    return file;
}

static void free_importable_file(importable_file *file) {
    if (!file) return;
    free(file->name);
    if (file->location) free(file->location);
    if (file->namespace_name) free(file->namespace_name);
    free(file);
}

/* Get a list of files of a type in a directory (can be null), skipping skip_name (can be null) */
static void list_files_in_dir(char *directory, file_type type, char* skip_name, char *skip_module,
                              importable_file ***list, size_t *number, int debug_mode, char source_root) {

    void *dir_ptr;
    char* name;
    importable_file *file;
    char* type_name;
    char *file_prefix = 0;

    switch (type) {
        case REXX_FILE:
            type_name = "rexx";
            break;
        case RXAS_FILE:
            type_name = "rxas";
            break;
        case RXBIN_FILE:
            type_name = "rxbin";
            break;
        case NATIVE_FILE:
            type_name = "rxplugin";
            file_prefix = "rx";
            break;
        default:
            return;
    }

    name = dirfstfl(directory, file_prefix, type_name, &dir_ptr);
    while (name) {
        if ((!skip_name || strcmp(name, skip_name) != 0) &&
            (!skip_module || !module_name_equals(name, skip_module))) {
            if (debug_mode >= 2) fprintf(stderr, "Found importable %s file: %s in %s\n", type_name, name, directory);
            file = importable_file_f(name, type, directory);
            if (file) {
                file->source_root = source_root;
                add_file_to_list(file, number, list);
            }
        }
        name = dirnxtfl(&dir_ptr);
    }
    dirclose(&dir_ptr);
}

static void list_source_files_in_dir(char *directory, const char *extension, RexxLevel default_level,
                                     char* skip_name, char *skip_module,
                                     importable_file ***list, size_t *number, int debug_mode, char source_root) {

    void *dir_ptr;
    char* name;
    importable_file *file;

    if (!extension || !extension[0]) return;

    name = dirfstfl(directory, 0, (char*) extension, &dir_ptr);
    while (name) {
        if ((!skip_name || strcmp(name, skip_name) != 0) &&
            (!skip_module || !module_name_equals(name, skip_module))) {
            if (debug_mode >= 2) fprintf(stderr, "Found importable source file: %s in %s\n", name, directory);
            file = importable_file_f(name, REXX_FILE, directory);
            if (file) {
                file->source_root = source_root;
                file->source_default_level = default_level;
                add_file_to_list(file, number, list);
            }
        }
        name = dirnxtfl(&dir_ptr);
    }
    dirclose(&dir_ptr);
}

static importable_file *find_stage_module(importable_file **list, int stage, const char *name) {
    size_t i;

    if (!list || !name) return 0;
    for (i = 0; list[i]; i++) {
        if (importable_file_stage(list[i]) != stage) continue;
        if (module_name_equals(list[i]->name, name)) return list[i];
    }
    return 0;
}

static void add_unique_stage_file(importable_file ***list, size_t *number, importable_file *file) {
    int stage;

    if (!list || !number || !file) return;
    stage = importable_file_stage(file);
    if (find_stage_module(*list, stage, file->name)) {
        free_importable_file(file);
        return;
    }
    add_file_to_list(file, number, list);
}

static void collect_root_files(char *directory, file_type type, char *skip_name, char *skip_module,
                               importable_file ***list, size_t *number, int debug_mode, char source_root) {
    importable_file **root_list;
    size_t root_count;
    size_t i;

    root_list = malloc(sizeof(importable_file *));
    if (!root_list) return;
    root_list[0] = 0;
    root_count = 0;

    list_files_in_dir(directory, type, skip_name, skip_module, &root_list, &root_count, debug_mode, source_root);
    for (i = 0; i < root_count; i++) {
        add_unique_stage_file(list, number, root_list[i]);
    }
    free(root_list);
}

static void collect_source_root_files(char *directory, const RxcpSourceExtension *extension,
                                      char *skip_name, char *skip_module,
                                      importable_file ***list, size_t *number, int debug_mode, char source_root) {
    importable_file **root_list;
    size_t root_count;
    size_t i;

    if (!extension || !extension->extension) return;

    root_list = malloc(sizeof(importable_file *));
    if (!root_list) return;
    root_list[0] = 0;
    root_count = 0;

    list_source_files_in_dir(directory, extension->extension, extension->default_level,
                             skip_name, skip_module, &root_list, &root_count, debug_mode, source_root);
    for (i = 0; i < root_count; i++) {
        add_unique_stage_file(list, number, root_list[i]);
    }
    free(root_list);
}

static void collect_binary_root_files(char *directory, char *skip_module, importable_file ***list,
                                      size_t *number, int debug_mode, int auto_import_rxas) {
    importable_file **root_list;
    size_t root_count;
    size_t i;

    root_list = malloc(sizeof(importable_file *));
    if (!root_list) return;
    root_list[0] = 0;
    root_count = 0;

    list_files_in_dir(directory, RXBIN_FILE, 0, skip_module, &root_list, &root_count, debug_mode, 0);
    if (auto_import_rxas) {
        importable_file **rxas_list;
        size_t rxas_count;

        rxas_list = malloc(sizeof(importable_file *));
        if (!rxas_list) {
            for (i = 0; i < root_count; i++) free_importable_file(root_list[i]);
            free(root_list);
            return;
        }
        rxas_list[0] = 0;
        rxas_count = 0;
        list_files_in_dir(directory, RXAS_FILE, 0, skip_module, &rxas_list, &rxas_count, debug_mode, 0);

        for (i = 0; i < rxas_count; i++) {
            importable_file *existing = find_stage_module(root_list, 2, rxas_list[i]->name);
            if (existing) {
                if (should_replace_importable_candidate(existing, rxas_list[i])) {
                    size_t j;
                    for (j = 0; root_list[j]; j++) {
                        if (root_list[j] == existing) {
                            root_list[j] = rxas_list[i];
                            free_importable_file(existing);
                            break;
                        }
                    }
                } else {
                    free_importable_file(rxas_list[i]);
                }
            } else {
                add_file_to_list(rxas_list[i], &root_count, &root_list);
            }
        }
        free(rxas_list);
    }

    for (i = 0; i < root_count; i++) {
        add_unique_stage_file(list, number, root_list[i]);
    }
    free(root_list);
}

/* Get the list of importable files as a null terminated malloced array */
importable_file **rxfl_lst(Context *context) {
    size_t number = 0;
    importable_file **list = 0;
    char *skip_module;
    size_t d;
    RxcpSourceExtension source_extensions[RXCP_SOURCE_EXTENSION_MAX];
    size_t source_extension_count;
    size_t e;

    list = malloc(sizeof(importable_file *));
    if (!list) return 0;
    list[0] = 0;
    skip_module = context ? context->file_name : 0;
    source_extension_count = rxcp_source_extension_list(context ? context->initial_source_extension : 0,
                                                        source_extensions);

    if (context->debug_mode >= 2) {
        fprintf(stderr, "Scanning source import roots. Primary source root: %s\n", context->location ? context->location : ".");
    }

    for (e = 0; e < source_extension_count; e++) {
        collect_source_root_files(context->location, &source_extensions[e], context->file_name, skip_module,
                                  &list, &number, context->debug_mode, 1);
    }
    if (context->source_import_locations) {
        for (d = 0; context->source_import_locations[d]; d++) {
            if (context->debug_mode >= 2) fprintf(stderr, "Scanning source import root: %s\n", context->source_import_locations[d]);
            for (e = 0; e < source_extension_count; e++) {
                collect_source_root_files(context->source_import_locations[d], &source_extensions[e], 0, skip_module,
                                          &list, &number, context->debug_mode, 1);
            }
        }
    }

    if (context->import_locations) {
        for (d = 0; context->import_locations[d]; d++) {
            if (context->debug_mode >= 2) fprintf(stderr, "Scanning binary import root: %s\n", context->import_locations[d]);
            collect_binary_root_files(context->import_locations[d], skip_module, &list, &number, context->debug_mode, context->auto_import_rxas);
        }
    }

    if (context->import_locations) {
        for (d = 0; context->import_locations[d]; d++) {
            collect_root_files(context->import_locations[d], NATIVE_FILE, 0, skip_module, &list, &number, context->debug_mode, 0);
        }
    }

    if (context->debug_mode >= 2) fprintf(stderr, "Scanning for importable files finished. Found %zu files.\n", number);
    return list;
}

// Free statically linked functions list
void free_static_linked_functions()
{
    while (static_linked_functions) {
        struct static_linked_function *next = static_linked_functions->next;
        free(static_linked_functions);
        static_linked_functions = next;
    }
    free_rxpa_metadata_list(&static_linked_metadata);
}

/* Public API: eagerly scan importable files to populate functions and register classes */
int rxcp_scan_imports(Context *context)
{
    int loaded = 0;
    if (!context) return 0;
    while (load_another_file(context)) {
        loaded = 1;
    }
    return loaded;
}
