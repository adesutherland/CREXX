/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxas_flow_batch.h"
#include "rxasassm.h"

#include <stdlib.h>
#include <string.h>

static int queue_operands_equal(const instruction_queue *left,
                                const instruction_queue *right) {
    size_t operand;
    if (left->operandCount != right->operandCount) return 0;
    for (operand = 0; operand < left->operandCount; operand++)
        if (rxas_queue_operand(left, operand) !=
            rxas_queue_operand(right, operand))
            return 0;
    return 1;
}

static int queue_named_operands_equal(const instruction_queue *left,
                                      const instruction_queue *right) {
    return left->operand1Token == right->operand1Token &&
           left->operand2Token == right->operand2Token &&
           left->operand3Token == right->operand3Token &&
           left->operand4Token == right->operand4Token &&
           left->operand5Token == right->operand5Token &&
           left->operand6Token == right->operand6Token &&
           left->operand7Token == right->operand7Token &&
           left->operand8Token == right->operand8Token &&
           left->operand9Token == right->operand9Token &&
           left->operand10Token == right->operand10Token;
}

static int queue_trace_register_rewrite_valid(
        const instruction_queue *original,
        const instruction_queue *planned) {
    return original && planned &&
           original->instrType == TRACE_EVENT &&
           planned->instrType == TRACE_EVENT &&
           original->instrToken == planned->instrToken &&
           original->operand1Token == planned->operand1Token &&
           original->operand2Token == planned->operand2Token &&
           original->operand3Token == planned->operand3Token &&
           original->operand4Token == planned->operand4Token &&
           original->operand6Token == planned->operand6Token &&
           original->operand7Token == planned->operand7Token &&
           original->operand8Token == planned->operand8Token &&
           original->operand9Token == planned->operand9Token &&
           original->operand10Token == planned->operand10Token &&
           original->operand2Token &&
           original->operand2Token->token_type == STRING &&
           !strcmp((const char *)original->operand2Token->token_value.string,
                   "R") &&
           original->operand4Token && planned->operand4Token &&
           original->operand4Token->token_type == STRING &&
           planned->operand4Token->token_type == STRING &&
           !strcmp((const char *)original->operand4Token->token_value.string,
                   (const char *)planned->operand4Token->token_value.string) &&
           original->operand5Token && planned->operand5Token &&
           original->operand5Token->token_type == INT &&
           planned->operand5Token->token_type == INT;
}

static int queue_record_equal(const instruction_queue *left,
                              const instruction_queue *right) {
    return left->instrType == right->instrType &&
           left->instrToken == right->instrToken &&
           queue_operands_equal(left, right) &&
           queue_named_operands_equal(left, right);
}

static int queue_opcode_named_operands_valid(
        const instruction_queue *item) {
    if (item->instrType != OP_CODE) return 1;
    if (item->operandCount && !item->operandTokens) return 0;
    return item->operand1Token == rxas_queue_operand(item, 0) &&
           item->operand2Token == rxas_queue_operand(item, 1) &&
           item->operand3Token == rxas_queue_operand(item, 2) &&
           item->operand4Token == rxas_queue_operand(item, 3) &&
           item->operand5Token == rxas_queue_operand(item, 4) &&
           item->operand6Token == rxas_queue_operand(item, 5) &&
           item->operand7Token == rxas_queue_operand(item, 6) &&
           item->operand8Token == rxas_queue_operand(item, 7) &&
           item->operand9Token == rxas_queue_operand(item, 8) &&
           item->operand10Token == rxas_queue_operand(item, 9);
}

static void queue_clone(Assembler_Context *context,
                        instruction_queue *destination,
                        const instruction_queue *source) {
    *destination = *source;
    destination->operandTokens = 0;
    if (!source->operandCount) return;
    destination->operandTokens = malloc(
            source->operandCount * sizeof(*destination->operandTokens));
    if (!destination->operandTokens)
        RX_PANIC_OOM("malloc RXAS semantic batch operands",
                     source->operandCount *
                     sizeof(*destination->operandTokens),
                     context && context->file_name ? context->file_name : 0);
    memcpy(destination->operandTokens, source->operandTokens,
           source->operandCount * sizeof(*destination->operandTokens));
}

static size_t queue_batch_find_index(
        const RxasFlowQueueBatch *batch, size_t record_id) {
    size_t entry;
    if (!batch) return (size_t)-1;
    for (entry = 0; entry < batch->entry_count; entry++)
        if (batch->entries[entry].record_id == record_id)
            return entry;
    return (size_t)-1;
}

static void queue_batch_rollback(RxasFlowQueueBatch *batch) {
    size_t entry;
    if (!batch || !batch->original_items) return;
    for (entry = 0; entry < batch->entry_count; entry++) {
        instruction_queue *live;
        live = &batch->original_items[batch->entries[entry].record_id];
        rxas_free_queue_item(live);
        *live = *batch->entries[entry].original;
        batch->entries[entry].original->operandTokens = 0;
        batch->entries[entry].original->operandCount = 0;
    }
}

