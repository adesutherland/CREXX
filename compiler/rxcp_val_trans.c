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
 * Validation Pass: Transformation/Lowering
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "rxcp_val.h"
#include "rxcp_exit.h"

/*
 * Converts EXIT Instruction to _exit System Function
 */
walker_result rewrite_exit_walker(walker_direction direction,
                                            ASTNode* node, __attribute__((unused)) void *payload) {

    Context *context = (Context*)payload;

    if (direction == out) {
        /* Bottom Up */
        switch (node->node_type) {

            case EXIT: {
                /* Rewrite to call of _exit */
                ASTRewriteTemplate *call_tmpl = ast_rw_new(CALL, NULL);
                ASTRewriteTemplate *func_tmpl = ast_rw_new(FUNCTION, "_exit");

                /* Move Param(s) */
                ASTNode *args_node = node->child;
                while (args_node) {
                    ASTNode *next = args_node->sibling;
                    /* Disconnect child */
                    if (args_node == node->child) node->child = next;
                    args_node->sibling = NULL;
                    args_node->parent = NULL;

                    ast_rw_add(func_tmpl, ast_rw_reuse(args_node));
                    args_node = next;
                }

                ast_rw_add(call_tmpl, func_tmpl);
                ast_execute_rewrite(context, node, call_tmpl);
                break;
            }

            default: ;
        }
    }

    return result_normal;
}

/*
 * Adds rxsysb if needed
 */
walker_result add_rxsysb_walker(walker_direction direction,
                                       ASTNode* node, __attribute__((unused)) void *payload) {

    Context *context = (Context*)payload;

    ASTNode* _rxsysb_import_node;
    ASTNode* _rxsysb_node;

    if (direction == out) {
        /* Bottom Up */
        if (node->node_type == PROGRAM_FILE) {
            if (context->need_rxsysb && !context->has_rxsysb) {
                /* we need to import _rxsysb */
                _rxsysb_import_node = ast_ft(context, IMPORT);
                /* Insert at the beginning of PROGRAM_FILE's children */
                _rxsysb_import_node->sibling = node->child;
                node->child = _rxsysb_import_node;
                _rxsysb_import_node->parent = node;

                _rxsysb_node = ast_ft(context, LITERAL);
                ast_str(_rxsysb_node, "_rxsysb");
                add_ast(_rxsysb_import_node,_rxsysb_node);
                context->has_rxsysb = 1;
            }
        }

        else if (node->node_type == IMPORT) {
            /* Have we imported _rxsysb already */
            if (node->child && is_node_string(node->child, "_rxsysb")) context->has_rxsysb = 1;
        }

        else if (node->node_type == NAMESPACE) {
            /* Have we imported _rxsysb already */
            if (node->child && is_node_string(node->child, "_rxsysb")) context->has_rxsysb = 1;
        }
    }

    return result_normal;
}

/*
 * Sees if rxsysb is needed
 */
static void rxcp_warn_implicit_address(ASTNode *node);
static ASTNode *rxcp_first_implicit_cmd_leaf(ASTNode *node);
static const char *rxcp_get_node_text(ASTNode *node, size_t *len);
static const char *rxcp_disabled_certified_exit_keyword(ASTNode *node);

walker_result needs_rxsysb_walker(walker_direction direction,
                                       ASTNode* node, __attribute__((unused)) void *payload) {

    Context *context = (Context*)payload;

    if (direction == out) {
        /* Bottom Up */
        if (node->node_type == IMPLICIT_CMD) {
            const char *certified_keyword;

            certified_keyword = NULL;
            if (context->disable_exits) {
                certified_keyword = rxcp_disabled_certified_exit_keyword(node);
            }
            if (certified_keyword) {
                mknd_err_unique(node, "CERTIFIED_EXIT_DISABLED, \"%s\"", certified_keyword);
                return result_normal;
            }

            context->need_rxsysb = 1;
            rxcp_warn_implicit_address(node);
            ast_hoist_var(context, node, "rc", -1);
        }
        else if (node->node_type == EXIT_EXTENDED) {
            const char *certified_keyword;

            certified_keyword = NULL;
            if (context->disable_exits && node->token) {
                certified_keyword = rxcp_match_certified_exit_primary(node->token->token_string, node->token->length);
            }
            if (certified_keyword) {
                mknd_err_unique(node, "CERTIFIED_EXIT_DISABLED, \"%s\"", certified_keyword);
                return result_normal;
            }

            context->need_rxsysb = 1;
            if (node->token) {
                unsigned int flags;

                flags = rxcp_get_exit_flags(context, node->token->token_string, node->token->length);
                if (flags & RXCP_EXIT_FLAG_IMPLICIT_COMMAND) {
                    ast_hoist_var(context, node, "rc", -1);
                }
            }
        }
        else if (node->node_type == EXIT) {
            context->need_rxsysb = 1;
        }
    }

    return result_normal;
}

