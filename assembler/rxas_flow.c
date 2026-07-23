/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
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

/* RXAS transient whole-procedure machine-flow analysis. */

#include "rxasassm.h"
#include "rxdefs.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLOW_WORD_BITS (sizeof(unsigned long) * 8)
/* Bound global value analysis only for procedures newly admitted by exact
 * indirect-table edges.  Reachability remains linear and unbounded. */
#define FLOW_MAX_INDIRECT_VALUE_CELLS 1000000

enum flow_view {
    FLOW_VIEW_INTEGER,
    FLOW_VIEW_FLOAT,
    FLOW_VIEW_STRING,
    FLOW_VIEW_DECIMAL,
    FLOW_VIEW_BINARY,
    FLOW_VIEW_ATTRIBUTES,
    FLOW_VIEW_REFERENCE,
    FLOW_VIEW_COUNT
};

#define FLOW_VIEW_BIT(VIEW) (1u << (VIEW))
#define FLOW_ALL_VIEWS ((1u << FLOW_VIEW_COUNT) - 1u)

typedef struct flow_register {
    char type;
    size_t number;
    Assembler_Token *token;
} flow_register;

typedef struct flow_node {
    const OpInfo *op;
    RxOpEffects effects;
    size_t *successors;
    size_t successor_count;
    size_t successor_capacity;
    size_t block;
    int unknown_successor;
    int reachable;
    unsigned long *uses;
    unsigned long *kills;
    unsigned long *live_in;
    unsigned long *live_out;
} flow_node;

typedef struct flow_graph {
    Assembler_Context *context;
    instruction_queue *items;
    size_t item_count;
    flow_node *nodes;
    flow_register *registers;
    size_t register_count;
    size_t register_capacity;
    size_t bit_count;
    size_t word_count;
    unsigned long *bit_storage;
    size_t *predecessor_offsets;
    size_t *predecessors;
    size_t *async_handler_targets;
    size_t async_handler_target_count;
    size_t async_handler_target_capacity;
    unsigned char *tainted_registers;
    size_t block_count;
    size_t resolved_indirect_branches;
    int complete_control_flow;
} flow_graph;

typedef struct flow_stats {
    size_t procedures;
    size_t blocks;
    size_t unreachable_removed;
    size_t dead_results_removed;
    size_t typed_copies_removed;
    size_t compare_preparations_removed;
    size_t full_copies_removed;
    size_t redundant_loads_removed;
    size_t redundant_initializations_removed;
    size_t redundant_conversions_removed;
    size_t producer_destinations_forwarded;
    size_t operands_redirected;
    size_t rejected_live;
    size_t rejected_trace;
    size_t rejected_tainted;
    size_t rejected_effect;
} flow_stats;

static OperandType flow_operand_type(Assembler_Token *token) {
    if (!token) return OP_NONE;
    switch (token->token_type) {
        case ID: return OP_ID;
        case RREG:
        case GREG:
        case AREG: return OP_REG;
        case FUNC: return OP_FUNC;
        case INT: return OP_INT;
        case FLOAT: return OP_FLOAT;
        case CHAR: return OP_CHAR;
        case STRING: return OP_STRING;
        case DECIMAL: return OP_DECIMAL;
        case HEX: return OP_BINARY;
        default: return OP_NONE;
    }
}

static int flow_mnemonic_matches(const char *mnemonic, const char *table_name) {
    size_t index;
    if (!mnemonic || !table_name) return 0;
    index = 0;
    while (mnemonic[index]) {
        if (toupper((unsigned char)mnemonic[index]) != table_name[index]) return 0;
        index++;
    }
    return table_name[index] == 0 || table_name[index] == '_';
}

static const OpInfo *flow_find_opcode(Assembler_Context *context,
                                      const instruction_queue *item) {
    const char *mnemonic;
    size_t operand_index;
    int table_index;
    int matches;

    if (!item || item->instrType != OP_CODE || !item->instrToken) return 0;
    mnemonic = (const char *)item->instrToken->token_value.string;
    for (table_index = 0; op_table[table_index].mnemonic; table_index++) {
        if (!rxop_is_source_mnemonic(op_table[table_index].mnemonic)) continue;
        if (!flow_mnemonic_matches(mnemonic, op_table[table_index].mnemonic)) continue;
        if (rxop_format_operand_count(op_table[table_index].format) != item->operandCount) continue;
        matches = 1;
        for (operand_index = 0; operand_index < item->operandCount; operand_index++) {
            OperandType expected;
            OperandType actual;
            Assembler_Token *operand;
            size_t jump_table_cases;
            expected = rxop_format_operand_type(op_table[table_index].format,
                                                operand_index);
            operand = rxas_queue_operand(item, operand_index);
            actual = flow_operand_type(operand);
            if (expected != actual &&
                !(expected == OP_BINARY && actual == OP_ID &&
                  rxas_jump_table_case_count(context, operand,
                                             &jump_table_cases))) {
                matches = 0;
                break;
            }
        }
        if (matches) return &op_table[table_index];
    }
    return 0;
}

static char flow_register_type(Assembler_Token *token) {
    if (!token) return 0;
    switch (token->token_type) {
        case RREG: return 'r';
        case AREG: return 'a';
        case GREG: return 'g';
        default: return 0;
    }
}

static int flow_register_index(const flow_graph *graph, char type, size_t number) {
    size_t index;
    for (index = 0; index < graph->register_count; index++) {
        if (graph->registers[index].type == type &&
            graph->registers[index].number == number) return (int)index;
    }
    return -1;
}

static int flow_add_register(flow_graph *graph, Assembler_Token *token) {
    char type;
    size_t number;
    size_t new_capacity;
    flow_register *new_registers;
    int existing;

    type = flow_register_type(token);
    if (!type || token->token_value.integer < 0) return -1;
    number = (size_t)token->token_value.integer;
    existing = flow_register_index(graph, type, number);
    if (existing >= 0) {
        if (!graph->registers[existing].token) graph->registers[existing].token = token;
        return existing;
    }
    if (graph->register_count == graph->register_capacity) {
        new_capacity = graph->register_capacity ? graph->register_capacity * 2 : 32;
        new_registers = realloc(graph->registers,
                                new_capacity * sizeof(*new_registers));
        if (!new_registers) {
            RX_PANIC_OOM("realloc RXAS flow register universe",
                         new_capacity * sizeof(*new_registers),
                         graph->context && graph->context->file_name
                                 ? graph->context->file_name : 0);
        }
        graph->registers = new_registers;
        graph->register_capacity = new_capacity;
    }
    graph->registers[graph->register_count].type = type;
    graph->registers[graph->register_count].number = number;
    graph->registers[graph->register_count].token = token;
    graph->register_count++;
    return (int)(graph->register_count - 1);
}

static void flow_add_numbered_register(flow_graph *graph, char type, size_t number) {
    size_t new_capacity;
    flow_register *new_registers;
    if (flow_register_index(graph, type, number) >= 0) return;
    if (graph->register_count == graph->register_capacity) {
        new_capacity = graph->register_capacity ? graph->register_capacity * 2 : 32;
        new_registers = realloc(graph->registers,
                                new_capacity * sizeof(*new_registers));
        if (!new_registers) {
            RX_PANIC_OOM("realloc RXAS implicit flow register universe",
                         new_capacity * sizeof(*new_registers),
                         graph->context && graph->context->file_name
                                 ? graph->context->file_name : 0);
        }
        graph->registers = new_registers;
        graph->register_capacity = new_capacity;
    }
    graph->registers[graph->register_count].type = type;
    graph->registers[graph->register_count].number = number;
    graph->registers[graph->register_count].token = 0;
    graph->register_count++;
}

static unsigned int flow_read_views(int opcode, size_t operand_index) {
    if (opcode == OP_COPY_REG_REG && operand_index == 1) return FLOW_ALL_VIEWS;
    if (opcode == OP_ICOPY_REG_REG && operand_index == 1) return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if (opcode == OP_FCOPY_REG_REG && operand_index == 1) return FLOW_VIEW_BIT(FLOW_VIEW_FLOAT);
    if (opcode == OP_SCOPY_REG_REG && operand_index == 1) return FLOW_VIEW_BIT(FLOW_VIEW_STRING);
    if (opcode == OP_DCOPY_REG_REG && operand_index == 1) return FLOW_VIEW_BIT(FLOW_VIEW_DECIMAL);
    if (opcode == OP_ACOPY_REG_REG && operand_index == 1) return FLOW_VIEW_BIT(FLOW_VIEW_ATTRIBUTES);
    if (opcode == OP_BCOPY_REG_REG && operand_index == 1) return FLOW_VIEW_BIT(FLOW_VIEW_BINARY);

    if (opcode >= OP_IADD_REG_REG_REG && opcode <= OP_DEC_REG)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if (opcode >= OP_IEQ_REG_REG_REG && opcode <= OP_ILTE_REG_INT_REG)
        return operand_index == 0 ? FLOW_ALL_VIEWS : FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if (opcode == OP_BRT_ID_REG || opcode == OP_BRF_ID_REG || opcode == OP_BRTF_ID_ID_REG ||
        opcode == OP_BEQ_ID_REG_REG || opcode == OP_BEQ_ID_REG_INT ||
        opcode == OP_BNE_ID_REG_REG || opcode == OP_BNE_ID_REG_INT)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if ((opcode == OP_SEQ_REG_REG_REG || opcode == OP_SEQ_REG_REG_STRING ||
         (opcode >= OP_SNE_REG_REG_REG && opcode <= OP_SLTE_REG_STRING_REG)) &&
        operand_index != 0)
        return FLOW_VIEW_BIT(FLOW_VIEW_STRING);
    if (opcode >= OP_FEQ_REG_REG_REG && opcode <= OP_FLTE_REG_FLOAT_REG)
        return operand_index == 0 ? FLOW_ALL_VIEWS : FLOW_VIEW_BIT(FLOW_VIEW_FLOAT);
    if (opcode >= OP_DEQ_REG_REG_REG && opcode <= OP_DLTE_REG_DECIMAL_REG)
        return operand_index == 0 ? FLOW_ALL_VIEWS : FLOW_VIEW_BIT(FLOW_VIEW_DECIMAL);
    if (opcode == OP_BINEQ_REG_REG_REG || opcode == OP_BINEQ_REG_REG_BINARY ||
        opcode == OP_BINNE_REG_REG_REG || opcode == OP_BINNE_REG_REG_BINARY)
        return operand_index == 0 ? FLOW_ALL_VIEWS : FLOW_VIEW_BIT(FLOW_VIEW_BINARY);
    if (opcode == OP_ITOF_REG || opcode == OP_ITOB_REG)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if (opcode == OP_FTOI_REG || opcode == OP_FTOB_REG)
        return FLOW_VIEW_BIT(FLOW_VIEW_FLOAT);
    if (opcode == OP_ITOF_REG_REG && operand_index == 1)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    return FLOW_ALL_VIEWS;
}

