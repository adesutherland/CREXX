/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 *
 * Bounded, body-driven constant evaluation for ordinary Level B callables.
 * The evaluator is transactional: it mutates only private value state and
 * returns success only after one complete, non-signalling return path.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxcpmain.h"
#include "rxcp_constant.h"
#include "rxcp_partial_call.h"
#include "rxvmvars.h"
#include "rxvalue.h"

#define RXCP_PARTIAL_MAX_SYMBOLS 256u
#define RXCP_PARTIAL_MAX_STEPS 4096u
#define RXCP_PARTIAL_MAX_RESULT_CODEPOINTS ((size_t)1048576u)
#define RXCP_PARTIAL_MAX_STRING_BYTES (RXCP_PARTIAL_MAX_RESULT_CODEPOINTS * 4u)

typedef struct RxcpPartialValue {
    ValueType type;
    int known;
    rxinteger integer;
    value string;
} RxcpPartialValue;

typedef struct RxcpPartialSlot {
    Symbol *symbol;
    int provided;
    RxcpPartialValue value;
} RxcpPartialSlot;

typedef struct RxcpPartialState {
    Context *context;
    ASTNode *call;
    Scope *callable_scope;
    RxcpPartialSlot slots[RXCP_PARTIAL_MAX_SYMBOLS];
    size_t slot_count;
    size_t steps;
    int returned;
    RxcpPartialValue return_value;
} RxcpPartialState;

static int partial_scope_is_within(Scope *scope, Scope *root) {
    if (!root) return 0;
    while (scope) {
        if (scope == root) return 1;
        scope = scope->parent;
    }
    return 0;
}

static void partial_value_init(RxcpPartialValue *item) {
    if (!item) return;
    memset(item, 0, sizeof(*item));
    value_init(&item->string);
}

static void partial_value_clear(RxcpPartialValue *item) {
    if (!item) return;
    clear_value(&item->string);
    memset(item, 0, sizeof(*item));
}

static int partial_value_within_budget(const RxcpPartialValue *item) {
    if (!item || !item->known || item->type != TP_STRING) return item && item->known;
    if (item->string.string_length > RXCP_PARTIAL_MAX_STRING_BYTES) return 0;
#ifdef NUTF8
    return item->string.string_length <= RXCP_PARTIAL_MAX_RESULT_CODEPOINTS;
#else
    return item->string.string_chars <= RXCP_PARTIAL_MAX_RESULT_CODEPOINTS;
#endif
}

static int partial_value_copy(RxcpPartialValue *target,
                              const RxcpPartialValue *source) {
    if (!target || !source || !partial_value_within_budget(source)) return 0;
    partial_value_clear(target);
    partial_value_init(target);
    target->type = source->type;
    target->known = 1;
    target->integer = source->integer;
    if (source->type == TP_STRING) {
        set_value_string(&target->string, (value *)&source->string);
    }
    return partial_value_within_budget(target);
}

static RxcpPartialSlot *partial_slot(RxcpPartialState *state,
                                     Symbol *symbol,
                                     int create) {
    size_t index;

    if (!state || !symbol) return 0;
    for (index = 0; index < state->slot_count; index++) {
        if (state->slots[index].symbol == symbol) return &state->slots[index];
    }
    if (!create || state->slot_count >= RXCP_PARTIAL_MAX_SYMBOLS ||
        !partial_scope_is_within(symbol->scope, state->callable_scope)) return 0;
    index = state->slot_count++;
    memset(&state->slots[index], 0, sizeof(state->slots[index]));
    state->slots[index].symbol = symbol;
    partial_value_init(&state->slots[index].value);
    return &state->slots[index];
}

static Symbol *partial_node_symbol(ASTNode *node) {
    return node && node->symbolNode ? node->symbolNode->symbol : 0;
}

static int partial_scalar_type_supported(ValueType type) {
    return type == TP_BOOLEAN || type == TP_INTEGER || type == TP_STRING;
}

static int partial_callable_node(ASTNode *node) {
    return node && (node->node_type == PROCEDURE ||
                    node->node_type == METHOD ||
                    node->node_type == FACTORY);
}

static ASTNode *partial_callable_template(Symbol *symbol) {
    ASTNode *local;

    if (!symbol) return 0;
    if (symbol->ast_template) {
        return symbol->is_inlinable && partial_callable_node(symbol->ast_template) ?
               symbol->ast_template : 0;
    }
    local = symbol->defines_scope ? symbol->defines_scope->defining_node : 0;
    return partial_callable_node(local) ? local : 0;
}

