/* Focused executable contract for the immutable PERF3 RXAS flow graph. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxas_flow_graph.h"
#include "rxas_flow_batch.h"
#include "rxas_flow_pass.h"
#include "rxas_flow_analysis.h"
#include "rxas_flow_proof.h"
#include "rxas_flow_rewrite.h"
#include "rxas_flow_signal.h"
#include "rxas_flow_ssa.h"
#include "rxas_flow_use.h"
#include "rxasgrmr.h"

#define FIXTURE_MAX_ITEMS 256
#define FIXTURE_MAX_TOKENS 512

typedef struct FlowFixture {
    instruction_queue items[FIXTURE_MAX_ITEMS];
    size_t item_count;
    Assembler_Token *tokens[FIXTURE_MAX_TOKENS];
    size_t token_count;
} FlowFixture;

static int failures;

static void check(int condition, const char *message) {
    if (condition) return;
    fprintf(stderr, "%s\n", message);
    failures++;
}

static Assembler_Token *fixture_text_token(FlowFixture *fixture, int type,
                                           const char *text) {
    Assembler_Token *token;
    size_t length;
    length = strlen(text);
    token = calloc(1, sizeof(*token) + length);
    if (!token || fixture->token_count >= FIXTURE_MAX_TOKENS) exit(2);
    token->token_type = type;
    memcpy(token->token_value.string, text, length + 1);
    fixture->tokens[fixture->token_count++] = token;
    return token;
}

static Assembler_Token *fixture_integer_token(FlowFixture *fixture, int type,
                                              rxinteger value) {
    Assembler_Token *token;
    token = calloc(1, sizeof(*token));
    if (!token || fixture->token_count >= FIXTURE_MAX_TOKENS) exit(2);
    token->token_type = type;
    token->token_value.integer = value;
    fixture->tokens[fixture->token_count++] = token;
    return token;
}

static Assembler_Token *fixture_label_ref(FlowFixture *fixture,
                                          const char *name) {
    return fixture_text_token(fixture, ID, name);
}

static Assembler_Token *fixture_register(FlowFixture *fixture,
                                         rxinteger number) {
    return fixture_integer_token(fixture, RREG, number);
}

static Assembler_Token *fixture_integer(FlowFixture *fixture,
                                        rxinteger value) {
    return fixture_integer_token(fixture, INT, value);
}

static Assembler_Token *fixture_float(FlowFixture *fixture, double value) {
    Assembler_Token *token;
    token = calloc(1, sizeof(*token));
    if (!token || fixture->token_count >= FIXTURE_MAX_TOKENS) exit(2);
    token->token_type = FLOAT;
    token->token_value.real = value;
    fixture->tokens[fixture->token_count++] = token;
    return token;
}

static Assembler_Token *fixture_string(FlowFixture *fixture,
                                       const char *value) {
    return fixture_text_token(fixture, STRING, value);
}

static void fixture_label(FlowFixture *fixture, const char *name) {
    instruction_queue *item;
    if (fixture->item_count >= FIXTURE_MAX_ITEMS) exit(2);
    item = &fixture->items[fixture->item_count++];
    memset(item, 0, sizeof(*item));
    item->instrType = ASM_LABEL;
    item->instrToken = fixture_text_token(fixture, LABEL, name);
}

static void fixture_record(FlowFixture *fixture, enum queue_item_type type) {
    instruction_queue *item;
    if (fixture->item_count >= FIXTURE_MAX_ITEMS) exit(2);
    item = &fixture->items[fixture->item_count++];
    memset(item, 0, sizeof(*item));
    item->instrType = type;
    item->instrToken = fixture_text_token(fixture, ID, "anchor");
}

static void fixture_trace_register(FlowFixture *fixture,
                                   rxinteger register_number,
                                   const char *value_type) {
    instruction_queue *trace;
    fixture_record(fixture, TRACE_EVENT);
    trace = &fixture->items[fixture->item_count - 1];
    trace->operand2Token = fixture_string(fixture, "R");
    trace->operand3Token = fixture_string(fixture, value_type);
    trace->operand4Token = fixture_string(fixture, "r");
    trace->operand5Token = fixture_integer(fixture, register_number);
}

static void fixture_op(FlowFixture *fixture, const char *mnemonic,
                       Assembler_Token **operands, size_t operand_count) {
    instruction_queue *item;
    size_t index;
    if (fixture->item_count >= FIXTURE_MAX_ITEMS) exit(2);
    item = &fixture->items[fixture->item_count++];
    memset(item, 0, sizeof(*item));
    item->instrType = OP_CODE;
    item->instrToken = fixture_text_token(fixture, ID, mnemonic);
    item->operandCount = operand_count;
    if (operand_count) {
        item->operandTokens = malloc(operand_count * sizeof(*item->operandTokens));
        if (!item->operandTokens) exit(2);
        for (index = 0; index < operand_count; index++)
            item->operandTokens[index] = operands[index];
        item->operand1Token = operands[0];
        if (operand_count > 1) item->operand2Token = operands[1];
        if (operand_count > 2) item->operand3Token = operands[2];
    }
}

static void fixture_destroy(FlowFixture *fixture) {
    size_t index;
    for (index = 0; index < fixture->item_count; index++)
        free(fixture->items[index].operandTokens);
    for (index = 0; index < fixture->token_count; index++)
        free(fixture->tokens[index]);
    memset(fixture, 0, sizeof(*fixture));
}

static size_t fixture_block_for_record(const RxasFlowProcedure *procedure,
                                       unsigned long epoch,
                                       size_t record_id) {
    const RxasFlowRecord *record;
    record = rxas_flow_procedure_record(procedure, epoch, record_id);
    return record ? record->block_id : RXAS_FLOW_ID_NONE;
}

static int fixture_has_edge(const RxasFlowProcedure *procedure,
                            unsigned long epoch, size_t source, size_t target,
                            RxasFlowEdgeKind kind) {
    const RxasFlowMetrics *metrics;
    size_t index;
    metrics = rxas_flow_procedure_metrics(procedure, epoch);
    if (!metrics) return 0;
    for (index = 0; index < metrics->edges; index++) {
        const RxasFlowEdge *edge;
        edge = rxas_flow_procedure_edge(procedure, epoch, index);
        if (edge && edge->source == source && edge->target == target &&
            edge->kind == kind)
            return 1;
    }
    return 0;
}

static size_t fixture_edge_id(const RxasFlowProcedure *procedure,
                              unsigned long epoch, size_t source,
                              size_t target, RxasFlowEdgeKind kind) {
    const RxasFlowMetrics *metrics;
    size_t index;
    metrics = rxas_flow_procedure_metrics(procedure, epoch);
    if (!metrics) return RXAS_FLOW_ID_NONE;
    for (index = 0; index < metrics->edges; index++) {
        const RxasFlowEdge *edge;
        edge = rxas_flow_procedure_edge(procedure, epoch, index);
        if (edge && edge->source == source && edge->target == target &&
            edge->kind == kind)
            return index;
    }
    return RXAS_FLOW_ID_NONE;
}

static int fixture_path_exists(const RxasFlowProcedure *procedure,
                               unsigned long epoch, size_t source,
                               size_t target) {
    const RxasFlowMetrics *metrics;
    unsigned char *seen;
    size_t *queue;
    size_t head;
    size_t tail;
    size_t edge_index;
    int found;
    metrics = rxas_flow_procedure_metrics(procedure, epoch);
    if (!metrics || source >= metrics->blocks || target >= metrics->blocks)
        return 0;
    seen = calloc(metrics->blocks, 1);
    queue = malloc(metrics->blocks * sizeof(*queue));
    if (!seen || !queue) exit(2);
    head = 0;
    tail = 0;
    queue[tail++] = source;
    seen[source] = 1;
    found = 0;
    while (head < tail && !found) {
        size_t current;
        current = queue[head++];
        if (current == target) {
            found = 1;
            break;
        }
        for (edge_index = 0; edge_index < metrics->edges; edge_index++) {
            const RxasFlowEdge *edge;
            edge = rxas_flow_procedure_edge(procedure, epoch, edge_index);
            if (edge->source == current && !seen[edge->target]) {
                seen[edge->target] = 1;
                queue[tail++] = edge->target;
            }
        }
    }
    free(seen);
    free(queue);
    return found;
}

static char *fixture_read_stream(FILE *stream) {
    char *buffer;
    long length;
    fflush(stream);
    if (fseek(stream, 0, SEEK_END) != 0) exit(2);
    length = ftell(stream);
    if (length < 0 || fseek(stream, 0, SEEK_SET) != 0) exit(2);
    buffer = malloc((size_t)length + 1);
    if (!buffer) exit(2);
    if (length && fread(buffer, 1, (size_t)length, stream) != (size_t)length)
        exit(2);
    buffer[length] = 0;
    fclose(stream);
    return buffer;
}

static char *fixture_dump(const RxasFlowProcedure *procedure,
                          unsigned long epoch) {
    FILE *stream;
    stream = tmpfile();
    if (!stream) exit(2);
    if (!rxas_flow_procedure_dump(procedure, epoch, stream)) exit(2);
    return fixture_read_stream(stream);
}

static char *fixture_analysis_dump(
        const RxasFlowStructuralAnalysis *analysis, unsigned long epoch) {
    FILE *stream;
    stream = tmpfile();
    if (!stream) exit(2);
    if (!rxas_flow_structural_dump(analysis, epoch, stream)) exit(2);
    return fixture_read_stream(stream);
}

static char *fixture_signal_dump(
        const RxasFlowSignalAnalysis *analysis, unsigned long epoch) {
    FILE *stream;
    stream = tmpfile();
    if (!stream) exit(2);
    if (!rxas_flow_signal_dump(analysis, epoch, stream)) exit(2);
    return fixture_read_stream(stream);
}

static char *fixture_ssa_dump(
        const RxasFlowSsaAnalysis *analysis, unsigned long epoch) {
    FILE *stream;
    stream = tmpfile();
    if (!stream) exit(2);
    if (!rxas_flow_ssa_dump(analysis, epoch, stream)) exit(2);
    return fixture_read_stream(stream);
}

static char *fixture_use_dump(
        const RxasFlowUseAnalysis *analysis, unsigned long epoch) {
    FILE *stream;
    stream = tmpfile();
    if (!stream) exit(2);
    if (!rxas_flow_use_dump(analysis, epoch, stream)) exit(2);
    return fixture_read_stream(stream);
}

static int fixture_value_reaches_value(
        const RxasFlowUseAnalysis *analysis, unsigned long epoch,
        size_t source_value, size_t target_value) {
    size_t stack[FIXTURE_MAX_TOKENS * 4];
    size_t seen[FIXTURE_MAX_TOKENS * 4];
    size_t stack_count;
    size_t seen_count;
    size_t index;
    stack_count = 0;
    seen_count = 0;
    stack[stack_count++] = source_value;
    while (stack_count) {
        size_t value;
        size_t dependent_count;
        value = stack[--stack_count];
        if (value == target_value) return 1;
        for (index = 0; index < seen_count; index++)
            if (seen[index] == value) break;
        if (index < seen_count) continue;
        if (seen_count >= sizeof(seen) / sizeof(seen[0])) return 0;
        seen[seen_count++] = value;
        dependent_count = rxas_flow_value_dependent_count(
                analysis, epoch, value);
        for (index = 0; index < dependent_count; index++) {
            size_t dependent;
            dependent = rxas_flow_value_dependent(
                    analysis, epoch, value, index);
            if (dependent == RXAS_FLOW_ID_NONE ||
                stack_count >= sizeof(stack) / sizeof(stack[0]))
                return 0;
            stack[stack_count++] = dependent;
        }
    }
    return 0;
}

static void test_unreachable_and_mapping(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[2];
    const RxasFlowRecord *record;
    const RxasFlowStructuralAnalysis *analysis;
    const RxasFlowStructuralMetrics *structural_metrics;
    const RxasFlowSignalAnalysis *signal_analysis;
    const RxasFlowSignalMetrics *signal_metrics;
    size_t dead_block;
    size_t live_block;
    size_t entry;
    memset(&fixture, 0, sizeof(fixture));
    fixture_label(&fixture, "entry");
    operands[0] = fixture_label_ref(&fixture, "live");
    fixture_op(&fixture, "br", operands, 1);
    fixture_label(&fixture, "dead");
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "load", operands, 2);
    fixture_op(&fixture, "ret", 0, 0);
    fixture_label(&fixture, "live");
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 2);
    fixture_op(&fixture, "load", operands, 2);
    fixture_op(&fixture, "ret", 0, 0);

    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 1);
    check(procedure != 0, "unreachable graph construction failed");
    if (procedure) {
        check(rxas_flow_require_structural_analysis(procedure, 1, 1) == 0,
              "bounded structural analysis did not fail closed");
        structural_metrics = rxas_flow_last_structural_metrics(procedure, 1);
        check(structural_metrics && structural_metrics->status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED,
              "bounded structural analysis did not report budget exhaustion");
        analysis = rxas_flow_require_structural_analysis(procedure, 1, 0);
        check(analysis != 0,
              "structural analysis did not recover with the default budget");
        structural_metrics = rxas_flow_structural_metrics(analysis, 1);
        dead_block = fixture_block_for_record(procedure, 1, 2);
        live_block = fixture_block_for_record(procedure, 1, 5);
        entry = rxas_flow_procedure_entry_block(procedure, 1);
        check(!fixture_path_exists(procedure, 1, entry, dead_block),
              "unreachable block acquired an entry path");
        check(fixture_path_exists(procedure, 1, entry, live_block),
              "branch target is not reachable from entry");
        check(!rxas_flow_procedure_block_reachable(
                    procedure, 1, dead_block) &&
              !rxas_flow_procedure_record_reachable(procedure, 1, 3) &&
              rxas_flow_procedure_block_reachable(
                    procedure, 1, live_block) &&
              rxas_flow_procedure_record_reachable(procedure, 1, 6),
              "immutable CFG reachability query disagrees with entry paths");
        check(structural_metrics &&
              structural_metrics->unreachable_blocks >= 1 &&
              structural_metrics->scc_count == 0 &&
              structural_metrics->loops == 0,
              "structural reachability did not preserve unreachable code");
        check(rxas_flow_require_structural_analysis(procedure, 1, 0) ==
                    analysis,
              "structural analysis was not cached by epoch");
        check(rxas_flow_require_signal_analysis(procedure, 1, 1) == 0,
              "bounded signal analysis did not fail closed");
        signal_metrics = rxas_flow_last_signal_metrics(procedure, 1);
        check(signal_metrics && signal_metrics->status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED,
              "bounded signal analysis did not report budget exhaustion");
        signal_analysis = rxas_flow_require_signal_analysis(
                procedure, 1, 0);
        check(signal_analysis != 0 &&
              rxas_flow_require_signal_analysis(procedure, 1, 0) ==
                    signal_analysis,
              "signal analysis did not recover and cache by epoch");
        record = rxas_flow_procedure_record(procedure, 1, 2);
        check(record && record->emitted_address == 102,
              "dead label emitted-address mapping drifted");
        record = rxas_flow_procedure_record(procedure, 1, 5);
        check(record && record->emitted_address == 106,
              "live label emitted-address mapping drifted");
        check(rxas_flow_procedure_metrics(procedure, 2) == 0 &&
              rxas_flow_procedure_record(procedure, 2, 0) == 0 &&
              rxas_flow_procedure_entry_block(procedure, 2) ==
                    RXAS_FLOW_ID_NONE &&
              !rxas_flow_procedure_record_reachable(procedure, 2, 6),
              "stale epoch access did not fail closed");
        check(rxas_flow_require_structural_analysis(procedure, 2, 0) == 0 &&
              rxas_flow_structural_metrics(analysis, 2) == 0,
              "stale structural analysis access did not fail closed");
        check(rxas_flow_require_signal_analysis(procedure, 2, 0) == 0 &&
              rxas_flow_signal_metrics(signal_analysis, 2) == 0,
              "stale signal analysis access did not fail closed");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_diamond(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[2];
    size_t entry_block;
    size_t right_block;
    size_t left_block;
    size_t join_block;
    const RxasFlowStructuralAnalysis *analysis;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_label_ref(&fixture, "left");
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "brt", operands, 2);
    fixture_label(&fixture, "right");
    operands[0] = fixture_label_ref(&fixture, "join");
    fixture_op(&fixture, "br", operands, 1);
    fixture_label(&fixture, "left");
    operands[0] = fixture_label_ref(&fixture, "join");
    fixture_op(&fixture, "br", operands, 1);
    fixture_label(&fixture, "join");
    fixture_op(&fixture, "ret", 0, 0);

    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 2);
    check(procedure != 0, "diamond graph construction failed");
    if (procedure) {
        analysis = rxas_flow_require_structural_analysis(procedure, 2, 0);
        check(analysis != 0, "diamond structural analysis failed");
        entry_block = fixture_block_for_record(procedure, 2, 0);
        right_block = fixture_block_for_record(procedure, 2, 1);
        left_block = fixture_block_for_record(procedure, 2, 3);
        join_block = fixture_block_for_record(procedure, 2, 5);
        check(fixture_has_edge(procedure, 2, entry_block, right_block,
                               RXAS_FLOW_EDGE_NORMAL) &&
              fixture_has_edge(procedure, 2, entry_block, left_block,
                               RXAS_FLOW_EDGE_BRANCH),
              "diamond split edges are incomplete");
        check(fixture_has_edge(procedure, 2, right_block, join_block,
                               RXAS_FLOW_EDGE_BRANCH) &&
              fixture_has_edge(procedure, 2, left_block, join_block,
                               RXAS_FLOW_EDGE_BRANCH),
              "diamond join edges are incomplete");
        check(analysis &&
              rxas_flow_structural_immediate_dominator(
                    analysis, 2, join_block) == entry_block &&
              rxas_flow_structural_dominates(
                    analysis, 2, entry_block, join_block),
              "diamond immediate dominator is incorrect");
        check(analysis &&
              rxas_flow_structural_frontier_count(
                    analysis, 2, right_block) == 1 &&
              rxas_flow_structural_frontier(
                    analysis, 2, right_block, 0) == join_block &&
              rxas_flow_structural_frontier_count(
                    analysis, 2, left_block) == 1 &&
              rxas_flow_structural_frontier(
                    analysis, 2, left_block, 0) == join_block,
              "diamond dominance frontiers are incorrect");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_nested_and_irreducible(Assembler_Context *context) {
    FlowFixture nested;
    FlowFixture irreducible;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[2];
    size_t outer_block;
    size_t inner_block;
    size_t latch_block;
    size_t a_block;
    size_t b_block;
    const RxasFlowStructuralAnalysis *analysis;
    const RxasFlowStructuralMetrics *metrics;
    size_t loop_index;
    int found_irreducible;
    memset(&nested, 0, sizeof(nested));
    fixture_label(&nested, "outer");
    operands[0] = fixture_label_ref(&nested, "exit");
    operands[1] = fixture_register(&nested, 0);
    fixture_op(&nested, "brt", operands, 2);
    fixture_label(&nested, "inner");
    operands[0] = fixture_label_ref(&nested, "outer");
    operands[1] = fixture_register(&nested, 0);
    fixture_op(&nested, "brt", operands, 2);
    operands[0] = fixture_label_ref(&nested, "inner");
    fixture_op(&nested, "br", operands, 1);
    fixture_label(&nested, "exit");
    fixture_op(&nested, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, nested.items,
                                          nested.item_count, 3);
    check(procedure != 0, "nested-loop graph construction failed");
    if (procedure) {
        analysis = rxas_flow_require_structural_analysis(procedure, 3, 0);
        metrics = rxas_flow_structural_metrics(analysis, 3);
        check(analysis && metrics && metrics->scc_count == 0 &&
              metrics->backedges == 0 && metrics->loops == 0,
              "dormant loop capability was built with dominance");
        check(rxas_flow_require_loop_analysis(procedure, 3, 0) == analysis,
              "loop capability did not extend the cached structural graph");
        metrics = rxas_flow_structural_metrics(analysis, 3);
        outer_block = fixture_block_for_record(procedure, 3, 0);
        inner_block = fixture_block_for_record(procedure, 3, 2);
        latch_block = fixture_block_for_record(procedure, 3, 4);
        check(fixture_has_edge(procedure, 3, inner_block, outer_block,
                               RXAS_FLOW_EDGE_BRANCH) &&
              fixture_has_edge(procedure, 3, latch_block, inner_block,
                               RXAS_FLOW_EDGE_BRANCH),
              "nested-loop backedges are incomplete");
        check(metrics && metrics->backedges >= 2 && metrics->loops >= 2 &&
              metrics->max_loop_depth >= 2,
              "nested-loop forest is incomplete");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&nested);

    memset(&irreducible, 0, sizeof(irreducible));
    operands[0] = fixture_label_ref(&irreducible, "a");
    operands[1] = fixture_register(&irreducible, 0);
    fixture_op(&irreducible, "brt", operands, 2);
    fixture_label(&irreducible, "b");
    operands[0] = fixture_label_ref(&irreducible, "a");
    fixture_op(&irreducible, "br", operands, 1);
    fixture_label(&irreducible, "a");
    operands[0] = fixture_label_ref(&irreducible, "b");
    operands[1] = fixture_register(&irreducible, 0);
    fixture_op(&irreducible, "brt", operands, 2);
    fixture_label(&irreducible, "done");
    fixture_op(&irreducible, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, irreducible.items,
                                          irreducible.item_count, 4);
    check(procedure != 0, "irreducible graph construction failed");
    if (procedure) {
        analysis = rxas_flow_require_loop_analysis(procedure, 4, 0);
        metrics = rxas_flow_structural_metrics(analysis, 4);
        b_block = fixture_block_for_record(procedure, 4, 1);
        a_block = fixture_block_for_record(procedure, 4, 3);
        check(fixture_has_edge(procedure, 4, b_block, a_block,
                               RXAS_FLOW_EDGE_BRANCH) &&
              fixture_has_edge(procedure, 4, a_block, b_block,
                               RXAS_FLOW_EDGE_BRANCH),
              "irreducible cycle edges are incomplete");
        check(fixture_path_exists(
                    procedure, 4,
                    rxas_flow_procedure_entry_block(procedure, 4), a_block) &&
              fixture_path_exists(
                    procedure, 4,
                    rxas_flow_procedure_entry_block(procedure, 4), b_block),
              "irreducible cycle does not retain both entry paths");
        found_irreducible = 0;
        if (metrics) {
            for (loop_index = 0; loop_index < metrics->loops; loop_index++) {
                const RxasFlowLoop *loop;
                loop = rxas_flow_structural_loop(analysis, 4, loop_index);
                if (loop && (loop->flags & RXAS_FLOW_LOOP_IRREDUCIBLE))
                    found_irreducible = 1;
            }
        }
        check(metrics && metrics->irreducible_scc_count == 1 &&
              found_irreducible,
              "irreducible SCC was not represented in the loop forest");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&irreducible);
}

static void test_signal_roots_and_determinism(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *first;
    RxasFlowProcedure *second;
    Assembler_Token *operands[2];
    const RxasFlowMetrics *metrics;
    size_t inc_block;
    size_t ret_block;
    size_t handler_block;
    size_t handler_root;
    size_t async_root;
    char *first_dump;
    char *second_dump;
    char *first_analysis_dump;
    char *second_analysis_dump;
    FILE *stale_stream;
    const RxasFlowStructuralAnalysis *first_analysis;
    const RxasFlowStructuralAnalysis *second_analysis;
    const RxasFlowStructuralMetrics *structural_metrics;
    const RxasFlowSignalAnalysis *first_signal;
    const RxasFlowSignalAnalysis *second_signal;
    const RxasFlowSignalMetrics *signal_metrics;
    RxasFlowPolicyFact fact;
    size_t sigbr_block;
    size_t sigbr_normal_edge;
    size_t sigbr_skip_edge;
    size_t inc_handler_edge;
    size_t sigbr_instruction;
    size_t inc_instruction;
    size_t ret_instruction;
    size_t trace_before;
    size_t trace_after;
    char *first_signal_dump;
    char *second_signal_dump;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_label_ref(&fixture, "handler");
    operands[1] = fixture_string(&fixture, "OVERFLOW_UNDERFLOW");
    fixture_op(&fixture, "sigbr", operands, 2);
    fixture_record(&fixture, SRC_STEP);
    operands[0] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "inc", operands, 1);
    fixture_record(&fixture, TRACE_EVENT);
    fixture_op(&fixture, "ret", 0, 0);
    fixture_label(&fixture, "handler");
    fixture_op(&fixture, "ret", 0, 0);

    first = rxas_flow_procedure_build(context, fixture.items,
                                      fixture.item_count, 7);
    second = rxas_flow_procedure_build(context, fixture.items,
                                       fixture.item_count, 7);
    check(first != 0 && second != 0,
          "signal graph construction failed");
    if (first && second) {
        first_analysis = rxas_flow_require_structural_analysis(first, 7, 0);
        second_analysis = rxas_flow_require_structural_analysis(second, 7, 0);
        first_signal = rxas_flow_require_signal_analysis(first, 7, 0);
        second_signal = rxas_flow_require_signal_analysis(second, 7, 0);
        check(first_analysis != 0 && second_analysis != 0,
              "signal structural analysis failed");
        check(first_signal != 0 && second_signal != 0,
              "signal-policy analysis failed");
        sigbr_block = fixture_block_for_record(first, 7, 0);
        inc_block = fixture_block_for_record(first, 7, 2);
        ret_block = fixture_block_for_record(first, 7, 4);
        handler_block = fixture_block_for_record(first, 7, 5);
        handler_root = rxas_flow_procedure_handler_root(first, 7);
        async_root = rxas_flow_procedure_async_root(first, 7);
        check(fixture_has_edge(first, 7, inc_block, ret_block,
                               RXAS_FLOW_EDGE_NORMAL) &&
              fixture_has_edge(first, 7, inc_block, ret_block,
                               RXAS_FLOW_EDGE_SIGNAL_SKIP),
              "signal normal/skip continuations are incomplete");
        check(fixture_has_edge(first, 7, inc_block, handler_root,
                               RXAS_FLOW_EDGE_HANDLER) &&
              fixture_has_edge(first, 7, inc_block,
                               rxas_flow_procedure_unwind_exit(first, 7),
                               RXAS_FLOW_EDGE_UNWIND) &&
              fixture_has_edge(first, 7, inc_block,
                               rxas_flow_procedure_terminal_exit(first, 7),
                               RXAS_FLOW_EDGE_TERMINAL),
              "signal handler/unwind/terminal continuations are incomplete");
        check(fixture_has_edge(first, 7, handler_root, handler_block,
                               RXAS_FLOW_EDGE_HANDLER) &&
              fixture_has_edge(first, 7, async_root, handler_block,
                               RXAS_FLOW_EDGE_HANDLER),
              "handler and asynchronous roots do not reach the handler");
        metrics = rxas_flow_procedure_metrics(first, 7);
        check(metrics && metrics->complete_control_flow &&
              metrics->signal_skip_edges >= 2,
              "signal graph metrics are incomplete");
        check(first_analysis &&
              rxas_flow_structural_predecessor_count(
                    first_analysis, 7, ret_block) == 1 &&
              rxas_flow_structural_predecessor(
                    first_analysis, 7, ret_block, 0) == inc_block,
              "parallel normal/skip edges did not form a predecessor set");
        check(rxas_flow_procedure_record(first, 7, 1)->block_id == inc_block &&
              rxas_flow_procedure_record(first, 7, 3)->block_id == ret_block,
              "source/TRACE records lost their continuation block mapping");
        sigbr_instruction = rxas_flow_procedure_record(first, 7, 0)->instruction_id;
        inc_instruction = rxas_flow_procedure_record(first, 7, 2)->instruction_id;
        ret_instruction = rxas_flow_procedure_record(first, 7, 4)->instruction_id;
        check(first_signal && rxas_flow_policy_at_instruction(
                    first_signal, 7, sigbr_instruction, 1,
                    "OVERFLOW_UNDERFLOW", &fact) &&
              fact.state == RXAS_FLOW_POLICY_EXACT &&
              fact.effect == RXOP_POLICY_EFFECT_BRANCH &&
              fact.defining_instruction == sigbr_instruction,
              "SIGBR normal transfer did not install an exact policy");
        sigbr_normal_edge = fixture_edge_id(
                first, 7, sigbr_block, inc_block, RXAS_FLOW_EDGE_NORMAL);
        sigbr_skip_edge = fixture_edge_id(
                first, 7, sigbr_block, inc_block,
                RXAS_FLOW_EDGE_SIGNAL_SKIP);
        check(sigbr_normal_edge != RXAS_FLOW_ID_NONE && first_signal &&
              rxas_flow_policy_on_edge(
                    first_signal, 7, sigbr_normal_edge,
                    "OVERFLOW_UNDERFLOW", &fact) &&
              fact.state == RXAS_FLOW_POLICY_EXACT,
              "SIGBR normal edge lost its successful policy write");
        check(sigbr_skip_edge != RXAS_FLOW_ID_NONE && first_signal &&
              rxas_flow_policy_on_edge(
                    first_signal, 7, sigbr_skip_edge,
                    "OVERFLOW_UNDERFLOW", &fact) &&
              fact.state == RXAS_FLOW_POLICY_INHERITED_UNKNOWN,
              "SIGBR failure edge incorrectly observed a policy write");
        check(first_signal && rxas_flow_policy_at_instruction(
                    first_signal, 7, inc_instruction, 0,
                    "OVERFLOW_UNDERFLOW", &fact) &&
              fact.state == RXAS_FLOW_POLICY_MERGED_UNKNOWN,
              "parallel success/skip edges did not retain distinct policy states");
        inc_handler_edge = fixture_edge_id(
                first, 7, inc_block, handler_root,
                RXAS_FLOW_EDGE_HANDLER);
        check(inc_handler_edge != RXAS_FLOW_ID_NONE && first_signal &&
              rxas_flow_policy_on_edge(
                    first_signal, 7, inc_handler_edge,
                    "OVERFLOW_UNDERFLOW", &fact) &&
              fact.state == RXAS_FLOW_POLICY_MERGED_UNKNOWN,
              "handler edge did not preserve the incoming policy merge");
        trace_before = first_signal ? rxas_flow_effect_at_instruction(
                first_signal, 7, inc_instruction, 1,
                RXAS_FLOW_EFFECT_TRACE) : RXAS_FLOW_ID_NONE;
        trace_after = first_signal ? rxas_flow_effect_at_instruction(
                first_signal, 7, ret_instruction, 0,
                RXAS_FLOW_EFFECT_TRACE) : RXAS_FLOW_ID_NONE;
        check(trace_before != RXAS_FLOW_ID_NONE &&
              trace_after != RXAS_FLOW_ID_NONE &&
              trace_before != trace_after,
              "TRACE event did not advance the independent TRACE effect");
        signal_metrics = rxas_flow_signal_metrics(first_signal, 7);
        check(signal_metrics && signal_metrics->policy_writes == 1 &&
              signal_metrics->policy_phis >= 1 &&
              signal_metrics->trace_effect_writes == 1,
              "signal-policy metrics omitted sparse writes or joins");
        first_dump = fixture_dump(first, 7);
        second_dump = fixture_dump(second, 7);
        check(strcmp(first_dump, second_dump) == 0,
              "graph dump is not deterministic");
        check(strstr(first_dump, "kind=signal-skip") != 0 &&
              strstr(first_dump, "kind=async-root") != 0,
              "graph dump omits typed signal structure");
        free(first_dump);
        free(second_dump);
        first_analysis_dump = fixture_analysis_dump(first_analysis, 7);
        second_analysis_dump = fixture_analysis_dump(second_analysis, 7);
        check(strcmp(first_analysis_dump, second_analysis_dump) == 0,
              "structural analysis dump is not deterministic");
        check(strstr(first_analysis_dump, "irreducible-sccs=") != 0,
              "structural analysis dump omits reusable structure");
        free(first_analysis_dump);
        free(second_analysis_dump);
        first_signal_dump = fixture_signal_dump(first_signal, 7);
        second_signal_dump = fixture_signal_dump(second_signal, 7);
        check(strcmp(first_signal_dump, second_signal_dump) == 0,
              "signal-policy analysis dump is not deterministic");
        check(strstr(first_signal_dump,
                     "PERF3 flow-policy-version") != 0 &&
              strstr(first_signal_dump, "PERF3 flow-signal-edge") != 0,
              "signal-policy dump omits reusable versions or edge states");
        free(first_signal_dump);
        free(second_signal_dump);
        stale_stream = tmpfile();
        if (!stale_stream) exit(2);
        check(!rxas_flow_procedure_dump(first, 8, stale_stream),
              "stale graph dump did not fail closed");
        check(!rxas_flow_structural_dump(first_analysis, 8, stale_stream),
              "stale structural dump did not fail closed");
        check(!rxas_flow_signal_dump(first_signal, 8, stale_stream),
              "stale signal-policy dump did not fail closed");
        fclose(stale_stream);
    }
    rxas_flow_procedure_destroy(first);
    rxas_flow_procedure_destroy(second);
    fixture_destroy(&fixture);
}

static void test_call_boundary_and_unknown(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[2];
    const RxasFlowMetrics *metrics;
    size_t call_block;
    size_t following_block;
    size_t unknown_block;
    size_t unknown_following;
    const RxasFlowSignalAnalysis *signal_analysis;
    RxasFlowPolicyFact before_policy;
    RxasFlowPolicyFact after_policy;
    size_t call_instruction;
    size_t ret_instruction;
    size_t call_before;
    size_t call_after;
    size_t reference_before;
    size_t reference_after;
    size_t return_edge;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_text_token(&fixture, FUNC, "callee");
    fixture_op(&fixture, "call", operands, 1);
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "load", operands, 2);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 9);
    check(procedure != 0, "call-boundary graph construction failed");
    if (procedure) {
        call_block = fixture_block_for_record(procedure, 9, 0);
        following_block = fixture_block_for_record(procedure, 9, 1);
        check(call_block != following_block &&
              fixture_has_edge(procedure, 9, call_block, following_block,
                               RXAS_FLOW_EDGE_NORMAL),
              "call did not terminate its basic block");
        signal_analysis = rxas_flow_require_signal_analysis(
                procedure, 9, 0);
        call_instruction = rxas_flow_procedure_record(
                procedure, 9, 0)->instruction_id;
        ret_instruction = rxas_flow_procedure_record(
                procedure, 9, 2)->instruction_id;
        call_before = rxas_flow_effect_at_instruction(
                signal_analysis, 9, call_instruction, 0,
                RXAS_FLOW_EFFECT_CALL);
        call_after = rxas_flow_effect_at_instruction(
                signal_analysis, 9, call_instruction, 1,
                RXAS_FLOW_EFFECT_CALL);
        reference_before = rxas_flow_effect_at_instruction(
                signal_analysis, 9, call_instruction, 0,
                RXAS_FLOW_EFFECT_REFERENCE);
        reference_after = rxas_flow_effect_at_instruction(
                signal_analysis, 9, call_instruction, 1,
                RXAS_FLOW_EFFECT_REFERENCE);
        check(signal_analysis && call_before != call_after &&
              reference_before != reference_after,
              "call did not version caller-visible reference effects");
        check(rxas_flow_policy_at_instruction(
                    signal_analysis, 9, call_instruction, 0,
                    "FAILURE", &before_policy) &&
              rxas_flow_policy_at_instruction(
                    signal_analysis, 9, call_instruction, 1,
                    "FAILURE", &after_policy) &&
              before_policy.state == RXAS_FLOW_POLICY_INHERITED_UNKNOWN &&
              after_policy.state == before_policy.state &&
              after_policy.version_id == before_policy.version_id,
              "call incorrectly clobbered frame-local handler policy");
        return_edge = fixture_edge_id(
                procedure, 9, fixture_block_for_record(procedure, 9, 2),
                rxas_flow_procedure_normal_exit(procedure, 9),
                RXAS_FLOW_EDGE_NORMAL);
        check(return_edge != RXAS_FLOW_ID_NONE &&
              rxas_flow_policy_on_edge(
                    signal_analysis, 9, return_edge, "FAILURE",
                    &after_policy) &&
              after_policy.state == RXAS_FLOW_POLICY_INHERITED_UNKNOWN,
              "return edge did not restore the frame-entry policy parameter");
        check(ret_instruction != RXAS_FLOW_ID_NONE,
              "call fixture RET instruction mapping failed");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

    memset(&fixture, 0, sizeof(fixture));
    fixture_op(&fixture, "not_an_opcode", 0, 0);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 10);
    check(procedure != 0, "unknown-opcode graph construction failed");
    if (procedure) {
        metrics = rxas_flow_procedure_metrics(procedure, 10);
        unknown_block = fixture_block_for_record(procedure, 10, 0);
        unknown_following = fixture_block_for_record(procedure, 10, 1);
        check(metrics && !metrics->complete_control_flow &&
              metrics->unknown_edges == 1,
              "unknown opcode did not mark control flow incomplete");
        check(fixture_has_edge(procedure, 10, unknown_block,
                               unknown_following,
                               RXAS_FLOW_EDGE_SIGNAL_SKIP) &&
              fixture_has_edge(procedure, 10, unknown_block,
                               rxas_flow_procedure_unknown_exit(procedure, 10),
                               RXAS_FLOW_EDGE_UNKNOWN),
              "unknown opcode did not retain worst-case continuations");
        signal_analysis = rxas_flow_require_signal_analysis(
                procedure, 10, 0);
        check(signal_analysis != 0,
              "unknown opcode disabled the fail-closed signal analysis");
        if (signal_analysis) {
            size_t unknown_edge;
            unknown_edge = fixture_edge_id(
                    procedure, 10, unknown_block,
                    rxas_flow_procedure_unknown_exit(procedure, 10),
                    RXAS_FLOW_EDGE_UNKNOWN);
            check(unknown_edge != RXAS_FLOW_ID_NONE &&
                  rxas_flow_policy_on_edge(
                        signal_analysis, 10, unknown_edge, "FAILURE",
                        &after_policy) &&
                  after_policy.state == RXAS_FLOW_POLICY_CLOBBERED,
                  "unknown opcode edge did not clobber policy proofs");
        }
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_policy_stack_uncertainty(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[1];
    const RxasFlowSignalAnalysis *analysis;
    RxasFlowPolicyFact fact;
    size_t instruction_id;
    memset(&fixture, 0, sizeof(fixture));
    fixture_op(&fixture, "bpoff", 0, 0);
    operands[0] = fixture_string(&fixture, "BREAKPOINT");
    fixture_op(&fixture, "sigpush", operands, 1);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 11);
    check(procedure != 0, "SIGPUSH policy fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_signal_analysis(procedure, 11, 0);
        instruction_id = rxas_flow_procedure_record(
                procedure, 11, 1)->instruction_id;
        check(analysis && rxas_flow_policy_at_instruction(
                    analysis, 11, instruction_id, 1, "BREAKPOINT", &fact) &&
              fact.state == RXAS_FLOW_POLICY_EXACT &&
              fact.effect == RXOP_POLICY_EFFECT_BREAKPOINT_DISABLE,
              "SIGPUSH changed the active handler policy");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

    memset(&fixture, 0, sizeof(fixture));
    fixture_op(&fixture, "bpoff", 0, 0);
    operands[0] = fixture_string(&fixture, "BREAKPOINT");
    fixture_op(&fixture, "sigpop", operands, 1);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 12);
    check(procedure != 0, "SIGPOP policy fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_signal_analysis(procedure, 12, 0);
        instruction_id = rxas_flow_procedure_record(
                procedure, 12, 1)->instruction_id;
        check(analysis && rxas_flow_policy_at_instruction(
                    analysis, 12, instruction_id, 1, "BREAKPOINT", &fact) &&
              fact.state == RXAS_FLOW_POLICY_STACK_UNKNOWN,
              "SIGPOP incorrectly proved a restored handler policy");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_policy_loop_identity(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[2];
    const RxasFlowSignalAnalysis *analysis;
    RxasFlowPolicyFact fact;
    size_t loop_instruction;
    memset(&fixture, 0, sizeof(fixture));
    fixture_op(&fixture, "bpoff", 0, 0);
    fixture_label(&fixture, "loop");
    operands[0] = fixture_label_ref(&fixture, "done");
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "brt", operands, 2);
    operands[0] = fixture_label_ref(&fixture, "loop");
    fixture_op(&fixture, "br", operands, 1);
    fixture_label(&fixture, "done");
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 13);
    check(procedure != 0, "policy-loop fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_signal_analysis(procedure, 13, 0);
        loop_instruction = rxas_flow_procedure_record(
                procedure, 13, 2)->instruction_id;
        check(analysis && rxas_flow_policy_at_instruction(
                    analysis, 13, loop_instruction, 0, "BREAKPOINT", &fact) &&
              fact.state == RXAS_FLOW_POLICY_EXACT &&
              fact.effect == RXOP_POLICY_EFFECT_BREAKPOINT_DISABLE,
              "loop phi did not preserve an unchanged write-once policy");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static RxasFlowRegister fixture_local_register(size_t number) {
    RxasFlowRegister reg;
    reg.register_class = RXAS_FLOW_REGISTER_LOCAL;
    reg.number = number;
    return reg;
}

static RxasFlowRegister fixture_argument_register(size_t number) {
    RxasFlowRegister reg;
    reg.register_class = RXAS_FLOW_REGISTER_ARGUMENT;
    reg.number = number;
    return reg;
}

static void test_sparse_storage_and_components(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *first;
    RxasFlowProcedure *second;
    Assembler_Token *operands[2];
    const RxasFlowSsaAnalysis *first_analysis;
    const RxasFlowSsaAnalysis *second_analysis;
    const RxasFlowSsaMetrics *metrics;
    RxasFlowStorageFact r0_storage;
    RxasFlowStorageFact r1_storage;
    RxasFlowStorageFact r2_storage;
    RxasFlowStorageFact r2_base;
    RxasFlowComponentFact integer_fact;
    RxasFlowComponentFact first_string;
    RxasFlowComponentFact repeated_string;
    RxasFlowComponentFact absent_fact;
    RxasFlowComponentFact absent_decimal;
    RxasFlowComponentFact copy_fact;
    RxasFlowComponentFact implicit_source;
    RxasFlowComponentFact implicit_copy;
    RxasFlowComponentFact implicit_target;
    RxasFlowComponentFact implicit_increment_before;
    RxasFlowComponentFact implicit_increment_after;
    size_t link_instruction;
    size_t first_itos_instruction;
    size_t second_itos_instruction;
    size_t swap_instruction;
    size_t unlink_instruction;
    size_t null_instruction;
    size_t copy_instruction;
    size_t implicit_copy_instruction;
    size_t implicit_target_instruction;
    size_t implicit_increment_instruction;
    char *first_dump;
    char *second_dump;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 7);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "link", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_register(&fixture, 1);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "swap", operands, 2);
    operands[0] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "unlink", operands, 1);
    operands[0] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "null", operands, 1);
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "dcopy", operands, 2);
    operands[0] = fixture_integer(&fixture, 1);
    operands[1] = fixture_integer(&fixture, 0);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_integer(&fixture, 2);
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "load", operands, 2);
    fixture_op(&fixture, "inc0", 0, 0);
    fixture_op(&fixture, "ret", 0, 0);

    first = rxas_flow_procedure_build(context, fixture.items,
                                      fixture.item_count, 14);
    second = rxas_flow_procedure_build(context, fixture.items,
                                       fixture.item_count, 14);
    check(first != 0 && second != 0,
          "sparse storage fixture construction failed");
    if (first && second) {
        check(rxas_flow_require_ssa_analysis(first, 14, 1) == 0,
              "bounded sparse SSA did not fail closed");
        metrics = rxas_flow_last_ssa_metrics(first, 14);
        check(metrics && metrics->status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED,
              "bounded sparse SSA did not report budget exhaustion");
        first_analysis = rxas_flow_require_ssa_analysis(first, 14, 0);
        second_analysis = rxas_flow_require_ssa_analysis(second, 14, 0);
        check(first_analysis != 0 && second_analysis != 0,
              "sparse storage/component SSA construction failed");
        check(first_analysis && rxas_flow_require_ssa_analysis(first, 14, 0) ==
                    first_analysis,
              "sparse SSA was not cached by epoch");
        first_dump = fixture_ssa_dump(first_analysis, 14);
        second_dump = fixture_ssa_dump(second_analysis, 14);
        check(strcmp(first_dump, second_dump) == 0,
              "sparse SSA dump is not deterministic");
        check(strstr(first_dump, "PERF3 flow-storage") != 0 &&
              strstr(first_dump, "PERF3 flow-ssa-edge") != 0,
              "sparse SSA dump omits storage versions or edge states");
        free(first_dump);
        free(second_dump);
        link_instruction = rxas_flow_procedure_record(
                first, 14, 1)->instruction_id;
        first_itos_instruction = rxas_flow_procedure_record(
                first, 14, 2)->instruction_id;
        second_itos_instruction = rxas_flow_procedure_record(
                first, 14, 3)->instruction_id;
        swap_instruction = rxas_flow_procedure_record(
                first, 14, 4)->instruction_id;
        unlink_instruction = rxas_flow_procedure_record(
                first, 14, 5)->instruction_id;
        null_instruction = rxas_flow_procedure_record(
                first, 14, 6)->instruction_id;
        copy_instruction = rxas_flow_procedure_record(
                first, 14, 7)->instruction_id;
        implicit_copy_instruction = rxas_flow_procedure_record(
                first, 14, 8)->instruction_id;
        implicit_target_instruction = rxas_flow_procedure_record(
                first, 14, 9)->instruction_id;
        implicit_increment_instruction = rxas_flow_procedure_record(
                first, 14, 10)->instruction_id;
        check(first_analysis &&
              rxas_flow_storage_at_instruction(
                    first_analysis, 14, link_instruction, 1,
                    fixture_local_register(0), &r0_storage) &&
              rxas_flow_storage_at_instruction(
                    first_analysis, 14, link_instruction, 1,
                    fixture_local_register(1), &r1_storage) &&
              r0_storage.storage_id == r1_storage.storage_id &&
              r0_storage.kind == RXAS_FLOW_STORAGE_BASE,
              "LINK did not preserve the actual storage identity");
        check(rxas_flow_component_at_instruction(
                    first_analysis, 14, first_itos_instruction, 1,
                    fixture_local_register(0), RXOP_COMPONENT_INTEGER,
                    &integer_fact) &&
              rxas_flow_component_at_instruction(
                    first_analysis, 14, first_itos_instruction, 1,
                    fixture_local_register(0), RXOP_COMPONENT_STRING,
                    &first_string) &&
              integer_fact.kind == RXAS_FLOW_VALUE_CONSTANT &&
              first_string.kind == RXAS_FLOW_VALUE_DERIVED &&
              first_string.derivation ==
                    RXOP_DERIVATION_INTEGER_TO_STRING &&
              first_string.source_value_id == integer_fact.value_id &&
              first_string.definition_numeric_context ==
                    first_string.current_numeric_context,
              "ITOS did not retain its source ValueId and numeric context");
        check(rxas_flow_component_at_instruction(
                    first_analysis, 14, second_itos_instruction, 0,
                    fixture_local_register(1), RXOP_COMPONENT_STRING,
                    &repeated_string) &&
              repeated_string.value_id == first_string.value_id &&
              repeated_string.source_value_id == integer_fact.value_id,
              "repeated conversion could not see the prior derived value");
        check(rxas_flow_storage_at_instruction(
                    first_analysis, 14, swap_instruction, 0,
                    fixture_local_register(2), &r2_base) &&
              rxas_flow_storage_at_instruction(
                    first_analysis, 14, swap_instruction, 1,
                    fixture_local_register(2), &r2_storage) &&
              r2_storage.storage_id == r0_storage.storage_id &&
              rxas_flow_storage_at_instruction(
                    first_analysis, 14, swap_instruction, 1,
                    fixture_local_register(1), &r1_storage) &&
              r1_storage.storage_id == r2_base.storage_id,
              "SWAP did not follow storage identities");
        check(rxas_flow_storage_at_instruction(
                    first_analysis, 14, unlink_instruction, 1,
                    fixture_local_register(2), &r2_storage) &&
              r2_storage.storage_id == r2_base.storage_id &&
              r2_storage.kind == RXAS_FLOW_STORAGE_BASE,
              "UNLINK did not restore the register's base storage");
        check(rxas_flow_component_at_instruction(
                    first_analysis, 14, null_instruction, 1,
                    fixture_local_register(2), RXOP_COMPONENT_STRING,
                    &absent_fact) &&
              absent_fact.kind == RXAS_FLOW_VALUE_ABSENT &&
              absent_fact.presence == RXAS_FLOW_COMPONENT_ABSENT,
              "NULL was conflated with an unknown component value");
        check(rxas_flow_component_at_instruction(
                    first_analysis, 14, null_instruction, 1,
                    fixture_local_register(2), RXOP_COMPONENT_DECIMAL,
                    &absent_decimal) &&
              absent_decimal.kind == RXAS_FLOW_VALUE_ABSENT &&
              absent_decimal.presence == RXAS_FLOW_COMPONENT_ABSENT,
              "NULL did not clear the decimal component");
        check(rxas_flow_component_at_instruction(
                    first_analysis, 14, copy_instruction, 1,
                    fixture_local_register(0), RXOP_COMPONENT_DECIMAL,
                    &copy_fact) &&
              copy_fact.kind == RXAS_FLOW_VALUE_COPY &&
              copy_fact.presence == RXAS_FLOW_COMPONENT_ABSENT &&
              copy_fact.source_value_id == absent_decimal.value_id,
              "DCOPY did not preserve the source component's null state");
        check(rxas_flow_component_at_instruction(
                    first_analysis, 14, implicit_copy_instruction, 1,
                    fixture_local_register(0), RXOP_COMPONENT_INTEGER,
                    &implicit_source) &&
              rxas_flow_component_at_instruction(
                    first_analysis, 14, implicit_copy_instruction, 1,
                    fixture_local_register(1), RXOP_COMPONENT_INTEGER,
                    &implicit_copy) &&
              implicit_copy.kind == RXAS_FLOW_VALUE_COPY &&
              implicit_copy.source_value_id == implicit_source.value_id,
              "integer-coded local copy did not create component ValueIds");
        check(rxas_flow_component_at_instruction(
                    first_analysis, 14, implicit_target_instruction, 1,
                    fixture_local_register(2), RXOP_COMPONENT_INTEGER,
                    &implicit_target) &&
              implicit_target.kind == RXAS_FLOW_VALUE_COPY &&
              implicit_target.source_value_id == implicit_source.value_id,
              "integer-coded local target did not create component ValueIds");
        check(rxas_flow_component_at_instruction(
                    first_analysis, 14, implicit_increment_instruction, 0,
                    fixture_local_register(0), RXOP_COMPONENT_INTEGER,
                    &implicit_increment_before) &&
              rxas_flow_component_at_instruction(
                    first_analysis, 14, implicit_increment_instruction, 1,
                    fixture_local_register(0), RXOP_COMPONENT_INTEGER,
                    &implicit_increment_after) &&
              implicit_increment_after.kind == RXAS_FLOW_VALUE_WRITE &&
              implicit_increment_before.value_id !=
                    implicit_increment_after.value_id,
              "implicit fixed-register write did not advance its ValueId");
        metrics = rxas_flow_ssa_metrics(first_analysis, 14);
        check(metrics && metrics->states < 80 && metrics->registers == 3 &&
              metrics->component_updates < 96,
              "sparse SSA counters are not definition-scaled on the fixture");
        check(rxas_flow_require_ssa_analysis(first, 15, 0) == 0 &&
              rxas_flow_ssa_metrics(first_analysis, 15) == 0,
              "stale sparse SSA access did not fail closed");
    }
    rxas_flow_procedure_destroy(first);
    rxas_flow_procedure_destroy(second);
    fixture_destroy(&fixture);
}

static void test_attribute_path_storage(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[3];
    const RxasFlowSsaAnalysis *analysis;
    const RxasFlowSsaMetrics *metrics;
    RxasFlowStorageFact first_path;
    RxasFlowStorageFact repeated_path;
    RxasFlowStorageFact changed_count_path;
    RxasFlowStorageNode first_node;
    RxasFlowStorageNode repeated_node;
    RxasFlowStorageNode changed_count_node;
    size_t first_instruction;
    size_t repeated_instruction;
    size_t changed_count_instruction;
    memset(&fixture, 0, sizeof(fixture));

    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 2);
    fixture_op(&fixture, "setattrs", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 0);
    operands[2] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "linkattr1", operands, 3);
    operands[0] = fixture_register(&fixture, 1);
    fixture_op(&fixture, "unlink", operands, 1);
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_register(&fixture, 0);
    operands[2] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "linkattr1", operands, 3);
    operands[0] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "unlink", operands, 1);
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 3);
    fixture_op(&fixture, "setattrs", operands, 2);
    operands[0] = fixture_register(&fixture, 3);
    operands[1] = fixture_register(&fixture, 0);
    operands[2] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "linkattr1", operands, 3);
    fixture_op(&fixture, "ret", 0, 0);

    procedure = rxas_flow_procedure_build(context, fixture.items,
                                           fixture.item_count, 32);
    check(procedure != 0, "attribute-path fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 32, 0);
        first_instruction = rxas_flow_procedure_record(
                procedure, 32, 1)->instruction_id;
        repeated_instruction = rxas_flow_procedure_record(
                procedure, 32, 3)->instruction_id;
        changed_count_instruction = rxas_flow_procedure_record(
                procedure, 32, 6)->instruction_id;
        check(analysis &&
              rxas_flow_storage_at_instruction(
                    analysis, 32, first_instruction, 1,
                    fixture_local_register(1), &first_path) &&
              rxas_flow_storage_at_instruction(
                    analysis, 32, repeated_instruction, 1,
                    fixture_local_register(2), &repeated_path) &&
              rxas_flow_storage_at_instruction(
                    analysis, 32, changed_count_instruction, 1,
                    fixture_local_register(3), &changed_count_path),
              "attribute paths were unavailable from storage SSA");
        check(analysis &&
              rxas_flow_storage_node(
                    analysis, 32, first_path.storage_id, &first_node) &&
              rxas_flow_storage_node(
                    analysis, 32, repeated_path.storage_id,
                    &repeated_node) &&
              rxas_flow_storage_node(
                    analysis, 32, changed_count_path.storage_id,
                    &changed_count_node),
              "attribute-path storage nodes were unavailable");
        check(first_node.kind == RXAS_FLOW_STORAGE_ATTRIBUTE_PATH &&
              repeated_node.kind == RXAS_FLOW_STORAGE_ATTRIBUTE_PATH &&
              first_path.storage_id == repeated_path.storage_id &&
              first_node.owner_storage_id == repeated_node.owner_storage_id &&
              first_node.attribute_count_value_id ==
                    repeated_node.attribute_count_value_id &&
              first_node.reference_effect_id ==
                    repeated_node.reference_effect_id &&
              first_node.attribute_slot == 1,
              "identical attribute paths did not intern to one StorageId");
        check(changed_count_node.kind ==
                    RXAS_FLOW_STORAGE_ATTRIBUTE_PATH &&
              changed_count_path.storage_id != first_path.storage_id &&
              changed_count_node.owner_storage_id ==
                    first_node.owner_storage_id &&
              changed_count_node.attribute_count_value_id !=
                    first_node.attribute_count_value_id &&
              changed_count_node.attribute_slot == first_node.attribute_slot,
              "attribute-count generation did not distinguish the path");
        metrics = rxas_flow_ssa_metrics(analysis, 32);
        check(metrics && metrics->storage_attribute_paths >= 2,
              "attribute-path storage metric was not populated");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_argument_storage_and_call_effects(
        Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[2];
    const RxasFlowSsaAnalysis *analysis;
    RxasFlowStorageFact argument_storage;
    RxasFlowStorageFact local_storage;
    RxasFlowComponentFact argument_value;
    RxasFlowComponentFact before_call;
    RxasFlowComponentFact after_call;
    size_t linkarg_instruction;
    size_t load_instruction;
    size_t call_instruction;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 0);
    fixture_op(&fixture, "linkarg", operands, 2);
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 41);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_text_token(&fixture, FUNC, "callee");
    fixture_op(&fixture, "call", operands, 1);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 15);
    check(procedure != 0, "argument-storage fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 15, 0);
        linkarg_instruction = rxas_flow_procedure_record(
                procedure, 15, 0)->instruction_id;
        load_instruction = rxas_flow_procedure_record(
                procedure, 15, 1)->instruction_id;
        call_instruction = rxas_flow_procedure_record(
                procedure, 15, 2)->instruction_id;
        check(analysis && rxas_flow_storage_at_instruction(
                    analysis, 15, linkarg_instruction, 0,
                    fixture_argument_register(0), &argument_storage) &&
              rxas_flow_storage_at_instruction(
                    analysis, 15, linkarg_instruction, 1,
                    fixture_local_register(0), &local_storage) &&
              argument_storage.storage_id == local_storage.storage_id,
              "LINKARG did not map the local to caller-owned argument storage");
        check(rxas_flow_component_at_instruction(
                    analysis, 15, load_instruction, 1,
                    fixture_argument_register(0), RXOP_COMPONENT_INTEGER,
                    &argument_value) &&
              argument_value.kind == RXAS_FLOW_VALUE_CONSTANT,
              "write through LINKARG did not update argument storage");
        check(rxas_flow_component_at_instruction(
                    analysis, 15, call_instruction, 0,
                    fixture_local_register(0), RXOP_COMPONENT_INTEGER,
                    &before_call) &&
              rxas_flow_component_at_instruction(
                    analysis, 15, call_instruction, 1,
                    fixture_local_register(0), RXOP_COMPONENT_INTEGER,
                    &after_call) &&
              before_call.storage_id == after_call.storage_id &&
              before_call.value_id == after_call.value_id &&
              before_call.current_reference_effect !=
                    after_call.current_reference_effect,
              "call did not separate restored mapping from reference-visible mutation");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_propagated_call_failure_state(
        Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[3];
    const RxasFlowSsaAnalysis *analysis;
    RxasFlowComponentFact count_before;
    RxasFlowComponentFact count_failure;
    RxasFlowComponentFact argument_before;
    RxasFlowComponentFact argument_failure;
    RxasFlowComponentFact unrelated_before;
    RxasFlowComponentFact unrelated_failure;
    RxasFlowComponentFact result_failure;
    size_t call_instruction;
    size_t call_block;
    size_t following_block;
    size_t failure_edge;
    size_t base_register;
    size_t last_register;

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 3);
    operands[1] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 4);
    operands[1] = fixture_integer(&fixture, 17);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 5);
    operands[1] = fixture_integer(&fixture, 23);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 8);
    operands[1] = fixture_text_token(&fixture, FUNC, "callee");
    operands[2] = fixture_register(&fixture, 3);
    fixture_op(&fixture, "call", operands, 3);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 26);
    check(procedure != 0,
          "propagated counted-call fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 26, 0);
        call_instruction = rxas_flow_procedure_record(
                procedure, 26, 3)->instruction_id;
        call_block = fixture_block_for_record(procedure, 26, 3);
        following_block = fixture_block_for_record(procedure, 26, 4);
        failure_edge = fixture_edge_id(
                procedure, 26, call_block, following_block,
                RXAS_FLOW_EDGE_SIGNAL_SKIP);
        check(analysis && failure_edge != RXAS_FLOW_ID_NONE &&
              rxas_flow_call_window_bounds_at_instruction(
                    analysis, 26, call_instruction,
                    &base_register, &last_register) &&
              base_register == 3 && last_register == 4,
              "counted CALL did not retain its exact argument window");
        check(rxas_flow_component_at_instruction(
                    analysis, 26, call_instruction, 0,
                    fixture_local_register(3), RXOP_COMPONENT_INTEGER,
                    &count_before) &&
              rxas_flow_component_on_edge(
                    analysis, 26, failure_edge,
                    fixture_local_register(3), RXOP_COMPONENT_INTEGER,
                    &count_failure) &&
              count_failure.value_id == count_before.value_id,
              "counted CALL failure clobbered its unaffected count ValueId");
        check(rxas_flow_component_at_instruction(
                    analysis, 26, call_instruction, 0,
                    fixture_local_register(4), RXOP_COMPONENT_INTEGER,
                    &argument_before) &&
              rxas_flow_component_on_edge(
                    analysis, 26, failure_edge,
                    fixture_local_register(4), RXOP_COMPONENT_INTEGER,
                    &argument_failure) &&
              argument_failure.value_id != argument_before.value_id &&
              argument_failure.kind == RXAS_FLOW_VALUE_UNKNOWN,
              "counted CALL failure preserved caller-owned argument state");
        check(rxas_flow_component_at_instruction(
                    analysis, 26, call_instruction, 0,
                    fixture_local_register(5), RXOP_COMPONENT_INTEGER,
                    &unrelated_before) &&
              rxas_flow_component_on_edge(
                    analysis, 26, failure_edge,
                    fixture_local_register(5), RXOP_COMPONENT_INTEGER,
                    &unrelated_failure) &&
              unrelated_failure.value_id == unrelated_before.value_id,
              "counted CALL failure clobbered an unrelated local ValueId");
        check(rxas_flow_component_on_edge(
                    analysis, 26, failure_edge,
                    fixture_local_register(8), RXOP_COMPONENT_INTEGER,
                    &result_failure) &&
              result_failure.kind == RXAS_FLOW_VALUE_UNKNOWN,
              "counted CALL failure omitted a possible partial result write");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 3);
    operands[1] = fixture_register(&fixture, 4);
    fixture_op(&fixture, "link", operands, 2);
    operands[0] = fixture_register(&fixture, 3);
    operands[1] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 8);
    operands[1] = fixture_text_token(&fixture, FUNC, "callee");
    operands[2] = fixture_register(&fixture, 3);
    fixture_op(&fixture, "call", operands, 3);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 27);
    check(procedure != 0,
          "count/argument alias fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 27, 0);
        call_instruction = rxas_flow_procedure_record(
                procedure, 27, 2)->instruction_id;
        call_block = fixture_block_for_record(procedure, 27, 2);
        following_block = fixture_block_for_record(procedure, 27, 3);
        failure_edge = fixture_edge_id(
                procedure, 27, call_block, following_block,
                RXAS_FLOW_EDGE_SIGNAL_SKIP);
        check(analysis && failure_edge != RXAS_FLOW_ID_NONE &&
              rxas_flow_component_at_instruction(
                    analysis, 27, call_instruction, 0,
                    fixture_local_register(3), RXOP_COMPONENT_INTEGER,
                    &count_before) &&
              rxas_flow_component_on_edge(
                    analysis, 27, failure_edge,
                    fixture_local_register(3), RXOP_COMPONENT_INTEGER,
                    &count_failure) &&
              count_failure.value_id != count_before.value_id &&
              count_failure.kind == RXAS_FLOW_VALUE_UNKNOWN,
              "argument alias did not invalidate the failure count ValueId");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_storage_joins_and_loops(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[2];
    const RxasFlowSsaAnalysis *analysis;
    RxasFlowStorageFact fact;
    RxasFlowStorageFact source;
    RxasFlowComponentFact value;
    size_t join_instruction;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_label_ref(&fixture, "left");
    operands[1] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "brt", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "link", operands, 2);
    operands[0] = fixture_label_ref(&fixture, "join");
    fixture_op(&fixture, "br", operands, 1);
    fixture_label(&fixture, "left");
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "link", operands, 2);
    fixture_label(&fixture, "join");
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 16);
    check(procedure != 0, "storage-join fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 16, 0);
        join_instruction = rxas_flow_procedure_record(
                procedure, 16, 6)->instruction_id;
        check(analysis && rxas_flow_storage_at_instruction(
                    analysis, 16, join_instruction, 0,
                    fixture_local_register(1), &fact) &&
              fact.kind == RXAS_FLOW_STORAGE_PHI,
              "disagreeing storage paths did not produce a StorageId phi");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_integer(&fixture, 7);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_label_ref(&fixture, "left");
    operands[1] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "brt", operands, 2);
    operands[0] = fixture_label_ref(&fixture, "join");
    fixture_op(&fixture, "br", operands, 1);
    fixture_label(&fixture, "left");
    fixture_label(&fixture, "join");
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 33);
    check(procedure != 0, "equal-storage join fixture construction failed");
    if (procedure) {
        const RxasFlowSsaMetrics *metrics;
        analysis = rxas_flow_require_ssa_analysis(procedure, 33, 0);
        join_instruction = rxas_flow_procedure_record(
                procedure, 33, 5)->instruction_id;
        check(analysis && rxas_flow_storage_at_instruction(
                    analysis, 33, join_instruction, 0,
                    fixture_local_register(1), &fact) &&
              fact.kind == RXAS_FLOW_STORAGE_BASE,
              "equal acyclic storage paths did not retain the base StorageId");
        metrics = analysis ? rxas_flow_ssa_metrics(analysis, 33) : 0;
        check(metrics && metrics->storage_phis == 0 &&
              metrics->storage_phi_elisions > 0,
              "equal acyclic storage paths retained a redundant phi");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 3);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "link", operands, 2);
    fixture_label(&fixture, "loop");
    operands[0] = fixture_label_ref(&fixture, "done");
    operands[1] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "brt", operands, 2);
    operands[0] = fixture_label_ref(&fixture, "loop");
    fixture_op(&fixture, "br", operands, 1);
    fixture_label(&fixture, "done");
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 17);
    check(procedure != 0, "storage-loop fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 17, 0);
        join_instruction = rxas_flow_procedure_record(
                procedure, 17, 3)->instruction_id;
        check(analysis && rxas_flow_storage_at_instruction(
                    analysis, 17, join_instruction, 0,
                    fixture_local_register(0), &source) &&
              rxas_flow_storage_at_instruction(
                    analysis, 17, join_instruction, 0,
                    fixture_local_register(1), &fact) &&
              fact.storage_id == source.storage_id &&
              rxas_flow_component_at_instruction(
                    analysis, 17, join_instruction, 0,
                    fixture_local_register(1), RXOP_COMPONENT_INTEGER,
                    &value) &&
              value.kind == RXAS_FLOW_VALUE_CONSTANT,
              "loop phi did not preserve unchanged storage and ValueIds");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_signal_phase_component_edges(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[3];
    const RxasFlowSsaAnalysis *analysis;
    const RxasFlowSignalAnalysis *signal_analysis;
    RxasFlowStorageFact before_storage;
    RxasFlowStorageFact normal_storage;
    RxasFlowStorageFact skip_storage;
    RxasFlowStorageFact before_call_storage;
    RxasFlowStorageFact after_call_storage;
    RxasFlowComponentFact normal_value;
    RxasFlowComponentFact skip_value;
    RxasFlowComponentFact before_call_value;
    RxasFlowComponentFact after_call_value;
    size_t signal_block;
    size_t following_block;
    size_t normal_edge;
    size_t skip_edge;
    size_t call_instruction;
    size_t call_block;
    size_t after_call_block;
    size_t call_skip_edge;
    size_t numeric_before_call;
    size_t numeric_on_call_skip;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_register(&fixture, 1);
    operands[2] = fixture_integer(&fixture, 0);
    fixture_op(&fixture, "linkattr", operands, 3);
    operands[0] = fixture_text_token(&fixture, FUNC, "callee");
    fixture_op(&fixture, "call", operands, 1);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 18);
    check(procedure != 0, "signal mapping-edge fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 18, 0);
        signal_block = fixture_block_for_record(procedure, 18, 0);
        following_block = fixture_block_for_record(procedure, 18, 1);
        normal_edge = fixture_edge_id(procedure, 18, signal_block,
                                      following_block, RXAS_FLOW_EDGE_NORMAL);
        skip_edge = fixture_edge_id(procedure, 18, signal_block,
                                    following_block,
                                    RXAS_FLOW_EDGE_SIGNAL_SKIP);
        check(analysis && rxas_flow_storage_at_instruction(
                    analysis, 18,
                    rxas_flow_procedure_record(
                            procedure, 18, 0)->instruction_id,
                    0, fixture_local_register(0), &before_storage) &&
              rxas_flow_storage_on_edge(
                    analysis, 18, normal_edge,
                    fixture_local_register(0), &normal_storage) &&
              rxas_flow_storage_on_edge(
                    analysis, 18, skip_edge,
                    fixture_local_register(0), &skip_storage) &&
              normal_storage.kind == RXAS_FLOW_STORAGE_SITE &&
              skip_storage.storage_id == before_storage.storage_id,
              "before-write signal edge observed LINKATTR's success mapping");
        call_instruction = rxas_flow_procedure_record(
                procedure, 18, 1)->instruction_id;
        call_block = fixture_block_for_record(procedure, 18, 1);
        after_call_block = fixture_block_for_record(procedure, 18, 2);
        call_skip_edge = fixture_edge_id(
                procedure, 18, call_block, after_call_block,
                RXAS_FLOW_EDGE_SIGNAL_SKIP);
        signal_analysis = rxas_flow_require_signal_analysis(
                procedure, 18, 0);
        numeric_before_call = signal_analysis
                ? rxas_flow_effect_at_instruction(
                        signal_analysis, 18, call_instruction, 0,
                        RXAS_FLOW_EFFECT_NUMERIC_CONTEXT)
                : RXAS_FLOW_ID_NONE;
        numeric_on_call_skip = signal_analysis
                ? rxas_flow_effect_on_edge(
                        signal_analysis, 18, call_skip_edge,
                        RXAS_FLOW_EFFECT_NUMERIC_CONTEXT)
                : RXAS_FLOW_ID_NONE;
        check(rxas_flow_storage_at_instruction(
                    analysis, 18, call_instruction, 0,
                    fixture_local_register(0), &before_call_storage) &&
              rxas_flow_storage_at_instruction(
                    analysis, 18, call_instruction, 1,
                    fixture_local_register(0), &after_call_storage) &&
              before_call_storage.kind == RXAS_FLOW_STORAGE_PHI &&
              after_call_storage.storage_id ==
                    before_call_storage.storage_id &&
              after_call_storage.kind == RXAS_FLOW_STORAGE_PHI,
              "call did not preserve a caller register-to-storage mapping");
        check(rxas_flow_component_at_instruction(
                    analysis, 18, call_instruction, 0,
                    fixture_local_register(0), RXOP_COMPONENT_INTEGER,
                    &before_call_value) &&
              rxas_flow_component_at_instruction(
                    analysis, 18, call_instruction, 1,
                    fixture_local_register(0), RXOP_COMPONENT_INTEGER,
                    &after_call_value) &&
              after_call_value.value_id != before_call_value.value_id &&
              after_call_value.kind == RXAS_FLOW_VALUE_UNKNOWN,
              "call preserved a possibly externally mutable component");
        check(signal_analysis &&
              numeric_before_call == numeric_on_call_skip,
              "unknown call signal poisoned frame-local numeric context");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_string(&fixture, "seed");
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_register(&fixture, 1);
    fixture_op(&fixture, "freadcdpt", operands, 2);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 19);
    check(procedure != 0, "partial-write edge fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 19, 0);
        signal_block = fixture_block_for_record(procedure, 19, 1);
        following_block = fixture_block_for_record(procedure, 19, 2);
        normal_edge = fixture_edge_id(procedure, 19, signal_block,
                                      following_block, RXAS_FLOW_EDGE_NORMAL);
        skip_edge = fixture_edge_id(procedure, 19, signal_block,
                                    following_block,
                                    RXAS_FLOW_EDGE_SIGNAL_SKIP);
        check(analysis && rxas_flow_component_on_edge(
                    analysis, 19, normal_edge, fixture_local_register(0),
                    RXOP_COMPONENT_STRING, &normal_value) &&
              rxas_flow_component_on_edge(
                    analysis, 19, skip_edge, fixture_local_register(0),
                    RXOP_COMPONENT_STRING, &skip_value) &&
              normal_value.kind == RXAS_FLOW_VALUE_WRITE &&
              skip_value.kind == RXAS_FLOW_VALUE_UNKNOWN,
              "partial-write signal edge did not invalidate its component");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_ordered_swapn_mappings(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[4];
    const RxasFlowSsaAnalysis *analysis;
    RxasFlowStorageFact before0;
    RxasFlowStorageFact before1;
    RxasFlowStorageFact before2;
    RxasFlowStorageFact after0;
    RxasFlowStorageFact after1;
    RxasFlowStorageFact after2;
    size_t instruction;

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_register(&fixture, 1);
    operands[2] = fixture_register(&fixture, 1);
    operands[3] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "swapn", operands, 4);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 35);
    check(procedure != 0,
          "ordered overlapping SWAPN fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 35, 0);
        instruction = rxas_flow_procedure_record(
                procedure, 35, 0)->instruction_id;
        check(analysis &&
              rxas_flow_storage_at_instruction(
                    analysis, 35, instruction, 0,
                    fixture_local_register(0), &before0) &&
              rxas_flow_storage_at_instruction(
                    analysis, 35, instruction, 0,
                    fixture_local_register(1), &before1) &&
              rxas_flow_storage_at_instruction(
                    analysis, 35, instruction, 0,
                    fixture_local_register(2), &before2) &&
              rxas_flow_storage_at_instruction(
                    analysis, 35, instruction, 1,
                    fixture_local_register(0), &after0) &&
              rxas_flow_storage_at_instruction(
                    analysis, 35, instruction, 1,
                    fixture_local_register(1), &after1) &&
              rxas_flow_storage_at_instruction(
                    analysis, 35, instruction, 1,
                    fixture_local_register(2), &after2) &&
              after0.storage_id == before1.storage_id &&
              after1.storage_id == before2.storage_id &&
              after2.storage_id == before0.storage_id,
              "overlapping SWAPN pairs were not composed in operand order");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_register(&fixture, 1);
    operands[2] = fixture_register(&fixture, 0);
    operands[3] = fixture_register(&fixture, 1);
    fixture_op(&fixture, "swapn", operands, 4);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 36);
    check(procedure != 0,
          "self-cancelling SWAPN fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 36, 0);
        instruction = rxas_flow_procedure_record(
                procedure, 36, 0)->instruction_id;
        check(analysis &&
              rxas_flow_storage_at_instruction(
                    analysis, 36, instruction, 0,
                    fixture_local_register(0), &before0) &&
              rxas_flow_storage_at_instruction(
                    analysis, 36, instruction, 0,
                    fixture_local_register(1), &before1) &&
              rxas_flow_storage_at_instruction(
                    analysis, 36, instruction, 1,
                    fixture_local_register(0), &after0) &&
              rxas_flow_storage_at_instruction(
                    analysis, 36, instruction, 1,
                    fixture_local_register(1), &after1) &&
              after0.storage_id == before0.storage_id &&
              after1.storage_id == before1.storage_id,
              "repeated SWAPN pair did not restore its input mapping");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_derivation_contexts(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[2];
    const RxasFlowSsaAnalysis *analysis;
    RxasFlowComponentFact integer_source;
    RxasFlowComponentFact initial_string;
    RxasFlowComponentFact stale_string;
    RxasFlowComponentFact float_source;
    RxasFlowComponentFact float_string;
    RxasFlowComponentFact decimal_source;
    RxasFlowComponentFact decimal_string;
    RxasFlowComponentFact two_register_source;
    RxasFlowComponentFact two_register_float;
    size_t initial_itos;
    size_t repeated_itos;
    size_t ftos_instruction;
    size_t dtos_instruction;
    size_t two_register_itof;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_float(&fixture, 1.5);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    fixture_op(&fixture, "ftos", operands, 1);
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_text_token(&fixture, DECIMAL, "2.5");
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "dtos", operands, 1);
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 5);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_integer(&fixture, 20);
    fixture_op(&fixture, "setnumdgts", operands, 1);
    operands[0] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_register(&fixture, 3);
    operands[1] = fixture_integer(&fixture, 9);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 4);
    operands[1] = fixture_register(&fixture, 3);
    fixture_op(&fixture, "itof", operands, 2);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 20);
    check(procedure != 0, "derivation-context fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 20, 0);
        initial_itos = rxas_flow_procedure_record(
                procedure, 20, 5)->instruction_id;
        repeated_itos = rxas_flow_procedure_record(
                procedure, 20, 7)->instruction_id;
        ftos_instruction = rxas_flow_procedure_record(
                procedure, 20, 1)->instruction_id;
        dtos_instruction = rxas_flow_procedure_record(
                procedure, 20, 3)->instruction_id;
        two_register_itof = rxas_flow_procedure_record(
                procedure, 20, 9)->instruction_id;
        check(analysis && rxas_flow_component_at_instruction(
                    analysis, 20, initial_itos, 1,
                    fixture_local_register(0), RXOP_COMPONENT_INTEGER,
                    &integer_source) &&
              rxas_flow_component_at_instruction(
                    analysis, 20, initial_itos, 1,
                    fixture_local_register(0), RXOP_COMPONENT_STRING,
                    &initial_string) &&
              rxas_flow_component_at_instruction(
                    analysis, 20, repeated_itos, 0,
                    fixture_local_register(0), RXOP_COMPONENT_STRING,
                    &stale_string) &&
              initial_string.value_id == stale_string.value_id &&
              initial_string.source_value_id == integer_source.value_id &&
              initial_string.definition_effects[
                    RXAS_FLOW_EFFECT_NUMERIC_CONTEXT] ==
                    initial_string.current_effects[
                            RXAS_FLOW_EFFECT_NUMERIC_CONTEXT] &&
              stale_string.definition_effects[
                    RXAS_FLOW_EFFECT_NUMERIC_CONTEXT] !=
                    stale_string.current_effects[
                            RXAS_FLOW_EFFECT_NUMERIC_CONTEXT],
              "numeric-context write did not stale an available derivation");
        check(rxas_flow_component_at_instruction(
                    analysis, 20, ftos_instruction, 1,
                    fixture_local_register(1), RXOP_COMPONENT_FLOAT,
                    &float_source) &&
              rxas_flow_component_at_instruction(
                    analysis, 20, ftos_instruction, 1,
                    fixture_local_register(1), RXOP_COMPONENT_STRING,
                    &float_string) &&
              float_string.derivation ==
                    RXOP_DERIVATION_FLOAT_TO_STRING &&
              float_string.source_value_id == float_source.value_id &&
              float_string.signal_dependencies ==
                    RXOP_SIGNAL_DEP_NUMERIC_CONTEXT,
              "FTOS did not retain its source and derivation context");
        check(rxas_flow_component_at_instruction(
                    analysis, 20, dtos_instruction, 1,
                    fixture_local_register(2), RXOP_COMPONENT_DECIMAL,
                    &decimal_source) &&
              rxas_flow_component_at_instruction(
                    analysis, 20, dtos_instruction, 1,
                    fixture_local_register(2), RXOP_COMPONENT_STRING,
                    &decimal_string) &&
              decimal_string.derivation ==
                    RXOP_DERIVATION_DECIMAL_TO_STRING &&
              decimal_string.source_value_id == decimal_source.value_id &&
              decimal_string.signal_dependencies ==
                    (RXOP_SIGNAL_DEP_NUMERIC_CONTEXT |
                     RXOP_SIGNAL_DEP_PLUGIN) &&
              decimal_string.definition_effects[RXAS_FLOW_EFFECT_PLUGIN] !=
                    RXAS_FLOW_ID_NONE &&
              decimal_string.definition_effects[RXAS_FLOW_EFFECT_PLUGIN] ==
                    decimal_string.current_effects[RXAS_FLOW_EFFECT_PLUGIN],
              "DTOS did not retain its numeric/plugin derivation contexts");
        check(rxas_flow_component_at_instruction(
                    analysis, 20, two_register_itof, 1,
                    fixture_local_register(3), RXOP_COMPONENT_INTEGER,
                    &two_register_source) &&
              rxas_flow_component_at_instruction(
                    analysis, 20, two_register_itof, 1,
                    fixture_local_register(4), RXOP_COMPONENT_FLOAT,
                    &two_register_float) &&
              two_register_float.derivation ==
                    RXOP_DERIVATION_INTEGER_TO_FLOAT &&
              two_register_float.source_value_id ==
                    two_register_source.value_id,
              "two-register derivation used the destination as its source");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_fused_failure_mappings(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[7];
    const RxasFlowSsaAnalysis *analysis;
    RxasFlowStorageFact before_left;
    RxasFlowStorageFact before_right;
    RxasFlowStorageFact failure_left;
    RxasFlowStorageFact failure_right;
    RxasFlowStorageFact normal_link;
    RxasFlowStorageFact failure_link;
    RxasFlowComponentFact joined_component;
    RxasFlowComponentFact fused_value;
    size_t instruction_id;
    size_t instruction_block;
    size_t following_block;
    size_t normal_edge;
    size_t skip_edge;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_text_token(&fixture, FUNC, "callee");
    operands[2] = fixture_register(&fixture, 1);
    operands[3] = fixture_register(&fixture, 2);
    operands[4] = fixture_register(&fixture, 3);
    fixture_op(&fixture, "swapcall", operands, 5);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 21);
    check(procedure != 0, "swap-call failure fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 21, 0);
        instruction_id = rxas_flow_procedure_record(
                procedure, 21, 0)->instruction_id;
        instruction_block = fixture_block_for_record(procedure, 21, 0);
        following_block = fixture_block_for_record(procedure, 21, 1);
        skip_edge = fixture_edge_id(
                procedure, 21, instruction_block, following_block,
                RXAS_FLOW_EDGE_SIGNAL_SKIP);
        check(analysis && rxas_flow_storage_at_instruction(
                    analysis, 21, instruction_id, 0,
                    fixture_local_register(2), &before_left) &&
              rxas_flow_storage_at_instruction(
                    analysis, 21, instruction_id, 0,
                    fixture_local_register(3), &before_right) &&
              rxas_flow_storage_on_edge(
                    analysis, 21, skip_edge,
                    fixture_local_register(2), &failure_left) &&
              rxas_flow_storage_on_edge(
                    analysis, 21, skip_edge,
                    fixture_local_register(3), &failure_right) &&
              failure_left.storage_id == before_right.storage_id &&
              failure_right.storage_id == before_left.storage_id,
              "swap-call failure edge did not retain its pre-call swap");
        check(rxas_flow_component_at_instruction(
                    analysis, 21, instruction_id, 1,
                    fixture_local_register(0), RXOP_COMPONENT_INTEGER,
                    &fused_value) &&
              fused_value.kind == RXAS_FLOW_VALUE_UNKNOWN,
              "fused remap/value opcode invented an intra-instruction order");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_register(&fixture, 1);
    operands[2] = fixture_integer(&fixture, 0);
    operands[3] = fixture_integer(&fixture, 0);
    operands[4] = fixture_register(&fixture, 2);
    operands[5] = fixture_register(&fixture, 3);
    operands[6] = fixture_integer(&fixture, 0);
    fixture_op(&fixture, "linksetattrslinkadd", operands, 7);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 22);
    check(procedure != 0, "fused-link failure fixture construction failed");
    if (procedure) {
        analysis = rxas_flow_require_ssa_analysis(procedure, 22, 0);
        instruction_block = fixture_block_for_record(procedure, 22, 0);
        following_block = fixture_block_for_record(procedure, 22, 1);
        normal_edge = fixture_edge_id(
                procedure, 22, instruction_block, following_block,
                RXAS_FLOW_EDGE_NORMAL);
        skip_edge = fixture_edge_id(
                procedure, 22, instruction_block, following_block,
                RXAS_FLOW_EDGE_SIGNAL_SKIP);
        check(analysis && rxas_flow_storage_on_edge(
                    analysis, 22, normal_edge,
                    fixture_local_register(0), &normal_link) &&
              rxas_flow_storage_on_edge(
                    analysis, 22, skip_edge,
                    fixture_local_register(0), &failure_link) &&
              normal_link.kind == RXAS_FLOW_STORAGE_SITE &&
              failure_link.kind == RXAS_FLOW_STORAGE_UNKNOWN,
              "fused-link partial failure did not meet its mapping to unknown");
        instruction_id = rxas_flow_procedure_record(
                procedure, 22, 1)->instruction_id;
        check(rxas_flow_component_at_instruction(
                    analysis, 22, instruction_id, 0,
                    fixture_local_register(0), RXOP_COMPONENT_STRING,
                    &joined_component) &&
              joined_component.kind == RXAS_FLOW_VALUE_PHI &&
              joined_component.presence ==
                    RXAS_FLOW_COMPONENT_PRESENCE_UNKNOWN,
              "unknown mapping input was conflated with ValueId zero");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_proof_service(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[3];
    const RxasFlowProofService *proof;
    const RxasFlowProofMetrics *metrics;
    RxasFlowProofResult result;
    size_t first_itos;
    size_t repeated_itos;
    size_t changed_context_itos;
    size_t after_reference_itos;
    size_t before_signal_itos;
    size_t after_signal_itos;
    size_t aliased_itos;
    size_t after_mutation_itos;
    size_t call_instruction;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 5);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_integer(&fixture, 20);
    fixture_op(&fixture, "setnumdgts", operands, 1);
    operands[0] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_register(&fixture, 3);
    fixture_op(&fixture, "link", operands, 2);
    operands[0] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_register(&fixture, 6);
    operands[1] = fixture_integer(&fixture, 9);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 6);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 2);
    operands[2] = fixture_register(&fixture, 3);
    fixture_op(&fixture, "stemset", operands, 3);
    operands[0] = fixture_register(&fixture, 6);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_register(&fixture, 4);
    operands[1] = fixture_integer(&fixture, 7);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 4);
    fixture_op(&fixture, "itof", operands, 1);
    operands[0] = fixture_register(&fixture, 5);
    operands[1] = fixture_register(&fixture, 4);
    fixture_op(&fixture, "link", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 2);
    operands[2] = fixture_register(&fixture, 3);
    fixture_op(&fixture, "stemset", operands, 3);
    operands[0] = fixture_register(&fixture, 4);
    fixture_op(&fixture, "itof", operands, 1);
    operands[0] = fixture_text_token(&fixture, FUNC, "callee");
    fixture_op(&fixture, "call", operands, 1);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 23);
    check(procedure != 0, "proof-service fixture construction failed");
    if (procedure) {
        check(rxas_flow_require_proof_service(procedure, 23, 1) == 0,
              "bounded proof service did not fail closed");
        metrics = rxas_flow_last_proof_metrics(procedure, 23);
        check(metrics && metrics->status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED,
              "bounded proof service did not report exhaustion");
        proof = rxas_flow_require_proof_service(procedure, 23, 0);
        first_itos = rxas_flow_procedure_record(
                procedure, 23, 1)->instruction_id;
        repeated_itos = rxas_flow_procedure_record(
                procedure, 23, 2)->instruction_id;
        changed_context_itos = rxas_flow_procedure_record(
                procedure, 23, 4)->instruction_id;
        after_reference_itos = rxas_flow_procedure_record(
                procedure, 23, 6)->instruction_id;
        before_signal_itos = rxas_flow_procedure_record(
                procedure, 23, 8)->instruction_id;
        after_signal_itos = rxas_flow_procedure_record(
                procedure, 23, 10)->instruction_id;
        aliased_itos = rxas_flow_procedure_record(
                procedure, 23, 12)->instruction_id;
        after_mutation_itos = rxas_flow_procedure_record(
                procedure, 23, 15)->instruction_id;
        call_instruction = rxas_flow_procedure_record(
                procedure, 23, 16)->instruction_id;
        check(proof && rxas_flow_prove_repetition(
                    proof, 23, first_itos, repeated_itos, &result) &&
              result.proved && result.reason == RXAS_FLOW_PROOF_PROVED,
              "proof service rejected an unchanged repeated ITOS");
        check(rxas_flow_prove_repetition(
                    proof, 23, first_itos, repeated_itos, &result) &&
              result.proved &&
              rxas_flow_proof_metrics(proof, 23)->repetition_cache_hits == 1,
              "repetition proof was not cached by epoch");
        check(rxas_flow_prove_repetition(
                    proof, 23, first_itos, changed_context_itos, &result) &&
              !result.proved &&
              result.reason == RXAS_FLOW_PROOF_EFFECT_CHANGED,
              "opaque numeric-context write did not invalidate the result");
        check(rxas_flow_prove_repetition(
                    proof, 23, changed_context_itos,
                    after_reference_itos, &result) &&
              result.proved,
              "unrelated alias topology invalidated a stable value");
        check(rxas_flow_prove_repetition(
                    proof, 23, before_signal_itos,
                    after_signal_itos, &result) &&
              result.proved && result.reason == RXAS_FLOW_PROOF_PROVED,
              "pre-write signal continuation poisoned frame-local numeric context");
        check(rxas_flow_prove_repetition(
                    proof, 23, aliased_itos,
                    after_mutation_itos, &result) &&
              !result.proved && result.reason ==
                    RXAS_FLOW_PROOF_REFERENCE_EFFECT_CHANGED,
              "aliased storage crossed a reference-visible mutation");
        check(rxas_flow_prove_instruction_speculatable(
                    proof, 23, first_itos, &result) && result.proved,
              "total context-reading ITOS was not speculatable");
        check(rxas_flow_prove_instruction_speculatable(
                    proof, 23, call_instruction, &result) && !result.proved &&
              result.reason == RXAS_FLOW_PROOF_NOT_SPECULATABLE,
              "call was incorrectly classified as speculatable");
        check(rxas_flow_prove_repetition(
                    proof, 24, first_itos, repeated_itos, &result) &&
              !result.proved && result.reason ==
                    RXAS_FLOW_PROOF_STALE_EPOCH,
              "stale proof service did not fail closed");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_redundant_constant_proof(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[2];
    const RxasFlowProofService *proof;
    const RxasFlowSsaAnalysis *ssa;
    RxasFlowProofResult result;
    RxasFlowComponentFact cleanup;
    size_t repeated_integer;
    size_t changed_integer;
    size_t first_float;
    size_t repeated_float;
    size_t first_reference_load;
    size_t cleanup_load;
    size_t native_cleanup_load;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 5);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 5);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 6);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_float(&fixture, 1.5);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_float(&fixture, 1.5);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_integer(&fixture, 0);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "mkref", operands, 2);
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_integer(&fixture, 0);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 3);
    operands[1] = fixture_integer(&fixture, 0);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 3);
    operands[1] = fixture_register(&fixture, 4);
    fixture_op(&fixture, "bcopy", operands, 2);
    operands[0] = fixture_register(&fixture, 3);
    operands[1] = fixture_integer(&fixture, 0);
    fixture_op(&fixture, "load", operands, 2);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 26);
    check(procedure != 0,
          "redundant-constant proof fixture construction failed");
    if (procedure) {
        proof = rxas_flow_require_proof_service(procedure, 26, 0);
        ssa = rxas_flow_require_ssa_analysis(procedure, 26, 0);
        repeated_integer = rxas_flow_procedure_record(
                procedure, 26, 1)->instruction_id;
        changed_integer = rxas_flow_procedure_record(
                procedure, 26, 2)->instruction_id;
        first_float = rxas_flow_procedure_record(
                procedure, 26, 3)->instruction_id;
        repeated_float = rxas_flow_procedure_record(
                procedure, 26, 4)->instruction_id;
        first_reference_load = rxas_flow_procedure_record(
                procedure, 26, 5)->instruction_id;
        cleanup_load = rxas_flow_procedure_record(
                procedure, 26, 7)->instruction_id;
        native_cleanup_load = rxas_flow_procedure_record(
                procedure, 26, 10)->instruction_id;
        check(proof && rxas_flow_prove_redundant_constant_write(
                    proof, 26, repeated_integer, &result) && result.proved,
              "repeated integer constant was not proved redundant");
        check(rxas_flow_prove_redundant_constant_write(
                    proof, 26, changed_integer, &result) && !result.proved &&
              result.reason == RXAS_FLOW_PROOF_CONSTANT_CHANGED,
              "changed integer constant was incorrectly proved redundant");
        check(rxas_flow_prove_redundant_constant_write(
                    proof, 26, first_float, &result) && !result.proved,
              "first float write was incorrectly proved redundant");
        check(rxas_flow_prove_redundant_constant_write(
                    proof, 26, repeated_float, &result) && result.proved,
              "bitwise-identical float constant was not proved redundant");
        check(ssa && rxas_flow_component_at_instruction(
                    ssa, 26, first_reference_load, 1,
                    fixture_local_register(2), RXOP_COMPONENT_REFERENCE,
                    &cleanup) &&
              cleanup.kind == RXAS_FLOW_VALUE_ABSENT &&
              cleanup.presence == RXAS_FLOW_COMPONENT_ABSENT &&
              rxas_flow_component_at_instruction(
                    ssa, 26, first_reference_load, 1,
                    fixture_local_register(2),
                    RXOP_COMPONENT_NATIVE_PAYLOAD, &cleanup) &&
              cleanup.kind == RXAS_FLOW_VALUE_ABSENT,
              "scalar load did not record hidden cleanup as absent");
        check(rxas_flow_prove_redundant_constant_write(
                    proof, 26, cleanup_load, &result) && !result.proved,
              "reference-payload cleanup was incorrectly deleted");
        check(rxas_flow_prove_redundant_constant_write(
                    proof, 26, native_cleanup_load, &result) &&
              !result.proved &&
              result.reason == RXAS_FLOW_PROOF_CLEANUP_REQUIRED,
              "native-payload cleanup was not proved necessary");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_redundant_absent_proof(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[2];
    const RxasFlowProofService *proof;
    RxasFlowProofResult result;
    size_t repeated_null;
    size_t changed_null;
    size_t linked_null;
    size_t reference_cleanup_null;
    size_t native_cleanup_null;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "null", operands, 1);
    operands[0] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "null", operands, 1);
    operands[0] = fixture_register(&fixture, 1);
    fixture_op(&fixture, "null", operands, 1);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    fixture_op(&fixture, "null", operands, 1);
    operands[0] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "null", operands, 1);
    operands[0] = fixture_register(&fixture, 3);
    operands[1] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "link", operands, 2);
    operands[0] = fixture_register(&fixture, 3);
    fixture_op(&fixture, "null", operands, 1);
    operands[0] = fixture_register(&fixture, 4);
    fixture_op(&fixture, "null", operands, 1);
    operands[0] = fixture_register(&fixture, 4);
    operands[1] = fixture_register(&fixture, 5);
    fixture_op(&fixture, "mkref", operands, 2);
    operands[0] = fixture_register(&fixture, 4);
    fixture_op(&fixture, "null", operands, 1);
    operands[0] = fixture_register(&fixture, 6);
    fixture_op(&fixture, "null", operands, 1);
    operands[0] = fixture_register(&fixture, 6);
    operands[1] = fixture_register(&fixture, 7);
    fixture_op(&fixture, "bcopy", operands, 2);
    operands[0] = fixture_register(&fixture, 6);
    fixture_op(&fixture, "null", operands, 1);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 27);
    check(procedure != 0,
          "redundant-absent proof fixture construction failed");
    if (procedure) {
        proof = rxas_flow_require_proof_service(procedure, 27, 0);
        repeated_null = rxas_flow_procedure_record(
                procedure, 27, 1)->instruction_id;
        changed_null = rxas_flow_procedure_record(
                procedure, 27, 4)->instruction_id;
        linked_null = rxas_flow_procedure_record(
                procedure, 27, 7)->instruction_id;
        reference_cleanup_null = rxas_flow_procedure_record(
                procedure, 27, 10)->instruction_id;
        native_cleanup_null = rxas_flow_procedure_record(
                procedure, 27, 13)->instruction_id;
        check(proof && rxas_flow_prove_redundant_absent_write(
                    proof, 27, repeated_null, &result) && result.proved,
              "repeated all-component absence was not proved redundant");
        check(rxas_flow_prove_redundant_absent_write(
                    proof, 27, changed_null, &result) && !result.proved &&
              result.reason == RXAS_FLOW_PROOF_COMPONENT_PRESENT,
              "present scalar component was incorrectly treated as absent");
        check(rxas_flow_prove_redundant_absent_write(
                    proof, 27, linked_null, &result) && result.proved,
              "linked all-component absence was not proved redundant");
        check(rxas_flow_prove_redundant_absent_write(
                    proof, 27, reference_cleanup_null, &result) &&
              !result.proved &&
              result.reason == RXAS_FLOW_PROOF_COMPONENT_PRESENT,
              "reference cleanup NULL was incorrectly deleted");
        check(rxas_flow_prove_redundant_absent_write(
                    proof, 27, native_cleanup_null, &result) &&
              !result.proved &&
              result.reason == RXAS_FLOW_PROOF_COMPONENT_PRESENT,
              "native-payload cleanup NULL was incorrectly deleted");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_redundant_self_copy_proof(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[2];
    const RxasFlowProofService *proof;
    RxasFlowProofResult result;
    size_t raw_copy;
    size_t raw_binary_copy;
    size_t linked_copy;
    size_t different_copy;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "copy", operands, 2);
    operands[0] = fixture_register(&fixture, 5);
    operands[1] = fixture_register(&fixture, 5);
    fixture_op(&fixture, "bcopy", operands, 2);
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_register(&fixture, 1);
    fixture_op(&fixture, "link", operands, 2);
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_register(&fixture, 1);
    fixture_op(&fixture, "dcopy", operands, 2);
    operands[0] = fixture_register(&fixture, 3);
    operands[1] = fixture_register(&fixture, 4);
    fixture_op(&fixture, "copy", operands, 2);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 28);
    check(procedure != 0,
          "redundant-self-copy proof fixture construction failed");
    if (procedure) {
        proof = rxas_flow_require_proof_service(procedure, 28, 0);
        raw_copy = rxas_flow_procedure_record(
                procedure, 28, 0)->instruction_id;
        raw_binary_copy = rxas_flow_procedure_record(
                procedure, 28, 1)->instruction_id;
        linked_copy = rxas_flow_procedure_record(
                procedure, 28, 3)->instruction_id;
        different_copy = rxas_flow_procedure_record(
                procedure, 28, 4)->instruction_id;
        check(proof && rxas_flow_prove_redundant_self_copy(
                    proof, 28, raw_copy, &result) && result.proved,
              "raw full self-copy was not proved redundant");
        check(rxas_flow_prove_redundant_self_copy(
                    proof, 28, raw_binary_copy, &result) && result.proved,
              "raw BCOPY self-copy was blocked by its general signal contract");
        check(rxas_flow_prove_redundant_self_copy(
                    proof, 28, linked_copy, &result) && result.proved,
              "linked typed copy did not follow physical storage identity");
        check(rxas_flow_prove_redundant_self_copy(
                    proof, 28, different_copy, &result) && !result.proved &&
              result.reason == RXAS_FLOW_PROOF_STORAGE_NOT_IDENTICAL,
              "different storage was incorrectly proved as a self-copy");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_sparse_use_and_liveness(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[3];
    instruction_queue *metadata;
    instruction_queue *trace;
    const RxasFlowSsaAnalysis *ssa;
    const RxasFlowUseAnalysis *uses;
    const RxasFlowUseMetrics *metrics;
    RxasFlowComponentFact fact;
    const RxasFlowUse *use;
    char *dump;
    size_t copy_instruction;
    size_t candidate_value;
    size_t index;
    int saw_explicit;
    int saw_metadata;
    int saw_trace;

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "icopy", operands, 2);
    fixture_record(&fixture, REG_META);
    metadata = &fixture.items[fixture.item_count - 1];
    metadata->operand3Token = fixture_register(&fixture, 1);
    fixture_record(&fixture, TRACE_EVENT);
    trace = &fixture.items[fixture.item_count - 1];
    trace->operand2Token = fixture_string(&fixture, "R");
    trace->operand3Token = fixture_string(&fixture, "I");
    trace->operand4Token = fixture_string(&fixture, "r");
    trace->operand5Token = fixture_integer(&fixture, 1);
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_register(&fixture, 1);
    operands[2] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "iadd", operands, 3);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 29);
    check(procedure != 0, "sparse-use fixture construction failed");
    if (procedure) {
        ssa = rxas_flow_require_ssa_analysis(procedure, 29, 0);
        copy_instruction = rxas_flow_procedure_record(
                procedure, 29, 1)->instruction_id;
        check(ssa && rxas_flow_component_at_instruction(
                    ssa, 29, copy_instruction, 1,
                    fixture_local_register(1), RXOP_COMPONENT_INTEGER,
                    &fact),
              "typed-copy result ValueId was unavailable");
        candidate_value = fact.value_id;
        check(rxas_flow_require_use_analysis(procedure, 29, 1) == 0,
              "bounded use analysis did not fail closed");
        metrics = rxas_flow_last_use_metrics(procedure, 29);
        check(metrics && metrics->status ==
                    RXAS_FLOW_ANALYSIS_BUDGET_EXHAUSTED,
              "bounded use analysis did not report budget exhaustion");
        uses = rxas_flow_require_use_analysis(procedure, 29, 0);
        metrics = rxas_flow_use_metrics(uses, 29);
        check(uses && metrics &&
              rxas_flow_require_use_analysis(procedure, 29, 0) == uses,
              "use analysis did not recover and cache by epoch");
        check(uses && rxas_flow_value_is_live(uses, 29, candidate_value),
              "typed-copy value did not retain sparse liveness");
        saw_explicit = 0;
        saw_metadata = 0;
        saw_trace = 0;
        for (index = 0; uses && index <
                    rxas_flow_use_count(uses, 29); index++) {
            use = rxas_flow_use(uses, 29, index);
            if (!use || !fixture_value_reaches_value(
                                uses, 29, candidate_value, use->value_id))
                continue;
            if (use && use->kind == RXAS_FLOW_USE_EXPLICIT_READ)
                saw_explicit = 1;
            if (use && use->kind == RXAS_FLOW_USE_METADATA_READ)
                saw_metadata = 1;
            if (use && use->kind == RXAS_FLOW_USE_TRACE_READ)
                saw_trace = 1;
        }
        check(saw_explicit && saw_metadata && saw_trace,
              "use index lost an explicit, metadata or TRACE observation");
        check(metrics && metrics->metadata_reads >= 8 &&
              metrics->trace_reads == 1 &&
              metrics->retained_bytes > 0,
              "use metrics did not classify retained observations");
        dump = uses ? fixture_use_dump(uses, 29) : 0;
        check(dump && strstr(dump, "PERF3 flow-use epoch=29") &&
              strstr(dump, "kind=metadata-read") &&
              strstr(dump, "kind=trace-read"),
              "use dump omitted stable observation diagnostics");
        free(dump);
        check(rxas_flow_use_metrics(uses, 30) == 0 &&
              rxas_flow_value_direct_use_count(
                    uses, 30, candidate_value) == 0,
              "stale use-analysis access did not fail closed");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "icopy", operands, 2);
    operands[0] = fixture_label_ref(&fixture, "left");
    operands[1] = fixture_register(&fixture, 3);
    fixture_op(&fixture, "brt", operands, 2);
    fixture_label(&fixture, "right");
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_integer(&fixture, 9);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_label_ref(&fixture, "join");
    fixture_op(&fixture, "br", operands, 1);
    fixture_label(&fixture, "left");
    fixture_label(&fixture, "join");
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_register(&fixture, 1);
    operands[2] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "iadd", operands, 3);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 30);
    check(procedure != 0, "phi-use fixture construction failed");
    if (procedure) {
        ssa = rxas_flow_require_ssa_analysis(procedure, 30, 0);
        copy_instruction = rxas_flow_procedure_record(
                procedure, 30, 1)->instruction_id;
        check(ssa && rxas_flow_component_at_instruction(
                    ssa, 30, copy_instruction, 1,
                    fixture_local_register(1), RXOP_COMPONENT_INTEGER,
                    &fact),
              "phi-use copy result was unavailable");
        candidate_value = fact.value_id;
        uses = rxas_flow_require_use_analysis(procedure, 30, 0);
        saw_explicit = 0;
        for (index = 0; uses && index <
                    rxas_flow_use_count(uses, 30); index++) {
            use = rxas_flow_use(uses, 30, index);
            if (use && use->kind == RXAS_FLOW_USE_EXPLICIT_READ &&
                fixture_value_reaches_value(
                        uses, 30, candidate_value, use->value_id))
                saw_explicit = 1;
        }
        check(uses && rxas_flow_value_dependent_count(
                    uses, 30, candidate_value) >= 1 &&
              saw_explicit &&
              rxas_flow_value_is_live(uses, 30, candidate_value),
              "phi dependency did not carry liveness to its input value");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

}

static void test_typed_copy_redirect_proof(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[4];
    instruction_queue *metadata;
    const RxasFlowProofService *proof;
    RxasFlowTypedCopyPlan plan;
    RxasFlowOperandRewrite rewrite;
    size_t candidate;

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "icopy", operands, 2);
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_register(&fixture, 1);
    operands[2] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "ieq", operands, 3);
    operands[0] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "ret", operands, 1);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 32);
    check(procedure != 0,
          "typed-copy redirect proof fixture construction failed");
    if (procedure) {
        proof = rxas_flow_require_proof_service(procedure, 32, 0);
        candidate = rxas_flow_procedure_record(
                procedure, 32, 1)->instruction_id;
        check(proof && rxas_flow_prove_typed_copy_redirect(
                    proof, 32, candidate, &plan) && plan.proved &&
              plan.reason == RXAS_FLOW_PROOF_PROVED &&
              plan.component == RXOP_COMPONENT_INTEGER &&
              plan.rewrite_count == 1,
              "exact typed-copy use was not proved atomically redirectable");
        check(rxas_flow_typed_copy_plan_rewrite(
                    proof, 32, &plan, 0, &rewrite) &&
              rewrite.instruction_id ==
                    rxas_flow_procedure_record(
                            procedure, 32, 2)->instruction_id &&
              rewrite.operand_index == 1 &&
              rewrite.expected_register.register_class ==
                    RXAS_FLOW_REGISTER_LOCAL &&
              rewrite.expected_register.number == 1 &&
              rewrite.replacement_register.register_class ==
                    RXAS_FLOW_REGISTER_LOCAL &&
              rewrite.replacement_register.number == 0,
              "typed-copy proof did not return the exact immutable rewrite");
        check(!rxas_flow_typed_copy_plan_rewrite(
                    proof, 33, &plan, 0, &rewrite),
              "stale typed-copy rewrite plan did not fail closed");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "icopy", operands, 2);
    fixture_record(&fixture, REG_META);
    metadata = &fixture.items[fixture.item_count - 1];
    metadata->operand3Token = fixture_register(&fixture, 1);
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_register(&fixture, 1);
    operands[2] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "ieq", operands, 3);
    operands[0] = fixture_register(&fixture, 2);
    fixture_op(&fixture, "ret", operands, 1);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 33);
    check(procedure != 0,
          "metadata-observed typed-copy fixture construction failed");
    if (procedure) {
        proof = rxas_flow_require_proof_service(procedure, 33, 0);
        candidate = rxas_flow_procedure_record(
                procedure, 33, 1)->instruction_id;
        check(proof && rxas_flow_prove_typed_copy_redirect(
                    proof, 33, candidate, &plan) && !plan.proved &&
              plan.reason == RXAS_FLOW_PROOF_USE_NOT_REDIRECTABLE &&
              plan.rewrite_count == 0,
              "metadata-observed typed copy was not rejected atomically");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_string(&fixture, "abc");
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 1);
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "scopy", operands, 2);
    operands[0] = fixture_register(&fixture, 2);
    operands[1] = fixture_integer(&fixture, 0);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 3);
    operands[1] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 4);
    operands[1] = fixture_register(&fixture, 1);
    operands[2] = fixture_register(&fixture, 2);
    operands[3] = fixture_register(&fixture, 3);
    fixture_op(&fixture, "substring", operands, 4);
    operands[0] = fixture_register(&fixture, 4);
    fixture_op(&fixture, "ret", operands, 1);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 34);
    check(procedure != 0,
          "explicit-slice typed-copy fixture construction failed");
    if (procedure) {
        proof = rxas_flow_require_proof_service(procedure, 34, 0);
        candidate = rxas_flow_procedure_record(
                procedure, 34, 1)->instruction_id;
        check(proof && rxas_flow_prove_typed_copy_redirect(
                    proof, 34, candidate, &plan) && plan.proved &&
              plan.reason == RXAS_FLOW_PROOF_PROVED &&
              plan.component == RXOP_COMPONENT_STRING &&
              plan.rewrite_count == 1,
              "explicit SUBSTRING did not permit SCOPY redirection");
        check(rxas_flow_typed_copy_plan_rewrite(
                    proof, 34, &plan, 0, &rewrite) &&
              rewrite.instruction_id ==
                    rxas_flow_procedure_record(
                            procedure, 34, 4)->instruction_id &&
              rewrite.operand_index == 1 &&
              rewrite.expected_register.number == 1 &&
              rewrite.replacement_register.number == 0,
              "explicit SUBSTRING redirection targeted the wrong operand");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_loop_proofs(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    Assembler_Token *operands[3];
    const RxasFlowProofService *proof;
    const RxasFlowStructuralAnalysis *structural;
    RxasFlowProofResult result;
    RxasFlowRepetitionKey first_key;
    RxasFlowRepetitionKey repeated_key;
    size_t loop_instruction;
    size_t loop_id;
    size_t first_itos;
    size_t repeated_itos;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 3);
    fixture_op(&fixture, "load", operands, 2);
    fixture_label(&fixture, "loop");
    operands[0] = fixture_label_ref(&fixture, "done");
    operands[1] = fixture_register(&fixture, 1);
    fixture_op(&fixture, "brt", operands, 2);
    operands[0] = fixture_label_ref(&fixture, "loop");
    fixture_op(&fixture, "br", operands, 1);
    fixture_label(&fixture, "done");
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 24);
    check(procedure != 0, "loop-proof fixture construction failed");
    if (procedure) {
        proof = rxas_flow_require_proof_service(procedure, 24, 0);
        structural = rxas_flow_require_loop_analysis(procedure, 24, 0);
        loop_instruction = rxas_flow_procedure_record(
                procedure, 24, 2)->instruction_id;
        loop_id = structural ? rxas_flow_structural_innermost_loop(
                structural, 24,
                rxas_flow_procedure_instruction(
                        procedure, 24, loop_instruction)->block_id)
                : RXAS_FLOW_ID_NONE;
        check(proof && loop_id != RXAS_FLOW_ID_NONE &&
              rxas_flow_prove_must_execute_in_loop(
                    proof, 24, loop_instruction, loop_id, &result) &&
              result.proved,
              "loop header was not proved must-execute");
        check(rxas_flow_prove_loop_component_invariant(
                    proof, 24, loop_instruction, loop_id,
                    fixture_local_register(0), RXOP_COMPONENT_INTEGER,
                    &result) && result.proved,
              "unchanged loop component was not proved invariant");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);

    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "load", operands, 2);
    fixture_label(&fixture, "repeat");
    operands[0] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_register(&fixture, 0);
    operands[2] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "iadd", operands, 3);
    operands[0] = fixture_label_ref(&fixture, "repeat");
    operands[1] = fixture_register(&fixture, 1);
    fixture_op(&fixture, "brt", operands, 2);
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 25);
    check(procedure != 0,
          "loop-carried repetition fixture construction failed");
    if (procedure) {
        proof = rxas_flow_require_proof_service(procedure, 25, 0);
        first_itos = rxas_flow_procedure_record(
                procedure, 25, 2)->instruction_id;
        repeated_itos = rxas_flow_procedure_record(
                procedure, 25, 3)->instruction_id;
        check(proof && rxas_flow_repetition_key(
                    proof, 25, first_itos, &first_key) &&
              rxas_flow_repetition_key(
                    proof, 25, repeated_itos, &repeated_key) &&
              first_key.storage_id == repeated_key.storage_id,
              "loop-carried local storage did not retain one proof key");
        check(rxas_flow_prove_repetition(
                    proof, 25, first_itos, repeated_itos, &result) &&
              result.proved && result.reason == RXAS_FLOW_PROOF_PROVED,
              "same-iteration write-once value was lost in a loop phi");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

typedef enum JoinedKeyFixtureMutation {
    JOINED_KEY_FIXTURE_NONE = 0,
    JOINED_KEY_FIXTURE_RIGHT_CHANGE,
    JOINED_KEY_FIXTURE_CACHE_CHANGE,
    JOINED_KEY_FIXTURE_REFERENCE_CHANGE,
    JOINED_KEY_FIXTURE_CALL_CHANGE,
    JOINED_KEY_FIXTURE_SIGNAL_TRACE,
    JOINED_KEY_FIXTURE_NO_LOOP
} JoinedKeyFixtureMutation;

typedef struct JoinedKeyFixtureRecords {
    size_t seed;
    size_t seed_trace;
    size_t candidate;
    size_t candidate_trace;
    size_t stem;
} JoinedKeyFixtureRecords;

static void fixture_joined_key_loop(
        FlowFixture *fixture, JoinedKeyFixtureMutation mutation,
        JoinedKeyFixtureRecords *records) {
    Assembler_Token *operands[3];
    memset(records, 0, sizeof(*records));
    operands[0] = fixture_register(fixture, 1);
    operands[1] = fixture_integer(fixture, 1);
    fixture_op(fixture, "load", operands, 2);
    operands[0] = fixture_register(fixture, 6);
    operands[1] = fixture_integer(fixture, 0);
    fixture_op(fixture, "load", operands, 2);
    operands[0] = fixture_register(fixture, 3);
    operands[1] = fixture_string(fixture, "value");
    fixture_op(fixture, "load", operands, 2);
    fixture_label(fixture, "joined_loop");
    operands[0] = fixture_label_ref(fixture, "joined_done");
    operands[1] = fixture_register(fixture, 6);
    fixture_op(fixture, "brt", operands, 2);
    operands[0] = fixture_register(fixture, 1);
    operands[1] = fixture_register(fixture, 1);
    operands[2] = fixture_integer(fixture, 1);
    fixture_op(fixture, "iadd", operands, 3);
    operands[0] = fixture_register(fixture, 1);
    fixture_op(fixture, "itos", operands, 1);
    records->seed = fixture->item_count;
    operands[0] = fixture_register(fixture, 5);
    operands[1] = fixture_string(fixture, "Key.");
    operands[2] = fixture_register(fixture, 1);
    fixture_op(fixture, "concat", operands, 3);
    records->seed_trace = fixture->item_count;
    fixture_trace_register(fixture, 5, "S");

    if (mutation == JOINED_KEY_FIXTURE_RIGHT_CHANGE) {
        operands[0] = fixture_register(fixture, 1);
        operands[1] = fixture_register(fixture, 1);
        operands[2] = fixture_integer(fixture, 1);
        fixture_op(fixture, "iadd", operands, 3);
        operands[0] = fixture_register(fixture, 1);
        fixture_op(fixture, "itos", operands, 1);
    }
    else if (mutation == JOINED_KEY_FIXTURE_CACHE_CHANGE) {
        operands[0] = fixture_register(fixture, 5);
        operands[1] = fixture_string(fixture, "changed");
        fixture_op(fixture, "load", operands, 2);
    }
    else if (mutation == JOINED_KEY_FIXTURE_REFERENCE_CHANGE) {
        operands[0] = fixture_register(fixture, 8);
        operands[1] = fixture_register(fixture, 5);
        fixture_op(fixture, "link", operands, 2);
        operands[0] = fixture_register(fixture, 8);
        operands[1] = fixture_string(fixture, "changed-through-link");
        fixture_op(fixture, "load", operands, 2);
    }
    else if (mutation == JOINED_KEY_FIXTURE_CALL_CHANGE) {
        operands[0] = fixture_register(fixture, 4);
        operands[1] = fixture_integer(fixture, 1);
        fixture_op(fixture, "load", operands, 2);
        operands[0] = fixture_register(fixture, 9);
        operands[1] = fixture_text_token(fixture, FUNC, "callee");
        operands[2] = fixture_register(fixture, 4);
        fixture_op(fixture, "call", operands, 3);
    }

    records->candidate = fixture->item_count;
    operands[0] = fixture_register(fixture, 2);
    operands[1] = fixture_string(fixture, "Key.");
    operands[2] = fixture_register(fixture, 1);
    fixture_op(fixture, "concat", operands, 3);
    records->candidate_trace = fixture->item_count;
    fixture_trace_register(fixture, 2, "S");
    records->stem = fixture->item_count;
    operands[0] = fixture_register(fixture, 4);
    operands[1] = fixture_register(fixture, 2);
    operands[2] = fixture_register(fixture, 3);
    fixture_op(fixture, "stemset", operands, 3);

    if (mutation == JOINED_KEY_FIXTURE_SIGNAL_TRACE) {
        operands[0] = fixture_register(fixture, 2);
        operands[1] = fixture_register(fixture, 4);
        operands[2] = fixture_register(fixture, 7);
        fixture_op(fixture, "stemget", operands, 3);
        fixture_trace_register(fixture, 2, "S");
    }
    if (mutation != JOINED_KEY_FIXTURE_NO_LOOP) {
        operands[0] = fixture_label_ref(fixture, "joined_loop");
        fixture_op(fixture, "br", operands, 1);
    }
    fixture_label(fixture, "joined_done");
    fixture_op(fixture, "ret", 0, 0);
}

static void check_joined_key_case(
        Assembler_Context *context, JoinedKeyFixtureMutation mutation,
        unsigned long epoch, int expected_proved,
        RxasFlowProofReason expected_reason) {
    FlowFixture fixture;
    JoinedKeyFixtureRecords records;
    RxasFlowProcedure *procedure;
    const RxasOptimisationPassDescriptor *descriptor;
    const RxasFlowProofService *proof;
    RxasFlowJoinedKeyReusePlan plan;
    RxasFlowOperandRewrite rewrite;
    RxasFlowTraceDeletion deletion;
    const RxasFlowRecord *seed;
    const RxasFlowRecord *candidate;
    const RxasFlowRecord *stem;
    memset(&fixture, 0, sizeof(fixture));
    fixture_joined_key_loop(&fixture, mutation, &records);
    procedure = rxas_flow_procedure_build(
            context, fixture.items, fixture.item_count, epoch);
    check(procedure != 0, "joined-key proof fixture construction failed");
    descriptor = rxas_optimisation_pass_descriptor(
            RXAS_PASS_H01_JOINED_KEY_REUSE);
    seed = procedure ? rxas_flow_procedure_record(
            procedure, epoch, records.seed) : 0;
    candidate = procedure ? rxas_flow_procedure_record(
            procedure, epoch, records.candidate) : 0;
    stem = procedure ? rxas_flow_procedure_record(
            procedure, epoch, records.stem) : 0;
    proof = procedure && descriptor
            ? rxas_flow_require_proof_capabilities(
                    procedure, epoch, descriptor->capabilities, 0)
            : 0;
    memset(&plan, 0, sizeof(plan));
    check(proof && seed && candidate && stem &&
          rxas_flow_prove_joined_key_reuse(
                  proof, epoch, seed->instruction_id,
                  candidate->instruction_id, stem->instruction_id, &plan),
          "joined-key proof query failed");
    if (plan.proved != expected_proved ||
        plan.reason != expected_reason)
        fprintf(stderr,
                "joined-key mutation=%d expected=%d/%s actual=%d/%s\n",
                (int)mutation, expected_proved,
                rxas_flow_proof_reason_name(expected_reason),
                plan.proved, rxas_flow_proof_reason_name(plan.reason));
    check(plan.proved == expected_proved &&
          plan.reason == expected_reason,
          "joined-key proof returned the wrong verdict");
    if (expected_proved) {
        check(plan.loop_id != RXAS_FLOW_ID_NONE &&
              plan.cache_register.number == 5 &&
              plan.candidate_register.number == 2 &&
              plan.stem_key_operand == 1 &&
              plan.seed_rewrite_count == 1 &&
              plan.trace_deletion_count == 1 &&
              !plan.preheader_eligible &&
              !plan.preheader_must_execute &&
              !plan.preheader_right_invariant &&
              !plan.preheader_trace_free,
              "joined-key proof lost its loop/cache/preheader audit");
        check(rxas_flow_joined_key_reuse_plan_seed_rewrite(
                      proof, epoch, &plan, 0, &rewrite) &&
              rewrite.record_id == records.seed_trace &&
              rewrite.instruction_id == RXAS_FLOW_ID_NONE &&
              rewrite.operand_index == RXAS_FLOW_ID_NONE &&
              rewrite.expected_register.number == 5,
              "joined-key proof returned the wrong seed TRACE rewrite");
        check(rxas_flow_joined_key_reuse_plan_trace_deletion(
                      proof, epoch, &plan, 0, &deletion) &&
              deletion.record_id == records.candidate_trace &&
              deletion.expected_register.number == 2 &&
              deletion.component == RXOP_COMPONENT_STRING,
              "joined-key proof returned the wrong TRACE deletion");
        check(!rxas_flow_joined_key_reuse_plan_trace_deletion(
                      proof, epoch + 1, &plan, 0, &deletion),
              "joined-key TRACE plan accepted a stale epoch");
    }
    if (procedure) rxas_flow_procedure_destroy(procedure);
    fixture_destroy(&fixture);
}

static void test_joined_key_reuse_proof(Assembler_Context *context) {
    check_joined_key_case(context, JOINED_KEY_FIXTURE_NONE, 42, 1,
                          RXAS_FLOW_PROOF_PROVED);
    check_joined_key_case(context, JOINED_KEY_FIXTURE_RIGHT_CHANGE, 43, 0,
                          RXAS_FLOW_PROOF_JOINED_KEY_NOT_EQUIVALENT);
    check_joined_key_case(context, JOINED_KEY_FIXTURE_CACHE_CHANGE, 44, 1,
                          RXAS_FLOW_PROOF_PROVED);
    check_joined_key_case(context, JOINED_KEY_FIXTURE_REFERENCE_CHANGE, 45, 0,
                          RXAS_FLOW_PROOF_USE_NOT_REDIRECTABLE);
    check_joined_key_case(context, JOINED_KEY_FIXTURE_CALL_CHANGE, 46, 0,
                          RXAS_FLOW_PROOF_CALL_WINDOW_OBSERVED);
    check_joined_key_case(context, JOINED_KEY_FIXTURE_SIGNAL_TRACE, 47, 0,
                          RXAS_FLOW_PROOF_TRACE_OBSERVED);
    check_joined_key_case(context, JOINED_KEY_FIXTURE_NO_LOOP, 48, 0,
                          RXAS_FLOW_PROOF_NOT_IN_LOOP);
}

static void test_optimisation_routing(Assembler_Context *context) {
    FlowFixture fixture;
    Assembler_Token *operands[3];
    RxasOptimisationCensus census;
    const RxasOptimisationPassDescriptor *descriptor;
    const RxasOptimisationPassDescriptor *base_descriptor;
    const RxasOptimisationPassDescriptor *use_descriptor;
    RxasFlowProcedure *procedure;
    const RxasFlowProofService *base_proof;
    const RxasFlowProofService *use_proof;
    RxasFlowProofResult result;
    RxasFlowTypedCopyPlan typed_plan;
    unsigned int capabilities;
    size_t index;
    memset(&fixture, 0, sizeof(fixture));
    operands[0] = fixture_register(&fixture, 0);
    operands[1] = fixture_integer(&fixture, 1);
    fixture_op(&fixture, "load", operands, 2);
    operands[0] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "itos", operands, 1);
    operands[0] = fixture_label_ref(&fixture, "done");
    operands[1] = fixture_register(&fixture, 0);
    fixture_op(&fixture, "brt", operands, 2);
    fixture_label(&fixture, "done");
    fixture_op(&fixture, "ret", 0, 0);

    check(rxas_optimisation_pass_count() == RXAS_PASS_COUNT,
          "optimisation route ledger is incomplete");
    for (index = 0; index < RXAS_PASS_COUNT; index++) {
        descriptor = rxas_optimisation_pass_descriptor(
                (RxasOptimisationPassId)index);
        check(descriptor && descriptor->id == (RxasOptimisationPassId)index &&
              descriptor->name && descriptor->capabilities &&
              descriptor->owner >= RXAS_OPT_OWNER_LOCAL &&
              descriptor->owner <= RXAS_OPT_OWNER_DIAGNOSTIC,
              "optimisation route descriptor is incomplete");
    }
    check(rxas_optimisation_census(
                  context, fixture.items, fixture.item_count, &census),
          "optimisation candidate census failed");
    check(census.instructions == 4,
          "optimisation census instruction count is wrong");
    check(census.candidates[RXAS_PASS_M00_REACHABILITY] == 4,
          "M00 structural candidate census is wrong");
    check(census.candidates[RXAS_PASS_M01_DERIVATION] == 1,
          "M01 derivation candidate census is wrong");
    check(census.candidates[RXAS_PASS_M02_CONSTANT] == 1,
          "M02 constant candidate census is wrong");
    check(census.candidates[RXAS_PASS_K05_BRANCH_THREAD] == 1,
          "K05 CFG candidate census is wrong");
    descriptor = rxas_optimisation_pass_descriptor(
            RXAS_PASS_K05_BRANCH_THREAD);
    check(descriptor && descriptor->owner == RXAS_OPT_OWNER_CFG &&
          descriptor->capabilities == RXAS_FLOW_CAP_CFG,
          "K05 requested a non-CFG capability");
    descriptor = rxas_optimisation_pass_descriptor(
            RXAS_PASS_K06_STATUS_COPY);
    check(descriptor && descriptor->owner == RXAS_OPT_OWNER_LOCAL &&
          descriptor->capabilities == RXAS_FLOW_CAP_LOCAL_SCAN,
          "K06 was not classified as local mechanical algebra");
    descriptor = rxas_optimisation_pass_descriptor(
            RXAS_PASS_DIAGNOSTIC_FLOW_DUMP);
    check(descriptor && descriptor->owner == RXAS_OPT_OWNER_DIAGNOSTIC &&
          (descriptor->capabilities & RXAS_FLOW_CAP_USE) != 0 &&
          (descriptor->capabilities & RXAS_FLOW_CAP_LOOPS) == 0,
          "explicit flow diagnostics do not declare their use facts");
    check((census.requested_capabilities & RXAS_FLOW_CAP_LOOPS) == 0,
          "current candidates requested dormant loop analysis");
    capabilities = rxas_optimisation_capabilities_for_owner(
            &census, RXAS_OPT_OWNER_SSA);
    check((capabilities & RXAS_FLOW_CAP_VALUE) != 0 &&
          (capabilities & RXAS_FLOW_CAP_USE) == 0 &&
          (capabilities & RXAS_FLOW_CAP_LOOPS) == 0,
          "SSA owner capability union is not candidate-lazy");

    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 41);
    base_descriptor = rxas_optimisation_pass_descriptor(
            RXAS_PASS_M01_DERIVATION);
    use_descriptor = rxas_optimisation_pass_descriptor(
            RXAS_PASS_K04_COMPARE_BRANCH);
    base_proof = procedure && base_descriptor
            ? rxas_flow_require_proof_capabilities(
                    procedure, 41, base_descriptor->capabilities, 0)
            : 0;
    capabilities = rxas_flow_proof_capabilities(base_proof, 41);
    check(base_proof &&
          (capabilities & base_descriptor->capabilities) ==
                  base_descriptor->capabilities &&
          (capabilities & (RXAS_FLOW_CAP_USE | RXAS_FLOW_CAP_LOOPS)) == 0,
          "base semantic consumer acquired undeclared use/loop facts");
    check(base_proof && rxas_flow_prove_typed_copy_redirect(
                  base_proof, 41, 0, &typed_plan) &&
          !typed_plan.proved &&
          typed_plan.reason == RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE &&
          (rxas_flow_proof_capabilities(base_proof, 41) &
                   RXAS_FLOW_CAP_USE) == 0,
          "undeclared use query did not fail closed");
    check(base_proof && rxas_flow_prove_must_execute_in_loop(
                  base_proof, 41, 0, RXAS_FLOW_ID_NONE, &result) &&
          !result.proved &&
          result.reason == RXAS_FLOW_PROOF_ANALYSIS_UNAVAILABLE &&
          (rxas_flow_proof_capabilities(base_proof, 41) &
                   RXAS_FLOW_CAP_LOOPS) == 0,
          "undeclared loop query did not fail closed");
    use_proof = procedure && use_descriptor
            ? rxas_flow_require_proof_capabilities(
                    procedure, 41, use_descriptor->capabilities, 0)
            : 0;
    capabilities = rxas_flow_proof_capabilities(use_proof, 41);
    check(use_proof == base_proof &&
          (capabilities & use_descriptor->capabilities) ==
                  use_descriptor->capabilities &&
          (capabilities & RXAS_FLOW_CAP_LOOPS) == 0,
          "use consumer did not monotonically extend the epoch service");
    check(!rxas_flow_require_proof_capabilities(
                  procedure, 41, RXAS_FLOW_CAP_LOCAL_SCAN, 0),
          "proof service accepted a local-only capability");
    if (procedure) rxas_flow_procedure_destroy(procedure);
    fixture_destroy(&fixture);
}

static void fixture_branch_thread_source(FlowFixture *fixture,
                                         const char *mnemonic,
                                         rxinteger condition) {
    Assembler_Token *operands[2];
    operands[0] = fixture_label_ref(fixture, "target");
    if (strcmp(mnemonic, "br") == 0)
        fixture_op(fixture, mnemonic, operands, 1);
    else {
        operands[1] = fixture_register(fixture, condition);
        fixture_op(fixture, mnemonic, operands, 2);
    }
}

static void fixture_branch_thread_target(FlowFixture *fixture,
                                         const char *mnemonic,
                                         rxinteger condition) {
    Assembler_Token *operands[3];
    operands[0] = fixture_label_ref(fixture, "true_exit");
    if (strcmp(mnemonic, "brtf") == 0) {
        operands[1] = fixture_label_ref(fixture, "false_exit");
        operands[2] = fixture_register(fixture, condition);
        fixture_op(fixture, mnemonic, operands, 3);
    }
    else {
        operands[1] = fixture_register(fixture, condition);
        fixture_op(fixture, mnemonic, operands, 2);
    }
}

static void fixture_branch_thread_named_case(FlowFixture *fixture,
                                             size_t case_index) {
    Assembler_Token *operands[2];
    char entry_name[32];
    char target_name[32];
    char true_name[32];
    snprintf(entry_name, sizeof(entry_name), "batch_entry_%lu",
             (unsigned long)case_index);
    snprintf(target_name, sizeof(target_name), "batch_target_%lu",
             (unsigned long)case_index);
    snprintf(true_name, sizeof(true_name), "batch_true_%lu",
             (unsigned long)case_index);
    fixture_label(fixture, entry_name);
    operands[0] = fixture_label_ref(fixture, target_name);
    fixture_op(fixture, "br", operands, 1);
    fixture_op(fixture, "ret", 0, 0);
    fixture_label(fixture, target_name);
    operands[0] = fixture_label_ref(fixture, true_name);
    operands[1] = fixture_register(fixture, (rxinteger)case_index);
    fixture_op(fixture, "brt", operands, 2);
    fixture_op(fixture, "ret", 0, 0);
    fixture_label(fixture, true_name);
    fixture_op(fixture, "ret", 0, 0);
}

static void check_branch_thread_case(
        Assembler_Context *context, const char *source_mnemonic,
        const char *target_mnemonic, RxasFlowBranchThreadKind expected_kind) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    RxasFlowBranchThreadPlan plan;
    memset(&fixture, 0, sizeof(fixture));
    fixture_label(&fixture, "entry");
    fixture_branch_thread_source(&fixture, source_mnemonic, 4);
    fixture_op(&fixture, "ret", 0, 0);
    fixture_label(&fixture, "target");
    fixture_record(&fixture, SRC_STEP);
    fixture_record(&fixture, TRACE_EVENT);
    fixture_branch_thread_target(&fixture, target_mnemonic, 4);
    fixture_op(&fixture, "ret", 0, 0);
    fixture_label(&fixture, "true_exit");
    fixture_op(&fixture, "ret", 0, 0);
    fixture_label(&fixture, "false_exit");
    fixture_op(&fixture, "ret", 0, 0);

    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 31);
    check(procedure != 0, "branch-thread graph construction failed");
    if (procedure) {
        check(rxas_flow_plan_branch_thread(procedure, 31, &plan) &&
              plan.kind == expected_kind &&
              plan.source_record_id == 1 && plan.target_record_id == 6,
              "branch-thread plan did not preserve an inherited rule shape");
        check(!rxas_flow_plan_branch_thread(procedure, 32, &plan),
              "branch-thread planner accepted a stale epoch");
        rxas_flow_procedure_destroy(procedure);
    }
    fixture_destroy(&fixture);
}

static void test_branch_thread_plans(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowProcedure *procedure;
    RxasFlowBranchThreadPlan plan;
    RxasFlowBranchThreadBatch batch;
    size_t index;

    check_branch_thread_case(context, "br", "brt",
                             RXAS_FLOW_BRANCH_THREAD_BR_BRT);
    check_branch_thread_case(context, "br", "brf",
                             RXAS_FLOW_BRANCH_THREAD_BR_BRF);
    check_branch_thread_case(context, "br", "brtf",
                             RXAS_FLOW_BRANCH_THREAD_BR_BRTF);
    check_branch_thread_case(context, "brt", "brt",
                             RXAS_FLOW_BRANCH_THREAD_BRT_BRT);
    check_branch_thread_case(context, "brf", "brf",
                             RXAS_FLOW_BRANCH_THREAD_BRF_BRF);
    check_branch_thread_case(context, "brt", "brf",
                             RXAS_FLOW_BRANCH_THREAD_BRT_BRF);
    check_branch_thread_case(context, "brf", "brt",
                             RXAS_FLOW_BRANCH_THREAD_BRF_BRT);

    /* One immutable epoch collects an independent large branch set instead
     * of forcing one complete-procedure rebuild per source. */
    memset(&fixture, 0, sizeof(fixture));
    for (index = 0; index < 24; index++)
        fixture_branch_thread_named_case(&fixture, index);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 37);
    memset(&batch, 0, sizeof(batch));
    check(procedure && rxas_flow_plan_branch_threads(
                  procedure, 37, &batch) && batch.plan_count == 24,
          "branch-thread planner did not batch all independent candidates");
    rxas_flow_branch_thread_batch_destroy(&batch);
    if (procedure) {
        check(!rxas_flow_plan_branch_threads(procedure, 38, &batch),
              "branch-thread batch planner accepted a stale epoch");
        rxas_flow_procedure_destroy(procedure);
    }
    rxas_flow_branch_thread_batch_destroy(&batch);
    fixture_destroy(&fixture);

    /* A target behind more than the old twenty-record queue remains visible. */
    memset(&fixture, 0, sizeof(fixture));
    fixture_label(&fixture, "entry");
    fixture_branch_thread_source(&fixture, "br", 2);
    for (index = 0; index < 25; index++) fixture_record(&fixture, SRC_STEP);
    fixture_label(&fixture, "target");
    fixture_branch_thread_target(&fixture, "brt", 2);
    fixture_op(&fixture, "ret", 0, 0);
    fixture_label(&fixture, "true_exit");
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 33);
    check(procedure && rxas_flow_plan_branch_thread(procedure, 33, &plan) &&
          plan.kind == RXAS_FLOW_BRANCH_THREAD_BR_BRT,
          "branch-thread planner retained the old queue-distance bound");
    if (procedure) rxas_flow_procedure_destroy(procedure);
    fixture_destroy(&fixture);

    /* Backward edges use the same label index and are not a special case. */
    memset(&fixture, 0, sizeof(fixture));
    fixture_label(&fixture, "target");
    fixture_branch_thread_target(&fixture, "brt", 2);
    fixture_op(&fixture, "ret", 0, 0);
    fixture_label(&fixture, "entry");
    fixture_branch_thread_source(&fixture, "br", 2);
    fixture_op(&fixture, "ret", 0, 0);
    fixture_label(&fixture, "true_exit");
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 34);
    check(procedure && rxas_flow_plan_branch_thread(procedure, 34, &plan) &&
          plan.kind == RXAS_FLOW_BRANCH_THREAD_BR_BRT &&
          plan.target_record_id < plan.source_record_id,
          "branch-thread planner rejected a backward target");
    if (procedure) rxas_flow_procedure_destroy(procedure);
    fixture_destroy(&fixture);

    /* Different physical conditions do not prove a conditional redirect. */
    memset(&fixture, 0, sizeof(fixture));
    fixture_label(&fixture, "entry");
    fixture_branch_thread_source(&fixture, "brt", 2);
    fixture_op(&fixture, "ret", 0, 0);
    fixture_label(&fixture, "target");
    fixture_branch_thread_target(&fixture, "brt", 3);
    fixture_op(&fixture, "ret", 0, 0);
    fixture_label(&fixture, "true_exit");
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 35);
    check(procedure && !rxas_flow_plan_branch_thread(procedure, 35, &plan),
          "branch-thread planner ignored different condition registers");
    if (procedure) rxas_flow_procedure_destroy(procedure);
    fixture_destroy(&fixture);

    /* The conditional must be the target block's first executable record. */
    memset(&fixture, 0, sizeof(fixture));
    fixture_label(&fixture, "entry");
    fixture_branch_thread_source(&fixture, "br", 2);
    fixture_op(&fixture, "ret", 0, 0);
    fixture_label(&fixture, "target");
    {
        Assembler_Token *operands[1];
        operands[0] = fixture_register(&fixture, 9);
        fixture_op(&fixture, "null", operands, 1);
    }
    fixture_branch_thread_target(&fixture, "brt", 2);
    fixture_op(&fixture, "ret", 0, 0);
    fixture_label(&fixture, "true_exit");
    fixture_op(&fixture, "ret", 0, 0);
    procedure = rxas_flow_procedure_build(context, fixture.items,
                                          fixture.item_count, 36);
    check(procedure && !rxas_flow_plan_branch_thread(procedure, 36, &plan),
          "branch-thread planner skipped a target executable instruction");
    if (procedure) rxas_flow_procedure_destroy(procedure);
    fixture_destroy(&fixture);
}

