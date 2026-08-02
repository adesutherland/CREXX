/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

/* Sparse signal-policy and non-register effect versions for an immutable
 * RXAS procedure epoch.  This is proof infrastructure only: it does not
 * rewrite queued instructions or affect the emitted image. */

#include "rxas_flow_signal.h"
#include "rxas_flow_graph_internal.h"
#include "rxasassm.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef enum FlowPolicyVersionKind {
    FLOW_POLICY_ENTRY = 0,
    FLOW_POLICY_WRITE,
    FLOW_POLICY_PHI,
    FLOW_POLICY_CLOBBER
} FlowPolicyVersionKind;

typedef struct FlowPolicyVersion {
    FlowPolicyVersionKind kind;
    size_t parent;
    size_t instruction_id;
    size_t block_id;
    size_t input_offset;
    size_t input_count;
    RxOpSignalPolicyEffect effect;
    const char *name;
} FlowPolicyVersion;

typedef enum FlowEffectVersionKind {
    FLOW_EFFECT_ENTRY = 0,
    FLOW_EFFECT_WRITE,
    FLOW_EFFECT_PHI,
    FLOW_EFFECT_UNKNOWN
} FlowEffectVersionKind;

typedef struct FlowEffectVersion {
    FlowEffectVersionKind kind;
    RxasFlowEffectClass effect_class;
    size_t parent;
    size_t instruction_id;
    size_t block_id;
} FlowEffectVersion;

struct RxasFlowSignalAnalysis {
    const RxasFlowProcedure *procedure;
    const RxasFlowStructuralAnalysis *structural;
    RxasFlowSignalMetrics metrics;
    size_t block_count;
    size_t edge_count;
    size_t instruction_count;
    size_t record_count;
    size_t entry_policy;
    size_t entry_effect[RXAS_FLOW_EFFECT_CLASS_COUNT];
    size_t *incoming_offsets;
    size_t *incoming_edges;
    size_t *outgoing_offsets;
    size_t *outgoing_edges;
    size_t *policy_phi_inputs;
    size_t *block_policy_in;
    size_t *block_policy_out;
    size_t *instruction_policy_before;
    size_t *instruction_policy_after;
    size_t *edge_policy;
    size_t *block_effect_in;
    size_t *block_effect_out;
    size_t *instruction_effect_before;
    size_t *instruction_effect_after;
    size_t *edge_effect;
    unsigned char effect_active[RXAS_FLOW_EFFECT_CLASS_COUNT];
    FlowPolicyVersion *policy_versions;
    size_t policy_version_count;
    size_t policy_version_capacity;
    FlowEffectVersion *effect_versions;
    size_t effect_version_count;
    size_t effect_version_capacity;
};

#define FLOW_EFFECT_INDEX(OWNER, EFFECT_CLASS) \
    ((OWNER) * (size_t)RXAS_FLOW_EFFECT_CLASS_COUNT + (size_t)(EFFECT_CLASS))

static int flow_signal_consume(RxasFlowSignalAnalysis *analysis,
                               size_t amount) {
    size_t remaining;
    if (!analysis ||
        analysis->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE)
        return 0;
    remaining = analysis->metrics.work <= analysis->metrics.budget_limit
            ? analysis->metrics.budget_limit - analysis->metrics.work : 0;
    if (amount > remaining) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED;
        return 0;
    }
    analysis->metrics.work += amount;
    return 1;
}

static void *flow_signal_calloc(RxasFlowSignalAnalysis *analysis,
                                size_t count, size_t size) {
    void *memory;
    if (!count) count = 1;
    if (size && count > ((size_t)-1) / size) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    memory = calloc(count, size);
    if (!memory)
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
    return memory;
}

static size_t flow_signal_default_budget(const RxasFlowMetrics *metrics) {
    size_t scale;
    if (!metrics || metrics->blocks > (size_t)-1 - metrics->edges ||
        metrics->blocks + metrics->edges >
                (size_t)-1 - metrics->instructions ||
        metrics->blocks + metrics->edges + metrics->instructions >
                (size_t)-1 - metrics->records - 1)
        return (size_t)-1;
    scale = metrics->blocks + metrics->edges + metrics->instructions +
            metrics->records + 1;
    if (scale > ((size_t)-1 - 4096) / 512) return (size_t)-1;
    return scale * 512 + 4096;
}

static int flow_signal_name_equal(const char *left, const char *right) {
    unsigned char lch;
    unsigned char rch;
    if (!left || !right) return 0;
    do {
        lch = (unsigned char)*left++;
        rch = (unsigned char)*right++;
        if (toupper(lch) != toupper(rch)) return 0;
    } while (lch && rch);
    return lch == rch;
}

static Assembler_Token *flow_signal_operand(
        const instruction_queue *item, size_t operand_index) {
    if (!item || operand_index >= item->operandCount) return 0;
    if (item->operandTokens) return item->operandTokens[operand_index];
    switch (operand_index) {
        case 0: return item->operand1Token;
        case 1: return item->operand2Token;
        case 2: return item->operand3Token;
        case 3: return item->operand4Token;
        case 4: return item->operand5Token;
        case 5: return item->operand6Token;
        case 6: return item->operand7Token;
        case 7: return item->operand8Token;
        case 8: return item->operand9Token;
        case 9: return item->operand10Token;
    }
    return 0;
}

static size_t flow_signal_add_policy_version(
        RxasFlowSignalAnalysis *analysis, FlowPolicyVersionKind kind,
        size_t parent, size_t instruction_id, size_t block_id,
        size_t input_offset, size_t input_count,
        RxOpSignalPolicyEffect effect, const char *name) {
    FlowPolicyVersion *versions;
    FlowPolicyVersion *version;
    size_t capacity;
    size_t id;
    if (!flow_signal_consume(analysis, 1)) return RXAS_FLOW_ID_NONE;
    if (analysis->policy_version_count == analysis->policy_version_capacity) {
        capacity = analysis->policy_version_capacity
                ? analysis->policy_version_capacity * 2 : 16;
        if (capacity < analysis->policy_version_capacity ||
            capacity > ((size_t)-1) / sizeof(*versions)) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return RXAS_FLOW_ID_NONE;
        }
        versions = realloc(analysis->policy_versions,
                           capacity * sizeof(*versions));
        if (!versions) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return RXAS_FLOW_ID_NONE;
        }
        analysis->policy_versions = versions;
        analysis->policy_version_capacity = capacity;
    }
    id = analysis->policy_version_count++;
    version = &analysis->policy_versions[id];
    version->kind = kind;
    version->parent = parent;
    version->instruction_id = instruction_id;
    version->block_id = block_id;
    version->input_offset = input_offset;
    version->input_count = input_count;
    version->effect = effect;
    version->name = name;
    analysis->metrics.policy_versions++;
    if (kind == FLOW_POLICY_WRITE) {
        analysis->metrics.policy_writes++;
        if (effect == RXOP_POLICY_EFFECT_PUSH ||
            effect == RXOP_POLICY_EFFECT_POP)
            analysis->metrics.stack_writes++;
    }
    else if (kind == FLOW_POLICY_PHI) analysis->metrics.policy_phis++;
    else if (kind == FLOW_POLICY_CLOBBER)
        analysis->metrics.policy_clobbers++;
    return id;
}