static int partial_set_default(RxcpPartialValue *result, ValueType type) {
    if (!result) return 0;
    partial_value_clear(result);
    partial_value_init(result);
    result->type = type;
    result->known = 1;
    result->integer = 0;
    if (type == TP_STRING) {
        return set_string_validated(&result->string, "", 0u) == 0;
    }
    return type == TP_INTEGER || type == TP_BOOLEAN;
}

static int partial_eval_expr(RxcpPartialState *state,
                             ASTNode *node,
                             RxcpPartialValue *result);

static int partial_eval_constant(ASTNode *node, RxcpPartialValue *result) {
    unsigned char *decoded;
    size_t decoded_length;

    if (!node || !result || node->node_type != CONSTANT) return 0;
    if (node->value_type == TP_INTEGER || node->value_type == TP_BOOLEAN) {
        if (!partial_set_default(result, node->value_type)) return 0;
        result->integer = node->int_value;
        return 1;
    }
    if (node->value_type != TP_STRING) return 0;
    decoded = rxcp_constant_string_decode(node->node_string ? node->node_string : "",
                                          node->node_string_length,
                                          &decoded_length);
    if (!decoded) return 0;
    if (decoded_length > RXCP_PARTIAL_MAX_STRING_BYTES) {
        free(decoded);
        return 0;
    }
    partial_value_clear(result);
    partial_value_init(result);
    result->type = TP_STRING;
    result->known = set_string_validated(&result->string,
                                         (const char *)decoded,
                                         decoded_length) == 0;
    free(decoded);
    return result->known;
}

static int partial_eval_integer_binary(RxcpPartialState *state,
                                       ASTNode *node,
                                       RxcpPartialValue *result) {
    RxcpPartialValue left;
    RxcpPartialValue right;
    rxinteger folded;
    int ok;

    partial_value_init(&left);
    partial_value_init(&right);
    ok = 0;
    if (!node->child || !node->child->sibling || node->child->sibling->sibling ||
        !partial_eval_expr(state, node->child, &left) ||
        !partial_eval_expr(state, node->child->sibling, &right) ||
        (left.type != TP_INTEGER && left.type != TP_BOOLEAN) ||
        (right.type != TP_INTEGER && right.type != TP_BOOLEAN)) goto done;

    if (node->node_type == OP_PLUS || node->node_type == OP_ADD) {
        if (!rxinteger_checked_add(left.integer, right.integer, &folded)) goto done;
        if (!partial_set_default(result, TP_INTEGER)) goto done;
        result->integer = folded;
        ok = 1;
    } else if (node->node_type == OP_MINUS) {
        if (!rxinteger_checked_sub(left.integer, right.integer, &folded)) goto done;
        if (!partial_set_default(result, TP_INTEGER)) goto done;
        result->integer = folded;
        ok = 1;
    } else {
        if (!partial_set_default(result, TP_BOOLEAN)) goto done;
        switch (node->node_type) {
            case OP_COMPARE_EQUAL: result->integer = left.integer == right.integer; break;
            case OP_COMPARE_NEQ: result->integer = left.integer != right.integer; break;
            case OP_COMPARE_LT: result->integer = left.integer < right.integer; break;
            case OP_COMPARE_GT: result->integer = left.integer > right.integer; break;
            case OP_COMPARE_LTE: result->integer = left.integer <= right.integer; break;
            case OP_COMPARE_GTE: result->integer = left.integer >= right.integer; break;
            case OP_AND: result->integer = left.integer && right.integer; break;
            case OP_OR: result->integer = left.integer || right.integer; break;
            default: goto done;
        }
        ok = 1;
    }

done:
    partial_value_clear(&right);
    partial_value_clear(&left);
    return ok;
}

