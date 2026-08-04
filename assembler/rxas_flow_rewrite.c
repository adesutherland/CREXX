/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

/* Immutable-CFG rewrite plans and their mutable queue consumers. */

#include "rxas_flow_rewrite.h"
#include "rxasassm.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int branch_thread_register_equal(const Assembler_Token *left,
                                        const Assembler_Token *right) {
    if (!left || !right || left->token_type != right->token_type) return 0;
    if (left->token_type != RREG && left->token_type != GREG &&
        left->token_type != AREG)
        return 0;
    return left->token_value.integer == right->token_value.integer;
}

static int branch_thread_label_equal(const Assembler_Token *left,
                                     const Assembler_Token *right) {
    if (!left || !right) return 0;
    if ((left->token_type != ID && left->token_type != LABEL) ||
        (right->token_type != ID && right->token_type != LABEL))
        return 0;
    return strcmp((const char *)left->token_value.string,
                  (const char *)right->token_value.string) == 0;
}

static int branch_thread_kind(int source_opcode, int target_opcode,
                              const instruction_queue *source,
                              const instruction_queue *target,
                              RxasFlowBranchThreadKind *kind) {
    Assembler_Token *source_condition;
    Assembler_Token *target_condition;
    if (!source || !target || !kind) return 0;
    *kind = RXAS_FLOW_BRANCH_THREAD_NONE;
    if (source_opcode == OP_BR_ID) {
        if (target_opcode == OP_BRT_ID_REG)
            *kind = RXAS_FLOW_BRANCH_THREAD_BR_BRT;
        else if (target_opcode == OP_BRF_ID_REG)
            *kind = RXAS_FLOW_BRANCH_THREAD_BR_BRF;
        else if (target_opcode == OP_BRTF_ID_ID_REG)
            *kind = RXAS_FLOW_BRANCH_THREAD_BR_BRTF;
        return *kind != RXAS_FLOW_BRANCH_THREAD_NONE;
    }
    if ((source_opcode != OP_BRT_ID_REG &&
         source_opcode != OP_BRF_ID_REG) ||
        (target_opcode != OP_BRT_ID_REG &&
         target_opcode != OP_BRF_ID_REG))
        return 0;
    source_condition = rxas_queue_operand(source, 1);
    target_condition = rxas_queue_operand(target, 1);
    if (!branch_thread_register_equal(source_condition, target_condition))
        return 0;
    if (source_opcode == OP_BRT_ID_REG &&
        target_opcode == OP_BRT_ID_REG)
        *kind = RXAS_FLOW_BRANCH_THREAD_BRT_BRT;
    else if (source_opcode == OP_BRF_ID_REG &&
             target_opcode == OP_BRF_ID_REG)
        *kind = RXAS_FLOW_BRANCH_THREAD_BRF_BRF;
    else if (source_opcode == OP_BRT_ID_REG &&
             target_opcode == OP_BRF_ID_REG)
        *kind = RXAS_FLOW_BRANCH_THREAD_BRT_BRF;
    else if (source_opcode == OP_BRF_ID_REG &&
             target_opcode == OP_BRT_ID_REG)
        *kind = RXAS_FLOW_BRANCH_THREAD_BRF_BRT;
    return *kind != RXAS_FLOW_BRANCH_THREAD_NONE;
}

static int branch_thread_is_noop(RxasFlowBranchThreadKind kind,
                                 const instruction_queue *source,
                                 const instruction_queue *target) {
    if (kind == RXAS_FLOW_BRANCH_THREAD_BRT_BRT ||
        kind == RXAS_FLOW_BRANCH_THREAD_BRF_BRF)
        return branch_thread_label_equal(rxas_queue_operand(source, 0),
                                         rxas_queue_operand(target, 0));
    return 0;
}

