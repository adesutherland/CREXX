/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

/* Sparse symbolic storage and component identities for an immutable RXAS
 * procedure.  Persistent states retain only mapping/component writes; lookup
 * materializes phis lazily and never builds a point-by-register matrix. */

#include "rxas_flow_ssa.h"
#include "rxas_flow_graph_internal.h"
#include "rxasassm.h"

#include <stdlib.h>
#include <string.h>

typedef enum FlowSsaStateKind {
    FLOW_SSA_STATE_ENTRY = 0,
    FLOW_SSA_STATE_TRANSFER,
    FLOW_SSA_STATE_JOIN
} FlowSsaStateKind;

typedef enum FlowMapUpdateKind {
    FLOW_MAP_SET_BASE = 0,
    FLOW_MAP_SET_SITE,
    FLOW_MAP_COPY_REGISTER,
    FLOW_MAP_UNKNOWN
} FlowMapUpdateKind;

typedef struct FlowMapUpdate {
    size_t destination;
    FlowMapUpdateKind kind;
    size_t source_state;
    size_t source_register;
    size_t storage_id;
} FlowMapUpdate;

typedef struct FlowValueUpdate {
    size_t target_state;
    size_t target_register;
    unsigned int component;
    size_t value_id;
} FlowValueUpdate;

typedef struct FlowSsaState {
    FlowSsaStateKind kind;
    size_t parent;
    size_t block_id;
    size_t instruction_id;
    size_t input_offset;
    size_t input_count;
    size_t map_offset;
    size_t map_count;
    size_t value_offset;
    size_t value_count;
    unsigned int flags;
} FlowSsaState;

#define FLOW_SSA_MAP_CLOBBER_ALL 1u
#define FLOW_SSA_MAP_CLOBBER_DYNAMIC 2u
#define FLOW_SSA_VALUE_CLOBBER_ALL 4u
#define FLOW_SSA_VALUE_CLOBBER_DYNAMIC 8u

typedef struct FlowStorageVersion {
    RxasFlowStorageKind kind;
    size_t replacement;
    size_t register_id;
    size_t defining_instruction;
    size_t defining_block;
    size_t join_state;
    size_t input_offset;
    size_t input_count;
} FlowStorageVersion;

typedef struct FlowValueVersion {
    RxasFlowValueKind kind;
    size_t replacement;
    RxasFlowComponentPresence presence;
    unsigned int component;
    size_t defining_instruction;
    size_t source_state;
    size_t source_register;
    unsigned int source_component;
    size_t source_value_id;
    RxOpValueDerivation derivation;
    unsigned int signal_dependencies;
    size_t definition_effects[RXAS_FLOW_EFFECT_CLASS_COUNT];
    size_t definition_numeric_context;
    const Assembler_Token *constant_token;
    size_t input_offset;
    size_t input_count;
} FlowValueVersion;

typedef struct FlowStorageCacheEntry {
    size_t state_id;
    size_t register_id;
    size_t result;
    size_t next;
    unsigned char resolving;
} FlowStorageCacheEntry;

typedef struct FlowValueCacheEntry {
    size_t state_id;
    size_t storage_id;
    unsigned int component;
    size_t result;
    size_t next;
    unsigned char resolving;
} FlowValueCacheEntry;

typedef struct FlowBuildMapUpdate {
    size_t destination;
    FlowMapUpdateKind kind;
    size_t source_register;
    size_t storage_id;
} FlowBuildMapUpdate;

typedef struct FlowBuildValueUpdate {
    size_t target_register;
    unsigned int component;
    RxasFlowValueKind kind;
    RxasFlowComponentPresence presence;
    size_t source_register;
    unsigned int source_component;
    RxOpValueDerivation derivation;
    const Assembler_Token *constant_token;
} FlowBuildValueUpdate;

struct RxasFlowSsaAnalysis {
    const RxasFlowProcedure *procedure;
    const RxasFlowStructuralAnalysis *structural;
    const RxasFlowSignalAnalysis *signal;
    RxasFlowSsaMetrics metrics;
    size_t block_count;
    size_t edge_count;
    size_t instruction_count;
    size_t record_count;
    RxasFlowRegister *registers;
    size_t register_count;
    size_t register_capacity;
    FlowSsaState *states;
    size_t state_count;
    size_t state_capacity;
    FlowMapUpdate *map_updates;
    size_t map_update_count;
    size_t map_update_capacity;
    FlowValueUpdate *value_updates;
    size_t value_update_count;
    size_t value_update_capacity;
    size_t *state_inputs;
    size_t state_input_count;
    size_t state_input_capacity;
    size_t *incoming_offsets;
    size_t *incoming_edges;
    size_t *outgoing_offsets;
    size_t *outgoing_edges;
    size_t *block_state;
    size_t *instruction_before;
    size_t *instruction_after;
    size_t *edge_state;
    FlowStorageVersion *storage_versions;
    size_t storage_version_count;
    size_t storage_version_capacity;
    size_t *storage_dynamic_marks;
    size_t storage_dynamic_mark_capacity;
    size_t storage_dynamic_generation;
    size_t *storage_inputs;
    size_t storage_input_count;
    size_t storage_input_capacity;
    FlowValueVersion *value_versions;
    size_t value_version_count;
    size_t value_version_capacity;
    size_t *value_inputs;
    size_t value_input_count;
    size_t value_input_capacity;
    size_t *storage_buckets;
    size_t storage_bucket_count;
    FlowStorageCacheEntry *storage_cache;
    size_t storage_cache_count;
    size_t storage_cache_capacity;
    size_t *value_buckets;
    size_t value_bucket_count;
    FlowValueCacheEntry *value_cache;
    size_t value_cache_count;
    size_t value_cache_capacity;
    size_t unmapped_value_plus_one[7];
    size_t entry_state;
    size_t unknown_state;
};

