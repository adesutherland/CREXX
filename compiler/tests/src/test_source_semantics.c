#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxcpmain.h"
#include "rxcp_val.h"
#include "rxcp_source_tree.h"
#include "rxcp_highlight_controller.h"
#include "dslsyntax_common.h"
#include "dslsyntax_editor.h"

static int failures = 0;

static const char *test_source =
        "options levelb\n"
        "value = .int\n"
        "value = 1\n"
        "say value\n"
        "say twice(value)\n"
        "return\n"
        "\n"
        "twice: procedure = .int\n"
        "    arg n = .int\n"
        "    return n + n\n";

static void expect_true(int condition, const char *message) {
    if (condition) return;
    fprintf(stderr, "FAIL: %s\n", message);
    failures = 1;
}

static int ast_name_equals(ASTNode *node, const char *name) {
    size_t length;

    if (!name) return 1;
    if (!node || !node->node_string) return 0;

    length = strlen(name);
    return node->node_string_length == length &&
           strncmp(node->node_string, name, length) == 0;
}

static ASTNode *find_ast_node(ASTNode *node, NodeType type, const char *name, int line) {
    ASTNode *found;

    while (node) {
        if (node->node_type == type &&
            ast_name_equals(node, name) &&
            (line < 0 || node->line == line)) {
            return node;
        }
        if (node->child) {
            found = find_ast_node(node->child, type, name, line);
            if (found) return found;
        }
        node = node->sibling;
    }
    return 0;
}

static ASTNode *find_ast_node_any_name(ASTNode *node,
                                       NodeType type,
                                       const char *first_name,
                                       const char *second_name) {
    ASTNode *found;

    found = find_ast_node(node, type, first_name, -1);
    if (found) return found;
    if (second_name) return find_ast_node(node, type, second_name, -1);
    return 0;
}

static void test_source_tree_semantics_sync(void) {
    Context *context;
    ASTNode *value_definition;
    ASTNode *value_reference;
    ASTNode *procedure_definition;
    ASTNode *function_call;
    SourceSemanticInfo *value_def_semantics;
    SourceSemanticInfo *value_ref_semantics;
    SourceSemanticInfo *procedure_semantics;
    SourceSemanticInfo *call_semantics;

    context = rxcp_parse_buffer((char *)test_source, 0);
    expect_true(context != 0, "parser should return a context");
    if (!context) return;

    context->disable_exits = 1;
    rxcp_prepare_source_ast(context);
    source_tree_sync_diagnostics(context);
    expect_true(!context->source_diagnostics_list, "source tree should be syntax-error free before validation");

    validate_ast(context);
    source_tree_sync_diagnostics(context);
    source_tree_sync_semantics(context);

    expect_true(context->source_tree != 0, "source tree should exist after validation");
    expect_true(!context->source_diagnostics_list, "validation should not produce diagnostics for the semantics fixture");

    value_definition = find_ast_node(context->ast, VAR_TARGET, "value", -1);
    value_reference = find_ast_node(context->ast, VAR_SYMBOL, "value", -1);
    procedure_definition = find_ast_node_any_name(context->ast, PROCEDURE, "twice", "twice:");
    function_call = find_ast_node(context->ast, FUNCTION, "twice", -1);

    expect_true(value_definition != 0, "typed value definition should be found");
    expect_true(value_reference != 0, "value reference should be found");
    expect_true(procedure_definition != 0, "procedure definition should be found");
    expect_true(function_call != 0, "function call should be found");

    if (!value_definition || !value_reference || !procedure_definition || !function_call) {
        fre_cntx(context);
        return;
    }

    value_def_semantics = value_definition->source_node ? value_definition->source_node->semantics : 0;
    value_ref_semantics = value_reference->source_node ? value_reference->source_node->semantics : 0;
    procedure_semantics = procedure_definition->source_node ? procedure_definition->source_node->semantics : 0;
    call_semantics = function_call->source_node ? function_call->source_node->semantics : 0;

    expect_true(value_def_semantics != 0, "definition source node should receive semantic sidecar");
    expect_true(value_ref_semantics != 0, "reference source node should receive semantic sidecar");
    expect_true(procedure_semantics != 0, "procedure source node should receive semantic sidecar");
    expect_true(call_semantics != 0, "call source node should receive semantic sidecar");

    if (!value_def_semantics || !value_ref_semantics || !procedure_semantics || !call_semantics) {
        fre_cntx(context);
        return;
    }

    expect_true(value_def_semantics->identifier_id > 0, "value definition should have identifier_id");
    expect_true(value_def_semantics->identifier_id == value_ref_semantics->identifier_id,
                "value definition and reference should share identifier_id");
    expect_true(value_def_semantics->identifier_kind == SOURCE_SEMANTIC_VARIABLE,
                "value definition should be classified as a variable");
    expect_true(value_def_semantics->symbol_type == VARIABLE_SYMBOL,
                "value definition should carry variable symbol type");
    expect_true(value_def_semantics->value_type == TP_INTEGER,
                "value definition should carry resolved integer value type");
    expect_true(value_ref_semantics->value_type == TP_INTEGER,
                "value reference should carry resolved integer value type");
    expect_true(value_def_semantics->is_definition,
                "value definition should be marked as a definition");
    expect_true(value_ref_semantics->read_usage,
                "value reference should carry read usage");

    expect_true(procedure_semantics->identifier_id > 0, "procedure should have identifier_id");
    expect_true(procedure_semantics->identifier_id == call_semantics->identifier_id,
                "procedure definition and call should share identifier_id");
    expect_true(procedure_semantics->identifier_kind == SOURCE_SEMANTIC_FUNCTION,
                "procedure definition should be classified as a function");
    expect_true(call_semantics->identifier_kind == SOURCE_SEMANTIC_FUNCTION,
                "function call should be classified as a function");
    expect_true(procedure_semantics->value_type == TP_INTEGER,
                "procedure definition should carry resolved return type");

    fre_cntx(context);
}

