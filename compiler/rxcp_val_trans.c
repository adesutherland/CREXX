#include <string.h>
#include <stdlib.h>
#include <ctype.h>
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

    ASTNode* args_node;
    ASTNode* function_node;
    ASTNode* temp_node;
    ASTNode* current_child;
    ASTNode* last_child;
    ASTNode* next_child;
    ASTNode* var_name;

    if (direction == out) {
        /* Bottom Up */
        switch (node->node_type) {

            case ADDRESS:
                /* Rewrite to an assignment from a function */

                /* Assignment node and remember the command */
                node->node_type = ASSIGN;

                /* Function */
                function_node = ast_ft(context, FUNCTION);
                ast_str(function_node, "_address");
                /* Fix up position for error messages */
                function_node->column = node->column;
                function_node->line = node->line;

                /* Move Params */
                args_node = node->child;
                while (args_node) {
                    ast_del(args_node);
                    add_ast(function_node,args_node);
                    args_node = node->child;
                }

                /* rc is the target */
                temp_node = ast_ft(context, VAR_TARGET);
                ast_str(temp_node, "rc");
                add_ast(node,temp_node);

                /* Add Function */
                add_ast(node,function_node);
                break;

            case REDIRECT_IN:
                if (node->child->value_type == TP_VOID) {
                    /* Just remove the node and convert to a noredir function */
                    ast_del(node->child);
                    node->node_type = FUNCTION;
                    ast_str(node, "_noredir");
                }
                else if (node->child->value_dims) {
                    /* Array Redirect */
                    node->node_type = FUNCTION;
                    ast_str(node, "_array2redir");
                }
                else {
                    /* String Redirect */
                    node->node_type = FUNCTION;
                    ast_str(node, "_string2redir");
                }
                break;

            case REDIRECT_OUT:
            case REDIRECT_ERROR:
                if (node->child->value_type == TP_VOID) {
                    /* Just remove the node and convert to a noredir function */
                    ast_del(node->child);
                    node->node_type = FUNCTION;
                    ast_str(node, "_noredir");
                }
                else if (node->child->value_dims) {
                    /* Array Redirect */
                    node->node_type = FUNCTION;
                    ast_str(node, "_redir2array");
                }
                else {
                    /* String Redirect */
                    node->node_type = FUNCTION;
                    ast_str(node, "_redir2string");
                }
                break;

            case REDIRECT_EXPOSE:
                /* Replace this node with its children (if any)
                 * Each child turns into a string (name of variable) followed by the variable itself */
                if (node->child) {
                    last_child = ast_chdn(node->parent, ast_chdi(node) - 1); /* node's older sibling */
                    current_child = node->child;
                    next_child = current_child->sibling;
                    while (current_child) {
                        /* Link in new string */
                        var_name = ast_fstk(context, current_child);
                        var_name->node_type = STRING;
                        var_name->parent = node->parent;
                        if (last_child) last_child->sibling = var_name;
                        else node->parent->child = var_name;

                        /* Link in current_child - as VAR_SYMBOL */
                        current_child->parent = node->parent;
                        var_name->sibling = current_child;

                        /* Next child */
                        last_child = current_child;
                        current_child = next_child;
                        if (current_child) next_child = current_child->sibling;
                        else next_child = 0;
                    }
                    /* Link to the next node */
                    last_child->sibling = node->sibling;

                    /* Remove NODE safely */
                    node->sibling = 0;
                    node->child = 0;
                    ast_del(node);
                }
                else {
                    /* No children / environment variables - delete node */
                    ast_del(node);
                }
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
        if (node->node_type == REXX_OPTIONS) {
            if (context->need_rxsysb && !context->has_rxsysb) {
                /* we need to import _rxsysb */
                _rxsysb_import_node = ast_ft(context, IMPORT);
                add_ast(node,_rxsysb_import_node);

                _rxsysb_node = ast_ft(context, LITERAL);
                ast_str(_rxsysb_node, "_rxsysb");
                add_ast(_rxsysb_import_node,_rxsysb_node);
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
        if (node->node_type == ADDRESS) {
            context->need_rxsysb = 1;
        }
        else if (node->node_type == EXIT) {
            context->need_rxsysb = 1;
        }
    }

    return result_normal;
}
