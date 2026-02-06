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
 * Code Generator / RXAS Emitter - Register Allocation
 */

#include "rxcpmain.h"
#include "rxcp_emit.h"

/* Tests if a node uses a symbol register */
static int use_symbol_reg(ASTNode* node) {
    if (    node->symbolNode                 // It's a symbol
            && node->symbolNode->symbol->symbol_type != FUNCTION_SYMBOL   // It's not a function
            && node->node_type != OP_ARG_EXISTS // If it's not OP_ARG_EXISTS (if this list becomes long we need a better way ...)
            && !(node->child)           // It's not an array element
            && !(node->symbolNode->symbol->scope &&
                 node->symbolNode->symbol->scope->defining_node &&
                 node->symbolNode->symbol->scope->defining_node->node_type == CLASS_DEF) // It's not an attribute
        ) return 1;
    else return 0;
}

/* This function returns 1 if the node register should not be used by the parent (it should be returned AFTER the
 * parent has finished with it) */
static int defer_reg_return(ASTNode* node) {

    ASTNode* child1 = node->child;
    switch (node->node_type)
    {
        case VAR_SYMBOL:
            if (node->child && node->child->node_type != NOVAL) return 1;
            if (node->symbolNode && node->symbolNode->symbol && node->symbolNode->symbol->scope &&
                node->symbolNode->symbol->scope->defining_node &&
                node->symbolNode->symbol->scope->defining_node->node_type == CLASS_DEF) return 1;
            break;

        case VAR_TARGET:
            if (node->child) return 1;
            break;

        case OP_ARG_VALUE:
            return 1;

        default: ;
    }
    return 0;
}

