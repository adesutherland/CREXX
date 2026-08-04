/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

/* Cached structural analyses for an immutable RXAS procedure epoch. */

#include "rxas_flow_analysis.h"
#include "rxas_flow_graph_internal.h"
#include "rxas_flow_proof.h"
#include "rxas_flow_signal.h"
#include "rxas_flow_ssa.h"
#include "rxas_flow_use.h"

#include <stdlib.h>
#include <string.h>

typedef struct FlowIndexVector {
    size_t *values;
    size_t count;
    size_t capacity;
} FlowIndexVector;

typedef struct FlowLoopBuild {
    size_t header;
    size_t scc;
    unsigned int flags;
    FlowIndexVector latches;
    FlowIndexVector members;
} FlowLoopBuild;

struct RxasFlowStructuralAnalysis {
    const RxasFlowProcedure *procedure;
    RxasFlowStructuralMetrics metrics;
    size_t block_count;
    size_t edge_count;
    size_t virtual_root;
    size_t *successor_offsets;
    size_t *successor_edges;
    size_t *predecessor_offsets;
    size_t *predecessors;
    size_t *rpo;
    size_t *rpo_index;
    size_t *idom;
    size_t *dom_pre;
    size_t *dom_post;
    size_t *frontier_offsets;
    size_t *frontiers;
    size_t *scc;
    size_t *scc_sizes;
    size_t *scc_entries;
    unsigned char *scc_cyclic;
    unsigned char *scc_irreducible;
    unsigned char *edge_backedge;
    RxasFlowLoop *loops;
    size_t *loop_members;
    size_t *innermost_loop;
    int loops_ready;
};

static int flow_analysis_consume(RxasFlowStructuralAnalysis *analysis,
                                 size_t amount) {
    if (!analysis ||
        analysis->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE)
        return 0;
    if (amount > analysis->metrics.budget_limit -
                 (analysis->metrics.work <= analysis->metrics.budget_limit
                          ? analysis->metrics.work
                          : analysis->metrics.budget_limit)) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED;
        return 0;
    }
    analysis->metrics.work += amount;
    return 1;
}

static void *flow_analysis_calloc(RxasFlowStructuralAnalysis *analysis,
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

static int flow_vector_add(RxasFlowStructuralAnalysis *analysis,
                           FlowIndexVector *vector, size_t value) {
    size_t new_capacity;
    size_t *new_values;
    if (!flow_analysis_consume(analysis, 1)) return 0;
    if (vector->count == vector->capacity) {
        new_capacity = vector->capacity ? vector->capacity * 2 : 4;
        if (new_capacity < vector->capacity ||
            new_capacity > ((size_t)-1) / sizeof(*new_values)) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        new_values = realloc(vector->values,
                             new_capacity * sizeof(*new_values));
        if (!new_values) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        vector->values = new_values;
        vector->capacity = new_capacity;
    }
    vector->values[vector->count++] = value;
    return 1;
}

static void flow_vectors_free(FlowIndexVector *vectors, size_t count) {
    size_t index;
    if (!vectors) return;
    for (index = 0; index < count; index++) free(vectors[index].values);
    free(vectors);
}

static size_t flow_analysis_default_budget(size_t blocks, size_t edges) {
    size_t scale;
    if (blocks > (size_t)-1 - edges - 1) return (size_t)-1;
    scale = blocks + edges + 1;
    if (scale > ((size_t)-1 - 4096) / 256) return (size_t)-1;
    return scale * 256 + 4096;
}

static int flow_analysis_build_adjacency(
        RxasFlowStructuralAnalysis *analysis) {
    const RxasFlowMetrics *graph_metrics;
    size_t *successor_fill;
    size_t *predecessor_fill;
    size_t *target_seen;
    size_t predecessor_entries;
    size_t edge_index;
    size_t block;

    graph_metrics = rxas_flow_procedure_metrics(
            analysis->procedure, analysis->metrics.epoch);
    if (!graph_metrics) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
        return 0;
    }
    analysis->block_count = graph_metrics->blocks;
    analysis->edge_count = graph_metrics->edges;
    analysis->virtual_root = analysis->block_count;
    analysis->successor_offsets = flow_analysis_calloc(
            analysis, analysis->block_count + 1, sizeof(size_t));
    analysis->predecessor_offsets = flow_analysis_calloc(
            analysis, analysis->block_count + 1, sizeof(size_t));
    analysis->successor_edges = flow_analysis_calloc(
            analysis, analysis->edge_count, sizeof(size_t));
    analysis->predecessors = flow_analysis_calloc(
            analysis, analysis->edge_count, sizeof(size_t));
    successor_fill = flow_analysis_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    predecessor_fill = flow_analysis_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    target_seen = flow_analysis_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    if (!analysis->successor_offsets || !analysis->predecessor_offsets ||
        !analysis->successor_edges || !analysis->predecessors ||
        !successor_fill || !predecessor_fill || !target_seen) {
        free(successor_fill);
        free(predecessor_fill);
        free(target_seen);
        return 0;
    }
    for (edge_index = 0; edge_index < analysis->edge_count; edge_index++) {
        const RxasFlowEdge *edge;
        edge = rxas_flow_procedure_edge(analysis->procedure,
                                        analysis->metrics.epoch, edge_index);
        if (!edge || edge->source >= analysis->block_count ||
            edge->target >= analysis->block_count) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
            free(successor_fill);
            free(predecessor_fill);
            free(target_seen);
            return 0;
        }
        analysis->successor_offsets[edge->source + 1]++;
        if (!flow_analysis_consume(analysis, 1)) {
            free(successor_fill);
            free(predecessor_fill);
            free(target_seen);
            return 0;
        }
    }
    for (block = 1; block <= analysis->block_count; block++)
        analysis->successor_offsets[block] +=
                analysis->successor_offsets[block - 1];
    for (edge_index = 0; edge_index < analysis->edge_count; edge_index++) {
        const RxasFlowEdge *edge;
        edge = rxas_flow_procedure_edge(analysis->procedure,
                                        analysis->metrics.epoch, edge_index);
        analysis->successor_edges[
                analysis->successor_offsets[edge->source] +
                successor_fill[edge->source]++] = edge_index;
    }
    if (analysis->edge_count > ((size_t)-1) / 2 ||
        !flow_analysis_consume(analysis, analysis->edge_count * 2)) {
        free(successor_fill);
        free(predecessor_fill);
        free(target_seen);
        return 0;
    }
    /* Structural predecessors are block sets, not edge multisets. Parallel
     * normal/skip or handler edges retain their edge identity in the graph but
     * contribute one predecessor block to dominance and SCC algorithms. */
    for (block = 0; block < analysis->block_count; block++) {
        size_t successor_index;
        for (successor_index = analysis->successor_offsets[block];
             successor_index < analysis->successor_offsets[block + 1];
             successor_index++) {
            const RxasFlowEdge *edge;
            edge = rxas_flow_procedure_edge(
                    analysis->procedure, analysis->metrics.epoch,
                    analysis->successor_edges[successor_index]);
            if (target_seen[edge->target] != block + 1) {
                target_seen[edge->target] = block + 1;
                analysis->predecessor_offsets[edge->target + 1]++;
            }
        }
    }
    for (block = 1; block <= analysis->block_count; block++)
        analysis->predecessor_offsets[block] +=
                analysis->predecessor_offsets[block - 1];
    memset(target_seen, 0, analysis->block_count * sizeof(*target_seen));
    for (block = 0; block < analysis->block_count; block++) {
        size_t successor_index;
        for (successor_index = analysis->successor_offsets[block];
             successor_index < analysis->successor_offsets[block + 1];
             successor_index++) {
            const RxasFlowEdge *edge;
            edge = rxas_flow_procedure_edge(
                    analysis->procedure, analysis->metrics.epoch,
                    analysis->successor_edges[successor_index]);
            if (target_seen[edge->target] == block + 1) continue;
            target_seen[edge->target] = block + 1;
            analysis->predecessors[
                    analysis->predecessor_offsets[edge->target] +
                    predecessor_fill[edge->target]++] = block;
        }
    }
    predecessor_entries = analysis->predecessor_offsets[analysis->block_count];
    free(successor_fill);
    free(predecessor_fill);
    free(target_seen);
    analysis->metrics.predecessor_entries = predecessor_entries;
    return 1;
}

