/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

/* Cached proof queries over the immutable structural, signal and sparse SSA
 * analyses.  Consumers receive an explicit rejection reason and never inspect
 * solver internals or infer identity from raw register numbers. */

#include "rxas_flow_proof.h"
#include "rxas_flow_graph_internal.h"
#include "rxasassm.h"

#include <stdlib.h>
#include <string.h>

typedef struct FlowProofCacheEntry {
    size_t generator;
    size_t candidate;
    RxasFlowProofResult result;
} FlowProofCacheEntry;

struct RxasFlowProofService {
    RxasFlowProcedure *procedure;
    const RxasFlowStructuralAnalysis *structural;
    const RxasFlowSignalAnalysis *signal;
    const RxasFlowSsaAnalysis *ssa;
    const RxasFlowUseAnalysis *use;
    RxasFlowProofMetrics metrics;
    size_t block_count;
    size_t edge_count;
    size_t *outgoing_offsets;
    size_t *outgoing_edges;
    size_t *visit_marks;
    size_t visit_generation;
    size_t *visit_queue;
    size_t *value_marks;
    size_t *value_stack;
    size_t *value_aux;
    size_t *value_set_marks;
    size_t value_capacity;
    size_t value_generation;
    size_t value_set_generation;
    size_t *storage_marks;
    size_t *storage_stack;
    size_t storage_capacity;
    size_t storage_generation;
    size_t *effect_marks;
    size_t *effect_stack;
    size_t *effect_set_marks;
    size_t effect_capacity;
    size_t effect_generation;
    size_t effect_set_generation;
    FlowProofCacheEntry *cache;
    size_t cache_count;
    size_t cache_capacity;
    RxasFlowOperandRewrite *rewrites;
    size_t rewrite_count;
    size_t rewrite_capacity;
    RxasFlowTraceDeletion *trace_deletions;
    size_t trace_deletion_count;
    size_t trace_deletion_capacity;
};

#define FLOW_PROOF_ALLOWED_CAPABILITIES \
    (RXAS_FLOW_CAP_CFG | RXAS_FLOW_CAP_DOMINANCE | RXAS_FLOW_CAP_SIGNAL | \
     RXAS_FLOW_CAP_STORAGE | RXAS_FLOW_CAP_VALUE | RXAS_FLOW_CAP_USE | \
     RXAS_FLOW_CAP_LOOPS)

#define FLOW_PROOF_BASE_CAPABILITIES \
    (RXAS_FLOW_CAP_CFG | RXAS_FLOW_CAP_DOMINANCE | RXAS_FLOW_CAP_SIGNAL | \
     RXAS_FLOW_CAP_STORAGE | RXAS_FLOW_CAP_VALUE)

#define FLOW_PROOF_USE_CAPABILITIES \
    (FLOW_PROOF_BASE_CAPABILITIES | RXAS_FLOW_CAP_USE)

#define FLOW_PROOF_LOOP_CAPABILITIES \
    (FLOW_PROOF_BASE_CAPABILITIES | RXAS_FLOW_CAP_LOOPS)

static int flow_proof_consume(RxasFlowProofService *service, size_t amount) {
    size_t remaining;
    if (!service || service->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE)
        return 0;
    remaining = service->metrics.work <= service->metrics.budget_limit
            ? service->metrics.budget_limit - service->metrics.work : 0;
    if (amount > remaining) {
        service->metrics.status = RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED;
        return 0;
    }
    service->metrics.work += amount;
    return 1;
}

static size_t flow_proof_default_budget(const RxasFlowMetrics *metrics) {
    size_t scale;
    if (!metrics || metrics->blocks > (size_t)-1 - metrics->edges ||
        metrics->blocks + metrics->edges >
                (size_t)-1 - metrics->instructions - 1)
        return (size_t)-1;
    scale = metrics->blocks + metrics->edges + metrics->instructions + 1;
    if (scale > ((size_t)-1 - 4096) / 64) return (size_t)-1;
    return scale * 64 + 4096;
}

static void flow_proof_result_init(RxasFlowProofResult *result,
                                   RxasFlowProofReason reason) {
    if (!result) return;
    memset(result, 0, sizeof(*result));
    result->reason = reason;
    result->generator_instruction = RXAS_FLOW_ID_NONE;
    result->candidate_instruction = RXAS_FLOW_ID_NONE;
    result->loop_id = RXAS_FLOW_ID_NONE;
    result->source_value_id = RXAS_FLOW_ID_NONE;
    result->result_value_id = RXAS_FLOW_ID_NONE;
    result->candidate_source_value_id = RXAS_FLOW_ID_NONE;
    result->candidate_result_value_id = RXAS_FLOW_ID_NONE;
    result->effect_class = RXAS_FLOW_ID_NONE;
    result->generator_effect_id = RXAS_FLOW_ID_NONE;
    result->candidate_effect_id = RXAS_FLOW_ID_NONE;
}

