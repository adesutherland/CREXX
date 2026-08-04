/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXAS_FLOW_BATCH_H
#define CREXX_RXAS_FLOW_BATCH_H

#include "rxas.h"

typedef struct RxasFlowQueueBatchEntry {
    size_t record_id;
    instruction_queue original;
} RxasFlowQueueBatchEntry;

/* A queue batch is a sparse copy-on-write transaction over one immutable
 * analysis epoch.  Only edited records are snapshotted.  The owning flow
 * graph pins proof-facing records to those snapshots while later consumers
 * see the provisional live queue in established priority order. */
typedef struct RxasFlowQueueBatch {
    Assembler_Context *context;
    instruction_queue *original_items;
    size_t item_count;
    RxasFlowQueueBatchEntry *entries;
    size_t entry_count;
    size_t entry_capacity;
    int active;
    int finished;
} RxasFlowQueueBatch;

typedef struct RxasFlowQueueBatchMetrics {
    size_t records_changed;
    size_t records_deleted;
    size_t opcodes_replaced;
    size_t operand_records_rewritten;
} RxasFlowQueueBatchMetrics;

int rxas_flow_queue_batch_begin(
        RxasFlowQueueBatch *batch, Assembler_Context *context,
        instruction_queue *items, size_t item_count);

/* Register a record before its first provisional edit and return the live
 * queue item.  epoch_item receives the stable snapshot used by immutable
 * proof queries. */
instruction_queue *rxas_flow_queue_batch_edit(
        RxasFlowQueueBatch *batch, size_t record_id,
        const instruction_queue **epoch_item);

/* True only while the record has not been registered for provisional edit.
 * Delete-only proof consumers use this conservative claim to avoid applying
 * an old fact to a record owned by an earlier consumer in the same batch. */
int rxas_flow_queue_batch_record_matches_epoch(
        const RxasFlowQueueBatch *batch, size_t record_id);

/* Returns non-zero after a fully validated commit, including an empty commit.
 * A zero result rolls every registered edit back before returning. */
int rxas_flow_queue_batch_commit(
        RxasFlowQueueBatch *batch, RxasFlowQueueBatchMetrics *metrics);

void rxas_flow_queue_batch_destroy(RxasFlowQueueBatch *batch);

#endif
