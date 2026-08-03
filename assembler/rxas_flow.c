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
#include "rxas_flow_pass.h"
#include "rxas_flow_proof.h"
#include "rxas_flow_rewrite.h"
#include "rxas_flow_signal.h"
#include "rxas_flow_ssa.h"
#include "rxas_flow_use.h"
#include "rxdefs.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Bound global value analysis only for procedures newly admitted by exact
 * indirect-table edges.  Reachability remains linear and unbounded. */
#define FLOW_MAX_INDIRECT_VALUE_CELLS 1000000

typedef struct flow_register {
    char type;
    size_t number;
    Assembler_Token *token;
} flow_register;

typedef struct flow_node {
    const OpInfo *op;
    RxOpEffects effects;
    int reachable;
} flow_node;

/* Temporary rewrite-facing compatibility view. It owns no control-flow facts:
 * all edges, blocks, reachability and lazy analyses belong to procedure. D0.3
 * removes the remaining register/opcode convenience fields as each semantic
 * consumer moves to explicit capabilities. */
typedef struct flow_graph {
    Assembler_Context *context;
    instruction_queue *items;
    size_t item_count;
    RxasFlowProcedure *procedure;
    unsigned long epoch;
    flow_node *nodes;
    flow_register *registers;
    size_t register_count;
    size_t register_capacity;
    size_t word_count;
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
    size_t storage_permutation_queries;
    size_t storage_permutation_proved;
    size_t storage_permutation_rejected;
    size_t swap_round_trip_instructions_removed;
    size_t branch_thread_queries;
    size_t branch_thread_proved;
    size_t branch_thread_rejected;
    size_t branch_threads_applied;
    size_t branch_thread_batches_applied;
    size_t operands_redirected;
    size_t rejected_live;
    size_t rejected_trace;
    size_t rejected_tainted;
    size_t rejected_effect;
} flow_stats;

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

static int flow_build_graph(flow_graph *graph, Assembler_Context *context,
                            instruction_queue *items, size_t item_count,
                            unsigned long epoch) {
    size_t index;
    const RxasFlowMetrics *metrics;
    memset(graph, 0, sizeof(*graph));
    graph->context = context;
    graph->items = items;
    graph->item_count = item_count;
    graph->epoch = epoch;
    graph->procedure = rxas_flow_procedure_build(
            context, items, item_count, epoch);
    if (!graph->procedure) return 0;
    metrics = rxas_flow_procedure_metrics(graph->procedure, epoch);
    if (!metrics) {
        rxas_flow_procedure_destroy(graph->procedure);
        memset(graph, 0, sizeof(*graph));
        return 0;
    }
    flow_collect_registers(graph);
    graph->word_count = (graph->register_count * 7 +
                         sizeof(unsigned long) * 8 - 1) /
                        (sizeof(unsigned long) * 8);
    if (!graph->word_count) graph->word_count = 1;
    graph->nodes = calloc(item_count ? item_count : 1,
                          sizeof(*graph->nodes));
    if (!graph->nodes)
        RX_PANIC_OOM("calloc RXAS flow rewrite view",
                     item_count * sizeof(*graph->nodes),
                     context && context->file_name ? context->file_name : 0);
    for (index = 0; index < item_count; index++) {
        const RxasFlowRecord *record;
        const RxasFlowInstruction *instruction;
        record = rxas_flow_procedure_record(
                graph->procedure, epoch, index);
        graph->nodes[index].reachable =
                rxas_flow_procedure_record_reachable(
                        graph->procedure, epoch, index);
        instruction = record && record->instruction_id != RXAS_FLOW_ID_NONE
                ? rxas_flow_procedure_instruction(
                        graph->procedure, epoch, record->instruction_id)
                : 0;
        graph->nodes[index].op = instruction ? instruction->op : 0;
        if (instruction) {
            graph->nodes[index].effects = instruction->effects;
            if (instruction->effects.semantics & RXOP_SEM_INDIRECT_BRANCH)
                graph->resolved_indirect_branches++;
        }
    }
    graph->block_count = metrics->code_blocks;
    graph->complete_control_flow = metrics->complete_control_flow;
    return 1;
}

static void flow_free_graph(flow_graph *graph) {
    if (graph->procedure)
        rxas_flow_procedure_destroy(graph->procedure);
    free(graph->nodes);
    free(graph->registers);
    memset(graph, 0, sizeof(*graph));
}