static Assembler_Token *flow_proof_operand(const instruction_queue *item,
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

static int flow_proof_register(const Assembler_Token *token,
                               RxasFlowRegister *reg) {
    if (!token || !reg || token->token_value.integer < 0) return 0;
    if (token->token_type == RREG)
        reg->register_class = RXAS_FLOW_REGISTER_LOCAL;
    else if (token->token_type == AREG)
        reg->register_class = RXAS_FLOW_REGISTER_ARGUMENT;
    else if (token->token_type == GREG)
        reg->register_class = RXAS_FLOW_REGISTER_GLOBAL;
    else return 0;
    reg->number = (size_t)token->token_value.integer;
    return 1;
}

/* Sparse writes live in the use index so component-interval proofs can find
 * them.  They overwrite a value; they do not observe that value.  Consumers
 * which audit unknown ValueIds must keep that distinction from the read and
 * read/write kinds. */
static int flow_proof_use_is_pure_write(const RxasFlowUse *use) {
    return use &&
           (use->kind == RXAS_FLOW_USE_EXPLICIT_WRITE ||
            use->kind == RXAS_FLOW_USE_OPAQUE_WRITE ||
            use->kind == RXAS_FLOW_USE_CURSOR_WRITE);
}

static int flow_proof_same_scalar_constant(const Assembler_Token *left,
                                           const Assembler_Token *right) {
    if (!left || !right || left->token_type != right->token_type) return 0;
    if (left->token_type == INT)
        return left->token_value.integer == right->token_value.integer;
    if (left->token_type == FLOAT)
        return memcmp(&left->token_value.real, &right->token_value.real,
                      sizeof(left->token_value.real)) == 0;
    return 0;
}

static int flow_proof_build_adjacency(RxasFlowProofService *service) {
    size_t *fill;
    size_t edge_id;
    size_t block;
    service->outgoing_offsets = calloc(service->block_count + 1,
                                       sizeof(*service->outgoing_offsets));
    service->outgoing_edges = calloc(service->edge_count ? service->edge_count : 1,
                                     sizeof(*service->outgoing_edges));
    service->visit_marks = calloc(service->block_count ? service->block_count : 1,
                                  sizeof(*service->visit_marks));
    service->visit_queue = calloc(service->block_count ? service->block_count : 1,
                                  sizeof(*service->visit_queue));
    fill = calloc(service->block_count ? service->block_count : 1,
                  sizeof(*fill));
    if (!service->outgoing_offsets || !service->outgoing_edges ||
        !service->visit_marks || !service->visit_queue || !fill) {
        free(fill);
        service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    for (edge_id = 0; edge_id < service->edge_count; edge_id++) {
        const RxasFlowEdge *edge;
        edge = rxas_flow_procedure_edge(service->procedure,
                                        service->metrics.epoch, edge_id);
        if (!edge || edge->source >= service->block_count ||
            edge->target >= service->block_count) {
            free(fill);
            service->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
            return 0;
        }
        service->outgoing_offsets[edge->source + 1]++;
    }
    for (block = 1; block <= service->block_count; block++)
        service->outgoing_offsets[block] +=
                service->outgoing_offsets[block - 1];
    for (edge_id = 0; edge_id < service->edge_count; edge_id++) {
        const RxasFlowEdge *edge;
        size_t offset;
        edge = rxas_flow_procedure_edge(service->procedure,
                                        service->metrics.epoch, edge_id);
        offset = service->outgoing_offsets[edge->source] +
                 fill[edge->source]++;
        service->outgoing_edges[offset] = edge_id;
    }
    free(fill);
    return flow_proof_consume(service, service->edge_count * 2);
}

static void flow_proof_set_retained_bytes(RxasFlowProofService *service) {
    if (!service) return;
    service->metrics.retained_bytes = sizeof(*service) +
            (service->outgoing_offsets ? service->block_count + 1 : 0) *
                    sizeof(size_t) +
            (service->outgoing_edges ? service->edge_count : 0) *
                    sizeof(size_t) +
            (service->visit_marks ? service->block_count : 0) *
                    sizeof(size_t) +
            (service->visit_queue ? service->block_count : 0) *
                    sizeof(size_t) +
            service->value_capacity * 4 * sizeof(size_t) +
            service->storage_capacity * 2 * sizeof(size_t) +
            service->effect_capacity * 3 * sizeof(size_t) +
            service->cache_capacity * sizeof(*service->cache) +
            service->rewrite_capacity * sizeof(*service->rewrites) +
            service->trace_deletion_capacity *
                    sizeof(*service->trace_deletions);
}

static unsigned int flow_proof_expand_capabilities(
        unsigned int capabilities) {
    if (capabilities & RXAS_FLOW_CAP_USE)
        capabilities |= RXAS_FLOW_CAP_VALUE | RXAS_FLOW_CAP_STORAGE |
                        RXAS_FLOW_CAP_SIGNAL | RXAS_FLOW_CAP_DOMINANCE;
    if (capabilities & (RXAS_FLOW_CAP_VALUE | RXAS_FLOW_CAP_STORAGE))
        capabilities |= RXAS_FLOW_CAP_VALUE | RXAS_FLOW_CAP_STORAGE |
                        RXAS_FLOW_CAP_SIGNAL | RXAS_FLOW_CAP_DOMINANCE;
    if (capabilities & RXAS_FLOW_CAP_SIGNAL)
        capabilities |= RXAS_FLOW_CAP_DOMINANCE;
    if (capabilities & RXAS_FLOW_CAP_LOOPS)
        capabilities |= RXAS_FLOW_CAP_DOMINANCE;
    if (capabilities & (RXAS_FLOW_CAP_DOMINANCE | RXAS_FLOW_CAP_SIGNAL |
                        RXAS_FLOW_CAP_STORAGE | RXAS_FLOW_CAP_VALUE |
                        RXAS_FLOW_CAP_USE | RXAS_FLOW_CAP_LOOPS))
        capabilities |= RXAS_FLOW_CAP_CFG;
    return capabilities;
}

static int flow_proof_acquire_capabilities(
        RxasFlowProofService *service, unsigned int requested) {
    unsigned int capabilities;
    unsigned long epoch;
    if (!service || !requested ||
        (requested & ~FLOW_PROOF_ALLOWED_CAPABILITIES))
        return 0;
    capabilities = flow_proof_expand_capabilities(requested);
    service->metrics.requested_capabilities |= requested;
    if ((service->metrics.acquired_capabilities & capabilities) ==
            capabilities)
        return 1;
    epoch = service->metrics.epoch;
    if ((capabilities & RXAS_FLOW_CAP_DOMINANCE) && !service->structural)
        service->structural = rxas_flow_require_structural_analysis(
                service->procedure, epoch, 0);
    if ((capabilities & RXAS_FLOW_CAP_LOOPS))
        service->structural = rxas_flow_require_loop_analysis(
                service->procedure, epoch, 0);
    if ((capabilities & RXAS_FLOW_CAP_SIGNAL) && !service->signal)
        service->signal = rxas_flow_require_signal_analysis(
                service->procedure, epoch, 0);
    if ((capabilities & (RXAS_FLOW_CAP_STORAGE | RXAS_FLOW_CAP_VALUE)) &&
        !service->ssa)
        service->ssa = rxas_flow_require_ssa_analysis(
                service->procedure, epoch, 0);
    if ((capabilities & RXAS_FLOW_CAP_USE) && !service->use)
        service->use = rxas_flow_require_use_analysis(
                service->procedure, epoch, 0);
    if ((capabilities & RXAS_FLOW_CAP_CFG) && !service->outgoing_offsets &&
        !flow_proof_build_adjacency(service))
        return 0;
    if (((capabilities & RXAS_FLOW_CAP_DOMINANCE) && !service->structural) ||
        ((capabilities & RXAS_FLOW_CAP_LOOPS) && !service->structural) ||
        ((capabilities & RXAS_FLOW_CAP_SIGNAL) && !service->signal) ||
        ((capabilities & (RXAS_FLOW_CAP_STORAGE | RXAS_FLOW_CAP_VALUE)) &&
         !service->ssa) ||
        ((capabilities & RXAS_FLOW_CAP_USE) && !service->use)) {
        /* A stronger capability may exhaust its own analysis budget while
         * the already-acquired facts remain sound.  Reject that route without
         * poisoning later lower-capability consumers in the same epoch. */
        return 0;
    }
    service->metrics.acquired_capabilities |= capabilities;
    flow_proof_set_retained_bytes(service);
    return 1;
}

static int flow_proof_valid(const RxasFlowProofService *service,
                            unsigned long epoch) {
    return service && epoch && service->metrics.epoch == epoch &&
           service->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE &&
           rxas_flow_procedure_epoch_matches(service->procedure, epoch);
}

static int flow_proof_has_capabilities(
        const RxasFlowProofService *service, unsigned int capabilities) {
    unsigned int required;
    if (!service) return 0;
    required = flow_proof_expand_capabilities(capabilities);
    return (service->metrics.acquired_capabilities & required) == required;
}

static int flow_proof_query_available(
        const RxasFlowProofService *service, unsigned long epoch,
        RxasFlowProofResult *result) {
    if (!service || !epoch || service->metrics.epoch != epoch ||
        !rxas_flow_procedure_epoch_matches(service->procedure, epoch)) {
        result->reason = RXAS_FLOW_PROOF_STALE_EPOCH;
        return 0;
    }
    if (service->metrics.status == RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED) {
        result->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        return 0;
    }
    if (service->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE) {
        result->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 0;
    }
    if (!flow_proof_has_capabilities(
                service, FLOW_PROOF_BASE_CAPABILITIES)) {
        result->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 0;
    }
    return 1;
}

static int flow_proof_instruction_dominates(
        const RxasFlowProofService *service,
        const RxasFlowInstruction *generator,
        const RxasFlowInstruction *candidate) {
    if (generator->block_id == candidate->block_id)
        return generator->record_id < candidate->record_id;
    return rxas_flow_structural_dominates(
            service->structural, service->metrics.epoch,
            generator->block_id, candidate->block_id);
}

static int flow_proof_mark_root(RxasFlowProofService *service, size_t root,
                                size_t *tail) {
    if (root == RXAS_FLOW_ID_NONE || root >= service->block_count ||
        service->visit_marks[root] == service->visit_generation)
        return 1;
    service->visit_marks[root] = service->visit_generation;
    service->visit_queue[(*tail)++] = root;
    return flow_proof_consume(service, 1);
}

static int flow_proof_success_edge_dominates(
        RxasFlowProofService *service, const RxasFlowInstruction *generator,
        const RxasFlowInstruction *candidate) {
    size_t normal_edge;
    size_t normal_count;
    size_t offset;
    size_t head;
    size_t tail;
    service->metrics.success_edge_queries++;
    normal_edge = RXAS_FLOW_ID_NONE;
    normal_count = 0;
    for (offset = service->outgoing_offsets[generator->block_id];
         offset < service->outgoing_offsets[generator->block_id + 1]; offset++) {
        size_t edge_id;
        const RxasFlowEdge *edge;
        edge_id = service->outgoing_edges[offset];
        edge = rxas_flow_procedure_edge(
                service->procedure, service->metrics.epoch, edge_id);
        if (edge && edge->kind == RXAS_FLOW_EDGE_NORMAL) {
            normal_edge = edge_id;
            normal_count++;
        }
    }
    if (normal_count != 1) return 0;
    service->visit_generation++;
    if (!service->visit_generation) {
        memset(service->visit_marks, 0,
               service->block_count * sizeof(*service->visit_marks));
        service->visit_generation = 1;
    }
    head = 0;
    tail = 0;
    if (!flow_proof_mark_root(
                service, rxas_flow_procedure_entry_block(
                        service->procedure, service->metrics.epoch), &tail) ||
        !flow_proof_mark_root(
                service, rxas_flow_procedure_handler_root(
                        service->procedure, service->metrics.epoch), &tail) ||
        !flow_proof_mark_root(
                service, rxas_flow_procedure_async_root(
                        service->procedure, service->metrics.epoch), &tail))
        return 0;
    while (head < tail) {
        size_t block;
        block = service->visit_queue[head++];
        if (!flow_proof_consume(service, 1)) return 0;
        for (offset = service->outgoing_offsets[block];
             offset < service->outgoing_offsets[block + 1]; offset++) {
            size_t edge_id;
            const RxasFlowEdge *edge;
            edge_id = service->outgoing_edges[offset];
            if (edge_id == normal_edge) continue;
            edge = rxas_flow_procedure_edge(
                    service->procedure, service->metrics.epoch, edge_id);
            if (!flow_proof_mark_root(service, edge->target, &tail)) return 0;
        }
    }
    return service->visit_marks[candidate->block_id] !=
           service->visit_generation;
}

static unsigned int flow_proof_dependency_for_effect(size_t effect) {
    if (effect == RXAS_FLOW_EFFECT_NUMERIC_CONTEXT)
        return RXOP_SIGNAL_DEP_NUMERIC_CONTEXT;
    if (effect == RXAS_FLOW_EFFECT_PLUGIN)
        return RXOP_SIGNAL_DEP_PLUGIN;
    if (effect == RXAS_FLOW_EFFECT_LOCALE)
        return RXOP_SIGNAL_DEP_LOCALE;
    if (effect == RXAS_FLOW_EFFECT_EXTERNAL)
        return RXOP_SIGNAL_DEP_EXTERNAL_STATE;
    return RXOP_SIGNAL_DEP_NONE;
}

static int flow_proof_append_cache(RxasFlowProofService *service,
                                   const RxasFlowProofResult *result) {
    FlowProofCacheEntry *resized;
    size_t next;
    if (service->cache_count == service->cache_capacity) {
        if (service->cache_capacity > (size_t)-1 / 2 /
                                      sizeof(*service->cache)) {
            service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        next = service->cache_capacity ? service->cache_capacity * 2 : 16;
        resized = realloc(service->cache, next * sizeof(*service->cache));
        if (!resized) {
            service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        service->cache = resized;
        service->cache_capacity = next;
        service->metrics.retained_bytes = sizeof(*service) +
                (service->block_count * 3 + service->edge_count + 1) *
                        sizeof(size_t) +
                service->value_capacity * 4 * sizeof(size_t) +
                service->storage_capacity * 2 * sizeof(size_t) +
                service->effect_capacity * 3 * sizeof(size_t) +
                service->cache_capacity * sizeof(*service->cache) +
                service->rewrite_capacity * sizeof(*service->rewrites) +
                service->trace_deletion_capacity *
                        sizeof(*service->trace_deletions);
    }
    service->cache[service->cache_count].generator =
            result->generator_instruction;
    service->cache[service->cache_count].candidate =
            result->candidate_instruction;
    service->cache[service->cache_count].result = *result;
    service->cache_count++;
    return 1;
}

static int flow_proof_append_rewrite(
        RxasFlowProofService *service, const RxasFlowUse *use,
        RxasFlowRegister replacement) {
    RxasFlowOperandRewrite *resized;
    size_t next;
    if (!service || !use) return 0;
    if (service->rewrite_count == service->rewrite_capacity) {
        if (service->rewrite_capacity > (size_t)-1 / 2 /
                                        sizeof(*service->rewrites)) {
            service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        next = service->rewrite_capacity
                ? service->rewrite_capacity * 2 : 16;
        resized = realloc(service->rewrites,
                          next * sizeof(*service->rewrites));
        if (!resized) {
            service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        service->rewrites = resized;
        service->rewrite_capacity = next;
    }
    service->rewrites[service->rewrite_count].record_id = use->record_id;
    service->rewrites[service->rewrite_count].instruction_id =
            use->instruction_id;
    service->rewrites[service->rewrite_count].operand_index =
            use->operand_index;
    service->rewrites[service->rewrite_count].expected_register =
            use->register_id;
    service->rewrites[service->rewrite_count].replacement_register =
            replacement;
    service->rewrite_count++;
    return flow_proof_consume(service, 1);
}

static int flow_proof_append_trace_deletion(
        RxasFlowProofService *service, const RxasFlowUse *use) {
    RxasFlowTraceDeletion *resized;
    size_t next;
    if (!service || !use || use->kind != RXAS_FLOW_USE_TRACE_READ ||
        use->record_id == RXAS_FLOW_ID_NONE ||
        use->value_id == RXAS_FLOW_ID_NONE ||
        use->component == RXOP_COMPONENT_NONE)
        return 0;
    if (service->trace_deletion_count == service->trace_deletion_capacity) {
        if (service->trace_deletion_capacity > (size_t)-1 / 2 /
                    sizeof(*service->trace_deletions)) {
            service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        next = service->trace_deletion_capacity
                ? service->trace_deletion_capacity * 2 : 8;
        resized = realloc(service->trace_deletions,
                          next * sizeof(*service->trace_deletions));
        if (!resized) {
            service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        service->trace_deletions = resized;
        service->trace_deletion_capacity = next;
    }
    service->trace_deletions[service->trace_deletion_count].record_id =
            use->record_id;
    service->trace_deletions[service->trace_deletion_count].value_id =
            use->value_id;
    service->trace_deletions[service->trace_deletion_count].component =
            use->component;
    service->trace_deletions[
            service->trace_deletion_count].expected_register =
            use->register_id;
    service->trace_deletion_count++;
    return flow_proof_consume(service, 1);
}

static int flow_proof_prepare_value_walk(RxasFlowProofService *service) {
    size_t count;
    size_t old_capacity;
    size_t *marks;
    size_t *stack;
    size_t *aux;
    size_t *set_marks;
    count = rxas_flow_value_version_count(
            service->ssa, service->metrics.epoch);
    if (!count) return 0;
    if (count <= service->value_capacity) return 1;
    old_capacity = service->value_capacity;
    marks = realloc(service->value_marks, count * sizeof(*marks));
    if (!marks) {
        service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    service->value_marks = marks;
    stack = realloc(service->value_stack, count * sizeof(*stack));
    if (!stack) {
        service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    service->value_stack = stack;
    aux = realloc(service->value_aux, count * sizeof(*aux));
    if (!aux) {
        service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    service->value_aux = aux;
    set_marks = realloc(service->value_set_marks,
                        count * sizeof(*set_marks));
    if (!set_marks) {
        service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    service->value_set_marks = set_marks;
    memset(service->value_marks + old_capacity, 0,
           (count - old_capacity) * sizeof(*service->value_marks));
    memset(service->value_set_marks + old_capacity, 0,
           (count - old_capacity) * sizeof(*service->value_set_marks));
    service->value_capacity = count;
    service->metrics.retained_bytes = sizeof(*service) +
            (service->block_count * 3 + service->edge_count + 1) *
                    sizeof(size_t) +
            service->value_capacity * 4 * sizeof(size_t) +
            service->storage_capacity * 2 * sizeof(size_t) +
            service->effect_capacity * 3 * sizeof(size_t) +
            service->cache_capacity * sizeof(*service->cache) +
            service->rewrite_capacity * sizeof(*service->rewrites) +
            service->trace_deletion_capacity *
                    sizeof(*service->trace_deletions);
    return 1;
}

typedef enum FlowProofValueWalkResult {
    FLOW_PROOF_VALUE_UNAVAILABLE = 0,
    FLOW_PROOF_VALUE_UNIQUE,
    FLOW_PROOF_VALUE_MULTIPLE
} FlowProofValueWalkResult;

typedef enum FlowProofStorageWalkResult {
    FLOW_PROOF_STORAGE_UNAVAILABLE = 0,
    FLOW_PROOF_STORAGE_UNIQUE,
    FLOW_PROOF_STORAGE_MULTIPLE
} FlowProofStorageWalkResult;

typedef enum FlowProofEffectWalkResult {
    FLOW_PROOF_EFFECT_UNAVAILABLE = 0,
    FLOW_PROOF_EFFECT_UNIQUE,
    FLOW_PROOF_EFFECT_MULTIPLE
} FlowProofEffectWalkResult;

static int flow_proof_prepare_storage_walk(RxasFlowProofService *service) {
    size_t count;
    size_t old_capacity;
    size_t *marks;
    size_t *stack;
    count = rxas_flow_storage_version_count(
            service->ssa, service->metrics.epoch);
    if (!count) return 0;
    if (count < service->storage_capacity) return 1;
    old_capacity = service->storage_capacity;
    marks = realloc(service->storage_marks, (count + 1) * sizeof(*marks));
    if (!marks) {
        service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    service->storage_marks = marks;
    stack = realloc(service->storage_stack, (count + 1) * sizeof(*stack));
    if (!stack) {
        service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    service->storage_stack = stack;
    memset(service->storage_marks + old_capacity, 0,
           (count + 1 - old_capacity) * sizeof(*service->storage_marks));
    service->storage_capacity = count + 1;
    service->metrics.retained_bytes = sizeof(*service) +
            (service->block_count * 3 + service->edge_count + 1) *
                    sizeof(size_t) +
            service->value_capacity * 4 * sizeof(size_t) +
            service->storage_capacity * 2 * sizeof(size_t) +
            service->effect_capacity * 3 * sizeof(size_t) +
            service->cache_capacity * sizeof(*service->cache) +
            service->rewrite_capacity * sizeof(*service->rewrites) +
            service->trace_deletion_capacity *
                    sizeof(*service->trace_deletions);
    return 1;
}

static int flow_proof_prepare_effect_walk(RxasFlowProofService *service) {
    size_t count;
    size_t old_capacity;
    size_t *marks;
    size_t *stack;
    size_t *set_marks;
    count = rxas_flow_effect_version_count(
            service->signal, service->metrics.epoch);
    if (!count) return 0;
    if (count <= service->effect_capacity) return 1;
    old_capacity = service->effect_capacity;
    marks = realloc(service->effect_marks, count * sizeof(*marks));
    if (!marks) {
        service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    service->effect_marks = marks;
    stack = realloc(service->effect_stack, count * sizeof(*stack));
    if (!stack) {
        service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    service->effect_stack = stack;
    set_marks = realloc(service->effect_set_marks,
                        count * sizeof(*set_marks));
    if (!set_marks) {
        service->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
        return 0;
    }
    service->effect_set_marks = set_marks;
    memset(service->effect_marks + old_capacity, 0,
           (count - old_capacity) * sizeof(*service->effect_marks));
    memset(service->effect_set_marks + old_capacity, 0,
           (count - old_capacity) * sizeof(*service->effect_set_marks));
    service->effect_capacity = count;
    service->metrics.retained_bytes = sizeof(*service) +
            (service->block_count * 3 + service->edge_count + 1) *
                    sizeof(size_t) +
            service->value_capacity * 4 * sizeof(size_t) +
            service->storage_capacity * 2 * sizeof(size_t) +
            service->effect_capacity * 3 * sizeof(size_t) +
            service->cache_capacity * sizeof(*service->cache) +
            service->rewrite_capacity * sizeof(*service->rewrites) +
            service->trace_deletion_capacity *
                    sizeof(*service->trace_deletions);
    return 1;
}

/* Effect versions are write-once tokens.  A phi is the same token when every
 * reachable non-cyclic input is the same entry/write leaf. */
static FlowProofEffectWalkResult flow_proof_unique_effect_leaf(
        RxasFlowProofService *service, size_t effect_id, size_t *leaf_id) {
    RxasFlowEffectNode first;
    size_t head;
    size_t tail;
    size_t leaf;
    int have_leaf;
    if (!leaf_id || !flow_proof_prepare_effect_walk(service) ||
        !rxas_flow_effect_node(
                service->signal, service->metrics.epoch,
                effect_id, &first) || first.id >= service->effect_capacity)
        return FLOW_PROOF_EFFECT_UNAVAILABLE;
    service->effect_generation++;
    if (!service->effect_generation) {
        memset(service->effect_marks, 0,
               service->effect_capacity * sizeof(*service->effect_marks));
        service->effect_generation = 1;
    }
    head = 0;
    tail = 0;
    leaf = RXAS_FLOW_ID_NONE;
    have_leaf = 0;
    service->effect_marks[first.id] = service->effect_generation;
    service->effect_stack[tail++] = first.id;
    while (head < tail) {
        RxasFlowEffectNode node;
        size_t input;
        effect_id = service->effect_stack[head++];
        if (!rxas_flow_effect_node(
                    service->signal, service->metrics.epoch,
                    effect_id, &node) || node.id >= service->effect_capacity ||
            !flow_proof_consume(service, 1))
            return FLOW_PROOF_EFFECT_UNAVAILABLE;
        if (node.kind == RXAS_FLOW_EFFECT_NODE_PHI) {
            if (!node.input_count) return FLOW_PROOF_EFFECT_UNAVAILABLE;
            for (input = 0; input < node.input_count; input++) {
                size_t input_id;
                RxasFlowEffectNode input_node;
                input_id = rxas_flow_effect_input(
                        service->signal, service->metrics.epoch,
                        node.id, input);
                if (input_id == RXAS_FLOW_ID_NONE ||
                    !rxas_flow_effect_node(
                            service->signal, service->metrics.epoch,
                            input_id, &input_node) ||
                    input_node.id >= service->effect_capacity)
                    return FLOW_PROOF_EFFECT_UNAVAILABLE;
                if (service->effect_marks[input_node.id] ==
                        service->effect_generation)
                    continue;
                service->effect_marks[input_node.id] =
                        service->effect_generation;
                if (tail >= service->effect_capacity)
                    return FLOW_PROOF_EFFECT_UNAVAILABLE;
                service->effect_stack[tail++] = input_node.id;
            }
            continue;
        }
        if (node.kind == RXAS_FLOW_EFFECT_NODE_UNKNOWN)
            return FLOW_PROOF_EFFECT_UNAVAILABLE;
        if (!have_leaf) {
            leaf = node.id;
            have_leaf = 1;
        }
        else if (leaf != node.id) return FLOW_PROOF_EFFECT_MULTIPLE;
    }
    if (!have_leaf) return FLOW_PROOF_EFFECT_UNAVAILABLE;
    *leaf_id = leaf;
    return FLOW_PROOF_EFFECT_UNIQUE;
}

static FlowProofEffectWalkResult flow_proof_effect_leaf_set_walk(
        RxasFlowProofService *service, size_t effect_id, int record_set,
        size_t *leaf_count) {
    RxasFlowEffectNode first;
    size_t head;
    size_t tail;
    if (!leaf_count || !flow_proof_prepare_effect_walk(service) ||
        !rxas_flow_effect_node(
                service->signal, service->metrics.epoch,
                effect_id, &first) || first.id >= service->effect_capacity)
        return FLOW_PROOF_EFFECT_UNAVAILABLE;
    service->effect_generation++;
    if (!service->effect_generation) {
        memset(service->effect_marks, 0,
               service->effect_capacity * sizeof(*service->effect_marks));
        service->effect_generation = 1;
    }
    head = 0;
    tail = 0;
    *leaf_count = 0;
    service->effect_marks[first.id] = service->effect_generation;
    service->effect_stack[tail++] = first.id;
    while (head < tail) {
        RxasFlowEffectNode node;
        size_t input;
        effect_id = service->effect_stack[head++];
        if (!rxas_flow_effect_node(
                    service->signal, service->metrics.epoch,
                    effect_id, &node) || node.id >= service->effect_capacity ||
            !flow_proof_consume(service, 1))
            return FLOW_PROOF_EFFECT_UNAVAILABLE;
        if (node.kind == RXAS_FLOW_EFFECT_NODE_PHI) {
            if (!node.input_count) return FLOW_PROOF_EFFECT_UNAVAILABLE;
            for (input = 0; input < node.input_count; input++) {
                size_t input_id;
                RxasFlowEffectNode input_node;
                input_id = rxas_flow_effect_input(
                        service->signal, service->metrics.epoch,
                        node.id, input);
                if (input_id == RXAS_FLOW_ID_NONE ||
                    !rxas_flow_effect_node(
                            service->signal, service->metrics.epoch,
                            input_id, &input_node) ||
                    input_node.id >= service->effect_capacity)
                    return FLOW_PROOF_EFFECT_UNAVAILABLE;
                if (service->effect_marks[input_node.id] ==
                        service->effect_generation)
                    continue;
                service->effect_marks[input_node.id] =
                        service->effect_generation;
                if (tail >= service->effect_capacity)
                    return FLOW_PROOF_EFFECT_UNAVAILABLE;
                service->effect_stack[tail++] = input_node.id;
            }
            continue;
        }
        if (node.kind == RXAS_FLOW_EFFECT_NODE_UNKNOWN)
            return FLOW_PROOF_EFFECT_UNAVAILABLE;
        if (record_set) {
            if (service->effect_set_marks[node.id] !=
                    service->effect_set_generation) {
                service->effect_set_marks[node.id] =
                        service->effect_set_generation;
                (*leaf_count)++;
            }
        }
        else {
            if (service->effect_set_marks[node.id] !=
                    service->effect_set_generation)
                return FLOW_PROOF_EFFECT_MULTIPLE;
            (*leaf_count)++;
        }
    }
    return *leaf_count ? FLOW_PROOF_EFFECT_UNIQUE
                       : FLOW_PROOF_EFFECT_UNAVAILABLE;
}

static FlowProofEffectWalkResult flow_proof_equivalent_effect_leaf_set(
        RxasFlowProofService *service, size_t left_id, size_t right_id) {
    FlowProofEffectWalkResult walk;
    size_t left_count;
    size_t right_count;
    if (!flow_proof_prepare_effect_walk(service))
        return FLOW_PROOF_EFFECT_UNAVAILABLE;
    service->effect_set_generation++;
    if (!service->effect_set_generation) {
        memset(service->effect_set_marks, 0,
               service->effect_capacity * sizeof(*service->effect_set_marks));
        service->effect_set_generation = 1;
    }
    walk = flow_proof_effect_leaf_set_walk(
            service, left_id, 1, &left_count);
    if (walk != FLOW_PROOF_EFFECT_UNIQUE) return walk;
    walk = flow_proof_effect_leaf_set_walk(
            service, right_id, 0, &right_count);
    if (walk != FLOW_PROOF_EFFECT_UNIQUE) return walk;
    return left_count == right_count ? FLOW_PROOF_EFFECT_UNIQUE
                                     : FLOW_PROOF_EFFECT_MULTIPLE;
}

typedef enum FlowProofEffectReduceResult {
    FLOW_PROOF_EFFECT_REDUCE_UNAVAILABLE = 0,
    FLOW_PROOF_EFFECT_REDUCE_NO,
    FLOW_PROOF_EFFECT_REDUCE_YES
} FlowProofEffectReduceResult;

/* Prove that a phi wrapper denotes an existing effect token.  This handles
 * loop/retry forms such as phi(anchor, self) without requiring the anchor
 * itself to have only one compile-time predecessor. */
static FlowProofEffectReduceResult flow_proof_effect_reduces_to(
        RxasFlowProofService *service, size_t wrapper_id, size_t anchor_id) {
    RxasFlowEffectNode wrapper;
    RxasFlowEffectNode anchor;
    size_t head;
    size_t tail;
    int saw_anchor;
    if (!flow_proof_prepare_effect_walk(service) ||
        !rxas_flow_effect_node(
                service->signal, service->metrics.epoch,
                wrapper_id, &wrapper) ||
        !rxas_flow_effect_node(
                service->signal, service->metrics.epoch,
                anchor_id, &anchor) ||
        wrapper.id >= service->effect_capacity ||
        anchor.id >= service->effect_capacity ||
        wrapper.effect_class != anchor.effect_class)
        return FLOW_PROOF_EFFECT_REDUCE_UNAVAILABLE;
    if (wrapper.id == anchor.id) return FLOW_PROOF_EFFECT_REDUCE_YES;
    service->effect_generation++;
    if (!service->effect_generation) {
        memset(service->effect_marks, 0,
               service->effect_capacity * sizeof(*service->effect_marks));
        service->effect_generation = 1;
    }
    head = 0;
    tail = 0;
    saw_anchor = 0;
    service->effect_marks[wrapper.id] = service->effect_generation;
    service->effect_stack[tail++] = wrapper.id;
    while (head < tail) {
        RxasFlowEffectNode node;
        size_t input;
        wrapper_id = service->effect_stack[head++];
        if (!rxas_flow_effect_node(
                    service->signal, service->metrics.epoch,
                    wrapper_id, &node) || node.id >= service->effect_capacity ||
            !flow_proof_consume(service, 1))
            return FLOW_PROOF_EFFECT_REDUCE_UNAVAILABLE;
        if (node.id == anchor.id) {
            saw_anchor = 1;
            continue;
        }
        if (node.kind != RXAS_FLOW_EFFECT_NODE_PHI || !node.input_count)
            return FLOW_PROOF_EFFECT_REDUCE_NO;
        for (input = 0; input < node.input_count; input++) {
            size_t input_id;
            RxasFlowEffectNode input_node;
            input_id = rxas_flow_effect_input(
                    service->signal, service->metrics.epoch,
                    node.id, input);
            if (input_id == RXAS_FLOW_ID_NONE ||
                !rxas_flow_effect_node(
                        service->signal, service->metrics.epoch,
                        input_id, &input_node) ||
                input_node.id >= service->effect_capacity)
                return FLOW_PROOF_EFFECT_REDUCE_UNAVAILABLE;
            if (service->effect_marks[input_node.id] ==
                    service->effect_generation)
                continue;
            service->effect_marks[input_node.id] = service->effect_generation;
            if (tail >= service->effect_capacity)
                return FLOW_PROOF_EFFECT_REDUCE_UNAVAILABLE;
            service->effect_stack[tail++] = input_node.id;
        }
    }
    return saw_anchor ? FLOW_PROOF_EFFECT_REDUCE_YES
                      : FLOW_PROOF_EFFECT_REDUCE_NO;
}

static FlowProofEffectWalkResult flow_proof_equivalent_effect(
        RxasFlowProofService *service, size_t left_id, size_t right_id) {
    FlowProofEffectReduceResult reduce;
    FlowProofEffectWalkResult walk;
    size_t left_leaf;
    size_t right_leaf;
    if (left_id == right_id) return FLOW_PROOF_EFFECT_UNIQUE;
    reduce = flow_proof_effect_reduces_to(service, right_id, left_id);
    if (reduce == FLOW_PROOF_EFFECT_REDUCE_UNAVAILABLE)
        return FLOW_PROOF_EFFECT_UNAVAILABLE;
    if (reduce == FLOW_PROOF_EFFECT_REDUCE_YES)
        return FLOW_PROOF_EFFECT_UNIQUE;
    reduce = flow_proof_effect_reduces_to(service, left_id, right_id);
    if (reduce == FLOW_PROOF_EFFECT_REDUCE_UNAVAILABLE)
        return FLOW_PROOF_EFFECT_UNAVAILABLE;
    if (reduce == FLOW_PROOF_EFFECT_REDUCE_YES)
        return FLOW_PROOF_EFFECT_UNIQUE;
    walk = flow_proof_unique_effect_leaf(service, left_id, &left_leaf);
    if (walk == FLOW_PROOF_EFFECT_UNAVAILABLE) return walk;
    if (walk == FLOW_PROOF_EFFECT_UNIQUE) {
        walk = flow_proof_unique_effect_leaf(service, right_id, &right_leaf);
        if (walk == FLOW_PROOF_EFFECT_UNAVAILABLE) return walk;
        if (walk == FLOW_PROOF_EFFECT_UNIQUE)
            return left_leaf == right_leaf ? FLOW_PROOF_EFFECT_UNIQUE
                                           : FLOW_PROOF_EFFECT_MULTIPLE;
    }
    return flow_proof_equivalent_effect_leaf_set(
            service, left_id, right_id);
}

/* A loop-carried storage phi denotes one storage when every non-cyclic leaf
 * is the same write-once StorageId. The caller decides whether a non-base
 * leaf has enough dynamic-execution proof for its use. */
static FlowProofStorageWalkResult flow_proof_unique_storage_leaf(
        RxasFlowProofService *service, size_t storage_id, size_t *leaf_id,
        RxasFlowStorageNode *leaf_node) {
    size_t head;
    size_t tail;
    size_t leaf;
    int have_leaf;
    RxasFlowStorageNode first_node;
    if (!leaf_id || !leaf_node || !flow_proof_prepare_storage_walk(service))
        return FLOW_PROOF_STORAGE_UNAVAILABLE;
    service->storage_generation++;
    if (!service->storage_generation) {
        memset(service->storage_marks, 0,
               service->storage_capacity * sizeof(*service->storage_marks));
        service->storage_generation = 1;
    }
    if (!rxas_flow_storage_node(
                service->ssa, service->metrics.epoch,
                storage_id, &first_node) ||
        first_node.id >= service->storage_capacity)
        return FLOW_PROOF_STORAGE_UNAVAILABLE;
    head = 0;
    tail = 0;
    service->storage_marks[first_node.id] = service->storage_generation;
    service->storage_stack[tail++] = first_node.id;
    leaf = RXAS_FLOW_ID_NONE;
    have_leaf = 0;
    while (head < tail) {
        RxasFlowStorageNode node;
        size_t input;
        storage_id = service->storage_stack[head++];
        if (!rxas_flow_storage_node(
                    service->ssa, service->metrics.epoch,
                    storage_id, &node) ||
            node.id >= service->storage_capacity)
            return FLOW_PROOF_STORAGE_UNAVAILABLE;
        if (!flow_proof_consume(service, 1))
            return FLOW_PROOF_STORAGE_UNAVAILABLE;
        if (node.kind == RXAS_FLOW_STORAGE_PHI) {
            for (input = 0; input < node.input_count; input++) {
                size_t input_id;
                RxasFlowStorageNode input_node;
                input_id = rxas_flow_storage_input(
                        service->ssa, service->metrics.epoch,
                        node.id, input);
                if (input_id == RXAS_FLOW_ID_NONE ||
                    !rxas_flow_storage_node(
                            service->ssa, service->metrics.epoch,
                            input_id, &input_node) ||
                    input_node.id >= service->storage_capacity)
                    return FLOW_PROOF_STORAGE_UNAVAILABLE;
                if (service->storage_marks[input_node.id] ==
                    service->storage_generation)
                    continue;
                service->storage_marks[input_node.id] =
                        service->storage_generation;
                if (tail >= service->storage_capacity)
                    return FLOW_PROOF_STORAGE_UNAVAILABLE;
                service->storage_stack[tail++] = input_node.id;
            }
            continue;
        }
        if (node.kind == RXAS_FLOW_STORAGE_UNKNOWN)
            return FLOW_PROOF_STORAGE_UNAVAILABLE;
        if (!have_leaf) {
            leaf = node.id;
            *leaf_node = node;
            have_leaf = 1;
        }
        else if (leaf != node.id) return FLOW_PROOF_STORAGE_MULTIPLE;
    }
    if (!have_leaf) return FLOW_PROOF_STORAGE_UNAVAILABLE;
    *leaf_id = leaf;
    return FLOW_PROOF_STORAGE_UNIQUE;
}

/* A cyclic phi SCC denotes one value when all reachable non-cyclic leaves are
 * the same write-once ValueId. COPY nodes are transparent identities. */
static FlowProofValueWalkResult flow_proof_unique_value_leaf(
        RxasFlowProofService *service, size_t value_id, size_t *leaf_id,
        RxasFlowValueNode *leaf_node) {
    size_t head;
    size_t tail;
    size_t leaf;
    int have_leaf;
    RxasFlowValueNode first_node;
    if (!leaf_id || !leaf_node || !flow_proof_prepare_value_walk(service))
        return FLOW_PROOF_VALUE_UNAVAILABLE;
    service->value_generation++;
    if (!service->value_generation) {
        memset(service->value_marks, 0,
               service->value_capacity * sizeof(*service->value_marks));
        service->value_generation = 1;
    }
    head = 0;
    tail = 0;
    if (!rxas_flow_value_node(
                service->ssa, service->metrics.epoch,
                value_id, &first_node) ||
        first_node.id >= service->value_capacity)
        return FLOW_PROOF_VALUE_UNAVAILABLE;
    service->value_marks[first_node.id] = service->value_generation;
    service->value_stack[tail++] = first_node.id;
    leaf = RXAS_FLOW_ID_NONE;
    have_leaf = 0;
    while (head < tail) {
        RxasFlowValueNode node;
        size_t input;
        value_id = service->value_stack[head++];
        if (!rxas_flow_value_node(
                    service->ssa, service->metrics.epoch,
                    value_id, &node) || node.id >= service->value_capacity)
            return FLOW_PROOF_VALUE_UNAVAILABLE;
        if (!flow_proof_consume(service, 1))
            return FLOW_PROOF_VALUE_UNAVAILABLE;
        if (node.kind == RXAS_FLOW_VALUE_PHI) {
            for (input = 0; input < node.input_count; input++) {
                size_t input_id;
                RxasFlowValueNode input_node;
                input_id = rxas_flow_value_input(
                        service->ssa, service->metrics.epoch,
                        node.id, input);
                if (input_id == RXAS_FLOW_ID_NONE ||
                    !rxas_flow_value_node(
                            service->ssa, service->metrics.epoch,
                            input_id, &input_node) ||
                    input_node.id >= service->value_capacity)
                    return FLOW_PROOF_VALUE_UNAVAILABLE;
                if (service->value_marks[input_node.id] ==
                    service->value_generation)
                    continue;
                service->value_marks[input_node.id] =
                        service->value_generation;
                if (tail >= service->value_capacity)
                    return FLOW_PROOF_VALUE_UNAVAILABLE;
                service->value_stack[tail++] = input_node.id;
            }
            continue;
        }
        if (node.kind == RXAS_FLOW_VALUE_COPY) {
            RxasFlowValueNode source_node;
            if (node.source_value_id == RXAS_FLOW_ID_NONE ||
                !rxas_flow_value_node(
                        service->ssa, service->metrics.epoch,
                        node.source_value_id, &source_node) ||
                source_node.id >= service->value_capacity)
                return FLOW_PROOF_VALUE_UNAVAILABLE;
            if (service->value_marks[source_node.id] !=
                service->value_generation) {
                service->value_marks[source_node.id] =
                        service->value_generation;
                if (tail >= service->value_capacity)
                    return FLOW_PROOF_VALUE_UNAVAILABLE;
                service->value_stack[tail++] = source_node.id;
            }
            continue;
        }
        if (node.kind == RXAS_FLOW_VALUE_UNKNOWN)
            return FLOW_PROOF_VALUE_UNAVAILABLE;
        if (!have_leaf) {
            leaf = node.id;
            *leaf_node = node;
            have_leaf = 1;
        }
        else if (leaf != node.id) return FLOW_PROOF_VALUE_MULTIPLE;
    }
    if (!have_leaf) return FLOW_PROOF_VALUE_UNAVAILABLE;
    *leaf_id = leaf;
    return FLOW_PROOF_VALUE_UNIQUE;
}

/* Prove a semantic leaf property across copy wrappers and cyclic phis.  A
 * constant proof compares integer values or exact float bits; an absence
 * proof accepts distinct write-once ABSENT leaves because they denote the
 * same lack of lifetime-bearing state. */
static FlowProofValueWalkResult flow_proof_value_leaves_match(
        RxasFlowProofService *service, size_t value_id,
        const Assembler_Token *constant, int require_absent) {
    size_t head;
    size_t tail;
    size_t leaf_count;
    RxasFlowValueNode first;
    if (!flow_proof_prepare_value_walk(service) ||
        !rxas_flow_value_node(
                service->ssa, service->metrics.epoch, value_id, &first) ||
        first.id >= service->value_capacity)
        return FLOW_PROOF_VALUE_UNAVAILABLE;
    service->value_generation++;
    if (!service->value_generation) {
        memset(service->value_marks, 0,
               service->value_capacity * sizeof(*service->value_marks));
        service->value_generation = 1;
    }
    head = 0;
    tail = 0;
    leaf_count = 0;
    service->value_marks[first.id] = service->value_generation;
    service->value_stack[tail++] = first.id;
    while (head < tail) {
        RxasFlowValueNode node;
        size_t input;
        value_id = service->value_stack[head++];
        if (!rxas_flow_value_node(
                    service->ssa, service->metrics.epoch,
                    value_id, &node) || node.id >= service->value_capacity ||
            !flow_proof_consume(service, 1))
            return FLOW_PROOF_VALUE_UNAVAILABLE;
        if (node.kind == RXAS_FLOW_VALUE_PHI) {
            if (!node.input_count) return FLOW_PROOF_VALUE_UNAVAILABLE;
            for (input = 0; input < node.input_count; input++) {
                size_t input_id;
                RxasFlowValueNode input_node;
                input_id = rxas_flow_value_input(
                        service->ssa, service->metrics.epoch,
                        node.id, input);
                if (input_id == RXAS_FLOW_ID_NONE ||
                    !rxas_flow_value_node(
                            service->ssa, service->metrics.epoch,
                            input_id, &input_node) ||
                    input_node.id >= service->value_capacity)
                    return FLOW_PROOF_VALUE_UNAVAILABLE;
                if (service->value_marks[input_node.id] ==
                        service->value_generation)
                    continue;
                service->value_marks[input_node.id] =
                        service->value_generation;
                if (tail >= service->value_capacity)
                    return FLOW_PROOF_VALUE_UNAVAILABLE;
                service->value_stack[tail++] = input_node.id;
            }
            continue;
        }
        if (node.kind == RXAS_FLOW_VALUE_COPY) {
            RxasFlowValueNode source;
            if (node.source_value_id == RXAS_FLOW_ID_NONE ||
                !rxas_flow_value_node(
                        service->ssa, service->metrics.epoch,
                        node.source_value_id, &source) ||
                source.id >= service->value_capacity)
                return FLOW_PROOF_VALUE_UNAVAILABLE;
            if (service->value_marks[source.id] !=
                    service->value_generation) {
                service->value_marks[source.id] = service->value_generation;
                if (tail >= service->value_capacity)
                    return FLOW_PROOF_VALUE_UNAVAILABLE;
                service->value_stack[tail++] = source.id;
            }
            continue;
        }
        leaf_count++;
        if (require_absent) {
            if (node.kind != RXAS_FLOW_VALUE_ABSENT ||
                node.presence != RXAS_FLOW_COMPONENT_ABSENT)
                return FLOW_PROOF_VALUE_MULTIPLE;
        }
        else if (node.kind != RXAS_FLOW_VALUE_CONSTANT ||
                 node.presence != RXAS_FLOW_COMPONENT_PRESENT ||
                 !flow_proof_same_scalar_constant(
                        node.constant_token, constant))
            return FLOW_PROOF_VALUE_MULTIPLE;
    }
    return leaf_count ? FLOW_PROOF_VALUE_UNIQUE
                      : FLOW_PROOF_VALUE_UNAVAILABLE;
}

static FlowProofValueWalkResult flow_proof_value_leaf_set_walk(
        RxasFlowProofService *service, size_t value_id, int record_set,
        size_t *leaf_count) {
    RxasFlowValueNode first;
    size_t head;
    size_t tail;
    if (!leaf_count || !flow_proof_prepare_value_walk(service) ||
        !rxas_flow_value_node(
                service->ssa, service->metrics.epoch,
                value_id, &first) || first.id >= service->value_capacity)
        return FLOW_PROOF_VALUE_UNAVAILABLE;
    service->value_generation++;
    if (!service->value_generation) {
        memset(service->value_marks, 0,
               service->value_capacity * sizeof(*service->value_marks));
        service->value_generation = 1;
    }
    head = 0;
    tail = 0;
    *leaf_count = 0;
    service->value_marks[first.id] = service->value_generation;
    service->value_stack[tail++] = first.id;
    while (head < tail) {
        RxasFlowValueNode node;
        size_t input;
        value_id = service->value_stack[head++];
        if (!rxas_flow_value_node(
                    service->ssa, service->metrics.epoch,
                    value_id, &node) || node.id >= service->value_capacity ||
            !flow_proof_consume(service, 1))
            return FLOW_PROOF_VALUE_UNAVAILABLE;
        if (node.kind == RXAS_FLOW_VALUE_PHI) {
            if (!node.input_count) return FLOW_PROOF_VALUE_UNAVAILABLE;
            for (input = 0; input < node.input_count; input++) {
                size_t input_id;
                RxasFlowValueNode input_node;
                input_id = rxas_flow_value_input(
                        service->ssa, service->metrics.epoch,
                        node.id, input);
                if (input_id == RXAS_FLOW_ID_NONE ||
                    !rxas_flow_value_node(
                            service->ssa, service->metrics.epoch,
                            input_id, &input_node) ||
                    input_node.id >= service->value_capacity)
                    return FLOW_PROOF_VALUE_UNAVAILABLE;
                if (service->value_marks[input_node.id] ==
                        service->value_generation)
                    continue;
                service->value_marks[input_node.id] =
                        service->value_generation;
                if (tail >= service->value_capacity)
                    return FLOW_PROOF_VALUE_UNAVAILABLE;
                service->value_stack[tail++] = input_node.id;
            }
            continue;
        }
        if (node.kind == RXAS_FLOW_VALUE_COPY) {
            RxasFlowValueNode source;
            if (node.source_value_id == RXAS_FLOW_ID_NONE ||
                !rxas_flow_value_node(
                        service->ssa, service->metrics.epoch,
                        node.source_value_id, &source) ||
                source.id >= service->value_capacity)
                return FLOW_PROOF_VALUE_UNAVAILABLE;
            if (service->value_marks[source.id] !=
                    service->value_generation) {
                service->value_marks[source.id] = service->value_generation;
                if (tail >= service->value_capacity)
                    return FLOW_PROOF_VALUE_UNAVAILABLE;
                service->value_stack[tail++] = source.id;
            }
            continue;
        }
        if (node.kind == RXAS_FLOW_VALUE_UNKNOWN)
            return FLOW_PROOF_VALUE_UNAVAILABLE;
        if (record_set) {
            if (service->value_set_marks[node.id] !=
                    service->value_set_generation) {
                service->value_set_marks[node.id] =
                        service->value_set_generation;
                (*leaf_count)++;
            }
        }
        else {
            if (service->value_set_marks[node.id] !=
                    service->value_set_generation)
                return FLOW_PROOF_VALUE_MULTIPLE;
            (*leaf_count)++;
        }
    }
    return *leaf_count ? FLOW_PROOF_VALUE_UNIQUE
                       : FLOW_PROOF_VALUE_UNAVAILABLE;
}

/* Under the caller's dominance/storage proof, two phi SCCs with exactly the
 * same write-once leaves represent unchanged storage contents even when the
 * sparse state graph materialises distinct join nodes at two program points. */
static FlowProofValueWalkResult flow_proof_equivalent_value_leaf_set(
        RxasFlowProofService *service, size_t left_id, size_t right_id) {
    FlowProofValueWalkResult walk;
    size_t left_count;
    size_t right_count;
    if (!flow_proof_prepare_value_walk(service))
        return FLOW_PROOF_VALUE_UNAVAILABLE;
    service->value_set_generation++;
    if (!service->value_set_generation) {
        memset(service->value_set_marks, 0,
               service->value_capacity * sizeof(*service->value_set_marks));
        service->value_set_generation = 1;
    }
    walk = flow_proof_value_leaf_set_walk(
            service, left_id, 1, &left_count);
    if (walk != FLOW_PROOF_VALUE_UNIQUE) return walk;
    walk = flow_proof_value_leaf_set_walk(
            service, right_id, 0, &right_count);
    if (walk != FLOW_PROOF_VALUE_UNIQUE) return walk;
    return left_count == right_count ? FLOW_PROOF_VALUE_UNIQUE
                                     : FLOW_PROOF_VALUE_MULTIPLE;
}

typedef enum FlowProofValueReduceResult {
    FLOW_PROOF_VALUE_REDUCE_UNAVAILABLE = 0,
    FLOW_PROOF_VALUE_REDUCE_NO,
    FLOW_PROOF_VALUE_REDUCE_YES
} FlowProofValueReduceResult;

/* Prove that a copy/phi wrapper denotes one existing write-once ValueId.
 * The anchor itself may be a loop phi with several runtime alternatives; the
 * proof is that every wrapper input selects that same SSA value, not that the
 * anchor has one compile-time leaf. */
static FlowProofValueReduceResult flow_proof_value_reduces_to(
        RxasFlowProofService *service, size_t wrapper_id, size_t anchor_id) {
    RxasFlowValueNode wrapper;
    RxasFlowValueNode anchor;
    size_t head;
    size_t tail;
    int saw_anchor;
    if (!flow_proof_prepare_value_walk(service) ||
        !rxas_flow_value_node(
                service->ssa, service->metrics.epoch,
                wrapper_id, &wrapper) ||
        !rxas_flow_value_node(
                service->ssa, service->metrics.epoch,
                anchor_id, &anchor) ||
        wrapper.id >= service->value_capacity ||
        anchor.id >= service->value_capacity)
        return FLOW_PROOF_VALUE_REDUCE_UNAVAILABLE;
    if (wrapper.id == anchor.id) return FLOW_PROOF_VALUE_REDUCE_YES;
    service->value_generation++;
    if (!service->value_generation) {
        memset(service->value_marks, 0,
               service->value_capacity * sizeof(*service->value_marks));
        service->value_generation = 1;
    }
    head = 0;
    tail = 0;
    saw_anchor = 0;
    service->value_marks[wrapper.id] = service->value_generation;
    service->value_stack[tail++] = wrapper.id;
    while (head < tail) {
        RxasFlowValueNode node;
        size_t input;
        wrapper_id = service->value_stack[head++];
        if (!rxas_flow_value_node(
                    service->ssa, service->metrics.epoch,
                    wrapper_id, &node) ||
            node.id >= service->value_capacity ||
            !flow_proof_consume(service, 1))
            return FLOW_PROOF_VALUE_REDUCE_UNAVAILABLE;
        if (node.id == anchor.id) {
            saw_anchor = 1;
            continue;
        }
        if (node.kind == RXAS_FLOW_VALUE_COPY) {
            RxasFlowValueNode source;
            if (node.source_value_id == RXAS_FLOW_ID_NONE ||
                !rxas_flow_value_node(
                        service->ssa, service->metrics.epoch,
                        node.source_value_id, &source) ||
                source.id >= service->value_capacity)
                return FLOW_PROOF_VALUE_REDUCE_UNAVAILABLE;
            if (service->value_marks[source.id] !=
                    service->value_generation) {
                service->value_marks[source.id] = service->value_generation;
                if (tail >= service->value_capacity)
                    return FLOW_PROOF_VALUE_REDUCE_UNAVAILABLE;
                service->value_stack[tail++] = source.id;
            }
            continue;
        }
        if (node.kind != RXAS_FLOW_VALUE_PHI)
            return FLOW_PROOF_VALUE_REDUCE_NO;
        for (input = 0; input < node.input_count; input++) {
            size_t input_id;
            RxasFlowValueNode input_node;
            input_id = rxas_flow_value_input(
                    service->ssa, service->metrics.epoch,
                    node.id, input);
            if (input_id == RXAS_FLOW_ID_NONE ||
                !rxas_flow_value_node(
                        service->ssa, service->metrics.epoch,
                        input_id, &input_node) ||
                input_node.id >= service->value_capacity)
                return FLOW_PROOF_VALUE_REDUCE_UNAVAILABLE;
            if (service->value_marks[input_node.id] ==
                    service->value_generation)
                continue;
            service->value_marks[input_node.id] = service->value_generation;
            if (tail >= service->value_capacity)
                return FLOW_PROOF_VALUE_REDUCE_UNAVAILABLE;
            service->value_stack[tail++] = input_node.id;
        }
    }
    return saw_anchor ? FLOW_PROOF_VALUE_REDUCE_YES
                      : FLOW_PROOF_VALUE_REDUCE_NO;
}

static FlowProofValueWalkResult flow_proof_equivalent_value(
        RxasFlowProofService *service, size_t left_id, size_t right_id,
        size_t *root_id, RxasFlowValueNode *root_node) {
    RxasFlowValueNode left;
    RxasFlowValueNode right;
    RxasFlowValueNode left_leaf;
    RxasFlowValueNode right_leaf;
    size_t left_root;
    size_t right_root;
    FlowProofValueReduceResult reduce;
    FlowProofValueWalkResult walk;
    if (!root_id || !root_node ||
        !rxas_flow_value_node(
                service->ssa, service->metrics.epoch, left_id, &left) ||
        !rxas_flow_value_node(
                service->ssa, service->metrics.epoch, right_id, &right))
        return FLOW_PROOF_VALUE_UNAVAILABLE;
    if (left.id == right.id) {
        *root_id = left.id;
        *root_node = left;
        return FLOW_PROOF_VALUE_UNIQUE;
    }
    reduce = flow_proof_value_reduces_to(service, right.id, left.id);
    if (reduce == FLOW_PROOF_VALUE_REDUCE_UNAVAILABLE)
        return FLOW_PROOF_VALUE_UNAVAILABLE;
    if (reduce == FLOW_PROOF_VALUE_REDUCE_YES) {
        *root_id = left.id;
        *root_node = left;
        return FLOW_PROOF_VALUE_UNIQUE;
    }
    reduce = flow_proof_value_reduces_to(service, left.id, right.id);
    if (reduce == FLOW_PROOF_VALUE_REDUCE_UNAVAILABLE)
        return FLOW_PROOF_VALUE_UNAVAILABLE;
    if (reduce == FLOW_PROOF_VALUE_REDUCE_YES) {
        *root_id = right.id;
        *root_node = right;
        return FLOW_PROOF_VALUE_UNIQUE;
    }
    walk = flow_proof_unique_value_leaf(
            service, left.id, &left_root, &left_leaf);
    if (walk == FLOW_PROOF_VALUE_UNAVAILABLE) return walk;
    if (walk == FLOW_PROOF_VALUE_UNIQUE) {
        walk = flow_proof_unique_value_leaf(
                service, right.id, &right_root, &right_leaf);
        if (walk == FLOW_PROOF_VALUE_UNAVAILABLE) return walk;
        if (walk == FLOW_PROOF_VALUE_UNIQUE) {
            if (left_root != right_root) return FLOW_PROOF_VALUE_MULTIPLE;
            *root_id = left_root;
            *root_node = left_leaf;
            return FLOW_PROOF_VALUE_UNIQUE;
        }
    }
    walk = flow_proof_equivalent_value_leaf_set(
            service, left.id, right.id);
    if (walk != FLOW_PROOF_VALUE_UNIQUE) return walk;
    *root_id = left.id;
    *root_node = left;
    return FLOW_PROOF_VALUE_UNIQUE;
}

/* A result phi may contain the same pure derivation produced at different
 * static executions (for example, a source loop around a repeated ITOS). Prove
 * semantic value numbering from derivation, source ValueId and the exact
 * effect versions read by the operation; do not require one defining site. */
static FlowProofValueWalkResult flow_proof_equivalent_derivation_result(
        RxasFlowProofService *service, size_t value_id,
        const RxasFlowValueNode *generator_result, size_t source_root) {
    size_t head;
    size_t tail;
    size_t leaf_count;
    size_t leaf;
    if (!generator_result ||
        generator_result->kind != RXAS_FLOW_VALUE_DERIVED ||
        !flow_proof_prepare_value_walk(service))
        return FLOW_PROOF_VALUE_UNAVAILABLE;
    service->value_generation++;
    if (!service->value_generation) {
        memset(service->value_marks, 0,
               service->value_capacity * sizeof(*service->value_marks));
        service->value_generation = 1;
    }
    {
        RxasFlowValueNode root;
        if (!rxas_flow_value_node(
                    service->ssa, service->metrics.epoch,
                    value_id, &root) || root.id >= service->value_capacity)
            return FLOW_PROOF_VALUE_UNAVAILABLE;
        head = 0;
        tail = 0;
        leaf_count = 0;
        service->value_marks[root.id] = service->value_generation;
        service->value_stack[tail++] = root.id;
    }
    while (head < tail) {
        RxasFlowValueNode node;
        size_t input;
        value_id = service->value_stack[head++];
        if (!rxas_flow_value_node(
                    service->ssa, service->metrics.epoch,
                    value_id, &node) || node.id >= service->value_capacity ||
            !flow_proof_consume(service, 1))
            return FLOW_PROOF_VALUE_UNAVAILABLE;
        if (node.kind == RXAS_FLOW_VALUE_PHI) {
            for (input = 0; input < node.input_count; input++) {
                size_t input_id;
                RxasFlowValueNode input_node;
                input_id = rxas_flow_value_input(
                        service->ssa, service->metrics.epoch,
                        node.id, input);
                if (input_id == RXAS_FLOW_ID_NONE ||
                    !rxas_flow_value_node(
                            service->ssa, service->metrics.epoch,
                            input_id, &input_node) ||
                    input_node.id >= service->value_capacity)
                    return FLOW_PROOF_VALUE_UNAVAILABLE;
                if (service->value_marks[input_node.id] ==
                        service->value_generation)
                    continue;
                service->value_marks[input_node.id] =
                        service->value_generation;
                if (tail >= service->value_capacity)
                    return FLOW_PROOF_VALUE_UNAVAILABLE;
                service->value_stack[tail++] = input_node.id;
            }
            continue;
        }
        if (node.kind == RXAS_FLOW_VALUE_COPY) {
            RxasFlowValueNode source;
            if (node.source_value_id == RXAS_FLOW_ID_NONE ||
                !rxas_flow_value_node(
                        service->ssa, service->metrics.epoch,
                        node.source_value_id, &source) ||
                source.id >= service->value_capacity)
                return FLOW_PROOF_VALUE_UNAVAILABLE;
            if (service->value_marks[source.id] !=
                    service->value_generation) {
                service->value_marks[source.id] = service->value_generation;
                if (tail >= service->value_capacity)
                    return FLOW_PROOF_VALUE_UNAVAILABLE;
                service->value_stack[tail++] = source.id;
            }
            continue;
        }
        if (leaf_count >= service->value_capacity)
            return FLOW_PROOF_VALUE_UNAVAILABLE;
        service->value_aux[leaf_count++] = node.id;
    }
    if (!leaf_count) return FLOW_PROOF_VALUE_UNAVAILABLE;
    for (leaf = 0; leaf < leaf_count; leaf++) {
        RxasFlowValueNode node;
        RxasFlowValueNode equivalent_source;
        FlowProofValueWalkResult walk;
        size_t equivalent_source_id;
        size_t effect;
        FlowProofEffectWalkResult effect_walk;
        if (!rxas_flow_value_node(
                    service->ssa, service->metrics.epoch,
                    service->value_aux[leaf], &node) ||
            node.kind != RXAS_FLOW_VALUE_DERIVED ||
            node.derivation != generator_result->derivation ||
            node.signal_dependencies !=
                    generator_result->signal_dependencies)
            return FLOW_PROOF_VALUE_MULTIPLE;
        walk = flow_proof_equivalent_value(
                service, node.source_value_id, source_root,
                &equivalent_source_id, &equivalent_source);
        if (walk != FLOW_PROOF_VALUE_UNIQUE)
            return walk;
        for (effect = 0; effect < RXAS_FLOW_EFFECT_CLASS_COUNT; effect++) {
            unsigned int dependency;
            dependency = flow_proof_dependency_for_effect(effect);
            if (!(node.signal_dependencies & dependency)) continue;
            effect_walk = flow_proof_equivalent_effect(
                    service, node.definition_effects[effect],
                    generator_result->definition_effects[effect]);
            if (effect_walk == FLOW_PROOF_EFFECT_UNAVAILABLE)
                return FLOW_PROOF_VALUE_UNAVAILABLE;
            if (effect_walk == FLOW_PROOF_EFFECT_MULTIPLE)
                return FLOW_PROOF_VALUE_MULTIPLE;
        }
    }
    return FLOW_PROOF_VALUE_UNIQUE;
}

static void flow_proof_repetition_uncached(
        RxasFlowProofService *service, size_t generator_id,
        size_t candidate_id, RxasFlowProofResult *result) {
    const RxasFlowInstruction *generator;
    const RxasFlowInstruction *candidate;
    const RxasFlowRecord *generator_record;
    const RxasFlowRecord *candidate_record;
    const instruction_queue *generator_item;
    const instruction_queue *candidate_item;
    RxOpValueDerivation derivation;
    RxasFlowRegister generator_target;
    RxasFlowRegister candidate_target;
    RxasFlowRegister generator_source;
    RxasFlowRegister candidate_source;
    RxasFlowComponentFact generator_value;
    RxasFlowComponentFact candidate_value;
    RxasFlowComponentFact generator_source_value;
    RxasFlowComponentFact candidate_source_value;
    RxasFlowValueNode generator_source_leaf;
    RxasFlowValueNode candidate_source_leaf;
    RxasFlowValueNode generator_result_leaf;
    RxasFlowValueNode candidate_result_leaf;
    RxasFlowValueNode result_source_leaf;
    RxasFlowStorageNode generator_storage_leaf;
    RxasFlowStorageNode candidate_storage_leaf;
    unsigned int source_component;
    unsigned int target_component;
    size_t source_operand;
    size_t effect;
    size_t generator_source_root;
    size_t candidate_source_root;
    size_t generator_result_root;
    size_t candidate_result_root;
    size_t result_source_root;
    size_t generator_storage_root;
    size_t candidate_storage_root;
    size_t generator_aliases;
    size_t candidate_aliases;
    int generator_external;
    int candidate_external;
    int generator_alias_query;
    int candidate_alias_query;
    int local_storage;
    FlowProofValueWalkResult value_walk;
    FlowProofStorageWalkResult storage_walk;
    FlowProofEffectWalkResult effect_walk;
    generator = rxas_flow_procedure_instruction(
            service->procedure, service->metrics.epoch, generator_id);
    candidate = rxas_flow_procedure_instruction(
            service->procedure, service->metrics.epoch, candidate_id);
    if (!generator || !candidate || !generator->op || !candidate->op) {
        result->reason = RXAS_FLOW_PROOF_INVALID_INSTRUCTION;
        return;
    }
    if (generator->op->opcode != candidate->op->opcode) {
        result->reason = RXAS_FLOW_PROOF_OPCODE_MISMATCH;
        return;
    }
    derivation = rxop_value_derivation(generator->op->opcode);
    if (derivation == RXOP_DERIVATION_NONE ||
        !(generator->signal.properties & RXOP_SIGNAL_PROP_SUCCESS_STABLE)) {
        result->reason = RXAS_FLOW_PROOF_NOT_SUCCESS_STABLE;
        return;
    }
    if (!flow_proof_instruction_dominates(service, generator, candidate)) {
        result->reason = RXAS_FLOW_PROOF_NOT_DOMINATED;
        return;
    }
    if (generator->signal.state != RXOP_SIGNAL_STATE_NONE &&
        !flow_proof_success_edge_dominates(service, generator, candidate)) {
        result->reason = RXAS_FLOW_PROOF_SUCCESS_EDGE_NOT_DOMINATING;
        return;
    }
    if (generator->signal.dependencies &
        (RXOP_SIGNAL_DEP_HANDLER_POLICY | RXOP_SIGNAL_DEP_UNKNOWN)) {
        result->reason = RXAS_FLOW_PROOF_UNSUPPORTED_SIGNAL_DEPENDENCY;
        return;
    }
    generator_record = rxas_flow_procedure_record(
            service->procedure, service->metrics.epoch, generator->record_id);
    candidate_record = rxas_flow_procedure_record(
            service->procedure, service->metrics.epoch, candidate->record_id);
    generator_item = generator_record ? generator_record->queue_record : 0;
    candidate_item = candidate_record ? candidate_record->queue_record : 0;
    source_operand = rxop_derivation_source_operand(generator->op->opcode);
    source_component = rxop_derivation_source_component(generator->op->opcode);
    target_component = rxop_component_writes(generator->op->opcode, 0);
    if (!generator_item || !candidate_item || !source_component ||
        !target_component || (target_component & (target_component - 1)) ||
        !flow_proof_register(flow_proof_operand(generator_item, 0),
                             &generator_target) ||
        !flow_proof_register(flow_proof_operand(candidate_item, 0),
                             &candidate_target) ||
        !flow_proof_register(flow_proof_operand(generator_item, source_operand),
                             &generator_source) ||
        !flow_proof_register(flow_proof_operand(candidate_item, source_operand),
                             &candidate_source)) {
        result->reason = RXAS_FLOW_PROOF_INVALID_INSTRUCTION;
        return;
    }
    if (!rxas_flow_component_at_instruction(
                service->ssa, service->metrics.epoch, generator_id, 1,
                generator_target, target_component, &generator_value)) {
        result->reason = RXAS_FLOW_PROOF_GENERATOR_RESULT_UNKNOWN;
        return;
    }
    if (!rxas_flow_component_at_instruction(
                service->ssa, service->metrics.epoch, candidate_id, 0,
                candidate_target, target_component, &candidate_value)) {
        result->reason = RXAS_FLOW_PROOF_RESULT_UNAVAILABLE;
        return;
    }
    if (!rxas_flow_component_at_instruction(
                service->ssa, service->metrics.epoch, generator_id, 0,
                generator_source, source_component, &generator_source_value)) {
        result->reason = RXAS_FLOW_PROOF_GENERATOR_SOURCE_UNKNOWN;
        return;
    }
    if (!rxas_flow_component_at_instruction(
                service->ssa, service->metrics.epoch, candidate_id, 0,
                candidate_source, source_component, &candidate_source_value)) {
        result->reason = RXAS_FLOW_PROOF_CANDIDATE_SOURCE_UNKNOWN;
        return;
    }
    result->storage_id = generator_value.storage_id;
    result->source_value_id = generator_source_value.value_id;
    result->result_value_id = generator_value.value_id;
    result->candidate_source_value_id = candidate_source_value.value_id;
    result->candidate_result_value_id = candidate_value.value_id;
    result->source_kind = generator_source_value.kind;
    result->result_kind = generator_value.kind;
    result->candidate_source_kind = candidate_source_value.kind;
    result->candidate_result_kind = candidate_value.kind;
    if (!generator_value.storage_id || !candidate_value.storage_id) {
        result->reason = RXAS_FLOW_PROOF_STORAGE_UNKNOWN;
        return;
    }
    generator_storage_root = generator_value.storage_id;
    candidate_storage_root = candidate_value.storage_id;
    if (generator_storage_root != candidate_storage_root) {
        storage_walk = flow_proof_unique_storage_leaf(
                service, generator_storage_root,
                &generator_storage_root, &generator_storage_leaf);
        if (storage_walk != FLOW_PROOF_STORAGE_UNIQUE ||
            generator_storage_leaf.kind != RXAS_FLOW_STORAGE_BASE) {
            result->reason = RXAS_FLOW_PROOF_STORAGE_CHANGED;
            return;
        }
        storage_walk = flow_proof_unique_storage_leaf(
                service, candidate_storage_root,
                &candidate_storage_root, &candidate_storage_leaf);
        if (storage_walk != FLOW_PROOF_STORAGE_UNIQUE ||
            candidate_storage_leaf.kind != RXAS_FLOW_STORAGE_BASE ||
            generator_storage_root != candidate_storage_root) {
            result->reason = RXAS_FLOW_PROOF_STORAGE_CHANGED;
            return;
        }
    }
    value_walk = flow_proof_equivalent_value(
            service, generator_source_value.value_id,
            candidate_source_value.value_id,
            &generator_source_root, &generator_source_leaf);
    if (value_walk != FLOW_PROOF_VALUE_UNIQUE) {
        result->reason = value_walk == FLOW_PROOF_VALUE_MULTIPLE
                ? RXAS_FLOW_PROOF_SOURCE_CHANGED
                : RXAS_FLOW_PROOF_GENERATOR_SOURCE_UNKNOWN;
        return;
    }
    candidate_source_root = generator_source_root;
    candidate_source_leaf = generator_source_leaf;
    generator_result_root = generator_value.value_id;
    if (!rxas_flow_value_node(
                service->ssa, service->metrics.epoch,
                generator_result_root, &generator_result_leaf) ||
        generator_result_leaf.kind != RXAS_FLOW_VALUE_DERIVED ||
        generator_result_leaf.derivation != derivation ||
        generator_result_leaf.defining_instruction != generator_id ||
        generator_result_leaf.source_value_id == RXAS_FLOW_ID_NONE) {
        result->reason = RXAS_FLOW_PROOF_GENERATOR_RESULT_UNKNOWN;
        return;
    }
    value_walk = flow_proof_equivalent_value(
            service, generator_result_leaf.source_value_id,
            generator_source_root, &result_source_root,
            &result_source_leaf);
    if (value_walk != FLOW_PROOF_VALUE_UNIQUE) {
        result->reason = RXAS_FLOW_PROOF_SOURCE_UNKNOWN;
        return;
    }
    /* Check the operation's observable dependencies before comparing the
     * available result.  A result produced under a different numeric/plugin/
     * locale/external context is not the same derivation, even when its source
     * ValueId is unchanged. */
    for (effect = 0; effect < RXAS_FLOW_EFFECT_CLASS_COUNT; effect++) {
        unsigned int dependency;
        dependency = flow_proof_dependency_for_effect(effect);
        if (!(generator_result_leaf.signal_dependencies & dependency))
            continue;
        result->effect_class = effect;
        result->generator_effect_id =
                generator_result_leaf.definition_effects[effect];
        result->candidate_effect_id = candidate_value.current_effects[effect];
        effect_walk = flow_proof_equivalent_effect(
                service, generator_result_leaf.definition_effects[effect],
                candidate_value.current_effects[effect]);
        if (effect_walk != FLOW_PROOF_EFFECT_UNIQUE) {
            result->reason = RXAS_FLOW_PROOF_EFFECT_CHANGED;
            return;
        }
    }
    value_walk = flow_proof_equivalent_derivation_result(
            service, candidate_value.value_id,
            &generator_result_leaf, generator_source_root);
    if (value_walk != FLOW_PROOF_VALUE_UNIQUE) {
        result->reason = value_walk == FLOW_PROOF_VALUE_MULTIPLE
                ? RXAS_FLOW_PROOF_RESULT_UNAVAILABLE
                : RXAS_FLOW_PROOF_GENERATOR_RESULT_UNKNOWN;
        return;
    }
    candidate_result_root = generator_result_root;
    candidate_result_leaf = generator_result_leaf;
    result->source_value_id = generator_source_root;
    result->candidate_source_value_id = candidate_source_root;
    result->result_value_id = generator_result_root;
    result->candidate_result_value_id = candidate_result_root;
    result->source_kind = generator_source_leaf.kind;
    result->candidate_source_kind = candidate_source_leaf.kind;
    result->result_kind = generator_result_leaf.kind;
    result->candidate_result_kind = candidate_result_leaf.kind;
    if (generator_value.current_reference_effect !=
        candidate_value.current_reference_effect) {
        generator_aliases = 0;
        candidate_aliases = 0;
        generator_external = 1;
        candidate_external = 1;
        effect_walk = flow_proof_equivalent_effect(
                service,
                generator_value.current_effects[RXAS_FLOW_EFFECT_ALIAS],
                candidate_value.current_effects[RXAS_FLOW_EFFECT_ALIAS]);
        local_storage = rxas_flow_storage_is_local_base(
                service->ssa, service->metrics.epoch,
                generator_storage_root);
        generator_alias_query = rxas_flow_storage_aliases_at_instruction(
                service->ssa, service->metrics.epoch,
                generator_id, 1, generator_value.storage_id,
                &generator_aliases, &generator_external);
        candidate_alias_query = rxas_flow_storage_aliases_at_instruction(
                service->ssa, service->metrics.epoch,
                candidate_id, 0, candidate_value.storage_id,
                &candidate_aliases, &candidate_external);
        if (effect_walk != FLOW_PROOF_EFFECT_UNIQUE || !local_storage ||
            !generator_alias_query || !candidate_alias_query ||
            generator_aliases != 1 || candidate_aliases != 1 ||
            generator_external || candidate_external) {
            result->reason = RXAS_FLOW_PROOF_REFERENCE_EFFECT_CHANGED;
            return;
        }
    }
    result->proved = 1;
    result->reason = RXAS_FLOW_PROOF_PROVED;
    result->storage_id = generator_storage_root;
    result->source_value_id = generator_source_root;
    result->result_value_id = generator_result_root;
}

static RxasFlowProofService *flow_proof_build(
        RxasFlowProcedure *procedure, unsigned long epoch, size_t budget) {
    RxasFlowProofService *service;
    const RxasFlowMetrics *metrics;
    service = calloc(1, sizeof(*service));
    if (!service) return 0;
    service->procedure = procedure;
    service->metrics.status = RXAS_FLOW_ANALYSIS_AVAILABLE;
    service->metrics.epoch = epoch;
    service->metrics.budget_limit = budget;
    metrics = rxas_flow_procedure_metrics(procedure, epoch);
    if (!metrics || !metrics->complete_control_flow) {
        service->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
        return service;
    }
    service->block_count = metrics->blocks;
    service->edge_count = metrics->edges;
    flow_proof_set_retained_bytes(service);
    return service;
}

void rxas_flow_proof_service_destroy(struct RxasFlowProofService *service) {
    if (!service) return;
    free(service->outgoing_offsets);
    free(service->outgoing_edges);
    free(service->visit_marks);
    free(service->visit_queue);
    free(service->value_marks);
    free(service->value_stack);
    free(service->value_aux);
    free(service->value_set_marks);
    free(service->storage_marks);
    free(service->storage_stack);
    free(service->effect_marks);
    free(service->effect_stack);
    free(service->effect_set_marks);
    free(service->cache);
    free(service->rewrites);
    free(service->trace_deletions);
    free(service);
}

const RxasFlowProofService *rxas_flow_require_proof_capabilities(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        unsigned int capabilities, size_t work_budget) {
    struct RxasFlowAnalysisManager *manager;
    const RxasFlowMetrics *metrics;
    size_t requested;
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch) ||
        !capabilities ||
        (capabilities & ~FLOW_PROOF_ALLOWED_CAPABILITIES))
        return 0;
    metrics = rxas_flow_procedure_metrics(procedure, expected_epoch);
    if (!metrics) return 0;
    requested = work_budget ? work_budget : flow_proof_default_budget(metrics);
    manager = procedure->analysis_manager;
    if (manager && manager->epoch == expected_epoch && manager->proof) {
        if (manager->proof->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE) {
            if (requested > manager->proof_budget) {
                manager->proof_budget = requested;
                manager->proof->metrics.budget_limit = requested;
            }
            return flow_proof_acquire_capabilities(
                    manager->proof, capabilities) ? manager->proof : 0;
        }
        if (requested <= manager->proof_budget) return 0;
        rxas_flow_proof_service_destroy(manager->proof);
        manager->proof = 0;
    }
    if (!manager) {
        manager = calloc(1, sizeof(*manager));
        if (!manager) return 0;
        procedure->analysis_manager = manager;
    }
    manager->epoch = expected_epoch;
    manager->proof_budget = requested;
    manager->proof = flow_proof_build(procedure, expected_epoch, requested);
    if (!manager->proof ||
        manager->proof->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE ||
        !flow_proof_acquire_capabilities(manager->proof, capabilities))
        return 0;
    return manager->proof;
}

const RxasFlowProofService *rxas_flow_require_proof_service(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget) {
    return rxas_flow_require_proof_capabilities(
            procedure, expected_epoch, FLOW_PROOF_ALLOWED_CAPABILITIES,
            work_budget);
}

unsigned int rxas_flow_proof_capabilities(
        const RxasFlowProofService *service, unsigned long expected_epoch) {
    if (!flow_proof_valid(service, expected_epoch)) return 0;
    return service->metrics.acquired_capabilities;
}

const RxasFlowProofMetrics *rxas_flow_last_proof_metrics(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch) {
    const struct RxasFlowAnalysisManager *manager;
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch) ||
        !procedure->analysis_manager)
        return 0;
    manager = procedure->analysis_manager;
    if (manager->epoch != expected_epoch || !manager->proof) return 0;
    return &manager->proof->metrics;
}

const RxasFlowProofMetrics *rxas_flow_proof_metrics(
        const RxasFlowProofService *service, unsigned long expected_epoch) {
    if (!flow_proof_valid(service, expected_epoch)) return 0;
    return &service->metrics;
}

int rxas_flow_prove_repetition(
        const RxasFlowProofService *const_service,
        unsigned long expected_epoch, size_t generator_instruction,
        size_t candidate_instruction, RxasFlowProofResult *result) {
    RxasFlowProofService *service;
    size_t index;
    if (!result) return 0;
    flow_proof_result_init(result, RXAS_FLOW_PROOF_STALE_EPOCH);
    if (!flow_proof_query_available(
                const_service, expected_epoch, result)) return 1;
    service = (RxasFlowProofService *)const_service;
    result->generator_instruction = generator_instruction;
    result->candidate_instruction = candidate_instruction;
    service->metrics.repetition_queries++;
    for (index = 0; index < service->cache_count; index++) {
        if (service->cache[index].generator == generator_instruction &&
            service->cache[index].candidate == candidate_instruction) {
            *result = service->cache[index].result;
            service->metrics.repetition_cache_hits++;
            return 1;
        }
    }
    /* Reserve both the query and cache-entry work before doing a proof. */
    if (!flow_proof_consume(service, 2)) {
        result->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        return 1;
    }
    flow_proof_repetition_uncached(
            service, generator_instruction, candidate_instruction, result);
    if (!flow_proof_append_cache(service, result)) {
        result->proved = 0;
        result->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    if (result->proved) service->metrics.repetition_proved++;
    else service->metrics.repetition_rejected++;
    return 1;
}

int rxas_flow_repetition_key(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t instruction_id, RxasFlowRepetitionKey *key) {
    const RxasFlowInstruction *instruction;
    const RxasFlowRecord *record;
    const instruction_queue *item;
    RxasFlowRegister target;
    RxasFlowStorageFact storage;
    RxasFlowStorageNode storage_leaf;
    unsigned int component;
    size_t storage_root;
    if (!key) return 0;
    memset(key, 0, sizeof(*key));
    key->opcode = -1;
    if (!flow_proof_valid(service, expected_epoch) ||
        !flow_proof_has_capabilities(
                service, FLOW_PROOF_BASE_CAPABILITIES))
        return 0;
    instruction = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, instruction_id);
    record = instruction ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, instruction->record_id) : 0;
    item = record ? record->queue_record : 0;
    if (!instruction || !instruction->op || !item ||
        rxop_value_derivation(instruction->op->opcode) ==
                RXOP_DERIVATION_NONE ||
        !flow_proof_register(flow_proof_operand(item, 0), &target))
        return 0;
    component = rxop_component_writes(instruction->op->opcode, 0);
    if (!component || (component & (component - 1)) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, instruction_id, 0,
                target, &storage) || !storage.storage_id)
        return 0;
    key->opcode = instruction->op->opcode;
    key->derivation = rxop_value_derivation(instruction->op->opcode);
    key->storage_id = storage.storage_id;
    storage_root = storage.storage_id;
    if (flow_proof_unique_storage_leaf(
                (RxasFlowProofService *)service, storage.storage_id,
                &storage_root, &storage_leaf) == FLOW_PROOF_STORAGE_UNIQUE &&
        storage_leaf.kind == RXAS_FLOW_STORAGE_BASE)
        key->storage_id = storage_root;
    return 1;
}

int rxas_flow_prove_redundant_constant_write(
        const RxasFlowProofService *const_service,
        unsigned long expected_epoch, size_t candidate_instruction,
        RxasFlowProofResult *result) {
    RxasFlowProofService *service;
    const RxasFlowInstruction *instruction;
    const RxasFlowRecord *record;
    const instruction_queue *item;
    const Assembler_Token *constant;
    RxasFlowRegister target;
    RxasFlowComponentFact before;
    RxasFlowComponentFact after;
    RxasFlowComponentFact cleanup;
    unsigned int component;
    unsigned int clear_components;
    unsigned int clear_component;
    FlowProofValueWalkResult walk;
    if (!result) return 0;
    flow_proof_result_init(result, RXAS_FLOW_PROOF_STALE_EPOCH);
    result->candidate_instruction = candidate_instruction;
    if (!flow_proof_query_available(
                const_service, expected_epoch, result)) return 1;
    service = (RxasFlowProofService *)const_service;
    service->metrics.redundant_constant_queries++;
    if (!flow_proof_consume(service, 2)) {
        result->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        goto complete;
    }
    instruction = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, candidate_instruction);
    record = instruction ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, instruction->record_id) : 0;
    item = record ? record->queue_record : 0;
    component = instruction && instruction->op
            ? rxop_component_writes(instruction->op->opcode, 0)
            : RXOP_COMPONENT_NONE;
    clear_components = instruction && instruction->op
            ? rxop_component_clears(instruction->op->opcode, 0)
            : RXOP_COMPONENT_NONE;
    constant = item ? flow_proof_operand(item, 1) : 0;
    if (!instruction || !instruction->op || !item ||
        item->operandCount != 2 ||
        instruction->effects.state != RXOP_EFFECT_CLASSIFIED ||
        instruction->effects.reads != RXOP_OP_NONE ||
        instruction->effects.writes != RXOP_OP_1 ||
        instruction->effects.kills != RXOP_OP_1 ||
        instruction->effects.branch_targets != RXOP_OP_NONE ||
        instruction->effects.implicit != RXOP_IMPLICIT_NONE ||
        instruction->effects.semantics != RXOP_SEM_NONE ||
        instruction->effects.cursor_reads != RXOP_OP_NONE ||
        instruction->effects.cursor_writes != RXOP_OP_NONE ||
        instruction->effects.flow != FLOW_NEXT ||
        instruction->effects.optimizer_barrier ||
        instruction->signal.state != RXOP_SIGNAL_STATE_NONE ||
        rxop_context_writes(instruction->op->opcode) != RXOP_CONTEXT_NONE ||
        (component != RXOP_COMPONENT_INTEGER &&
         component != RXOP_COMPONENT_FLOAT) ||
        clear_components != (RXOP_COMPONENT_REFERENCE |
                             RXOP_COMPONENT_NATIVE_PAYLOAD) ||
        !constant ||
        (component == RXOP_COMPONENT_INTEGER &&
         constant->token_type != INT) ||
        (component == RXOP_COMPONENT_FLOAT &&
         constant->token_type != FLOAT) ||
        !flow_proof_register(flow_proof_operand(item, 0), &target)) {
        result->reason = RXAS_FLOW_PROOF_NOT_EXACT_CONSTANT_WRITE;
        goto complete;
    }
    if (!rxas_flow_component_at_instruction(
                service->ssa, expected_epoch, candidate_instruction, 0,
                target, component, &before) ||
        !rxas_flow_component_at_instruction(
                service->ssa, expected_epoch, candidate_instruction, 1,
                target, component, &after)) {
        result->reason = RXAS_FLOW_PROOF_CONSTANT_UNKNOWN;
        goto complete;
    }
    result->storage_id = before.storage_id;
    result->source_value_id = before.value_id;
    result->result_value_id = after.value_id;
    result->source_kind = before.kind;
    result->result_kind = after.kind;
    if (!before.storage_id || before.storage_id != after.storage_id) {
        result->reason = before.storage_id
                ? RXAS_FLOW_PROOF_STORAGE_CHANGED
                : RXAS_FLOW_PROOF_STORAGE_UNKNOWN;
        goto complete;
    }
    if (after.kind != RXAS_FLOW_VALUE_CONSTANT ||
        after.presence != RXAS_FLOW_COMPONENT_PRESENT ||
        after.defining_instruction != candidate_instruction ||
        !flow_proof_same_scalar_constant(after.constant_token, constant)) {
        result->reason = RXAS_FLOW_PROOF_CONSTANT_UNKNOWN;
        goto complete;
    }
    for (clear_component = RXOP_COMPONENT_REFERENCE;
         clear_component <= RXOP_COMPONENT_NATIVE_PAYLOAD;
         clear_component <<= 1) {
        if (!(clear_components & clear_component)) continue;
        if (!rxas_flow_component_at_instruction(
                    service->ssa, expected_epoch, candidate_instruction, 0,
                    target, clear_component, &cleanup)) {
            result->reason = RXAS_FLOW_PROOF_CLEANUP_REQUIRED;
            goto complete;
        }
        walk = flow_proof_value_leaves_match(
                service, cleanup.value_id, 0, 1);
        if (walk != FLOW_PROOF_VALUE_UNIQUE) {
            result->reason = service->metrics.status ==
                        RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                    ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                    : RXAS_FLOW_PROOF_CLEANUP_REQUIRED;
            goto complete;
        }
    }
    walk = flow_proof_value_leaves_match(
            service, before.value_id, constant, 0);
    if (walk != FLOW_PROOF_VALUE_UNIQUE) {
        result->reason = service->metrics.status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                : walk == FLOW_PROOF_VALUE_MULTIPLE
                        ? RXAS_FLOW_PROOF_CONSTANT_CHANGED
                        : RXAS_FLOW_PROOF_CONSTANT_UNKNOWN;
        goto complete;
    }
    result->proved = 1;
    result->reason = RXAS_FLOW_PROOF_PROVED;

complete:
    if (result->proved) service->metrics.redundant_constant_proved++;
    else service->metrics.redundant_constant_rejected++;
    return 1;
}

int rxas_flow_prove_redundant_absent_write(
        const RxasFlowProofService *const_service,
        unsigned long expected_epoch, size_t candidate_instruction,
        RxasFlowProofResult *result) {
    RxasFlowProofService *service;
    const RxasFlowInstruction *instruction;
    const RxasFlowRecord *record;
    const instruction_queue *item;
    RxasFlowRegister target;
    RxasFlowStorageFact storage;
    RxasFlowComponentFact before;
    FlowProofValueWalkResult walk;
    unsigned int component;
    size_t component_index;
    static const unsigned int components[] = {
        RXOP_COMPONENT_INTEGER,
        RXOP_COMPONENT_FLOAT,
        RXOP_COMPONENT_STRING,
        RXOP_COMPONENT_DECIMAL,
        RXOP_COMPONENT_BINARY,
        RXOP_COMPONENT_ATTRIBUTES,
        RXOP_COMPONENT_REFERENCE,
        RXOP_COMPONENT_NATIVE_PAYLOAD,
        RXOP_COMPONENT_ATTRIBUTE_COUNT
    };
    if (!result) return 0;
    flow_proof_result_init(result, RXAS_FLOW_PROOF_STALE_EPOCH);
    result->candidate_instruction = candidate_instruction;
    if (!flow_proof_query_available(
                const_service, expected_epoch, result)) return 1;
    service = (RxasFlowProofService *)const_service;
    service->metrics.redundant_absent_queries++;
    if (!flow_proof_consume(service, 2 + sizeof(components) /
                                     sizeof(components[0]))) {
        result->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        goto complete;
    }
    instruction = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, candidate_instruction);
    record = instruction ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, instruction->record_id) : 0;
    item = record ? record->queue_record : 0;
    component = instruction && instruction->op
            ? rxop_component_writes(instruction->op->opcode, 0)
            : RXOP_COMPONENT_NONE;
    if (!instruction || !instruction->op || !item ||
        item->operandCount != 1 ||
        instruction->op->opcode != OP_NULL_REG ||
        instruction->effects.state != RXOP_EFFECT_CLASSIFIED ||
        instruction->effects.reads != RXOP_OP_NONE ||
        instruction->effects.writes != RXOP_OP_1 ||
        instruction->effects.kills != RXOP_OP_1 ||
        instruction->effects.branch_targets != RXOP_OP_NONE ||
        instruction->effects.implicit != RXOP_IMPLICIT_NONE ||
        instruction->effects.semantics != RXOP_SEM_NONE ||
        instruction->effects.cursor_reads != RXOP_OP_NONE ||
        instruction->effects.cursor_writes != RXOP_OP_NONE ||
        instruction->effects.flow != FLOW_NEXT ||
        instruction->effects.optimizer_barrier ||
        instruction->signal.state != RXOP_SIGNAL_STATE_NONE ||
        rxop_context_writes(instruction->op->opcode) != RXOP_CONTEXT_NONE ||
        component != RXOP_COMPONENT_ALL ||
        !flow_proof_register(flow_proof_operand(item, 0), &target)) {
        result->reason = RXAS_FLOW_PROOF_NOT_EXACT_ABSENT_WRITE;
        goto complete;
    }
    if (!rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, candidate_instruction, 0,
                target, &storage) || !storage.storage_id) {
        result->reason = RXAS_FLOW_PROOF_STORAGE_UNKNOWN;
        goto complete;
    }
    result->storage_id = storage.storage_id;
    for (component_index = 0;
         component_index < sizeof(components) / sizeof(components[0]);
         component_index++) {
        if (!rxas_flow_component_at_instruction(
                    service->ssa, expected_epoch, candidate_instruction, 0,
                    target, components[component_index], &before)) {
            result->reason = RXAS_FLOW_PROOF_ABSENCE_UNKNOWN;
            goto complete;
        }
        if (!component_index) {
            result->source_value_id = before.value_id;
            result->source_kind = before.kind;
        }
        walk = flow_proof_value_leaves_match(
                service, before.value_id, 0, 1);
        if (walk != FLOW_PROOF_VALUE_UNIQUE) {
            result->reason = service->metrics.status ==
                        RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                    ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                    : walk == FLOW_PROOF_VALUE_MULTIPLE
                            ? RXAS_FLOW_PROOF_COMPONENT_PRESENT
                            : RXAS_FLOW_PROOF_ABSENCE_UNKNOWN;
            goto complete;
        }
    }
    result->proved = 1;
    result->reason = RXAS_FLOW_PROOF_PROVED;

complete:
    if (result->proved) service->metrics.redundant_absent_proved++;
    else service->metrics.redundant_absent_rejected++;
    return 1;
}

int rxas_flow_prove_redundant_self_copy(
        const RxasFlowProofService *const_service,
        unsigned long expected_epoch, size_t candidate_instruction,
        RxasFlowProofResult *result) {
    RxasFlowProofService *service;
    const RxasFlowInstruction *instruction;
    const RxasFlowRecord *record;
    const instruction_queue *item;
    RxasFlowRegister destination;
    RxasFlowRegister source;
    RxasFlowStorageFact destination_storage;
    RxasFlowStorageFact source_storage;
    RxasFlowStorageNode destination_leaf;
    RxasFlowStorageNode source_leaf;
    size_t destination_root;
    size_t source_root;
    FlowProofStorageWalkResult walk;
    if (!result) return 0;
    flow_proof_result_init(result, RXAS_FLOW_PROOF_STALE_EPOCH);
    result->candidate_instruction = candidate_instruction;
    if (!flow_proof_query_available(
                const_service, expected_epoch, result)) return 1;
    service = (RxasFlowProofService *)const_service;
    service->metrics.redundant_self_copy_queries++;
    if (!flow_proof_consume(service, 2)) {
        result->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        goto complete;
    }
    instruction = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, candidate_instruction);
    record = instruction ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, instruction->record_id) : 0;
    item = record ? record->queue_record : 0;
    if (!instruction || !instruction->op || !item ||
        item->operandCount != 2 ||
        !rxop_same_storage_copy_is_noop(instruction->op->opcode) ||
        !flow_proof_register(flow_proof_operand(item, 0), &destination) ||
        !flow_proof_register(flow_proof_operand(item, 1), &source)) {
        result->reason = RXAS_FLOW_PROOF_NOT_EXACT_SELF_COPY;
        goto complete;
    }

    /* Identical register operands are intrinsically the same address.  This
     * preserves the old optimization floor even if an entry mapping is
     * deliberately unknown to component SSA. */
    if (destination.register_class == source.register_class &&
        destination.number == source.number) {
        result->proved = 1;
        result->reason = RXAS_FLOW_PROOF_PROVED;
        goto complete;
    }
    if (!rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, candidate_instruction, 0,
                destination, &destination_storage) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, candidate_instruction, 0,
                source, &source_storage) ||
        !destination_storage.storage_id || !source_storage.storage_id) {
        result->reason = RXAS_FLOW_PROOF_STORAGE_UNKNOWN;
        goto complete;
    }
    result->storage_id = destination_storage.storage_id;
    destination_root = destination_storage.storage_id;
    source_root = source_storage.storage_id;
    if (destination_root == source_root) {
        result->proved = 1;
        result->reason = RXAS_FLOW_PROOF_PROVED;
        goto complete;
    }
    walk = flow_proof_unique_storage_leaf(
            service, destination_root, &destination_root, &destination_leaf);
    if (walk != FLOW_PROOF_STORAGE_UNIQUE) {
        result->reason = service->metrics.status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                : RXAS_FLOW_PROOF_STORAGE_NOT_IDENTICAL;
        goto complete;
    }
    walk = flow_proof_unique_storage_leaf(
            service, source_root, &source_root, &source_leaf);
    if (walk != FLOW_PROOF_STORAGE_UNIQUE) {
        result->reason = service->metrics.status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                : RXAS_FLOW_PROOF_STORAGE_NOT_IDENTICAL;
        goto complete;
    }
    if (destination_root != source_root) {
        result->reason = RXAS_FLOW_PROOF_STORAGE_NOT_IDENTICAL;
        goto complete;
    }
    result->storage_id = destination_root;
    result->proved = 1;
    result->reason = RXAS_FLOW_PROOF_PROVED;

complete:
    if (result->proved) service->metrics.redundant_self_copy_proved++;
    else service->metrics.redundant_self_copy_rejected++;
    return 1;
}

static void flow_proof_typed_copy_plan_init(
        RxasFlowTypedCopyPlan *plan, size_t candidate_instruction) {
    memset(plan, 0, sizeof(*plan));
    plan->reason = RXAS_FLOW_PROOF_STALE_EPOCH;
    plan->candidate_instruction = candidate_instruction;
    plan->destination_storage_id = 0;
    plan->source_value_id = RXAS_FLOW_ID_NONE;
    plan->result_value_id = RXAS_FLOW_ID_NONE;
    plan->rewrite_offset = RXAS_FLOW_ID_NONE;
}

int rxas_flow_prove_typed_copy_redirect(
        const RxasFlowProofService *const_service,
        unsigned long expected_epoch, size_t candidate_instruction,
        RxasFlowTypedCopyPlan *plan) {
    RxasFlowProofService *service;
    const RxasFlowInstruction *instruction;
    const RxasFlowRecord *record;
    const instruction_queue *item;
    const RxasFlowUseMetrics *use_metrics;
    RxasFlowRegister destination;
    RxasFlowRegister source;
    RxasFlowStorageFact destination_before;
    RxasFlowStorageFact destination_after;
    RxasFlowComponentFact source_before;
    RxasFlowComponentFact result_after;
    RxasFlowValueNode start;
    RxasFlowValueNode equivalent;
    size_t equivalent_id;
    size_t alias_count;
    int externally_visible;
    unsigned int component;
    size_t rewrite_start;
    size_t head;
    size_t tail;
    size_t storage_use_index;
    FlowProofValueWalkResult walk;
    if (!plan) return 0;
    flow_proof_typed_copy_plan_init(plan, candidate_instruction);
    if (!const_service || !expected_epoch ||
        const_service->metrics.epoch != expected_epoch ||
        !rxas_flow_procedure_epoch_matches(
                const_service->procedure, expected_epoch))
        return 1;
    if (const_service->metrics.status ==
            RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED) {
        plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        return 1;
    }
    if (const_service->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    if (!flow_proof_has_capabilities(
                const_service, FLOW_PROOF_USE_CAPABILITIES)) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    service = (RxasFlowProofService *)const_service;
    service->metrics.typed_copy_redirect_queries++;
    rewrite_start = service->rewrite_count;
    if (!flow_proof_consume(service, 4)) {
        plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        goto complete;
    }
    instruction = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, candidate_instruction);
    record = instruction ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, instruction->record_id) : 0;
    item = record ? record->queue_record : 0;
    component = RXOP_COMPONENT_NONE;
    if (instruction && instruction->op) {
        if (instruction->op->opcode == OP_ICOPY_REG_REG)
            component = RXOP_COMPONENT_INTEGER;
        else if (instruction->op->opcode == OP_FCOPY_REG_REG)
            component = RXOP_COMPONENT_FLOAT;
        else if (instruction->op->opcode == OP_SCOPY_REG_REG)
            component = RXOP_COMPONENT_STRING;
    }
    plan->component = component;
    if (!instruction || !instruction->op || !item ||
        item->operandCount != 2 || !component ||
        instruction->effects.state != RXOP_EFFECT_CLASSIFIED ||
        instruction->effects.reads != RXOP_OP_2 ||
        instruction->effects.writes != RXOP_OP_1 ||
        instruction->effects.kills != RXOP_OP_1 ||
        instruction->effects.branch_targets != RXOP_OP_NONE ||
        instruction->effects.implicit != RXOP_IMPLICIT_NONE ||
        instruction->effects.semantics != RXOP_SEM_NONE ||
        instruction->effects.cursor_reads != RXOP_OP_NONE ||
        instruction->effects.flow != FLOW_NEXT ||
        instruction->effects.optimizer_barrier ||
        instruction->signal.state != RXOP_SIGNAL_STATE_NONE ||
        rxop_context_writes(instruction->op->opcode) != RXOP_CONTEXT_NONE ||
        rxop_component_reads(instruction->op->opcode, 1) != component ||
        rxop_component_writes(instruction->op->opcode, 0) != component ||
        !flow_proof_register(
                flow_proof_operand(item, 0), &destination) ||
        destination.register_class != RXAS_FLOW_REGISTER_LOCAL ||
        !flow_proof_register(flow_proof_operand(item, 1), &source)) {
        plan->reason = RXAS_FLOW_PROOF_NOT_EXACT_TYPED_COPY;
        goto complete;
    }
    if (!rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, candidate_instruction, 0,
                destination, &destination_before) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, candidate_instruction, 1,
                destination, &destination_after) ||
        !destination_before.storage_id ||
        destination_before.storage_id != destination_after.storage_id) {
        plan->reason = RXAS_FLOW_PROOF_STORAGE_CHANGED;
        goto complete;
    }
    plan->destination_storage_id = destination_after.storage_id;
    if (!rxas_flow_storage_is_local_base(
                service->ssa, expected_epoch,
                destination_after.storage_id)) {
        plan->reason = RXAS_FLOW_PROOF_DESTINATION_NOT_LOCAL;
        goto complete;
    }
    alias_count = 0;
    externally_visible = 1;
    if (!rxas_flow_storage_aliases_at_instruction(
                service->ssa, expected_epoch, candidate_instruction, 1,
                destination_after.storage_id, &alias_count,
                &externally_visible) || alias_count != 1 ||
        externally_visible) {
        plan->reason = RXAS_FLOW_PROOF_DESTINATION_OBSERVABLE;
        goto complete;
    }
    if (!rxas_flow_component_at_instruction(
                service->ssa, expected_epoch, candidate_instruction, 0,
                source, component, &source_before) ||
        !rxas_flow_component_at_instruction(
                service->ssa, expected_epoch, candidate_instruction, 1,
                destination, component, &result_after) ||
        source_before.value_id == RXAS_FLOW_ID_NONE ||
        result_after.value_id == RXAS_FLOW_ID_NONE ||
        result_after.kind != RXAS_FLOW_VALUE_COPY ||
        result_after.defining_instruction != candidate_instruction) {
        plan->reason = RXAS_FLOW_PROOF_RESULT_UNAVAILABLE;
        goto complete;
    }
    plan->source_value_id = source_before.value_id;
    plan->result_value_id = result_after.value_id;
    walk = flow_proof_equivalent_value(
            service, source_before.value_id, result_after.source_value_id,
            &equivalent_id, &equivalent);
    if (walk != FLOW_PROOF_VALUE_UNIQUE) {
        plan->reason = service->metrics.status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                : RXAS_FLOW_PROOF_SOURCE_NOT_EQUIVALENT;
        goto complete;
    }

    service->use = rxas_flow_require_use_analysis(
            service->procedure, expected_epoch, 0);
    use_metrics = service->use ? rxas_flow_use_metrics(
            service->use, expected_epoch) : 0;
    if (!service->use || !use_metrics) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        goto complete;
    }
    if (use_metrics->opaque_observations) {
        plan->reason = RXAS_FLOW_PROOF_USE_NOT_REDIRECTABLE;
        goto complete;
    }
    if (use_metrics->unknown_values) {
        size_t all_use_count;
        size_t all_use_index;
        all_use_count = rxas_flow_use_count(service->use, expected_epoch);
        for (all_use_index = 0; all_use_index < all_use_count;
             all_use_index++) {
            const RxasFlowUse *unknown_use;
            unknown_use = rxas_flow_use(
                    service->use, expected_epoch, all_use_index);
            if (unknown_use &&
                !flow_proof_use_is_pure_write(unknown_use) &&
                unknown_use->kind != RXAS_FLOW_USE_CALL_WINDOW_READ &&
                unknown_use->value_id == RXAS_FLOW_ID_NONE &&
                unknown_use->component == component &&
                unknown_use->register_id.register_class ==
                        destination.register_class &&
                unknown_use->register_id.number == destination.number) {
                plan->reason = RXAS_FLOW_PROOF_USE_NOT_REDIRECTABLE;
                goto complete;
            }
        }
    }
    if (component == RXOP_COMPONENT_STRING) {
        size_t storage_uses;
        storage_uses = rxas_flow_storage_use_count(
                service->use, expected_epoch,
                destination_after.storage_id);
        for (storage_use_index = 0;
             storage_use_index < storage_uses; storage_use_index++) {
            const RxasFlowUse *use;
            use = rxas_flow_storage_use(
                    service->use, expected_epoch,
                    destination_after.storage_id, storage_use_index);
            if (use && use->kind == RXAS_FLOW_USE_CURSOR_READ) {
                plan->reason = RXAS_FLOW_PROOF_CURSOR_OBSERVED;
                goto complete;
            }
        }
    }
    if (!flow_proof_prepare_value_walk(service) ||
        !rxas_flow_value_node(
                service->ssa, expected_epoch,
                result_after.value_id, &start) ||
        start.id >= service->value_capacity) {
        plan->reason = RXAS_FLOW_PROOF_RESULT_UNAVAILABLE;
        goto complete;
    }
    service->value_generation++;
    if (!service->value_generation) {
        memset(service->value_marks, 0,
               service->value_capacity * sizeof(*service->value_marks));
        service->value_generation = 1;
    }
    head = 0;
    tail = 0;
    service->value_marks[start.id] = service->value_generation;
    service->value_stack[tail++] = start.id;
    while (head < tail) {
        size_t value_id;
        size_t use_count;
        size_t use_index;
        size_t dependent_count;
        size_t dependent_index;
        value_id = service->value_stack[head++];
        if (!flow_proof_consume(service, 1)) {
            plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
            goto complete;
        }
        use_count = rxas_flow_value_direct_use_count(
                service->use, expected_epoch, value_id);
        for (use_index = 0; use_index < use_count; use_index++) {
            const RxasFlowUse *use;
            RxasFlowComponentFact source_at_use;
            use = rxas_flow_value_direct_use(
                    service->use, expected_epoch, value_id, use_index);
            if (!use || use->kind != RXAS_FLOW_USE_EXPLICIT_READ ||
                use->component != component ||
                use->read_components != component ||
                use->instruction_id == RXAS_FLOW_ID_NONE ||
                use->operand_index == RXAS_FLOW_ID_NONE ||
                (use->register_id.register_class == source.register_class &&
                 use->register_id.number == source.number) ||
                !rxas_flow_component_at_instruction(
                        service->ssa, expected_epoch,
                        use->instruction_id, 0, source, component,
                        &source_at_use)) {
                plan->reason = RXAS_FLOW_PROOF_USE_NOT_REDIRECTABLE;
                goto complete;
            }
            walk = flow_proof_equivalent_value(
                    service, source_at_use.value_id, use->value_id,
                    &equivalent_id, &equivalent);
            if (walk != FLOW_PROOF_VALUE_UNIQUE) {
                plan->reason = service->metrics.status ==
                            RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                        ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                        : RXAS_FLOW_PROOF_SOURCE_NOT_EQUIVALENT;
                goto complete;
            }
            if (!flow_proof_append_rewrite(service, use, source)) {
                plan->reason = service->metrics.status ==
                            RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                        ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                        : RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
                goto complete;
            }
        }
        dependent_count = rxas_flow_value_dependent_count(
                service->use, expected_epoch, value_id);
        for (dependent_index = 0; dependent_index < dependent_count;
             dependent_index++) {
            size_t dependent;
            RxasFlowValueNode dependent_node;
            dependent = rxas_flow_value_dependent(
                    service->use, expected_epoch, value_id,
                    dependent_index);
            if (dependent == RXAS_FLOW_ID_NONE ||
                !rxas_flow_value_node(
                        service->ssa, expected_epoch,
                        dependent, &dependent_node) ||
                dependent_node.id >= service->value_capacity) {
                plan->reason = RXAS_FLOW_PROOF_RESULT_UNAVAILABLE;
                goto complete;
            }
            if (service->value_marks[dependent_node.id] ==
                    service->value_generation)
                continue;
            service->value_marks[dependent_node.id] =
                    service->value_generation;
            if (tail >= service->value_capacity) {
                plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
                goto complete;
            }
            service->value_stack[tail++] = dependent_node.id;
        }
    }
    if (use_metrics->call_window_reads) {
        size_t all_use_count;
        size_t all_use_index;
        all_use_count = rxas_flow_use_count(service->use, expected_epoch);
        for (all_use_index = 0; all_use_index < all_use_count;
             all_use_index++) {
            const RxasFlowUse *call_use;
            RxasFlowComponentFact at_call;
            RxasFlowValueNode at_call_node;
            call_use = rxas_flow_use(
                    service->use, expected_epoch, all_use_index);
            if (!call_use ||
                call_use->kind != RXAS_FLOW_USE_CALL_WINDOW_READ)
                continue;
            if (call_use->instruction_id == RXAS_FLOW_ID_NONE ||
                !rxas_flow_component_at_instruction(
                        service->ssa, expected_epoch,
                        call_use->instruction_id, 0, destination,
                        component, &at_call) ||
                !rxas_flow_value_node(
                        service->ssa, expected_epoch,
                        at_call.value_id, &at_call_node)) {
                plan->reason = RXAS_FLOW_PROOF_CALL_WINDOW_OBSERVED;
                goto complete;
            }
            if (at_call_node.id < service->value_capacity &&
                service->value_marks[at_call_node.id] ==
                        service->value_generation) {
                plan->reason = RXAS_FLOW_PROOF_CALL_WINDOW_OBSERVED;
                goto complete;
            }
        }
    }
    if (service->rewrite_count == rewrite_start) {
        plan->reason = RXAS_FLOW_PROOF_NO_REDIRECTS;
        goto complete;
    }
    plan->proved = 1;
    plan->reason = RXAS_FLOW_PROOF_PROVED;
    plan->rewrite_offset = rewrite_start;
    plan->rewrite_count = service->rewrite_count - rewrite_start;
    service->metrics.typed_copy_operand_rewrites += plan->rewrite_count;