static int partial_eval_expr(RxcpPartialState *state,
                             ASTNode *node,
                             RxcpPartialValue *result) {
    RxcpPartialSlot *slot;
    RxcpPartialValue operand;
    Symbol *symbol;

    if (!state || !node || !result || ++state->steps > RXCP_PARTIAL_MAX_STEPS) return 0;
    switch (node->node_type) {
        case CONSTANT:
            return partial_eval_constant(node, result);

        case VAR_SYMBOL:
        case VAR_TARGET:
            slot = partial_slot(state, partial_node_symbol(node), 0);
            return slot && partial_value_copy(result, &slot->value);

        case CLASS:
            if (node->value_dims != 0 || node->target_dims != 0) return 0;
            return partial_set_default(result,
                                       node->value_type != TP_UNKNOWN ?
                                       node->value_type : node->target_type);

        case OP_ARG_EXISTS:
            symbol = partial_node_symbol(node);
            slot = partial_slot(state, symbol, 0);
            if (!slot && node->node_string) {
                size_t index;
                for (index = 0; index < state->slot_count; index++) {
                    if (state->slots[index].symbol && state->slots[index].symbol->name &&
                        strlen(state->slots[index].symbol->name) == node->node_string_length &&
                        strncmp(state->slots[index].symbol->name,
                                node->node_string,
                                node->node_string_length) == 0) {
                        slot = &state->slots[index];
                        break;
                    }
                }
            }
            if (!slot || !partial_set_default(result, TP_BOOLEAN)) return 0;
            result->integer = slot->provided != 0;
            return 1;

        case OP_PLUS:
        case OP_ADD:
        case OP_MINUS:
        case OP_COMPARE_EQUAL:
        case OP_COMPARE_NEQ:
        case OP_COMPARE_LT:
        case OP_COMPARE_GT:
        case OP_COMPARE_LTE:
        case OP_COMPARE_GTE:
        case OP_AND:
        case OP_OR:
            return partial_eval_integer_binary(state, node, result);

        case OP_NOT:
            partial_value_init(&operand);
            if (!node->child || node->child->sibling ||
                !partial_eval_expr(state, node->child, &operand) ||
                (operand.type != TP_INTEGER && operand.type != TP_BOOLEAN) ||
                !partial_set_default(result, TP_BOOLEAN)) {
                partial_value_clear(&operand);
                return 0;
            }
            result->integer = !operand.integer;
            partial_value_clear(&operand);
            return 1;

        default:
            return 0;
    }
}

static OperandType partial_operand_type(ASTNode *node) {
    if (!node) return OP_NONE;
    switch (node->node_type) {
        case INTEGER: return OP_INT;
        case FLOAT: return OP_FLOAT;
        case DECIMAL: return OP_DECIMAL;
        case STRING: return OP_STRING;
        case BINARY: return OP_BINARY;
        case FUNC_SYMBOL: return OP_FUNC;
        case CONSTANT:
            switch (node->target_type != TP_UNKNOWN ? node->target_type : node->value_type) {
                case TP_BOOLEAN:
                case TP_INTEGER: return OP_INT;
                case TP_FLOAT: return OP_FLOAT;
                case TP_DECIMAL: return OP_DECIMAL;
                case TP_STRING: return OP_STRING;
                case TP_BINARY: return OP_BINARY;
                default: return OP_REG;
            }
        default: return OP_REG;
    }
}

static int partial_assembler_instruction(ASTNode *node,
                                         const OpInfo **instruction_out) {
    ASTNode *child;
    OperandType types[16];
    size_t count;
    char *name;
    char *cursor;
    const OpInfo *instruction;

    if (instruction_out) *instruction_out = 0;
    if (!node || node->node_type != ASSEMBLER || !node->node_string ||
        node->node_string_length == 0) return 0;
    count = 0;
    for (child = node->child; child; child = child->sibling) {
        if (count >= sizeof(types) / sizeof(types[0])) return 0;
        types[count++] = partial_operand_type(child);
    }
    name = malloc(node->node_string_length + 1u);
    if (!name) return 0;
    memcpy(name, node->node_string, node->node_string_length);
    name[node->node_string_length] = 0;
    for (cursor = name; *cursor; cursor++) *cursor = (char)tolower((unsigned char)*cursor);
    instruction = (const OpInfo *)src_instv(name, types, count);
    free(name);
    if (!instruction) return 0;
    if (instruction_out) *instruction_out = instruction;
    return 1;
}

static int partial_get_operand(RxcpPartialState *state,
                               ASTNode *node,
                               RxcpPartialValue *value_out,
                               RxcpPartialSlot **slot_out) {
    RxcpPartialSlot *slot;

    if (slot_out) *slot_out = 0;
    if (!node) return 0;
    slot = partial_slot(state, partial_node_symbol(node), 0);
    if (slot) {
        if (slot_out) *slot_out = slot;
        return partial_value_copy(value_out, &slot->value);
    }
    return partial_eval_expr(state, node, value_out);
}