static int flow_ssa_consume(RxasFlowSsaAnalysis *analysis, size_t amount) {
    size_t remaining;
    if (!analysis || analysis->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE)
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

static void *flow_ssa_calloc(RxasFlowSsaAnalysis *analysis,
                             size_t count, size_t size) {
    void *memory;
    if (!count) count = 1;
    if (size && count > ((size_t)-1) / size) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    memory = calloc(count, size);
    if (!memory) analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
    return memory;
}

static int flow_ssa_grow(RxasFlowSsaAnalysis *analysis, void **items,
                         size_t *capacity, size_t count, size_t item_size,
                         size_t minimum) {
    size_t next;
    void *resized;
    if (count + minimum <= *capacity) return 1;
    next = *capacity ? *capacity * 2 : 16;
    while (next < count + minimum) {
        if (next > (size_t)-1 / 2) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        next *= 2;
    }
    if (item_size && next > (size_t)-1 / item_size) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    resized = realloc(*items, next * item_size);
    if (!resized) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    *items = resized;
    *capacity = next;
    return 1;
}

static size_t flow_ssa_default_budget(const RxasFlowMetrics *metrics,
                                      const RxasFlowProcedure *procedure) {
    size_t scale;
    if (!metrics || metrics->records > (size_t)-1 - metrics->instructions ||
        metrics->records + metrics->instructions >
                (size_t)-1 - metrics->blocks ||
        metrics->records + metrics->instructions + metrics->blocks >
                (size_t)-1 - metrics->edges - 1)
        return (size_t)-1;
    scale = metrics->records + metrics->instructions + metrics->blocks +
            metrics->edges + 1;
    if (procedure &&
        (procedure->local_count > (size_t)-1 - scale ||
         procedure->global_count >
                (size_t)-1 - scale - procedure->local_count))
        return (size_t)-1;
    if (procedure)
        scale += procedure->local_count + procedure->global_count;
    if (scale > ((size_t)-1 - 8192) / 2048) return (size_t)-1;
    return scale * 2048 + 8192;
}

static RxasFlowRegisterClass flow_register_class(int token_type) {
    if (token_type == AREG) return RXAS_FLOW_REGISTER_ARGUMENT;
    if (token_type == GREG) return RXAS_FLOW_REGISTER_GLOBAL;
    return RXAS_FLOW_REGISTER_LOCAL;
}

static int flow_token_is_register(const Assembler_Token *token) {
    return token && (token->token_type == RREG || token->token_type == AREG ||
                     token->token_type == GREG) &&
           token->token_value.integer >= 0;
}

static Assembler_Token *flow_ssa_operand(const instruction_queue *item,
                                         size_t operand_index) {
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

static int flow_register_compare(const void *left, const void *right) {
    const RxasFlowRegister *lreg;
    const RxasFlowRegister *rreg;
    lreg = (const RxasFlowRegister *)left;
    rreg = (const RxasFlowRegister *)right;
    if (lreg->register_class != rreg->register_class)
        return lreg->register_class < rreg->register_class ? -1 : 1;
    if (lreg->number == rreg->number) return 0;
    return lreg->number < rreg->number ? -1 : 1;
}

static int flow_ssa_add_register(RxasFlowSsaAnalysis *analysis,
                                 RxasFlowRegister reg) {
    size_t index;
    for (index = 0; index < analysis->register_count; index++)
        if (analysis->registers[index].register_class == reg.register_class &&
            analysis->registers[index].number == reg.number)
            return 1;
    if (!flow_ssa_grow(analysis, (void **)&analysis->registers,
                       &analysis->register_capacity, analysis->register_count,
                       sizeof(*analysis->registers), 1))
        return 0;
    analysis->registers[analysis->register_count++] = reg;
    return flow_ssa_consume(analysis, 1);
}

static size_t flow_ssa_register_id(const RxasFlowSsaAnalysis *analysis,
                                   RxasFlowRegister reg) {
    size_t left;
    size_t right;
    left = 0;
    right = analysis->register_count;
    while (left < right) {
        size_t middle;
        const RxasFlowRegister *candidate;
        middle = left + (right - left) / 2;
        candidate = &analysis->registers[middle];
        if (candidate->register_class < reg.register_class ||
            (candidate->register_class == reg.register_class &&
             candidate->number < reg.number))
            left = middle + 1;
        else right = middle;
    }
    if (left < analysis->register_count &&
        analysis->registers[left].register_class == reg.register_class &&
        analysis->registers[left].number == reg.number)
        return left;
    return RXAS_FLOW_ID_NONE;
}

static size_t flow_ssa_operand_register(const RxasFlowSsaAnalysis *analysis,
                                        const instruction_queue *item,
                                        size_t operand_index) {
    Assembler_Token *token;
    RxasFlowRegister reg;
    token = flow_ssa_operand(item, operand_index);
    if (!flow_token_is_register(token)) return RXAS_FLOW_ID_NONE;
    reg.register_class = flow_register_class(token->token_type);
    reg.number = (size_t)token->token_value.integer;
    return flow_ssa_register_id(analysis, reg);
}

static size_t flow_ssa_numbered_register(
        const RxasFlowSsaAnalysis *analysis, RxasFlowRegisterClass reg_class,
        const Assembler_Token *token) {
    RxasFlowRegister reg;
    if (!token || token->token_type != INT || token->token_value.integer < 0)
        return RXAS_FLOW_ID_NONE;
    reg.register_class = reg_class;
    reg.number = (size_t)token->token_value.integer;
    return flow_ssa_register_id(analysis, reg);
}

static int flow_ssa_collect_registers(RxasFlowSsaAnalysis *analysis) {
    size_t index;
    RxasFlowRegister reg;
    for (index = 0; index < analysis->procedure->local_count; index++) {
        reg.register_class = RXAS_FLOW_REGISTER_LOCAL;
        reg.number = index;
        if (!flow_ssa_add_register(analysis, reg)) return 0;
    }
    for (index = 0; index < analysis->procedure->global_count; index++) {
        reg.register_class = RXAS_FLOW_REGISTER_GLOBAL;
        reg.number = index;
        if (!flow_ssa_add_register(analysis, reg)) return 0;
    }
    for (index = 0; index < analysis->record_count; index++) {
        const RxasFlowRecord *record;
        const instruction_queue *item;
        size_t operand;
        record = rxas_flow_procedure_record(
                analysis->procedure, analysis->metrics.epoch, index);
        if (!record) return 0;
        item = record->queue_record;
        if (record->type == OP_CODE && item) {
            for (operand = 0; operand < item->operandCount; operand++) {
                Assembler_Token *token;
                token = flow_ssa_operand(item, operand);
                if (!flow_token_is_register(token)) continue;
                reg.register_class = flow_register_class(token->token_type);
                reg.number = (size_t)token->token_value.integer;
                if (!flow_ssa_add_register(analysis, reg)) return 0;
            }
            if (record->instruction_id != RXAS_FLOW_ID_NONE) {
                const RxasFlowInstruction *instruction;
                instruction = rxas_flow_procedure_instruction(
                        analysis->procedure, analysis->metrics.epoch,
                        record->instruction_id);
                if (instruction && instruction->effects.implicit ==
                        RXOP_IMPLICIT_ARGUMENT_INDEX && item->operand2Token &&
                    item->operand2Token->token_type == INT &&
                    item->operand2Token->token_value.integer >= 0) {
                    reg.register_class = RXAS_FLOW_REGISTER_ARGUMENT;
                    reg.number = (size_t)item->operand2Token->token_value.integer;
                    if (!flow_ssa_add_register(analysis, reg)) return 0;
                }
            }
        }
    }
    qsort(analysis->registers, analysis->register_count,
          sizeof(*analysis->registers), flow_register_compare);
    analysis->metrics.registers = analysis->register_count;
    return 1;
}

static size_t flow_ssa_add_state(RxasFlowSsaAnalysis *analysis,
                                 FlowSsaState state) {
    size_t id;
    if (!flow_ssa_consume(analysis, 1) ||
        !flow_ssa_grow(analysis, (void **)&analysis->states,
                       &analysis->state_capacity, analysis->state_count,
                       sizeof(*analysis->states), 1))
        return RXAS_FLOW_ID_NONE;
    id = analysis->state_count++;
    analysis->states[id] = state;
    analysis->metrics.states++;
    if (state.kind == FLOW_SSA_STATE_JOIN) analysis->metrics.join_states++;
    if (state.flags & (FLOW_SSA_MAP_CLOBBER_ALL |
                       FLOW_SSA_MAP_CLOBBER_DYNAMIC))
        analysis->metrics.mapping_clobbers++;
    return id;
}

static size_t flow_ssa_add_storage_version(
        RxasFlowSsaAnalysis *analysis, FlowStorageVersion version) {
    size_t id;
    if (!flow_ssa_consume(analysis, 1) ||
        !flow_ssa_grow(analysis, (void **)&analysis->storage_versions,
                       &analysis->storage_version_capacity,
                       analysis->storage_version_count,
                       sizeof(*analysis->storage_versions), 1))
        return RXAS_FLOW_ID_NONE;
    id = analysis->storage_version_count++;
    analysis->storage_versions[id] = version;
    analysis->metrics.storage_versions++;
    if (version.kind == RXAS_FLOW_STORAGE_SITE)
        analysis->metrics.storage_sites++;
    if (version.kind == RXAS_FLOW_STORAGE_PHI)
        analysis->metrics.storage_phis++;
    return id;
}

static size_t flow_ssa_add_value_version(
        RxasFlowSsaAnalysis *analysis, FlowValueVersion version) {
    size_t id;
    if (!flow_ssa_consume(analysis, 1) ||
        !flow_ssa_grow(analysis, (void **)&analysis->value_versions,
                       &analysis->value_version_capacity,
                       analysis->value_version_count,
                       sizeof(*analysis->value_versions), 1))
        return RXAS_FLOW_ID_NONE;
    id = analysis->value_version_count++;
    analysis->value_versions[id] = version;
    analysis->metrics.value_versions++;
    if (version.kind == RXAS_FLOW_VALUE_PHI) analysis->metrics.value_phis++;
    if (version.kind == RXAS_FLOW_VALUE_ABSENT) analysis->metrics.absent_values++;
    if (version.kind == RXAS_FLOW_VALUE_CONSTANT) analysis->metrics.constant_values++;
    if (version.kind == RXAS_FLOW_VALUE_DERIVED) analysis->metrics.derived_values++;
    if (version.kind == RXAS_FLOW_VALUE_UNKNOWN) analysis->metrics.unknown_values++;
    return id;
}

static int flow_ssa_build_adjacency(RxasFlowSsaAnalysis *analysis) {
    size_t *incoming_fill;
    size_t *outgoing_fill;
    size_t edge_id;
    size_t block;
    analysis->incoming_offsets = flow_ssa_calloc(
            analysis, analysis->block_count + 1, sizeof(size_t));
    analysis->outgoing_offsets = flow_ssa_calloc(
            analysis, analysis->block_count + 1, sizeof(size_t));
    analysis->incoming_edges = flow_ssa_calloc(
            analysis, analysis->edge_count, sizeof(size_t));
    analysis->outgoing_edges = flow_ssa_calloc(
            analysis, analysis->edge_count, sizeof(size_t));
    incoming_fill = flow_ssa_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    outgoing_fill = flow_ssa_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    if (!analysis->incoming_offsets || !analysis->outgoing_offsets ||
        !analysis->incoming_edges || !analysis->outgoing_edges ||
        !incoming_fill || !outgoing_fill) {
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
    }
    for (block = 1; block <= analysis->block_count; block++) {
        analysis->incoming_offsets[block] += analysis->incoming_offsets[block - 1];
        analysis->outgoing_offsets[block] += analysis->outgoing_offsets[block - 1];
    }
    for (edge_id = 0; edge_id < analysis->edge_count; edge_id++) {
        const RxasFlowEdge *edge;
        size_t offset;
        edge = rxas_flow_procedure_edge(
                analysis->procedure, analysis->metrics.epoch, edge_id);
        offset = analysis->incoming_offsets[edge->target] +
                 incoming_fill[edge->target]++;
        analysis->incoming_edges[offset] = edge_id;
        offset = analysis->outgoing_offsets[edge->source] +
                 outgoing_fill[edge->source]++;
        analysis->outgoing_edges[offset] = edge_id;
    }
    free(incoming_fill);
    free(outgoing_fill);
    if (!flow_ssa_consume(analysis, analysis->edge_count)) return 0;
    return flow_ssa_consume(analysis, analysis->edge_count);
}

static int flow_ssa_append_map_updates(
        RxasFlowSsaAnalysis *analysis, const FlowBuildMapUpdate *updates,
        size_t count, size_t source_state, size_t *offset) {
    size_t index;
    *offset = analysis->map_update_count;
    if (!count) return 1;
    if (!flow_ssa_grow(analysis, (void **)&analysis->map_updates,
                       &analysis->map_update_capacity,
                       analysis->map_update_count,
                       sizeof(*analysis->map_updates), count))
        return 0;
    for (index = 0; index < count; index++) {
        FlowMapUpdate *target;
        target = &analysis->map_updates[analysis->map_update_count++];
        target->destination = updates[index].destination;
        target->kind = updates[index].kind;
        target->source_state = source_state;
        target->source_register = updates[index].source_register;
        target->storage_id = updates[index].storage_id;
    }
    analysis->metrics.mapping_updates += count;
    return flow_ssa_consume(analysis, count);
}

static size_t flow_ssa_component_source(
        RxOpValueDerivation derivation) {
    if (derivation == RXOP_DERIVATION_INTEGER_TO_FLOAT ||
        derivation == RXOP_DERIVATION_INTEGER_TO_STRING)
        return RXOP_COMPONENT_INTEGER;
    if (derivation == RXOP_DERIVATION_FLOAT_TO_STRING)
        return RXOP_COMPONENT_FLOAT;
    if (derivation == RXOP_DERIVATION_DECIMAL_TO_STRING)
        return RXOP_COMPONENT_DECIMAL;
    return RXOP_COMPONENT_NONE;
}

static unsigned int flow_ssa_effect_dependency(
        RxasFlowEffectClass effect_class) {
    if (effect_class == RXAS_FLOW_EFFECT_NUMERIC_CONTEXT)
        return RXOP_SIGNAL_DEP_NUMERIC_CONTEXT;
    if (effect_class == RXAS_FLOW_EFFECT_PLUGIN)
        return RXOP_SIGNAL_DEP_PLUGIN;
    if (effect_class == RXAS_FLOW_EFFECT_LOCALE)
        return RXOP_SIGNAL_DEP_LOCALE;
    if (effect_class == RXAS_FLOW_EFFECT_EXTERNAL)
        return RXOP_SIGNAL_DEP_EXTERNAL_STATE;
    return RXOP_SIGNAL_DEP_NONE;
}

static int flow_ssa_append_value_updates(
        RxasFlowSsaAnalysis *analysis, const FlowBuildValueUpdate *updates,
        size_t count, size_t source_state, size_t instruction_id,
        size_t *offset) {
    size_t index;
    *offset = analysis->value_update_count;
    if (!count) return 1;
    if (!flow_ssa_grow(analysis, (void **)&analysis->value_updates,
                       &analysis->value_update_capacity,
                       analysis->value_update_count,
                       sizeof(*analysis->value_updates), count))
        return 0;
    for (index = 0; index < count; index++) {
        FlowValueVersion version;
        FlowValueUpdate *target;
        size_t value_id;
        size_t effect;
        const RxasFlowInstruction *instruction;
        memset(&version, 0, sizeof(version));
        version.kind = updates[index].kind;
        version.replacement = RXAS_FLOW_ID_NONE;
        version.presence = updates[index].presence;
        version.component = updates[index].component;
        version.defining_instruction = instruction_id;
        version.source_state = source_state;
        version.source_register = updates[index].source_register;
        version.source_component = updates[index].source_component;
        version.source_value_id = RXAS_FLOW_ID_NONE;
        version.derivation = updates[index].derivation;
        version.constant_token = updates[index].constant_token;
        instruction = rxas_flow_procedure_instruction(
                analysis->procedure, analysis->metrics.epoch, instruction_id);
        version.signal_dependencies =
                updates[index].kind == RXAS_FLOW_VALUE_DERIVED && instruction
                ? instruction->signal.dependencies : RXOP_SIGNAL_DEP_NONE;
        for (effect = 0; effect < RXAS_FLOW_EFFECT_CLASS_COUNT; effect++) {
            version.definition_effects[effect] = RXAS_FLOW_ID_NONE;
            if (updates[index].kind == RXAS_FLOW_VALUE_DERIVED &&
                (version.signal_dependencies & flow_ssa_effect_dependency(
                        (RxasFlowEffectClass)effect)))
                version.definition_effects[effect] =
                        rxas_flow_effect_at_instruction(
                                analysis->signal, analysis->metrics.epoch,
                                instruction_id, 0,
                                (RxasFlowEffectClass)effect);
        }
        version.definition_numeric_context = version.definition_effects[
                RXAS_FLOW_EFFECT_NUMERIC_CONTEXT];
        value_id = flow_ssa_add_value_version(analysis, version);
        if (value_id == RXAS_FLOW_ID_NONE) return 0;
        target = &analysis->value_updates[analysis->value_update_count++];
        target->target_state = source_state;
        target->target_register = updates[index].target_register;
        target->component = updates[index].component;
        target->value_id = value_id;
    }
    analysis->metrics.component_updates += count;
    return flow_ssa_consume(analysis, count);
}

static int flow_build_map_add(FlowBuildMapUpdate *updates, size_t *count,
                              size_t capacity, size_t destination,
                              FlowMapUpdateKind kind, size_t source,
                              size_t storage) {
    if (destination == RXAS_FLOW_ID_NONE || *count >= capacity) return 0;
    updates[*count].destination = destination;
    updates[*count].kind = kind;
    updates[*count].source_register = source;
    updates[*count].storage_id = storage;
    (*count)++;
    return 1;
}

static int flow_build_value_add(FlowBuildValueUpdate *updates, size_t *count,
                                size_t capacity, size_t target,
                                unsigned int component,
                                RxasFlowValueKind kind,
                                RxasFlowComponentPresence presence,
                                size_t source, unsigned int source_component,
                                RxOpValueDerivation derivation,
                                const Assembler_Token *constant_token) {
    if (target == RXAS_FLOW_ID_NONE || !component || *count >= capacity)
        return 0;
    updates[*count].target_register = target;
    updates[*count].component = component;
    updates[*count].kind = kind;
    updates[*count].presence = presence;
    updates[*count].source_register = source;
    updates[*count].source_component = source_component;
    updates[*count].derivation = derivation;
    updates[*count].constant_token = constant_token;
    (*count)++;
    return 1;
}

static size_t flow_ssa_create_site(RxasFlowSsaAnalysis *analysis,
                                   size_t instruction_id,
                                   size_t block_id, size_t register_id) {
    FlowStorageVersion version;
    size_t index;
    memset(&version, 0, sizeof(version));
    version.kind = RXAS_FLOW_STORAGE_SITE;
    version.replacement = RXAS_FLOW_ID_NONE;
    version.register_id = register_id;
    version.defining_instruction = instruction_id;
    version.defining_block = block_id;
    version.join_state = RXAS_FLOW_ID_NONE;
    index = flow_ssa_add_storage_version(analysis, version);
    return index == RXAS_FLOW_ID_NONE ? RXAS_FLOW_ID_NONE : index + 1;
}

static int flow_ssa_is_plain_mapping_opcode(int opcode) {
    return opcode == OP_LINK_REG_REG || opcode == OP_LINKARG_REG_INT ||
           opcode == OP_LINKARG_REG_REG_INT ||
           opcode == OP_METALINKPREG_REG_REG ||
           opcode == OP_LINKATTR_REG_REG_REG ||
           opcode == OP_LINKATTR_REG_REG_INT ||
           opcode == OP_LINKATTR1_REG_REG_REG ||
           opcode == OP_LINKATTR1_REG_REG_INT ||
           opcode == OP_LINKREF_REG_REG || opcode == OP_SWAP_REG_REG ||
           opcode == OP_SWAPN_REG_REG_REG_REG ||
           opcode == OP_SWAPN_REG_REG_REG_REG_REG_REG ||
           opcode == OP_SWAPN_REG_REG_REG_REG_REG_REG_REG_REG ||
           opcode == OP_UNLINK_REG || opcode == OP_UNLINKN_REG_REG ||
           opcode == OP_UNLINKBR_REG_ID;
}

static void flow_ssa_add_swap(const RxasFlowSsaAnalysis *analysis,
                              const instruction_queue *item,
                              FlowBuildMapUpdate *updates, size_t *count,
                              size_t capacity, size_t left, size_t right) {
    size_t lreg;
    size_t rreg;
    lreg = flow_ssa_operand_register(analysis, item, left);
    rreg = flow_ssa_operand_register(analysis, item, right);
    flow_build_map_add(updates, count, capacity, lreg,
                       FLOW_MAP_COPY_REGISTER, rreg, 0);
    flow_build_map_add(updates, count, capacity, rreg,
                       FLOW_MAP_COPY_REGISTER, lreg, 0);
}

static unsigned int flow_component_bits[] = {
    RXOP_COMPONENT_INTEGER, RXOP_COMPONENT_FLOAT, RXOP_COMPONENT_STRING,
    RXOP_COMPONENT_DECIMAL, RXOP_COMPONENT_BINARY,
    RXOP_COMPONENT_ATTRIBUTES, RXOP_COMPONENT_REFERENCE
};

static size_t flow_ssa_build_normal_transfer(
        RxasFlowSsaAnalysis *analysis, const RxasFlowInstruction *instruction,
        size_t input_state) {
    const RxasFlowRecord *record;
    const instruction_queue *item;
    FlowBuildMapUpdate *map;
    FlowBuildValueUpdate *values;
    size_t map_capacity;
    size_t value_capacity;
    size_t map_count;
    size_t value_count;
    size_t operand;
    size_t map_offset;
    size_t value_offset;
    unsigned int flags;
    int opcode;
    FlowSsaState state;
    record = rxas_flow_procedure_record(
            analysis->procedure, analysis->metrics.epoch,
            instruction->record_id);
    item = record ? record->queue_record : 0;
    if (!item) return input_state;
    map_capacity = item->operandCount * 2 + 8;
    value_capacity = (item->operandCount + 2) * 7;
    map = flow_ssa_calloc(analysis, map_capacity, sizeof(*map));
    values = flow_ssa_calloc(analysis, value_capacity, sizeof(*values));
    if (!map || !values) {
        free(map);
        free(values);
        return RXAS_FLOW_ID_NONE;
    }
    map_count = 0;
    value_count = 0;
    flags = 0;
    opcode = instruction->op ? instruction->op->opcode : -1;
    if (!instruction->op ||
        instruction->effects.state != RXOP_EFFECT_CLASSIFIED) {
        flags = FLOW_SSA_MAP_CLOBBER_ALL | FLOW_SSA_VALUE_CLOBBER_ALL;
    }
    else {
        size_t first;
        size_t second;
        size_t site;
        switch (opcode) {
            case OP_LINK_REG_REG:
                first = flow_ssa_operand_register(analysis, item, 0);
                second = flow_ssa_operand_register(analysis, item, 1);
                flow_build_map_add(map, &map_count, map_capacity, first,
                                   FLOW_MAP_COPY_REGISTER, second, 0);
                break;
            case OP_LINKARG_REG_INT:
                first = flow_ssa_operand_register(analysis, item, 0);
                second = RXAS_FLOW_ID_NONE;
                if (item->operand2Token && item->operand2Token->token_type == INT &&
                    item->operand2Token->token_value.integer >= 0) {
                    RxasFlowRegister argument;
                    argument.register_class = RXAS_FLOW_REGISTER_ARGUMENT;
                    argument.number = (size_t)item->operand2Token->token_value.integer;
                    second = flow_ssa_register_id(analysis, argument);
                }
                if (second == RXAS_FLOW_ID_NONE) {
                    site = flow_ssa_create_site(analysis, instruction->id,
                                                instruction->block_id, first);
                    flow_build_map_add(map, &map_count, map_capacity, first,
                                       FLOW_MAP_SET_SITE, 0, site);
                }
                else flow_build_map_add(map, &map_count, map_capacity, first,
                                        FLOW_MAP_SET_BASE, 0, second + 1);
                break;
            case OP_LINKATTR_REG_REG_REG:
            case OP_LINKATTR_REG_REG_INT:
            case OP_LINKATTR1_REG_REG_REG:
            case OP_LINKATTR1_REG_REG_INT:
            case OP_LINKREF_REG_REG:
            case OP_LINKARG_REG_REG_INT:
            case OP_METALINKPREG_REG_REG:
                first = flow_ssa_operand_register(analysis, item, 0);
                site = flow_ssa_create_site(analysis, instruction->id,
                                            instruction->block_id, first);
                flow_build_map_add(map, &map_count, map_capacity, first,
                                   FLOW_MAP_SET_SITE, 0, site);
                break;
            case OP_SETLINKATTR1_REG_REG_INT_REG:
            case OP_SETLINKATTR1_REG_REG_INT_REG_INT:
            case OP_MINLINKATTR1_REG_REG_INT:
            case OP_MINLINKATTR1_REG_REG_REG_INT:
            case OP_SETLINKILOAD_REG_REG_INT_REG_REG_INT:
                flags |= FLOW_SSA_MAP_CLOBBER_DYNAMIC |
                         FLOW_SSA_VALUE_CLOBBER_DYNAMIC;
                first = flow_ssa_operand_register(analysis, item, 0);
                site = flow_ssa_create_site(analysis, instruction->id,
                                            instruction->block_id, first);
                flow_build_map_add(map, &map_count, map_capacity, first,
                                   FLOW_MAP_SET_SITE, 0, site);
                break;
            case OP_LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT:
                flags |= FLOW_SSA_MAP_CLOBBER_DYNAMIC |
                         FLOW_SSA_VALUE_CLOBBER_DYNAMIC;
                for (operand = 0; operand <= 4; operand += 4) {
                    first = flow_ssa_operand_register(analysis, item, operand);
                    site = flow_ssa_create_site(analysis, instruction->id,
                                                instruction->block_id, first);
                    flow_build_map_add(map, &map_count, map_capacity, first,
                                       FLOW_MAP_SET_SITE, 0, site);
                }
                break;
            case OP_SWAP_REG_REG:
                flow_ssa_add_swap(analysis, item, map, &map_count,
                                  map_capacity, 0, 1);
                break;
            case OP_SWAPN_REG_REG_REG_REG:
                flow_ssa_add_swap(analysis, item, map, &map_count,
                                  map_capacity, 0, 1);
                flow_ssa_add_swap(analysis, item, map, &map_count,
                                  map_capacity, 2, 3);
                break;
            case OP_SWAPN_REG_REG_REG_REG_REG_REG:
                for (operand = 0; operand < 6; operand += 2)
                    flow_ssa_add_swap(analysis, item, map, &map_count,
                                      map_capacity, operand, operand + 1);
                break;
            case OP_SWAPN_REG_REG_REG_REG_REG_REG_REG_REG:
                for (operand = 0; operand < 8; operand += 2)
                    flow_ssa_add_swap(analysis, item, map, &map_count,
                                      map_capacity, operand, operand + 1);
                break;
            case OP_SETTPSWAP_REG_INT_REG:
                flow_ssa_add_swap(analysis, item, map, &map_count,
                                  map_capacity, 0, 2);
                break;
            case OP_LOADSETTPSWAP_REG_INT_REG_INT_REG:
                flow_ssa_add_swap(analysis, item, map, &map_count,
                                  map_capacity, 2, 4);
                break;
            case OP_SWAPSETTP_REG_REG_REG_INT:
                flow_ssa_add_swap(analysis, item, map, &map_count,
                                  map_capacity, 0, 1);
                break;
            case OP_SWAPSETTPSWAP_REG_REG_REG_INT_REG:
                flow_ssa_add_swap(analysis, item, map, &map_count,
                                  map_capacity, 0, 1);
                flow_ssa_add_swap(analysis, item, map, &map_count,
                                  map_capacity, 2, 4);
                break;
            case OP_SETTPSWAPSETTPSWAP_REG_INT_REG_REG_REG:
                flow_ssa_add_swap(analysis, item, map, &map_count,
                                  map_capacity, 0, 2);
                flow_ssa_add_swap(analysis, item, map, &map_count,
                                  map_capacity, 3, 4);
                break;
            case OP_SWAPCALL_REG_FUNC_REG_REG_REG:
                flow_ssa_add_swap(analysis, item, map, &map_count,
                                  map_capacity, 3, 4);
                flags |= FLOW_SSA_MAP_CLOBBER_DYNAMIC |
                         FLOW_SSA_VALUE_CLOBBER_DYNAMIC;
                break;
            case OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG:
                flow_ssa_add_swap(analysis, item, map, &map_count,
                                  map_capacity, 3, 5);
                flags |= FLOW_SSA_MAP_CLOBBER_DYNAMIC |
                         FLOW_SSA_VALUE_CLOBBER_DYNAMIC;
                break;
            case OP_UNLINK_REG:
            case OP_UNLINKBR_REG_ID:
                first = flow_ssa_operand_register(analysis, item, 0);
                flow_build_map_add(map, &map_count, map_capacity, first,
                                   FLOW_MAP_SET_BASE, 0, first + 1);
                break;
            case OP_UNLINKN_REG_REG:
                for (operand = 0; operand < 2; operand++) {
                    first = flow_ssa_operand_register(analysis, item, operand);
                    flow_build_map_add(map, &map_count, map_capacity, first,
                                       FLOW_MAP_SET_BASE, 0, first + 1);
                }
                break;
            case OP_ISETUNLINK_REG_REG:
            case OP_ILOADSETUNLINK_REG_INT:
                first = flow_ssa_operand_register(analysis, item, 0);
                flow_build_map_add(map, &map_count, map_capacity, first,
                                   FLOW_MAP_SET_BASE, 0, first + 1);
                break;
            case OP_IGETUNLINK_REG_REG:
                first = flow_ssa_operand_register(analysis, item, 1);
                flow_build_map_add(map, &map_count, map_capacity, first,
                                   FLOW_MAP_SET_BASE, 0, first + 1);
                break;
            case OP_ISETUNLINKN_REG_REG_REG:
            case OP_ILOADSETUNLINKN_REG_INT_REG:
                for (operand = 0; operand <= 2; operand += 2) {
                    first = flow_ssa_operand_register(analysis, item, operand);
                    flow_build_map_add(map, &map_count, map_capacity, first,
                                       FLOW_MAP_SET_BASE, 0, first + 1);
                }
                break;
            case OP_ILOADSETUNLINKN_REG_REG_INT_REG:
                for (operand = 1; operand <= 3; operand += 2) {
                    first = flow_ssa_operand_register(analysis, item, operand);
                    flow_build_map_add(map, &map_count, map_capacity, first,
                                       FLOW_MAP_SET_BASE, 0, first + 1);
                }
                break;
            default:
                break;
        }
        if (opcode == OP_COPY_REG_REG || opcode == OP_ACOPY_REG_REG ||
            opcode == OP_NULL_REG ||
            (instruction->effects.semantics &
             (RXOP_SEM_LIFETIME_END | RXOP_SEM_REFERENCE_RELEASE |
              RXOP_SEM_INDIRECT_WRITE | RXOP_SEM_CALL |
              RXOP_SEM_DYNAMIC_CALL | RXOP_SEM_OPAQUE)))
            flags |= FLOW_SSA_MAP_CLOBBER_DYNAMIC |
                     FLOW_SSA_VALUE_CLOBBER_DYNAMIC;
        if ((instruction->effects.semantics &
             (RXOP_SEM_ALIAS_CREATE | RXOP_SEM_ALIAS_RELEASE)) && !map_count) {
            for (operand = 0; operand < item->operandCount; operand++) {
                if (!rxop_effect_writes_operand(&instruction->effects, operand))
                    continue;
                first = flow_ssa_operand_register(analysis, item, operand);
                flow_build_map_add(map, &map_count, map_capacity, first,
                                   FLOW_MAP_UNKNOWN, 0, 0);
            }
        }
        if (!flow_ssa_is_plain_mapping_opcode(opcode)) {
            for (operand = 0; operand < item->operandCount; operand++) {
                size_t target;
                size_t source;
                unsigned int components;
                size_t bit;
                RxasFlowValueKind kind;
                RxasFlowComponentPresence presence;
                RxOpValueDerivation derivation;
                const Assembler_Token *constant_token;
                if (!rxop_effect_writes_operand(&instruction->effects, operand))
                    continue;
                target = flow_ssa_operand_register(analysis, item, operand);
                if (target == RXAS_FLOW_ID_NONE) continue;
                components = rxop_component_writes(opcode, operand);
                if (!components) components = RXOP_COMPONENT_ALL;
                derivation = rxop_value_derivation(opcode);
                source = RXAS_FLOW_ID_NONE;
                kind = RXAS_FLOW_VALUE_WRITE;
                presence = RXAS_FLOW_COMPONENT_PRESENT;
                constant_token = 0;
                if (opcode == OP_NULL_REG) {
                    kind = RXAS_FLOW_VALUE_ABSENT;
                    presence = RXAS_FLOW_COMPONENT_ABSENT;
                    components = RXOP_COMPONENT_ALL;
                }
                else if (derivation != RXOP_DERIVATION_NONE) {
                    kind = RXAS_FLOW_VALUE_DERIVED;
                    source = flow_ssa_operand_register(
                            analysis, item,
                            rxop_derivation_source_operand(opcode));
                }
                else if ((opcode == OP_COPY_REG_REG ||
                          opcode == OP_ICOPY_REG_REG ||
                          opcode == OP_FCOPY_REG_REG ||
                          opcode == OP_SCOPY_REG_REG ||
                          opcode == OP_DCOPY_REG_REG ||
                          opcode == OP_BCOPY_REG_REG ||
                          opcode == OP_ACOPY_REG_REG) &&
                         item->operandCount > 1) {
                    kind = RXAS_FLOW_VALUE_COPY;
                    presence = RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN;
                    source = flow_ssa_operand_register(analysis, item, 1);
                }
                else if (item->operandCount > 1 && operand == 0 &&
                         !flow_token_is_register(flow_ssa_operand(item, 1))) {
                    kind = RXAS_FLOW_VALUE_CONSTANT;
                    constant_token = flow_ssa_operand(item, 1);
                }
                for (bit = 0; bit < 7; bit++) {
                    if (!(components & flow_component_bits[bit])) continue;
                    flow_build_value_add(
                            values, &value_count, value_capacity, target,
                            flow_component_bits[bit], kind, presence, source,
                            kind == RXAS_FLOW_VALUE_DERIVED
                                    ? (unsigned int)flow_ssa_component_source(
                                            derivation)
                                    : flow_component_bits[bit],
                            derivation, constant_token);
                }
            }
        }
        switch (instruction->effects.implicit) {
            case RXOP_IMPLICIT_LOCAL_COPY:
                first = flow_ssa_numbered_register(
                        analysis, RXAS_FLOW_REGISTER_LOCAL,
                        item->operand1Token);
                second = flow_ssa_numbered_register(
                        analysis, RXAS_FLOW_REGISTER_LOCAL,
                        item->operand2Token);
                if (first == RXAS_FLOW_ID_NONE ||
                    second == RXAS_FLOW_ID_NONE) {
                    flags |= FLOW_SSA_VALUE_CLOBBER_ALL;
                    break;
                }
                flags |= FLOW_SSA_MAP_CLOBBER_DYNAMIC |
                         FLOW_SSA_VALUE_CLOBBER_DYNAMIC;
                for (operand = 0; operand < 7; operand++)
                    flow_build_value_add(
                            values, &value_count, value_capacity, first,
                            flow_component_bits[operand], RXAS_FLOW_VALUE_COPY,
                            RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN, second,
                            flow_component_bits[operand],
                            RXOP_DERIVATION_NONE, 0);
                break;
            case RXOP_IMPLICIT_LOCAL_TARGET:
                first = flow_ssa_numbered_register(
                        analysis, RXAS_FLOW_REGISTER_LOCAL,
                        item->operand1Token);
                second = flow_ssa_operand_register(analysis, item, 1);
                if (first == RXAS_FLOW_ID_NONE ||
                    second == RXAS_FLOW_ID_NONE) {
                    flags |= FLOW_SSA_VALUE_CLOBBER_ALL;
                    break;
                }
                flags |= FLOW_SSA_MAP_CLOBBER_DYNAMIC |
                         FLOW_SSA_VALUE_CLOBBER_DYNAMIC;
                for (operand = 0; operand < 7; operand++)
                    flow_build_value_add(
                            values, &value_count, value_capacity, first,
                            flow_component_bits[operand], RXAS_FLOW_VALUE_COPY,
                            RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN, second,
                            flow_component_bits[operand],
                            RXOP_DERIVATION_NONE, 0);
                break;
            case RXOP_IMPLICIT_LOCAL_R0_READ_WRITE:
            case RXOP_IMPLICIT_LOCAL_R1_READ_WRITE:
            case RXOP_IMPLICIT_LOCAL_R2_READ_WRITE:
                first = instruction->effects.implicit ==
                                RXOP_IMPLICIT_LOCAL_R0_READ_WRITE ? 0 :
                        instruction->effects.implicit ==
                                RXOP_IMPLICIT_LOCAL_R1_READ_WRITE ? 1 : 2;
                {
                    RxasFlowRegister implicit_reg;
                    implicit_reg.register_class = RXAS_FLOW_REGISTER_LOCAL;
                    implicit_reg.number = first;
                    first = flow_ssa_register_id(analysis, implicit_reg);
                }
                if (first == RXAS_FLOW_ID_NONE)
                    flags |= FLOW_SSA_VALUE_CLOBBER_ALL;
                else flow_build_value_add(
                        values, &value_count, value_capacity, first,
                        RXOP_COMPONENT_INTEGER, RXAS_FLOW_VALUE_WRITE,
                        RXAS_FLOW_COMPONENT_PRESENT, RXAS_FLOW_ID_NONE,
                        RXOP_COMPONENT_NONE, RXOP_DERIVATION_NONE, 0);
                break;
            case RXOP_IMPLICIT_ARGUMENT_INDEX:
            case RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3:
            case RXOP_IMPLICIT_NONE:
                break;
            default:
                flags |= FLOW_SSA_VALUE_CLOBBER_ALL;
                break;
        }
    }
    /* A fused opcode may write a value before, between or after its mapping
     * sub-operations.  Until that intra-instruction order is canonical
     * metadata, retain the exact mapping result but do not attach the write to
     * an invented pre- or post-mapping storage. */
    if (map_count && value_count) {
        value_count = 0;
        flags |= FLOW_SSA_VALUE_CLOBBER_ALL;
    }
    if (!map_count && !value_count && !flags) {
        free(map);
        free(values);
        return input_state;
    }
    memset(&state, 0, sizeof(state));
    state.kind = FLOW_SSA_STATE_TRANSFER;
    state.parent = input_state;
    state.block_id = instruction->block_id;
    state.instruction_id = instruction->id;
    state.flags = flags;
    if (!flow_ssa_append_map_updates(
                analysis, map, map_count, input_state, &map_offset) ||
        !flow_ssa_append_value_updates(
                analysis, values, value_count, input_state, instruction->id,
                &value_offset)) {
        free(map);
        free(values);
        return RXAS_FLOW_ID_NONE;
    }
    state.map_offset = map_offset;
    state.map_count = map_count;
    state.value_offset = value_offset;
    state.value_count = value_count;
    free(map);
    free(values);
    return flow_ssa_add_state(analysis, state);
}

static size_t flow_ssa_add_clobber_state(
        RxasFlowSsaAnalysis *analysis, size_t parent, size_t block_id,
        size_t instruction_id, unsigned int flags) {
    FlowSsaState state;
    memset(&state, 0, sizeof(state));
    state.kind = FLOW_SSA_STATE_TRANSFER;
    state.parent = parent;
    state.block_id = block_id;
    state.instruction_id = instruction_id;
    state.flags = flags;
    return flow_ssa_add_state(analysis, state);
}

static size_t flow_ssa_build_failure_transfer(
        RxasFlowSsaAnalysis *analysis, const RxasFlowInstruction *instruction,
        size_t before_state, size_t after_state) {
    const RxasFlowRecord *record;
    const instruction_queue *item;
    FlowBuildMapUpdate *map;
    FlowBuildValueUpdate *values;
    size_t map_capacity;
    size_t map_count;
    size_t value_capacity;
    size_t value_count;
    size_t operand;
    size_t map_offset;
    size_t value_offset;
    FlowSsaState state;
    unsigned int flags;
    if (instruction->signal.phase == RXOP_SIGNAL_PHASE_BEFORE_WRITES)
        return before_state;
    if (instruction->signal.phase == RXOP_SIGNAL_PHASE_AFTER_WRITES)
        return after_state;
    if (!instruction->op)
        return flow_ssa_add_clobber_state(
                analysis, before_state, instruction->block_id,
                instruction->id,
                FLOW_SSA_MAP_CLOBBER_ALL | FLOW_SSA_VALUE_CLOBBER_ALL);
    record = rxas_flow_procedure_record(
            analysis->procedure, analysis->metrics.epoch,
            instruction->record_id);
    item = record ? record->queue_record : 0;
    if (!item) return before_state;
    map_capacity = item->operandCount * 2 + 4;
    value_capacity = (item->operandCount + 1) * 7;
    map = flow_ssa_calloc(analysis, map_capacity, sizeof(*map));
    values = flow_ssa_calloc(analysis, value_capacity, sizeof(*values));
    if (!map || !values) {
        free(map);
        free(values);
        return RXAS_FLOW_ID_NONE;
    }
    map_count = 0;
    value_count = 0;
    flags = 0;
    if (instruction->signal.phase == RXOP_SIGNAL_PHASE_UNKNOWN)
        flags |= FLOW_SSA_MAP_CLOBBER_DYNAMIC |
                 FLOW_SSA_VALUE_CLOBBER_DYNAMIC;
    switch (instruction->op->opcode) {
        case OP_LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT:
            flow_build_map_add(
                    map, &map_count, map_capacity,
                    flow_ssa_operand_register(analysis, item, 0),
                    FLOW_MAP_UNKNOWN, 0, 0);
            break;
        case OP_SWAPCALL_REG_FUNC_REG_REG_REG:
            flow_ssa_add_swap(analysis, item, map, &map_count,
                              map_capacity, 3, 4);
            break;
        case OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG:
            flow_ssa_add_swap(analysis, item, map, &map_count,
                              map_capacity, 3, 5);
            break;
        default:
            break;
    }
    for (operand = 0; operand < item->operandCount; operand++) {
        size_t target;
        size_t bit;
        unsigned int components;
        if (!rxop_signal_failure_writes_operand(
                    &instruction->signal, operand))
            continue;
        target = flow_ssa_operand_register(analysis, item, operand);
        components = instruction->signal.failure_component_writes;
        if (!components) components = RXOP_COMPONENT_ALL;
        for (bit = 0; bit < 7; bit++)
            if (components & flow_component_bits[bit])
                flow_build_value_add(
                        values, &value_count, value_capacity, target,
                        flow_component_bits[bit], RXAS_FLOW_VALUE_UNKNOWN,
                        RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN,
                        RXAS_FLOW_ID_NONE, RXOP_COMPONENT_NONE,
                        RXOP_DERIVATION_NONE, 0);
    }
    if (map_count && value_count) {
        value_count = 0;
        flags |= FLOW_SSA_VALUE_CLOBBER_ALL;
    }
    if (!map_count && !value_count && !flags) {
        free(map);
        free(values);
        return before_state;
    }
    memset(&state, 0, sizeof(state));
    state.kind = FLOW_SSA_STATE_TRANSFER;
    state.parent = before_state;
    state.block_id = instruction->block_id;
    state.instruction_id = instruction->id;
    state.flags = flags;
    if (!flow_ssa_append_map_updates(
                analysis, map, map_count, before_state, &map_offset) ||
        !flow_ssa_append_value_updates(
                analysis, values, value_count, before_state, instruction->id,
                &value_offset)) {
        free(map);
        free(values);
        return RXAS_FLOW_ID_NONE;
    }
    state.map_offset = map_offset;
    state.map_count = map_count;
    state.value_offset = value_offset;
    state.value_count = value_count;
    free(map);
    free(values);
    return flow_ssa_add_state(analysis, state);
}

static int flow_ssa_create_block_states(RxasFlowSsaAnalysis *analysis) {
    FlowSsaState state;
    size_t block_id;
    memset(&state, 0, sizeof(state));
    state.kind = FLOW_SSA_STATE_ENTRY;
    state.parent = RXAS_FLOW_ID_NONE;
    state.block_id = RXAS_FLOW_ID_NONE;
    state.instruction_id = RXAS_FLOW_ID_NONE;
    analysis->entry_state = flow_ssa_add_state(analysis, state);
    if (analysis->entry_state == RXAS_FLOW_ID_NONE) return 0;
    analysis->unknown_state = flow_ssa_add_clobber_state(
            analysis, analysis->entry_state, RXAS_FLOW_ID_NONE,
            RXAS_FLOW_ID_NONE,
            FLOW_SSA_MAP_CLOBBER_ALL | FLOW_SSA_VALUE_CLOBBER_ALL);
    if (analysis->unknown_state == RXAS_FLOW_ID_NONE) return 0;
    for (block_id = 0; block_id < analysis->block_count; block_id++) {
        const RxasFlowBlock *block;
        size_t incoming;
        block = rxas_flow_procedure_block(
                analysis->procedure, analysis->metrics.epoch, block_id);
        if (!block) return 0;
        if (block->kind == RXAS_FLOW_BLOCK_ENTRY)
            analysis->block_state[block_id] = analysis->entry_state;
        else if (block->kind == RXAS_FLOW_BLOCK_HANDLER_ROOT ||
                 block->kind == RXAS_FLOW_BLOCK_ASYNC_ROOT)
            analysis->block_state[block_id] = analysis->unknown_state;
        else {
            incoming = analysis->incoming_offsets[block_id + 1] -
                       analysis->incoming_offsets[block_id];
            if (!incoming) analysis->block_state[block_id] = analysis->unknown_state;
            else {
                size_t input_offset;
                size_t state_id;
                if (!flow_ssa_grow(analysis, (void **)&analysis->state_inputs,
                                   &analysis->state_input_capacity,
                                   analysis->state_input_count,
                                   sizeof(*analysis->state_inputs), incoming))
                    return 0;
                input_offset = analysis->state_input_count;
                while (incoming--) analysis->state_inputs[analysis->state_input_count++] =
                        RXAS_FLOW_ID_NONE;
                memset(&state, 0, sizeof(state));
                state.kind = FLOW_SSA_STATE_JOIN;
                state.parent = RXAS_FLOW_ID_NONE;
                state.block_id = block_id;
                state.instruction_id = RXAS_FLOW_ID_NONE;
                state.input_offset = input_offset;
                state.input_count = analysis->incoming_offsets[block_id + 1] -
                                    analysis->incoming_offsets[block_id];
                state_id = flow_ssa_add_state(analysis, state);
                if (state_id == RXAS_FLOW_ID_NONE) return 0;
                analysis->block_state[block_id] = state_id;
            }
        }
    }
    return 1;
}

static int flow_ssa_set_edges(RxasFlowSsaAnalysis *analysis,
                              size_t block_id, size_t before_last,
                              size_t after_last) {
    const RxasFlowBlock *block;
    const RxasFlowInstruction *instruction;
    size_t failure_state;
    size_t offset;
    block = rxas_flow_procedure_block(
            analysis->procedure, analysis->metrics.epoch, block_id);
    instruction = block && block->last_instruction != RXAS_FLOW_ID_NONE
            ? rxas_flow_procedure_instruction(
                    analysis->procedure, analysis->metrics.epoch,
                    block->last_instruction) : 0;
    failure_state = instruction &&
                    instruction->signal.state != RXOP_SIGNAL_STATE_NONE
            ? flow_ssa_build_failure_transfer(
                    analysis, instruction, before_last, after_last)
            : before_last;
    if (failure_state == RXAS_FLOW_ID_NONE) return 0;
    for (offset = analysis->outgoing_offsets[block_id];
         offset < analysis->outgoing_offsets[block_id + 1]; offset++) {
        size_t edge_id;
        const RxasFlowEdge *edge;
        size_t state_id;
        edge_id = analysis->outgoing_edges[offset];
        edge = rxas_flow_procedure_edge(
                analysis->procedure, analysis->metrics.epoch, edge_id);
        state_id = after_last;
        if (edge->kind == RXAS_FLOW_EDGE_SIGNAL_SKIP ||
            edge->kind == RXAS_FLOW_EDGE_SIGNAL_RETRY ||
            edge->kind == RXAS_FLOW_EDGE_HANDLER ||
            edge->kind == RXAS_FLOW_EDGE_UNWIND ||
            edge->kind == RXAS_FLOW_EDGE_TERMINAL)
            state_id = failure_state;
        if (edge->kind == RXAS_FLOW_EDGE_UNKNOWN)
            state_id = flow_ssa_add_clobber_state(
                    analysis, after_last, block_id,
                    instruction ? instruction->id : RXAS_FLOW_ID_NONE,
                    FLOW_SSA_MAP_CLOBBER_ALL |
                    FLOW_SSA_VALUE_CLOBBER_ALL);
        if (state_id == RXAS_FLOW_ID_NONE) return 0;
        analysis->edge_state[edge_id] = state_id;
        analysis->metrics.edge_states++;
    }
    return 1;
}

static int flow_ssa_process_block(RxasFlowSsaAnalysis *analysis,
                                  size_t block_id) {
    const RxasFlowBlock *block;
    size_t current;
    size_t before_last;
    size_t record_id;
    block = rxas_flow_procedure_block(
            analysis->procedure, analysis->metrics.epoch, block_id);
    if (!block) return 0;
    current = analysis->block_state[block_id];
    before_last = current;
    if (block->first_record != RXAS_FLOW_ID_NONE) {
        for (record_id = block->first_record; record_id <= block->last_record;
             record_id++) {
            const RxasFlowRecord *record;
            record = rxas_flow_procedure_record(
                    analysis->procedure, analysis->metrics.epoch, record_id);
            if (record && record->instruction_id != RXAS_FLOW_ID_NONE) {
                const RxasFlowInstruction *instruction;
                instruction = rxas_flow_procedure_instruction(
                        analysis->procedure, analysis->metrics.epoch,
                        record->instruction_id);
                before_last = current;
                analysis->instruction_before[instruction->id] = current;
                current = flow_ssa_build_normal_transfer(
                        analysis, instruction, current);
                if (current == RXAS_FLOW_ID_NONE) return 0;
                analysis->instruction_after[instruction->id] = current;
            }
        }
    }
    return flow_ssa_set_edges(analysis, block_id, before_last, current);
}

static int flow_ssa_fill_join_inputs(RxasFlowSsaAnalysis *analysis) {
    size_t block;
    for (block = 0; block < analysis->block_count; block++) {
        FlowSsaState *state;
        size_t offset;
        state = &analysis->states[analysis->block_state[block]];
        if (state->kind != FLOW_SSA_STATE_JOIN) continue;
        for (offset = 0; offset < state->input_count; offset++) {
            size_t edge_id;
            edge_id = analysis->incoming_edges[
                    analysis->incoming_offsets[block] + offset];
            state = &analysis->states[analysis->block_state[block]];
            analysis->state_inputs[state->input_offset + offset] =
                    analysis->edge_state[edge_id];
            if (analysis->edge_state[edge_id] == RXAS_FLOW_ID_NONE)
                return 0;
        }
    }
    return 1;
}

static size_t flow_hash_pair(size_t left, size_t right) {
    size_t value;
    value = left + (size_t)0x9e3779b9u;
    value ^= right + (value << 6) + (value >> 2);
    return value;
}

static int flow_ssa_cache_init(RxasFlowSsaAnalysis *analysis) {
    analysis->storage_bucket_count = 1024;
    analysis->value_bucket_count = 1024;
    analysis->storage_buckets = flow_ssa_calloc(
            analysis, analysis->storage_bucket_count, sizeof(size_t));
    analysis->value_buckets = flow_ssa_calloc(
            analysis, analysis->value_bucket_count, sizeof(size_t));
    return analysis->storage_buckets && analysis->value_buckets;
}

static int flow_ssa_storage_cache_rehash(RxasFlowSsaAnalysis *analysis) {
    size_t new_count;
    size_t *buckets;
    size_t index;
    new_count = analysis->storage_bucket_count * 2;
    buckets = flow_ssa_calloc(analysis, new_count, sizeof(size_t));
    if (!buckets) return 0;
    for (index = 0; index < analysis->storage_cache_count; index++) {
        size_t bucket;
        bucket = flow_hash_pair(analysis->storage_cache[index].state_id,
                                analysis->storage_cache[index].register_id) &
                 (new_count - 1);
        analysis->storage_cache[index].next = buckets[bucket];
        buckets[bucket] = index + 1;
    }
    free(analysis->storage_buckets);
    analysis->storage_buckets = buckets;
    analysis->storage_bucket_count = new_count;
    return 1;
}

static size_t flow_ssa_storage_cache_find(
        const RxasFlowSsaAnalysis *analysis, size_t state, size_t reg) {
    size_t link;
    size_t bucket;
    bucket = flow_hash_pair(state, reg) & (analysis->storage_bucket_count - 1);
    link = analysis->storage_buckets[bucket];
    while (link) {
        const FlowStorageCacheEntry *entry;
        entry = &analysis->storage_cache[link - 1];
        if (entry->state_id == state && entry->register_id == reg)
            return link - 1;
        link = entry->next;
    }
    return RXAS_FLOW_ID_NONE;
}

static size_t flow_ssa_storage_cache_add(
        RxasFlowSsaAnalysis *analysis, size_t state, size_t reg,
        size_t result) {
    size_t bucket;
    size_t index;
    FlowStorageCacheEntry *entry;
    if (analysis->storage_cache_count * 2 >= analysis->storage_bucket_count &&
        !flow_ssa_storage_cache_rehash(analysis))
        return RXAS_FLOW_ID_NONE;
    if (!flow_ssa_grow(analysis, (void **)&analysis->storage_cache,
                       &analysis->storage_cache_capacity,
                       analysis->storage_cache_count,
                       sizeof(*analysis->storage_cache), 1))
        return RXAS_FLOW_ID_NONE;
    index = analysis->storage_cache_count++;
    entry = &analysis->storage_cache[index];
    memset(entry, 0, sizeof(*entry));
    entry->state_id = state;
    entry->register_id = reg;
    entry->result = result;
    entry->resolving = 1;
    bucket = flow_hash_pair(state, reg) & (analysis->storage_bucket_count - 1);
    entry->next = analysis->storage_buckets[bucket];
    analysis->storage_buckets[bucket] = index + 1;
    return index;
}

static int flow_ssa_value_cache_rehash(RxasFlowSsaAnalysis *analysis) {
    size_t new_count;
    size_t *buckets;
    size_t index;
    new_count = analysis->value_bucket_count * 2;
    buckets = flow_ssa_calloc(analysis, new_count, sizeof(size_t));
    if (!buckets) return 0;
    for (index = 0; index < analysis->value_cache_count; index++) {
        size_t bucket;
        bucket = flow_hash_pair(
                flow_hash_pair(analysis->value_cache[index].state_id,
                               analysis->value_cache[index].storage_id),
                analysis->value_cache[index].component) & (new_count - 1);
        analysis->value_cache[index].next = buckets[bucket];
        buckets[bucket] = index + 1;
    }
    free(analysis->value_buckets);
    analysis->value_buckets = buckets;
    analysis->value_bucket_count = new_count;
    return 1;
}

static size_t flow_ssa_value_cache_find(
        const RxasFlowSsaAnalysis *analysis, size_t state, size_t storage,
        unsigned int component) {
    size_t link;
    size_t bucket;
    bucket = flow_hash_pair(flow_hash_pair(state, storage), component) &
             (analysis->value_bucket_count - 1);
    link = analysis->value_buckets[bucket];
    while (link) {
        const FlowValueCacheEntry *entry;
        entry = &analysis->value_cache[link - 1];
        if (entry->state_id == state && entry->storage_id == storage &&
            entry->component == component)
            return link - 1;
        link = entry->next;
    }
    return RXAS_FLOW_ID_NONE;
}

static size_t flow_ssa_value_cache_add(
        RxasFlowSsaAnalysis *analysis, size_t state, size_t storage,
        unsigned int component, size_t result) {
    size_t bucket;
    size_t index;
    FlowValueCacheEntry *entry;
    if (analysis->value_cache_count * 2 >= analysis->value_bucket_count &&
        !flow_ssa_value_cache_rehash(analysis))
        return RXAS_FLOW_ID_NONE;
    if (!flow_ssa_grow(analysis, (void **)&analysis->value_cache,
                       &analysis->value_cache_capacity,
                       analysis->value_cache_count,
                       sizeof(*analysis->value_cache), 1))
        return RXAS_FLOW_ID_NONE;
    index = analysis->value_cache_count++;
    entry = &analysis->value_cache[index];
    memset(entry, 0, sizeof(*entry));
    entry->state_id = state;
    entry->storage_id = storage;
    entry->component = component;
    entry->result = result;
    entry->resolving = 1;
    bucket = flow_hash_pair(flow_hash_pair(state, storage), component) &
             (analysis->value_bucket_count - 1);
    entry->next = analysis->value_buckets[bucket];
    analysis->value_buckets[bucket] = index + 1;
    return index;
}

static size_t flow_ssa_resolve_storage(RxasFlowSsaAnalysis *analysis,
                                       size_t state_id, size_t register_id);
static size_t flow_ssa_resolve_value(RxasFlowSsaAnalysis *analysis,
                                     size_t state_id, size_t storage_id,
                                     unsigned int component);

static size_t flow_ssa_create_storage_phi(
        RxasFlowSsaAnalysis *analysis, size_t state_id, size_t register_id) {
    FlowStorageVersion version;
    FlowSsaState *state;
    size_t index;
    size_t offset;
    state = &analysis->states[state_id];
    if (!flow_ssa_grow(analysis, (void **)&analysis->storage_inputs,
                       &analysis->storage_input_capacity,
                       analysis->storage_input_count,
                       sizeof(*analysis->storage_inputs), state->input_count))
        return RXAS_FLOW_ID_NONE;
    offset = analysis->storage_input_count;
    analysis->storage_input_count += state->input_count;
    {
        size_t input;
        for (input = 0; input < state->input_count; input++)
            analysis->storage_inputs[offset + input] = RXAS_FLOW_ID_NONE;
    }
    memset(&version, 0, sizeof(version));
    version.kind = RXAS_FLOW_STORAGE_PHI;
    version.replacement = RXAS_FLOW_ID_NONE;
    version.register_id = register_id;
    version.defining_instruction = RXAS_FLOW_ID_NONE;
    version.defining_block = state->block_id;
    version.join_state = state_id;
    version.input_offset = offset;
    version.input_count = state->input_count;
    index = flow_ssa_add_storage_version(analysis, version);
    return index == RXAS_FLOW_ID_NONE ? RXAS_FLOW_ID_NONE : index + 1;
}

static size_t flow_ssa_storage_canonical(
        const RxasFlowSsaAnalysis *analysis, size_t storage_id) {
    size_t depth;
    depth = 0;
    while (storage_id && storage_id <= analysis->storage_version_count &&
           analysis->storage_versions[storage_id - 1].replacement !=
                    RXAS_FLOW_ID_NONE &&
           analysis->storage_versions[storage_id - 1].replacement !=
                    storage_id &&
           depth++ <= analysis->storage_version_count)
        storage_id = analysis->storage_versions[
                storage_id - 1].replacement;
    return storage_id;
}

static int flow_ssa_storage_dynamic_marks_ready(
        RxasFlowSsaAnalysis *analysis) {
    size_t old_capacity;
    if (analysis->storage_dynamic_mark_capacity >=
        analysis->storage_version_count)
        return 1;
    old_capacity = analysis->storage_dynamic_mark_capacity;
    if (!flow_ssa_grow(analysis, (void **)&analysis->storage_dynamic_marks,
                       &analysis->storage_dynamic_mark_capacity, old_capacity,
                       sizeof(*analysis->storage_dynamic_marks),
                       analysis->storage_version_count - old_capacity))
        return 0;
    memset(analysis->storage_dynamic_marks + old_capacity, 0,
           (analysis->storage_dynamic_mark_capacity - old_capacity) *
                   sizeof(*analysis->storage_dynamic_marks));
    return 1;
}

static int flow_ssa_storage_is_dynamic_recursive(
        RxasFlowSsaAnalysis *analysis, size_t storage_id) {
    const FlowStorageVersion *version;
    size_t input;
    storage_id = flow_ssa_storage_canonical(analysis, storage_id);
    if (!storage_id) return 1;
    if (storage_id > analysis->storage_version_count) return 1;
    if (analysis->storage_dynamic_marks[storage_id - 1] ==
        analysis->storage_dynamic_generation)
        return 0;
    analysis->storage_dynamic_marks[storage_id - 1] =
            analysis->storage_dynamic_generation;
    if (!flow_ssa_consume(analysis, 1)) return 1;
    version = &analysis->storage_versions[storage_id - 1];
    if (version->kind == RXAS_FLOW_STORAGE_BASE) return 0;
    if (version->kind == RXAS_FLOW_STORAGE_SITE ||
        version->kind == RXAS_FLOW_STORAGE_UNKNOWN)
        return 1;
    for (input = 0; input < version->input_count; input++) {
        size_t input_storage;
        input_storage = analysis->storage_inputs[version->input_offset + input];
        if (input_storage == RXAS_FLOW_ID_NONE ||
            input_storage == storage_id)
            continue;
        if (flow_ssa_storage_is_dynamic_recursive(analysis, input_storage))
            return 1;
    }
    return 0;
}

static int flow_ssa_storage_is_dynamic(
        RxasFlowSsaAnalysis *analysis, size_t storage_id) {
    if (!flow_ssa_storage_dynamic_marks_ready(analysis)) return 1;
    analysis->storage_dynamic_generation++;
    if (!analysis->storage_dynamic_generation) {
        memset(analysis->storage_dynamic_marks, 0,
               analysis->storage_dynamic_mark_capacity *
                       sizeof(*analysis->storage_dynamic_marks));
        analysis->storage_dynamic_generation = 1;
    }
    return flow_ssa_storage_is_dynamic_recursive(analysis, storage_id);
}

static size_t flow_ssa_resolve_storage(RxasFlowSsaAnalysis *analysis,
                                       size_t state_id, size_t register_id) {
    size_t cache_id;
    FlowSsaState *state;
    size_t result;
    if (state_id >= analysis->state_count ||
        register_id >= analysis->register_count)
        return 0;
    cache_id = flow_ssa_storage_cache_find(analysis, state_id, register_id);
    if (cache_id != RXAS_FLOW_ID_NONE) {
        int preserve_dynamic;
        result = flow_ssa_storage_canonical(
                analysis, analysis->storage_cache[cache_id].result);
        state = &analysis->states[state_id];
        preserve_dynamic = 0;
        if (state->kind == FLOW_SSA_STATE_TRANSFER) {
            size_t update_index;
            for (update_index = state->map_count; update_index;
                 update_index--) {
                FlowMapUpdate *update;
                update = &analysis->map_updates[
                        state->map_offset + update_index - 1];
                if (update->destination != register_id) continue;
                preserve_dynamic = update->kind == FLOW_MAP_SET_BASE ||
                                   update->kind == FLOW_MAP_SET_SITE;
                break;
            }
        }
        if ((state->flags & FLOW_SSA_MAP_CLOBBER_ALL) ||
            ((state->flags & FLOW_SSA_MAP_CLOBBER_DYNAMIC) &&
             !preserve_dynamic &&
             flow_ssa_storage_is_dynamic(analysis, result))) {
            result = 0;
            analysis->storage_cache[cache_id].result = 0;
        }
        return result;
    }
    state = &analysis->states[state_id];
    result = state->kind == FLOW_SSA_STATE_JOIN
            ? flow_ssa_create_storage_phi(analysis, state_id, register_id)
            : register_id + 1;
    if (result == RXAS_FLOW_ID_NONE) return 0;
    cache_id = flow_ssa_storage_cache_add(
            analysis, state_id, register_id, result);
    if (cache_id == RXAS_FLOW_ID_NONE) return 0;
    if (state->kind == FLOW_SSA_STATE_ENTRY) result = register_id + 1;
    else if (state->kind == FLOW_SSA_STATE_TRANSFER) {
        size_t update_index;
        int found;
        found = 0;
        for (update_index = state->map_count; update_index; update_index--) {
            FlowMapUpdate *update;
            update = &analysis->map_updates[
                    state->map_offset + update_index - 1];
            if (update->destination != register_id) continue;
            found = 1;
            if (update->kind == FLOW_MAP_SET_BASE ||
                update->kind == FLOW_MAP_SET_SITE)
                result = update->storage_id;
            else if (update->kind == FLOW_MAP_COPY_REGISTER)
                result = flow_ssa_resolve_storage(
                        analysis, update->source_state,
                        update->source_register);
            else result = 0;
            if ((state->flags & FLOW_SSA_MAP_CLOBBER_DYNAMIC) &&
                update->kind == FLOW_MAP_COPY_REGISTER &&
                flow_ssa_storage_is_dynamic(analysis, result))
                result = 0;
            break;
        }
        if (!found) {
            result = flow_ssa_resolve_storage(
                    analysis, state->parent, register_id);
            if (state->flags & FLOW_SSA_MAP_CLOBBER_ALL) result = 0;
            else if ((state->flags & FLOW_SSA_MAP_CLOBBER_DYNAMIC) &&
                     flow_ssa_storage_is_dynamic(analysis, result))
                result = 0;
        }
    }
    else {
        size_t phi_index;
        size_t phi_storage_id;
        size_t phi_input_offset;
        size_t input;
        size_t same;
        int have_same;
        int different;
        phi_storage_id = result;
        phi_index = phi_storage_id - 1;
        phi_input_offset = analysis->storage_versions[phi_index].input_offset;
        same = 0;
        have_same = 0;
        different = 0;
        for (input = 0; input < state->input_count; input++) {
            size_t input_storage;
            input_storage = flow_ssa_resolve_storage(
                    analysis,
                    analysis->state_inputs[state->input_offset + input],
                    register_id);
            analysis->storage_inputs[phi_input_offset + input] = input_storage;
            input_storage = flow_ssa_storage_canonical(
                    analysis, input_storage);
            if (input_storage == phi_storage_id) continue;
            if (!have_same) {
                same = input_storage;
                have_same = 1;
            }
            else if (same != input_storage) different = 1;
        }
        if (have_same && !different) result = same;
        else if (!have_same) result = 0;
        if (result != phi_storage_id)
            analysis->storage_versions[phi_index].replacement = result;
    }
    result = flow_ssa_storage_canonical(analysis, result);
    cache_id = flow_ssa_storage_cache_find(analysis, state_id, register_id);
    analysis->storage_cache[cache_id].result = result;
    analysis->storage_cache[cache_id].resolving = 0;
    return result;
}

static size_t flow_ssa_create_value_version(
        RxasFlowSsaAnalysis *analysis, RxasFlowValueKind kind,
        RxasFlowComponentPresence presence, unsigned int component) {
    FlowValueVersion version;
    size_t effect;
    memset(&version, 0, sizeof(version));
    version.kind = kind;
    version.replacement = RXAS_FLOW_ID_NONE;
    version.presence = presence;
    version.component = component;
    version.defining_instruction = RXAS_FLOW_ID_NONE;
    version.source_state = RXAS_FLOW_ID_NONE;
    version.source_register = RXAS_FLOW_ID_NONE;
    version.source_value_id = RXAS_FLOW_ID_NONE;
    version.definition_numeric_context = RXAS_FLOW_ID_NONE;
    for (effect = 0; effect < RXAS_FLOW_EFFECT_CLASS_COUNT; effect++)
        version.definition_effects[effect] = RXAS_FLOW_ID_NONE;
    return flow_ssa_add_value_version(analysis, version);
}

static size_t flow_ssa_unmapped_value(RxasFlowSsaAnalysis *analysis,
                                      unsigned int component) {
    size_t bit;
    size_t value_id;
    for (bit = 0; bit < 7; bit++) {
        if (flow_component_bits[bit] != component) continue;
        if (analysis->unmapped_value_plus_one[bit])
            return analysis->unmapped_value_plus_one[bit] - 1;
        value_id = flow_ssa_create_value_version(
                analysis, RXAS_FLOW_VALUE_UNKNOWN,
                RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN, component);
        if (value_id == RXAS_FLOW_ID_NONE) return RXAS_FLOW_ID_NONE;
        analysis->unmapped_value_plus_one[bit] = value_id + 1;
        return value_id;
    }
    return RXAS_FLOW_ID_NONE;
}

static size_t flow_ssa_value_canonical(
        const RxasFlowSsaAnalysis *analysis, size_t value_id) {
    size_t depth;
    depth = 0;
    while (value_id != RXAS_FLOW_ID_NONE &&
           value_id < analysis->value_version_count &&
           analysis->value_versions[value_id].replacement !=
                    RXAS_FLOW_ID_NONE &&
           analysis->value_versions[value_id].replacement != value_id &&
           depth++ <= analysis->value_version_count)
        value_id = analysis->value_versions[value_id].replacement;
    return value_id;
}

static size_t flow_ssa_resolve_value(RxasFlowSsaAnalysis *analysis,
                                     size_t state_id, size_t storage_id,
                                     unsigned int component) {
    size_t cache_id;
    FlowSsaState *state;
    size_t result;
    storage_id = flow_ssa_storage_canonical(analysis, storage_id);
    if (state_id >= analysis->state_count) return RXAS_FLOW_ID_NONE;
    if (!storage_id) return flow_ssa_unmapped_value(analysis, component);
    cache_id = flow_ssa_value_cache_find(
            analysis, state_id, storage_id, component);
    if (cache_id != RXAS_FLOW_ID_NONE) {
        int direct_update;
        result = flow_ssa_value_canonical(
                analysis, analysis->value_cache[cache_id].result);
        state = &analysis->states[state_id];
        direct_update = 0;
        if (state->kind == FLOW_SSA_STATE_TRANSFER) {
            size_t update_index;
            for (update_index = state->value_count; update_index;
                 update_index--) {
                FlowValueUpdate *update;
                size_t target_storage;
                update = &analysis->value_updates[
                        state->value_offset + update_index - 1];
                if (update->component != component) continue;
                target_storage = flow_ssa_resolve_storage(
                        analysis, update->target_state,
                        update->target_register);
                target_storage = flow_ssa_storage_canonical(
                        analysis, target_storage);
                if (target_storage == storage_id) {
                    direct_update = 1;
                    break;
                }
            }
        }
        if (!direct_update &&
            ((state->flags & FLOW_SSA_VALUE_CLOBBER_ALL) ||
             ((state->flags & FLOW_SSA_VALUE_CLOBBER_DYNAMIC) &&
              flow_ssa_storage_is_dynamic(analysis, storage_id)))) {
            if (result >= analysis->value_version_count ||
                analysis->value_versions[result].kind !=
                        RXAS_FLOW_VALUE_UNKNOWN) {
                result = flow_ssa_create_value_version(
                        analysis, RXAS_FLOW_VALUE_UNKNOWN,
                        RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN, component);
                analysis->value_cache[cache_id].result = result;
            }
        }
        return result;
    }
    state = &analysis->states[state_id];
    result = flow_ssa_create_value_version(
            analysis,
            state->kind == FLOW_SSA_STATE_JOIN
                    ? RXAS_FLOW_VALUE_PHI
                    : state->kind == FLOW_SSA_STATE_ENTRY &&
                      storage_id > analysis->register_count
                            ? RXAS_FLOW_VALUE_UNKNOWN
                            : RXAS_FLOW_VALUE_ENTRY,
            RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN, component);
    if (result == RXAS_FLOW_ID_NONE) return 0;
    cache_id = flow_ssa_value_cache_add(
            analysis, state_id, storage_id, component, result);
    if (cache_id == RXAS_FLOW_ID_NONE) return 0;
    if (state->kind == FLOW_SSA_STATE_ENTRY) {
        /* The lazily created entry definition is the result. */
    }
    else if (state->kind == FLOW_SSA_STATE_TRANSFER) {
        size_t update_index;
        int found;
        found = 0;
        for (update_index = state->value_count; update_index; update_index--) {
            FlowValueUpdate *update;
            size_t target_storage;
            update = &analysis->value_updates[
                    state->value_offset + update_index - 1];
            if (update->component != component) continue;
            target_storage = flow_ssa_resolve_storage(
                    analysis, update->target_state,
                    update->target_register);
            target_storage = flow_ssa_storage_canonical(
                    analysis, target_storage);
            if (target_storage == storage_id) {
                result = flow_ssa_value_canonical(
                        analysis, update->value_id);
                found = 1;
                break;
            }
        }
        if (!found) {
            if (state->flags & FLOW_SSA_VALUE_CLOBBER_ALL)
                result = flow_ssa_create_value_version(
                        analysis, RXAS_FLOW_VALUE_UNKNOWN,
                        RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN, component);
            else if ((state->flags & FLOW_SSA_VALUE_CLOBBER_DYNAMIC) &&
                     flow_ssa_storage_is_dynamic(analysis, storage_id))
                result = flow_ssa_create_value_version(
                        analysis, RXAS_FLOW_VALUE_UNKNOWN,
                        RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN, component);
            else result = flow_ssa_resolve_value(
                    analysis, state->parent, storage_id, component);
        }
    }
    else {
        size_t phi_id;
        size_t phi_input_offset;
        size_t storage_join_state;
        size_t storage_input_offset;
        size_t storage_input_count;
        RxasFlowStorageKind storage_kind;
        size_t input;
        size_t same;
        int have_same;
        int different;
        phi_id = result;
        if (!flow_ssa_grow(analysis, (void **)&analysis->value_inputs,
                           &analysis->value_input_capacity,
                           analysis->value_input_count,
                           sizeof(*analysis->value_inputs), state->input_count))
            return 0;
        phi_input_offset = analysis->value_input_count;
        analysis->value_versions[phi_id].input_offset = phi_input_offset;
        analysis->value_versions[phi_id].input_count = state->input_count;
        analysis->value_input_count += state->input_count;
        storage_kind = RXAS_FLOW_STORAGE_UNKNOWN;
        storage_join_state = RXAS_FLOW_ID_NONE;
        storage_input_offset = 0;
        storage_input_count = 0;
        if (storage_id <= analysis->storage_version_count) {
            FlowStorageVersion *storage;
            storage = &analysis->storage_versions[storage_id - 1];
            storage_kind = storage->kind;
            storage_join_state = storage->join_state;
            storage_input_offset = storage->input_offset;
            storage_input_count = storage->input_count;
        }
        same = 0;
        have_same = 0;
        different = 0;
        for (input = 0; input < state->input_count; input++) {
            size_t input_storage;
            size_t input_value;
            input_storage = storage_id;
            if (storage_kind == RXAS_FLOW_STORAGE_PHI &&
                storage_join_state == state_id &&
                input < storage_input_count)
                input_storage = analysis->storage_inputs[
                        storage_input_offset + input];
            input_value = flow_ssa_resolve_value(
                    analysis,
                    analysis->state_inputs[state->input_offset + input],
                    input_storage, component);
            input_value = flow_ssa_value_canonical(analysis, input_value);
            analysis->value_inputs[phi_input_offset + input] = input_value;
            if (input_value == result) continue;
            if (!have_same) {
                same = input_value;
                have_same = 1;
            }
            else if (same != input_value) different = 1;
        }
        if (have_same && !different) result = same;
        else if (!have_same) result = flow_ssa_create_value_version(
                analysis, RXAS_FLOW_VALUE_UNKNOWN,
                RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN, component);
        else {
            size_t value_input;
            int all_present;
            int all_absent;
            all_present = 1;
            all_absent = 1;
            for (value_input = 0;
                 value_input < analysis->value_versions[phi_id].input_count;
                 value_input++) {
                size_t value_id;
                FlowValueVersion *input_version;
                value_id = analysis->value_inputs[
                        phi_input_offset + value_input];
                if (value_id >= analysis->value_version_count) {
                    all_present = 0;
                    all_absent = 0;
                    continue;
                }
                input_version = &analysis->value_versions[value_id];
                if (input_version->presence != RXAS_FLOW_COMPONENT_PRESENT)
                    all_present = 0;
                if (input_version->presence != RXAS_FLOW_COMPONENT_ABSENT)
                    all_absent = 0;
            }
            analysis->value_versions[phi_id].presence =
                    all_present ? RXAS_FLOW_COMPONENT_PRESENT
                    : all_absent ? RXAS_FLOW_COMPONENT_ABSENT
                                 : RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN;
        }
        if (result != phi_id)
            analysis->value_versions[phi_id].replacement = result;
    }
    result = flow_ssa_value_canonical(analysis, result);
    cache_id = flow_ssa_value_cache_find(
            analysis, state_id, storage_id, component);
    analysis->value_cache[cache_id].result = result;
    analysis->value_cache[cache_id].resolving = 0;
    return result;
}

static int flow_ssa_materialize_derivations(RxasFlowSsaAnalysis *analysis) {
    size_t instruction_id;
    for (instruction_id = 0; instruction_id < analysis->instruction_count;
         instruction_id++) {
        const RxasFlowInstruction *instruction;
        const RxasFlowRecord *record;
        const instruction_queue *item;
        RxOpValueDerivation derivation;
        size_t reg;
        size_t source_reg;
        size_t before_storage;
        size_t source_storage;
        size_t after_storage;
        unsigned int components;
        unsigned int source_component;
        size_t bit;
        instruction = rxas_flow_procedure_instruction(
                analysis->procedure, analysis->metrics.epoch, instruction_id);
        record = instruction ? rxas_flow_procedure_record(
                analysis->procedure, analysis->metrics.epoch,
                instruction->record_id) : 0;
        item = record ? record->queue_record : 0;
        if (!instruction || !instruction->op || !item) continue;
        derivation = rxop_value_derivation(instruction->op->opcode);
        if (derivation == RXOP_DERIVATION_NONE) continue;
        reg = flow_ssa_operand_register(analysis, item, 0);
        source_reg = flow_ssa_operand_register(
                analysis, item,
                rxop_derivation_source_operand(instruction->op->opcode));
        if (reg == RXAS_FLOW_ID_NONE || source_reg == RXAS_FLOW_ID_NONE)
            continue;
        before_storage = flow_ssa_resolve_storage(
                analysis, analysis->instruction_before[instruction_id], reg);
        source_storage = flow_ssa_resolve_storage(
                analysis, analysis->instruction_before[instruction_id],
                source_reg);
        after_storage = flow_ssa_resolve_storage(
                analysis, analysis->instruction_after[instruction_id], reg);
        source_component = (unsigned int)flow_ssa_component_source(derivation);
        if (source_component)
            (void)flow_ssa_resolve_value(
                    analysis, analysis->instruction_before[instruction_id],
                    source_storage, source_component);
        components = rxop_component_writes(instruction->op->opcode, 0);
        for (bit = 0; bit < 7; bit++) {
            if (!(components & flow_component_bits[bit])) continue;
            (void)flow_ssa_resolve_value(
                    analysis, analysis->instruction_before[instruction_id],
                    before_storage, flow_component_bits[bit]);
            (void)flow_ssa_resolve_value(
                    analysis, analysis->instruction_after[instruction_id],
                    after_storage, flow_component_bits[bit]);
        }
    }
    return analysis->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE;
}

static void flow_ssa_set_retained_bytes(RxasFlowSsaAnalysis *analysis) {
    size_t bytes;
    bytes = sizeof(*analysis);
    bytes += analysis->register_capacity * sizeof(*analysis->registers);
    bytes += analysis->state_capacity * sizeof(*analysis->states);
    bytes += analysis->map_update_capacity * sizeof(*analysis->map_updates);
    bytes += analysis->value_update_capacity * sizeof(*analysis->value_updates);
    bytes += analysis->state_input_capacity * sizeof(*analysis->state_inputs);
    bytes += (analysis->block_count * 3 + analysis->instruction_count * 2 +
              analysis->edge_count * 3 + 2) * sizeof(size_t);
    bytes += analysis->storage_version_capacity *
             sizeof(*analysis->storage_versions);
    bytes += analysis->storage_dynamic_mark_capacity *
             sizeof(*analysis->storage_dynamic_marks);
    bytes += analysis->storage_input_capacity * sizeof(*analysis->storage_inputs);
    bytes += analysis->value_version_capacity * sizeof(*analysis->value_versions);
    bytes += analysis->value_input_capacity * sizeof(*analysis->value_inputs);
    bytes += analysis->storage_bucket_count * sizeof(size_t);
    bytes += analysis->storage_cache_capacity * sizeof(*analysis->storage_cache);
    bytes += analysis->value_bucket_count * sizeof(size_t);
    bytes += analysis->value_cache_capacity * sizeof(*analysis->value_cache);
    analysis->metrics.retained_bytes = bytes;
}

static void flow_ssa_free(RxasFlowSsaAnalysis *analysis) {
    if (!analysis) return;
    free(analysis->registers);
    free(analysis->states);
    free(analysis->map_updates);
    free(analysis->value_updates);
    free(analysis->state_inputs);
    free(analysis->incoming_offsets);
    free(analysis->incoming_edges);
    free(analysis->outgoing_offsets);
    free(analysis->outgoing_edges);
    free(analysis->block_state);
    free(analysis->instruction_before);
    free(analysis->instruction_after);
    free(analysis->edge_state);
    free(analysis->storage_versions);
    free(analysis->storage_dynamic_marks);
    free(analysis->storage_inputs);
    free(analysis->value_versions);
    free(analysis->value_inputs);
    free(analysis->storage_buckets);
    free(analysis->storage_cache);
    free(analysis->value_buckets);
    free(analysis->value_cache);
    free(analysis);
}

void rxas_flow_ssa_analysis_destroy(struct RxasFlowSsaAnalysis *analysis) {
    flow_ssa_free(analysis);
}

static RxasFlowSsaAnalysis *flow_ssa_build(
        RxasFlowProcedure *procedure, unsigned long epoch, size_t budget) {
    RxasFlowSsaAnalysis *analysis;
    const RxasFlowMetrics *metrics;
    unsigned char *processed;
    size_t rpo_count;
    size_t index;
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
    analysis->signal = rxas_flow_require_signal_analysis(procedure, epoch, 0);
    if (!analysis->structural || !analysis->signal) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
        return analysis;
    }
    if (!flow_ssa_collect_registers(analysis)) return analysis;
    analysis->block_state = flow_ssa_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    analysis->instruction_before = flow_ssa_calloc(
            analysis, analysis->instruction_count, sizeof(size_t));
    analysis->instruction_after = flow_ssa_calloc(
            analysis, analysis->instruction_count, sizeof(size_t));
    analysis->edge_state = flow_ssa_calloc(
            analysis, analysis->edge_count, sizeof(size_t));
    if (!analysis->block_state || !analysis->instruction_before ||
        !analysis->instruction_after || !analysis->edge_state ||
        !flow_ssa_build_adjacency(analysis) ||
        !flow_ssa_create_block_states(analysis))
        return analysis;
    /* Base storage IDs are register_id + 1. */
    for (index = 0; index < analysis->register_count; index++) {
        FlowStorageVersion version;
        size_t id;
        memset(&version, 0, sizeof(version));
        version.kind = RXAS_FLOW_STORAGE_BASE;
        version.replacement = RXAS_FLOW_ID_NONE;
        version.register_id = index;
        version.defining_instruction = RXAS_FLOW_ID_NONE;
        version.defining_block = RXAS_FLOW_ID_NONE;
        version.join_state = RXAS_FLOW_ID_NONE;
        id = flow_ssa_add_storage_version(analysis, version);
        if (id == RXAS_FLOW_ID_NONE || id != index) return analysis;
    }
    processed = flow_ssa_calloc(analysis, analysis->block_count, 1);
    if (!processed) return analysis;
    rpo_count = rxas_flow_structural_rpo_count(analysis->structural, epoch);
    for (index = 0; index < rpo_count; index++) {
        size_t block;
        block = rxas_flow_structural_rpo_block(analysis->structural, epoch, index);
        if (block == RXAS_FLOW_ID_NONE || !flow_ssa_process_block(analysis, block)) {
            free(processed);
            return analysis;
        }
        processed[block] = 1;
    }
    for (index = 0; index < analysis->block_count; index++) {
        if (processed[index]) continue;
        analysis->block_state[index] = analysis->unknown_state;
        if (!flow_ssa_process_block(analysis, index)) {
            free(processed);
            return analysis;
        }
    }
    free(processed);
    if (!flow_ssa_fill_join_inputs(analysis) || !flow_ssa_cache_init(analysis) ||
        !flow_ssa_materialize_derivations(analysis))
        return analysis;
    flow_ssa_set_retained_bytes(analysis);
    return analysis;
}

const RxasFlowSsaAnalysis *rxas_flow_require_ssa_analysis(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget) {
    struct RxasFlowAnalysisManager *manager;
    const RxasFlowMetrics *metrics;
    size_t requested;
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch)) return 0;
    metrics = rxas_flow_procedure_metrics(procedure, expected_epoch);
    if (!metrics) return 0;
    requested = work_budget ? work_budget
                            : flow_ssa_default_budget(metrics, procedure);
    manager = procedure->analysis_manager;
    if (manager && manager->epoch == expected_epoch && manager->ssa) {
        if (manager->ssa->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE)
            return manager->ssa;
        if (requested <= manager->ssa_budget) return 0;
        flow_ssa_free(manager->ssa);
        manager->ssa = 0;
    }
    if (!manager) {
        manager = calloc(1, sizeof(*manager));
        if (!manager) return 0;
        procedure->analysis_manager = manager;
    }
    manager->epoch = expected_epoch;
    manager->ssa_budget = requested;
    manager->ssa = flow_ssa_build(procedure, expected_epoch, requested);
    if (!manager->ssa ||
        manager->ssa->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE)
        return 0;
    return manager->ssa;
}