int rxas_flow_queue_batch_begin(
        RxasFlowQueueBatch *batch, Assembler_Context *context,
        instruction_queue *items, size_t item_count) {
    if (!batch || !context || (!items && item_count)) return 0;
    memset(batch, 0, sizeof(*batch));
    batch->context = context;
    batch->original_items = items;
    batch->item_count = item_count;
    batch->active = 1;
    return 1;
}

instruction_queue *rxas_flow_queue_batch_edit(
        RxasFlowQueueBatch *batch, size_t record_id,
        const instruction_queue **epoch_item) {
    RxasFlowQueueBatchEntry *entry;
    size_t entry_index;
    size_t new_capacity;
    RxasFlowQueueBatchEntry *new_entries;
    if (epoch_item) *epoch_item = 0;
    if (!batch || !batch->active || batch->finished ||
        record_id >= batch->item_count)
        return 0;
    entry_index = queue_batch_find_index(batch, record_id);
    if (entry_index == (size_t)-1) {
        if (batch->entry_count == batch->entry_capacity) {
            new_capacity = batch->entry_capacity
                    ? batch->entry_capacity * 2 : 16;
            new_entries = realloc(
                    batch->entries,
                    new_capacity * sizeof(*new_entries));
            if (!new_entries)
                RX_PANIC_OOM("realloc RXAS semantic batch records",
                             new_capacity * sizeof(*new_entries),
                             batch->context && batch->context->file_name
                                     ? batch->context->file_name : 0);
            batch->entries = new_entries;
            batch->entry_capacity = new_capacity;
        }
        entry = &batch->entries[batch->entry_count++];
        memset(entry, 0, sizeof(*entry));
        entry->record_id = record_id;
        entry->original = calloc(1, sizeof(*entry->original));
        if (!entry->original)
            RX_PANIC_OOM("calloc RXAS semantic batch record",
                         sizeof(*entry->original),
                         batch->context && batch->context->file_name
                                 ? batch->context->file_name : 0);
        queue_clone(batch->context, entry->original,
                    &batch->original_items[record_id]);
    }
    else entry = &batch->entries[entry_index];
    if (epoch_item) *epoch_item = entry->original;
    return &batch->original_items[record_id];
}

int rxas_flow_queue_batch_record_matches_epoch(
        const RxasFlowQueueBatch *batch, size_t record_id) {
    if (!batch || !batch->active || record_id >= batch->item_count)
        return 0;
    return queue_batch_find_index(batch, record_id) == (size_t)-1;
}

int rxas_flow_queue_batch_commit(
        RxasFlowQueueBatch *batch, RxasFlowQueueBatchMetrics *metrics) {
    size_t entry_index;
    RxasFlowQueueBatchMetrics result;
    int valid;
    if (metrics) memset(metrics, 0, sizeof(*metrics));
    if (!batch || !batch->active || batch->finished || !batch->context ||
        batch->context->procedure_queue != batch->original_items ||
        batch->context->procedure_queue_items != batch->item_count)
        return 0;
    memset(&result, 0, sizeof(result));
    valid = 1;

    for (entry_index = 0; entry_index < batch->entry_count; entry_index++) {
        const instruction_queue *original;
        const instruction_queue *planned;
        int opcode_changed;
        int operands_changed;
        int named_operands_changed;
        int trace_register_rewrite;
        original = batch->entries[entry_index].original;
        planned = &batch->original_items[
                batch->entries[entry_index].record_id];
        if (queue_record_equal(original, planned)) continue;
        if (!queue_opcode_named_operands_valid(planned)) {
            valid = 0;
            break;
        }
        if (original->instrType != planned->instrType &&
            planned->instrType != EMPTY) {
            valid = 0;
            break;
        }
        opcode_changed = original->instrToken != planned->instrToken;
        operands_changed = !queue_operands_equal(original, planned);
        named_operands_changed =
                !queue_named_operands_equal(original, planned);
        trace_register_rewrite = named_operands_changed &&
                !opcode_changed && !operands_changed &&
                queue_trace_register_rewrite_valid(original, planned);
        if ((opcode_changed || operands_changed) &&
            (original->instrType != OP_CODE ||
             planned->instrType != OP_CODE)) {
            valid = 0;
            break;
        }
        if (original->instrType == planned->instrType &&
            !opcode_changed && !operands_changed &&
            named_operands_changed && !trace_register_rewrite) {
            valid = 0;
            break;
        }
        result.records_changed++;
        if (planned->instrType == EMPTY) result.records_deleted++;
        if (opcode_changed) result.opcodes_replaced++;
        if (operands_changed || trace_register_rewrite)
            result.operand_records_rewritten++;
    }
    if (!valid) queue_batch_rollback(batch);
    batch->finished = 1;
    if (!valid) return 0;
    if (metrics) *metrics = result;
    return 1;
}

void rxas_flow_queue_batch_destroy(RxasFlowQueueBatch *batch) {
    size_t entry;
    if (!batch) return;
    if (batch->active && !batch->finished)
        queue_batch_rollback(batch);
    for (entry = 0; entry < batch->entry_count; entry++) {
        rxas_free_queue_item(batch->entries[entry].original);
        free(batch->entries[entry].original);
    }
    free(batch->entries);
    memset(batch, 0, sizeof(*batch));
}
