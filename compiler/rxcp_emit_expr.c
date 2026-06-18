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

static Symbol *dereference_assignment_target(ASTNode *node) {
    ASTNode *assign;
    ASTNode *target;

    if (!node || !node->parent || node->parent->node_type != ASSIGN) return 0;
    assign = node->parent;
    target = ast_chdn(assign, 0);
    if (!target || !target->symbolNode) return 0;
    return target->symbolNode->symbol;
}

static int is_interface_member_call(ASTNode *node) {
    Symbol *fsym;
    SymbolNode *defsn;
    ASTNode *defnode;

    if (!node || node->node_type != MEMBER_CALL || !node->symbolNode || !node->symbolNode->symbol) return 0;

    fsym = node->symbolNode->symbol;
    if (sym_nond(fsym) == 0) return 0;

    defsn = sym_trnd(fsym, 0);
    defnode = defsn ? defsn->node : 0;

    return defnode &&
           defnode->node_type == METHOD &&
           defnode->parent &&
           defnode->parent->node_type == INTERFACE_DEF;
}

static int is_interface_factory_call(ASTNode *node) {
    Symbol *fsym;
    SymbolNode *defsn;
    ASTNode *defnode;

    if (!node || node->node_type != FACTORY_CALL || !node->symbolNode || !node->symbolNode->symbol) return 0;

    fsym = node->symbolNode->symbol;
    if (sym_nond(fsym) == 0) return 0;

    defsn = sym_trnd(fsym, 0);
    defnode = defsn ? defsn->node : 0;

    return defnode &&
           defnode->node_type == FACTORY &&
           defnode->parent &&
           defnode->parent->node_type == INTERFACE_DEF;
}

static char *build_interface_factory_selector(ASTNode *node) {
    char *iface_name = 0;
    char *selector = 0;

    if (!node || !node->symbolNode || !node->symbolNode->symbol || !node->symbolNode->symbol->scope) return 0;

    iface_name = scp_frnm(node->symbolNode->symbol->scope);
    if (!iface_name) return 0;

    if (node->association && node->association->node_string && node->association->node_string_length) {
        selector = mprintf("%s..%.*s",
                           iface_name,
                           (int) node->association->node_string_length,
                           node->association->node_string);
    } else {
        selector = strdup(iface_name);
    }

    free(iface_name);
    return selector;
}

static char *build_source_type_name(ValueType type, const char *internal_class_name) {
    if (type == TP_OBJECT && internal_class_name) {
        return rxcp_internal_name_to_source_qualified(internal_class_name, 1);
    }

    return strdup(type_nm(type));
}

static int is_builtin_object_contract_name(const char *name) {
    static const char object_name[] = "object";
    size_t i;
    size_t start;

    if (!name) return 0;
    start = name[0] == '.' ? 1 : 0;
    if (strlen(name + start) != sizeof(object_name) - 1) return 0;
    for (i = 0; i < sizeof(object_name) - 1; i++) {
        if (tolower((unsigned char) name[start + i]) != object_name[i]) return 0;
    }
    return 1;
}

static int semantic_context_is_sugar_get(ASTSemanticContextKind kind) {
    return kind == AST_SEMANTIC_CONTEXT_PROPERTY_GET ||
           kind == AST_SEMANTIC_CONTEXT_INDEX_GET;
}

static int semantic_context_is_sugar_set(ASTSemanticContextKind kind) {
    return kind == AST_SEMANTIC_CONTEXT_PROPERTY_SET ||
           kind == AST_SEMANTIC_CONTEXT_INDEX_SET;
}

static int semantic_context_is_sugar_access(ASTSemanticContextKind kind) {
    return semantic_context_is_sugar_get(kind) ||
           semantic_context_is_sugar_set(kind);
}

static int semantic_context_is_internal_operand(ASTSemanticContextKind kind) {
    return kind == AST_SEMANTIC_CONTEXT_INTERNAL_OPERAND;
}

static void append_semantic_compound_trace_event(OutputFragment *output,
                                                 ASTNode *receiver_node,
                                                 ASTNode *tail_node) {
    char *symbol_name;

    if (!output || !tail_node || tail_node->register_num < 0) return;
    symbol_name = trace_symbol_name_malloc(receiver_node);
    output_append_trace_event_register(output,
                                       RXBIN_TRACE_KIND_COMPOUND,
                                       RXBIN_TRACE_MODE_I,
                                       tail_node,
                                       0,
                                       0,
                                       symbol_name,
                                       "");
    if (symbol_name) free(symbol_name);
}