const RxasFlowSsaMetrics *rxas_flow_last_ssa_metrics(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch) {
    const struct RxasFlowAnalysisManager *manager;
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch) ||
        !procedure->analysis_manager)
        return 0;
    manager = procedure->analysis_manager;
    if (manager->epoch != expected_epoch || !manager->ssa) return 0;
    return &manager->ssa->metrics;
}

static int flow_ssa_valid(const RxasFlowSsaAnalysis *analysis,
                          unsigned long epoch) {
    return analysis && epoch && analysis->metrics.epoch == epoch &&
           analysis->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE &&
           rxas_flow_procedure_epoch_matches(analysis->procedure, epoch);
}

const RxasFlowSsaMetrics *rxas_flow_ssa_metrics(
        const RxasFlowSsaAnalysis *analysis, unsigned long expected_epoch) {
    if (!flow_ssa_valid(analysis, expected_epoch)) return 0;
    return &analysis->metrics;
}

static int flow_ssa_storage_fact(RxasFlowSsaAnalysis *analysis,
                                 size_t state_id, RxasFlowRegister reg,
                                 RxasFlowStorageFact *fact) {
    size_t register_id;
    size_t storage_id;
    if (!fact) return 0;
    register_id = flow_ssa_register_id(analysis, reg);
    if (register_id == RXAS_FLOW_ID_NONE) return 0;
    storage_id = flow_ssa_resolve_storage(analysis, state_id, register_id);
    memset(fact, 0, sizeof(*fact));
    fact->storage_id = storage_id;
    fact->defining_instruction = RXAS_FLOW_ID_NONE;
    fact->defining_block = RXAS_FLOW_ID_NONE;
    if (!storage_id) {
        fact->kind = RXAS_FLOW_STORAGE_UNKNOWN;
        return 1;
    }
    if (storage_id > analysis->storage_version_count) return 0;
    fact->kind = analysis->storage_versions[storage_id - 1].kind;
    fact->defining_instruction =
            analysis->storage_versions[storage_id - 1].defining_instruction;
    fact->defining_block =
            analysis->storage_versions[storage_id - 1].defining_block;
    return 1;
}

