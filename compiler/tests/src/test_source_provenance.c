#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxcpmain.h"
#include "rxcp_ast_rewrite.h"
#include "rxcp_source_tree.h"

static int failures = 0;

static void expect_true(int condition, const char *message) {
    if (condition) return;
    fprintf(stderr, "FAIL: %s\n", message);
    failures = 1;
}

static void test_source_tree_marks_exact(void) {
    Context *context;
    ASTNode *root;
    ASTNode *file_node;
    ASTNode *instructions;
    SourceNode *source_root;

    context = cntx_f();
    root = ast_ft(context, REXX_UNIVERSE);
    file_node = ast_ft(context, PROGRAM_FILE);
    instructions = ast_ft(context, INSTRUCTIONS);
    context->ast = root;

    add_ast(root, file_node);
    add_ast(file_node, instructions);

    source_root = source_tree_build(context, root);

    expect_true(source_root != 0, "source tree root should be built");
    expect_true(root->source_node == source_root, "root should map to the source tree root");
    expect_true(root->source_provenance == AST_SOURCE_EXACT, "root provenance should be exact");
    expect_true(file_node->source_node != 0, "child node should map to a source node");
    expect_true(file_node->source_provenance == AST_SOURCE_EXACT, "child provenance should be exact");
    expect_true(instructions->source_node != 0, "grandchild node should map to a source node");
    expect_true(instructions->source_provenance == AST_SOURCE_EXACT, "grandchild provenance should be exact");

    fre_cntx(context);
}

static void test_add_ast_inherits_source_anchor(void) {
    Context *context;
    ASTNode *parent;
    ASTNode *child;
    SourceNode dummy_source;

    context = cntx_f();
    parent = ast_ft(context, PROGRAM_FILE);
    child = ast_ft(context, INSTRUCTIONS);

    dummy_source.context = context;
    ast_set_primary_source_node(parent, &dummy_source, AST_SOURCE_EXACT);
    add_ast(parent, child);

    expect_true(child->source_node == &dummy_source, "add_ast should inherit the parent's source node");
    expect_true(child->source_provenance == AST_SOURCE_INHERITED, "add_ast should mark inherited provenance");

    fre_cntx(context);
}

static void test_ast_fstk_preserves_source_anchor(void) {
    Context *context;
    ASTNode *source_node;
    ASTNode *copy_node;
    SourceNode dummy_source;

    context = cntx_f();
    source_node = ast_ftt(context, VAR_SYMBOL, "value");

    dummy_source.context = context;
    ast_set_primary_source_node(source_node, &dummy_source, AST_SOURCE_EXACT);

    copy_node = ast_fstk(context, source_node);

    expect_true(copy_node->source_node == &dummy_source, "ast_fstk should preserve the source node");
    expect_true(copy_node->source_provenance == AST_SOURCE_EXACT, "ast_fstk should preserve exact provenance");

    fre_cntx(context);
}

static void test_ast_execute_rewrite_marks_synthetic(void) {
    Context *context;
    ASTNode *root;
    ASTNode *target;
    ASTNode *rewritten;
    ASTRewriteTemplate *template;
    SourceNode dummy_source;

    context = cntx_f();
    root = ast_ft(context, REXX_UNIVERSE);
    target = ast_ft(context, NOP);

    dummy_source.context = context;
    ast_set_primary_source_node(target, &dummy_source, AST_SOURCE_EXACT);
    add_ast(root, target);

    template = ast_rw_new(CALL, NULL);
    rewritten = ast_execute_rewrite(context, target, template);

    expect_true(rewritten != 0, "rewrite should create a replacement node");
    expect_true(root->child == rewritten, "rewrite should replace the original child");
    expect_true(rewritten->source_node == &dummy_source, "rewrite should keep the primary source node");
    expect_true(rewritten->source_provenance == AST_SOURCE_SYNTHETIC, "rewrite should mark synthetic provenance");

    fre_cntx(context);
}

