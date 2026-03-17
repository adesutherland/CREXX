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
 * Converts ADDRESS Instruction to _address and redirect system functions
 */
walker_result rewrite_address_walker(walker_direction direction,
                                            ASTNode* node, __attribute__((unused)) void *payload) {

    Context *context = (Context*)payload;

    ASTNode* c;
    ASTNode* next;

    if (direction == out) {
        /* Bottom Up */
        switch (node->node_type) {

            case ADDRESS:
                if (node->exit_obj_reg != -1) return result_normal;
                if (context->debug_mode >= 2) fprintf(stderr, "Lowering ADDRESS Instruction at line %d\n", node->line);
                /* Rewrite to an assignment from a function */
                context->changed_flags |= FLAG_VAL_TRANS;

                ASTNode *env = NULL;
                ASTNode *cmd = NULL;
                ASTNode *in = NULL;
                ASTNode *out = NULL;
                ASTNode *err = NULL;
                ASTNode *expose_head = NULL;
                ASTNode *expose_tail = NULL;
                ASTNode *diag_head = NULL;
                ASTNode *diag_tail = NULL;

                /* Categorize children and remove them from 'node' */
                c = node->child;
                int pos = 0;
                while (c) {
                    next = c->sibling;
                    /* Disconnect child */
                    if (c == node->child) node->child = next;
                    c->sibling = NULL;
                    c->parent = NULL;

                    if (c->node_type == ERROR || c->node_type == WARNING) {
                        if (diag_tail) diag_tail->sibling = c;
                        else diag_head = c;
                        diag_tail = c;
                    }
                    else if (c->node_type == REDIRECT_IN) {
                        if (!in) in = c;
                    }
                    else if (c->node_type == REDIRECT_OUT) {
                        if (!out) out = c;
                    }
                    else if (c->node_type == REDIRECT_ERROR) {
                        if (!err) err = c;
                    }
                    else if (c->node_type == REDIRECT_EXPOSE) {
                        /* Gather expose children */
                        ASTNode *ec = c->child;
                        while (ec) {
                            ASTNode *enext = ec->sibling;
                            ec->parent = NULL;
                            ec->sibling = NULL;

                            /* We will add name as a string node, and variable as a target in the rewrite step */
                            if (expose_tail) { expose_tail->sibling = ec; }
                            else { expose_head = ec; }
                            expose_tail = ec;

                            ec = enext;
                        }
                    } else {
                        if (pos == 0) env = c;
                        else if (pos == 1) cmd = c;
                        pos++;
                    }
                    c = next;
                }

                ASTRewriteTemplate *call_tmpl = ast_rw_new(FUNCTION, "_address");

                /* 1. Env */
                ast_rw_add(call_tmpl, env ? ast_rw_reuse(env) : ast_rw_new(STRING, "SYSTEM"));

                /* 2. Cmd */
                ast_rw_add(call_tmpl, cmd ? ast_rw_reuse(cmd) : ast_rw_new(STRING, ""));

                /* Helper to transform redirect node to FUNCTION call */
                #define ADD_RED(r, i_i) do { \
                    if (r) { \
                        ASTNode *rc = r->child; \
                        if (!rc || rc->value_type == TP_VOID) { \
                            ast_rw_add(call_tmpl, ast_rw_new(FUNCTION, "_noredir")); \
                        } else if (i_i) { \
                            ASTRewriteTemplate *func = ast_rw_new(FUNCTION, rc->value_dims ? "_array2redir" : "_string2redir"); \
                            ast_rw_add(func, ast_rw_reuse(rc)); \
                            ast_rw_add(call_tmpl, func); \
                        } else { \
                            ASTRewriteTemplate *func = ast_rw_new(FUNCTION, rc->value_dims ? "_redir2array" : "_redir2string"); \
                            ast_rw_add(func, ast_rw_reuse(rc)); \
                            ast_rw_add(call_tmpl, func); \
                        } \
                    } else { \
                        ast_rw_add(call_tmpl, ast_rw_new(FUNCTION, "_noredir")); \
                    } \
                } while(0)

                ADD_RED(in, 1);
                ADD_RED(out, 0);
                ADD_RED(err, 0);
                #undef ADD_RED

                /* Add exposed */
                c = expose_head;
                while (c) {
                    next = c->sibling;
                    c->sibling = NULL;

                    ASTRewriteTemplate *name_node = ast_rw_new(STRING, c->node_string);
                    ast_rw_add(call_tmpl, name_node);
                    ast_rw_add(call_tmpl, ast_rw_reuse(c));

                    c = next;
                }

                /* Assignment node rc = _address(...) */
                ASTRewriteTemplate *assign_tmpl = ast_rw_add(ast_rw_add(
                    ast_rw_new(ASSIGN, "="),
                    ast_rw_new(VAR_TARGET, "rc")),
                    call_tmpl
                );

                ASTNode *new_assign = ast_execute_rewrite(context, node, assign_tmpl);

                /* Re-attach diagnostics */
                if (new_assign) {
                    c = diag_head;
                    while (c) {
                        next = c->sibling;
                        c->sibling = NULL;
                        add_ast(new_assign, c);
                        c = next;
                    }
                }

                break;

            case REDIRECT_IN:
            case REDIRECT_OUT:
            case REDIRECT_ERROR:
            case REDIRECT_EXPOSE:
                /* Handled by ADDRESS */
                break;

            default:;
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
 * `Sees if rxsysb is needed
 * - If ADDRESS is used
 */
walker_result needs_rxsysb_walker(walker_direction direction,
                                       ASTNode* node, __attribute__((unused)) void *payload) {

    Context *context = (Context*)payload;

    if (direction == out) {
        /* Bottom Up */
        if (node->node_type == ADDRESS || node->node_type == IMPLICIT_CMD || node->node_type == EXIT_EXTENDED) {
            context->need_rxsysb = 1;

            if (node->node_type == ADDRESS || node->node_type == IMPLICIT_CMD) {
                ast_hoist_var(context, node, "rc", -1);
            }

            if (node->node_type == IMPLICIT_CMD || node->node_type == EXIT_EXTENDED) {
                rxcp_exit_bridge_pre_invoke(context, node);
            }
        }
        else if (node->node_type == EXIT) {
            context->need_rxsysb = 1;
        }
    }

    return result_normal;
}

/*
 * Rewrites IMPLICIT_CMD to ADDRESS
 */
walker_result rewrite_implicit_cmd_walker(walker_direction direction,
                                          ASTNode* node, void *payload) {
    Context *context = (Context *) payload;

    if (direction == out && node->node_type == IMPLICIT_CMD) {
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

        if (node->exit_obj_reg != -1) return result_normal;

        /* Warn about implicit address.
         * Guard with ast_hase(node) to ensure we only warn once.
         * Suppress warning if the command starts with a quote (' or "). */
        int starts_with_quote = 0;
        if (node->source_start && (node->source_start[0] == '\'' || node->source_start[0] == '"')) {
            starts_with_quote = 1;
        }

        /* We add the warning before rewrite so it gets caught by ast_execute_rewrite and moved */
        if (!ast_hase(node) && !starts_with_quote) {
            mknd_war(node, "IMPLICIT_ADDRESS");
        }

        ASTRewriteTemplate *addr_tmpl = ast_rw_new(ADDRESS, NULL);

        /* Create explicit environment "SYSTEM" */
        ast_rw_add(addr_tmpl, ast_rw_new(STRING, "SYSTEM"));

        /* Reuse all existing children (command, redirections, diagnostics) */
        ASTNode *c = node->child;
        while (c) {
            ASTNode *next = c->sibling;
            if (c == node->child) node->child = next;
            c->sibling = NULL;
            c->parent = NULL;
            ast_rw_add(addr_tmpl, ast_rw_reuse(c));
            c = next;
        }

        ast_execute_rewrite(context, node, addr_tmpl);
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

                /* Rewrite if node is an Object and has an index (array/property access) */
                int is_native_array = 0;
                if (target->symbolNode && target->symbolNode->symbol) {
                    if (target->symbolNode->symbol->value_dims > 0) is_native_array = 1;
                } else if (target->value_dims > 0) {
                    is_native_array = 1;
                }

                if (index && target->value_type == TP_OBJECT && !is_native_array) {
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
        else if (node->node_type == VAR_SYMBOL) {
            ASTNode *index = node->child;

            /* Rewrite if node is an Object and has an index (array/property access) */
            int is_native_array = 0;
            if (node->symbolNode && node->symbolNode->symbol) {
                if (node->symbolNode->symbol->value_dims > 0) is_native_array = 1;
            } else if (node->value_dims > 0) {
                is_native_array = 1;
            }

            if (index && node->value_type == TP_OBJECT && !is_native_array) {
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
                Symbol *class_sym = sym_rvfn(root, (char*)cname);
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
                member_call->line = node->line;
                member_call->column = node->column;
                member_call->source_start = node->source_start;
                member_call->source_end = node->source_end;

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
