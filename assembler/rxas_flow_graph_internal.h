/* Internal ownership shared by RXAS flow construction and cached analyses. */

#ifndef CREXX_RXAS_FLOW_GRAPH_INTERNAL_H
#define CREXX_RXAS_FLOW_GRAPH_INTERNAL_H

#include "rxas_flow_graph.h"

typedef struct RxasFlowLabelSlot {
    const char *name;
    size_t record_id;
} RxasFlowLabelSlot;

struct RxasFlowStructuralAnalysis;
struct RxasFlowSignalAnalysis;

struct RxasFlowAnalysisManager {
    unsigned long epoch;
    size_t structural_budget;
    struct RxasFlowStructuralAnalysis *structural;
    size_t signal_budget;
    struct RxasFlowSignalAnalysis *signal;
};

struct RxasFlowProcedure {
    unsigned long epoch;
    char *name;
    const instruction_queue *items;
    size_t item_count;
    RxasFlowRecord *records;
    RxasFlowInstruction *instructions;
    RxasFlowBlock *blocks;
    RxasFlowEdge *edges;
    size_t edge_capacity;
    RxasFlowLabelSlot *labels;
    size_t label_capacity;
    size_t entry_block;
    size_t handler_root;
    size_t async_root;
    size_t normal_exit;
    size_t unwind_exit;
    size_t terminal_exit;
    size_t unknown_exit;
    RxasFlowMetrics metrics;
    struct RxasFlowAnalysisManager *analysis_manager;
};

#endif
