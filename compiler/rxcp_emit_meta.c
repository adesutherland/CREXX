/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2025 Adrian Sutherland, Peter Jacob, René Jansen
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

#define REGTP_VAL 1
#define REGTP_NOTSYM 2

static int symbol_has_live_node_in_proc(Symbol *symbol, ASTNode *proc_node) {
    size_t i;

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
/* Adds Class Symbol metadata */
void add_class_symbol(Symbol *symbol, void *payload) {
    ASTNode* node = (ASTNode*)payload;
    OutputFragment *output = node->output;
    char* buffer;
    char* symbol_fqn;

    if (symbol->symbol_type == CLASS_SYMBOL) {
        symbol_fqn = sym_frnm(symbol);
        buffer = mprintf(".meta \"%s\"=\"b\" \"%s\" .class\n",
                         symbol_fqn,
                         type_nm(symbol->type)
        );
        free(symbol_fqn);

        /* Add the metadata to the output fragment */
        output_append_text(output, buffer);
        free(buffer);

        /* Add attribute metadata by walking class scope symbols */
        if (symbol->defines_scope) {
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

/* Add all class metadata in a scope */
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