static size_t flow_analysis_root_count(
        const RxasFlowStructuralAnalysis *analysis, size_t *roots) {
    size_t candidates[3];
    size_t count;
    size_t index;
    size_t prior;
    candidates[0] = rxas_flow_procedure_entry_block(
            analysis->procedure, analysis->metrics.epoch);
    candidates[1] = rxas_flow_procedure_handler_root(
            analysis->procedure, analysis->metrics.epoch);
    candidates[2] = rxas_flow_procedure_async_root(
            analysis->procedure, analysis->metrics.epoch);
    count = 0;
    for (index = 0; index < 3; index++) {
        int duplicate;
        duplicate = 0;
        if (candidates[index] >= analysis->block_count) continue;
        for (prior = 0; prior < count; prior++) {
            if (roots[prior] == candidates[index]) duplicate = 1;
        }
        if (!duplicate) roots[count++] = candidates[index];
    }
    return count;
}

static int flow_analysis_is_root(const size_t *roots, size_t root_count,
                                 size_t block) {
    size_t index;
    for (index = 0; index < root_count; index++)
        if (roots[index] == block) return 1;
    return 0;
}

static int flow_analysis_build_rpo(RxasFlowStructuralAnalysis *analysis,
                                   const size_t *roots,
                                   size_t root_count) {
    unsigned char *seen;
    size_t *stack_nodes;
    size_t *stack_next;
    size_t *postorder;
    size_t stack_count;
    size_t post_count;
    size_t order_index;
    size_t reachable;

    seen = flow_analysis_calloc(analysis, analysis->block_count + 1, 1);
    stack_nodes = flow_analysis_calloc(
            analysis, analysis->block_count + 1, sizeof(size_t));
    stack_next = flow_analysis_calloc(
            analysis, analysis->block_count + 1, sizeof(size_t));
    postorder = flow_analysis_calloc(
            analysis, analysis->block_count + 1, sizeof(size_t));
    analysis->rpo = flow_analysis_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    analysis->rpo_index = flow_analysis_calloc(
            analysis, analysis->block_count + 1, sizeof(size_t));
    if (!seen || !stack_nodes || !stack_next || !postorder ||
        !analysis->rpo || !analysis->rpo_index) {
        free(seen);
        free(stack_nodes);
        free(stack_next);
        free(postorder);
        return 0;
    }
    for (order_index = 0; order_index <= analysis->block_count; order_index++)
        analysis->rpo_index[order_index] = RXAS_FLOW_ID_NONE;
    stack_count = 1;
    post_count = 0;
    stack_nodes[0] = analysis->virtual_root;
    stack_next[0] = 0;
    seen[analysis->virtual_root] = 1;
    while (stack_count) {
        size_t node;
        size_t next_index;
        size_t successor_count;
        size_t successor;
        node = stack_nodes[stack_count - 1];
        next_index = stack_next[stack_count - 1];
        successor_count = node == analysis->virtual_root
                ? root_count
                : analysis->successor_offsets[node + 1] -
                  analysis->successor_offsets[node];
        if (next_index < successor_count) {
            if (node == analysis->virtual_root) successor = roots[next_index];
            else {
                size_t edge_id;
                const RxasFlowEdge *edge;
                edge_id = analysis->successor_edges[
                        analysis->successor_offsets[node] + next_index];
                edge = rxas_flow_procedure_edge(
                        analysis->procedure, analysis->metrics.epoch, edge_id);
                successor = edge->target;
            }
            stack_next[stack_count - 1]++;
            if (!flow_analysis_consume(analysis, 1)) break;
            if (!seen[successor]) {
                seen[successor] = 1;
                stack_nodes[stack_count] = successor;
                stack_next[stack_count] = 0;
                stack_count++;
            }
        }
        else {
            postorder[post_count++] = node;
            stack_count--;
        }
    }
    if (analysis->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE) {
        free(seen);
        free(stack_nodes);
        free(stack_next);
        free(postorder);
        return 0;
    }
    reachable = 0;
    order_index = post_count;
    while (order_index) {
        size_t node;
        order_index--;
        node = postorder[order_index];
        if (node == analysis->virtual_root) {
            analysis->rpo_index[node] = 0;
            continue;
        }
        analysis->rpo[reachable] = node;
        analysis->rpo_index[node] = reachable + 1;
        reachable++;
    }
    analysis->metrics.reachable_blocks = reachable;
    analysis->metrics.unreachable_blocks = analysis->block_count - reachable;
    analysis->metrics.rpo_blocks = reachable;
    free(seen);
    free(stack_nodes);
    free(stack_next);
    free(postorder);
    return 1;
}

static size_t flow_analysis_intersect(
        RxasFlowStructuralAnalysis *analysis, size_t left, size_t right) {
    while (left != right) {
        while (analysis->rpo_index[left] > analysis->rpo_index[right]) {
            left = analysis->idom[left];
            if (!flow_analysis_consume(analysis, 1)) return RXAS_FLOW_ID_NONE;
        }
        while (analysis->rpo_index[right] > analysis->rpo_index[left]) {
            right = analysis->idom[right];
            if (!flow_analysis_consume(analysis, 1)) return RXAS_FLOW_ID_NONE;
        }
    }
    return left;
}

