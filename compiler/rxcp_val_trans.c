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

    ASTNode* args_node;
    ASTNode* function_node;

    if (direction == out) {
        /* Bottom Up */
        switch (node->node_type) {

            case EXIT:
                /* Rewrite to call of _exit */

                /* Assignment node and remember the command */
                node->node_type = CALL;

                /* Function */
                function_node = ast_ft(context, FUNCTION);
                ast_str(function_node, "_exit");
                /* Fix up position for error messages */
                function_node->column = node->column;
                function_node->line = node->line;

                /* Move Param(s) */
                args_node = node->child;
                while (args_node) {
                    ast_del(args_node);
                    add_ast(function_node, args_node);
                    args_node = node->child;
                }

                /* Add Function */
                add_ast(node, function_node);
                context->changed_flags |= FLAG_VAL_TRANS;
                break;

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

    ASTNode* function_node;
    ASTNode* temp_node;
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
                    ast_del(c);

                    if (c->node_type == ERROR || c->node_type == WARNING) {
                        if (diag_tail) diag_tail->sibling = c;
                        else diag_head = c;
                        diag_tail = c;
                        c->sibling = NULL;
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
                            ast_del(ec);

                            /* Name string */
                            ASTNode *name_node = ast_fstk(context, ec);
                            name_node->node_type = STRING;

                            /* Add name and then the variable */
                            if (expose_tail) { expose_tail->sibling = name_node; }
                            else { expose_head = name_node; }
                            name_node->sibling = ec;
                            expose_tail = ec;
                            ec->sibling = NULL;

                            ec = enext;
                        }
                    } else {
                        if (pos == 0) env = c;
                        else if (pos == 1) cmd = c;
                        else {
                            /* Extra children (e.g. from bad_nodes) - just ignore for now or append to expose?
                             * Better to ignore as they likely have errors. */
                        }
                        pos++;
                    }
                    c = next;
                }

                /* Assignment node rc = _address(...) */
                node->node_type = ASSIGN;

                /* rc is the target */
                temp_node = ast_ft(context, VAR_TARGET);
                ast_str(temp_node, "rc");
                add_ast(node,temp_node);

                /* Function _address */
                function_node = ast_ft(context, FUNCTION);
                ast_str(function_node, "_address");
                function_node->column = node->column;
                function_node->line = node->line;
                add_ast(node,function_node);

                /* 1. Env */
                if (!env) { env = ast_ft(context, STRING); ast_str(env, "SYSTEM"); }
                add_ast(function_node, env);

                /* 2. Cmd */
                if (!cmd) { cmd = ast_ft(context, STRING); ast_str(cmd, ""); }
                add_ast(function_node, cmd);

                /* Helper to transform redirect node to FUNCTION call */
                #define TRANS_RED(r, i_i) \
                if (r) { \
                    ASTNode *rc = r->child; \
                    if (!rc || rc->value_type == TP_VOID) { \
                        if (rc) ast_del(rc); \
                        r->node_type = FUNCTION; ast_str(r, "_noredir"); \
                    } else if (i_i) { \
                        r->node_type = FUNCTION; \
                        if (rc->value_dims) ast_str(r, "_array2redir"); \
                        else ast_str(r, "_string2redir"); \
                    } else { \
                        r->node_type = FUNCTION; \
                        if (rc->value_dims) ast_str(r, "_redir2array"); \
                        else ast_str(r, "_redir2string"); \
                    } \
                } else { \
                    r = ast_ft(context, FUNCTION); ast_str(r, "_noredir"); \
                } \
                add_ast(function_node, r);

                TRANS_RED(in, 1);
                TRANS_RED(out, 0);
                TRANS_RED(err, 0);
                #undef TRANS_RED

                /* Add exposed */
                c = expose_head;
                while (c) {
                    next = c->sibling;
                    c->sibling = NULL;
                    add_ast(function_node, c);
                    c = next;
                }

                /* Re-attach diagnostics */
                c = diag_head;
                while (c) {
                    next = c->sibling;
                    c->sibling = NULL;
                    add_ast(node, c);
                    c = next;
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
    ASTNode *env_node;

    if (direction == in && node->node_type == IMPLICIT_CMD) {
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
         * Guard with ast_hase(node) to ensure we only warn once. */
        if (!ast_hase(node)) {
            mknd_war(node, "IMPLICIT_ADDRESS");
        }

        node->node_type = ADDRESS;

        /* Create explicit environment "SYSTEM" */
        env_node = ast_ft(context, STRING);
        ast_str(env_node, "SYSTEM");

        /* Insert at the beginning of children */
        env_node->sibling = node->child;
        if (node->child) node->child->parent = node; /* Ensure parent is set though ast_ft does it usually */
        node->child = env_node;
        env_node->parent = node;

        /* node->node_string = "SYSTEM"; // Not strictly needed by walker but good for debug */
        context->changed_flags |= FLAG_VAL_TRANS;
    }
    return result_normal;
}

/*
 * Lowers constructor-style declarations: x = .int(1)
 */
walker_result rewrite_constructor_walker(walker_direction direction,
                                         ASTNode* node, void *payload) {
    Context *context = (Context *) payload;

    if (direction == in) {
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

                    ASTNode *class_node = ast_ft(context, CLASS);
                    ast_sstr(class_node, mprintf(".%s", type_name), strlen(type_name) + 1);
                    class_node->free_node_string = 1;
                    class_node->line = rhs->line;
                    class_node->column = rhs->column;
                    class_node->source_start = rhs->source_start;
                    class_node->source_end = rhs->source_end;
                    class_node->token_start = rhs->token_start;
                    class_node->token_end = rhs->token_end;
                    class_node->node_string_length = strlen(class_node->node_string);

                    ast_rpl(rhs, class_node);
                    node->node_type = DEFINE;

                    if (val) {
                        ASTNode *new_assign = ast_ft(context, ASSIGN);
                        ast_str(new_assign, "=");
                        new_assign->line = node->line;
                        new_assign->column = node->column;
                        new_assign->token = node->token;
                        new_assign->source_start = node->source_start;
                        new_assign->source_end = node->source_end;
                        new_assign->token_start = node->token_start;
                        new_assign->token_end = node->token_end;

                        ASTNode *new_target = ast_fstk(context, target);
                        new_target->node_type = VAR_TARGET;
                        new_target->line = target->line;
                        new_target->column = target->column;
                        new_target->token = target->token;
                        new_target->source_start = target->source_start;
                        new_target->source_end = target->source_end;
                        new_target->token_start = target->token_start;
                        new_target->token_end = target->token_end;

                        add_ast(new_assign, new_target);
                        add_ast(new_assign, val);

                        if (node->sibling) {
                            new_assign->sibling = node->sibling;
                        }
                        node->sibling = new_assign;
                        new_assign->parent = node->parent;
                    }
                    context->changed_flags |= FLAG_VAL_TRANS;

                    /* If we had errors, return normal to avoid loop, but let the error be reported */
                    if (has_error) return result_normal;

                    return result_abort;
                }
            }
        }
    }
    return result_normal;
}
