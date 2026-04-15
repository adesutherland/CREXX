#include <stdio.h>
#include <stdlib.h>

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

int main(void) {
    test_source_tree_marks_exact();
    test_add_ast_inherits_source_anchor();
    test_ast_fstk_preserves_source_anchor();
    test_ast_execute_rewrite_marks_synthetic();

    if (failures) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
