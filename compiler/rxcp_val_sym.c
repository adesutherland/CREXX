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

/* Pass 1 - Structural Map (Functions, Classes, Namespaces) */
walker_result structure_symbols_walker(walker_direction direction,
                                       ASTNode* node,
                                       void *payload) {
    Context *context = (Context*)payload;
    Symbol *symbol;
    ASTNode *n;

    if (direction == in) {
        if (node->node_type == REXX_UNIVERSE) {
            if (node->scope) {
                context->current_scope = node->scope;
            } else {
                context->current_scope = scp_f(context, context->current_scope, node, 0, SCOPE_UNIVERSE);
                node->scope = context->current_scope;
            }
        }
        else if (node->node_type == PROGRAM_FILE || node->node_type == IMPORTED_FILE) {
            if (node->node_type == PROGRAM_FILE && context->namespace) {
                symbol = sym_f(context->current_scope, context->namespace);
            } else {
                symbol = sym_f(context->current_scope, node);
            }
            symbol->symbol_type = NAMESPACE_SYMBOL;
            symbol->status = SYM_STATUS_LOCAL_DEF;
            sym_adnd(symbol, node, 1, 1);
            if (node->node_type == PROGRAM_FILE && context->namespace) sym_adnd(symbol, context->namespace, 0, 1);

            if (symbol->defines_scope) {
                context->current_scope = symbol->defines_scope;
            } else {
                context->current_scope = scp_f(context, context->current_scope, node, symbol, SCOPE_NAMESPACE);
            }
            node->scope = context->current_scope;
        }
        else if (node->node_type == CLASS_DEF) {
            if (node->node_string_length > 0 && node->node_string[node->node_string_length - 1] == ':') {
                node->node_string_length--;
            }

            symbol = sym_rslv_global(context->current_scope, node);
            if (symbol && symbol->scope == context->current_scope) {
                if (sym_trnd(symbol, 0)->node != node && !symislnk(node, symbol)) {
                    mknd_err(node, "DUPLICATE_SYMBOL");
                }
            } else {
                symbol = 0;
            }

            if (!symbol) {
                symbol = sym_f(context->current_scope, node);
                symbol->symbol_type = CLASS_SYMBOL;
                symbol->status = SYM_STATUS_LOCAL_DEF;
            }

            sym_adnd(symbol, node, 0, 1);

            /* Check for varargs */
            {
                ASTNode *args = ast_chld(node, ARGS, NOP);
                if (args) {
                    ASTNode *arg_node = args->child;
                    while (arg_node) {
                        if (arg_node->node_type == ARG && arg_node->is_varg) {
                            symbol->has_vargs = 1;
                            break;
                        }
                        arg_node = arg_node->sibling;
                    }
                }
            }

            if (symbol->defines_scope) {
                context->current_scope = symbol->defines_scope;
            } else {
                context->current_scope = scp_f(context, context->current_scope, node, symbol, SCOPE_CLASS);
            }
            node->scope = context->current_scope;
        }
        else if (node->node_type == PROCEDURE || node->node_type == METHOD || node->node_type == FACTORY) {
            if (node->node_string_length > 0 && node->node_string[node->node_string_length - 1] == ':') {
                node->node_string_length--;
            }

            n = ast_chld(node, CLASS, VOID);
            if (n) {
                n->value_type = node_to_type(context, n, &(n->value_dims), &(n->value_dim_base), &(n->value_dim_elements), &(n->value_class));
                ast_rttp(n);
            }

            symbol = sym_rslv_global(context->current_scope, node);
            if (symbol && symbol->scope == context->current_scope) {
                if (sym_trnd(symbol, 0)->node != node && !symislnk(node, symbol)) {
                    mknd_err(node, "DUPLICATE_SYMBOL");
                }
            } else {
                symbol = 0;
            }

            if (!symbol) {
                if (node->node_type == FACTORY) {
                    symbol = sym_fn(context->current_scope, "\xc2\xa7" "factory", 9);
                } else {
                    symbol = sym_f(context->current_scope, node);
                }
                symbol->symbol_type = FUNCTION_SYMBOL;
                symbol->status = SYM_STATUS_LOCAL_DEF;
            }

            sym_adnd(symbol, node, 0, 1);

            /* Check for varargs */
            {
                ASTNode *args = ast_chld(node, ARGS, NOP);
                if (args) {
                    ASTNode *arg_node = args->child;
                    while (arg_node) {
                        if (arg_node->node_type == ARG && arg_node->is_varg) {
                            symbol->has_vargs = 1;
                            break;
                        }
                        arg_node = arg_node->sibling;
                    }
                }
            }

            if (symbol->defines_scope) {
                context->current_scope = symbol->defines_scope;
            } else {
                context->current_scope = scp_f(context, context->current_scope, node, symbol, SCOPE_PROCEDURE);
            }
            node->scope = context->current_scope;

            if (node->node_type == FACTORY) {
                Symbol *star = sym_fn(context->current_scope, "\xc2\xa7" "factory", 9);
                if (star) {
                    star->symbol_type = VARIABLE_SYMBOL;
                    star->type = TP_OBJECT;
                    ASTNode *class_node = ast_class(node);
                    if (class_node && class_node->symbolNode && class_node->symbolNode->symbol) {
                        if (star->value_class) free(star->value_class);
                        star->value_class = malloc(strlen(class_node->symbolNode->symbol->name) + 1);
                        strcpy(star->value_class, class_node->symbolNode->symbol->name);
                    }
                }
            } else if (node->node_type == METHOD) {
                Symbol *this_sym = sym_fn(context->current_scope, "\xc2\xa7" "this", 6);
                if (this_sym) {
                    this_sym->symbol_type = VARIABLE_SYMBOL;
                    this_sym->type = TP_OBJECT;
                    ASTNode *class_node = ast_class(node);
                    if (class_node && class_node->symbolNode && class_node->symbolNode->symbol) {
                        if (this_sym->value_class) free(this_sym->value_class);
                        this_sym->value_class = malloc(strlen(class_node->symbolNode->symbol->name) + 1);
                        strcpy(this_sym->value_class, class_node->symbolNode->symbol->name);
                    }
                }
            }
        }
        else if (node->node_type == NAMESPACE) {
            node->scope = context->current_scope;
        }
        else if (node->node_type == IMPORT) {
            Scope* namespaces = context->ast->scope;
            symbol = sym_f(namespaces, node->child);
            if (symbol) {
                symbol->symbol_type = NAMESPACE_SYMBOL;
                symbol->status = SYM_STATUS_LOCAL_DEF;
                sym_adnd(symbol, node->child, 0, 1);
                if (symbol->defines_scope) {
                    context->current_scope = symbol->defines_scope;
                } else {
                    context->current_scope = scp_f(context, namespaces, node->child, symbol, SCOPE_NAMESPACE);
                }
                node->scope = context->current_scope;
            } else {
                mknd_err(node->child, "DUPLICATE_NAMESPACE");
                node->scope = namespaces;
            }
        }
        else {
            if (node->scope) context->current_scope = node->scope;
        }
    }
    else {
        if (node->parent) context->current_scope = node->parent->scope;
        else context->current_scope = 0;
    }

    return result_normal;
}