complete:
    if (plan->proved) service->metrics.typed_copy_redirect_proved++;
    else {
        service->rewrite_count = rewrite_start;
        service->metrics.typed_copy_redirect_rejected++;
    }
    service->metrics.retained_bytes = sizeof(*service) +
            (service->block_count * 3 + service->edge_count + 1) *
                    sizeof(size_t) +
            service->value_capacity * 4 * sizeof(size_t) +
            service->storage_capacity * 2 * sizeof(size_t) +
            service->effect_capacity * 3 * sizeof(size_t) +
            service->cache_capacity * sizeof(*service->cache) +
            service->rewrite_capacity * sizeof(*service->rewrites) +
            service->trace_deletion_capacity *
                    sizeof(*service->trace_deletions);
    return 1;
}

int rxas_flow_typed_copy_plan_rewrite(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        const RxasFlowTypedCopyPlan *plan, size_t rewrite_index,
        RxasFlowOperandRewrite *rewrite) {
    size_t index;
    if (!rewrite || !plan || !plan->proved ||
        !flow_proof_valid(service, expected_epoch) ||
        rewrite_index >= plan->rewrite_count ||
        plan->rewrite_offset == RXAS_FLOW_ID_NONE ||
        plan->rewrite_offset > (size_t)-1 - rewrite_index)
        return 0;
    index = plan->rewrite_offset + rewrite_index;
    if (index >= service->rewrite_count) return 0;
    *rewrite = service->rewrites[index];
    return 1;
}