static int branch_thread_batch_append(
        RxasFlowBranchThreadBatch *batch,
        const RxasFlowBranchThreadPlan *plan) {
    RxasFlowBranchThreadPlan *plans;
    size_t capacity;
    if (!batch || !plan) return 0;
    if (batch->plan_count == batch->plan_capacity) {
        capacity = batch->plan_capacity ? batch->plan_capacity * 2 : 16;
        if (capacity < batch->plan_capacity) return 0;
        plans = realloc(batch->plans, capacity * sizeof(*plans));
        if (!plans) return 0;
        batch->plans = plans;
        batch->plan_capacity = capacity;
    }
    batch->plans[batch->plan_count++] = *plan;
    return 1;
}

void rxas_flow_branch_thread_batch_destroy(
        RxasFlowBranchThreadBatch *batch) {
    if (!batch) return;
    free(batch->plans);
    memset(batch, 0, sizeof(*batch));
}

int rxas_flow_plan_branch_threads(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        RxasFlowBranchThreadBatch *batch) {
    const RxasFlowMetrics *metrics;
    size_t instruction_id;
    if (!batch) return 0;
    memset(batch, 0, sizeof(*batch));
    metrics = rxas_flow_procedure_metrics(procedure, expected_epoch);
    if (!metrics || !metrics->complete_control_flow) return 0;
    batch->epoch = expected_epoch;
    batch->expected_item_count = metrics->records;

    for (instruction_id = 0; instruction_id < metrics->instructions;
         instruction_id++) {
        const RxasFlowInstruction *source_instruction;
        const RxasFlowInstruction *target_instruction;
        const RxasFlowRecord *target_label_record;
        const RxasFlowBlock *target_block;
        const instruction_queue *source;
        const instruction_queue *target;
        Assembler_Token *target_label;
        size_t target_label_record_id;
        RxasFlowBranchThreadKind kind;
        RxasFlowBranchThreadPlan plan;

        source_instruction = rxas_flow_procedure_instruction(
                procedure, expected_epoch, instruction_id);
        if (!source_instruction || !source_instruction->op ||
            source_instruction->signal.state != RXOP_SIGNAL_STATE_NONE)
            continue;
        if (source_instruction->op->opcode != OP_BR_ID &&
            source_instruction->op->opcode != OP_BRT_ID_REG &&
            source_instruction->op->opcode != OP_BRF_ID_REG)
            continue;
        source = rxas_flow_procedure_record(
                procedure, expected_epoch,
                source_instruction->record_id)->queue_record;
        if (!source || (source_instruction->op->opcode == OP_BR_ID
                        ? source->operandCount != 1
                        : source->operandCount != 2))
            continue;
        target_label = rxas_queue_operand(source, 0);
        target_label_record_id = rxas_flow_procedure_label_record(
                procedure, expected_epoch, target_label);
        if (target_label_record_id == RXAS_FLOW_ID_NONE) continue;
        target_label_record = rxas_flow_procedure_record(
                procedure, expected_epoch, target_label_record_id);
        if (!target_label_record ||
            target_label_record->block_id == RXAS_FLOW_ID_NONE)
            continue;
        target_block = rxas_flow_procedure_block(
                procedure, expected_epoch, target_label_record->block_id);
        if (!target_block ||
            target_block->first_instruction == RXAS_FLOW_ID_NONE)
            continue;
        target_instruction = rxas_flow_procedure_instruction(
                procedure, expected_epoch, target_block->first_instruction);
        if (!target_instruction || !target_instruction->op ||
            target_instruction->record_id == source_instruction->record_id ||
            target_instruction->signal.state != RXOP_SIGNAL_STATE_NONE)
            continue;
        target = rxas_flow_procedure_record(
                procedure, expected_epoch,
                target_instruction->record_id)->queue_record;
        if (!target || !branch_thread_kind(
                source_instruction->op->opcode,
                target_instruction->op->opcode, source, target, &kind) ||
            branch_thread_is_noop(kind, source, target))
            continue;

        memset(&plan, 0, sizeof(plan));
        plan.epoch = expected_epoch;
        plan.expected_item_count = metrics->records;
        plan.source_record_id = source_instruction->record_id;
        plan.target_label_record_id = target_label_record_id;
        plan.target_record_id = target_instruction->record_id;
        plan.kind = kind;
        plan.expected_source_opcode = source_instruction->op->opcode;
        plan.expected_target_opcode = target_instruction->op->opcode;
        plan.source_operand_count = source->operandCount;
        plan.source_operands[0] = rxas_queue_operand(source, 0);
        if (source->operandCount > 1)
            plan.source_operands[1] = rxas_queue_operand(source, 1);
        plan.target_operand_count = target->operandCount;
        plan.target_operands[0] = rxas_queue_operand(target, 0);
        plan.target_operands[1] = rxas_queue_operand(target, 1);
        if (target->operandCount > 2)
            plan.target_operands[2] = rxas_queue_operand(target, 2);
        if (!branch_thread_batch_append(batch, &plan)) {
            rxas_flow_branch_thread_batch_destroy(batch);
            return 0;
        }
    }
    return batch->plan_count != 0;
}