static size_t flow_signal_add_effect_version(
        RxasFlowSignalAnalysis *analysis, FlowEffectVersionKind kind,
        RxasFlowEffectClass effect_class, size_t parent,
        size_t instruction_id, size_t block_id) {
    FlowEffectVersion *versions;
    FlowEffectVersion *version;
    size_t capacity;
    size_t id;
    if (!flow_signal_consume(analysis, 1)) return RXAS_FLOW_ID_NONE;
    if (analysis->effect_version_count == analysis->effect_version_capacity) {
        capacity = analysis->effect_version_capacity
                ? analysis->effect_version_capacity * 2 : 32;
        if (capacity < analysis->effect_version_capacity ||
            capacity > ((size_t)-1) / sizeof(*versions)) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return RXAS_FLOW_ID_NONE;
        }
        versions = realloc(analysis->effect_versions,
                           capacity * sizeof(*versions));
        if (!versions) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return RXAS_FLOW_ID_NONE;
        }
        analysis->effect_versions = versions;
        analysis->effect_version_capacity = capacity;
    }
    id = analysis->effect_version_count++;
    version = &analysis->effect_versions[id];
    version->kind = kind;
    version->effect_class = effect_class;
    version->parent = parent;
    version->instruction_id = instruction_id;
    version->block_id = block_id;
    analysis->metrics.effect_versions++;
    if (kind == FLOW_EFFECT_PHI) analysis->metrics.effect_phis++;
    return id;
}

static unsigned int flow_signal_all_effects(void) {
    return (1u << RXAS_FLOW_EFFECT_CLASS_COUNT) - 1u;
}

static unsigned int flow_signal_instruction_effect_mask(
        const RxasFlowInstruction *instruction) {
    unsigned int mask;
    unsigned int alias_semantics;
    unsigned int mutation_semantics;
    if (!instruction || !instruction->op) return flow_signal_all_effects();
    /* Signal-set/dependency uncertainty belongs to exceptional-edge state.
     * It does not turn a classified normal instruction transfer into a write
     * of every independent effect class. */
    if (instruction->effects.state == RXOP_EFFECT_CONSERVATIVE)
        return flow_signal_all_effects();
    mask = 0;
    if (rxop_context_writes(instruction->op->opcode) & RXOP_CONTEXT_NUMERIC)
        mask |= 1u << RXAS_FLOW_EFFECT_NUMERIC_CONTEXT;
    if (instruction->effects.semantics &
        (RXOP_SEM_CALL | RXOP_SEM_DYNAMIC_CALL)) {
        /* VM call arguments are pointer-bound to caller-owned value storage.
         * The callee's handler table is frame-local/COW, but argument and
         * external effects are caller-visible and cannot be restored away. */
        mask |= (1u << RXAS_FLOW_EFFECT_CALL) |
                (1u << RXAS_FLOW_EFFECT_REFERENCE) |
                (1u << RXAS_FLOW_EFFECT_EXTERNAL) |
                (1u << RXAS_FLOW_EFFECT_PLUGIN) |
                (1u << RXAS_FLOW_EFFECT_LOCALE);
    }
    alias_semantics = RXOP_SEM_ALIAS_CREATE | RXOP_SEM_ALIAS_RELEASE |
            RXOP_SEM_REFERENCE_CREATE | RXOP_SEM_REFERENCE_READ |
            RXOP_SEM_REFERENCE_RELEASE;
    mutation_semantics = RXOP_SEM_REFERENCE_WRITE |
            RXOP_SEM_LIFETIME_END | RXOP_SEM_INDIRECT_WRITE;
    if (instruction->effects.semantics & alias_semantics)
        mask |= 1u << RXAS_FLOW_EFFECT_ALIAS;
    if (instruction->effects.semantics & mutation_semantics)
        mask |= 1u << RXAS_FLOW_EFFECT_REFERENCE;
    if (instruction->effects.semantics & RXOP_SEM_OPAQUE)
        mask |= 1u << RXAS_FLOW_EFFECT_EXTERNAL;
    return mask;
}

static int flow_signal_build_incoming(RxasFlowSignalAnalysis *analysis) {
    size_t *incoming_fill;
    size_t *outgoing_fill;
    size_t edge_id;
    size_t block_id;
    analysis->incoming_offsets = flow_signal_calloc(
            analysis, analysis->block_count + 1, sizeof(size_t));
    analysis->incoming_edges = flow_signal_calloc(
            analysis, analysis->edge_count, sizeof(size_t));
    analysis->outgoing_offsets = flow_signal_calloc(
            analysis, analysis->block_count + 1, sizeof(size_t));
    analysis->outgoing_edges = flow_signal_calloc(
            analysis, analysis->edge_count, sizeof(size_t));
    analysis->policy_phi_inputs = flow_signal_calloc(
            analysis, analysis->edge_count, sizeof(size_t));
    incoming_fill = flow_signal_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    outgoing_fill = flow_signal_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    if (!analysis->incoming_offsets || !analysis->incoming_edges ||
        !analysis->outgoing_offsets || !analysis->outgoing_edges ||
        !analysis->policy_phi_inputs || !incoming_fill || !outgoing_fill) {
        free(incoming_fill);
        free(outgoing_fill);
        return 0;
    }
    for (edge_id = 0; edge_id < analysis->edge_count; edge_id++) {
        const RxasFlowEdge *edge;
        edge = rxas_flow_procedure_edge(
                analysis->procedure, analysis->metrics.epoch, edge_id);
        if (!edge || edge->source >= analysis->block_count ||
            edge->target >= analysis->block_count) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
            free(incoming_fill);
            free(outgoing_fill);
            return 0;
        }
        analysis->incoming_offsets[edge->target + 1]++;
        analysis->outgoing_offsets[edge->source + 1]++;
        if (!flow_signal_consume(analysis, 1)) {
            free(incoming_fill);
            free(outgoing_fill);
            return 0;
        }
    }
    for (block_id = 1; block_id <= analysis->block_count; block_id++) {
        analysis->incoming_offsets[block_id] +=
                analysis->incoming_offsets[block_id - 1];
        analysis->outgoing_offsets[block_id] +=
                analysis->outgoing_offsets[block_id - 1];
    }
    for (edge_id = 0; edge_id < analysis->edge_count; edge_id++) {
        const RxasFlowEdge *edge;
        size_t offset;
        edge = rxas_flow_procedure_edge(
                analysis->procedure, analysis->metrics.epoch, edge_id);
        offset = analysis->incoming_offsets[edge->target] +
                 incoming_fill[edge->target]++;
        analysis->incoming_edges[offset] = edge_id;
        analysis->policy_phi_inputs[offset] = RXAS_FLOW_ID_NONE;
        offset = analysis->outgoing_offsets[edge->source] +
                 outgoing_fill[edge->source]++;
        analysis->outgoing_edges[offset] = edge_id;
    }
    free(incoming_fill);
    free(outgoing_fill);
    return flow_signal_consume(analysis, analysis->edge_count) &&
           flow_signal_consume(analysis, analysis->edge_count);
}

