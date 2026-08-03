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
#include "rxas_flow_analysis.h"
#include "rxas_flow_graph.h"
#include "rxas_flow_proof.h"
#include "rxas_flow_signal.h"
#include "rxas_flow_ssa.h"
#include "rxas_flow_use.h"
#include "rxdefs.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLOW_WORD_BITS (sizeof(unsigned long) * 8)
/* Bound global value analysis only for procedures newly admitted by exact
 * indirect-table edges.  Reachability remains linear and unbounded. */
#define FLOW_MAX_INDIRECT_VALUE_CELLS 1000000
/* C2-E2 symbolic storage service. The environment has one storage identity per
 * register slot and program point; keep its maximum retained state explicit so
 * normal optimisation cost and memory remain bounded before rewrite consumers
 * are admitted. */
#define FLOW_MAX_STORAGE_IDENTITY_CELLS 2000000

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

typedef enum flow_edge_kind {
    FLOW_EDGE_NORMAL = 0,
    FLOW_EDGE_SIGNAL_SKIP
} flow_edge_kind;

typedef struct flow_edge {
    size_t target;
    flow_edge_kind kind;
} flow_edge;

typedef struct flow_storage_analysis flow_storage_analysis;

typedef struct flow_node {
    const OpInfo *op;
    RxOpEffects effects;
    flow_edge *successors;
    size_t successor_count;
    size_t successor_capacity;
    size_t block;
    int unknown_successor;
    int reachable;
    unsigned long *uses;
    unsigned long *kills;
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
    flow_storage_analysis *storage;
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
    size_t constant_proof_queries;
    size_t constant_proof_proved;
    size_t constant_proof_rejected;
    size_t constant_proof_unavailable;
    size_t absent_proof_queries;
    size_t absent_proof_proved;
    size_t absent_proof_rejected;
    size_t absent_proof_unavailable;
    size_t self_copy_proof_queries;
    size_t self_copy_proof_proved;
    size_t self_copy_proof_rejected;
    size_t self_copy_proof_unavailable;
    size_t derivation_proof_queries;
    size_t derivation_proof_proved;
    size_t derivation_proof_rejected;
    size_t derivation_proof_unavailable;
    size_t producer_proof_queries;
    size_t producer_proof_proved;
    size_t producer_proof_rejected;
    size_t producer_proof_unavailable;
    size_t producer_destinations_forwarded;
    size_t compare_branch_queries;
    size_t compare_branch_proved;
    size_t compare_branch_rejected;
    size_t compare_branches_fused;
    size_t compare_trace_events_removed;
    size_t duplicate_linked_read_queries;
    size_t duplicate_linked_read_proved;
    size_t duplicate_linked_read_rejected;
    size_t duplicate_linked_reads_reused;
    size_t operands_redirected;
    size_t rejected_live;
    size_t rejected_trace;
    size_t rejected_tainted;
    size_t rejected_effect;
} flow_stats;

typedef struct flow_storage_stats {
    size_t reachable_nodes;
    size_t state_cells;
    size_t exact_cells;
    size_t unknown_cells;
    size_t aliased_cells;
    size_t link_transfers;
    size_t exact_links;
    size_t unknown_links;
    size_t success_only_links;
    size_t swap_pairs;
    size_t exact_swap_pairs;
    size_t unknown_swap_pairs;
    size_t unlink_transfers;
    size_t join_nodes;
    size_t join_unknown_cells;
    size_t backward_edges;
    size_t unsupported_mapping_ops;
    size_t full_copies;
    size_t exact_full_copies;
    size_t base_full_copies;
    size_t tainted_full_copies;
    size_t exact_tainted_full_copies;
    size_t base_destination_tainted_full_copies;
    size_t alias_self_copies;
    size_t swap_round_trips;
    size_t round_trip_instructions;
    size_t normal_edges;
    size_t signal_skip_edges;
    size_t signal_handler_entries;
} flow_storage_stats;

struct flow_storage_analysis {
    const flow_graph *graph;
    size_t *in;
    unsigned char *has_in;
    unsigned char *queued;
    size_t *queue;
    size_t queue_capacity;
};

static char flow_register_type(const Assembler_Token *token) {
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
    return rxop_component_reads(opcode, operand_index);
}

/* A zero result means the effects inventory proves a write, but NR-27 does not
 * yet have a component-exact kill for it. This distinction is essential: the
 * canonical kills bit is not a whole multi-view value kill. */