/* Assign registers */
walker_result register_walker(walker_direction direction,
                                 ASTNode* node,
                                 void *pl) {
    walker_payload *payload = (walker_payload*) pl;
    ASTNode *child1, *child2, *child3, *c;
    int a, i;

    child1 = node->child;
    if (child1) child2 = child1->sibling;
    else child2 = NULL;
    if (child2) child3 = child2->sibling;
    else child3 = NULL;

    if (direction == in) {
        /* IN - TOP DOWN */
        switch (node->node_type) {
            case FACTORY:
            case METHOD:
            case PROCEDURE:
                /* Return Type */
                c = ast_chld(node, CLASS, VOID);
                if (c) c->register_num = DONT_ASSIGN_REGISTER;
                break;

            case ARGS:
                /*
                 * Assign Argument registers to Arguments
                 */
                /* Loop through arguments setting the symbol register */
                c = node->child;
                if (node->parent->node_type == METHOD) a = 2; /* a1 is reserved for "this" */
                else a = 1;
                while (c) {
                    if (c->child->node_type == VAR_TARGET || c->child->node_type == VAR_REFERENCE) {
                        c->register_num = a;
                        c->register_type = 'a';
                        if (c->is_ref_arg || c->is_const_arg) {
                            /* Constant or Pass by reference - no copy so just use the 'a' register */
                            c->child->symbolNode->symbol->register_num = a;
                            c->child->symbolNode->symbol->register_type = 'a';
                        }
                        /* Otherwise, a register will be assigned to the symbol later */
                        a++;
                    }
                    c = c->sibling;
                }
                break;

            case DEFINE:
            case ASSIGN:
                /*
                 * If an assignment from an expression (rather than a symbol) then
                 * mark the register as don't assign (DONT_ASSIGN_REGISTER) so we can assign
                 * it to the target register on the way out (bottom up) and save
                 * a copy instruction
                 */
                if (!use_symbol_reg(child2) || is_constant(child2))
                    child2->register_num = DONT_ASSIGN_REGISTER; /* DONT_ASSIGN_REGISTER Don't assign register */
                break;

            case ARG:
                /*
                 * If there is a default value (not CLASS node) and if it is
                 * from an expression then mark the register as don't assign
                 * (DONT_ASSIGN_REGISTER) so we can assign it to the target
                 * register on the way out (bottom up) and save a copy instruction
                 */
                if ((!use_symbol_reg(child2) || is_constant(child2)))
                    child2->register_num = DONT_ASSIGN_REGISTER; /* DONT_ASSIGN_REGISTER Don't assign register */
                break;

            case FACTORY_CALL:
            case MEMBER_CALL:
            case FUNCTION:
                /* Additional Registers for Arguments - need to be assigned now */
                i = 0;
                if (node->node_type == MEMBER_CALL) {
                    c = child2; /* Skip instance */
                    i = 1;      /* For instance */
                } else {
                    c = child1;
                }
                while (c) {
                    i++;
                    c = c->sibling;
                }
                node->num_additional_registers = i + 1;
                node->additional_registers = get_regs(node->scope, node->num_additional_registers);

                /* The children register need to be assigned */
                if (node->node_type == MEMBER_CALL) {
                    /* Instance */
                    if (!use_symbol_reg(child1) || is_constant(child1))
                        child1->register_num = DONT_ASSIGN_REGISTER;
                    c = child2;
                    i = node->additional_registers + 2; /* First is argc, second is instance */
                } else {
                    c = child1;
                    i = node->additional_registers + 1; /* First one is the number of arguments */
                }

                while (c) {
                    /* DONT_ASSIGN_REGISTER if it makes sense to directly assign
                     * the register later from the call sequence of registers
                     * used in the call instruction
                     * 1. If it is a symbol with call by reference or constant it may be possible to
                     *    assign the symbol to the right register
                     * In this case we try to set the symbol register */
                    if (c->symbolNode && (c->is_ref_arg || c->is_const_arg)) {
                        /* If the register has not been assigned a register set it
                         * to the arguments register - later the node will therefore be giving
                         * this register too */

                        /* NOTE This does nothing as the symbols have always
                         * already been assigned :-( TODO to solve this */

                        if (c->symbolNode->symbol->register_num == UNSET_REGISTER && !(c->symbolNode->symbol->exposed))
                            c->symbolNode->symbol->register_num = i;
                    }

                     /* 2. If it is a non-symbol expression we set the register later */
                    else if (!use_symbol_reg(c) || is_constant(c))
                        c->register_num = DONT_ASSIGN_REGISTER;

                    c = c->sibling;
                    i++;
                }
                break;

            case ADDRESS:
            case SAY:
            case RETURN:
                /*
                 * We do not need a register as we can handle a constant directly
                 */
                if (child1 && is_constant(child1)) child1->register_num = DONT_ASSIGN_REGISTER;
                break;


            case ASSEMBLER:
                /*
                * Constants do not need a register
                */
                if (child1 && (child1->node_type == FUNC_SYMBOL || is_constant(child1))) child1->register_num = DONT_ASSIGN_REGISTER;
                if (child2 && (child2->node_type == FUNC_SYMBOL || is_constant(child2))) child2->register_num = DONT_ASSIGN_REGISTER;
                if (child3 && (child3->node_type == FUNC_SYMBOL || is_constant(child3))) child3->register_num = DONT_ASSIGN_REGISTER;
                break;

                /* The order of the operands of these instructions are not order
                 * specific but the instructions only support operand 3 being a
                 * constant */
            case OP_COMPARE_EQUAL:
            case OP_COMPARE_NEQ:
            case OP_ADD:
            case OP_MULT:
                if (is_constant(child2)) child2->register_num = DONT_ASSIGN_REGISTER;
                else if (is_constant(child1)) {
                    /* We need to swap the two children round because the last one needs
                     * to be the constant */
                    child1->parent->child = child2;
                    child2 = child1;
                    child2->sibling = 0;
                    child1 = child2->parent->child;
                    child1->sibling = child2;

                    child2->register_num = DONT_ASSIGN_REGISTER;
                }
                break;

                /* The order of the operands of these instructions are significant
                 * however the instructions do not support both being a constant */
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
            case OP_MINUS:
            case OP_POWER:
            case OP_DIV:
            case OP_IDIV:
            case OP_MOD:
            case OP_CONCAT:
            case OP_SCONCAT:
                /* one or the other can be a constant - not both */
                if (is_constant(child1)) child1->register_num = DONT_ASSIGN_REGISTER;
                else if (is_constant(child2)) child2->register_num = DONT_ASSIGN_REGISTER;
                break;

            case OP_AND:
            case OP_OR:
                /*  These should not have constants if the optimiser has been run and
                  * anyway the instructions cannot accept constants
                  * But we do want this node and all children to have the
                  * same register if possible to avoid register copies */
                if (!use_symbol_reg(child1)
                && !defer_reg_return(child1))
                    child1->register_num = DONT_ASSIGN_REGISTER;
                if (!use_symbol_reg(child2) && !defer_reg_return(child2)) child2->register_num = DONT_ASSIGN_REGISTER;
                break;

            case VAR_SYMBOL:
            case VAR_TARGET:
                for (c=child1; c; c = c->sibling) {
                    if (is_constant(c)) c->register_num = DONT_ASSIGN_REGISTER; /* Don't assign register */
                }
                break;

            case OP_ARG_VALUE:
            case OP_ARG_IX_EXISTS:
                if (is_constant(child1)) child1->register_num = DONT_ASSIGN_REGISTER; /* Don't assign register */
                break;

            default:
                ;
        }
    }
    else {
        /* OUT - BOTTOM UP */
        switch (node->node_type) {

            /* The order of the operands if these instructions are not order
             * specific but the instructions only support operand 3 being a
             * constant */
            case OP_COMPARE_EQUAL:
            case OP_COMPARE_NEQ:
            case OP_ADD:
            case OP_MULT:

            /* The order of the operands of these instructions are significant
             * however the instructions do not support both being a constant */
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
            case OP_MINUS:
            case OP_POWER:
            case OP_DIV:
            case OP_IDIV:
            case OP_MOD:
            case OP_CONCAT:
            case OP_SCONCAT:

                /* If it is a temporary mark the register for reuse - if the register CAN be resued by this node */
                if (!defer_reg_return(child1) && !use_symbol_reg(child1))
                    ret_reg(node->scope, child1->register_num);
                if (!defer_reg_return(child2) && !use_symbol_reg(child2))
                    ret_reg(node->scope, child2->register_num);

                /* Set result temporary register */
                /* DONT_ASSIGN_REGISTER means that the register number will be set later  but this must be overrider
                 * if we have deferred register actions (unlink) */
                if (node->register_num != DONT_ASSIGN_REGISTER
                    || defer_reg_return(child1)
                    || defer_reg_return(child2))
                        node->register_num = get_reg(node->scope);

                /* If it is a temporary mark the register for reuse - if the register CANNOT be resued by this node */
                if (defer_reg_return(child1) && !use_symbol_reg(child1))
                    ret_reg(node->scope, child1->register_num);
                if (defer_reg_return(child2) && !use_symbol_reg(child2))
                    ret_reg(node->scope, child2->register_num);

                break;

            case OP_AND:
            case OP_OR:
                /* What we try and do here is use the same register for the
                 * node and children to avoid copies */
                if ( (!defer_reg_return(child1) && child1->register_num == DONT_ASSIGN_REGISTER) ||
                     (!defer_reg_return(child2) && child2->register_num == DONT_ASSIGN_REGISTER) ) {
                    /* If we are assigning a register to either children we
                     * will assign to this node and children, overriding/ignoring
                     * any DONT_ASSIGN_REGISTER flag for this node
                     * HOWEVER this is only possible if we have defer_reg_return children */
                    node->register_num = get_reg(node->scope);
                    if (!defer_reg_return(child1) && child1->register_num == DONT_ASSIGN_REGISTER)
                        child1->register_num = node->register_num;
                    if (!defer_reg_return(child1) && child2->register_num == DONT_ASSIGN_REGISTER)
                        child2->register_num = node->register_num;
                }
                else {
                    /* Else both children have a register assigned (and must be symbols)
                     * so just set the node's register */
                    if (node->register_num != DONT_ASSIGN_REGISTER)
                        /* DONT_ASSIGN_REGISTER means that the register number will be set later */
                        node->register_num = get_reg(node->scope);
                }
                break;

            case OP_NOT:
            case OP_NEG:
            case OP_PLUS:
                /* Set result temporary register */
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later */
                    node->register_num = get_reg(node->scope);

                /* If it is a temporary mark the register for reuse */
                if (!use_symbol_reg(child1))
                    ret_reg(node->scope, child1->register_num);
                break;

            case OP_ARG_EXISTS:
                /* This node needs an array for the result but we also have to make sure the symbol has a register */
                /* Set the symbols register */
                if (node->symbolNode->symbol->register_num == UNSET_REGISTER) {
                    if (node->symbolNode->symbol->exposed) {
                        /* Should never happen - as its an arg but in case this changes */
                        node->symbolNode->symbol->register_num = payload->globals++;
                        node->symbolNode->symbol->register_type = 'g';
                    }
                    else node->symbolNode->symbol->register_num = get_reg(node->scope);
                }
                /* Set result temporary register */
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later */
                    node->register_num = get_reg(node->scope);
                break;

            case VAR_SYMBOL:
            case VAR_TARGET:
            case VAR_REFERENCE:
                /* Set the symbols register */
                if (node->symbolNode && node->symbolNode->symbol &&
                    node->symbolNode->symbol->register_num == UNSET_REGISTER) {
                    if (node->symbolNode->symbol->exposed) {
                        node->symbolNode->symbol->register_num = payload->globals++;
                        node->symbolNode->symbol->register_type = 'g';
                    }
                    else if (!(node->symbolNode->symbol->scope &&
                               node->symbolNode->symbol->scope->defining_node &&
                               node->symbolNode->symbol->scope->defining_node->node_type == CLASS_DEF)) {
                        node->symbolNode->symbol->register_num = get_reg(node->scope);
                    }
                }

                /* If we are a define no code is generated so no registers needed */
                if (node->parent->node_type == DEFINE) break;

                if (node->child) {
                    /* If it has a child it is an array element - so we need registers for the node */
                    char unlink_needed = 0;
                    int base = node->symbolNode->symbol->dim_base[ast_chdi(child1)];
                    node->register_num = get_reg(node->scope);

                    /* Do we need a temporary register for making array parameters 1-base or for getting the array size */
                    char needs_extra_reg = 0;
                    c = node->child;
                    while (c && !needs_extra_reg) {

                        if (node->node_type == VAR_SYMBOL && c->node_type == NOVAL) {
                            /* This is the logic for getting the number of elements in an array */
                            /* This is last parameter - we may have done earlier parameters */

                            if (unlink_needed) {
                                /* The register of the attribute is linked so ... */
                                unlink_needed = 0;   /* ... we will unlink it here */
                                if (!(node->symbolNode->symbol->dim_elements[ast_chdi(c)])) {
                                    /* For fixed arrays we just return the upperbound (taking into the base)
                                     * But variable arrays the max array element we need to copy via the additional
                                     * register so we can unlink correctly */
                                    needs_extra_reg = 1;
                                }
                            }
                            /* Should be no more dimensions so we are done */
                            break;
                        }

                        /* This is the logic to get the register attribute for this parameter (child1) */
                        /* Link Array element */
                        if (!(c->node_type == INTEGER || c->node_type == CONSTANT || base == 1)) {
                            /* Need to make it 1 base */
                            needs_extra_reg = 1;
                        }

                        unlink_needed = 1; /* We will need to define a cleanup action to unlink */

                        c = c->sibling;
                    }

                    if (needs_extra_reg) {
                        /* Yes we do need an additional register */
                        node->num_additional_registers = 1;
                        node->additional_registers = get_regs(node->scope, node->num_additional_registers);
                        /* We return it straight away - we only need it for this node */
                        ret_reg(node->scope, node->additional_registers);
                    }

                    /* Release child registers */
                    c = node->child;
                    while (c) {
                        /* release the temporary register */
                        if (!use_symbol_reg(c))
                            ret_reg(node->scope, c->register_num);
                        c = c->sibling;
                    }
                }
                else {
                    /* The node uses the symbol register number */
                    if (node->symbolNode && node->symbolNode->symbol &&
                        node->symbolNode->symbol->scope &&
                        node->symbolNode->symbol->scope->defining_node &&
                        node->symbolNode->symbol->scope->defining_node->node_type == CLASS_DEF) {
                        /* Attribute - needs a temporary register */
                        node->register_num = get_reg(node->scope);
                        node->register_type = 'r';
                    } else if (node->symbolNode && node->symbolNode->symbol) {
                        node->register_num = node->symbolNode->symbol->register_num;
                        node->register_type = node->symbolNode->symbol->register_type;
                    }
                }
                break;

            case OP_ARG_VALUE:
                /* We need a register for the node
                 * Note we ignore DONT_ASSIGN_REGISTER - we always want our own so we can link/unlink it without
                 * any weird side effects */
                node->register_num = get_reg(node->scope);

                /* Release child registers */
                if (!use_symbol_reg(child1)) ret_reg(node->scope, child1->register_num);
                break;

            case OP_ARG_IX_EXISTS:
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later (or is not needed) */
                    node->register_num = get_reg(node->scope);

                /* Release child registers */
                if (!use_symbol_reg(child1)) ret_reg(node->scope, child1->register_num);
                break;

            case NOVAL:
                if (node->parent->node_type == VAR_SYMBOL) {
                    /* If parent is a variable this is a request for the array length
                     * So we need a temporary register to hold the length */
                    node->register_num = get_reg(node->scope);
                }
                break;

            case OP_ARGS:
                /* Set result temporary register */
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later (or is not needed) */
                    node->register_num = get_reg(node->scope);
                break;

            case FLOAT:
            case DECIMAL:
            case INTEGER:
            case STRING:
            case CONSTANT:
            case CONST_SYMBOL:
            case CLASS:
                /* Set result temporary register */
                if (node->parent->node_type != RANGE && node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later (or is not needed) */
                    node->register_num = get_reg(node->scope);
                break;

            case FACTORY_CALL:
            case MEMBER_CALL:
            case FUNCTION:
                /* Set result temporary register */
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later (or is not needed) */
                    node->register_num = get_reg(node->scope);

                /* Assign additional Registers for arguments if assignment was deferred  */
                if (node->node_type == MEMBER_CALL) {
                    if (child1->register_num == DONT_ASSIGN_REGISTER) {
                        child1->register_num = node->additional_registers + 1;
                        child1->register_type = 'r';
                    }
                    c = child2;
                    i = node->additional_registers + 2;
                } else {
                    c = child1;
                    i = node->additional_registers + 1; /* First one is the number of arguments */
                }
                while (c) {
                    if (c->register_num == DONT_ASSIGN_REGISTER) {
                        c->register_num = i;
                        c->register_type = 'r';
                    }
                    i++;
                    c = c->sibling;
                }

                /* Free registers except where it has been given to a symbol */
                ret_reg(node->scope, node->additional_registers); /* First one is the number of arguments */
                if (node->node_type == MEMBER_CALL) {
                    if ( !(use_symbol_reg(child1) &&
                           child1->symbolNode && child1->symbolNode->symbol &&
                           child1->symbolNode->symbol->register_num == node->additional_registers + 1 &&
                           child1->symbolNode->symbol->register_type == 'r') )
                        ret_reg(node->scope, node->additional_registers + 1);
                    c = child2;
                    i = node->additional_registers + 2;
                } else {
                    c = child1;
                    i = node->additional_registers + 1;
                }
                while (c) {
                    /* If it is a symbol with the same register as i don't return the register */
                    if ( !(use_symbol_reg(c) &&
                           c->symbolNode && c->symbolNode->symbol &&
                           c->symbolNode->symbol->register_num == i &&
                           c->symbolNode->symbol->register_type == 'r') )
                        ret_reg(node->scope, i);

                    i++;
                    c = c->sibling;
                }

                break;

            case DEFINE:
            case ASSIGN:
                if (child2->register_num == DONT_ASSIGN_REGISTER) {
                    /* Marked earlier so set the register to the target register */
                    child2->register_num = child1->register_num;
                    child2->register_type = child1->register_type;
                }
                else if (!use_symbol_reg(child2))
                    ret_reg(node->scope, child2->register_num);
                node->register_num = child1->register_num;
                node->register_type = child1->register_type;
                if (!use_symbol_reg(child1))
                    if (node->parent->node_type != REPEAT) ret_reg(node->scope, child1->register_num);
                break;

            case ARG:
                if (child2->register_num == DONT_ASSIGN_REGISTER) {
                    /* Marked earlier so set the register to the target register */
                    child2->register_num = child1->register_num;
                    child2->register_type = child1->register_type;
                }
                else if (!use_symbol_reg(child2))
                    ret_reg(node->scope, child2->register_num);
                break;

            case ADDRESS: // TODO Does Address make it here - I think it is rewritten to a function call earlier
            case SAY:
                node->register_num = child1->register_num;
                node->register_type = child1->register_type;
                /* Return temporary registers */
                if (!use_symbol_reg(child1))
                    ret_reg(node->scope, child1->register_num);
                break;

            case RETURN:
                if (child1) {
                    node->register_num = child1->register_num;
                    node->register_type = child1->register_type;
                    /* If a register is needed at all ... */
                    if (node->register_num != DONT_ASSIGN_REGISTER) {
                        /* Then if it is a temporary mark the register for reuse */
                        if (!use_symbol_reg(child1))
                            ret_reg(node->scope,
                                    child1->register_num);
                    }
                }
                break;

            case IF:
                node->register_num = child1->register_num;
                node->register_type = child1->register_type;
                /* If it is a temporary mark the register for reuse */
                if (!use_symbol_reg(child1))
                    ret_reg(node->scope, child1->register_num);
                break;

            case TO:
                /* Set the register number based on child */
                if (node->child) {
                    node->register_num = node->child->register_num;
                    node->register_type = node->child->register_type;
                }
                /* Additional register for comparison result (was hardcoded r0) */
                node->num_additional_registers = 1;
                node->additional_registers = get_reg(node->scope);
                break;

            case BY:
            case UNTIL:
            case WHILE:
                /* Set the register number based on child */
                if (node->child) {
                    node->register_num = node->child->register_num;
                    node->register_type = node->child->register_type;
                }
                break;

            case FOR:
                if (!use_symbol_reg(node->child)) {
                    /* Not a symbol - use the temp register */
                    node->register_num = node->child->register_num;
                    node->register_type = node->child->register_type;
                }
                /* Else new register for copy */
                else node->register_num = get_reg(node->scope);
                break;

            case REPEAT:
                /* Set the register number to the assignment register number
                 * (if the assignment node exists) */
                c = child1;
                while (c) {
                    if (c->node_type == ASSIGN) {
                        node->register_num = c->register_num;
                        node->register_type = c->register_type;
                        break;
                    }
                    c = c->sibling;
                }
                break;

            case DO:
                /* We need to free temporary registers for the children of the
                 * REPEAT Node at this (the DO) level - because they need to retained
                 * while the do loop is in progress.
                 * Nodes TO/BY/FOR/UNTIL/WHILE or ASSIGN are under REPEAT (child1) */

                c = child1->child; /* The first child under the REPEAT */
                while (c) {
                    if (c->node_type == FOR || (c->child && !use_symbol_reg(c->child))) {
                        ret_reg(node->scope, c->register_num);
                    }
                    if (c->node_type == TO && c->num_additional_registers) {
                        ret_reg(node->scope, c->additional_registers);
                    }
                    c = c->sibling;
                }
                break;

            /* NOTE than ASSEMBLER children should never have temporary registers
             * than need returning as they are either symbols or constants,
             * so we do not have a case ASSEMBLER: */
            default:;
        }
    }

    return result_normal;
}
