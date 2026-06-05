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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxcp_emit.h"
#include "rxcp_val.h"
#include "rxcp_util.h"

static int symbol_has_live_node_in_proc(Symbol *symbol, ASTNode *proc_node) {
    size_t i;

    if (symbol && symbol->name && strncmp(symbol->name, "__inline_", 9) == 0) return 0;

    for (i = 0; i < sym_nond(symbol); i++) {
        SymbolNode *sn = sym_trnd(symbol, i);
        ASTNode *n = sn ? sn->node : NULL;

        while (n) {
            if (n == proc_node) return 1;
            n = n->parent;
        }
    }

    return 0;
}

static int symbol_is_exit_token_only_in_proc(Symbol *symbol, ASTNode *proc_node) {
    size_t i;
    int saw_live_node = 0;

    for (i = 0; i < sym_nond(symbol); i++) {
        SymbolNode *sn = sym_trnd(symbol, i);
        ASTNode *n = sn ? sn->node : NULL;
        ASTNode *walk = n;

        while (walk) {
            if (walk == proc_node) {
                saw_live_node = 1;
                if (!n || n->node_type != EXIT_TOKEN) return 0;
                break;
            }
            walk = walk->parent;
        }
    }

    return saw_live_node;
}

static int node_is_imported_definition(ASTNode *node) {
    while (node) {
        if (node->node_type == IMPORTED_FILE) return 1;
        if (node->node_type == PROGRAM_FILE) return 0;
        node = node->parent;
    }
    return 0;
}

static const char *interface_member_kind_string(ASTNode *member) {
    if (!member) return "";
    if (member->node_type == FACTORY) return "factory";
    if (member->node_type == METHOD) {
        if (member->is_interface_default_method) return "method final";
        return "method";
    }
    return "";
}

static Symbol *resolve_metadata_class_name(Context *context, const char *class_name) {
    Symbol *symbol = 0;

    if (!context || !context->ast || !class_name || !*class_name) return 0;

    if (strchr(class_name, '.')) symbol = sym_rfqv(context->ast, class_name);
    else symbol = sym_rvfn(context->ast, (char *)class_name);

    if (!symbol) {
        ensure_class_imported(context, class_name, strlen(class_name));
        if (strchr(class_name, '.')) symbol = sym_rfqv(context->ast, class_name);
        else symbol = sym_rvfn(context->ast, (char *)class_name);
    }

    if (!symbol || symbol->symbol_type != CLASS_SYMBOL) return 0;
    return symbol;
}

static char *metadata_type_string(Context *context,
                                  ValueType type,
                                  size_t dims,
                                  int *dim_base,
                                  int *dim_elements,
                                  const char *class_name) {
    char *base;
    char *array;
    char *result;
    char *resolved_class_name = 0;
    int free_base = 0;

    if (type == TP_OBJECT && class_name && *class_name) {
        Symbol *class_symbol = resolve_metadata_class_name(context, class_name);
        if (class_symbol) resolved_class_name = sym_frnm(class_symbol);
        base = rxcp_internal_name_to_source_qualified(resolved_class_name ? resolved_class_name : class_name, 1);
        free_base = 1;
    } else {
        base = type_nm(type);
    }

    array = ast_astr(dims, dim_base, dim_elements);
    result = malloc(strlen(base) + strlen(array) + 1);
    strcpy(result, base);
    strcat(result, array);

    if (resolved_class_name) free(resolved_class_name);
    if (free_base) free(base);
    free(array);

    return result;
}