static void test_semantic_queue_batch(Assembler_Context *context) {
    FlowFixture fixture;
    RxasFlowQueueBatch batch;
    RxasFlowQueueBatchMetrics metrics;
    instruction_queue *planned;
    const instruction_queue *epoch_item;
    const instruction_queue *first_epoch_item;
    size_t record_id;

    memset(&fixture, 0, sizeof(fixture));
    fixture_op(&fixture, "ret", 0, 0);
    fixture_op(&fixture, "ret", 0, 0);
    fixture_trace_register(&fixture, 5, "S");
    context->procedure_queue = fixture.items;
    context->procedure_queue_items = fixture.item_count;
    check(rxas_flow_queue_batch_begin(
                  &batch, context, fixture.items, fixture.item_count),
          "semantic queue batch construction failed");
    check(rxas_flow_queue_batch_record_matches_epoch(&batch, 0),
          "semantic queue batch did not retain its epoch snapshot");
    planned = rxas_flow_queue_batch_edit(&batch, 0, &epoch_item);
    check(planned != 0 && epoch_item != 0 &&
          epoch_item->instrType == OP_CODE,
          "semantic queue batch did not snapshot an edited record");
    if (planned) planned->instrType = EMPTY;
    planned = rxas_flow_queue_batch_edit(&batch, 1, &epoch_item);
    if (planned) planned->instrType = ASM_LABEL;
    check(!rxas_flow_queue_batch_record_matches_epoch(&batch, 0),
          "semantic queue batch missed a planned record change");
    check(!rxas_flow_queue_batch_commit(&batch, &metrics) &&
          fixture.items[0].instrType == OP_CODE &&
          fixture.items[1].instrType == OP_CODE,
          "rejected semantic queue batch partially changed the live queue");
    rxas_flow_queue_batch_destroy(&batch);

    check(rxas_flow_queue_batch_begin(
                  &batch, context, fixture.items, fixture.item_count),
          "second semantic queue batch construction failed");
    planned = rxas_flow_queue_batch_edit(&batch, 0, &epoch_item);
    if (planned) planned->instrType = EMPTY;
    check(rxas_flow_queue_batch_commit(&batch, &metrics) &&
          metrics.records_changed == 1 &&
          metrics.records_deleted == 1 &&
          fixture.items[0].instrType == EMPTY &&
          fixture.items[1].instrType == OP_CODE &&
          fixture.items[2].instrType == TRACE_EVENT,
          "validated semantic queue batch did not commit atomically");
    rxas_flow_queue_batch_destroy(&batch);

    check(rxas_flow_queue_batch_begin(
                  &batch, context, fixture.items, fixture.item_count),
          "TRACE rewrite semantic queue batch construction failed");
    planned = rxas_flow_queue_batch_edit(&batch, 2, &epoch_item);
    if (planned) planned->operand5Token = fixture_integer(&fixture, 12);
    check(rxas_flow_queue_batch_commit(&batch, &metrics) &&
          metrics.records_changed == 1 &&
          metrics.operand_records_rewritten == 1 &&
          fixture.items[2].operand5Token &&
          fixture.items[2].operand5Token->token_type == INT &&
          fixture.items[2].operand5Token->token_value.integer == 12,
          "validated TRACE register rewrite did not commit atomically");
    rxas_flow_queue_batch_destroy(&batch);

    context->procedure_queue = 0;
    context->procedure_queue_items = 0;
    fixture_destroy(&fixture);
    for (record_id = 0; record_id < 20; record_id++)
        fixture_op(&fixture, "ret", 0, 0);
    context->procedure_queue = fixture.items;
    context->procedure_queue_items = fixture.item_count;
    check(rxas_flow_queue_batch_begin(
                  &batch, context, fixture.items, fixture.item_count),
          "growing semantic queue batch construction failed");
    first_epoch_item = 0;
    for (record_id = 0; record_id < fixture.item_count; record_id++) {
        planned = rxas_flow_queue_batch_edit(&batch, record_id, &epoch_item);
        if (!record_id) first_epoch_item = epoch_item;
        check(planned != 0 && epoch_item != 0,
              "growing semantic queue batch did not snapshot a record");
    }
    check(first_epoch_item == batch.entries[0].original &&
          first_epoch_item->instrType == OP_CODE &&
          first_epoch_item->instrToken != 0,
          "semantic queue batch growth invalidated an epoch snapshot");
    check(rxas_flow_queue_batch_commit(&batch, &metrics) &&
          metrics.records_changed == 0,
          "unchanged growing semantic queue batch did not commit");
    rxas_flow_queue_batch_destroy(&batch);
    context->procedure_queue = 0;
    context->procedure_queue_items = 0;
    fixture_destroy(&fixture);
}