/* A zero result means the effects inventory proves a write, but NR-27 does not
 * yet have a component-exact kill for it. This distinction is essential: the
 * canonical kills bit is not a whole multi-view value kill. */
static unsigned int flow_precise_write_views(int opcode, size_t operand_index) {
    if (operand_index != 0) return 0;
    if (opcode == OP_COPY_REG_REG || opcode == OP_NULL_REG) return FLOW_ALL_VIEWS;
    if (opcode == OP_ICOPY_REG_REG || opcode == OP_LOAD_REG_INT)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if (opcode == OP_FCOPY_REG_REG || opcode == OP_LOAD_REG_FLOAT)
        return FLOW_VIEW_BIT(FLOW_VIEW_FLOAT);
    if (opcode == OP_SCOPY_REG_REG || opcode == OP_LOAD_REG_STRING)
        return FLOW_VIEW_BIT(FLOW_VIEW_STRING);
    if (opcode == OP_DCOPY_REG_REG || opcode == OP_LOAD_REG_DECIMAL)
        return FLOW_VIEW_BIT(FLOW_VIEW_DECIMAL);
    if (opcode == OP_ACOPY_REG_REG) return FLOW_VIEW_BIT(FLOW_VIEW_ATTRIBUTES);
    if (opcode == OP_BCOPY_REG_REG || opcode == OP_LOAD_REG_BINARY)
        return FLOW_VIEW_BIT(FLOW_VIEW_BINARY);
    if (opcode >= OP_IADD_REG_REG_REG && opcode <= OP_DEC_REG)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if (opcode >= OP_IEQ_REG_REG_REG && opcode <= OP_SLTE_REG_STRING_REG)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if (opcode >= OP_FEQ_REG_REG_REG && opcode <= OP_FLTE_REG_FLOAT_REG)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if (opcode >= OP_DEQ_REG_REG_REG && opcode <= OP_DLTE_REG_DECIMAL_REG)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if (opcode == OP_BINEQ_REG_REG_REG || opcode == OP_BINEQ_REG_REG_BINARY ||
        opcode == OP_BINNE_REG_REG_REG || opcode == OP_BINNE_REG_REG_BINARY)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if (opcode == OP_ITOF_REG) return FLOW_VIEW_BIT(FLOW_VIEW_FLOAT);
    if (opcode == OP_FTOI_REG || opcode == OP_FTOB_REG || opcode == OP_ITOB_REG)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if (opcode == OP_ITOF_REG_REG)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER) | FLOW_VIEW_BIT(FLOW_VIEW_FLOAT);
    return 0;
}

static void flow_set_bit(unsigned long *bits, size_t bit) {
    bits[bit / FLOW_WORD_BITS] |= 1ul << (bit % FLOW_WORD_BITS);
}

static int flow_test_bit(const unsigned long *bits, size_t bit) {
    return (bits[bit / FLOW_WORD_BITS] & (1ul << (bit % FLOW_WORD_BITS))) != 0;
}

static void flow_set_register_views(const flow_graph *graph, unsigned long *bits,
                                    int register_index, unsigned int views) {
    size_t view;
    if (register_index < 0) return;
    for (view = 0; view < FLOW_VIEW_COUNT; view++) {
        if (views & FLOW_VIEW_BIT(view))
            flow_set_bit(bits, (size_t)register_index * FLOW_VIEW_COUNT + view);
    }
}

static void flow_set_all_bits(const flow_graph *graph, unsigned long *bits) {
    size_t word;
    for (word = 0; word < graph->word_count; word++) bits[word] = ~0ul;
    if (graph->bit_count % FLOW_WORD_BITS) {
        bits[graph->word_count - 1] &=
                (1ul << (graph->bit_count % FLOW_WORD_BITS)) - 1ul;
    }
}

static int flow_token_is_string(Assembler_Token *token, const char *text) {
    return token && token->token_type == STRING &&
           strcmp((const char *)token->token_value.string, text) == 0;
}

static void flow_collect_registers(flow_graph *graph) {
    size_t item_index;
    size_t operand_index;
    size_t local_index;
    instruction_queue *item;
    const OpInfo *op;
    RxOpEffects effects;
    Assembler_Token *token;

    for (local_index = 0;
         local_index < (size_t)(graph->context->current_locals > 0
                 ? graph->context->current_locals : 0);
         local_index++) flow_add_numbered_register(graph, 'r', local_index);
    for (local_index = 0;
         local_index < (size_t)(graph->context->binary.globals > 0
                 ? graph->context->binary.globals : 0);
         local_index++) flow_add_numbered_register(graph, 'g', local_index);

    for (item_index = 0; item_index < graph->item_count; item_index++) {
        item = &graph->items[item_index];
        if (item->instrType == OP_CODE) {
            for (operand_index = 0; operand_index < item->operandCount; operand_index++)
                flow_add_register(graph, rxas_queue_operand(item, operand_index));
            op = flow_find_opcode(graph->context, item);
            if (!op) continue;
            effects = rxop_effects(op->opcode);
            if ((effects.implicit == RXOP_IMPLICIT_LOCAL_COPY ||
                 effects.implicit == RXOP_IMPLICIT_LOCAL_TARGET) &&
                item->operand1Token && item->operand1Token->token_type == INT &&
                item->operand1Token->token_value.integer >= 0)
                flow_add_numbered_register(graph, 'r',
                        (size_t)item->operand1Token->token_value.integer);
            if (effects.implicit == RXOP_IMPLICIT_LOCAL_COPY &&
                item->operand2Token && item->operand2Token->token_type == INT &&
                item->operand2Token->token_value.integer >= 0)
                flow_add_numbered_register(graph, 'r',
                        (size_t)item->operand2Token->token_value.integer);
            if (effects.implicit == RXOP_IMPLICIT_ARGUMENT_INDEX &&
                item->operand2Token && item->operand2Token->token_type == INT &&
                item->operand2Token->token_value.integer >= 0)
                flow_add_numbered_register(graph, 'a',
                        (size_t)item->operand2Token->token_value.integer);
        }
        else if (item->instrType == REG_META) {
            flow_add_register(graph, item->operand3Token);
        }
        else if (item->instrType == TRACE_EVENT &&
                 flow_token_is_string(item->operand2Token, "R") &&
                 item->operand4Token && item->operand4Token->token_type == STRING &&
                 item->operand4Token->token_value.string[0] &&
                 item->operand5Token && item->operand5Token->token_type == INT &&
                 item->operand5Token->token_value.integer >= 0) {
            char register_type;
            register_type = (char)tolower((unsigned char)item->operand4Token->token_value.string[0]);
            token = 0;
            flow_add_numbered_register(graph, register_type,
                    (size_t)item->operand5Token->token_value.integer);
            (void)token;
        }
    }
}

static int flow_label_index(const flow_graph *graph, Assembler_Token *token) {
    size_t index;
    if (!token || (token->token_type != ID && token->token_type != LABEL)) return -1;
    for (index = 0; index < graph->item_count; index++) {
        if (graph->items[index].instrType == ASM_LABEL &&
            graph->items[index].instrToken &&
            graph->items[index].instrToken->token_type == LABEL &&
            strcmp((const char *)graph->items[index].instrToken->token_value.string,
                   (const char *)token->token_value.string) == 0)
            return (int)index;
    }
    return -1;
}

static void flow_add_successor(flow_graph *graph, flow_node *node,
                               size_t successor) {
    size_t index;
    size_t new_capacity;
    size_t *new_successors;
    for (index = 0; index < node->successor_count; index++)
        if (node->successors[index] == successor) return;
    if (node->successor_count == node->successor_capacity) {
        new_capacity = node->successor_capacity
                ? node->successor_capacity * 2 : 4;
        new_successors = realloc(node->successors,
                                 new_capacity * sizeof(*new_successors));
        if (!new_successors)
            RX_PANIC_OOM("realloc RXAS flow successors",
                         new_capacity * sizeof(*new_successors),
                         graph->context && graph->context->file_name
                                 ? graph->context->file_name : 0);
        node->successors = new_successors;
        node->successor_capacity = new_capacity;
    }
    node->successors[node->successor_count++] = successor;
}