static int flow_signal_allocate_state(RxasFlowSignalAnalysis *analysis) {
    size_t block_effect_cells;
    size_t instruction_effect_cells;
    size_t edge_effect_cells;
    if (analysis->block_count >
            (size_t)-1 / RXAS_FLOW_EFFECT_CLASS_COUNT ||
        analysis->instruction_count >
            (size_t)-1 / RXAS_FLOW_EFFECT_CLASS_COUNT ||
        analysis->edge_count >
            (size_t)-1 / RXAS_FLOW_EFFECT_CLASS_COUNT) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    block_effect_cells = analysis->block_count *
                         RXAS_FLOW_EFFECT_CLASS_COUNT;
    instruction_effect_cells = analysis->instruction_count *
                               RXAS_FLOW_EFFECT_CLASS_COUNT;
    edge_effect_cells = analysis->edge_count *
                        RXAS_FLOW_EFFECT_CLASS_COUNT;
    analysis->block_policy_in = flow_signal_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    analysis->block_policy_out = flow_signal_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    analysis->instruction_policy_before = flow_signal_calloc(
            analysis, analysis->instruction_count, sizeof(size_t));
    analysis->instruction_policy_after = flow_signal_calloc(
            analysis, analysis->instruction_count, sizeof(size_t));
    analysis->edge_policy = flow_signal_calloc(
            analysis, analysis->edge_count, sizeof(size_t));
    analysis->block_effect_in = flow_signal_calloc(
            analysis, block_effect_cells, sizeof(size_t));
    analysis->block_effect_out = flow_signal_calloc(
            analysis, block_effect_cells, sizeof(size_t));
    analysis->instruction_effect_before = flow_signal_calloc(
            analysis, instruction_effect_cells, sizeof(size_t));
    analysis->instruction_effect_after = flow_signal_calloc(
            analysis, instruction_effect_cells, sizeof(size_t));
    analysis->edge_effect = flow_signal_calloc(
            analysis, edge_effect_cells, sizeof(size_t));
    return analysis->block_policy_in && analysis->block_policy_out &&
           analysis->instruction_policy_before &&
           analysis->instruction_policy_after && analysis->edge_policy &&
           analysis->block_effect_in && analysis->block_effect_out &&
           analysis->instruction_effect_before &&
           analysis->instruction_effect_after && analysis->edge_effect;
}

static int flow_signal_is_root_block(const RxasFlowBlock *block) {
    return block && (block->kind == RXAS_FLOW_BLOCK_ENTRY ||
                     block->kind == RXAS_FLOW_BLOCK_HANDLER_ROOT ||
                     block->kind == RXAS_FLOW_BLOCK_ASYNC_ROOT);
}

static int flow_signal_scan_active_effects(
        RxasFlowSignalAnalysis *analysis) {
    size_t instruction_id;
    size_t record_id;
    for (instruction_id = 0; instruction_id < analysis->instruction_count;
         instruction_id++) {
        const RxasFlowInstruction *instruction;
        unsigned int mask;
        size_t effect_class;
        instruction = rxas_flow_procedure_instruction(
                analysis->procedure, analysis->metrics.epoch,
                instruction_id);
        if (!instruction) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
            return 0;
        }
        mask = flow_signal_instruction_effect_mask(instruction);
        for (effect_class = 0;
             effect_class < RXAS_FLOW_EFFECT_CLASS_COUNT; effect_class++)
            if (mask & (1u << effect_class))
                analysis->effect_active[effect_class] = 1;
    }
    for (record_id = 0; record_id < analysis->record_count; record_id++) {
        const RxasFlowRecord *record;
        record = rxas_flow_procedure_record(
                analysis->procedure, analysis->metrics.epoch, record_id);
        if (!record) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
            return 0;
        }
        if (record->type == TRACE_EVENT)
            analysis->effect_active[RXAS_FLOW_EFFECT_TRACE] = 1;
    }
    return flow_signal_consume(
            analysis, analysis->instruction_count + analysis->record_count);
}

static int flow_signal_create_entries_and_phis(
        RxasFlowSignalAnalysis *analysis) {
    size_t effect_class;
    size_t block_id;
    analysis->entry_policy = flow_signal_add_policy_version(
            analysis, FLOW_POLICY_ENTRY, RXAS_FLOW_ID_NONE,
            RXAS_FLOW_ID_NONE, RXAS_FLOW_ID_NONE, 0, 0,
            RXOP_POLICY_EFFECT_NONE, 0);
    if (analysis->entry_policy == RXAS_FLOW_ID_NONE) return 0;
    for (effect_class = 0;
         effect_class < RXAS_FLOW_EFFECT_CLASS_COUNT; effect_class++) {
        analysis->entry_effect[effect_class] = flow_signal_add_effect_version(
                analysis, FLOW_EFFECT_ENTRY, (RxasFlowEffectClass)effect_class,
                RXAS_FLOW_ID_NONE, RXAS_FLOW_ID_NONE, RXAS_FLOW_ID_NONE);
        if (analysis->entry_effect[effect_class] == RXAS_FLOW_ID_NONE)
            return 0;
    }
    for (block_id = 0; block_id < analysis->block_count; block_id++) {
        const RxasFlowBlock *block;
        size_t incoming_count;
        block = rxas_flow_procedure_block(
                analysis->procedure, analysis->metrics.epoch, block_id);
        if (!block) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
            return 0;
        }
        incoming_count = analysis->incoming_offsets[block_id + 1] -
                         analysis->incoming_offsets[block_id];
        if (!flow_signal_is_root_block(block) && incoming_count > 1) {
            size_t version;
            version = flow_signal_add_policy_version(
                    analysis, FLOW_POLICY_PHI, RXAS_FLOW_ID_NONE,
                    RXAS_FLOW_ID_NONE, block_id,
                    analysis->incoming_offsets[block_id], incoming_count,
                    RXOP_POLICY_EFFECT_NONE, 0);
            if (version == RXAS_FLOW_ID_NONE) return 0;
            analysis->block_policy_in[block_id] = version;
            for (effect_class = 0;
                 effect_class < RXAS_FLOW_EFFECT_CLASS_COUNT; effect_class++) {
                if (analysis->effect_active[effect_class]) {
                    version = flow_signal_add_effect_version(
                            analysis, FLOW_EFFECT_PHI,
                            (RxasFlowEffectClass)effect_class,
                            RXAS_FLOW_ID_NONE, RXAS_FLOW_ID_NONE, block_id);
                    if (version == RXAS_FLOW_ID_NONE) return 0;
                    analysis->block_effect_in[
                            FLOW_EFFECT_INDEX(block_id, effect_class)] =
                            version;
                }
            }
        }
    }
    return 1;
}

static size_t flow_signal_policy_transfer(
        RxasFlowSignalAnalysis *analysis,
        const RxasFlowInstruction *instruction, size_t current) {
    const RxOpSignalContract *contract;
    const RxasFlowRecord *record;
    const instruction_queue *item;
    Assembler_Token *operand;
    const char *name;
    if (!instruction) return current;
    contract = &instruction->signal;
    /* An unknown signal set is not an unknown handler-policy transfer.  In
     * particular, a CALL may signal and mutate pointer-bound argument values,
     * but its callee-local COW handler table is discarded on return/unwind.
     * Only an unresolved opcode, or an explicit but unknown policy writer,
     * clobbers the caller frame's policy proof. */
    if (!instruction->op ||
        ((contract->properties & RXOP_SIGNAL_PROP_POLICY_WRITE) &&
         contract->policy_effect == RXOP_POLICY_EFFECT_UNKNOWN))
        return flow_signal_add_policy_version(
                analysis, FLOW_POLICY_CLOBBER, current, instruction->id,
                instruction->block_id, 0, 0, RXOP_POLICY_EFFECT_UNKNOWN, 0);
    if (!(contract->properties & RXOP_SIGNAL_PROP_POLICY_WRITE))
        return current;
    name = 0;
    if (contract->policy_source == RXOP_SIGNAL_SOURCE_STATIC_NAMES)
        name = contract->policy_static_name;
    else if (contract->policy_source ==
             RXOP_SIGNAL_SOURCE_LITERAL_OPERAND) {
        record = rxas_flow_procedure_record(
                analysis->procedure, analysis->metrics.epoch,
                instruction->record_id);
        item = record ? record->queue_record : 0;
        operand = flow_signal_operand(item, contract->policy_source_operand);
        if (operand && operand->token_type == STRING)
            name = (const char *)operand->token_value.string;
    }
    if (!name || !*name)
        return flow_signal_add_policy_version(
                analysis, FLOW_POLICY_CLOBBER, current, instruction->id,
                instruction->block_id, 0, 0, RXOP_POLICY_EFFECT_UNKNOWN, 0);
    return flow_signal_add_policy_version(
            analysis, FLOW_POLICY_WRITE, current, instruction->id,
            instruction->block_id, 0, 0, contract->policy_effect, name);
}