static void flow_proof_producer_plan_init(
        RxasFlowProducerDestinationPlan *plan,
        size_t producer_instruction, size_t copy_instruction) {
    memset(plan, 0, sizeof(*plan));
    plan->reason = RXAS_FLOW_PROOF_STALE_EPOCH;
    plan->producer_instruction = producer_instruction;
    plan->copy_instruction = copy_instruction;
    plan->producer_record_id = RXAS_FLOW_ID_NONE;
    plan->copy_record_id = RXAS_FLOW_ID_NONE;
    plan->temporary_result_value_id = RXAS_FLOW_ID_NONE;
    plan->copy_result_value_id = RXAS_FLOW_ID_NONE;
    plan->producer_rewrite.record_id = RXAS_FLOW_ID_NONE;
    plan->producer_rewrite.instruction_id = RXAS_FLOW_ID_NONE;
    plan->producer_rewrite.operand_index = RXAS_FLOW_ID_NONE;
}

static int flow_proof_same_register(RxasFlowRegister left,
                                    RxasFlowRegister right) {
    return left.register_class == right.register_class &&
           left.number == right.number;
}

static int flow_proof_address_observed_after_copy(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t copy_record_id) {
    const RxasFlowMetrics *metrics;
    size_t record_id;
    metrics = rxas_flow_procedure_metrics(service->procedure, expected_epoch);
    if (!metrics || copy_record_id >= metrics->records) return -1;
    for (record_id = copy_record_id + 1;
         record_id < metrics->records; record_id++) {
        const RxasFlowRecord *record;
        record = rxas_flow_procedure_record(
                service->procedure, expected_epoch, record_id);
        if (!record) return -1;
        if (record->instruction_id != RXAS_FLOW_ID_NONE) return 0;
        if (record->type == TRACE_EVENT || record->type == SRC_STEP) return 1;
    }
    return 0;
}