char *callable_effective_return_type(ASTNode *node) {
    ASTNode *return_node;
    Context *context;

    if (!node) return strdup(".void");
    context = node->context;

    if (node->symbolNode && node->symbolNode->symbol &&
        (node->symbolNode->symbol->type != TP_UNKNOWN ||
         node->symbolNode->symbol->value_class ||
         node->symbolNode->symbol->value_dims)) {
        Symbol *symbol = node->symbolNode->symbol;
        return metadata_type_string(context,
                                    symbol->type,
                                    symbol->value_dims,
                                    symbol->dim_base,
                                    symbol->dim_elements,
                                    symbol->value_class);
    }

    return_node = ast_chld(node, CLASS, VOID);
    if (!return_node) return strdup(".void");
    if (node->node_type == FACTORY && return_node && return_node->node_type == VOID) {
        ASTNode *owner = node->parent;
        while (owner && owner->node_type != CLASS_DEF && owner->node_type != INTERFACE_DEF) {
            owner = owner->parent;
        }
        if (owner) {
            const char *name = 0;
            char *normalized = 0;
            char *result;

            if (owner->symbolNode && owner->symbolNode->symbol && owner->symbolNode->symbol->name) {
                normalized = sym_frnm(owner->symbolNode->symbol);
                name = normalized;
            }
            else if (owner->node_string) {
                normalized = rxcp_normalize_source_symbol_name(owner->node_string,
                                                               owner->node_string_length,
                                                               0,
                                                               1);
                name = normalized;
            }

            if (name && name[0]) {
                result = metadata_type_string(context, TP_OBJECT, 0, 0, 0, name);
                if (normalized) free(normalized);
                return result;
            }
            if (normalized) free(normalized);
        }
    }

    if (context) {
        size_t dims = 0;
        int *dim_base = 0;
        int *dim_elements = 0;
        char *class_name = 0;
        ValueType type = node_to_type(context, return_node, &dims, &dim_base, &dim_elements, &class_name);
        char *result = metadata_type_string(context, type, dims, dim_base, dim_elements, class_name);

        if (dim_base) free(dim_base);
        if (dim_elements) free(dim_elements);
        if (class_name) free(class_name);

        return result;
    }

    return ast_n2tp(return_node);
}

/* Adds Symbol metadata */
void meta_set_symbol(Symbol *symbol, void *payload) {
    ASTNode* node = (ASTNode*)payload;
    OutputFragment *output = node->output;
    char* buffer;
    char* symbol_fqn;
    int symbol_ordinal;
    char *type;
    SymbolNode *symbol_node;

    if (symbol->symbol_type != FUNCTION_SYMBOL) {

        /* Logic that works out if we should emit the variable meta data here */
        if (symbol->meta_emitted) return;     /* Aleady done */
        if (node->high_ordinal == -1) return; /* Weird optimiser added node - skip as we don't know what's going on */
        symbol_ordinal = symbol->creation_ordinal;
        if (symbol_ordinal == -1 || symbol_ordinal > node->high_ordinal) return; /* Symbol is not yet valid */
        symbol->meta_emitted = 1;

        if (symbol->symbol_type == CONSTANT_SYMBOL) {
            type = sym_2tp(symbol);
            symbol_fqn = sym_frnm(symbol);

            symbol_node = sym_trnd(symbol, 0);
            if (symbol_node) {
                buffer = mprintf("   .meta \"%s\"=\"b\" \"%s\" \"%.*s\"\n",
                                 symbol_fqn,
                                 type,
                                 (int) symbol_node->node->node_string_length, symbol_node->node->node_string);
            }
            else {
                /* Must be a taken constant - the name is its value */
                buffer = mprintf("   .meta \"%s\"=\"b\" \"%s\" \"%s\"\n",
                                 symbol_fqn,
                                 type,
                                 symbol->name);
            }
            free(symbol_fqn);
            free(type);
        }

        else if (symbol->register_num >= 0) {
            symbol_fqn = sym_frnm(symbol);
            type = sym_2tp(symbol);
            buffer = mprintf("   .meta \"%s\"=\"b\" \"%s\" %c%d\n",
                             symbol_fqn,
                             type,
                             symbol->register_type, symbol->register_num
            );
            free(type);
            free(symbol_fqn);
        }

        else return; /* No symbol information ... */

        /* Add the metadata to the output fragment */
        output_append_text(output,buffer);
        free(buffer);
    }
}