int rxas_flow_plan_branch_thread(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        RxasFlowBranchThreadPlan *plan) {
    RxasFlowBranchThreadBatch batch;
    int result;
    if (!plan) return 0;
    memset(plan, 0, sizeof(*plan));
    plan->source_record_id = RXAS_FLOW_ID_NONE;
    plan->target_label_record_id = RXAS_FLOW_ID_NONE;
    plan->target_record_id = RXAS_FLOW_ID_NONE;
    result = rxas_flow_plan_branch_threads(
            procedure, expected_epoch, &batch);
    if (result) *plan = batch.plans[0];
    rxas_flow_branch_thread_batch_destroy(&batch);
    return result;
}

static int branch_thread_operand_equal(const Assembler_Token *current,
                                       const Assembler_Token *expected) {
    if (current == expected) return 1;
    if (!current || !expected || current->token_type != expected->token_type)
        return 0;
    if (current->token_type == RREG || current->token_type == GREG ||
        current->token_type == AREG || current->token_type == INT)
        return current->token_value.integer == expected->token_value.integer;
    if (current->token_type == ID || current->token_type == LABEL)
        return strcmp((const char *)current->token_value.string,
                      (const char *)expected->token_value.string) == 0;
    return 0;
}

static int branch_thread_item_matches(Assembler_Context *context,
                                      const instruction_queue *item,
                                      int opcode,
                                      Assembler_Token *const *operands,
                                      size_t operand_count) {
    const OpInfo *op;
    size_t operand_index;
    if (!item || item->instrType != OP_CODE ||
        item->operandCount != operand_count)
        return 0;
    op = rxas_flow_resolve_opcode(context, item);
    if (!op || op->opcode != opcode) return 0;
    for (operand_index = 0; operand_index < operand_count; operand_index++)
        if (!branch_thread_operand_equal(rxas_queue_operand(item, operand_index),
                                         operands[operand_index]))
            return 0;
    return 1;
}

static int branch_thread_plan_revalidates(
        Assembler_Context *context, const RxasFlowBranchThreadPlan *plan) {
    instruction_queue *items;
    RxasFlowBranchThreadKind current_kind;
    size_t index;
    if (!context || !plan ||
        context->procedure_queue_items != plan->expected_item_count ||
        plan->source_record_id >= context->procedure_queue_items ||
        plan->target_label_record_id >= context->procedure_queue_items ||
        plan->target_record_id >= context->procedure_queue_items)
        return 0;
    items = context->procedure_queue;
    if (!branch_thread_item_matches(
            context, &items[plan->source_record_id],
            plan->expected_source_opcode, plan->source_operands,
            plan->source_operand_count) ||
        !branch_thread_item_matches(
            context, &items[plan->target_record_id],
            plan->expected_target_opcode, plan->target_operands,
            plan->target_operand_count) ||
        items[plan->target_label_record_id].instrType != ASM_LABEL ||
        !branch_thread_label_equal(
                items[plan->target_label_record_id].instrToken,
                plan->source_operands[0]) ||
        plan->target_label_record_id >= plan->target_record_id)
        return 0;
    if (!branch_thread_kind(
            plan->expected_source_opcode, plan->expected_target_opcode,
            &items[plan->source_record_id], &items[plan->target_record_id],
            &current_kind) || current_kind != plan->kind ||
        branch_thread_is_noop(current_kind,
                              &items[plan->source_record_id],
                              &items[plan->target_record_id]))
        return 0;
    for (index = plan->target_label_record_id + 1;
         index < plan->target_record_id; index++)
        if (items[index].instrType == OP_CODE) return 0;
    return 1;
}

