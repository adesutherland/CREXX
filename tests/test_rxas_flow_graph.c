/* Focused executable contract for the immutable PERF3 RXAS flow graph. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxas_flow_graph.h"
#include "rxas_flow_analysis.h"
#include "rxas_flow_signal.h"
#include "rxasgrmr.h"

#define FIXTURE_MAX_ITEMS 64
#define FIXTURE_MAX_TOKENS 160

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
        check(structural_metrics && structural_metrics->unreachable_blocks >= 1 &&
              rxas_flow_structural_scc(analysis, 1, dead_block) ==
                    RXAS_FLOW_ID_NONE,
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
                    RXAS_FLOW_ID_NONE,
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
        analysis = rxas_flow_require_structural_analysis(procedure, 4, 0);
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
    size_t loop_index;
    size_t retry_only_loops;
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
                               RXAS_FLOW_EDGE_SIGNAL_SKIP) &&
              fixture_has_edge(first, 7, inc_block, inc_block,
                               RXAS_FLOW_EDGE_SIGNAL_RETRY),
              "signal normal/skip/retry continuations are incomplete");
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
              metrics->signal_skip_edges >= 2 &&
              metrics->signal_retry_edges >= 2,
              "signal graph metrics are incomplete");
        structural_metrics = rxas_flow_structural_metrics(first_analysis, 7);
        retry_only_loops = 0;
        if (structural_metrics) {
            for (loop_index = 0; loop_index < structural_metrics->loops;
                 loop_index++) {
                const RxasFlowLoop *loop;
                loop = rxas_flow_structural_loop(
                        first_analysis, 7, loop_index);
                if (loop &&
                    (loop->flags & RXAS_FLOW_LOOP_SIGNAL_RETRY_ONLY))
                    retry_only_loops++;
            }
        }
        check(structural_metrics && retry_only_loops >= 2,
              "signal retry cycles were not distinguished in the loop forest");
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
        check(strstr(first_dump, "kind=signal-retry") != 0 &&
              strstr(first_dump, "kind=async-root") != 0,
              "graph dump omits typed signal structure");
        free(first_dump);
        free(second_dump);
        first_analysis_dump = fixture_analysis_dump(first_analysis, 7);
        second_analysis_dump = fixture_analysis_dump(second_analysis, 7);
        check(strcmp(first_analysis_dump, second_analysis_dump) == 0,
              "structural analysis dump is not deterministic");
        check(strstr(first_analysis_dump, "irreducible-sccs=") != 0 &&
              strstr(first_analysis_dump, "PERF3 flow-loop") != 0,
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
              fixture_has_edge(procedure, 10, unknown_block, unknown_block,
                               RXAS_FLOW_EDGE_SIGNAL_RETRY) &&
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

    if (failures) {
        fprintf(stderr, "RXAS immutable flow graph failures: %d\n", failures);
        return 1;
    }
    printf("RXAS immutable flow graph: PASS\n");
    return 0;
}