static size_t flow_signal_unknown_effect(
        RxasFlowSignalAnalysis *analysis, RxasFlowEffectClass effect_class,
        size_t parent, size_t instruction_id, size_t block_id) {
    size_t version;
    version = flow_signal_add_effect_version(
            analysis, FLOW_EFFECT_UNKNOWN, effect_class, parent,
            instruction_id, block_id);
    if (version != RXAS_FLOW_ID_NONE)
        analysis->metrics.unknown_edge_states++;
    return version;
}

static int flow_signal_set_edge_state(
        RxasFlowSignalAnalysis *analysis, size_t edge_id,
        size_t block_policy, const size_t *block_effect) {
    const RxasFlowEdge *edge;
    const RxasFlowBlock *source_block;
    const RxasFlowInstruction *instruction;
    size_t policy;
    size_t instruction_id;
    size_t effect_class;
    unsigned int write_mask;
    int failure_edge;
    edge = rxas_flow_procedure_edge(
            analysis->procedure, analysis->metrics.epoch, edge_id);
    if (!edge) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
        return 0;
    }
    source_block = rxas_flow_procedure_block(
            analysis->procedure, analysis->metrics.epoch, edge->source);
    instruction_id = source_block ? source_block->last_instruction
                                  : RXAS_FLOW_ID_NONE;
    instruction = instruction_id == RXAS_FLOW_ID_NONE ? 0
            : rxas_flow_procedure_instruction(
                    analysis->procedure, analysis->metrics.epoch,
                    instruction_id);
    policy = block_policy;
    failure_edge = edge->kind == RXAS_FLOW_EDGE_SIGNAL_SKIP ||
                   edge->kind == RXAS_FLOW_EDGE_SIGNAL_RETRY ||
                   edge->kind == RXAS_FLOW_EDGE_HANDLER ||
                   edge->kind == RXAS_FLOW_EDGE_UNWIND ||
                   edge->kind == RXAS_FLOW_EDGE_TERMINAL;
    write_mask = instruction
            ? flow_signal_instruction_effect_mask(instruction) : 0;
    if (edge->kind == RXAS_FLOW_EDGE_UNKNOWN) {
        policy = flow_signal_add_policy_version(
                analysis, FLOW_POLICY_CLOBBER, block_policy,
                instruction_id, edge->source, 0, 0,
                RXOP_POLICY_EFFECT_UNKNOWN, 0);
    }
    else if (failure_edge && instruction &&
             instruction->signal.state != RXOP_SIGNAL_STATE_NONE) {
        if (instruction->signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES)
            policy = analysis->instruction_policy_before[instruction_id];
        else if (instruction->signal.phase == RXOP_SIGNAL_PHASE_AFTER_WRITES)
            policy = analysis->instruction_policy_after[instruction_id];
        else if ((instruction->signal.properties &
                  RXOP_SIGNAL_PROP_POLICY_WRITE) || !instruction->op)
            policy = flow_signal_add_policy_version(
                    analysis, FLOW_POLICY_CLOBBER,
                    analysis->instruction_policy_before[instruction_id],
                    instruction_id, edge->source, 0, 0,
                    RXOP_POLICY_EFFECT_UNKNOWN, 0);
    }
    if (edge->target == rxas_flow_procedure_normal_exit(
                                analysis->procedure,
                                analysis->metrics.epoch) ||
        edge->target == rxas_flow_procedure_unwind_exit(
                                analysis->procedure,
                                analysis->metrics.epoch))
        policy = analysis->entry_policy;
    if (policy == RXAS_FLOW_ID_NONE) return 0;
    analysis->edge_policy[edge_id] = policy;
    for (effect_class = 0;
         effect_class < RXAS_FLOW_EFFECT_CLASS_COUNT; effect_class++) {
        size_t value;
        value = block_effect[effect_class];
        if (failure_edge && instruction &&
            instruction->signal.state != RXOP_SIGNAL_STATE_NONE) {
            if (instruction->signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES)
                value = analysis->instruction_effect_before[
                        FLOW_EFFECT_INDEX(instruction_id, effect_class)];
            else if (instruction->signal.phase ==
                     RXOP_SIGNAL_PHASE_AFTER_WRITES)
                value = analysis->instruction_effect_after[
                        FLOW_EFFECT_INDEX(instruction_id, effect_class)];
            /* An unknown signal set/phase does not invent a numeric-context
             * write for an otherwise classified opcode.  Possible context
             * writes come from rxop_context_writes(); an exact known failure
             * contract may additionally name failure-only context state. */
            else if ((write_mask & (1u << effect_class)) ||
                     (instruction->signal.state ==
                              RXOP_SIGNAL_STATE_KNOWN &&
                      effect_class == RXAS_FLOW_EFFECT_NUMERIC_CONTEXT &&
                      instruction->signal.failure_context_writes &
                              RXOP_CONTEXT_NUMERIC))
                value = flow_signal_unknown_effect(
                        analysis, (RxasFlowEffectClass)effect_class,
                        analysis->instruction_effect_before[
                                FLOW_EFFECT_INDEX(instruction_id,
                                                  effect_class)],
                        instruction_id, edge->source);
        }
        if (edge->kind == RXAS_FLOW_EDGE_UNKNOWN)
            value = flow_signal_unknown_effect(
                    analysis, (RxasFlowEffectClass)effect_class,
                    block_effect[effect_class], instruction_id, edge->source);
        if (value == RXAS_FLOW_ID_NONE) return 0;
        analysis->edge_effect[
                FLOW_EFFECT_INDEX(edge_id, effect_class)] = value;
    }
    analysis->metrics.edge_states++;
    return flow_signal_consume(analysis, 1);
}

static int flow_signal_block_initial_state(
        RxasFlowSignalAnalysis *analysis, size_t block_id,
        size_t *policy, size_t *effects) {
    const RxasFlowBlock *block;
    size_t incoming_count;
    size_t effect_class;
    size_t incoming_edge;
    block = rxas_flow_procedure_block(
            analysis->procedure, analysis->metrics.epoch, block_id);
    if (!block) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
        return 0;
    }
    incoming_count = analysis->incoming_offsets[block_id + 1] -
                     analysis->incoming_offsets[block_id];
    if (flow_signal_is_root_block(block) || !incoming_count) {
        *policy = analysis->entry_policy;
        for (effect_class = 0;
             effect_class < RXAS_FLOW_EFFECT_CLASS_COUNT; effect_class++)
            effects[effect_class] = analysis->entry_effect[effect_class];
    }
    else if (incoming_count == 1) {
        incoming_edge = analysis->incoming_edges[
                analysis->incoming_offsets[block_id]];
        *policy = analysis->edge_policy[incoming_edge];
        for (effect_class = 0;
             effect_class < RXAS_FLOW_EFFECT_CLASS_COUNT; effect_class++)
            effects[effect_class] = analysis->edge_effect[
                    FLOW_EFFECT_INDEX(incoming_edge, effect_class)];
    }
    else {
        *policy = analysis->block_policy_in[block_id];
        for (effect_class = 0;
             effect_class < RXAS_FLOW_EFFECT_CLASS_COUNT; effect_class++) {
            size_t version;
            version = analysis->block_effect_in[
                    FLOW_EFFECT_INDEX(block_id, effect_class)];
            effects[effect_class] = version ? version
                    : analysis->entry_effect[effect_class];
        }
    }
    if (*policy == RXAS_FLOW_ID_NONE) {
        /* A reachable single-predecessor block should have a forward edge in
         * RPO.  Fail closed if a malformed graph violates that invariant. */
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
        return 0;
    }
    return 1;
}

