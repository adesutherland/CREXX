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
#include "rxas_flow_batch.h"
#include "rxas_flow_graph.h"
#include "rxas_flow_pass.h"
#include "rxas_flow_proof.h"
#include "rxas_flow_rewrite.h"
#include "rxas_flow_signal.h"
#include "rxas_flow_ssa.h"
#include "rxas_flow_use.h"
#include "rxdefs.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Bound global value analysis only for procedures newly admitted by exact
 * indirect-table edges.  Reachability remains linear and unbounded. */
#define FLOW_MAX_INDIRECT_VALUE_CELLS 1000000
/* A whole-procedure semantic epoch can retain one sparse state entry for a
 * queried register at each join. Keep that cross product comfortably inside
 * the 256 MB Parse design target; larger procedures stay on local/CFG routes
 * until candidate-sliced SSA is implemented. */
#define FLOW_MAX_SEMANTIC_JOIN_REGISTER_CELLS 262144

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
    RxasFlowQueueBatch *queue_batch;
    flow_node *nodes;
    flow_register *registers;
    size_t register_count;
    size_t register_capacity;
    size_t word_count;
    size_t block_count;
    size_t resolved_indirect_branches;
    size_t private_locals_planned;
    int complete_control_flow;
} flow_graph;

typedef struct flow_stats {
    size_t procedures;
    size_t blocks;
    size_t unreachable_removed;
    size_t dead_results_removed;
    size_t typed_copies_removed;
    size_t local_single_use_copies_removed;
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
    size_t component_placement_queries;
    size_t component_placement_proved;
    size_t component_placement_rejected;
    size_t component_placement_unavailable;
    size_t component_placements_applied;
    size_t component_placement_trace_events_removed;
    size_t compare_branch_queries;
    size_t compare_branch_proved;
    size_t compare_branch_rejected;
    size_t compare_branches_fused;
    size_t compare_trace_events_removed;
    size_t joined_key_reuse_queries;
    size_t joined_key_reuse_proved;
    size_t joined_key_reuse_rejected;
    size_t joined_keys_reused;
    size_t joined_key_private_locals;
    size_t joined_key_trace_events_removed;
    size_t joined_key_preheader_eligible;
    size_t string_literal_reuse_queries;
    size_t string_literal_reuse_proved;
    size_t string_literal_reuse_rejected;
    size_t string_literal_reuse_unavailable;
    size_t string_literal_loads_reused;
    size_t string_literal_operands_redirected;
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
    size_t semantic_batches_applied;
    size_t semantic_batches_rejected;
    size_t semantic_records_changed;
    size_t semantic_records_deleted;
    size_t semantic_opcodes_replaced;
    size_t semantic_operand_records_rewritten;
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

static int flow_record_matches_epoch(const flow_graph *graph,
                                     size_t record_id) {
    if (!graph || record_id >= graph->item_count) return 0;
    if (!graph->queue_batch) return 1;
    return rxas_flow_queue_batch_record_matches_epoch(
            graph->queue_batch, record_id);
}

static instruction_queue *flow_edit_record(flow_graph *graph,
                                           size_t record_id) {
    const instruction_queue *epoch_item;
    instruction_queue *item;
    size_t entry;
    int snapshots_may_have_moved;
    if (!graph || record_id >= graph->item_count) return 0;
    if (!graph->queue_batch) return &graph->items[record_id];
    item = rxas_flow_queue_batch_edit(
            graph->queue_batch, record_id, &epoch_item,
            &snapshots_may_have_moved);
    if (!item) return 0;
    if (snapshots_may_have_moved) {
        for (entry = 0; entry < graph->queue_batch->entry_count; entry++) {
            RxasFlowQueueBatchEntry *batch_entry =
                    &graph->queue_batch->entries[entry];
            if (!rxas_flow_procedure_pin_queue_record(
                        graph->procedure, graph->epoch,
                        batch_entry->record_id,
                        &batch_entry->original))
                return 0;
        }
    }
    else if (!rxas_flow_procedure_pin_queue_record(
                 graph->procedure, graph->epoch, record_id, epoch_item)) {
        return 0;
    }
    return item;
}

static int flow_delete_record(flow_graph *graph, size_t record_id) {
    instruction_queue *item;
    item = flow_edit_record(graph, record_id);
    if (!item) return 0;
    item->instrType = EMPTY;
    return 1;
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
    char expected_value_type;
    if (!item || !deletion || item->instrType != TRACE_EVENT ||
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
    expected_value_type = deletion->component == RXOP_COMPONENT_INTEGER
            ? 'I' : deletion->component == RXOP_COMPONENT_FLOAT
                    ? 'F' : deletion->component == RXOP_COMPONENT_STRING
                            ? 'S' : deletion->component ==
                                        RXOP_COMPONENT_DECIMAL
                                    ? 'D' : deletion->component ==
                                                RXOP_COMPONENT_BINARY
                                            ? 'X' : deletion->component ==
                                                        RXOP_COMPONENT_REFERENCE
                                                    ? 'R' : '\0';
    if (!expected_value_type ||
        (value_type != expected_value_type &&
         !(deletion->component == RXOP_COMPONENT_INTEGER &&
           value_type == 'B')))
        return 0;
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
        !flow_record_matches_epoch(graph, copy_index) ||
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
    for (rewrite_index = 0; rewrite_index < plan.rewrite_count;
         rewrite_index++) {
        rxas_flow_typed_copy_plan_rewrite(
                proof, session->epoch, &plan, rewrite_index, &rewrite);
        if (!flow_edit_record(graph, rewrite.record_id)) {
            stats->rejected_effect++;
            return 0;
        }
    }
    copy = flow_edit_record(graph, copy_index);
    if (!copy) {
        stats->rejected_effect++;
        return 0;
    }
    compare_redirects = 0;
    for (rewrite_index = 0; rewrite_index < plan.rewrite_count;
         rewrite_index++) {
        rxas_flow_typed_copy_plan_rewrite(
                proof, session->epoch, &plan, rewrite_index, &rewrite);
        flow_set_operand(flow_edit_record(graph, rewrite.record_id),
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
            !flow_record_matches_epoch(graph, producer_index) ||
            !flow_record_matches_epoch(graph, producer_index + 1) ||
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
        producer = flow_edit_record(graph, producer_index);
        copy = flow_edit_record(graph, producer_index + 1);
        if (!producer || !copy) {
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

/* X01 places a one-register derivation on the copied source component.  The
 * immutable proof owns all storage, ValueId, observation and signal policy;
 * this consumer validates and applies only complete, disjoint plans. */
static size_t flow_place_copied_derivations(
        flow_graph *graph, flow_stats *stats,
        flow_proof_session *session) {
    const RxasFlowProofService *proof;
    const RxasFlowRecord *copy_record;
    const RxasFlowRecord *derivation_record;
    RxasFlowComponentPlacementPlan plan;
    RxasFlowOperandRewrite rewrite;
    RxasFlowTraceDeletion deletion;
    size_t copy_index;
    size_t derivation_index;
    size_t copy_instruction;
    size_t derivation_instruction;
    size_t rewrite_index;
    size_t deletion_index;
    size_t applied;
    instruction_queue *copy;
    instruction_queue *derivation;
    Assembler_Token *temporary;
    Assembler_Token *source;
    Assembler_Token *derivation_operand;
    int temporary_register;
    int source_register;
    int valid;
    unsigned char *claimed_registers;
    claimed_registers = calloc(
            graph->register_count ? graph->register_count : 1, 1);
    if (!claimed_registers)
        RX_PANIC_OOM("calloc RXAS component-placement batch",
                     graph->register_count, 0);
    applied = 0;
    for (copy_index = 0; copy_index + 1 < graph->item_count;
         copy_index++) {
        derivation_index = copy_index + 1;
        copy = &graph->items[copy_index];
        derivation = &graph->items[derivation_index];
        if (!graph->nodes[copy_index].reachable ||
            !graph->nodes[derivation_index].reachable ||
            copy->instrType != OP_CODE || derivation->instrType != OP_CODE ||
            !graph->nodes[copy_index].op ||
            !graph->nodes[derivation_index].op ||
            copy->operandCount != 2 || derivation->operandCount != 1 ||
            rxop_value_derivation(
                    graph->nodes[derivation_index].op->opcode) ==
                    RXOP_DERIVATION_NONE)
            continue;
        temporary = rxas_queue_operand(copy, 0);
        source = rxas_queue_operand(copy, 1);
        derivation_operand = rxas_queue_operand(derivation, 0);
        if (flow_register_type(temporary) != 'r' ||
            flow_register_type(source) != 'r' ||
            flow_register_type(derivation_operand) != 'r' ||
            temporary->token_value.integer < 0 ||
            source->token_value.integer < 0 ||
            derivation_operand->token_value.integer !=
                    temporary->token_value.integer)
            continue;
        temporary_register = flow_register_index(
                graph, 'r', (size_t)temporary->token_value.integer);
        source_register = flow_register_index(
                graph, 'r', (size_t)source->token_value.integer);
        if (temporary_register < 0 || source_register < 0 ||
            temporary_register == source_register ||
            claimed_registers[temporary_register] ||
            claimed_registers[source_register])
            continue;
        proof = flow_proof_session_require(
                session, graph, RXAS_PASS_X01_COMPONENT_PLACEMENT);
        copy_record = session->procedure ? rxas_flow_procedure_record(
                session->procedure, session->epoch, copy_index) : 0;
        derivation_record = session->procedure
                ? rxas_flow_procedure_record(
                        session->procedure, session->epoch,
                        derivation_index) : 0;
        copy_instruction = copy_record
                ? copy_record->instruction_id : RXAS_FLOW_ID_NONE;
        derivation_instruction = derivation_record
                ? derivation_record->instruction_id : RXAS_FLOW_ID_NONE;
        if (!proof || copy_instruction == RXAS_FLOW_ID_NONE ||
            derivation_instruction == RXAS_FLOW_ID_NONE ||
            !rxas_flow_prove_component_placement(
                    proof, session->epoch, copy_instruction,
                    derivation_instruction, &plan)) {
            stats->component_placement_unavailable++;
            stats->rejected_effect++;
            continue;
        }
        stats->component_placement_queries++;
        if (!plan.proved) {
            if (graph->context->debug_mode)
                fprintf(stderr,
                        "PERF3 component-placement-proof procedure=%s "
                        "copy=%llu:%s derivation=%llu:%s proved=0 "
                        "reason=%s rejected-kind=%d rejected-record=%llu "
                        "rejected-instruction=%llu rejected-operand=%llu "
                        "rejected-value=%llu\n",
                        graph->context->current_proc_name
                                ? graph->context->current_proc_name
                                : "(directives)",
                        (unsigned long long)copy_index,
                        graph->nodes[copy_index].op->mnemonic,
                        (unsigned long long)derivation_index,
                        graph->nodes[derivation_index].op->mnemonic,
                        rxas_flow_proof_reason_name(plan.reason),
                        (int)plan.rejected_use_kind,
                        (unsigned long long)plan.rejected_use_record_id,
                        (unsigned long long)
                                plan.rejected_use_instruction_id,
                        (unsigned long long)plan.rejected_use_operand_index,
                        (unsigned long long)plan.rejected_use_value_id);
            if (flow_proof_reason_unavailable(plan.reason)) {
                stats->component_placement_unavailable++;
                stats->rejected_effect++;
            }
            else {
                stats->component_placement_rejected++;
                if (plan.reason == RXAS_FLOW_PROOF_TRACE_OBSERVED)
                    stats->rejected_trace++;
                else stats->rejected_live++;
            }
            continue;
        }
        valid = plan.copy_record_id == copy_index &&
                plan.derivation_record_id == derivation_index &&
                plan.expected_copy_opcode ==
                        graph->nodes[copy_index].op->opcode &&
                plan.expected_derivation_opcode ==
                        graph->nodes[derivation_index].op->opcode &&
                flow_record_matches_epoch(graph, copy_index) &&
                flow_record_matches_epoch(graph, derivation_index) &&
                plan.derivation_rewrite.record_id == derivation_index &&
                plan.derivation_rewrite.instruction_id ==
                        derivation_instruction &&
                plan.derivation_rewrite.operand_index == 0 &&
                flow_operand_matches_proof_register(
                        temporary,
                        plan.derivation_rewrite.expected_register) &&
                flow_operand_matches_proof_register(
                        derivation_operand,
                        plan.derivation_rewrite.expected_register) &&
                flow_operand_matches_proof_register(
                        source,
                        plan.derivation_rewrite.replacement_register);
        for (rewrite_index = 0; valid &&
             rewrite_index < plan.rewrite_count; rewrite_index++) {
            instruction_queue *use_item;
            Assembler_Token *use_operand;
            if (!rxas_flow_component_placement_plan_rewrite(
                        proof, session->epoch, &plan,
                        rewrite_index, &rewrite) ||
                rewrite.record_id >= graph->item_count) {
                valid = 0;
                break;
            }
            use_item = &graph->items[rewrite.record_id];
            use_operand = rxas_queue_operand(
                    use_item, rewrite.operand_index);
            if (use_item->instrType != OP_CODE ||
                rewrite.operand_index >= use_item->operandCount ||
                !flow_operand_matches_proof_register(
                        use_operand, rewrite.expected_register) ||
                !flow_operand_matches_proof_register(
                        source, rewrite.replacement_register))
                valid = 0;
        }
        for (deletion_index = 0; valid &&
             deletion_index < plan.trace_deletion_count;
             deletion_index++) {
            if (!rxas_flow_component_placement_plan_trace_deletion(
                        proof, session->epoch, &plan,
                        deletion_index, &deletion) ||
                deletion.record_id >= graph->item_count ||
                deletion.value_id != plan.derivation_result_value_id ||
                !flow_record_matches_epoch(graph, deletion.record_id) ||
                !flow_trace_event_matches_deletion(
                        &graph->items[deletion.record_id], &deletion))
                valid = 0;
        }
        if (!valid) {
            stats->component_placement_unavailable++;
            stats->rejected_effect++;
            continue;
        }
        if (!flow_edit_record(graph, copy_index) ||
            !flow_edit_record(graph, derivation_index)) {
            stats->component_placement_unavailable++;
            stats->rejected_effect++;
            continue;
        }
        for (rewrite_index = 0; rewrite_index < plan.rewrite_count;
             rewrite_index++) {
            rxas_flow_component_placement_plan_rewrite(
                    proof, session->epoch, &plan,
                    rewrite_index, &rewrite);
            if (!flow_edit_record(graph, rewrite.record_id)) break;
        }
        if (rewrite_index != plan.rewrite_count) {
            stats->component_placement_unavailable++;
            stats->rejected_effect++;
            continue;
        }
        for (deletion_index = 0;
             deletion_index < plan.trace_deletion_count;
             deletion_index++) {
            rxas_flow_component_placement_plan_trace_deletion(
                    proof, session->epoch, &plan,
                    deletion_index, &deletion);
            if (!flow_edit_record(graph, deletion.record_id)) break;
        }
        if (deletion_index != plan.trace_deletion_count) {
            stats->component_placement_unavailable++;
            stats->rejected_effect++;
            continue;
        }
        copy = flow_edit_record(graph, copy_index);
        derivation = flow_edit_record(graph, derivation_index);
        flow_set_operand(derivation, 0, source);
        for (rewrite_index = 0; rewrite_index < plan.rewrite_count;
             rewrite_index++) {
            rxas_flow_component_placement_plan_rewrite(
                    proof, session->epoch, &plan,
                    rewrite_index, &rewrite);
            flow_set_operand(
                    flow_edit_record(graph, rewrite.record_id),
                    rewrite.operand_index, source);
        }
        for (deletion_index = 0;
             deletion_index < plan.trace_deletion_count;
             deletion_index++) {
            rxas_flow_component_placement_plan_trace_deletion(
                    proof, session->epoch, &plan,
                    deletion_index, &deletion);
            flow_delete_record(graph, deletion.record_id);
        }
        copy->instrType = EMPTY;
        claimed_registers[temporary_register] = 1;
        claimed_registers[source_register] = 1;
        stats->component_placement_proved++;
        stats->component_placements_applied++;
        stats->component_placement_trace_events_removed +=
                plan.trace_deletion_count;
        stats->typed_copies_removed++;
        stats->operands_redirected += plan.rewrite_count + 1;
        flow_debug_accept(
                graph, derivation_index,
                "component-placement-ssa",
                plan.rewrite_count + plan.trace_deletion_count + 1);
        applied++;
    }
    free(claimed_registers);
    return applied;
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
 * two exact triples, storage/path identity, component equivalence and
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
                !flow_record_matches_epoch(
                        graph, plan.first_link_record_id) ||
                !flow_record_matches_epoch(
                        graph, plan.first_copy_record_id) ||
                !flow_record_matches_epoch(
                        graph, plan.first_unlink_record_id) ||
                !flow_record_matches_epoch(
                        graph, plan.second_link_record_id) ||
                !flow_record_matches_epoch(
                        graph, plan.second_copy_record_id) ||
                !flow_record_matches_epoch(
                        graph, plan.second_unlink_record_id) ||
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
            second_link = flow_edit_record(
                    graph, plan.second_link_record_id);
            second_copy = flow_edit_record(
                    graph, plan.second_copy_record_id);
            second_unlink = flow_edit_record(
                    graph, plan.second_unlink_record_id);
            if (!second_link || !second_copy || !second_unlink) {
                stats->rejected_effect++;
                continue;
            }
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
    size_t source_index;
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
                        "reason=%s component=0x%x kind=%d presence=%d "
                        "value=%llu leaf=%llu leaf-kind=%d "
                        "leaf-presence=%d leaf-def=%llu use-kind=%d "
                        "use-record=%llu use-instruction=%llu "
                        "use-operand=%llu use-value=%llu\n",
                        graph->context->current_proc_name
                                ? graph->context->current_proc_name
                                : "(directives)",
                        (unsigned long long)compare_index,
                        graph->nodes[compare_index].op->mnemonic,
                        (unsigned long long)branch_index,
                        graph->nodes[branch_index].op->mnemonic,
                        rxas_flow_proof_reason_name(plan.reason),
                        plan.rejected_component,
                        (int)plan.rejected_component_kind,
                        (int)plan.rejected_component_presence,
                        (unsigned long long)
                                plan.rejected_component_value_id,
                        (unsigned long long)plan.rejected_leaf_value_id,
                        (int)plan.rejected_leaf_kind,
                        (int)plan.rejected_leaf_presence,
                        (unsigned long long)
                                plan.rejected_leaf_defining_instruction,
                        (int)plan.rejected_use_kind,
                        (unsigned long long)plan.rejected_use_record_id,
                        (unsigned long long)
                                plan.rejected_use_instruction_id,
                        (unsigned long long)plan.rejected_use_operand_index,
                        (unsigned long long)plan.rejected_use_value_id);
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
            !flow_record_matches_epoch(graph, compare_index) ||
            !flow_record_matches_epoch(graph, branch_index) ||
            plan.expected_compare_opcode !=
                    graph->nodes[compare_index].op->opcode ||
            plan.expected_branch_opcode !=
                    graph->nodes[branch_index].op->opcode ||
            plan.fused_opcode < 0 ||
            plan.fused_opcode >= OP_MAX_INSTRUCTIONS ||
            plan.left_source_operand >= compare->operandCount ||
            plan.right_source_operand >= compare->operandCount ||
            (plan.result_source_operands & ~3u) != 0 ||
            !flow_operand_matches_proof_register(
                    result, plan.result_register) ||
            !flow_operand_matches_proof_register(
                    branch_result, plan.result_register) ||
            branch->operandCount != 2 ||
            !rxas_queue_operand(branch, 0)) {
            stats->rejected_effect++;
            continue;
        }
        for (source_index = 0; source_index < 2; source_index++) {
            size_t source_operand;
            if (!(plan.result_source_operands & (1u << source_index)))
                continue;
            source_operand = source_index ? plan.right_source_operand
                                          : plan.left_source_operand;
            if (plan.result_source_value_ids[source_index] ==
                        RXAS_FLOW_ID_NONE ||
                source_operand >= compare->operandCount ||
                !flow_operand_matches_proof_register(
                        rxas_queue_operand(compare, source_operand),
                        plan.result_register))
                break;
        }
        if (source_index != 2) {
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
                !flow_record_matches_epoch(
                        graph, trace_deletion.record_id) ||
                !flow_trace_event_matches_deletion(
                        &graph->items[trace_deletion.record_id],
                        &trace_deletion)) {
                stats->rejected_effect++;
                break;
            }
        }
        if (trace_deletion_index != plan.trace_deletion_count) continue;
        compare = flow_edit_record(graph, compare_index);
        branch = flow_edit_record(graph, branch_index);
        if (!compare || !branch) {
            stats->rejected_effect++;
            continue;
        }
        for (trace_deletion_index = 0;
             trace_deletion_index < plan.trace_deletion_count;
             trace_deletion_index++) {
            rxas_flow_compare_branch_plan_trace_deletion(
                    proof, session->epoch, &plan,
                    trace_deletion_index, &trace_deletion);
            if (!flow_edit_record(graph, trace_deletion.record_id)) {
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
            if (!flow_delete_record(graph, trace_deletion.record_id))
                break;
        }
        if (trace_deletion_index != plan.trace_deletion_count) continue;
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

typedef struct flow_joined_key_selection {
    RxasFlowJoinedKeyReusePlan plan;
    size_t seed_index;
    size_t candidate_index;
    size_t stem_index;
    size_t group_index;
} flow_joined_key_selection;

typedef struct flow_joined_key_group {
    size_t seed_index;
    size_t representative_selection;
    Assembler_Token *cache_token;
    Assembler_Token *trace_number_token;
} flow_joined_key_group;

static Assembler_Token *flow_new_numbered_token(
        flow_graph *graph, const Assembler_Token *from,
        int token_type, char token_subtype, size_t number) {
    Assembler_Token *token;
    if (!graph || !graph->context || number > (size_t)RXINTEGER_MAX)
        return 0;
    token = rxas_tid(
            graph->context, (Assembler_Token *)from, (char *)"0");
    if (!token) return 0;
    token->token_type = token_type;
    token->token_subtype = token_subtype;
    token->token_value.integer = (rxinteger)number;
    token->optimised = 1;
    return token;
}

static int flow_joined_seed_rewrite_matches(
        const flow_graph *graph, const RxasFlowProofService *proof,
        unsigned long epoch, const RxasFlowJoinedKeyReusePlan *plan,
        size_t rewrite_index, RxasFlowOperandRewrite *rewrite) {
    const RxasFlowRecord *record;
    const instruction_queue *item;
    RxasFlowTraceDeletion trace;
    if (!rxas_flow_joined_key_reuse_plan_seed_rewrite(
                proof, epoch, plan, rewrite_index, rewrite) ||
        rewrite->record_id >= graph->item_count ||
        !flow_record_matches_epoch(graph, rewrite->record_id) ||
        rewrite->expected_register.register_class !=
                plan->cache_register.register_class ||
        rewrite->expected_register.number != plan->cache_register.number ||
        rewrite->replacement_register.register_class !=
                plan->cache_register.register_class ||
        rewrite->replacement_register.number != plan->cache_register.number)
        return 0;
    record = rxas_flow_procedure_record(
            graph->procedure, epoch, rewrite->record_id);
    item = &graph->items[rewrite->record_id];
    if (!record) return 0;
    if (rewrite->instruction_id == RXAS_FLOW_ID_NONE &&
        rewrite->operand_index == RXAS_FLOW_ID_NONE) {
        memset(&trace, 0, sizeof(trace));
        trace.record_id = rewrite->record_id;
        trace.value_id = plan->cache_value_id;
        trace.component = RXOP_COMPONENT_STRING;
        trace.expected_register = rewrite->expected_register;
        return record->instruction_id == RXAS_FLOW_ID_NONE &&
               flow_trace_event_matches_deletion(item, &trace);
    }
    return record->instruction_id == rewrite->instruction_id &&
           item->instrType == OP_CODE &&
           rewrite->operand_index < item->operandCount &&
           flow_operand_matches_proof_register(
                   rxas_queue_operand(item, rewrite->operand_index),
                   rewrite->expected_register);
}

static int flow_joined_seed_plans_equal(
        const RxasFlowProofService *proof, unsigned long epoch,
        const RxasFlowJoinedKeyReusePlan *left,
        const RxasFlowJoinedKeyReusePlan *right) {
    size_t index;
    if (!left || !right ||
        left->seed_record_id != right->seed_record_id ||
        left->cache_register.register_class !=
                right->cache_register.register_class ||
        left->cache_register.number != right->cache_register.number ||
        left->cache_storage_root != right->cache_storage_root ||
        left->cache_value_id != right->cache_value_id ||
        left->seed_rewrite_count != right->seed_rewrite_count)
        return 0;
    for (index = 0; index < left->seed_rewrite_count; index++) {
        RxasFlowOperandRewrite left_rewrite;
        RxasFlowOperandRewrite right_rewrite;
        if (!rxas_flow_joined_key_reuse_plan_seed_rewrite(
                    proof, epoch, left, index, &left_rewrite) ||
            !rxas_flow_joined_key_reuse_plan_seed_rewrite(
                    proof, epoch, right, index, &right_rewrite) ||
            left_rewrite.record_id != right_rewrite.record_id ||
            left_rewrite.instruction_id != right_rewrite.instruction_id ||
            left_rewrite.operand_index != right_rewrite.operand_index ||
            left_rewrite.expected_register.register_class !=
                    right_rewrite.expected_register.register_class ||
            left_rewrite.expected_register.number !=
                    right_rewrite.expected_register.number)
            return 0;
    }
    return 1;
}

static size_t flow_joined_group_find(
        const flow_joined_key_group *groups, size_t group_count,
        size_t seed_index) {
    size_t index;
    for (index = 0; index < group_count; index++)
        if (groups[index].seed_index == seed_index) return index;
    return RXAS_FLOW_ID_NONE;
}

/* H01 keeps the first materialisation at its original lazy program point and
 * redirects its complete string-value use set into one fresh RXAS-private local.
 * Later equivalent joined keys may reuse that local only through the immutable
 * loop/value/use proof. No preheader movement is performed by this route. */
static size_t flow_reuse_joined_keys(
        flow_graph *graph, flow_stats *stats,
        flow_proof_session *session) {
    const RxasFlowProofService *proof;
    flow_joined_key_selection *selections;
    flow_joined_key_group *groups;
    unsigned char *claimed;
    size_t selection_count;
    size_t group_count;
    size_t candidate_index;
    size_t index;

    proof = flow_proof_session_require(
            session, graph, RXAS_PASS_H01_JOINED_KEY_REUSE);
    if (!proof) {
        stats->rejected_effect++;
        return 0;
    }
    selections = calloc(
            graph->item_count ? graph->item_count : 1,
            sizeof(*selections));
    groups = calloc(
            graph->item_count ? graph->item_count : 1, sizeof(*groups));
    claimed = calloc(graph->item_count ? graph->item_count : 1, 1);
    if (!selections || !groups || !claimed)
        RX_PANIC_OOM(
                "calloc RXAS joined-key production plan",
                graph->item_count *
                        (sizeof(*selections) + sizeof(*groups) + 1),
                graph->context->file_name);
    selection_count = 0;
    group_count = 0;

    for (candidate_index = 0; candidate_index < graph->item_count;
         candidate_index++) {
        instruction_queue *candidate;
        Assembler_Token *candidate_result;
        size_t stem_index;
        size_t seed_index;
        int selected_candidate;

        candidate = &graph->items[candidate_index];
        if (!flow_record_matches_epoch(graph, candidate_index) ||
            !graph->nodes[candidate_index].reachable ||
            candidate->instrType != OP_CODE ||
            !graph->nodes[candidate_index].op ||
            graph->nodes[candidate_index].op->opcode !=
                    OP_CONCAT_REG_STRING_REG ||
            candidate->operandCount != 3)
            continue;
        candidate_result = rxas_queue_operand(candidate, 0);
        if (flow_register_type(candidate_result) != 'r') continue;
        stem_index = RXAS_FLOW_ID_NONE;
        for (index = candidate_index + 1;
             index < graph->item_count; index++) {
            instruction_queue *possible_stem;
            size_t key_operand;
            possible_stem = &graph->items[index];
            if (!flow_record_matches_epoch(graph, index) ||
                possible_stem->instrType != OP_CODE ||
                !graph->nodes[index].op ||
                (graph->nodes[index].op->opcode !=
                        OP_STEMGET_REG_REG_REG &&
                 graph->nodes[index].op->opcode !=
                        OP_STEMSET_REG_REG_REG) ||
                possible_stem->operandCount != 3)
                continue;
            key_operand = graph->nodes[index].op->opcode ==
                    OP_STEMGET_REG_REG_REG ? 2 : 1;
            if (flow_operand_matches_proof_register(
                        rxas_queue_operand(possible_stem, key_operand),
                        (RxasFlowRegister){
                            RXAS_FLOW_REGISTER_LOCAL,
                            (size_t)candidate_result->token_value.integer})) {
                stem_index = index;
                break;
            }
        }
        if (stem_index == RXAS_FLOW_ID_NONE) continue;

        selected_candidate = 0;
        seed_index = candidate_index;
        while (!selected_candidate && seed_index) {
            const RxasFlowRecord *seed_record;
            const RxasFlowRecord *candidate_record;
            const RxasFlowRecord *stem_record;
            RxasFlowJoinedKeyReusePlan plan;
            instruction_queue *seed;
            Assembler_Token *candidate_literal;
            Assembler_Token *seed_literal;
            size_t key_operand;
            size_t candidate_deletion;
            size_t group_index;
            int new_group;
            int valid;

            seed_index--;
            seed = &graph->items[seed_index];
            if (!flow_record_matches_epoch(graph, seed_index) ||
                !graph->nodes[seed_index].reachable ||
                seed->instrType != OP_CODE ||
                !graph->nodes[seed_index].op ||
                graph->nodes[seed_index].op->opcode !=
                        OP_CONCAT_REG_STRING_REG ||
                seed->operandCount != 3)
                continue;
            group_index = flow_joined_group_find(
                    groups, group_count, seed_index);
            if (claimed[seed_index] && group_index == RXAS_FLOW_ID_NONE)
                continue;
            candidate_literal = rxas_queue_operand(candidate, 1);
            seed_literal = rxas_queue_operand(seed, 1);
            if (!candidate_literal || !seed_literal ||
                candidate_literal->token_type != STRING ||
                seed_literal->token_type != STRING ||
                strcmp((const char *)candidate_literal->token_value.string,
                       (const char *)seed_literal->token_value.string) != 0)
                continue;

            seed_record = rxas_flow_procedure_record(
                    session->procedure, session->epoch, seed_index);
            candidate_record = rxas_flow_procedure_record(
                    session->procedure, session->epoch, candidate_index);
            stem_record = rxas_flow_procedure_record(
                    session->procedure, session->epoch, stem_index);
            if (!seed_record || !candidate_record || !stem_record ||
                seed_record->instruction_id == RXAS_FLOW_ID_NONE ||
                candidate_record->instruction_id == RXAS_FLOW_ID_NONE ||
                stem_record->instruction_id == RXAS_FLOW_ID_NONE)
                continue;

            memset(&plan, 0, sizeof(plan));
            stats->joined_key_reuse_queries++;
            if (!rxas_flow_prove_joined_key_reuse(
                        proof, session->epoch,
                        seed_record->instruction_id,
                        candidate_record->instruction_id,
                        stem_record->instruction_id, &plan)) {
                stats->rejected_effect++;
                continue;
            }
            if (!plan.proved) {
                stats->joined_key_reuse_rejected++;
                if (graph->context->debug_mode)
                    fprintf(stderr,
                            "PERF3 joined-key-reuse-proof procedure=%s "
                            "seed=%llu candidate=%llu stem=%llu proved=0 "
                            "reason=%s rejected-kind=%d "
                            "rejected-record=%llu rejected-instruction=%llu "
                            "rejected-operand=%llu rejected-value=%llu\n",
                            graph->context->current_proc_name
                                    ? graph->context->current_proc_name
                                    : "(directives)",
                            (unsigned long long)seed_index,
                            (unsigned long long)candidate_index,
                            (unsigned long long)stem_index,
                            rxas_flow_proof_reason_name(plan.reason),
                            (int)plan.rejected_use_kind,
                            (unsigned long long)
                                    plan.rejected_use_record_id,
                            (unsigned long long)
                                    plan.rejected_use_instruction_id,
                            (unsigned long long)
                                    plan.rejected_use_operand_index,
                            (unsigned long long)
                                    plan.rejected_use_value_id);
                continue;
            }

            key_operand = graph->nodes[stem_index].op->opcode ==
                    OP_STEMGET_REG_REG_REG ? 2 : 1;
            valid = plan.seed_record_id == seed_index &&
                    plan.candidate_record_id == candidate_index &&
                    plan.stem_record_id == stem_index &&
                    plan.stem_key_operand == key_operand &&
                    plan.expected_concat_opcode ==
                            graph->nodes[candidate_index].op->opcode &&
                    plan.expected_stem_opcode ==
                            graph->nodes[stem_index].op->opcode &&
                    flow_operand_matches_proof_register(
                            rxas_queue_operand(seed, 0),
                            plan.cache_register) &&
                    flow_operand_matches_proof_register(
                            candidate_result, plan.candidate_register) &&
                    flow_operand_matches_proof_register(
                            rxas_queue_operand(
                                    &graph->items[stem_index], key_operand),
                            plan.candidate_register) &&
                    !claimed[candidate_index] && !claimed[stem_index];
            new_group = group_index == RXAS_FLOW_ID_NONE;
            if (valid && new_group) {
                size_t rewrite_index;
                if (claimed[seed_index]) valid = 0;
                for (rewrite_index = 0;
                     valid && rewrite_index < plan.seed_rewrite_count;
                     rewrite_index++) {
                    RxasFlowOperandRewrite rewrite;
                    valid = flow_joined_seed_rewrite_matches(
                            graph, proof, session->epoch, &plan,
                            rewrite_index, &rewrite) &&
                            rewrite.record_id != seed_index &&
                            !claimed[rewrite.record_id];
                }
            }
            else if (valid) {
                valid = flow_joined_seed_plans_equal(
                        proof, session->epoch,
                        &selections[
                                groups[group_index].
                                        representative_selection].plan,
                        &plan);
            }

            for (candidate_deletion = 0;
                 valid &&
                 candidate_deletion < plan.trace_deletion_count;
                 candidate_deletion++) {
                RxasFlowTraceDeletion deletion;
                size_t earlier;
                valid = rxas_flow_joined_key_reuse_plan_trace_deletion(
                            proof, session->epoch, &plan,
                            candidate_deletion, &deletion) &&
                        deletion.record_id < graph->item_count &&
                        deletion.value_id == plan.candidate_value_id &&
                        !claimed[deletion.record_id] &&
                        flow_record_matches_epoch(
                                graph, deletion.record_id) &&
                        flow_trace_event_matches_deletion(
                                &graph->items[deletion.record_id],
                                &deletion);
                for (earlier = 0; valid && earlier < candidate_deletion;
                     earlier++) {
                    RxasFlowTraceDeletion prior;
                    valid = rxas_flow_joined_key_reuse_plan_trace_deletion(
                                    proof, session->epoch, &plan,
                                    earlier, &prior) &&
                            prior.record_id != deletion.record_id;
                }
            }
            if (!valid) {
                stats->rejected_effect++;
                continue;
            }

            if (new_group) {
                size_t rewrite_index;
                group_index = group_count++;
                groups[group_index].seed_index = seed_index;
                groups[group_index].representative_selection =
                        selection_count;
                claimed[seed_index] = 1;
                for (rewrite_index = 0;
                     rewrite_index < plan.seed_rewrite_count;
                     rewrite_index++) {
                    RxasFlowOperandRewrite rewrite;
                    rxas_flow_joined_key_reuse_plan_seed_rewrite(
                            proof, session->epoch, &plan,
                            rewrite_index, &rewrite);
                    claimed[rewrite.record_id] = 1;
                }
            }
            claimed[candidate_index] = 1;
            claimed[stem_index] = 1;
            for (candidate_deletion = 0;
                 candidate_deletion < plan.trace_deletion_count;
                 candidate_deletion++) {
                RxasFlowTraceDeletion deletion;
                rxas_flow_joined_key_reuse_plan_trace_deletion(
                        proof, session->epoch, &plan,
                        candidate_deletion, &deletion);
                claimed[deletion.record_id] = 1;
            }
            selections[selection_count].plan = plan;
            selections[selection_count].seed_index = seed_index;
            selections[selection_count].candidate_index = candidate_index;
            selections[selection_count].stem_index = stem_index;
            selections[selection_count].group_index = group_index;
            selection_count++;
            selected_candidate = 1;
        }
    }

    if (!selection_count || graph->context->current_locals < 0 ||
        group_count > (size_t)(INT_MAX - graph->context->current_locals)) {
        if (selection_count) stats->rejected_effect += selection_count;
        free(claimed);
        free(groups);
        free(selections);
        return 0;
    }

    /* Pin every touched epoch record before the first mutation. Unexpected
     * pinning failure therefore leaves the semantic batch fully rollbackable. */
    for (index = 0; index < graph->item_count; index++) {
        if (claimed[index] && !flow_edit_record(graph, index)) {
            stats->rejected_effect += selection_count;
            free(claimed);
            free(groups);
            free(selections);
            return 0;
        }
    }

    for (index = 0; index < group_count; index++) {
        flow_joined_key_selection *representative;
        instruction_queue *seed;
        size_t local_number;
        size_t rewrite_index;
        representative =
                &selections[groups[index].representative_selection];
        seed = &graph->items[groups[index].seed_index];
        local_number = (size_t)graph->context->current_locals + index;
        groups[index].cache_token = flow_new_numbered_token(
                graph, rxas_queue_operand(seed, 0),
                RREG, 'r', local_number);
        groups[index].trace_number_token = flow_new_numbered_token(
                graph, rxas_queue_operand(seed, 0),
                INT, 0, local_number);
        if (!groups[index].cache_token ||
            !groups[index].trace_number_token)
            RX_PANIC_OOM(
                    "create RXAS joined-key private-local tokens",
                    sizeof(Assembler_Token) * 2,
                    graph->context->file_name);
        flow_set_operand(seed, 0, groups[index].cache_token);
        for (rewrite_index = 0;
             rewrite_index <
                    representative->plan.seed_rewrite_count;
             rewrite_index++) {
            RxasFlowOperandRewrite rewrite;
            rxas_flow_joined_key_reuse_plan_seed_rewrite(
                    proof, session->epoch, &representative->plan,
                    rewrite_index, &rewrite);
            if (rewrite.instruction_id == RXAS_FLOW_ID_NONE &&
                rewrite.operand_index == RXAS_FLOW_ID_NONE)
                graph->items[rewrite.record_id].operand5Token =
                        groups[index].trace_number_token;
            else
                flow_set_operand(
                        &graph->items[rewrite.record_id],
                        rewrite.operand_index,
                        groups[index].cache_token);
        }
    }

    for (index = 0; index < selection_count; index++) {
        flow_joined_key_selection *selection;
        flow_joined_key_group *group;
        size_t deletion_index;
        selection = &selections[index];
        group = &groups[selection->group_index];
        flow_set_operand(
                &graph->items[selection->stem_index],
                selection->plan.stem_key_operand,
                group->cache_token);
        graph->items[selection->candidate_index].instrType = EMPTY;
        for (deletion_index = 0;
             deletion_index < selection->plan.trace_deletion_count;
             deletion_index++) {
            RxasFlowTraceDeletion deletion;
            rxas_flow_joined_key_reuse_plan_trace_deletion(
                    proof, session->epoch, &selection->plan,
                    deletion_index, &deletion);
            graph->items[deletion.record_id].instrType = EMPTY;
        }
        stats->joined_key_reuse_proved++;
        stats->joined_keys_reused++;
        stats->joined_key_trace_events_removed +=
                selection->plan.trace_deletion_count;
        stats->joined_key_preheader_eligible +=
                selection->plan.preheader_eligible ? 1 : 0;
        if (graph->context->debug_mode)
            fprintf(stderr,
                    "PERF3 joined-key-reuse-select procedure=%s "
                    "seed=%llu candidate=%llu stem=%llu loop=%llu "
                    "cache=r%llu candidate=r%llu seed-redirect=%llu "
                    "trace-delete=%llu preheader=spec:%d,must:%d(%s),"
                    "invariant:%d(%s),trace-free:%d,eligible:%d\n",
                    graph->context->current_proc_name
                            ? graph->context->current_proc_name
                            : "(directives)",
                    (unsigned long long)selection->seed_index,
                    (unsigned long long)selection->candidate_index,
                    (unsigned long long)selection->stem_index,
                    (unsigned long long)selection->plan.loop_id,
                    (unsigned long long)
                            group->cache_token->token_value.integer,
                    (unsigned long long)
                            selection->plan.candidate_register.number,
                    (unsigned long long)
                            selection->plan.seed_rewrite_count,
                    (unsigned long long)
                            selection->plan.trace_deletion_count,
                    selection->plan.preheader_speculatable,
                    selection->plan.preheader_must_execute,
                    rxas_flow_proof_reason_name(
                            selection->plan.preheader_must_execute_reason),
                    selection->plan.preheader_right_invariant,
                    rxas_flow_proof_reason_name(
                            selection->plan.preheader_invariant_reason),
                    selection->plan.preheader_trace_free,
                    selection->plan.preheader_eligible);
        flow_debug_accept(
                graph, selection->candidate_index,
                "joined-key-loop-reuse-private-local",
                selection->plan.trace_deletion_count + 1);
    }
    graph->private_locals_planned += group_count;
    stats->joined_key_private_locals += group_count;

    free(claimed);
    free(groups);
    free(selections);
    return selection_count;
}

typedef struct flow_string_literal_selection {
    RxasFlowStringLiteralReusePlan plan;
    size_t seed_index;
    size_t candidate_index;
} flow_string_literal_selection;

static int flow_string_literal_tokens_equal(
        const Assembler_Token *left, const Assembler_Token *right) {
    size_t left_length;
    size_t right_length;
    if (!left || !right || left->token_type != STRING ||
        right->token_type != STRING || left->length < 2 || right->length < 2)
        return 0;
    left_length = left->length - 2;
    right_length = right->length - 2;
    return left_length == right_length &&
           memcmp(left->token_value.string,
                  right->token_value.string, left_length) == 0;
}

static int flow_is_string_literal_load(const flow_graph *graph,
                                       size_t record_id) {
    const instruction_queue *item;
    if (!graph || record_id >= graph->item_count ||
        !flow_record_matches_epoch(graph, record_id) ||
        !graph->nodes[record_id].reachable ||
        !graph->nodes[record_id].op ||
        graph->nodes[record_id].op->opcode != OP_LOAD_REG_STRING)
        return 0;
    item = &graph->items[record_id];
    return item->instrType == OP_CODE && item->operandCount == 2 &&
           flow_register_type(rxas_queue_operand(item, 0)) == 'r' &&
           rxas_queue_operand(item, 1) &&
           rxas_queue_operand(item, 1)->token_type == STRING;
}

static int flow_string_literal_rewrite_matches(
        const flow_graph *graph, const RxasFlowProofService *proof,
        unsigned long epoch, const RxasFlowStringLiteralReusePlan *plan,
        size_t rewrite_index, RxasFlowOperandRewrite *rewrite) {
    const RxasFlowRecord *record;
    const instruction_queue *item;
    RxasFlowTraceDeletion trace;
    if (!rxas_flow_string_literal_reuse_plan_rewrite(
                proof, epoch, plan, rewrite_index, rewrite) ||
        rewrite->record_id >= graph->item_count ||
        !flow_record_matches_epoch(graph, rewrite->record_id) ||
        rewrite->expected_register.register_class !=
                plan->candidate_register.register_class ||
        rewrite->expected_register.number !=
                plan->candidate_register.number ||
        rewrite->replacement_register.register_class !=
                plan->seed_register.register_class ||
        rewrite->replacement_register.number != plan->seed_register.number)
        return 0;
    record = rxas_flow_procedure_record(
            graph->procedure, epoch, rewrite->record_id);
    item = &graph->items[rewrite->record_id];
    if (!record) return 0;
    if (rewrite->instruction_id == RXAS_FLOW_ID_NONE &&
        rewrite->operand_index == RXAS_FLOW_ID_NONE) {
        memset(&trace, 0, sizeof(trace));
        trace.record_id = rewrite->record_id;
        trace.value_id = plan->candidate_value_id;
        trace.component = RXOP_COMPONENT_STRING;
        trace.expected_register = rewrite->expected_register;
        return record->instruction_id == RXAS_FLOW_ID_NONE &&
               flow_trace_event_matches_deletion(item, &trace);
    }
    return record->instruction_id == rewrite->instruction_id &&
           item->instrType == OP_CODE &&
           rewrite->operand_index < item->operandCount &&
           flow_operand_matches_proof_register(
                   rxas_queue_operand(item, rewrite->operand_index),
                   rewrite->expected_register);
}

/* H02 reuses only an already-executed string literal materialisation. The
 * immutable proof owns dominance, loop membership, private storage, cleanup
 * absence and the complete redirectable use set. No instruction is moved. */
static size_t flow_reuse_string_literals(
        flow_graph *graph, flow_stats *stats,
        flow_proof_session *session) {
    const RxasFlowProofService *proof;
    flow_string_literal_selection *selections;
    unsigned char *claimed;
    size_t selection_count;
    size_t candidate_index;
    size_t index;

    proof = flow_proof_session_require(
            session, graph, RXAS_PASS_H02_STRING_LITERAL_REUSE);
    if (!proof) {
        stats->string_literal_reuse_unavailable++;
        return 0;
    }
    selections = calloc(
            graph->item_count ? graph->item_count : 1,
            sizeof(*selections));
    claimed = calloc(graph->item_count ? graph->item_count : 1, 1);
    if (!selections || !claimed)
        RX_PANIC_OOM(
                "calloc RXAS string-literal reuse plan",
                graph->item_count * (sizeof(*selections) + 1),
                graph->context->file_name);
    selection_count = 0;

    for (candidate_index = 0; candidate_index < graph->item_count;
         candidate_index++) {
        const instruction_queue *candidate;
        size_t seed_index;
        int selected;
        if (!flow_is_string_literal_load(graph, candidate_index) ||
            claimed[candidate_index])
            continue;
        candidate = &graph->items[candidate_index];
        seed_index = candidate_index;
        selected = 0;
        while (!selected && seed_index) {
            const instruction_queue *seed;
            const RxasFlowRecord *seed_record;
            const RxasFlowRecord *candidate_record;
            RxasFlowStringLiteralReusePlan plan;
            size_t rewrite_index;
            int valid;
            seed_index--;
            if (!flow_is_string_literal_load(graph, seed_index) ||
                claimed[seed_index])
                continue;
            seed = &graph->items[seed_index];
            if (!flow_string_literal_tokens_equal(
                        rxas_queue_operand(seed, 1),
                        rxas_queue_operand(candidate, 1)))
                continue;
            seed_record = rxas_flow_procedure_record(
                    session->procedure, session->epoch, seed_index);
            candidate_record = rxas_flow_procedure_record(
                    session->procedure, session->epoch, candidate_index);
            if (!seed_record || !candidate_record ||
                seed_record->instruction_id == RXAS_FLOW_ID_NONE ||
                candidate_record->instruction_id == RXAS_FLOW_ID_NONE)
                continue;
            memset(&plan, 0, sizeof(plan));
            stats->string_literal_reuse_queries++;
            if (!rxas_flow_prove_string_literal_reuse(
                        proof, session->epoch,
                        seed_record->instruction_id,
                        candidate_record->instruction_id, &plan)) {
                stats->string_literal_reuse_unavailable++;
                continue;
            }
            if (!plan.proved) {
                stats->string_literal_reuse_rejected++;
                if (graph->context->debug_mode)
                    fprintf(stderr,
                            "PERF3 string-literal-reuse-proof procedure=%s "
                            "seed=%llu candidate=%llu proved=0 reason=%s "
                            "rejected-kind=%d rejected-record=%llu "
                            "rejected-instruction=%llu rejected-operand=%llu "
                            "rejected-value=%llu seed-value=%llu "
                            "at-candidate=%llu kind=%d presence=%d def=%llu "
                            "cleanup-component=0x%x cleanup-value=%llu "
                            "cleanup-kind=%d cleanup-presence=%d\n",
                            graph->context->current_proc_name
                                    ? graph->context->current_proc_name
                                    : "(directives)",
                            (unsigned long long)seed_index,
                            (unsigned long long)candidate_index,
                            rxas_flow_proof_reason_name(plan.reason),
                            (int)plan.rejected_use_kind,
                            (unsigned long long)plan.rejected_use_record_id,
                            (unsigned long long)
                                    plan.rejected_use_instruction_id,
                            (unsigned long long)
                                    plan.rejected_use_operand_index,
                            (unsigned long long)plan.rejected_use_value_id,
                            (unsigned long long)plan.seed_value_id,
                            (unsigned long long)
                                    plan.seed_candidate_value_id,
                            (int)plan.seed_candidate_kind,
                            (int)plan.seed_candidate_presence,
                            (unsigned long long)
                                    plan.seed_candidate_defining_instruction,
                            plan.rejected_cleanup_component,
                            (unsigned long long)
                                    plan.rejected_cleanup_value_id,
                            (int)plan.rejected_cleanup_kind,
                            (int)plan.rejected_cleanup_presence);
                continue;
            }
            valid = plan.seed_record_id == seed_index &&
                    plan.candidate_record_id == candidate_index &&
                    plan.expected_opcode == OP_LOAD_REG_STRING &&
                    (!plan.preserve_candidate_register ||
                     !plan.rewrite_count) &&
                    flow_operand_matches_proof_register(
                            rxas_queue_operand(seed, 0),
                            plan.seed_register) &&
                    flow_operand_matches_proof_register(
                            rxas_queue_operand(candidate, 0),
                            plan.candidate_register) &&
                    !claimed[candidate_index];
            for (rewrite_index = 0;
                 valid && rewrite_index < plan.rewrite_count;
                 rewrite_index++) {
                RxasFlowOperandRewrite rewrite;
                valid = flow_string_literal_rewrite_matches(
                                graph, proof, session->epoch, &plan,
                                rewrite_index, &rewrite) &&
                        rewrite.record_id != candidate_index &&
                        !claimed[rewrite.record_id];
            }
            if (!valid) {
                stats->string_literal_reuse_rejected++;
                continue;
            }
            claimed[candidate_index] = 1;
            for (rewrite_index = 0; rewrite_index < plan.rewrite_count;
                 rewrite_index++) {
                RxasFlowOperandRewrite rewrite;
                rxas_flow_string_literal_reuse_plan_rewrite(
                        proof, session->epoch, &plan,
                        rewrite_index, &rewrite);
                claimed[rewrite.record_id] = 1;
            }
            selections[selection_count].plan = plan;
            selections[selection_count].seed_index = seed_index;
            selections[selection_count].candidate_index = candidate_index;
            selection_count++;
            selected = 1;
        }
    }

    for (index = 0; index < graph->item_count; index++) {
        if (claimed[index] && !flow_edit_record(graph, index)) {
            stats->string_literal_reuse_rejected += selection_count;
            free(claimed);
            free(selections);
            return 0;
        }
    }
    for (index = 0; index < selection_count; index++) {
        flow_string_literal_selection *selection;
        Assembler_Token *seed_token;
        size_t rewrite_index;
        selection = &selections[index];
        seed_token = rxas_queue_operand(
                &graph->items[selection->seed_index], 0);
        for (rewrite_index = 0;
             rewrite_index < selection->plan.rewrite_count;
             rewrite_index++) {
            RxasFlowOperandRewrite rewrite;
            rxas_flow_string_literal_reuse_plan_rewrite(
                    proof, session->epoch, &selection->plan,
                    rewrite_index, &rewrite);
            if (rewrite.instruction_id == RXAS_FLOW_ID_NONE &&
                rewrite.operand_index == RXAS_FLOW_ID_NONE) {
                Assembler_Token *trace_number;
                trace_number = flow_new_numbered_token(
                        graph, seed_token, INT, 0,
                        selection->plan.seed_register.number);
                if (!trace_number)
                    RX_PANIC_OOM(
                            "create RXAS string-literal TRACE token",
                            sizeof(Assembler_Token),
                            graph->context->file_name);
                graph->items[rewrite.record_id].operand5Token = trace_number;
            }
            else
                flow_set_operand(
                        &graph->items[rewrite.record_id],
                        rewrite.operand_index, seed_token);
        }
        if (selection->plan.preserve_candidate_register) {
            instruction_queue *candidate;
            Assembler_Token *replacement_operands[2];
            candidate = &graph->items[selection->candidate_index];
            replacement_operands[0] = rxas_queue_operand(candidate, 0);
            replacement_operands[1] = seed_token;
            candidate->instrToken = rxas_tid(
                    graph->context, candidate->instrToken,
                    (char *)op_table[OP_LINK_REG_REG].mnemonic);
            rxas_set_queue_operands(
                    graph->context, candidate, replacement_operands, 2);
        }
        else graph->items[selection->candidate_index].instrType = EMPTY;
        stats->string_literal_reuse_proved++;
        stats->string_literal_loads_reused++;
        stats->string_literal_operands_redirected +=
                selection->plan.rewrite_count;
        stats->operands_redirected += selection->plan.rewrite_count;
        if (graph->context->debug_mode)
            fprintf(stderr,
                    "PERF3 string-literal-reuse-select procedure=%s "
                    "seed=%llu candidate=%llu loop=%llu seed=r%llu "
                    "candidate=r%llu rewrites=%llu mode=%s\n",
                    graph->context->current_proc_name
                            ? graph->context->current_proc_name
                            : "(directives)",
                    (unsigned long long)selection->seed_index,
                    (unsigned long long)selection->candidate_index,
                    (unsigned long long)selection->plan.loop_id,
                    (unsigned long long)
                            selection->plan.seed_register.number,
                    (unsigned long long)
                            selection->plan.candidate_register.number,
                    (unsigned long long)selection->plan.rewrite_count,
                    selection->plan.preserve_candidate_register
                            ? "link" : "redirect");
        flow_debug_accept(
                graph, selection->candidate_index,
                "dominated-loop-string-literal-reuse",
                selection->plan.rewrite_count);
    }
    free(claimed);
    free(selections);
    return selection_count;
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

static int flow_token_matches_local_register(
        const Assembler_Token *token, size_t number) {
    return token && token->token_type == RREG &&
           token->token_value.integer >= 0 &&
           (size_t)token->token_value.integer == number;
}

typedef struct flow_local_register_summary {
    size_t local_count;
    size_t *explicit_occurrences;
    unsigned char *observed;
    size_t lowest_observed_range_base;
    int observes_all;
} flow_local_register_summary;

static void flow_local_summary_mark(
        flow_local_register_summary *summary, size_t number) {
    if (summary && number < summary->local_count)
        summary->observed[number] = 1;
}

static int flow_build_local_register_summary(
        const flow_graph *graph, flow_local_register_summary *summary) {
    size_t record;
    if (!graph || !summary) return 0;
    memset(summary, 0, sizeof(*summary));
    summary->local_count = graph->context->current_locals > 0
            ? (size_t)graph->context->current_locals : 0;
    summary->lowest_observed_range_base = RXAS_FLOW_ID_NONE;
    if (!summary->local_count) return 1;
    summary->explicit_occurrences = calloc(
            summary->local_count, sizeof(*summary->explicit_occurrences));
    summary->observed = calloc(
            summary->local_count, sizeof(*summary->observed));
    if (!summary->explicit_occurrences || !summary->observed) {
        free(summary->explicit_occurrences);
        free(summary->observed);
        memset(summary, 0, sizeof(*summary));
        return 0;
    }
    for (record = 0; record < graph->item_count; record++) {
        const instruction_queue *item;
        const flow_node *node;
        size_t operand;
        size_t implicit_number;
        item = &graph->items[record];
        node = &graph->nodes[record];
        if (item->instrType == REG_META && item->operand3Token &&
            item->operand3Token->token_type == RREG &&
            item->operand3Token->token_value.integer >= 0) {
            flow_local_summary_mark(
                    summary,
                    (size_t)item->operand3Token->token_value.integer);
            continue;
        }
        if (item->instrType == TRACE_EVENT) {
            if (item->operand2Token &&
                item->operand2Token->token_type == STRING &&
                !strcmp((const char *)
                        item->operand2Token->token_value.string, "R") &&
                item->operand4Token &&
                item->operand4Token->token_type == STRING &&
                item->operand4Token->token_value.string[0] &&
                tolower((unsigned char)
                        item->operand4Token->token_value.string[0]) == 'r' &&
                item->operand5Token &&
                item->operand5Token->token_type == INT &&
                item->operand5Token->token_value.integer >= 0)
                flow_local_summary_mark(
                        summary,
                        (size_t)item->operand5Token->token_value.integer);
            continue;
        }
        if (item->instrType != OP_CODE) continue;
        for (operand = 0; operand < item->operandCount; operand++) {
            const Assembler_Token *token;
            token = rxas_queue_operand(item, operand);
            if (token && token->token_type == RREG &&
                token->token_value.integer >= 0 &&
                (size_t)token->token_value.integer < summary->local_count)
                summary->explicit_occurrences[
                        (size_t)token->token_value.integer]++;
        }
        if (!node->op ||
            node->effects.state != RXOP_EFFECT_CLASSIFIED) {
            summary->observes_all = 1;
            continue;
        }
        switch (node->effects.implicit) {
            case RXOP_IMPLICIT_LOCAL_R0_READ_WRITE:
            case RXOP_IMPLICIT_LOCAL_R1_READ_WRITE:
            case RXOP_IMPLICIT_LOCAL_R2_READ_WRITE:
                implicit_number = node->effects.implicit ==
                                RXOP_IMPLICIT_LOCAL_R0_READ_WRITE ? 0 :
                        node->effects.implicit ==
                                RXOP_IMPLICIT_LOCAL_R1_READ_WRITE ? 1 : 2;
                flow_local_summary_mark(summary, implicit_number);
                break;
            case RXOP_IMPLICIT_LOCAL_COPY:
                if (!item->operand1Token ||
                    item->operand1Token->token_type != INT ||
                    item->operand1Token->token_value.integer < 0 ||
                    !item->operand2Token ||
                    item->operand2Token->token_type != INT ||
                    item->operand2Token->token_value.integer < 0) {
                    summary->observes_all = 1;
                    break;
                }
                flow_local_summary_mark(
                        summary,
                        (size_t)item->operand1Token->token_value.integer);
                flow_local_summary_mark(
                        summary,
                        (size_t)item->operand2Token->token_value.integer);
                break;
            case RXOP_IMPLICIT_LOCAL_TARGET:
                summary->observes_all = 1;
                break;
            case RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3:
                if (!item->operand3Token ||
                    item->operand3Token->token_type != RREG ||
                    item->operand3Token->token_value.integer < 0) {
                    summary->observes_all = 1;
                    break;
                }
                implicit_number =
                        (size_t)item->operand3Token->token_value.integer;
                if (summary->lowest_observed_range_base ==
                            RXAS_FLOW_ID_NONE ||
                    implicit_number < summary->lowest_observed_range_base)
                    summary->lowest_observed_range_base = implicit_number;
                break;
            case RXOP_IMPLICIT_ARGUMENT_INDEX:
            case RXOP_IMPLICIT_NONE:
                break;
            default:
                summary->observes_all = 1;
                break;
        }
        if ((node->effects.semantics &
             (RXOP_SEM_CALL | RXOP_SEM_DYNAMIC_CALL)) &&
            node->effects.implicit != RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3)
            summary->observes_all = 1;
    }
    return 1;
}

static void flow_free_local_register_summary(
        flow_local_register_summary *summary) {
    if (!summary) return;
    free(summary->explicit_occurrences);
    free(summary->observed);
    memset(summary, 0, sizeof(*summary));
}

static int flow_local_summary_observes(
        const flow_local_register_summary *summary, size_t number) {
    return !summary || number >= summary->local_count ||
           summary->observes_all || summary->observed[number] ||
           (summary->lowest_observed_range_base != RXAS_FLOW_ID_NONE &&
            number > summary->lowest_observed_range_base);
}

/* The common compiler-temporary form does not need whole-procedure SSA: an
 * exact local typed copy followed immediately by its sole raw read can be
 * substituted when the destination was never remapped, is not observed by
 * metadata or an implicit call window, and never appears again. This is a
 * deliberately strict write-once proof; all other copies remain SSA cases. */
static int flow_plan_local_single_use_copy(
        const flow_graph *graph,
        const flow_local_register_summary *summary, size_t copy_record,
        size_t *use_record, size_t *use_operand) {
    const instruction_queue *copy;
    const instruction_queue *use;
    const Assembler_Token *destination;
    const Assembler_Token *source;
    unsigned int component;
    size_t destination_number;
    size_t operand;
    size_t matched_operand;
    size_t matches;
    if (!graph || !use_record || !use_operand ||
        copy_record + 1 >= graph->item_count ||
        !graph->nodes[copy_record].reachable ||
        graph->items[copy_record].instrType != OP_CODE ||
        !graph->nodes[copy_record].op)
        return 0;
    copy = &graph->items[copy_record];
    if (copy->operandCount != 2) return 0;
    component = graph->nodes[copy_record].op->opcode == OP_ICOPY_REG_REG
            ? RXOP_COMPONENT_INTEGER :
            graph->nodes[copy_record].op->opcode == OP_FCOPY_REG_REG
            ? RXOP_COMPONENT_FLOAT :
            graph->nodes[copy_record].op->opcode == OP_SCOPY_REG_REG
            ? RXOP_COMPONENT_STRING : RXOP_COMPONENT_NONE;
    destination = rxas_queue_operand(copy, 0);
    source = rxas_queue_operand(copy, 1);
    if (!component || !destination || destination->token_type != RREG ||
        destination->token_value.integer < 0 ||
        !source || !flow_register_type(source) ||
        (flow_register_type(source) == 'r' &&
         source->token_value.integer == destination->token_value.integer) ||
        graph->nodes[copy_record].effects.state != RXOP_EFFECT_CLASSIFIED ||
        graph->nodes[copy_record].effects.flow != FLOW_NEXT ||
        graph->nodes[copy_record].effects.optimizer_barrier ||
        graph->nodes[copy_record].effects.reads != RXOP_OP_2 ||
        graph->nodes[copy_record].effects.writes != RXOP_OP_1 ||
        graph->nodes[copy_record].effects.branch_targets != RXOP_OP_NONE ||
        graph->nodes[copy_record].effects.implicit != RXOP_IMPLICIT_NONE ||
        graph->nodes[copy_record].effects.semantics != RXOP_SEM_NONE ||
        rxop_signal_contract(graph->nodes[copy_record].op->opcode).state !=
                RXOP_SIGNAL_STATE_NONE ||
        rxop_context_writes(graph->nodes[copy_record].op->opcode) !=
                RXOP_CONTEXT_NONE ||
        rxop_component_reads(graph->nodes[copy_record].op->opcode, 1) !=
                component ||
        rxop_component_writes(graph->nodes[copy_record].op->opcode, 0) !=
                component)
        return 0;
    destination_number = (size_t)destination->token_value.integer;
    if (flow_local_summary_observes(summary, destination_number) ||
        summary->explicit_occurrences[destination_number] != 2)
        return 0;

    use = &graph->items[copy_record + 1];
    if (!graph->nodes[copy_record + 1].reachable ||
        use->instrType != OP_CODE || !graph->nodes[copy_record + 1].op ||
        graph->nodes[copy_record + 1].effects.state !=
                RXOP_EFFECT_CLASSIFIED ||
        graph->nodes[copy_record + 1].effects.optimizer_barrier ||
        rxop_signal_contract(
                graph->nodes[copy_record + 1].op->opcode).state !=
                RXOP_SIGNAL_STATE_NONE ||
        rxop_context_writes(
                graph->nodes[copy_record + 1].op->opcode) !=
                RXOP_CONTEXT_NONE)
        return 0;

    matches = 0;
    matched_operand = RXAS_FLOW_ID_NONE;
    for (operand = 0; operand < use->operandCount; operand++) {
        const flow_node *node;
        node = &graph->nodes[copy_record + 1];
        if (!flow_token_matches_local_register(
                    rxas_queue_operand(use, operand), destination_number))
            continue;
        if (matches ||
            !rxop_effect_reads_operand(&node->effects, operand) ||
            rxop_effect_writes_operand(&node->effects, operand) ||
            rxop_component_reads(node->op->opcode, operand) != component)
            return 0;
        matched_operand = operand;
        matches++;
    }
    if (matches != 1 || matched_operand == RXAS_FLOW_ID_NONE) return 0;
    *use_record = copy_record + 1;
    *use_operand = matched_operand;
    return 1;
}

static size_t flow_apply_local_single_use_copies(
        flow_graph *graph, flow_stats *stats) {
    flow_local_register_summary summary;
    size_t copy_record;
    size_t use_record;
    size_t use_operand;
    size_t removed;
    removed = 0;
    if (!flow_build_local_register_summary(graph, &summary))
        RX_PANIC_OOM("calloc local register summary",
                     (size_t)(graph->context->current_locals > 0
                             ? graph->context->current_locals : 0) *
                             (sizeof(size_t) + 1), 0);
    for (copy_record = 0; copy_record < graph->item_count; copy_record++) {
        instruction_queue *copy;
        instruction_queue *use;
        Assembler_Token *source;
        if (!flow_record_matches_epoch(graph, copy_record) ||
            !flow_plan_local_single_use_copy(
                    graph, &summary, copy_record,
                    &use_record, &use_operand) ||
            !flow_record_matches_epoch(graph, use_record))
            continue;
        copy = &graph->items[copy_record];
        source = rxas_queue_operand(copy, 1);
        use = flow_edit_record(graph, use_record);
        copy = flow_edit_record(graph, copy_record);
        if (!use || !copy) continue;
        flow_set_operand(use, use_operand, source);
        copy->instrType = EMPTY;
        stats->typed_copies_removed++;
        stats->local_single_use_copies_removed++;
        stats->operands_redirected++;
        removed++;
        flow_debug_accept(graph, copy_record,
                          "single-use-local-copy", 1);
    }
    flow_free_local_register_summary(&summary);
    return removed;
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
        if (!flow_record_matches_epoch(graph, index)) continue;
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
        if (!flow_delete_record(graph, index)) continue;
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
        if (!flow_record_matches_epoch(graph, index)) continue;
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
        if (!flow_delete_record(graph, index)) continue;
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
        if (!flow_record_matches_epoch(graph, index)) continue;
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
        if (!flow_delete_record(graph, index)) continue;
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
        if (!flow_record_matches_epoch(graph, index)) continue;
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
                !flow_record_matches_epoch(
                        graph, candidate->record_id) ||
                !flow_record_matches_epoch(
                        graph, generator->record_id) ||
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
            if (!flow_delete_record(graph, candidate->record_id))
                continue;
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
            "unreachable=%llu dead=%llu typed-copy=%llu "
            "local-single-use-copy=%llu compare-prep=%llu "
            "full-copy=%llu redundant-load=%llu redundant-init=%llu "
            "redundant-conversion=%llu producer-forward=%llu "
            "swap-roundtrip=%llu "
            "compare-branch=%llu trace-delete=%llu redirects=%llu "
            "constant-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "absent-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "self-copy-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "derivation-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "producer-proof=%llu/%llu rejected=%llu unavailable=%llu "
            "component-placement-proof=%llu/%llu rejected=%llu "
            "unavailable=%llu applied=%llu trace-delete=%llu "
            "duplicate-linked-read-proof=%llu/%llu rejected=%llu reused=%llu "
            "storage-permutation-proof=%llu/%llu rejected=%llu "
            "compare-branch-proof=%llu/%llu rejected=%llu "
            "joined-key-reuse-proof=%llu/%llu rejected=%llu reused=%llu "
            "trace-delete=%llu private-locals=%llu preheader=%llu "
            "string-literal-reuse-proof=%llu/%llu rejected=%llu "
            "unavailable=%llu reused=%llu rewrites=%llu "
            "branch-thread=%llu/%llu rejected=%llu applied=%llu batches=%llu "
            "semantic-batch=%llu rejected=%llu records=%llu deleted=%llu "
            "opcodes=%llu operand-records=%llu "
            "reject-live=%llu reject-trace=%llu reject-tainted=%llu reject-effect=%llu\n",
            graph->context->current_proc_name ? graph->context->current_proc_name : "(directives)",
            (unsigned long long)graph->block_count,
            (unsigned long long)graph->register_count,
            (unsigned long long)before_instructions,
            (unsigned long long)after_instructions,
            (unsigned long long)stats->unreachable_removed,
            (unsigned long long)stats->dead_results_removed,
            (unsigned long long)stats->typed_copies_removed,
            (unsigned long long)stats->local_single_use_copies_removed,
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
            (unsigned long long)stats->component_placement_proved,
            (unsigned long long)stats->component_placement_queries,
            (unsigned long long)stats->component_placement_rejected,
            (unsigned long long)stats->component_placement_unavailable,
            (unsigned long long)stats->component_placements_applied,
            (unsigned long long)
                    stats->component_placement_trace_events_removed,
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
            (unsigned long long)stats->joined_key_reuse_proved,
            (unsigned long long)stats->joined_key_reuse_queries,
            (unsigned long long)stats->joined_key_reuse_rejected,
            (unsigned long long)stats->joined_keys_reused,
            (unsigned long long)stats->joined_key_trace_events_removed,
            (unsigned long long)stats->joined_key_private_locals,
            (unsigned long long)stats->joined_key_preheader_eligible,
            (unsigned long long)stats->string_literal_reuse_proved,
            (unsigned long long)stats->string_literal_reuse_queries,
            (unsigned long long)stats->string_literal_reuse_rejected,
            (unsigned long long)stats->string_literal_reuse_unavailable,
            (unsigned long long)stats->string_literal_loads_reused,
            (unsigned long long)
                    stats->string_literal_operands_redirected,
            (unsigned long long)stats->branch_thread_proved,
            (unsigned long long)stats->branch_thread_queries,
            (unsigned long long)stats->branch_thread_rejected,
            (unsigned long long)stats->branch_threads_applied,
            (unsigned long long)stats->branch_thread_batches_applied,
            (unsigned long long)stats->semantic_batches_applied,
            (unsigned long long)stats->semantic_batches_rejected,
            (unsigned long long)stats->semantic_records_changed,
            (unsigned long long)stats->semantic_records_deleted,
            (unsigned long long)stats->semantic_opcodes_replaced,
            (unsigned long long)stats->semantic_operand_records_rewritten,
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

static int flow_semantic_analysis_within_bound(const flow_graph *graph) {
    if (!graph || !flow_value_analysis_within_bound(graph)) return 0;
    if (!graph->register_count) return 1;
    return graph->block_count <=
           FLOW_MAX_SEMANTIC_JOIN_REGISTER_CELLS / graph->register_count;
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

static int flow_has_semantic_candidates(
        const RxasOptimisationCensus *census) {
    return rxas_optimisation_has_candidates(
                   census, RXAS_PASS_LOCAL_SINGLE_USE_COPY) ||
           rxas_optimisation_has_candidates(
                   census, RXAS_PASS_M04_SELF_COPY) ||
           rxas_optimisation_has_candidates(
                   census, RXAS_PASS_K02_K03_LINKED_READ) ||
           rxas_optimisation_has_candidates(
                   census, RXAS_PASS_M05_TYPED_COPY) ||
           rxas_optimisation_has_candidates(
                   census, RXAS_PASS_M06_PRODUCER_FORWARD) ||
           rxas_optimisation_has_candidates(
                   census, RXAS_PASS_X01_COMPONENT_PLACEMENT) ||
           rxas_optimisation_has_candidates(
                   census, RXAS_PASS_K04_COMPARE_BRANCH) ||
           rxas_optimisation_has_candidates(
                   census, RXAS_PASS_H01_JOINED_KEY_REUSE) ||
           rxas_optimisation_has_candidates(
                   census, RXAS_PASS_H02_STRING_LITERAL_REUSE) ||
           rxas_optimisation_has_candidates(
                   census, RXAS_PASS_M02_CONSTANT) ||
           rxas_optimisation_has_candidates(
                   census, RXAS_PASS_M03_ABSENT) ||
           rxas_optimisation_has_candidates(
                   census, RXAS_PASS_M01_DERIVATION);
}

static proc_constant *flow_private_local_procedure(flow_graph *graph) {
    Assembler_Context *context;
    proc_constant *procedure;
    size_t offset;
    if (!graph || !(context = graph->context) ||
        context->proc_tail < 0 || !context->binary.const_pool)
        return 0;
    offset = (size_t)context->proc_tail;
    if (offset > context->binary.const_size ||
        context->binary.const_size - offset < sizeof(*procedure))
        return 0;
    procedure = (proc_constant *)(context->binary.const_pool + offset);
    return procedure->base.type == PROC_CONST ? procedure : 0;
}

static int flow_private_locals_can_commit(flow_graph *graph) {
    proc_constant *procedure;
    if (!graph || !graph->private_locals_planned) return 1;
    procedure = flow_private_local_procedure(graph);
    return procedure && graph->context->current_locals >= 0 &&
           procedure->locals == graph->context->current_locals &&
           graph->private_locals_planned <=
                   (size_t)(INT_MAX - graph->context->current_locals);
}

static void flow_commit_private_locals(flow_graph *graph) {
    proc_constant *procedure;
    int new_count;
    if (!graph || !graph->private_locals_planned) return;
    procedure = flow_private_local_procedure(graph);
    new_count = graph->context->current_locals +
            (int)graph->private_locals_planned;
    procedure->locals = new_count;
    graph->context->current_locals = new_count;
}

/* D0.4 keeps proof facts immutable while compatible semantic consumers edit a
 * private queue overlay in their established priority order.  Every consumer
 * still validates its typed plan; delete-only consumers additionally require
 * their target record to match the epoch.  The live queue changes only after
 * complete batch validation, and K05 remains a later CFG-only epoch. */
static size_t flow_apply_semantic_epoch(
        flow_graph *graph, flow_stats *stats, flow_proof_session *session,
        const RxasOptimisationCensus *census, int allow_ssa) {
    RxasFlowQueueBatch batch;
    RxasFlowQueueBatchMetrics metrics;
    instruction_queue *original_items;
    size_t planned;

    if (!flow_has_semantic_candidates(census)) return 0;
    memset(&batch, 0, sizeof(batch));
    memset(&metrics, 0, sizeof(metrics));
    original_items = graph->items;
    if (!rxas_flow_queue_batch_begin(
                &batch, graph->context, original_items,
                graph->item_count))
        return 0;
    graph->queue_batch = &batch;
    planned = 0;

    if (rxas_optimisation_has_candidates(
            census, RXAS_PASS_LOCAL_SINGLE_USE_COPY))
        planned += flow_apply_local_single_use_copies(graph, stats);
    if (allow_ssa && rxas_optimisation_has_candidates(
            census, RXAS_PASS_M04_SELF_COPY))
        planned += flow_remove_redundant_self_copies(
                graph, stats, session);
    if (allow_ssa && rxas_optimisation_has_candidates(
            census, RXAS_PASS_K02_K03_LINKED_READ))
        planned += flow_reuse_duplicate_linked_read(
                graph, stats, session);
    if (allow_ssa && rxas_optimisation_has_candidates(
            census, RXAS_PASS_M05_TYPED_COPY))
        planned += flow_propagate_copies(graph, stats, session);
    if (allow_ssa && rxas_optimisation_has_candidates(
            census, RXAS_PASS_M06_PRODUCER_FORWARD))
        planned += flow_forward_producer_destination(
                graph, stats, session);
    if (allow_ssa && rxas_optimisation_has_candidates(
            census, RXAS_PASS_X01_COMPONENT_PLACEMENT))
        planned += flow_place_copied_derivations(
                graph, stats, session);
    if (allow_ssa && rxas_optimisation_has_candidates(
            census, RXAS_PASS_K04_COMPARE_BRANCH))
        planned += flow_fuse_compare_branches(graph, stats, session);
    if (allow_ssa && rxas_optimisation_has_candidates(
            census, RXAS_PASS_H01_JOINED_KEY_REUSE))
        planned += flow_reuse_joined_keys(graph, stats, session);
    if (allow_ssa && rxas_optimisation_has_candidates(
            census, RXAS_PASS_H02_STRING_LITERAL_REUSE))
        planned += flow_reuse_string_literals(graph, stats, session);
    if (allow_ssa && rxas_optimisation_has_candidates(
            census, RXAS_PASS_M02_CONSTANT))
        planned += flow_remove_redundant_loads(graph, stats, session);
    if (allow_ssa && rxas_optimisation_has_candidates(
            census, RXAS_PASS_M03_ABSENT))
        planned += flow_remove_redundant_initializations(
                graph, stats, session);
    if (allow_ssa && rxas_optimisation_has_candidates(
            census, RXAS_PASS_M01_DERIVATION))
        planned += flow_remove_redundant_conversions(
                graph, stats, session);

    if (!rxas_flow_procedure_rebind_queue_records(
                graph->procedure, graph->epoch,
                original_items, graph->item_count)) {
        graph->queue_batch = 0;
        rxas_flow_queue_batch_destroy(&batch);
        if (planned) stats->semantic_batches_rejected++;
        return 0;
    }
    graph->queue_batch = 0;
    if (planned && flow_private_locals_can_commit(graph) &&
        rxas_flow_queue_batch_commit(&batch, &metrics) &&
        metrics.records_changed) {
        flow_commit_private_locals(graph);
        stats->semantic_batches_applied++;
        stats->semantic_records_changed += metrics.records_changed;
        stats->semantic_records_deleted += metrics.records_deleted;
        stats->semantic_opcodes_replaced += metrics.opcodes_replaced;
        stats->semantic_operand_records_rewritten +=
                metrics.operand_records_rewritten;
        if (graph->context->debug_mode)
            fprintf(stderr,
                    "PERF3 semantic-batch procedure=%s epoch=%lu "
                    "plans=%llu records=%llu deleted=%llu opcodes=%llu "
                    "operand-records=%llu private-locals=%llu "
                    "status=applied\n",
                    graph->context->current_proc_name
                            ? graph->context->current_proc_name
                            : "(directives)",
                    graph->epoch,
                    (unsigned long long)planned,
                    (unsigned long long)metrics.records_changed,
                    (unsigned long long)metrics.records_deleted,
                    (unsigned long long)metrics.opcodes_replaced,
                    (unsigned long long)
                            metrics.operand_records_rewritten,
                    (unsigned long long)graph->private_locals_planned);
    }
    else if (planned) {
        stats->semantic_batches_rejected++;
        metrics.records_changed = 0;
        if (graph->context->debug_mode)
            fprintf(stderr,
                    "PERF3 semantic-batch procedure=%s epoch=%lu "
                    "plans=%llu status=rejected\n",
                    graph->context->current_proc_name
                            ? graph->context->current_proc_name
                            : "(directives)",
                    graph->epoch, (unsigned long long)planned);
    }
    rxas_flow_queue_batch_destroy(&batch);
    return metrics.records_changed;
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
    int semantic_admitted;
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
         * graph for the epoch. Structural mutation ends the epoch. Compatible
         * semantic edits pin proof-facing records to their originals until
         * the sparse transaction commits and then rebuild once. */
        if (!graph.complete_control_flow) {
            if (context->debug_mode)
                fprintf(stderr,
                        "NR27 reject procedure=%s candidate=whole-procedure "
                        "reason=incomplete-control-flow\\n",
                        context->current_proc_name);
            changed = 0;
        }
        else {
            semantic_admitted =
                    flow_semantic_analysis_within_bound(&graph);
            changed = rxas_optimisation_has_candidates(
                    &census, RXAS_PASS_M00_REACHABILITY)
                    ? flow_remove_unreachable(&graph, &stats) : 0;
            /* Every SSA consumer shares the same scale admission. Mechanical
             * local rewrites and CFG-only consumers remain available when a
             * procedure exceeds it. */
            if (!changed && semantic_admitted &&
                rxas_optimisation_has_candidates(
                    &census, RXAS_PASS_K01_STORAGE_PERMUTATION))
                changed += flow_remove_swap_round_trips(
                        &graph, &stats, &proof_session);
            if (!changed)
                changed = flow_apply_semantic_epoch(
                        &graph, &stats, &proof_session, &census,
                        semantic_admitted);
            if (!changed && !semantic_admitted && context->debug_mode) {
                fprintf(stderr,
                        "NR27 bound procedure=%s scope=local-cfg "
                        "value-cells=%llu value-limit=%llu "
                        "join-register-cells=%llu semantic-limit=%llu\n",
                        context->current_proc_name,
                        (unsigned long long)
                                (graph.item_count * graph.word_count),
                        (unsigned long long)FLOW_MAX_INDIRECT_VALUE_CELLS,
                        (unsigned long long)
                                (graph.block_count * graph.register_count),
                        (unsigned long long)
                                FLOW_MAX_SEMANTIC_JOIN_REGISTER_CELLS);
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
            if (!flow_semantic_analysis_within_bound(&graph)) {
                analysis_capabilities &= RXAS_FLOW_CAP_CFG;
                fprintf(stderr,
                        "PERF3 flow-diagnostic procedure=%s "
                        "disabled=semantic-scale-bound "
                        "join-register-cells=%llu limit=%llu\n",
                        context->current_proc_name ? context->current_proc_name
                                                   : "(directives)",
                        (unsigned long long)
                                (graph.block_count * graph.register_count),
                        (unsigned long long)
                                FLOW_MAX_SEMANTIC_JOIN_REGISTER_CELLS);
            }
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