static unsigned int flow_precise_write_views(int opcode, size_t operand_index) {
    return rxop_component_writes(opcode, operand_index);
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

static unsigned int flow_trace_event_views(const instruction_queue *item) {
    char value_type;
    if (!item || !item->operand3Token ||
        item->operand3Token->token_type != STRING ||
        !item->operand3Token->token_value.string[0])
        return FLOW_ALL_VIEWS;
    value_type = (char)toupper((unsigned char)
            item->operand3Token->token_value.string[0]);
    if (value_type == 'B' || value_type == 'I')
        return FLOW_VIEW_BIT(FLOW_VIEW_INTEGER);
    if (value_type == 'F') return FLOW_VIEW_BIT(FLOW_VIEW_FLOAT);
    if (value_type == 'S') return FLOW_VIEW_BIT(FLOW_VIEW_STRING);
    if (value_type == 'D') return FLOW_VIEW_BIT(FLOW_VIEW_DECIMAL);
    if (value_type == 'X') return FLOW_VIEW_BIT(FLOW_VIEW_BINARY);
    if (value_type == 'R') return FLOW_VIEW_BIT(FLOW_VIEW_REFERENCE);
    return FLOW_ALL_VIEWS;
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
            op = rxas_flow_resolve_opcode(graph->context, item);
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
                               size_t successor, flow_edge_kind kind) {
    size_t index;
    size_t new_capacity;
    flow_edge *new_successors;
    for (index = 0; index < node->successor_count; index++)
        if (node->successors[index].target == successor &&
            node->successors[index].kind == kind) return;
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
    node->successors[node->successor_count].target = successor;
    node->successors[node->successor_count].kind = kind;
    node->successor_count++;
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
        flow_add_successor(graph, node, (size_t)label_index,
                           FLOW_EDGE_NORMAL);
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
                flow_add_successor(graph, node, index + 1, FLOW_EDGE_NORMAL);
            continue;
        }
        node->op = rxas_flow_resolve_opcode(graph->context, item);
        if (!node->op) {
            graph->complete_control_flow = 0;
            node->unknown_successor = 1;
            if (index + 1 < graph->item_count)
                flow_add_successor(graph, node, index + 1, FLOW_EDGE_NORMAL);
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
                flow_add_successor(graph, node, (size_t)label_index,
                                   FLOW_EDGE_NORMAL);
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
                flow_add_successor(graph, node, index + 1, FLOW_EDGE_NORMAL);
                if (node->op->flow != FLOW_NEXT) leaders[index + 1] = 1;
            }
        }
        if (rxop_can_signal(node->op->opcode)) {
            /* Action-aware handlers can be inherited from a caller, so every
             * potentially throwing instruction has a logical skip
             * continuation even when this procedure contains no SIGCALLA.
             * Skip resumes at the already-advanced sequential address. Legacy
             * rewrite consumers explicitly continue to use normal edges only
             * in this infrastructure slice. */
            if (index + 1 < graph->item_count) {
                flow_add_successor(graph, node, index + 1,
                                   FLOW_EDGE_SIGNAL_SKIP);
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
        for (predecessor_index = 0;
             predecessor_index < graph->nodes[index].successor_count;
             predecessor_index++)
            if (graph->nodes[index].successors[predecessor_index].kind ==
                FLOW_EDGE_NORMAL)
                edge_count++;
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
            if (node->successors[predecessor_index].kind == FLOW_EDGE_NORMAL)
                graph->predecessor_offsets[
                        node->successors[predecessor_index].target + 1]++;
    }
    for (index = 1; index <= graph->item_count; index++)
        graph->predecessor_offsets[index] += graph->predecessor_offsets[index - 1];
    for (index = 0; index < graph->item_count; index++) {
        node = &graph->nodes[index];
        for (predecessor_index = 0;
             predecessor_index < node->successor_count;
             predecessor_index++) {
            size_t successor;
            if (node->successors[predecessor_index].kind != FLOW_EDGE_NORMAL)
                continue;
            successor = node->successors[predecessor_index].target;
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
                if (graph->nodes[index].successors[successor_index].kind !=
                    FLOW_EDGE_NORMAL)
                    continue;
                successor = graph->nodes[index].successors[successor_index].target;
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
                flow_set_register_views(graph, node->uses, register_index,
                                        flow_trace_event_views(item));
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
    graph->bit_storage = calloc(stride * 2, sizeof(unsigned long));
    graph->tainted_registers = calloc(graph->register_count ? graph->register_count : 1, 1);
    if (!graph->nodes || !graph->bit_storage || !graph->tainted_registers)
        RX_PANIC_OOM("calloc RXAS whole-procedure flow graph",
                     item_count * sizeof(*graph->nodes) + stride * 2 * sizeof(unsigned long),
                     context && context->file_name ? context->file_name : 0);
    for (index = 0; index < item_count; index++) {
        graph->nodes[index].uses = graph->bit_storage + index * graph->word_count;
        graph->nodes[index].kills = graph->bit_storage + stride + index * graph->word_count;
    }
    flow_build_edges(graph);
    flow_mark_reachable(graph);
    flow_collect_async_handler_targets(graph);
    flow_build_use_kill_and_taint(graph);
    return 1;
}

static void flow_storage_free(flow_storage_analysis *analysis);

static void flow_free_graph(flow_graph *graph) {
    size_t index;
    if (graph->storage) {
        flow_storage_free(graph->storage);
        free(graph->storage);
    }
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

static int flow_is_async_handler_target(const flow_graph *graph,
                                        size_t node_index);

/* A register number identifies a slot in the current frame's mapping table;
 * LINK and SWAP change the value* addressed by that slot.  C2-E2 gives each
 * frame-owned slot a stable base identity (register index + 1) and each
 * dynamic link definition a procedure-local site identity.  Zero is the
 * must-analysis top: the addressed storage is not identical on every path.
 *
 * Site identities are deliberately not equated across distinct LINKATTR or
 * LINKREF instructions.  That loses some true aliases, but never invents one.
 * Non-base identities are also forgotten on a backward edge, preventing one
 * static link site from being mistaken for the same dynamic storage across
 * loop iterations. */
static size_t flow_storage_base_id(int register_index) {
    return register_index >= 0 ? (size_t)register_index + 1 : 0;
}

static size_t flow_storage_site_id(const flow_graph *graph, size_t node_index,
                                   size_t ordinal) {
    return graph->register_count + node_index * 2 + ordinal + 1;
}

static int flow_storage_operand_register(const flow_graph *graph,
                                         instruction_queue *item,
                                         size_t operand_index) {
    Assembler_Token *operand;
    char type;
    if (!item || operand_index >= item->operandCount) return -1;
    operand = rxas_queue_operand(item, operand_index);
    type = flow_register_type(operand);
    if (!type || !operand || operand->token_value.integer < 0) return -1;
    return flow_register_index(graph, type,
            (size_t)operand->token_value.integer);
}

static void flow_storage_set_all_unknown(size_t *state, size_t width) {
    memset(state, 0, width * sizeof(*state));
}

static void flow_storage_forget_nonbase(size_t *state, size_t width) {
    size_t index;
    for (index = 0; index < width; index++)
        if (state[index] > width) state[index] = 0;
}

static void flow_storage_forget_nonbase_except(size_t *state, size_t width,
                                               size_t keep) {
    size_t index;
    for (index = 0; index < width; index++)
        if (state[index] > width && state[index] != keep) state[index] = 0;
}

static void flow_storage_define_site(const flow_graph *graph,
                                     instruction_queue *item,
                                     size_t node_index, size_t operand_index,
                                     size_t ordinal, size_t *state,
                                     flow_storage_stats *stats) {
    int destination;
    destination = flow_storage_operand_register(graph, item, operand_index);
    if (stats) stats->link_transfers++;
    if (destination < 0) {
        if (stats) stats->unknown_links++;
        return;
    }
    state[destination] = flow_storage_site_id(graph, node_index, ordinal);
    if (stats) stats->exact_links++;
}

static void flow_storage_define_throwing_site(
        const flow_graph *graph, instruction_queue *item, size_t node_index,
        size_t operand_index, size_t ordinal, size_t *state,
        flow_storage_stats *stats) {
    flow_storage_define_site(graph, item, node_index, operand_index, ordinal,
                             state, stats);
    if (stats) stats->success_only_links++;
}

static void flow_storage_link_operand(const flow_graph *graph,
                                      instruction_queue *item,
                                      size_t destination_operand,
                                      size_t source_operand, size_t *state,
                                      flow_storage_stats *stats) {
    int destination;
    int source;
    destination = flow_storage_operand_register(graph, item,
                                                destination_operand);
    source = flow_storage_operand_register(graph, item, source_operand);
    if (stats) stats->link_transfers++;
    if (destination < 0 || source < 0 || !state[source]) {
        if (destination >= 0) state[destination] = 0;
        if (stats) stats->unknown_links++;
        return;
    }
    state[destination] = state[source];
    if (stats) stats->exact_links++;
}

static void flow_storage_link_argument(const flow_graph *graph,
                                       instruction_queue *item,
                                       size_t node_index, size_t *state,
                                       flow_storage_stats *stats) {
    int destination;
    int argument;
    destination = flow_storage_operand_register(graph, item, 0);
    argument = -1;
    if (item->operandCount > 1 && item->operand2Token &&
        item->operand2Token->token_type == INT &&
        item->operand2Token->token_value.integer >= 0)
        argument = flow_register_index(graph, 'a',
                (size_t)item->operand2Token->token_value.integer);
    if (stats) stats->link_transfers++;
    if (destination < 0) {
        if (stats) stats->unknown_links++;
        return;
    }
    if (argument >= 0) {
        state[destination] = flow_storage_base_id(argument);
        if (stats) stats->exact_links++;
    }
    else {
        state[destination] = flow_storage_site_id(graph, node_index, 0);
        if (stats) stats->exact_links++;
    }
}

static void flow_storage_swap_operands(const flow_graph *graph,
                                       instruction_queue *item,
                                       size_t first_operand,
                                       size_t second_operand, size_t *state,
                                       flow_storage_stats *stats) {
    int first;
    int second;
    size_t temporary;
    first = flow_storage_operand_register(graph, item, first_operand);
    second = flow_storage_operand_register(graph, item, second_operand);
    if (stats) stats->swap_pairs++;
    if (first < 0 || second < 0) {
        if (first >= 0) state[first] = 0;
        if (second >= 0) state[second] = 0;
        if (stats) stats->unknown_swap_pairs++;
        return;
    }
    if (stats) {
        if (state[first] && state[second]) stats->exact_swap_pairs++;
        else stats->unknown_swap_pairs++;
    }
    temporary = state[first];
    state[first] = state[second];
    state[second] = temporary;
}

static void flow_storage_unlink_operand(const flow_graph *graph,
                                        instruction_queue *item,
                                        size_t operand_index, size_t *state,
                                        flow_storage_stats *stats) {
    int destination;
    destination = flow_storage_operand_register(graph, item, operand_index);
    if (stats) stats->unlink_transfers++;
    if (destination >= 0)
        state[destination] = flow_storage_base_id(destination);
}

static int flow_storage_is_pure_swap_opcode(int opcode) {
    return opcode == OP_SWAP_REG_REG ||
           opcode == OP_SWAPN_REG_REG_REG_REG ||
           opcode == OP_SWAPN_REG_REG_REG_REG_REG_REG ||
           opcode == OP_SWAPN_REG_REG_REG_REG_REG_REG_REG_REG;
}

static void flow_storage_transfer_normal(const flow_graph *graph,
                                         size_t node_index,
                                         const size_t *input, size_t *output,
                                         flow_storage_stats *stats) {
    flow_node *node;
    instruction_queue *item;
    size_t keep;
    size_t operand_index;
    int register_index;

    memcpy(output, input, graph->register_count * sizeof(*output));
    item = &graph->items[node_index];
    node = &graph->nodes[node_index];
    if (item->instrType != OP_CODE) return;
    if (!node->op || node->effects.state != RXOP_EFFECT_CLASSIFIED) {
        flow_storage_set_all_unknown(output, graph->register_count);
        if (stats) stats->unsupported_mapping_ops++;
        return;
    }

    switch (node->op->opcode) {
        case OP_LINK_REG_REG:
            flow_storage_link_operand(graph, item, 0, 1, output, stats);
            return;

        case OP_LINKARG_REG_INT:
            flow_storage_link_argument(graph, item, node_index, output, stats);
            return;

        case OP_LINKATTR_REG_REG_REG:
        case OP_LINKATTR_REG_REG_INT:
        case OP_LINKATTR1_REG_REG_REG:
        case OP_LINKATTR1_REG_REG_INT:
        case OP_LINKREF_REG_REG:
            /* The normal edge completed the link. Signal skip carries a
             * separately modelled pre-link state. */
            flow_storage_define_throwing_site(graph, item, node_index, 0, 0,
                                              output, stats);
            return;

        case OP_LINKARG_REG_REG_INT:
        case OP_METALINKPREG_REG_REG:
            flow_storage_define_site(graph, item, node_index, 0, 0,
                                     output, stats);
            return;

        case OP_SETLINKATTR1_REG_REG_INT_REG:
        case OP_SETLINKATTR1_REG_REG_INT_REG_INT:
        case OP_MINLINKATTR1_REG_REG_INT:
        case OP_MINLINKATTR1_REG_REG_REG_INT:
        case OP_SETLINKILOAD_REG_REG_INT_REG_REG_INT:
            flow_storage_forget_nonbase(output, graph->register_count);
            flow_storage_define_throwing_site(graph, item, node_index, 0, 0,
                                              output, stats);
            return;

        case OP_LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT:
            /* On the normal edge op1 names the selected outer attribute and
             * op5 the selected nested attribute. Partial signal states are
             * handled separately below. */
            flow_storage_forget_nonbase(output, graph->register_count);
            flow_storage_define_throwing_site(graph, item, node_index, 0, 0,
                                              output, stats);
            flow_storage_define_throwing_site(graph, item, node_index, 4, 1,
                                              output, stats);
            return;

        case OP_SWAP_REG_REG:
            flow_storage_swap_operands(graph, item, 0, 1, output, stats);
            return;
        case OP_SWAPN_REG_REG_REG_REG:
            flow_storage_swap_operands(graph, item, 0, 1, output, stats);
            flow_storage_swap_operands(graph, item, 2, 3, output, stats);
            return;
        case OP_SWAPN_REG_REG_REG_REG_REG_REG:
            flow_storage_swap_operands(graph, item, 0, 1, output, stats);
            flow_storage_swap_operands(graph, item, 2, 3, output, stats);
            flow_storage_swap_operands(graph, item, 4, 5, output, stats);
            return;
        case OP_SWAPN_REG_REG_REG_REG_REG_REG_REG_REG:
            flow_storage_swap_operands(graph, item, 0, 1, output, stats);
            flow_storage_swap_operands(graph, item, 2, 3, output, stats);
            flow_storage_swap_operands(graph, item, 4, 5, output, stats);
            flow_storage_swap_operands(graph, item, 6, 7, output, stats);
            return;
        case OP_SETTPSWAP_REG_INT_REG:
            flow_storage_swap_operands(graph, item, 0, 2, output, stats);
            return;
        case OP_LOADSETTPSWAP_REG_INT_REG_INT_REG:
            flow_storage_swap_operands(graph, item, 2, 4, output, stats);
            return;
        case OP_SWAPSETTP_REG_REG_REG_INT:
            flow_storage_swap_operands(graph, item, 0, 1, output, stats);
            return;
        case OP_SWAPSETTPSWAP_REG_REG_REG_INT_REG:
            flow_storage_swap_operands(graph, item, 0, 1, output, stats);
            flow_storage_swap_operands(graph, item, 2, 4, output, stats);
            return;
        case OP_SETTPSWAPSETTPSWAP_REG_INT_REG_REG_REG:
            flow_storage_swap_operands(graph, item, 0, 2, output, stats);
            flow_storage_swap_operands(graph, item, 3, 4, output, stats);
            return;
        case OP_SWAPCALL_REG_FUNC_REG_REG_REG:
            flow_storage_swap_operands(graph, item, 3, 4, output, stats);
            flow_storage_forget_nonbase(output, graph->register_count);
            return;
        case OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG:
            flow_storage_swap_operands(graph, item, 3, 5, output, stats);
            flow_storage_forget_nonbase(output, graph->register_count);
            return;

        case OP_UNLINK_REG:
            flow_storage_unlink_operand(graph, item, 0, output, stats);
            return;
        case OP_UNLINKN_REG_REG:
            flow_storage_unlink_operand(graph, item, 0, output, stats);
            flow_storage_unlink_operand(graph, item, 1, output, stats);
            return;
        case OP_ISETUNLINK_REG_REG:
        case OP_ILOADSETUNLINK_REG_INT:
            flow_storage_unlink_operand(graph, item, 0, output, stats);
            return;
        case OP_IGETUNLINK_REG_REG:
            flow_storage_unlink_operand(graph, item, 1, output, stats);
            return;
        case OP_ISETUNLINKN_REG_REG_REG:
        case OP_ILOADSETUNLINKN_REG_INT_REG:
            flow_storage_unlink_operand(graph, item, 0, output, stats);
            flow_storage_unlink_operand(graph, item, 2, output, stats);
            return;
        case OP_UNLINKBR_REG_ID:
            flow_storage_unlink_operand(graph, item, 0, output, stats);
            return;
        case OP_ILOADSETUNLINKN_REG_REG_INT_REG:
            flow_storage_unlink_operand(graph, item, 1, output, stats);
            flow_storage_unlink_operand(graph, item, 3, output, stats);
            return;
        default:
            break;
    }

    /* Whole-value/attribute lifetime changes can invalidate storage selected
     * by an earlier attribute/reference link.  Keep only a known destination
     * storage itself; nested or unrelated dynamic link identities fail closed. */
    if (node->op->opcode == OP_COPY_REG_REG ||
        node->op->opcode == OP_ACOPY_REG_REG ||
        node->op->opcode == OP_NULL_REG ||
        (node->effects.semantics &
         (RXOP_SEM_LIFETIME_END | RXOP_SEM_REFERENCE_RELEASE |
          RXOP_SEM_INDIRECT_WRITE | RXOP_SEM_CALL |
          RXOP_SEM_DYNAMIC_CALL | RXOP_SEM_OPAQUE))) {
        register_index = flow_storage_operand_register(graph, item, 0);
        keep = register_index >= 0 ? output[register_index] : 0;
        flow_storage_forget_nonbase_except(output, graph->register_count,
                                           keep);
    }

    if (node->effects.semantics &
        (RXOP_SEM_ALIAS_CREATE | RXOP_SEM_ALIAS_RELEASE)) {
        int changed_mapping;
        changed_mapping = 0;
        for (operand_index = 0; operand_index < item->operandCount;
             operand_index++) {
            if (!rxop_effect_writes_operand(&node->effects, operand_index))
                continue;
            register_index = flow_storage_operand_register(graph, item,
                                                            operand_index);
            if (register_index >= 0) output[register_index] = 0;
            changed_mapping = 1;
        }
        if (changed_mapping && stats) stats->unsupported_mapping_ops++;
    }
}

/* Mapping state at the point where a signalling instruction raised a signal.
 * This state feeds SIGCALLA skip at the sequential successor. Simple
 * attribute/reference links validate before installing their destination,
 * whereas fused forms may already have resized attribute storage or installed
 * an earlier link. */
static void flow_storage_transfer_signal(const flow_graph *graph,
                                         size_t node_index,
                                         const size_t *input,
                                         size_t *output) {
    flow_node *node;
    instruction_queue *item;
    size_t operand_index;
    int register_index;

    memcpy(output, input, graph->register_count * sizeof(*output));
    item = &graph->items[node_index];
    node = &graph->nodes[node_index];
    if (item->instrType != OP_CODE || !node->op ||
        node->effects.state != RXOP_EFFECT_CLASSIFIED) {
        flow_storage_set_all_unknown(output, graph->register_count);
        return;
    }

    switch (node->op->opcode) {
        case OP_LINKATTR_REG_REG_REG:
        case OP_LINKATTR_REG_REG_INT:
        case OP_LINKATTR1_REG_REG_REG:
        case OP_LINKATTR1_REG_REG_INT:
        case OP_LINKREF_REG_REG:
            /* Validation signals before changing the destination mapping. */
            return;

        case OP_SETLINKATTR1_REG_REG_INT_REG:
        case OP_SETLINKATTR1_REG_REG_INT_REG_INT:
        case OP_MINLINKATTR1_REG_REG_INT:
        case OP_MINLINKATTR1_REG_REG_REG_INT:
        case OP_SETLINKILOAD_REG_REG_INT_REG_REG_INT:
            /* Attribute storage may already have been resized, but the link
             * destination is not installed until after the checked index. */
            flow_storage_forget_nonbase(output, graph->register_count);
            return;

        case OP_LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT:
            /* The first range check can signal before op1 is linked; later
             * checks can signal after op1 is linked and attributes resized.
             * Meet those partial states by forgetting op1 and all dynamic
             * attribute/reference identities. op5 is never linked on failure. */
            flow_storage_forget_nonbase(output, graph->register_count);
            register_index = flow_storage_operand_register(graph, item, 0);
            if (register_index >= 0) output[register_index] = 0;
            return;

        case OP_SWAPCALL_REG_FUNC_REG_REG_REG:
            flow_storage_swap_operands(graph, item, 3, 4, output, 0);
            flow_storage_forget_nonbase(output, graph->register_count);
            return;

        case OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG:
            flow_storage_swap_operands(graph, item, 3, 5, output, 0);
            flow_storage_forget_nonbase(output, graph->register_count);
            return;

        default:
            break;
    }

    if (node->effects.semantics &
        (RXOP_SEM_LIFETIME_END | RXOP_SEM_REFERENCE_RELEASE |
         RXOP_SEM_INDIRECT_WRITE | RXOP_SEM_CALL |
         RXOP_SEM_DYNAMIC_CALL | RXOP_SEM_OPAQUE))
        flow_storage_forget_nonbase(output, graph->register_count);

    if (node->effects.semantics &
        (RXOP_SEM_ALIAS_CREATE | RXOP_SEM_ALIAS_RELEASE)) {
        for (operand_index = 0; operand_index < item->operandCount;
             operand_index++) {
            if (!rxop_effect_writes_operand(&node->effects, operand_index))
                continue;
            register_index = flow_storage_operand_register(graph, item,
                                                            operand_index);
            if (register_index >= 0) output[register_index] = 0;
        }
    }
}

static int flow_storage_merge(size_t *destination, const size_t *source,
                              size_t width) {
    size_t index;
    int changed;
    changed = 0;
    for (index = 0; index < width; index++) {
        if (destination[index] && destination[index] != source[index]) {
            destination[index] = 0;
            changed = 1;
        }
    }
    return changed;
}

static void flow_storage_enqueue(flow_storage_analysis *analysis, size_t node,
                                 size_t *tail) {
    if (analysis->queued[node]) return;
    analysis->queue[*tail] = node;
    *tail = (*tail + 1) % analysis->queue_capacity;
    analysis->queued[node] = 1;
}

static int flow_storage_analyse(const flow_graph *graph,
                                flow_storage_analysis *analysis) {
    size_t cells;
    size_t width;
    size_t index;
    size_t successor_index;
    size_t successor;
    size_t head;
    size_t tail;
    size_t node_index;
    size_t *input;
    size_t *successor_input;
    size_t *normal_output;
    size_t *signal_output;
    size_t *edge_output;
    flow_edge edge;

    memset(analysis, 0, sizeof(*analysis));
    analysis->graph = graph;
    width = graph->register_count;
    if (!graph->item_count || !width || !graph->complete_control_flow)
        return 0;
    if (graph->item_count > FLOW_MAX_STORAGE_IDENTITY_CELLS / width)
        return 0;
    cells = graph->item_count * width;
    analysis->in = calloc(cells, sizeof(*analysis->in));
    analysis->has_in = calloc(graph->item_count, 1);
    analysis->queued = calloc(graph->item_count, 1);
    analysis->queue_capacity = graph->item_count + 1;
    analysis->queue = calloc(analysis->queue_capacity,
                             sizeof(*analysis->queue));
    normal_output = calloc(width, sizeof(*normal_output));
    signal_output = calloc(width, sizeof(*signal_output));
    edge_output = calloc(width, sizeof(*edge_output));
    if (!analysis->in || !analysis->has_in || !analysis->queued ||
        !analysis->queue || !normal_output || !signal_output || !edge_output)
        RX_PANIC_OOM("calloc RXAS symbolic storage analysis",
                     cells * sizeof(*analysis->in), 0);

    input = analysis->in;
    for (index = 0; index < width; index++)
        input[index] = flow_storage_base_id((int)index);
    analysis->has_in[0] = 1;
    head = 0;
    tail = 0;
    flow_storage_enqueue(analysis, 0, &tail);

    /* A registered same-frame handler may be entered after any executable
     * instruction, including after a partially completed mapping operation.
     * Its entry mapping is therefore unknown unless a future exceptional-edge
     * model proves more. */
    for (index = 0; index < graph->async_handler_target_count; index++) {
        node_index = graph->async_handler_targets[index];
        input = analysis->in + node_index * width;
        flow_storage_set_all_unknown(input, width);
        analysis->has_in[node_index] = 1;
        flow_storage_enqueue(analysis, node_index, &tail);
    }

    while (head != tail) {
        node_index = analysis->queue[head];
        head = (head + 1) % analysis->queue_capacity;
        analysis->queued[node_index] = 0;
        input = analysis->in + node_index * width;
        flow_storage_transfer_normal(graph, node_index, input, normal_output,
                                     0);
        flow_storage_transfer_signal(graph, node_index, input, signal_output);
        for (successor_index = 0;
             successor_index < graph->nodes[node_index].successor_count;
             successor_index++) {
            edge = graph->nodes[node_index].successors[successor_index];
            successor = edge.target;
            memcpy(edge_output,
                   edge.kind == FLOW_EDGE_NORMAL ? normal_output : signal_output,
                   width * sizeof(*edge_output));
            /* A normal backedge can execute a static link site in a new loop
             * iteration, so its dynamic identity cannot be reused. */
            if (edge.kind == FLOW_EDGE_NORMAL && successor <= node_index)
                flow_storage_forget_nonbase(edge_output, width);
            successor_input = analysis->in + successor * width;
            if (!analysis->has_in[successor]) {
                memcpy(successor_input, edge_output,
                       width * sizeof(*successor_input));
                analysis->has_in[successor] = 1;
                flow_storage_enqueue(analysis, successor, &tail);
            }
            else if (flow_storage_merge(successor_input, edge_output, width))
                flow_storage_enqueue(analysis, successor, &tail);
        }
    }

    free(normal_output);
    free(signal_output);
    free(edge_output);
    return 1;
}

static int flow_storage_attach(flow_graph *graph) {
    if (graph->storage) return 1;
    graph->storage = calloc(1, sizeof(*graph->storage));
    if (!graph->storage)
        RX_PANIC_OOM("calloc RXAS graph storage service",
                     sizeof(*graph->storage), 0);
    if (flow_storage_analyse(graph, graph->storage)) return 1;
    free(graph->storage);
    graph->storage = 0;
    return 0;
}

static void flow_storage_free(flow_storage_analysis *analysis) {
    free(analysis->in);
    free(analysis->has_in);
    free(analysis->queued);
    free(analysis->queue);
    memset(analysis, 0, sizeof(*analysis));
}

static int flow_storage_state_equal(const size_t *left, const size_t *right,
                                    size_t width) {
    return memcmp(left, right, width * sizeof(*left)) == 0;
}

static int flow_storage_node_observes_permutation(const flow_graph *graph,
                                                  size_t node_index,
                                                  const size_t *checkpoint,
                                                  const size_t *current) {
    size_t register_index;
    size_t view;
    const flow_node *node;
    const instruction_queue *item;
    node = &graph->nodes[node_index];
    item = &graph->items[node_index];
    if (item->instrType == OP_CODE) {
        if (!node->op || node->op->flow != FLOW_NEXT ||
            node->effects.optimizer_barrier || node->effects.semantics != 0 ||
            rxop_can_signal(node->op->opcode))
            return 1;
    }
    for (register_index = 0; register_index < graph->register_count;
         register_index++) {
        if (checkpoint[register_index] == current[register_index]) continue;
        for (view = 0; view < FLOW_VIEW_COUNT; view++) {
            size_t bit;
            bit = register_index * FLOW_VIEW_COUNT + view;
            if (flow_test_bit(node->uses, bit) || flow_test_bit(node->kills, bit))
                return 1;
        }
    }
    return 0;
}

static void flow_storage_count_swap_round_trips(
        const flow_graph *graph, const flow_storage_analysis *analysis,
        flow_storage_stats *stats) {
    size_t index;
    size_t block;
    size_t pending_instructions;
    size_t *checkpoint;
    size_t *current;
    size_t *next;
    flow_node *node;
    instruction_queue *item;
    int active;

    checkpoint = calloc(graph->register_count, sizeof(*checkpoint));
    current = calloc(graph->register_count, sizeof(*current));
    next = calloc(graph->register_count, sizeof(*next));
    if (!checkpoint || !current || !next)
        RX_PANIC_OOM("calloc RXAS swap round-trip proof",
                     graph->register_count * sizeof(*checkpoint) * 3, 0);
    block = (size_t)-1;
    pending_instructions = 0;
    active = 0;
    for (index = 0; index < graph->item_count; index++) {
        node = &graph->nodes[index];
        item = &graph->items[index];
        if (!analysis->has_in[index]) continue;
        if (node->block != block) {
            block = node->block;
            memcpy(current, analysis->in + index * graph->register_count,
                   graph->register_count * sizeof(*current));
            memcpy(checkpoint, current,
                   graph->register_count * sizeof(*checkpoint));
            pending_instructions = 0;
            active = 0;
        }
        if (item->instrType == OP_CODE && node->op &&
            flow_storage_is_pure_swap_opcode(node->op->opcode)) {
            if (!active) {
                memcpy(checkpoint, current,
                       graph->register_count * sizeof(*checkpoint));
                pending_instructions = 0;
                active = 1;
            }
            flow_storage_transfer_normal(graph, index, current, next, 0);
            memcpy(current, next, graph->register_count * sizeof(*current));
            pending_instructions++;
            if (pending_instructions >= 2 &&
                flow_storage_state_equal(checkpoint, current,
                                         graph->register_count)) {
                stats->swap_round_trips++;
                stats->round_trip_instructions += pending_instructions;
                pending_instructions = 0;
                active = 0;
            }
            continue;
        }
        if (active && flow_storage_node_observes_permutation(
                              graph, index, checkpoint, current)) {
            active = 0;
            pending_instructions = 0;
        }
        flow_storage_transfer_normal(graph, index, current, next, 0);
        memcpy(current, next, graph->register_count * sizeof(*current));
        if (!active) memcpy(checkpoint, current,
                            graph->register_count * sizeof(*checkpoint));
    }
    free(checkpoint);
    free(current);
    free(next);
}

static void flow_storage_collect_stats(const flow_graph *graph,
                                       const flow_storage_analysis *analysis,
                                       flow_storage_stats *stats) {
    size_t index;
    size_t register_index;
    size_t predecessor_count;
    size_t successor_index;
    size_t *output;
    size_t *input;
    instruction_queue *item;
    flow_node *node;
    int destination;
    int source;

    memset(stats, 0, sizeof(*stats));
    stats->signal_handler_entries = graph->async_handler_target_count;
    output = calloc(graph->register_count, sizeof(*output));
    if (!output)
        RX_PANIC_OOM("calloc RXAS storage statistics",
                     graph->register_count * sizeof(*output), 0);
    for (index = 0; index < graph->item_count; index++) {
        node = &graph->nodes[index];
        if (!analysis->has_in[index]) continue;
        input = analysis->in + index * graph->register_count;
        stats->reachable_nodes++;
        for (register_index = 0; register_index < graph->register_count;
             register_index++) {
            stats->state_cells++;
            if (input[register_index]) {
                stats->exact_cells++;
                if (input[register_index] !=
                    flow_storage_base_id((int)register_index))
                    stats->aliased_cells++;
            }
            else stats->unknown_cells++;
        }
        predecessor_count = graph->predecessor_offsets[index + 1] -
                            graph->predecessor_offsets[index];
        if (predecessor_count > 1 || flow_is_async_handler_target(graph, index)) {
            stats->join_nodes++;
            for (register_index = 0; register_index < graph->register_count;
                 register_index++)
                if (!input[register_index]) stats->join_unknown_cells++;
        }
        for (successor_index = 0;
             successor_index < node->successor_count;
             successor_index++) {
            flow_edge edge;
            edge = node->successors[successor_index];
            if (edge.kind == FLOW_EDGE_NORMAL) {
                stats->normal_edges++;
                if (edge.target <= index) stats->backward_edges++;
            }
            else if (edge.kind == FLOW_EDGE_SIGNAL_SKIP)
                stats->signal_skip_edges++;
        }

        flow_storage_transfer_normal(graph, index, input, output, stats);
        item = &graph->items[index];
        if (item->instrType != OP_CODE || !node->op ||
            node->op->opcode != OP_COPY_REG_REG) continue;
        destination = flow_storage_operand_register(graph, item, 0);
        source = flow_storage_operand_register(graph, item, 1);
        if (destination < 0 || source < 0) continue;
        stats->full_copies++;
        if (input[destination] && input[source]) {
            stats->exact_full_copies++;
            if (input[destination] == flow_storage_base_id(destination) &&
                input[source] == flow_storage_base_id(source))
                stats->base_full_copies++;
        }
        if (input[destination] && input[destination] == input[source])
            stats->alias_self_copies++;
        if (!graph->tainted_registers[destination]) continue;
        stats->tainted_full_copies++;
        if (input[destination] && input[source]) {
            stats->exact_tainted_full_copies++;
            if (input[destination] == flow_storage_base_id(destination))
                stats->base_destination_tainted_full_copies++;
        }
    }
    flow_storage_count_swap_round_trips(graph, analysis, stats);
    free(output);
}

static void flow_debug_storage_identity(const flow_graph *graph) {
    flow_storage_stats stats;
    size_t cells;
    if (!graph->context->debug_mode) return;
    if (!graph->complete_control_flow || !graph->register_count ||
        graph->item_count >
            FLOW_MAX_STORAGE_IDENTITY_CELLS / graph->register_count) {
        cells = graph->register_count &&
                graph->item_count <= (size_t)-1 / graph->register_count
                ? graph->item_count * graph->register_count : (size_t)-1;
        fprintf(stderr,
                "NR27 identity procedure=%s status=skipped reason=%s "
                "cells=%llu limit=%llu\n",
                graph->context->current_proc_name
                        ? graph->context->current_proc_name : "(directives)",
                !graph->complete_control_flow ? "incomplete-control-flow" :
                                                "analysis-bound",
                (unsigned long long)cells,
                (unsigned long long)FLOW_MAX_STORAGE_IDENTITY_CELLS);
        return;
    }
    if (!graph->storage) return;
    flow_storage_collect_stats(graph, graph->storage, &stats);
    fprintf(stderr,
            "NR27 identity procedure=%s status=complete nodes=%llu cells=%llu "
            "exact=%llu unknown=%llu aliased=%llu links=%llu/%llu "
            "success-only-links=%llu "
            "swaps=%llu/%llu unlinks=%llu joins=%llu join-unknown=%llu "
            "backedges=%llu unsupported=%llu full-copy=%llu "
            "full-copy-exact=%llu full-copy-base=%llu "
            "tainted-full=%llu tainted-full-exact=%llu "
            "tainted-full-base=%llu alias-self-copy=%llu "
            "swap-roundtrip=%llu roundtrip-instructions=%llu "
            "edges=%llu/%llu handler-entries=%llu\n",
            graph->context->current_proc_name
                    ? graph->context->current_proc_name : "(directives)",
            (unsigned long long)stats.reachable_nodes,
            (unsigned long long)stats.state_cells,
            (unsigned long long)stats.exact_cells,
            (unsigned long long)stats.unknown_cells,
            (unsigned long long)stats.aliased_cells,
            (unsigned long long)stats.exact_links,
            (unsigned long long)stats.unknown_links,
            (unsigned long long)stats.success_only_links,
            (unsigned long long)stats.exact_swap_pairs,
            (unsigned long long)stats.unknown_swap_pairs,
            (unsigned long long)stats.unlink_transfers,
            (unsigned long long)stats.join_nodes,
            (unsigned long long)stats.join_unknown_cells,
            (unsigned long long)stats.backward_edges,
            (unsigned long long)stats.unsupported_mapping_ops,
            (unsigned long long)stats.full_copies,
            (unsigned long long)stats.exact_full_copies,
            (unsigned long long)stats.base_full_copies,
            (unsigned long long)stats.tainted_full_copies,
            (unsigned long long)stats.exact_tainted_full_copies,
            (unsigned long long)stats.base_destination_tainted_full_copies,
            (unsigned long long)stats.alias_self_copies,
            (unsigned long long)stats.swap_round_trips,
            (unsigned long long)stats.round_trip_instructions,
            (unsigned long long)stats.normal_edges,
            (unsigned long long)stats.signal_skip_edges,
            (unsigned long long)stats.signal_handler_entries);
}

static int flow_is_async_handler_target(const flow_graph *graph, size_t node_index) {
    size_t index;
    for (index = 0; index < graph->async_handler_target_count; index++)
        if (graph->async_handler_targets[index] == node_index) return 1;
    return 0;
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

static RxasFlowProcedure *flow_build_proof_procedure(
        const flow_graph *graph, unsigned long epoch);
static int flow_proof_reason_unavailable(RxasFlowProofReason reason);

typedef struct flow_proof_session {
    RxasFlowProcedure *procedure;
    const RxasFlowProofService *proof;
    unsigned long epoch;
    int attempted;
} flow_proof_session;

static const RxasFlowProofService *flow_proof_session_require(
        flow_proof_session *session, const flow_graph *graph);
static void flow_proof_session_destroy(flow_proof_session *session);


static char flow_proof_register_type(RxasFlowRegister reg) {
    if (reg.register_class == RXAS_FLOW_REGISTER_ARGUMENT) return 'a';
    if (reg.register_class == RXAS_FLOW_REGISTER_GLOBAL) return 'g';
    return 'r';
}

static int flow_operand_matches_proof_register(
        const Assembler_Token *token, RxasFlowRegister reg) {
    return token && flow_register_type(token) ==
                    flow_proof_register_type(reg) &&
           token->token_value.integer >= 0 &&
           (size_t)token->token_value.integer == reg.number;
}

static int flow_trace_event_matches_deletion(
        const instruction_queue *item,
        const RxasFlowTraceDeletion *deletion) {
    char register_type;
    char value_type;
    if (!item || !deletion || item->instrType != TRACE_EVENT ||
        deletion->component != RXOP_COMPONENT_INTEGER ||
        !flow_token_is_string(item->operand2Token, "R") ||
        !item->operand3Token || item->operand3Token->token_type != STRING ||
        !item->operand3Token->token_value.string[0] ||
        item->operand3Token->token_value.string[1] ||
        !item->operand4Token || item->operand4Token->token_type != STRING ||
        !item->operand4Token->token_value.string[0] ||
        item->operand4Token->token_value.string[1] ||
        !item->operand5Token || item->operand5Token->token_type != INT ||
        item->operand5Token->token_value.integer < 0)
        return 0;
    value_type = (char)toupper((unsigned char)
            item->operand3Token->token_value.string[0]);
    if (value_type != 'B' && value_type != 'I') return 0;
    register_type = (char)tolower((unsigned char)
            item->operand4Token->token_value.string[0]);
    return register_type == flow_proof_register_type(
                                    deletion->expected_register) &&
           (size_t)item->operand5Token->token_value.integer ==
                    deletion->expected_register.number;
}

static size_t flow_propagate_one_copy_ssa(
        flow_graph *graph, size_t copy_index, flow_stats *stats,
        flow_proof_session *session) {
    const RxasFlowProofService *proof;
    const RxasFlowRecord *record;
    RxasFlowTypedCopyPlan plan;
    RxasFlowOperandRewrite rewrite;
    instruction_queue *copy;
    Assembler_Token *source;
    size_t instruction_id;
    size_t rewrite_index;
    size_t compare_redirects;
    if (!graph || !stats || !session || copy_index >= graph->item_count ||
        graph->items[copy_index].instrType != OP_CODE ||
        !graph->nodes[copy_index].op ||
        (graph->nodes[copy_index].op->opcode != OP_ICOPY_REG_REG &&
         graph->nodes[copy_index].op->opcode != OP_FCOPY_REG_REG &&
         graph->nodes[copy_index].op->opcode != OP_SCOPY_REG_REG))
        return 0;
    copy = &graph->items[copy_index];
    source = rxas_queue_operand(copy, 1);
    proof = flow_proof_session_require(session, graph);
    record = session->procedure ? rxas_flow_procedure_record(
            session->procedure, session->epoch, copy_index) : 0;
    instruction_id = record ? record->instruction_id : RXAS_FLOW_ID_NONE;
    if (!proof || instruction_id == RXAS_FLOW_ID_NONE ||
        !rxas_flow_prove_typed_copy_redirect(
                proof, session->epoch, instruction_id, &plan)) {
        stats->rejected_effect++;
        return 0;
    }
    if (!plan.proved) {
        if (graph->context->debug_mode)
            fprintf(stderr,
                    "PERF3 typed-copy-proof procedure=%s candidate=%llu:%s "
                    "proved=0 reason=%s\n",
                    graph->context->current_proc_name
                            ? graph->context->current_proc_name
                            : "(directives)",
                    (unsigned long long)copy_index,
                    graph->nodes[copy_index].op->mnemonic,
                    rxas_flow_proof_reason_name(plan.reason));
        if (flow_proof_reason_unavailable(plan.reason))
            stats->rejected_effect++;
        else stats->rejected_live++;
        return 0;
    }

    /* Validate the complete immutable plan against the still-current queue
     * before changing any operand. Disjoint batches then either apply every
     * redirect and delete the generator, or perform no mutation. */
    for (rewrite_index = 0; rewrite_index < plan.rewrite_count;
         rewrite_index++) {
        instruction_queue *use_item;
        Assembler_Token *use_operand;
        if (!rxas_flow_typed_copy_plan_rewrite(
                    proof, session->epoch, &plan,
                    rewrite_index, &rewrite) ||
            rewrite.record_id >= graph->item_count) {
            stats->rejected_effect++;
            return 0;
        }
        use_item = &graph->items[rewrite.record_id];
        use_operand = rxas_queue_operand(use_item, rewrite.operand_index);
        if (use_item->instrType != OP_CODE ||
            rewrite.operand_index >= use_item->operandCount ||
            !flow_operand_matches_proof_register(
                    use_operand, rewrite.expected_register) ||
            !flow_operand_matches_proof_register(
                    source, rewrite.replacement_register)) {
            stats->rejected_effect++;
            return 0;
        }
    }
    compare_redirects = 0;
    for (rewrite_index = 0; rewrite_index < plan.rewrite_count;
         rewrite_index++) {
        rxas_flow_typed_copy_plan_rewrite(
                proof, session->epoch, &plan, rewrite_index, &rewrite);
        flow_set_operand(&graph->items[rewrite.record_id],
                         rewrite.operand_index, source);
        stats->operands_redirected++;
        if (graph->nodes[rewrite.record_id].op &&
            flow_is_compare_opcode(
                    graph->nodes[rewrite.record_id].op->opcode))
            compare_redirects++;
    }
    copy->instrType = EMPTY;
    stats->typed_copies_removed++;
    stats->compare_preparations_removed += compare_redirects;
    flow_debug_accept(graph, copy_index,
                      "all-uses-redirected-ssa", plan.rewrite_count);
    return 1;
}

static size_t flow_propagate_copies(
        flow_graph *graph, flow_stats *stats,
        flow_proof_session *session) {
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
        changed = flow_propagate_one_copy_ssa(
                graph, index, stats, session);
        if (!changed) continue;
        removed += changed;
        if (destination_register >= 0)
            claimed_registers[destination_register] = 1;
        if (source_register >= 0) claimed_registers[source_register] = 1;
    }
    free(claimed_registers);
    return removed;
}

/* M06 is an immutable proof-plan consumer. The proof service owns component,
 * storage, cleanup, signal and observation policy; this queue-facing layer
 * only demand-filters adjacent typed copies, validates the complete plan and
 * applies disjoint producer/copy substitutions atomically. */
static size_t flow_forward_producer_destination(flow_graph *graph,
                                                flow_stats *stats,
                                                flow_proof_session *session) {
    const RxasFlowProofService *proof;
    const RxasFlowRecord *producer_record;
    const RxasFlowRecord *copy_record;
    RxasFlowProducerDestinationPlan plan;
    size_t producer_index;
    size_t producer_instruction;
    size_t copy_instruction;
    size_t forwarded;
    instruction_queue *producer;
    instruction_queue *copy;
    Assembler_Token *temporary;
    Assembler_Token *destination;
    Assembler_Token *copy_source;
    int temporary_register;
    int destination_register;
    unsigned char *claimed_registers;

    claimed_registers = calloc(graph->register_count ? graph->register_count : 1, 1);
    if (!claimed_registers)
        RX_PANIC_OOM("calloc RXAS producer-forward batch",
                     graph->register_count, 0);
    forwarded = 0;
    for (producer_index = 0; producer_index + 1 < graph->item_count;
         producer_index++) {
        producer = &graph->items[producer_index];
        copy = &graph->items[producer_index + 1];
        if (!graph->nodes[producer_index].reachable ||
            !graph->nodes[producer_index + 1].reachable ||
            producer->instrType != OP_CODE || copy->instrType != OP_CODE ||
            !graph->nodes[producer_index].op ||
            !graph->nodes[producer_index + 1].op ||
            producer->operandCount == 0 || copy->operandCount != 2 ||
            (graph->nodes[producer_index + 1].op->opcode !=
                    OP_ICOPY_REG_REG &&
             graph->nodes[producer_index + 1].op->opcode !=
                    OP_FCOPY_REG_REG))
            continue;
        temporary = rxas_queue_operand(producer, 0);
        destination = rxas_queue_operand(copy, 0);
        copy_source = rxas_queue_operand(copy, 1);
        if (flow_register_type(temporary) != 'r' ||
            flow_register_type(copy_source) != 'r' ||
            flow_register_type(destination) != 'r')
            continue;
        temporary_register = flow_register_index(graph, 'r',
                (size_t)temporary->token_value.integer);
        destination_register = flow_register_index(graph, 'r',
                (size_t)destination->token_value.integer);
        if (temporary_register < 0 || destination_register < 0 ||
            temporary_register == destination_register)
            continue;
        if (claimed_registers[temporary_register] ||
            claimed_registers[destination_register])
            continue;
        proof = flow_proof_session_require(session, graph);
        producer_record = session->procedure ? rxas_flow_procedure_record(
                session->procedure, session->epoch, producer_index) : 0;
        copy_record = session->procedure ? rxas_flow_procedure_record(
                session->procedure, session->epoch, producer_index + 1) : 0;
        producer_instruction = producer_record
                ? producer_record->instruction_id : RXAS_FLOW_ID_NONE;
        copy_instruction = copy_record
                ? copy_record->instruction_id : RXAS_FLOW_ID_NONE;
        if (!proof || producer_instruction == RXAS_FLOW_ID_NONE ||
            copy_instruction == RXAS_FLOW_ID_NONE ||
            !rxas_flow_prove_producer_destination_forward(
                    proof, session->epoch, producer_instruction,
                    copy_instruction, &plan)) {
            stats->producer_proof_unavailable++;
            stats->rejected_effect++;
            continue;
        }
        stats->producer_proof_queries++;
        if (!plan.proved) {
            if (graph->context->debug_mode)
                fprintf(stderr,
                        "PERF3 producer-forward-proof procedure=%s "
                        "candidate=%llu:%s proved=0 reason=%s\n",
                        graph->context->current_proc_name
                                ? graph->context->current_proc_name
                                : "(directives)",
                        (unsigned long long)producer_index,
                        graph->nodes[producer_index].op->mnemonic,
                        rxas_flow_proof_reason_name(plan.reason));
            if (flow_proof_reason_unavailable(plan.reason)) {
                stats->producer_proof_unavailable++;
                stats->rejected_effect++;
            }
            else {
                stats->producer_proof_rejected++;
                if (plan.reason == RXAS_FLOW_PROOF_ADDRESS_OBSERVED)
                    stats->rejected_trace++;
                else stats->rejected_live++;
            }
            continue;
        }
        if (plan.producer_record_id != producer_index ||
            plan.copy_record_id != producer_index + 1 ||
            plan.producer_rewrite.record_id != producer_index ||
            plan.producer_rewrite.instruction_id != producer_instruction ||
            plan.producer_rewrite.operand_index != 0 ||
            !flow_operand_matches_proof_register(
                    temporary,
                    plan.producer_rewrite.expected_register) ||
            !flow_operand_matches_proof_register(
                    destination,
                    plan.producer_rewrite.replacement_register) ||
            !flow_operand_matches_proof_register(
                    copy_source,
                    plan.producer_rewrite.expected_register)) {
            stats->producer_proof_unavailable++;
            stats->rejected_effect++;
            continue;
        }
        flow_set_operand(producer, 0, destination);
        copy->instrType = EMPTY;
        flow_debug_accept(graph, producer_index,
                          "producer-destination-forwarded-ssa", 1);
        stats->producer_proof_proved++;
        stats->producer_destinations_forwarded++;
        stats->typed_copies_removed++;
        claimed_registers[temporary_register] = 1;
        claimed_registers[destination_register] = 1;
        forwarded++;
    }
    free(claimed_registers);
    return forwarded;
}

typedef struct flow_duplicate_linked_read_candidate {
    size_t record_id;
    size_t instruction_id;
    int link_opcode;
    int copy_opcode;
} flow_duplicate_linked_read_candidate;

static int flow_duplicate_linked_read_copy_opcode(int opcode) {
    return opcode == OP_COPY_REG_REG || opcode == OP_BCOPY_REG_REG ||
           opcode == OP_ICOPY_REG_REG || opcode == OP_SCOPY_REG_REG ||
           opcode == OP_FCOPY_REG_REG || opcode == OP_DCOPY_REG_REG;
}

/* K02/K03 selection is a linear demand filter. The immutable proof owns the
 * two exact triples, storage/path identity, component/cursor equivalence and
 * LINKATTR1 range safety. Apply one plan per epoch, then rebuild before
 * considering an overlapping repeated-read chain. */
static size_t flow_reuse_duplicate_linked_read(
        flow_graph *graph, flow_stats *stats, flow_proof_session *session) {
    flow_duplicate_linked_read_candidate *candidates;
    size_t candidate_count;
    size_t record_id;
    size_t second_index;
    const RxasFlowProofService *proof;
    candidate_count = 0;
    candidates = calloc(graph->item_count, sizeof(*candidates));
    if (!candidates)
        RX_PANIC_OOM("calloc duplicate linked-read candidates",
                     graph->item_count * sizeof(*candidates), 0);
    for (record_id = 0; record_id + 2 < graph->item_count; record_id++) {
        int link_opcode;
        int copy_opcode;
        if (!graph->nodes[record_id].reachable ||
            !graph->nodes[record_id + 1].reachable ||
            !graph->nodes[record_id + 2].reachable ||
            graph->items[record_id].instrType != OP_CODE ||
            graph->items[record_id + 1].instrType != OP_CODE ||
            graph->items[record_id + 2].instrType != OP_CODE ||
            !graph->nodes[record_id].op ||
            !graph->nodes[record_id + 1].op ||
            !graph->nodes[record_id + 2].op)
            continue;
        link_opcode = graph->nodes[record_id].op->opcode;
        copy_opcode = graph->nodes[record_id + 1].op->opcode;
        if ((link_opcode != OP_LINK_REG_REG &&
             link_opcode != OP_LINKATTR1_REG_REG_INT) ||
            !flow_duplicate_linked_read_copy_opcode(copy_opcode) ||
            graph->nodes[record_id + 2].op->opcode != OP_UNLINK_REG)
            continue;
        candidates[candidate_count].record_id = record_id;
        candidates[candidate_count].link_opcode = link_opcode;
        candidates[candidate_count].copy_opcode = copy_opcode;
        candidate_count++;
    }
    if (candidate_count < 2) {
        free(candidates);
        return 0;
    }
    proof = flow_proof_session_require(session, graph);
    if (!proof) {
        free(candidates);
        stats->rejected_effect++;
        return 0;
    }
    for (record_id = 0; record_id < candidate_count; record_id++) {
        const RxasFlowRecord *record;
        record = rxas_flow_procedure_record(
                session->procedure, session->epoch,
                candidates[record_id].record_id);
        candidates[record_id].instruction_id = record
                ? record->instruction_id : RXAS_FLOW_ID_NONE;
    }
    for (second_index = 1; second_index < candidate_count; second_index++) {
        size_t first_index;
        for (first_index = second_index; first_index; ) {
            RxasFlowDuplicateLinkedReadPlan plan;
            instruction_queue *first_copy;
            instruction_queue *second_link;
            instruction_queue *second_copy;
            instruction_queue *second_unlink;
            Assembler_Token *replacement_operands[2];
            first_index--;
            if (candidates[first_index].link_opcode !=
                        candidates[second_index].link_opcode ||
                candidates[first_index].copy_opcode !=
                        candidates[second_index].copy_opcode ||
                candidates[first_index].instruction_id ==
                        RXAS_FLOW_ID_NONE ||
                candidates[second_index].instruction_id ==
                        RXAS_FLOW_ID_NONE)
                continue;
            memset(&plan, 0, sizeof(plan));
            if (!rxas_flow_prove_duplicate_linked_read(
                        proof, session->epoch,
                        candidates[first_index].instruction_id,
                        candidates[second_index].instruction_id, &plan)) {
                stats->rejected_effect++;
                continue;
            }
            stats->duplicate_linked_read_queries++;
            if (!plan.proved) {
                stats->duplicate_linked_read_rejected++;
                if (graph->context->debug_mode)
                    fprintf(stderr,
                            "PERF3 duplicate-linked-read-proof procedure=%s "
                            "first=%llu second=%llu link=%s copy=%s "
                            "proved=0 reason=%s storage=%llu/%llu "
                            "owner=%llu/%llu count=%llu/%llu ref=%llu/%llu "
                            "component=0x%x value=%llu/%llu\n",
                            graph->context->current_proc_name
                                    ? graph->context->current_proc_name
                                    : "(directives)",
                            (unsigned long long)
                                    candidates[first_index].record_id,
                            (unsigned long long)
                                    candidates[second_index].record_id,
                            graph->nodes[candidates[second_index].record_id]
                                    .op->mnemonic,
                            graph->nodes[candidates[second_index].record_id + 1]
                                    .op->mnemonic,
                            rxas_flow_proof_reason_name(plan.reason),
                            (unsigned long long)plan.linked_storage_id,
                            (unsigned long long)
                                    plan.candidate_linked_storage_id,
                            (unsigned long long)plan.owner_storage_id,
                            (unsigned long long)
                                    plan.candidate_owner_storage_id,
                            (unsigned long long)plan.attribute_count_value_id,
                            (unsigned long long)
                                    plan.candidate_attribute_count_value_id,
                            (unsigned long long)plan.reference_effect_id,
                            (unsigned long long)
                                    plan.candidate_reference_effect_id,
                            plan.rejected_component,
                            (unsigned long long)plan.first_value_id,
                            (unsigned long long)plan.candidate_value_id);
                continue;
            }
            first_copy = &graph->items[plan.first_copy_record_id];
            second_link = &graph->items[plan.second_link_record_id];
            second_copy = &graph->items[plan.second_copy_record_id];
            second_unlink = &graph->items[plan.second_unlink_record_id];
            if (plan.first_link_record_id !=
                        candidates[first_index].record_id ||
                plan.second_link_record_id !=
                        candidates[second_index].record_id ||
                plan.first_copy_record_id !=
                        plan.first_link_record_id + 1 ||
                plan.first_unlink_record_id !=
                        plan.first_link_record_id + 2 ||
                plan.second_copy_record_id !=
                        plan.second_link_record_id + 1 ||
                plan.second_unlink_record_id !=
                        plan.second_link_record_id + 2 ||
                plan.expected_link_opcode !=
                        candidates[second_index].link_opcode ||
                plan.expected_copy_opcode !=
                        candidates[second_index].copy_opcode ||
                !flow_operand_matches_proof_register(
                        rxas_queue_operand(first_copy, 0),
                        plan.first_detached) ||
                !flow_operand_matches_proof_register(
                        rxas_queue_operand(second_copy, 0),
                        plan.second_destination) ||
                !flow_operand_matches_proof_register(
                        rxas_queue_operand(second_copy, 1),
                        plan.second_temporary) ||
                !flow_operand_matches_proof_register(
                        rxas_queue_operand(second_unlink, 0),
                        plan.second_temporary)) {
                stats->rejected_effect++;
                continue;
            }
            replacement_operands[0] = rxas_queue_operand(second_copy, 0);
            replacement_operands[1] = rxas_queue_operand(first_copy, 0);
            second_link->instrToken = rxas_tid(
                    graph->context, second_link->instrToken,
                    (char *)op_table[plan.expected_copy_opcode].mnemonic);
            rxas_set_queue_operands(
                    graph->context, second_link, replacement_operands, 2);
            second_copy->instrType = EMPTY;
            second_unlink->instrType = EMPTY;
            stats->duplicate_linked_read_proved++;
            stats->duplicate_linked_reads_reused++;
            if (graph->context->debug_mode)
                fprintf(stderr,
                        "PERF3 duplicate-linked-read-proof procedure=%s "
                        "first=%llu second=%llu link=%s copy=%s proved=1 "
                        "reason=proved storage=%llu owner=%llu count=%llu "
                        "ref=%llu components=0x%x\n",
                        graph->context->current_proc_name
                                ? graph->context->current_proc_name
                                : "(directives)",
                        (unsigned long long)plan.first_link_record_id,
                        (unsigned long long)plan.second_link_record_id,
                        graph->nodes[plan.second_link_record_id].op->mnemonic,
                        graph->nodes[plan.second_copy_record_id].op->mnemonic,
                        (unsigned long long)plan.linked_storage_id,
                        (unsigned long long)plan.owner_storage_id,
                        (unsigned long long)plan.attribute_count_value_id,
                        (unsigned long long)plan.reference_effect_id,
                        plan.read_components);
            flow_debug_accept(
                    graph, plan.second_link_record_id,
                    plan.expected_link_opcode == OP_LINK_REG_REG
                            ? "duplicate-link-read-reused-ssa"
                            : "duplicate-linkattr-read-reused-ssa",
                    2);
            free(candidates);
            return 1;
        }
    }
    free(candidates);
    return 0;
}

/* K04 is selected from the whole procedure, not from the retiring keyhole
 * queue.  The linear scan is only a demand filter; the immutable proof plan
 * owns opcode equivalence, CFG adjacency, storage, component cleanup, sparse
 * uses, call windows and exact matching TRACE-event deletion. */
static size_t flow_fuse_compare_branches(flow_graph *graph,
                                         flow_stats *stats,
                                         flow_proof_session *session) {
    const RxasFlowProofService *proof;
    const RxasFlowRecord *compare_record;
    const RxasFlowRecord *branch_record;
    RxasFlowCompareBranchPlan plan;
    RxasFlowTraceDeletion trace_deletion;
    size_t compare_index;
    size_t branch_index;
    size_t compare_instruction;
    size_t branch_instruction;
    size_t trace_deletion_index;
    size_t fused;
    instruction_queue *compare;
    instruction_queue *branch;
    Assembler_Token *result;
    Assembler_Token *branch_result;
    Assembler_Token *replacement_operands[3];

    fused = 0;
    for (compare_index = 0; compare_index < graph->item_count;
         compare_index++) {
        if (!graph->nodes[compare_index].reachable ||
            graph->items[compare_index].instrType != OP_CODE ||
            !graph->nodes[compare_index].op)
            continue;
        branch_index = compare_index + 1;
        while (branch_index < graph->item_count &&
               graph->items[branch_index].instrType != OP_CODE)
            branch_index++;
        if (branch_index >= graph->item_count ||
            !graph->nodes[branch_index].reachable ||
            !graph->nodes[branch_index].op ||
            !rxop_compare_branch_fusion(
                    graph->nodes[compare_index].op->opcode,
                    graph->nodes[branch_index].op->opcode, 0))
            continue;
        proof = flow_proof_session_require(session, graph);
        compare_record = session->procedure ? rxas_flow_procedure_record(
                session->procedure, session->epoch, compare_index) : 0;
        branch_record = session->procedure ? rxas_flow_procedure_record(
                session->procedure, session->epoch, branch_index) : 0;
        compare_instruction = compare_record
                ? compare_record->instruction_id : RXAS_FLOW_ID_NONE;
        branch_instruction = branch_record
                ? branch_record->instruction_id : RXAS_FLOW_ID_NONE;
        if (!proof || compare_instruction == RXAS_FLOW_ID_NONE ||
            branch_instruction == RXAS_FLOW_ID_NONE ||
            !rxas_flow_prove_compare_branch_fusion(
                    proof, session->epoch, compare_instruction,
                    branch_instruction, &plan)) {
            stats->rejected_effect++;
            continue;
        }
        stats->compare_branch_queries++;
        if (!plan.proved) {
            stats->compare_branch_rejected++;
            if (graph->context->debug_mode)
                fprintf(stderr,
                        "PERF3 compare-branch-proof procedure=%s "
                        "candidate=%llu:%s branch=%llu:%s proved=0 "
                        "reason=%s\n",
                        graph->context->current_proc_name
                                ? graph->context->current_proc_name
                                : "(directives)",
                        (unsigned long long)compare_index,
                        graph->nodes[compare_index].op->mnemonic,
                        (unsigned long long)branch_index,
                        graph->nodes[branch_index].op->mnemonic,
                        rxas_flow_proof_reason_name(plan.reason));
            if (flow_proof_reason_unavailable(plan.reason))
                stats->rejected_effect++;
            else if (plan.reason == RXAS_FLOW_PROOF_TRACE_OBSERVED)
                stats->rejected_trace++;
            else stats->rejected_live++;
            continue;
        }
        compare = &graph->items[compare_index];
        branch = &graph->items[branch_index];
        result = rxas_queue_operand(compare, 0);
        branch_result = rxas_queue_operand(branch, 1);
        if (plan.compare_record_id != compare_index ||
            plan.branch_record_id != branch_index ||
            plan.expected_compare_opcode !=
                    graph->nodes[compare_index].op->opcode ||
            plan.expected_branch_opcode !=
                    graph->nodes[branch_index].op->opcode ||
            plan.fused_opcode < 0 ||
            plan.fused_opcode >= OP_MAX_INSTRUCTIONS ||
            plan.left_source_operand >= compare->operandCount ||
            plan.right_source_operand >= compare->operandCount ||
            !flow_operand_matches_proof_register(
                    result, plan.result_register) ||
            !flow_operand_matches_proof_register(
                    branch_result, plan.result_register) ||
            branch->operandCount != 2 ||
            !rxas_queue_operand(branch, 0)) {
            stats->rejected_effect++;
            continue;
        }
        replacement_operands[0] = rxas_queue_operand(branch, 0);
        replacement_operands[1] = rxas_queue_operand(
                compare, plan.left_source_operand);
        replacement_operands[2] = rxas_queue_operand(
                compare, plan.right_source_operand);
        for (trace_deletion_index = 0;
             trace_deletion_index < plan.trace_deletion_count;
             trace_deletion_index++) {
            if (!rxas_flow_compare_branch_plan_trace_deletion(
                        proof, session->epoch, &plan,
                        trace_deletion_index, &trace_deletion) ||
                trace_deletion.record_id <= compare_index ||
                trace_deletion.record_id >= branch_index ||
                trace_deletion.value_id != plan.result_value_id ||
                trace_deletion.record_id >= graph->item_count ||
                !flow_trace_event_matches_deletion(
                        &graph->items[trace_deletion.record_id],
                        &trace_deletion)) {
                stats->rejected_effect++;
                break;
            }
        }
        if (trace_deletion_index != plan.trace_deletion_count) continue;
        branch->instrToken = rxas_tid(
                graph->context, branch->instrToken,
                (char *)op_table[plan.fused_opcode].mnemonic);
        rxas_set_queue_operands(
                graph->context, branch, replacement_operands, 3);
        compare->instrType = EMPTY;
        for (trace_deletion_index = 0;
             trace_deletion_index < plan.trace_deletion_count;
             trace_deletion_index++) {
            rxas_flow_compare_branch_plan_trace_deletion(
                    proof, session->epoch, &plan,
                    trace_deletion_index, &trace_deletion);
            graph->items[trace_deletion.record_id].instrType = EMPTY;
        }
        stats->compare_branch_proved++;
        stats->compare_branches_fused++;
        stats->compare_trace_events_removed += plan.trace_deletion_count;
        flow_debug_accept(graph, compare_index,
                          "compare-branch-fused-ssa",
                          plan.trace_deletion_count + 1);
        fused++;
    }
    return fused;
}

static int flow_is_scalar_constant_candidate(const flow_graph *graph,
                                             size_t index) {
    unsigned int component;
    const Assembler_Token *constant;
    if (graph->items[index].instrType != OP_CODE ||
        !graph->nodes[index].reachable || !graph->nodes[index].op ||
        graph->items[index].operandCount != 2)
        return 0;
    component = rxop_component_writes(
            graph->nodes[index].op->opcode, 0);
    constant = rxas_queue_operand(&graph->items[index], 1);
    return constant &&
           ((component == RXOP_COMPONENT_INTEGER &&
             constant->token_type == INT) ||
            (component == RXOP_COMPONENT_FLOAT &&
            constant->token_type == FLOAT));
}

static int flow_same_scalar_constant(const flow_graph *graph,
                                     size_t left, size_t right) {
    const Assembler_Token *left_constant;
    const Assembler_Token *right_constant;
    unsigned int left_component;
    unsigned int right_component;
    left_component = rxop_component_writes(
            graph->nodes[left].op->opcode, 0);
    right_component = rxop_component_writes(
            graph->nodes[right].op->opcode, 0);
    if (left_component != right_component) return 0;
    left_constant = rxas_queue_operand(&graph->items[left], 1);
    right_constant = rxas_queue_operand(&graph->items[right], 1);
    if (!left_constant || !right_constant ||
        left_constant->token_type != right_constant->token_type)
        return 0;
    if (left_constant->token_type == INT)
        return left_constant->token_value.integer ==
               right_constant->token_value.integer;
    if (left_constant->token_type == FLOAT)
        return memcmp(&left_constant->token_value.real,
                      &right_constant->token_value.real,
                      sizeof(left_constant->token_value.real)) == 0;
    return 0;
}

static int flow_same_raw_destination(const flow_graph *graph,
                                     size_t left, size_t right) {
    const Assembler_Token *left_target;
    const Assembler_Token *right_target;
    left_target = rxas_queue_operand(&graph->items[left], 0);
    right_target = rxas_queue_operand(&graph->items[right], 0);
    return left_target && right_target &&
           flow_register_type(left_target) == flow_register_type(right_target) &&
           left_target->token_value.integer == right_target->token_value.integer;
}

static int flow_opcode_may_remap_storage(const flow_node *node) {
    int opcode;
    if (!node || !node->op ||
        node->effects.state != RXOP_EFFECT_CLASSIFIED)
        return 0;
    if (node->effects.semantics &
        (RXOP_SEM_ALIAS_CREATE | RXOP_SEM_ALIAS_RELEASE))
        return 1;
    opcode = node->op->opcode;
    return flow_storage_is_pure_swap_opcode(opcode) ||
           opcode == OP_SETTPSWAP_REG_INT_REG ||
           opcode == OP_LOADSETTPSWAP_REG_INT_REG_INT_REG ||
           opcode == OP_SWAPSETTP_REG_REG_REG_INT ||
           opcode == OP_SWAPSETTPSWAP_REG_REG_REG_INT_REG ||
           opcode == OP_SETTPSWAPSETTPSWAP_REG_INT_REG_REG_REG ||
           opcode == OP_SWAPCALL_REG_FUNC_REG_REG_REG ||
           opcode == OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG;
}

static int flow_storage_remap_connects_candidates(
        const flow_graph *graph, size_t left, size_t right) {
    const Assembler_Token *left_target;
    const Assembler_Token *right_target;
    size_t index;
    left_target = rxas_queue_operand(&graph->items[left], 0);
    right_target = rxas_queue_operand(&graph->items[right], 0);
    if (!left_target || !right_target) return 0;
    for (index = left + 1; index < right; index++) {
        const instruction_queue *item;
        size_t operand;
        int touches_left;
        int touches_right;
        if (!flow_opcode_may_remap_storage(&graph->nodes[index])) continue;
        item = &graph->items[index];
        touches_left = 0;
        touches_right = 0;
        for (operand = 0; operand < item->operandCount; operand++) {
            const Assembler_Token *token;
            token = rxas_queue_operand(item, operand);
            if (!token || !flow_register_type(token)) continue;
            if (flow_register_type(token) == flow_register_type(left_target) &&
                token->token_value.integer == left_target->token_value.integer)
                touches_left = 1;
            if (flow_register_type(token) == flow_register_type(right_target) &&
                token->token_value.integer == right_target->token_value.integer)
                touches_right = 1;
        }
        if (touches_left && touches_right) return 1;
    }
    return 0;
}

/* This is a demand filter, never a proof.  It avoids constructing hidden
 * component phis for the first occurrence of a scalar literal while retaining
 * same-register, phi and explicitly connected link/swap opportunities. */
static int flow_scalar_constant_may_repeat(const flow_graph *graph,
                                           size_t candidate) {
    size_t earlier;
    if (!flow_is_scalar_constant_candidate(graph, candidate)) return 0;
    for (earlier = 0; earlier < candidate; earlier++) {
        if (!flow_is_scalar_constant_candidate(graph, earlier) ||
            !flow_same_scalar_constant(graph, earlier, candidate))
            continue;
        if (flow_same_raw_destination(graph, earlier, candidate) ||
            flow_storage_remap_connects_candidates(
                    graph, earlier, candidate))
            return 1;
    }
    return 0;
}

static size_t flow_remove_redundant_loads(flow_graph *graph, flow_stats *stats,
                                          flow_proof_session *session) {
    size_t index;
    size_t candidate_count;
    const RxasFlowProofService *proof;
    const RxasFlowRecord *record;
    RxasFlowProofResult proof_result;
    int query_available;
    size_t removed;
    removed = 0;
    candidate_count = 0;
    for (index = 0; index < graph->item_count; index++)
        if (flow_scalar_constant_may_repeat(graph, index))
            candidate_count++;
    if (!candidate_count) return 0;
    proof = flow_proof_session_require(session, graph);
    if (!proof) {
        stats->constant_proof_unavailable++;
        return 0;
    }
    for (index = 0; index < graph->item_count; index++) {
        if (!flow_scalar_constant_may_repeat(graph, index)) continue;
        record = rxas_flow_procedure_record(
                session->procedure, session->epoch, index);
        if (!record || record->instruction_id == RXAS_FLOW_ID_NONE) continue;
        memset(&proof_result, 0, sizeof(proof_result));
        query_available = rxas_flow_prove_redundant_constant_write(
                proof, session->epoch, record->instruction_id, &proof_result) &&
                !flow_proof_reason_unavailable(proof_result.reason);
        stats->constant_proof_queries++;
        if (graph->context->debug_mode)
            fprintf(stderr,
                    "PERF3 constant-proof-candidate opcode=%s record=%llu "
                    "instruction=%llu proved=%d reason=%s storage=%llu "
                    "before=%llu after=%llu\n",
                    graph->nodes[index].op->mnemonic,
                    (unsigned long long)index,
                    (unsigned long long)record->instruction_id,
                    proof_result.proved,
                    rxas_flow_proof_reason_name(proof_result.reason),
                    (unsigned long long)proof_result.storage_id,
                    (unsigned long long)proof_result.source_value_id,
                    (unsigned long long)proof_result.result_value_id);
        if (!query_available) {
            stats->constant_proof_unavailable++;
            return removed;
        }
        if (!proof_result.proved) {
            stats->constant_proof_rejected++;
            continue;
        }
        graph->items[index].instrType = EMPTY;
        flow_debug_accept(graph, index,
                          "redundant-component-ssa-constant", 0);
        removed++;
        stats->redundant_loads_removed++;
        stats->constant_proof_proved++;
    }
    return removed;
}

/* This is a demand filter, never a proof.  One linear, path-insensitive pass
 * marks registers that may denote storage already cleared by an earlier NULL
 * and propagates that possibility across explicit mapping instructions.  It
 * may overselect at joins, but only the component-SSA query can authorize a
 * deletion. */
static size_t flow_mark_null_repeat_candidates(
        const flow_graph *graph, unsigned char *candidates) {
    unsigned char *may_be_absent;
    size_t index;
    size_t count;
    if (!graph || !candidates || !graph->register_count) return 0;
    may_be_absent = calloc(graph->register_count, 1);
    if (!may_be_absent)
        RX_PANIC_OOM("calloc RXAS NULL repeat demand filter",
                     graph->register_count, 0);
    count = 0;
    for (index = 0; index < graph->item_count; index++) {
        const instruction_queue *item;
        size_t operand;
        int touches_absent;
        if (graph->items[index].instrType != OP_CODE ||
            !graph->nodes[index].reachable || !graph->nodes[index].op)
            continue;
        item = &graph->items[index];
        if (flow_opcode_may_remap_storage(&graph->nodes[index])) {
            touches_absent = 0;
            for (operand = 0; operand < item->operandCount; operand++) {
                const Assembler_Token *token;
                int register_index;
                token = rxas_queue_operand(item, operand);
                if (!token || !flow_register_type(token)) continue;
                register_index = flow_register_index(
                        graph, flow_register_type(token),
                        (size_t)token->token_value.integer);
                if (register_index >= 0 && may_be_absent[register_index]) {
                    touches_absent = 1;
                    break;
                }
            }
            if (touches_absent) {
                for (operand = 0; operand < item->operandCount; operand++) {
                    const Assembler_Token *token;
                    int register_index;
                    token = rxas_queue_operand(item, operand);
                    if (!token || !flow_register_type(token)) continue;
                    register_index = flow_register_index(
                            graph, flow_register_type(token),
                            (size_t)token->token_value.integer);
                    if (register_index >= 0)
                        may_be_absent[register_index] = 1;
                }
            }
        }
        if (graph->nodes[index].op->opcode == OP_NULL_REG) {
            const Assembler_Token *target;
            int register_index;
            target = rxas_queue_operand(item, 0);
            register_index = target ? flow_register_index(
                    graph, flow_register_type(target),
                    (size_t)target->token_value.integer) : -1;
            if (register_index >= 0) {
                if (may_be_absent[register_index]) {
                    candidates[index] = 1;
                    count++;
                }
                may_be_absent[register_index] = 1;
            }
        }
    }
    free(may_be_absent);
    return count;
}

static size_t flow_remove_redundant_initializations(
        flow_graph *graph, flow_stats *stats, flow_proof_session *session) {
    size_t index;
    size_t candidate_count;
    const RxasFlowProofService *proof;
    const RxasFlowRecord *record;
    RxasFlowProofResult proof_result;
    int query_available;
    unsigned char *candidates;
    size_t removed;
    removed = 0;
    candidates = calloc(graph->item_count, 1);
    if (!candidates)
        RX_PANIC_OOM("calloc RXAS NULL proof candidates",
                     graph->item_count, 0);
    candidate_count = flow_mark_null_repeat_candidates(graph, candidates);
    if (!candidate_count) {
        free(candidates);
        return 0;
    }
    proof = flow_proof_session_require(session, graph);
    if (!proof) {
        stats->absent_proof_unavailable++;
        free(candidates);
        return 0;
    }
    for (index = 0; index < graph->item_count; index++) {
        if (!candidates[index]) continue;
        record = rxas_flow_procedure_record(
                session->procedure, session->epoch, index);
        if (!record || record->instruction_id == RXAS_FLOW_ID_NONE) continue;
        memset(&proof_result, 0, sizeof(proof_result));
        query_available = rxas_flow_prove_redundant_absent_write(
                proof, session->epoch, record->instruction_id, &proof_result) &&
                !flow_proof_reason_unavailable(proof_result.reason);
        stats->absent_proof_queries++;
        if (graph->context->debug_mode)
            fprintf(stderr,
                    "PERF3 absent-proof-candidate opcode=%s record=%llu "
                    "instruction=%llu proved=%d reason=%s storage=%llu "
                    "before=%llu\n",
                    graph->nodes[index].op->mnemonic,
                    (unsigned long long)index,
                    (unsigned long long)record->instruction_id,
                    proof_result.proved,
                    rxas_flow_proof_reason_name(proof_result.reason),
                    (unsigned long long)proof_result.storage_id,
                    (unsigned long long)proof_result.source_value_id);
        if (!query_available) {
            stats->absent_proof_unavailable++;
            free(candidates);
            return removed;
        }
        if (!proof_result.proved) {
            stats->absent_proof_rejected++;
            continue;
        }
        graph->items[index].instrType = EMPTY;
        flow_debug_accept(graph, index,
                          "redundant-component-ssa-absent", 0);
        removed++;
        stats->redundant_initializations_removed++;
        stats->absent_proof_proved++;
    }
    free(candidates);
    return removed;
}

static RxasFlowProcedure *flow_build_proof_procedure(
        const flow_graph *graph, unsigned long epoch) {
    const OpInfo **resolved_ops;
    RxasFlowProcedure *procedure;
    size_t index;
    if (!graph || !graph->item_count) return 0;
    resolved_ops = malloc(graph->item_count * sizeof(*resolved_ops));
    if (!resolved_ops) return 0;
    for (index = 0; index < graph->item_count; index++)
        resolved_ops[index] = graph->nodes[index].op;
    procedure = rxas_flow_procedure_build_resolved(
            graph->context, graph->items, graph->item_count,
            epoch, resolved_ops);
    free(resolved_ops);
    return procedure;
}

/* One immutable proof graph serves every migrated consumer in a fixed-point
 * epoch.  Rebuilding it per query family needlessly multiplies peak allocator
 * pages on large inlined procedures and defeats the analysis-manager design. */
static const RxasFlowProofService *flow_proof_session_require(
        flow_proof_session *session, const flow_graph *graph) {
    if (!session || !graph) return 0;
    if (session->attempted) return session->proof;
    session->attempted = 1;
    session->epoch = 1ul;
    session->procedure = flow_build_proof_procedure(graph, session->epoch);
    session->proof = session->procedure ? rxas_flow_require_proof_service(
            session->procedure, session->epoch, 0) : 0;
    return session->proof;
}

static void flow_proof_session_destroy(flow_proof_session *session) {
    if (!session) return;
    if (session->procedure)
        rxas_flow_procedure_destroy(session->procedure);
    memset(session, 0, sizeof(*session));
}

static int flow_proof_reason_unavailable(RxasFlowProofReason reason) {
    return reason == RXAS_FLOW_PROOF_STALE_EPOCH ||
           reason == RXAS_FLOW_PROOF_INVALID_GRAPH ||
           reason == RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE ||
           reason == RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
}

static int flow_same_raw_copy_operands(const flow_graph *graph,
                                       size_t index) {
    const Assembler_Token *destination;
    const Assembler_Token *source;
    if (!graph || index >= graph->item_count ||
        graph->items[index].operandCount != 2)
        return 0;
    destination = rxas_queue_operand(&graph->items[index], 0);
    source = rxas_queue_operand(&graph->items[index], 1);
    return destination && source && flow_register_type(destination) &&
           flow_register_type(destination) == flow_register_type(source) &&
           destination->token_value.integer == source->token_value.integer;
}

/* This linear pass is only a demand filter.  Exact raw self-copies preserve
 * the old optimization floor; copies involving a register touched by a link,
 * swap, or other mapping operation are sent to component SSA so actual
 * storage identity (including agreeing phis) can be proved. */
static size_t flow_mark_self_copy_candidates(
        const flow_graph *graph, unsigned char *candidates) {
    unsigned char *mapping_touched;
    size_t index;
    size_t count;
    if (!graph || !candidates || !graph->register_count) return 0;
    mapping_touched = calloc(graph->register_count, 1);
    if (!mapping_touched)
        RX_PANIC_OOM("calloc RXAS self-copy demand filter",
                     graph->register_count, 0);
    count = 0;
    for (index = 0; index < graph->item_count; index++) {
        const instruction_queue *item;
        size_t operand_index;
        int destination_register;
        int source_register;
        if (graph->items[index].instrType != OP_CODE ||
            !graph->nodes[index].reachable || !graph->nodes[index].op)
            continue;
        item = &graph->items[index];
        if (flow_opcode_may_remap_storage(&graph->nodes[index])) {
            for (operand_index = 0; operand_index < item->operandCount;
                 operand_index++) {
                const Assembler_Token *operand;
                int register_index;
                operand = rxas_queue_operand(item, operand_index);
                register_index = operand && flow_register_type(operand)
                        ? flow_register_index(
                                graph, flow_register_type(operand),
                                (size_t)operand->token_value.integer)
                        : -1;
                if (register_index >= 0)
                    mapping_touched[register_index] = 1;
            }
        }
        if (item->operandCount != 2 ||
            !rxop_same_storage_copy_is_noop(graph->nodes[index].op->opcode))
            continue;
        if (flow_same_raw_copy_operands(graph, index)) {
            candidates[index] = 1;
            count++;
            continue;
        }
        {
            const Assembler_Token *destination;
            const Assembler_Token *source;
            destination = rxas_queue_operand(item, 0);
            source = rxas_queue_operand(item, 1);
            destination_register = destination &&
                    flow_register_type(destination)
                    ? flow_register_index(
                            graph, flow_register_type(destination),
                            (size_t)destination->token_value.integer)
                    : -1;
            source_register = source && flow_register_type(source)
                    ? flow_register_index(
                            graph, flow_register_type(source),
                            (size_t)source->token_value.integer)
                    : -1;
        }
        if ((destination_register >= 0 &&
             mapping_touched[destination_register]) ||
            (source_register >= 0 && mapping_touched[source_register])) {
            candidates[index] = 1;
            count++;
        }
    }
    free(mapping_touched);
    return count;
}

static size_t flow_remove_redundant_self_copies(
        flow_graph *graph, flow_stats *stats, flow_proof_session *session) {
    unsigned char *candidates;
    size_t candidate_count;
    size_t index;
    size_t removed;
    const RxasFlowProofService *proof;
    const RxasFlowRecord *record;
    RxasFlowProofResult proof_result;
    int query_available;
    candidates = calloc(graph->item_count, 1);
    if (!candidates)
        RX_PANIC_OOM("calloc RXAS self-copy proof candidates",
                     graph->item_count, 0);
    candidate_count = flow_mark_self_copy_candidates(graph, candidates);
    if (!candidate_count) {
        free(candidates);
        return 0;
    }
    proof = flow_proof_session_require(session, graph);
    if (!proof) {
        stats->self_copy_proof_unavailable++;
        free(candidates);
        return 0;
    }
    removed = 0;
    for (index = 0; index < graph->item_count; index++) {
        if (!candidates[index]) continue;
        record = rxas_flow_procedure_record(
                session->procedure, session->epoch, index);
        if (!record || record->instruction_id == RXAS_FLOW_ID_NONE) continue;
        memset(&proof_result, 0, sizeof(proof_result));
        query_available = rxas_flow_prove_redundant_self_copy(
                proof, session->epoch, record->instruction_id,
                &proof_result) &&
                !flow_proof_reason_unavailable(proof_result.reason);
        stats->self_copy_proof_queries++;
        if (graph->context->debug_mode)
            fprintf(stderr,
                    "PERF3 self-copy-proof-candidate opcode=%s record=%llu "
                    "instruction=%llu proved=%d reason=%s storage=%llu\n",
                    graph->nodes[index].op->mnemonic,
                    (unsigned long long)index,
                    (unsigned long long)record->instruction_id,
                    proof_result.proved,
                    rxas_flow_proof_reason_name(proof_result.reason),
                    (unsigned long long)proof_result.storage_id);
        if (!query_available) {
            stats->self_copy_proof_unavailable++;
            free(candidates);
            return removed;
        }
        if (!proof_result.proved) {
            stats->self_copy_proof_rejected++;
            continue;
        }
        graph->items[index].instrType = EMPTY;
        flow_debug_accept(graph, index,
                          "redundant-component-ssa-self-copy", 0);
        removed++;
        stats->self_copy_proof_proved++;
        if (graph->nodes[index].op->opcode == OP_COPY_REG_REG)
            stats->full_copies_removed++;
        else
            stats->typed_copies_removed++;
    }
    free(candidates);
    return removed;
}

typedef struct flow_derivation_candidate {
    size_t record_id;
    size_t instruction_id;
    size_t storage_id;
    int opcode;
} flow_derivation_candidate;

static int flow_is_one_register_derivation(const flow_graph *graph,
                                           size_t index) {
    unsigned int component;
    if (graph->items[index].instrType != OP_CODE ||
        !graph->nodes[index].reachable || !graph->nodes[index].op ||
        graph->items[index].operandCount != 1 ||
        rxop_value_derivation(graph->nodes[index].op->opcode) ==
                RXOP_DERIVATION_NONE)
        return 0;
    component = rxop_component_writes(graph->nodes[index].op->opcode, 0);
    return component && !(component & (component - 1));
}

static size_t flow_remove_redundant_derivations(
        flow_graph *graph, flow_stats *stats, flow_proof_session *session) {
    size_t index;
    size_t candidate_index;
    size_t generator_index;
    const RxasFlowProofService *proof;
    const RxasFlowRecord *record;
    flow_derivation_candidate *candidates;
    size_t candidate_count;
    RxasFlowProofResult proof_result;
    RxasFlowRepetitionKey key;
    size_t removed;
    int query_available;

    removed = 0;
    candidate_count = 0;
    for (index = 0; index < graph->item_count; index++)
        if (flow_is_one_register_derivation(graph, index))
            candidate_count++;
    if (candidate_count < 2) return 0;
    candidates = calloc(graph->item_count, sizeof(*candidates));
    if (!candidates)
        RX_PANIC_OOM("calloc RXAS derivation proof candidates",
                     graph->item_count * sizeof(*candidates), 0);
    proof = flow_proof_session_require(session, graph);
    if (!proof) {
        stats->derivation_proof_unavailable++;
        goto cleanup;
    }
    candidate_count = 0;
    for (index = 0; index < graph->item_count; index++) {
        if (!flow_is_one_register_derivation(graph, index))
            continue;
        record = rxas_flow_procedure_record(
                session->procedure, session->epoch, index);
        if (!record || record->instruction_id == RXAS_FLOW_ID_NONE ||
            !rxas_flow_repetition_key(
                    proof, session->epoch, record->instruction_id, &key) ||
            key.opcode != graph->nodes[index].op->opcode || !key.storage_id)
            continue;
        candidates[candidate_count].record_id = index;
        candidates[candidate_count].instruction_id = record->instruction_id;
        candidates[candidate_count].storage_id = key.storage_id;
        candidates[candidate_count].opcode = key.opcode;
        if (graph->context->debug_mode)
            fprintf(stderr,
                    "PERF3 derivation-proof-candidate opcode=%s record=%llu "
                    "instruction=%llu storage=%llu\n",
                    op_table[key.opcode].mnemonic,
                    (unsigned long long)index,
                    (unsigned long long)record->instruction_id,
                    (unsigned long long)key.storage_id);
        candidate_count++;
    }
    for (candidate_index = 1; candidate_index < candidate_count;
         candidate_index++) {
        flow_derivation_candidate *candidate;
        candidate = &candidates[candidate_index];
        if (graph->items[candidate->record_id].instrType != OP_CODE) continue;
        for (generator_index = candidate_index; generator_index; ) {
            flow_derivation_candidate *generator;
            generator_index--;
            generator = &candidates[generator_index];
            if (generator->opcode != candidate->opcode ||
                generator->storage_id != candidate->storage_id ||
                graph->items[generator->record_id].instrType != OP_CODE)
                continue;
            memset(&proof_result, 0, sizeof(proof_result));
            proof_result.reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
            query_available = rxas_flow_prove_repetition(
                    proof, session->epoch, generator->instruction_id,
                    candidate->instruction_id, &proof_result) &&
                    !flow_proof_reason_unavailable(proof_result.reason);
            stats->derivation_proof_queries++;
            if (!query_available) {
                stats->derivation_proof_unavailable++;
                goto cleanup;
            }
            if (!proof_result.proved) {
                stats->derivation_proof_rejected++;
                continue;
            }
            graph->items[candidate->record_id].instrType = EMPTY;
            flow_debug_accept(
                    graph, candidate->record_id,
                    "redundant-component-ssa-conversion", 0);
            removed++;
            stats->redundant_conversions_removed++;
            stats->derivation_proof_proved++;
            break;
        }
    }
cleanup:
    if (graph->context->debug_mode && proof)
        rxas_flow_proof_dump(proof, session->epoch, stderr);
    free(candidates);
    return removed;
}

static size_t flow_remove_redundant_conversions(flow_graph *graph,
                                                flow_stats *stats,
                                                flow_proof_session *session) {
    return flow_remove_redundant_derivations(graph, stats, session);
}

static void flow_debug_summary(const flow_graph *graph, const flow_stats *stats,
                               size_t before_instructions, size_t after_instructions) {
    if (!graph->context->debug_mode) return;
    fprintf(stderr,
            "NR27 flow procedure=%s blocks=%llu registers=%llu instructions=%llu->%llu "
            "unreachable=%llu dead=%llu typed-copy=%llu compare-prep=%llu "
            "full-copy=%llu redundant-load=%llu redundant-init=%llu "
            "redundant-conversion=%llu producer-forward=%llu "
            "compare-branch=%llu trace-delete=%llu redirects=%llu "
            "constant-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "absent-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "self-copy-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "derivation-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "producer-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "duplicate-linked-read-proof=%llu/%llu rejected=%llu reused=%llu "
            "compare-branch-proof=%llu/%llu rejected=%llu "
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
            (unsigned long long)stats->compare_branches_fused,
            (unsigned long long)stats->compare_trace_events_removed,
            (unsigned long long)stats->operands_redirected,
            (unsigned long long)stats->constant_proof_proved,
            (unsigned long long)stats->constant_proof_queries,
            (unsigned long long)stats->constant_proof_rejected,
            (unsigned long long)stats->constant_proof_unavailable,
            (unsigned long long)stats->absent_proof_proved,
            (unsigned long long)stats->absent_proof_queries,
            (unsigned long long)stats->absent_proof_rejected,
            (unsigned long long)stats->absent_proof_unavailable,
            (unsigned long long)stats->self_copy_proof_proved,
            (unsigned long long)stats->self_copy_proof_queries,
            (unsigned long long)stats->self_copy_proof_rejected,
            (unsigned long long)stats->self_copy_proof_unavailable,
            (unsigned long long)stats->derivation_proof_proved,
            (unsigned long long)stats->derivation_proof_queries,
            (unsigned long long)stats->derivation_proof_rejected,
            (unsigned long long)stats->derivation_proof_unavailable,
            (unsigned long long)stats->producer_proof_proved,
            (unsigned long long)stats->producer_proof_queries,
            (unsigned long long)stats->producer_proof_rejected,
            (unsigned long long)stats->producer_proof_unavailable,
            (unsigned long long)stats->duplicate_linked_read_proved,
            (unsigned long long)stats->duplicate_linked_read_queries,
            (unsigned long long)stats->duplicate_linked_read_rejected,
            (unsigned long long)stats->duplicate_linked_reads_reused,
            (unsigned long long)stats->compare_branch_proved,
            (unsigned long long)stats->compare_branch_queries,
            (unsigned long long)stats->compare_branch_rejected,
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
    RxasFlowProcedure *procedure;
    const RxasFlowStructuralAnalysis *structural;
    const RxasFlowSignalAnalysis *signal_analysis;
    const RxasFlowSsaAnalysis *ssa_analysis;
    const RxasFlowUseAnalysis *use_analysis;
    const RxasFlowProofService *proof_service;
    const OpInfo **resolved_ops;
    size_t before_instructions;
    size_t after_instructions;
    size_t changed;
    size_t iterations;
    flow_proof_session proof_session;

    if (!context || !items || !item_count || !context->current_proc_name) return;
    memset(&stats, 0, sizeof(stats));
    before_instructions = flow_instruction_count(items, item_count);
    iterations = 0;
    do {
        memset(&proof_session, 0, sizeof(proof_session));
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
                changed += flow_remove_redundant_self_copies(
                        &graph, &stats, &proof_session);
                if (!changed) changed += flow_reuse_duplicate_linked_read(
                        &graph, &stats, &proof_session);
                if (!changed) changed += flow_propagate_copies(
                        &graph, &stats, &proof_session);
                if (!changed) changed += flow_forward_producer_destination(
                        &graph, &stats, &proof_session);
                if (!changed) changed += flow_fuse_compare_branches(
                        &graph, &stats, &proof_session);
                if (!changed) changed += flow_remove_redundant_loads(
                        &graph, &stats, &proof_session);
                if (!changed) changed += flow_remove_redundant_initializations(
                        &graph, &stats, &proof_session);
                if (!changed) changed += flow_remove_redundant_conversions(
                        &graph, &stats, &proof_session);
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
        flow_proof_session_destroy(&proof_session);
        flow_free_graph(&graph);
        iterations++;
    } while (changed && iterations <= before_instructions + 1);

    flow_build_graph(&graph, context, items, item_count);
    after_instructions = flow_instruction_count(items, item_count);
    /* The storage service is graph-owned and demand driven.  At present its
     * only consumer is the debug identity report; ordinary assembly used to
     * build the complete point environment here and immediately free it.
     * A future rewrite consumer must attach the same bounded service before
     * querying it rather than making ordinary diagnostic work eager again. */
    if (context->debug_mode) {
        flow_storage_attach(&graph);
        flow_debug_storage_identity(&graph);
    }
    flow_debug_summary(&graph, &stats, before_instructions, after_instructions);
    /* Stage 2 builds the immutable graph beside the legacy rewrite graph.  It
     * is intentionally not a rewrite consumer yet; construction therefore
     * cannot alter the queued records or emitted image. */
    resolved_ops = malloc(item_count * sizeof(*resolved_ops));
    if (resolved_ops) {
        size_t record_index;
        for (record_index = 0; record_index < item_count; record_index++)
            resolved_ops[record_index] = graph.nodes[record_index].op;
    }
    /* Only OpInfo pointers into the immutable opcode table survive this
     * point. Do not overlap the legacy graph's dense liveness/storage memory
     * with the new descriptor graph merely for orchestration convenience. */
    flow_free_graph(&graph);
    if (resolved_ops) {
        procedure = rxas_flow_procedure_build_resolved(
                context, items, item_count,
                (unsigned long)iterations + 1ul, resolved_ops);
        free(resolved_ops);
    }
    else procedure = 0;
    if (procedure) {
        if (context->debug_mode) {
            /* Structural analyses are demand-driven.  Until an optimizer
             * consumer requests them, ordinary assembly must not retain or
             * solve facts that only diagnostics use. */
            structural = rxas_flow_require_structural_analysis(
                    procedure, rxas_flow_procedure_epoch(procedure), 0);
            rxas_flow_procedure_dump(
                    procedure, rxas_flow_procedure_epoch(procedure), stderr);
            if (structural)
                rxas_flow_structural_dump(
                        structural, rxas_flow_procedure_epoch(procedure),
                        stderr);
            else {
                const RxasFlowStructuralMetrics *failed;
                failed = rxas_flow_last_structural_metrics(
                        procedure, rxas_flow_procedure_epoch(procedure));
                fprintf(stderr,
                        "PERF3 flow-analysis procedure=%s disabled=%s "
                        "budget=%llu work=%llu\n",
                        context->current_proc_name ? context->current_proc_name
                                                   : "(directives)",
                        failed && failed->status ==
                                RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                                ? "budget-exhausted"
                                : failed && failed->status ==
                                        RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY
                                        ? "out-of-memory"
                                        : "invalid-graph",
                        (unsigned long long)(failed ? failed->budget_limit : 0),
                        (unsigned long long)(failed ? failed->work : 0));
            }
            signal_analysis = rxas_flow_require_signal_analysis(
                    procedure, rxas_flow_procedure_epoch(procedure), 0);
            if (signal_analysis)
                rxas_flow_signal_dump(
                        signal_analysis, rxas_flow_procedure_epoch(procedure),
                        stderr);
            else {
                const RxasFlowSignalMetrics *failed_signal;
                failed_signal = rxas_flow_last_signal_metrics(
                        procedure, rxas_flow_procedure_epoch(procedure));
                fprintf(stderr,
                        "PERF3 flow-signal-analysis procedure=%s disabled=%s "
                        "budget=%llu work=%llu\n",
                        context->current_proc_name ? context->current_proc_name
                                                   : "(directives)",
                        failed_signal && failed_signal->status ==
                                RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                                ? "budget-exhausted"
                                : failed_signal && failed_signal->status ==
                                        RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY
                                        ? "out-of-memory"
                                        : "invalid-graph",
                        (unsigned long long)(failed_signal
                                ? failed_signal->budget_limit : 0),
                        (unsigned long long)(failed_signal
                                ? failed_signal->work : 0));
            }
            ssa_analysis = rxas_flow_require_ssa_analysis(
                    procedure, rxas_flow_procedure_epoch(procedure), 0);
            if (ssa_analysis)
                rxas_flow_ssa_dump(
                        ssa_analysis, rxas_flow_procedure_epoch(procedure),
                        stderr);
            else {
                const RxasFlowSsaMetrics *failed_ssa;
                failed_ssa = rxas_flow_last_ssa_metrics(
                        procedure, rxas_flow_procedure_epoch(procedure));
                fprintf(stderr,
                        "PERF3 flow-ssa-analysis procedure=%s disabled=%s "
                        "budget=%llu work=%llu\n",
                        context->current_proc_name ? context->current_proc_name
                                                   : "(directives)",
                        failed_ssa && failed_ssa->status ==
                                RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                                ? "budget-exhausted"
                                : failed_ssa && failed_ssa->status ==
                                        RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY
                                        ? "out-of-memory"
                                        : "invalid-graph",
                        (unsigned long long)(failed_ssa
                                ? failed_ssa->budget_limit : 0),
                        (unsigned long long)(failed_ssa ? failed_ssa->work : 0));
            }
            proof_service = rxas_flow_require_proof_service(
                    procedure, rxas_flow_procedure_epoch(procedure), 0);
            if (proof_service)
                rxas_flow_proof_dump(
                        proof_service, rxas_flow_procedure_epoch(procedure),
                        stderr);
            else {
                const RxasFlowProofMetrics *failed_proof;
                failed_proof = rxas_flow_last_proof_metrics(
                        procedure, rxas_flow_procedure_epoch(procedure));
                fprintf(stderr,
                        "PERF3 flow-proof procedure=%s disabled=%s "
                        "budget=%llu work=%llu\n",
                        context->current_proc_name ? context->current_proc_name
                                                   : "(directives)",
                        failed_proof && failed_proof->status ==
                                RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                                ? "budget-exhausted"
                                : failed_proof && failed_proof->status ==
                                        RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY
                                        ? "out-of-memory"
                                        : "invalid-graph",
                        (unsigned long long)(failed_proof
                                ? failed_proof->budget_limit : 0),
                        (unsigned long long)(failed_proof
                                ? failed_proof->work : 0));
            }
            use_analysis = rxas_flow_require_use_analysis(
                    procedure, rxas_flow_procedure_epoch(procedure), 0);
            if (use_analysis)
                rxas_flow_use_dump(
                        use_analysis, rxas_flow_procedure_epoch(procedure),
                        stderr);
            else {
                const RxasFlowUseMetrics *failed_use;
                failed_use = rxas_flow_last_use_metrics(
                        procedure, rxas_flow_procedure_epoch(procedure));
                fprintf(stderr,
                        "PERF3 flow-use procedure=%s disabled=%s "
                        "budget=%llu work=%llu\n",
                        context->current_proc_name ? context->current_proc_name
                                                   : "(directives)",
                        failed_use && failed_use->status ==
                                RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                                ? "budget-exhausted"
                                : failed_use && failed_use->status ==
                                        RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY
                                        ? "out-of-memory"
                                        : "invalid-graph",
                        (unsigned long long)(failed_use
                                ? failed_use->budget_limit : 0),
                        (unsigned long long)(failed_use
                                ? failed_use->work : 0));
            }
        }
        rxas_flow_procedure_destroy(procedure);
    }
    else if (context->debug_mode) {
        fprintf(stderr,
                "PERF3 flow-graph procedure=%s disabled=construction-failed\n",
                context->current_proc_name ? context->current_proc_name
                                           : "(directives)");
    }
}
