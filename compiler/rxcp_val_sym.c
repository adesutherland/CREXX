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
 * Validation Pass: Symbol Harvesting & Resolution
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "rxcp_val.h"

/* Step 2a
 * - Builds the Symbol Table
 */
walker_result build_symbols_walker(walker_direction direction,
                                          ASTNode* node,
                                          void *payload) {

    Context *context = (Context*)payload;
    Symbol *symbol;
    ASTNode *n;

    if (direction == in) {

        /* IN - TOP DOWN */
        if (node->scope) {
            context->current_scope = node->scope;
            return result_normal; /* Skip if we have already processed this node */
        }

        if (node->node_type == REXX_UNIVERSE) {
            /* This top level scope will contain the project file scope & imported file scopes */
            context->current_scope = scp_f(context, context->current_scope, node, 0);
            node->scope = context->current_scope;
        }

        else if (node->node_type == PROGRAM_FILE) {
            /* Now create the namespace symbol and scope */
            /* Make the new symbol */
            symbol = sym_f(context->current_scope, node);
            symbol->symbol_type = NAMESPACE_SYMBOL;
            sym_adnd(symbol, node, 1, 1);
            if (context->namespace) sym_adnd(symbol, context->namespace, 0, 1);

            /* Move down to the project file scope */
            context->current_scope = scp_f(context, context->current_scope, node, symbol);
            node->scope = context->current_scope;
        }

        else if (node->node_type == CLASS_DEF) {
            if (node->node_string_length > 0 && node->node_string[node->node_string_length - 1] == ':') {
                node->node_string_length--; /* Remove the ':' */
            }

            /* Check for duplicated */
            symbol = sym_rslv(context->current_scope, node);
            if (symbol) {
                if (symbol->scope == context->current_scope) {
                    /* Error */
                    mknd_err(node, "DUPLICATE_SYMBOL");
                }
                else symbol = 0;
            }

            /* Make a new symbol */
            if (!symbol) {
                symbol = sym_f(context->current_scope, node);
                symbol->symbol_type = CLASS_SYMBOL;
            }

            sym_adnd(symbol, node, 0, 1);

            /* Move down to the class scope */
            context->current_scope = scp_f(context, context->current_scope, node, symbol);
            node->scope = context->current_scope;
        }

        else if (node->node_type == PROCEDURE || node->node_type == METHOD || node->node_type == FACTORY) {
            if (node->node_string_length > 0 && node->node_string[node->node_string_length - 1] == ':') {
                node->node_string_length--; /* Remove the ':' */
            }

            /* Set the return value node value_type */
            n = ast_chld(node, CLASS, VOID);
            if (n) {
                n->value_type = node_to_type(n,
                                             &(n->value_dims), &(n->value_dim_base), &(n->value_dim_elements),
                                             &(n->value_class));

                /* Reset node Target Type to be the same as the node Value Type */
                ast_rttp(n);
            }

            /* Check for duplicated */
            symbol = sym_rslv(context->current_scope, node);
            if (symbol) {
                if (symbol->scope == context->current_scope) {
                    /* Error */
                    mknd_err(node, "DUPLICATE_SYMBOL");
                }
                else symbol = 0;
            }

            /* Make a new symbol */
            if (!symbol) { /* If there is a symbol we are in an error condition but are pressing on */
                symbol = sym_f(context->current_scope, node);
                symbol->symbol_type = FUNCTION_SYMBOL;
            }

            sym_adnd(symbol, node, 0, 1);

            /* Move down to the procedure scope */
            context->current_scope = scp_f(context, context->current_scope, node, symbol);
            node->scope = context->current_scope;

            /* Level B Class Instance Support */
            if (node->node_type == FACTORY) {
                /* Add instance symbol '*' */
                Symbol *star = sym_fn(context->current_scope, "*", 1);
                if (star) {
                    star->symbol_type = VARIABLE_SYMBOL;
                }
            }
        }

        else if (node->node_type == IMPORT) {
            /* Get the toplevel scope that contains all the namespaces */
            Scope* namespaces = context->ast->scope;

            /* Now create the imported namespace symbol and scope */
            /* Make the new symbol */
            symbol = sym_f(namespaces, node->child);
            if (symbol) {
                symbol->symbol_type = NAMESPACE_SYMBOL;
                sym_adnd(symbol, node->child, 0, 1);

                /* New scope scope */
                context->current_scope = scp_f(context, namespaces, node->child, symbol);
                node->scope = context->current_scope;
            }
            else {
                mknd_err(node->child, "DUPLICATE_NAMESPACE");
                node->scope = namespaces;
            }
        }

        else if (node->node_type == VAR_TARGET || node->node_type == VAR_REFERENCE) {
            if (node->symbolNode) {
                node->scope = context->current_scope;
            } else {
                node->scope = context->current_scope;
                /* Find the symbol */
                if (node->parent->node_type == ARG) /* Only search current scope */
                    symbol = sym_lrsv(context->current_scope, node);
                else /* Search parent scopes */
                    symbol = sym_rslv(context->current_scope, node);

                /* Make a new symbol if it does not exist */
                if (!symbol) {
                    symbol = sym_f(context->current_scope, node);
                } else if (symbol->symbol_type == FUNCTION_SYMBOL) {
                    mknd_err(node, "IS_A_FUNCTION");
                } else if (symbol->symbol_type == CLASS_SYMBOL) {
                    mknd_err(node, "IS_A_CLASS");
                } else if (symbol->symbol_type == NAMESPACE_SYMBOL) {
                    mknd_err(node, "IS_A_NAMESPACE");
                } else if (node->parent->node_type == DEFINE) {
                    mknd_err(node, "ALREADY_DECLARED");
                }

                /* Set Argument flags - set by the parser grammar */
                if (node->parent->node_type == ARG) {
                    symbol->is_arg = 1;
                    symbol->is_opt_arg = node->parent->is_opt_arg;
                    symbol->is_ref_arg = node->parent->is_ref_arg;
                    /* Add the count of fixed args for the procedure symbol */
                    ast_proc(node)->symbolNode->symbol->fixed_args++;
                }

                if (node->parent->node_type == ASSIGN) sym_adnd(symbol, node, 0, 1);
                else sym_adnd(symbol, node, 0, 0);
            }
        }

        else if (node->node_type == VARG || node->node_type == VARG_REFERENCE) {
            if (node->symbolNode) {
                 /* Already processed */
            } else {
                /* Set the procedure symbol has_vargs flag */
                ast_proc(node)->symbolNode->symbol->has_vargs = 1;
            }
        }

        else if (node->node_type == OP_ARG_EXISTS) {
            if (node->symbolNode) {
                node->scope = context->current_scope;
            } else {
                node->scope = context->current_scope;
                /* Find the symbol */
                symbol = sym_rslv(context->current_scope, node);

                /* At this point the arguments will have been processed so no need to attempt to add a symbol */
                if (!symbol) {
                    mknd_err(node, "UNKNOWN_SYMBOL");
                } else if (symbol->symbol_type == FUNCTION_SYMBOL) {
                    mknd_err(node, "IS_A_FUNCTION");
                } else if (symbol->symbol_type == CLASS_SYMBOL) {
                    mknd_err(node, "IS_A_CLASS");
                } else if (symbol->symbol_type == NAMESPACE_SYMBOL) {
                    mknd_err(node, "IS_A_NAMESPACE");
                } else if (!symbol->is_arg) {
                    mknd_err(node, "NOT_AN_ARGUMENT");
                } else if (!symbol->is_opt_arg) {
                    /* It's not options, so it must always exist (be specified in the call) */
                    node->node_type = INTEGER;
                    node->value_type = TP_BOOLEAN;
                    node->int_value = 1;
                    node->node_string = "1";
                    node->node_string_length = 1;
                } else sym_adnd(symbol, node, 0, 0);
            }
        }

        else if (node->node_type == VAR_SYMBOL) {
            if (node->symbolNode) {
                node->scope = context->current_scope;
            } else {
                node->scope = context->current_scope;
                /* Find the symbol */
                symbol = sym_rslv(context->current_scope, node);

                /* Make a new symbol if it does not exist */
                if (!symbol) {
                    symbol = sym_f(context->current_scope, node);
                } else if (symbol->symbol_type == FUNCTION_SYMBOL) {
                    mknd_err(node, "IS_A_FUNCTION");
                } else if (symbol->symbol_type == CLASS_SYMBOL) {
                    mknd_err(node, "IS_A_CLASS");
                } else if (symbol->symbol_type == NAMESPACE_SYMBOL) {
                    mknd_err(node, "IS_A_NAMESPACE");
                }

                if (node->parent->node_type == ASSEMBLER) {
                    /* If an assembler instruction we need to assume read/write
                     * access - and therefore disable some optimisations */
                    sym_adnd(symbol, node, 1, 1);
                } else sym_adnd(symbol, node, 1, 0);
            }
        }

        else if (node->node_type == TO) {
            node->scope = context->current_scope;
            /* Find the symbol, the parents (REPEAT)'s first child (ASSIGN)'s
             * first child (VAR_TARGET)'s symbol
             * Note: If the REPEAT has a TO it has an assign */
            symbol = node->parent->child->child->symbolNode->symbol;
            sym_adnd(symbol, node, 1, 0);
        }

        else if (node->node_type == BY) {
            node->scope = context->current_scope;
            /* Find the symbol, the parents (REPEAT)'s first child (ASSIGN)'s
             * first child (VAR_TARGET)'s symbol
             * Note: If the REPEAT has a BY it has an assign*/
            symbol = node->parent->child->child->symbolNode->symbol;
            sym_adnd(symbol, node, 1, 1); /* Increment = read & write */
        }

        else {
            node->scope = context->current_scope;
        }
    }

    else {
        /* OUT - BOTTOM UP */
        if (node->parent) context->current_scope = node->parent->scope;
        else context->current_scope = 0;
    }

    return result_normal;
}