static void append_semantic_access_value_trace_event(OutputFragment *output,
                                                     ASTSemanticContextKind kind,
                                                     ASTNode *receiver_node,
                                                     ASTNode *result_node,
                                                     ASTNode *value_node) {
    char *symbol_name;

    if (!output) return;
    symbol_name = trace_symbol_name_malloc(receiver_node);
    if (semantic_context_is_sugar_set(kind)) {
        output_append_trace_event_register(output,
                                           RXBIN_TRACE_KIND_ASSIGNMENT,
                                           RXBIN_TRACE_MODE_R | RXBIN_TRACE_MODE_I,
                                           value_node,
                                           0,
                                           0,
                                           symbol_name,
                                           "");
    } else if (semantic_context_is_sugar_get(kind)) {
        output_append_trace_event_register(output,
                                           RXBIN_TRACE_KIND_VARIABLE,
                                           RXBIN_TRACE_MODE_R | RXBIN_TRACE_MODE_I,
                                           result_node,
                                           0,
                                           0,
                                           symbol_name,
                                           "");
    }
    if (symbol_name) free(symbol_name);
}

static void append_semantic_operation_trace_event(OutputFragment *output,
                                                  int trace_kind,
                                                  ASTSemanticContextKind semantic_kind,
                                                  ASTNode *node) {
    if (semantic_context_is_internal_operand(semantic_kind)) return;
    output_append_trace_event_register(output,
                                       trace_kind,
                                       RXBIN_TRACE_MODE_I,
                                       node,
                                       0,
                                       0,
                                       "",
                                       "");
}

static ValueType operand_type_from_prefix(char *tp_prefix, ASTNode *node) {
    if (tp_prefix) {
        switch (*tp_prefix) {
            case 's':
                return TP_STRING;
            case 'f':
                return TP_FLOAT;
            case 'd':
                return TP_DECIMAL;
            case 'i':
                return TP_INTEGER;
            default:
                break;
        }
    }

    return node ? node->target_type : TP_UNKNOWN;
}

static char *resolve_object_contract_name(ASTNode *type_node) {
    Symbol *symbol;

    if (!type_node) return 0;
    symbol = type_node->symbolNode ? type_node->symbolNode->symbol : 0;
    if (!symbol && type_node->context && type_node->context->ast) {
        symbol = sym_rvfc(type_node->context->ast, type_node);
    }
    if (symbol) {
        return sym_frnm(symbol);
    }
    if (type_node->target_class) {
        return strdup(type_node->target_class);
    }
    return 0;
}

