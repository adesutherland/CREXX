/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXAS_FLOW_SSA_H
#define CREXX_RXAS_FLOW_SSA_H

#include <stddef.h>
#include <stdio.h>

#include "rxas_flow_signal.h"

typedef enum RxasFlowRegisterClass {
    RXAS_FLOW_REGISTER_LOCAL = 0,
    RXAS_FLOW_REGISTER_ARGUMENT,
    RXAS_FLOW_REGISTER_GLOBAL
} RxasFlowRegisterClass;

typedef struct RxasFlowRegister {
    RxasFlowRegisterClass register_class;
    size_t number;
} RxasFlowRegister;

typedef enum RxasFlowStorageKind {
    RXAS_FLOW_STORAGE_BASE = 0,
    RXAS_FLOW_STORAGE_SITE,
    RXAS_FLOW_STORAGE_PHI,
    RXAS_FLOW_STORAGE_UNKNOWN
} RxasFlowStorageKind;

typedef struct RxasFlowStorageFact {
    RxasFlowStorageKind kind;
    size_t storage_id;
    size_t defining_instruction;
    size_t defining_block;
} RxasFlowStorageFact;

typedef enum RxasFlowComponentPresence {
    RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN = 0,
    RXAS_FLOW_COMPONENT_PRESENT,
    RXAS_FLOW_COMPONENT_ABSENT
} RxasFlowComponentPresence;

typedef enum RxasFlowValueKind {
    RXAS_FLOW_VALUE_ENTRY = 0,
    RXAS_FLOW_VALUE_WRITE,
    RXAS_FLOW_VALUE_CONSTANT,
    RXAS_FLOW_VALUE_COPY,
    RXAS_FLOW_VALUE_DERIVED,
    RXAS_FLOW_VALUE_ABSENT,
    RXAS_FLOW_VALUE_PHI,
    RXAS_FLOW_VALUE_UNKNOWN
} RxasFlowValueKind;

typedef struct RxasFlowComponentFact {
    RxasFlowValueKind kind;
    RxasFlowComponentPresence presence;
    size_t storage_id;
    size_t value_id;
    unsigned int component;
    size_t defining_instruction;
    size_t source_value_id;
    RxOpValueDerivation derivation;
    unsigned int signal_dependencies;
    size_t definition_effects[RXAS_FLOW_EFFECT_CLASS_COUNT];
    size_t current_effects[RXAS_FLOW_EFFECT_CLASS_COUNT];
    size_t definition_numeric_context;
    size_t current_numeric_context;
    size_t current_reference_effect;
    const Assembler_Token *constant_token;
} RxasFlowComponentFact;

typedef struct RxasFlowSsaMetrics {
    RxasFlowAnalysisStatus status;
    unsigned long epoch;
    size_t budget_limit;
    size_t work;
    size_t retained_bytes;
    size_t registers;
    size_t states;
    size_t join_states;
    size_t mapping_updates;
    size_t mapping_clobbers;
    size_t storage_versions;
    size_t storage_sites;
    size_t storage_phis;
    size_t component_updates;
    size_t value_versions;
    size_t value_phis;
    size_t absent_values;
    size_t constant_values;
    size_t derived_values;
    size_t unknown_values;
    size_t edge_states;
} RxasFlowSsaMetrics;

typedef struct RxasFlowSsaAnalysis RxasFlowSsaAnalysis;

const RxasFlowSsaAnalysis *rxas_flow_require_ssa_analysis(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget);
const RxasFlowSsaMetrics *rxas_flow_last_ssa_metrics(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch);
const RxasFlowSsaMetrics *rxas_flow_ssa_metrics(
        const RxasFlowSsaAnalysis *analysis, unsigned long expected_epoch);

int rxas_flow_storage_at_instruction(
        const RxasFlowSsaAnalysis *analysis, unsigned long expected_epoch,
        size_t instruction_id, int after_instruction,
        RxasFlowRegister register_id, RxasFlowStorageFact *fact);
int rxas_flow_storage_on_edge(
        const RxasFlowSsaAnalysis *analysis, unsigned long expected_epoch,
        size_t edge_id, RxasFlowRegister register_id,
        RxasFlowStorageFact *fact);
int rxas_flow_component_at_instruction(
        const RxasFlowSsaAnalysis *analysis, unsigned long expected_epoch,
        size_t instruction_id, int after_instruction,
        RxasFlowRegister register_id, unsigned int component,
        RxasFlowComponentFact *fact);
int rxas_flow_component_on_edge(
        const RxasFlowSsaAnalysis *analysis, unsigned long expected_epoch,
        size_t edge_id, RxasFlowRegister register_id, unsigned int component,
        RxasFlowComponentFact *fact);

int rxas_flow_ssa_dump(const RxasFlowSsaAnalysis *analysis,
                       unsigned long expected_epoch, FILE *stream);

/* Internal analysis-manager hook. */
void rxas_flow_ssa_analysis_destroy(struct RxasFlowSsaAnalysis *analysis);

#endif