int rxas_flow_storage_at_instruction(
        const RxasFlowSsaAnalysis *analysis, unsigned long expected_epoch,
        size_t instruction_id, int after_instruction,
        RxasFlowRegister reg, RxasFlowStorageFact *fact) {
    size_t state;
    if (!flow_ssa_valid(analysis, expected_epoch) ||
        instruction_id >= analysis->instruction_count)
        return 0;
    state = after_instruction ? analysis->instruction_after[instruction_id]
                              : analysis->instruction_before[instruction_id];
    return flow_ssa_storage_fact(
            (RxasFlowSsaAnalysis *)analysis, state, reg, fact);
}

int rxas_flow_storage_on_edge(
        const RxasFlowSsaAnalysis *analysis, unsigned long expected_epoch,
        size_t edge_id, RxasFlowRegister reg, RxasFlowStorageFact *fact) {
    if (!flow_ssa_valid(analysis, expected_epoch) || edge_id >= analysis->edge_count)
        return 0;
    return flow_ssa_storage_fact((RxasFlowSsaAnalysis *)analysis,
                                 analysis->edge_state[edge_id], reg, fact);
}

static int flow_ssa_component_fact(
        RxasFlowSsaAnalysis *analysis, size_t state_id,
        RxasFlowRegister reg, unsigned int component,
        size_t instruction_id, int after_instruction, size_t edge_id,
        RxasFlowComponentFact *fact) {
    size_t register_id;
    size_t storage_id;
    size_t value_id;
    FlowValueVersion *version;
    size_t effect;
    if (!fact || !component || (component & (component - 1)) ||
        !(component & RXOP_COMPONENT_ALL))
        return 0;
    register_id = flow_ssa_register_id(analysis, reg);
    if (register_id == RXAS_FLOW_ID_NONE) return 0;
    storage_id = flow_ssa_resolve_storage(analysis, state_id, register_id);
    memset(fact, 0, sizeof(*fact));
    fact->storage_id = storage_id;
    fact->component = component;
    fact->defining_instruction = RXAS_FLOW_ID_NONE;
    fact->source_value_id = RXAS_FLOW_ID_NONE;
    fact->definition_numeric_context = RXAS_FLOW_ID_NONE;
    for (effect = 0; effect < RXAS_FLOW_EFFECT_CLASS_COUNT; effect++) {
        fact->definition_effects[effect] = RXAS_FLOW_ID_NONE;
        fact->current_effects[effect] = RXAS_FLOW_ID_NONE;
    }
    if (!storage_id) {
        fact->kind = RXAS_FLOW_VALUE_UNKNOWN;
        fact->presence = RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN;
        fact->value_id = RXAS_FLOW_ID_NONE;
        return 1;
    }
    value_id = flow_ssa_resolve_value(analysis, state_id, storage_id, component);
    value_id = flow_ssa_value_canonical(analysis, value_id);
    if (value_id >= analysis->value_version_count) return 0;
    version = &analysis->value_versions[value_id];
    fact->value_id = value_id;
    fact->kind = version->kind;
    fact->presence = version->presence;
    fact->defining_instruction = version->defining_instruction;
    fact->derivation = version->derivation;
    fact->signal_dependencies = version->signal_dependencies;
    for (effect = 0; effect < RXAS_FLOW_EFFECT_CLASS_COUNT; effect++)
        fact->definition_effects[effect] = version->definition_effects[effect];
    fact->definition_numeric_context = version->definition_numeric_context;
    fact->constant_token = version->constant_token;
    if ((version->kind == RXAS_FLOW_VALUE_COPY ||
         version->kind == RXAS_FLOW_VALUE_DERIVED) &&
        version->source_register != RXAS_FLOW_ID_NONE) {
        size_t source_storage;
        size_t source_state;
        size_t source_register;
        unsigned int source_component;
        RxasFlowValueKind value_kind;
        size_t source_value_id;
        value_kind = version->kind;
        source_state = version->source_state;
        source_register = version->source_register;
        source_component = version->source_component;
        source_storage = flow_ssa_resolve_storage(
                analysis, source_state, source_register);
        source_value_id = flow_ssa_resolve_value(
                analysis, source_state, source_storage, source_component);
        source_value_id = flow_ssa_value_canonical(
                analysis, source_value_id);
        analysis->value_versions[value_id].source_value_id = source_value_id;
        fact->source_value_id = source_value_id;
        if (value_kind == RXAS_FLOW_VALUE_COPY &&
            source_value_id < analysis->value_version_count) {
            fact->presence =
                    analysis->value_versions[source_value_id].presence;
            analysis->value_versions[value_id].presence = fact->presence;
        }
    }
    if (instruction_id != RXAS_FLOW_ID_NONE) {
        for (effect = 0; effect < RXAS_FLOW_EFFECT_CLASS_COUNT; effect++)
            fact->current_effects[effect] =
                    rxas_flow_effect_at_instruction(
                            analysis->signal, analysis->metrics.epoch,
                            instruction_id, after_instruction,
                            (RxasFlowEffectClass)effect);
    }
    else {
        for (effect = 0; effect < RXAS_FLOW_EFFECT_CLASS_COUNT; effect++)
            fact->current_effects[effect] = rxas_flow_effect_on_edge(
                    analysis->signal, analysis->metrics.epoch, edge_id,
                    (RxasFlowEffectClass)effect);
    }
    fact->current_numeric_context = fact->current_effects[
            RXAS_FLOW_EFFECT_NUMERIC_CONTEXT];
    fact->current_reference_effect = fact->current_effects[
            RXAS_FLOW_EFFECT_REFERENCE];
    return 1;
}