static int flow_analysis_build_dominators(
        RxasFlowStructuralAnalysis *analysis, const size_t *roots,
        size_t root_count) {
    size_t index;
    int changed;
    analysis->idom = flow_analysis_calloc(
            analysis, analysis->block_count + 1, sizeof(size_t));
    if (!analysis->idom) return 0;
    for (index = 0; index <= analysis->block_count; index++)
        analysis->idom[index] = RXAS_FLOW_ID_NONE;
    analysis->idom[analysis->virtual_root] = analysis->virtual_root;
    do {
        changed = 0;
        analysis->metrics.dominator_iterations++;
        for (index = 0; index < analysis->metrics.rpo_blocks; index++) {
            size_t block;
            size_t candidate;
            size_t predecessor_index;
            block = analysis->rpo[index];
            candidate = flow_analysis_is_root(roots, root_count, block)
                    ? analysis->virtual_root : RXAS_FLOW_ID_NONE;
            for (predecessor_index = analysis->predecessor_offsets[block];
                 predecessor_index < analysis->predecessor_offsets[block + 1];
                 predecessor_index++) {
                size_t predecessor;
                if (!flow_analysis_consume(analysis, 1)) return 0;
                predecessor = analysis->predecessors[predecessor_index];
                if (analysis->rpo_index[predecessor] == RXAS_FLOW_ID_NONE ||
                    analysis->idom[predecessor] == RXAS_FLOW_ID_NONE)
                    continue;
                if (candidate == RXAS_FLOW_ID_NONE) candidate = predecessor;
                else candidate = flow_analysis_intersect(
                        analysis, candidate, predecessor);
                if (analysis->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE)
                    return 0;
            }
            if (candidate == RXAS_FLOW_ID_NONE) {
                analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
                return 0;
            }
            if (analysis->idom[block] != candidate) {
                analysis->idom[block] = candidate;
                changed = 1;
            }
            if (!flow_analysis_consume(analysis, 1)) return 0;
        }
    } while (changed);
    return 1;
}

static int flow_analysis_build_dom_intervals(
        RxasFlowStructuralAnalysis *analysis) {
    size_t node_count;
    size_t *child_offsets;
    size_t *children;
    size_t *fill;
    size_t *stack_nodes;
    size_t *stack_next;
    size_t index;
    size_t stack_count;
    size_t timestamp;

    node_count = analysis->block_count + 1;
    child_offsets = flow_analysis_calloc(analysis, node_count + 1,
                                         sizeof(size_t));
    children = flow_analysis_calloc(analysis, analysis->metrics.rpo_blocks,
                                    sizeof(size_t));
    fill = flow_analysis_calloc(analysis, node_count, sizeof(size_t));
    stack_nodes = flow_analysis_calloc(analysis, node_count, sizeof(size_t));
    stack_next = flow_analysis_calloc(analysis, node_count, sizeof(size_t));
    analysis->dom_pre = flow_analysis_calloc(analysis, node_count,
                                              sizeof(size_t));
    analysis->dom_post = flow_analysis_calloc(analysis, node_count,
                                               sizeof(size_t));
    if (!child_offsets || !children || !fill || !stack_nodes || !stack_next ||
        !analysis->dom_pre || !analysis->dom_post) {
        free(child_offsets);
        free(children);
        free(fill);
        free(stack_nodes);
        free(stack_next);
        return 0;
    }
    for (index = 0; index < analysis->metrics.rpo_blocks; index++)
        child_offsets[analysis->idom[analysis->rpo[index]] + 1]++;
    for (index = 1; index <= node_count; index++)
        child_offsets[index] += child_offsets[index - 1];
    for (index = 0; index < analysis->metrics.rpo_blocks; index++) {
        size_t block;
        size_t parent;
        block = analysis->rpo[index];
        parent = analysis->idom[block];
        children[child_offsets[parent] + fill[parent]++] = block;
    }
    stack_count = 1;
    timestamp = 1;
    stack_nodes[0] = analysis->virtual_root;
    stack_next[0] = 0;
    analysis->dom_pre[analysis->virtual_root] = timestamp++;
    while (stack_count) {
        size_t node;
        size_t child_count;
        node = stack_nodes[stack_count - 1];
        child_count = child_offsets[node + 1] - child_offsets[node];
        if (stack_next[stack_count - 1] < child_count) {
            size_t child;
            child = children[child_offsets[node] +
                             stack_next[stack_count - 1]++];
            analysis->dom_pre[child] = timestamp++;
            stack_nodes[stack_count] = child;
            stack_next[stack_count] = 0;
            stack_count++;
        }
        else {
            analysis->dom_post[node] = timestamp++;
            stack_count--;
        }
        if (!flow_analysis_consume(analysis, 1)) break;
    }
    free(child_offsets);
    free(children);
    free(fill);
    free(stack_nodes);
    free(stack_next);
    return analysis->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE;
}

static int flow_analysis_dominates_raw(
        const RxasFlowStructuralAnalysis *analysis, size_t dominator,
        size_t block) {
    if (!analysis || dominator > analysis->block_count ||
        block > analysis->block_count ||
        analysis->rpo_index[dominator] == RXAS_FLOW_ID_NONE ||
        analysis->rpo_index[block] == RXAS_FLOW_ID_NONE)
        return 0;
    return analysis->dom_pre[dominator] <= analysis->dom_pre[block] &&
           analysis->dom_post[block] <= analysis->dom_post[dominator];
}

static int flow_analysis_build_frontiers(
        RxasFlowStructuralAnalysis *analysis, const size_t *roots,
        size_t root_count) {
    FlowIndexVector *frontiers;
    size_t *last_added;
    size_t block_index;
    size_t total;
    frontiers = flow_analysis_calloc(analysis, analysis->block_count,
                                     sizeof(*frontiers));
    last_added = flow_analysis_calloc(analysis, analysis->block_count,
                                      sizeof(size_t));
    if (!frontiers || !last_added) {
        flow_vectors_free(frontiers, analysis->block_count);
        free(last_added);
        return 0;
    }
    for (block_index = 0; block_index < analysis->block_count; block_index++)
        last_added[block_index] = RXAS_FLOW_ID_NONE;
    for (block_index = 0; block_index < analysis->metrics.rpo_blocks;
         block_index++) {
        size_t block;
        size_t predecessor_count;
        size_t predecessor_index;
        block = analysis->rpo[block_index];
        predecessor_count = analysis->predecessor_offsets[block + 1] -
                            analysis->predecessor_offsets[block];
        if (flow_analysis_is_root(roots, root_count, block))
            predecessor_count++;
        if (predecessor_count < 2) continue;
        for (predecessor_index = analysis->predecessor_offsets[block];
             predecessor_index < analysis->predecessor_offsets[block + 1];
             predecessor_index++) {
            size_t runner;
            runner = analysis->predecessors[predecessor_index];
            if (analysis->rpo_index[runner] == RXAS_FLOW_ID_NONE) continue;
            while (runner != analysis->idom[block] &&
                   runner != analysis->virtual_root) {
                if (last_added[runner] != block) {
                    if (!flow_vector_add(analysis, &frontiers[runner], block)) {
                        flow_vectors_free(frontiers, analysis->block_count);
                        free(last_added);
                        return 0;
                    }
                    last_added[runner] = block;
                }
                runner = analysis->idom[runner];
                if (!flow_analysis_consume(analysis, 1)) {
                    flow_vectors_free(frontiers, analysis->block_count);
                    free(last_added);
                    return 0;
                }
            }
        }
    }
    total = 0;
    for (block_index = 0; block_index < analysis->block_count; block_index++)
        total += frontiers[block_index].count;
    analysis->frontier_offsets = flow_analysis_calloc(
            analysis, analysis->block_count + 1, sizeof(size_t));
    analysis->frontiers = flow_analysis_calloc(analysis, total, sizeof(size_t));
    if (!analysis->frontier_offsets || !analysis->frontiers) {
        flow_vectors_free(frontiers, analysis->block_count);
        free(last_added);
        return 0;
    }
    total = 0;
    for (block_index = 0; block_index < analysis->block_count; block_index++) {
        size_t item;
        analysis->frontier_offsets[block_index] = total;
        for (item = 0; item < frontiers[block_index].count; item++)
            analysis->frontiers[total++] = frontiers[block_index].values[item];
    }
    analysis->frontier_offsets[analysis->block_count] = total;
    analysis->metrics.dominance_frontier_entries = total;
    flow_vectors_free(frontiers, analysis->block_count);
    free(last_added);
    return 1;
}