static int flow_proof_local_component_absent(
        RxasFlowProofService *service, unsigned long expected_epoch,
        const RxasFlowComponentFact *fact, size_t local_storage_id) {
    FlowProofValueWalkResult walk;
    if (!fact || !local_storage_id || fact->storage_id != local_storage_id)
        return 0;
    if (fact->kind == RXAS_FLOW_VALUE_ENTRY &&
        rxas_flow_storage_is_local_base(
                service->ssa, expected_epoch, local_storage_id))
        return 1;
    walk = flow_proof_value_leaves_match(service, fact->value_id, 0, 1);
    return walk == FLOW_PROOF_VALUE_UNIQUE;
}

static int flow_proof_mark_value_dependents(
        RxasFlowProofService *service, unsigned long expected_epoch,
        size_t root_value_id, size_t *marked_count) {
    RxasFlowValueNode root;
    size_t head;
    size_t tail;
    if (marked_count) *marked_count = 0;
    if (!marked_count || !flow_proof_prepare_value_walk(service) ||
        !rxas_flow_value_node(
                service->ssa, expected_epoch, root_value_id, &root) ||
        root.id >= service->value_capacity)
        return 0;
    service->value_generation++;
    if (!service->value_generation) {
        memset(service->value_marks, 0,
               service->value_capacity * sizeof(*service->value_marks));
        service->value_generation = 1;
    }
    head = 0;
    tail = 0;
    service->value_marks[root.id] = service->value_generation;
    service->value_stack[tail++] = root.id;
    while (head < tail) {
        size_t value_id;
        size_t dependent_count;
        size_t dependent_index;
        value_id = service->value_stack[head++];
        if (!flow_proof_consume(service, 1)) return 0;
        dependent_count = rxas_flow_value_dependent_count(
                service->use, expected_epoch, value_id);
        for (dependent_index = 0; dependent_index < dependent_count;
             dependent_index++) {
            RxasFlowValueNode dependent_node;
            size_t dependent;
            dependent = rxas_flow_value_dependent(
                    service->use, expected_epoch,
                    value_id, dependent_index);
            if (dependent == RXAS_FLOW_ID_NONE ||
                !rxas_flow_value_node(
                        service->ssa, expected_epoch,
                        dependent, &dependent_node) ||
                dependent_node.id >= service->value_capacity)
                return 0;
            if (service->value_marks[dependent_node.id] ==
                    service->value_generation)
                continue;
            if (tail >= service->value_capacity) return 0;
            service->value_marks[dependent_node.id] =
                    service->value_generation;
            service->value_stack[tail++] = dependent_node.id;
        }
    }
    *marked_count = tail;
    return 1;
}

static int flow_proof_value_dependent_is_marked(
        const RxasFlowProofService *service, size_t value_id) {
    return value_id != RXAS_FLOW_ID_NONE &&
           value_id < service->value_capacity &&
           service->value_marks[value_id] == service->value_generation;
}

typedef enum FlowProofDependencyResult {
    FLOW_PROOF_DEPENDENCY_UNAVAILABLE = 0,
    FLOW_PROOF_DEPENDENCY_ABSENT,
    FLOW_PROOF_DEPENDENCY_PRESENT
} FlowProofDependencyResult;

/* Walk from a value visible at a sparse observation point back through its
 * write-once provenance.  Unlike the forward dependent index, this remains
 * correct when a component query lazily materialises a new phi after use
 * analysis was built.  COPY and DERIVED nodes retain source provenance;
 * PHIs retain every incoming ValueId. */
static FlowProofDependencyResult flow_proof_value_depends_on(
        RxasFlowProofService *service, unsigned long expected_epoch,
        size_t candidate_value_id, size_t target_value_id) {
    RxasFlowValueNode candidate;
    RxasFlowValueNode target;
    size_t head;
    size_t tail;
    if (!rxas_flow_value_node(
                service->ssa, expected_epoch,
                candidate_value_id, &candidate) ||
        !rxas_flow_value_node(
                service->ssa, expected_epoch,
                target_value_id, &target) ||
        !flow_proof_prepare_value_walk(service) ||
        candidate.id >= service->value_capacity ||
        target.id >= service->value_capacity)
        return FLOW_PROOF_DEPENDENCY_UNAVAILABLE;
    service->value_generation++;
    if (!service->value_generation) {
        memset(service->value_marks, 0,
               service->value_capacity * sizeof(*service->value_marks));
        service->value_generation = 1;
    }
    head = 0;
    tail = 0;
    service->value_marks[candidate.id] = service->value_generation;
    service->value_stack[tail++] = candidate.id;
    while (head < tail) {
        RxasFlowValueNode node;
        size_t input;
        size_t next_id;
        RxasFlowValueNode next;
        candidate_value_id = service->value_stack[head++];
        if (!rxas_flow_value_node(
                    service->ssa, expected_epoch,
                    candidate_value_id, &node) ||
            !flow_proof_prepare_value_walk(service) ||
            node.id >= service->value_capacity ||
            !flow_proof_consume(service, 1))
            return FLOW_PROOF_DEPENDENCY_UNAVAILABLE;
        if (node.id == target.id)
            return FLOW_PROOF_DEPENDENCY_PRESENT;
        if (node.kind == RXAS_FLOW_VALUE_PHI) {
            if (!node.input_count)
                return FLOW_PROOF_DEPENDENCY_UNAVAILABLE;
            for (input = 0; input < node.input_count; input++) {
                next_id = rxas_flow_value_input(
                        service->ssa, expected_epoch, node.id, input);
                if (next_id == RXAS_FLOW_ID_NONE ||
                    !rxas_flow_value_node(
                            service->ssa, expected_epoch, next_id, &next) ||
                    !flow_proof_prepare_value_walk(service) ||
                    next.id >= service->value_capacity)
                    return FLOW_PROOF_DEPENDENCY_UNAVAILABLE;
                if (service->value_marks[next.id] ==
                        service->value_generation)
                    continue;
                if (tail >= service->value_capacity)
                    return FLOW_PROOF_DEPENDENCY_UNAVAILABLE;
                service->value_marks[next.id] = service->value_generation;
                service->value_stack[tail++] = next.id;
            }
            continue;
        }
        if (node.kind != RXAS_FLOW_VALUE_COPY &&
            node.kind != RXAS_FLOW_VALUE_DERIVED)
            continue;
        next_id = node.source_value_id;
        if (next_id == RXAS_FLOW_ID_NONE ||
            !rxas_flow_value_node(
                    service->ssa, expected_epoch, next_id, &next) ||
            !flow_proof_prepare_value_walk(service) ||
            next.id >= service->value_capacity)
            return FLOW_PROOF_DEPENDENCY_UNAVAILABLE;
        if (service->value_marks[next.id] == service->value_generation)
            continue;
        if (tail >= service->value_capacity)
            return FLOW_PROOF_DEPENDENCY_UNAVAILABLE;
        service->value_marks[next.id] = service->value_generation;
        service->value_stack[tail++] = next.id;
    }
    return FLOW_PROOF_DEPENDENCY_ABSENT;
}

int rxas_flow_prove_producer_destination_forward(
        const RxasFlowProofService *const_service,
        unsigned long expected_epoch, size_t producer_instruction,
        size_t copy_instruction, RxasFlowProducerDestinationPlan *plan) {
    RxasFlowProofService *service;
    const RxasFlowInstruction *producer;
    const RxasFlowInstruction *copy;
    const RxasFlowRecord *producer_record;
    const RxasFlowRecord *copy_record;
    const instruction_queue *producer_item;
    const instruction_queue *copy_item;
    const RxasFlowUseMetrics *use_metrics;
    RxasFlowRegister temporary;
    RxasFlowRegister copy_source;
    RxasFlowRegister destination;
    RxasFlowStorageFact temporary_before;
    RxasFlowStorageFact temporary_after;
    RxasFlowStorageFact source_before;
    RxasFlowStorageFact destination_at_producer;
    RxasFlowStorageFact destination_before;
    RxasFlowStorageFact destination_after;
    RxasFlowComponentFact temporary_result;
    RxasFlowComponentFact copy_source_value;
    RxasFlowComponentFact copy_result;
    RxasFlowValueNode equivalent;
    FlowProofValueWalkResult walk;
    size_t equivalent_id;
    size_t operand_index;
    size_t write_count;
    size_t alias_count;
    size_t use_count;
    size_t use_index;
    size_t all_use_count;
    size_t all_use_index;
    size_t copy_reads;
    size_t marked_count;
    size_t marked_index;
    size_t cleanup_index;
    int externally_visible;
    int address_observed;
    unsigned int component;
    unsigned int cleanup_components;
    static const unsigned int cleanup_bits[] = {
        RXOP_COMPONENT_INTEGER,
        RXOP_COMPONENT_FLOAT,
        RXOP_COMPONENT_STRING,
        RXOP_COMPONENT_DECIMAL,
        RXOP_COMPONENT_BINARY,
        RXOP_COMPONENT_ATTRIBUTES,
        RXOP_COMPONENT_REFERENCE,
        RXOP_COMPONENT_NATIVE_PAYLOAD,
        RXOP_COMPONENT_ATTRIBUTE_COUNT
    };
    if (!plan) return 0;
    flow_proof_producer_plan_init(
            plan, producer_instruction, copy_instruction);
    if (!const_service || !expected_epoch ||
        const_service->metrics.epoch != expected_epoch ||
        !rxas_flow_procedure_epoch_matches(
                const_service->procedure, expected_epoch))
        return 1;
    if (const_service->metrics.status ==
            RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED) {
        plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        return 1;
    }
    if (const_service->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    if (!flow_proof_has_capabilities(
                const_service, FLOW_PROOF_USE_CAPABILITIES)) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    service = (RxasFlowProofService *)const_service;
    service->metrics.producer_forward_queries++;
    if (!flow_proof_consume(service, 8)) {
        plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        goto complete;
    }
    producer = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, producer_instruction);
    copy = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, copy_instruction);
    producer_record = producer ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, producer->record_id) : 0;
    copy_record = copy ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, copy->record_id) : 0;
    producer_item = producer_record ? producer_record->queue_record : 0;
    copy_item = copy_record ? copy_record->queue_record : 0;
    if (!producer || !copy || !producer_record || !copy_record ||
        !producer_item || !copy_item) {
        plan->reason = RXAS_FLOW_PROOF_INVALID_INSTRUCTION;
        goto complete;
    }
    plan->producer_record_id = producer->record_id;
    plan->copy_record_id = copy->record_id;
    if (producer->record_id >= copy->record_id ||
        producer->record_id + 1 != copy->record_id) {
        plan->reason = RXAS_FLOW_PROOF_NOT_ADJACENT_COPY;
        goto complete;
    }
    component = copy->op && copy->op->opcode == OP_ICOPY_REG_REG
            ? RXOP_COMPONENT_INTEGER
            : copy->op && copy->op->opcode == OP_FCOPY_REG_REG
                    ? RXOP_COMPONENT_FLOAT : RXOP_COMPONENT_NONE;
    plan->component = component;
    if (!component || copy_item->operandCount != 2 ||
        copy->effects.state != RXOP_EFFECT_CLASSIFIED ||
        copy->effects.reads != RXOP_OP_2 ||
        copy->effects.writes != RXOP_OP_1 ||
        copy->effects.kills != RXOP_OP_1 ||
        copy->effects.branch_targets != RXOP_OP_NONE ||
        copy->effects.implicit != RXOP_IMPLICIT_NONE ||
        copy->effects.semantics != RXOP_SEM_NONE ||
        copy->effects.cursor_reads != RXOP_OP_NONE ||
        copy->effects.cursor_writes != RXOP_OP_NONE ||
        copy->effects.flow != FLOW_NEXT ||
        copy->effects.optimizer_barrier ||
        copy->signal.state != RXOP_SIGNAL_STATE_NONE ||
        rxop_context_writes(copy->op->opcode) != RXOP_CONTEXT_NONE ||
        rxop_component_reads(copy->op->opcode, 1) != component ||
        rxop_component_writes(copy->op->opcode, 0) != component ||
        !flow_proof_register(
                flow_proof_operand(copy_item, 0), &destination) ||
        !flow_proof_register(
                flow_proof_operand(copy_item, 1), &copy_source) ||
        destination.register_class != RXAS_FLOW_REGISTER_LOCAL ||
        copy_source.register_class != RXAS_FLOW_REGISTER_LOCAL) {
        plan->reason = RXAS_FLOW_PROOF_NOT_EXACT_PRODUCER_FORWARD;
        goto complete;
    }
    cleanup_components = producer->op
            ? rxop_component_clears(producer->op->opcode, 0)
            : RXOP_COMPONENT_NONE;
    plan->cleanup_components = cleanup_components;
    write_count = 0;
    if (!producer->op || !producer_item->operandCount ||
        producer->effects.state != RXOP_EFFECT_CLASSIFIED ||
        producer->effects.flow != FLOW_NEXT ||
        producer->effects.optimizer_barrier ||
        producer->effects.implicit != RXOP_IMPLICIT_NONE ||
        producer->effects.semantics != RXOP_SEM_NONE ||
        producer->effects.branch_targets != RXOP_OP_NONE ||
        producer->effects.cursor_reads != RXOP_OP_NONE ||
        producer->effects.cursor_writes != RXOP_OP_NONE ||
        producer->signal.state != RXOP_SIGNAL_STATE_NONE ||
        rxop_context_writes(producer->op->opcode) != RXOP_CONTEXT_NONE ||
        rxop_component_writes(producer->op->opcode, 0) != component ||
        rxop_effect_reads_operand(&producer->effects, 0) ||
        !rxop_effect_kills_operand(&producer->effects, 0) ||
        !flow_proof_register(
                flow_proof_operand(producer_item, 0), &temporary) ||
        temporary.register_class != RXAS_FLOW_REGISTER_LOCAL) {
        plan->reason = RXAS_FLOW_PROOF_NOT_EXACT_PRODUCER_FORWARD;
        goto complete;
    }
    for (operand_index = 0; operand_index < producer_item->operandCount;
         operand_index++) {
        if (!rxop_effect_writes_operand(
                    &producer->effects, operand_index))
            continue;
        write_count++;
        if (operand_index != 0)
            write_count = producer_item->operandCount + 1;
    }
    if (write_count != 1 || flow_proof_same_register(temporary, destination)) {
        plan->reason = RXAS_FLOW_PROOF_NOT_EXACT_PRODUCER_FORWARD;
        goto complete;
    }
    if (!rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, producer_instruction, 0,
                temporary, &temporary_before) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, producer_instruction, 1,
                temporary, &temporary_after) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, copy_instruction, 0,
                copy_source, &source_before) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, producer_instruction, 0,
                destination, &destination_at_producer) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, copy_instruction, 0,
                destination, &destination_before) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, copy_instruction, 1,
                destination, &destination_after) ||
        !temporary_before.storage_id ||
        temporary_before.storage_id != temporary_after.storage_id ||
        temporary_after.storage_id != source_before.storage_id ||
        !destination_at_producer.storage_id ||
        destination_at_producer.storage_id != destination_before.storage_id ||
        destination_before.storage_id != destination_after.storage_id ||
        temporary_after.storage_id == destination_before.storage_id) {
        plan->reason = RXAS_FLOW_PROOF_STORAGE_CHANGED;
        goto complete;
    }
    plan->temporary_storage_id = temporary_after.storage_id;
    plan->destination_storage_id = destination_after.storage_id;
    if (!rxas_flow_storage_is_local_base(
                service->ssa, expected_epoch,
                temporary_after.storage_id) ||
        !rxas_flow_storage_is_local_base(
                service->ssa, expected_epoch,
                destination_after.storage_id)) {
        plan->reason = RXAS_FLOW_PROOF_DESTINATION_NOT_LOCAL;
        goto complete;
    }
    alias_count = 0;
    externally_visible = 1;
    if (!rxas_flow_storage_aliases_at_instruction(
                service->ssa, expected_epoch, producer_instruction, 0,
                temporary_before.storage_id, &alias_count,
                &externally_visible) || alias_count != 1 ||
        externally_visible) {
        plan->reason = RXAS_FLOW_PROOF_TEMPORARY_OBSERVABLE;
        goto complete;
    }
    alias_count = 0;
    externally_visible = 1;
    if (!rxas_flow_storage_aliases_at_instruction(
                service->ssa, expected_epoch, producer_instruction, 0,
                destination_at_producer.storage_id, &alias_count,
                &externally_visible) || alias_count != 1 ||
        externally_visible) {
        plan->reason = RXAS_FLOW_PROOF_DESTINATION_OBSERVABLE;
        goto complete;
    }
    for (operand_index = 0; operand_index < producer_item->operandCount;
         operand_index++) {
        RxasFlowRegister input;
        RxasFlowStorageFact input_storage;
        if (!rxop_effect_reads_operand(&producer->effects, operand_index))
            continue;
        if (!flow_proof_register(
                    flow_proof_operand(producer_item, operand_index), &input) ||
            !rxas_flow_storage_at_instruction(
                    service->ssa, expected_epoch, producer_instruction, 0,
                    input, &input_storage) || !input_storage.storage_id ||
            input_storage.storage_id == destination_at_producer.storage_id) {
            plan->reason = RXAS_FLOW_PROOF_DESTINATION_OBSERVABLE;
            goto complete;
        }
    }
    if (!rxas_flow_component_at_instruction(
                service->ssa, expected_epoch, producer_instruction, 1,
                temporary, component, &temporary_result) ||
        !rxas_flow_component_at_instruction(
                service->ssa, expected_epoch, copy_instruction, 0,
                copy_source, component, &copy_source_value) ||
        !rxas_flow_component_at_instruction(
                service->ssa, expected_epoch, copy_instruction, 1,
                destination, component, &copy_result) ||
        temporary_result.value_id == RXAS_FLOW_ID_NONE ||
        temporary_result.defining_instruction != producer_instruction ||
        copy_source_value.value_id == RXAS_FLOW_ID_NONE ||
        copy_result.value_id == RXAS_FLOW_ID_NONE ||
        copy_result.kind != RXAS_FLOW_VALUE_COPY ||
        copy_result.defining_instruction != copy_instruction) {
        plan->reason = RXAS_FLOW_PROOF_RESULT_UNAVAILABLE;
        goto complete;
    }
    walk = flow_proof_equivalent_value(
            service, temporary_result.value_id,
            copy_source_value.value_id, &equivalent_id, &equivalent);
    if (walk != FLOW_PROOF_VALUE_UNIQUE) {
        plan->reason = service->metrics.status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                : RXAS_FLOW_PROOF_SOURCE_NOT_EQUIVALENT;
        goto complete;
    }
    walk = flow_proof_equivalent_value(
            service, temporary_result.value_id,
            copy_result.source_value_id, &equivalent_id, &equivalent);
    if (walk != FLOW_PROOF_VALUE_UNIQUE) {
        plan->reason = service->metrics.status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                : RXAS_FLOW_PROOF_SOURCE_NOT_EQUIVALENT;
        goto complete;
    }
    plan->temporary_result_value_id = temporary_result.value_id;
    plan->copy_result_value_id = copy_result.value_id;
    for (cleanup_index = 0;
         cleanup_index < sizeof(cleanup_bits) / sizeof(cleanup_bits[0]);
         cleanup_index++) {
        RxasFlowComponentFact temporary_cleanup;
        RxasFlowComponentFact destination_cleanup;
        if (!(cleanup_components & cleanup_bits[cleanup_index])) continue;
        if (!rxas_flow_component_at_instruction(
                    service->ssa, expected_epoch, producer_instruction, 0,
                    temporary, cleanup_bits[cleanup_index],
                    &temporary_cleanup) ||
            !rxas_flow_component_at_instruction(
                    service->ssa, expected_epoch, producer_instruction, 0,
                    destination, cleanup_bits[cleanup_index],
                    &destination_cleanup) ||
            !flow_proof_local_component_absent(
                    service, expected_epoch, &temporary_cleanup,
                    temporary_before.storage_id) ||
            !flow_proof_local_component_absent(
                    service, expected_epoch, &destination_cleanup,
                    destination_at_producer.storage_id)) {
            plan->reason = service->metrics.status ==
                        RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                    ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                    : RXAS_FLOW_PROOF_CLEANUP_REQUIRED;
            goto complete;
        }
    }
    address_observed = flow_proof_address_observed_after_copy(
            service, expected_epoch, copy->record_id);
    if (address_observed < 0) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        goto complete;
    }
    if (address_observed) {
        plan->reason = RXAS_FLOW_PROOF_ADDRESS_OBSERVED;
        goto complete;
    }
    service->use = rxas_flow_require_use_analysis(
            service->procedure, expected_epoch, 0);
    use_metrics = service->use ? rxas_flow_use_metrics(
            service->use, expected_epoch) : 0;
    if (!service->use || !use_metrics) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        goto complete;
    }
    if (use_metrics->opaque_observations) {
        plan->reason = RXAS_FLOW_PROOF_TEMPORARY_OBSERVABLE;
        goto complete;
    }
    all_use_count = rxas_flow_use_count(service->use, expected_epoch);
    for (all_use_index = 0; all_use_index < all_use_count; all_use_index++) {
        const RxasFlowUse *use;
        use = rxas_flow_use(service->use, expected_epoch, all_use_index);
        if (!use || use->value_id != RXAS_FLOW_ID_NONE ||
            flow_proof_use_is_pure_write(use) ||
            use->kind == RXAS_FLOW_USE_CALL_WINDOW_READ ||
            use->component != component)
            continue;
        if (flow_proof_same_register(use->register_id, temporary) ||
            flow_proof_same_register(use->register_id, destination)) {
            plan->reason = RXAS_FLOW_PROOF_TEMPORARY_OBSERVABLE;
            goto complete;
        }
    }
    if (!flow_proof_mark_value_dependents(
                service, expected_epoch, temporary_result.value_id,
                &marked_count)) {
        plan->reason = service->metrics.status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                : RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        goto complete;
    }
    copy_reads = 0;
    for (marked_index = 0; marked_index < marked_count; marked_index++) {
        equivalent_id = service->value_stack[marked_index];
        use_count = rxas_flow_value_direct_use_count(
                service->use, expected_epoch, equivalent_id);
        for (use_index = 0; use_index < use_count; use_index++) {
            const RxasFlowUse *use;
            use = rxas_flow_value_direct_use(
                    service->use, expected_epoch,
                    equivalent_id, use_index);
            if (use && use->instruction_id == copy_instruction &&
                use->operand_index == 1 &&
                use->kind == RXAS_FLOW_USE_EXPLICIT_READ &&
                use->component == component &&
                use->read_components == component &&
                flow_proof_same_register(use->register_id, copy_source)) {
                copy_reads++;
                continue;
            }
            plan->reason = RXAS_FLOW_PROOF_TEMPORARY_OBSERVABLE;
            goto complete;
        }
    }
    if (copy_reads != 1) {
        plan->reason = RXAS_FLOW_PROOF_TEMPORARY_OBSERVABLE;
        goto complete;
    }
    if (use_metrics->call_window_reads) {
        for (all_use_index = 0; all_use_index < all_use_count;
             all_use_index++) {
            const RxasFlowUse *use;
            RxasFlowComponentFact at_call;
            use = rxas_flow_use(
                    service->use, expected_epoch, all_use_index);
            if (!use || use->kind != RXAS_FLOW_USE_CALL_WINDOW_READ ||
                use->instruction_id == RXAS_FLOW_ID_NONE ||
                use->register_id.number == RXAS_FLOW_ID_NONE ||
                temporary.number <= use->register_id.number)
                continue;
            if (!rxas_flow_component_at_instruction(
                        service->ssa, expected_epoch,
                        use->instruction_id, 0,
                        temporary, component, &at_call) ||
                flow_proof_value_dependent_is_marked(
                        service, at_call.value_id)) {
                plan->reason = RXAS_FLOW_PROOF_CALL_WINDOW_OBSERVED;
                goto complete;
            }
        }
    }
    plan->producer_rewrite.record_id = producer->record_id;
    plan->producer_rewrite.instruction_id = producer_instruction;
    plan->producer_rewrite.operand_index = 0;
    plan->producer_rewrite.expected_register = temporary;
    plan->producer_rewrite.replacement_register = destination;
    plan->proved = 1;
    plan->reason = RXAS_FLOW_PROOF_PROVED;