static size_t flow_remove_unreachable(flow_graph *graph, flow_stats *stats) {
    size_t index;
    size_t removed;
    removed = 0;
    if (!graph->complete_control_flow) return 0;
    for (index = 0; index < graph->item_count; index++) {
        if (!rxas_flow_procedure_record_reachable(
                    graph->procedure, graph->epoch, index)) {
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
                removed++;
            }
        }
    }
    return removed;
}
static int flow_storage_is_pure_swap_opcode(int opcode) {
    return opcode == OP_SWAP_REG_REG ||
           opcode == OP_SWAPN_REG_REG_REG_REG ||
           opcode == OP_SWAPN_REG_REG_REG_REG_REG_REG ||
           opcode == OP_SWAPN_REG_REG_REG_REG_REG_REG_REG_REG;
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

static int flow_proof_reason_unavailable(RxasFlowProofReason reason);

typedef struct flow_proof_session {
    RxasFlowProcedure *procedure;
    const RxasFlowProofService *proof;
    unsigned long epoch;
} flow_proof_session;

static const RxasFlowProofService *flow_proof_session_require(
        flow_proof_session *session, const flow_graph *graph,
        RxasOptimisationPassId pass_id);
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

typedef struct flow_swap_pair_slot {
    int occupied;
    char left_type;
    char right_type;
    size_t left_number;
    size_t right_number;
    size_t record_id;
} flow_swap_pair_slot;

static int flow_swap_pair_from_operands(
        const instruction_queue *item, size_t first_operand,
        char *left_type, size_t *left_number,
        char *right_type, size_t *right_number) {
    const Assembler_Token *left;
    const Assembler_Token *right;
    char temporary_type;
    size_t temporary_number;
    if (!item || !left_type || !left_number || !right_type ||
        !right_number || first_operand + 1 >= item->operandCount)
        return 0;
    left = rxas_queue_operand(item, first_operand);
    right = rxas_queue_operand(item, first_operand + 1);
    *left_type = flow_register_type(left);
    *right_type = flow_register_type(right);
    if (!*left_type || !*right_type ||
        left->token_value.integer < 0 || right->token_value.integer < 0)
        return 0;
    *left_number = (size_t)left->token_value.integer;
    *right_number = (size_t)right->token_value.integer;
    if (*left_type > *right_type ||
        (*left_type == *right_type && *left_number > *right_number)) {
        temporary_type = *left_type;
        *left_type = *right_type;
        *right_type = temporary_type;
        temporary_number = *left_number;
        *left_number = *right_number;
        *right_number = temporary_number;
    }
    return *left_type != *right_type || *left_number != *right_number;
}

static size_t flow_swap_pair_hash(char left_type, size_t left_number,
                                  char right_type, size_t right_number) {
    size_t hash;
    hash = (size_t)(unsigned char)left_type;
    hash = hash * 131 + left_number;
    hash = hash * 131 + (size_t)(unsigned char)right_type;
    return hash * 131 + right_number;
}

static flow_swap_pair_slot *flow_swap_pair_find_slot(
        flow_swap_pair_slot *slots, size_t capacity,
        char left_type, size_t left_number,
        char right_type, size_t right_number) {
    size_t index;
    size_t start;
    if (!slots || !capacity) return 0;
    index = flow_swap_pair_hash(
            left_type, left_number, right_type, right_number) &
            (capacity - 1);
    start = index;
    do {
        if (!slots[index].occupied ||
            (slots[index].left_type == left_type &&
             slots[index].left_number == left_number &&
             slots[index].right_type == right_type &&
             slots[index].right_number == right_number))
            return &slots[index];
        index = (index + 1) & (capacity - 1);
    } while (index != start);
    return 0;
}

static int flow_storage_permutation_plan_matches(
        const flow_graph *graph,
        const RxasFlowStoragePermutationPlan *plan) {
    const instruction_queue *first;
    const instruction_queue *second;
    if (!graph || !plan || !plan->proved ||
        plan->first_record_id >= graph->item_count ||
        plan->second_record_id >= graph->item_count)
        return 0;
    first = &graph->items[plan->first_record_id];
    second = &graph->items[plan->second_record_id];
    if (first->instrType != OP_CODE || second->instrType != OP_CODE ||
        !graph->nodes[plan->first_record_id].op ||
        !graph->nodes[plan->second_record_id].op ||
        graph->nodes[plan->first_record_id].op->opcode !=
                plan->expected_first_opcode ||
        graph->nodes[plan->second_record_id].op->opcode !=
                plan->expected_second_opcode ||
        !flow_operand_matches_proof_register(
                rxas_queue_operand(first, 0), plan->first_left) ||
        !flow_operand_matches_proof_register(
                rxas_queue_operand(first, 1), plan->first_right))
        return 0;
    if (plan->deletion_count == 1)
        return plan->first_record_id == plan->second_record_id &&
               first->operandCount == 4 &&
               flow_operand_matches_proof_register(
                    rxas_queue_operand(first, 2), plan->second_left) &&
               flow_operand_matches_proof_register(
                    rxas_queue_operand(first, 3), plan->second_right);
    return plan->deletion_count == 2 &&
           plan->first_record_id < plan->second_record_id &&
           second->operandCount == 2 &&
           flow_operand_matches_proof_register(
                rxas_queue_operand(second, 0), plan->second_left) &&
           flow_operand_matches_proof_register(
                rxas_queue_operand(second, 1), plan->second_right);
}

/* K01 demand-filters exact repeated physical register pairs. The immutable
 * proof owns storage restoration, signals and every intervening observation
 * or write. One accepted plan is applied per epoch, then the graph is rebuilt. */
static size_t flow_remove_swap_round_trips(
        flow_graph *graph, flow_stats *stats,
        flow_proof_session *session) {
    const RxasFlowProofService *proof;
    const RxasFlowRecord *first_record;
    const RxasFlowRecord *second_record;
    RxasFlowStoragePermutationPlan plan;
    flow_swap_pair_slot *slots;
    flow_swap_pair_slot *slot;
    size_t capacity;
    size_t required_capacity;
    size_t swap_count;
    size_t index;
    size_t first_instruction;
    size_t second_instruction;
    char left_type;
    char right_type;
    char encoded_left_type;
    char encoded_right_type;
    size_t left_number;
    size_t right_number;
    size_t encoded_left_number;
    size_t encoded_right_number;
    swap_count = 0;
    for (index = 0; index < graph->item_count; index++)
        if (graph->items[index].instrType == OP_CODE &&
            graph->nodes[index].reachable && graph->nodes[index].op &&
            (graph->nodes[index].op->opcode == OP_SWAP_REG_REG ||
             graph->nodes[index].op->opcode ==
                    OP_SWAPN_REG_REG_REG_REG))
            swap_count++;
    if (!swap_count) return 0;
    if (swap_count > ((size_t)-1 - 1) / 2) return 0;
    required_capacity = swap_count * 2 + 1;
    capacity = 8;
    while (capacity < required_capacity) {
        if (capacity > (size_t)-1 / 2) return 0;
        capacity *= 2;
    }
    if (capacity > (size_t)-1 / sizeof(*slots)) return 0;
    slots = calloc(capacity, sizeof(*slots));
    if (!slots)
        RX_PANIC_OOM("calloc RXAS swap-pair candidates",
                     capacity * sizeof(*slots), 0);
    proof = 0;
    for (index = 0; index < graph->item_count; index++) {
        if (graph->items[index].instrType != OP_CODE ||
            !graph->nodes[index].reachable || !graph->nodes[index].op)
            continue;
        if (graph->nodes[index].op->opcode ==
                OP_SWAPN_REG_REG_REG_REG &&
            flow_swap_pair_from_operands(
                    &graph->items[index], 0,
                    &left_type, &left_number,
                    &right_type, &right_number) &&
            flow_swap_pair_from_operands(
                    &graph->items[index], 2,
                    &encoded_left_type, &encoded_left_number,
                    &encoded_right_type, &encoded_right_number) &&
            left_type == encoded_left_type &&
            left_number == encoded_left_number &&
            right_type == encoded_right_type &&
            right_number == encoded_right_number) {
            proof = flow_proof_session_require(
                    session, graph, RXAS_PASS_K01_STORAGE_PERMUTATION);
            first_record = session->procedure ? rxas_flow_procedure_record(
                    session->procedure, session->epoch, index) : 0;
            first_instruction = first_record
                    ? first_record->instruction_id : RXAS_FLOW_ID_NONE;
            if (!proof || first_instruction == RXAS_FLOW_ID_NONE ||
                !rxas_flow_prove_storage_permutation_round_trip(
                        proof, session->epoch, first_instruction,
                        first_instruction, &plan)) {
                stats->storage_permutation_rejected++;
                continue;
            }
            stats->storage_permutation_queries++;
            if (plan.proved &&
                flow_storage_permutation_plan_matches(graph, &plan)) {
                graph->items[index].instrType = EMPTY;
                stats->storage_permutation_proved++;
                stats->swap_round_trip_instructions_removed++;
                flow_debug_accept(
                        graph, index, "storage-permutation-round-trip", 0);
                free(slots);
                return 1;
            }
            stats->storage_permutation_rejected++;
            if (graph->context->debug_mode)
                fprintf(stderr,
                        "PERF3 storage-permutation-proof procedure=%s "
                        "first=%llu:swapn second=%llu:swapn proved=0 "
                        "reason=%s\n",
                        graph->context->current_proc_name
                                ? graph->context->current_proc_name
                                : "(directives)",
                        (unsigned long long)index,
                        (unsigned long long)index,
                        rxas_flow_proof_reason_name(plan.reason));
            continue;
        }
        if (graph->nodes[index].op->opcode != OP_SWAP_REG_REG ||
            !flow_swap_pair_from_operands(
                    &graph->items[index], 0,
                    &left_type, &left_number,
                    &right_type, &right_number))
            continue;
        slot = flow_swap_pair_find_slot(
                slots, capacity, left_type, left_number,
                right_type, right_number);
        if (!slot) continue;
        if (!slot->occupied) {
            slot->occupied = 1;
            slot->left_type = left_type;
            slot->left_number = left_number;
            slot->right_type = right_type;
            slot->right_number = right_number;
            slot->record_id = index;
            continue;
        }
        proof = flow_proof_session_require(
                session, graph, RXAS_PASS_K01_STORAGE_PERMUTATION);
        first_record = session->procedure ? rxas_flow_procedure_record(
                session->procedure, session->epoch, slot->record_id) : 0;
        second_record = session->procedure ? rxas_flow_procedure_record(
                session->procedure, session->epoch, index) : 0;
        first_instruction = first_record
                ? first_record->instruction_id : RXAS_FLOW_ID_NONE;
        second_instruction = second_record
                ? second_record->instruction_id : RXAS_FLOW_ID_NONE;
        if (!proof || first_instruction == RXAS_FLOW_ID_NONE ||
            second_instruction == RXAS_FLOW_ID_NONE ||
            !rxas_flow_prove_storage_permutation_round_trip(
                    proof, session->epoch, first_instruction,
                    second_instruction, &plan)) {
            stats->storage_permutation_rejected++;
            slot->record_id = index;
            continue;
        }
        stats->storage_permutation_queries++;
        if (!plan.proved) {
            stats->storage_permutation_rejected++;
            if (graph->context->debug_mode)
                fprintf(stderr,
                        "PERF3 storage-permutation-proof procedure=%s "
                        "first=%llu:swap second=%llu:swap proved=0 "
                        "reason=%s\n",
                        graph->context->current_proc_name
                                ? graph->context->current_proc_name
                                : "(directives)",
                        (unsigned long long)slot->record_id,
                        (unsigned long long)index,
                        rxas_flow_proof_reason_name(plan.reason));
            slot->record_id = index;
            continue;
        }
        if (!flow_storage_permutation_plan_matches(graph, &plan)) {
            stats->storage_permutation_rejected++;
            slot->record_id = index;
            continue;
        }
        graph->items[plan.first_record_id].instrType = EMPTY;
        graph->items[plan.second_record_id].instrType = EMPTY;
        stats->storage_permutation_proved++;
        stats->swap_round_trip_instructions_removed += 2;
        flow_debug_accept(
                graph, plan.second_record_id,
                "storage-permutation-round-trip", 0);
        free(slots);
        return 2;
    }
    free(slots);
    return 0;
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
    proof = flow_proof_session_require(
            session, graph, RXAS_PASS_M05_TYPED_COPY);
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
        proof = flow_proof_session_require(
                session, graph, RXAS_PASS_M06_PRODUCER_FORWARD);
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
    proof = flow_proof_session_require(
            session, graph, RXAS_PASS_K02_K03_LINKED_READ);
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
        proof = flow_proof_session_require(
                session, graph, RXAS_PASS_K04_COMPARE_BRANCH);
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
    proof = flow_proof_session_require(
            session, graph, RXAS_PASS_M02_CONSTANT);
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
    proof = flow_proof_session_require(
            session, graph, RXAS_PASS_M03_ABSENT);
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

/* One immutable proof graph serves every migrated consumer in a fixed-point
 * epoch.  Rebuilding it per query family needlessly multiplies peak allocator
 * pages on large inlined procedures and defeats the analysis-manager design. */
static const RxasFlowProofService *flow_proof_session_require(
        flow_proof_session *session, const flow_graph *graph,
        RxasOptimisationPassId pass_id) {
    const RxasOptimisationPassDescriptor *descriptor;
    unsigned int acquired;
    if (!session || !graph) return 0;
    descriptor = rxas_optimisation_pass_descriptor(pass_id);
    if (!descriptor || descriptor->owner != RXAS_OPT_OWNER_SSA ||
        !descriptor->capabilities ||
        (descriptor->capabilities & RXAS_FLOW_CAP_LOCAL_SCAN))
        return 0;
    if (!session->procedure) {
        session->epoch = graph->epoch;
        session->procedure = graph->procedure;
    }
    if (session->procedure != graph->procedure ||
        session->epoch != graph->epoch)
        return 0;
    session->proof = session->procedure
            ? rxas_flow_require_proof_capabilities(
                    session->procedure, session->epoch,
                    descriptor->capabilities, 0)
            : 0;
    acquired = rxas_flow_proof_capabilities(
            session->proof, session->epoch);
    if ((acquired & descriptor->capabilities) != descriptor->capabilities)
        session->proof = 0;
    return session->proof;
}

static void flow_proof_session_destroy(flow_proof_session *session) {
    if (!session) return;
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
    proof = flow_proof_session_require(
            session, graph, RXAS_PASS_M04_SELF_COPY);
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
    proof = flow_proof_session_require(
            session, graph, RXAS_PASS_M01_DERIVATION);
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
            "swap-roundtrip=%llu "
            "compare-branch=%llu trace-delete=%llu redirects=%llu "
            "constant-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "absent-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "self-copy-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "derivation-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "producer-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "duplicate-linked-read-proof=%llu/%llu rejected=%llu reused=%llu "
            "storage-permutation-proof=%llu/%llu rejected=%llu "
            "compare-branch-proof=%llu/%llu rejected=%llu "
            "branch-thread=%llu/%llu rejected=%llu applied=%llu batches=%llu "
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
            (unsigned long long)stats->swap_round_trip_instructions_removed,
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
            (unsigned long long)stats->storage_permutation_proved,
            (unsigned long long)stats->storage_permutation_queries,
            (unsigned long long)stats->storage_permutation_rejected,
            (unsigned long long)stats->compare_branch_proved,
            (unsigned long long)stats->compare_branch_queries,
            (unsigned long long)stats->compare_branch_rejected,
            (unsigned long long)stats->branch_thread_proved,
            (unsigned long long)stats->branch_thread_queries,
            (unsigned long long)stats->branch_thread_rejected,
            (unsigned long long)stats->branch_threads_applied,
            (unsigned long long)stats->branch_thread_batches_applied,
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

/* M07 is an executable diagnostic oracle, not an optimisation owner. Query
 * only storage-mapping destinations from the sparse SSA service and report
 * its retained node counts; never materialise a record-by-register matrix. */
static void flow_debug_sparse_storage_oracle(
        const flow_graph *graph, const RxasFlowSsaAnalysis *analysis) {
    const RxasFlowSsaMetrics *metrics;
    size_t instruction_id;
    size_t queries;
    size_t exact;
    size_t unknown;
    size_t base_nodes;
    size_t site_nodes;
    size_t attribute_nodes;
    size_t phi_nodes;
    size_t unknown_nodes;
    if (!graph || !analysis || !graph->context->debug_mode) return;
    metrics = rxas_flow_ssa_metrics(analysis, graph->epoch);
    if (!metrics) return;
    queries = 0;
    exact = 0;
    unknown = 0;
    base_nodes = 0;
    site_nodes = 0;
    attribute_nodes = 0;
    phi_nodes = 0;
    unknown_nodes = 0;
    for (instruction_id = 1;
         instruction_id <= metrics->storage_versions;
         instruction_id++) {
        RxasFlowStorageNode node;
        if (!rxas_flow_storage_node(
                    analysis, graph->epoch, instruction_id, &node))
            continue;
        if (node.kind == RXAS_FLOW_STORAGE_BASE) base_nodes++;
        else if (node.kind == RXAS_FLOW_STORAGE_SITE) site_nodes++;
        else if (node.kind == RXAS_FLOW_STORAGE_ATTRIBUTE_PATH)
            attribute_nodes++;
        else if (node.kind == RXAS_FLOW_STORAGE_PHI) phi_nodes++;
        else unknown_nodes++;
    }
    for (instruction_id = 0;
         instruction_id < rxas_flow_procedure_metrics(
                 graph->procedure, graph->epoch)->instructions;
         instruction_id++) {
        const RxasFlowInstruction *instruction;
        const instruction_queue *item;
        Assembler_Token *destination;
        RxasFlowRegister reg;
        RxasFlowStorageFact fact;
        char type;
        instruction = rxas_flow_procedure_instruction(
                graph->procedure, graph->epoch, instruction_id);
        if (!instruction || !instruction->op ||
            !rxas_flow_opcode_is_plain_mapping(instruction->op->opcode))
            continue;
        item = &graph->items[instruction->record_id];
        destination = rxas_queue_operand(item, 0);
        type = flow_register_type(destination);
        if (!type || destination->token_value.integer < 0) continue;
        reg.register_class = type == 'a' ? RXAS_FLOW_REGISTER_ARGUMENT
                           : type == 'g' ? RXAS_FLOW_REGISTER_GLOBAL
                                         : RXAS_FLOW_REGISTER_LOCAL;
        reg.number = (size_t)destination->token_value.integer;
        queries++;
        if (rxas_flow_storage_at_instruction(
                    analysis, graph->epoch, instruction_id, 1, reg, &fact) &&
            fact.kind != RXAS_FLOW_STORAGE_UNKNOWN)
            exact++;
        else unknown++;
    }
    fprintf(stderr,
            "PERF3 sparse-storage-oracle procedure=%s status=complete "
            "queries=%llu exact=%llu unknown=%llu storage=%llu base=%llu "
            "site=%llu attribute=%llu phi=%llu unknown-nodes=%llu "
            "retained-bytes=%llu\n",
            graph->context->current_proc_name
                    ? graph->context->current_proc_name : "(directives)",
            (unsigned long long)queries,
            (unsigned long long)exact,
            (unsigned long long)unknown,
            (unsigned long long)(base_nodes + site_nodes + attribute_nodes +
                                 phi_nodes + unknown_nodes),
            (unsigned long long)base_nodes,
            (unsigned long long)site_nodes,
            (unsigned long long)attribute_nodes,
            (unsigned long long)phi_nodes,
            (unsigned long long)unknown_nodes,
            (unsigned long long)metrics->retained_bytes);
}

static size_t flow_apply_branch_thread_epoch(flow_graph *graph,
                                             flow_stats *stats) {
    Assembler_Context *context;
    RxasFlowBranchThreadBatch batch;
    size_t applied;
    size_t plan_index;

    if (!graph || !stats || !graph->procedure) return 0;
    context = graph->context;
    memset(&batch, 0, sizeof(batch));
    applied = 0;
    stats->branch_thread_queries++;
    if (rxas_flow_plan_branch_threads(
            graph->procedure, graph->epoch, &batch)) {
        stats->branch_thread_proved += batch.plan_count;
    }
    else stats->branch_thread_rejected++;

    if (batch.plan_count) {
        applied = rxas_flow_apply_branch_threads(context, &batch);
        if (applied == batch.plan_count) {
            stats->branch_threads_applied += applied;
            stats->branch_thread_batches_applied++;
            if (context->debug_mode) {
                for (plan_index = 0; plan_index < batch.plan_count;
                     plan_index++)
                    fprintf(stderr,
                            "NR27 accept procedure=%s record=%llu "
                            "reason=immutable-cfg-branch-thread\n",
                            context->current_proc_name,
                            (unsigned long long)
                            batch.plans[plan_index].source_record_id);
            }
        }
        else {
            stats->branch_thread_rejected++;
            applied = 0;
        }
    }
    rxas_flow_branch_thread_batch_destroy(&batch);
    return applied;
}

void rxas_flow_optimise(Assembler_Context *context) {
    flow_graph graph;
    flow_stats stats;
    RxasFlowProcedure *procedure;
    const RxasFlowStructuralAnalysis *structural;
    const RxasFlowSignalAnalysis *signal_analysis;
    const RxasFlowSsaAnalysis *ssa_analysis;
    const RxasFlowUseAnalysis *use_analysis;
    const RxasFlowProofService *proof_service;
    instruction_queue *items;
    size_t item_count;
    size_t before_instructions;
    size_t after_instructions;
    size_t changed;
    size_t iterations;
    int graph_built;
    unsigned int semantic_capabilities;
    unsigned int diagnostic_capabilities;
    unsigned int analysis_capabilities;
    RxasOptimisationCensus census;
    flow_proof_session proof_session;

    if (!context || !context->procedure_queue ||
        !context->procedure_queue_items || !context->current_proc_name)
        return;
    items = context->procedure_queue;
    item_count = context->procedure_queue_items;
    memset(&stats, 0, sizeof(stats));
    before_instructions = flow_instruction_count(items, item_count);
    iterations = 0;
    do {
        items = context->procedure_queue;
        item_count = context->procedure_queue_items;
        if (!rxas_optimisation_census(
                    context, items, item_count, &census))
            return;
        if (context->debug_mode && !iterations)
            rxas_optimisation_census_dump(
                    &census, context->current_proc_name, stderr);
        memset(&proof_session, 0, sizeof(proof_session));
        graph_built = flow_build_graph(
                &graph, context, items, item_count,
                (unsigned long)iterations + 1ul);
        if (!graph_built) {
            if (context->debug_mode)
                fprintf(stderr,
                        "PERF3 flow-graph procedure=%s "
                        "disabled=construction-failed\\n",
                        context->current_proc_name);
            return;
        }
        stats.procedures = 1;
        stats.blocks = graph.block_count;
        /* Every structural and semantic consumer shares this one immutable
         * graph for the epoch. A mutation ends the epoch before any later
         * consumer can observe stale block, signal, storage or value facts. */
        if (!graph.complete_control_flow) {
            if (context->debug_mode)
                fprintf(stderr,
                        "NR27 reject procedure=%s candidate=whole-procedure "
                        "reason=incomplete-control-flow\\n",
                        context->current_proc_name);
            changed = 0;
        }
        else {
            changed = rxas_optimisation_has_candidates(
                    &census, RXAS_PASS_M00_REACHABILITY)
                    ? flow_remove_unreachable(&graph, &stats) : 0;
            /* K01 is a sparse, independently budgeted proof and must not be
             * coupled to the temporary indirect-value admission bound. */
            if (!changed && rxas_optimisation_has_candidates(
                    &census, RXAS_PASS_K01_STORAGE_PERMUTATION))
                changed += flow_remove_swap_round_trips(
                        &graph, &stats, &proof_session);
            if (!changed && flow_value_analysis_within_bound(&graph)) {
                if (rxas_optimisation_has_candidates(
                        &census, RXAS_PASS_M04_SELF_COPY))
                    changed += flow_remove_redundant_self_copies(
                            &graph, &stats, &proof_session);
                if (!changed && rxas_optimisation_has_candidates(
                        &census, RXAS_PASS_K02_K03_LINKED_READ))
                    changed += flow_reuse_duplicate_linked_read(
                            &graph, &stats, &proof_session);
                if (!changed && rxas_optimisation_has_candidates(
                        &census, RXAS_PASS_M05_TYPED_COPY))
                    changed += flow_propagate_copies(
                            &graph, &stats, &proof_session);
                if (!changed && rxas_optimisation_has_candidates(
                        &census, RXAS_PASS_M06_PRODUCER_FORWARD))
                    changed += flow_forward_producer_destination(
                            &graph, &stats, &proof_session);
                if (!changed && rxas_optimisation_has_candidates(
                        &census, RXAS_PASS_K04_COMPARE_BRANCH))
                    changed += flow_fuse_compare_branches(
                            &graph, &stats, &proof_session);
                if (!changed && rxas_optimisation_has_candidates(
                        &census, RXAS_PASS_M02_CONSTANT))
                    changed += flow_remove_redundant_loads(
                            &graph, &stats, &proof_session);
                if (!changed && rxas_optimisation_has_candidates(
                        &census, RXAS_PASS_M03_ABSENT))
                    changed += flow_remove_redundant_initializations(
                            &graph, &stats, &proof_session);
                if (!changed && rxas_optimisation_has_candidates(
                        &census, RXAS_PASS_M01_DERIVATION))
                    changed += flow_remove_redundant_conversions(
                            &graph, &stats, &proof_session);
            }
            else if (!changed && context->debug_mode) {
                fprintf(stderr,
                        "NR27 bound procedure=%s scope=reachability-only "
                        "value-cells=%llu limit=%llu\\n",
                        context->current_proc_name,
                        (unsigned long long)
                                (graph.item_count * graph.word_count),
                        (unsigned long long)FLOW_MAX_INDIRECT_VALUE_CELLS);
            }
            /* K05 plans every compatible branch thread against the same CFG
             * and applies the batch only after all semantic consumers decline
             * the epoch. Value rewrites therefore retain their accepted
             * precedence while branch threading no longer rebuilds per site. */
            if (!changed && rxas_optimisation_has_candidates(
                    &census, RXAS_PASS_K05_BRANCH_THREAD))
                changed = flow_apply_branch_thread_epoch(&graph, &stats);
            /* P3 dead-result deletion is deliberately absent. A nominal
             * integer/float write may release hidden reference or native
             * payload state; numeric liveness alone cannot prove that effect
             * unobservable. */
        }
        flow_proof_session_destroy(&proof_session);
        flow_free_graph(&graph);
        iterations++;
    } while (changed && iterations <= before_instructions + 1);

    items = context->procedure_queue;
    item_count = context->procedure_queue_items;
    graph_built = flow_build_graph(
            &graph, context, items, item_count,
            (unsigned long)iterations + 1ul);
    after_instructions = flow_instruction_count(items, item_count);
    if (!graph_built) {
        if (context->debug_mode)
            fprintf(stderr,
                    "PERF3 flow-graph procedure=%s "
                    "disabled=construction-failed\\n",
                    context->current_proc_name);
        return;
    }
    flow_debug_summary(&graph, &stats,
                       before_instructions, after_instructions);
    procedure = graph.procedure;
    if (procedure) {
        if (context->debug_mode) {
            semantic_capabilities =
                    rxas_optimisation_capabilities_for_owner(
                            &census, RXAS_OPT_OWNER_SSA);
            diagnostic_capabilities =
                    rxas_optimisation_capabilities_for_owner(
                            &census, RXAS_OPT_OWNER_DIAGNOSTIC);
            analysis_capabilities = semantic_capabilities |
                                    diagnostic_capabilities;
            /* Structural analyses are demand-driven.  Until an optimizer
             * consumer requests them, ordinary assembly must not retain or
             * solve facts that only diagnostics use. */
            structural = (analysis_capabilities & RXAS_FLOW_CAP_LOOPS)
                    ? rxas_flow_require_loop_analysis(
                            procedure, rxas_flow_procedure_epoch(procedure), 0)
                    : (analysis_capabilities &
                       (RXAS_FLOW_CAP_DOMINANCE | RXAS_FLOW_CAP_SIGNAL |
                        RXAS_FLOW_CAP_STORAGE | RXAS_FLOW_CAP_VALUE |
                        RXAS_FLOW_CAP_USE))
                            ? rxas_flow_require_structural_analysis(
                                    procedure,
                                    rxas_flow_procedure_epoch(procedure), 0)
                            : 0;
            rxas_flow_procedure_dump(
                    procedure, rxas_flow_procedure_epoch(procedure), stderr);
            if (structural)
                rxas_flow_structural_dump(
                        structural, rxas_flow_procedure_epoch(procedure),
                        stderr);
            else if (analysis_capabilities &
                     (RXAS_FLOW_CAP_DOMINANCE | RXAS_FLOW_CAP_SIGNAL |
                      RXAS_FLOW_CAP_STORAGE | RXAS_FLOW_CAP_VALUE |
                      RXAS_FLOW_CAP_USE | RXAS_FLOW_CAP_LOOPS)) {
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
            signal_analysis = (analysis_capabilities &
                               (RXAS_FLOW_CAP_SIGNAL |
                                RXAS_FLOW_CAP_STORAGE |
                                RXAS_FLOW_CAP_VALUE | RXAS_FLOW_CAP_USE))
                    ? rxas_flow_require_signal_analysis(
                            procedure, rxas_flow_procedure_epoch(procedure), 0)
                    : 0;
            if (signal_analysis)
                rxas_flow_signal_dump(
                        signal_analysis, rxas_flow_procedure_epoch(procedure),
                        stderr);
            else if (analysis_capabilities &
                     (RXAS_FLOW_CAP_SIGNAL | RXAS_FLOW_CAP_STORAGE |
                      RXAS_FLOW_CAP_VALUE | RXAS_FLOW_CAP_USE)) {
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
            ssa_analysis = (analysis_capabilities &
                            (RXAS_FLOW_CAP_STORAGE | RXAS_FLOW_CAP_VALUE |
                             RXAS_FLOW_CAP_USE))
                    ? rxas_flow_require_ssa_analysis(
                            procedure, rxas_flow_procedure_epoch(procedure), 0)
                    : 0;
            if (ssa_analysis) {
                rxas_flow_ssa_dump(
                        ssa_analysis, rxas_flow_procedure_epoch(procedure),
                        stderr);
                flow_debug_sparse_storage_oracle(&graph, ssa_analysis);
            }
            else if (analysis_capabilities &
                     (RXAS_FLOW_CAP_STORAGE | RXAS_FLOW_CAP_VALUE |
                      RXAS_FLOW_CAP_USE)) {
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
            proof_service = analysis_capabilities
                    ? rxas_flow_require_proof_capabilities(
                            procedure, rxas_flow_procedure_epoch(procedure),
                            analysis_capabilities, 0)
                    : 0;
            if (proof_service)
                rxas_flow_proof_dump(
                        proof_service, rxas_flow_procedure_epoch(procedure),
                        stderr);
            else if (analysis_capabilities) {
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
            use_analysis = (analysis_capabilities & RXAS_FLOW_CAP_USE)
                    ? rxas_flow_require_use_analysis(
                            procedure, rxas_flow_procedure_epoch(procedure), 0)
                    : 0;
            if (use_analysis)
                rxas_flow_use_dump(
                        use_analysis, rxas_flow_procedure_epoch(procedure),
                        stderr);
            else if (analysis_capabilities & RXAS_FLOW_CAP_USE) {
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
        flow_free_graph(&graph);
    }
}
