/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXAS_FLOW_PROOF_H
#define CREXX_RXAS_FLOW_PROOF_H

#include <stddef.h>
#include <stdio.h>

#include "rxas_flow_ssa.h"

typedef enum RxasFlowProofReason {
    RXAS_FLOW_PROOF_PROVED = 0,
    RXAS_FLOW_PROOF_STALE_EPOCH,
    RXAS_FLOW_PROOF_INVALID_GRAPH,
    RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE,
    RXAS_FLOW_PROOF_BUDGET_EXHAUSTED,
    RXAS_FLOW_PROOF_INVALID_INSTRUCTION,
    RXAS_FLOW_PROOF_OPCODE_MISMATCH,
    RXAS_FLOW_PROOF_NOT_SUCCESS_STABLE,
    RXAS_FLOW_PROOF_NOT_DOMINATED,
    RXAS_FLOW_PROOF_SUCCESS_EDGE_NOT_DOMINATING,
    RXAS_FLOW_PROOF_UNSUPPORTED_SIGNAL_DEPENDENCY,
    RXAS_FLOW_PROOF_STORAGE_UNKNOWN,
    RXAS_FLOW_PROOF_STORAGE_CHANGED,
    RXAS_FLOW_PROOF_SOURCE_UNKNOWN,
    RXAS_FLOW_PROOF_GENERATOR_RESULT_UNKNOWN,
    RXAS_FLOW_PROOF_GENERATOR_SOURCE_UNKNOWN,
    RXAS_FLOW_PROOF_CANDIDATE_SOURCE_UNKNOWN,
    RXAS_FLOW_PROOF_SOURCE_CHANGED,
    RXAS_FLOW_PROOF_RESULT_UNAVAILABLE,
    RXAS_FLOW_PROOF_EFFECT_CHANGED,
    RXAS_FLOW_PROOF_REFERENCE_EFFECT_CHANGED,
    RXAS_FLOW_PROOF_NOT_EXACT_CONSTANT_WRITE,
    RXAS_FLOW_PROOF_CONSTANT_UNKNOWN,
    RXAS_FLOW_PROOF_CONSTANT_CHANGED,
    RXAS_FLOW_PROOF_CLEANUP_REQUIRED,
    RXAS_FLOW_PROOF_NOT_EXACT_ABSENT_WRITE,
    RXAS_FLOW_PROOF_COMPONENT_PRESENT,
    RXAS_FLOW_PROOF_ABSENCE_UNKNOWN,
    RXAS_FLOW_PROOF_NOT_IN_LOOP,
    RXAS_FLOW_PROOF_IRREDUCIBLE_LOOP,
    RXAS_FLOW_PROOF_NOT_MUST_EXECUTE,
    RXAS_FLOW_PROOF_NOT_SPECULATABLE,
    RXAS_FLOW_PROOF_NOT_INVARIANT
} RxasFlowProofReason;

typedef struct RxasFlowProofResult {
    int proved;
    RxasFlowProofReason reason;
    size_t generator_instruction;
    size_t candidate_instruction;
    size_t loop_id;
    size_t storage_id;
    size_t source_value_id;
    size_t result_value_id;
    size_t candidate_source_value_id;
    size_t candidate_result_value_id;
    size_t effect_class;
    size_t generator_effect_id;
    size_t candidate_effect_id;
    RxasFlowValueKind source_kind;
    RxasFlowValueKind result_kind;
    RxasFlowValueKind candidate_source_kind;
    RxasFlowValueKind candidate_result_kind;
} RxasFlowProofResult;

typedef struct RxasFlowProofMetrics {
    RxasFlowAnalysisStatus status;
    unsigned long epoch;
    size_t budget_limit;
    size_t work;
    size_t retained_bytes;
    size_t repetition_queries;
    size_t repetition_cache_hits;
    size_t repetition_proved;
    size_t repetition_rejected;
    size_t redundant_constant_queries;
    size_t redundant_constant_proved;
    size_t redundant_constant_rejected;
    size_t redundant_absent_queries;
    size_t redundant_absent_proved;
    size_t redundant_absent_rejected;
    size_t success_edge_queries;
    size_t loop_queries;
} RxasFlowProofMetrics;

typedef struct RxasFlowRepetitionKey {
    int opcode;
    RxOpValueDerivation derivation;
    size_t storage_id;
} RxasFlowRepetitionKey;

typedef struct RxasFlowProofService RxasFlowProofService;

const RxasFlowProofService *rxas_flow_require_proof_service(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget);
const RxasFlowProofMetrics *rxas_flow_last_proof_metrics(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch);
const RxasFlowProofMetrics *rxas_flow_proof_metrics(
        const RxasFlowProofService *service, unsigned long expected_epoch);

int rxas_flow_prove_repetition(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t generator_instruction, size_t candidate_instruction,
        RxasFlowProofResult *result);
int rxas_flow_repetition_key(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t instruction_id, RxasFlowRepetitionKey *key);
int rxas_flow_prove_redundant_constant_write(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t candidate_instruction, RxasFlowProofResult *result);
int rxas_flow_prove_redundant_absent_write(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t candidate_instruction, RxasFlowProofResult *result);
int rxas_flow_prove_instruction_speculatable(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t instruction_id, RxasFlowProofResult *result);
int rxas_flow_prove_must_execute_in_loop(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t instruction_id, size_t loop_id, RxasFlowProofResult *result);
int rxas_flow_prove_loop_component_invariant(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t instruction_id, size_t loop_id, RxasFlowRegister register_id,
        unsigned int component, RxasFlowProofResult *result);

const char *rxas_flow_proof_reason_name(RxasFlowProofReason reason);
int rxas_flow_proof_dump(const RxasFlowProofService *service,
                         unsigned long expected_epoch, FILE *stream);

/* Internal analysis-manager hook. */
void rxas_flow_proof_service_destroy(struct RxasFlowProofService *service);

#endif