static int flow_analysis_build_sccs(
        RxasFlowStructuralAnalysis *analysis, const size_t *roots,
        size_t root_count) {
    size_t *stack;
    unsigned char *entry_block;
    size_t *scc_entries;
    size_t rpo_index;
    size_t scc_count;
    size_t edge_index;
    size_t block;

    analysis->scc = flow_analysis_calloc(analysis, analysis->block_count,
                                         sizeof(size_t));
    analysis->scc_sizes = flow_analysis_calloc(analysis, analysis->block_count,
                                               sizeof(size_t));
    analysis->scc_entries = flow_analysis_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    analysis->scc_cyclic = flow_analysis_calloc(analysis,
                                                 analysis->block_count, 1);
    analysis->scc_irreducible = flow_analysis_calloc(
            analysis, analysis->block_count, 1);
    stack = flow_analysis_calloc(analysis, analysis->block_count,
                                 sizeof(size_t));
    entry_block = flow_analysis_calloc(analysis, analysis->block_count, 1);
    scc_entries = analysis->scc_entries;
    if (!analysis->scc || !analysis->scc_sizes || !analysis->scc_entries ||
        !analysis->scc_cyclic || !analysis->scc_irreducible || !stack ||
        !entry_block) {
        free(stack);
        free(entry_block);
        return 0;
    }
    for (block = 0; block < analysis->block_count; block++)
        analysis->scc[block] = RXAS_FLOW_ID_NONE;
    scc_count = 0;
    for (rpo_index = 0; rpo_index < analysis->metrics.rpo_blocks;
         rpo_index++) {
        size_t start;
        size_t stack_count;
        start = analysis->rpo[rpo_index];
        if (analysis->scc[start] != RXAS_FLOW_ID_NONE) continue;
        stack_count = 1;
        stack[0] = start;
        analysis->scc[start] = scc_count;
        while (stack_count) {
            size_t node;
            size_t predecessor_index;
            node = stack[--stack_count];
            analysis->scc_sizes[scc_count]++;
            for (predecessor_index = analysis->predecessor_offsets[node];
                 predecessor_index < analysis->predecessor_offsets[node + 1];
                 predecessor_index++) {
                size_t predecessor;
                if (!flow_analysis_consume(analysis, 1)) {
                    free(stack);
                    free(entry_block);
                    return 0;
                }
                predecessor = analysis->predecessors[predecessor_index];
                if (analysis->rpo_index[predecessor] == RXAS_FLOW_ID_NONE ||
                    analysis->scc[predecessor] != RXAS_FLOW_ID_NONE)
                    continue;
                analysis->scc[predecessor] = scc_count;
                stack[stack_count++] = predecessor;
            }
            if (!flow_analysis_consume(analysis, 1)) {
                free(stack);
                free(entry_block);
                return 0;
            }
        }
        scc_count++;
    }
    for (block = 0; block < scc_count; block++) {
        if (analysis->scc_sizes[block] > 1) analysis->scc_cyclic[block] = 1;
    }
    for (edge_index = 0; edge_index < analysis->edge_count; edge_index++) {
        const RxasFlowEdge *edge;
        size_t source_scc;
        size_t target_scc;
        edge = rxas_flow_procedure_edge(analysis->procedure,
                                        analysis->metrics.epoch, edge_index);
        if (analysis->rpo_index[edge->source] == RXAS_FLOW_ID_NONE ||
            analysis->rpo_index[edge->target] == RXAS_FLOW_ID_NONE)
            continue;
        source_scc = analysis->scc[edge->source];
        target_scc = analysis->scc[edge->target];
        if (source_scc == target_scc && edge->source == edge->target)
            analysis->scc_cyclic[source_scc] = 1;
        if (source_scc != target_scc && !entry_block[edge->target]) {
            entry_block[edge->target] = 1;
            scc_entries[target_scc]++;
        }
    }
    for (block = 0; block < root_count; block++) {
        size_t root;
        root = roots[block];
        if (!entry_block[root]) {
            entry_block[root] = 1;
            scc_entries[analysis->scc[root]]++;
        }
    }
    for (block = 0; block < scc_count; block++) {
        if (analysis->scc_cyclic[block]) {
            analysis->metrics.cyclic_scc_count++;
            if (scc_entries[block] > 1) {
                analysis->scc_irreducible[block] = 1;
                analysis->metrics.irreducible_scc_count++;
            }
        }
    }
    analysis->metrics.scc_count = scc_count;
    free(stack);
    free(entry_block);
    return 1;
}

static int flow_analysis_build_backedges(
        RxasFlowStructuralAnalysis *analysis) {
    size_t edge_index;
    analysis->edge_backedge = flow_analysis_calloc(
            analysis, analysis->edge_count, 1);
    if (!analysis->edge_backedge) return 0;
    for (edge_index = 0; edge_index < analysis->edge_count; edge_index++) {
        const RxasFlowEdge *edge;
        edge = rxas_flow_procedure_edge(analysis->procedure,
                                        analysis->metrics.epoch, edge_index);
        if (analysis->rpo_index[edge->source] != RXAS_FLOW_ID_NONE &&
            analysis->rpo_index[edge->target] != RXAS_FLOW_ID_NONE &&
            flow_analysis_dominates_raw(analysis, edge->target,
                                        edge->source)) {
            analysis->edge_backedge[edge_index] = 1;
            analysis->metrics.backedges++;
        }
        if (!flow_analysis_consume(analysis, 1)) return 0;
    }
    return 1;
}

static void flow_loop_build_free(FlowLoopBuild *loops, size_t loop_count) {
    size_t index;
    if (!loops) return;
    for (index = 0; index < loop_count; index++) {
        free(loops[index].latches.values);
        free(loops[index].members.values);
    }
    free(loops);
}

