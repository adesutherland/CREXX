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
};

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

static int flow_proof_valid(const RxasFlowProofService *service,
                            unsigned long epoch) {
    return service && epoch && service->metrics.epoch == epoch &&
           service->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE &&
           rxas_flow_procedure_epoch_matches(service->procedure, epoch);
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
                service->cache_capacity * sizeof(*service->cache);
    }
    service->cache[service->cache_count].generator =
            result->generator_instruction;
    service->cache[service->cache_count].candidate =
            result->candidate_instruction;
    service->cache[service->cache_count].result = *result;
    service->cache_count++;
    return 1;
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
            service->cache_capacity * sizeof(*service->cache);
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
            service->cache_capacity * sizeof(*service->cache);
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
            service->cache_capacity * sizeof(*service->cache);
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
 * static executions (for example, a retry edge around a repeated ITOS). Prove
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
    service->structural = rxas_flow_require_structural_analysis(
            procedure, epoch, 0);
    service->signal = rxas_flow_require_signal_analysis(procedure, epoch, 0);
    service->ssa = rxas_flow_require_ssa_analysis(procedure, epoch, 0);
    if (!service->structural || !service->signal || !service->ssa ||
        !flow_proof_build_adjacency(service)) {
        if (service->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE)
            service->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
        return service;
    }
    service->metrics.retained_bytes = sizeof(*service) +
            (service->block_count * 3 + service->edge_count + 1) *
                    sizeof(size_t) +
            service->value_capacity * 4 * sizeof(size_t) +
            service->storage_capacity * 2 * sizeof(size_t) +
            service->effect_capacity * 3 * sizeof(size_t) +
            service->cache_capacity * sizeof(*service->cache);
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
    free(service);
}

const RxasFlowProofService *rxas_flow_require_proof_service(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget) {
    struct RxasFlowAnalysisManager *manager;
    const RxasFlowMetrics *metrics;
    size_t requested;
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch)) return 0;
    metrics = rxas_flow_procedure_metrics(procedure, expected_epoch);
    if (!metrics) return 0;
    requested = work_budget ? work_budget : flow_proof_default_budget(metrics);
    manager = procedure->analysis_manager;
    if (manager && manager->epoch == expected_epoch && manager->proof) {
        if (manager->proof->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE)
            return manager->proof;
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
        manager->proof->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE)
        return 0;
    return manager->proof;
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
    if (!flow_proof_valid(service, expected_epoch)) return 0;
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
        (loop->flags & (RXAS_FLOW_LOOP_IRREDUCIBLE |
                        RXAS_FLOW_LOOP_SIGNAL_RETRY_ONLY))) {
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
        (loop->flags & (RXAS_FLOW_LOOP_IRREDUCIBLE |
                        RXAS_FLOW_LOOP_SIGNAL_RETRY_ONLY))) {
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
    size_t index;
    if (!stream || !flow_proof_valid(service, expected_epoch)) return 0;
    fprintf(stream,
            "PERF3 flow-proof epoch=%lu status=available budget=%llu work=%llu "
            "bytes=%llu repetition=%llu cache-hits=%llu proved=%llu "
            "rejected=%llu success-edge=%llu loop=%llu\n",
            service->metrics.epoch,
            (unsigned long long)service->metrics.budget_limit,
            (unsigned long long)service->metrics.work,
            (unsigned long long)service->metrics.retained_bytes,
            (unsigned long long)service->metrics.repetition_queries,
            (unsigned long long)service->metrics.repetition_cache_hits,
            (unsigned long long)service->metrics.repetition_proved,
            (unsigned long long)service->metrics.repetition_rejected,
            (unsigned long long)service->metrics.success_edge_queries,
            (unsigned long long)service->metrics.loop_queries);
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