static Assembler_Token *flow_jump_table_operand(const flow_node *node,
                                                const instruction_queue *item) {
    size_t operand_index;
    if (!node || !node->op || !item) return 0;
    switch (node->op->opcode) {
        case OP_JUMPS_REG_BINARY:
        case OP_JUMPB_REG_BINARY:
        case OP_JUMPI_REG_BINARY:
        case OP_JUMPR_REG_BINARY:
        case OP_JUMPN_REG_BINARY:
            operand_index = 1;
            break;
        case OP_JUMPBS_REG_REG_BINARY:
            operand_index = 2;
            break;
        default:
            return 0;
    }
    return rxas_queue_operand(item, operand_index);
}

static int flow_add_jump_table_successors(flow_graph *graph, flow_node *node,
                                          instruction_queue *item,
                                          unsigned char *leaders) {
    Assembler_Token *table;
    Assembler_Token *label;
    size_t case_count;
    size_t case_index;
    int label_index;

    table = flow_jump_table_operand(node, item);
    if (!table ||
        !rxas_jump_table_case_count(graph->context, table, &case_count))
        return 0;
    for (case_index = 0; case_index < case_count; case_index++) {
        label = rxas_jump_table_case_label(graph->context, table, case_index);
        label_index = flow_label_index(graph, label);
        if (label_index < 0) return 0;
        flow_add_successor(graph, node, (size_t)label_index);
        leaders[label_index] = 1;
    }
    return 1;
}

static int flow_build_edges(flow_graph *graph) {
    size_t index;
    size_t operand_index;
    size_t target_count;
    size_t edge_count;
    size_t predecessor_index;
    size_t *fill;
    unsigned char *leaders;
    int label_index;
    flow_node *node;
    instruction_queue *item;

    leaders = calloc(graph->item_count ? graph->item_count : 1, 1);
    if (!leaders) RX_PANIC_OOM("calloc RXAS flow leaders", graph->item_count, 0);
    if (graph->item_count) leaders[0] = 1;
    edge_count = 0;
    graph->complete_control_flow = 1;

    for (index = 0; index < graph->item_count; index++) {
        node = &graph->nodes[index];
        item = &graph->items[index];
        if (item->instrType != OP_CODE) {
            if (index + 1 < graph->item_count)
                flow_add_successor(graph, node, index + 1);
            continue;
        }
        node->op = flow_find_opcode(graph->context, item);
        if (!node->op) {
            graph->complete_control_flow = 0;
            node->unknown_successor = 1;
            if (index + 1 < graph->item_count)
                flow_add_successor(graph, node, index + 1);
            continue;
        }
        node->effects = rxop_effects(node->op->opcode);
        target_count = 0;
        for (operand_index = 0; operand_index < item->operandCount; operand_index++) {
            if (!rxop_effect_branch_target_operand(&node->effects, operand_index)) continue;
            target_count++;
            label_index = flow_label_index(graph, rxas_queue_operand(item, operand_index));
            if (label_index < 0) {
                graph->complete_control_flow = 0;
                node->unknown_successor = 1;
            }
            else {
                flow_add_successor(graph, node, (size_t)label_index);
                leaders[label_index] = 1;
            }
        }
        if (node->effects.semantics & RXOP_SEM_INDIRECT_BRANCH) {
            /* Every packed-table lookup falls through on a miss.  A table at
             * the end of the retained procedure has an off-graph successor,
             * so it is not a complete procedure-local CFG. */
            if (index + 1 >= graph->item_count ||
                !flow_add_jump_table_successors(graph, node, item, leaders)) {
                node->unknown_successor = 1;
                graph->complete_control_flow = 0;
            }
            else graph->resolved_indirect_branches++;
        }
        if (node->op->flow == FLOW_NEXT ||
            (node->op->flow == FLOW_COND && target_count < 2)) {
            if (index + 1 < graph->item_count) {
                flow_add_successor(graph, node, index + 1);
                if (node->op->flow != FLOW_NEXT) leaders[index + 1] = 1;
            }
        }
        if ((node->op->flow == FLOW_JUMP || node->op->flow == FLOW_COND ||
             node->op->flow == FLOW_TERM) && index + 1 < graph->item_count)
            leaders[index + 1] = 1;
    }

    graph->block_count = 0;
    for (index = 0; index < graph->item_count; index++) {
        if (leaders[index]) graph->block_count++;
        graph->nodes[index].block = graph->block_count ? graph->block_count - 1 : 0;
        edge_count += graph->nodes[index].successor_count;
    }
    free(leaders);

    graph->predecessor_offsets = calloc(graph->item_count + 1, sizeof(size_t));
    graph->predecessors = calloc(edge_count ? edge_count : 1, sizeof(size_t));
    fill = calloc(graph->item_count ? graph->item_count : 1, sizeof(size_t));
    if (!graph->predecessor_offsets || !graph->predecessors || !fill)
        RX_PANIC_OOM("calloc RXAS flow predecessor graph",
                     (graph->item_count + edge_count) * sizeof(size_t), 0);
    for (index = 0; index < graph->item_count; index++) {
        node = &graph->nodes[index];
        for (predecessor_index = 0;
             predecessor_index < node->successor_count;
             predecessor_index++)
            graph->predecessor_offsets[node->successors[predecessor_index] + 1]++;
    }
    for (index = 1; index <= graph->item_count; index++)
        graph->predecessor_offsets[index] += graph->predecessor_offsets[index - 1];
    for (index = 0; index < graph->item_count; index++) {
        node = &graph->nodes[index];
        for (predecessor_index = 0;
             predecessor_index < node->successor_count;
             predecessor_index++) {
            size_t successor;
            successor = node->successors[predecessor_index];
            graph->predecessors[graph->predecessor_offsets[successor] + fill[successor]++] = index;
        }
    }
    free(fill);
    return 1;
}

static void flow_mark_reachable(flow_graph *graph) {
    size_t index;
    size_t successor_index;
    int changed;
    if (!graph->item_count) return;
    graph->nodes[0].reachable = 1;
    do {
        changed = 0;
        for (index = 0; index < graph->item_count; index++) {
            if (!graph->nodes[index].reachable) continue;
            if (graph->nodes[index].unknown_successor) {
                size_t all_index;
                for (all_index = 0; all_index < graph->item_count; all_index++) {
                    if (!graph->nodes[all_index].reachable) {
                        graph->nodes[all_index].reachable = 1;
                        changed = 1;
                    }
                }
            }
            for (successor_index = 0;
                 successor_index < graph->nodes[index].successor_count;
                 successor_index++) {
                size_t successor;
                successor = graph->nodes[index].successors[successor_index];
                if (!graph->nodes[successor].reachable) {
                    graph->nodes[successor].reachable = 1;
                    changed = 1;
                }
            }
        }
    } while (changed);
}

static int flow_is_async_handler_registration(int opcode) {
    return opcode == OP_SIGBR_ID_STRING || opcode == OP_SIGBRV_ID_REG_STRING ||
           opcode == OP_SIGCALLBR_ID_FUNC_STRING;
}

static void flow_add_async_handler_target(flow_graph *graph, size_t target) {
    size_t index;
    size_t new_capacity;
    size_t *new_targets;
    for (index = 0; index < graph->async_handler_target_count; index++)
        if (graph->async_handler_targets[index] == target) return;
    if (graph->async_handler_target_count == graph->async_handler_target_capacity) {
        new_capacity = graph->async_handler_target_capacity
                ? graph->async_handler_target_capacity * 2 : 4;
        new_targets = realloc(graph->async_handler_targets,
                              new_capacity * sizeof(*new_targets));
        if (!new_targets)
            RX_PANIC_OOM("realloc RXAS async-handler targets",
                         new_capacity * sizeof(*new_targets), 0);
        graph->async_handler_targets = new_targets;
        graph->async_handler_target_capacity = new_capacity;
    }
    graph->async_handler_targets[graph->async_handler_target_count++] = target;
}

/* A branch signal handler may be entered asynchronously after registration.
 * The exact enabled-handler set is runtime state, so retain a conservative
 * procedure-local target set. Dataflow treats every executable instruction as
 * a possible exceptional predecessor of these targets; ordinary CFG remains
 * precise for all other blocks. */
static void flow_collect_async_handler_targets(flow_graph *graph) {
    size_t index;
    size_t operand_index;
    int target;
    flow_node *node;
    instruction_queue *item;
    for (index = 0; index < graph->item_count; index++) {
        node = &graph->nodes[index];
        item = &graph->items[index];
        if (!node->reachable || item->instrType != OP_CODE || !node->op ||
            !flow_is_async_handler_registration(node->op->opcode)) continue;
        for (operand_index = 0; operand_index < item->operandCount; operand_index++) {
            if (!rxop_effect_branch_target_operand(&node->effects, operand_index)) continue;
            target = flow_label_index(graph, rxas_queue_operand(item, operand_index));
            if (target >= 0) flow_add_async_handler_target(graph, (size_t)target);
        }
    }
}