static int flow_analysis_add_loop(RxasFlowStructuralAnalysis *analysis,
                                  FlowLoopBuild **loops,
                                  size_t *loop_count,
                                  size_t *loop_capacity,
                                  size_t header, size_t scc,
                                  unsigned int flags) {
    FlowLoopBuild *new_loops;
    size_t new_capacity;
    if (*loop_count == *loop_capacity) {
        new_capacity = *loop_capacity ? *loop_capacity * 2 : 16;
        if (new_capacity < *loop_capacity ||
            new_capacity > ((size_t)-1) / sizeof(**loops)) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        new_loops = realloc(*loops, new_capacity * sizeof(**loops));
        if (!new_loops) {
            analysis->metrics.status = RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY;
            return 0;
        }
        memset(new_loops + *loop_capacity, 0,
               (new_capacity - *loop_capacity) * sizeof(**loops));
        *loops = new_loops;
        *loop_capacity = new_capacity;
    }
    (*loops)[*loop_count].header = header;
    (*loops)[*loop_count].scc = scc;
    (*loops)[*loop_count].flags = flags;
    (*loop_count)++;
    return 1;
}

static int flow_analysis_build_natural_members(
        RxasFlowStructuralAnalysis *analysis, FlowLoopBuild *loop,
        size_t *seen, size_t generation, size_t *stack) {
    size_t stack_count;
    size_t latch_index;
    seen[loop->header] = generation;
    if (!flow_vector_add(analysis, &loop->members, loop->header)) return 0;
    stack_count = 0;
    for (latch_index = 0; latch_index < loop->latches.count; latch_index++) {
        size_t latch;
        latch = loop->latches.values[latch_index];
        if (seen[latch] != generation) {
            seen[latch] = generation;
            if (!flow_vector_add(analysis, &loop->members, latch)) return 0;
            stack[stack_count++] = latch;
        }
    }
    while (stack_count) {
        size_t node;
        size_t predecessor_index;
        node = stack[--stack_count];
        for (predecessor_index = analysis->predecessor_offsets[node];
             predecessor_index < analysis->predecessor_offsets[node + 1];
             predecessor_index++) {
            size_t predecessor;
            if (!flow_analysis_consume(analysis, 1)) return 0;
            predecessor = analysis->predecessors[predecessor_index];
            if (analysis->rpo_index[predecessor] == RXAS_FLOW_ID_NONE ||
                seen[predecessor] == generation ||
                !flow_analysis_dominates_raw(analysis, loop->header,
                                             predecessor))
                continue;
            seen[predecessor] = generation;
            if (!flow_vector_add(analysis, &loop->members, predecessor))
                return 0;
            stack[stack_count++] = predecessor;
        }
        if (!flow_analysis_consume(analysis, 1)) return 0;
    }
    return 1;
}

