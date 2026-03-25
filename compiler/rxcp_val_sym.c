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
#include "utf.h"
#include "rxcp_val.h"

/* Step 2a
 * - Builds the Structure Symbol Table (Pass 1)
 *   Handles Namespaces, Classes, and Procedures.
 */
walker_result structure_symbols_walker(walker_direction direction,
                                              ASTNode* node,
                                              void *payload) {

    Context *context = (Context*)payload;
    Symbol *symbol;
    ASTNode *n;

    if (direction == in) {

        /* IN - TOP DOWN */

        if (node->node_type == REXX_UNIVERSE) {
            if (node->scope) {
                context->current_scope = node->scope;
            } else {
                /* This top level scope will contain the project file scope & imported file scopes */
                context->current_scope = scp_f(context, context->current_scope, node, 0, SCOPE_UNIVERSE);
                node->scope = context->current_scope;
            }
        }

        else if (node->node_type == PROGRAM_FILE || node->node_type == IMPORTED_FILE) {
            /* Now create the namespace symbol and scope */
            /* Make the new symbol */
            if (node->node_type == PROGRAM_FILE && context->namespace) {
                symbol = sym_f(context->current_scope, context->namespace);
            } else {
                symbol = sym_f(context->current_scope, node);
            }
            sym_promote_symtype(context, symbol, NAMESPACE_SYMBOL);
            sym_promote_status(context, symbol, SYM_STATUS_LOCAL_DEF);
            sym_adnd(symbol, node, 1, 1);
            if (node->node_type == PROGRAM_FILE && context->namespace) sym_adnd(symbol, context->namespace, 0, 1);

            /* Move down to the project file scope */
            if (symbol->defines_scope) {
                context->current_scope = symbol->defines_scope;
            } else {
                context->current_scope = scp_f(context, context->current_scope, node, symbol, SCOPE_NAMESPACE);
            }
            node->scope = context->current_scope;
        }

        else if (node->node_type == CLASS_DEF) {
            if (node->node_string_length > 0 && node->node_string[node->node_string_length - 1] == ':') {
                node->node_string_length--; /* Remove the ':' */
            }

            /* Check for duplicated */
            symbol = sym_rslv_global(context->current_scope, node);
            if (symbol && symbol->scope == context->current_scope) {
                /* If it's a different node, then it's a duplicate */
                if (sym_trnd(symbol, 0)->node != node) {
                    mknd_err(node, "DUPLICATE_SYMBOL");
                }
            } else {
                symbol = 0;
            }

            /* Make a new symbol */
            if (!symbol) {
                symbol = sym_f(context->current_scope, node);
                sym_promote_symtype(context, symbol, CLASS_SYMBOL);
                sym_promote_status(context, symbol, SYM_STATUS_LOCAL_DEF);
            }

            sym_adnd(symbol, node, 0, 1);

            /* Move down to the class scope */
            if (symbol->defines_scope) {
                context->current_scope = symbol->defines_scope;
            } else {
                context->current_scope = scp_f(context, context->current_scope, node, symbol, SCOPE_CLASS);
            }
            node->scope = context->current_scope;
        }


        else if (node->node_type == PROCEDURE || node->node_type == METHOD || node->node_type == FACTORY) {
            if (node->node_string_length > 0 && node->node_string[node->node_string_length - 1] == ':') {
                node->node_string_length--; /* Remove the ':' */
            }

            /* Set the return value node value_type */
            n = ast_chld(node, CLASS, VOID);
            if (n) {
                size_t dims = 0;
                int *db = 0, *de = 0;
                char *cn = 0;
                ValueType vt = node_to_type(context, n, &dims, &db, &de, &cn);
                ast_promote_type(context, n, vt, dims, db, de, cn);
                if (db) free(db);
                if (de) free(de);
                if (cn) free(cn);

                /* Reset node Target Type to be the same as the node Value Type */
                ast_rttp(n);
            }

            /* Check for duplicated */
            symbol = sym_rslv_global(context->current_scope, node);
            if (symbol && symbol->scope == context->current_scope) {
                /* If it's a different node, then it's a duplicate */
                if (sym_trnd(symbol, 0)->node != node) {
                    mknd_err(node, "DUPLICATE_SYMBOL");
                }
            } else {
                symbol = 0;
            }

            /* Make a new symbol */
            if (!symbol) { /* If there is a symbol we are in an error condition but are pressing on */
                if (node->node_type == FACTORY) {
                    symbol = sym_fn(context->current_scope, "§factory", 9);
                } else {
                    symbol = sym_f(context->current_scope, node);
                }
                sym_promote_symtype(context, symbol, FUNCTION_SYMBOL);
                sym_promote_status(context, symbol, SYM_STATUS_LOCAL_DEF);
            }

            sym_adnd(symbol, node, 0, 1);

            /* Pass 1 Varargs Detection */
            ASTNode *args = ast_chld(node, ARGS, 0);
            if (args) {
                ASTNode *arg = args->child;
                while (arg) {
                    if (arg->node_type == ARG) {
                        ASTNode *varg = ast_chld(arg, VARG, 0);
                        if (!varg) varg = ast_chld(arg, VARG_REFERENCE, 0);
                        if (varg) {
                            symbol->has_vargs = 1;
                        }
                    }
                    arg = arg->sibling;
                }
            }

            /* Move down to the procedure scope */
            if (symbol->defines_scope) {
                context->current_scope = symbol->defines_scope;
            } else {
                context->current_scope = scp_f(context, context->current_scope, node, symbol, SCOPE_PROCEDURE);
            }
            node->scope = context->current_scope;
        }

        else if (node->node_type == IMPORT) {
            /* Get the toplevel scope that contains all the namespaces */
            Scope* namespaces = context->ast->scope;

            /* Now create the imported namespace symbol and scope */
            /* Make the new symbol */
            symbol = sym_f(namespaces, node->child);
            if (symbol) {
                sym_promote_symtype(context, symbol, NAMESPACE_SYMBOL);
                sym_promote_status(context, symbol, SYM_STATUS_LOCAL_DEF);
                sym_adnd(symbol, node->child, 0, 1);

                /* New scope scope */
                if (symbol->defines_scope) {
                    context->current_scope = symbol->defines_scope;
                } else {
                    context->current_scope = scp_f(context, namespaces, node->child, symbol, SCOPE_NAMESPACE);
                }
                node->scope = context->current_scope;
            }
            else {
                mknd_err(node->child, "DUPLICATE_NAMESPACE");
                node->scope = namespaces;
            }
        }

        else if (node->node_type == BLOCK_EXPR ||
                 (node->node_type == INSTRUCTIONS && node->force_local_scope)) {
            if (node->scope) {
                context->current_scope = node->scope;
            } else {
                context->current_scope = scp_f(context, context->current_scope, node, 0, SCOPE_LOCAL);
                node->scope = context->current_scope;
            }
        }

        else if (node->node_type == DO) {
            /* Create a new scope for every DO block (simple, counted, or conditional) */
            if (node->scope) {
                context->current_scope = node->scope;
            } else {
                context->current_scope = scp_f(context, context->current_scope, node, 0, SCOPE_LOCAL);
                node->scope = context->current_scope;
            }
        }

        else if (node->node_type == INSTRUCTIONS) {
            /* Create a child scope for any nested INSTRUCTIONS whose parent is an IF or another INSTRUCTIONS block.
             * This handles THEN/ELSE bodies and simple DO blocks (which are represented as nested INSTRUCTIONS).
             * If the parent is a DO, we use the DO's scope created above. */
            if (node->force_local_scope ||
                (node->parent && (node->parent->node_type == IF || node->parent->node_type == INSTRUCTIONS))) {
                if (node->scope) {
                    context->current_scope = node->scope;
                } else {
                    context->current_scope = scp_f(context, context->current_scope, node, 0, SCOPE_LOCAL);
                    node->scope = context->current_scope;
                }
            } else {
                node->scope = context->current_scope;
            }
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
 * - Builds the full Symbol Table (Pass 3)
 *   Handles Instructions, Variables, and Exposed lists.
 */
walker_result build_symbols_walker(walker_direction direction,
                                          ASTNode* node,
                                          void *payload) {

    Context *context = (Context*)payload;
    Symbol *symbol;
    ASTNode *n;

    if (direction == in) {

        /* IN - TOP DOWN */

        if (node->node_type == REXX_UNIVERSE || node->node_type == PROGRAM_FILE || node->node_type == IMPORTED_FILE ||
            node->node_type == CLASS_DEF || node->node_type == PROCEDURE || node->node_type == METHOD ||
            node->node_type == FACTORY || node->node_type == IMPORT ||
            node->node_type == DO || node->node_type == INSTRUCTIONS ||
            node->node_type == BLOCK_EXPR) {
            /* Pass 1 has already created these scopes and symbols. Just navigate. */
            if (node->scope) {
                context->current_scope = node->scope;
            } else if (node->node_type == DO ||
                      node->node_type == BLOCK_EXPR ||
                      (node->node_type == INSTRUCTIONS &&
                       (node->force_local_scope ||
                        (node->parent &&
                         (node->parent->node_type == IF || node->parent->node_type == INSTRUCTIONS))))) {
                /* If scope is missing, create it (handles nodes added by exits) */
                context->current_scope = scp_f(context, context->current_scope, node, 0, SCOPE_LOCAL);
                node->scope = context->current_scope;
                context->changed_flags |= FLAG_VAL_SYM;
            }
            /* Level B Class Instance Support (Idempotent) */
            if (node->node_type == FACTORY) {
                /* Add instance symbol '§factory' */
                Symbol *star = sym_fn(context->current_scope, "§factory", 9);
                if (star) {
                    if (star->type == TP_UNKNOWN) {
                        sym_promote_symtype(context, star, VARIABLE_SYMBOL);
                        ASTNode *class_node = ast_class(node);
                        if (class_node && class_node->symbolNode && class_node->symbolNode->symbol) {
                            sym_promote_type(context, star, TP_OBJECT, 0, 0, 0, class_node->symbolNode->symbol->name);
                        } else {
                            sym_promote_type(context, star, TP_OBJECT, 0, 0, 0, 0);
                        }
                    }
                    /* Sync ordinal in every iteration */
                    star->creation_node = node;
                    star->creation_ordinal = node->low_ordinal;
                }
            } else if (node->node_type == METHOD) {
                /* Add instance symbol '§this' */
                Symbol *this_sym = sym_fn(context->current_scope, "§this", 6);
                if (this_sym) {
                    if (this_sym->type == TP_UNKNOWN) {
                        sym_promote_symtype(context, this_sym, VARIABLE_SYMBOL);
                        ASTNode *class_node = ast_class(node);
                        if (class_node && class_node->symbolNode && class_node->symbolNode->symbol) {
                            sym_promote_type(context, this_sym, TP_OBJECT, 0, 0, 0, class_node->symbolNode->symbol->name);
                        } else {
                            sym_promote_type(context, this_sym, TP_OBJECT, 0, 0, 0, 0);
                        }
                    }
                    /* Sync ordinal in every iteration */
                    this_sym->creation_node = node;
                    this_sym->creation_ordinal = node->low_ordinal;
                }
            }
        }

        else if (node->node_type == EXPOSED) {
            /* Pass 3: Sophisticated Seeding
             * If the variable is in an EXPOSED list, we ensure it's seeded as a global
             * VARIABLE_SYMBOL in the namespace scope before it's encountered as a local.
             */
            n = node->child;
            while (n) {
                if (n->symbolNode) {
                    n = n->sibling;
                    continue; /* Idempotency Guard */
                }

                /* Try to find it as a global function or variable */
                symbol = sym_rslv_global(context->current_scope, n);

                if (!symbol) {
                    /* Seed as global variable if not found anywhere */
                    Scope *namespace_scope = context->current_scope;
                    while (namespace_scope && namespace_scope->type != SCOPE_NAMESPACE) {
                        namespace_scope = namespace_scope->parent;
                    }
                    if (namespace_scope) {
                        symbol = sym_f(namespace_scope, n);
                        sym_promote_symtype(context, symbol, VARIABLE_SYMBOL);
                        sym_promote_status(context, symbol, SYM_STATUS_LOCAL_VAR);
                        symbol->exposed = 1;
                        symbol->is_global_var = 1;
                        context->changed_flags |= FLAG_VAL_SYM;
                    }
                }

                if (symbol) {
                    sym_adnd(symbol, n, 1, 0);
                    if (symbol->symbol_type == VARIABLE_SYMBOL || symbol->symbol_type == UNKNOWN_SYMBOL) {
                        if (symbol->exposed == 0) {
                            symbol->exposed = 1;
                            symbol->is_global_var = 1;
                            context->changed_flags |= FLAG_VAL_SYM;
                        }
                    }
                    /* Ensure creation info is set for exposed/global variables */
                    if (symbol->creation_ordinal == -1) {
                        symbol->creation_ordinal = 0;
                    }
                    /* Link to the Procedure's INSTRUCTIONS if applicable */
                    ASTNode *proc = ast_proc(node);
                    if (proc) {
                        ASTNode *instr = ast_chld(proc, INSTRUCTIONS, NOP);
                        if (instr) sym_adnd(symbol, instr, 0, 0);
                    }
                }
                n = n->sibling;
            }
            return request_skip;
        }

        else if (node->node_type == VAR_TARGET || node->node_type == VAR_REFERENCE) {
            if (node->symbolNode) {
                node->scope = context->current_scope;
            } else {
                node->scope = context->current_scope;
                /* Find the symbol */
                if (node->parent->node_type == ARG) {
                    /* Arguments are local to the current scope */
                    symbol = sym_lrsv(context->current_scope, node);
                } else if (node->parent->node_type == DEFINE) {
                    /* Typed declarations shadow in local blocks, but assert in procedures/namespaces */
                    if (context->current_scope->type == SCOPE_LOCAL) {
                        /* Force shadowing in local blocks */
                        symbol = NULL;
                    } else {
                        /* Bind to existing (e.g. exposed global) in procedure/namespace scope */
                        symbol = sym_lrsv(context->current_scope, node);
                        if (!symbol) {
                            /* Auto-expose: If it's an exposed global, let's type the global, not shadow it */
                            Symbol *glob = sym_rslv_global(context->current_scope, node);
                            if (glob && glob->symbol_type == VARIABLE_SYMBOL && glob->exposed) {
                                symbol = glob;
                            }
                        }
                    }
                } else {
                    /* Untyped usage resolves outward; if not found, will create in procedure scope (hoisting) */
                    symbol = sym_rslv_tiered(context->current_scope, node);
                    if (symbol) {
                        if (symbol->symbol_type != VARIABLE_SYMBOL && symbol->symbol_type != CONSTANT_SYMBOL) {
                            /* This is a function or class shadowing candidate.
                             * In REXX, a variable can shadow a function of the same name.
                             * So we ignore the non-variable symbol and force a new local variable. */
                            symbol = NULL;
                        } else {
                            /* TEMPORAL CHECK: Only bind if the symbol was used/defined EARLIER in the source */
                            if (symbol->creation_ordinal != -1 && !symbol->is_global_var && symbol->creation_ordinal > node->high_ordinal) {
                                /* The existing symbol appears later in the source.
                                 * In Level B, we ignore "future" definitions to allow local creation/hoisting. */
                                symbol = NULL;
                            }
                        }
                    }
                }

                /* Make a new symbol if it does not exist */
                if (!symbol) {
                    Scope *target_scope = context->current_scope;

                    symbol = sym_f(target_scope, node);
                    sym_promote_status(context, symbol, SYM_STATUS_LOCAL_VAR);
                } else if (node->parent->node_type == DEFINE && symbol->type != TP_UNKNOWN) {
                    mknd_err(node, "ALREADY_DECLARED");
                }

                if (symbol && symbol->symbol_type == UNKNOWN_SYMBOL) {
                    sym_promote_symtype(context, symbol, VARIABLE_SYMBOL);
                }

                /* Ensure creation info is set */
                if (symbol) {
                    if (symbol->creation_node == 0 || symbol->creation_node == node) {
                        symbol->creation_node = node;
                        symbol->creation_ordinal = node->high_ordinal;
                    } else if (node->parent->node_type == DEFINE && symbol->creation_node->parent && symbol->creation_node->parent->node_type != DEFINE) {
                        symbol->creation_node = node;
                        symbol->creation_ordinal = node->high_ordinal;
                    } else if (node->high_ordinal < symbol->creation_ordinal) {
                        symbol->creation_node = node;
                        symbol->creation_ordinal = node->high_ordinal;
                    }
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
                symbol = sym_rslv_local(context->current_scope, node);

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


        else if (node->node_type == VAR_SYMBOL || (node->node_type == EXIT_TOKEN && node->token && node->token->token_type == TK_VAR_SYMBOL)) {
            if (node->symbolNode) {
                node->scope = context->current_scope;
            } else {
                node->scope = context->current_scope;
                /* Find the symbol */
                symbol = sym_rslv_tiered(context->current_scope, node);
                if (symbol) {
                    if (symbol->symbol_type != VARIABLE_SYMBOL && symbol->symbol_type != CONSTANT_SYMBOL) {
                        /* Shadowing: Ignore existing non-variable symbols */
                        symbol = NULL;
                    } else {
                        /* TEMPORAL CHECK: Only bind if the symbol was used/defined EARLIER in the source */
                        if (symbol->creation_ordinal != -1 && !symbol->is_global_var && symbol->creation_ordinal > node->high_ordinal) {
                            symbol = NULL;
                        }
                    }
                }

                /* Make a new symbol if it does not exist */
                if (!symbol) {
                    Scope *target_scope = context->current_scope;

                    symbol = sym_f(target_scope, node);
                    sym_promote_status(context, symbol, SYM_STATUS_UNRESOLVED);
                }

                if (symbol && symbol->symbol_type == UNKNOWN_SYMBOL) {
                    sym_promote_symtype(context, symbol, VARIABLE_SYMBOL);
                }

                /* Ensure creation info is set */
                if (symbol) {
                    if (symbol->creation_node == 0 || symbol->creation_node == node) {
                        symbol->creation_node = node;
                        symbol->creation_ordinal = node->high_ordinal;
                    } else if (node->parent->node_type == DEFINE && symbol->creation_node->parent && symbol->creation_node->parent->node_type != DEFINE) {
                        symbol->creation_node = node;
                        symbol->creation_ordinal = node->high_ordinal;
                    } else if (node->high_ordinal < symbol->creation_ordinal) {
                        symbol->creation_node = node;
                        symbol->creation_ordinal = node->high_ordinal;
                    }
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

        else if (node->node_type == NODE_REGISTER) {
            node->scope = context->current_scope;
            n = node->child;
            while (n) {
                n->scope = context->current_scope;
                n = n->sibling;
            }
            return request_skip;
        }

        else if (node->node_type == DO || node->node_type == BLOCK_EXPR) {
            /* Create/Navigate to scope - handled in the navigation block above */
        }

        else if (node->node_type == INSTRUCTIONS) {
            /* Create/Navigate to scope - handled in the navigation block above */
        }

        else {
            node->scope = context->current_scope;
        }
    }

    else {
        /* OUT - BOTTOM UP */

        if (node->node_type == CLASS_DEF) {
            /* Automatic Register Allocation for Attributes */
            if (node->scope) {
                Symbol **symbols = scp_syms(node->scope);
                if (symbols) {
                    char used_regs[1024]; /* Support up to 1024 registers for now */
                    int i, j, next_free;
                    memset(used_regs, 0, sizeof(used_regs));

                    /* Pass 1: Find explicitly used registers */
                    for (i = 0; symbols[i]; i++) {
                        Symbol *s = symbols[i];
                        if (s->symbol_type == VARIABLE_SYMBOL) {
                            for (j = 0; j < (int)sym_nond(s); j++) {
                                ASTNode *sn = sym_trnd(s, j)->node;
                                if (sn->parent && sn->parent->node_type == DEFINE) {
                                    ASTNode *nr = ast_chld(sn->parent, NODE_REGISTER, 0);
                                    if (nr) {
                                        ASTNode *idx = ast_chld(nr, INTEGER, 0);
                                        int reg_idx = -1;
                                        if (idx) reg_idx = node_to_integer(idx);
                                        else if (nr->int_value) reg_idx = (int)nr->int_value;

                                        if (reg_idx >= 0 && reg_idx < 1024) {
                                            used_regs[reg_idx] = 1;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    /* Pass 2: Assign automatic registers to those that don't have one */
                    next_free = 1;
                    for (i = 0; symbols[i]; i++) {
                        Symbol *s = symbols[i];
                        if (s->symbol_type == VARIABLE_SYMBOL) {
                            char has_reg = 0;
                            ASTNode *def_node = NULL;
                            for (j = 0; j < (int)sym_nond(s); j++) {
                                ASTNode *sn = sym_trnd(s, j)->node;
                                if (sn->parent && sn->parent->node_type == DEFINE) {
                                    def_node = sn->parent;
                                    if (ast_chld(def_node, NODE_REGISTER, 0)) {
                                        has_reg = 1;
                                        break;
                                    }
                                }
                            }

                            if (!has_reg && def_node) {
                                while (next_free < 1024 && used_regs[next_free]) next_free++;
                                if (next_free < 1024) {
                                    /* Create synthesized NODE_REGISTER node */
                                    ASTNode *nr = ast_ft(context, NODE_REGISTER);
                                    char reg_str[16];
                                    ASTNode *idx = ast_ft(context, INTEGER);
                                    sprintf(reg_str, "%d", next_free);
                                    ast_copy_str(idx, reg_str);
                                    idx->int_value = next_free;
                                    nr->int_value = next_free; /* Set on both for robustness */
                                    add_ast(nr, idx);
                                    add_ast(def_node, nr);
                                    used_regs[next_free] = 1;
                                }
                            }
                        }
                    }
                    free(symbols);
                }
            }
        }

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
            int is_literal_call = 0;
            if (node->node_string && node->node_string_length >= 2 && (node->node_string[0] == '\'' || node->node_string[0] == '\"')) {
                is_literal_call = 1;
            }

            /* Find the symbol */
            Symbol *local_symbol = 0;
            if (!is_literal_call) {
                local_symbol = sym_rslv_tiered(node->scope ? node->scope : context->current_scope, node);
            }

            if (local_symbol && local_symbol->status == SYM_STATUS_LOCAL_DEF && local_symbol->symbol_type == FUNCTION_SYMBOL ) {
                if (!node->symbolNode) {
                    sym_adnd(local_symbol, node, 1, 0);
                    context->changed_flags |= FLAG_VAL_SYM;
                }
            } else {
                /* Try global search */
                symbol = sym_rvfc(context->ast, node);

                if (is_literal_call && symbol && symbol->symbol_type == FUNCTION_SYMBOL) {
                    /* Literal calls bypass unexposed local functions. 
                     * If sym_rvfc found a local unexposed function, we force a search for the external one. */
                    if (!symbol->exposed) {
                        symbol = sym_imfn(context, node);
                    }

                    if (symbol && symbol->exposed) {
                        /* Emit the specific shadowing reversal warning only once */
                        if (!node->symbolNode) {
                            char *fqn = sym_frnm(symbol);
                            char *local_name = malloc(node->node_string_length + 1);
                            size_t start = 1;
                            size_t len = node->node_string_length - 2;
                            memcpy(local_name, node->node_string + start, len);
                            local_name[len] = 0;
                            #ifdef NUTF8
                            char *c;
                            for (c = local_name; *c; ++c) *c = (char)tolower(*c);
                            #else
                            utf8lwr(local_name);
                            #endif

                            mknd_war(node, "EXTERNAL_SHADOW_BYPASS, external procedure \"%s\" shadowing local procedure \"%s\"", fqn, local_name);
                            free(fqn);
                            free(local_name);
                        }

                        /* Link to the external symbol */
                        if (!node->symbolNode) {
                            sym_adnd(symbol, node, 1, 0);
                            context->changed_flags |= FLAG_VAL_SYM;
                        }
                    }
                }

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
        if (node->scope) context->current_scope = node->scope;

        if (node->node_type == EXPOSED) {
            if (node->parent && node->parent->node_type == NAMESPACE) {
                /* We are exposing functions / variables globally to user functions */
                ASTNode* n = node->child;
                while (n) {
                    symbol = sym_rvfc(context->ast, n); /* Is this a procedure? */
                    if (!symbol) {
                         /* Is it already a global variable? */
                         symbol = sym_rslv_global(context->current_scope, n);
                    }

                    if (!symbol) {
                        /* Procedure Symbol not found so it "must" be a variable we need to expose from our procedures */
                        found = 0;
                        /* We need to loop through all the procedures in the program_file in order. */
                        if (context->ast && context->ast->child && context->ast->child->child) {
                            for (proc_node = context->ast->child->child; proc_node; proc_node = proc_node->sibling) {
                                if (proc_node->node_type != PROCEDURE) continue;

                                /* We might be exposing one of the procedure's variables */
                                symbol = sym_drsv(proc_node->scope, n); /* find it deep */
                                if (symbol && symbol->symbol_type == VARIABLE_SYMBOL && !symbol->is_arg) {
                                    /* We found a variable to expose - so expose it by moving its scope */
                                    merged_symbol = sym_hoist_to_namespace(symbol, symbol->scope ? symbol->scope->parent : 0);

                                    /* IDEMPOTENT LINKING */
                                    sym_adnd(merged_symbol, n, 1, 1);
                                    
                                    /* Link to the Procedure's INSTRUCTION node for visibility check */
                                    ASTNode *instr = ast_chld(proc_node, INSTRUCTIONS, NOP);
                                    if (instr) sym_adnd(merged_symbol, instr, 0, 0);

                                    if (merged_symbol->exposed == 0) {
                                        merged_symbol->exposed = 1;
                                        merged_symbol->is_global_var = 1;
                                        context->changed_flags |= FLAG_VAL_SYM;
                                    }
                                    found = 1;
                                }
                            }
                        }
                        
                        if (!found) {
                             /* Still not found. Seed it in the namespace! */
                             Scope *namespace_scope = context->current_scope;
                             while (namespace_scope && namespace_scope->type != SCOPE_NAMESPACE) {
                                 namespace_scope = namespace_scope->parent;
                             }
                             if (namespace_scope) {
                                 symbol = sym_f(namespace_scope, n);
                                 if (symbol && symbol->symbol_type == UNKNOWN_SYMBOL) {
                                     sym_promote_symtype(context, symbol, VARIABLE_SYMBOL);
                                     sym_promote_status(context, symbol, SYM_STATUS_LOCAL_VAR);
                                 }                                 if (symbol && symbol->symbol_type == VARIABLE_SYMBOL) {
                                     if (symbol->exposed == 0) {
                                         symbol->exposed = 1;
                                         symbol->is_global_var = 1;
                                         context->changed_flags |= FLAG_VAL_SYM;
                                     }
                                     sym_adnd(symbol, n, 1, 1);
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
                        if (temp_node) temp_node = ast_chld(temp_node, INSTRUCTIONS, NOP); /* Instructions */
                        if (temp_node && temp_node->node_type == INSTRUCTIONS) {
                            if (symbol->exposed == 0) {
                                /* Link and expose - if not already processed */
                                sym_adnd(symbol, n, 1, 1);
                                symbol->exposed = 1;
                                context->changed_flags |= FLAG_VAL_SYM;
                            }
                        }
                        else {
                            /* Add an error - if it has not already errored */
                            if (ast_chld(n, ERROR, 0) == 0)
                                mknd_err(n, "IMPORTED_FUNCTION");
                        }
                    }
                    else if (symbol->symbol_type ==  CLASS_SYMBOL) {
                        if (symbol->exposed == 0) {
                            /* Requirement: If the class is exposed, then all methods are exposed. */
                            symbol->exposed = 1;
                            sym_adnd(symbol, n, 1, 1);
                            /* Now expose all methods/factories in the class scope */
                            if (symbol->defines_scope) {
                                scp_4all(symbol->defines_scope, expose_class_symbols_worker, n);
                            }
                            context->changed_flags |= FLAG_VAL_SYM;
                        }
                    }
                    else if (symbol->symbol_type ==  VARIABLE_SYMBOL) {
                        /* Already global variable, just mark as exposed and global */
                        if (symbol->exposed == 0 || symbol->is_global_var == 0) {
                            symbol->exposed = 1;
                            symbol->is_global_var = 1;
                            context->changed_flags |= FLAG_VAL_SYM;
                        }
                        sym_adnd(symbol, n, 1, 1);
                    }
                    else {
                        /* Add an error - if it has not already errored */
                        if (ast_chld(n, ERROR, 0) == 0)
                            mknd_err(n, "INVALID_SYMBOL_TYPE");
                    }
                    n = n->sibling;
                }
            }

            else if (node->parent && node->parent->node_type == PROCEDURE) {
                /* We are exposing variables in a procedure */
                ASTNode* n = node->child;
                while (n) {
                    /* Check if it is a global symbol (already) */
                    symbol = sym_rvfc(context->ast, n); /* Is this is a procedure/function? */
                    if (!symbol) {
                         /* Is it already a global variable? */
                         symbol = sym_rslv_global(context->current_scope, n);
                    }

                    if (!symbol) {
                        /* It is not global yet, so we should be exposing one of the procedure's variables */
                        symbol = sym_drsv(node->parent->scope, n); /* find it deep */
                        if (symbol && symbol->symbol_type == VARIABLE_SYMBOL && !symbol->is_arg) {
                            if (symbol->exposed == 0) { /* Avoid double processing */
                                /* We found a variable to expose - so expose it by moving its scope */
                                merged_symbol = sym_hoist_to_namespace(symbol, symbol->scope ? symbol->scope->parent : 0);
                                /* Link to the exposed node */
                                sym_adnd(merged_symbol, n, 1, 1);
                                /* Link to the Procedure's INSTRUCTION node */
                                ASTNode *instr = ast_chld(node->parent, INSTRUCTIONS, NOP);
                                if (instr) sym_adnd(merged_symbol, instr, 0, 0);

                                if (merged_symbol->exposed == 0) {
                                    merged_symbol->exposed = 1;
                                    merged_symbol->is_global_var = 1;
                                    context->changed_flags |= FLAG_VAL_SYM;
                                }
                            }
                        }
                        else {
                            /* Not found locally. Seed it in the namespace! */
                            Scope *namespace_scope = context->current_scope;
                            while (namespace_scope && namespace_scope->type != SCOPE_NAMESPACE) {
                                namespace_scope = namespace_scope->parent;
                            }
                            if (namespace_scope) {
                                symbol = sym_f(namespace_scope, n);
                                if (symbol && symbol->symbol_type == UNKNOWN_SYMBOL) {
                                    sym_promote_symtype(context, symbol, VARIABLE_SYMBOL);
                                    sym_promote_status(context, symbol, SYM_STATUS_LOCAL_VAR);
                                }                                if (symbol && symbol->symbol_type == VARIABLE_SYMBOL) {
                                    if (symbol->exposed == 0) {
                                        symbol->exposed = 1;
                                        symbol->is_global_var = 1;
                                        context->changed_flags |= FLAG_VAL_SYM;
                                    }
                                    /* Link to the exposed node */
                                    sym_adnd(symbol, n, 1, 1);
                                    /* Link to the Procedure's INSTRUCTION node */
                                    ASTNode *instr = ast_chld(node->parent, INSTRUCTIONS, NOP);
                                    if (instr) sym_adnd(symbol, instr, 0, 0);
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
                    }

                    else if (symbol->symbol_type ==  VARIABLE_SYMBOL) {
                        /* Already a global symbol - does it exist in the procedure? */
                        Symbol *proc_symbol = node->parent ? sym_drsv(node->parent->scope, n) : 0;
                        if (proc_symbol) {
                            if (proc_symbol->is_arg) {
                                /* If it is an arg it can't b e exposed */
                                /* Add an error - if it has not already errored */
                                if (ast_chld(n, ERROR, 0) == 0)
                                    mknd_err(n, "CANNOT_EXPOSED_ARG");
                            }
                            else {
                                /* Expose it */
                                merged_symbol = sym_hoist_to_namespace(proc_symbol, proc_symbol->scope->parent);
                                /* Link to the exposed node */
                                sym_adnd(merged_symbol, n, 1, 1);
                                /* Link to the Procedure's INSTRUCTION node */
                                ASTNode *instr = ast_chld(node->parent, INSTRUCTIONS, NOP);
                                if (instr) sym_adnd(merged_symbol, instr, 0, 0);

                                if (merged_symbol->exposed == 0) {
                                    merged_symbol->exposed = 1;
                                    merged_symbol->is_global_var = 1;
                                    context->changed_flags |= FLAG_VAL_SYM;
                                }
                            }
                        }
                        else {
                            /* Either we have already processed this symbol (duplicate) or it is not used in the proc at all */
                            ASTNode *instr = ast_chld(node->parent, INSTRUCTIONS, NOP);
                            if (instr && symislnk(instr, symbol)) {
                                /* It's linked to the procedure's instructions - therefore a duplicate */
                                /* Add a warning - if it has not already errored/warned */
                                if (!context->after_rewrite && ast_chld(n, ERROR, WARNING) == 0)
                                    mknd_war(n, "DUPLICATE_SYMBOL");
                            }
                            else {
                                /* Not yet linked to this procedure's instructions - so link it now */
                                sym_adnd(symbol, n, 1, 1);
                                if (instr) sym_adnd(symbol, instr, 0, 0);
                                if (symbol->exposed == 0 || symbol->is_global_var == 0) {
                                    symbol->exposed = 1;
                                    symbol->is_global_var = 1;
                                    context->changed_flags |= FLAG_VAL_SYM;
                                }
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
    } else {
        /* OUT - BOTTOM UP */
        if (node->parent) context->current_scope = node->parent->scope;
        else context->current_scope = 0;
    }

    return result_normal;
}

struct val_sym_payload {
    Context *context;
    Scope *scope;
};

/* Step 3 - Validate Symbols
   This is called for every symbol */
static void validate_symbol_in_scope(Symbol *symbol, void *payload) {
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
                Symbol *shadowed = 0;

                /* Check if it shadows a variable in a parent local scope within the same procedure */
                if (symbol->scope && symbol->scope->parent) {
                    /* Tiered search but STOP at Procedure level */
                    Scope *search_scope = symbol->scope->parent;
                    while (search_scope) {
                        shadowed = sym_lrsv(search_scope, rep_node);
                        if (shadowed && (shadowed->symbol_type == VARIABLE_SYMBOL || shadowed->symbol_type == CONSTANT_SYMBOL)) {
                            /* TEMPORAL CHECK: Only shadow if the shadowed symbol was defined EARLIER */
                            if (shadowed->creation_ordinal != -1 && symbol->creation_ordinal != -1 &&
                                shadowed->creation_ordinal < symbol->creation_ordinal) {
                                shadows_var = 1;
                                symbol->shadowed_symbol = shadowed;
                                break;
                            }
                        }
                        if (search_scope->type == SCOPE_PROCEDURE || search_scope->type == SCOPE_CLASS) break;
                        search_scope = search_scope->parent;
                    }
                }

                if (!shadows_var) {
                    /* Check if it shadows a global/namespace variable */
                    Symbol *glob_sym = sym_rslv_global(symbol->scope, rep_node);
                    if (glob_sym && glob_sym != symbol && (glob_sym->symbol_type == VARIABLE_SYMBOL || glob_sym->symbol_type == CONSTANT_SYMBOL)) {
                        /* Globals/Namespace variables are effectively defined at the start of the program (ordinal 0 or -1) */
                        if (glob_sym->creation_ordinal <= 0 || (symbol->creation_ordinal != -1 && glob_sym->creation_ordinal < symbol->creation_ordinal)) {
                            shadows_var = 1;
                            symbol->shadowed_symbol = glob_sym;
                        }
                    }
                }
                if (shadows_var) {
                    /* Suppress shadowing warning for symbols in signature-only procedures (declarations) */
                    int suppress = 0;
                    if (symbol->scope && symbol->scope->type == SCOPE_PROCEDURE && symbol->scope->defining_node) {
                        if (ast_chld(symbol->scope->defining_node, INSTRUCTIONS, NOP)->node_type == NOP) {
                            suppress = 1;
                        }
                    }
                    if (!suppress) symbol->is_shadowing = 1;
                }
            } else if (symbol->status == SYM_STATUS_LOCAL_DEF) {
                if (symbol->symbol_type == FUNCTION_SYMBOL) {
                    if (sym_is_imfn(context, rep_node)) {
                        symbol->is_shadowing = 1;
                        symbol->shadowed_symbol = sym_imfn(context, rep_node);
                    }
                } else if (symbol->symbol_type == CLASS_SYMBOL) {
                    if (sym_is_imcls(context, rep_node)) {
                        symbol->is_shadowing = 1;
                        symbol->shadowed_symbol = sym_imcls(context, rep_node);
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
            sym_promote_status(context, symbol, SYM_STATUS_LOCAL_VAR);
        }
    }

    /* Process special symbols */
    if (symbol->is_rc) {
        symbol->type = TP_INTEGER;
        sym_promote_symtype(context, symbol, VARIABLE_SYMBOL);
        symbol->value_dims = 0;
        return;
    }

    if (symbol->is_factory && scope->defining_node && scope->defining_node->node_type == FACTORY) {
        symbol->type = TP_OBJECT;
        sym_promote_symtype(context, symbol, VARIABLE_SYMBOL);
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
        sym_promote_symtype(context, symbol, VARIABLE_SYMBOL);
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

            p = ast_proc(defining_node_link->node);
            if (p == proc && symbol->type != TP_UNKNOWN) continue; /* We have already looked at this proc and have a type */
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
                    node_to_dims(context, defining_node_link->node, &(symbol->value_dims),
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
                node_to_dims(context, defining_node_link->node, &(symbol->value_dims),
                             &(symbol->dim_base), &(symbol->dim_elements));

            ast_svtp(defining_node_link->node, symbol);
            ast_svtn(defining_node_link->node->parent, defining_node_link->node);
            
        }

        else if (symbol->symbol_type != NAMESPACE_SYMBOL && symbol->symbol_type != FUNCTION_SYMBOL && symbol->symbol_type != CLASS_SYMBOL) {
            /* Used without definition/declaration - Taken Constant */
            /* TODO - for Level A/C/D we will need flow analysis to determine taken constant status */
            sym_promote_type(context, symbol, TP_STRING, 0, 0, 0, 0);
            sym_promote_symtype(context, symbol, CONSTANT_SYMBOL);
            if (symbol->status == SYM_STATUS_UNRESOLVED) sym_promote_status(context, symbol, SYM_STATUS_LOCAL_VAR);
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

/* Hoist a variable definition to a specific scope level.
 * levels: -1 = Procedure/Method level, 1 = Parent level (up 1), 0 = Current level
 * Returns 1 if hoisted/already exists, 0 on failure.
 */
/* Helper to scan AST backwards for prior definition of a variable */
static int is_var_defined_before(ASTNode* start_node, const char* var_name) {
    ASTNode *current = start_node;
    while (current) {
        ASTNode *p = current->parent;
        if (!p) break;
        ASTNode *n = p->child;
        while (n && n != current) {
            if (n->node_type == DEFINE || n->node_type == ASSIGN) {
                ASTNode *target = ast_chdn(n, 0);
                if (target && (target->node_type == VAR_TARGET || target->node_type == VAR_SYMBOL)) {
                    const char *t_name = target->node_string;
                    size_t t_len = target->node_string_length;
                    if (!t_name && target->token) {
                        t_name = target->token->token_string;
                        t_len = target->token->length;
                    }
                    if (t_name && t_len == strlen(var_name) && strncasecmp(t_name, var_name, t_len) == 0) {
                        return 1;
                    }
                }
            }
            n = n->sibling;
        }
        current = p;
    }
    return 0;
}

/* Hoist a variable definition to a specific scope level.
 * levels: -1 = Procedure/Method level, 0 = Current level (inserted just before current_node), 1 = Parent level
 * Returns 1 if hoisted/already exists, 0 on failure.
 */
int ast_hoist_var(Context* ctx, ASTNode* current_node, const char* var_name, int levels) {
    ASTNode *target_scope_node = current_node;

    if (levels == -1) {
        target_scope_node = ast_proc(current_node);
        if (!target_scope_node) target_scope_node = ast_ns(current_node); /* Fallback to namespace if not in proc */
    } else {
        int climbs = levels;
        target_scope_node = target_scope_node->parent; /* start from parent */
        while (climbs > 0 && target_scope_node) {
            if (target_scope_node->node_type == DO ||
                target_scope_node->node_type == BLOCK_EXPR ||
                (target_scope_node->node_type == INSTRUCTIONS && target_scope_node->force_local_scope) ||
                target_scope_node->node_type == PROCEDURE ||
                target_scope_node->node_type == METHOD ||
                target_scope_node->node_type == NAMESPACE ||
                target_scope_node->node_type == PROGRAM_FILE) {
                climbs--;
                if (climbs == 0) break;
            }
            target_scope_node = target_scope_node->parent;
        }
    }

    if (!target_scope_node) return 0;

    /* Check if already defined anywhere before this node */
    int found = is_var_defined_before(current_node, var_name);

    /* Search for INSTRUCTIONS child for fallback block prepending */
    ASTNode *instructions = ast_chld(target_scope_node, INSTRUCTIONS, NOP);
    if (!instructions) instructions = target_scope_node;

    if (!found) {
        ASTNode *injected_node = NULL;

        /* Create DEFINE node: var_name = .unknown */
        ASTNode *def_node = ast_ft(ctx, DEFINE);
        ASTNode *var_node = ast_ftt(ctx, VAR_TARGET, strdup(var_name));
        var_node->free_node_string = 1;
        ASTNode *type_node = ast_ft(ctx, CLASS);
        type_node->node_string = strdup(".unknown");
        type_node->node_string_length = strlen(".unknown");
        type_node->free_node_string = 1;

        add_ast(def_node, var_node);
        add_ast(def_node, type_node);

        /* Match location to current_node */
        def_node->line = current_node->line;
        def_node->column = current_node->column;
        def_node->source_start = current_node->source_start;
        def_node->source_end = current_node->source_end;
        var_node->line = current_node->line;
        var_node->column = current_node->column;
        var_node->source_start = current_node->source_start;
        var_node->source_end = current_node->source_end;

        injected_node = def_node;

        if (levels == 0 && current_node->parent) {
            /* Insert just before current_node */
            ASTNode *p = current_node->parent;
            if (p->child == current_node) {
                injected_node->sibling = current_node;
                p->child = injected_node;
            } else {
                ASTNode *sib = p->child;
                while (sib && sib->sibling != current_node) sib = sib->sibling;
                if (sib) {
                    injected_node->sibling = current_node;
                    sib->sibling = injected_node;
                } else {
                    /* Fallback */
                    injected_node->sibling = instructions->child;
                    instructions->child = injected_node;
                }
            }
            injected_node->parent = p;
        } else {
            /* Prepend to top of target block */
            injected_node->sibling = instructions->child;
            instructions->child = injected_node;
            injected_node->parent = instructions;
        }

        ctx->changed_flags |= FLAG_VAL_TRANS;
    }

    return 1;
}