static void rxcp_warn_implicit_address(ASTNode *node) {
    int starts_with_quote;

    if (!node || ast_hase(node)) return;

    starts_with_quote = 0;
    if (node->source_start && (node->source_start[0] == '\'' || node->source_start[0] == '"')) {
        starts_with_quote = 1;
    }

    if (!starts_with_quote) {
        mknd_war(node, "IMPLICIT_ADDRESS");
    }
}

static ASTNode *rxcp_first_implicit_cmd_leaf(ASTNode *node) {
    ASTNode *child;

    if (!node) return NULL;
    if (node->node_type == ERROR || node->node_type == WARNING) return NULL;

    if (node->node_type == OP_CONCAT || node->node_type == OP_SCONCAT || node->node_type == IMPLICIT_CMD) {
        child = node->child;
        while (child) {
            ASTNode *leaf = rxcp_first_implicit_cmd_leaf(child);
            if (leaf) return leaf;
            child = child->sibling;
        }
        return NULL;
    }

    return node;
}

static const char *rxcp_get_node_text(ASTNode *node, size_t *len) {
    if (len) *len = 0;
    if (!node) return NULL;

    if (node->token && node->token->token_string && node->token->length > 0) {
        if (len) *len = node->token->length;
        return node->token->token_string;
    }

    if (node->node_string) {
        size_t text_len = node->node_string_length ? node->node_string_length : strlen(node->node_string);
        if (text_len > 0) {
            if (len) *len = text_len;
            return node->node_string;
        }
    }

    return NULL;
}

static const char *rxcp_disabled_certified_exit_keyword(ASTNode *node) {
    ASTNode *leaf;
    const char *text;
    size_t len;

    leaf = rxcp_first_implicit_cmd_leaf(node);
    if (!leaf) return NULL;

    if (leaf->node_type != VAR_SYMBOL && leaf->node_type != CONST_SYMBOL) {
        return NULL;
    }

    text = rxcp_get_node_text(leaf, &len);
    if (!text || len == 0) return NULL;

    return rxcp_match_certified_exit_primary(text, len);
}

/*
 * Warns on implicit command usage while preserving IMPLICIT_CMD fallback for
 * non-certified commands. Certified primaries are rejected earlier when exits
 * are disabled.
 */
walker_result rewrite_implicit_cmd_walker(walker_direction direction,
                                          ASTNode* node, void *payload) {
    Context *context = (Context *) payload;
    (void)context;

    if (direction == out && node->node_type == IMPLICIT_CMD) {
        if (ast_hase(node)) return result_normal;

        /* Fix up location if missing - typically copied from the expression child */
        if (node->line == -1 && node->child) {
            node->line = node->child->line;
            node->column = node->child->column;
            node->file_name = node->child->file_name;
            node->source_start = node->child->source_start;
            node->source_end = node->child->source_end;
            node->token_start = node->child->token_start;
            node->token_end = node->child->token_end;
        }

        rxcp_warn_implicit_address(node);
    }
    return result_normal;
}

/*
 * Lowers constructor-style declarations: x = .int(1)
 */
