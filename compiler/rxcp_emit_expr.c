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
#include "rxsignature.h"

static int emit_is_rxinteger_min_magnitude_literal(ASTNode *node) {
    static const char magnitude[] = "9223372036854775808";
    Token *token;

    if (!node) return 0;
    token = node->token;
    return token &&
           token->length == sizeof(magnitude) - 1 &&
           memcmp(token->token_string, magnitude, sizeof(magnitude) - 1) == 0;
}

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

static char *build_dynamic_callable_descriptor_with_return(Symbol *fsym,
                                                           const char *lookup_name,
                                                           const char *return_type_override) {
    SymbolNode *defsn;
    ASTNode *defnode;
    char *rtype;
    char *args;
    char *descriptor;

    if (!fsym || !lookup_name || sym_nond(fsym) == 0) return 0;

    defsn = sym_trnd(fsym, 0);
    defnode = defsn ? defsn->node : 0;
    if (!defnode) return 0;

    rtype = return_type_override ? strdup(return_type_override) : callable_effective_return_type(defnode);
    args = meta_narg(ast_chld(defnode, ARGS, 0));
    descriptor = rx_sig_build_descriptor(lookup_name, rtype, args);
    if (rtype) free(rtype);
    if (args) free(args);
    return descriptor;
}

static char *build_dynamic_callable_descriptor(Symbol *fsym, const char *lookup_name) {
    return build_dynamic_callable_descriptor_with_return(fsym, lookup_name, 0);
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

enum direct_call_prep_kind {
    DIRECT_CALL_PREP_NONE,
    DIRECT_CALL_PREP_SWAP,
    DIRECT_CALL_PREP_SETTP_SWAP,
    DIRECT_CALL_PREP_SETTP
};

static char *format_direct_call_instruction(char ret_type,
                                            int ret_num,
                                            const char *call_name,
                                            int count_register,
                                            enum direct_call_prep_kind prep_kind,
                                            char prep_type,
                                            int prep_num,
                                            int prep_flags,
                                            int prep_window) {
    switch (prep_kind) {
        case DIRECT_CALL_PREP_SWAP:
            return mprintf("   swapcall %c%d,%s(),r%d,%c%d,r%d\n",
                           ret_type, ret_num, call_name, count_register,
                           prep_type, prep_num, prep_window);
        case DIRECT_CALL_PREP_SETTP_SWAP:
            return mprintf("   settpswapcall %c%d,%s(),r%d,%c%d,%d,r%d\n",
                           ret_type, ret_num, call_name, count_register,
                           prep_type, prep_num, prep_flags, prep_window);
        case DIRECT_CALL_PREP_SETTP:
            return mprintf("   settpcall %c%d,%s(),r%d,r%d,%d\n",
                           ret_type, ret_num, call_name, count_register,
                           prep_window, prep_flags);
        case DIRECT_CALL_PREP_NONE:
        default:
            return mprintf("   call %c%d,%s(),r%d\n",
                           ret_type, ret_num, call_name, count_register);
    }
}

static int fixed_call_local_bytecode(ASTNode *node) {
    Symbol *symbol;
    SymbolNode *definition;
    ASTNode *body;

    if (!node || !node->symbolNode || !node->symbolNode->symbol ||
        is_interface_member_call(node) || is_interface_factory_call(node)) return 0;
    symbol = node->symbolNode->symbol;
    if (sym_nond(symbol) <= 0) return 0;
    definition = sym_trnd(symbol, 0);
    if (!definition || !definition->node) return 0;
    switch (definition->node->node_type) {
        case PROCEDURE:
        case METHOD:
        case FACTORY:
        case MATCH:
            break;
        default:
            return 0;
    }
    body = ast_chld(definition->node, INSTRUCTIONS, NOP);
    return body && body->node_type != NOP;
}

static int fixed_call_argument_status(ASTNode *argument, int *flags) {
    int needs_status = 0;
    int status = 0;

    if (!argument) return 0;
    if (argument->node_type != NOVAL) status |= REGTP_VAL;
    if (!argument->is_ref_arg &&
        (argument->value_dims || argument->target_type == TP_STRING ||
         argument->target_type == TP_OBJECT ||
         argument->target_type == TP_BINARY ||
         argument->target_type == TP_REFERENCE)) {
        needs_status = 1;
        if (!argument->symbolNode) status |= REGTP_NOTSYM;
    }
    if (argument->is_opt_arg) needs_status = 1;
    if (flags) *flags = status;
    return needs_status;
}

static int fixed_call_eligible(ASTNode *node) {
    ASTNode *argument;
    ASTNode *earlier;
    int arity = 0;

    if (!fixed_call_local_bytecode(node)) return 0;
    for (argument = node->child; argument; argument = argument->sibling) {
        arity++;
        if (arity > 4) return 0;
        for (earlier = node->child; earlier != argument; earlier = earlier->sibling) {
            if (earlier->register_type == argument->register_type &&
                earlier->register_num == argument->register_num &&
                (fixed_call_argument_status(earlier, 0) ||
                 fixed_call_argument_status(argument, 0))) {
                /* One physical value cannot simultaneously carry independent
                 * per-formal status. Keep the existing snapshot/window path. */
                return 0;
            }
        }
    }
    return 1;
}

static char *format_fixed_call_instruction(char ret_type,
                                           int ret_num,
                                           const char *call_name,
                                           ASTNode *arguments) {
    ASTNode *argument;
    char *line;
    char *next;
    int arity = 0;

    for (argument = arguments; argument; argument = argument->sibling) arity++;
    if (!arity) return mprintf("   call %c%d,%s()\n", ret_type, ret_num, call_name);

    line = mprintf("   call%d %c%d,%s()", arity, ret_type, ret_num, call_name);
    for (argument = arguments; argument; argument = argument->sibling) {
        next = mprintf("%s,%c%d", line, argument->register_type, argument->register_num);
        free(line);
        line = next;
    }
    next = mprintf("%s\n", line);
    free(line);
    return next;
}

static int same_register(const ASTNode *left, const ASTNode *right) {
    return left && right &&
           left->register_type == right->register_type &&
           left->register_num == right->register_num;
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

enum native_stem_call_kind {
    NATIVE_STEM_CALL_NONE = 0,
    NATIVE_STEM_CALL_GET,
    NATIVE_STEM_CALL_SET,
    NATIVE_STEM_CALL_SIZE,
    NATIVE_STEM_CALL_KEY,
    NATIVE_STEM_CALL_VALUE
};

/*
 * Bypass method selection/call scaffolding only for a concrete rxfnsb.stem
 * held in simple storage. Complex receiver expressions and class attributes
 * retain the ordinary method path because their mutation/copyback contract is
 * not represented by one register operand.
 */
static enum native_stem_call_kind native_stem_call_kind(ASTNode *node) {
    ASTNode *receiver;
    Symbol *method_symbol;
    Symbol *receiver_symbol;
    char *scope_name;
    enum native_stem_call_kind kind = NATIVE_STEM_CALL_NONE;

    if (!node || node->node_type != MEMBER_CALL ||
        !node->symbolNode || !node->symbolNode->symbol ||
        !node->node_string) return NATIVE_STEM_CALL_NONE;

    receiver = node->child;
    if (!receiver || receiver->node_type != VAR_SYMBOL || receiver->child ||
        !receiver->symbolNode || !receiver->symbolNode->symbol)
        return NATIVE_STEM_CALL_NONE;
    receiver_symbol = receiver->symbolNode->symbol;
    if (receiver_symbol->scope &&
        (receiver_symbol->scope->type == SCOPE_CLASS ||
         (receiver_symbol->scope->defining_node &&
          receiver_symbol->scope->defining_node->node_type == CLASS_DEF)))
        return NATIVE_STEM_CALL_NONE;

    method_symbol = node->symbolNode->symbol;
    if (!method_symbol->scope) return NATIVE_STEM_CALL_NONE;
    scope_name = scp_frnm(method_symbol->scope);
    if (!scope_name) return NATIVE_STEM_CALL_NONE;
    if (strcmp(scope_name, "rxfnsb.stem") != 0) {
        free(scope_name);
        return NATIVE_STEM_CALL_NONE;
    }
    free(scope_name);

    if (node->node_string_length == 3 &&
        strncasecmp(node->node_string, "get", 3) == 0)
        kind = NATIVE_STEM_CALL_GET;
    else if (node->node_string_length == 3 &&
             strncasecmp(node->node_string, "set", 3) == 0)
        kind = NATIVE_STEM_CALL_SET;
    else if (node->node_string_length == 4 &&
             strncasecmp(node->node_string, "size", 4) == 0)
        kind = NATIVE_STEM_CALL_SIZE;
    else if (node->node_string_length == 3 &&
             strncasecmp(node->node_string, "key", 3) == 0)
        kind = NATIVE_STEM_CALL_KEY;
    else if ((node->node_string_length == 5 &&
              strncasecmp(node->node_string, "value", 5) == 0) ||
             (node->node_string_length == 7 &&
              strncasecmp(node->node_string, "valueat", 7) == 0))
        kind = NATIVE_STEM_CALL_VALUE;
    return kind;
}

static int emit_native_stem_call(ASTNode *node) {
    enum native_stem_call_kind kind = native_stem_call_kind(node);
    ASTSemanticContextKind semantic_kind;
    ASTNode *receiver;
    ASTNode *arg1;
    ASTNode *arg2;
    ASTNode *operand;
    char *instruction = 0;

    if (kind == NATIVE_STEM_CALL_NONE) return 0;
    receiver = node->child;
    arg1 = receiver ? receiver->sibling : 0;
    arg2 = arg1 ? arg1->sibling : 0;

    if (kind == NATIVE_STEM_CALL_GET &&
        (!arg1 || arg1->node_type == NOVAL || arg2)) return 0;
    if (kind == NATIVE_STEM_CALL_SET &&
        (!arg1 || !arg2 || arg2->sibling)) return 0;
    if (kind == NATIVE_STEM_CALL_SIZE &&
        ((arg1 && arg1->node_type != NOVAL) || arg2)) return 0;
    if ((kind == NATIVE_STEM_CALL_KEY || kind == NATIVE_STEM_CALL_VALUE) &&
        (!arg1 || arg1->node_type == NOVAL || arg2)) return 0;

    if (!node->output) node->output = output_f();
    add_variable_metadata(node);
    semantic_kind = ast_semantic_context_kind(node);
    for (operand = receiver; operand; operand = operand->sibling) {
        if (operand->output) output_concat(node->output, operand->output);
        if (operand == arg1 && arg1->node_type != NOVAL &&
            semantic_context_is_sugar_access(semantic_kind))
            append_semantic_compound_trace_event(node->output, receiver, arg1);
    }

    switch (kind) {
        case NATIVE_STEM_CALL_GET:
            instruction = mprintf("   stemget %c%d,%c%d,%c%d\n",
                                  node->register_type, node->register_num,
                                  receiver->register_type, receiver->register_num,
                                  arg1->register_type, arg1->register_num);
            break;
        case NATIVE_STEM_CALL_SET:
            if (arg1->node_type == NOVAL) {
                instruction = mprintf("   stemreset %c%d,%c%d\n",
                                      receiver->register_type, receiver->register_num,
                                      arg2->register_type, arg2->register_num);
            } else {
                instruction = mprintf("   stemset %c%d,%c%d,%c%d\n",
                                      receiver->register_type, receiver->register_num,
                                      arg1->register_type, arg1->register_num,
                                      arg2->register_type, arg2->register_num);
            }
            break;
        case NATIVE_STEM_CALL_SIZE:
            instruction = mprintf("   stemsize %c%d,%c%d\n",
                                  node->register_type, node->register_num,
                                  receiver->register_type, receiver->register_num);
            break;
        case NATIVE_STEM_CALL_KEY:
            instruction = mprintf("   stemkeyat %c%d,%c%d,%c%d\n",
                                  node->register_type, node->register_num,
                                  receiver->register_type, receiver->register_num,
                                  arg1->register_type, arg1->register_num);
            break;
        case NATIVE_STEM_CALL_VALUE:
            instruction = mprintf("   stemvalueat %c%d,%c%d,%c%d\n",
                                  node->register_type, node->register_num,
                                  receiver->register_type, receiver->register_num,
                                  arg1->register_type, arg1->register_num);
            break;
        default:
            return 0;
    }
    output_append_text(node->output, instruction);
    free(instruction);

    for (operand = receiver; operand; operand = operand->sibling) {
        if (operand->cleanup) output_concat(node->output, operand->cleanup);
    }

    type_promotion(node);
    if (semantic_context_is_sugar_access(semantic_kind)) {
        append_semantic_access_value_trace_event(node->output,
                                                 semantic_kind,
                                                 receiver,
                                                 node,
                                                 arg2);
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
    return 1;
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

static int output_fragment_in_chain(OutputFragment *chain, OutputFragment *fragment) {
    OutputFragment *current;

    if (!chain || !fragment) return 0;
    while (chain->before) chain = chain->before;
    for (current = chain; current; current = current->after) {
        if (current == fragment) return 1;
    }
    return 0;
}

static void concat_binary_memory_output_fragment(OutputFragment *output, OutputFragment *fragment) {
    if (!output || !fragment) return;
    if (output_fragment_in_chain(output, fragment)) return;
    output_concat(output, fragment);
}

static void concat_binary_memory_operand_output(OutputFragment *output, ASTNode *node) {
    ASTNode *base = 0;
    ASTNode *offset = 0;
    ASTNode *length = 0;

    if (!output || !rxcp_binary_memory_at_parts(node, 0, &base, &offset)) return;
    if (node && node->node_type == OP_BINARY_FOR) length = ast_chdn(node, 1);
    if (base && base->output) concat_binary_memory_output_fragment(output, base->output);
    if (offset && offset->output) concat_binary_memory_output_fragment(output, offset->output);
    if (length && length->output) concat_binary_memory_output_fragment(output, length->output);
}

static void concat_binary_memory_operand_cleanup(OutputFragment *output, ASTNode *node) {
    ASTNode *base = 0;
    ASTNode *offset = 0;
    ASTNode *length = 0;

    if (!output || !rxcp_binary_memory_at_parts(node, 0, &base, &offset)) return;
    if (node && node->node_type == OP_BINARY_FOR) length = ast_chdn(node, 1);
    if (length && length->cleanup) concat_binary_memory_output_fragment(output, length->cleanup);
    if (offset && offset->cleanup) concat_binary_memory_output_fragment(output, offset->cleanup);
    if (base && base->cleanup) concat_binary_memory_output_fragment(output, base->cleanup);
}

static void set_binary_memory_operand_cleanup(ASTNode *node) {
    if (!node) return;
    if (!node->cleanup) node->cleanup = output_f();
    concat_binary_memory_operand_cleanup(node->cleanup, node);
}

static char *binary_memory_operand_text(ASTNode *node, ValueType fallback_type) {
    ValueType type;

    if (!node) return strdup("0");
    if (node->register_num == DONT_ASSIGN_REGISTER) {
        type = node->target_type != TP_UNKNOWN ? node->target_type : node->value_type;
        if (type == TP_UNKNOWN) type = fallback_type;
        return format_constant(type, node);
    }
    return mprintf("%c%d", node->register_type ? node->register_type : 'r', node->register_num);
}

static void concat_node_output(OutputFragment *output, ASTNode *node) {
    if (output && node && node->output) concat_binary_memory_output_fragment(output, node->output);
}

static void concat_node_cleanup(OutputFragment *output, ASTNode *node) {
    if (output && node && node->cleanup) concat_binary_memory_output_fragment(output, node->cleanup);
}

static void emit_integer_value_to_register(OutputFragment *output,
                                           char register_type,
                                           int register_num,
                                           ASTNode *value) {
    char *operand;
    char *text;

    if (!output || !value) return;
    if (value->register_num == DONT_ASSIGN_REGISTER) {
        operand = binary_memory_operand_text(value, TP_INTEGER);
        text = mprintf("   load %c%d,%s\n", register_type, register_num, operand);
        output_append_text(output, text);
        free(text);
        free(operand);
    } else if (value->register_type != register_type || value->register_num != register_num) {
        text = mprintf("   icopy %c%d,%c%d\n",
                       register_type,
                       register_num,
                       value->register_type,
                       value->register_num);
        output_append_text(output, text);
        free(text);
    }
}

static int binary_compare_arg_count(ASTNode *node) {
    ASTNode *first;
    int count;

    count = ast_nchd(node) - 1;
    first = ast_chdn(node, 1);
    if (count == 1 && first && first->node_type == NOVAL) return 0;
    return count;
}

static void emit_binary_memory_compare_fixed(OutputFragment *output,
                                             ASTNode *node,
                                             RxcpBinaryStorageInfo *info,
                                             ASTNode *memory,
                                             ASTNode *offset,
                                             ASTNode *value) {
    const char *lt_op;
    const char *gt_op;
    char *memory_operand;
    char *value_operand;
    char *text;
    int bool_reg;

    if (!output || !node || !info || !memory || !offset || !value) return;
    bool_reg = node->additional_registers;
    lt_op = info->value_type == TP_FLOAT ? "flt" : "ilt";
    gt_op = info->value_type == TP_FLOAT ? "fgt" : "igt";
    memory_operand = binary_memory_operand_text(memory, TP_BINARY);
    value_operand = binary_memory_operand_text(value, info->value_type);

    text = mprintf("   %s %c%d,%s,%c%d\n"
                   "   %s r%d,%c%d,%s\n"
                   "   brf l%dcompare_ge,r%d\n"
                   "   load %c%d,-1\n"
                   "   br l%dcompare_end\n"
                   "l%dcompare_ge:\n"
                   "   %s r%d,%c%d,%s\n"
                   "   brf l%dcompare_eq,r%d\n"
                   "   load %c%d,1\n"
                   "   br l%dcompare_end\n"
                   "l%dcompare_eq:\n"
                   "   load %c%d,0\n"
                   "l%dcompare_end:\n",
                   info->rxas_get,
                   node->register_type,
                   node->register_num,
                   memory_operand,
                   offset->register_type,
                   offset->register_num,
                   lt_op,
                   bool_reg,
                   node->register_type,
                   node->register_num,
                   value_operand,
                   node->node_number,
                   bool_reg,
                   node->register_type,
                   node->register_num,
                   node->node_number,
                   node->node_number,
                   gt_op,
                   bool_reg,
                   node->register_type,
                   node->register_num,
                   value_operand,
                   node->node_number,
                   bool_reg,
                   node->register_type,
                   node->register_num,
                   node->node_number,
                   node->node_number,
                   node->register_type,
                   node->register_num,
                   node->node_number);
    output_append_text(output, text);
    free(text);
    free(memory_operand);
    free(value_operand);
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

static int call_arguments_share_register(ASTNode *left, ASTNode *right) {
    return left && right &&
           left->register_type == right->register_type &&
           left->register_num == right->register_num;
}

/* Select one argument to carry a shared source through the existing swap/
 * restore path. Other occurrences are copied into their call-frame slots
 * before any source register can be mutated. Prefer an occurrence already in
 * its destination slot; otherwise use the first occurrence. */
static ASTNode *call_argument_group_primary(ASTNode *first,
                                            ASTNode *argument,
                                            int first_destination) {
    ASTNode *current;
    ASTNode *primary = 0;
    int destination = first_destination;

    for (current = first; current; current = current->sibling, destination++) {
        if (!call_arguments_share_register(current, argument)) continue;
        if (!primary) primary = current;
        if (current->register_type == 'r' && current->register_num == destination) {
            return current;
        }
    }
    return primary;
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
    enum direct_call_prep_kind direct_prep_kind = DIRECT_CALL_PREP_NONE;
    char direct_prep_type = 'r';
    int direct_prep_num = -1;
    int direct_prep_flags = 0;
    int direct_prep_window = -1;
    ASTNode *child1 = node->child;
    ASTNode *child2 = node->child ? node->child->sibling : 0;
    ASTNode *child3 = node->child && node->child->sibling ? node->child->sibling->sibling : 0;
    ASTSemanticContextKind semantic_kind = ast_semantic_context_kind(node);

    switch (node->node_type) {

        case FACTORY_CALL:
        case MEMBER_CALL:
        case FUNCTION:
            {
            if (node->node_type == MEMBER_CALL && emit_native_stem_call(node))
                break;
            int can_fuse_direct_call =
                    !is_interface_member_call(node) &&
                    !is_interface_factory_call(node);
            int use_fixed_call = fixed_call_eligible(node);
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

            /* Fixed calls keep the existing per-argument status contract.
             * They separate SETTP from the call instead of fusing it with
             * contiguous-window marshalling. */
            if (use_fixed_call) {
                for (n = child1; n; n = n->sibling) {
                    if (fixed_call_argument_status(n, &j)) {
                        temp1 = mprintf("   settp %c%d,%d\n",
                                        n->register_type,
                                        n->register_num,
                                        j);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }
                }
            }

            /* Number of arguments. Keep this after argument expression
             * evaluation: inlined argument blocks may use temporary
             * registers from the call frame before marshalling starts. */
            if (!use_fixed_call) {
                temp1 = mprintf("   load r%d,%d\n",
                                node->additional_registers,
                                node->num_additional_registers - 1);
                output_append_text(node->output, temp1);
                free(temp1);
            }

            /* A repeated source register is not a permutation and cannot be
             * marshalled by destructive swaps alone. Snapshot every
             * non-primary occurrence into its final call-frame slot first. */
            n = child1;
            i = node->additional_registers + 1;
            while (!use_fixed_call && n) {
                ASTNode *primary = call_argument_group_primary(child1,
                                                               n,
                                                               node->additional_registers + 1);
                if (primary && primary != n) {
                    temp1 = mprintf("   copy r%d,%c%d\n",
                                    i, n->register_type, n->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                n = n->sibling;
                i++;
            }

            /* Now step through the arguments - marshalling them in order and
             * setting argument flags as required */
            n = child1;
            i = node->additional_registers + 1; /* The first one is the number of arguments */
            while (!use_fixed_call && n) {
                ASTNode *primary = call_argument_group_primary(child1,
                                                               n,
                                                               node->additional_registers + 1);
                int staged_duplicate = primary && primary != n;
                char marshalled_type = staged_duplicate ? 'r' : n->register_type;
                int marshalled_num = staged_duplicate ? i : n->register_num;
                int needs_swap = !staged_duplicate &&
                                 (n->register_type != 'r' || n->register_num != i);
                int can_fuse_this_prep = can_fuse_direct_call && !n->sibling;
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
                if (can_fuse_this_prep && (k || needs_swap)) {
                    direct_prep_type = marshalled_type;
                    direct_prep_num = marshalled_num;
                    direct_prep_flags = j;
                    direct_prep_window = i;
                    if (k && needs_swap) {
                        direct_prep_kind = DIRECT_CALL_PREP_SETTP_SWAP;
                    } else if (needs_swap) {
                        direct_prep_kind = DIRECT_CALL_PREP_SWAP;
                    } else {
                        direct_prep_kind = DIRECT_CALL_PREP_SETTP;
                    }
                }

                if (k && direct_prep_kind == DIRECT_CALL_PREP_NONE) { /* We need to settp */
                    temp1 = mprintf("   settp %c%d,%d\n",
                                    marshalled_type,
                                    marshalled_num,
                                    j);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }

                if (needs_swap) {
                    /* We need to swap registers to get it right for the call */
                    if (direct_prep_kind == DIRECT_CALL_PREP_NONE) {
                        temp1 = mprintf("   swap r%d,%c%d\n",
                                        i, n->register_type, n->register_num);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }

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
            if (use_fixed_call) {
                char *call_name;
                Symbol *fsym = node->symbolNode->symbol;
                SymbolNode *defsn = sym_trnd(fsym, 0);
                int use_mangled = defsn && defsn->node &&
                                  (defsn->node->node_type == METHOD ||
                                   defsn->node->node_type == FACTORY ||
                                   defsn->node->node_type == MATCH);
                if (use_mangled) call_name = sym_mngd_frnm(fsym);
                else if (node->node_string &&
                         rxcp_source_symbol_is_qualified(node->node_string,
                                                         node->node_string_length)) {
                    call_name = strdup(fsym->name);
                } else if (node->node_string) {
                    size_t start = 0;
                    size_t len = node->node_string_length;
                    if (len >= 2 &&
                        (node->node_string[0] == '\'' || node->node_string[0] == '"') &&
                        node->node_string[len - 1] == node->node_string[0]) {
                        start = 1;
                        len -= 2;
                    }
                    call_name = malloc(len + 1);
                    memcpy(call_name, node->node_string + start, len);
                    call_name[len] = 0;
                } else call_name = strdup(fsym->name);
                temp1 = format_fixed_call_instruction(ret_type,
                                                      ret_num,
                                                      call_name,
                                                      child1);
                free(call_name);
            }
            else if (is_interface_member_call(node)) {
                Symbol *fsym = node->symbolNode->symbol;
                char *descriptor = build_dynamic_callable_descriptor(fsym, fsym->name);

                temp1 = mprintf("   srcmethodsel %c%d,r%d,\"%s\"\n",
                                ret_type, ret_num,
                                node->additional_registers + 1,
                                descriptor ? descriptor : "");
                output_append_text(node->output, temp1);
                free(temp1);
                if (descriptor) free(descriptor);

                temp1 = mprintf("   dcall %c%d,%c%d,r%d\n",
                                ret_type, ret_num,
                                ret_type, ret_num,
                                node->additional_registers);
            }
            else if (is_interface_factory_call(node)) {
                char *selector = build_interface_factory_selector(node);
                Symbol *fsym = node->symbolNode->symbol;
                ValueType resolved_type = node->target_type != TP_UNKNOWN ? node->target_type : node->value_type;
                const char *resolved_class = node->target_class ? node->target_class : node->value_class;
                char *resolved_return = build_source_type_name(resolved_type, resolved_class);
                char *descriptor = build_dynamic_callable_descriptor_with_return(fsym,
                                                                                 selector ? selector : "",
                                                                                 resolved_return);

                temp1 = mprintf("   srcfprocsel %c%d,\"%s\",r%d\n",
                                ret_type, ret_num,
                                descriptor ? descriptor : "",
                                node->additional_registers);
                output_append_text(node->output, temp1);
                free(temp1);
                if (descriptor) free(descriptor);
                if (resolved_return) free(resolved_return);
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
                temp1 = format_direct_call_instruction(ret_type,
                                                       ret_num,
                                                       call_name,
                                                       node->additional_registers,
                                                       direct_prep_kind,
                                                       direct_prep_type,
                                                       direct_prep_num,
                                                       direct_prep_flags,
                                                       direct_prep_window);
                free(call_name);
            } else {
                char *call_name = mprintf("%.*s",
                                          (int)node->node_string_length,
                                          node->node_string);
                temp1 = format_direct_call_instruction(ret_type,
                                                       ret_num,
                                                       call_name,
                                                       node->additional_registers,
                                                       direct_prep_kind,
                                                       direct_prep_type,
                                                       direct_prep_num,
                                                       direct_prep_flags,
                                                       direct_prep_window);
                free(call_name);
            }
            output_append_text(node->output, temp1);
            free(temp1);

            /* Step through for swapping registers back */
            n = child1;
            i = node->additional_registers + 1; /* First one is the number of arguments */
            while (n) {
                ASTNode *primary = call_argument_group_primary(child1,
                                                               n,
                                                               node->additional_registers + 1);
                int staged_duplicate = primary && primary != n;
                if (!use_fixed_call && !staged_duplicate &&
                    (n->register_type != 'r' || n->register_num != i)) {
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
            }

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
            if (child1->target_type == TP_BINARY &&
                (node->node_type == OP_COMPARE_EQUAL || node->node_type == OP_COMPARE_NEQ)) {
                tp_prefix = "";
                op = node->node_type == OP_COMPARE_EQUAL ? "bineq" : "binne";
            }
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

            /* The float expression (a / b) - constant commonly reuses b as
             * the compiler-only division destination. Preserve that
             * intermediate write for source TRACE, but execute the complete
             * arithmetic unit in one dispatch. */
            if (node->node_type == OP_MINUS &&
                node->target_type == TP_FLOAT &&
                child1->node_type == OP_DIV &&
                child1->target_type == TP_FLOAT &&
                child1->child && child1->child->sibling &&
                same_register(child1, child1->child->sibling) &&
                child2->register_num == DONT_ASSIGN_REGISTER) {
                ASTNode *numerator = child1->child;
                ASTNode *divisor = child1->child->sibling;
                char *constant = format_constant(TP_FLOAT, child2);
                char *old_div = mprintf("   fdiv %c%d,%c%d,%c%d\n",
                                        child1->register_type,
                                        child1->register_num,
                                        numerator->register_type,
                                        numerator->register_num,
                                        divisor->register_type,
                                        divisor->register_num);
                char *new_div = mprintf("   fdivsub %c%d,%c%d,%c%d,%s\n",
                                        node->register_type,
                                        node->register_num,
                                        numerator->register_type,
                                        numerator->register_num,
                                        divisor->register_type,
                                        divisor->register_num,
                                        constant);
                int replaced = output_replace_text_once(child1->output,
                                                        old_div,
                                                        new_div);
                free(constant);
                free(old_div);
                free(new_div);
                if (replaced) {
                    output_concat(node->output, child1->output);
                    if (child1->cleanup) output_concat(node->output, child1->cleanup);
                    type_promotion(node);
                    append_semantic_operation_trace_event(node->output,
                                                          RXBIN_TRACE_KIND_BINARY_OP,
                                                          semantic_kind,
                                                          node);
                    break;
                }
            }

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
            if (node->value_type == TP_INTEGER &&
                emit_is_rxinteger_min_magnitude_literal(child1)) {
                temp1 = mprintf("   load %c%d,-9223372036854775808\n",
                                node->register_type,
                                node->register_num);
            }
            else if (node->value_type == TP_FLOAT) {
                if (child1->output) output_concat(node->output, child1->output);
                temp1 = mprintf("   fsub %c%d,0.0,%c%d\n",
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num);
            }
            else if (node->value_type == TP_DECIMAL) {
                if (child1->output) output_concat(node->output, child1->output);
                temp1 = mprintf("   dsub %c%d,0d,%c%d\n",
                                node->register_type,
                                node->register_num,
                                child1->register_type,
                                child1->register_num);
            }
            else {
                if (child1->output) output_concat(node->output, child1->output);
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

            if (node->target_type != TP_OBJECT &&
                (node->register_type != child1->register_type ||
                 node->register_num != child1->register_num) &&
                ((child1->value_type == TP_INTEGER && node->target_type == TP_FLOAT) ||
                 (child1->value_type == TP_STRING && node->target_type == TP_INTEGER))) {
                const char *direct_promotion =
                        emit_promotion[child1->value_type][node->target_type];
                if (direct_promotion) {
                    temp1 = mprintf("   %s %c%d,%c%d\n",
                                    direct_promotion,
                                    node->register_type,
                                    node->register_num,
                                    child1->register_type,
                                    child1->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                    if (child1->cleanup) output_concat(node->output, child1->cleanup);
                    break;
                }
            }

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

        case OP_SIZEOF: {
            RxcpBinaryStorageInfo info;
            int width = 0;

            if (rxcp_binary_storage_info(child1, &info) && info.is_fixed) {
                width = info.width;
            }
            if (!node->output) node->output = output_f();
            temp1 = mprintf("   load %c%d,%d\n",
                            node->register_type,
                            node->register_num,
                            width);
            output_append_text(node->output, temp1);
            free(temp1);
            type_promotion(node);
            break;
        }

        case OP_BINARY_LENGTH: {
            ASTNode *memory = child1;
            char *memory_operand;

            if (!node->output) node->output = output_f();
            concat_node_output(node->output, memory);

            memory_operand = binary_memory_operand_text(memory, TP_BINARY);
            temp1 = mprintf("   blen %c%d,%s\n",
                            node->register_type,
                            node->register_num,
                            memory_operand);
            output_append_text(node->output, temp1);
            free(temp1);
            free(memory_operand);

            concat_node_cleanup(node->output, memory);
            type_promotion(node);
            break;
        }

        case OP_BINARY_AT:
        case OP_PACKED_AT: {
            RxcpBinaryStorageInfo info;
            ASTNode *base = 0;
            ASTNode *offset = 0;

            if (!node->output) node->output = output_f();
            concat_binary_memory_operand_output(node->output, node);

            if (rxcp_binary_memory_is_lhs(node) ||
                (node->parent && node->parent->node_type == OP_BINARY_FOR)) {
                set_binary_memory_operand_cleanup(node);
                break;
            }

            if ((node->node_type == OP_PACKED_AT
                     ? rxcp_packed_storage_info(child1, &info)
                     : rxcp_binary_storage_info(child1, &info)) &&
                rxcp_binary_memory_at_parts(node, 0, &base, &offset) &&
                base && offset) {
                if (info.is_fixed) {
                    char *base_operand = binary_memory_operand_text(base, TP_BINARY);
                    temp1 = mprintf("   %s %c%d,%s,%c%d\n",
                                    info.rxas_get,
                                    node->register_type,
                                    node->register_num,
                                    base_operand,
                                    offset->register_type,
                                    offset->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                    free(base_operand);
                } else if (info.value_type == TP_STRING || info.value_type == TP_DECIMAL) {
                    char *base_operand = binary_memory_operand_text(base, TP_BINARY);
                    temp1 = mprintf("   bgets %c%d,%s,%c%d\n",
                                    node->register_type,
                                    node->register_num,
                                    base_operand,
                                    offset->register_type,
                                    offset->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                    free(base_operand);
                    if (info.value_type == TP_DECIMAL) {
                        temp1 = mprintf("   stod %c%d\n",
                                        node->register_type,
                                        node->register_num);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }
                }
            }

            concat_binary_memory_operand_cleanup(node->output, node);
            type_promotion(node);
            break;
        }

        case OP_BINARY_FOR: {
            ASTNode *base = 0;
            ASTNode *offset = 0;
            ASTNode *length = child2;

            if (!node->output) node->output = output_f();
            concat_binary_memory_operand_output(node->output, node);

            if (!rxcp_binary_memory_is_lhs(node) &&
                rxcp_binary_memory_at_parts(node, 0, &base, &offset) &&
                base && offset && length) {
                temp1 = mprintf("   bresize %c%d,%c%d\n"
                                "   bcopy %c%d,%c%d,%c%d\n",
                                node->register_type,
                                node->register_num,
                                length->register_type,
                                length->register_num,
                                node->register_type,
                                node->register_num,
                                base->register_type,
                                base->register_num,
                                offset->register_type,
                                offset->register_num);
                output_append_text(node->output, temp1);
                free(temp1);

                if (node->value_type == TP_STRING || node->target_type == TP_STRING ||
                    node->value_type == TP_DECIMAL || node->target_type == TP_DECIMAL) {
                    temp1 = mprintf("   bintos %c%d\n",
                                    node->register_type,
                                    node->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                if (node->value_type == TP_DECIMAL || node->target_type == TP_DECIMAL) {
                    temp1 = mprintf("   stod %c%d\n",
                                    node->register_type,
                                    node->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
            }

            concat_binary_memory_operand_cleanup(node->output, node);
            type_promotion(node);
            break;
        }

        case OP_BINARY_COMPARE: {
            RxcpBinaryStorageInfo info;
            ASTNode *type_node = child1;
            ASTNode *memory = child2;
            ASTNode *offset = child3;
            ASTNode *third = ast_chdn(node, 3);
            int argc = binary_compare_arg_count(node);

            if (!node->output) node->output = output_f();
            concat_node_output(node->output, memory);
            concat_node_output(node->output, offset);
            concat_node_output(node->output, third);

            if (rxcp_binary_storage_info(type_node, &info)) {
                if (info.value_type == TP_BINARY && !info.is_fixed && argc == 3) {
                    char *memory_operand = binary_memory_operand_text(memory, TP_BINARY);
                    char *needle_operand = binary_memory_operand_text(third, TP_BINARY);
                    emit_integer_value_to_register(node->output,
                                                   node->register_type,
                                                   node->register_num,
                                                   offset);
                    temp1 = mprintf("   bcmpb %c%d,%s,%s\n",
                                    node->register_type,
                                    node->register_num,
                                    memory_operand,
                                    needle_operand);
                    output_append_text(node->output, temp1);
                    free(temp1);
                    free(memory_operand);
                    free(needle_operand);
                } else if (info.value_type == TP_STRING && !info.is_fixed && argc == 3) {
                    char *memory_operand = binary_memory_operand_text(memory, TP_BINARY);
                    char *needle_operand = binary_memory_operand_text(third, TP_STRING);
                    emit_integer_value_to_register(node->output,
                                                   node->register_type,
                                                   node->register_num,
                                                   offset);
                    temp1 = mprintf("   bcmps %c%d,%s,%s\n",
                                    node->register_type,
                                    node->register_num,
                                    memory_operand,
                                    needle_operand);
                    output_append_text(node->output, temp1);
                    free(temp1);
                    free(memory_operand);
                    free(needle_operand);
                } else if (info.is_fixed && argc == 3) {
                    emit_binary_memory_compare_fixed(node->output,
                                                     node,
                                                     &info,
                                                     memory,
                                                     offset,
                                                     third);
                }
            }

            concat_node_cleanup(node->output, third);
            concat_node_cleanup(node->output, offset);
            concat_node_cleanup(node->output, memory);
            type_promotion(node);
            break;
        }

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
                emit_scope_reference_lifetimes(node->output, node);
                clear_scope_variable_metadata(node->output, node);
            }

            type_promotion(node);
            break;

        default:
            break;
    }

}