void emit_expression(ASTNode *node, void *payload) {
    walker_payload *wp = (walker_payload*)payload;
    char *op = 0;
    char *tp_prefix = type_to_prefix(node->value_type);
    char *temp1;
    char *temp2;
    char *comment_meta;
    ASTNode *n;
    int i, j, k;
    int loose_string_compare = 0;
    char ret_type;
    int ret_num;
    ASTNode *child1 = node->child;
    ASTNode *child2 = node->child ? node->child->sibling : 0;
    ASTNode *child3 = node->child && node->child->sibling ? node->child->sibling->sibling : 0;
    ASTSemanticContextKind semantic_kind = ast_semantic_context_kind(node);

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

            /* First Step through the arguments evaluating any expressions
             * This must be done BEFORE argument marshalling using swaps   */
            n = child1;
            while (n) {
                if (n->output) output_concat(node->output, n->output);
                if (node->node_type == MEMBER_CALL &&
                    semantic_context_is_sugar_access(semantic_kind) &&
                    n == child2) {
                    append_semantic_compound_trace_event(node->output, child1, child2);
                }
                n = n->sibling;
            }

            /* Number of arguments. Keep this after argument expression
             * evaluation: inlined argument blocks may use temporary
             * registers from the call frame before marshalling starts. */
            temp1 = mprintf("   load r%d,%d\n",
                            node->additional_registers,
                            node->num_additional_registers - 1);
            output_append_text(node->output, temp1);
            free(temp1);

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
                    (n->value_dims || n->target_type == TP_STRING || n->target_type == TP_OBJECT ||
                     n->target_type == TP_BINARY || n->target_type == TP_REFERENCE)) {
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
            if (is_interface_member_call(node)) {
                Symbol *fsym = node->symbolNode->symbol;

                temp1 = mprintf("   srcmethod %c%d,r%d,\"%s\"\n",
                                ret_type, ret_num,
                                node->additional_registers + 1,
                                fsym->name);
                output_append_text(node->output, temp1);
                free(temp1);

                temp1 = mprintf("   dcall %c%d,%c%d,r%d\n",
                                ret_type, ret_num,
                                ret_type, ret_num,
                                node->additional_registers);
            }
            else if (is_interface_factory_call(node)) {
                char *selector = build_interface_factory_selector(node);

                temp1 = mprintf("   srcfproc %c%d,\"%s\",r%d\n",
                                ret_type, ret_num,
                                selector ? selector : "",
                                node->additional_registers);
                output_append_text(node->output, temp1);
                free(temp1);
                if (selector) free(selector);

                temp1 = mprintf("   dcall %c%d,%c%d,r%d\n",
                                ret_type, ret_num,
                                ret_type, ret_num,
                                node->additional_registers);
            }
            else if (node->symbolNode) {
                char *call_name;
                int use_mangled = 0;
                Symbol *fsym = node->symbolNode->symbol;
                if (fsym && sym_nond(fsym) > 0) {
                    SymbolNode *defsn = sym_trnd(fsym, 0);
                    if (defsn && defsn->node &&
                        (defsn->node->node_type == METHOD ||
                         defsn->node->node_type == FACTORY ||
                         defsn->node->node_type == MATCH)) {
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
                        if (node->node_string && rxcp_source_symbol_is_qualified(node->node_string, node->node_string_length)) {
                            call_name = strdup(node->symbolNode->symbol->name);
                        } else if (node->node_string) {
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
	            if (semantic_context_is_sugar_access(semantic_kind)) {
	                append_semantic_access_value_trace_event(node->output,
	                                                         semantic_kind,
	                                                         child1,
	                                                         node,
	                                                         child3);
	            } else {
	                char *symbol_name = trace_symbol_name_malloc(node);
	                output_append_trace_event_register(node->output,
	                                                   RXBIN_TRACE_KIND_FUNCTION,
	                                                   RXBIN_TRACE_MODE_I,
	                                                   node,
	                                                   0,
	                                                   0,
	                                                   symbol_name,
	                                                   "");
	                if (symbol_name) free(symbol_name);
	            }
	            break;

        case OP_CONCAT:
            op="concat";
        case OP_SCONCAT:
            if (!op) op="sconcat";
            if (!node->output) node->output = output_f();
            if (node->value_type == TP_BINARY || node->target_type == TP_BINARY) {
                if (child1->output) output_concat(node->output, child1->output);
                if (child2->output) output_concat(node->output, child2->output);
                temp1 = mprintf("   bconcat %c%d,%c%d,%c%d\n",
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
	            type_promotion(node);
	            append_semantic_operation_trace_event(node->output,
	                                                  RXBIN_TRACE_KIND_BINARY_OP,
	                                                  semantic_kind,
	                                                  node);
	            break;
            }
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
	            append_semantic_operation_trace_event(node->output,
	                                                  RXBIN_TRACE_KIND_BINARY_OP,
	                                                  semantic_kind,
	                                                  node);
	            break;

        case OP_XOR:
            if (!node->output) node->output = output_f();
            if (child1->output) output_concat(node->output, child1->output);
            if (child2->output) output_concat(node->output, child2->output);

            temp1 = mprintf(
                    "   brf l%dxorleftfalse,%c%d\n"
                    "   brf l%dxortrue,%c%d\n"
                    "   load %c%d,0\n"
                    "   br l%dxorend\n"
                    "l%dxorleftfalse:\n"
                    "   brt l%dxortrue,%c%d\n"
                    "   load %c%d,0\n"
                    "   br l%dxorend\n"
                    "l%dxortrue:\n"
                    "   load %c%d,1\n"
                    "l%dxorend:\n",
                    node->node_number,
                    child1->register_type,
                    child1->register_num,
                    node->node_number,
                    child2->register_type,
                    child2->register_num,
                    node->register_type,
                    node->register_num,
                    node->node_number,
                    node->node_number,
                    node->node_number,
                    child2->register_type,
                    child2->register_num,
                    node->register_type,
                    node->register_num,
                    node->node_number,
                    node->node_number,
                    node->register_type,
                    node->register_num,
                    node->node_number);
            output_append_text(node->output, temp1);
            free(temp1);
            if (child2->cleanup) output_concat(node->output, child2->cleanup);
            if (child1->cleanup) output_concat(node->output, child1->cleanup);
	            type_promotion(node);
	            append_semantic_operation_trace_event(node->output,
	                                                  RXBIN_TRACE_KIND_BINARY_OP,
	                                                  semantic_kind,
	                                                  node);
	            break;

        /* These operators have a prefix type of that of the first child */
        case OP_COMPARE_EQUAL:
            loose_string_compare = 1;
            if (!op) op="eq";
        case OP_COMPARE_NEQ:
            loose_string_compare = 1;
            if (!op) op="ne";
        case OP_COMPARE_GT:
            loose_string_compare = 1;
            if (!op) op="gt";
        case OP_COMPARE_LT:
            loose_string_compare = 1;
            if (!op) op="lt";
        case OP_COMPARE_GTE:
            loose_string_compare = 1;
            if (!op) op="gte";
        case OP_COMPARE_LTE:
            loose_string_compare = 1;
            if (!op) op="lte";
        case OP_COMPARE_S_EQ:
            if (!op) op="eq";
        case OP_COMPARE_S_NEQ:
            if (!op) op="ne";
        case OP_COMPARE_S_GT:
            if (!op) op="gt";
        case OP_COMPARE_S_LT:
            if (!op) op="lt";
        case OP_COMPARE_S_GTE:
            if (!op) op="gte";
        case OP_COMPARE_S_LTE:
            if (!op) op="lte";

            tp_prefix = type_to_prefix(child1->target_type);
            if (loose_string_compare && child1->target_type == TP_STRING) tp_prefix = "r";

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
                temp2 = format_constant(operand_type_from_prefix(tp_prefix, child1), child1);
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
                temp2 = format_constant(operand_type_from_prefix(tp_prefix, child2), child2);
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
	            append_semantic_operation_trace_event(node->output,
	                                                  RXBIN_TRACE_KIND_BINARY_OP,
	                                                  semantic_kind,
	                                                  node);
	            break;

        case OP_BIT_AND:
            op = "iand";
        case OP_BIT_OR:
            if (!op) op = "ior";
        case OP_BIT_XOR:
            if (!op) op = "ixor";
        case OP_BIT_SHL:
            if (!op) op = "ishl";
        case OP_BIT_SHR:
            if (!op) op = "ishr";
        case OP_FLAG_HAS:
            if (!op) op = "iand";

            if (!node->output) node->output = output_f();

            if (child1->register_num == DONT_ASSIGN_REGISTER) {
                if (child2->output) output_concat(node->output, child2->output);
                temp2 = format_constant(TP_INTEGER, child1);
                temp1 = mprintf("   %s %c%d,%c%d,%s\n",
                                op,
                                node->register_type,
                                node->register_num,
                                child2->register_type,
                                child2->register_num,
                                temp2);
                output_append_text(node->output, temp1);
                free(temp1);
                free(temp2);
                if (child2->cleanup) output_concat(node->output, child2->cleanup);
            }
            else if (child2->register_num == DONT_ASSIGN_REGISTER) {
                if (child1->output) output_concat(node->output, child1->output);
                temp2 = format_constant(TP_INTEGER, child2);
                temp1 = mprintf("   %s %c%d,%c%d,%s\n",
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

            if (node->node_type == OP_FLAG_HAS) {
                temp1 = mprintf("   ine %c%d,%c%d,0\n",
                                node->register_type,
                                node->register_num,
                                node->register_type,
                                node->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
            }

            type_promotion(node);
            append_semantic_operation_trace_event(node->output,
                                                  RXBIN_TRACE_KIND_BINARY_OP,
                                                  semantic_kind,
                                                  node);
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
	            append_semantic_operation_trace_event(node->output,
	                                                  RXBIN_TRACE_KIND_BINARY_OP,
	                                                  semantic_kind,
	                                                  node);
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
            append_semantic_operation_trace_event(node->output,
                                                  RXBIN_TRACE_KIND_BINARY_OP,
                                                  semantic_kind,
                                                  node);
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
	            append_semantic_operation_trace_event(node->output,
	                                                  RXBIN_TRACE_KIND_PREFIX_OP,
	                                                  semantic_kind,
	                                                  node);
	            break;

        case OP_BIT_NOT:
            if (!node->output) node->output = output_f();
            if (child1->register_num == DONT_ASSIGN_REGISTER) {
                temp2 = format_constant(TP_INTEGER, child1);
                temp1 = mprintf("   inot %c%d,%s\n",
                                node->register_type,
                                node->register_num,
                                temp2);
                output_append_text(node->output, temp1);
                free(temp1);
                free(temp2);
            }
            else {
                if (child1->output) output_concat(node->output, child1->output);
                temp1 = mprintf("   inot %c%d,%c%d\n",
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
                if (child1->cleanup) output_concat(node->output, child1->cleanup);
            }

            type_promotion(node);
            append_semantic_operation_trace_event(node->output,
                                                  RXBIN_TRACE_KIND_PREFIX_OP,
                                                  semantic_kind,
                                                  node);
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
	            append_semantic_operation_trace_event(node->output,
	                                                  RXBIN_TRACE_KIND_PREFIX_OP,
	                                                  semantic_kind,
	                                                  node);
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
            append_semantic_operation_trace_event(node->output,
                                                  RXBIN_TRACE_KIND_PREFIX_OP,
                                                  semantic_kind,
                                                  node);
            break;

        case OP_REFERENCE:
            if (!node->output) node->output = output_f();
            if (child1->output) output_concat(node->output, child1->output);
            temp1 = mprintf("   mkref %c%d,%c%d\n",
                            node->register_type,
                            node->register_num,
                            child1->register_type,
                            child1->register_num);
            output_append_text(node->output, temp1);
            free(temp1);
            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            type_promotion(node);
            break;

        case OP_DEREFERENCE:
            if (!node->output) node->output = output_f();
            if (child1->output) output_concat(node->output, child1->output);
            temp1 = mprintf("   unlink %c%d\n"
                            "   linkref %c%d,%c%d\n",
                            node->register_type,
                            node->register_num,
                            node->register_type,
                            node->register_num,
                            child1->register_type,
                            child1->register_num);
            output_append_text(node->output, temp1);
            free(temp1);
            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            scp_add_dereference_symbol(node->scope, dereference_assignment_target(node));
            type_promotion(node);
            break;

        case OP_SNAPSHOT:
            if (!node->output) node->output = output_f();
            if (child1->output) output_concat(node->output, child1->output);
            temp1 = mprintf("   deref %c%d,%c%d\n",
                            node->register_type,
                            node->register_num,
                            child1->register_type,
                            child1->register_num);
            output_append_text(node->output, temp1);
            free(temp1);
            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            type_promotion(node);
            break;

        case OP_REFVALID:
            if (!node->output) node->output = output_f();
            if (child1->output) output_concat(node->output, child1->output);
            temp1 = mprintf("   refvalid %c%d,%c%d\n",
                            node->register_type,
                            node->register_num,
                            child1->register_type,
                            child1->register_num);
            output_append_text(node->output, temp1);
            free(temp1);
            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            type_promotion(node);
            break;

        case OP_INITIALIZED:
            if (!node->output) node->output = output_f();
            if (child1->output) output_concat(node->output, child1->output);
            temp1 = mprintf("   isinitialized %c%d,%c%d\n",
                            node->register_type,
                            node->register_num,
                            child1->register_type,
                            child1->register_num);
            output_append_text(node->output, temp1);
            free(temp1);
            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            type_promotion(node);
            break;

        case OP_TYPE_CAST:
            temp2 = 0;
            if (!node->output) node->output = output_f();
            if (child1->output) output_concat(node->output, child1->output);

            if (node->register_type != child1->register_type ||
                node->register_num != child1->register_num) {
                char *child_prefix = type_to_prefix(child1->value_type);
                temp1 = mprintf("   %scopy %c%d,%c%d\n",
                                child_prefix,
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
            }

            if (node->target_type == TP_OBJECT &&
                node->target_class &&
                !is_builtin_object_contract_name(node->target_class)) {
                temp2 = resolve_object_contract_name(child2);
                if (!temp2) temp2 = strdup(node->target_class);
                temp1 = mprintf("   asserttype %c%d,\"%s\"\n",
                                node->register_type,
                                node->register_num,
                                temp2 ? temp2 : node->target_class);
                output_append_text(node->output, temp1);
                free(temp1);
                if (temp2) free(temp2);
            } else if (node->target_type != TP_OBJECT) {
                const char *promotion = emit_promotion[child1->value_type][node->target_type];
                if (promotion) {
                    temp1 = mprintf("   %s %c%d\n",
                                    promotion,
                                    node->register_type,
                                    node->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
            }

            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            break;

        case OP_TYPE_IS:
            temp2 = 0;
            if (!node->output) node->output = output_f();
            if (child1->output) output_concat(node->output, child1->output);

            if (child2 &&
                child2->target_type == TP_OBJECT &&
                child2->target_class &&
                !is_builtin_object_contract_name(child2->target_class) &&
                child1 &&
                child1->value_type == TP_OBJECT) {
                temp2 = resolve_object_contract_name(child2);
                if (!temp2) temp2 = strdup(child2->target_class);
                temp1 = mprintf("   istype %c%d,%c%d,\"%s\"\n",
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num,
                                temp2 ? temp2 : child2->target_class);
                output_append_text(node->output, temp1);
                free(temp1);
                if (temp2) free(temp2);
            } else {
                int matches = 0;

                if (child2 && child2->target_type == TP_OBJECT) {
                    matches = child1 &&
                              child1->value_type == TP_OBJECT &&
                              (!child2->target_class ||
                               is_builtin_object_contract_name(child2->target_class));
                } else if (child2 && child1) {
                    matches = child1->value_type == child2->target_type;
                }

                temp1 = mprintf("   load %c%d,%d\n",
                                node->register_type,
                                node->register_num,
                                matches);
                output_append_text(node->output, temp1);
                free(temp1);
            }

            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            type_promotion(node);
            break;

        case OP_TYPEOF:
            if (!node->output) node->output = output_f();
            if (child1->output) output_concat(node->output, child1->output);

            if (child1 && child1->value_type == TP_OBJECT) {
                temp1 = mprintf("   typeof %c%d,%c%d\n",
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
            } else {
                if (child1 && child1->value_type == TP_REFERENCE) temp2 = ast_n2tp(child1);
                else temp2 = build_source_type_name(child1 ? child1->value_type : TP_UNKNOWN, 0);
                if (!temp2) break;
                temp1 = mprintf("   load %c%d,\"%s\"\n",
                                node->register_type,
                                node->register_num,
                                temp2);
                output_append_text(node->output, temp1);
                free(temp1);
                free(temp2);
            }

            if (child1->cleanup) output_concat(node->output, child1->cleanup);
            type_promotion(node);
            break;

        case VAR_REFERENCE:
            break;

        case CONSTANT:
        case CONST_SYMBOL:
        case STRING:
        case BINARY:
        case FLOAT:
        case DECIMAL:
        case INTEGER:
            /* If register is not set then the parent node will handle this
             * as a constant - we just set the value as a string */
            if (node->register_num != DONT_ASSIGN_REGISTER) {
                ValueType load_type = node->value_type;
                int skip_promotion = 0;
                if (node->target_type == TP_BINARY &&
                    (node->value_type == TP_STRING || node->value_type == TP_BINARY)) {
                    load_type = TP_BINARY;
                    if (node->value_type == TP_STRING) skip_promotion = 1;
                }

                /* Get the constant string */
                temp2 = format_constant(load_type, node);

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
	                if (!skip_promotion) type_promotion(node);
	                if (ast_semantic_context_kind(node) != AST_SEMANTIC_CONTEXT_INTERNAL_OPERAND) {
	                    output_append_trace_event_register(node->output,
	                                                       RXBIN_TRACE_KIND_LITERAL,
	                                                       RXBIN_TRACE_MODE_I,
	                                                       node,
	                                                       0,
	                                                       0,
	                                                       "",
	                                                       "");
	                }
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

            if (node->scope && node->scope->defining_node == node) {
                size_t i;
                for (i = scp_dereference_symbol_count(node->scope); i > 0; i--) {
                    Symbol *symbol = scp_dereference_symbol_at(node->scope, i - 1);
                    if (!symbol || symbol->register_num < 0 || symbol->register_type != 'r') continue;
                    temp1 = mprintf("   unlink %c%d\n", symbol->register_type, symbol->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
            }

            type_promotion(node);
            break;

        default:
            break;
    }

}