/* Pass 3 - Variable Symbols, Scopes and Seeding */
walker_result build_symbols_walker(walker_direction direction,
                                   ASTNode* node,
                                   void *payload) {
    Context *context = (Context*)payload;
    Symbol *symbol;
    ASTNode *n;

    if (direction == in) {
        /* Navigate using scopes created by structure_symbols_walker */
        if (node->scope) {
            context->current_scope = node->scope;
        } 
        else if (node->node_type == EXIT_OWNED) {
            context->current_scope = scp_f(context, context->current_scope, node, 0, SCOPE_LOCAL);
            node->scope = context->current_scope;
        }
        else if (node->node_type == INSTRUCTIONS) {
            if (node->parent && node->parent->node_type == INSTRUCTIONS) {
                context->current_scope = scp_f(context, context->current_scope, node, 0, SCOPE_LOCAL);
                node->scope = context->current_scope;
            } else {
                node->scope = context->current_scope;
            }
        }
        else {
            node->scope = context->current_scope;
        }

        /* Sophisticated EXPOSED Seeding */
        if (node->node_type == EXPOSED) {
            if (node->parent && (node->parent->node_type == NAMESPACE || node->parent->node_type == PROCEDURE)) {
                ASTNode *n = node->child;
                Scope *namespace_scope = context->current_scope;
                
                while (namespace_scope && namespace_scope->type != SCOPE_NAMESPACE) {
                    namespace_scope = namespace_scope->parent;
                }
                
                if (namespace_scope) {
                    while (n) {
                        if (n->node_type == VAR_SYMBOL || n->node_type == LITERAL) {
                            symbol = sym_lrsv(namespace_scope, n);
                            if (!symbol) symbol = sym_rvfc(context->ast, n); /* Imported Proc/Class */

                            if (symbol) {
                                symbol->exposed = 1;
                                if (symbol->symbol_type == VARIABLE_SYMBOL || symbol->symbol_type == UNKNOWN_SYMBOL) {
                                    symbol->is_global_var = 1;
                                    symbol->symbol_type = VARIABLE_SYMBOL;
                                }
                                
                                /* If it was from a PROCEDURE EXPOSE, make sure the local proc also links to it */
                                if (node->parent && node->parent->node_type == PROCEDURE) {
                                    sym_adnd(symbol, n, 1, 1); 
                                    ASTNode *instr = ast_chld(node->parent, INSTRUCTIONS, NOP);
                                    if (instr) sym_adnd(symbol, instr, 0, 0); 
                                } else {
                                    sym_adnd(symbol, n, 1, 1);
                                }
                            } else {
                                /* Not found locally or globally as a procedure. Assume Variable. */
                                symbol = sym_f(namespace_scope, n);
                                symbol->symbol_type = VARIABLE_SYMBOL;
                                symbol->status = SYM_STATUS_UNRESOLVED; /* Let sym_imva check it */
                                sym_imva(context, symbol);
                                
                                if (symbol->status != SYM_STATUS_RESOLVED_GLOBAL) {
                                    symbol->status = SYM_STATUS_LOCAL_VAR;
                                }
                                symbol->exposed = 1;
                                symbol->is_global_var = 1;
                                
                                if (node->parent && node->parent->node_type == PROCEDURE) {
                                    sym_adnd(symbol, n, 1, 1);
                                    ASTNode *instr = ast_chld(node->parent, INSTRUCTIONS, NOP);
                                    if (instr) sym_adnd(symbol, instr, 0, 0);
                                } else {
                                    sym_adnd(symbol, n, 1, 1);
                                }
                            }
                        }
                        n = n->sibling;
                    }
                }
            }
        }

        else if (node->node_type == VAR_TARGET || node->node_type == VAR_REFERENCE || node->node_type == VAR_SYMBOL) {
            if (node->symbolNode) {
                node->scope = context->current_scope;
                return result_normal;
            } else {
                if (node->parent->node_type == ARG) {
                    symbol = sym_lrsv(context->current_scope, node);
                } else if (node->parent->node_type == DEFINE) {
                    symbol = sym_lrsv(context->current_scope, node);
                    if (!symbol) {
                        if (context->current_scope->type == SCOPE_PROCEDURE || context->current_scope->type == SCOPE_NAMESPACE) {
                            Symbol *glob = sym_rslv_global(context->current_scope, node);
                            if (glob && glob->symbol_type == VARIABLE_SYMBOL && glob->exposed) {
                                symbol = glob;
                            }
                        }
                    }
                } else {
                    symbol = sym_rslv_tiered(context->current_scope, node);
                }

                if (!symbol) {
                    symbol = sym_f(context->current_scope, node);
                    symbol->symbol_type = VARIABLE_SYMBOL;
                    symbol->status = SYM_STATUS_LOCAL_VAR;
                } else if (symbol->symbol_type == NAMESPACE_SYMBOL) {
                    mknd_err(node, "IS_A_NAMESPACE");
                } else if (node->parent->node_type == DEFINE) {
                    int allow_redef = 0;
                    if (symbol->is_global_var && (context->current_scope->type == SCOPE_PROCEDURE || context->current_scope->type == SCOPE_NAMESPACE)) {
                        allow_redef = 1;
                    }
                    if (!allow_redef) {
                        mknd_err(node, "ALREADY_DECLARED");
                    }
                }

                if (node->parent->node_type == ARG) {
                    symbol->is_arg = 1;
                }

                sym_adnd(symbol, node, (node->node_type == VAR_REFERENCE || node->node_type == VAR_SYMBOL), (node->node_type == VAR_TARGET));
                node->scope = context->current_scope;
            }
        }

        else if (node->node_type == DO) {
            node->scope = context->current_scope;
        }

        else if (node->node_type == UNKNOWN && is_node_string(node, "LEVELB")) {
            return request_skip;
        }
        else {
            node->scope = context->current_scope;
        }
    }

    else {
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
        if (node->scope) context->current_scope = node->scope;
    }
    else {
        if (node->node_type == FUNCTION || node->node_type == FUNC_SYMBOL) {
            /* Find the symbol */
            Symbol *local_symbol = sym_rslv_tiered(node->scope ? node->scope : context->current_scope, node);

            if (local_symbol && local_symbol->status == SYM_STATUS_LOCAL_DEF && local_symbol->symbol_type == FUNCTION_SYMBOL ) {
                if (!node->symbolNode) {
                    sym_adnd(local_symbol, node, 1, 0);
                    context->changed_flags |= FLAG_VAL_SYM;
                }
            } else {
                /* Try global search */
                symbol = sym_rvfc(context->ast, node);
                if (symbol && symbol->symbol_type == FUNCTION_SYMBOL ) {
                    /* Option A: Redirect if already linked to an UNRESOLVED local symbol */
                    if (node->symbolNode && node->symbolNode->symbol->status == SYM_STATUS_UNRESOLVED && node->symbolNode->symbol != symbol) {
                        sym_dno(node->symbolNode->symbol, node);
                    }
                    if (!node->symbolNode) {
                        sym_adnd(symbol, node, 1, 0);
                        context->changed_flags |= FLAG_VAL_SYM;
                    }
                } else {
                    /* Try BIFs */
                    symbol = sym_imfn(context, node);
                    if (symbol) {
                        /* Option A: Redirect if already linked to an UNRESOLVED local symbol */
                        if (node->symbolNode && node->symbolNode->symbol->status == SYM_STATUS_UNRESOLVED && node->symbolNode->symbol != symbol) {
                            sym_dno(node->symbolNode->symbol, node);
                        }
                        if (!node->symbolNode) {
                            sym_adnd(symbol, node, 1, 0);
                            context->changed_flags |= FLAG_VAL_SYM;
                        }
                    } else if (local_symbol && local_symbol->status != SYM_STATUS_UNRESOLVED) {
                        /* Found something locally but it's not a function, and no global function found */
                        if (!ast_chld(node, ERROR, 0)) mknd_err(node, "NOT_A_FUNCTION");
                    } else if (context->after_rewrite) {
                        /* Not found anywhere */
                        if (!ast_chld(node, ERROR, 0)) mknd_err(node, "FUNCTION_NOT_FOUND");
                    }
                }
            }
        }

        if (node->parent) context->current_scope = node->parent->scope;
        else context->current_scope = 0;
    }

    return result_normal;
}