typedef struct LeafMatchCollector {
    CodeBuffer *cb;
    const char *text;
    CB_NodeType expected_type;
    int require_type;
    int prefix_match;
    int ids[16];
    CB_NodeType types[16];
    size_t count;
} LeafMatchCollector;

static void collect_leaf_matches(CB_Node *node, size_t depth, void *user_data) {
    LeafMatchCollector *collector;
    char *value;

    (void)depth;

    if (!node || node->type >= PARSE_TREE) return;

    collector = (LeafMatchCollector *)user_data;
    value = 0;
    get_code_buffer_part(collector->cb, node->pos, node->length, 0, 0, &value);
    if (!value) return;

    if (((collector->prefix_match && strncmp(value, collector->text, strlen(collector->text)) == 0) ||
         (!collector->prefix_match && strcmp(value, collector->text) == 0)) &&
        (!collector->require_type || node->type == collector->expected_type) &&
        collector->count < (sizeof(collector->ids) / sizeof(collector->ids[0]))) {
        collector->ids[collector->count] = node->identifier_id;
        collector->types[collector->count] = node->type;
        collector->count++;
    }

    free(value);
}

static void test_highlight_semantic_projection(void) {
    CodeBuffer *cb;
    InitialLoad *load;
    LeafMatchCollector value_matches;
    LeafMatchCollector twice_matches;
    size_t i;

    rxcp_highlight_controller_reset_cache();

    cb = create_code_buffer(NULL, rxc_highlight_controller_parse);
    expect_true(cb != 0, "code buffer should be created");
    if (!cb) return;

    load = create_initial_load("stage5_semantics.rexx", test_source);
    expect_true(load != 0, "initial load should be created");
    if (!load) {
        free_code_buffer(cb);
        return;
    }

    base_load_initial_content(cb, load);
    expect_true(cb->parse_tree != 0 && cb->parse_tree->root != 0, "parser mode should produce a parse tree");
    expect_true(cb->parse_tree != 0 && cb->parse_tree->root->type == PARSE_TREE_FILE,
                "parser mode should produce PARSE_TREE_FILE root");

    memset(&value_matches, 0, sizeof(value_matches));
    value_matches.cb = cb;
    value_matches.text = "value";
    value_matches.expected_type = LEXER_IDENTIFIER;
    value_matches.require_type = 1;
    cb_walk_tree_top_down(cb->parse_tree, collect_leaf_matches, &value_matches);

    memset(&twice_matches, 0, sizeof(twice_matches));
    twice_matches.cb = cb;
    twice_matches.text = "twice";
    twice_matches.expected_type = LEXER_FUNCTION_IDENTIFIER;
    twice_matches.require_type = 1;
    twice_matches.prefix_match = 1;
    cb_walk_tree_top_down(cb->parse_tree, collect_leaf_matches, &twice_matches);

    expect_true(value_matches.count >= 4, "semantic projection should keep all value identifiers");
    expect_true(twice_matches.count >= 2, "semantic projection should classify procedure and call as function identifiers");

    if (value_matches.count > 0) {
        expect_true(value_matches.ids[0] > 0, "value identifiers should carry non-zero identifier_id");
        for (i = 1; i < value_matches.count; i++) {
            expect_true(value_matches.ids[i] == value_matches.ids[0],
                        "all value identifiers should share identifier_id");
        }
    }

    if (twice_matches.count > 0) {
        expect_true(twice_matches.ids[0] > 0, "function identifiers should carry non-zero identifier_id");
        for (i = 1; i < twice_matches.count; i++) {
            expect_true(twice_matches.ids[i] == twice_matches.ids[0],
                        "all function identifiers should share identifier_id");
        }
    }

    free_code_buffer(cb);
    rxcp_highlight_controller_reset_cache();
}

int main(void) {
    test_source_tree_semantics_sync();
    test_highlight_semantic_projection();

    if (failures) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
