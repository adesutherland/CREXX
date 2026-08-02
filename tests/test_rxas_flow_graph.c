/* Focused executable contract for the immutable PERF3 RXAS flow graph. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxas_flow_graph.h"
#include "rxas_flow_analysis.h"
#include "rxas_flow_proof.h"
#include "rxas_flow_signal.h"
#include "rxas_flow_ssa.h"
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
              after_call_storage.kind == RXAS_FLOW_STORAGE_UNKNOWN,
              "call did not invalidate a merged dynamic mapping");
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
        structural = rxas_flow_require_structural_analysis(procedure, 24, 0);
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
    test_argument_storage_and_call_effects(&context);
    test_storage_joins_and_loops(&context);
    test_signal_phase_component_edges(&context);
    test_derivation_contexts(&context);
    test_fused_failure_mappings(&context);
    test_proof_service(&context);
    test_loop_proofs(&context);

    if (failures) {
        fprintf(stderr, "RXAS immutable flow graph failures: %d\n", failures);
        return 1;
    }
    printf("RXAS immutable flow graph: PASS\n");
    return 0;
}