/* Add Variable Metadata */
void add_variable_metadata(ASTNode* node) {

    Scope *scope = node->scope;
    ASTNode *n = node;

    while (!scope) {
        n = n->parent;
        if (!n) return; /* No scope ... ! */
        scope = n->scope;
    }

    /* Sets the Procedure's Symbols from metadata */
    while (scope) {
        scp_4all(scope, meta_set_symbol, node);
        if (scope->type == SCOPE_PROCEDURE) break;
        scope = scope->parent;
    }
}

/* Adds Symbol default initiator */
void add_initiator(Symbol *symbol, void *payload) {
    ASTNode* node = (ASTNode*)payload;
    OutputFragment *output = node->output;
    char* buffer;
    char* symbol_fqn;
    char *type;
    SymbolNode *symbol_node;

    if (symbol->needs_default_initiation && !symbol->init_emitted) {
        symbol->init_emitted = 1;

        if (symbol->symbol_type == CONSTANT_SYMBOL) {
            /* Ignore symbols that were optimised with constant folding */
            return;
        }

        /* Output Variable metadata here - as it is "locked in" and has been initiated */
        if (symbol->register_num >= 0 && !symbol->meta_emitted) { // Should be true
            symbol_fqn = sym_frnm(symbol);
            type = sym_2tp(symbol);
            buffer = mprintf("   .meta \"%s\"=\"b\" \"%s\" %c%d\n",
                             symbol_fqn,
                             type,
                             symbol->register_type, symbol->register_num
            );

            /* Add the metadata to the output fragment */
            output_append_text(output,buffer);

            free(buffer);
            free(type);
            free(symbol_fqn);
            symbol->meta_emitted = 1;
        }

        /* Add source metadata */
        symbol_node = sym_trnd(symbol, 0);
        if (symbol_node) {
            buffer = get_metaline(symbol_node->node->parent);
            output_append_text(output,buffer);
            free(buffer);
        }

        /* We need to clear the register */
        char* init = mprintf("   null %c%d\n", symbol->register_type, symbol->register_num);
        output_append_text(output, init);
        free(init);
    }
}

/* Add Initiators for variables in the node's scope  */
void add_scope_initiators(ASTNode* node) {

    Scope *scope = node->scope;
    ASTNode *n = node;

    while (!scope) {
        n = n->parent;
        if (!n) return; /* No scope ... ! */
        scope = n->scope;
    }

    /* Add Initiators */
    while (scope) {
        scp_4all(scope, add_initiator, node);
        if (scope->type == SCOPE_PROCEDURE) break;
        scope = scope->parent;
    }
}

/* Adds and exposed Global Variable Symbol */
void add_global_symbol(Symbol *symbol, void *payload) {
    ASTNode* node = (ASTNode*)payload;
    OutputFragment *output = node->output;
    char* buffer;
    char* symbol_fqn;

    if (symbol->symbol_type == VARIABLE_SYMBOL && symbol->exposed) {

        symbol_fqn = sym_frnm(symbol);
        buffer = mprintf("%c%d .expose=%s\n",
                         symbol->register_type, symbol->register_num,
                         symbol_fqn
        );
        free(symbol_fqn);

        /* Add the metadata to the output fragment */
        output_append_text(output,buffer);
        free(buffer);
    }
}

/* Add exposed Global Variables - node is the PROGRAM_FILE node */
void add_exposed_global_variable(ASTNode* node) {

    Scope *scope = node->scope;
    ASTNode *n = node;

    /*  Find the node (file / namespace scope) */
    while (!scope) {
        n = n->parent;
        if (!n) return; /* No scope ... ! */
        scope = n->scope;
    }

    /* Sets the Procedure's Global Symbols from metadata */
    scp_4all(scope, add_global_symbol, node);
}

/* Adds Global Variable Symbol metadata */
void meta_set_global_symbol(Symbol *symbol, void *payload) {
    ASTNode* node = (ASTNode*)payload; /* The PROCEDURE node */
    OutputFragment *output = node->output;
    char* buffer;
    char* symbol_fqn;

    if (symbol->symbol_type == VARIABLE_SYMBOL) {
        /* Is the global used in the procedure? */
        if ( symislnk(ast_chld(node, INSTRUCTIONS, NOP), symbol) ) {
            symbol_fqn = sym_frnm(symbol);
            buffer = mprintf("   .meta \"%s\"=\"b\" \"%s\" %c%d\n",
                             symbol_fqn,
                             type_nm(symbol->type),
                             symbol->register_type, symbol->register_num
            );
            free(symbol_fqn);

            /* Add the metadata to the output fragment */
            output_append_text(output, buffer);
            free(buffer);
        }
    }
}