static int partial_exec_assembler(RxcpPartialState *state, ASTNode *node) {
    const OpInfo *instruction;
    RxOpEffects effects;
    ASTNode *operand[3];
    RxcpPartialValue value1;
    RxcpPartialValue value2;
    RxcpPartialValue value3;
    RxcpPartialSlot *slot1;
    RxcpPartialSlot *slot2;
    size_t char_count;
    rxinteger count;
    int codepoint;
    int want_blank;
    int is_ascii;
    int found;
    size_t index;
    int ok;

    partial_value_init(&value1);
    partial_value_init(&value2);
    partial_value_init(&value3);
    slot1 = 0;
    slot2 = 0;
    ok = 0;
    if (!partial_assembler_instruction(node, &instruction)) goto done;
    effects = rxop_effects(instruction->opcode);
    if (effects.state != RXOP_EFFECT_CLASSIFIED ||
        effects.const_evaluator == RXOP_CONST_EVAL_NONE) goto done;
    operand[0] = node->child;
    operand[1] = operand[0] ? operand[0]->sibling : 0;
    operand[2] = operand[1] ? operand[1]->sibling : 0;
    if (operand[2] && operand[2]->sibling) goto done;
    if (!operand[0] || !operand[1] ||
        !partial_get_operand(state, operand[0], &value1, &slot1) ||
        !partial_get_operand(state, operand[1], &value2, &slot2)) goto done;
    if (operand[2] && !partial_get_operand(state, operand[2], &value3, 0)) goto done;
    if (!slot1) goto done;

    switch (effects.const_evaluator) {
        case RXOP_CONST_EVAL_STRLEN:
            if (value2.type != TP_STRING ||
                !partial_set_default(&slot1->value, TP_INTEGER)) goto done;
#ifdef NUTF8
            char_count = value2.string.string_length;
#else
            char_count = value2.string.string_chars;
#endif
            if (char_count > (size_t)RXINTEGER_MAX) goto done;
            slot1->value.integer = (rxinteger)char_count;
            ok = 1;
            break;

        case RXOP_CONST_EVAL_SETSTRPOS:
            if (slot1->value.type != TP_STRING ||
                (value2.type != TP_INTEGER && value2.type != TP_BOOLEAN)) goto done;
            string_set_byte_pos(&slot1->value.string, (size_t)value2.integer);
            ok = 1;
            break;

        case RXOP_CONST_EVAL_GETSTRPOS:
            if (!slot2 || slot2->value.type != TP_STRING ||
                !partial_set_default(&slot1->value, TP_INTEGER)) goto done;
#ifdef NUTF8
            slot1->value.integer = (rxinteger)slot2->value.string.string_pos;
#else
            slot1->value.integer = (rxinteger)slot2->value.string.string_char_pos;
#endif
            ok = 1;
            break;

        case RXOP_CONST_EVAL_STRCHAR_AT:
            if (value2.type != TP_STRING || value3.type != TP_INTEGER || !slot2 ||
                !partial_set_default(&slot1->value, TP_INTEGER)) goto done;
            string_set_byte_pos(&slot2->value.string, (size_t)value3.integer);
#ifdef NUTF8
            if (slot2->value.string.string_pos >= slot2->value.string.string_length) goto done;
            codepoint = (unsigned char)slot2->value.string.string_value[slot2->value.string.string_pos];
#else
            if (slot2->value.string.string_char_pos >= slot2->value.string.string_chars) goto done;
            utf8codepoint(slot2->value.string.string_value + slot2->value.string.string_pos,
                          &codepoint);
#endif
            slot1->value.integer = (rxinteger)codepoint;
            ok = 1;
            break;

        case RXOP_CONST_EVAL_SUBSTRING:
            if (value2.type != TP_STRING || value3.type != TP_INTEGER || !slot2 ||
                !partial_set_default(&slot1->value, TP_STRING)) goto done;
            count = value3.integer;
            if (count <= 0) {
                if (set_string_validated(&slot1->value.string, "", 0u) != 0) goto done;
            } else {
                string_slice_from_cursor(&slot1->value.string,
                                         &slot2->value.string,
                                         (size_t)count);
            }
            ok = 1;
            break;

        case RXOP_CONST_EVAL_PADSTR:
            if (slot1->value.type != TP_STRING || value2.type != TP_INTEGER ||
                value3.type != TP_INTEGER || value3.integer < 0 ||
                (uint64_t)value3.integer > RXCP_PARTIAL_MAX_RESULT_CODEPOINTS ||
#ifdef NUTF8
                slot1->value.string.string_length >
                    RXCP_PARTIAL_MAX_RESULT_CODEPOINTS - (size_t)value3.integer
#else
                slot1->value.string.string_chars >
                    RXCP_PARTIAL_MAX_RESULT_CODEPOINTS - (size_t)value3.integer
#endif
                ) goto done;
            count = value3.integer;
            for (index = 0; index < (size_t)count; index++) {
                value char_value;
                value_init(&char_value);
                char_value.int_value = value2.integer;
                string_concat_char(&slot1->value.string, &char_value);
                clear_value(&char_value);
            }
            ok = 1;
            break;

        case RXOP_CONST_EVAL_FNDBLNK:
        case RXOP_CONST_EVAL_FNDNBLNK:
            if (value2.type != TP_STRING || value3.type != TP_INTEGER || !slot2 ||
                value3.integer < 0 || !partial_set_default(&slot1->value, TP_INTEGER)) goto done;
#ifdef NUTF8
            char_count = slot2->value.string.string_length;
            is_ascii = 1;
#else
            char_count = slot2->value.string.string_chars;
            is_ascii = slot2->value.string.string_chars == slot2->value.string.string_length;
#endif
            want_blank = effects.const_evaluator == RXOP_CONST_EVAL_FNDBLNK;
            found = 0;
            count = value3.integer;
            for (index = (size_t)count; index < char_count; index++) {
#ifdef NUTF8
                codepoint = (unsigned char)slot2->value.string.string_value[index];
#else
                if (!is_ascii) string_set_byte_pos(&slot2->value.string, index);
                if (is_ascii) {
                    codepoint = (unsigned char)slot2->value.string.string_value[index];
                } else {
                    utf8codepoint(slot2->value.string.string_value +
                                  slot2->value.string.string_pos,
                                  &codepoint);
                }
#endif
                if (((codepoint == 0x0009 || codepoint == 0x000a ||
                      codepoint == 0x000b || codepoint == 0x000c ||
                      codepoint == 0x000d || codepoint == 0x0020 ||
                      codepoint == 0x0085 || codepoint == 0x00a0 ||
                      codepoint == 0x1680 ||
                      (codepoint >= 0x2000 && codepoint <= 0x200a) ||
                      codepoint == 0x2028 || codepoint == 0x2029 ||
                      codepoint == 0x202f || codepoint == 0x205f ||
                      codepoint == 0x3000) != 0) == want_blank) {
                    slot1->value.integer = (rxinteger)index;
                    found = 1;
                    break;
                }
            }
            if (!found) slot1->value.integer = -(rxinteger)char_count;
            ok = 1;
            break;

        case RXOP_CONST_EVAL_SCOPY:
            if (!slot2 || value2.type != TP_STRING || slot1->value.type != TP_STRING) goto done;
            set_value_string(&slot1->value.string, &slot2->value.string);
            ok = 1;
            break;

        case RXOP_CONST_EVAL_APPEND:
            if (!slot2 || value2.type != TP_STRING || slot1->value.type != TP_STRING) goto done;
            if (slot2->value.string.string_length >
                    RXCP_PARTIAL_MAX_STRING_BYTES - slot1->value.string.string_length) goto done;
#ifndef NUTF8
            if (slot2->value.string.string_chars >
                    RXCP_PARTIAL_MAX_RESULT_CODEPOINTS - slot1->value.string.string_chars) goto done;
#endif
            string_append(&slot1->value.string, &slot2->value.string);
            ok = 1;
            break;

        case RXOP_CONST_EVAL_STRLOWER:
        case RXOP_CONST_EVAL_STRUPPER:
            if (!slot2 || value2.type != TP_STRING || slot1->value.type != TP_STRING) goto done;
            set_value_string(&slot1->value.string, &slot2->value.string);
#ifdef NUTF8
            for (index = 0; index < slot1->value.string.string_length; index++) {
                slot1->value.string.string_value[index] =
                    (char)(effects.const_evaluator == RXOP_CONST_EVAL_STRUPPER ?
                           toupper((unsigned char)slot1->value.string.string_value[index]) :
                           tolower((unsigned char)slot1->value.string.string_value[index]));
            }
#else
            {
                char *current = slot1->value.string.string_value;
                char *next;
                char *end = current + slot1->value.string.string_length;
                utf8_int32_t source_codepoint;
                utf8_int32_t mapped;
                while (current < end) {
                    next = utf8codepoint(current, &source_codepoint);
                    mapped = effects.const_evaluator == RXOP_CONST_EVAL_STRUPPER ?
                             utf8uprcodepoint(source_codepoint) :
                             utf8lwrcodepoint(source_codepoint);
                    if (mapped != source_codepoint) {
                        utf8catcodepoint(current, mapped, (size_t)(next - current));
                    }
                    current = next;
                }
            }
#endif
            ok = 1;
            break;

        default:
            break;
    }

    if (ok && !partial_value_within_budget(&slot1->value)) ok = 0;

done:
    partial_value_clear(&value3);
    partial_value_clear(&value2);
    partial_value_clear(&value1);
    return ok;
}