int main(void) {
    Assembler_Context context;
    memset(&context, 0, sizeof(context));
    context.current_proc_name = "flow_graph_contract";
    context.binary.inst_size = 100;
    failures = 0;

    test_unreachable_and_mapping(&context);
    test_diamond(&context);
    test_nested_and_irreducible(&context);
    test_signal_roots_and_determinism(&context);
    test_call_boundary_and_unknown(&context);
    test_policy_stack_uncertainty(&context);
    test_policy_loop_identity(&context);
    test_sparse_storage_and_components(&context);
    test_attribute_path_storage(&context);
    test_argument_storage_and_call_effects(&context);
    test_propagated_call_failure_state(&context);
    test_storage_joins_and_loops(&context);
    test_signal_phase_component_edges(&context);
    test_ordered_swapn_mappings(&context);
    test_derivation_contexts(&context);
    test_fused_failure_mappings(&context);
    test_proof_service(&context);
    test_redundant_constant_proof(&context);
    test_redundant_absent_proof(&context);
    test_redundant_self_copy_proof(&context);
    test_sparse_use_and_liveness(&context);
    test_typed_copy_redirect_proof(&context);
    test_loop_proofs(&context);
    test_joined_key_reuse_proof(&context);
    test_optimisation_routing(&context);
    test_branch_thread_plans(&context);
    test_semantic_queue_batch(&context);

    if (failures) {
        fprintf(stderr, "RXAS immutable flow graph failures: %d\n", failures);
        return 1;
    }
    printf("RXAS immutable flow graph: PASS\n");
    return 0;
}
