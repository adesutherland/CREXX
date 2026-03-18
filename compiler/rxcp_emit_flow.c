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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxcp_emit.h"
#include "rxcp_util.h"

void emit_flow(ASTNode *node, void *pl) {
    walker_payload *payload = (walker_payload*) pl;
    ASTNode *child1, *child2, *child3, *n;
    char *tp_prefix;
    char *temp1;
    char *temp2;
    char *comment_meta;
    char *op;
    int j;

    child1 = node->child;
    if (child1) child2 = child1->sibling;
    else child2 = NULL;
    if (child2) child3 = child2->sibling;
    else child3 = NULL;

    if (node->value_dims) tp_prefix = "";
    else tp_prefix = type_to_prefix(node->value_type);

    if (tp_prefix[0] == 's' && (node->node_type == TO || node->node_type == BY)) tp_prefix = "f";

    switch (node->node_type) {

        case ARGS:
        case INSTRUCTIONS:
        case COMPILER_ADDED_BLOCK:
            if (!node->output) node->output = output_f();
            n = child1;
            while (n) {
                if (n->output) output_concat(node->output, n->output);
                if (n->cleanup) output_concat(node->output, n->cleanup);
                n = ast_nsib(n);
            }
            break;

        case NOP:
            if (!node->output) node->output = output_f();
            break;

        case SAY:
            /* Add source metadata */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            if (child1->register_num == DONT_ASSIGN_REGISTER) {
                /* If the register is not set then the child is a constant
                 * which we SAY directly. Get the constant string - target type */
                temp2 = format_constant(child1->target_type, child1);
                temp1 = mprintf("   say %s\n", temp2);
                free(temp2);
            }
            else {
                output_concat(node->output, child1->output);
                temp1 = mprintf("   say %c%d\n",
                                child1->register_type,
                                child1->register_num);
            }
            output_append_text(node->output, temp1);
            free(temp1);

            /* Cleanup child */
            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            break;

        case RETURN:
            /* Add source metadata */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            if (child1 == 0) {
                temp1 = mprintf("   ret\n");
            }
            else if (child1->register_num == DONT_ASSIGN_REGISTER) {
                /* If the register is not set then the child is a constant
                 * which we RET directly. Get the constant string - target type */
                temp2 = format_constant(child1->target_type, child1);
                temp1 = mprintf("   ret %s\n", temp2);
                free(temp2);
            }
            else {
                output_concat(node->output, child1->output);
                temp1 = mprintf("   ret %c%d\n",
                                child1->register_type,
                                child1->register_num);
                // TODO - Test array element as we have not unlinked
            }
            output_append_text(node->output, temp1);
            free(temp1);
            break;

        case IF:
            /* Add source metadata */
            comment_meta = get_metaline_range(node, child1);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            if (child1->output) output_concat(node->output, child1->output);
            comment_meta = get_metaline_token_after(child1);
            temp1 = mprintf("   brf l%diffalse,%c%d\n%s",
                            node->node_number,
                            node->register_type,
                            node->register_num,
                            comment_meta);
            output_append_text(node->output, temp1);
            free(temp1);
            free(comment_meta);
            output_concat(node->output, child2->output);
            if (child3) {
                comment_meta = get_metaline_token_after(child2);
                temp1 = mprintf("   br l%difend\n%sl%diffalse:\n",
                                node->node_number,
                                comment_meta,
                                node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                free(comment_meta);
                output_concat(node->output, child3->output);

                temp1 = mprintf("l%difend:\n",
                                node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
                if (child3->cleanup) output_concat(node->output, child3->cleanup);
            }
            else {
                temp1 = mprintf("l%diffalse:\n",
                                node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
            }
            break;

        case DO: /* DO LOOP */
            /* Loop Assignments REPEAT->output */

            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */

            /* child1 is the REPEAT node, child2 is the INSTRUCTIONS */

            comment_meta = get_metaline_token_at(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Init */
            output_concat(node->output, child1->output);

            /* Loop Start */
            temp1 = mprintf("l%ddostart:\n",
                            node->node_number);
            output_append_text(node->output, temp1);
            free(temp1);

            /* Loop Begin Checks REPEAT->loopstartchecks */
            output_concat(node->output, child1->loopstartchecks);

            /* Loop Body - instructions */
            output_concat(node->output, child2->output);

            /* Loop End Checks REPEAT->loopendchecks */
            temp1 = mprintf("l%ddoinc:\n",
                            node->node_number);
            output_append_text(node->output, temp1);
            free(temp1);
            output_concat(node->output, child1->loopendchecks);

            /* Loop increments REPEAT->loopinc */
            output_concat(node->output, child1->loopinc);

            /* Loop End */
            comment_meta = get_metaline_token_after(child2);

            output_append_text(node->output, comment_meta);
            temp1 = mprintf("   br l%ddostart\nl%ddoend:\n",
                            node->node_number, node->node_number);
            output_append_text(node->output, temp1);
            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            free(temp1);
            free(comment_meta);
            break;

        case REPEAT:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */
            if (!node->output) node->output = output_f(); /* Assign / init instruction */
            node->loopstartchecks = output_f(); /* Begin Loop exit checks */
            node->loopinc = output_f(); /* Loop increments */
            node->loopendchecks = output_f(); /* End Loop exit checks */
            while (child1) {
                if (child1->node_type == ASSIGN) {
                    /* Only output is valid - does not follow convention */
                    if (child1->output) output_concat(node->output, child1->output);
                }
                else {
                    if (child1->output)
                        output_concat(node->output, child1->output);
                    if (child1->loopstartchecks)
                        output_concat(node->loopstartchecks, child1->loopstartchecks);
                    if (child1->loopinc)
                        output_concat(node->loopinc, child1->loopinc);
                    if (child1->loopendchecks)
                        output_concat(node->loopendchecks, child1->loopendchecks);
                }
                child1 = child1->sibling;
            }
            /* Output Cleanups */
            child1 = node->child;
            while (child1) {
                if (child1->cleanup) {
                    if (!node->cleanup) node->cleanup = output_f();
                    output_concat(node->cleanup, child1->cleanup);
                }
                child1 = child1->sibling;
            }

            break;

        case FOR:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            output_concat(node->output, child1->output);
            if (child1->register_num != node->register_num ||
                child1->register_type != node->register_type) {
                temp1 = mprintf("   icopy %c%d,%c%d\n",
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
            }
            node->loopstartchecks = output_fs(comment_meta);
            temp1 = mprintf("   bcf l%ddoend,%c%d\n",
                            node->parent->parent->node_number,
                            node->register_type,
                            node->register_num);
            output_append_text(node->loopstartchecks, temp1);
            if (child1->cleanup) {
                if (!node->cleanup) node->cleanup = output_f();
                output_concat(node->cleanup, child1->cleanup);
            }
            free(comment_meta);
            free(temp1);
            break;

        case TO:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */

            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            output_concat(node->output, child1->output);

            /* Need to determine the sign of the BY */
            /* Find the BY */
            j = 1; /* J is the sign: 1 (default)=positive, -1=negative, 0=dynamic */
            n = node->parent->child; /* First sibling */
            while (n) {
                if (n->node_type == BY) {
                    if (ast_chdn(n, 0)) {
                        if (is_constant(ast_chdn(n, 0))) {
                            if (ast_chdn(n, 0)->value_type == ast_chdn(n, 0)->target_type) {
                                /* Is a constant */
                                if (ast_chdn(n, 0)->value_type == TP_INTEGER) {
                                    if (ast_chdn(n, 0)->int_value >= 0) j = 1;
                                    else j = -1;
                                }
                                else if (ast_chdn(n, 0)->value_type == TP_FLOAT) {
                                    if (ast_chdn(n, 0)->float_value >= 0.0) j = 1;
                                    else j = -1;
                                }
                                else if (ast_chdn(n, 0)->value_type == TP_DECIMAL) {
                                    if (ast_chdn(n, 0)->decimal_value[0] == '-') j = -1;
                                    else j = 1;
                                }
                                else j = 1;
                            }
                            else j = 0; /* Not a constant */
                        }
                        else j = 0; /* Not a constant */
                    }
                    else j = 1; /* Implicit by */
                    break;
                }
                n = n->sibling;
            }
            /* n is set by the BY node */

            /* If the REPEAT has a TO it has an ASSIGN and its register
             * number will have been set to the ASSIGN Variable */
            node->loopstartchecks = output_fs(comment_meta);
            switch (j) {
                case 1: /* Positive */
                    temp1 = mprintf("   %sgt r%d,%c%d,%c%d\n   brt l%ddoend,r%d\n",
                                    tp_prefix,
                                    node->additional_registers,
                                    node->parent->register_type,
                                    node->parent->register_num,
                                    child1->register_type,
                                    child1->register_num,
                                    node->parent->parent->node_number,
                                    node->additional_registers);
                    break;
                case -1: /* Negative */
                    temp1 = mprintf("   %slt r%d,%c%d,%c%d\n   brt l%ddoend,r%d\n",
                                    tp_prefix,
                                    node->additional_registers,
                                    node->parent->register_type,
                                    node->parent->register_num,
                                    child1->register_type,
                                    child1->register_num,
                                    node->parent->parent->node_number,
                                    node->additional_registers);
                    break;
                default: /* Dynamic by value */
                    /* We need a zero (int or flaot */
                    if (*tp_prefix == 'i') op = "0";
                    else op = "0.0";
                    temp1 = mprintf(
                            "   %slt r%d,%c%d,%s\n" /* Check the by value sign */
                            "   brt l%ddoneg1,r%d\n"    /* JMP to Negative BY */

                            "   %sgt r%d,%c%d,%c%d\n"   /* Pos BY */
                            "   brtf l%ddoend,l%ddoneg2,r%d\n"

                            "l%ddoneg1:\n"
                            "   %slt r%d,%c%d,%c%d\n"   /* Neg BY */
                            "   brt l%ddoend,r%d\n"

                            "l%ddoneg2:\n",

                            tp_prefix,
                            node->additional_registers,
                            ast_chdn(n, 0)->register_type,
                            ast_chdn(n, 0)->register_num,
                            op,
                            node->parent->parent->node_number,
                            node->additional_registers,

                            tp_prefix,
                            node->additional_registers,
                            node->parent->register_type,
                            node->parent->register_num,
                            child1->register_type,
                            child1->register_num,
                            node->parent->parent->node_number,
                            node->parent->parent->node_number,
                            node->additional_registers,

                            node->parent->parent->node_number,
                            tp_prefix,
                            node->additional_registers,
                            node->parent->register_type,
                            node->parent->register_num,
                            child1->register_type,
                            child1->register_num,
                            node->parent->parent->node_number,
                            node->additional_registers,

                            node->parent->parent->node_number);
            }
            output_append_text(node->loopstartchecks, temp1);
            free(temp1);
            free(comment_meta);
            if (child1->cleanup) {
                if (!node->cleanup) node->cleanup = output_f();
                output_concat(node->cleanup, child1->cleanup);
            }
            break;

        case BY:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */

            /* If the REPEAT has a BY it has an ASSIGN and its register
             * number will have been set to the ASSIGN Variable */

            if (child1) {
                /* BY explicitly stated */
                comment_meta = get_metaline(node);
                if (node->output) output_prepend_text(comment_meta, node->output);
                else node->output = output_fs(comment_meta);
                output_concat(node->output, child1->output);

                node->loopinc = output_fs(comment_meta);
                temp1 = mprintf(" %sadd %c%d,%c%d,%c%d\n",
                                tp_prefix,
                                node->parent->register_type,
                                node->parent->register_num,
                                node->child->register_type,
                                node->child->register_num,
                                node->parent->register_type,
                                node->parent->register_num);
                output_append_text(node->loopinc, temp1);
                free(comment_meta);
                free(temp1);
                if (child1->cleanup) {
                    if (!node->cleanup) node->cleanup = output_f();
                    output_concat(node->cleanup, child1->cleanup);
                }
            }
            else {
                /* BY Added implicitly - increment by 1 */
                /* For the source we can only reference the symbol in the loop assignment node */
//                    comment_meta = get_comment_line_number_only(node->parent, "{Implicit \"BY 1\"}");
                comment_meta = get_metaline_token_at(node->parent->child);
                node->loopinc = output_fs(comment_meta);
                free(comment_meta);

                if (*tp_prefix == 'i') {
                    temp1 = mprintf("   inc %c%d\n",
                                    node->parent->register_type,
                                    node->parent->register_num);
                }
                else {
                    temp1 = mprintf("   %sadd %c%d,%c%d,1.0\n",
                                    tp_prefix,
                                    node->parent->register_type,
                                    node->parent->register_num,
                                    node->parent->register_type,
                                    node->parent->register_num);
                }
                output_append_text(node->loopinc, temp1);
                free(temp1);
            }
            break;

        case WHILE:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */
            comment_meta = get_metaline(node);
            node->loopstartchecks = output_fs(comment_meta);
            free(comment_meta);
            output_concat(node->loopstartchecks, child1->output);
            temp1 = mprintf("   brf l%ddoend,%c%d\n",
                            node->parent->parent->node_number,
                            node->register_type,
                            node->register_num);
            output_append_text(node->loopstartchecks, temp1);
            free(temp1);
            if (child1->cleanup) {
                if (!node->cleanup) node->cleanup = output_f();
                output_concat(node->cleanup, child1->cleanup);
            }
            break;

        case UNTIL:
            /* Loop output mapping / convention
             * output =  Loop Assign / init instruction
             * loopstartchecks = Loop iteration beginning exit checks
             * loopinc = Loop iteration increments
             * loopendchecks = Loop iteration end exit checks */
            comment_meta = get_metaline(node);
            node->loopendchecks = output_fs(comment_meta);
            free(comment_meta);
            output_concat(node->loopendchecks, child1->output);
            temp1 = mprintf("   brt l%ddoend,%c%d\n",
                            node->parent->parent->node_number,
                            node->register_type,
                            node->register_num);
            output_append_text(node->loopendchecks, temp1);
            free(temp1);
            if (child1->cleanup) {
                if (!node->cleanup) node->cleanup = output_f();
                output_concat(node->cleanup, child1->cleanup);
            }
            break;

        case LEAVE:
            /* Leave Loop */
            /* Add source metadata */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            temp1 = mprintf("   br l%ddoend\n",
                            node->association->node_number);
            output_append_text(node->output, temp1);
            free(temp1);
            break;

        case ITERATE:
            /* Iterate Loop */
            /* Add source metadata */
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            /* Add Variable Metadata */
            add_variable_metadata(node);

            temp1 = mprintf("   br l%ddoinc\n",
                            node->association->node_number);
            output_append_text(node->output, temp1);
            free(temp1);
            break;

        default:
            break;
    }
}