complete:
    if (plan->proved) service->metrics.producer_forward_proved++;
    else service->metrics.producer_forward_rejected++;
    return 1;
}

static void flow_proof_compare_branch_plan_init(
        RxasFlowCompareBranchPlan *plan,
        size_t compare_instruction, size_t branch_instruction) {
    memset(plan, 0, sizeof(*plan));
    plan->reason = RXAS_FLOW_PROOF_STALE_EPOCH;
    plan->compare_instruction = compare_instruction;
    plan->branch_instruction = branch_instruction;
    plan->compare_record_id = RXAS_FLOW_ID_NONE;
    plan->branch_record_id = RXAS_FLOW_ID_NONE;
    plan->expected_compare_opcode = -1;
    plan->expected_branch_opcode = -1;
    plan->fused_opcode = -1;
    plan->left_source_operand = RXAS_FLOW_ID_NONE;
    plan->right_source_operand = RXAS_FLOW_ID_NONE;
    plan->result_value_id = RXAS_FLOW_ID_NONE;
    plan->trace_deletion_offset = RXAS_FLOW_ID_NONE;
    plan->result_register.number = RXAS_FLOW_ID_NONE;
}

int rxas_flow_prove_compare_branch_fusion(
        const RxasFlowProofService *const_service,
        unsigned long expected_epoch, size_t compare_instruction,
        size_t branch_instruction, RxasFlowCompareBranchPlan *plan) {
    RxasFlowProofService *service;
    const RxasFlowInstruction *compare;
    const RxasFlowInstruction *branch;
    const RxasFlowRecord *compare_record;
    const RxasFlowRecord *branch_record;
    const instruction_queue *compare_item;
    const instruction_queue *branch_item;
    const RxasFlowUseMetrics *use_metrics;
    RxOpCompareBranchFusion fusion;
    RxOpEffects fused_effects;
    RxOpSignalContract fused_signal;
    RxasFlowRegister result_register;
    RxasFlowRegister branch_register;
    RxasFlowStorageFact result_before;
    RxasFlowStorageFact result_after;
    RxasFlowStorageFact result_at_branch;
    RxasFlowComponentFact result_value;
    RxasFlowComponentFact branch_value;
    size_t alias_count;
    size_t marked_count;
    size_t marked_index;
    size_t branch_reads;
    size_t use_count;
    size_t use_index;
    size_t all_use_count;
    size_t all_use_index;
    size_t cleanup_index;
    size_t trace_deletion_start;
    int externally_visible;
    unsigned int cleanup_components;
    static const unsigned int cleanup_bits[] = {
        RXOP_COMPONENT_INTEGER,
        RXOP_COMPONENT_FLOAT,
        RXOP_COMPONENT_STRING,
        RXOP_COMPONENT_DECIMAL,
        RXOP_COMPONENT_BINARY,
        RXOP_COMPONENT_ATTRIBUTES,
        RXOP_COMPONENT_REFERENCE,
        RXOP_COMPONENT_NATIVE_PAYLOAD,
        RXOP_COMPONENT_ATTRIBUTE_COUNT
    };
    if (!plan) return 0;
    flow_proof_compare_branch_plan_init(
            plan, compare_instruction, branch_instruction);
    if (!const_service || !expected_epoch ||
        const_service->metrics.epoch != expected_epoch ||
        !rxas_flow_procedure_epoch_matches(
                const_service->procedure, expected_epoch))
        return 1;
    if (const_service->metrics.status ==
            RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED) {
        plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        return 1;
    }
    if (const_service->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    if (!flow_proof_has_capabilities(
                const_service, FLOW_PROOF_USE_CAPABILITIES)) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    service = (RxasFlowProofService *)const_service;
    service->metrics.compare_branch_queries++;
    trace_deletion_start = service->trace_deletion_count;
    if (!flow_proof_consume(service, 12)) {
        plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        goto complete;
    }
    compare = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, compare_instruction);
    branch = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, branch_instruction);
    compare_record = compare ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, compare->record_id) : 0;
    branch_record = branch ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, branch->record_id) : 0;
    compare_item = compare_record ? compare_record->queue_record : 0;
    branch_item = branch_record ? branch_record->queue_record : 0;
    if (!compare || !branch || !compare_record || !branch_record ||
        !compare_item || !branch_item || !compare->op || !branch->op) {
        plan->reason = RXAS_FLOW_PROOF_INVALID_INSTRUCTION;
        goto complete;
    }
    plan->compare_record_id = compare->record_id;
    plan->branch_record_id = branch->record_id;
    plan->expected_compare_opcode = compare->op->opcode;
    plan->expected_branch_opcode = branch->op->opcode;
    if (!rxop_compare_branch_fusion(
                compare->op->opcode, branch->op->opcode, &fusion)) {
        plan->reason = RXAS_FLOW_PROOF_NOT_EXACT_COMPARE_BRANCH_FUSION;
        goto complete;
    }
    plan->fused_opcode = fusion.fused_opcode;
    plan->left_source_operand = fusion.left_source_operand;
    plan->right_source_operand = fusion.right_source_operand;
    if (compare_instruction + 1 != branch_instruction ||
        compare->block_id != branch->block_id ||
        compare->record_id >= branch->record_id) {
        plan->reason = RXAS_FLOW_PROOF_NOT_ADJACENT_BRANCH;
        goto complete;
    }
    fused_effects = rxop_effects(fusion.fused_opcode);
    fused_signal = rxop_signal_contract(fusion.fused_opcode);
    if (compare_item->operandCount != 3 ||
        branch_item->operandCount != 2 ||
        compare->effects.state != RXOP_EFFECT_CLASSIFIED ||
        compare->effects.flow != FLOW_NEXT ||
        compare->effects.optimizer_barrier ||
        compare->effects.writes != RXOP_OP_1 ||
        compare->effects.kills != RXOP_OP_1 ||
        compare->effects.branch_targets != RXOP_OP_NONE ||
        compare->effects.implicit != RXOP_IMPLICIT_NONE ||
        compare->effects.semantics != RXOP_SEM_NONE ||
        compare->effects.cursor_reads != RXOP_OP_NONE ||
        compare->effects.cursor_writes != RXOP_OP_NONE ||
        compare->signal.state != RXOP_SIGNAL_STATE_NONE ||
        rxop_context_writes(compare->op->opcode) != RXOP_CONTEXT_NONE ||
        rxop_component_writes(compare->op->opcode, 0) !=
                RXOP_COMPONENT_INTEGER ||
        branch->effects.state != RXOP_EFFECT_CLASSIFIED ||
        branch->effects.flow != FLOW_COND ||
        branch->effects.optimizer_barrier ||
        branch->effects.reads != RXOP_OP_2 ||
        branch->effects.writes != RXOP_OP_NONE ||
        branch->effects.kills != RXOP_OP_NONE ||
        branch->effects.branch_targets != RXOP_OP_1 ||
        branch->effects.implicit != RXOP_IMPLICIT_NONE ||
        branch->effects.semantics != RXOP_SEM_NONE ||
        branch->effects.cursor_reads != RXOP_OP_NONE ||
        branch->effects.cursor_writes != RXOP_OP_NONE ||
        branch->signal.state != RXOP_SIGNAL_STATE_NONE ||
        rxop_context_writes(branch->op->opcode) != RXOP_CONTEXT_NONE ||
        rxop_component_reads(branch->op->opcode, 1) !=
                RXOP_COMPONENT_INTEGER ||
        fused_effects.state != RXOP_EFFECT_CLASSIFIED ||
        fused_effects.flow != FLOW_COND ||
        fused_effects.optimizer_barrier ||
        fused_effects.writes != RXOP_OP_NONE ||
        fused_effects.kills != RXOP_OP_NONE ||
        fused_effects.branch_targets != RXOP_OP_1 ||
        fused_effects.implicit != RXOP_IMPLICIT_NONE ||
        fused_effects.semantics != RXOP_SEM_NONE ||
        fused_effects.cursor_reads != RXOP_OP_NONE ||
        fused_effects.cursor_writes != RXOP_OP_NONE ||
        fused_signal.state != RXOP_SIGNAL_STATE_NONE ||
        rxop_context_writes(fusion.fused_opcode) != RXOP_CONTEXT_NONE ||
        rxop_format_operand_count(op_table[fusion.fused_opcode].format) != 3 ||
        rxop_format_operand_type(op_table[fusion.fused_opcode].format, 0) !=
                OP_ID ||
        rxop_format_operand_type(op_table[fusion.fused_opcode].format, 1) !=
                OP_REG ||
        !branch_item->operand1Token ||
        branch_item->operand1Token->token_type != ID ||
        !flow_proof_register(
                flow_proof_operand(compare_item, 0), &result_register) ||
        !flow_proof_register(
                flow_proof_operand(branch_item, 1), &branch_register) ||
        result_register.register_class != RXAS_FLOW_REGISTER_LOCAL ||
        !flow_proof_same_register(result_register, branch_register)) {
        plan->reason = RXAS_FLOW_PROOF_NOT_EXACT_COMPARE_BRANCH_FUSION;
        goto complete;
    }
    plan->result_register = result_register;
    for (use_index = 0; use_index < 2; use_index++) {
        size_t source_operand;
        Assembler_Token *source_token;
        OperandType expected_type;
        RxasFlowRegister source_register;
        RxasFlowStorageFact source_storage;
        source_operand = use_index ? fusion.right_source_operand
                                   : fusion.left_source_operand;
        source_token = flow_proof_operand(compare_item, source_operand);
        expected_type = rxop_format_operand_type(
                op_table[fusion.fused_opcode].format, use_index + 1);
        if (!source_token ||
            (expected_type == OP_REG &&
             (!flow_proof_register(source_token, &source_register) ||
              rxop_component_reads(compare->op->opcode, source_operand) !=
                    RXOP_COMPONENT_INTEGER)) ||
            (expected_type == OP_INT && source_token->token_type != INT)) {
            plan->reason = RXAS_FLOW_PROOF_NOT_EXACT_COMPARE_BRANCH_FUSION;
            goto complete;
        }
        if (expected_type != OP_REG) continue;
        if (!rxas_flow_storage_at_instruction(
                    service->ssa, expected_epoch, compare_instruction, 0,
                    source_register, &source_storage) ||
            !source_storage.storage_id) {
            plan->reason = RXAS_FLOW_PROOF_STORAGE_UNKNOWN;
            goto complete;
        }
        if (!rxas_flow_storage_at_instruction(
                    service->ssa, expected_epoch, compare_instruction, 0,
                    result_register, &result_before) ||
            source_storage.storage_id == result_before.storage_id) {
            plan->reason = RXAS_FLOW_PROOF_DESTINATION_OBSERVABLE;
            goto complete;
        }
    }
    if (!rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, compare_instruction, 0,
                result_register, &result_before) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, compare_instruction, 1,
                result_register, &result_after) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, branch_instruction, 0,
                result_register, &result_at_branch) ||
        !result_before.storage_id ||
        result_before.storage_id != result_after.storage_id ||
        result_after.storage_id != result_at_branch.storage_id) {
        plan->reason = RXAS_FLOW_PROOF_STORAGE_CHANGED;
        goto complete;
    }
    plan->result_storage_id = result_after.storage_id;
    if (!rxas_flow_storage_is_local_base(
                service->ssa, expected_epoch, result_after.storage_id)) {
        plan->reason = RXAS_FLOW_PROOF_DESTINATION_NOT_LOCAL;
        goto complete;
    }
    alias_count = 0;
    externally_visible = 1;
    if (!rxas_flow_storage_aliases_at_instruction(
                service->ssa, expected_epoch, compare_instruction, 0,
                result_before.storage_id, &alias_count, &externally_visible) ||
        alias_count != 1 || externally_visible) {
        plan->reason = RXAS_FLOW_PROOF_DESTINATION_OBSERVABLE;
        goto complete;
    }
    cleanup_components = rxop_component_clears(compare->op->opcode, 0);
    for (cleanup_index = 0;
         cleanup_index < sizeof(cleanup_bits) / sizeof(cleanup_bits[0]);
         cleanup_index++) {
        RxasFlowComponentFact cleanup;
        if (!(cleanup_components & cleanup_bits[cleanup_index])) continue;
        if (!rxas_flow_component_at_instruction(
                    service->ssa, expected_epoch, compare_instruction, 0,
                    result_register, cleanup_bits[cleanup_index], &cleanup) ||
            !flow_proof_local_component_absent(
                    service, expected_epoch, &cleanup,
                    result_before.storage_id)) {
            plan->reason = service->metrics.status ==
                        RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                    ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                    : RXAS_FLOW_PROOF_CLEANUP_REQUIRED;
            goto complete;
        }
    }
    if (!rxas_flow_component_at_instruction(
                service->ssa, expected_epoch, compare_instruction, 1,
                result_register, RXOP_COMPONENT_INTEGER, &result_value) ||
        !rxas_flow_component_at_instruction(
                service->ssa, expected_epoch, branch_instruction, 0,
                branch_register, RXOP_COMPONENT_INTEGER, &branch_value) ||
        result_value.value_id == RXAS_FLOW_ID_NONE ||
        result_value.defining_instruction != compare_instruction ||
        branch_value.value_id != result_value.value_id) {
        plan->reason = RXAS_FLOW_PROOF_RESULT_UNAVAILABLE;
        goto complete;
    }
    plan->result_value_id = result_value.value_id;
    service->use = rxas_flow_require_use_analysis(
            service->procedure, expected_epoch, 0);
    use_metrics = service->use ? rxas_flow_use_metrics(
            service->use, expected_epoch) : 0;
    if (!service->use || !use_metrics) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        goto complete;
    }
    if (use_metrics->opaque_observations) {
        plan->reason = RXAS_FLOW_PROOF_COMPARE_RESULT_OBSERVED;
        goto complete;
    }
    if (!flow_proof_mark_value_dependents(
                service, expected_epoch, result_value.value_id,
                &marked_count)) {
        plan->reason = service->metrics.status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                : RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        goto complete;
    }
    branch_reads = 0;
    for (marked_index = 0; marked_index < marked_count; marked_index++) {
        size_t value_id;
        value_id = service->value_stack[marked_index];
        use_count = rxas_flow_value_direct_use_count(
                service->use, expected_epoch, value_id);
        for (use_index = 0; use_index < use_count; use_index++) {
            const RxasFlowUse *use;
            use = rxas_flow_value_direct_use(
                    service->use, expected_epoch, value_id, use_index);
            if (use && use->instruction_id == branch_instruction &&
                use->operand_index == 1 &&
                use->kind == RXAS_FLOW_USE_EXPLICIT_READ &&
                use->component == RXOP_COMPONENT_INTEGER &&
                use->read_components == RXOP_COMPONENT_INTEGER &&
                flow_proof_same_register(
                        use->register_id, result_register)) {
                branch_reads++;
                continue;
            }
            if (use && value_id == result_value.value_id &&
                use->kind == RXAS_FLOW_USE_TRACE_READ &&
                use->instruction_id == RXAS_FLOW_ID_NONE &&
                use->operand_index == RXAS_FLOW_ID_NONE &&
                use->record_id > compare->record_id &&
                use->record_id < branch->record_id &&
                use->value_id == result_value.value_id &&
                use->storage_id == result_after.storage_id &&
                use->component == RXOP_COMPONENT_INTEGER &&
                use->read_components == RXOP_COMPONENT_INTEGER &&
                flow_proof_same_register(
                        use->register_id, result_register)) {
                if (!flow_proof_append_trace_deletion(service, use)) {
                    plan->reason = service->metrics.status ==
                                RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                            ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                            : RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
                    goto complete;
                }
                continue;
            }
            plan->reason = use && use->kind == RXAS_FLOW_USE_TRACE_READ
                    ? RXAS_FLOW_PROOF_TRACE_OBSERVED
                    : RXAS_FLOW_PROOF_COMPARE_RESULT_OBSERVED;
            goto complete;
        }
    }
    if (branch_reads != 1) {
        plan->reason = RXAS_FLOW_PROOF_COMPARE_RESULT_OBSERVED;
        goto complete;
    }
    all_use_count = rxas_flow_use_count(service->use, expected_epoch);
    for (all_use_index = 0; all_use_index < all_use_count; all_use_index++) {
        const RxasFlowUse *use;
        use = rxas_flow_use(
                service->use, expected_epoch, all_use_index);
        if (!use) continue;
        /* Sparse writes share the use index so later component proofs can
         * find them, but they do not observe the overwritten ValueId. */
        if (use->value_id == RXAS_FLOW_ID_NONE &&
            !flow_proof_use_is_pure_write(use) &&
            use->kind != RXAS_FLOW_USE_CALL_WINDOW_READ &&
            flow_proof_same_register(use->register_id, result_register)) {
            plan->reason = RXAS_FLOW_PROOF_COMPARE_RESULT_OBSERVED;
            goto complete;
        }
        if (use->kind == RXAS_FLOW_USE_CALL_WINDOW_READ) {
            RxasFlowRegister window_register;
            size_t base_register;
            size_t last_register;
            size_t register_number;
            int observed;
            if (use->instruction_id == RXAS_FLOW_ID_NONE ||
                use->register_id.register_class !=
                        RXAS_FLOW_REGISTER_LOCAL ||
                use->register_id.number == RXAS_FLOW_ID_NONE ||
                !rxas_flow_call_window_bounds_at_instruction(
                        service->ssa, expected_epoch,
                        use->instruction_id,
                        &base_register, &last_register) ||
                base_register != use->register_id.number ||
                last_register < base_register) {
                plan->reason = RXAS_FLOW_PROOF_CALL_WINDOW_OBSERVED;
                goto complete;
            }
            observed = 0;
            window_register.register_class = RXAS_FLOW_REGISTER_LOCAL;
            register_number = base_register;
            while (register_number < last_register) {
                RxasFlowComponentFact at_call;
                FlowProofDependencyResult dependency;
                register_number++;
                window_register.number = register_number;
                if (!rxas_flow_component_at_instruction(
                            service->ssa, expected_epoch,
                            use->instruction_id, 0,
                            window_register, RXOP_COMPONENT_INTEGER,
                            &at_call)) {
                    observed = 1;
                    break;
                }
                dependency = flow_proof_value_depends_on(
                        service, expected_epoch, at_call.value_id,
                        result_value.value_id);
                if (dependency != FLOW_PROOF_DEPENDENCY_ABSENT) {
                    observed = 1;
                    break;
                }
            }
            if (observed) {
                plan->reason = service->metrics.status ==
                            RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                        ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                        : RXAS_FLOW_PROOF_CALL_WINDOW_OBSERVED;
                goto complete;
            }
        }
    }
    plan->proved = 1;
    plan->reason = RXAS_FLOW_PROOF_PROVED;
    plan->trace_deletion_offset = trace_deletion_start;
    plan->trace_deletion_count =
            service->trace_deletion_count - trace_deletion_start;

complete:
    if (plan->proved) {
        service->metrics.compare_branch_proved++;
        service->metrics.compare_branch_trace_deletions +=
                plan->trace_deletion_count;
    }
    else {
        service->trace_deletion_count = trace_deletion_start;
        service->metrics.compare_branch_rejected++;
    }
    service->metrics.retained_bytes = sizeof(*service) +
            (service->block_count * 3 + service->edge_count + 1) *
                    sizeof(size_t) +
            service->value_capacity * 4 * sizeof(size_t) +
            service->storage_capacity * 2 * sizeof(size_t) +
            service->effect_capacity * 3 * sizeof(size_t) +
            service->cache_capacity * sizeof(*service->cache) +
            service->rewrite_capacity * sizeof(*service->rewrites) +
            service->trace_deletion_capacity *
                    sizeof(*service->trace_deletions);
    return 1;
}

int rxas_flow_compare_branch_plan_trace_deletion(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        const RxasFlowCompareBranchPlan *plan, size_t deletion_index,
        RxasFlowTraceDeletion *deletion) {
    size_t index;
    if (!deletion || !plan || !plan->proved ||
        !flow_proof_valid(service, expected_epoch) ||
        deletion_index >= plan->trace_deletion_count ||
        plan->trace_deletion_offset == RXAS_FLOW_ID_NONE ||
        plan->trace_deletion_offset > (size_t)-1 - deletion_index)
        return 0;
    index = plan->trace_deletion_offset + deletion_index;
    if (index >= service->trace_deletion_count) return 0;
    *deletion = service->trace_deletions[index];
    return 1;
}

static void flow_proof_duplicate_linked_read_plan_init(
        RxasFlowDuplicateLinkedReadPlan *plan,
        size_t first_link_instruction, size_t second_link_instruction) {
    memset(plan, 0, sizeof(*plan));
    plan->reason = RXAS_FLOW_PROOF_STALE_EPOCH;
    plan->first_link_instruction = first_link_instruction;
    plan->second_link_instruction = second_link_instruction;
    plan->first_link_record_id = RXAS_FLOW_ID_NONE;
    plan->first_copy_record_id = RXAS_FLOW_ID_NONE;
    plan->first_unlink_record_id = RXAS_FLOW_ID_NONE;
    plan->second_link_record_id = RXAS_FLOW_ID_NONE;
    plan->second_copy_record_id = RXAS_FLOW_ID_NONE;
    plan->second_unlink_record_id = RXAS_FLOW_ID_NONE;
    plan->expected_link_opcode = -1;
    plan->expected_copy_opcode = -1;
}

static int flow_proof_duplicate_copy_opcode(int opcode) {
    return opcode == OP_COPY_REG_REG || opcode == OP_BCOPY_REG_REG ||
           opcode == OP_ICOPY_REG_REG || opcode == OP_SCOPY_REG_REG ||
           opcode == OP_FCOPY_REG_REG || opcode == OP_DCOPY_REG_REG;
}

static int flow_proof_record_instruction(
        RxasFlowProofService *service, unsigned long expected_epoch,
        size_t record_id, const RxasFlowInstruction **instruction,
        const instruction_queue **item) {
    const RxasFlowRecord *record;
    record = rxas_flow_procedure_record(
            service->procedure, expected_epoch, record_id);
    if (!record || record->instruction_id == RXAS_FLOW_ID_NONE) return 0;
    *instruction = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, record->instruction_id);
    *item = record->queue_record;
    return *instruction && (*instruction)->op && *item;
}

static size_t flow_proof_unique_normal_edge(
        RxasFlowProofService *service,
        const RxasFlowInstruction *instruction) {
    size_t edge_id;
    size_t normal_edge;
    size_t normal_count;
    size_t offset;
    if (!service || !instruction ||
        instruction->block_id >= service->block_count)
        return RXAS_FLOW_ID_NONE;
    normal_edge = RXAS_FLOW_ID_NONE;
    normal_count = 0;
    for (offset = service->outgoing_offsets[instruction->block_id];
         offset < service->outgoing_offsets[instruction->block_id + 1];
         offset++) {
        const RxasFlowEdge *edge;
        if (!flow_proof_consume(service, 1)) return RXAS_FLOW_ID_NONE;
        edge_id = service->outgoing_edges[offset];
        edge = rxas_flow_procedure_edge(
                service->procedure, service->metrics.epoch, edge_id);
        if (edge && edge->kind == RXAS_FLOW_EDGE_NORMAL) {
            normal_edge = edge_id;
            normal_count++;
        }
    }
    return normal_count == 1 ? normal_edge : RXAS_FLOW_ID_NONE;
}

static int flow_proof_attribute_link_in_range(
        RxasFlowProofService *service, unsigned long expected_epoch,
        size_t instruction_id, RxasFlowRegister owner, size_t slot) {
    RxasFlowComponentFact count;
    RxasFlowValueNode leaf;
    size_t leaf_id;
    FlowProofValueWalkResult walk;
    if (!slot || !rxas_flow_component_at_instruction(
                service->ssa, expected_epoch, instruction_id, 0,
                owner, RXOP_COMPONENT_ATTRIBUTE_COUNT, &count) ||
        count.value_id == RXAS_FLOW_ID_NONE)
        return 0;
    walk = flow_proof_unique_value_leaf(
            service, count.value_id, &leaf_id, &leaf);
    return walk == FLOW_PROOF_VALUE_UNIQUE &&
           leaf.kind == RXAS_FLOW_VALUE_CONSTANT &&
           leaf.constant_token && leaf.constant_token->token_type == INT &&
           leaf.constant_token->token_value.integer >= 0 &&
           (size_t)leaf.constant_token->token_value.integer >= slot;
}

static int flow_proof_cursor_unchanged_between(
        RxasFlowProofService *service, unsigned long expected_epoch,
        size_t storage_id, size_t first_record, size_t second_record) {
    size_t use_count;
    size_t use_index;
    use_count = rxas_flow_storage_use_count(
            service->use, expected_epoch, storage_id);
    for (use_index = 0; use_index < use_count; use_index++) {
        const RxasFlowUse *use;
        use = rxas_flow_storage_use(
                service->use, expected_epoch, storage_id, use_index);
        if (use && use->kind == RXAS_FLOW_USE_CURSOR_WRITE &&
            use->record_id > first_record && use->record_id < second_record)
            return 0;
    }
    return 1;
}

static int flow_proof_component_unwritten_between(
        RxasFlowProofService *service, unsigned long expected_epoch,
        size_t storage_id, unsigned int component,
        size_t first_record, size_t second_record) {
    size_t use_count;
    size_t use_index;
    size_t all_use_count;
    size_t low;
    size_t high;
    service->use = rxas_flow_require_use_analysis(
            service->procedure, expected_epoch, 0);
    if (!service->use) return 0;
    use_count = rxas_flow_storage_use_count(
            service->use, expected_epoch, storage_id);
    for (use_index = 0; use_index < use_count; use_index++) {
        const RxasFlowUse *use;
        if (!flow_proof_consume(service, 1)) return 0;
        use = rxas_flow_storage_use(
                service->use, expected_epoch, storage_id, use_index);
        if (use && use->kind == RXAS_FLOW_USE_EXPLICIT_WRITE &&
            use->component == component &&
            use->record_id > first_record &&
            use->record_id < second_record)
            return 0;
    }
    /* Unknown-storage writes and call windows are global sparse barriers.
     * Uses are collected in record order, so binary-search the relevant
     * interval instead of rescanning an inlined procedure per component. */
    all_use_count = rxas_flow_use_count(
            service->use, expected_epoch);
    low = 0;
    high = all_use_count;
    while (low < high) {
        const RxasFlowUse *use;
        size_t middle;
        middle = low + (high - low) / 2;
        use = rxas_flow_use(
                service->use, expected_epoch, middle);
        if (!use || use->record_id <= first_record) low = middle + 1;
        else high = middle;
    }
    for (use_index = low; use_index < all_use_count; use_index++) {
        const RxasFlowUse *use;
        if (!flow_proof_consume(service, 1)) return 0;
        use = rxas_flow_use(
                service->use, expected_epoch, use_index);
        if (!use || use->record_id >= second_record) break;
        if (use->kind == RXAS_FLOW_USE_OPAQUE_OBSERVATION ||
            use->kind == RXAS_FLOW_USE_OPAQUE_WRITE ||
            use->kind == RXAS_FLOW_USE_CALL_WINDOW_READ)
            return 0;
    }
    return 1;
}