static int branch_thread_needs_fallthrough_label(
        RxasFlowBranchThreadKind kind) {
    return kind == RXAS_FLOW_BRANCH_THREAD_BR_BRT ||
           kind == RXAS_FLOW_BRANCH_THREAD_BR_BRF ||
           kind == RXAS_FLOW_BRANCH_THREAD_BRT_BRF ||
           kind == RXAS_FLOW_BRANCH_THREAD_BRF_BRT;
}

typedef struct BranchThreadFallthrough {
    size_t target_record_id;
    Assembler_Token *branch_token;
    Assembler_Token *label_token;
} BranchThreadFallthrough;

static int branch_thread_fallthrough_compare(const void *left,
                                             const void *right) {
    const BranchThreadFallthrough *left_label;
    const BranchThreadFallthrough *right_label;
    left_label = left;
    right_label = right;
    if (left_label->target_record_id < right_label->target_record_id) return 1;
    if (left_label->target_record_id > right_label->target_record_id) return -1;
    return 0;
}

static BranchThreadFallthrough *branch_thread_find_fallthrough(
        BranchThreadFallthrough *labels, size_t label_count,
        size_t target_record_id) {
    size_t index;
    for (index = 0; index < label_count; index++)
        if (labels[index].target_record_id == target_record_id)
            return &labels[index];
    return 0;
}

static int branch_thread_rewrite_source(
        Assembler_Context *context, const RxasFlowBranchThreadPlan *plan,
        Assembler_Token *fallthrough_branch) {
    instruction_queue *source;
    Assembler_Token *replacement[3];
    const char *mnemonic;
    size_t replacement_count;

    if (!context || !plan) return 0;
    replacement_count = 2;
    mnemonic = plan->expected_source_opcode == OP_BR_ID ? "brtf" :
               plan->expected_source_opcode == OP_BRT_ID_REG ? "brt" : "brf";

    switch (plan->kind) {
        case RXAS_FLOW_BRANCH_THREAD_BR_BRT:
            replacement[0] = plan->target_operands[0];
            replacement[1] = fallthrough_branch;
            replacement[2] = plan->target_operands[1];
            replacement_count = 3;
            break;
        case RXAS_FLOW_BRANCH_THREAD_BR_BRF:
            replacement[0] = fallthrough_branch;
            replacement[1] = plan->target_operands[0];
            replacement[2] = plan->target_operands[1];
            replacement_count = 3;
            break;
        case RXAS_FLOW_BRANCH_THREAD_BR_BRTF:
            replacement[0] = plan->target_operands[0];
            replacement[1] = plan->target_operands[1];
            replacement[2] = plan->target_operands[2];
            replacement_count = 3;
            break;
        case RXAS_FLOW_BRANCH_THREAD_BRT_BRT:
        case RXAS_FLOW_BRANCH_THREAD_BRF_BRF:
            replacement[0] = plan->target_operands[0];
            replacement[1] = plan->source_operands[1];
            break;
        case RXAS_FLOW_BRANCH_THREAD_BRT_BRF:
        case RXAS_FLOW_BRANCH_THREAD_BRF_BRT:
            if (!fallthrough_branch) return 0;
            replacement[0] = fallthrough_branch;
            replacement[1] = plan->source_operands[1];
            break;
        default:
            return 0;
    }
    source = &context->procedure_queue[plan->source_record_id];
    source->instrToken = rxas_tid(context, source->instrToken, (char *)mnemonic);
    rxas_set_queue_operands(context, source, replacement, replacement_count);
    return 1;
}