static int partial_exec_statement(RxcpPartialState *state, ASTNode *node);

static int partial_exec_sequence(RxcpPartialState *state, ASTNode *node) {
    ASTNode *statement;

    if (!state || !node) return 0;
    for (statement = node->child; statement && !state->returned;
         statement = statement->sibling) {
        if (!partial_exec_statement(state, statement)) return 0;
    }
    return 1;
}

static int partial_exec_statement(RxcpPartialState *state, ASTNode *node) {
    ASTNode *lhs;
    ASTNode *rhs;
    ASTNode *selected;
    RxcpPartialSlot *slot;
    RxcpPartialValue value;
    int ok;

    if (!state || !node || ++state->steps > RXCP_PARTIAL_MAX_STEPS) return 0;
    partial_value_init(&value);
    ok = 0;
    switch (node->node_type) {
        case NOP:
            ok = 1;
            break;

        case INSTRUCTIONS:
            ok = partial_exec_sequence(state, node);
            break;

        case ASSIGN:
        case DEFINE:
            lhs = node->child;
            rhs = lhs ? lhs->sibling : 0;
            slot = partial_slot(state, partial_node_symbol(lhs), 1);
            if (!lhs || !rhs || rhs->sibling || !slot ||
                !partial_eval_expr(state, rhs, &value) ||
                !partial_value_copy(&slot->value, &value)) break;
            ok = 1;
            break;

        case IF:
            if (!node->child || !partial_eval_expr(state, node->child, &value) ||
                (value.type != TP_BOOLEAN && value.type != TP_INTEGER)) break;
            selected = value.integer ? node->child->sibling :
                       (node->child->sibling ? node->child->sibling->sibling : 0);
            ok = !selected || selected->node_type == NOP ||
                 partial_exec_statement(state, selected);
            break;

        case DO:
            {
                ASTNode *repeat = node->child;
                ASTNode *body = repeat ? repeat->sibling : 0;
                ASTNode *while_node = repeat && repeat->node_type == REPEAT ?
                                      repeat->child : 0;
                if (!repeat || !body || body->sibling ||
                    !while_node || while_node->node_type != WHILE ||
                    !while_node->child || while_node->child->sibling) break;
                ok = 1;
                while (!state->returned) {
                    RxcpPartialValue condition;
                    partial_value_init(&condition);
                    if (!partial_eval_expr(state, while_node->child, &condition) ||
                        (condition.type != TP_BOOLEAN && condition.type != TP_INTEGER)) {
                        partial_value_clear(&condition);
                        ok = 0;
                        break;
                    }
                    if (!condition.integer) {
                        partial_value_clear(&condition);
                        break;
                    }
                    partial_value_clear(&condition);
                    if (!partial_exec_statement(state, body)) {
                        ok = 0;
                        break;
                    }
                    if (state->steps > RXCP_PARTIAL_MAX_STEPS) {
                        ok = 0;
                        break;
                    }
                }
            }
            break;

        case ASSEMBLER:
            ok = partial_exec_assembler(state, node);
            break;

        case RETURN:
            if (!node->child || node->child->sibling ||
                !partial_eval_expr(state, node->child, &value) ||
                !partial_value_copy(&state->return_value, &value)) break;
            state->returned = 1;
            ok = 1;
            break;

        default:
            break;
    }
    partial_value_clear(&value);
    return ok;
}

