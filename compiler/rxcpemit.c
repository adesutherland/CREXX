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
#include "rxcp_val.h"

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

static int is_large_value(ASTNode *node) {
    if (!node) return 0;

    if (node->value_dims || node->target_dims) return 1;

    return node->value_type == TP_STRING ||
           node->value_type == TP_OBJECT ||
           node->value_type == TP_BINARY ||
           node->target_type == TP_STRING ||
           node->target_type == TP_OBJECT ||
           node->target_type == TP_BINARY;
}

static Symbol *current_procedure_symbol(ASTNode *node) {
    ASTNode *proc_node;

    proc_node = ast_proc(node);
    if (!proc_node || !proc_node->symbolNode) return 0;
    return proc_node->symbolNode->symbol;
}

static int uses_implicit_main_args(ASTNode *node) {
    Symbol *symbol = current_procedure_symbol(node);
    return symbol && symbol->is_implicit_main;
}

static int visible_fixed_arg_count(ASTNode *node) {
    Symbol *symbol = current_procedure_symbol(node);
    int fixed_args;

    if (!symbol) return 0;

    fixed_args = (int)symbol->fixed_args;

    /* Explicit main() with no declared parameters still receives the hidden argv array. */
    if (symbol->is_main && !symbol->is_implicit_main && symbol->fixed_args == 0) fixed_args++;

    return fixed_args;
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
    int fixed_arg_count;
    int implicit_main_args;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char ret_type;
    int ret_num;

    child1 = node->child;
    if (child1) child2 = child1->sibling;
    else child2 = NULL;
    if (child2) child3 = child2->sibling;
    else child3 = NULL;
    fixed_arg_count = visible_fixed_arg_count(node);
    implicit_main_args = uses_implicit_main_args(node);

    if (direction == out) {
        /* OUT - BOTTOM UP */

        /* Operator and type prefix */
        op = 0;
        if (node->value_dims) tp_prefix = "";
        else tp_prefix = type_to_prefix(node->value_type);

        switch (node->node_type) {

            case REXX_UNIVERSE:
                emit_proc(node, pl);
                break;

            case PROGRAM_FILE:
                emit_proc(node, pl);
                break;

            case IMPORTED_FILE:
            {
                char *buf = mprintf("\n/* Imported Declaration from file: %s */\n",
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

            case CLASS_DEF:
            case FACTORY:
            case METHOD:
            case PROCEDURE:
                emit_proc(node, pl);
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
                if (child1->node_type == VAR_TARGET || child1->node_type == VAR_REFERENCE) {
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
                            /* `.ref` and read-only by-value formals already
                             * share the incoming argument register, so no
                             * defensive copy is required here. */
                            temp1 = mprintf("l%da:\n", child1->node_number);
                            output_append_text(node->output, temp1);
                            free(temp1);
                        } else {
                            /* Writable by-value formals still need an isolated
                             * local register when the caller value must be
                             * preserved. */

                            /* Only worry about it if it is a big register */
                            if (is_large_value(node)) {
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
                        /* Writable by-value formals may need a defensive copy;
                         * read-only by-value formals were already marked
                         * `is_const_arg` by semantic analysis. */

                        /* Only worry about it if it is a big register */
                        if (is_large_value(node)) {
                            temp1 = mprintf("   brtpandt l%dc,%c%d,%d\n"
                                            "   %scopy %c%d,%c%d\n"
                                            "   acopy %c%d,%c%d\n"
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

            case FACTORY_CALL:
            case MEMBER_CALL:
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
                if (implicit_main_args) {
                    temp1 = mprintf("   getattrs %c%d,a1,0\n",
                                    node->register_type,
                                    node->register_num);
                } else {
                    /* Get the total args and subtract the visible fixed args of the procedure we are in. */
                    temp1 = mprintf("   icopy %c%d,a0\n"
                                    "   isub %c%d,%c%d,%d\n",
                                    node->register_type,
                                    node->register_num,
                                    node->register_type,
                                    node->register_num,
                                    node->register_type,
                                    node->register_num,
                                    fixed_arg_count
                    );
                }
                output_append_text(node->output, temp1);
                free(temp1);
                type_promotion(node);
                break;

            case OP_ARG_VALUE:
                if (!node->output) node->output = output_f();

                if (implicit_main_args) {
                    if (child1->register_num == DONT_ASSIGN_REGISTER) {
                        /* Child is a constant. */
                        temp2 = format_constant(child1->value_type, child1);
                        temp1 = mprintf("   getattrs %c%d,a1,0\n"
                                        "   ichkrng %s,1,%c%d\n"
                                        "   linkattr1 %c%d,a1,%s\n",
                                        node->register_type, node->register_num,
                                        temp2,
                                        node->register_type, node->register_num,
                                        node->register_type, node->register_num,
                                        temp2);
                        free(temp2);
                    } else {
                        /* Child is a register. */
                        temp1 = mprintf("   getattrs %c%d,a1,0\n"
                                        "   ichkrng %c%d,1,%c%d\n"
                                        "   linkattr1 %c%d,a1,%c%d\n",
                                        node->register_type, node->register_num,
                                        child1->register_type, child1->register_num,
                                        node->register_type, node->register_num,
                                        node->register_type, node->register_num,
                                        child1->register_type, child1->register_num);
                    }
                } else if (child1->register_num == DONT_ASSIGN_REGISTER) {
                    /* Child is a constant. */

                    /* Needed to calculate the argument number taking into account the number of visible fixed args. */
                    temp2 = format_constant(child1->value_type, child1);
                    {
                        int arg_ix = atoi(temp2) + fixed_arg_count;
                        temp1 = mprintf("   icopy %c%d,a0\n" /* Total number of arguments */
                                        "   isub %c%d,%c%d,%d\n"    /* Deduct # fixed arguments */
                                        "   ichkrng %s,1,%c%d\n"    /* Validate Range */
                                        "   linkarg %c%d,%d\n",     /* Link to argument (with added # fixed arguments) */

                                        node->register_type, node->register_num,

                                        node->register_type,
                                        node->register_num,
                                        node->register_type,
                                        node->register_num,
                                        fixed_arg_count,

                                        temp2,
                                        node->register_type, node->register_num,

                                        node->register_type, node->register_num,
                                        arg_ix);
                    }
                    free(temp2);
                } else {
                    /* Child is a register. */
                    temp1 = mprintf("   icopy %c%d,a0\n"    /* Total number of arguments */
                                    "   isub %c%d,%c%d,%d\n"       /* Deduct # of fixed arguments */
                                    "   ichkrng %c%d,1,%c%d\n"     /* Validate Range */
                                    "   linkarg %c%d,%c%d,%d\n",   /* Link to argument (third param adds # fixed arguments) */

                                    node->register_type, node->register_num,

                                    node->register_type,
                                    node->register_num,
                                    node->register_type,
                                    node->register_num,
                                    fixed_arg_count,

                                    child1->register_type, child1->register_num,
                                    node->register_type, node->register_num,

                                    node->register_type, node->register_num,
                                    child1->register_type, child1->register_num,
                                    fixed_arg_count);
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
                if (implicit_main_args) {
                    if (child1->register_num == DONT_ASSIGN_REGISTER) {
                        /* Child is a constant. */
                        temp2 = format_constant(child1->value_type, child1);
                        temp1 = mprintf("   getattrs %c%d,a1,0\n"
                                        "   ilte %c%d,%s,%c%d\n",
                                        node->register_type, node->register_num,
                                        node->register_type, node->register_num,
                                        temp2,
                                        node->register_type, node->register_num);
                        free(temp2);
                    } else {
                        /* Child is a register. */
                        temp1 = mprintf("   ilt %c%d,%c%d,1\n"
                                        "   signalt \"OUT_OF_RANGE\",%c%d\n"
                                        "   getattrs %c%d,a1,0\n"
                                        "   ilte %c%d,%c%d,%c%d\n",
                                        node->register_type, node->register_num,
                                        child1->register_type, child1->register_num,
                                        node->register_type, node->register_num,
                                        node->register_type, node->register_num,
                                        node->register_type, node->register_num,
                                        child1->register_type, child1->register_num,
                                        node->register_type, node->register_num);
                    }
                } else if (child1->register_num == DONT_ASSIGN_REGISTER) {
                    /* Child is a constant. */
                    /* < 1 will already be checked */

                    /* Needed to calculate the argument number by adding #fixed args. */
                    temp2 = format_constant(child1->value_type, child1);
                    {
                        int arg_ix = atoi(temp2) + fixed_arg_count;
                        temp1 = mprintf("   icopy %c%d,a0\n"       /* Total number of arguments (fixed and variable) */
                                        "   ilte %c%d,%d,%c%d\n",  /* `Is <= number of registers? */

                                        node->register_type, node->register_num,

                                        node->register_type, node->register_num,
                                        arg_ix,
                                        node->register_type, node->register_num);
                    }
                    free(temp2);
                } else {
                    /* Child is a register. */
                    temp1 = mprintf("   ilt %c%d,%c%d,1\n"         /* Is parm < 1? */
                                    "   signalt \"OUT_OF_RANGE\",%c%d\n"   /* Signal if so */
                                    "   icopy %c%d,a0\n"           /* Total number of arguments */
                                    "   isub %c%d,%c%d,%d\n"       /* Deduct # of fixed arguments */
                                    "   ilte %c%d,%c%d,%c%d\n",    /* Is <= number of registers? */

                                    node->register_type, node->register_num,
                                    child1->register_type, child1->register_num,

                                    node->register_type, node->register_num,

                                    node->register_type, node->register_num,

                                    node->register_type, node->register_num,
                                    node->register_type, node->register_num,
                                    fixed_arg_count,

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

                if (node->node_type == VAR_SYMBOL && !child1 &&
                    node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->scope &&
                    node->symbolNode->symbol->scope->defining_node &&
                    node->symbolNode->symbol->scope->defining_node->node_type == CLASS_DEF) {
                    /* Attribute Read */
                    int index = 0;
                    if (sym_nond(node->symbolNode->symbol) > 0) {
                        ASTNode *def_node = sym_trnd(node->symbolNode->symbol, 0)->node;
                        if (def_node && def_node->parent && def_node->parent->node_type == DEFINE) {
                            ASTNode *nr = ast_chld(def_node->parent, NODE_REGISTER, 0);
                            if (nr) {
                                ASTNode *idx = ast_chld(nr, INTEGER, 0);
                                if (idx) index = node_to_integer(idx);
                                else if (nr->int_value) index = (int)nr->int_value;
                            }
                        }
                    }

                    ASTNode *proc = ast_proc(node);
                    char this_type = 'a'; int this_num = 1; /* Default for METHOD */
                    if (proc && proc->node_type == FACTORY) {
                        ASTNode star_node;
                        memset(&star_node, 0, sizeof(ASTNode));
                        star_node.node_string = "\xc2\xa7" "factory"; star_node.node_string_length = 9;
                        Symbol *star_sym = sym_lrsv(proc->scope, &star_node);
                        if (star_sym) { this_type = star_sym->register_type; this_num = star_sym->register_num; }
                    }
                    temp1 = mprintf("   linkattr1 %c%d,%c%d,%d\n",
                                    node->register_type, node->register_num,
                                    this_type, this_num, index);
                    output_append_text(node->output, temp1);
                    free(temp1);

                    /* Cleanup */
                    temp1 = mprintf("   unlink %c%d\n", node->register_type, node->register_num);
                    node->cleanup = output_fs(temp1);
                    free(temp1);
                    break;
                } else if (child1) {
                    /* We are an array */
                    /* Essentially, we are linking the found array element as the nodes result - which will need unlinking later */
                    char from_reg_type = node->symbolNode->symbol->register_type;
                    int from_reg_num = node->symbolNode->symbol->register_num;
                    char unlink_needed = 0;

                    char is_property = 0;
                    if (node->symbolNode && node->symbolNode->symbol &&
                        node->symbolNode->symbol->scope &&
                        node->symbolNode->symbol->scope->defining_node &&
                        node->symbolNode->symbol->scope->defining_node->node_type == CLASS_DEF) {
                        is_property = 1;
                        
                        /* Attribute Read - link the array into the first additional register */
                        int index = 0;
                        if (sym_nond(node->symbolNode->symbol) > 0) {
                            ASTNode *def_node = sym_trnd(node->symbolNode->symbol, 0)->node;
                            if (def_node && def_node->parent && def_node->parent->node_type == DEFINE) {
                                ASTNode *nr = ast_chld(def_node->parent, NODE_REGISTER, 0);
                                if (nr) {
                                    ASTNode *idx = ast_chld(nr, INTEGER, 0);
                                    if (idx) index = node_to_integer(idx);
                                    else if (nr->int_value) index = (int)nr->int_value;
                                }
                            }
                        }

                        ASTNode *proc = ast_proc(node);
                        char this_type = 'a'; int this_num = 1; /* Default for METHOD */
                        if (proc && proc->node_type == FACTORY) {
                            ASTNode star_node;
                            memset(&star_node, 0, sizeof(ASTNode));
                            star_node.node_string = "\xc2\xa7" "factory"; star_node.node_string_length = 9;
                            Symbol *star_sym = sym_lrsv(proc->scope, &star_node);
                            if (star_sym) { this_type = star_sym->register_type; this_num = star_sym->register_num; }
                        }

                        temp1 = mprintf("   linkattr1 r%d,%c%d,%d\n",
                                        node->additional_registers,
                                        this_type, this_num, index);
                        output_append_text(node->output, temp1);
                        free(temp1);

                        /* Add cleanup to unlink this property reference */
                        temp1 = mprintf("   unlink r%d\n", node->additional_registers);
                        node->cleanup = output_fs(temp1);
                        free(temp1);

                        from_reg_type = 'r';
                        from_reg_num = node->additional_registers;
                    }
                    int math_reg = node->additional_registers + is_property;

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
                                                    math_reg,
                                                    from_reg_type,
                                                    from_reg_num,
                                                    base - 1,

                                                    node->register_num,

                                                    node->register_num,
                                                    math_reg);
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

                        /* Make temp2 the base 1 element index number */
                        if (child1->node_type == INTEGER || child1->node_type == CONSTANT || child1->node_type == STRING) {
                            /* Make temp2 the base 1 element index number */
                            temp2 = format_constant(child1->value_type, child1);
                            if (base != 1 && (child1->node_type == INTEGER || child1->node_type == CONSTANT)) {
                                int ix = atoi(temp2) + 1 - base;
                                free(temp2);
                                temp2 = mprintf("%d", ix);
                            }
                        } else temp2 = 0;

                        /* Make sure there are enough attributes */
                        if (node->symbolNode->symbol->dim_elements[ast_chdi(child1)]) {
                            /* Fixed array set to the dimension size - later linkattr1 might throw a signal if out of range by design */
                            temp1 = mprintf("   setattrs %c%d,%d\n",
                                            from_reg_type, from_reg_num,
                                            node->symbolNode->symbol->dim_elements[ast_chdi(child1)]);
                        } else if (child1->node_type == INTEGER || child1->node_type == CONSTANT || child1->node_type == STRING) {
                            /* Variable array and constant parameter - set min attributes which gives a growth buffer */
                            if (child1->value_type != TP_INTEGER) {
                                // This should never happen - print an in fatal internal error to stderr and bail
                                fprintf(stderr, "INTERNAL ERROR: non-integer constant used as array index\n");
                                exit(1);
                            }
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
                        if (child1->node_type == INTEGER || child1->node_type == CONSTANT || child1->node_type == STRING) {
                            /* Constant Parameter */
                            if (child1->value_type != TP_INTEGER) {
                                mknd_err(child1, "BAD_CONVERSION");
                            }
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
                                            math_reg,
                                            child1->register_type, child1->register_num,
                                            1 - base,
                                            node->register_num,
                                            from_reg_type, from_reg_num,
                                            math_reg);
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

            case CLASS:
                /* A class literal (e.g. .int) used as a value - this represents the default value
                 * of that class. For Level B this is always null / zero */
                if (!node->output) node->output = output_f();
                temp1 = mprintf("   null %c%d\n", node->register_type, node->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
                break;

            case CONSTANT:
            case CONST_SYMBOL:
            case STRING:
            case FLOAT:
            case DECIMAL:
            case INTEGER:
            case BLOCK_EXPR:
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

                if (child1->symbolNode && child1->symbolNode->symbol && child1->symbolNode->symbol->scope &&
                    !child1->child &&
                    child1->symbolNode->symbol->scope->defining_node &&
                    child1->symbolNode->symbol->scope->defining_node->node_type == CLASS_DEF) {
                    /* Attribute Write */
                    int index = 0;
                    if (sym_nond(child1->symbolNode->symbol) > 0) {
                        ASTNode *def_node = sym_trnd(child1->symbolNode->symbol, 0)->node;
                        if (def_node && def_node->parent && def_node->parent->node_type == DEFINE) {
                            ASTNode *nr = ast_chld(def_node->parent, NODE_REGISTER, 0);
                            if (nr) {
                                ASTNode *idx = ast_chld(nr, INTEGER, 0);
                                if (idx) index = node_to_integer(idx);
                                else if (nr->int_value) index = (int)nr->int_value;
                            }
                        }
                    }

                    ASTNode *proc = ast_proc(node);
                    char this_type = 'a'; int this_num = 1; /* Default for METHOD */
                    if (proc && proc->node_type == FACTORY) {
                        ASTNode star_node;
                        memset(&star_node, 0, sizeof(ASTNode));
                        star_node.node_string = "\xc2\xa7" "factory"; star_node.node_string_length = 9;
                        Symbol *star_sym = sym_lrsv(proc->scope, &star_node);
                        if (star_sym) { this_type = star_sym->register_type; this_num = star_sym->register_num; }
                    }
                    temp1 = mprintf("   linkattr1 %c%d,%c%d,%d\n"
                                    "   %scopy %c%d,%c%d\n"
                                    "   unlink %c%d\n",
                                    child1->register_type, child1->register_num,
                                    this_type, this_num,
                                    index,
                                    tp_prefix,
                                    child1->register_type, child1->register_num,
                                    child2->register_type, child2->register_num,
                                    child1->register_type, child1->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                } else if (child1->register_num != child2->register_num ||
                    child1->register_type != child2->register_type) {
                    int aggregate_assign =
                            !child1->child &&
                            (child1->value_dims > 0 || child1->target_dims > 0 ||
                             child2->value_dims > 0 || child2->target_dims > 0 ||
                             child1->value_type == TP_BINARY || child1->target_type == TP_BINARY ||
                             child2->value_type == TP_BINARY || child2->target_type == TP_BINARY);

                    if (aggregate_assign) {
                        temp1 = mprintf("   copy %c%d,%c%d\n"
                                        "   acopy %c%d,%c%d\n",
                                        child1->register_type,
                                        child1->register_num,
                                        child2->register_type,
                                        child2->register_num,
                                        child1->register_type,
                                        child1->register_num,
                                        child2->register_type,
                                        child2->register_num);
                    } else {
                        temp1 = mprintf("   %scopy %c%d,%c%d\n",
                                        tp_prefix,
                                        child1->register_type,
                                        child1->register_num,
                                        child2->register_type,
                                        child2->register_num);
                    }
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

            case LEAVE_WITH:
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