static int flow_signal_process_block(
        RxasFlowSignalAnalysis *analysis, size_t block_id) {
    const RxasFlowBlock *block;
    size_t policy;
    size_t effects[RXAS_FLOW_EFFECT_CLASS_COUNT];
    size_t record_id;
    size_t edge_id;
    size_t effect_class;
    if (!flow_signal_block_initial_state(
                analysis, block_id, &policy, effects))
        return 0;
    analysis->block_policy_in[block_id] = policy;
    for (effect_class = 0;
         effect_class < RXAS_FLOW_EFFECT_CLASS_COUNT; effect_class++)
        analysis->block_effect_in[
                FLOW_EFFECT_INDEX(block_id, effect_class)] =
                effects[effect_class];
    block = rxas_flow_procedure_block(
            analysis->procedure, analysis->metrics.epoch, block_id);
    if (block->first_record != RXAS_FLOW_ID_NONE) {
        for (record_id = block->first_record;
             record_id <= block->last_record; record_id++) {
            const RxasFlowRecord *record;
            record = rxas_flow_procedure_record(
                    analysis->procedure, analysis->metrics.epoch, record_id);
            if (!record) {
                analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
                return 0;
            }
            if (record->type == TRACE_EVENT) {
                effects[RXAS_FLOW_EFFECT_TRACE] =
                        flow_signal_add_effect_version(
                                analysis, FLOW_EFFECT_WRITE,
                                RXAS_FLOW_EFFECT_TRACE,
                                effects[RXAS_FLOW_EFFECT_TRACE],
                                RXAS_FLOW_ID_NONE, block_id);
                if (effects[RXAS_FLOW_EFFECT_TRACE] == RXAS_FLOW_ID_NONE)
                    return 0;
                analysis->metrics.trace_effect_writes++;
            }
            else if (record->type == OP_CODE &&
                     record->instruction_id != RXAS_FLOW_ID_NONE) {
                const RxasFlowInstruction *instruction;
                unsigned int mask;
                size_t instruction_id;
                instruction_id = record->instruction_id;
                instruction = rxas_flow_procedure_instruction(
                        analysis->procedure, analysis->metrics.epoch,
                        instruction_id);
                if (!instruction) {
                    analysis->metrics.status =
                            RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
                    return 0;
                }
                analysis->instruction_policy_before[instruction_id] = policy;
                for (effect_class = 0;
                     effect_class < RXAS_FLOW_EFFECT_CLASS_COUNT;
                     effect_class++)
                    analysis->instruction_effect_before[
                            FLOW_EFFECT_INDEX(instruction_id, effect_class)] =
                            effects[effect_class];
                policy = flow_signal_policy_transfer(
                        analysis, instruction, policy);
                if (policy == RXAS_FLOW_ID_NONE) return 0;
                mask = flow_signal_instruction_effect_mask(instruction);
                for (effect_class = 0;
                     effect_class < RXAS_FLOW_EFFECT_CLASS_COUNT;
                     effect_class++) {
                    if (mask & (1u << effect_class)) {
                        effects[effect_class] =
                                flow_signal_add_effect_version(
                                        analysis, FLOW_EFFECT_WRITE,
                                        (RxasFlowEffectClass)effect_class,
                                        effects[effect_class], instruction_id,
                                        block_id);
                        if (effects[effect_class] == RXAS_FLOW_ID_NONE)
                            return 0;
                        if (effect_class == RXAS_FLOW_EFFECT_CALL)
                            analysis->metrics.call_effect_writes++;
                    }
                    analysis->instruction_effect_after[
                            FLOW_EFFECT_INDEX(instruction_id, effect_class)] =
                            effects[effect_class];
                }
                analysis->instruction_policy_after[instruction_id] = policy;
            }
            if (!flow_signal_consume(analysis, 1)) return 0;
        }
    }
    analysis->block_policy_out[block_id] = policy;
    for (effect_class = 0;
         effect_class < RXAS_FLOW_EFFECT_CLASS_COUNT; effect_class++)
        analysis->block_effect_out[
                FLOW_EFFECT_INDEX(block_id, effect_class)] =
                effects[effect_class];
    for (edge_id = analysis->outgoing_offsets[block_id];
         edge_id < analysis->outgoing_offsets[block_id + 1]; edge_id++) {
        size_t outgoing_edge;
        outgoing_edge = analysis->outgoing_edges[edge_id];
        if (!flow_signal_set_edge_state(
                    analysis, outgoing_edge, policy, effects))
            return 0;
    }
    return 1;
}

static int flow_signal_fill_phi_inputs(
        RxasFlowSignalAnalysis *analysis) {
    size_t block_id;
    for (block_id = 0; block_id < analysis->block_count; block_id++) {
        size_t offset;
        size_t end;
        offset = analysis->incoming_offsets[block_id];
        end = analysis->incoming_offsets[block_id + 1];
        while (offset < end) {
            size_t edge_id;
            edge_id = analysis->incoming_edges[offset];
            analysis->policy_phi_inputs[offset] =
                    analysis->edge_policy[edge_id];
            if (analysis->policy_phi_inputs[offset] == RXAS_FLOW_ID_NONE) {
                analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
                return 0;
            }
            offset++;
        }
    }
    return flow_signal_consume(analysis, analysis->edge_count);
}

static void flow_signal_set_retained_bytes(
        RxasFlowSignalAnalysis *analysis) {
    size_t bytes;
    bytes = sizeof(*analysis);
    bytes += (analysis->block_count + 1) * sizeof(size_t) * 2;
    bytes += analysis->edge_count * sizeof(size_t) * 4;
    bytes += analysis->block_count * sizeof(size_t) * 2;
    bytes += analysis->instruction_count * sizeof(size_t) * 2;
    bytes += analysis->block_count * RXAS_FLOW_EFFECT_CLASS_COUNT *
             sizeof(size_t) * 2;
    bytes += analysis->instruction_count * RXAS_FLOW_EFFECT_CLASS_COUNT *
             sizeof(size_t) * 2;
    bytes += analysis->edge_count * RXAS_FLOW_EFFECT_CLASS_COUNT *
             sizeof(size_t);
    bytes += analysis->policy_version_capacity *
             sizeof(FlowPolicyVersion);
    bytes += analysis->effect_version_capacity *
             sizeof(FlowEffectVersion);
    analysis->metrics.retained_bytes = bytes;
}