static int flow_analysis_build_loops(RxasFlowStructuralAnalysis *analysis) {
    FlowLoopBuild *loop_build;
    FlowIndexVector *block_loops;
    FlowIndexVector *scc_members;
    size_t loop_count;
    size_t loop_capacity;
    size_t *header_loop;
    size_t *seen;
    size_t *stack;
    size_t edge_index;
    size_t loop_index;
    size_t block;
    size_t total_members;
    size_t *depth_stack;

    loop_build = 0;
    loop_count = 0;
    loop_capacity = 0;
    header_loop = flow_analysis_calloc(analysis, analysis->block_count,
                                       sizeof(size_t));
    seen = flow_analysis_calloc(analysis, analysis->block_count,
                                sizeof(size_t));
    stack = flow_analysis_calloc(analysis, analysis->block_count,
                                 sizeof(size_t));
    if (!header_loop || !seen || !stack) {
        free(header_loop);
        free(seen);
        free(stack);
        return 0;
    }
    for (block = 0; block < analysis->block_count; block++)
        header_loop[block] = RXAS_FLOW_ID_NONE;
    for (edge_index = 0; edge_index < analysis->edge_count; edge_index++) {
        const RxasFlowEdge *edge;
        size_t current_loop;
        if (!analysis->edge_backedge[edge_index]) continue;
        edge = rxas_flow_procedure_edge(analysis->procedure,
                                        analysis->metrics.epoch, edge_index);
        current_loop = header_loop[edge->target];
        if (current_loop == RXAS_FLOW_ID_NONE) {
            if (!flow_analysis_add_loop(
                    analysis, &loop_build, &loop_count, &loop_capacity,
                    edge->target, analysis->scc[edge->target],
                    RXAS_FLOW_LOOP_NATURAL)) {
                flow_loop_build_free(loop_build, loop_count);
                free(header_loop);
                free(seen);
                free(stack);
                return 0;
            }
            current_loop = loop_count - 1;
            header_loop[edge->target] = current_loop;
        }
        if (!flow_vector_add(analysis,
                             &loop_build[current_loop].latches,
                             edge->source)) {
            flow_loop_build_free(loop_build, loop_count);
            free(header_loop);
            free(seen);
            free(stack);
            return 0;
        }
    }
    for (loop_index = 0; loop_index < loop_count; loop_index++) {
        if (!flow_analysis_build_natural_members(
                analysis, &loop_build[loop_index], seen, loop_index + 1,
                stack)) {
            flow_loop_build_free(loop_build, loop_count);
            free(header_loop);
            free(seen);
            free(stack);
            return 0;
        }
    }
    scc_members = flow_analysis_calloc(
            analysis, analysis->metrics.scc_count, sizeof(*scc_members));
    if (!scc_members) {
        flow_loop_build_free(loop_build, loop_count);
        free(header_loop);
        free(seen);
        free(stack);
        return 0;
    }
    for (loop_index = 0; loop_index < analysis->metrics.rpo_blocks;
         loop_index++) {
        size_t member;
        member = analysis->rpo[loop_index];
        if (!flow_vector_add(analysis, &scc_members[analysis->scc[member]],
                             member)) {
            flow_vectors_free(scc_members, analysis->metrics.scc_count);
            flow_loop_build_free(loop_build, loop_count);
            free(header_loop);
            free(seen);
            free(stack);
            return 0;
        }
    }
    for (block = 0; block < analysis->metrics.scc_count; block++) {
        size_t representative;
        if (!analysis->scc_irreducible[block]) continue;
        if (!scc_members[block].count) continue;
        representative = scc_members[block].values[0];
        if (!flow_analysis_add_loop(
                analysis, &loop_build, &loop_count, &loop_capacity,
                representative, block, RXAS_FLOW_LOOP_IRREDUCIBLE)) {
            flow_loop_build_free(loop_build, loop_count);
            free(header_loop);
            free(seen);
            free(stack);
            flow_vectors_free(scc_members, analysis->metrics.scc_count);
            return 0;
        }
        for (loop_index = 0; loop_index < scc_members[block].count;
             loop_index++) {
            size_t member;
            member = scc_members[block].values[loop_index];
            if (!flow_vector_add(analysis,
                                 &loop_build[loop_count - 1].members,
                                 member)) {
                flow_loop_build_free(loop_build, loop_count);
                free(header_loop);
                free(seen);
                free(stack);
                flow_vectors_free(scc_members,
                                  analysis->metrics.scc_count);
                return 0;
            }
        }
    }
    flow_vectors_free(scc_members, analysis->metrics.scc_count);
    free(header_loop);
    free(seen);
    free(stack);

    block_loops = flow_analysis_calloc(analysis, analysis->block_count,
                                       sizeof(*block_loops));
    if (!block_loops) {
        flow_loop_build_free(loop_build, loop_count);
        return 0;
    }
    total_members = 0;
    for (loop_index = 0; loop_index < loop_count; loop_index++) {
        size_t member_index;
        total_members += loop_build[loop_index].members.count;
        for (member_index = 0;
             member_index < loop_build[loop_index].members.count;
             member_index++) {
            size_t member;
            member = loop_build[loop_index].members.values[member_index];
            if (!flow_vector_add(analysis, &block_loops[member], loop_index)) {
                flow_vectors_free(block_loops, analysis->block_count);
                flow_loop_build_free(loop_build, loop_count);
                return 0;
            }
        }
    }
    analysis->loops = flow_analysis_calloc(analysis, loop_count,
                                            sizeof(*analysis->loops));
    analysis->loop_members = flow_analysis_calloc(
            analysis, total_members, sizeof(size_t));
    analysis->innermost_loop = flow_analysis_calloc(
            analysis, analysis->block_count, sizeof(size_t));
    depth_stack = flow_analysis_calloc(analysis, loop_count, sizeof(size_t));
    if (!analysis->loops || !analysis->loop_members ||
        !analysis->innermost_loop || !depth_stack) {
        free(depth_stack);
        flow_vectors_free(block_loops, analysis->block_count);
        flow_loop_build_free(loop_build, loop_count);
        return 0;
    }
    for (block = 0; block < analysis->block_count; block++)
        analysis->innermost_loop[block] = RXAS_FLOW_ID_NONE;
    total_members = 0;
    for (loop_index = 0; loop_index < loop_count; loop_index++) {
        RxasFlowLoop *loop;
        size_t member_index;
        size_t candidate_index;
        size_t parent;
        loop = &analysis->loops[loop_index];
        loop->id = loop_index;
        loop->header = loop_build[loop_index].header;
        loop->parent = RXAS_FLOW_ID_NONE;
        loop->member_offset = total_members;
        loop->member_count = loop_build[loop_index].members.count;
        loop->latch_count = loop_build[loop_index].latches.count;
        loop->flags = loop_build[loop_index].flags;
        for (member_index = 0; member_index < loop->member_count;
             member_index++)
            analysis->loop_members[total_members++] =
                    loop_build[loop_index].members.values[member_index];
        parent = RXAS_FLOW_ID_NONE;
        for (candidate_index = 0;
             candidate_index < block_loops[loop->header].count;
             candidate_index++) {
            size_t candidate;
            candidate = block_loops[loop->header].values[candidate_index];
            if (candidate == loop_index ||
                loop_build[candidate].members.count <= loop->member_count)
                continue;
            if (parent == RXAS_FLOW_ID_NONE ||
                loop_build[candidate].members.count <
                    loop_build[parent].members.count)
                parent = candidate;
        }
        loop->parent = parent;
    }
    for (loop_index = 0; loop_index < loop_count; loop_index++) {
        size_t cursor;
        size_t path_count;
        size_t depth;
        if (analysis->loops[loop_index].depth) continue;
        cursor = loop_index;
        path_count = 0;
        while (cursor != RXAS_FLOW_ID_NONE &&
               !analysis->loops[cursor].depth) {
            if (path_count >= loop_count) {
                analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
                free(depth_stack);
                flow_vectors_free(block_loops, analysis->block_count);
                flow_loop_build_free(loop_build, loop_count);
                return 0;
            }
            depth_stack[path_count++] = cursor;
            cursor = analysis->loops[cursor].parent;
        }
        depth = cursor == RXAS_FLOW_ID_NONE
                ? 0 : analysis->loops[cursor].depth;
        while (path_count) {
            size_t member_loop;
            member_loop = depth_stack[--path_count];
            analysis->loops[member_loop].depth = ++depth;
            if (depth > analysis->metrics.max_loop_depth)
                analysis->metrics.max_loop_depth = depth;
        }
    }
    for (block = 0; block < analysis->block_count; block++) {
        size_t candidate_index;
        size_t selected;
        selected = RXAS_FLOW_ID_NONE;
        for (candidate_index = 0;
             candidate_index < block_loops[block].count;
             candidate_index++) {
            size_t candidate;
            candidate = block_loops[block].values[candidate_index];
            if (selected == RXAS_FLOW_ID_NONE ||
                analysis->loops[candidate].member_count <
                    analysis->loops[selected].member_count)
                selected = candidate;
        }
        analysis->innermost_loop[block] = selected;
    }
    analysis->metrics.loops = loop_count;
    analysis->metrics.loop_memberships = total_members;
    free(depth_stack);
    flow_vectors_free(block_loops, analysis->block_count);
    flow_loop_build_free(loop_build, loop_count);
    return 1;
}

static void flow_structural_free(RxasFlowStructuralAnalysis *analysis) {
    if (!analysis) return;
    free(analysis->successor_offsets);
    free(analysis->successor_edges);
    free(analysis->predecessor_offsets);
    free(analysis->predecessors);
    free(analysis->rpo);
    free(analysis->rpo_index);
    free(analysis->idom);
    free(analysis->dom_pre);
    free(analysis->dom_post);
    free(analysis->frontier_offsets);
    free(analysis->frontiers);
    free(analysis->scc);
    free(analysis->scc_sizes);
    free(analysis->scc_entries);
    free(analysis->scc_cyclic);
    free(analysis->scc_irreducible);
    free(analysis->edge_backedge);
    free(analysis->loops);
    free(analysis->loop_members);
    free(analysis->innermost_loop);
    free(analysis);
}

static void flow_analysis_set_retained_bytes(
        RxasFlowStructuralAnalysis *analysis) {
    size_t blocks;
    size_t edges;
    size_t retained;
    blocks = analysis->block_count;
    edges = analysis->edge_count;
    retained = sizeof(*analysis) + sizeof(struct RxasFlowAnalysisManager) +
            (blocks + 1) * sizeof(size_t) * 3 +
            (edges + analysis->metrics.predecessor_entries +
             analysis->metrics.rpo_blocks + blocks * 4 +
             analysis->metrics.dominance_frontier_entries) * sizeof(size_t);
    if (analysis->loops_ready)
        retained += (blocks * 4 + analysis->metrics.loop_memberships) *
                            sizeof(size_t) +
                    blocks * 2 + edges +
                    analysis->metrics.loops * sizeof(RxasFlowLoop);
    analysis->metrics.retained_bytes = retained;
}