walker_result rewrite_constructor_walker(walker_direction direction,
                                         ASTNode* node, void *payload) {
    Context *context = (Context *) payload;

    if (direction == out) {
        if (node->node_type == ASSIGN) {
            ASTNode *target = ast_chdn(node, 0);
            ASTNode *rhs = ast_chdn(node, 1);

            if (rhs && rhs->node_type == FACTORY_CALL) {
                int fundamental = 0;
                char* type_name = NULL;

                if (is_node_string(rhs, "int")) { fundamental = 1; type_name = "int"; }
                else if (is_node_string(rhs, "string")) { fundamental = 1; type_name = "string"; }
                else if (is_node_string(rhs, "float")) { fundamental = 1; type_name = "float"; }
                else if (is_node_string(rhs, "boolean")) { fundamental = 1; type_name = "boolean"; }
                else if (is_node_string(rhs, "decimal")) { fundamental = 1; type_name = "decimal"; }

                int in_do_loop = 0;
                ASTNode *do_node = NULL;
                if (node->parent && node->parent->node_type == REPEAT) {
                    if (node->parent->parent && node->parent->parent->node_type == DO) {
                        in_do_loop = 1;
                        do_node = node->parent->parent;
                    }
                }

                if (!fundamental && in_do_loop) {
                    if (rhs->child) {
                        ast_del(rhs->child);
                        rhs->child = NULL;
                    }
                    mknd_err_unique(rhs, "LOOP_CLASSES_NOT_SUPPORTED");
                    return result_normal;
                }

                if (fundamental) {
                    ASTNode *params = rhs->child;
                    ASTNode *val = NULL;
                    int has_error = 0;

                    if (params) {
                        if (params->node_type != NOVAL) {
                            val = params;
                            ASTNode *extra = val->sibling;
                            while (extra) {
                                mknd_err(extra, "UNEXPECTED_ARGUMENT");
                                extra = extra->sibling;
                                has_error = 1;
                            }
                            ast_del(val);
                            val->sibling = NULL;
                        }
                    }

                    /* Using AST Rewrite Utility */
                    ASTRewriteTemplate *class_node_tmpl = ast_rw_new(CLASS, mprintf(".%s", type_name));

                    if (in_do_loop) {
                        char *target_str = rx_strndup(target->node_string, target->node_string_length);
                        char *do_str = rx_strndup(do_node->node_string, do_node->node_string_length);
                        /* do i = .int(1) to 3 -> do; i = .int; do i = 1 to 3; ... ; end; end */
                        ASTRewriteTemplate *define_tmpl = ast_rw_add(ast_rw_add(
                            ast_rw_loc(ast_rw_new(DEFINE, "="), target),
                            ast_rw_loc(ast_rw_new(VAR_TARGET, target_str), target)),
                            class_node_tmpl
                        );

                        ASTRewriteTemplate *assign_tmpl = NULL;
                        if (val) {
                            assign_tmpl = ast_rw_add(ast_rw_add(
                                ast_rw_loc(ast_rw_new(ASSIGN, "="), target),
                                ast_rw_loc(ast_rw_new(VAR_TARGET, target_str), target)),
                                ast_rw_reuse(val)
                            );
                        } else {
                            /* No initial value, remove ASSIGN completely or assign null? 
                               Actually loop needs an initializer, but let's just make it assign NOVAL to keep REPEAT child valid?
                               Let's assign NOVAL to keep AST structure.
                             */
                            assign_tmpl = ast_rw_add(ast_rw_add(
                                ast_rw_loc(ast_rw_new(ASSIGN, "="), target),
                                ast_rw_loc(ast_rw_new(VAR_TARGET, target_str), target)),
                                ast_rw_new(NOVAL, NULL)
                            );
                        }
                        free(target_str);

                        /* Rebuild DO node */
                        ASTRewriteTemplate *new_do_tmpl = ast_rw_new(DO, do_node->node_string);
                        
                        ASTRewriteTemplate *instr_tmpl = ast_rw_add(ast_rw_add(
                            ast_rw_new(INSTRUCTIONS, ""),
                            define_tmpl),
                            new_do_tmpl
                        );
                        
                        /* Move all children of DO to new inner DO, except REPEAT -> ASSIGN which gets replaced */
                        ASTNode *c = do_node->child;
                        while (c) {
                            ASTNode *next = c->sibling;
                            if (c->node_type == REPEAT) {
                                ASTRewriteTemplate *new_repeat = ast_rw_new(REPEAT, c->node_string);
                                ast_rw_move_children_replace(new_repeat, c, node, assign_tmpl);
                                ast_rw_add(new_do_tmpl, new_repeat);
                            } else {
                                c->sibling = NULL;
                                c->parent = NULL;
                                ast_rw_add(new_do_tmpl, ast_rw_reuse(c));
                            }
                            c = next;
                        }

                        do_node->child = NULL; /* prevent double free/reuse issues */
                        ast_execute_rewrite(context, do_node, instr_tmpl);
                        context->changed_flags |= FLAG_VAL_TRANS;
                        return result_normal;
                    }

                    if (val) {
                        /* x = .int(10) -> DEFINE x = .int ; ASSIGN x = 10 */
                        ASTRewriteTemplate *define_tmpl = ast_rw_add(ast_rw_add(
                            ast_rw_new(DEFINE, "="),
                            ast_rw_reuse(target)),
                            class_node_tmpl
                        );

                        ASTRewriteTemplate *assign_tmpl = ast_rw_add(ast_rw_add(
                            ast_rw_new(ASSIGN, "="),
                            ast_rw_new(VAR_TARGET, target->node_string)), /* Duplicate target */
                            ast_rw_reuse(val)
                        );

                        ASTRewriteTemplate *wrapper = ast_rw_add(ast_rw_add(
                            ast_rw_children(),
                            define_tmpl),
                            assign_tmpl
                        );

                        ast_execute_rewrite(context, node, wrapper);
                        context->changed_flags |= FLAG_VAL_TRANS;

                        /* If we had errors, return normal to avoid loop, but let the error be reported */
                        if (has_error) return result_normal;

                        return result_normal;
                    }
                    else {
                        /* x = .int() -> DEFINE x = .int */
                        ASTRewriteTemplate *define_tmpl = ast_rw_add(ast_rw_add(
                            ast_rw_new(DEFINE, "="),
                            ast_rw_reuse(target)),
                            class_node_tmpl
                        );
                        ast_execute_rewrite(context, node, define_tmpl);
                        context->changed_flags |= FLAG_VAL_TRANS;

                        /* If there was an error in args, just return normal to avoid loop, but let the error be reported */
                        if (has_error) return result_normal;

                        return result_normal;
                    }
                }
            }
        }
    }
    return result_normal;
}