size_t rxas_flow_apply_branch_threads(
        Assembler_Context *context, const RxasFlowBranchThreadBatch *batch) {
    BranchThreadFallthrough *labels;
    instruction_queue *items;
    size_t label_count;
    size_t plan_index;
    size_t label_index;
    char label_name[20];

    if (!context || !batch || !batch->plan_count || !batch->plans ||
        context->procedure_queue_items != batch->expected_item_count)
        return 0;
    for (plan_index = 0; plan_index < batch->plan_count; plan_index++)
        if (batch->plans[plan_index].epoch != batch->epoch ||
            !branch_thread_plan_revalidates(context,
                                            &batch->plans[plan_index]))
            return 0;

    labels = calloc(batch->plan_count, sizeof(*labels));
    if (!labels) return 0;
    label_count = 0;
    for (plan_index = 0; plan_index < batch->plan_count; plan_index++) {
        const RxasFlowBranchThreadPlan *plan;
        BranchThreadFallthrough *fallthrough;
        plan = &batch->plans[plan_index];
        if (!branch_thread_needs_fallthrough_label(plan->kind)) continue;
        fallthrough = branch_thread_find_fallthrough(
                labels, label_count, plan->target_record_id);
        if (fallthrough) continue;
        fallthrough = &labels[label_count++];
        fallthrough->target_record_id = plan->target_record_id;
        snprintf(label_name, sizeof(label_name), "%d",
                 context->optimiser_counter++);
        fallthrough->branch_token = rxas_tid(context, 0, label_name);
        fallthrough->label_token = rxas_tid(
                context, fallthrough->branch_token, label_name);
        fallthrough->label_token->token_type = LABEL;
    }

    rxas_reserve_procedure_queue(
            context, context->procedure_queue_items + label_count);
    for (plan_index = 0; plan_index < batch->plan_count; plan_index++) {
        const RxasFlowBranchThreadPlan *plan;
        BranchThreadFallthrough *fallthrough;
        plan = &batch->plans[plan_index];
        fallthrough = branch_thread_find_fallthrough(
                labels, label_count, plan->target_record_id);
        if (!branch_thread_rewrite_source(
                context, plan,
                fallthrough ? fallthrough->branch_token : 0)) {
            free(labels);
            return 0;
        }
    }

    qsort(labels, label_count, sizeof(*labels),
          branch_thread_fallthrough_compare);
    items = context->procedure_queue;
    for (label_index = 0; label_index < label_count; label_index++) {
        size_t insert_at;
        insert_at = labels[label_index].target_record_id + 1;
        if (context->procedure_queue_items > insert_at) {
            memmove(&items[insert_at + 1], &items[insert_at],
                    (context->procedure_queue_items - insert_at) *
                    sizeof(*items));
        }
        context->procedure_queue_items++;
        memset(&items[insert_at], 0, sizeof(*items));
        items[insert_at].instrType = ASM_LABEL;
        items[insert_at].instrToken = labels[label_index].label_token;
    }
    free(labels);
    return batch->plan_count;
}

int rxas_flow_apply_branch_thread(Assembler_Context *context,
                                  const RxasFlowBranchThreadPlan *plan) {
    RxasFlowBranchThreadBatch batch;
    if (!plan) return 0;
    memset(&batch, 0, sizeof(batch));
    batch.epoch = plan->epoch;
    batch.expected_item_count = plan->expected_item_count;
    batch.plans = (RxasFlowBranchThreadPlan *)plan;
    batch.plan_count = 1;
    batch.plan_capacity = 1;
    return rxas_flow_apply_branch_threads(context, &batch) == 1;
}
