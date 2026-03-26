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
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxcp_emit.h"

void emit_expression(ASTNode *node, void *payload) {
    walker_payload *wp = (walker_payload*)payload;
    char *op = 0;
    char *tp_prefix = type_to_prefix(node->value_type);
    char *temp1;
    char *temp2;
    char *comment_meta;
    ASTNode *n;
    int i, j, k;
    char ret_type;
    int ret_num;
    ASTNode *child1 = node->child;
    ASTNode *child2 = node->child ? node->child->sibling : 0;
    ASTNode *child3 = node->child && node->child->sibling ? node->child->sibling->sibling : 0;

    switch (node->node_type) {

        case FACTORY_CALL:
        case MEMBER_CALL:
        case FUNCTION:
            /* Return Registers */
            ret_type = node->register_type;
            ret_num = node->register_num;

            /* META */
            /*
            comment_meta = get_metaline(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else ode->output = output_fs(comment_meta);
            free(comment_meta);
            */
            if (!node->output) node->output = output_f();

            /* Add Variable Metadata */
            add_variable_metadata(node);

            /* Number of arguments */
            temp1 = mprintf("   load r%d,%d\n",
                            node->additional_registers,
                            node->num_additional_registers - 1);
            output_append_text(node->output, temp1);
            free(temp1);

            /* First Step through the arguments evaluating any expressions
             * This must be done BEFORE argument marshalling using swaps   */
            n = child1;
            while (n) {
                if (n->output) output_concat(node->output, n->output);
                n = n->sibling;
            }

            /* Now step through the arguments - marshalling them in order and
             * setting argument flags as required */
            n = child1;
            i = node->additional_registers + 1; /* The first one is the number of arguments */
            while (n) {
                k = 0; /* 1 if we need to settp */
                j = 0; /* The required value of settp */

                /* Set value provided flag */
                if (n->node_type != NOVAL) j |= REGTP_VAL;

                /* Used for "pass be value" large (strings, objects) registers ONLY
                 * set (2) means that it is not a symbol so its value does not need
                 * preserving */
                if (!n->is_ref_arg &&
                    (n->value_dims || n->target_type == TP_STRING || n->target_type == TP_OBJECT || n->target_type == TP_BINARY)) {
                    k = 1; /* This means we will settp */
                    if (!n->symbolNode) j |= REGTP_NOTSYM; /* Mark it as not a symbol */
                }

                /* Optional arguments need to use the settp flag */
                if (n->is_opt_arg) {
                    k = 1; /* means we have to settp */
                }
                if (k) { /* We need to settp */
                    temp1 = mprintf("   settp %c%d,%d\n",
                                    n->register_type,
                                    n->register_num,
                                    j);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }

                if (n->register_type != 'r' ||  n->register_num != i) {
                    /* We need to swap registers to get it right for the call */
                    temp1 = mprintf("   swap r%d,%c%d\n",
                                    i, n->register_type, n->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);

                    /* Map the call result through the restore-swap sequence so
                     * it lands in the node's final register after marshalling
                     * is unwound. */
                    if (ret_type == 'r' && ret_num == i) {
                        ret_type = n->register_type;
                        ret_num = n->register_num;
                    } else if (ret_type == n->register_type &&
                               ret_num == n->register_num) {
                        ret_type = 'r';
                        ret_num = (int)i;
                    }
                }

                n = n->sibling; i++;
            }

            /* Actual Call */
            if (node->symbolNode) {
                char *call_name;
                int use_mangled = 0;
                Symbol *fsym = node->symbolNode->symbol;
                if (fsym && sym_nond(fsym) > 0) {
                    SymbolNode *defsn = sym_trnd(fsym, 0);
                    if (defsn && defsn->node && (defsn->node->node_type == METHOD || defsn->node->node_type == FACTORY)) {
                        use_mangled = 1;
                    }
                }
                if (use_mangled) call_name = sym_mngd_frnm(node->symbolNode->symbol);
                else {
                    int is_imported = 0;
                    if (node->symbolNode->symbol) {
                        SymbolNode *defsn = sym_trnd(node->symbolNode->symbol, 0);
                        if (defsn && defsn->node && defsn->node->node_type == PROCEDURE) {
                            if (ast_chld(defsn->node, INSTRUCTIONS, NOP)->node_type == NOP) {
                                is_imported = 1;
                            }
                        }
                    }
                    if (is_imported) {
                        call_name = sym_frnm(node->symbolNode->symbol);
                    } else {
                        /* For PROCEDURE, preserve case if possible */
                        if (node->node_string) {
                            size_t start = 0;
                            size_t len = node->node_string_length;
                            if (len >= 2 && (node->node_string[0] == '\'' || node->node_string[0] == '\"') && node->node_string[len - 1] == node->node_string[0]) {
                                start = 1;
                                len -= 2;
                            }
                            call_name = malloc(len + 1);
                            memcpy(call_name, node->node_string + start, len);
                            call_name[len] = 0;
                        } else call_name = strdup(node->symbolNode->symbol->name);
                    }
                }
                temp1 = mprintf("   call %c%d,%s(),r%d\n",
                                ret_type, ret_num,
                                call_name,
                                node->additional_registers);
                free(call_name);
            } else {
                temp1 = mprintf("   call %c%d,%.*s(),r%d\n",
                                ret_type, ret_num,
                                (int) node->node_string_length, node->node_string,
                                node->additional_registers);
            }
            output_append_text(node->output, temp1);
            free(temp1);

            /* Step through for swapping registers back */
            n = child1;
            i = node->additional_registers + 1; /* First one is the number of arguments */
            while (n) {
                if (n->register_type != 'r' ||  n->register_num != i) {
                    /* We need to swap registers */
                    /* I have reversed arguments just for readability */
                    temp1 = mprintf("   swap %c%d,r%d\n",
                                    n->register_type, n->register_num, i);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                if (n->cleanup) output_concat(node->output, n->cleanup);
                n = n->sibling; i++;
            }

            type_promotion(node);
            break;

        case OP_CONCAT:
            op="concat";
        case OP_SCONCAT:
            if (!op) op="sconcat";
            if (!node->output) node->output = output_f();
            /* One or other of the operands may be a constant */
            /* If the register is not set then the child is a constant */
            if (child1->register_num == DONT_ASSIGN_REGISTER) {
                if (child2->output) output_concat(node->output, child2->output);
                /* It MUST have been converted to a STRING
                 * We don't need to worry about ".0" to show a float literal */
                temp1 = mprintf("   %s %c%d,\"%.*s\",%c%d\n",
                                op,
                                node->register_type,
                                node->register_num,
                                (int) child1->node_string_length, child1->node_string,
                                child2->register_type,
                                child2->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
            }

            /* If the register is not set then the child is a constant */
            else if (child2->register_num == DONT_ASSIGN_REGISTER) {
                if (child1->output) output_concat(node->output, child1->output);
                /* It MUST have been converted to a STRING
                 * We don't need to worry about ".0" to show a float literal */
                temp1 = mprintf("   %s %c%d,%c%d,\"%.*s\"\n",
                                op,
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num,
                                (int) child2->node_string_length, child2->node_string);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
            }

            /* Neither are constants */
            else {
                if (child1->output) output_concat(node->output, child1->output);
                if (child2->output) output_concat(node->output, child2->output);
                temp1 = mprintf("   %s %c%d,%c%d,%c%d\n",
                                op,
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num,
                                child2->register_type,
                                child2->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
            }

            type_promotion(node);
            break;

        /* These operators have a prefix type of that of the first child */
        case OP_COMPARE_EQUAL:
            if (!op) op="eq";
        case OP_COMPARE_NEQ:
            if (!op) op="ne";
        case OP_COMPARE_GT:
            if (!op) op="gt";
        case OP_COMPARE_LT:
            if (!op) op="lt";
        case OP_COMPARE_GTE:
            if (!op) op="gte";
        case OP_COMPARE_LTE:
            if (!op) op="lte";
        case OP_COMPARE_S_EQ:
            if (!op) op="eqs";
        case OP_COMPARE_S_NEQ:
            if (!op) op="nes";
        case OP_COMPARE_S_GT:
            if (!op) op="gts";
        case OP_COMPARE_S_LT:
            if (!op) op="lts";
        case OP_COMPARE_S_GTE:
            if (!op) op="gtes";
        case OP_COMPARE_S_LTE:
            if (!op) op="ltes";

            tp_prefix = type_to_prefix(child1->target_type);

        /* These operators use the type prefix already set (i.e. of their type) */
        case OP_ADD:
            if (!op) op="add";
        case OP_MULT:
            if (!op) op="mult";
        case OP_MINUS:
            if (!op) op="sub";
        case OP_POWER:
            if (!op) op="pow";
        case OP_DIV:
            if (!op) op="div";
        case OP_IDIV:
            if (!op) {
                if (*tp_prefix == 'i') {
                    op="div"; /* we will append the type later; noting that idiv is correct, not iidiv */
                } else {
                    op="idiv"; /* i.e. it will become didiv or fidiv */
                }
            }
        case OP_MOD:
            if (!op) op="mod";

            if (!node->output) node->output = output_f();

            /* One or other of the operands may be a constant */
            /* If the register is not set then the child is a constant */
            if (child1->register_num == DONT_ASSIGN_REGISTER) {
                if (child2->output) output_concat(node->output, child2->output);
                temp2 = format_constant(child1->value_type, child1);
                temp1 = mprintf("   %s%s %c%d,%s,%c%d\n",
                                tp_prefix,
                                op,
                                node->register_type,
                                node->register_num,
                                temp2,
                                child2->register_type,
                                child2->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
                free(temp2);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
            }

            /* If the register is not set then the child is a constant */
            else if (child2->register_num == DONT_ASSIGN_REGISTER) {
                if (child1->output) output_concat(node->output, child1->output);
                temp2 = format_constant(child2->value_type, child2);
                temp1 = mprintf("   %s%s %c%d,%c%d,%s\n",
                                tp_prefix,
                                op,
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num,
                                temp2);
                output_append_text(node->output, temp1);
                free(temp1);
                free(temp2);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
            }

            /* Neither are constants */
            else {
                if (child1->output) output_concat(node->output, child1->output);
                if (child2->output) output_concat(node->output, child2->output);
                temp1 = mprintf("   %s%s %c%d,%c%d,%c%d\n",
                                tp_prefix,
                                op,
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num,
                                child2->register_type,
                                child2->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
            }

            type_promotion(node);
            break;

        case OP_AND:
            if (!node->output) node->output = output_f();
            if (node->register_num == child1->register_num &&
                node->register_type == child1->register_type) {

                output_concat(node->output, child1->output);

                /* If child1 and result are the same registers the logic
                 * is slightly shorter
                 *
                 * If result is false - we can just lazily set the result to false
                 * and not bother with the second expression */
                temp1 = mprintf("   brf l%dandend,%c%d\n",
                                node->node_number,
                                child1->register_type,
                                child1->register_num);
                output_append_text(node->output, temp1);
                free(temp1);

                /* Evaluate child2 */
                output_concat(node->output, child2->output);

                /* Result is child2's result */
                if (! (node->register_num == child2->register_num &&
                       node->register_type == child2->register_type) ) {
                    temp1 = mprintf("   icopy %c%d,%c%d\n",
                                    node->register_type,
                                    node->register_num,
                                    child2->register_type,
                                    child2->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }

                /* End of logic */
                /* Result is already set */
                temp1 = mprintf(
                        "l%dandend:\n",
                        node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
            }
            else {

                output_concat(node->output, child1->output);

                /* If child1 and result are not the same registers the logic
                 * is slightly longer
                 *
                 * If result is false - we can just lazily set the result to false
                 * and not bother with the second expression */
                temp1 = mprintf("   brf l%dandfalse,%c%d\n",
                                node->node_number,
                                child1->register_type,
                                child1->register_num);
                output_append_text(node->output, temp1);
                free(temp1);

                /* Evaluate child2 */
                output_concat(node->output, child2->output);

                /* Result is child2's result & branch to end */
                if (node->register_num == child2->register_num &&
                    node->register_type == child2->register_type) {
                    /* No need to copy if the registers are the same */
                    temp1 = mprintf("   br l%dandend\n", node->node_number);
                }
                else {
                    temp1 = mprintf("   icopy %c%d,%c%d\n   br l%dandend\n",
                                    node->register_type,
                                    node->register_num,
                                    child2->register_type,
                                    child2->register_num,
                                    node->node_number);
                }
                output_append_text(node->output, temp1);
                free(temp1);

                /* End of logic */
                /* Result is 0/false */
                temp1 = mprintf(
                        "l%dandfalse:\n   load %c%d,0\nl%dandend:\n",
                        node->node_number,
                        node->register_type,
                        node->register_num,
                        node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
            }
            type_promotion(node);
            break;

        case OP_OR:
            if (!node->output) node->output = output_f();
            if (node->register_num == child1->register_num &&
                node->register_type == child1->register_type) {

                output_concat(node->output, child1->output);

                /* If child1 and result are the same registers the logic
                 * is slightly shorter
                 *
                 * If result is true - we can just lazily set the result to true
                 * and not bother with the second expression */
                temp1 = mprintf("   brt l%dorend,%c%d\n",
                                node->node_number,
                                child1->register_type,
                                child1->register_num);
                output_append_text(node->output, temp1);
                free(temp1);

                /* Evaluate child2 */
                output_concat(node->output, child2->output);

                /* Result is child2's result */
                if (! (node->register_num == child2->register_num &&
                       node->register_type == child2->register_type) ) {
                    temp1 = mprintf("   icopy %c%d,%c%d\n",
                                    node->register_type,
                                    node->register_num,
                                    child2->register_type,
                                    child2->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }

                /* End of logic */
                /* Result is already set */
                temp1 = mprintf(
                        "l%dorend:\n",
                        node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);

            }
            else {

                output_concat(node->output, child1->output);

                /* If child1 and result are not the same registers the logic
                 * is slightly longer
                 *
                 * If result is true - we can just lazily set the result to true
                 * and not bother with the second expression */
                temp1 = mprintf("   brt l%dortrue,%c%d\n",
                                node->node_number,
                                child1->register_type,
                                child1->register_num);
                output_append_text(node->output, temp1);
                free(temp1);

                /* Evaluate child2 */
                output_concat(node->output, child2->output);

                /* Result is child2's result & branch to end */
                if (node->register_num == child2->register_num &&
                    node->register_type == child2->register_type) {
                    /* No need to copy if the registers are the same */
                    temp1 = mprintf("   br l%dorend\n", node->node_number);
                }
                else {
                    temp1 = mprintf("   icopy %c%d,%c%d\n   br l%dorend\n",
                                    node->register_type,
                                    node->register_num,
                                    child2->register_type,
                                    child2->register_num,
                                    node->node_number);
                }
                output_append_text(node->output, temp1);
                free(temp1);

                /* End of logic */
                /* Result is 1/true */
                temp1 = mprintf(
                        "l%dortrue:\n   load %c%d,1\nl%dorend:\n",
                        node->node_number,
                        node->register_type,
                        node->register_num,
                        node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
            }
            type_promotion(node);
            break;

        case OP_NOT:
            if (!node->output) node->output = output_f();
            if (child1->output) output_concat(node->output, child1->output);
            temp1 = mprintf("   not %c%d,%c%d\n",
                            node->register_type,
                            node->register_num,
                            child1->register_type,
                            child1->register_num);
            output_append_text(node->output, temp1);
            free(temp1);
            if (child1->cleanup) output_concat(node->output, child1->cleanup);

            type_promotion(node);
            break;

        case OP_NEG:
            if (!node->output) node->output = output_f();
            if (child1->output) output_concat(node->output, child1->output);
            if (node->value_type == TP_FLOAT) {
                temp1 = mprintf("   fsub %c%d,0.0,%c%d\n",
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num);
            }
            else if (node->value_type == TP_DECIMAL) {
                temp1 = mprintf("   dsub %c%d,0d,%c%d\n",
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num);
            }
            else {
                temp1 = mprintf("   isub %c%d,0,%c%d\n",
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num);
            }
            output_append_text(node->output, temp1);
            free(temp1);
            if (child1->cleanup) output_concat(node->output, child1->cleanup);

            type_promotion(node);
            break;

        case OP_PLUS:
            /* Same as assignment really */
            if (!node->output) node->output = output_f();
            if (child1->output) output_concat(node->output, child1->output);

            /* Check if type conversion is necessary */
            /* TODO */

            if (node->register_type != child1->register_type ||
                node->register_num != child1->register_num) {
                temp1 = mprintf("   %scopy %c%d,%c%d\n",
                                tp_prefix,
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num);
            }
            else {
                temp1 = mprintf("   * \"+\" is a nop here\n");
            }
            output_append_text(node->output, temp1);
            free(temp1);
            if (child1->cleanup) output_concat(node->output, child1->cleanup);

            type_promotion(node);
            break;

        case VAR_REFERENCE:
            break;

        case CONSTANT:
        case CONST_SYMBOL:
        case STRING:
        case FLOAT:
        case DECIMAL:
        case INTEGER:
            /* If register is not set then the parent node will handle this
             * as a constant - we just set the value as a string */
            if (node->register_num != DONT_ASSIGN_REGISTER) {
                /* Get the constant string */
                temp2 = format_constant(node->value_type, node);

                /* Make the register load instruction */
                temp1 = mprintf("   load %c%d,%s\n",
                                node->register_type,
                                node->register_num,
                                temp2);

                /* Set the node output */
                if (node->output) output_append_text(node->output, temp1);
                else node->output = output_fs(temp1);
                free(temp1);
                free(temp2);

                /* Do any type promotion */
                type_promotion(node);
            }
            break;

        case BLOCK_EXPR:
            comment_meta = get_metaline_token_at(node);
            if (node->output) output_prepend_text(comment_meta, node->output);
            else node->output = output_fs(comment_meta);
            free(comment_meta);

            if (child1 && child1->output) output_concat(node->output, child1->output);

            temp1 = mprintf("l%dbexprend:\n", node->node_number);
            output_append_text(node->output, temp1);
            free(temp1);

            type_promotion(node);
            break;

        default:
            break;
    }

}
