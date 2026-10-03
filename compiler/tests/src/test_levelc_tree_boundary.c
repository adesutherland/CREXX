#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxcpmain.h"
#include "rxcp_levelc_lower.h"

static int failures = 0;

static void expect_result(ASTNode *root, int expected, const char *reason, const char *message) {
    const char *actual_reason = NULL;
    int result = rxcp_levelc_verify_lowered_tree(root, &actual_reason);

    if (result == expected &&
        (expected || (actual_reason && strcmp(actual_reason, reason) == 0))) return;
    fprintf(stderr, "FAIL: %s (result=%d, reason=%s)\n",
            message, result, actual_reason ? actual_reason : "none");
    failures = 1;
}

int main(void) {
    Context *context = cntx_f();
    ASTNode *root = ast_ft(context, REXX_UNIVERSE);
    ASTNode *file = ast_ft(context, PROGRAM_FILE);
    ASTNode *first = ast_ft(context, NOP);
    ASTNode *second = ast_ft(context, NOP);

    context->ast = root;
    add_ast(root, file);
    add_ast(file, first);
    add_ast(file, second);

    expect_result(root, 1, NULL, "canonical nested tree should pass");

    first->parent = root;
    expect_result(root, 0, "lowered tree has inconsistent parent ownership",
                  "wrong parent should fail");
    first->parent = file;

    second->node_type = LEVELC_DROP;
    expect_result(root, 0, "lowered tree retains a Level C-only node",
                  "residual Level C instruction should fail");
    second->node_type = PARSE;
    expect_result(root, 0, "lowered tree retains a Level C-only node",
                  "residual Classic PARSE instruction should fail");
    second->node_type = TEMPLATES;
    expect_result(root, 0, "lowered tree retains a Level C-only node",
                  "residual Classic template should fail");
    second->node_type = NOP;

    second->sibling = first;
    expect_result(root, 0, "lowered tree has a sibling cycle",
                  "sibling cycle should fail");
    second->sibling = NULL;

    expect_result(NULL, 0, "lowered tree has no root", "missing root should fail");
    expect_result(root, 1, NULL, "restored tree should pass");

    fre_cntx(context);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