static void flow_signal_free(RxasFlowSignalAnalysis *analysis) {
    if (!analysis) return;
    free(analysis->incoming_offsets);
    free(analysis->incoming_edges);
    free(analysis->outgoing_offsets);
    free(analysis->outgoing_edges);
    free(analysis->policy_phi_inputs);
    free(analysis->block_policy_in);
    free(analysis->block_policy_out);
    free(analysis->instruction_policy_before);
    free(analysis->instruction_policy_after);
    free(analysis->edge_policy);
    free(analysis->block_effect_in);
    free(analysis->block_effect_out);
    free(analysis->instruction_effect_before);
    free(analysis->instruction_effect_after);
    free(analysis->edge_effect);
    free(analysis->policy_versions);
    free(analysis->effect_versions);
    free(analysis);
}

void rxas_flow_signal_analysis_destroy(
        struct RxasFlowSignalAnalysis *analysis) {
    flow_signal_free(analysis);
}

static RxasFlowSignalAnalysis *flow_signal_build(
        RxasFlowProcedure *procedure, unsigned long epoch, size_t budget) {
    RxasFlowSignalAnalysis *analysis;
    const RxasFlowMetrics *metrics;
    size_t rpo_count;
    size_t rpo_index;
    size_t block_id;
    unsigned char *processed;
    analysis = calloc(1, sizeof(*analysis));
    if (!analysis) return 0;
    analysis->procedure = procedure;
    analysis->metrics.status = RXAS_FLOW_ANALYSIS_AVAILABLE;
    analysis->metrics.epoch = epoch;
    analysis->metrics.budget_limit = budget;
    metrics = rxas_flow_procedure_metrics(procedure, epoch);
    if (!metrics) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
        return analysis;
    }
    analysis->block_count = metrics->blocks;
    analysis->edge_count = metrics->edges;
    analysis->instruction_count = metrics->instructions;
    analysis->record_count = metrics->records;
    analysis->structural = rxas_flow_require_structural_analysis(
            procedure, epoch, 0);
    if (!analysis->structural) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
        return analysis;
    }
    if (!flow_signal_build_incoming(analysis) ||
        !flow_signal_allocate_state(analysis) ||
        !flow_signal_scan_active_effects(analysis) ||
        !flow_signal_create_entries_and_phis(analysis))
        return analysis;
    processed = flow_signal_calloc(
            analysis, analysis->block_count, 1);
    if (!processed) return analysis;
    rpo_count = rxas_flow_structural_rpo_count(analysis->structural, epoch);
    for (rpo_index = 0; rpo_index < rpo_count; rpo_index++) {
        block_id = rxas_flow_structural_rpo_block(
                analysis->structural, epoch, rpo_index);
        if (block_id == RXAS_FLOW_ID_NONE ||
            !flow_signal_process_block(analysis, block_id)) {
            free(processed);
            return analysis;
        }
        processed[block_id] = 1;
    }
    /* Unreachable blocks have no trustworthy entry fact.  Give each an
     * explicit inherited-unknown root state so queries fail closed while the
     * edge multiset remains complete and deterministic. */
    for (block_id = 0; block_id < analysis->block_count; block_id++) {
        size_t effect_class;
        size_t edge_id;
        if (processed[block_id]) continue;
        analysis->block_policy_in[block_id] = analysis->entry_policy;
        analysis->block_policy_out[block_id] = analysis->entry_policy;
        for (effect_class = 0;
             effect_class < RXAS_FLOW_EFFECT_CLASS_COUNT; effect_class++) {
            analysis->block_effect_in[
                    FLOW_EFFECT_INDEX(block_id, effect_class)] =
                    analysis->entry_effect[effect_class];
            analysis->block_effect_out[
                    FLOW_EFFECT_INDEX(block_id, effect_class)] =
                    analysis->entry_effect[effect_class];
        }
        for (edge_id = analysis->outgoing_offsets[block_id];
             edge_id < analysis->outgoing_offsets[block_id + 1]; edge_id++) {
            size_t outgoing_edge;
            outgoing_edge = analysis->outgoing_edges[edge_id];
            if (!flow_signal_set_edge_state(
                        analysis, outgoing_edge, analysis->entry_policy,
                        &analysis->block_effect_out[
                                FLOW_EFFECT_INDEX(block_id, 0)])) {
                free(processed);
                return analysis;
            }
        }
    }
    free(processed);
    if (!flow_signal_fill_phi_inputs(analysis)) return analysis;
    flow_signal_set_retained_bytes(analysis);
    return analysis;
}

const RxasFlowSignalAnalysis *rxas_flow_require_signal_analysis(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget) {
    struct RxasFlowAnalysisManager *manager;
    const RxasFlowMetrics *metrics;
    size_t requested_budget;
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch)) return 0;
    metrics = rxas_flow_procedure_metrics(procedure, expected_epoch);
    if (!metrics) return 0;
    requested_budget = work_budget ? work_budget
            : flow_signal_default_budget(metrics);
    manager = procedure->analysis_manager;
    if (manager && manager->epoch == expected_epoch && manager->signal) {
        if (manager->signal->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE)
            return manager->signal;
        if (requested_budget <= manager->signal_budget) return 0;
        flow_signal_free(manager->signal);
        manager->signal = 0;
    }
    if (!manager) {
        manager = calloc(1, sizeof(*manager));
        if (!manager) return 0;
        procedure->analysis_manager = manager;
    }
    manager->epoch = expected_epoch;
    manager->signal_budget = requested_budget;
    manager->signal = flow_signal_build(
            procedure, expected_epoch, requested_budget);
    if (!manager->signal ||
        manager->signal->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE)
        return 0;
    return manager->signal;
}

const RxasFlowSignalMetrics *rxas_flow_last_signal_metrics(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch) {
    const struct RxasFlowAnalysisManager *manager;
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch) ||
        !procedure->analysis_manager)
        return 0;
    manager = procedure->analysis_manager;
    if (manager->epoch != expected_epoch || !manager->signal) return 0;
    return &manager->signal->metrics;
}

static int flow_signal_valid(const RxasFlowSignalAnalysis *analysis,
                             unsigned long epoch) {
    return analysis && epoch && analysis->metrics.epoch == epoch &&
           analysis->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE &&
           rxas_flow_procedure_epoch_matches(analysis->procedure, epoch);
}

const RxasFlowSignalMetrics *rxas_flow_signal_metrics(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch) {
    if (!flow_signal_valid(analysis, expected_epoch)) return 0;
    return &analysis->metrics;
}

static int flow_policy_effect_is_constant(
        RxOpSignalPolicyEffect effect) {
    return effect == RXOP_POLICY_EFFECT_BREAKPOINT_DISABLE ||
           effect == RXOP_POLICY_EFFECT_IGNORE ||
           effect == RXOP_POLICY_EFFECT_HALT ||
           effect == RXOP_POLICY_EFFECT_SILENT_HALT ||
           effect == RXOP_POLICY_EFFECT_RETURN;
}

static int flow_policy_fact_equal(const RxasFlowPolicyFact *left,
                                  const RxasFlowPolicyFact *right) {
    if (left->state != right->state) return 0;
    if (left->state != RXAS_FLOW_POLICY_EXACT) return 1;
    if (left->effect != right->effect) return 0;
    if (flow_policy_effect_is_constant(left->effect)) return 1;
    return left->version_id == right->version_id;
}

/* Returns 1 for a fact, 2 for a pure phi-cycle with no external definition,
 * and 0 for invalid input.  Treating the cycle as deferred (rather than as an
 * immediate clobber) proves loop-invariant policies while still merging every
 * write that enters the cycle. */