int rxas_flow_prove_duplicate_linked_read(
        const RxasFlowProofService *const_service,
        unsigned long expected_epoch, size_t first_link_instruction,
        size_t second_link_instruction,
        RxasFlowDuplicateLinkedReadPlan *plan) {
    RxasFlowProofService *service;
    const RxasFlowInstruction *first_link;
    const RxasFlowInstruction *first_copy;
    const RxasFlowInstruction *first_unlink;
    const RxasFlowInstruction *second_link;
    const RxasFlowInstruction *second_copy;
    const RxasFlowInstruction *second_unlink;
    const RxasFlowRecord *first_link_record;
    const RxasFlowRecord *second_link_record;
    const instruction_queue *first_link_item;
    const instruction_queue *first_copy_item;
    const instruction_queue *first_unlink_item;
    const instruction_queue *second_link_item;
    const instruction_queue *second_copy_item;
    const instruction_queue *second_unlink_item;
    RxasFlowRegister first_owner;
    RxasFlowRegister second_owner;
    RxasFlowRegister observed;
    RxasFlowStorageFact first_linked;
    RxasFlowStorageFact second_linked;
    RxasFlowStorageFact first_detached_after;
    RxasFlowStorageFact first_detached_later;
    unsigned int read_components;
    size_t component_index;
    size_t first_source_edge;
    size_t second_source_edge;
    size_t slot;
    static const unsigned int components[] = {
        RXOP_COMPONENT_INTEGER,
        RXOP_COMPONENT_FLOAT,
        RXOP_COMPONENT_STRING,
        RXOP_COMPONENT_DECIMAL,
        RXOP_COMPONENT_BINARY,
        RXOP_COMPONENT_ATTRIBUTES,
        RXOP_COMPONENT_REFERENCE,
        RXOP_COMPONENT_NATIVE_PAYLOAD,
        RXOP_COMPONENT_ATTRIBUTE_COUNT
    };
    if (!plan) return 0;
    flow_proof_duplicate_linked_read_plan_init(
            plan, first_link_instruction, second_link_instruction);
    if (!const_service || !expected_epoch ||
        const_service->metrics.epoch != expected_epoch ||
        !rxas_flow_procedure_epoch_matches(
                const_service->procedure, expected_epoch))
        return 1;
    if (const_service->metrics.status ==
            RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED) {
        plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        return 1;
    }
    if (const_service->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    if (!flow_proof_has_capabilities(
                const_service, FLOW_PROOF_USE_CAPABILITIES)) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    service = (RxasFlowProofService *)const_service;
    service->metrics.duplicate_linked_read_queries++;
    if (!flow_proof_consume(service, 18)) {
        plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        goto complete;
    }
    first_link = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, first_link_instruction);
    second_link = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, second_link_instruction);
    first_link_record = first_link ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, first_link->record_id) : 0;
    second_link_record = second_link ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, second_link->record_id) : 0;
    first_link_item = first_link_record ? first_link_record->queue_record : 0;
    second_link_item = second_link_record ? second_link_record->queue_record : 0;
    if (!first_link || !second_link || !first_link->op || !second_link->op ||
        !first_link_item || !second_link_item ||
        first_link->record_id + 2 >= second_link->record_id ||
        first_link->op->opcode != second_link->op->opcode ||
        (first_link->op->opcode != OP_LINK_REG_REG &&
         first_link->op->opcode != OP_LINKATTR1_REG_REG_INT) ||
        first_link_item->operandCount !=
                (first_link->op->opcode == OP_LINK_REG_REG ? 2 : 3) ||
        second_link_item->operandCount != first_link_item->operandCount ||
        !flow_proof_record_instruction(
                service, expected_epoch, first_link->record_id + 1,
                &first_copy, &first_copy_item) ||
        !flow_proof_record_instruction(
                service, expected_epoch, first_link->record_id + 2,
                &first_unlink, &first_unlink_item) ||
        !flow_proof_record_instruction(
                service, expected_epoch, second_link->record_id + 1,
                &second_copy, &second_copy_item) ||
        !flow_proof_record_instruction(
                service, expected_epoch, second_link->record_id + 2,
                &second_unlink, &second_unlink_item) ||
        first_copy->op->opcode != second_copy->op->opcode ||
        !flow_proof_duplicate_copy_opcode(first_copy->op->opcode) ||
        first_unlink->op->opcode != OP_UNLINK_REG ||
        second_unlink->op->opcode != OP_UNLINK_REG ||
        first_copy_item->operandCount != 2 ||
        second_copy_item->operandCount != 2 ||
        first_unlink_item->operandCount != 1 ||
        second_unlink_item->operandCount != 1 ||
        first_copy->signal.state != RXOP_SIGNAL_STATE_NONE ||
        second_copy->signal.state != RXOP_SIGNAL_STATE_NONE ||
        first_unlink->signal.state != RXOP_SIGNAL_STATE_NONE ||
        second_unlink->signal.state != RXOP_SIGNAL_STATE_NONE ||
        (first_link->op->opcode == OP_LINK_REG_REG &&
         (first_link->signal.state != RXOP_SIGNAL_STATE_NONE ||
          second_link->signal.state != RXOP_SIGNAL_STATE_NONE)) ||
        (first_link->op->opcode == OP_LINKATTR1_REG_REG_INT &&
         (first_link->signal.state != RXOP_SIGNAL_STATE_KNOWN ||
          second_link->signal.state != RXOP_SIGNAL_STATE_KNOWN))) {
        plan->reason = RXAS_FLOW_PROOF_NOT_EXACT_DUPLICATE_LINKED_READ;
        goto complete;
    }
    plan->first_link_record_id = first_link->record_id;
    plan->first_copy_record_id = first_link->record_id + 1;
    plan->first_unlink_record_id = first_link->record_id + 2;
    plan->second_link_record_id = second_link->record_id;
    plan->second_copy_record_id = second_link->record_id + 1;
    plan->second_unlink_record_id = second_link->record_id + 2;
    plan->expected_link_opcode = first_link->op->opcode;
    plan->expected_copy_opcode = first_copy->op->opcode;
    first_source_edge = RXAS_FLOW_ID_NONE;
    second_source_edge = RXAS_FLOW_ID_NONE;
    read_components = rxop_component_reads(first_copy->op->opcode, 1);
    plan->read_components = read_components;
    if (!read_components ||
        !flow_proof_register(flow_proof_operand(first_link_item, 0),
                             &plan->first_temporary) ||
        !flow_proof_register(flow_proof_operand(first_link_item, 1),
                             &first_owner) ||
        !flow_proof_register(flow_proof_operand(first_copy_item, 0),
                             &plan->first_detached) ||
        !flow_proof_register(flow_proof_operand(first_copy_item, 1),
                             &observed) ||
        !flow_proof_same_register(plan->first_temporary, observed) ||
        !flow_proof_register(flow_proof_operand(first_unlink_item, 0),
                             &observed) ||
        !flow_proof_same_register(plan->first_temporary, observed) ||
        !flow_proof_register(flow_proof_operand(second_link_item, 0),
                             &plan->second_temporary) ||
        !flow_proof_register(flow_proof_operand(second_link_item, 1),
                             &second_owner) ||
        !flow_proof_register(flow_proof_operand(second_copy_item, 0),
                             &plan->second_destination) ||
        !flow_proof_register(flow_proof_operand(second_copy_item, 1),
                             &observed) ||
        !flow_proof_same_register(plan->second_temporary, observed) ||
        !flow_proof_register(flow_proof_operand(second_unlink_item, 0),
                             &observed) ||
        !flow_proof_same_register(plan->second_temporary, observed) ||
        plan->first_temporary.register_class != RXAS_FLOW_REGISTER_LOCAL ||
        plan->first_detached.register_class != RXAS_FLOW_REGISTER_LOCAL ||
        plan->second_temporary.register_class != RXAS_FLOW_REGISTER_LOCAL ||
        plan->second_destination.register_class !=
                RXAS_FLOW_REGISTER_LOCAL) {
        plan->reason = RXAS_FLOW_PROOF_NOT_EXACT_DUPLICATE_LINKED_READ;
        goto complete;
    }
    if (!flow_proof_instruction_dominates(
                service, first_link, second_link)) {
        plan->reason = RXAS_FLOW_PROOF_NOT_DOMINATED;
        goto complete;
    }
    if (first_link->op->opcode == OP_LINKATTR1_REG_REG_INT) {
        const Assembler_Token *first_slot;
        const Assembler_Token *second_slot;
        first_slot = flow_proof_operand(first_link_item, 2);
        second_slot = flow_proof_operand(second_link_item, 2);
        if (!first_slot || !second_slot || first_slot->token_type != INT ||
            second_slot->token_type != INT ||
            first_slot->token_value.integer <= 0 ||
            second_slot->token_value.integer <= 0 ||
            first_slot->token_value.integer !=
                    second_slot->token_value.integer) {
            plan->reason = RXAS_FLOW_PROOF_ATTRIBUTE_PATH_CHANGED;
            goto complete;
        }
        slot = (size_t)first_slot->token_value.integer;
        if (!flow_proof_attribute_link_in_range(
                    service, expected_epoch, first_link_instruction,
                    first_owner, slot) ||
            !flow_proof_attribute_link_in_range(
                    service, expected_epoch, second_link_instruction,
                    second_owner, slot)) {
            plan->reason = RXAS_FLOW_PROOF_ATTRIBUTE_RANGE_UNKNOWN;
            goto complete;
        }
        first_source_edge = flow_proof_unique_normal_edge(
                service, first_link);
        second_source_edge = flow_proof_unique_normal_edge(
                service, second_link);
        if (first_source_edge == RXAS_FLOW_ID_NONE ||
            second_source_edge == RXAS_FLOW_ID_NONE) {
            plan->reason = service->metrics.status ==
                        RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                    ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                    : RXAS_FLOW_PROOF_RESULT_UNAVAILABLE;
            goto complete;
        }
    }
    if (!rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, first_link_instruction, 1,
                plan->first_temporary, &first_linked) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, second_link_instruction, 1,
                plan->second_temporary, &second_linked) ||
        !first_linked.storage_id) {
        plan->reason = first_link->op->opcode == OP_LINKATTR1_REG_REG_INT
                ? RXAS_FLOW_PROOF_ATTRIBUTE_PATH_CHANGED
                : RXAS_FLOW_PROOF_STORAGE_CHANGED;
        goto complete;
    }
    plan->linked_storage_id = first_linked.storage_id;
    plan->candidate_linked_storage_id = second_linked.storage_id;
    if (first_link->op->opcode == OP_LINKATTR1_REG_REG_INT) {
        RxasFlowStorageNode first_path;
        RxasFlowStorageNode second_path;
        if (!rxas_flow_storage_node(
                    service->ssa, expected_epoch,
                    first_linked.storage_id, &first_path) ||
            !rxas_flow_storage_node(
                    service->ssa, expected_epoch,
                    second_linked.storage_id, &second_path) ||
            first_path.kind != RXAS_FLOW_STORAGE_ATTRIBUTE_PATH ||
            second_path.kind != RXAS_FLOW_STORAGE_ATTRIBUTE_PATH) {
            plan->reason = RXAS_FLOW_PROOF_ATTRIBUTE_PATH_CHANGED;
            goto complete;
        }
        plan->owner_storage_id = first_path.owner_storage_id;
        plan->candidate_owner_storage_id = second_path.owner_storage_id;
        plan->attribute_count_value_id =
                first_path.attribute_count_value_id;
        plan->candidate_attribute_count_value_id =
                second_path.attribute_count_value_id;
        plan->reference_effect_id = first_path.reference_effect_id;
        plan->candidate_reference_effect_id = second_path.reference_effect_id;
    }
    if (first_linked.storage_id != second_linked.storage_id) {
        plan->reason = first_link->op->opcode == OP_LINKATTR1_REG_REG_INT
                ? RXAS_FLOW_PROOF_ATTRIBUTE_PATH_CHANGED
                : RXAS_FLOW_PROOF_STORAGE_CHANGED;
        goto complete;
    }
    if (!rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, first_copy->id, 1,
                plan->first_detached, &first_detached_after) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, second_copy->id, 0,
                plan->first_detached, &first_detached_later) ||
        !first_detached_after.storage_id ||
        first_detached_after.storage_id !=
                first_detached_later.storage_id) {
        plan->reason = RXAS_FLOW_PROOF_STORAGE_CHANGED;
        goto complete;
    }
    for (component_index = 0;
         component_index < sizeof(components) / sizeof(components[0]);
         component_index++) {
        RxasFlowComponentFact first_source;
        RxasFlowComponentFact second_source;
        RxasFlowComponentFact detached_after;
        RxasFlowComponentFact detached_later;
        RxasFlowValueNode equivalent;
        size_t equivalent_id;
        FlowProofValueWalkResult walk;
        if (!(read_components & components[component_index])) continue;
        if (!(first_link->op->opcode == OP_LINKATTR1_REG_REG_INT
                    ? rxas_flow_component_on_edge(
                            service->ssa, expected_epoch,
                            first_source_edge, plan->first_temporary,
                            components[component_index], &first_source)
                    : rxas_flow_component_at_instruction(
                            service->ssa, expected_epoch,
                            first_link_instruction, 1,
                            plan->first_temporary,
                            components[component_index], &first_source)) ||
            !(second_link->op->opcode == OP_LINKATTR1_REG_REG_INT
                    ? rxas_flow_component_on_edge(
                            service->ssa, expected_epoch,
                            second_source_edge, plan->second_temporary,
                            components[component_index], &second_source)
                    : rxas_flow_component_at_instruction(
                            service->ssa, expected_epoch,
                            second_link_instruction, 1,
                            plan->second_temporary,
                            components[component_index], &second_source)) ||
            !rxas_flow_component_at_instruction(
                    service->ssa, expected_epoch, first_copy->id, 1,
                    plan->first_detached, components[component_index],
                    &detached_after) ||
            !rxas_flow_component_at_instruction(
                    service->ssa, expected_epoch, second_copy->id, 0,
                    plan->first_detached, components[component_index],
                    &detached_later)) {
            plan->reason = RXAS_FLOW_PROOF_RESULT_UNAVAILABLE;
            goto complete;
        }
        walk = flow_proof_equivalent_value(
                service, first_source.value_id, second_source.value_id,
                &equivalent_id, &equivalent);
        if (walk != FLOW_PROOF_VALUE_UNIQUE) {
            /* Full COPY deliberately invalidates dynamic ValueIds because it
             * can replace a destination's child attributes.  Path identity
             * already owns the owner/count/reference guards; a sparse write
             * query recovers the unchanged source when the only clobber was
             * a copy to a different storage. */
            if (!flow_proof_component_unwritten_between(
                        service, expected_epoch,
                        first_linked.storage_id,
                        components[component_index],
                        first_copy->record_id,
                        second_link->record_id)) {
                plan->rejected_component = components[component_index];
                plan->first_value_id = first_source.value_id;
                plan->candidate_value_id = second_source.value_id;
                plan->reason = service->metrics.status ==
                            RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                        ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                        : RXAS_FLOW_PROOF_SOURCE_NOT_EQUIVALENT;
                goto complete;
            }
        }
        walk = flow_proof_equivalent_value(
                service, detached_after.value_id, detached_later.value_id,
                &equivalent_id, &equivalent);
        if (walk != FLOW_PROOF_VALUE_UNIQUE) {
            plan->rejected_component = components[component_index];
            plan->first_value_id = detached_after.value_id;
            plan->candidate_value_id = detached_later.value_id;
            plan->reason = service->metrics.status ==
                        RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                    ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                    : RXAS_FLOW_PROOF_SOURCE_NOT_EQUIVALENT;
            goto complete;
        }
    }
    if (first_copy->effects.cursor_reads != RXOP_OP_NONE) {
        service->use = rxas_flow_require_use_analysis(
                service->procedure, expected_epoch, 0);
        if (!service->use ||
            !flow_proof_cursor_unchanged_between(
                    service, expected_epoch, first_linked.storage_id,
                    first_copy->record_id, second_copy->record_id) ||
            !flow_proof_cursor_unchanged_between(
                    service, expected_epoch,
                    first_detached_after.storage_id,
                    first_copy->record_id, second_copy->record_id)) {
            plan->reason = RXAS_FLOW_PROOF_CURSOR_OBSERVED;
            goto complete;
        }
    }
    plan->proved = 1;
    plan->reason = RXAS_FLOW_PROOF_PROVED;

complete:
    if (plan->proved) service->metrics.duplicate_linked_read_proved++;
    else service->metrics.duplicate_linked_read_rejected++;
    return 1;
}

static void flow_proof_storage_permutation_plan_init(
        RxasFlowStoragePermutationPlan *plan, size_t first_instruction,
        size_t second_instruction) {
    memset(plan, 0, sizeof(*plan));
    plan->reason = RXAS_FLOW_PROOF_STALE_EPOCH;
    plan->first_instruction = first_instruction;
    plan->second_instruction = second_instruction;
    plan->first_record_id = RXAS_FLOW_ID_NONE;
    plan->second_record_id = RXAS_FLOW_ID_NONE;
    plan->expected_first_opcode = -1;
    plan->expected_second_opcode = -1;
}

static int flow_proof_exact_mapping_instruction(
        const RxasFlowInstruction *instruction,
        const instruction_queue *item, int opcode, size_t operand_count) {
    return instruction && instruction->op && item &&
           instruction->op->opcode == opcode &&
           item->operandCount == operand_count &&
           instruction->effects.state == RXOP_EFFECT_CLASSIFIED &&
           instruction->effects.flow == FLOW_NEXT &&
           instruction->effects.branch_targets == RXOP_OP_NONE &&
           instruction->effects.implicit == RXOP_IMPLICIT_NONE &&
           instruction->effects.semantics == RXOP_SEM_NONE &&
           instruction->effects.cursor_reads == RXOP_OP_NONE &&
           instruction->effects.cursor_writes == RXOP_OP_NONE &&
           !instruction->effects.optimizer_barrier &&
           instruction->signal.state == RXOP_SIGNAL_STATE_NONE &&
           rxop_context_writes(opcode) == RXOP_CONTEXT_NONE &&
           rxas_flow_opcode_is_plain_mapping(opcode);
}

static int flow_proof_same_unordered_register_pair(
        RxasFlowRegister first_left, RxasFlowRegister first_right,
        RxasFlowRegister second_left, RxasFlowRegister second_right) {
    return (flow_proof_same_register(first_left, second_left) &&
            flow_proof_same_register(first_right, second_right)) ||
           (flow_proof_same_register(first_left, second_right) &&
            flow_proof_same_register(first_right, second_left));
}

static int flow_proof_call_window_observes_permutation(
        RxasFlowProofService *service, unsigned long expected_epoch,
        const RxasFlowUse *use, RxasFlowRegister left,
        RxasFlowRegister right) {
    size_t base_register;
    size_t last_register;
    if (left.register_class != RXAS_FLOW_REGISTER_LOCAL &&
        right.register_class != RXAS_FLOW_REGISTER_LOCAL)
        return 0;
    if (!use || use->instruction_id == RXAS_FLOW_ID_NONE ||
        use->register_id.register_class != RXAS_FLOW_REGISTER_LOCAL ||
        use->register_id.number == RXAS_FLOW_ID_NONE ||
        !rxas_flow_call_window_bounds_at_instruction(
                service->ssa, expected_epoch, use->instruction_id,
                &base_register, &last_register) ||
        base_register != use->register_id.number ||
        last_register < base_register)
        return 1;
    if (left.register_class == RXAS_FLOW_REGISTER_LOCAL &&
        left.number > base_register && left.number <= last_register)
        return 1;
    return right.register_class == RXAS_FLOW_REGISTER_LOCAL &&
           right.number > base_register && right.number <= last_register;
}

/* A known signal may split an otherwise linear interval without exposing the
 * current frame's temporary register mapping.  An inherited action either
 * resumes at the following instruction or leaves/unwinds the current frame;
 * a handler installed or merged in this procedure may branch back into the
 * frame and is therefore deliberately rejected.  Only canonical static-name
 * contracts can establish this for every signal raised by the instruction. */
static int flow_proof_signal_exits_discard_permutation(
        RxasFlowProofService *service, unsigned long expected_epoch,
        const RxasFlowInstruction *instruction) {
    const char *start;
    const char *end;
    if (!service || !instruction) return 0;
    if (instruction->signal.state == RXOP_SIGNAL_STATE_NONE) return 1;
    if (instruction->signal.state != RXOP_SIGNAL_STATE_KNOWN ||
        instruction->signal.source != RXOP_SIGNAL_SOURCE_STATIC_NAMES ||
        !instruction->signal.static_names ||
        !instruction->signal.static_names[0] ||
        (instruction->signal.properties &
                RXOP_SIGNAL_PROP_POLICY_WRITE))
        return 0;
    start = instruction->signal.static_names;
    while (*start) {
        char *name;
        size_t length;
        RxasFlowPolicyFact policy;
        end = strchr(start, '|');
        if (!end) end = start + strlen(start);
        length = (size_t)(end - start);
        if (!length) return 0;
        name = malloc(length + 1);
        if (!name) return 0;
        memcpy(name, start, length);
        name[length] = '\0';
        if (!rxas_flow_policy_at_instruction(
                    service->signal, expected_epoch, instruction->id, 0,
                    name, &policy) ||
            policy.state != RXAS_FLOW_POLICY_INHERITED_UNKNOWN) {
            free(name);
            return 0;
        }
        free(name);
        if (!*end) break;
        start = end + 1;
    }
    return 1;
}

static int flow_proof_mark_reachable_from_root(
        RxasFlowProofService *service, size_t root) {
    size_t head;
    size_t tail;
    if (!service || root >= service->block_count) return 0;
    service->visit_generation++;
    if (!service->visit_generation) {
        memset(service->visit_marks, 0,
               service->block_count * sizeof(*service->visit_marks));
        service->visit_generation = 1;
    }
    head = 0;
    tail = 0;
    if (!flow_proof_mark_root(service, root, &tail)) return 0;
    while (head < tail) {
        size_t block;
        size_t offset;
        block = service->visit_queue[head++];
        if (!flow_proof_consume(service, 1)) return 0;
        for (offset = service->outgoing_offsets[block];
             offset < service->outgoing_offsets[block + 1]; offset++) {
            const RxasFlowEdge *edge;
            edge = rxas_flow_procedure_edge(
                    service->procedure, service->metrics.epoch,
                    service->outgoing_edges[offset]);
            if (!edge || !flow_proof_mark_root(
                            service, edge->target, &tail))
                return 0;
        }
    }
    return 1;
}

int rxas_flow_prove_storage_permutation_round_trip(
        const RxasFlowProofService *const_service,
        unsigned long expected_epoch, size_t first_instruction,
        size_t second_instruction, RxasFlowStoragePermutationPlan *plan) {
    RxasFlowProofService *service;
    const RxasFlowInstruction *first;
    const RxasFlowInstruction *second;
    const RxasFlowRecord *first_record;
    const RxasFlowRecord *second_record;
    const instruction_queue *first_item;
    const instruction_queue *second_item;
    RxasFlowStorageFact before_left;
    RxasFlowStorageFact before_right;
    RxasFlowStorageFact after_first_left;
    RxasFlowStorageFact after_first_right;
    RxasFlowStorageFact after_second_left;
    RxasFlowStorageFact after_second_right;
    size_t record_id;
    size_t use_count;
    size_t use_index;
    int encoded_round_trip;
    if (!plan) return 0;
    flow_proof_storage_permutation_plan_init(
            plan, first_instruction, second_instruction);
    if (!const_service || !expected_epoch ||
        const_service->metrics.epoch != expected_epoch ||
        !rxas_flow_procedure_epoch_matches(
                const_service->procedure, expected_epoch))
        return 1;
    if (const_service->metrics.status ==
            RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED) {
        plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        return 1;
    }
    if (const_service->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    if (!flow_proof_has_capabilities(
                const_service, FLOW_PROOF_USE_CAPABILITIES)) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    service = (RxasFlowProofService *)const_service;
    service->metrics.storage_permutation_queries++;
    if (!flow_proof_consume(service, 10)) {
        plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        goto complete;
    }
    first = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, first_instruction);
    second = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, second_instruction);
    first_record = first ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, first->record_id) : 0;
    second_record = second ? rxas_flow_procedure_record(
            service->procedure, expected_epoch, second->record_id) : 0;
    first_item = first_record ? first_record->queue_record : 0;
    second_item = second_record ? second_record->queue_record : 0;
    encoded_round_trip = first_instruction == second_instruction;
    if (!first || !second || !first_record || !second_record ||
        !first_item || !second_item ||
        !(encoded_round_trip
                ? flow_proof_exact_mapping_instruction(
                        first, first_item,
                        OP_SWAPN_REG_REG_REG_REG, 4)
                : flow_proof_exact_mapping_instruction(
                        first, first_item, OP_SWAP_REG_REG, 2) &&
                  flow_proof_exact_mapping_instruction(
                        second, second_item, OP_SWAP_REG_REG, 2))) {
        plan->reason = RXAS_FLOW_PROOF_NOT_EXACT_STORAGE_PERMUTATION;
        goto complete;
    }
    plan->first_record_id = first->record_id;
    plan->second_record_id = second->record_id;
    plan->expected_first_opcode = first->op->opcode;
    plan->expected_second_opcode = second->op->opcode;
    plan->deletion_count = encoded_round_trip ? 1 : 2;
    if (!flow_proof_register(flow_proof_operand(first_item, 0),
                             &plan->first_left) ||
        !flow_proof_register(flow_proof_operand(first_item, 1),
                             &plan->first_right) ||
        !flow_proof_register(
                flow_proof_operand(second_item, encoded_round_trip ? 2 : 0),
                &plan->second_left) ||
        !flow_proof_register(
                flow_proof_operand(second_item, encoded_round_trip ? 3 : 1),
                &plan->second_right) ||
        flow_proof_same_register(plan->first_left, plan->first_right) ||
        !flow_proof_same_unordered_register_pair(
                plan->first_left, plan->first_right,
                plan->second_left, plan->second_right)) {
        plan->reason = RXAS_FLOW_PROOF_NOT_EXACT_STORAGE_PERMUTATION;
        goto complete;
    }
    if (!encoded_round_trip &&
        (first->record_id >= second->record_id ||
         !flow_proof_instruction_dominates(service, first, second))) {
        plan->reason = RXAS_FLOW_PROOF_NOT_DOMINATED;
        goto complete;
    }
    /* SWAPN executes its pairs in operand order.  The exact four-operand form
     * admitted above repeats one unordered physical pair, so its two
     * transpositions compose to identity for every possible starting
     * mapping, including an otherwise unknown or aliased StorageId. */
    if (encoded_round_trip) goto proved;
    if (!rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, first_instruction, 0,
                plan->first_left, &before_left) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, first_instruction, 0,
                plan->first_right, &before_right) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, first_instruction, 1,
                plan->first_left, &after_first_left) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, first_instruction, 1,
                plan->first_right, &after_first_right) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, second_instruction, 1,
                plan->first_left, &after_second_left) ||
        !rxas_flow_storage_at_instruction(
                service->ssa, expected_epoch, second_instruction, 1,
                plan->first_right, &after_second_right) ||
        !before_left.storage_id || !before_right.storage_id ||
        before_left.kind == RXAS_FLOW_STORAGE_UNKNOWN ||
        before_right.kind == RXAS_FLOW_STORAGE_UNKNOWN) {
        plan->reason = RXAS_FLOW_PROOF_STORAGE_UNKNOWN;
        goto complete;
    }
    plan->left_storage_id = before_left.storage_id;
    plan->right_storage_id = before_right.storage_id;
    if (after_first_left.storage_id != before_right.storage_id ||
        after_first_right.storage_id != before_left.storage_id ||
        after_second_left.storage_id != before_left.storage_id ||
        after_second_right.storage_id != before_right.storage_id) {
        plan->reason = RXAS_FLOW_PROOF_PERMUTATION_NOT_RESTORED;
        goto complete;
    }
    if (before_left.storage_id == before_right.storage_id)
        goto proved;
    for (record_id = first->record_id + 1;
         record_id < second->record_id; record_id++) {
        const RxasFlowRecord *record;
        const RxasFlowInstruction *instruction;
        if (!flow_proof_consume(service, 1)) {
            plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
            goto complete;
        }
        record = rxas_flow_procedure_record(
                service->procedure, expected_epoch, record_id);
        if (!record) {
            plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
            goto complete;
        }
        if (record->instruction_id == RXAS_FLOW_ID_NONE) continue;
        instruction = rxas_flow_procedure_instruction(
                service->procedure, expected_epoch,
                record->instruction_id);
        if (!instruction ||
            instruction->effects.state != RXOP_EFFECT_CLASSIFIED ||
            instruction->effects.flow != FLOW_NEXT ||
            instruction->effects.branch_targets != RXOP_OP_NONE ||
            instruction->effects.optimizer_barrier) {
            plan->reason = RXAS_FLOW_PROOF_PERMUTATION_OBSERVED;
            goto complete;
        }
        if (!flow_proof_signal_exits_discard_permutation(
                    service, expected_epoch, instruction)) {
            plan->reason = RXAS_FLOW_PROOF_PERMUTATION_SIGNAL_EXIT;
            goto complete;
        }
    }
    service->use = rxas_flow_require_use_analysis(
            service->procedure, expected_epoch, 0);
    if (!service->use) {
        plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        goto complete;
    }
    if (!flow_proof_mark_reachable_from_root(
                service, rxas_flow_procedure_async_root(
                        service->procedure, expected_epoch))) {
        plan->reason = service->metrics.status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED
                ? RXAS_FLOW_PROOF_BUDGET_EXHAUSTED
                : RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        goto complete;
    }
    use_count = rxas_flow_use_count(service->use, expected_epoch);
    for (use_index = 0; use_index < use_count; use_index++) {
        const RxasFlowUse *use;
        const RxasFlowRecord *use_record;
        int relevant;
        if (!flow_proof_consume(service, 1)) {
            plan->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
            goto complete;
        }
        use = rxas_flow_use(service->use, expected_epoch, use_index);
        if (!use) continue;
        relevant = use->kind == RXAS_FLOW_USE_OPAQUE_OBSERVATION ||
                   use->kind == RXAS_FLOW_USE_OPAQUE_WRITE ||
                   flow_proof_same_register(
                           use->register_id, plan->first_left) ||
                   flow_proof_same_register(
                           use->register_id, plan->first_right) ||
                   (use->kind == RXAS_FLOW_USE_CALL_WINDOW_READ &&
                    flow_proof_call_window_observes_permutation(
                            service, expected_epoch, use,
                            plan->first_left, plan->first_right));
        if (!relevant) continue;
        use_record = rxas_flow_procedure_record(
                service->procedure, expected_epoch, use->record_id);
        if (!use_record) {
            plan->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
            goto complete;
        }
        if ((use_record->block_id < service->block_count &&
             service->visit_marks[use_record->block_id] ==
                    service->visit_generation) ||
            (use->record_id > first->record_id &&
             use->record_id < second->record_id)) {
            plan->reason = use->kind == RXAS_FLOW_USE_CALL_WINDOW_READ
                    ? RXAS_FLOW_PROOF_CALL_WINDOW_OBSERVED
                    : RXAS_FLOW_PROOF_PERMUTATION_OBSERVED;
            goto complete;
        }
    }