static int partial_bind_formals(RxcpPartialState *state,
                                ASTNode *procedure,
                                ASTNode *call) {
    ASTNode *args;
    ASTNode *formal;
    ASTNode *actual;

    args = ast_chld(procedure, ARGS, 0);
    formal = args ? args->child : 0;
    actual = call ? call->child : 0;
    while (formal) {
        ASTNode *target;
        ASTNode *default_value;
        RxcpPartialSlot *slot;
        RxcpPartialValue bound;
        int provided;

        if (formal->node_type != ARG || !actual || formal->is_ref_arg) return 0;
        target = formal->child;
        default_value = target ? target->sibling : 0;
        slot = partial_slot(state, partial_node_symbol(target), 1);
        if (!target || target->is_ref_arg || !slot ||
            !slot->symbol || slot->symbol->is_ref_arg) return 0;
        partial_value_init(&bound);
        provided = actual->node_type != NOVAL;
        if (provided) {
            if (!partial_eval_expr(state, actual, &bound)) {
                partial_value_clear(&bound);
                return 0;
            }
        } else {
            if (!formal->is_opt_arg || !default_value ||
                !partial_eval_expr(state, default_value, &bound)) {
                partial_value_clear(&bound);
                return 0;
            }
        }
        if (!partial_value_copy(&slot->value, &bound)) {
            partial_value_clear(&bound);
            return 0;
        }
        slot->provided = provided;
        partial_value_clear(&bound);
        formal = formal->sibling;
        actual = actual->sibling;
    }
    return actual == 0;
}

