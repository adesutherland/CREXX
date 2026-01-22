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
 * Code Generator / RXAS Emitter
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

#define UNSET_REGISTER (-1)
#define DONT_ASSIGN_REGISTER (-2)

/* Register Type Flag Byte Values */
/* Used for optional arguments ONLY
 * set (1) means the register has a specified value */
#define REGTP_VAL 1

/* Used for "pass be value" large (strings, objects) registers ONLY
 * set (2) means that it is not a symbol so does not need copying as even if it is
 * changed the caller will not use its original value
 * Note: Small registers (int, float) are always copied as this is faster than
 *       setting and checking this flag anyway */
#define REGTP_NOTSYM 2

/* Adds Symbol metadata */
static void meta_set_symbol(Symbol *symbol, void *payload) {
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
        symbol_ordinal = sym_lord(symbol);
        if (symbol_ordinal > node->high_ordinal) return; /* Symbol is not yet valid */
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
    scp_4all(scope, meta_set_symbol, node);
}

/* Adds Symbol default initiator */
static void add_initiator(Symbol *symbol, void *payload) {
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
static void add_scope_initiators(ASTNode* node) {

    Scope *scope = node->scope;
    ASTNode *n = node;

    while (!scope) {
        n = n->parent;
        if (!n) return; /* No scope ... ! */
        scope = n->scope;
    }

    /* Add Initiators */
    scp_4all(scope, add_initiator, node);
}

/* Adds and exposed Global Variable Symbol */
static void add_global_symbol(Symbol *symbol, void *payload) {
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
static void add_exposed_global_variable(ASTNode* node) {

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
static void meta_set_global_symbol(Symbol *symbol, void *payload) {
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
static void add_global_variable_metadata(ASTNode* node) {

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

    /* Sets the Procedure's Global Symbols from metadata */
    scp_4all(scope, meta_set_global_symbol, node);
}

/* Clears Symbol metadata */
static void meta_clear_symbol(Symbol *symbol, void *payload) {
    ASTNode* value_node;
    ASTNode* node = (ASTNode*)payload;
    OutputFragment *output = node->output;
    char* buffer;
    char* symbol_fqn;

    if (symbol->symbol_type != FUNCTION_SYMBOL) {

        if (!symbol->meta_emitted) {
            fprintf(stderr, "WARNING: Did not emit metadata for symbol %s\n", symbol->name);
            return;
        }

        if (symbol->symbol_type == CONSTANT_SYMBOL || symbol->register_num >= 0) {
            symbol_fqn = sym_frnm(symbol);

            value_node = sym_trnd(symbol, 0)->node->sibling;
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
    scp_4all(scope, meta_clear_symbol, node);
}

/* Clear Global Variable Symbol metadata */
static void meta_clear_global_symbol(Symbol *symbol, void *payload) {
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

static walker_result emit_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *pl) {

    walker_payload *payload = (walker_payload*) pl;
    ASTNode *child1, *child2, *child3, *n;
    char *op;
    char *tp_prefix;
    OutputFragment *o;
    char *temp1;
    char *temp2;
    char *comment_meta;
    size_t i;
    int j, k;
    int flag;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char ret_type;
    int ret_num;

    child1 = node->child;
    if (child1) child2 = child1->sibling;
    else child2 = NULL;
    if (child2) child3 = child2->sibling;
    else child3 = NULL;

    if (direction == out) {
        /* OUT - BOTTOM UP */

        /* Operator and type prefix */
        op = 0;
        if (node->value_dims) tp_prefix = "";
        else tp_prefix = type_to_prefix(node->value_type);

        switch (node->node_type) {

            case REXX_UNIVERSE:
            {
                char *buf = mprintf("/*\n"
                                    " * cREXX COMPILER VERSION : %s\n"
                                    " * SOURCE                 : %s\n"
                                    " * BUILT                  : %d-%02d-%02d %02d:%02d:%02d\n"
                                    " */\n"
                                    "\n",
                                    rxversion,
                                    payload->context->file_name,
                                    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

                if (node->output) output_prepend_text(buf, node->output);
                else node->output = output_fs(buf);
                free(buf);

                n = child1;
                while (n) {
                    if (n->output) output_concat(node->output, n->output);
                    if (n->cleanup) output_concat(node->output, n->cleanup);
                    n = n->sibling;
                }

                print_output(payload->file, node->output);
            }
            break;

            case PROGRAM_FILE:
            {
                char *buf = mprintf(".srcfile=\"%s\"\n"
                                    ".globals=%d\n",
                                    payload->context->file_name,
                                    payload->globals);

                if (node->output) output_prepend_text(buf, node->output);
                else node->output = output_fs(buf);
                free(buf);

                /* Add exposed global variables */
                add_exposed_global_variable(node);

                n = child1;
                while (n) {
                    if (n->output) output_concat(node->output, n->output);
                    if (n->cleanup) output_concat(node->output, n->cleanup);
                    n = n->sibling;
                }
            }
            break;

            case IMPORTED_FILE:
            {
                char *buf = mprintf("\n/* Imported Declaration from file: %s */",
                                    node->file_name);

                if (node->output) output_prepend_text(buf, node->output);
                else node->output = output_fs(buf);
                free(buf);

                n = child1;
                while (n) {
                    if (n->output) output_concat(node->output, n->output);
                    if (n->cleanup) output_concat(node->output, n->cleanup);
                    n = n->sibling;
                }
            }
            break;

            case PROCEDURE:
                if (ast_chld(node, INSTRUCTIONS, NOP)->node_type == NOP) {
                    /* A declaration - external */
                    char* type = ast_n2tp(ast_chld(node, CLASS, VOID));
                    char* args = meta_narg(ast_chld(node, ARGS, 0));
                    char* proc_symbol= sym_frnm(node->symbolNode->symbol);
                    char* buf;
                    if (node->symbolNode->symbol->exposed) {
                        buf = mprintf("\n%.*s() .expose=%s\n"
                                      "   .meta \"%s\"=\"b\" \"%s\" %.*s() \"%s\"\n",
                                      (int) node->node_string_length, node->node_string,
                                      proc_symbol, /* FQ Symbol Name */
                                      proc_symbol, /* FQ Symbol Name */
                                      type, /* Type */
                                      (int) node->node_string_length, node->node_string, /* Func Name */
                                      args /* Args */
                        );
                    }
                    else {
                        buf = mprintf("\n%.*s()\n"
                                      "   .meta \"%s\"=\"b\" \"%s\" %.*s() \"%s\"\n",
                                      (int) node->node_string_length, node->node_string,
                                      proc_symbol, /* FQ Symbol Name */
                                      type, /* Type */
                                      (int) node->node_string_length, node->node_string, /* Func Name */
                                      args /* Args */
                        );
                    }
                    if (node->output) output_prepend_text(buf, node->output);
                    else node->output = output_fs(buf);
                    free(type);
                    free(args);
                    free(buf);
                    free(proc_symbol);
                }
                else {
                    /* Definition */
                    char* type = ast_n2tp(ast_chld(node, CLASS, VOID));
                    char* args = meta_narg(ast_chld(node, ARGS, 0));
                    char* proc_symbol= sym_frnm(node->symbolNode->symbol);
                    char* buf;
                    if (node->symbolNode->symbol->exposed) {
                        buf = mprintf("\n%.*s() .locals=%d .expose=%s\n"
                                      "   .meta \"%s\"=\"b\" \"%s\" %.*s() \"%s\" \"\"\n",
                                      (int) node->node_string_length, node->node_string, /* Function name */
                                      (int) node->scope->num_registers, /* Locals */
                                      proc_symbol, /* FQ Symbol name */
                                      proc_symbol, /* FQ Symbol Name */
                                      type, /* Return Type */
                                      (int) node->node_string_length, node->node_string, /* Function name */
                                      args /* Args */);
                    }
                    else {
                        buf = mprintf("\n%.*s() .locals=%d\n"
                                      "   .meta \"%s\"=\"b\" \"%s\" %.*s() \"%s\" \"\"\n",
                                      (int) node->node_string_length, node->node_string, /* Function name */
                                      (int) node->scope->num_registers, /* Locals */
                                      proc_symbol, /* FQ Symbol Name */
                                      type, /* Return Type */
                                      (int) node->node_string_length, node->node_string, /* Function name */
                                      args /* Args */);
                    }
                    if (node->output) output_prepend_text(buf, node->output);
                    else node->output = output_fs(buf);
                    free(type);
                    free(args);
                    free(buf);
                    free(proc_symbol);

                    /* Add source metadata */
                    if (node->token) {
                        comment_meta = get_metaline_clause(node);
                        output_append_text(node->output, comment_meta);
                        free(comment_meta);
                    }

                    /* Add Global Variables */
                    add_global_variable_metadata(node);

                    /* If numeric options have non-inherited values, set them */
                    if (node->scope->num_context.digits > -1) {
                        temp1 = mprintf("   setnumdgts %d\n", node->scope->num_context.digits);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }
                    if (node->scope->num_context.fuzz > -1) {
                        temp1 = mprintf("   setnumfuz %d\n", node->scope->num_context.fuzz);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }
                    if (node->scope->num_context.form > 0) {
                        /* 1 = SCIENTIFIC, 2 = ENGINEERING */
                        temp1 = mprintf("   setnumfrm %d\n", node->scope->num_context.form);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }
                    if (node->scope->num_context.casetype > 0) {
                        /* 1 = LOWER, 2 = UPPER */
                        temp1 = mprintf("   setnumcas %d\n", node->scope->num_context.casetype);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }
                    if (node->scope->num_context.standard > 0) {
                        /* 1 = COMMON, 2 = CLASSIC[REXX] */
                        temp1 = mprintf("   setnumstd %d\n", node->scope->num_context.standard);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }

                    n = child2;
                    while (n) {
                        if (n->output) output_concat(node->output, n->output);
                        if (n->cleanup) output_concat(node->output, n->cleanup);
                        n = n->sibling;
                    }

                    /* Clear all variable metadata */
                    clear_variable_metadata(node);
                    clear_global_variable_metadata(node);
                }
                break;

            case ARGS:
            case INSTRUCTIONS:
                emit_flow(node, pl);
                break;

            case ARG:
                /* Add source metadata */
                comment_meta = get_metaline(node);
                if (node->output) output_prepend_text(comment_meta, node->output);
                else node->output = output_fs(comment_meta);
                free(comment_meta);
                if (node->child->node_type == VAR_TARGET || node->child->node_type == VAR_REFERENCE) {
                    /* Add Variable Metadata */
                    add_variable_metadata(node);

                    if (node->is_opt_arg) { /* Optional Argument */
                        /* If the register flag is set then an argument was specified */
                        temp1 = mprintf("   brtpandt l%da,%c%d,%d\n",
                                        child1->node_number,
                                        node->register_type,
                                        node->register_num,
                                        REGTP_VAL);
                        output_append_text(node->output, temp1);
                        free(temp1);

                        /* Set the default value */
                        output_concat(node->output, child2->output);

                        if (child1->register_num != child2->register_num ||
                            child1->register_type != child2->register_type) {
                            temp1 = mprintf("   copy %c%d,%c%d\n",
                                            child1->register_type,
                                            child1->register_num,
                                            child2->register_type,
                                            child2->register_num);
                            output_append_text(node->output, temp1);
                            free(temp1);
                        }

                        /* End of logic */
                        if (node->is_ref_arg || node->is_const_arg) {
                            /* Constant or Reference so no copy needed */
                            temp1 = mprintf("l%da:\n", child1->node_number);
                            output_append_text(node->output, temp1);
                            free(temp1);
                        } else {
                            /* Pass by value - so if the default is not used we may need to
                             * do a copy - but check if the argument needs preserving */

                            /* Only worry about it if it is a big register */
                            if (node->value_dims || node->value_type == TP_STRING || node->value_type == TP_OBJECT ||
                                node->value_type == TP_BINARY) {
                                temp1 = mprintf(
                                        "   br l%dd\n"
                                        "l%da:\n"
                                        "   brtpandt l%dc,%c%d,%d\n"
                                        "   %scopy %c%d,%c%d\n"
                                        "   acopy %c%d,%c%d\n"
                                        "   br l%dd\n"
                                        "l%dc:\n"
                                        "   swap %c%d,%c%d\n"
                                        "l%dd:\n",
                                        child1->node_number, /* br l%dd */
                                        child1->node_number, /* l%da: */

                                        /* brtpandt l%dc,%c%d,%d */
                                        child1->node_number, node->register_type, node->register_num, REGTP_NOTSYM,

                                        /* %scopy %c%d,%c%d */
                                        tp_prefix,
                                        child1->register_type, child1->register_num,
                                        node->register_type, node->register_num,

                                        /* acopy %c%d,%c%d */
                                        child1->register_type, child1->register_num,
                                        node->register_type, node->register_num,

                                        child1->node_number, /* br l%dd */
                                        child1->node_number, /* l%dc: */

                                        /* swap %c%d,%c%d */
                                        child1->register_type, child1->register_num,
                                        node->register_type, node->register_num,

                                        child1->node_number); /* l%dd: */
                                output_append_text(node->output, temp1);
                                free(temp1);
                            } else {
                                temp1 = mprintf("   br l%db\n"
                                                "l%da:\n"
                                                "   %scopy %c%d,%c%d\n"
                                                "   acopy %c%d,%c%d\n"
                                                "l%db:\n",
                                                child1->node_number, /* br l%db */
                                                child1->node_number, /* l%da: */

                                                /* %scopy %c%d,%c%d */
                                                tp_prefix,
                                                child1->register_type, child1->register_num,
                                                node->register_type, node->register_num,

                                                /* acopy %c%d,%c%d */
                                                child1->register_type, child1->register_num,
                                                node->register_type, node->register_num,

                                                child1->node_number); /* l%db: */
                                output_append_text(node->output, temp1);
                                free(temp1);
                            }
                        }
                    } else if (!(node->is_ref_arg || node->is_const_arg)) {
                        /* Copy by value so may need to do a copy - but check if the argument needs preserving */

                        /* Only worry about it if it is a big register */
                        if (node->value_dims || node->value_type == TP_STRING || node->value_type == TP_OBJECT ||
                            node->value_type == TP_BINARY) {
                            temp1 = mprintf("   brtpandt l%dc,%c%d,%d\n"
                                            "   %scopy %c%d,%c%d\n"
                                            "   br l%dd\n"
                                            "l%dc:\n"
                                            "   swap %c%d,%c%d\n"
                                            "l%dd:\n",
                                            child1->node_number,
                                            node->register_type, node->register_num,
                                            REGTP_NOTSYM,
                                            tp_prefix,
                                            child1->register_type, child1->register_num,
                                            node->register_type, node->register_num,
                                            child1->node_number,
                                            child1->node_number,
                                            child1->register_type, child1->register_num,
                                            node->register_type, node->register_num,
                                            child1->node_number);
                            output_append_text(node->output, temp1);
                            free(temp1);
                        } else {
                            /* Just need to copy register */
                            temp1 = mprintf("   %scopy %c%d,%c%d\n",
                                            tp_prefix, child1->register_type,
                                            child1->register_num,
                                            node->register_type, node->register_num);
                            output_append_text(node->output, temp1);
                            free(temp1);
                        }
                    }
                }
                break;

            case CALL:
                /* Add source metadata */
                comment_meta = get_metaline(node);
                if (node->output) output_prepend_text(comment_meta, node->output);
                else node->output = output_fs(comment_meta);
                free(comment_meta);

                /* Add Variable Metadata */
                add_variable_metadata(node);

                /* TODO - set result */
                output_concat(node->output, child1->output);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
                break;

            case FUNCTION:
                emit_expression(node, payload);
                break;

            case OP_CONCAT:
                /* REMOVED CONCAT */
            case OP_SCONCAT:
                /* REMOVED SCONCAT */
                emit_expression(node, payload);
                break;

            /* These operators have a prefix type of that of the first child */
            case OP_COMPARE_EQUAL:
            case OP_COMPARE_NEQ:
            case OP_COMPARE_GT:
            case OP_COMPARE_LT:
            case OP_COMPARE_GTE:
            case OP_COMPARE_LTE:
            case OP_COMPARE_S_EQ:
            case OP_COMPARE_S_NEQ:
            case OP_COMPARE_S_GT:
            case OP_COMPARE_S_LT:
            case OP_COMPARE_S_GTE:
            case OP_COMPARE_S_LTE:

            /* These operators use the type prefix already set (i.e. of their type) */
            case OP_ADD:
            case OP_MULT:
            case OP_MINUS:
            case OP_POWER:
            case OP_DIV:
            case OP_IDIV:
            case OP_MOD:
                emit_expression(node, payload);
                break;

            case OP_AND:
            case OP_OR:
                emit_expression(node, payload);
                break;

            case OP_ARG_EXISTS:
                if (!node->output) node->output = output_f();
                temp1 = mprintf("   getandtp %c%d,%c%d,%d\n",
                                node->register_type,
                                node->register_num,
                                node->symbolNode->symbol->register_type,
                                node->symbolNode->symbol->register_num,
                                REGTP_VAL);
                output_append_text(node->output, temp1);
                free(temp1);
                type_promotion(node);
                break;

            case OP_ARGS:
                if (!node->output) node->output = output_f();
                /* Get the total args and subtract the fixed args of the procedure we are in */
                temp1 = mprintf("   icopy %c%d,a0\n"
                                "   isub %c%d,%c%d,%d\n",
                                node->register_type,
                                node->register_num,
                                node->register_type,
                                node->register_num,
                                node->register_type,
                                node->register_num,
                                ast_proc(node)->symbolNode->symbol->fixed_args
                );
                output_append_text(node->output, temp1);
                free(temp1);
                type_promotion(node);
                break;

            case OP_ARG_VALUE:
                if (!node->output) node->output = output_f();

                /* Link the argument */
                if (child1->register_num == DONT_ASSIGN_REGISTER) {
                    /* Child is a constant */

                    /* Needed to calculate the argument number taking into account the number of fixed args */
                    temp2 = format_constant(child1->value_type, child1);
                    int arg_ix = atoi(temp2) + ast_proc(node)->symbolNode->symbol->fixed_args;
                    free(temp2);

                    temp1 = mprintf("   icopy %c%d,a0\n" /* Total number of arguments */
                                    "   isub %c%d,%c%d,%d\n"    /* Deduct # fixed arguments */
                                    "   ichkrng %.*s,1,%c%d\n"  /* Validate Range */
                                    "   linkarg %c%d,%d\n",     /* Link to argument (with added # fixed arguments) */

                                    /* icopy %c%d,a0 */
                                    node->register_type, node->register_num,

                                    /* isub %c%d,%c%d,%d */
                                    node->register_type,
                                    node->register_num,
                                    node->register_type,
                                    node->register_num,
                                    ast_proc(node)->symbolNode->symbol->fixed_args,

                                    /* ichkrng %.*s,1,%c%d */
                                    child1->node_string_length, child1->node_string,
                                    node->register_type, node->register_num,

                                    /* linkarg %c%d,%.*s */
                                    node->register_type, node->register_num,
                                    arg_ix);
                }

                else {
                    /* Child is a register */
                    temp1 = mprintf("   icopy %c%d,a0\n"    /* Total number of arguments */
                                    "   isub %c%d,%c%d,%d\n"       /* Deduct # of fixed arguments */
                                    "   ichkrng %c%d,1,%c%d\n"     /* Validate Range */
                                    "   linkarg %c%d,%c%d,%d\n",   /* Link to argument (third param adds # fixed arguments) */

                                    /* icopy %c%d,a0 */
                                    node->register_type, node->register_num,

                                    /* isub %c%d,%c%d,%d */
                                    node->register_type,
                                    node->register_num,
                                    node->register_type,
                                    node->register_num,
                                    ast_proc(node)->symbolNode->symbol->fixed_args,

                                    /* ichkrng %.*s,1,%c%d */
                                    child1->register_type, child1->register_num,
                                    node->register_type, node->register_num,

                                    /* linkarg %c%d,%c%d,%d */
                                    node->register_type, node->register_num,
                                    child1->register_type, child1->register_num,
                                    ast_proc(node)->symbolNode->symbol->fixed_args);

                }

                output_append_text(node->output, temp1);
                free(temp1);

                /* Call child cleanup action */
                if (child1->cleanup) output_concat(node->output, child1->cleanup);

                /* Type Promotion */
                type_promotion(node);

                /* Set cleanup action */
                temp1 = mprintf("   unlink r%d\n", node->register_num);
                node->cleanup = output_fs(temp1);
                free(temp1);
                break;

            case OP_ARG_IX_EXISTS:
                if (!node->output) node->output = output_f();

                /* This is really a compatability operator - if the argument number given is smaller or equal
                 * to the number of variable arguments then it does exist otherwise it doesn't. If smaller than 1
                 * a signal should be thrown */
                if (child1->register_num == DONT_ASSIGN_REGISTER) {
                    /* Child is a constant */
                    /* < 1 will already be checked */

                    /* Needed to calculate the argument number by adding #fixed args */
                    temp2 = format_constant(child1->value_type, child1);
                    int arg_ix = atoi(temp2) + ast_proc(node)->symbolNode->symbol->fixed_args;
                    free(temp2);

                    temp1 = mprintf("   icopy %c%d,a0\n"       /* Total number of arguments (fixed and variable) */
                                    "   ilte %c%d,%d,%c%d\n",  /* `Is <= number of registers? */

                                    /* icopy %c%d,a0 */
                                    node->register_type, node->register_num,

                                    /* ilte %c%d,%d,%c%d */
                                    node->register_type, node->register_num,
                                    arg_ix,
                                    node->register_type, node->register_num);
                }

                else {
                    /* Child is a register */
                    temp1 = mprintf("   ilt %c%d,%c%d,1\n"         /* Is parm < 1? */
                                    "   signalt \"OUT_OF_RANGE\",%c%d\n"   /* Signal if so */
                                    "   icopy %c%d,a0\n"           /* Total number of arguments */
                                    "   isub %c%d,%c%d,%d\n"       /* Deduct # of fixed arguments */
                                    "   ilte %c%d,%c%d,%c%d\n",    /* Is <= number of registers? */

                                    /* ilt %c%d,%c%d,1 */
                                    node->register_type, node->register_num,
                                    child1->register_type, child1->register_num,

                                    /* signalt "OUT_OF_RANGE",%c%d */
                                    node->register_type, node->register_num,

                                    /* icopy %c%d,a0 */
                                    node->register_type, node->register_num,

                                    /* isub %c%d,%c%d,%d */
                                    node->register_type, node->register_num,
                                    node->register_type, node->register_num,
                                    ast_proc(node)->symbolNode->symbol->fixed_args,

                                    /* ilte %c%d,%c%d,%c%d */
                                    node->register_type, node->register_num,
                                    child1->register_type, child1->register_num,
                                    node->register_type, node->register_num);
                }

                output_append_text(node->output, temp1);
                free(temp1);

                /* Call child cleanup action */
                if (child1->cleanup) output_concat(node->output, child1->cleanup);

                /* Type Promotion */
                type_promotion(node);
                break;

            case OP_NOT:
            case OP_NEG:
            case OP_PLUS:
                emit_expression(node, payload);
                break;

            case VAR_SYMBOL:
            case VAR_TARGET:
                /* If we are a define no code is generated */
                if (node->parent->node_type == DEFINE) break;

                if (!node->output) node->output = output_f();

                if (child1) {
                    /* We are an array */
                    /* Essentially, we are linking the found array element as the nodes result - which will need unlinking later */
                    char from_reg_type = node->symbolNode->symbol->register_type;
                    int from_reg_num = node->symbolNode->symbol->register_num;
                    char unlink_needed = 0;

                    /* Now we need to link the array elements */
                    while (child1) {
                        int base = node->symbolNode->symbol->dim_base[ast_chdi(child1)];

                        if (child1->output) output_concat(node->output, child1->output);

                        if (node->node_type == VAR_SYMBOL && child1->node_type == NOVAL) {
                            /* This is the logic for getting the number of elements in an array */
                            /* This is last parameter - we may have done earlier parameters */

                            if (!unlink_needed) {
                                /* No unlinking funny business - i.e. we are the first dimension */
                                if (node->symbolNode->symbol->dim_elements[ast_chdi(child1)]) {
                                    /* For fixed arrays we just return the upperbound (taking into the base) */
                                    temp1 = mprintf("   load r%d,%d\n",
                                                    node->register_num,
                                                    node->symbolNode->symbol->dim_elements[ast_chdi(child1)] +
                                                    base - 1);
                                } else {
                                    /* Return the max array element taking into account the array base */
                                    temp1 = mprintf("   getattrs r%d,%c%d,%d\n",
                                                    node->register_num,
                                                    from_reg_type,
                                                    from_reg_num,
                                                    base - 1);
                                }
                            } else {
                                /* The register of the attribute is linked so ... */
                                unlink_needed = 0;   /* ... we will unlink it here */
                                if (node->symbolNode->symbol->dim_elements[ast_chdi(child1)]) {
                                    /* For fixed arrays we just return the upperbound (taking into the base) */
                                    /* We have linked and worked though all the dimensions to get here and then
                                     * don't actually use the linked register (!), but we have checked all the parameters
                                     * to this point so actually IT IS valid to do this */
                                    temp1 = mprintf(
                                            "   unlink r%d\n"
                                            "   load r%d,%d\n",
                                                    node->register_num,
                                                    node->register_num,
                                                    node->symbolNode->symbol->dim_elements[ast_chdi(child1)] +
                                                    base - 1);
                                } else {
                                    /* Return the max array element taking into account the array base */
                                    /* We need to copy via the additional register so we can unlink correctly */
                                    temp1 = mprintf("   getattrs r%d,%c%d,%d\n"
                                                    "   unlink r%d\n"
                                                    "   icopy r%d,r%d\n",
                                                    node->additional_registers,
                                                    from_reg_type,
                                                    from_reg_num,
                                                    base - 1,

                                                    node->register_num,

                                                    node->register_num,
                                                    node->additional_registers);
                                }
                            }
                            output_append_text(node->output, temp1);
                            free(temp1);

                            /* Call child cleanup action */
                            if (child1->cleanup) output_concat(node->output, child1->cleanup);

                            /* Should be no more dimensions so we are done */
                            goto var_symbol_end;
                        }

                        /* This is the logic to get the register attribute for this parameter (child1) */

                        /* We might need a string of the index number later (we need it twice) */
                        if (child1->node_type == INTEGER || child1->node_type == CONSTANT) {
                            /* Make temp2 the base 1 element index number */
                            temp2 = format_constant(child1->value_type, child1);
                            if (base != 1) {
                                int ix = atoi(temp2) + 1 - base;
                                free(temp2);
                                temp2 = mprintf("%d", ix);
                            }
                        } else temp2 = 0;

                        /* Make sure there is enough attributes */
                        if (node->symbolNode->symbol->dim_elements[ast_chdi(child1)]) {
                            /* Fixed array set to the dimension size - later linkattr1 might throw a signal if out of range by design */
                            temp1 = mprintf("   setattrs %c%d,%d\n",
                                            from_reg_type, from_reg_num,
                                            node->symbolNode->symbol->dim_elements[ast_chdi(child1)]);
                        } else if (child1->node_type == INTEGER || child1->node_type == CONSTANT) {
                            /* Variable array and constant parameter - set min attributes which gives a growth buffer */
                            temp1 = mprintf("   minattrs %c%d,%s\n",
                                            from_reg_type, from_reg_num,
                                            temp2);
                        } else {
                            /* Variable array set min attributes which gives a growth buffer */
                            temp1 = mprintf("   minattrs %c%d,%c%d,%d\n",
                                            from_reg_type, from_reg_num,
                                            child1->register_type, child1->register_num,
                                            1 - base);
                        }
                        output_append_text(node->output, temp1);
                        free(temp1);

                        /* Link Array element */
                        if (child1->node_type == INTEGER || child1->node_type == CONSTANT) {
                            /* Constant Parameter */
                            temp1 = mprintf("   linkattr1 r%d,%c%d,%s\n",
                                            node->register_num,
                                            from_reg_type, from_reg_num,
                                            temp2);
                        } else if (base == 1) {
                            /* Already 1 base - simpler */
                            temp1 = mprintf("   linkattr1 r%d,%c%d,%c%d\n",
                                            node->register_num,
                                            from_reg_type, from_reg_num,
                                            child1->register_type, child1->register_num);
                        } else {
                            /* Need to make it 1 base */
                            temp1 = mprintf("   iadd r%d,%c%d,%d\n"
                                            "   linkattr1 r%d,%c%d,r%d\n",
                                            node->additional_registers,
                                            child1->register_type, child1->register_num,
                                            1 - base,
                                            node->register_num,
                                            from_reg_type, from_reg_num,
                                            node->additional_registers);
                        }

                        unlink_needed = 1; /* We will need to define a cleanup action to unlink */
                        output_append_text(node->output, temp1);
                        free(temp1);
                        if (temp2) free(temp2);

                        /* Call child cleanup action */
                        if (child1->cleanup) output_concat(node->output, child1->cleanup);

                        /* Loop round to the next parameter */
                        from_reg_type = 'r';
                        from_reg_num = node->register_num;
                        child1 = child1->sibling;
                    }

                    /* Set cleanup action */
                    if (unlink_needed) {
                        temp1 = mprintf("   unlink r%d\n", node->register_num);
                        node->cleanup = output_fs(temp1);
                        free(temp1);
                    }
                }
                var_symbol_end:

                if (node->node_type == VAR_SYMBOL) type_promotion(node);
                break;

            case VAR_REFERENCE:
                break;

            case NOVAL:
                /* Set the node output as null */
                if (!node->output) node->output = output_f();
                break;

            case CONSTANT:
            case CONST_SYMBOL:
            case STRING:
            case FLOAT:
            case DECIMAL:
            case INTEGER:
                emit_expression(node, payload);
                break;

            case ASSEMBLER: {
                char *arg1 = 0, *arg2 = 0, *arg3 = 0;

                /* Add source metadata */
                comment_meta = get_metaline(node);
                if (node->output) output_prepend_text(comment_meta, node->output);
                else node->output = output_fs(comment_meta);
                free(comment_meta);

                /* Add Variable Metadata */
                add_variable_metadata(node);

                /* We will build the assembler instruction */
                /* First the command */
                char* inst = mprintf("   %.*s",
                                     node->node_string_length, node->node_string);

                /* Lower case the instruction */
                int l;
                for (l = 0; inst[l]; l++) {
                    inst[l] = (char)tolower(inst[l]);
                }

                /* Argument 1 */
                if (child1) {
                    if (child1->node_type == FUNC_SYMBOL) {
                        arg1 = mprintf("%.*s()", child1->node_string_length, child1->node_string);
                    }
                    else if (child1->register_num == DONT_ASSIGN_REGISTER) { /* A constant */
                        arg1 = format_constant(child1->target_type, child1);
                    } else { /* A register */
                        output_concat(node->output, child1->output);
                        arg1 = mprintf("%c%d",
                                       child1->register_type,
                                       child1->register_num);
                    }
                }

                /* Argument 2 */
                if (child2) {
                    if (child2->node_type == FUNC_SYMBOL) {
                        arg2 = mprintf("%.*s()", child2->node_string_length, child2->node_string);
                    }
                    else if (child2->register_num == DONT_ASSIGN_REGISTER) { /* A constant */
                        arg2 = format_constant(child2->target_type, child2);
                    } else { /* A register */
                        output_concat(node->output, child2->output);
                        arg2 = mprintf("%c%d",
                                       child2->register_type,
                                       child2->register_num);
                    }
                }

                /* Argument 3 */
                if (child3) {
                    if (child3->node_type == FUNC_SYMBOL) {
                        arg3 = mprintf("%.*s()", child3->node_string_length, child3->node_string);
                    }
                    else if (child3->register_num == DONT_ASSIGN_REGISTER) { /* A constant */
                        arg3 = format_constant(child3->target_type, child3);
                    } else { /* A register */
                        output_concat(node->output, child3->output);
                        arg3 = mprintf("%c%d",
                                       child3->register_type,
                                       child3->register_num);
                    }
                }

                /* Create the whole instruction */
                if (arg3) temp1 = mprintf("%s %s,%s,%s\n", inst, arg1, arg2, arg3);
                else if (arg2) temp1 = mprintf("%s %s,%s\n", inst, arg1, arg2);
                else if (arg1) temp1 = mprintf("%s %s\n", inst, arg1);
                else temp1 = mprintf("%s\n", inst);

                /* Finally, append it to the output */
                output_append_text(node->output, temp1);

                if (child1 && child1->cleanup) output_concat(node->output, child1->cleanup);
                if (child2 && child2->cleanup) output_concat(node->output, child2->cleanup);
                if (child3 && child3->cleanup) output_concat(node->output, child3->cleanup);

                /* Clean up */
                free(temp1);
                free(inst);
                if (arg1) free(arg1);
                if (arg2) free(arg2);
                if (arg3) free(arg3);
            }
            break;

            case ASSIGN:
                /* Add source metadata */
                comment_meta = get_metaline(node);
                if (node->output) output_prepend_text(comment_meta, node->output);
                else node->output = output_fs(comment_meta);
                free(comment_meta);

                /* Add Variable Metadata */
                add_variable_metadata(node);
                output_concat(node->output, child1->output);
                output_concat(node->output, child2->output);
                if (child1->register_num != child2->register_num ||
                    child1->register_type != child2->register_type) {
                    temp1 = mprintf("   %scopy %c%d,%c%d\n",
                                    tp_prefix,
                                    child1->register_type,
                                    child1->register_num,
                                    child2->register_type,
                                    child2->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                output_concat(node->output, child2->cleanup);
                if (node->parent->node_type == REPEAT) {
                    /* Defer cleanup for repeat - the inc/to needs the register */
                    node->cleanup = child1->cleanup;
                    child1->cleanup = 0;
                }
                else output_concat(node->output, child1->cleanup);
                break;

            case NOP:
                emit_flow(node, pl);
                break;

            case SAY:
                emit_flow(node, pl);
                break;

            case RETURN:
                emit_flow(node, pl);
                break;

            case IF:
                emit_flow(node, pl);
                break;

            case DO: /* DO LOOP */
                emit_flow(node, pl);
                break;

            case REPEAT:
                emit_flow(node, pl);
                break;

            case FOR:
                emit_flow(node, pl);
                break;

            case TO:
                emit_flow(node, pl);
                break;

            case BY:
                emit_flow(node, pl);
                break;

            case WHILE:
                emit_flow(node, pl);
                break;

            case UNTIL:
                emit_flow(node, pl);
                break;

            case LEAVE:
                emit_flow(node, pl);
                break;

            case ITERATE:
                emit_flow(node, pl);
                break;

            default:;
        }
    }

    else {
        /* IN - TOP DOWN */

        switch (node->node_type) {
            case INSTRUCTIONS:
                if (!node->output) node->output = output_f();
                add_scope_initiators(node);
                break;

            default:
                break;
        }
    }

    return result_normal;
}

void emit(Context *context, FILE *output) {
    walker_payload payload;

    payload.context = context;
    payload.file = output;
    payload.globals = 0;

    ast_wlkr(context->ast, register_walker, (void *) &payload);

    ast_wlkr(context->ast, emit_walker, (void *) &payload);
}