/* Add Global Variable Metadata
 * node is the PROCEDURE node*/
void add_global_variable_metadata(ASTNode* node) {

    Scope *scope = node->scope;
    ASTNode *n = node;

    while (!scope) {
        n = n->parent;
        if (!n) return; /* No scope ... ! */
        scope = n->scope;
    }

    /* Sets the Procedure's Symbols from metadata */
    scp_4all(scope, meta_set_global_symbol, node);
}

/* Clears Symbol metadata */
void meta_clear_symbol(Symbol *symbol, void *payload) {
    ASTNode* node = (ASTNode*)payload;
    OutputFragment *output = node->output;
    char* buffer;
    char* symbol_fqn;

    if (symbol->symbol_type != FUNCTION_SYMBOL) {

        if (!symbol->meta_emitted) {
            if (symbol->register_num < 0) {
                return;
            }
            if (symbol_is_exit_token_only_in_proc(symbol, node)) {
                return;
            }
            if (symbol_has_live_node_in_proc(symbol, node)) {
                fprintf(stderr, "WARNING: Did not emit metadata for symbol %s\n", symbol->name);
            }
            return;
        }

        if (symbol->symbol_type == CONSTANT_SYMBOL || symbol->register_num >= 0) {
            symbol_fqn = sym_frnm(symbol);

            buffer = mprintf("   .meta \"%s\"\n", symbol_fqn);

            free(symbol_fqn);
        }
        else return; /* No symbol information ... */

        /* Add the metadata to the output fragment */
        output_append_text(output,buffer);
        free(buffer);
    }
}

/* Clear all variable metadata */
void clear_variable_metadata(ASTNode *node) {

    Scope *scope = node->scope;
    ASTNode *n = node;

    while (!scope) {
        n = n->parent;
        if (!n) return; /* No scope ... ! */
        scope = n->scope;
    }

    /* Clears the Procedure's Symbols from metadata */
    while (scope) {
        scp_4all(scope, meta_clear_symbol, node);
        if (scope->type == SCOPE_PROCEDURE) break;
        scope = scope->parent;
    }
}

/* Clear Global Variable Symbol metadata */
void meta_clear_global_symbol(Symbol *symbol, void *payload) {
    ASTNode* node = (ASTNode*)payload; /* The PROCEDURE node */
    OutputFragment *output = node->output;
    char* buffer;
    char* symbol_fqn;

    if (symbol->symbol_type == VARIABLE_SYMBOL) {
        /* Is the global used in the procedure */
        if ( symislnk(ast_chld(node, INSTRUCTIONS, NOP), symbol) ) {
            symbol_fqn = sym_frnm(symbol);
            buffer = mprintf("   .meta \"%s\"\n", symbol_fqn);
            free(symbol_fqn);

            /* Add the metadata to the output fragment */
            output_append_text(output, buffer);
            free(buffer);
        }
    }
}

/* Clear Global Variable Metadata
 * node is the PROCEDURE node*/
void clear_global_variable_metadata(ASTNode* node) {

    Scope *scope = node->scope;
    ASTNode *n = node;

    /*  Find the node (procedure scope) */
    while (!scope) {
        n = n->parent;
        if (!n) return; /* No scope ... ! */
        scope = n->scope;
    }

    /* namespace scope */
    scope = scope->parent;

    /* Clears the Procedure's Global Symbols from metadata */
    scp_4all(scope, meta_clear_global_symbol, node);
}