/* Step 2b
 * - Resolve Function Symbols
 */
walker_result resolve_functions_walker(walker_direction direction,
                                              ASTNode* node,
                                              void *payload) {

    Context *context = (Context*)payload;
    Symbol *symbol;

    if (direction == in) {
        /* IN - TOP DOWN */
    }
    else {
        /* DEBUG PoC */
        if (is_node_string(node, "POCABS") || is_node_string(node, "POCSQUARE")) {
             printf("DEBUG: resolve_functions_walker sees %s as type %d\n", node->node_string, node->node_type);
        }

        if (node->node_type == FUNCTION || node->node_type == FUNC_SYMBOL) {
            if (node->symbolNode) return result_normal; /* Already Processed */
            if (ast_chld(node,ERROR,0)) return result_normal; /* Has an error - already processed */

            /* Find the symbol */
            symbol = sym_rvfc(context->ast, node);

            /* If there is a symbol and it's a function - found  */
            if (symbol && symbol->symbol_type == FUNCTION_SYMBOL ) {
                sym_adnd(symbol, node, 1, 0);
            }

            /* If there is a symbol (and it's not a function) - error */
            else if (symbol) {
                mknd_err(node, "NOT_A_FUNCTION");
            }

            /* We have to try and import the function  */
            else {
                symbol = sym_imfn(context, node);
                if (!symbol) mknd_err(node, "FUNCTION_NOT_FOUND");
                else sym_adnd(symbol, node, 1, 0);
            }
        }
    }

    return result_normal;
}

