#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef restrict
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#elif defined(__GNUC__) || defined(__clang__)
#define restrict __restrict
#elif defined(_MSC_VER)
#define restrict __restrict
#else
#define restrict
#endif
#endif

#include "dslsyntax_common.h"
#include "dslsyntax_editor.h"

#ifndef RXC_SYNTAXHIGHLIGHT_CMD
#error "RXC_SYNTAXHIGHLIGHT_CMD must be defined"
#endif

static int failures = 0;

static const char *syntax_error_source =
        "options levelb\n"
        "/* Syntax Error: Missing END */\n"
        "do i = 1 to 10\n"
        "    say i\n"
        "/* Missing END keyword */\n";

static const char *warning_source =
        "options levelb\n"
        "main: procedure\n"
        "  do\n"
        "    y = 3\n"
        "    say \"Inside block: y =\" y\n"
        "  end\n"
        "  say \"Outside block: y =\" y\n"
        "\n"
        "  return\n";

static const char *incomplete_parse_source =
        "options levelb\n"
        "main: procedure\n"
        "  parse into\n";

static const char *bare_parse_source =
        "options levelb\n"
        "main: procedure\n"
        "  parse\n";

static void expect_true(int condition, const char *message) {
    if (condition) return;
    fprintf(stderr, "FAIL: %s\n", message);
    failures = 1;
}

typedef struct TreeDiagnosticMatch {
    char severity;
    const char *message_substring;
    size_t count;
} TreeDiagnosticMatch;

typedef struct BufferDiagnosticMatch {
    char severity;
    const char *message_substring;
    size_t count;
} BufferDiagnosticMatch;

static void collect_tree_diagnostic_matches(CB_Node *node, size_t depth, void *user_data) {
    TreeDiagnosticMatch *match;

    (void)depth;

    if (!node || node->type != SYNTAX_ERROR) return;

    match = (TreeDiagnosticMatch *)user_data;
    if (node->severity != match->severity) return;
    if (match->message_substring &&
        (!node->message || !strstr(node->message, match->message_substring))) return;

    match->count++;
}

static void scan_buffer_for_leaf_diagnostics(CodeBuffer *cb, BufferDiagnosticMatch *match) {
    size_t line_index;
    size_t char_index;

    if (!cb || !match) return;

    for (line_index = 0; line_index < cb->line_count; line_index++) {
        for (char_index = 0; char_index < cb->lines[line_index].length; char_index++) {
            CodeBufferCharacter *ch;

            ch = &cb->lines[line_index].characters[char_index];
            if (ch->severity != match->severity) continue;
            if (!ch->node) continue;
            if (!ch->node->message || ch->node->message[0] == 0) continue;
            if (match->message_substring && !strstr(ch->node->message, match->message_substring)) continue;
            match->count++;
        }
    }
}

static CodeBuffer *load_document_in_editor(const char *document_id,
                                           const char *source,
                                           CommunicationFunctions **comm_out) {
    CommunicationFunctions *comm;
    CodeBuffer *cb;
    InitialLoad *load;

    if (comm_out) *comm_out = 0;

    comm = create_stdio_communication_functions(RXC_SYNTAXHIGHLIGHT_CMD);
    expect_true(comm != 0, "stdio parser communication should be created");
    if (!comm) return 0;

    cb = create_code_buffer(comm, 0);
    expect_true(cb != 0, "editor code buffer should be created");
    if (!cb) {
        free_stdio_communication_functions(comm);
        return 0;
    }

    load = create_initial_load(document_id, source);
    expect_true(load != 0, "initial load should be created");
    if (!load) {
        free_code_buffer(cb);
        free_stdio_communication_functions(comm);
        return 0;
    }

    load_initial_content(cb, load);
    if (editor_is_parsing_thread_active()) {
        expect_true(join_parser_thread() == 0, "editor parser thread should finish cleanly");
    }

    expect_true(cb->parse_tree != 0 && cb->parse_tree->root != 0, "editor parse should populate a parse tree");
    expect_true(cb->parse_tree != 0 && cb->parse_tree->root->type == PARSE_TREE_FILE,
                "editor parse should produce a PARSE_TREE_FILE root");

    if (comm_out) *comm_out = comm;
    else free_stdio_communication_functions(comm);
    return cb;
}

static void free_editor_parse(CodeBuffer *cb, CommunicationFunctions *comm) {
    if (editor_is_parsing_thread_active()) join_parser_thread();
    if (cb) free_code_buffer(cb);
    if (comm) free_stdio_communication_functions(comm);
}

