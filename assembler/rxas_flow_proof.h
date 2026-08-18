/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXAS_FLOW_PROOF_H
#define CREXX_RXAS_FLOW_PROOF_H

#include <stddef.h>
#include <stdio.h>

#include "rxas_flow_pass.h"
#include "rxas_flow_use.h"

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
    RXAS_FLOW_PROOF_NOT_EXACT_SELF_COPY,
    RXAS_FLOW_PROOF_STORAGE_NOT_IDENTICAL,
    RXAS_FLOW_PROOF_NOT_EXACT_TYPED_COPY,
    RXAS_FLOW_PROOF_DESTINATION_NOT_LOCAL,
    RXAS_FLOW_PROOF_DESTINATION_OBSERVABLE,
    RXAS_FLOW_PROOF_USE_NOT_REDIRECTABLE,
    RXAS_FLOW_PROOF_SOURCE_NOT_EQUIVALENT,
    RXAS_FLOW_PROOF_CALL_WINDOW_OBSERVED,
    RXAS_FLOW_PROOF_NO_REDIRECTS,
    RXAS_FLOW_PROOF_NOT_EXACT_PRODUCER_FORWARD,
    RXAS_FLOW_PROOF_NOT_ADJACENT_COPY,
    RXAS_FLOW_PROOF_TEMPORARY_OBSERVABLE,
    RXAS_FLOW_PROOF_NOT_EXACT_COMPONENT_PLACEMENT,
    RXAS_FLOW_PROOF_SOURCE_RESULT_OBSERVED,
    RXAS_FLOW_PROOF_TEMPORARY_INPUT_OBSERVED,
    RXAS_FLOW_PROOF_REFERENCE_OBSERVED,
    RXAS_FLOW_PROOF_ADDRESS_OBSERVED,
    RXAS_FLOW_PROOF_NOT_EXACT_COMPARE_BRANCH_FUSION,
    RXAS_FLOW_PROOF_NOT_EXACT_DUPLICATE_LINKED_READ,
    RXAS_FLOW_PROOF_ATTRIBUTE_RANGE_UNKNOWN,
    RXAS_FLOW_PROOF_ATTRIBUTE_PATH_CHANGED,
    RXAS_FLOW_PROOF_NOT_EXACT_STORAGE_PERMUTATION,
    RXAS_FLOW_PROOF_PERMUTATION_NOT_RESTORED,
    RXAS_FLOW_PROOF_PERMUTATION_OBSERVED,
    RXAS_FLOW_PROOF_PERMUTATION_SIGNAL_EXIT,
    RXAS_FLOW_PROOF_NOT_ADJACENT_BRANCH,
    RXAS_FLOW_PROOF_COMPARE_RESULT_OBSERVED,
    RXAS_FLOW_PROOF_TRACE_OBSERVED,
    RXAS_FLOW_PROOF_NOT_EXACT_JOINED_KEY_REUSE,
    RXAS_FLOW_PROOF_JOINED_KEY_NOT_EQUIVALENT,
    RXAS_FLOW_PROOF_JOINED_RESULT_OBSERVED,
    RXAS_FLOW_PROOF_NOT_EXACT_STRING_LITERAL_REUSE,
    RXAS_FLOW_PROOF_STRING_LITERAL_NOT_EQUIVALENT,
    RXAS_FLOW_PROOF_ALIAS_LIFETIME_UNSAFE,
    RXAS_FLOW_PROOF_NOT_IN_LOOP,
    RXAS_FLOW_PROOF_IRREDUCIBLE_LOOP,
    RXAS_FLOW_PROOF_NOT_MUST_EXECUTE,
    RXAS_FLOW_PROOF_NOT_SPECULATABLE,
    RXAS_FLOW_PROOF_NOT_INVARIANT,
    RXAS_FLOW_PROOF_NOT_EXACT_SUCCESSFUL_GUARD,
    RXAS_FLOW_PROOF_GUARD_NOT_COVERED
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
    unsigned int requested_capabilities;
    unsigned int acquired_capabilities;
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
    size_t redundant_self_copy_queries;
    size_t redundant_self_copy_proved;
    size_t redundant_self_copy_rejected;
    size_t typed_copy_redirect_queries;
    size_t typed_copy_redirect_proved;
    size_t typed_copy_redirect_rejected;
    size_t typed_copy_operand_rewrites;
    size_t producer_forward_queries;
    size_t producer_forward_proved;
    size_t producer_forward_rejected;
    size_t component_placement_queries;
    size_t component_placement_proved;
    size_t component_placement_rejected;
    size_t component_placement_operand_rewrites;
    size_t component_placement_trace_deletions;
    size_t compare_branch_queries;
    size_t compare_branch_proved;
    size_t compare_branch_rejected;
    size_t compare_branch_trace_deletions;
    size_t duplicate_linked_read_queries;
    size_t duplicate_linked_read_proved;
    size_t duplicate_linked_read_rejected;
    size_t storage_permutation_queries;
    size_t storage_permutation_proved;
    size_t storage_permutation_rejected;
    size_t joined_key_reuse_queries;
    size_t joined_key_reuse_proved;
    size_t joined_key_reuse_rejected;
    size_t joined_key_reuse_trace_deletions;
    size_t joined_key_preheader_eligible;
    size_t string_literal_reuse_queries;
    size_t string_literal_reuse_proved;
    size_t string_literal_reuse_rejected;
    size_t string_literal_operand_rewrites;
    size_t successful_guard_queries;
    size_t successful_guard_proved;
    size_t successful_guard_rejected;
    size_t success_edge_queries;
    size_t loop_queries;
} RxasFlowProofMetrics;