/* Step 2c
 * - Resolve Exposed Namespace Symbols
 */
walker_result exposed_symbols_walker(walker_direction direction,
                                            ASTNode* node,
                                            void *payload) {

    Context *context = (Context*)payload;
    Symbol *symbol, *merged_symbol;
    ASTNode *temp_node, *proc_node;
    int found;

    if (direction == in) {
        /* IN - TOP DOWN */
        if (node->node_type == EXPOSED) {
            if (node->parent->node_type == NAMESPACE) {
                /* We are exposing functions / variables globally to user functions */
                ASTNode* n = node->child;
                while (n) {
                    symbol = sym_rvfc(context->ast, n); /* Is this a procedure? */
                    if (!symbol) {
                        /* Procedure Symbol not found so it "must" be a variable we need to expose from our procedures */
                        found = 0;
                        /* We need to loop through all the procedures in the program_file in order */
                        for (proc_node = context->ast->child->child; proc_node; proc_node = proc_node->sibling) {
                            if (proc_node->node_type != PROCEDURE) continue;

                            /* We might be exposing one of the procedure's variables */
                            symbol = sym_lrsv(proc_node->scope, n); /* find it */
                            if (symbol && symbol->symbol_type == VARIABLE_SYMBOL && !symbol->is_arg) {
                                if (symbol->exposed == 0) { /* Avoid double processing */
                                    /* We found a variable to expose - so expose it by moving its scope */
                                    merged_symbol = sym_merg(symbol->scope->parent, symbol);
                                    /* Link to the exposed node */
                                    sym_adnd(merged_symbol, n, 1, 1);
                                    /* Link to the Procedure's INSTRUCTION node */
                                    sym_adnd(merged_symbol, ast_chld(proc_node, INSTRUCTIONS, NOP), 0, 0);
                                    merged_symbol->exposed = 1;
                                }
                                found = 1;
                            }
                        }
                        if (!found) {
                            /* Add an error - if it has not already errored */
                            if (ast_chld(n, ERROR, 0) == 0)
                                mknd_err(n, "UNKNOWN_SYMBOL");
                        }
                    }
                    else if (symbol->symbol_type ==  FUNCTION_SYMBOL) {
                        temp_node = sym_proc(symbol); /* Procedure */
                        temp_node = ast_chld(temp_node, INSTRUCTIONS, NOP); /* Instructions */
                        if (temp_node && temp_node->node_type == INSTRUCTIONS) {
                            if (symbol->exposed == 0) {
                                /* Link and expose - if not already processed */
                                sym_adnd(symbol, n, 1, 1);
                                symbol->exposed = 1;
                            }
                        }
                        else {
                            /* Add an error - if it has not already errored */
                            if (ast_chld(n, ERROR, 0) == 0)
                                mknd_err(n, "IMPORTED_FUNCTION");
                        }
                    }
                    else if (symbol->symbol_type ==  VARIABLE_SYMBOL) {
                        /* Add a warning - if it has not already errored/warned */
                        if (!context->after_rewrite && ast_chld(n, ERROR, WARNING) == 0)
                            mknd_war(n, "DUPLICATE_SYMBOL");
                    }
                    else {
                        /* Add an error - if it has not already errored */
                        if (ast_chld(n, ERROR, 0) == 0)
                            mknd_err(n, "INVALID_SYMBOL_TYPE");
                    }
                    n = n->sibling;
                }
            }

            else if (node->parent->node_type == PROCEDURE) {
                /* We are exposing variables in a procedure */
                ASTNode* n = node->child;
                while (n) {
                    /* Check if it is a global symbol (already) */
                    symbol = sym_rvfc(context->ast, n); /* Is this is a procedure/function? */
                    if (!symbol) {
                        /* It is not global yet, so we should be exposing one of the procedure's variables */
                        symbol = sym_lrsv(node->parent->scope, n); /* find it */
                        if (symbol && symbol->symbol_type == VARIABLE_SYMBOL && !symbol->is_arg) {
                            if (symbol->exposed == 0) { /* Avoid double processing */
                                /* We found a variable to expose - so expose it by moving its scope */
                                merged_symbol = sym_merg(symbol->scope->parent, symbol);
                                /* Link to the exposed node */
                                sym_adnd(merged_symbol, n, 1, 1);
                                /* Link to the Procedure's INSTRUCTION node */
                                sym_adnd(merged_symbol, ast_chld(node->parent, INSTRUCTIONS, NOP), 0, 0);
                                merged_symbol->exposed = 1;
                            }
                        }
                        else {
                            /* Add an error - if it has not already errored */
                            if (ast_chld(n, ERROR, 0) == 0) {
                                if (symbol && symbol->is_arg) mknd_err(n, "CANNOT_EXPOSED_ARG");
                                else mknd_err(n, "UNKNOWN_SYMBOL");
                            }
                        }
                    }

                    else if (symbol->symbol_type ==  VARIABLE_SYMBOL) {
                        /* Already a global symbol - does it exist in the procedure? */
                        Symbol *proc_symbol = sym_lrsv(node->parent->scope, n);
                        if (proc_symbol) {
                            if (proc_symbol->is_arg) {
                                /* If it is an arg it can't b e exposed */
                                /* Add an error - if it has not already errored */
                                if (ast_chld(n, ERROR, 0) == 0)
                                    mknd_err(n, "CANNOT_EXPOSED_ARG");
                            }
                            else {
                                /* Expose it */
                                if (proc_symbol->exposed == 0) { /* Avoid double processing */
                                    /* We need to promote this symbol to the parent scope */
                                    merged_symbol = sym_merg(proc_symbol->scope->parent, proc_symbol);
                                    /* Link to the exposed node */
                                    sym_adnd(merged_symbol, n, 1, 1);
                                    /* Link to the Procedure's INSTRUCTION node */
                                    sym_adnd(merged_symbol, ast_chld(node->parent, INSTRUCTIONS, NOP), 0, 0);
                                    merged_symbol->exposed = 1;
                                }
                            }
                        }
                        else {
                            /* Either we have already processed this symbol (duplicate) or it is not used in the proc at all */

                            if (symislnk(ast_chld(node->parent, INSTRUCTIONS, NOP), symbol)) {
                                /* It's linked to the procedure's instructions - therefore a duplicate */
                                /* Add a warning - if it has not already errored/warned */
                                if (!context->after_rewrite && ast_chld(n, ERROR, WARNING) == 0)
                                    mknd_war(n, "DUPLICATE_SYMBOL");
                            }
                            else {
                                /* Must be unused */
                                /* Add an error - if it has not already errored */
                                if (ast_chld(n, ERROR, 0) == 0)
                                    mknd_err(n, "UNKNOWN_SYMBOL");
                            }
                        }
                    }

                    else {
                        /* Add an error - if it has not already errored */
                        if (ast_chld(n, ERROR, 0) == 0)
                            mknd_err(n, "INVALID_SYMBOL_TYPE");
                    }

                    n = n->sibling;
                }
            }
        }
    }

    return result_normal;
}