/*
 * Syntactic Sugar: obj.member / obj[expr] -> obj.get(expr)
 *                  obj.member = val / obj[expr] = val -> obj.set(expr, val)
 */
walker_result syntax_sugar_walker(walker_direction direction,
                                  ASTNode* node, void *payload) {
    Context *context = (Context *) payload;

    if (direction == out) {
        if (node->node_type == ASSIGN) {
            ASTNode *target = node->child;
            if (target && target->node_type == VAR_TARGET) {
                ASTNode *index = target->child;
                if (index) {
                    /* printf("DEBUG: syntax_sugar_walker checking VAR_TARGET '%s', value_type=%d, is_native_array=%d\n", target->node_string, target->value_type, target->value_dims); */
                }
            }
        }
        if (node->node_type == INSTRUCTIONS) {
            /* Prove the approach with a new transformation use case: NOP removal */
            ASTNode *nop_node = ast_chld(node, NOP, 0);
            if (nop_node) {
                ASTRewriteTemplate *new_inst = ast_rw_new(INSTRUCTIONS, node->node_string);
                /* Using ast_rw_move_children_replace to move all children but drop the NOP node */
                ast_rw_move_children_replace(new_inst, node, nop_node, NULL);
                ast_execute_rewrite(context, node, new_inst);
                context->changed_flags |= FLAG_VAL_TRANS;
                return result_normal;
            }
        }
        else if (node->node_type == ASSIGN) {
            ASTNode *target = node->child;
            if (target && target->node_type == VAR_TARGET) {
                ASTNode *index = target->child;

                /* Rewrite if node is an Object and has an index (array/property access) */
                int is_native_array = 0;
                if (target->symbolNode && target->symbolNode->symbol) {
                    if (target->symbolNode->symbol->value_dims > 0) is_native_array = 1;
                } else if (target->value_dims > 0) {
                    is_native_array = 1;
                }

                int is_obj = 0;
                char *vclass = target->value_class;
                if (target->value_type == TP_OBJECT) is_obj = 1;
                else if (target->symbolNode && target->symbolNode->symbol && target->symbolNode->symbol->type == TP_OBJECT) {
                    is_obj = 1;
                    if (!vclass) vclass = target->symbolNode->symbol->value_class;
                }

                if (index && is_obj && !is_native_array) {
                    int method_exists = 0;
                    if (vclass) {
                        ASTNode dummy = {0};
                        dummy.node_string = (char*)vclass;
                        if (dummy.node_string && dummy.node_string[0] == '.') dummy.node_string++;
                        dummy.node_string_length = dummy.node_string ? strlen(dummy.node_string) : 0;
                        Symbol *class_sym = sym_rvfc(context->ast, &dummy);
                        if (class_sym && class_sym->symbol_type == CLASS_SYMBOL && class_sym->defines_scope) {
                            ASTNode dummy_node = {0};
                            dummy_node.node_string = (char*)"set";
                            dummy_node.node_string_length = 3;
                            Symbol *method_sym = sym_lrsv(class_sym->defines_scope, &dummy_node);
                            if (method_sym && method_sym->symbol_type == FUNCTION_SYMBOL) {
                                method_exists = 1;
                            }
                        }
                    }

                    if (method_exists) {
                        ASTNode *val = target->sibling;

                        /* Template: CALL -> MEMBER_CALL("set") -> [obj_stem, index, val] */
                        ASTRewriteTemplate *call_tmpl = ast_rw_new(CALL, NULL);
                        ASTRewriteTemplate *member_call_tmpl = ast_rw_new(MEMBER_CALL, "set");

                        /* 1. Stem (Target instance) */
                        ASTNode *stem_copy = ast_fstk(context, target);
                        stem_copy->node_type = VAR_SYMBOL;
                        ast_rw_add(member_call_tmpl, ast_rw_reuse(stem_copy));

                        /* 2. Arguments */
                        ast_rw_add(member_call_tmpl, ast_rw_reuse(index));
                        if (val) ast_rw_add(member_call_tmpl, ast_rw_reuse(val));

                        ast_rw_add(call_tmpl, member_call_tmpl);

                        ast_execute_rewrite(context, node, call_tmpl);
                        context->changed_flags |= FLAG_VAL_TRANS;
                        return result_normal;
                    }
                }
            }
        }
        else if (node->node_type == VAR_SYMBOL) {
            ASTNode *index = node->child;

            /* Rewrite if node is an Object and has an index (array/property access) */
            int is_native_array = 0;
            if (node->symbolNode && node->symbolNode->symbol) {
                if (node->symbolNode->symbol->value_dims > 0) is_native_array = 1;
            } else if (node->value_dims > 0) {
                is_native_array = 1;
            }

            int is_obj = 0;
            char *vclass = node->value_class;
            if (node->value_type == TP_OBJECT) is_obj = 1;
            else if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->type == TP_OBJECT) {
                is_obj = 1;
                if (!vclass) vclass = node->symbolNode->symbol->value_class;
            }

            if (index && is_obj && !is_native_array) {
                int method_exists = 0;
                if (vclass) {
                    ASTNode dummy = {0};
                    dummy.node_string = (char*)vclass;
                    if (dummy.node_string && dummy.node_string[0] == '.') dummy.node_string++;
                    dummy.node_string_length = dummy.node_string ? strlen(dummy.node_string) : 0;
                    Symbol *class_sym = sym_rvfc(context->ast, &dummy);
                    if (class_sym && class_sym->symbol_type == CLASS_SYMBOL && class_sym->defines_scope) {
                        ASTNode dummy_node = {0};
                        dummy_node.node_string = (char*)"get";
                        dummy_node.node_string_length = 3;
                        Symbol *method_sym = sym_lrsv(class_sym->defines_scope, &dummy_node);
                        if (method_sym && method_sym->symbol_type == FUNCTION_SYMBOL) {
                            method_exists = 1;
                        }
                    }
                }

                if (method_exists) {
                    /* Template: MEMBER_CALL("get") -> [obj_stem, index] */
                    ASTRewriteTemplate *member_call_tmpl = ast_rw_new(MEMBER_CALL, "get");

                    /* 1. Stem (Target instance) */
                    ASTNode *stem_copy = ast_fstk(context, node);
                    stem_copy->node_type = VAR_SYMBOL;
                    ast_rw_add(member_call_tmpl, ast_rw_reuse(stem_copy));

                    /* 2. Argument */
                    ast_rw_add(member_call_tmpl, ast_rw_reuse(index));

                    ast_execute_rewrite(context, node, member_call_tmpl);
                    context->changed_flags |= FLAG_VAL_TRANS;
                    return result_normal;
                }
            }
        }
    }
    return result_normal;
}