/* Returns Argument definition from the ARG Node as a malloced string to be used in meta-data */
/* Node should be an ARGS node else the program aborts */
/* Adds class/interface contract metadata */
void add_class_symbol(Symbol *symbol, void *payload) {
    ASTNode* node = (ASTNode*)payload;
    OutputFragment *output = node->output;
    char* buffer;
    char* symbol_fqn;

    if (symbol->symbol_type == CLASS_SYMBOL) {
        ASTNode *def_node = symbol->defines_scope ? symbol->defines_scope->defining_node : 0;

        if (!def_node || node_is_imported_definition(def_node)) return;

        symbol_fqn = sym_frnm(symbol);
        if (def_node && def_node->node_type == INTERFACE_DEF) {
            buffer = mprintf(".meta \"%s\"=\"b\" \"%s\" .interface\n",
                             symbol_fqn,
                             type_nm(symbol->type));
        } else {
            buffer = mprintf(".meta \"%s\"=\"b\" \"%s\" .class\n",
                             symbol_fqn,
                             type_nm(symbol->type));
        }
        free(symbol_fqn);

        /* Add the metadata to the output fragment */
        output_append_text(output, buffer);
        free(buffer);

        if (def_node && def_node->node_type == CLASS_DEF) {
            ASTNode *implements_node = ast_chld(def_node, IMPLEMENTS, 0);
            ASTNode *iface_ref = implements_node ? implements_node->child : 0;
            while (iface_ref) {
                Symbol *iface_symbol = iface_ref->symbolNode ? iface_ref->symbolNode->symbol : 0;
                if (!iface_symbol) iface_symbol = sym_rvfc(def_node->context ? def_node->context->ast : 0, iface_ref);
                if (iface_symbol && iface_symbol->symbol_type == CLASS_SYMBOL) {
                    char *class_fqn = sym_frnm(symbol);
                    char *iface_fqn = sym_frnm(iface_symbol);
                    char *buf_impl = mprintf(".meta \"%s\"=\"%s\" .implements\n", class_fqn, iface_fqn);
                    output_append_text(output, buf_impl);
                    free(buf_impl);
                    free(class_fqn);
                    free(iface_fqn);
                }
                iface_ref = iface_ref->sibling;
            }
        }

        if (def_node && def_node->node_type == INTERFACE_DEF && symbol->defines_scope) {
            Symbol **symbols = scp_syms(symbol->defines_scope);
            int i;
            for (i = 0; symbols[i]; i++) {
                Symbol *s = symbols[i];
                if (s->symbol_type == FUNCTION_SYMBOL && sym_nond(s) > 0) {
                    ASTNode *member = sym_trnd(s, 0)->node;
                    if (member && (member->node_type == METHOD || member->node_type == FACTORY)) {
                        char *owner_fqn = sym_frnm(symbol);
                        char *rtype = callable_effective_return_type(member);
                        char *args = meta_narg(ast_chld(member, ARGS, 0));
                        char *member_name;
                        member_name = malloc(member->node_string_length + 1);
                        memcpy(member_name, member->node_string, member->node_string_length);
                        member_name[member->node_string_length] = 0;
                        const char *member_kind = interface_member_kind_string(member);
                        char *buf_member = mprintf(".meta \"%s\"=\"%s\" \"%s\" \"%s\" \"%s\" .member\n",
                                                   owner_fqn,
                                                   member_kind,
                                                   member_name,
                                                   rtype,
                                                   args);
                        output_append_text(output, buf_member);
                        free(buf_member);
                        free(member_name);
                        free(args);
                        free(rtype);
                        free(owner_fqn);
                    }
                }
            }
            free(symbols);
        }

        /* Add attribute metadata by walking class scope symbols */
        if (def_node && def_node->node_type == CLASS_DEF && symbol->defines_scope) {
            Symbol **symbols = scp_syms(symbol->defines_scope);
            int i, j;
            for (i = 0; symbols[i]; i++) {
                Symbol *s = symbols[i];
                if (s->symbol_type == VARIABLE_SYMBOL) {
                    /* Look for NODE_REGISTER in the AST linked to the symbol */
                    for (j = 0; j < (int)sym_nond(s); j++) {
                        ASTNode *sn = sym_trnd(s, j)->node;
                        if (sn->parent && sn->parent->node_type == DEFINE) {
                            ASTNode *nr = ast_chld(sn->parent, NODE_REGISTER, 0);
                            if (nr) {
                                int reg_index = 0;
                                /* Prefer child INTEGER value for robustness */
                                ASTNode *idx = ast_chld(nr, INTEGER, 0);
                                if (idx) reg_index = node_to_integer(idx);
                                else if (nr->int_value) reg_index = (int)nr->int_value;
                                else if (nr->child && nr->child->token) {
                                    /* Fallback: parse from token text */
                                    reg_index = (int)strtol(nr->child->token->token_string, NULL, 10);
                                }
                                char *attr_fqn = sym_frnm(s);
                                char *type_str = type_nm(s->type);
                                char *buf2 = mprintf(".meta \"%s\"=\"b\" \"%s\" .attr %d\n",
                                                     attr_fqn, type_str, reg_index);
                                output_append_text(output, buf2);
                                free(buf2);
                                free(attr_fqn);
                                break; /* Found it */
                            }
                        }
                    }
                }
            }
            free(symbols);
        }
    }
}

