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

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "rxcpmain.h"
#include "rxcp_emit.h"
#include "rxcp_val.h"

static int symbol_is_class_attribute(Symbol *symbol) {
    return symbol &&
           symbol->scope &&
           (symbol->scope->type == SCOPE_CLASS ||
            (symbol->scope->defining_node &&
             symbol->scope->defining_node->node_type == CLASS_DEF));
}

static int class_attribute_register_index(Symbol *symbol) {
    int i;

    if (!symbol) return -1;
    for (i = 0; i < (int)sym_nond(symbol); i++) {
        ASTNode *def_node = sym_trnd(symbol, i)->node;
        if (def_node && def_node->parent && def_node->parent->node_type == DEFINE) {
            ASTNode *nr = ast_chld(def_node->parent, NODE_REGISTER, 0);
            if (nr) {
                ASTNode *idx = ast_chld(nr, INTEGER, 0);
                if (idx) return node_to_integer(idx);
                if (nr->int_value) return (int)nr->int_value;
                if (nr->child && nr->child->token) {
                    return (int)strtol(nr->child->token->token_string, NULL, 10);
                }
                if (nr->child && nr->child->node_string && nr->child->node_string_length) {
                    char *buffer = malloc(nr->child->node_string_length + 1);
                    int result;

                    if (!buffer) return -1;
                    memcpy(buffer, nr->child->node_string, nr->child->node_string_length);
                    buffer[nr->child->node_string_length] = 0;
                    result = (int)strtol(buffer, NULL, 10);
                    free(buffer);
                    return result;
                }
            }
        }
    }
    return -1;
}

static ASTNode *class_attribute_register_view(Symbol *symbol) {
    int i;

    if (!symbol) return 0;
    for (i = 0; i < (int)sym_nond(symbol); i++) {
        ASTNode *def_node = sym_trnd(symbol, i)->node;
        ASTNode *nr;
        ASTNode *child;

        if (!def_node || !def_node->parent || def_node->parent->node_type != DEFINE) continue;
        nr = ast_chld(def_node->parent, NODE_REGISTER, 0);
        if (!nr) continue;
        for (child = nr->child; child; child = child->sibling) {
            if (child->node_type == INTEGER || child->node_type == CONSTANT) continue;
            return child;
        }
    }
    return 0;
}

static int class_attribute_is_flag_view(Symbol *symbol) {
    ASTNode *view = class_attribute_register_view(symbol);

    return view &&
           view->node_string &&
           view->node_string_length > 6 &&
           strncasecmp(view->node_string, "flags.", 6) == 0;
}

static int class_attribute_is_complex(Symbol *symbol) {
    int index;
    Symbol **symbols;
    int i;

    if (!symbol_is_class_attribute(symbol)) return 0;
    index = class_attribute_register_index(symbol);
    if (index == 0) return 1;
    if (index < 0 || !symbol->scope) return 0;

    symbols = scp_syms(symbol->scope);
    if (!symbols) return 0;

    for (i = 0; symbols[i]; i++) {
        Symbol *other = symbols[i];

        if (other == symbol) continue;
        if (other->symbol_type != VARIABLE_SYMBOL) continue;
        if (class_attribute_register_index(other) == index) {
            free(symbols);
            return 1;
        }
    }

    free(symbols);
    return 0;
}

static int class_attribute_needs_read_link_register(Symbol *symbol) {
    if (!class_attribute_is_flag_view(symbol)) return class_attribute_is_complex(symbol);
    return class_attribute_register_index(symbol) > 0;
}

/* Tests if a node uses a symbol register */
static int use_symbol_reg(ASTNode* node) {
    if (    node->symbolNode                 // It's a symbol
            && node->symbolNode->symbol->symbol_type != FUNCTION_SYMBOL   // It's not a function
            && node->node_type != OP_ARG_EXISTS // If it's not OP_ARG_EXISTS (if this list becomes long we need a better way ...)
            && !(node->child)           // It's not an array element
            && !symbol_is_class_attribute(node->symbolNode->symbol) // It's not an attribute
        ) return 1;
    else return 0;
}

static int defer_reg_return(ASTNode* node);

