/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXAS_FLOW_REWRITE_H
#define CREXX_RXAS_FLOW_REWRITE_H

#include "rxas_flow_graph.h"

/* K05 branch threading is structural branch-edge algebra.  The immutable
 * planner deliberately does not request SSA, signal or value proofs. */
typedef enum RxasFlowBranchThreadKind {
    RXAS_FLOW_BRANCH_THREAD_NONE = 0,
    RXAS_FLOW_BRANCH_THREAD_BR_BRT,
    RXAS_FLOW_BRANCH_THREAD_BR_BRF,
    RXAS_FLOW_BRANCH_THREAD_BR_BRTF,
    RXAS_FLOW_BRANCH_THREAD_BRT_BRT,
    RXAS_FLOW_BRANCH_THREAD_BRF_BRF,
    RXAS_FLOW_BRANCH_THREAD_BRT_BRF,
    RXAS_FLOW_BRANCH_THREAD_BRF_BRT
} RxasFlowBranchThreadKind;

typedef struct RxasFlowBranchThreadPlan {
    unsigned long epoch;
    size_t expected_item_count;
    size_t source_record_id;
    size_t target_label_record_id;
    size_t target_record_id;
    RxasFlowBranchThreadKind kind;
    int expected_source_opcode;
    int expected_target_opcode;
    Assembler_Token *source_operands[2];
    size_t source_operand_count;
    Assembler_Token *target_operands[3];
    size_t target_operand_count;
} RxasFlowBranchThreadPlan;

typedef struct RxasFlowBranchThreadBatch {
    unsigned long epoch;
    size_t expected_item_count;
    RxasFlowBranchThreadPlan *plans;
    size_t plan_count;
    size_t plan_capacity;
} RxasFlowBranchThreadBatch;

/* Collect every source-order plan proved by one immutable graph epoch. */
int rxas_flow_plan_branch_threads(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        RxasFlowBranchThreadBatch *batch);

void rxas_flow_branch_thread_batch_destroy(
        RxasFlowBranchThreadBatch *batch);

/* Compatibility query used by focused consumers which need only the first
 * source-order plan.  Production mutation uses the complete batch. */
int rxas_flow_plan_branch_thread(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        RxasFlowBranchThreadPlan *plan);

/* Revalidate the complete immutable batch, rewrite all sources in place and
 * insert shared fallthrough labels in descending record order.  A zero return
 * means that no queued record was changed. */
size_t rxas_flow_apply_branch_threads(
        Assembler_Context *context, const RxasFlowBranchThreadBatch *batch);

/* Compatibility mutation for a single already-proved plan. */
int rxas_flow_apply_branch_thread(Assembler_Context *context,
                                  const RxasFlowBranchThreadPlan *plan);

#endif