static void test_source_diagnostics_sync_to_source_tree(void) {
    Context *context;
    ASTNode *root;
    ASTNode *file_node;
    ASTNode *instructions;
    SourceDiagnostic *diag;
    static char source_line[] = "say 'hello'\n";

    context = cntx_f();
    context->file_name = "test.rexx";
    root = ast_ft(context, REXX_UNIVERSE);
    file_node = ast_ft(context, PROGRAM_FILE);
    instructions = ast_ft(context, INSTRUCTIONS);
    context->ast = root;

    instructions->file_name = context->file_name;
    instructions->line = 0;
    instructions->column = 0;
    instructions->source_start = source_line;
    instructions->source_end = source_line + strlen(source_line) - 2;

    add_ast(root, file_node);
    add_ast(file_node, instructions);

    source_tree_build(context, root);
    expect_true(context->source_tree != 0, "source tree should exist before syncing diagnostics");
    expect_true(instructions->source_node != 0, "instruction should keep a source node anchor");
    mknd_err(instructions, "SYNC_TEST");
    expect_true(instructions->child != 0 && instructions->child->node_type == ERROR,
                "instruction should receive an ERROR child");
    expect_true(instructions->child != 0 && instructions->child->source_node == instructions->source_node,
                "diagnostic child should inherit the instruction source node");
    source_tree_sync_diagnostics(context);
    expect_true(instructions->source_node->diagnostics != 0, "source node should receive a diagnostic sidecar");

    diag = context->source_diagnostics_list;
    expect_true(diag != 0, "source diagnostics should be created");
    if (!diag) {
        fre_cntx(context);
        return;
    }
    expect_true(diag->owner == instructions->source_node, "diagnostic owner should be the mapped source node");
    expect_true(diag->severity == SOURCE_DIAG_ERROR, "diagnostic severity should be preserved");
    expect_true(strcmp(diag->message, "SYNC_TEST") == 0, "diagnostic message should be preserved");
    expect_true(!diag->is_internal, "user diagnostics should not be marked internal");

    fre_cntx(context);
}

static void test_synthetic_diagnostics_mark_internal(void) {
    Context *context;
    ASTNode *root;
    ASTNode *file_node;
    ASTNode *instructions;
    ASTNode *call_node;
    ASTNode *synthetic_block;
    SourceDiagnostic *diag;
    static char call_line[] = "call work()\n";

    context = cntx_f();
    context->file_name = "test.rexx";
    root = ast_ft(context, REXX_UNIVERSE);
    file_node = ast_ft(context, PROGRAM_FILE);
    instructions = ast_ft(context, INSTRUCTIONS);
    call_node = ast_ft(context, CALL);
    context->ast = root;

    call_node->file_name = context->file_name;
    call_node->line = 0;
    call_node->column = 0;
    call_node->source_start = call_line;
    call_node->source_end = call_line + strlen(call_line) - 2;

    add_ast(root, file_node);
    add_ast(file_node, instructions);
    add_ast(instructions, call_node);

    source_tree_build(context, root);
    expect_true(call_node->source_node != 0, "call node should keep a source node anchor");

    synthetic_block = ast_ft(context, INSTRUCTIONS);
    synthetic_block->mark_internal_diagnostics = 1;
    ast_set_primary_source_node(synthetic_block, call_node->source_node, AST_SOURCE_SYNTHETIC);
    add_ast(instructions, synthetic_block);
    mknd_err(synthetic_block, "INTERNAL_TEST");
    expect_true(synthetic_block->child != 0 && synthetic_block->child->source_node == call_node->source_node,
                "synthetic diagnostic should inherit the originating source node");

    source_tree_sync_diagnostics(context);
    diag = context->source_diagnostics_list;
    while (diag && strcmp(diag->message, "INTERNAL_TEST") != 0) diag = diag->next_in_context;

    expect_true(diag != 0, "synthetic diagnostic should be synced");
    if (!diag) {
        fre_cntx(context);
        return;
    }
    expect_true(diag->owner == call_node->source_node, "synthetic diagnostic should anchor to the originating source node");
    expect_true(diag->is_internal, "synthetic diagnostic should be marked internal");

    fre_cntx(context);
}