proved:
    plan->proved = 1;
    plan->reason = RXAS_FLOW_PROOF_PROVED;

complete:
    if (plan->proved) service->metrics.storage_permutation_proved++;
    else service->metrics.storage_permutation_rejected++;
    return 1;
}

int rxas_flow_prove_instruction_speculatable(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t instruction_id, RxasFlowProofResult *result) {
    const RxasFlowInstruction *instruction;
    unsigned int forbidden;
    if (!result) return 0;
    flow_proof_result_init(result, RXAS_FLOW_PROOF_STALE_EPOCH);
    result->candidate_instruction = instruction_id;
    if (!flow_proof_query_available(service, expected_epoch, result)) return 1;
    if (!flow_proof_consume((RxasFlowProofService *)service, 1)) {
        result->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        return 1;
    }
    instruction = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, instruction_id);
    forbidden = RXOP_SEM_CALL | RXOP_SEM_DYNAMIC_CALL | RXOP_SEM_RETURN |
                RXOP_SEM_ALIAS_CREATE | RXOP_SEM_ALIAS_RELEASE |
                RXOP_SEM_REFERENCE_CREATE | RXOP_SEM_REFERENCE_READ |
                RXOP_SEM_REFERENCE_WRITE | RXOP_SEM_REFERENCE_RELEASE |
                RXOP_SEM_LIFETIME_END | RXOP_SEM_INDIRECT_WRITE |
                RXOP_SEM_INDIRECT_BRANCH | RXOP_SEM_OPAQUE;
    if (!instruction || !instruction->op ||
        instruction->effects.state != RXOP_EFFECT_CLASSIFIED ||
        instruction->signal.state != RXOP_SIGNAL_STATE_NONE ||
        instruction->effects.flow != FLOW_NEXT ||
        instruction->effects.optimizer_barrier ||
        (instruction->effects.semantics & forbidden) ||
        rxop_context_writes(instruction->op->opcode) != RXOP_CONTEXT_NONE ||
        (instruction->signal.properties & RXOP_SIGNAL_PROP_POLICY_WRITE)) {
        result->reason = RXAS_FLOW_PROOF_NOT_SPECULATABLE;
        return 1;
    }
    result->proved = 1;
    result->reason = RXAS_FLOW_PROOF_PROVED;
    return 1;
}

static int flow_proof_loop_contains(const RxasFlowProofService *service,
                                    const RxasFlowLoop *loop, size_t block) {
    size_t member;
    for (member = 0; member < loop->member_count; member++)
        if (rxas_flow_structural_loop_member(
                    service->structural, service->metrics.epoch,
                    loop->id, member) == block)
            return 1;
    return 0;
}

int rxas_flow_prove_must_execute_in_loop(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t instruction_id, size_t loop_id, RxasFlowProofResult *result) {
    const RxasFlowInstruction *instruction;
    const RxasFlowLoop *loop;
    size_t edge_id;
    int saw_latch;
    if (!result) return 0;
    flow_proof_result_init(result, RXAS_FLOW_PROOF_STALE_EPOCH);
    result->candidate_instruction = instruction_id;
    result->loop_id = loop_id;
    if (!flow_proof_query_available(service, expected_epoch, result)) return 1;
    if (!flow_proof_has_capabilities(
                service, FLOW_PROOF_LOOP_CAPABILITIES)) {
        result->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    if (!flow_proof_consume((RxasFlowProofService *)service,
                            loop_id == RXAS_FLOW_ID_NONE ? 1 :
                            service->edge_count + 1)) {
        result->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        return 1;
    }
    ((RxasFlowProofService *)service)->metrics.loop_queries++;
    instruction = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, instruction_id);
    loop = rxas_flow_structural_loop(
            service->structural, expected_epoch, loop_id);
    if (!instruction || !loop ||
        !flow_proof_loop_contains(service, loop, instruction->block_id)) {
        result->reason = RXAS_FLOW_PROOF_NOT_IN_LOOP;
        return 1;
    }
    if (!(loop->flags & RXAS_FLOW_LOOP_NATURAL) ||
        (loop->flags & RXAS_FLOW_LOOP_IRREDUCIBLE)) {
        result->reason = RXAS_FLOW_PROOF_IRREDUCIBLE_LOOP;
        return 1;
    }
    saw_latch = 0;
    for (edge_id = 0; edge_id < service->edge_count; edge_id++) {
        const RxasFlowEdge *edge;
        int source_inside;
        int target_inside;
        edge = rxas_flow_procedure_edge(
                service->procedure, expected_epoch, edge_id);
        source_inside = flow_proof_loop_contains(service, loop, edge->source);
        if (!source_inside) continue;
        target_inside = flow_proof_loop_contains(service, loop, edge->target);
        if (edge->target == loop->header &&
            rxas_flow_structural_edge_is_backedge(
                    service->structural, expected_epoch, edge_id))
            saw_latch = 1;
        if ((!target_inside || (edge->target == loop->header &&
             rxas_flow_structural_edge_is_backedge(
                    service->structural, expected_epoch, edge_id))) &&
            !rxas_flow_structural_dominates(
                    service->structural, expected_epoch,
                    instruction->block_id, edge->source)) {
            result->reason = RXAS_FLOW_PROOF_NOT_MUST_EXECUTE;
            return 1;
        }
    }
    if (!saw_latch) {
        result->reason = RXAS_FLOW_PROOF_NOT_MUST_EXECUTE;
        return 1;
    }
    result->proved = 1;
    result->reason = RXAS_FLOW_PROOF_PROVED;
    return 1;
}

static int flow_proof_same_component(const RxasFlowComponentFact *left,
                                     const RxasFlowComponentFact *right) {
    size_t effect;
    if (!left->storage_id || left->storage_id != right->storage_id ||
        left->value_id != right->value_id ||
        left->current_reference_effect != right->current_reference_effect)
        return 0;
    for (effect = 0; effect < RXAS_FLOW_EFFECT_CLASS_COUNT; effect++) {
        unsigned int dependency;
        dependency = flow_proof_dependency_for_effect(effect);
        if ((left->signal_dependencies & dependency) &&
            left->current_effects[effect] != right->current_effects[effect])
            return 0;
    }
    return 1;
}

int rxas_flow_prove_loop_component_invariant(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t instruction_id, size_t loop_id, RxasFlowRegister reg,
        unsigned int component, RxasFlowProofResult *result) {
    const RxasFlowInstruction *instruction;
    const RxasFlowLoop *loop;
    const RxasFlowBlock *header;
    RxasFlowComponentFact at_instruction;
    RxasFlowComponentFact at_header;
    size_t edge_id;
    int saw_latch;
    if (!result) return 0;
    flow_proof_result_init(result, RXAS_FLOW_PROOF_STALE_EPOCH);
    result->candidate_instruction = instruction_id;
    result->loop_id = loop_id;
    if (!flow_proof_query_available(service, expected_epoch, result)) return 1;
    if (!flow_proof_has_capabilities(
                service, FLOW_PROOF_LOOP_CAPABILITIES)) {
        result->reason = RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE;
        return 1;
    }
    if (!flow_proof_consume((RxasFlowProofService *)service,
                            loop_id == RXAS_FLOW_ID_NONE ? 1 :
                            service->edge_count + 2)) {
        result->reason = RXAS_FLOW_PROOF_BUDGET_EXHAUSTED;
        return 1;
    }
    ((RxasFlowProofService *)service)->metrics.loop_queries++;
    instruction = rxas_flow_procedure_instruction(
            service->procedure, expected_epoch, instruction_id);
    loop = rxas_flow_structural_loop(
            service->structural, expected_epoch, loop_id);
    header = loop ? rxas_flow_procedure_block(
            service->procedure, expected_epoch, loop->header) : 0;
    if (!instruction || !loop || !header ||
        header->first_instruction == RXAS_FLOW_ID_NONE ||
        !flow_proof_loop_contains(service, loop, instruction->block_id)) {
        result->reason = RXAS_FLOW_PROOF_NOT_IN_LOOP;
        return 1;
    }
    if (!(loop->flags & RXAS_FLOW_LOOP_NATURAL) ||
        (loop->flags & RXAS_FLOW_LOOP_IRREDUCIBLE)) {
        result->reason = RXAS_FLOW_PROOF_IRREDUCIBLE_LOOP;
        return 1;
    }
    if (!rxas_flow_component_at_instruction(
                service->ssa, expected_epoch, instruction_id, 0,
                reg, component, &at_instruction) ||
        !rxas_flow_component_at_instruction(
                service->ssa, expected_epoch, header->first_instruction, 0,
                reg, component, &at_header) ||
        !flow_proof_same_component(&at_instruction, &at_header)) {
        result->reason = RXAS_FLOW_PROOF_NOT_INVARIANT;
        return 1;
    }
    saw_latch = 0;
    for (edge_id = 0; edge_id < service->edge_count; edge_id++) {
        const RxasFlowEdge *edge;
        RxasFlowComponentFact on_edge;
        edge = rxas_flow_procedure_edge(
                service->procedure, expected_epoch, edge_id);
        if (edge->target != loop->header ||
            !rxas_flow_structural_edge_is_backedge(
                    service->structural, expected_epoch, edge_id))
            continue;
        saw_latch = 1;
        if (!rxas_flow_component_on_edge(
                    service->ssa, expected_epoch, edge_id,
                    reg, component, &on_edge) ||
            !flow_proof_same_component(&at_header, &on_edge)) {
            result->reason = RXAS_FLOW_PROOF_NOT_INVARIANT;
            return 1;
        }
    }
    if (!saw_latch) {
        result->reason = RXAS_FLOW_PROOF_NOT_INVARIANT;
        return 1;
    }
    result->proved = 1;
    result->reason = RXAS_FLOW_PROOF_PROVED;
    result->storage_id = at_header.storage_id;
    result->result_value_id = at_header.value_id;
    return 1;
}

const char *rxas_flow_proof_reason_name(RxasFlowProofReason reason) {
    switch (reason) {
        case RXAS_FLOW_PROOF_PROVED: return "proved";
        case RXAS_FLOW_PROOF_STALE_EPOCH: return "stale-epoch";
        case RXAS_FLOW_PROOF_INVALID_GRAPH: return "invalid-graph";
        case RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE:
            return "analysis-unavailable";
        case RXAS_FLOW_PROOF_BUDGET_EXHAUSTED: return "budget-exhausted";
        case RXAS_FLOW_PROOF_INVALID_INSTRUCTION: return "invalid-instruction";
        case RXAS_FLOW_PROOF_OPCODE_MISMATCH: return "opcode-mismatch";
        case RXAS_FLOW_PROOF_NOT_SUCCESS_STABLE: return "not-success-stable";
        case RXAS_FLOW_PROOF_NOT_DOMINATED: return "not-dominated";
        case RXAS_FLOW_PROOF_SUCCESS_EDGE_NOT_DOMINATING:
            return "success-edge-not-dominating";
        case RXAS_FLOW_PROOF_UNSUPPORTED_SIGNAL_DEPENDENCY:
            return "unsupported-signal-dependency";
        case RXAS_FLOW_PROOF_STORAGE_UNKNOWN: return "storage-unknown";
        case RXAS_FLOW_PROOF_STORAGE_CHANGED: return "storage-changed";
        case RXAS_FLOW_PROOF_SOURCE_UNKNOWN: return "source-unknown";
        case RXAS_FLOW_PROOF_GENERATOR_RESULT_UNKNOWN:
            return "generator-result-unknown";
        case RXAS_FLOW_PROOF_GENERATOR_SOURCE_UNKNOWN:
            return "generator-source-unknown";
        case RXAS_FLOW_PROOF_CANDIDATE_SOURCE_UNKNOWN:
            return "candidate-source-unknown";
        case RXAS_FLOW_PROOF_SOURCE_CHANGED: return "source-changed";
        case RXAS_FLOW_PROOF_RESULT_UNAVAILABLE: return "result-unavailable";
        case RXAS_FLOW_PROOF_EFFECT_CHANGED: return "effect-changed";
        case RXAS_FLOW_PROOF_REFERENCE_EFFECT_CHANGED:
            return "reference-effect-changed";
        case RXAS_FLOW_PROOF_NOT_EXACT_CONSTANT_WRITE:
            return "not-exact-constant-write";
        case RXAS_FLOW_PROOF_CONSTANT_UNKNOWN: return "constant-unknown";
        case RXAS_FLOW_PROOF_CONSTANT_CHANGED: return "constant-changed";
        case RXAS_FLOW_PROOF_CLEANUP_REQUIRED: return "cleanup-required";
        case RXAS_FLOW_PROOF_NOT_EXACT_ABSENT_WRITE:
            return "not-exact-absent-write";
        case RXAS_FLOW_PROOF_COMPONENT_PRESENT: return "component-present";
        case RXAS_FLOW_PROOF_ABSENCE_UNKNOWN: return "absence-unknown";
        case RXAS_FLOW_PROOF_NOT_EXACT_SELF_COPY:
            return "not-exact-self-copy";
        case RXAS_FLOW_PROOF_STORAGE_NOT_IDENTICAL:
            return "storage-not-identical";
        case RXAS_FLOW_PROOF_NOT_EXACT_TYPED_COPY:
            return "not-exact-typed-copy";
        case RXAS_FLOW_PROOF_DESTINATION_NOT_LOCAL:
            return "destination-not-local";
        case RXAS_FLOW_PROOF_DESTINATION_OBSERVABLE:
            return "destination-observable";
        case RXAS_FLOW_PROOF_USE_NOT_REDIRECTABLE:
            return "use-not-redirectable";
        case RXAS_FLOW_PROOF_SOURCE_NOT_EQUIVALENT:
            return "source-not-equivalent";
        case RXAS_FLOW_PROOF_CURSOR_OBSERVED:
            return "cursor-observed";
        case RXAS_FLOW_PROOF_CALL_WINDOW_OBSERVED:
            return "call-window-observed";
        case RXAS_FLOW_PROOF_NO_REDIRECTS: return "no-redirects";
        case RXAS_FLOW_PROOF_NOT_EXACT_PRODUCER_FORWARD:
            return "not-exact-producer-forward";
        case RXAS_FLOW_PROOF_NOT_ADJACENT_COPY:
            return "not-adjacent-copy";
        case RXAS_FLOW_PROOF_TEMPORARY_OBSERVABLE:
            return "temporary-observable";
        case RXAS_FLOW_PROOF_ADDRESS_OBSERVED:
            return "address-observed";
        case RXAS_FLOW_PROOF_NOT_EXACT_COMPARE_BRANCH_FUSION:
            return "not-exact-compare-branch-fusion";
        case RXAS_FLOW_PROOF_NOT_EXACT_DUPLICATE_LINKED_READ:
            return "not-exact-duplicate-linked-read";
        case RXAS_FLOW_PROOF_ATTRIBUTE_RANGE_UNKNOWN:
            return "attribute-range-unknown";
        case RXAS_FLOW_PROOF_ATTRIBUTE_PATH_CHANGED:
            return "attribute-path-changed";
        case RXAS_FLOW_PROOF_NOT_EXACT_STORAGE_PERMUTATION:
            return "not-exact-storage-permutation";
        case RXAS_FLOW_PROOF_PERMUTATION_NOT_RESTORED:
            return "permutation-not-restored";
        case RXAS_FLOW_PROOF_PERMUTATION_OBSERVED:
            return "permutation-observed";
        case RXAS_FLOW_PROOF_PERMUTATION_SIGNAL_EXIT:
            return "permutation-signal-exit";
        case RXAS_FLOW_PROOF_NOT_ADJACENT_BRANCH:
            return "not-adjacent-branch";
        case RXAS_FLOW_PROOF_COMPARE_RESULT_OBSERVED:
            return "compare-result-observed";
        case RXAS_FLOW_PROOF_TRACE_OBSERVED: return "trace-observed";
        case RXAS_FLOW_PROOF_NOT_IN_LOOP: return "not-in-loop";
        case RXAS_FLOW_PROOF_IRREDUCIBLE_LOOP: return "irreducible-loop";
        case RXAS_FLOW_PROOF_NOT_MUST_EXECUTE: return "not-must-execute";
        case RXAS_FLOW_PROOF_NOT_SPECULATABLE: return "not-speculatable";
        case RXAS_FLOW_PROOF_NOT_INVARIANT: return "not-invariant";
    }
    return "invalid";
}

static void flow_proof_dump_value(
        const RxasFlowProofService *service, FILE *stream,
        size_t query, const char *role, size_t value_id) {
    RxasFlowValueNode node;
    size_t effect;
    size_t input;
    if (value_id == RXAS_FLOW_ID_NONE ||
        !rxas_flow_value_node(
                service->ssa, service->metrics.epoch,
                value_id, &node))
        return;
    fprintf(stream,
            "PERF3 flow-proof-value query=%llu role=%s id=%llu kind=%d "
            "definition=%llu source=%llu derivation=%d dependencies=%u "
            "effects=",
            (unsigned long long)query, role,
            (unsigned long long)node.id, (int)node.kind,
            (unsigned long long)node.defining_instruction,
            (unsigned long long)node.source_value_id,
            (int)node.derivation, node.signal_dependencies);
    for (effect = 0; effect < RXAS_FLOW_EFFECT_CLASS_COUNT; effect++) {
        if (effect) fputc(',', stream);
        fprintf(stream, "%llu",
                (unsigned long long)node.definition_effects[effect]);
    }
    fputs(" inputs=", stream);
    for (input = 0; input < node.input_count; input++) {
        size_t input_id;
        input_id = rxas_flow_value_input(
                service->ssa, service->metrics.epoch,
                node.id, input);
        if (input) fputc(',', stream);
        fprintf(stream, "%llu", (unsigned long long)input_id);
    }
    fputc('\n', stream);
    for (input = 0; input < node.input_count; input++) {
        RxasFlowValueNode input_node;
        size_t nested;
        size_t input_id;
        input_id = rxas_flow_value_input(
                service->ssa, service->metrics.epoch,
                node.id, input);
        if (!rxas_flow_value_node(
                    service->ssa, service->metrics.epoch,
                    input_id, &input_node))
            continue;
        fprintf(stream,
                "PERF3 flow-proof-value-input query=%llu role=%s "
                "parent=%llu id=%llu kind=%d definition=%llu "
                "source=%llu derivation=%d dependencies=%u effects=",
                (unsigned long long)query, role,
                (unsigned long long)node.id,
                (unsigned long long)input_node.id, (int)input_node.kind,
                (unsigned long long)input_node.defining_instruction,
                (unsigned long long)input_node.source_value_id,
                (int)input_node.derivation,
                input_node.signal_dependencies);
        for (effect = 0; effect < RXAS_FLOW_EFFECT_CLASS_COUNT; effect++) {
            if (effect) fputc(',', stream);
            fprintf(stream, "%llu", (unsigned long long)
                    input_node.definition_effects[effect]);
        }
        fputs(" inputs=", stream);
        for (nested = 0; nested < input_node.input_count; nested++) {
            if (nested) fputc(',', stream);
            fprintf(stream, "%llu", (unsigned long long)
                    rxas_flow_value_input(
                            service->ssa, service->metrics.epoch,
                            input_node.id, nested));
        }
        fputc('\n', stream);
    }
}

int rxas_flow_proof_dump(const RxasFlowProofService *service,
                         unsigned long expected_epoch, FILE *stream) {
    const RxasFlowSsaMetrics *ssa_metrics;
    size_t index;
    if (!stream || !flow_proof_valid(service, expected_epoch)) return 0;
    ssa_metrics = rxas_flow_ssa_metrics(service->ssa, expected_epoch);
    fprintf(stream,
            "PERF3 flow-proof epoch=%lu status=available budget=%llu work=%llu "
            "bytes=%llu requested=0x%x acquired=0x%x "
            "repetition=%llu cache-hits=%llu proved=%llu "
            "rejected=%llu redundant-constant=%llu/%llu rejected=%llu "
            "redundant-absent=%llu/%llu rejected=%llu "
            "redundant-self-copy=%llu/%llu rejected=%llu "
            "typed-copy=%llu/%llu rejected=%llu rewrites=%llu "
            "producer-forward=%llu/%llu rejected=%llu "
            "duplicate-linked-read=%llu/%llu rejected=%llu "
            "storage-permutation=%llu/%llu rejected=%llu "
            "compare-branch=%llu/%llu rejected=%llu trace-deletions=%llu "
            "success-edge=%llu loop=%llu ssa-bytes=%llu ssa-values=%llu "
            "ssa-storages=%llu\n",
            service->metrics.epoch,
            (unsigned long long)service->metrics.budget_limit,
            (unsigned long long)service->metrics.work,
            (unsigned long long)service->metrics.retained_bytes,
            service->metrics.requested_capabilities,
            service->metrics.acquired_capabilities,
            (unsigned long long)service->metrics.repetition_queries,
            (unsigned long long)service->metrics.repetition_cache_hits,
            (unsigned long long)service->metrics.repetition_proved,
            (unsigned long long)service->metrics.repetition_rejected,
            (unsigned long long)service->metrics.redundant_constant_proved,
            (unsigned long long)service->metrics.redundant_constant_queries,
            (unsigned long long)service->metrics.redundant_constant_rejected,
            (unsigned long long)service->metrics.redundant_absent_proved,
            (unsigned long long)service->metrics.redundant_absent_queries,
            (unsigned long long)service->metrics.redundant_absent_rejected,
            (unsigned long long)service->metrics.redundant_self_copy_proved,
            (unsigned long long)service->metrics.redundant_self_copy_queries,
            (unsigned long long)service->metrics.redundant_self_copy_rejected,
            (unsigned long long)service->metrics.typed_copy_redirect_proved,
            (unsigned long long)service->metrics.typed_copy_redirect_queries,
            (unsigned long long)service->metrics.typed_copy_redirect_rejected,
            (unsigned long long)service->metrics.typed_copy_operand_rewrites,
            (unsigned long long)service->metrics.producer_forward_proved,
            (unsigned long long)service->metrics.producer_forward_queries,
            (unsigned long long)service->metrics.producer_forward_rejected,
            (unsigned long long)
                    service->metrics.duplicate_linked_read_proved,
            (unsigned long long)
                    service->metrics.duplicate_linked_read_queries,
            (unsigned long long)
                    service->metrics.duplicate_linked_read_rejected,
            (unsigned long long)
                    service->metrics.storage_permutation_proved,
            (unsigned long long)
                    service->metrics.storage_permutation_queries,
            (unsigned long long)
                    service->metrics.storage_permutation_rejected,
            (unsigned long long)service->metrics.compare_branch_proved,
            (unsigned long long)service->metrics.compare_branch_queries,
            (unsigned long long)service->metrics.compare_branch_rejected,
            (unsigned long long)
                    service->metrics.compare_branch_trace_deletions,
            (unsigned long long)service->metrics.success_edge_queries,
            (unsigned long long)service->metrics.loop_queries,
            (unsigned long long)(ssa_metrics
                    ? ssa_metrics->retained_bytes : 0),
            (unsigned long long)(ssa_metrics
                    ? ssa_metrics->value_versions : 0),
            (unsigned long long)(ssa_metrics
                    ? ssa_metrics->storage_versions : 0));
    for (index = 0; index < service->cache_count; index++) {
        fprintf(stream,
                "PERF3 flow-proof-repetition generator=%llu candidate=%llu "
                "proved=%d reason=%s storage=%llu source=%llu "
                "candidate-source=%llu result=%llu candidate-result=%llu "
                "effect-class=%llu generator-effect=%llu "
                "candidate-effect=%llu\n",
                (unsigned long long)service->cache[index].generator,
                (unsigned long long)service->cache[index].candidate,
                service->cache[index].result.proved,
                rxas_flow_proof_reason_name(service->cache[index].result.reason),
                (unsigned long long)service->cache[index].result.storage_id,
                (unsigned long long)service->cache[index].result.source_value_id,
                (unsigned long long)service->cache[index].result.candidate_source_value_id,
                (unsigned long long)service->cache[index].result.result_value_id,
                (unsigned long long)service->cache[index].result.candidate_result_value_id,
                (unsigned long long)service->cache[index].result.effect_class,
                (unsigned long long)service->cache[index].result.generator_effect_id,
                (unsigned long long)service->cache[index].result.candidate_effect_id);
        flow_proof_dump_value(
                service, stream, index, "source",
                service->cache[index].result.source_value_id);
        flow_proof_dump_value(
                service, stream, index, "candidate-source",
                service->cache[index].result.candidate_source_value_id);
        flow_proof_dump_value(
                service, stream, index, "result",
                service->cache[index].result.result_value_id);
        flow_proof_dump_value(
                service, stream, index, "candidate-result",
                service->cache[index].result.candidate_result_value_id);
    }
    return 1;
}