static int node_is_block_expr_leave(ASTNode *node) {
    ASTNode *parent;

    if (!node || node->node_type != LEAVE_WITH) return 0;
    parent = node->parent;
    return parent &&
           parent->node_type == INSTRUCTIONS &&
           parent->parent &&
           parent->parent->node_type == BLOCK_EXPR;
}

static int scope_assigns_named_registers(Scope *scope) {
    ASTNode *owner;

    if (!scope || scope->type != SCOPE_LOCAL) return 0;
    owner = scope->defining_node;
    if (!owner) return 0;
    return 1;
}

static int scope_recycles_named_registers(Scope *scope) {
    ASTNode *owner;

    if (!scope_assigns_named_registers(scope)) return 0;
    owner = scope->defining_node;
    if (owner->inherit_parent_reg_scope) return 0;
    return 1;
}

static int symbol_name_starts_with(Symbol *symbol, const char *prefix) {
    size_t len;

    if (!symbol || !symbol->name || !prefix) return 0;
    len = strlen(prefix);
    return strncmp(symbol->name, prefix, len) == 0;
}

static int symbol_has_recyclable_local_storage_type(Symbol *symbol) {
    if (!symbol) return 0;

    switch (symbol->type) {
        case TP_UNKNOWN:
        case TP_BOOLEAN:
        case TP_INTEGER:
        case TP_FLOAT:
        case TP_DECIMAL:
        case TP_STRING:
        case TP_BINARY:
        case TP_OBJECT:
        case TP_REFERENCE:
            return 1;
        default:
            return 0;
    }
}

static int symbol_uses_scoped_register(Symbol *symbol) {
    if (!symbol || symbol->symbol_type != VARIABLE_SYMBOL) return 0;
    if (!scope_recycles_named_registers(symbol->scope)) return 0;
    if (symbol->exposed || symbol->is_arg || symbol->is_ref_arg ||
        symbol->is_this || symbol->is_factory) return 0;
    if (symbol->has_reference_target) return 0;
    if (symbol_name_starts_with(symbol, "__inline")) return 0;
    if (symbol_name_starts_with(symbol, "__rxtrace")) return 0;
    return symbol_has_recyclable_local_storage_type(symbol);
}

static void assign_symbol_registers_worker(Symbol *symbol, void *payload) {
    walker_payload *pl = (walker_payload*)payload;
    if (symbol->symbol_type == VARIABLE_SYMBOL) {
        if (symbol->register_num == UNSET_REGISTER) {
            if (symbol->exposed) {
                symbol->register_num = pl->globals++;
                symbol->register_type = 'g';
            } else if (!(symbol->scope &&
                         symbol->scope->defining_node &&
                         symbol->scope->defining_node->node_type == CLASS_DEF)) {
                if (symbol_uses_scoped_register(symbol)) {
                    symbol->register_num = get_reg(symbol->scope);
                } else {
                    symbol->register_num = get_reg_perm(symbol->scope);
                }
                symbol->register_type = 'r';
            }
        }
    }
}

static void assign_registers_in_scope(Scope *scope, walker_payload *payload) {
    if (!scope) return;
    scp_4all(scope, assign_symbol_registers_worker, payload);
}

static void assign_scoped_registers_for_node(ASTNode *node, walker_payload *payload) {
    if (!node || !scope_assigns_named_registers(node->scope)) return;
    if (node->scope->defining_node != node) return;
    assign_registers_in_scope(node->scope, payload);
}

static int int_compare(const void *left, const void *right) {
    int l = *(const int *)left;
    int r = *(const int *)right;

    if (l < r) return -1;
    if (l > r) return 1;
    return 0;
}