/* Step 3 - Validate Symbols
   This is called for every symbol */
static void validate_symbol_in_scope(Symbol *symbol, void *payload) {
    Scope* scope = (Scope*)payload;
    SymbolNode *defining_node_link;
    ASTNode *proc, *p, *p_type;
    char *buffer;
    size_t length;
    size_t i, nix;

    if (symbol->type != TP_UNKNOWN) return; /* Already processed */

    /* Process special symbols */
    if (strcmp(symbol->name,"rc") == 0) {
        symbol->type = TP_INTEGER;
        symbol->symbol_type = VARIABLE_SYMBOL;
        symbol->value_dims = 0;
        return;
    }

    if (strcmp(symbol->name, "*") == 0 && scope->defining_node->node_type == FACTORY) {
        symbol->type = TP_OBJECT;
        symbol->symbol_type = VARIABLE_SYMBOL;
        symbol->value_dims = 0;
        if (scope->parent && scope->parent->defining_node->node_type == CLASS_DEF) {
            symbol->value_class = malloc(strlen(scope->parent->name) + 1);
            strcpy(symbol->value_class, scope->parent->name);
        }
        return;
    }

    /* This sees if we are looking at exposed (global) variables by looking at the scope defining node type */
    if ( scope->defining_node->node_type == PROGRAM_FILE && symbol->symbol_type == VARIABLE_SYMBOL) {

        proc = 0;
        /* An Exposed Symbol needs special processing */

        /* Looking at every node using this symbol */
        for (nix = 0; nix < sym_nond(symbol); nix++) {

            /* We want to find the first usage of the symbol in each procedure it is used in */
            defining_node_link = sym_trnd(symbol, nix);

            p = ast_proc(defining_node_link->node);
            if (p == proc) continue; /* We have already looked at this proc so skip */
            proc = p;

            /* Ok we are the first usage in this proc */
            if (defining_node_link->node->node_type == PROCEDURE ||
                defining_node_link->node->node_type == METHOD ||
                defining_node_link->node->node_type == FACTORY) {

                if (defining_node_link->node->node_type == FACTORY) {
                    symbol->type = TP_OBJECT;
                    /* The FACTORY symbol is in the CLASS scope */
                    if (scope->defining_node->node_type == CLASS_DEF) {
                        if (symbol->value_class) free(symbol->value_class);
                        symbol->value_class = malloc(strlen(scope->name) + 1);
                        strcpy(symbol->value_class, scope->name);
                    }
                    ast_svtp(defining_node_link->node, symbol);
                } else {
                    p_type = ast_chld(defining_node_link->node, CLASS, VOID);
                    symbol->type = node_to_type(p_type,
                                                &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                                &(symbol->value_class));

                    ast_svtp(defining_node_link->node, symbol);
                    ast_svtp(p_type, symbol);
                }
                break;
            }

            if (defining_node_link->node->node_type == VAR_REFERENCE) {
                symbol->type = node_to_type(defining_node_link->node->sibling,
                                            &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                            &(symbol->value_class));
                ast_svtp(defining_node_link->node, symbol);
                ast_svtn(defining_node_link->node->parent, defining_node_link->node);
                break;
            }

            if (defining_node_link->node->node_type == VAR_TARGET) {
                symbol->type = node_to_type(defining_node_link->node->sibling,
                                            &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                            &(symbol->value_class));

                /* The dimensions can be defined on the left-hand side (lhs) or rhs but not both and not if the rhs is a class */
                /* node_to_type() above has checked the rhs - so now we look at the lhs */
                if (symbol->value_dims == 0 && defining_node_link->node->sibling->node_type != CLASS)
                    node_to_dims(defining_node_link->node, &(symbol->value_dims),
                                 &(symbol->dim_base), &(symbol->dim_elements));

                ast_svtp(defining_node_link->node, symbol);
                ast_svtn(defining_node_link->node->parent, defining_node_link->node);
                break;
            }

            /* That's it - a global/exposed symbol can't be a taken constant so the symbol type is defined in
             * a following procedure */
        }
        /* Search other modules */
        sym_imva(scope->defining_node->context, symbol);
    }
    else {

        if (sym_nond(symbol) == 0) return;

        /* For REXX Level B the variable type is defined by its first use */
        defining_node_link = sym_trnd(symbol, 0);
        if (defining_node_link->node->node_type == PROCEDURE ||
            defining_node_link->node->node_type == METHOD ||
            defining_node_link->node->node_type == FACTORY) {
            /* This sets the procedure symbol type */
            if (defining_node_link->node->node_type == FACTORY) {
                symbol->type = TP_OBJECT;
                /* The FACTORY symbol is in the CLASS scope */
                if (scope->defining_node->node_type == CLASS_DEF) {
                    if (symbol->value_class) free(symbol->value_class);
                    symbol->value_class = malloc(strlen(scope->name) + 1);
                    strcpy(symbol->value_class, scope->name);
                }
                ast_svtp(defining_node_link->node, symbol);
            } else {
                p_type = ast_chld(defining_node_link->node, CLASS, VOID);
                symbol->type = node_to_type(p_type,
                                            &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                            &(symbol->value_class));

                ast_svtp(defining_node_link->node, symbol);
                ast_svtp(p_type, symbol);
            }
        }

        else if (defining_node_link->node->node_type == VAR_REFERENCE) {
            /* This set the variable type */
            symbol->type = node_to_type(defining_node_link->node->sibling,
                                        &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                        &(symbol->value_class));
            ast_svtp(defining_node_link->node, symbol);
            ast_svtn(defining_node_link->node->parent, defining_node_link->node);
        }

        else if (defining_node_link->node->node_type == VAR_TARGET) {
            /* This set the variable type */
            symbol->type = node_to_type(defining_node_link->node->sibling,
                                        &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                        &(symbol->value_class));

            /* The dimensions can be defined on the left-hand side (lhs) or rhs but not both and not if the rhs is a class */
            /* node_to_type() above has checked the rhs - so now we look at the lhs */
            if (symbol->value_dims == 0 && defining_node_link->node->sibling->node_type != CLASS)
                node_to_dims(defining_node_link->node, &(symbol->value_dims),
                             &(symbol->dim_base), &(symbol->dim_elements));

            ast_svtp(defining_node_link->node, symbol);
            ast_svtn(defining_node_link->node->parent, defining_node_link->node);
        }

        else if (symbol->symbol_type != NAMESPACE_SYMBOL && symbol->symbol_type != FUNCTION_SYMBOL && symbol->symbol_type != CLASS_SYMBOL) {
            /* Used without definition/declaration - Taken Constant */
            /* TODO - for Level A/C/D we will need flow analysis to determine taken constant status */
            symbol->type = TP_STRING;
            symbol->symbol_type = CONSTANT_SYMBOL;
            symbol->value_dims = 0;
            /* Update all the attached AST Nodes to be constants */
            for (i = 0; i < sym_nond(symbol); i++) {
                defining_node_link = sym_trnd(symbol, i);
                if (defining_node_link->writeUsage) {
                    /* This means we are trying to write to a TAKEN CONSTANT
                     * which is illegal in Levels B/G/L */
                    mknd_err(defining_node_link->node, "UPDATING_TAKEN_CONSTANT");
                }
                defining_node_link->node->node_type = STRING;
                defining_node_link->node->value_dims = 0;
                length = strlen(symbol->name);
                buffer = malloc(length);
                memcpy(buffer, symbol->name, length);
                ast_sstr(defining_node_link->node, buffer, length);
            }
        }
    }
}