static int flow_signal_resolve_policy(
        const RxasFlowSignalAnalysis *analysis, size_t version_id,
        const char *signal_name, unsigned char *visiting,
        RxasFlowPolicyFact *fact) {
    const FlowPolicyVersion *version;
    int result;
    if (version_id >= analysis->policy_version_count) return 0;
    if (visiting[version_id]) return 2;
    visiting[version_id] = 1;
    version = &analysis->policy_versions[version_id];
    memset(fact, 0, sizeof(*fact));
    fact->version_id = version_id;
    fact->defining_instruction = RXAS_FLOW_ID_NONE;
    if (version->kind == FLOW_POLICY_ENTRY) {
        fact->state = RXAS_FLOW_POLICY_INHERITED_UNKNOWN;
        result = 1;
    }
    else if (version->kind == FLOW_POLICY_CLOBBER) {
        fact->state = RXAS_FLOW_POLICY_CLOBBERED;
        result = 1;
    }
    else if (version->kind == FLOW_POLICY_WRITE) {
        if (!flow_signal_name_equal(version->name, signal_name))
            result = flow_signal_resolve_policy(
                    analysis, version->parent, signal_name, visiting, fact);
        else if (version->effect == RXOP_POLICY_EFFECT_PUSH)
            result = flow_signal_resolve_policy(
                    analysis, version->parent, signal_name, visiting, fact);
        else if (version->effect == RXOP_POLICY_EFFECT_POP) {
            fact->state = RXAS_FLOW_POLICY_STACK_UNKNOWN;
            fact->effect = version->effect;
            fact->defining_instruction = version->instruction_id;
            result = 1;
        }
        else if (version->effect ==
                 RXOP_POLICY_EFFECT_BREAKPOINT_ENABLE_EXISTING) {
            fact->state = RXAS_FLOW_POLICY_MERGED_UNKNOWN;
            fact->effect = version->effect;
            fact->defining_instruction = version->instruction_id;
            result = 1;
        }
        else {
            fact->state = RXAS_FLOW_POLICY_EXACT;
            fact->effect = version->effect;
            fact->defining_instruction = version->instruction_id;
            result = 1;
        }
    }
    else {
        RxasFlowPolicyFact merged;
        int have_fact;
        int saw_cycle;
        size_t input_index;
        have_fact = 0;
        saw_cycle = 0;
        memset(&merged, 0, sizeof(merged));
        for (input_index = 0; input_index < version->input_count;
             input_index++) {
            RxasFlowPolicyFact input_fact;
            size_t input_version;
            int input_result;
            input_version = analysis->policy_phi_inputs[
                    version->input_offset + input_index];
            input_result = flow_signal_resolve_policy(
                    analysis, input_version, signal_name, visiting,
                    &input_fact);
            if (input_result == 2) {
                saw_cycle = 1;
                continue;
            }
            if (!input_result) {
                visiting[version_id] = 0;
                return 0;
            }
            if (!have_fact) {
                merged = input_fact;
                have_fact = 1;
            }
            else if (!flow_policy_fact_equal(&merged, &input_fact)) {
                fact->state = RXAS_FLOW_POLICY_MERGED_UNKNOWN;
                fact->version_id = version_id;
                result = 1;
                visiting[version_id] = 0;
                return result;
            }
        }
        if (!have_fact && saw_cycle) result = 2;
        else if (!have_fact) {
            fact->state = RXAS_FLOW_POLICY_MERGED_UNKNOWN;
            result = 1;
        }
        else {
            *fact = merged;
            if (fact->state == RXAS_FLOW_POLICY_EXACT &&
                flow_policy_effect_is_constant(fact->effect)) {
                fact->version_id = version_id;
                fact->defining_instruction = RXAS_FLOW_ID_NONE;
            }
            result = 1;
        }
    }
    visiting[version_id] = 0;
    return result;
}

static int flow_signal_policy_query(
        const RxasFlowSignalAnalysis *analysis, size_t version_id,
        const char *signal_name, RxasFlowPolicyFact *fact) {
    unsigned char *visiting;
    int result;
    if (!signal_name || !fact || version_id >= analysis->policy_version_count)
        return 0;
    visiting = calloc(analysis->policy_version_count
                              ? analysis->policy_version_count : 1, 1);
    if (!visiting) return 0;
    result = flow_signal_resolve_policy(
            analysis, version_id, signal_name, visiting, fact);
    free(visiting);
    if (result == 2) {
        memset(fact, 0, sizeof(*fact));
        fact->state = RXAS_FLOW_POLICY_MERGED_UNKNOWN;
        fact->version_id = version_id;
        fact->defining_instruction = RXAS_FLOW_ID_NONE;
        result = 1;
    }
    return result == 1;
}

int rxas_flow_policy_at_instruction(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch,
        size_t instruction_id, int after_instruction, const char *signal_name,
        RxasFlowPolicyFact *fact) {
    size_t version;
    if (!flow_signal_valid(analysis, expected_epoch) ||
        instruction_id >= analysis->instruction_count)
        return 0;
    version = after_instruction
            ? analysis->instruction_policy_after[instruction_id]
            : analysis->instruction_policy_before[instruction_id];
    return flow_signal_policy_query(analysis, version, signal_name, fact);
}

int rxas_flow_policy_on_edge(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch,
        size_t edge_id, const char *signal_name, RxasFlowPolicyFact *fact) {
    if (!flow_signal_valid(analysis, expected_epoch) ||
        edge_id >= analysis->edge_count)
        return 0;
    return flow_signal_policy_query(
            analysis, analysis->edge_policy[edge_id], signal_name, fact);
}

size_t rxas_flow_effect_at_instruction(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch,
        size_t instruction_id, int after_instruction,
        RxasFlowEffectClass effect_class) {
    if (!flow_signal_valid(analysis, expected_epoch) ||
        instruction_id >= analysis->instruction_count ||
        effect_class < 0 || effect_class >= RXAS_FLOW_EFFECT_CLASS_COUNT)
        return RXAS_FLOW_ID_NONE;
    return (after_instruction ? analysis->instruction_effect_after
                              : analysis->instruction_effect_before)[
            FLOW_EFFECT_INDEX(instruction_id, effect_class)];
}

size_t rxas_flow_effect_on_edge(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch,
        size_t edge_id, RxasFlowEffectClass effect_class) {
    if (!flow_signal_valid(analysis, expected_epoch) ||
        edge_id >= analysis->edge_count || effect_class < 0 ||
        effect_class >= RXAS_FLOW_EFFECT_CLASS_COUNT)
        return RXAS_FLOW_ID_NONE;
    return analysis->edge_effect[FLOW_EFFECT_INDEX(edge_id, effect_class)];
}

size_t rxas_flow_effect_version_count(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch) {
    if (!flow_signal_valid(analysis, expected_epoch)) return 0;
    return analysis->effect_version_count;
}

int rxas_flow_effect_node(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch,
        size_t effect_id, RxasFlowEffectNode *node) {
    const FlowEffectVersion *version;
    size_t input_count;
    if (!node || !flow_signal_valid(analysis, expected_epoch) ||
        effect_id >= analysis->effect_version_count)
        return 0;
    version = &analysis->effect_versions[effect_id];
    input_count = 0;
    if (version->kind == FLOW_EFFECT_PHI &&
        version->block_id < analysis->block_count)
        input_count = analysis->incoming_offsets[version->block_id + 1] -
                      analysis->incoming_offsets[version->block_id];
    memset(node, 0, sizeof(*node));
    node->id = effect_id;
    node->kind = (RxasFlowEffectNodeKind)version->kind;
    node->effect_class = version->effect_class;
    node->parent_id = version->parent;
    node->defining_instruction = version->instruction_id;
    node->block_id = version->block_id;
    node->input_count = input_count;
    return 1;
}

