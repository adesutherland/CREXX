/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXAS_FLOW_GRAPH_H
#define CREXX_RXAS_FLOW_GRAPH_H

#include <stddef.h>
#include <stdio.h>

#include "rxas.h"
#include "rxdefs.h"

/* Internal immutable control-flow representation.  It deliberately exposes
 * only read-only descriptors; rewrite consumers operate on a later plan and
 * must rebuild with a new epoch after mutating the queued procedure. */

#define RXAS_FLOW_ID_NONE ((size_t)-1)

typedef enum RxasFlowEdgeKind {
    RXAS_FLOW_EDGE_NORMAL = 0,
    RXAS_FLOW_EDGE_BRANCH,
    RXAS_FLOW_EDGE_SIGNAL_SKIP,
    RXAS_FLOW_EDGE_HANDLER,
    RXAS_FLOW_EDGE_TERMINAL,
    RXAS_FLOW_EDGE_UNWIND,
    RXAS_FLOW_EDGE_UNKNOWN
} RxasFlowEdgeKind;

typedef enum RxasFlowBlockKind {
    RXAS_FLOW_BLOCK_CODE = 0,
    RXAS_FLOW_BLOCK_ENTRY,
    RXAS_FLOW_BLOCK_HANDLER_ROOT,
    RXAS_FLOW_BLOCK_ASYNC_ROOT,
    RXAS_FLOW_BLOCK_NORMAL_EXIT,
    RXAS_FLOW_BLOCK_UNWIND_EXIT,
    RXAS_FLOW_BLOCK_TERMINAL_EXIT,
    RXAS_FLOW_BLOCK_UNKNOWN_EXIT
} RxasFlowBlockKind;

typedef struct RxasFlowRecord {
    size_t id;
    size_t instruction_id;
    size_t block_id;
    size_t emitted_address;
    enum queue_item_type type;
    const instruction_queue *queue_record;
    const Assembler_Token *source_token;
} RxasFlowRecord;

typedef struct RxasFlowInstruction {
    size_t id;
    size_t record_id;
    size_t block_id;
    size_t emitted_address;
    const OpInfo *op;
    RxOpEffects effects;
    RxOpSignalContract signal;
} RxasFlowInstruction;

typedef struct RxasFlowBlock {
    size_t id;
    RxasFlowBlockKind kind;
    size_t first_record;
    size_t last_record;
    size_t first_instruction;
    size_t last_instruction;
} RxasFlowBlock;

typedef struct RxasFlowEdge {
    size_t source;
    size_t target;
    RxasFlowEdgeKind kind;
} RxasFlowEdge;

typedef struct RxasFlowMetrics {
    size_t records;
    size_t instructions;
    size_t code_blocks;
    size_t blocks;
    size_t edges;
    size_t labels;
    size_t resolved_targets;
    size_t unresolved_targets;
    size_t normal_edges;
    size_t branch_edges;
    size_t signal_skip_edges;
    size_t handler_edges;
    size_t terminal_edges;
    size_t unwind_edges;
    size_t unknown_edges;
    int complete_control_flow;
} RxasFlowMetrics;

typedef struct RxasFlowProcedure RxasFlowProcedure;

/* Shared source-opcode resolver used by the legacy rewrite graph and the new
 * immutable graph.  A null result is conservative unknown control/effects. */
const OpInfo *rxas_flow_resolve_opcode(Assembler_Context *context,
                                       const instruction_queue *item);

RxasFlowProcedure *rxas_flow_procedure_build(Assembler_Context *context,
                                              const instruction_queue *items,
                                              size_t item_count,
                                              unsigned long epoch);
/* Integration form used when procedure parsing has already resolved each
 * queued record. Entries for non-opcode records are null. */
RxasFlowProcedure *rxas_flow_procedure_build_resolved(
        Assembler_Context *context, const instruction_queue *items,
        size_t item_count, unsigned long epoch,
        const OpInfo *const *resolved_ops);
void rxas_flow_procedure_destroy(RxasFlowProcedure *procedure);

unsigned long rxas_flow_procedure_epoch(const RxasFlowProcedure *procedure);
int rxas_flow_procedure_epoch_matches(const RxasFlowProcedure *procedure,
                                      unsigned long expected_epoch);
const RxasFlowMetrics *rxas_flow_procedure_metrics(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch);
const RxasFlowRecord *rxas_flow_procedure_record(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t record_id);
const RxasFlowInstruction *rxas_flow_procedure_instruction(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t instruction_id);
const RxasFlowBlock *rxas_flow_procedure_block(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t block_id);
const RxasFlowEdge *rxas_flow_procedure_edge(
        const RxasFlowProcedure *procedure, unsigned long expected_epoch,
        size_t edge_id);

size_t rxas_flow_procedure_entry_block(const RxasFlowProcedure *procedure,
                                       unsigned long expected_epoch);
size_t rxas_flow_procedure_handler_root(const RxasFlowProcedure *procedure,
                                        unsigned long expected_epoch);
size_t rxas_flow_procedure_async_root(const RxasFlowProcedure *procedure,
                                      unsigned long expected_epoch);
size_t rxas_flow_procedure_normal_exit(const RxasFlowProcedure *procedure,
                                       unsigned long expected_epoch);
size_t rxas_flow_procedure_unwind_exit(const RxasFlowProcedure *procedure,
                                       unsigned long expected_epoch);
size_t rxas_flow_procedure_terminal_exit(const RxasFlowProcedure *procedure,
                                         unsigned long expected_epoch);
size_t rxas_flow_procedure_unknown_exit(const RxasFlowProcedure *procedure,
                                        unsigned long expected_epoch);

/* Returns zero on a stale epoch or invalid stream. */
int rxas_flow_procedure_dump(const RxasFlowProcedure *procedure,
                             unsigned long expected_epoch,
                             FILE *stream);

#endif