/*
 * Rewrites control flow statements, e.g. Classic SELECT -> IF / THEN / ELSE
 */
walker_result control_flow_rewrite_walker(walker_direction direction,
                                          ASTNode* node, void *payload) {
    Context *context = (Context *) payload;

    if (direction == out) {
        if (node->node_type == SELECT) {
            ASTNode *when_list = node->child;
            ASTNode *otherwise = when_list ? when_list->sibling : NULL;

            if (when_list && when_list->node_type == INSTRUCTIONS) {
                ASTNode *when_node = when_list->child;
                ASTRewriteTemplate *root_if_tmpl = NULL;
                ASTRewriteTemplate *current_if_tmpl = NULL;

                while (when_node) {
                    if (when_node->node_type == WHEN) {
                        ASTNode *expr = when_node->child;
                        ASTNode *inst = expr ? expr->sibling : NULL;

                        ASTRewriteTemplate *new_if = ast_rw_new(IF, "if");
                        if (expr) ast_rw_add(new_if, ast_rw_reuse(expr));
                        if (inst) ast_rw_add(new_if, ast_rw_reuse(inst));

                        if (!root_if_tmpl) {
                            root_if_tmpl = new_if;
                        } else {
                            ast_rw_add(current_if_tmpl, new_if); /* Add as ELSE block of the current IF */
                        }
                        current_if_tmpl = new_if;
                    }
                    when_node = when_node->sibling;
                }

                if (root_if_tmpl) {
                    if (otherwise) {
                        /* The otherwise child is the instruction(s) to execute */
                        if (otherwise->child) {
                            ast_rw_add(current_if_tmpl, ast_rw_reuse(otherwise->child));
                        }
                    } else {
                        /* Add a runtime error block for no otherwise if needed, but for now we omit ELSE */
                        /* Actually, Rexx standard requires an error if no WHEN matches and no OTHERWISE */
                        /* Let's generate a NOP or leave it empty for Level B */
                    }

                    ast_execute_rewrite(context, node, root_if_tmpl);
                    context->changed_flags |= FLAG_VAL_TRANS;
                    return result_normal;
                }
            }
        } else if (node->node_type == SWITCH) {
            ASTNode *expr = node->child;
            ASTNode *when_list = expr ? expr->sibling : NULL;
            ASTNode *otherwise = when_list ? when_list->sibling : NULL;

            if (expr && when_list && when_list->node_type == INSTRUCTIONS) {
                ASTNode *when_node = when_list->child;
                ASTRewriteTemplate *root_if_tmpl = NULL;
                ASTRewriteTemplate *current_if_tmpl = NULL;

                ASTRewriteTemplate *block_tmpl = ast_rw_new(INSTRUCTIONS, "");

                ASTRewriteTemplate *decl_tmpl = ast_rw_new(DEFINE, "=");
                ast_rw_add(decl_tmpl, ast_rw_new(VAR_TARGET, "select_tmp"));
                ast_rw_add(decl_tmpl, ast_rw_new(CLASS, ".unknown"));
                ast_rw_add(block_tmpl, decl_tmpl);

                ASTRewriteTemplate *assgn_tmpl = ast_rw_new(ASSIGN, "=");
                ast_rw_add(assgn_tmpl, ast_rw_new(VAR_TARGET, "select_tmp"));
                ast_rw_add(assgn_tmpl, ast_rw_reuse(expr));
                ast_rw_add(block_tmpl, assgn_tmpl);

                while (when_node) {
                    if (when_node->node_type == WHEN) {
                        ASTNode *when_expr = when_node->child;
                        ASTNode *inst = when_expr ? when_expr->sibling : NULL;

                        ASTRewriteTemplate *new_if = ast_rw_new(IF, "if");
                        
                        ASTRewriteTemplate *eq_tmpl = ast_rw_new(OP_COMPARE_EQUAL, "=");
                        ast_rw_add(eq_tmpl, ast_rw_new(VAR_REFERENCE, "select_tmp"));
                        if (when_expr) ast_rw_add(eq_tmpl, ast_rw_reuse(when_expr));
                        
                        ast_rw_add(new_if, eq_tmpl);
                        if (inst) ast_rw_add(new_if, ast_rw_reuse(inst));

                        if (!root_if_tmpl) {
                            root_if_tmpl = new_if;
                        } else {
                            ast_rw_add(current_if_tmpl, new_if);
                        }
                        current_if_tmpl = new_if;
                    }
                    when_node = when_node->sibling;
                }

                if (root_if_tmpl) {
                    if (otherwise) {
                        if (otherwise->child) {
                            ast_rw_add(current_if_tmpl, ast_rw_reuse(otherwise->child));
                        }
                    }

                    ast_rw_add(block_tmpl, root_if_tmpl);

                    ASTNode *new_block = ast_execute_rewrite(context, node, block_tmpl);
                    ast_mark_compiler_generated_block(new_block);
                    context->changed_flags |= FLAG_VAL_TRANS;
                    return result_normal;
                }
            }
        }
    }
    return result_normal;
}