typedef struct RxasFlowRepetitionKey {
    int opcode;
    RxOpValueDerivation derivation;
    size_t storage_id;
} RxasFlowRepetitionKey;

typedef enum RxasFlowSuccessfulGuardKeyKind {
    RXAS_FLOW_GUARD_KEY_NONE = 0,
    RXAS_FLOW_GUARD_KEY_VALUE,
    RXAS_FLOW_GUARD_KEY_INTEGER
} RxasFlowSuccessfulGuardKeyKind;

/* Sparse primary-value key used only to select possible earlier guards.  The
 * full guard proof remains authoritative for type/bounds, dominance, signal
 * continuation and every component value. */
typedef struct RxasFlowSuccessfulGuardKey {
    RxasFlowSuccessfulGuardKeyKind kind;
    int family;
    size_t value_id;
    rxinteger integer_value;
} RxasFlowSuccessfulGuardKey;

typedef struct RxasFlowOperandRewrite {
    size_t record_id;
    size_t instruction_id;
    size_t operand_index;
    RxasFlowRegister expected_register;
    RxasFlowRegister replacement_register;
} RxasFlowOperandRewrite;

typedef struct RxasFlowTraceDeletion {
    size_t record_id;
    size_t value_id;
    unsigned int component;
    RxasFlowRegister expected_register;
} RxasFlowTraceDeletion;

typedef struct RxasFlowTypedCopyPlan {
    int proved;
    RxasFlowProofReason reason;
    size_t candidate_instruction;
    unsigned int component;
    size_t destination_storage_id;
    size_t source_value_id;
    size_t result_value_id;
    size_t rewrite_offset;
    size_t rewrite_count;
} RxasFlowTypedCopyPlan;

typedef struct RxasFlowProducerDestinationPlan {
    int proved;
    RxasFlowProofReason reason;
    size_t producer_instruction;
    size_t copy_instruction;
    size_t producer_record_id;
    size_t copy_record_id;
    unsigned int component;
    unsigned int cleanup_components;
    size_t temporary_storage_id;
    size_t destination_storage_id;
    size_t temporary_result_value_id;
    size_t copy_result_value_id;
    RxasFlowOperandRewrite producer_rewrite;
} RxasFlowProducerDestinationPlan;

typedef struct RxasFlowComponentPlacementPlan {
    int proved;
    RxasFlowProofReason reason;
    size_t copy_instruction;
    size_t derivation_instruction;
    size_t copy_record_id;
    size_t derivation_record_id;
    int expected_copy_opcode;
    int expected_derivation_opcode;
    unsigned int source_component;
    unsigned int result_component;
    size_t source_storage_id;
    size_t temporary_storage_id;
    size_t source_value_id;
    size_t copy_result_value_id;
    size_t displaced_source_result_value_id;
    size_t derivation_result_value_id;
    RxasFlowUseKind rejected_use_kind;
    size_t rejected_use_record_id;
    size_t rejected_use_instruction_id;
    size_t rejected_use_operand_index;
    size_t rejected_use_value_id;
    RxasFlowOperandRewrite derivation_rewrite;
    size_t rewrite_offset;
    size_t rewrite_count;
    size_t trace_deletion_offset;
    size_t trace_deletion_count;
} RxasFlowComponentPlacementPlan;