static void expose_class_symbols_worker(Symbol *symbol, void *payload) {
    ASTNode *n = (ASTNode*)payload;
    if (symbol->symbol_type == FUNCTION_SYMBOL) {
        if (symbol->exposed == 0) {
            symbol->exposed = 1;
            sym_adnd(symbol, n, 0, 0);
        }
    }
}

struct val_sym_payload {
    Context *context;
    Scope *scope;
};

/* Step 3 - Validate Symbols
   This is called for every symbol */
static void validate_symbol_in_scope(Symbol *symbol, void *payload) {
    ValueType old_type_dbg = symbol->type;
    size_t old_dims_dbg = symbol->value_dims;
    struct val_sym_payload *pld = (struct val_sym_payload*)payload;
    Context *context = pld->context;
    Scope* scope = pld->scope;
    SymbolNode *defining_node_link;
    ASTNode *proc, *p, *p_type;
    char *buffer;
    size_t length;
    size_t i, nix;
    ValueType old_type = symbol->type;
    size_t old_dims = symbol->value_dims;

    if (context->after_rewrite && !symbol->exposed && sym_nond(symbol) > 0) {
        ASTNode *rep_node = sym_trnd(symbol, 0)->node;
        if (rep_node && rep_node->node_string && rep_node->node_string_length > 0) {
            if (symbol->status == SYM_STATUS_LOCAL_VAR) {
                /* Rule: Warn on variables shadowing variables (local or global) */
                int shadows_var = 0;
                Symbol *parent_sym = 0;
                if (symbol->scope && symbol->scope->parent) {
                    parent_sym = sym_rslv_local(symbol->scope->parent, rep_node);
                    if (!parent_sym) parent_sym = sym_rslv_global(symbol->scope->parent, rep_node);
                }
                if (parent_sym && parent_sym->symbol_type == VARIABLE_SYMBOL) {
                    shadows_var = 1;
                } else if (sym_is_glob_var(context, rep_node)) {
                    shadows_var = 1;
                }
                if (shadows_var) {
                    symbol->is_shadowing = 1;
                }
            } else if (symbol->status == SYM_STATUS_LOCAL_DEF) {
                if (symbol->symbol_type == FUNCTION_SYMBOL) {
                    if (sym_is_imfn(context, rep_node)) {
                        symbol->is_shadowing = 1;
                    }
                } else if (symbol->symbol_type == CLASS_SYMBOL) {
                    if (sym_is_imcls(context, rep_node)) {
                        symbol->is_shadowing = 1;
                    }
                }
            }
            if (symbol->is_shadowing) {
                /* No warning emitted here, just flag the symbol */
            }
        }
    }

    if (symbol->type != TP_UNKNOWN) goto exit;

    /* Transition UNRESOLVED to LOCAL_VAR/CONSTANT if we are in finalization */
    if (symbol->status == SYM_STATUS_UNRESOLVED && context->after_rewrite) {
        /* Final attempt at global resolution (e.g. global variables) */
        sym_imva(context, symbol);
        if (symbol->status == SYM_STATUS_UNRESOLVED) {
            symbol->status = SYM_STATUS_LOCAL_VAR;
        }
    }

    /* Process special symbols */
    if (symbol->is_rc) {
        symbol->type = TP_INTEGER;
        symbol->symbol_type = VARIABLE_SYMBOL;
        symbol->value_dims = 0;
        return;
    }

    if (symbol->is_factory && scope->defining_node && scope->defining_node->node_type == FACTORY) {
        symbol->type = TP_OBJECT;
        symbol->symbol_type = VARIABLE_SYMBOL;
        symbol->value_dims = 0;
        if (scope->parent && scope->parent->defining_node && scope->parent->defining_node->node_type == CLASS_DEF) {
            if (symbol->value_class) free(symbol->value_class);
            symbol->value_class = malloc(strlen(scope->parent->name) + 1);
            strcpy(symbol->value_class, scope->parent->name);
        }
        return;
    }

    if (symbol->is_this && scope->defining_node && scope->defining_node->node_type == METHOD) {
        symbol->type = TP_OBJECT;
        symbol->symbol_type = VARIABLE_SYMBOL;
        symbol->value_dims = 0;
        if (scope->parent && scope->parent->defining_node && scope->parent->defining_node->node_type == CLASS_DEF) {
            if (symbol->value_class) free(symbol->value_class);
            symbol->value_class = malloc(strlen(scope->parent->name) + 1);
            strcpy(symbol->value_class, scope->parent->name);
        }
        return;
    }

    /* This sees if we are looking at exposed (global) variables by looking at the scope type */
    if (scope->type == SCOPE_NAMESPACE && symbol->symbol_type == VARIABLE_SYMBOL) {

        proc = 0;
        /* An Exposed Symbol needs special processing */

        /* Looking at every node using this symbol */
        for (nix = 0; nix < sym_nond(symbol); nix++) {

            /* We want to find the first usage of the symbol in each procedure it is used in */
            defining_node_link = sym_trnd(symbol, nix);

            /* Ignore EXPOSED nodes when trying to determine the variable's type and dimensions */
            if (defining_node_link->node && defining_node_link->node->parent && defining_node_link->node->parent->node_type == EXPOSED) continue;

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
                    if (scope->defining_node && scope->defining_node->node_type == CLASS_DEF) {
                        if (symbol->value_class) free(symbol->value_class);
                        symbol->value_class = malloc(strlen(scope->name) + 1);
                        strcpy(symbol->value_class, scope->name);
                    }
                    ast_svtp(defining_node_link->node, symbol);
                } else {
                    p_type = ast_chld(defining_node_link->node, CLASS, VOID);
                    symbol->type = node_to_type(context, p_type,
                                                &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                                &(symbol->value_class));

                    ast_svtp(defining_node_link->node, symbol);
                    ast_svtp(p_type, symbol);
                }
                break;
            }

            if (defining_node_link->node->node_type == VAR_REFERENCE) {
                symbol->type = node_to_type(context, defining_node_link->node->sibling,
                                            &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                            &(symbol->value_class));
                ast_svtp(defining_node_link->node, symbol);
                ast_svtn(defining_node_link->node->parent, defining_node_link->node);
                break;
            }

            if (defining_node_link->node->node_type == VAR_TARGET) {
                symbol->type = node_to_type(context, defining_node_link->node->sibling,
                                            &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                            &(symbol->value_class));

                /* The dimensions can be defined on the left-hand side (lhs) or rhs but not both and not if the rhs is a class */
                /* node_to_type(context, ) above has checked the rhs - so now we look at the lhs */
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

        /* For REXX Level B the variable type is defined by its first use that is NOT an EXPOSED node */
        size_t nidx = 0;
        defining_node_link = sym_trnd(symbol, nidx);
        while (defining_node_link && defining_node_link->node && defining_node_link->node->parent && defining_node_link->node->parent->node_type == EXPOSED) {
            nidx++;
            if (nidx >= sym_nond(symbol)) break;
            defining_node_link = sym_trnd(symbol, nidx);
        }
        if (!defining_node_link || (defining_node_link->node && defining_node_link->node->parent && defining_node_link->node->parent->node_type == EXPOSED)) return;

        if (defining_node_link->node->node_type == PROCEDURE ||
            defining_node_link->node->node_type == METHOD ||
            defining_node_link->node->node_type == FACTORY) {
            /* This sets the procedure symbol type */
            if (defining_node_link->node->node_type == FACTORY) {
                symbol->type = TP_OBJECT;
                /* The FACTORY symbol is in the CLASS scope */
                if (scope->defining_node && scope->defining_node->node_type == CLASS_DEF) {
                    if (symbol->value_class) free(symbol->value_class);
                    symbol->value_class = malloc(strlen(scope->name) + 1);
                    strcpy(symbol->value_class, scope->name);
                }
                ast_svtp(defining_node_link->node, symbol);
                } else {
                p_type = ast_chld(defining_node_link->node, CLASS, VOID);
                symbol->type = node_to_type(context, p_type,
                                            &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                            &(symbol->value_class));

                ast_svtp(defining_node_link->node, symbol);
                ast_svtp(p_type, symbol);
                
            }
        }

        else if (defining_node_link->node->node_type == VAR_REFERENCE) {
            /* This set the variable type */
            symbol->type = node_to_type(context, defining_node_link->node->sibling,
                                        &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                        &(symbol->value_class));
            ast_svtp(defining_node_link->node, symbol);
            ast_svtn(defining_node_link->node->parent, defining_node_link->node);
            
        }

        else if (defining_node_link->node->node_type == VAR_TARGET) {
            /* This set the variable type */
            symbol->type = node_to_type(context, defining_node_link->node->sibling,
                                        &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                        &(symbol->value_class));

            /* The dimensions can be defined on the left-hand side (lhs) or rhs but not both and not if the rhs is a class */
            /* node_to_type(context, ) above has checked the rhs - so now we look at the lhs */
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
            if (symbol->status == SYM_STATUS_UNRESOLVED) symbol->status = SYM_STATUS_LOCAL_VAR;
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
                ast_svtp(defining_node_link->node, symbol);
            }
        }
    }

    if (symbol->name && (strcmp(symbol->name, "a") == 0)) 
        
exit:
    if (symbol->type != old_type || symbol->value_dims != old_dims) {
        context->changed_flags |= FLAG_VAL_SYM;
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
    if (scope->type == SCOPE_NAMESPACE) {
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
    if (sym_nond(symbol) > 0) {
        defining_node_link = sym_trnd(symbol, 0);
        if (defining_node_link->node->parent->node_type == DEFINE) {
            symbol->needs_default_initiation = 1;
            return;
        }
    }
}

void validate_symbols(Context *context, Scope* scope) {
    int i;
    struct val_sym_payload payload;
    if (!scope) return;

    payload.context = context;
    payload.scope = scope;

    /* Validate Symbol */
    scp_4all(scope, validate_symbol_in_scope, &payload);

    /* Handle variable implicit initiation */
    scp_4all(scope, variable_initiation, scope);

    /* Do sub scopes */
    for (i=0; i < (int)scp_noch(scope); i++) {
        validate_symbols(context, scp_chd(scope, i));
    }
}