static void flow_add_implicit_effects(flow_graph *graph, size_t node_index) {
    flow_node *node;
    instruction_queue *item;
    int register_index;
    char type;
    size_t base;
    size_t index;

    node = &graph->nodes[node_index];
    item = &graph->items[node_index];
    switch (node->effects.implicit) {
        case RXOP_IMPLICIT_LOCAL_R0_READ_WRITE:
        case RXOP_IMPLICIT_LOCAL_R1_READ_WRITE:
        case RXOP_IMPLICIT_LOCAL_R2_READ_WRITE:
            base = node->effects.implicit == RXOP_IMPLICIT_LOCAL_R0_READ_WRITE ? 0 :
                   node->effects.implicit == RXOP_IMPLICIT_LOCAL_R1_READ_WRITE ? 1 : 2;
            register_index = flow_register_index(graph, 'r', base);
            flow_set_register_views(graph, node->uses, register_index, FLOW_VIEW_BIT(FLOW_VIEW_INTEGER));
            flow_set_register_views(graph, node->kills, register_index, FLOW_VIEW_BIT(FLOW_VIEW_INTEGER));
            break;
        case RXOP_IMPLICIT_LOCAL_COPY:
            if (item->operand1Token && item->operand1Token->token_type == INT &&
                item->operand1Token->token_value.integer >= 0) {
                register_index = flow_register_index(graph, 'r',
                        (size_t)item->operand1Token->token_value.integer);
                flow_set_register_views(graph, node->kills, register_index, FLOW_ALL_VIEWS);
            }
            if (item->operand2Token && item->operand2Token->token_type == INT &&
                item->operand2Token->token_value.integer >= 0) {
                register_index = flow_register_index(graph, 'r',
                        (size_t)item->operand2Token->token_value.integer);
                flow_set_register_views(graph, node->uses, register_index, FLOW_ALL_VIEWS);
            }
            break;
        case RXOP_IMPLICIT_LOCAL_TARGET:
            flow_set_all_bits(graph, node->uses);
            break;
        case RXOP_IMPLICIT_ARGUMENT_INDEX:
            if (item->operand2Token && item->operand2Token->token_type == INT &&
                item->operand2Token->token_value.integer >= 0) {
                register_index = flow_register_index(graph, 'a',
                        (size_t)item->operand2Token->token_value.integer);
                flow_set_register_views(graph, node->uses, register_index, FLOW_ALL_VIEWS);
            }
            break;
        case RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3:
            type = flow_register_type(item->operand3Token);
            if (!type) {
                flow_set_all_bits(graph, node->uses);
                break;
            }
            base = (size_t)item->operand3Token->token_value.integer;
            for (index = 0; index < graph->register_count; index++) {
                if (graph->registers[index].type == type &&
                    graph->registers[index].number > base)
                    flow_set_register_views(graph, node->uses, (int)index, FLOW_ALL_VIEWS);
            }
            break;
        case RXOP_IMPLICIT_NONE:
            break;
        default:
            flow_set_all_bits(graph, node->uses);
            break;
    }
}

static void flow_build_use_kill_and_taint(flow_graph *graph) {
    size_t node_index;
    size_t operand_index;
    int register_index;
    instruction_queue *item;
    flow_node *node;
    Assembler_Token *operand;
    char trace_type;
    unsigned int views;
    unsigned int persistent_semantics;

    persistent_semantics = RXOP_SEM_ALIAS_CREATE | RXOP_SEM_ALIAS_RELEASE |
                           RXOP_SEM_REFERENCE_CREATE | RXOP_SEM_REFERENCE_READ |
                           RXOP_SEM_REFERENCE_WRITE | RXOP_SEM_REFERENCE_RELEASE |
                           RXOP_SEM_LIFETIME_END | RXOP_SEM_INDIRECT_WRITE |
                           RXOP_SEM_OPAQUE;
    for (node_index = 0; node_index < graph->item_count; node_index++) {
        item = &graph->items[node_index];
        node = &graph->nodes[node_index];
        if (item->instrType == OP_CODE) {
            if (!node->op || node->effects.state != RXOP_EFFECT_CLASSIFIED) {
                flow_set_all_bits(graph, node->uses);
                continue;
            }
            for (operand_index = 0; operand_index < item->operandCount; operand_index++) {
                operand = rxas_queue_operand(item, operand_index);
                register_index = flow_register_index(graph,
                        flow_register_type(operand),
                        operand && operand->token_value.integer >= 0
                                ? (size_t)operand->token_value.integer : 0);
                if (register_index < 0) continue;
                if (rxop_effect_reads_operand(&node->effects, operand_index)) {
                    views = flow_read_views(node->op->opcode, operand_index);
                    flow_set_register_views(graph, node->uses, register_index, views);
                }
                if (rxop_effect_writes_operand(&node->effects, operand_index)) {
                    views = flow_precise_write_views(node->op->opcode, operand_index);
                    flow_set_register_views(graph, node->kills, register_index, views);
                }
                if ((node->effects.semantics & persistent_semantics) != 0)
                    graph->tainted_registers[register_index] = 1;
            }
            flow_add_implicit_effects(graph, node_index);
        }
        else if (item->instrType == REG_META) {
            register_index = flow_register_index(graph,
                    flow_register_type(item->operand3Token),
                    item->operand3Token && item->operand3Token->token_value.integer >= 0
                            ? (size_t)item->operand3Token->token_value.integer : 0);
            if (register_index >= 0)
                flow_set_register_views(graph, node->uses, register_index, FLOW_ALL_VIEWS);
        }
        else if (item->instrType == TRACE_EVENT &&
                 flow_token_is_string(item->operand2Token, "R") &&
                 item->operand4Token && item->operand4Token->token_type == STRING &&
                 item->operand5Token && item->operand5Token->token_type == INT &&
                 item->operand5Token->token_value.integer >= 0) {
            trace_type = (char)tolower((unsigned char)item->operand4Token->token_value.string[0]);
            register_index = flow_register_index(graph, trace_type,
                    (size_t)item->operand5Token->token_value.integer);
            if (register_index >= 0)
                flow_set_register_views(graph, node->uses, register_index, FLOW_ALL_VIEWS);
        }
    }
}

static int flow_build_graph(flow_graph *graph, Assembler_Context *context,
                            instruction_queue *items, size_t item_count) {
    size_t index;
    size_t stride;
    memset(graph, 0, sizeof(*graph));
    graph->context = context;
    graph->items = items;
    graph->item_count = item_count;
    if (!item_count) return 1;
    flow_collect_registers(graph);
    graph->bit_count = graph->register_count * FLOW_VIEW_COUNT;
    graph->word_count = (graph->bit_count + FLOW_WORD_BITS - 1) / FLOW_WORD_BITS;
    if (!graph->word_count) graph->word_count = 1;
    graph->nodes = calloc(item_count, sizeof(*graph->nodes));
    stride = item_count * graph->word_count;
    graph->bit_storage = calloc(stride * 4, sizeof(unsigned long));
    graph->tainted_registers = calloc(graph->register_count ? graph->register_count : 1, 1);
    if (!graph->nodes || !graph->bit_storage || !graph->tainted_registers)
        RX_PANIC_OOM("calloc RXAS whole-procedure flow graph",
                     item_count * sizeof(*graph->nodes) + stride * 4 * sizeof(unsigned long),
                     context && context->file_name ? context->file_name : 0);
    for (index = 0; index < item_count; index++) {
        graph->nodes[index].uses = graph->bit_storage + index * graph->word_count;
        graph->nodes[index].kills = graph->bit_storage + stride + index * graph->word_count;
        graph->nodes[index].live_in = graph->bit_storage + stride * 2 + index * graph->word_count;
        graph->nodes[index].live_out = graph->bit_storage + stride * 3 + index * graph->word_count;
    }
    flow_build_edges(graph);
    flow_mark_reachable(graph);
    flow_collect_async_handler_targets(graph);
    flow_build_use_kill_and_taint(graph);
    return 1;
}

static void flow_free_graph(flow_graph *graph) {
    size_t index;
    for (index = 0; index < graph->item_count; index++)
        free(graph->nodes[index].successors);
    free(graph->nodes);
    free(graph->registers);
    free(graph->bit_storage);
    free(graph->predecessor_offsets);
    free(graph->predecessors);
    free(graph->async_handler_targets);
    free(graph->tainted_registers);
    memset(graph, 0, sizeof(*graph));
}

static void flow_compute_liveness(flow_graph *graph) {
    size_t reverse_index;
    size_t word;
    size_t successor_index;
    unsigned long next_out;
    unsigned long next_in;
    int changed;

    do {
        changed = 0;
        reverse_index = graph->item_count;
        while (reverse_index) {
            flow_node *node;
            reverse_index--;
            node = &graph->nodes[reverse_index];
            if (!node->reachable) continue;
            for (word = 0; word < graph->word_count; word++) {
                next_out = 0;
                for (successor_index = 0;
                     successor_index < node->successor_count;
                     successor_index++)
                    next_out |= graph->nodes[node->successors[successor_index]].live_in[word];
                if (graph->items[reverse_index].instrType == OP_CODE) {
                    for (successor_index = 0;
                         successor_index < graph->async_handler_target_count;
                         successor_index++)
                        next_out |= graph->nodes[
                                graph->async_handler_targets[successor_index]].live_in[word];
                }
                if (node->unknown_successor) next_out = ~0ul;
                next_in = node->uses[word] | (next_out & ~node->kills[word]);
                if (node->live_out[word] != next_out || node->live_in[word] != next_in) {
                    node->live_out[word] = next_out;
                    node->live_in[word] = next_in;
                    changed = 1;
                }
            }
        }
    } while (changed);
}

static int flow_has_trace_after(const flow_graph *graph, size_t item_index) {
    size_t index;
    for (index = item_index + 1; index < graph->item_count; index++) {
        /* Labels do not advance the machine address. A TRACE record separated
         * from its instruction only by labels or other metadata is still an
         * observation of that instruction and must not drift. */
        if (graph->items[index].instrType == OP_CODE) return 0;
        if (graph->items[index].instrType == TRACE_EVENT) return 1;
    }
    return 0;
}

static int flow_has_address_observation_after(const flow_graph *graph,
                                              size_t item_index) {
    size_t index;
    for (index = item_index + 1; index < graph->item_count; index++) {
        if (graph->items[index].instrType == OP_CODE) return 0;
        if (graph->items[index].instrType == TRACE_EVENT ||
            graph->items[index].instrType == SRC_STEP)
            return 1;
    }
    return 0;
}

