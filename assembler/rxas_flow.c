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
#include "rxas_flow_signal.h"
#include "rxas_flow_ssa.h"
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
    FLOW_EDGE_SIGNAL_SKIP,
    FLOW_EDGE_SIGNAL_RETRY
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
    size_t producer_destinations_forwarded;
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
    size_t signal_retry_edges;
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
             * potentially throwing instruction has logical skip and retry
             * continuations even when this procedure contains no SIGCALLA.
             * Skip resumes at the already-advanced sequential address; retry
             * re-enters the signal point. Legacy rewrite consumers explicitly
             * continue to use normal edges only in this infrastructure slice. */
            if (index + 1 < graph->item_count) {
                flow_add_successor(graph, node, index + 1,
                                   FLOW_EDGE_SIGNAL_SKIP);
            }
            flow_add_successor(graph, node, index, FLOW_EDGE_SIGNAL_RETRY);
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
                     successor_index++) {
                    if (node->successors[successor_index].kind !=
                        FLOW_EDGE_NORMAL)
                        continue;
                    next_out |= graph->nodes[
                            node->successors[successor_index].target].live_in[word];
                }
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
            /* The normal edge completed the link. Signal skip/retry carry a
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

/* Mapping state at the point where a signalling instruction raised
 * a signal. This state feeds both SIGCALLA skip (the sequential successor) and
 * retry (the instruction itself). Simple attribute/reference links validate
 * before installing their destination, whereas fused forms may already have
 * resized attribute storage or installed an earlier link. */
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
             * iteration, so its dynamic identity cannot be reused. A signal
             * retry instead resumes the same interrupted execution context and
             * must retain the exact signal-point mapping. */
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
            else if (edge.kind == FLOW_EDGE_SIGNAL_RETRY)
                stats->signal_retry_edges++;
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
            "edges=%llu/%llu/%llu handler-entries=%llu\n",
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
            (unsigned long long)stats.signal_retry_edges,
            (unsigned long long)stats.signal_handler_entries);
}

static int flow_is_async_handler_target(const flow_graph *graph, size_t node_index) {
    size_t index;
    for (index = 0; index < graph->async_handler_target_count; index++)
        if (graph->async_handler_targets[index] == node_index) return 1;
    return 0;
}

static size_t flow_storage_identity_at(const flow_graph *graph,
                                       size_t node_index,
                                       int register_index) {
    if (!graph || !graph->storage || register_index < 0 ||
        node_index >= graph->item_count ||
        (size_t)register_index >= graph->register_count ||
        !graph->storage->has_in[node_index])
        return 0;
    return graph->storage->in[node_index * graph->register_count +
                              (size_t)register_index];
}

static int flow_storage_fact_is_unescaped_at(const flow_graph *graph,
                                             size_t node_index,
                                             size_t storage_id) {
    size_t register_index;
    if (!storage_id || storage_id > graph->register_count) return 0;
    if (graph->tainted_registers[storage_id - 1]) return 0;
    for (register_index = 0; register_index < graph->register_count;
         register_index++) {
        if (flow_storage_identity_at(graph, node_index,
                                     (int)register_index) == storage_id &&
            graph->tainted_registers[register_index])
            return 0;
    }
    return 1;
}

static int flow_node_kills_storage_derivation(const flow_graph *graph,
                                              size_t node_index,
                                              size_t storage_id,
                                              unsigned int components,
                                              unsigned int contexts) {
    const instruction_queue *item;
    const flow_node *node;
    size_t operand_index;
    int register_index;
    size_t operand_storage;
    unsigned int written_components;
    unsigned int global_barriers;

    item = &graph->items[node_index];
    node = &graph->nodes[node_index];
    if (item->instrType != OP_CODE) return 0;
    if (!node->op || node->effects.state != RXOP_EFFECT_CLASSIFIED) return 1;
    if (rxop_context_writes(node->op->opcode) & contexts) return 1;
    if (node->effects.semantics & (RXOP_SEM_CALL | RXOP_SEM_DYNAMIC_CALL))
        return 1;

    /* An indirect/opaque operation cannot reach a base local that never
     * participates in alias/reference machinery.  Once any register mapping
     * for this storage is tainted, retain the conservative global barrier. */
    global_barriers = RXOP_SEM_REFERENCE_CREATE | RXOP_SEM_REFERENCE_READ |
                      RXOP_SEM_REFERENCE_WRITE | RXOP_SEM_REFERENCE_RELEASE |
                      RXOP_SEM_INDIRECT_WRITE | RXOP_SEM_OPAQUE;
    if ((node->effects.semantics & global_barriers) &&
        !flow_storage_fact_is_unescaped_at(graph, node_index, storage_id))
        return 1;

    /* These instructions change register-to-storage mappings, not the value
     * components inside the mapped storage.  Their normal transfer is already
     * represented by the graph-owned storage service. */
    if (flow_storage_is_pure_swap_opcode(node->op->opcode) ||
        (node->effects.semantics &
         (RXOP_SEM_ALIAS_CREATE | RXOP_SEM_ALIAS_RELEASE)))
        return 0;

    for (operand_index = 0; operand_index < item->operandCount;
         operand_index++) {
        if (!rxop_effect_writes_operand(&node->effects, operand_index))
            continue;
        register_index = flow_storage_operand_register(
                graph, (instruction_queue *)item, operand_index);
        if (register_index < 0) continue;
        operand_storage = flow_storage_identity_at(graph, node_index,
                                                   register_index);
        if (!operand_storage) return 1;
        if (operand_storage != storage_id) continue;
        written_components = rxop_component_writes(node->op->opcode,
                                                   operand_index);
        if (!written_components || (written_components & components)) return 1;
    }
    if (node->effects.implicit != RXOP_IMPLICIT_NONE) return 1;
    return 0;
}