int rxas_flow_component_at_instruction(
        const RxasFlowSsaAnalysis *analysis, unsigned long expected_epoch,
        size_t instruction_id, int after_instruction,
        RxasFlowRegister reg, unsigned int component,
        RxasFlowComponentFact *fact) {
    size_t state;
    if (!flow_ssa_valid(analysis, expected_epoch) ||
        instruction_id >= analysis->instruction_count)
        return 0;
    state = after_instruction ? analysis->instruction_after[instruction_id]
                              : analysis->instruction_before[instruction_id];
    return flow_ssa_component_fact(
            (RxasFlowSsaAnalysis *)analysis, state, reg, component,
            instruction_id, after_instruction, RXAS_FLOW_ID_NONE, fact);
}

int rxas_flow_component_on_edge(
        const RxasFlowSsaAnalysis *analysis, unsigned long expected_epoch,
        size_t edge_id, RxasFlowRegister reg, unsigned int component,
        RxasFlowComponentFact *fact) {
    if (!flow_ssa_valid(analysis, expected_epoch) || edge_id >= analysis->edge_count)
        return 0;
    return flow_ssa_component_fact(
            (RxasFlowSsaAnalysis *)analysis, analysis->edge_state[edge_id],
            reg, component, RXAS_FLOW_ID_NONE, 0, edge_id, fact);
}