static int flow_destination_dead(const flow_graph *graph, size_t node_index,
                                 int register_index, unsigned int views) {
    size_t view;
    if (register_index < 0) return 0;
    for (view = 0; view < FLOW_VIEW_COUNT; view++) {
        if ((views & FLOW_VIEW_BIT(view)) &&
            flow_test_bit(graph->nodes[node_index].live_out,
                          (size_t)register_index * FLOW_VIEW_COUNT + view))
            return 0;
    }
    return 1;
}

static size_t flow_remove_unreachable(flow_graph *graph, flow_stats *stats) {
    size_t index;
    size_t removed;
    removed = 0;
    if (!graph->complete_control_flow) return 0;
    for (index = 0; index < graph->item_count; index++) {
        if (!graph->nodes[index].reachable) {
            if (graph->items[index].instrType == OP_CODE) {
                graph->items[index].instrType = EMPTY;
                removed++;
                stats->unreachable_removed++;
            }
            else if (graph->items[index].instrType == TRACE_EVENT ||
                     graph->items[index].instrType == SRC_STEP) {
                /* Address-bound TRACE and source-step records in a dead block
                 * must not drift to the next surviving code address when the
                 * instruction they describe vanishes. */
                graph->items[index].instrType = EMPTY;
            }
        }
    }
    return removed;
}

static int flow_fact_barrier(const flow_graph *graph, size_t node_index) {
    const instruction_queue *item;
    const flow_node *node;
    item = &graph->items[node_index];
    node = &graph->nodes[node_index];
    /* Labels and assembler metadata are not machine instructions and cannot
     * invalidate a value fact. Treating a join label as opaque would make a
     * must fact disappear precisely where predecessor intersection matters. */
    if (item->instrType != OP_CODE) return 0;
    if (!node->op || node->effects.state != RXOP_EFFECT_CLASSIFIED) return 1;
    return (node->effects.semantics &
            (RXOP_SEM_CALL | RXOP_SEM_DYNAMIC_CALL | RXOP_SEM_ALIAS_CREATE |
             RXOP_SEM_ALIAS_RELEASE | RXOP_SEM_REFERENCE_CREATE |
             RXOP_SEM_REFERENCE_READ | RXOP_SEM_REFERENCE_WRITE |
             RXOP_SEM_REFERENCE_RELEASE | RXOP_SEM_LIFETIME_END |
             RXOP_SEM_INDIRECT_WRITE | RXOP_SEM_INDIRECT_BRANCH |
             RXOP_SEM_OPAQUE)) != 0;
}

static int flow_node_writes_fact_register(const flow_graph *graph, size_t node_index,
                                          int register_index, unsigned int views) {
    size_t operand_index;
    instruction_queue *item;
    flow_node *node;
    Assembler_Token *operand;
    unsigned int written_views;
    item = &graph->items[node_index];
    node = &graph->nodes[node_index];
    if (item->instrType != OP_CODE || !node->op) return 0;
    for (operand_index = 0; operand_index < item->operandCount; operand_index++) {
        if (!rxop_effect_writes_operand(&node->effects, operand_index)) continue;
        operand = rxas_queue_operand(item, operand_index);
        if (flow_register_index(graph, flow_register_type(operand),
                operand && operand->token_value.integer >= 0
                        ? (size_t)operand->token_value.integer : 0) != register_index)
            continue;
        written_views = flow_precise_write_views(node->op->opcode, operand_index);
        if (!written_views || (written_views & views)) return 1;
    }
    if (node->effects.implicit != RXOP_IMPLICIT_NONE) return 1;
    return 0;
}

static int flow_is_async_handler_target(const flow_graph *graph, size_t node_index) {
    size_t index;
    for (index = 0; index < graph->async_handler_target_count; index++)
        if (graph->async_handler_targets[index] == node_index) return 1;
    return 0;
}

static void flow_compute_available_fact(const flow_graph *graph, size_t generator,
                                        int destination_register, int source_register,
                                        unsigned int views,
                                        unsigned char *available_in,
                                        unsigned char *available_out) {
    size_t index;
    size_t predecessor;
    size_t predecessor_start;
    size_t predecessor_end;
    int changed;
    int next_in;
    int next_out;
    for (index = 0; index < graph->item_count; index++) {
        if (graph->nodes[index].reachable) {
            available_in[index] = 1;
            available_out[index] = 1;
        }
    }
    if (graph->item_count) available_in[0] = 0;
    do {
        changed = 0;
        for (index = 0; index < graph->item_count; index++) {
            if (!graph->nodes[index].reachable) continue;
            predecessor_start = graph->predecessor_offsets[index];
            predecessor_end = graph->predecessor_offsets[index + 1];
            next_in = predecessor_start < predecessor_end ||
                      flow_is_async_handler_target(graph, index);
            for (predecessor = predecessor_start;
                 predecessor < predecessor_end;
                 predecessor++) {
                size_t predecessor_node;
                predecessor_node = graph->predecessors[predecessor];
                if (!graph->nodes[predecessor_node].reachable) continue;
                if (!available_out[predecessor_node]) next_in = 0;
            }
            if (flow_is_async_handler_target(graph, index)) {
                for (predecessor = 0; predecessor < graph->item_count; predecessor++) {
                    if (graph->nodes[predecessor].reachable &&
                        graph->items[predecessor].instrType == OP_CODE &&
                        !available_out[predecessor]) next_in = 0;
                }
            }
            if (index == 0) next_in = 0;
            if (index == generator) next_out = 1;
            else if (flow_fact_barrier(graph, index) ||
                     flow_node_writes_fact_register(graph, index, destination_register, views) ||
                     flow_node_writes_fact_register(graph, index, source_register, views))
                next_out = 0;
            else next_out = next_in;
            if (available_in[index] != (unsigned char)next_in ||
                available_out[index] != (unsigned char)next_out) {
                available_in[index] = (unsigned char)next_in;
                available_out[index] = (unsigned char)next_out;
                changed = 1;
            }
        }
    } while (changed);
}

static void flow_compute_may_reach_fact(const flow_graph *graph, size_t generator,
                                        int destination_register, int source_register,
                                        unsigned int views,
                                        unsigned char *may_in,
                                        unsigned char *may_out) {
    size_t index;
    size_t predecessor;
    size_t predecessor_start;
    size_t predecessor_end;
    int changed;
    int next_in;
    int next_out;
    do {
        changed = 0;
        for (index = 0; index < graph->item_count; index++) {
            if (!graph->nodes[index].reachable) continue;
            predecessor_start = graph->predecessor_offsets[index];
            predecessor_end = graph->predecessor_offsets[index + 1];
            next_in = 0;
            for (predecessor = predecessor_start;
                 predecessor < predecessor_end;
                 predecessor++) {
                size_t predecessor_node;
                predecessor_node = graph->predecessors[predecessor];
                if (graph->nodes[predecessor_node].reachable &&
                    may_out[predecessor_node]) next_in = 1;
            }
            if (flow_is_async_handler_target(graph, index)) {
                for (predecessor = 0; predecessor < graph->item_count; predecessor++) {
                    if (graph->nodes[predecessor].reachable &&
                        graph->items[predecessor].instrType == OP_CODE &&
                        may_out[predecessor]) next_in = 1;
                }
            }
            if (index == 0) next_in = 0;
            if (index == generator) next_out = 1;
            else if (flow_fact_barrier(graph, index) ||
                     flow_node_writes_fact_register(graph, index,
                                                    destination_register, views) ||
                     flow_node_writes_fact_register(graph, index,
                                                    source_register, views))
                next_out = 0;
            else next_out = next_in;
            if (may_in[index] != (unsigned char)next_in ||
                may_out[index] != (unsigned char)next_out) {
                may_in[index] = (unsigned char)next_in;
                may_out[index] = (unsigned char)next_out;
                changed = 1;
            }
        }
    } while (changed);
}

static void flow_set_operand(instruction_queue *item, size_t operand_index,
                             Assembler_Token *token) {
    if (!item || item->instrType != OP_CODE || operand_index >= item->operandCount) return;
    item->operandTokens[operand_index] = token;
    switch (operand_index) {
        case 0: item->operand1Token = token; break;
        case 1: item->operand2Token = token; break;
        case 2: item->operand3Token = token; break;
        case 3: item->operand4Token = token; break;
        case 4: item->operand5Token = token; break;
        case 5: item->operand6Token = token; break;
        case 6: item->operand7Token = token; break;
        case 7: item->operand8Token = token; break;
        case 8: item->operand9Token = token; break;
        case 9: item->operand10Token = token; break;
        default: break;
    }
}

static int flow_is_compare_opcode(int opcode) {
    return (opcode >= OP_IEQ_REG_REG_REG && opcode <= OP_SLTE_REG_STRING_REG) ||
           (opcode >= OP_FEQ_REG_REG_REG && opcode <= OP_FLTE_REG_FLOAT_REG) ||
           (opcode >= OP_DEQ_REG_REG_REG && opcode <= OP_DLTE_REG_DECIMAL_REG) ||
           (opcode >= OP_BINEQ_REG_REG_REG && opcode <= OP_BINNE_REG_REG_BINARY);
}

static int flow_node_uses_register_views(const flow_graph *graph,
                                         size_t node_index,
                                         int register_index,
                                         unsigned int views) {
    size_t view_index;
    for (view_index = 0; view_index < FLOW_VIEW_COUNT; view_index++) {
        if ((views & FLOW_VIEW_BIT(view_index)) &&
            flow_test_bit(graph->nodes[node_index].uses,
                          (size_t)register_index * FLOW_VIEW_COUNT + view_index))
            return 1;
    }
    return 0;
}