typedef struct RxasFlowCompareBranchPlan {
    int proved;
    RxasFlowProofReason reason;
    size_t compare_instruction;
    size_t branch_instruction;
    size_t compare_record_id;
    size_t branch_record_id;
    int expected_compare_opcode;
    int expected_branch_opcode;
    int fused_opcode;
    size_t left_source_operand;
    size_t right_source_operand;
    size_t result_storage_id;
    size_t result_value_id;
    unsigned int result_source_operands;
    size_t result_source_value_ids[2];
    unsigned int rejected_component;
    RxasFlowValueKind rejected_component_kind;
    RxasFlowComponentPresence rejected_component_presence;
    size_t rejected_component_value_id;
    RxasFlowValueKind rejected_leaf_kind;
    RxasFlowComponentPresence rejected_leaf_presence;
    size_t rejected_leaf_value_id;
    size_t rejected_leaf_defining_instruction;
    RxasFlowUseKind rejected_use_kind;
    size_t rejected_use_record_id;
    size_t rejected_use_instruction_id;
    size_t rejected_use_operand_index;
    size_t rejected_use_value_id;
    size_t trace_deletion_offset;
    size_t trace_deletion_count;
    RxasFlowRegister result_register;
} RxasFlowCompareBranchPlan;

typedef struct RxasFlowDuplicateLinkedReadPlan {
    int proved;
    RxasFlowProofReason reason;
    size_t first_link_instruction;
    size_t second_link_instruction;
    size_t first_link_record_id;
    size_t first_copy_record_id;
    size_t first_unlink_record_id;
    size_t second_link_record_id;
    size_t second_copy_record_id;
    size_t second_unlink_record_id;
    int expected_link_opcode;
    int expected_copy_opcode;
    unsigned int read_components;
    size_t linked_storage_id;
    size_t candidate_linked_storage_id;
    size_t owner_storage_id;
    size_t candidate_owner_storage_id;
    size_t attribute_count_value_id;
    size_t candidate_attribute_count_value_id;
    size_t reference_effect_id;
    size_t candidate_reference_effect_id;
    unsigned int rejected_component;
    size_t first_value_id;
    size_t candidate_value_id;
    RxasFlowRegister first_temporary;
    RxasFlowRegister first_detached;
    RxasFlowRegister second_temporary;
    RxasFlowRegister second_destination;
} RxasFlowDuplicateLinkedReadPlan;

typedef struct RxasFlowStoragePermutationPlan {
    int proved;
    RxasFlowProofReason reason;
    size_t first_instruction;
    size_t second_instruction;
    size_t first_record_id;
    size_t second_record_id;
    size_t deletion_count;
    int expected_first_opcode;
    int expected_second_opcode;
    size_t left_storage_id;
    size_t right_storage_id;
    RxasFlowRegister first_left;
    RxasFlowRegister first_right;
    RxasFlowRegister second_left;
    RxasFlowRegister second_right;
} RxasFlowStoragePermutationPlan;

typedef struct RxasFlowJoinedKeyReusePlan {
    int proved;
    RxasFlowProofReason reason;
    size_t seed_instruction;
    size_t candidate_instruction;
    size_t stem_instruction;
    size_t seed_record_id;
    size_t candidate_record_id;
    size_t stem_record_id;
    size_t loop_id;
    int expected_concat_opcode;
    int expected_stem_opcode;
    RxasFlowRegister cache_register;
    RxasFlowRegister candidate_register;
    RxasFlowRegister seed_right_register;
    RxasFlowRegister candidate_right_register;
    size_t cache_storage_id;
    size_t cache_storage_root;
    size_t candidate_storage_id;
    size_t candidate_storage_root;
    size_t cache_value_id;
    size_t candidate_value_id;
    size_t seed_right_value_id;
    size_t candidate_right_value_id;
    size_t stem_key_operand;
    int preheader_speculatable;
    int preheader_must_execute;
    int preheader_right_invariant;
    int preheader_trace_free;
    int preheader_eligible;
    RxasFlowProofReason preheader_must_execute_reason;
    RxasFlowProofReason preheader_invariant_reason;
    RxasFlowUseKind rejected_use_kind;
    size_t rejected_use_record_id;
    size_t rejected_use_instruction_id;
    size_t rejected_use_operand_index;
    size_t rejected_use_value_id;
    size_t seed_rewrite_offset;
    size_t seed_rewrite_count;
    size_t trace_deletion_offset;
    size_t trace_deletion_count;
} RxasFlowJoinedKeyReusePlan;