static const char *flow_ssa_status_name(RxasFlowAnalysisStatus status) {
    switch (status) {
        case RXAS_FLOW_ANALYSIS_AVAILABLE: return "available";
        case RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED: return "budget-exhausted";
        case RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY: return "out-of-memory";
        case RXAS_FLOW_ANALYSIS_INVALID_GRAPH: return "invalid-graph";
    }
    return "invalid";
}

int rxas_flow_ssa_dump(const RxasFlowSsaAnalysis *analysis,
                       unsigned long expected_epoch, FILE *stream) {
    size_t index;
    if (!stream || !flow_ssa_valid(analysis, expected_epoch)) return 0;
    fprintf(stream,
            "PERF3 flow-ssa epoch=%lu status=%s budget=%llu work=%llu "
            "bytes=%llu registers=%llu states=%llu joins=%llu "
            "map-updates=%llu map-clobbers=%llu storages=%llu sites=%llu "
            "storage-phis=%llu component-updates=%llu values=%llu "
            "value-phis=%llu absent=%llu constants=%llu derived=%llu "
            "unknown-values=%llu edge-states=%llu\n",
            analysis->metrics.epoch,
            flow_ssa_status_name(analysis->metrics.status),
            (unsigned long long)analysis->metrics.budget_limit,
            (unsigned long long)analysis->metrics.work,
            (unsigned long long)analysis->metrics.retained_bytes,
            (unsigned long long)analysis->metrics.registers,
            (unsigned long long)analysis->metrics.states,
            (unsigned long long)analysis->metrics.join_states,
            (unsigned long long)analysis->metrics.mapping_updates,
            (unsigned long long)analysis->metrics.mapping_clobbers,
            (unsigned long long)analysis->metrics.storage_versions,
            (unsigned long long)analysis->metrics.storage_sites,
            (unsigned long long)analysis->metrics.storage_phis,
            (unsigned long long)analysis->metrics.component_updates,
            (unsigned long long)analysis->metrics.value_versions,
            (unsigned long long)analysis->metrics.value_phis,
            (unsigned long long)analysis->metrics.absent_values,
            (unsigned long long)analysis->metrics.constant_values,
            (unsigned long long)analysis->metrics.derived_values,
            (unsigned long long)analysis->metrics.unknown_values,
            (unsigned long long)analysis->metrics.edge_states);
    for (index = 0; index < analysis->register_count; index++)
        fprintf(stream,
                "PERF3 flow-ssa-register id=%llu class=%d number=%llu base=%llu\n",
                (unsigned long long)index,
                (int)analysis->registers[index].register_class,
                (unsigned long long)analysis->registers[index].number,
                (unsigned long long)(index + 1));
    for (index = 0; index < analysis->storage_version_count; index++) {
        const FlowStorageVersion *version;
        version = &analysis->storage_versions[index];
        fprintf(stream,
                "PERF3 flow-storage id=%llu kind=%d register=%llu "
                "instruction=",
                (unsigned long long)(index + 1), (int)version->kind,
                (unsigned long long)version->register_id);
        if (version->defining_instruction == RXAS_FLOW_ID_NONE) fputc('-', stream);
        else fprintf(stream, "%llu",
                     (unsigned long long)version->defining_instruction);
        fprintf(stream, " block=");
        if (version->defining_block == RXAS_FLOW_ID_NONE) fputc('-', stream);
        else fprintf(stream, "%llu", (unsigned long long)version->defining_block);
        fputc('\n', stream);
    }
    for (index = 0; index < analysis->edge_count; index++)
        fprintf(stream, "PERF3 flow-ssa-edge id=%llu state=%llu\n",
                (unsigned long long)index,
                (unsigned long long)analysis->edge_state[index]);
    return 1;
}