static void flow_debug_copy_rejection(const flow_graph *graph,
                                      size_t copy_index, size_t use_index,
                                      const char *reason,
                                      unsigned int expected_views,
                                      unsigned int actual_views) {
    const flow_node *copy_node;
    const flow_node *use_node;
    if (!graph->context->debug_mode) return;
    copy_node = &graph->nodes[copy_index];
    use_node = &graph->nodes[use_index];
    fprintf(stderr,
            "NR27 reject procedure=%s candidate=%llu:%s node=%llu:%s "
            "reason=%s expected-views=0x%x actual-views=0x%x\n",
            graph->context->current_proc_name
                    ? graph->context->current_proc_name : "(directives)",
            (unsigned long long)copy_index,
            copy_node->op ? copy_node->op->mnemonic : "?",
            (unsigned long long)use_index,
            use_node->op ? use_node->op->mnemonic : "metadata",
            reason, expected_views, actual_views);
}

static void flow_debug_accept(const flow_graph *graph, size_t node_index,
                              const char *reason, size_t redirects) {
    const flow_node *node;
    if (!graph->context->debug_mode) return;
    node = &graph->nodes[node_index];
    fprintf(stderr,
            "NR27 accept procedure=%s candidate=%llu:%s reason=%s redirects=%llu\n",
            graph->context->current_proc_name
                    ? graph->context->current_proc_name : "(directives)",
            (unsigned long long)node_index,
            node->op ? node->op->mnemonic : "?",
            reason, (unsigned long long)redirects);
}

static size_t flow_propagate_one_copy(flow_graph *graph, size_t copy_index,
                                      flow_stats *stats) {
    instruction_queue *copy;
    flow_node *copy_node;
    Assembler_Token *destination;
    Assembler_Token *source;
    int destination_register;
    int source_register;
    unsigned int views;
    unsigned char *available_in;
    unsigned char *available_out;
    unsigned char *may_in;
    unsigned char *may_out;
    size_t index;
    size_t operand_index;
    size_t redirects;
    flow_node *node;
    instruction_queue *item;
    Assembler_Token *operand;
    unsigned int read_views;
    size_t compare_redirects;
    int unredirectable_use;
    int full_copy;

    copy = &graph->items[copy_index];
    copy_node = &graph->nodes[copy_index];
    if (!copy_node->op || copy->operandCount != 2) return 0;
    full_copy = 0;
    switch (copy_node->op->opcode) {
        case OP_COPY_REG_REG:
            views = FLOW_ALL_VIEWS;
            full_copy = 1;
            break;
        case OP_ICOPY_REG_REG: views = FLOW_VIEW_BIT(FLOW_VIEW_INTEGER); break;
        case OP_FCOPY_REG_REG: views = FLOW_VIEW_BIT(FLOW_VIEW_FLOAT); break;
        case OP_SCOPY_REG_REG: views = FLOW_VIEW_BIT(FLOW_VIEW_STRING); break;
        default: return 0;
    }
    destination = rxas_queue_operand(copy, 0);
    source = rxas_queue_operand(copy, 1);
    if (flow_register_type(destination) != 'r') return 0;
    destination_register = flow_register_index(graph, 'r',
            (size_t)destination->token_value.integer);
    source_register = flow_register_index(graph, flow_register_type(source),
            (size_t)source->token_value.integer);
    if (destination_register < 0 || source_register < 0) return 0;
    if (flow_has_trace_after(graph, copy_index)) {
        stats->rejected_trace++;
        return 0;
    }
    if (destination_register == source_register) {
        flow_debug_accept(graph, copy_index, "identity-copy", 0);
        copy->instrType = EMPTY;
        if (full_copy) stats->full_copies_removed++;
        else stats->typed_copies_removed++;
        return 1;
    }
    if (full_copy) {
        flow_debug_copy_rejection(graph, copy_index, copy_index,
                                  "full-value-ownership-unproved",
                                  FLOW_ALL_VIEWS, FLOW_ALL_VIEWS);
        stats->rejected_effect++;
        return 0;
    }
    if (graph->tainted_registers[destination_register]) {
        stats->rejected_tainted++;
        return 0;
    }

    available_in = calloc(graph->item_count, 1);
    available_out = calloc(graph->item_count, 1);
    may_in = calloc(graph->item_count, 1);
    may_out = calloc(graph->item_count, 1);
    if (!available_in || !available_out || !may_in || !may_out)
        RX_PANIC_OOM("calloc RXAS available-copy fact", graph->item_count * 4, 0);
    flow_compute_available_fact(graph, copy_index, destination_register,
                                source_register, views,
                                available_in, available_out);
    flow_compute_may_reach_fact(graph, copy_index, destination_register,
                                source_register, views, may_in, may_out);
    redirects = 0;
    compare_redirects = 0;
    unredirectable_use = 0;

    /* Prove that every surviving read of the copied component can use the
     * original register before mutating any operand. Metadata/TRACE reads and
     * reads reached after the equality fact is killed make the candidate fail
     * closed rather than leaving a count-neutral partial rewrite. */
    for (index = 0; index < graph->item_count; index++) {
        if (index == copy_index) continue;
        if (may_in[index] &&
            (graph->items[index].instrType == REG_META ||
             graph->items[index].instrType == TRACE_EVENT)) {
            size_t view_index;
            int metadata_read;
            metadata_read = 0;
            for (view_index = 0; view_index < FLOW_VIEW_COUNT; view_index++) {
                if ((views & FLOW_VIEW_BIT(view_index)) &&
                    flow_test_bit(graph->nodes[index].uses,
                            (size_t)destination_register * FLOW_VIEW_COUNT + view_index)) {
                    unredirectable_use = 1;
                    metadata_read = 1;
                }
            }
            if (metadata_read)
                flow_debug_copy_rejection(graph, copy_index, index,
                                          "metadata-read", views, FLOW_ALL_VIEWS);
            continue;
        }
        if (graph->items[index].instrType != OP_CODE) continue;
        node = &graph->nodes[index];
        item = &graph->items[index];
        if (may_in[index] &&
            (!node->op || node->effects.state != RXOP_EFFECT_CLASSIFIED)) {
            unredirectable_use = 1;
            flow_debug_copy_rejection(graph, copy_index, index,
                                      "unclassified-effect", views, 0);
            continue;
        }
        if (!node->op || node->effects.state != RXOP_EFFECT_CLASSIFIED) continue;
        if (may_in[index] && node->unknown_successor) {
            unredirectable_use = 1;
            flow_debug_copy_rejection(graph, copy_index, index,
                                      "unknown-successor",
                                      views, 0);
            continue;
        }
        if (may_in[index] && flow_fact_barrier(graph, index) &&
            (flow_node_uses_register_views(graph, index,
                                           destination_register, views) ||
             !flow_destination_dead(graph, index,
                                    destination_register, views))) {
            unredirectable_use = 1;
            flow_debug_copy_rejection(graph, copy_index, index,
                                      "live-at-effect-barrier", views, 0);
            continue;
        }
        if (may_in[index] && node->effects.implicit != RXOP_IMPLICIT_NONE &&
            flow_node_uses_register_views(graph, index,
                                          destination_register, views)) {
            unredirectable_use = 1;
            flow_debug_copy_rejection(graph, copy_index, index,
                                      "implicit-read", views, views);
            continue;
        }
        for (operand_index = 0; operand_index < item->operandCount; operand_index++) {
            operand = rxas_queue_operand(item, operand_index);
            if (flow_register_index(graph, flow_register_type(operand),
                    operand && operand->token_value.integer >= 0
                            ? (size_t)operand->token_value.integer : 0) != destination_register)
                continue;
            if (!may_in[index]) continue;
            if (!rxop_effect_reads_operand(&node->effects, operand_index) ||
                rxop_effect_writes_operand(&node->effects, operand_index)) {
                if (rxop_effect_reads_operand(&node->effects, operand_index))
                    unredirectable_use = 1;
                if (rxop_effect_reads_operand(&node->effects, operand_index))
                    flow_debug_copy_rejection(graph, copy_index, index,
                                              "read-write-use", views, 0);
                continue;
            }
            read_views = flow_read_views(node->op->opcode, operand_index);
            if (!available_in[index] || read_views != views) {
                unredirectable_use = 1;
                flow_debug_copy_rejection(graph, copy_index, index,
                        !available_in[index] ? "fact-not-available" : "view-mismatch",
                        views, read_views);
                continue;
            }
        }
    }
    if (unredirectable_use) {
        free(available_in);
        free(available_out);
        free(may_in);
        free(may_out);
        stats->rejected_live++;
        return 0;
    }

    for (index = 0; index < graph->item_count; index++) {
        if (!available_in[index] || index == copy_index ||
            graph->items[index].instrType != OP_CODE) continue;
        node = &graph->nodes[index];
        item = &graph->items[index];
        if (!node->op || node->effects.state != RXOP_EFFECT_CLASSIFIED) continue;
        for (operand_index = 0; operand_index < item->operandCount; operand_index++) {
            operand = rxas_queue_operand(item, operand_index);
            if (flow_register_index(graph, flow_register_type(operand),
                    operand && operand->token_value.integer >= 0
                            ? (size_t)operand->token_value.integer : 0) != destination_register)
                continue;
            if (!rxop_effect_reads_operand(&node->effects, operand_index) ||
                rxop_effect_writes_operand(&node->effects, operand_index)) continue;
            read_views = flow_read_views(node->op->opcode, operand_index);
            if (read_views != views) continue;
            flow_set_operand(item, operand_index, source);
            redirects++;
            stats->operands_redirected++;
            if (flow_is_compare_opcode(node->op->opcode))
                compare_redirects++;
        }
    }
    free(available_in);
    free(available_out);
    free(may_in);
    free(may_out);
    if (redirects) {
        /* The pre-scan proved that every may-reaching destination read was
         * redirected. Remove the generator in the same count-decreasing
         * rewrite, preserving the fixed-point termination measure. */
        flow_debug_accept(graph, copy_index, "all-uses-redirected", redirects);
        copy->instrType = EMPTY;
        stats->typed_copies_removed++;
        stats->compare_preparations_removed += compare_redirects;
        return 1;
    }

    if (flow_destination_dead(graph, copy_index, destination_register, views)) {
        flow_debug_accept(graph, copy_index, "copied-view-dead", 0);
        copy->instrType = EMPTY;
        stats->typed_copies_removed++;
        stats->compare_preparations_removed += compare_redirects;
        return 1;
    }
    stats->rejected_live++;
    return 0;
}