/* Step 3.1 - Determine if variables need initialization */
static void variable_initiation(Symbol *symbol, void *payload) {
    Scope* scope = (Scope*)payload;
    SymbolNode *defining_node_link;

    if (symbol->needs_default_initiation) return; /* Already determined */

    if (symbol->type == TP_UNKNOWN) return; /* Not ready to be determined */

    if (symbol->symbol_type != VARIABLE_SYMBOL) return; /* Not a variable */

    /* Is this an argument? */
    if (symbol->is_arg) {
        /* Arguments are initialized by the caller */
        return;
    }

    /* Is this a global variable? */
    if ( scope->defining_node->node_type == PROGRAM_FILE) {
        /* Global variable unlike locals these registers are initialised by the VM
         * and for complex cases (objects) the initialisation is done by the constructor,
         * so this is a NOP */
        return;
    }

    /* An initialiser is needed if the variable is an array */
    if (symbol->value_dims > 0) {
        symbol->needs_default_initiation = 1;
        return;
    }

    /* An initialiser is needed if the variable is defined not assigned (x = .int) */
    defining_node_link = sym_trnd(symbol, 0);
    if (defining_node_link->node->parent->node_type == DEFINE) {
        symbol->needs_default_initiation = 1;
        return;
    }
}

void validate_symbols(Scope* scope) {
    int i;
    if (!scope) return;

    /* Validate Symbol */
    scp_4all(scope, validate_symbol_in_scope, scope);

    /* Handle variable implicit initiation */
    scp_4all(scope, variable_initiation, scope);

    /* Do sub scopes */
    for (i=0; i < (int)scp_noch(scope); i++) {
        validate_symbols(scp_chd(scope, i));
    }
}