static void test_reporting_anchor_helpers(void) {
    Context *context;
    ASTNode *root;
    ASTNode *file_node;
    ASTNode *instructions;
    ASTNode *call_a;
    ASTNode *call_b;
    ASTNode *synthetic_block;
    static char call_a_line[] = "call a()\n";
    static char call_b_line[] = "call b()\n";

    context = cntx_f();
    context->file_name = "test.rexx";
    root = ast_ft(context, REXX_UNIVERSE);
    file_node = ast_ft(context, PROGRAM_FILE);
    instructions = ast_ft(context, INSTRUCTIONS);
    call_a = ast_ft(context, CALL);
    call_b = ast_ft(context, CALL);
    context->ast = root;

    call_a->file_name = context->file_name;
    call_a->line = 0;
    call_a->column = 0;
    call_a->source_start = call_a_line;
    call_a->source_end = call_a_line + strlen(call_a_line) - 2;

    call_b->file_name = context->file_name;
    call_b->line = 1;
    call_b->column = 0;
    call_b->source_start = call_b_line;
    call_b->source_end = call_b_line + strlen(call_b_line) - 2;

    add_ast(root, file_node);
    add_ast(file_node, instructions);
    add_ast(instructions, call_a);
    add_ast(instructions, call_b);

    source_tree_build(context, root);

    synthetic_block = ast_ft(context, INSTRUCTIONS);
    ast_set_primary_source_node(synthetic_block, call_a->source_node, AST_SOURCE_SYNTHETIC);
    ast_enable_primary_reporting_anchor(synthetic_block);
    ast_add_reporting_source_node(synthetic_block, call_b->source_node);

    expect_true(synthetic_block->emit_primary_reporting_anchor == 1, "primary reporting anchor flag should be set");
    expect_true(synthetic_block->reporting_source_count == 1, "additional reporting anchors should be stored");
    expect_true(synthetic_block->reporting_source_nodes[0] == call_b->source_node, "additional reporting anchor should match the source node");

    {
        char *metalines = get_reporting_metalines(synthetic_block);
        expect_true(strstr(metalines, ".src 1:1=\"call a()\"") != 0,
                    "primary reporting anchor should emit the original source line");
        expect_true(strstr(metalines, ".src 2:1=\"call b()\"") != 0,
                    "additional reporting anchors should emit extra source lines");
        free(metalines);
    }

    fre_cntx(context);
}

static void test_detached_diagnostics_do_not_duplicate_on_repeat_sync(void) {
    Context *context;
    ASTNode *root;
    ASTNode *file_node;
    ASTNode *instructions;
    ASTNode *diag;
    SourceDiagnostic *source_diag;
    int count;
    static char source_line[] = "say 'hello'\n";

    context = cntx_f();
    context->file_name = "test.rexx";
    root = ast_ft(context, REXX_UNIVERSE);
    file_node = ast_ft(context, PROGRAM_FILE);
    instructions = ast_ft(context, INSTRUCTIONS);
    context->ast = root;

    instructions->file_name = context->file_name;
    instructions->line = 0;
    instructions->column = 0;
    instructions->source_start = source_line;
    instructions->source_end = source_line + strlen(source_line) - 2;

    add_ast(root, file_node);
    add_ast(file_node, instructions);
    source_tree_build(context, root);

    diag = ast_ft(context, ERROR);
    ast_copy_str(diag, "DETACHED_SYNC_TEST");
    diag->file_name = context->file_name;
    diag->line = instructions->line;
    diag->column = instructions->column;
    diag->source_start = instructions->source_start;
    diag->source_end = instructions->source_end;
    ast_set_primary_source_node(diag, instructions->source_node, AST_SOURCE_INHERITED);
    context->diagnostics_list = diag;

    source_tree_sync_diagnostics(context);
    source_tree_sync_diagnostics(context);

    count = 0;
    source_diag = context->source_diagnostics_list;
    while (source_diag) {
        if (strcmp(source_diag->message, "DETACHED_SYNC_TEST") == 0) count++;
        source_diag = source_diag->next_in_context;
    }

    expect_true(count == 1, "detached diagnostics should be mirrored only once across repeated syncs");
    fre_cntx(context);
}

int main(void) {
    test_source_tree_marks_exact();
    test_add_ast_inherits_source_anchor();
    test_ast_fstk_preserves_source_anchor();
    test_ast_execute_rewrite_marks_synthetic();
    test_source_diagnostics_sync_to_source_tree();
    test_synthetic_diagnostics_mark_internal();
    test_reporting_anchor_helpers();
    test_detached_diagnostics_do_not_duplicate_on_repeat_sync();

    if (failures) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
