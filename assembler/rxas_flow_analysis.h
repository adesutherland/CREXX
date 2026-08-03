/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXAS_FLOW_ANALYSIS_H
#define CREXX_RXAS_FLOW_ANALYSIS_H

#include <stddef.h>
#include <stdio.h>

#include "rxas_flow_graph.h"

typedef enum RxasFlowAnalysisStatus {
    RXAS_FLOW_ANALYSIS_AVAILABLE = 0,
    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED,
    RXAS_FLOW_ANALYSIS_OUT_OF_MEMORY,
    RXAS_FLOW_ANALYSIS_INVALID_GRAPH
} RxasFlowAnalysisStatus;

typedef enum RxasFlowLoopFlags {
    RXAS_FLOW_LOOP_NATURAL = 1,
    RXAS_FLOW_LOOP_IRREDUCIBLE = 2
} RxasFlowLoopFlags;

typedef struct RxasFlowLoop {
    size_t id;
    size_t header;
    size_t parent;
    size_t member_offset;
    size_t member_count;
    size_t latch_count;
    size_t depth;
    unsigned int flags;
} RxasFlowLoop;

typedef struct RxasFlowStructuralMetrics {
    RxasFlowAnalysisStatus status;
    unsigned long epoch;
    size_t budget_limit;
    size_t work;
    size_t retained_bytes;
    size_t reachable_blocks;
    size_t unreachable_blocks;
    size_t predecessor_entries;
    size_t rpo_blocks;
    size_t dominator_iterations;
    size_t dominance_frontier_entries;
    size_t scc_count;
    size_t cyclic_scc_count;
    size_t irreducible_scc_count;
    size_t backedges;
    size_t loops;
    size_t loop_memberships;
    size_t max_loop_depth;
} RxasFlowStructuralMetrics;

typedef struct RxasFlowStructuralAnalysis RxasFlowStructuralAnalysis;

/* A zero budget selects the size-derived default. The first successful result
 * is cached by the owning procedure epoch. Failed low-budget requests may be
 * retried with a larger budget. */
const RxasFlowStructuralAnalysis *rxas_flow_require_structural_analysis(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget);
/* SCC, backedge and loop-forest construction is a separate capability.  It
 * extends the cached structural analysis only when an explicit loop consumer
 * requests it. */
const RxasFlowStructuralAnalysis *rxas_flow_require_loop_analysis(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget);
const RxasFlowStructuralMetrics *rxas_flow_last_structural_metrics(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch);

const RxasFlowStructuralMetrics *rxas_flow_structural_metrics(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch);
size_t rxas_flow_structural_rpo_count(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch);
size_t rxas_flow_structural_rpo_block(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t rpo_index);
size_t rxas_flow_structural_predecessor_count(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id);
size_t rxas_flow_structural_predecessor(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id,
        size_t predecessor_index);
size_t rxas_flow_structural_immediate_dominator(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id);
int rxas_flow_structural_dominates(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t dominator, size_t block_id);
size_t rxas_flow_structural_frontier_count(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id);
size_t rxas_flow_structural_frontier(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id,
        size_t frontier_index);
size_t rxas_flow_structural_scc(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id);
int rxas_flow_structural_edge_is_backedge(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t edge_id);
size_t rxas_flow_structural_loop_count(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch);
const RxasFlowLoop *rxas_flow_structural_loop(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t loop_id);
size_t rxas_flow_structural_loop_member(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t loop_id,
        size_t member_index);
size_t rxas_flow_structural_innermost_loop(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, size_t block_id);

int rxas_flow_structural_dump(
        const RxasFlowStructuralAnalysis *analysis,
        unsigned long expected_epoch, FILE *stream);

/* Internal graph-owner hook; safe on a procedure with no cached analysis. */
void rxas_flow_analysis_manager_destroy(RxasFlowProcedure *procedure);

#endif