static size_t flow_propagate_copies(flow_graph *graph, flow_stats *stats) {
    size_t index;
    size_t changed;
    size_t removed;
    int destination_register;
    int source_register;
    instruction_queue *item;
    Assembler_Token *destination;
    Assembler_Token *source;
    unsigned char *claimed_registers;

    removed = 0;
    flow_compute_liveness(graph);
    claimed_registers = calloc(graph->register_count ? graph->register_count : 1, 1);
    if (!claimed_registers)
        RX_PANIC_OOM("calloc RXAS copy-propagation batch",
                     graph->register_count, 0);
    for (index = 0; index < graph->item_count; index++) {
        if (!graph->nodes[index].reachable || graph->items[index].instrType != OP_CODE) continue;
        item = &graph->items[index];
        destination_register = -1;
        source_register = -1;
        if (item->operandCount == 2) {
            destination = rxas_queue_operand(item, 0);
            source = rxas_queue_operand(item, 1);
            if (flow_register_type(destination) &&
                destination->token_value.integer >= 0)
                destination_register = flow_register_index(graph,
                        flow_register_type(destination),
                        (size_t)destination->token_value.integer);
            if (flow_register_type(source) && source->token_value.integer >= 0)
                source_register = flow_register_index(graph,
                        flow_register_type(source),
                        (size_t)source->token_value.integer);
        }
        /* Proofs computed from one graph compose when their complete physical
         * register pairs are disjoint.  Their substitutions then commute and
         * cannot change one another's liveness or availability facts. */
        if ((destination_register >= 0 &&
             claimed_registers[destination_register]) ||
            (source_register >= 0 && claimed_registers[source_register]))
            continue;
        changed = flow_propagate_one_copy(graph, index, stats);
        if (!changed) continue;
        removed += changed;
        if (destination_register >= 0)
            claimed_registers[destination_register] = 1;
        if (source_register >= 0) claimed_registers[source_register] = 1;
    }
    free(claimed_registers);
    return removed;
}

/* Retarget a single, component-exact, nonthrowing producer into the destination
 * of its immediately following typed copy.  At the producer boundary the old
 * destination component must be dead; after the copy the temporary component
 * must be dead.  Those two liveness facts, including the conservative async
 * handler edges, prove the two streams observationally equivalent while the
 * immediate adjacency prevents any intervening observation. */
static size_t flow_forward_producer_destination(flow_graph *graph,
                                                flow_stats *stats) {
    size_t producer_index;
    size_t operand_index;
    size_t write_count;
    instruction_queue *producer;
    instruction_queue *copy;
    flow_node *producer_node;
    flow_node *copy_node;
    Assembler_Token *temporary;
    Assembler_Token *destination;
    Assembler_Token *operand;
    int temporary_register;
    int destination_register;
    unsigned int views;
    unsigned char *claimed_registers;
    size_t forwarded;

    flow_compute_liveness(graph);
    claimed_registers = calloc(graph->register_count ? graph->register_count : 1, 1);
    if (!claimed_registers)
        RX_PANIC_OOM("calloc RXAS producer-forward batch",
                     graph->register_count, 0);
    forwarded = 0;
    for (producer_index = 0; producer_index + 1 < graph->item_count;
         producer_index++) {
        producer = &graph->items[producer_index];
        copy = &graph->items[producer_index + 1];
        producer_node = &graph->nodes[producer_index];
        copy_node = &graph->nodes[producer_index + 1];
        if (!producer_node->reachable || !copy_node->reachable ||
            producer->instrType != OP_CODE || copy->instrType != OP_CODE ||
            !producer_node->op || !copy_node->op ||
            producer_node->effects.state != RXOP_EFFECT_CLASSIFIED ||
            producer_node->op->flow != FLOW_NEXT ||
            producer_node->effects.optimizer_barrier ||
            producer_node->effects.implicit != RXOP_IMPLICIT_NONE ||
            producer_node->effects.semantics != RXOP_SEM_NONE ||
            producer->operandCount == 0 || copy->operandCount != 2)
            continue;

        switch (copy_node->op->opcode) {
            case OP_ICOPY_REG_REG:
                views = FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
                break;
            case OP_FCOPY_REG_REG:
                views = FLOW_VIEW_BIT(FLOW_VIEW_FLOAT);
                break;
            default:
                continue;
        }
        if (copy_node->effects.state != RXOP_EFFECT_CLASSIFIED ||
            copy_node->effects.semantics != RXOP_SEM_NONE)
            continue;

        temporary = rxas_queue_operand(producer, 0);
        destination = rxas_queue_operand(copy, 0);
        if (flow_register_type(temporary) != 'r' ||
            flow_register_type(rxas_queue_operand(copy, 1)) != 'r' ||
            flow_register_type(destination) != 'r')
            continue;
        temporary_register = flow_register_index(graph, 'r',
                (size_t)temporary->token_value.integer);
        destination_register = flow_register_index(graph, 'r',
                (size_t)destination->token_value.integer);
        if (temporary_register < 0 || destination_register < 0 ||
            temporary_register == destination_register ||
            flow_register_index(graph, 'r',
                    (size_t)rxas_queue_operand(copy, 1)->token_value.integer) !=
                    temporary_register)
            continue;
        if (claimed_registers[temporary_register] ||
            claimed_registers[destination_register])
            continue;
        if (graph->tainted_registers[temporary_register] ||
            graph->tainted_registers[destination_register]) {
            stats->rejected_tainted++;
            continue;
        }

        write_count = 0;
        for (operand_index = 0; operand_index < producer->operandCount;
             operand_index++) {
            operand = rxas_queue_operand(producer, operand_index);
            if (rxop_effect_writes_operand(&producer_node->effects,
                                           operand_index)) {
                write_count++;
                if (operand_index != 0) write_count = producer->operandCount + 1;
            }
            if (operand_index > 0 && flow_register_type(operand) == 'r' &&
                flow_register_index(graph, 'r',
                        (size_t)operand->token_value.integer) ==
                        destination_register)
                write_count = producer->operandCount + 1;
        }
        if (write_count != 1 ||
            rxop_effect_reads_operand(&producer_node->effects, 0) ||
            !rxop_effect_kills_operand(&producer_node->effects, 0) ||
            flow_precise_write_views(producer_node->op->opcode, 0) != views) {
            stats->rejected_effect++;
            continue;
        }
        if (flow_has_address_observation_after(graph, producer_index + 1)) {
            stats->rejected_trace++;
            continue;
        }
        if (!flow_destination_dead(graph, producer_index,
                                   destination_register, views) ||
            !flow_destination_dead(graph, producer_index + 1,
                                   temporary_register, views)) {
            stats->rejected_live++;
            continue;
        }

        flow_set_operand(producer, 0, destination);
        copy->instrType = EMPTY;
        flow_debug_accept(graph, producer_index,
                          "producer-destination-forwarded", 1);
        stats->producer_destinations_forwarded++;
        stats->typed_copies_removed++;
        claimed_registers[temporary_register] = 1;
        claimed_registers[destination_register] = 1;
        forwarded++;
    }
    free(claimed_registers);
    return forwarded;
}

static int flow_same_literal(const Assembler_Token *left,
                             const Assembler_Token *right) {
    if (!left || !right || left->token_type != right->token_type) return 0;
    if (left->token_type == INT) return left->token_value.integer == right->token_value.integer;
    if (left->token_type == FLOAT)
        return memcmp(&left->token_value.real, &right->token_value.real,
                      sizeof(left->token_value.real)) == 0;
    return 0;
}

static size_t flow_remove_redundant_loads(flow_graph *graph, flow_stats *stats) {
    size_t generator;
    size_t index;
    int destination_register;
    unsigned int views;
    unsigned char *available_in;
    unsigned char *available_out;
    instruction_queue *first;
    instruction_queue *item;
    flow_node *node;
    size_t removed;
    removed = 0;
    for (generator = 0; generator < graph->item_count; generator++) {
        first = &graph->items[generator];
        node = &graph->nodes[generator];
        if (first->instrType != OP_CODE || !node->reachable || !node->op ||
            (node->op->opcode != OP_LOAD_REG_INT &&
             node->op->opcode != OP_LOAD_REG_FLOAT)) continue;
        if (flow_has_trace_after(graph, generator)) continue;
        destination_register = flow_register_index(graph,
                flow_register_type(first->operand1Token),
                (size_t)first->operand1Token->token_value.integer);
        if (destination_register < 0 ||
            graph->registers[destination_register].type != 'r' ||
            graph->tainted_registers[destination_register]) continue;
        views = node->op->opcode == OP_LOAD_REG_INT
                ? FLOW_VIEW_BIT(FLOW_VIEW_INTEGER) : FLOW_VIEW_BIT(FLOW_VIEW_FLOAT);
        available_in = calloc(graph->item_count, 1);
        available_out = calloc(graph->item_count, 1);
        if (!available_in || !available_out)
            RX_PANIC_OOM("calloc RXAS available-load fact", graph->item_count * 2, 0);
        flow_compute_available_fact(graph, generator, destination_register,
                                    destination_register, views,
                                    available_in, available_out);
        for (index = generator + 1; index < graph->item_count; index++) {
            item = &graph->items[index];
            if (!available_in[index] || item->instrType != OP_CODE ||
                !graph->nodes[index].op ||
                graph->nodes[index].op->opcode != node->op->opcode ||
                flow_register_index(graph, flow_register_type(item->operand1Token),
                        (size_t)item->operand1Token->token_value.integer) != destination_register ||
                !flow_same_literal(first->operand2Token, item->operand2Token) ||
                flow_has_trace_after(graph, index)) continue;
            item->instrType = EMPTY;
            flow_debug_accept(graph, index, "redundant-identical-load", 0);
            removed++;
            stats->redundant_loads_removed++;
        }
        free(available_in);
        free(available_out);
    }
    return removed;
}