static RxasFlowStructuralAnalysis *flow_structural_build(
        RxasFlowProcedure *procedure, unsigned long epoch,
        size_t work_budget) {
    RxasFlowStructuralAnalysis *analysis;
    size_t roots[3];
    size_t root_count;
    const RxasFlowMetrics *graph_metrics;
    graph_metrics = rxas_flow_procedure_metrics(procedure, epoch);
    if (!graph_metrics) return 0;
    analysis = calloc(1, sizeof(*analysis));
    if (!analysis) return 0;
    analysis->procedure = procedure;
    analysis->metrics.status = RXAS_FLOW_ANALYSIS_AVAILABLE;
    analysis->metrics.epoch = epoch;
    analysis->metrics.budget_limit = work_budget
            ? work_budget
            : flow_analysis_default_budget(graph_metrics->blocks,
                                           graph_metrics->edges);
    if (!flow_analysis_build_adjacency(analysis)) return analysis;
    root_count = flow_analysis_root_count(analysis, roots);
    if (!root_count) {
        analysis->metrics.status = RXAS_FLOW_ANALYSIS_INVALID_GRAPH;
        return analysis;
    }
    if (!flow_analysis_build_rpo(analysis, roots, root_count) ||
        !flow_analysis_build_dominators(analysis, roots, root_count) ||
        !flow_analysis_build_dom_intervals(analysis) ||
        !flow_analysis_build_frontiers(analysis, roots, root_count))
        return analysis;
    flow_analysis_set_retained_bytes(analysis);
    return analysis;
}

const RxasFlowStructuralAnalysis *rxas_flow_require_structural_analysis(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget) {
    struct RxasFlowAnalysisManager *manager;
    size_t requested_budget;
    const RxasFlowMetrics *metrics;
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch)) return 0;
    metrics = rxas_flow_procedure_metrics(procedure, expected_epoch);
    if (!metrics) return 0;
    requested_budget = work_budget ? work_budget
            : flow_analysis_default_budget(metrics->blocks, metrics->edges);
    manager = procedure->analysis_manager;
    if (manager && manager->epoch == expected_epoch && manager->structural) {
        if (manager->structural->metrics.status ==
                RXAS_FLOW_ANALYSIS_AVAILABLE)
            return manager->structural;
        if (requested_budget <= manager->structural_budget) return 0;
        flow_structural_free(manager->structural);
        manager->structural = 0;
    }
    if (!manager) {
        manager = calloc(1, sizeof(*manager));
        if (!manager) return 0;
        procedure->analysis_manager = manager;
    }
    manager->epoch = expected_epoch;
    manager->structural_budget = requested_budget;
    manager->structural = flow_structural_build(
            procedure, expected_epoch, requested_budget);
    if (!manager->structural ||
        manager->structural->metrics.status != RXAS_FLOW_ANALYSIS_AVAILABLE)
        return 0;
    return manager->structural;
}

const RxasFlowStructuralAnalysis *rxas_flow_require_loop_analysis(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget) {
    RxasFlowStructuralAnalysis *analysis;
    const RxasFlowStructuralAnalysis *base;
    size_t roots[3];
    size_t root_count;
    base = rxas_flow_require_structural_analysis(
            procedure, expected_epoch, work_budget);
    if (!base) return 0;
    analysis = (RxasFlowStructuralAnalysis *)base;
    if (analysis->loops_ready) return analysis;
    if (work_budget > analysis->metrics.budget_limit)
        analysis->metrics.budget_limit = work_budget;
    root_count = flow_analysis_root_count(analysis, roots);
    if (!root_count ||
        !flow_analysis_build_sccs(analysis, roots, root_count) ||
        !flow_analysis_build_backedges(analysis) ||
        !flow_analysis_build_loops(analysis))
        return 0;
    analysis->loops_ready = 1;
    flow_analysis_set_retained_bytes(analysis);
    return analysis;
}

const RxasFlowStructuralMetrics *rxas_flow_last_structural_metrics(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch) {
    const struct RxasFlowAnalysisManager *manager;
    if (!rxas_flow_procedure_epoch_matches(procedure, expected_epoch) ||
        !procedure->analysis_manager)
        return 0;
    manager = procedure->analysis_manager;
    if (manager->epoch != expected_epoch || !manager->structural) return 0;
    return &manager->structural->metrics;
}

void rxas_flow_analysis_manager_destroy(RxasFlowProcedure *procedure) {
    struct RxasFlowAnalysisManager *manager;
    if (!procedure || !procedure->analysis_manager) return;
    manager = procedure->analysis_manager;
    rxas_flow_proof_service_destroy(manager->proof);
    rxas_flow_use_analysis_destroy(manager->use);
    rxas_flow_ssa_analysis_destroy(manager->ssa);
    rxas_flow_signal_analysis_destroy(manager->signal);
    flow_structural_free(manager->structural);
    free(manager);
    procedure->analysis_manager = 0;
}

static int flow_structural_valid(const RxasFlowStructuralAnalysis *analysis,
                                 unsigned long epoch) {
    return analysis && epoch && analysis->metrics.epoch == epoch &&
           analysis->metrics.status == RXAS_FLOW_ANALYSIS_AVAILABLE &&
           rxas_flow_procedure_epoch_matches(analysis->procedure, epoch);
}

const RxasFlowStructuralMetrics *rxas_flow_structural_metrics(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch) {
    if (!flow_structural_valid(analysis, expected_epoch)) return 0;
    return &analysis->metrics;
}

size_t rxas_flow_structural_rpo_count(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch) {
    if (!flow_structural_valid(analysis, expected_epoch)) return 0;
    return analysis->metrics.rpo_blocks;
}

size_t rxas_flow_structural_rpo_block(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t rpo_index) {
    if (!flow_structural_valid(analysis, expected_epoch) ||
        rpo_index >= analysis->metrics.rpo_blocks)
        return RXAS_FLOW_ID_NONE;
    return analysis->rpo[rpo_index];
}

size_t rxas_flow_structural_predecessor_count(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id) {
    if (!flow_structural_valid(analysis, expected_epoch) ||
        block_id >= analysis->block_count)
        return 0;
    return analysis->predecessor_offsets[block_id + 1] -
           analysis->predecessor_offsets[block_id];
}

size_t rxas_flow_structural_predecessor(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id,
        size_t predecessor_index) {
    size_t count;
    if (!flow_structural_valid(analysis, expected_epoch) ||
        block_id >= analysis->block_count)
        return RXAS_FLOW_ID_NONE;
    count = analysis->predecessor_offsets[block_id + 1] -
            analysis->predecessor_offsets[block_id];
    if (predecessor_index >= count) return RXAS_FLOW_ID_NONE;
    return analysis->predecessors[
            analysis->predecessor_offsets[block_id] + predecessor_index];
}

size_t rxas_flow_structural_immediate_dominator(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id) {
    size_t dominator;
    if (!flow_structural_valid(analysis, expected_epoch) ||
        block_id >= analysis->block_count ||
        analysis->rpo_index[block_id] == RXAS_FLOW_ID_NONE)
        return RXAS_FLOW_ID_NONE;
    dominator = analysis->idom[block_id];
    return dominator == analysis->virtual_root ? RXAS_FLOW_ID_NONE : dominator;
}

int rxas_flow_structural_dominates(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t dominator, size_t block_id) {
    if (!flow_structural_valid(analysis, expected_epoch) ||
        dominator >= analysis->block_count || block_id >= analysis->block_count)
        return 0;
    return flow_analysis_dominates_raw(analysis, dominator, block_id);
}