/* Add all contract metadata in a scope */
void add_all_class_metadata(ASTNode* scope_node, ASTNode* output_node) {
    Scope *scope = scope_node->scope;
    if (!scope) return;

    /* Avoid duplication using the namespace symbol's meta_emitted flag */
    if (scope_node->symbolNode && scope_node->symbolNode->symbol) {
        if (scope_node->symbolNode->symbol->meta_emitted) return;
        scope_node->symbolNode->symbol->meta_emitted = 1;
    }

    scp_4all(scope, add_class_symbol, output_node);
}

char *meta_narg(ASTNode *node) {
    size_t args;
    size_t i;
    size_t buffer_len;
    char *buffer;
    ASTNode *a;
    char **type;
    char **name;

    if (!node) {
        buffer = malloc(1);
        buffer[0] = 0;
        return buffer;
    }

    if (node->node_type != ARGS) {
        fprintf(stderr,"INTERNAL ERROR - Not a ARG node in meta_args()\n");
        exit(9);
    }
    args = ast_nchd(node);

    if (!args) {
        /* Return an empty malloced string */
        buffer = malloc(1);
        buffer[0] = 0;
        return buffer;
    }

    /* Get all the args */
    type = malloc(args * sizeof(char*));
    name = malloc(args * sizeof(char*));
    buffer_len = 0;
    for (i=0; i<args; i++) {
        a = ast_chdn(node, i);
        if (a->child->node_type == VARG || a->child->node_type == VARG_REFERENCE) {
            type[i] = ast_n2tp(a->child->sibling);
            name[i] = malloc(4);
            strcpy(name[i], "...");
        }
        else {
            type[i] = sym_2tp(a->child->symbolNode->symbol);
            name[i] = malloc(strlen(a->child->symbolNode->symbol->name) + 1);
            strcpy(name[i], a->child->symbolNode->symbol->name);
        }

        /* Workout length */
        buffer_len += strlen(type[i]);
        buffer_len += strlen(name[i]);
        buffer_len += 1; /* "=" */
        if (a->is_opt_arg) buffer_len += 1; /* "?" */
        if (a->is_ref_arg) buffer_len += 7; /* "expose " */
    }
    /* Add space for comas between args and the null termination */
    buffer_len += (args - 1) + 1;

    /* Write the args out */
    buffer = malloc(buffer_len);
    buffer[0] = 0;
    for (i=0; i<args; i++) {
        a = ast_chdn(node, i);
        if (a->is_ref_arg) strcat(buffer,"expose ");
        if (a->is_opt_arg) strcat(buffer,"?");
        strcat(buffer,name[i]);
        strcat(buffer,"=");
        strcat(buffer,type[i]);

        /* Add the comma if not the last argument */
        if (i < args - 1) strcat(buffer,",");
    }

    /* free temporary buffers */
    for (i=0; i<args; i++) {
        free(type[i]);
        free(name[i]);
    }
    free(name);
    free(type);

    return buffer;
}