static void flow_compute_available_storage_derivation(
        const flow_graph *graph, size_t generator, size_t storage_id,
        unsigned int components, unsigned int contexts,
        unsigned char *available_in, unsigned char *available_out) {
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
                 predecessor < predecessor_end; predecessor++) {
                size_t predecessor_node;
                predecessor_node = graph->predecessors[predecessor];
                if (graph->nodes[predecessor_node].reachable &&
                    !available_out[predecessor_node])
                    next_in = 0;
            }
            if (flow_is_async_handler_target(graph, index)) next_in = 0;
            if (index == 0) next_in = 0;
            if (index == generator) next_out = 1;
            else if (flow_node_kills_storage_derivation(
                             graph, index, storage_id, components, contexts))
                next_out = 0;
            else
                next_out = next_in;
            if (available_in[index] != (unsigned char)next_in ||
                available_out[index] != (unsigned char)next_out) {
                available_in[index] = (unsigned char)next_in;
                available_out[index] = (unsigned char)next_out;
                changed = 1;
            }
        }
    } while (changed);
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
            rxop_can_signal(producer_node->op->opcode) ||
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
            copy_node->effects.semantics != RXOP_SEM_NONE ||
            rxop_can_signal(copy_node->op->opcode))
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

static size_t flow_remove_redundant_itos(flow_graph *graph,
                                         flow_stats *stats) {
    size_t generator;
    size_t index;
    int register_index;
    int candidate_register;
    size_t storage_id;
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
            graph->nodes[generator].op->opcode != OP_ITOS_REG)
            continue;
        if (rxop_value_derivation(OP_ITOS_REG) !=
                RXOP_DERIVATION_INTEGER_TO_STRING ||
            rxop_signal_phase(OP_ITOS_REG) != RXOP_SIGNAL_PHASE_NONE)
            continue;
        if (!flow_storage_attach(graph)) return removed;
        register_index = flow_register_index(
                graph, flow_register_type(first->operand1Token),
                (size_t)first->operand1Token->token_value.integer);
        if (register_index < 0) continue;
        storage_id = flow_storage_identity_at(graph, generator,
                                              register_index);
        if (!storage_id) continue;
        available_in = calloc(graph->item_count, 1);
        available_out = calloc(graph->item_count, 1);
        if (!available_in || !available_out)
            RX_PANIC_OOM("calloc RXAS storage derivation fact",
                         graph->item_count * 2, 0);
        flow_compute_available_storage_derivation(
                graph, generator, storage_id,
                RXOP_COMPONENT_INTEGER | RXOP_COMPONENT_STRING,
                rxop_derivation_context_reads(OP_ITOS_REG),
                available_in, available_out);
        for (index = generator + 1; index < graph->item_count; index++) {
            item = &graph->items[index];
            if (!available_in[index] || item->instrType != OP_CODE ||
                !graph->nodes[index].op ||
                graph->nodes[index].op->opcode != OP_ITOS_REG ||
                item->operandCount != 1)
                continue;
            candidate_register = flow_register_index(
                    graph, flow_register_type(item->operand1Token),
                    (size_t)item->operand1Token->token_value.integer);
            if (candidate_register < 0 ||
                flow_storage_identity_at(graph, index,
                                         candidate_register) != storage_id)
                continue;
            item->instrType = EMPTY;
            flow_debug_accept(graph, index,
                              "redundant-storage-component-conversion", 0);
            removed++;
            stats->redundant_conversions_removed++;
        }
        free(available_in);
        free(available_out);
    }
    return removed;
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
    removed = flow_remove_redundant_itos(graph, stats);
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
    RxasFlowProcedure *procedure;
    const RxasFlowStructuralAnalysis *structural;
    const RxasFlowSignalAnalysis *signal_analysis;
    const RxasFlowSsaAnalysis *ssa_analysis;
    const OpInfo **resolved_ops;
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