/*
 * Rewrites Object -> String conversions to use a tostring() method call.
 */
walker_result tostring_rewrite_walker(walker_direction direction,
                                      ASTNode* node, void *payload) {
    Context *context = (Context *) payload;

    if (direction == out) {
        /* Check if we have an object that needs to be a string. */
        if (node->value_type == TP_OBJECT && node->target_type == TP_STRING && node->node_type != MEMBER_CALL) {
            
            /* Verify it actually has a tostring method */
            int has_tostring = 0;
            if (node->value_class) {
                const char *cname = node->value_class;
                if (cname[0] == '.') cname++;
                ASTNode *root = node;
                while (root && root->parent) root = root->parent;
                ASTNode dummy = {0};
                dummy.node_string = (char*)cname;
                if (dummy.node_string && dummy.node_string[0] == '.') dummy.node_string++;
                dummy.node_string_length = dummy.node_string ? strlen(dummy.node_string) : 0;
                Symbol *class_sym = sym_rvfc(root, &dummy);
                if (class_sym && class_sym->symbol_type == CLASS_SYMBOL && class_sym->defines_scope) {
                    ASTNode mock_node;
                    memset(&mock_node, 0, sizeof(mock_node));
                    mock_node.node_string = "tostring";
                    mock_node.node_string_length = 8;
                    Symbol *tostring_sym = sym_lrsv(class_sym->defines_scope, &mock_node);
                    if (tostring_sym && tostring_sym->symbol_type == FUNCTION_SYMBOL) {
                        has_tostring = 1;
                    }
                }
            }
            
            if (has_tostring) {
                /* Wrap the target instance node in a MEMBER_CALL("tostring") node.
                 * We do manual tree surgery instead of ast_execute_rewrite because 
                 * the latter expects the target to be destroyed, but we need to wrap 
                 * the entire expression subtree unmodified. */
                ASTNode *member_call = ast_ft(context, MEMBER_CALL);
                ast_sstr(member_call, strdup("tostring"), 8);
                member_call->free_node_string = 1;
                ast_copy_source_anchor(member_call, node, AST_SOURCE_SYNTHETIC);

                /* Physically swap in the tree */
                ast_rpl(node, member_call);
                
                /* Now put the original expression node as a child */
                add_ast(member_call, node);

                /* Restore sibling pointer so walker can continue on direction == out */
                node->sibling = member_call->sibling;

                context->changed_flags |= FLAG_VAL_TYPE;
                return result_normal;
            }
        }
    }
    return result_normal;
}
