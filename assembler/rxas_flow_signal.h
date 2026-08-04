/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXAS_FLOW_SIGNAL_H
#define CREXX_RXAS_FLOW_SIGNAL_H

#include <stddef.h>
#include <stdio.h>

#include "rxas_flow_analysis.h"

typedef enum RxasFlowPolicyFactState {
    RXAS_FLOW_POLICY_EXACT = 0,
    RXAS_FLOW_POLICY_INHERITED_UNKNOWN,
    RXAS_FLOW_POLICY_MERGED_UNKNOWN,
    RXAS_FLOW_POLICY_CLOBBERED,
    RXAS_FLOW_POLICY_STACK_UNKNOWN
} RxasFlowPolicyFactState;

typedef struct RxasFlowPolicyFact {
    RxasFlowPolicyFactState state;
    RxOpSignalPolicyEffect effect;
    size_t version_id;
    size_t defining_instruction;
} RxasFlowPolicyFact;

typedef enum RxasFlowEffectClass {
    RXAS_FLOW_EFFECT_NUMERIC_CONTEXT = 0,
    RXAS_FLOW_EFFECT_PLUGIN,
    RXAS_FLOW_EFFECT_LOCALE,
    RXAS_FLOW_EFFECT_EXTERNAL,
    /* Changes to the alias/name topology are distinct from mutations that
     * may be observed through an existing alias. */
    RXAS_FLOW_EFFECT_ALIAS,
    RXAS_FLOW_EFFECT_REFERENCE,
    RXAS_FLOW_EFFECT_TRACE,
    RXAS_FLOW_EFFECT_CALL,
    RXAS_FLOW_EFFECT_CLASS_COUNT
} RxasFlowEffectClass;

typedef struct RxasFlowSignalMetrics {
    RxasFlowAnalysisStatus status;
    unsigned long epoch;
    size_t budget_limit;
    size_t work;
    size_t retained_bytes;
    size_t policy_versions;
    size_t policy_writes;
    size_t policy_phis;
    size_t policy_clobbers;
    size_t stack_writes;
    size_t effect_versions;
    size_t effect_phis;
    size_t trace_effect_writes;
    size_t call_effect_writes;
    size_t edge_states;
    size_t unknown_edge_states;
} RxasFlowSignalMetrics;

typedef enum RxasFlowEffectNodeKind {
    RXAS_FLOW_EFFECT_NODE_ENTRY = 0,
    RXAS_FLOW_EFFECT_NODE_WRITE,
    RXAS_FLOW_EFFECT_NODE_PHI,
    RXAS_FLOW_EFFECT_NODE_UNKNOWN
} RxasFlowEffectNodeKind;

typedef struct RxasFlowEffectNode {
    size_t id;
    RxasFlowEffectNodeKind kind;
    RxasFlowEffectClass effect_class;
    size_t parent_id;
    size_t defining_instruction;
    size_t block_id;
    size_t input_count;
} RxasFlowEffectNode;

typedef struct RxasFlowSignalAnalysis RxasFlowSignalAnalysis;

/* Zero selects a size-derived budget. A successful result is cached by the
 * immutable procedure epoch; a failed low-budget request may be retried with
 * a larger budget. */
const RxasFlowSignalAnalysis *rxas_flow_require_signal_analysis(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget);
const RxasFlowSignalMetrics *rxas_flow_last_signal_metrics(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch);
const RxasFlowSignalMetrics *rxas_flow_signal_metrics(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch);

int rxas_flow_policy_at_instruction(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch,
        size_t instruction_id, int after_instruction, const char *signal_name,
        RxasFlowPolicyFact *fact);
int rxas_flow_policy_on_edge(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch,
        size_t edge_id, const char *signal_name, RxasFlowPolicyFact *fact);

size_t rxas_flow_effect_at_instruction(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch,
        size_t instruction_id, int after_instruction,
        RxasFlowEffectClass effect_class);
size_t rxas_flow_effect_on_edge(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch,
        size_t edge_id, RxasFlowEffectClass effect_class);
size_t rxas_flow_effect_version_count(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch);
int rxas_flow_effect_node(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch,
        size_t effect_id, RxasFlowEffectNode *node);
size_t rxas_flow_effect_input(
        const RxasFlowSignalAnalysis *analysis, unsigned long expected_epoch,
        size_t effect_id, size_t input_index);

int rxas_flow_signal_dump(const RxasFlowSignalAnalysis *analysis,
                          unsigned long expected_epoch, FILE *stream);

/* Internal analysis-manager hook. */
void rxas_flow_signal_analysis_destroy(struct RxasFlowSignalAnalysis *analysis);

#endif