typedef struct RxasFlowStringLiteralReusePlan {
    int proved;
    RxasFlowProofReason reason;
    size_t seed_instruction;
    size_t candidate_instruction;
    size_t seed_record_id;
    size_t candidate_record_id;
    size_t loop_id;
    int expected_opcode;
    RxasFlowRegister seed_register;
    RxasFlowRegister candidate_register;
    size_t seed_storage_id;
    size_t seed_storage_root;
    size_t candidate_storage_id;
    size_t candidate_storage_root;
    size_t seed_value_id;
    size_t seed_candidate_value_id;
    size_t seed_candidate_defining_instruction;
    RxasFlowValueKind seed_candidate_kind;
    RxasFlowComponentPresence seed_candidate_presence;
    size_t candidate_value_id;
    unsigned int rejected_cleanup_component;
    size_t rejected_cleanup_value_id;
    RxasFlowValueKind rejected_cleanup_kind;
    RxasFlowComponentPresence rejected_cleanup_presence;
    int preserve_candidate_register;
    RxasFlowUseKind rejected_use_kind;
    size_t rejected_use_record_id;
    size_t rejected_use_instruction_id;
    size_t rejected_use_operand_index;
    size_t rejected_use_value_id;
    size_t rewrite_offset;
    size_t rewrite_count;
} RxasFlowStringLiteralReusePlan;

typedef struct RxasFlowProofService RxasFlowProofService;

const RxasFlowProofService *rxas_flow_require_proof_service(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t work_budget);
/* Production consumers request the exact capability mask declared by their
 * optimisation route. Capabilities are acquired monotonically and cached for
 * the immutable procedure epoch. */
const RxasFlowProofService *rxas_flow_require_proof_capabilities(
        RxasFlowProcedure *procedure, unsigned long expected_epoch,
        unsigned int capabilities, size_t work_budget);
unsigned int rxas_flow_proof_capabilities(
        const RxasFlowProofService *service, unsigned long expected_epoch);
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
int rxas_flow_prove_redundant_self_copy(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t candidate_instruction, RxasFlowProofResult *result);
int rxas_flow_successful_guard_key(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t instruction_id, RxasFlowSuccessfulGuardKey *key);
/* A generator of RXAS_FLOW_ID_NONE asks for a direct exact producer proof;
 * otherwise the generator is an earlier successfully completed guard. */
int rxas_flow_prove_redundant_successful_guard(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t generator_instruction, size_t candidate_instruction,
        RxasFlowProofResult *result);
int rxas_flow_prove_typed_copy_redirect(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t candidate_instruction, RxasFlowTypedCopyPlan *plan);
int rxas_flow_typed_copy_plan_rewrite(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        const RxasFlowTypedCopyPlan *plan, size_t rewrite_index,
        RxasFlowOperandRewrite *rewrite);
int rxas_flow_prove_producer_destination_forward(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t producer_instruction, size_t copy_instruction,
        RxasFlowProducerDestinationPlan *plan);
int rxas_flow_prove_component_placement(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t copy_instruction, size_t derivation_instruction,
        RxasFlowComponentPlacementPlan *plan);
int rxas_flow_component_placement_plan_rewrite(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        const RxasFlowComponentPlacementPlan *plan, size_t rewrite_index,
        RxasFlowOperandRewrite *rewrite);
int rxas_flow_component_placement_plan_trace_deletion(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        const RxasFlowComponentPlacementPlan *plan, size_t deletion_index,
        RxasFlowTraceDeletion *deletion);
int rxas_flow_prove_compare_branch_fusion(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t compare_instruction, size_t branch_instruction,
        RxasFlowCompareBranchPlan *plan);
int rxas_flow_prove_duplicate_linked_read(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t first_link_instruction, size_t second_link_instruction,
        RxasFlowDuplicateLinkedReadPlan *plan);
int rxas_flow_prove_storage_permutation_round_trip(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t first_instruction, size_t second_instruction,
        RxasFlowStoragePermutationPlan *plan);
int rxas_flow_compare_branch_plan_trace_deletion(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        const RxasFlowCompareBranchPlan *plan, size_t deletion_index,
        RxasFlowTraceDeletion *deletion);
int rxas_flow_prove_joined_key_reuse(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t seed_instruction, size_t candidate_instruction,
        size_t stem_instruction, RxasFlowJoinedKeyReusePlan *plan);
int rxas_flow_joined_key_reuse_plan_seed_rewrite(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        const RxasFlowJoinedKeyReusePlan *plan, size_t rewrite_index,
        RxasFlowOperandRewrite *rewrite);
int rxas_flow_joined_key_reuse_plan_trace_deletion(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        const RxasFlowJoinedKeyReusePlan *plan, size_t deletion_index,
        RxasFlowTraceDeletion *deletion);
int rxas_flow_prove_string_literal_reuse(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        size_t seed_instruction, size_t candidate_instruction,
        RxasFlowStringLiteralReusePlan *plan);
int rxas_flow_string_literal_reuse_plan_rewrite(
        const RxasFlowProofService *service, unsigned long expected_epoch,
        const RxasFlowStringLiteralReusePlan *plan, size_t rewrite_index,
        RxasFlowOperandRewrite *rewrite);
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