static void test_editor_receives_syntax_error_diagnostics(void) {
    CommunicationFunctions *comm;
    CodeBuffer *cb;
    TreeDiagnosticMatch tree_match;
    BufferDiagnosticMatch buffer_match;

    comm = 0;
    cb = load_document_in_editor("err_02_syntax.rexx", syntax_error_source, &comm);
    if (!cb) return;

    memset(&tree_match, 0, sizeof(tree_match));
    tree_match.severity = CB_ERROR;
    cb_walk_tree_top_down(cb->parse_tree, collect_tree_diagnostic_matches, &tree_match);
    expect_true(tree_match.count > 0, "syntax-error document should keep explicit SYNTAX_ERROR nodes");

    memset(&buffer_match, 0, sizeof(buffer_match));
    buffer_match.severity = CB_ERROR;
    expect_true(enter_codeblock_critical_section() == 0, "buffer scan should enter critical section");
    scan_buffer_for_leaf_diagnostics(cb, &buffer_match);
    expect_true(exit_codeblock_critical_section() == 0, "buffer scan should exit critical section");
    expect_true(buffer_match.count > 0,
                "syntax-error document should expose error severity/message on editor-visible characters");

    free_editor_parse(cb, comm);
}

static void test_editor_receives_warning_diagnostics(void) {
    CommunicationFunctions *comm;
    CodeBuffer *cb;
    TreeDiagnosticMatch tree_match;
    BufferDiagnosticMatch buffer_match;

    comm = 0;
    cb = load_document_in_editor("test_disjoint_detailed.rexx", warning_source, &comm);
    if (!cb) return;

    memset(&tree_match, 0, sizeof(tree_match));
    tree_match.severity = CB_WARNING;
    tree_match.message_substring = "original definition @ 4:5";
    cb_walk_tree_top_down(cb->parse_tree, collect_tree_diagnostic_matches, &tree_match);
    expect_true(tree_match.count > 0, "warning document should keep explicit SYNTAX_ERROR warning nodes");

    memset(&buffer_match, 0, sizeof(buffer_match));
    buffer_match.severity = CB_WARNING;
    buffer_match.message_substring = "original definition @ 4:5";
    expect_true(enter_codeblock_critical_section() == 0, "buffer scan should enter critical section");
    scan_buffer_for_leaf_diagnostics(cb, &buffer_match);
    expect_true(exit_codeblock_critical_section() == 0, "buffer scan should exit critical section");
    expect_true(buffer_match.count > 0,
                "warning document should expose warning severity/message on editor-visible characters");

    free_editor_parse(cb, comm);
}

static void test_editor_handles_incomplete_parse_exit(void) {
    CommunicationFunctions *comm;
    CodeBuffer *cb;
    TreeDiagnosticMatch tree_match;
    BufferDiagnosticMatch buffer_match;

    comm = 0;
    cb = load_document_in_editor("incomplete_parse.rexx", incomplete_parse_source, &comm);
    if (!cb) return;

    memset(&tree_match, 0, sizeof(tree_match));
    tree_match.severity = CB_ERROR;
    tree_match.message_substring = "PARSE INTO requires a target variable";
    cb_walk_tree_top_down(cb->parse_tree, collect_tree_diagnostic_matches, &tree_match);
    expect_true(tree_match.count > 0,
                "incomplete PARSE should report a recoverable syntax/exit diagnostic in the parse tree");

    memset(&buffer_match, 0, sizeof(buffer_match));
    buffer_match.severity = CB_ERROR;
    buffer_match.message_substring = "PARSE INTO requires a target variable";
    expect_true(enter_codeblock_critical_section() == 0, "buffer scan should enter critical section");
    scan_buffer_for_leaf_diagnostics(cb, &buffer_match);
    expect_true(exit_codeblock_critical_section() == 0, "buffer scan should exit critical section");
    expect_true(buffer_match.count > 0,
                "incomplete PARSE should expose its diagnostic on editor-visible characters");

    free_editor_parse(cb, comm);
}

static void test_editor_handles_bare_parse_exit(void) {
    CommunicationFunctions *comm;
    CodeBuffer *cb;
    TreeDiagnosticMatch tree_match;
    BufferDiagnosticMatch buffer_match;

    comm = 0;
    cb = load_document_in_editor("bare_parse.rexx", bare_parse_source, &comm);
    if (!cb) return;

    memset(&tree_match, 0, sizeof(tree_match));
    tree_match.severity = CB_ERROR;
    tree_match.message_substring = "PARSE requires arguments";
    cb_walk_tree_top_down(cb->parse_tree, collect_tree_diagnostic_matches, &tree_match);
    expect_true(tree_match.count > 0,
                "bare PARSE should report a user-facing missing-arguments diagnostic in the parse tree");

    memset(&buffer_match, 0, sizeof(buffer_match));
    buffer_match.severity = CB_ERROR;
    buffer_match.message_substring = "PARSE requires arguments";
    expect_true(enter_codeblock_critical_section() == 0, "buffer scan should enter critical section");
    scan_buffer_for_leaf_diagnostics(cb, &buffer_match);
    expect_true(exit_codeblock_critical_section() == 0, "buffer scan should exit critical section");
    expect_true(buffer_match.count > 0,
                "bare PARSE should expose a user-facing missing-arguments diagnostic on editor-visible characters");

    free_editor_parse(cb, comm);
}

int main(void) {
    editor_init();

    test_editor_receives_syntax_error_diagnostics();
    test_editor_receives_warning_diagnostics();
    test_editor_handles_incomplete_parse_exit();
    test_editor_handles_bare_parse_exit();

    editor_free();
    return failures ? 1 : 0;
}