size_t rxas_flow_effect_input(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch,
        size_t effect_id, size_t input_index) {
    const FlowEffectVersion *version;
    size_t incoming_offset;
    size_t incoming_count;
    size_t edge_id;
    if (!flow_signal_valid(analysis, expected_epoch) ||
        effect_id >= analysis->effect_version_count)
        return RXAS_FLOW_ID_NONE;
    version = &analysis->effect_versions[effect_id];
    if (version->kind != FLOW_EFFECT_PHI ||
        version->block_id >= analysis->block_count)
        return RXAS_FLOW_ID_NONE;
    incoming_offset = analysis->incoming_offsets[version->block_id];
    incoming_count = analysis->incoming_offsets[version->block_id + 1] -
                     incoming_offset;
    if (input_index >= incoming_count) return RXAS_FLOW_ID_NONE;
    edge_id = analysis->incoming_edges[incoming_offset + input_index];
    return analysis->edge_effect[
            FLOW_EFFECT_INDEX(edge_id, version->effect_class)];
}

static const char *flow_signal_status_name(RxasFlowAnalysisStatus status) {
    switch (status) {
        case RXAS_FLOW_ANALYSIS_AVAILABLE: return "available";
        case RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED: return "budget-exhausted";
        case RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY: return "out-of-memory";
        case RXAS_FLOW_ANALYSIS_INVALID_GRAPH: return "invalid-graph";
    }
    return "invalid";
}

static const char *flow_policy_kind_name(FlowPolicyVersionKind kind) {
    switch (kind) {
        case FLOW_POLICY_ENTRY: return "entry";
        case FLOW_POLICY_WRITE: return "write";
        case FLOW_POLICY_PHI: return "phi";
        case FLOW_POLICY_CLOBBER: return "clobber";
    }
    return "invalid";
}

int rxas_flow_signal_dump(const RxasFlowSignalAnalysis *analysis,
                          unsigned long expected_epoch, FILE *stream) {
    size_t version_id;
    size_t edge_id;
    if (!stream || !flow_signal_valid(analysis, expected_epoch)) return 0;
    fprintf(stream,
            "PERF3 flow-signal-analysis epoch=%lu status=%s budget=%llu "
            "work=%llu bytes=%llu policy-versions=%llu writes=%llu "
            "phis=%llu clobbers=%llu stack-writes=%llu "
            "effect-versions=%llu effect-phis=%llu trace-writes=%llu "
            "call-writes=%llu edge-states=%llu unknown-edge-states=%llu\n",
            analysis->metrics.epoch,
            flow_signal_status_name(analysis->metrics.status),
            (unsigned long long)analysis->metrics.budget_limit,
            (unsigned long long)analysis->metrics.work,
            (unsigned long long)analysis->metrics.retained_bytes,
            (unsigned long long)analysis->metrics.policy_versions,
            (unsigned long long)analysis->metrics.policy_writes,
            (unsigned long long)analysis->metrics.policy_phis,
            (unsigned long long)analysis->metrics.policy_clobbers,
            (unsigned long long)analysis->metrics.stack_writes,
            (unsigned long long)analysis->metrics.effect_versions,
            (unsigned long long)analysis->metrics.effect_phis,
            (unsigned long long)analysis->metrics.trace_effect_writes,
            (unsigned long long)analysis->metrics.call_effect_writes,
            (unsigned long long)analysis->metrics.edge_states,
            (unsigned long long)analysis->metrics.unknown_edge_states);
    for (version_id = 0; version_id < analysis->policy_version_count;
         version_id++) {
        const FlowPolicyVersion *version;
        size_t input_index;
        version = &analysis->policy_versions[version_id];
        fprintf(stream,
                "PERF3 flow-policy-version id=%llu kind=%s parent=",
                (unsigned long long)version_id,
                flow_policy_kind_name(version->kind));
        if (version->parent == RXAS_FLOW_ID_NONE) fputc('-', stream);
        else fprintf(stream, "%llu", (unsigned long long)version->parent);
        fputs(" instruction=", stream);
        if (version->instruction_id == RXAS_FLOW_ID_NONE) fputc('-', stream);
        else fprintf(stream, "%llu",
                     (unsigned long long)version->instruction_id);
        fputs(" block=", stream);
        if (version->block_id == RXAS_FLOW_ID_NONE) fputc('-', stream);
        else fprintf(stream, "%llu", (unsigned long long)version->block_id);
        fprintf(stream, " effect=%d name=%s inputs=",
                (int)version->effect, version->name ? version->name : "-");
        for (input_index = 0; input_index < version->input_count;
             input_index++) {
            if (input_index) fputc(',', stream);
            fprintf(stream, "%llu",
                    (unsigned long long)analysis->policy_phi_inputs[
                            version->input_offset + input_index]);
        }
        fputc('\n', stream);
    }
    for (version_id = 0; version_id < analysis->effect_version_count;
         version_id++) {
        const FlowEffectVersion *version;
        size_t input;
        size_t input_count;
        version = &analysis->effect_versions[version_id];
        input_count = version->kind == FLOW_EFFECT_PHI &&
                      version->block_id < analysis->block_count
                ? analysis->incoming_offsets[version->block_id + 1] -
                  analysis->incoming_offsets[version->block_id]
                : 0;
        fprintf(stream,
                "PERF3 flow-effect-version id=%llu kind=%d class=%d "
                "parent=",
                (unsigned long long)version_id, (int)version->kind,
                (int)version->effect_class);
        if (version->parent == RXAS_FLOW_ID_NONE) fputc('-', stream);
        else fprintf(stream, "%llu", (unsigned long long)version->parent);
        fputs(" instruction=", stream);
        if (version->instruction_id == RXAS_FLOW_ID_NONE) fputc('-', stream);
        else fprintf(stream, "%llu",
                     (unsigned long long)version->instruction_id);
        fputs(" block=", stream);
        if (version->block_id == RXAS_FLOW_ID_NONE) fputc('-', stream);
        else fprintf(stream, "%llu",
                     (unsigned long long)version->block_id);
        fputs(" inputs=", stream);
        for (input = 0; input < input_count; input++) {
            if (input) fputc(',', stream);
            fprintf(stream, "%llu", (unsigned long long)
                    rxas_flow_effect_input(
                            analysis, expected_epoch, version_id, input));
        }
        fputc('\n', stream);
    }
    for (edge_id = 0; edge_id < analysis->edge_count; edge_id++) {
        const RxasFlowEdge *edge;
        edge = rxas_flow_procedure_edge(
                analysis->procedure, expected_epoch, edge_id);
        fprintf(stream,
                "PERF3 flow-signal-edge id=%llu from=%llu to=%llu kind=%d "
                "policy=%llu effects=",
                (unsigned long long)edge_id,
                (unsigned long long)edge->source,
                (unsigned long long)edge->target, (int)edge->kind,
                (unsigned long long)analysis->edge_policy[edge_id]);
        for (version_id = 0;
             version_id < RXAS_FLOW_EFFECT_CLASS_COUNT; version_id++) {
            if (version_id) fputc(',', stream);
            fprintf(stream, "%llu",
                    (unsigned long long)analysis->edge_effect[
                            FLOW_EFFECT_INDEX(edge_id, version_id)]);
        }
        fputc('\n', stream);
    }
    return 1;
}
