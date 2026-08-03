/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXAS_FLOW_USE_H
#define CREXX_RXAS_FLOW_USE_H

#include <stddef.h>
#include <stdio.h>

#include "rxas_flow_ssa.h"

typedef enum RxasFlowUseKind {
    RXAS_FLOW_USE_EXPLICIT_READ = 0,
    RXAS_FLOW_USE_EXPLICIT_READ_WRITE,
    RXAS_FLOW_USE_IMPLICIT_READ,
    RXAS_FLOW_USE_METADATA_READ,
    RXAS_FLOW_USE_TRACE_READ,
    RXAS_FLOW_USE_CURSOR_READ,
    RXAS_FLOW_USE_CALL_WINDOW_READ,
    RXAS_FLOW_USE_OPAQUE_OBSERVATION
} RxasFlowUseKind;

typedef struct RxasFlowUse {
    size_t id;
    size_t record_id;
    size_t instruction_id;
    size_t operand_index;
    RxasFlowRegister register_id;
    unsigned int component;
    unsigned int read_components;
    size_t value_id;
    size_t storage_id;
    RxasFlowUseKind kind;
} RxasFlowUse;

typedef struct RxasFlowUseMetrics {
    RxasFlowAnalysisStatus status;
    unsigned long epoch;
    size_t budget_limit;
    size_t work;
    size_t retained_bytes;
    size_t uses;
    size_t explicit_reads;
    size_t read_write_uses;
    size_t implicit_reads;
    size_t metadata_reads;
    size_t trace_reads;
    size_t cursor_reads;
    size_t call_window_reads;
    size_t opaque_observations;
    size_t unknown_values;
    size_t phi_dependency_edges;
    size_t live_values;
} RxasFlowUseMetrics;

typedef struct RxasFlowUseAnalysis RxasFlowUseAnalysis;

const RxasFlowUseAnalysis *rxas_flow_require_use_analysis(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget);
const RxasFlowUseMetrics *rxas_flow_last_use_metrics(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch);
const RxasFlowUseMetrics *rxas_flow_use_metrics(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch);

size_t rxas_flow_use_count(const RxasFlowUseAnalysis *analysis,
                           unsigned long expected_epoch);
const RxasFlowUse *rxas_flow_use(const RxasFlowUseAnalysis *analysis,
                                 unsigned long expected_epoch, size_t use_id);
size_t rxas_flow_value_direct_use_count(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch,
        size_t value_id);
const RxasFlowUse *rxas_flow_value_direct_use(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch,
        size_t value_id, size_t use_index);
size_t rxas_flow_value_dependent_count(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch,
        size_t value_id);
size_t rxas_flow_value_dependent(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch,
        size_t value_id, size_t dependent_index);
int rxas_flow_value_is_live(const RxasFlowUseAnalysis *analysis,
                            unsigned long expected_epoch, size_t value_id);
size_t rxas_flow_storage_use_count(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch,
        size_t storage_id);
const RxasFlowUse *rxas_flow_storage_use(
        const RxasFlowUseAnalysis *analysis, unsigned long expected_epoch,
        size_t storage_id, size_t use_index);

int rxas_flow_use_dump(const RxasFlowUseAnalysis *analysis,
                       unsigned long expected_epoch, FILE *stream);

/* Internal analysis-manager hook. */
void rxas_flow_use_analysis_destroy(struct RxasFlowUseAnalysis *analysis);

#endif