static void release_scoped_registers_for_node(ASTNode *node) {
    Scope *scope;
    Symbol **symbols;
    int *registers;
    size_t count;
    size_t i;

    if (!node) return;
    scope = node->scope;
    if (!scope_recycles_named_registers(scope)) return;
    if (scope->defining_node != node) return;

    symbols = scp_syms(scope);
    if (!symbols) return;

    count = 0;
    for (i = 0; symbols[i]; i++) {
        Symbol *symbol = symbols[i];

        if (!symbol_uses_scoped_register(symbol)) continue;
        if (symbol->register_type != 'r' || symbol->register_num < 0) continue;
        if (node->register_type == symbol->register_type &&
            node->register_num == symbol->register_num) continue;
        count++;
    }

    if (!count) {
        free(symbols);
        return;
    }

    registers = malloc(sizeof(int) * count);
    if (!registers) {
        free(symbols);
        return;
    }

    count = 0;
    for (i = 0; symbols[i]; i++) {
        Symbol *symbol = symbols[i];

        if (!symbol_uses_scoped_register(symbol)) continue;
        if (symbol->register_type != 'r' || symbol->register_num < 0) continue;
        if (node->register_type == symbol->register_type &&
            node->register_num == symbol->register_num) continue;
        registers[count++] = symbol->register_num;
    }

    qsort(registers, count, sizeof(int), int_compare);
    for (i = 0; i < count; i++) {
        if (i && registers[i] == registers[i - 1]) continue;
        ret_reg(scope, registers[i]);
    }

    free(registers);
    free(symbols);
}

/* Returns a child's register to the pool, potentially deferring it if linked */
static void return_child_reg(ASTNode* child) {
    if (!child || child->register_num == DONT_ASSIGN_REGISTER || child->register_num == UNSET_REGISTER) return;
    if (use_symbol_reg(child)) return;
    ret_reg_later(child->scope, child->register_num);
}

static void return_child_reg_after_parent(ASTNode* parent, ASTNode* child) {
    if (!parent || !child) return;
    if (parent->register_num == child->register_num &&
        parent->register_type == child->register_type) {
        return;
    }
    return_child_reg(child);
}

/* Returns a child's register ONLY if it is not deferred */
static void return_child_reg_now(ASTNode* child) {
    if (!child || child->register_num == DONT_ASSIGN_REGISTER || child->register_num == UNSET_REGISTER) return;
    if (use_symbol_reg(child)) return;
    if (!defer_reg_return(child)) {
        ret_reg(child->scope, child->register_num);
    }
}

static void return_additional_regs_later(ASTNode *node) {
    int i;

    if (!node || node->additional_registers < 0 || node->num_additional_registers <= 0) return;
    for (i = 0; i < node->num_additional_registers; i++) {
        ret_reg_later(node->scope, node->additional_registers + i);
    }
}

static int allocate_call_result_reg(ASTNode *node) {
    int reg;
    int range_start;
    int range_end;
    int *skipped;
    size_t skipped_count;

    if (!node) return UNSET_REGISTER;

    range_start = node->additional_registers;
    range_end = node->additional_registers + (int)node->num_additional_registers - 1;
    skipped = NULL;
    skipped_count = 0;

    reg = get_reg(node->scope);
    while (reg >= range_start && reg <= range_end) {
        int *new_skipped;

        new_skipped = realloc(skipped, sizeof(int) * (skipped_count + 1));
        if (!new_skipped) {
            if (skipped) free(skipped);
            return reg;
        }

        skipped = new_skipped;
        skipped[skipped_count++] = reg;
        reg = get_reg(node->scope);
    }

    while (skipped_count) {
        skipped_count--;
        ret_reg(node->scope, skipped[skipped_count]);
    }
    if (skipped) free(skipped);

    return reg;
}

/* This function returns 1 if the node register should not be used by the parent (it should be returned AFTER the
 * parent has finished with it) */