static size_t flow_remove_redundant_initializations(flow_graph *graph,
                                                    flow_stats *stats) {
    size_t generator;
    size_t index;
    int destination_register;
    unsigned char *available_in;
    unsigned char *available_out;
    instruction_queue *first;
    instruction_queue *item;
    size_t removed;
    removed = 0;
    for (generator = 0; generator < graph->item_count; generator++) {
        first = &graph->items[generator];
        if (first->instrType != OP_CODE || !graph->nodes[generator].reachable ||
            !graph->nodes[generator].op ||
            graph->nodes[generator].op->opcode != OP_NULL_REG ||
            flow_has_trace_after(graph, generator)) continue;
        destination_register = flow_register_index(graph,
                flow_register_type(first->operand1Token),
                (size_t)first->operand1Token->token_value.integer);
        if (destination_register < 0 ||
            graph->registers[destination_register].type != 'r' ||
            graph->tainted_registers[destination_register]) continue;
        available_in = calloc(graph->item_count, 1);
        available_out = calloc(graph->item_count, 1);
        if (!available_in || !available_out)
            RX_PANIC_OOM("calloc RXAS available-initialization fact",
                         graph->item_count * 2, 0);
        flow_compute_available_fact(graph, generator, destination_register,
                                    destination_register, FLOW_ALL_VIEWS,
                                    available_in, available_out);
        for (index = generator + 1; index < graph->item_count; index++) {
            item = &graph->items[index];
            if (!available_in[index] || item->instrType != OP_CODE ||
                !graph->nodes[index].op ||
                graph->nodes[index].op->opcode != OP_NULL_REG ||
                flow_register_index(graph, flow_register_type(item->operand1Token),
                        (size_t)item->operand1Token->token_value.integer) !=
                        destination_register ||
                flow_has_trace_after(graph, index)) continue;
            item->instrType = EMPTY;
            flow_debug_accept(graph, index, "redundant-null", 0);
            removed++;
            stats->redundant_initializations_removed++;
        }
        free(available_in);
        free(available_out);
    }
    return removed;
}

static unsigned int flow_redundant_conversion_views(int opcode) {
    if (opcode == OP_ITOF_REG)
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER) |
               FLOW_VIEW_BIT(FLOW_VIEW_FLOAT);
    return 0;
}

static size_t flow_remove_redundant_conversions(flow_graph *graph,
                                                flow_stats *stats) {
    size_t generator;
    size_t index;
    int register_index;
    int opcode;
    unsigned int views;
    unsigned char *available_in;
    unsigned char *available_out;
    instruction_queue *first;
    instruction_queue *item;
    size_t removed;
    removed = 0;
    for (generator = 0; generator < graph->item_count; generator++) {
        first = &graph->items[generator];
        if (first->instrType != OP_CODE || !graph->nodes[generator].reachable ||
            !graph->nodes[generator].op || first->operandCount != 1 ||
            flow_has_trace_after(graph, generator)) continue;
        opcode = graph->nodes[generator].op->opcode;
        views = flow_redundant_conversion_views(opcode);
        if (!views) continue;
        register_index = flow_register_index(graph,
                flow_register_type(first->operand1Token),
                (size_t)first->operand1Token->token_value.integer);
        if (register_index < 0 || graph->registers[register_index].type != 'r' ||
            graph->tainted_registers[register_index]) continue;
        available_in = calloc(graph->item_count, 1);
        available_out = calloc(graph->item_count, 1);
        if (!available_in || !available_out)
            RX_PANIC_OOM("calloc RXAS available-conversion fact",
                         graph->item_count * 2, 0);
        flow_compute_available_fact(graph, generator, register_index,
                                    register_index, views,
                                    available_in, available_out);
        for (index = generator + 1; index < graph->item_count; index++) {
            item = &graph->items[index];
            if (!available_in[index] || item->instrType != OP_CODE ||
                !graph->nodes[index].op ||
                graph->nodes[index].op->opcode != opcode ||
                item->operandCount != 1 ||
                flow_register_index(graph, flow_register_type(item->operand1Token),
                        (size_t)item->operand1Token->token_value.integer) !=
                        register_index ||
                flow_has_trace_after(graph, index)) continue;
            item->instrType = EMPTY;
            flow_debug_accept(graph, index, "redundant-context-free-conversion", 0);
            removed++;
            stats->redundant_conversions_removed++;
        }
        free(available_in);
        free(available_out);
    }
    return removed;
}

static void flow_debug_summary(const flow_graph *graph, const flow_stats *stats,
                               size_t before_instructions, size_t after_instructions) {
    if (!graph->context->debug_mode) return;
    fprintf(stderr,
            "NR27 flow procedure=%s blocks=%llu registers=%llu instructions=%llu->%llu "
            "unreachable=%llu dead=%llu typed-copy=%llu compare-prep=%llu "
            "full-copy=%llu redundant-load=%llu redundant-init=%llu "
            "redundant-conversion=%llu producer-forward=%llu redirects=%llu "
            "reject-live=%llu reject-trace=%llu reject-tainted=%llu reject-effect=%llu\n",
            graph->context->current_proc_name ? graph->context->current_proc_name : "(directives)",
            (unsigned long long)graph->block_count,
            (unsigned long long)graph->register_count,
            (unsigned long long)before_instructions,
            (unsigned long long)after_instructions,
            (unsigned long long)stats->unreachable_removed,
            (unsigned long long)stats->dead_results_removed,
            (unsigned long long)stats->typed_copies_removed,
            (unsigned long long)stats->compare_preparations_removed,
            (unsigned long long)stats->full_copies_removed,
            (unsigned long long)stats->redundant_loads_removed,
            (unsigned long long)stats->redundant_initializations_removed,
            (unsigned long long)stats->redundant_conversions_removed,
            (unsigned long long)stats->producer_destinations_forwarded,
            (unsigned long long)stats->operands_redirected,
            (unsigned long long)stats->rejected_live,
            (unsigned long long)stats->rejected_trace,
            (unsigned long long)stats->rejected_tainted,
            (unsigned long long)stats->rejected_effect);
}

static size_t flow_instruction_count(instruction_queue *items, size_t item_count) {
    size_t index;
    size_t count;
    count = 0;
    for (index = 0; index < item_count; index++)
        if (items[index].instrType == OP_CODE) count++;
    return count;
}

static int flow_value_analysis_within_bound(const flow_graph *graph) {
    if (!graph->resolved_indirect_branches) return 1;
    if (!graph->word_count) return 1;
    return graph->item_count <=
           FLOW_MAX_INDIRECT_VALUE_CELLS / graph->word_count;
}

void rxas_flow_optimise(Assembler_Context *context,
                        instruction_queue *items,
                        size_t item_count) {
    flow_graph graph;
    flow_stats stats;
    size_t before_instructions;
    size_t after_instructions;
    size_t changed;
    size_t iterations;

    if (!context || !items || !item_count || !context->current_proc_name) return;
    memset(&stats, 0, sizeof(stats));
    before_instructions = flow_instruction_count(items, item_count);
    iterations = 0;
    do {
        flow_build_graph(&graph, context, items, item_count);
        stats.procedures = 1;
        stats.blocks = graph.block_count;
        /* An unresolved target or indirect jump can enter at any instruction.
         * Without those predecessor edges no whole-procedure must/may fact is
         * complete, so fail closed for every NR-27 rewrite in the procedure. */
        if (!graph.complete_control_flow) {
            if (context->debug_mode)
                fprintf(stderr,
                        "NR27 reject procedure=%s candidate=whole-procedure "
                        "reason=incomplete-control-flow\n",
                        context->current_proc_name);
            changed = 0;
        }
        else {
            changed = flow_remove_unreachable(&graph, &stats);
            if (!changed && flow_value_analysis_within_bound(&graph)) {
                changed += flow_propagate_copies(&graph, &stats);
                if (!changed) changed += flow_forward_producer_destination(&graph, &stats);
                if (!changed) changed += flow_remove_redundant_loads(&graph, &stats);
                if (!changed) changed += flow_remove_redundant_initializations(&graph, &stats);
                if (!changed) changed += flow_remove_redundant_conversions(&graph, &stats);
            }
            else if (!changed && context->debug_mode) {
                fprintf(stderr,
                        "NR27 bound procedure=%s scope=reachability-only "
                        "value-cells=%llu limit=%llu\n",
                        context->current_proc_name,
                        (unsigned long long)(graph.item_count * graph.word_count),
                        (unsigned long long)FLOW_MAX_INDIRECT_VALUE_CELLS);
            }
            /* P3 dead-result deletion is deliberately absent. A nominal
             * integer/float write may release hidden reference or native
             * payload state; numeric liveness alone cannot prove that effect
             * unobservable. */
        }
        flow_free_graph(&graph);
        iterations++;
    } while (changed && iterations <= before_instructions + 1);

    flow_build_graph(&graph, context, items, item_count);
    after_instructions = flow_instruction_count(items, item_count);
    flow_debug_summary(&graph, &stats, before_instructions, after_instructions);
    flow_free_graph(&graph);
}