static int partial_result_export(const RxcpPartialValue *value,
                                 RxcpPartialCallResult *result) {
    size_t encoded_length;

    if (!value || !partial_value_within_budget(value) || !result) return 0;
    result->type = value->type;
    if (value->type == TP_INTEGER || value->type == TP_BOOLEAN) {
        result->int_value = value->integer;
        return 1;
    }
    if (value->type != TP_STRING) return 0;
    result->string_value = rxcp_constant_string_encode(
            (const unsigned char *)(value->string.string_value ?
                                    value->string.string_value : ""),
            value->string.string_length,
            &encoded_length);
    if (!result->string_value) return 0;
    result->string_length = encoded_length;
    return 1;
}

static void partial_state_clear(RxcpPartialState *state) {
    size_t index;

    if (!state) return;
    for (index = 0; index < state->slot_count; index++) {
        partial_value_clear(&state->slots[index].value);
    }
    partial_value_clear(&state->return_value);
}

int rxcp_partial_call_evaluate(Context *context,
                               ASTNode *call,
                               RxcpPartialCallResult *result) {
    RxcpPartialState state;
    Symbol *symbol;
    ASTNode *procedure;
    ASTNode *body;
    int ok;

    if (!result) return 0;
    memset(result, 0, sizeof(*result));
    if (!context || !call || call->node_type != FUNCTION ||
        !call->symbolNode || !(symbol = call->symbolNode->symbol) ||
        !(procedure = partial_callable_template(symbol)) ||
        ast_chld(procedure, EXPOSED, 0) != 0 ||
        procedure->value_dims != 0 ||
        !partial_scalar_type_supported(procedure->value_type)) return 0;

    memset(&state, 0, sizeof(state));
    state.context = context;
    state.call = call;
    state.callable_scope = procedure->scope;
    partial_value_init(&state.return_value);
    ok = state.callable_scope && partial_bind_formals(&state, procedure, call);
    body = ok ? ast_chld(procedure, INSTRUCTIONS, NOP) : 0;
    if (!body || body->node_type == NOP ||
        !partial_exec_sequence(&state, body) || !state.returned ||
        !partial_result_export(&state.return_value, result)) {
        rxcp_partial_call_result_clear(result);
        ok = 0;
    } else {
        ok = 1;
        if (context->debug_mode >= 1) {
            char *name = sym_frnm(symbol);
            fprintf(stderr, "DEBUG_PARTIAL_CALL %s @ %s:%d:%d - fold: body-driven constant result\n",
                    name ? name : symbol->name,
                    call->file_name ? call->file_name : "<unknown>",
                    call->line > 0 ? call->line : 0,
                    call->column > 0 ? call->column : 0);
            free(name);
        }
    }
    partial_state_clear(&state);
    return ok;
}

void rxcp_partial_call_result_clear(RxcpPartialCallResult *result) {
    if (!result) return;
    free(result->decimal_value);
    free(result->string_value);
    memset(result, 0, sizeof(*result));
}