size_t rxas_flow_structural_frontier_count(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id) {
    if (!flow_structural_valid(analysis, expected_epoch) ||
        block_id >= analysis->block_count)
        return 0;
    return analysis->frontier_offsets[block_id + 1] -
           analysis->frontier_offsets[block_id];
}

size_t rxas_flow_structural_frontier(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id,
        size_t frontier_index) {
    size_t count;
    if (!flow_structural_valid(analysis, expected_epoch) ||
        block_id >= analysis->block_count)
        return RXAS_FLOW_ID_NONE;
    count = analysis->frontier_offsets[block_id + 1] -
            analysis->frontier_offsets[block_id];
    if (frontier_index >= count) return RXAS_FLOW_ID_NONE;
    return analysis->frontiers[
            analysis->frontier_offsets[block_id] + frontier_index];
}

size_t rxas_flow_structural_scc(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id) {
    if (!flow_structural_valid(analysis, expected_epoch) ||
        !analysis->loops_ready ||
        block_id >= analysis->block_count ||
        analysis->rpo_index[block_id] == RXAS_FLOW_ID_NONE)
        return RXAS_FLOW_ID_NONE;
    return analysis->scc[block_id];
}

int rxas_flow_structural_edge_is_backedge(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t edge_id) {
    if (!flow_structural_valid(analysis, expected_epoch) ||
        !analysis->loops_ready ||
        edge_id >= analysis->edge_count)
        return 0;
    return analysis->edge_backedge[edge_id] != 0;
}

size_t rxas_flow_structural_loop_count(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch) {
    if (!flow_structural_valid(analysis, expected_epoch) ||
        !analysis->loops_ready) return 0;
    return analysis->metrics.loops;
}

const RxasFlowLoop *rxas_flow_structural_loop(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t loop_id) {
    if (!flow_structural_valid(analysis, expected_epoch) ||
        !analysis->loops_ready ||
        loop_id >= analysis->metrics.loops)
        return 0;
    return &analysis->loops[loop_id];
}

size_t rxas_flow_structural_loop_member(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t loop_id,
        size_t member_index) {
    const RxasFlowLoop *loop;
    loop = rxas_flow_structural_loop(analysis, expected_epoch, loop_id);
    if (!loop || member_index >= loop->member_count) return RXAS_FLOW_ID_NONE;
    return analysis->loop_members[loop->member_offset + member_index];
}

size_t rxas_flow_structural_innermost_loop(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id) {
    if (!flow_structural_valid(analysis, expected_epoch) ||
        !analysis->loops_ready ||
        block_id >= analysis->block_count)
        return RXAS_FLOW_ID_NONE;
    return analysis->innermost_loop[block_id];
}

static const char *flow_analysis_status_name(RxasFlowAnalysisStatus status) {
    switch (status) {
        case RXAS_FLOW_ANALYSIS_AVAILABLE: return "available";
        case RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED: return "budget-exhausted";
        case RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY: return "out-of-memory";
        case RXAS_FLOW_ANALYSIS_INVALID_GRAPH: return "invalid-graph";
    }
    return "invalid";
}

int rxas_flow_structural_dump(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, FILE *stream) {
    size_t index;
    if (!stream || !flow_structural_valid(analysis, expected_epoch)) return 0;
    fprintf(stream,
            "PERF3 flow-analysis epoch=%lu status=%s budget=%llu work=%llu "
            "bytes=%llu reachable=%llu unreachable=%llu rpo=%llu "
            "preds=%llu dom-iterations=%llu frontiers=%llu sccs=%llu "
            "cyclic-sccs=%llu irreducible-sccs=%llu backedges=%llu "
            "loops=%llu memberships=%llu max-loop-depth=%llu\n",
            analysis->metrics.epoch,
            flow_analysis_status_name(analysis->metrics.status),
            (unsigned long long)analysis->metrics.budget_limit,
            (unsigned long long)analysis->metrics.work,
            (unsigned long long)analysis->metrics.retained_bytes,
            (unsigned long long)analysis->metrics.reachable_blocks,
            (unsigned long long)analysis->metrics.unreachable_blocks,
            (unsigned long long)analysis->metrics.rpo_blocks,
            (unsigned long long)analysis->metrics.predecessor_entries,
            (unsigned long long)analysis->metrics.dominator_iterations,
            (unsigned long long)analysis->metrics.dominance_frontier_entries,
            (unsigned long long)analysis->metrics.scc_count,
            (unsigned long long)analysis->metrics.cyclic_scc_count,
            (unsigned long long)analysis->metrics.irreducible_scc_count,
            (unsigned long long)analysis->metrics.backedges,
            (unsigned long long)analysis->metrics.loops,
            (unsigned long long)analysis->metrics.loop_memberships,
            (unsigned long long)analysis->metrics.max_loop_depth);
    for (index = 0; index < analysis->metrics.rpo_blocks; index++) {
        size_t block;
        size_t idom;
        size_t loop;
        block = analysis->rpo[index];
        idom = analysis->idom[block];
        loop = analysis->loops_ready
                ? analysis->innermost_loop[block] : RXAS_FLOW_ID_NONE;
        fprintf(stream,
                "PERF3 flow-analysis-block id=%llu rpo=%llu idom=",
                (unsigned long long)block, (unsigned long long)index);
        if (idom == analysis->virtual_root) fputc('-', stream);
        else fprintf(stream, "%llu", (unsigned long long)idom);
        if (analysis->loops_ready)
            fprintf(stream, " scc=%llu loop=",
                    (unsigned long long)analysis->scc[block]);
        else fputs(" scc=- loop=", stream);
        if (loop == RXAS_FLOW_ID_NONE) fputc('-', stream);
        else fprintf(stream, "%llu", (unsigned long long)loop);
        fprintf(stream, " predecessors=%llu frontier=%llu\n",
                (unsigned long long)(analysis->predecessor_offsets[block + 1] -
                                     analysis->predecessor_offsets[block]),
                (unsigned long long)(analysis->frontier_offsets[block + 1] -
                                     analysis->frontier_offsets[block]));
    }
    for (index = 0; index < analysis->metrics.loops; index++) {
        const RxasFlowLoop *loop;
        size_t member_index;
        loop = &analysis->loops[index];
        fprintf(stream,
                "PERF3 flow-loop id=%llu header=%llu parent=",
                (unsigned long long)loop->id,
                (unsigned long long)loop->header);
        if (loop->parent == RXAS_FLOW_ID_NONE) fputc('-', stream);
        else fprintf(stream, "%llu", (unsigned long long)loop->parent);
        fprintf(stream,
                " depth=%llu flags=%u latches=%llu members=",
                (unsigned long long)loop->depth, loop->flags,
                (unsigned long long)loop->latch_count);
        for (member_index = 0; member_index < loop->member_count;
             member_index++) {
            if (member_index) fputc(',', stream);
            fprintf(stream, "%llu",
                    (unsigned long long)analysis->loop_members[
                            loop->member_offset + member_index]);
        }
        fputc('\n', stream);
    }
    return 1;
}