static int defer_reg_return(ASTNode* node) {

    ASTNode* child1 = node->child;
    switch (node->node_type)
    {
        case VAR_SYMBOL:
            if (node->child && node->child->node_type != NOVAL) return 1;
            if (node->symbolNode && symbol_is_class_attribute(node->symbolNode->symbol)) return 1;
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
            case PROGRAM_FILE:
            case IMPORTED_FILE:
                if (node->scope) scp_4all(node->scope, assign_symbol_registers_worker, payload);
                break;

            case FACTORY:
            case MATCH:
            case METHOD:
            case PROCEDURE:
                /* Return Type */
                c = ast_type_child(node);
                if (c) c->register_num = DONT_ASSIGN_REGISTER;

                /* Pre-assign procedure locals; eligible SCOPE_LOCAL blocks allocate on entry. */
                if (node->scope) assign_registers_in_scope(node->scope, payload);

                if (node->node_type == FACTORY) {
                    /* Assign r_this from symbol §factory */
                    ASTNode star_node;
                    memset(&star_node, 0, sizeof(ASTNode));
                    star_node.node_string = "\xc2\xa7" "factory";
                    star_node.node_string_length = 9;
                    Symbol *star_sym = sym_lrsv(node->scope, &star_node);
                    if (star_sym && star_sym->register_num == UNSET_REGISTER) {
                        star_sym->register_num = get_reg_perm(node->scope);
                        star_sym->register_type = 'r';
                    }
                } else if (node->node_type == METHOD) {
                    /* Associate symbol "§this" with a1 */
                    ASTNode this_node;
                    memset(&this_node, 0, sizeof(ASTNode));
                    this_node.node_string = "\xc2\xa7" "this";
                    this_node.node_string_length = 6;
                    Symbol *this_sym = sym_lrsv(node->scope, &this_node);
                    if (this_sym) {
                        this_sym->register_num = 1;
                        this_sym->register_type = 'a';
                    }
                }

                break;

            case DO:
            case SIGNAL_BLOCK:
            case BLOCK_EXPR:
            case INSTRUCTIONS:
                assign_scoped_registers_for_node(node, payload);
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
                            /* `.ref` formals and read-only by-value formals keep
                             * the incoming argument register. Writable by-value
                             * formals must fall through so the emitter can
                             * assign a distinct local register and preserve
                             * caller-visible pass-by-value semantics. */
                            c->child->symbolNode->symbol->register_num = a;
                            c->child->symbolNode->symbol->register_type = 'a';
                        }
                        /* Otherwise, a register will be assigned to the symbol later */
                        a++;
                    }
                    c = c->sibling;
                }
                break;

            case CONSTANT_DEF:
                break;

            case DEFINE:
            case ASSIGN:
                /*
                 * If an assignment from an expression (rather than a symbol) then
                 * mark the register as don't assign (DONT_ASSIGN_REGISTER) so we can assign
                 * it to the target register on the way out (bottom up) and save
                 * a copy instruction
                 */
                if (use_symbol_reg(child1) &&
                    child2->node_type != BLOCK_EXPR &&
                    (!use_symbol_reg(child2) || is_constant(child2)))
                    child2->register_num = DONT_ASSIGN_REGISTER; /* DONT_ASSIGN_REGISTER Don't assign register */
                break;

            case ARG:
                /*
                 * If there is a default value (not CLASS node) and if it is
                 * from an expression then mark the register as don't assign
                 * (DONT_ASSIGN_REGISTER) so we can assign it to the target
                 * register on the way out (bottom up) and save a copy instruction
                 */
                if (child2->node_type != BLOCK_EXPR && (!use_symbol_reg(child2) || is_constant(child2)))
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
                    if (child1->node_type != BLOCK_EXPR &&
                        (!use_symbol_reg(child1) || is_constant(child1)))
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
                    else if (c->node_type != BLOCK_EXPR && (!use_symbol_reg(c) || is_constant(c)))
                        c->register_num = DONT_ASSIGN_REGISTER;

                    c = c->sibling;
                    i++;
                }
                break;

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
            case OP_COMPARE_S_EQ:
            case OP_COMPARE_S_NEQ:
            case OP_ADD:
            case OP_MULT:
            case OP_BIT_AND:
            case OP_BIT_OR:
            case OP_BIT_XOR:
            case OP_FLAG_HAS:
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
                if (node->value_type != TP_BINARY && node->target_type != TP_BINARY) {
                    if (is_constant(child1)) child1->register_num = DONT_ASSIGN_REGISTER;
                    else if (is_constant(child2)) child2->register_num = DONT_ASSIGN_REGISTER;
                }
                break;

            case OP_BIT_SHL:
            case OP_BIT_SHR:
                if (is_constant(child2)) child2->register_num = DONT_ASSIGN_REGISTER;
                break;

            case OP_BIT_NOT:
                if (is_constant(child1)) child1->register_num = DONT_ASSIGN_REGISTER;
                break;

            case OP_AND:
            case OP_OR:
                /*  These should not have constants if the optimiser has been run and
                  * anyway the instructions cannot accept constants
                  * But we do want this node and all children to have the
                  * same register if possible to avoid register copies */
                if (child1->node_type != BLOCK_EXPR &&
                    !use_symbol_reg(child1)
                && !defer_reg_return(child1))
                    child1->register_num = DONT_ASSIGN_REGISTER;
                if (child2->node_type != BLOCK_EXPR &&
                    !use_symbol_reg(child2) &&
                    !defer_reg_return(child2)) child2->register_num = DONT_ASSIGN_REGISTER;
                break;

            case VAR_SYMBOL:
            case VAR_TARGET:
            case VAR_REFERENCE:
                /* On-demand register allocation for symbols */
                if (node->symbolNode && node->symbolNode->symbol) {
                    Symbol *symbol = node->symbolNode->symbol;
                    if (symbol->register_num == UNSET_REGISTER && symbol->register_type != 'a') {
                        /* Check if it's an attribute */
                        if (!symbol_is_class_attribute(symbol)) {
                            if (symbol->exposed) {
                                symbol->register_num = payload->globals++;
                                symbol->register_type = 'g';
                            } else {
                                if (symbol_uses_scoped_register(symbol)) {
                                    symbol->register_num = get_reg(symbol->scope);
                                } else {
                                    symbol->register_num = get_reg_perm(symbol->scope);
                                }
                                symbol->register_type = 'r';
                            }
                        }
                    }
                }

                for (c=child1; c; c = c->sibling) {
                    if (is_constant(c)) c->register_num = DONT_ASSIGN_REGISTER; /* Don't assign register */
                }
                break;

            case OP_ARG_VALUE:
            case OP_ARG_IX_EXISTS:
                if (is_constant(child1)) child1->register_num = DONT_ASSIGN_REGISTER; /* Don't assign register */
                break;

            case OP_TYPE_IS:
            case OP_TYPE_CAST:
                if (child2) child2->register_num = DONT_ASSIGN_REGISTER;
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
            case OP_XOR:
            case OP_ADD:
            case OP_MULT:
            case OP_BIT_AND:
            case OP_BIT_OR:
            case OP_BIT_XOR:
            case OP_BIT_SHL:
            case OP_BIT_SHR:
            case OP_FLAG_HAS:

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
                return_child_reg_now(child1);
                return_child_reg_now(child2);

                /* Set result temporary register */
                /* DONT_ASSIGN_REGISTER means that the register number will be set later  but this must be overrider
                 * if we have deferred register actions (unlink) */
                if (node->register_num != DONT_ASSIGN_REGISTER
                    || defer_reg_return(child1)
                    || defer_reg_return(child2))
                        node->register_num = get_reg(node->scope);

                /* If it is a temporary mark the register for reuse - if the register CANNOT be resued by this node */
                return_child_reg_after_parent(node, child1);
                return_child_reg_after_parent(node, child2);

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
            case OP_BIT_NOT:
            case OP_NEG:
            case OP_PLUS:
            case OP_REFERENCE:
            case OP_DEREFERENCE:
            case OP_SNAPSHOT:
            case OP_REFVALID:
            case OP_INITIALIZED:
                /* Set result temporary register */
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later */
                    node->register_num = get_reg(node->scope);

                /* If it is a temporary mark the register for reuse */
                return_child_reg_after_parent(node, child1);
                break;

            case OP_ARG_EXISTS:
                /* This node needs an array for the result but we also have to make sure the symbol has a register */
                /* Symbol register should have been assigned already by PROCEDURE entry */
                /* Set result temporary register */
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later */
                    node->register_num = get_reg(node->scope);
                break;

            case VAR_SYMBOL:
            case VAR_TARGET:
            case VAR_REFERENCE:
                /* Symbol register should have been assigned already by PROCEDURE entry */

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

                    char needs_prop_reg = 0;
                    if (node->symbolNode && symbol_is_class_attribute(node->symbolNode->symbol)) {
                        needs_prop_reg = class_attribute_is_complex(node->symbolNode->symbol) ? 2 : 1;
                    }

                    if (needs_extra_reg || needs_prop_reg) {
                        /* Yes we do need an additional register */
                        node->num_additional_registers = needs_extra_reg + needs_prop_reg;
                        node->additional_registers = get_regs(node->scope, node->num_additional_registers);
                        /* Array/property helpers may stay linked until the parent emits cleanup. */
                        return_additional_regs_later(node);
                    }

                    /* Release child registers */
                    c = node->child;
                    while (c) {
                        /* release the temporary register */
                        return_child_reg(c);
                        c = c->sibling;
                    }
                }
                else {
                    /* The node uses the symbol register number */
                    if (node->symbolNode && symbol_is_class_attribute(node->symbolNode->symbol)) {
                        /* Attribute - needs a temporary register */
                        node->register_num = get_reg(node->scope);
                        node->register_type = 'r';
                        if (class_attribute_needs_read_link_register(node->symbolNode->symbol)) {
                            node->num_additional_registers = 1;
                            node->additional_registers = get_reg(node->scope);
                            ret_reg(node->scope, node->additional_registers);
                        }
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
                return_child_reg(child1);
                break;

            case OP_ARG_IX_EXISTS:
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later (or is not needed) */
                    node->register_num = get_reg(node->scope);

                /* Release child registers */
                return_child_reg(child1);
                break;

            case OP_TYPE_IS:
            case OP_TYPEOF:
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    node->register_num = get_reg(node->scope);
                return_child_reg_after_parent(node, child1);
                break;

            case OP_TYPE_CAST:
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    node->register_num = get_reg(node->scope);
                return_child_reg_after_parent(node, child1);
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
            case BINARY:
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
                    node->register_num = allocate_call_result_reg(node);

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

                    if (c->register_num != i) {
                        return_child_reg(c);
                    }

                    i++;
                    c = c->sibling;
                }

                break;

            case CONSTANT_DEF:
                break;

            case DEFINE:
            case ASSIGN: {
                int propagated = 0;
                if (child2->register_num == DONT_ASSIGN_REGISTER) {
                    /* Move the RHS temporary register to the target symbol register */
                    child2->register_num = child1->register_num;
                    child2->register_type = child1->register_type;
                    propagated = 1;
                }
                else {
                    return_child_reg(child2);
                }
                node->register_num = child1->register_num;
                node->register_type = child1->register_type;
                /* IMPORTANT: if we propagated the RHS temp to the LHS symbol, do NOT free it */
                if (!propagated) {
                    if (node->parent->node_type != REPEAT) return_child_reg(child1);
                }
                break;
            }

            case ARG:
                if (child2->register_num == DONT_ASSIGN_REGISTER) {
                    /* Marked earlier so set the register to the target register */
                    child2->register_num = child1->register_num;
                    child2->register_type = child1->register_type;
                }
                else {
                    return_child_reg(child2);
                }
                break;

            case SAY:
                node->register_num = child1->register_num;
                node->register_type = child1->register_type;
                /* Return temporary registers */
                return_child_reg(child1);
                break;

            case RETURN:
                if (child1) {
                    node->register_num = child1->register_num;
                    node->register_type = child1->register_type;
                    /* If a register is needed at all ... */
                    if (node->register_num != DONT_ASSIGN_REGISTER) {
                        /* Then if it is a temporary mark the register for reuse */
                        return_child_reg(child1);
                    }
                }
                break;

            case BLOCK_EXPR:
                node->register_num = get_reg(node->scope);
                node->register_type = 'r';
                break;

            case IF:
                node->register_num = child1->register_num;
                node->register_type = child1->register_type;
                /* If it is a temporary mark the register for reuse */
                return_child_reg(child1);
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

        /* If this is a statement level node, return all deferred registers */
        if (node->parent && node->parent->node_type == INSTRUCTIONS &&
            !node_is_block_expr_leave(node)) {
            ret_reg_all_deferred(node->scope);
        }

        release_scoped_registers_for_node(node);
    }

    return result_normal;
}
