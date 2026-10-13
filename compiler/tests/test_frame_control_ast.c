/* Canonical frame-control nodes must survive flow analysis and emit RXAS. */
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcp_emit.h"
#include "rxcp_flow.h"

static void check_output(ASTNode *node, const char *expected,
                         const char *also_expected) {
    FILE *stream;
    char buffer[4096];
    size_t length;
    stream = tmpfile();
    assert(stream);
    emit_flow(node, NULL);
    print_output(stream, node->output);
    rewind(stream);
    length = fread(buffer, 1, sizeof(buffer) - 1, stream);
    buffer[length] = 0;
    assert(strstr(buffer, expected));
    if (also_expected) assert(strstr(buffer, also_expected));
    fclose(stream);
}

/* Exercise the ancestor path independently of register allocation: only an
 * already-entered IF branch owns a live condition, and the target remains live.
 * The detached exit copy must not consume the normal-path cleanup chain. */
static void check_condition_exit(Context *context, ASTNode *procedure,
                                 NodeType control_type, int path) {
    ASTNode *target, *target_body, *repeat, *outer, *outer_condition, *outer_body;
    ASTNode *inner, *condition, *then_body, *else_body, *exit_node, *label;
    FILE *stream;
    OutputFragment *fragment;
    char buffer[4096];
    size_t length;
    const char *inner_unlink, *outer_unlink, *branch_text;

    target = ast_ft(context, control_type == LEAVE_WITH ? BLOCK_EXPR : DO);
    add_ast(procedure, target);
    if (control_type == LEAVE_WITH) {
        target->cleanup = output_fs("   unlink r73\n");
    } else {
        repeat = ast_ft(context, REPEAT);
        repeat->cleanup = output_fs("   unlink r73\n");
        add_ast(target, repeat);
    }
    target_body = ast_ft(context, INSTRUCTIONS);
    add_ast(target, target_body);
    outer = ast_ft(context, IF);
    outer_condition = ast_ft(context, BLOCK_EXPR);
    outer_condition->cleanup = output_fs("   unlink r72\n");
    outer_body = ast_ft(context, INSTRUCTIONS);
    add_ast(target_body, outer);
    add_ast(outer, outer_condition);
    add_ast(outer, outer_body);
    inner = ast_ft(context, IF);
    condition = ast_ft(context, BLOCK_EXPR);
    condition->cleanup = output_fs("   unlink r71\n");
    then_body = ast_ft(context, INSTRUCTIONS);
    else_body = ast_ft(context, INSTRUCTIONS);
    add_ast(outer_body, inner);
    add_ast(inner, condition);
    add_ast(inner, then_body);
    add_ast(inner, else_body);
    exit_node = ast_ft(context, control_type);
    add_ast(path == 0 ? condition : (path == 1 ? then_body : else_body), exit_node);
    exit_node->association = target;
    if (control_type == FRAME_BRANCH) {
        label = ast_ft(context, FRAME_LABEL);
        add_ast(target, label);
        exit_node->association = label;
    }
    emit_flow(exit_node, NULL);
    stream = tmpfile();
    assert(stream);
    /* Inspect emitter fragments before the final superinstruction combiner;
     * runtime coverage separately exercises optimized/fused assembly. */
    fragment = exit_node->output;
    while (fragment->before) fragment = fragment->before;
    for (; fragment; fragment = fragment->after) {
        if (fragment->output) fputs(fragment->output, stream);
    }
    rewind(stream);
    length = fread(buffer, 1, sizeof(buffer) - 1, stream);
    buffer[length] = 0;
    fclose(stream);
    inner_unlink = strstr(buffer, "unlink r71");
    outer_unlink = strstr(buffer, "unlink r72");
    branch_text = strstr(buffer, "   br ");
    if (!outer_unlink || !branch_text || outer_unlink >= branch_text) {
        fprintf(stderr, "control=%d path=%d output:\n%s", control_type, path, buffer);
    }
    assert((inner_unlink != NULL) == (path != 0));
    assert(outer_unlink && branch_text && outer_unlink < branch_text);
    if (inner_unlink) assert(inner_unlink < outer_unlink);
    assert(!strstr(buffer, "r73"));
    assert(!condition->cleanup->before && !condition->cleanup->after);
    assert(!strcmp(condition->cleanup->output, "   unlink r71\n"));
    assert(!outer_condition->cleanup->before && !outer_condition->cleanup->after);
    assert(!strcmp(outer_condition->cleanup->output, "   unlink r72\n"));
}

int main(void) {
    Context *context;
    ASTNode *root, *namespace_node, *procedure, *body, *branch, *label, *on, *on_bound, *off;
    ASTNode *binding;
    int path;
    const char fixture[] = "signal on syntax\nnext:\nsignal off syntax\n";
    char *source = malloc(sizeof(fixture));
    assert(source);
    memcpy(source, fixture, sizeof(fixture));
    context = cntx_f();
    assert(context);
    context->file_name = "frame_control.crexx";
    cntx_buf(context, source, sizeof(fixture) - 1);
    root = ast_ft(context, REXX_UNIVERSE);
    namespace_node = ast_ft(context, NAMESPACE);
    procedure = ast_ft(context, PROCEDURE);
    body = ast_ft(context, INSTRUCTIONS);
    branch = ast_ft(context, FRAME_BRANCH);
    label = ast_ft(context, FRAME_LABEL);
    on = ast_ft(context, FRAME_HANDLER_ON);
    on_bound = ast_ft(context, FRAME_HANDLER_ON);
    off = ast_ft(context, FRAME_HANDLER_OFF);
    binding = ast_ft(context, VAR_TARGET);
    ast_copy_str(binding, "caught_signal");
    binding->register_type = 'r';
    binding->register_num = 7;
    add_ast(on_bound, binding);
    context->ast = root;
    add_ast(root, namespace_node);
    add_ast(namespace_node, procedure);
    scp_f(context, NULL, root, NULL, SCOPE_UNIVERSE);
    scp_f(context, root->scope, namespace_node, NULL, SCOPE_NAMESPACE);
    scp_f(context, namespace_node->scope, procedure, NULL, SCOPE_PROCEDURE);
    add_ast(procedure, body);
    add_ast(body, branch);
    add_ast(body, label);
    add_ast(body, on);
    add_ast(body, on_bound);
    add_ast(body, off);
    branch->association = label;
    on->association = label;
    on_bound->association = label;
    ast_copy_str(label, "NEXT");
    ast_copy_str(on, "SYNTAX");
    ast_copy_str(on_bound, "SYNTAX");
    ast_copy_str(off, "SYNTAX");
    label->line = 1;
    label->column = 0;
    label->source_start = source + 17;
    label->source_end = source + 21;
    label->source_provenance = AST_SOURCE_EXACT;
    rxcp_validate_ast_and_symbols(root);
    assert(rxcp_flow_analyze(context, 0));
    assert(rxcp_flow_analyze(context, 1));
    check_output(branch, "br l", NULL);
    check_output(label, "frame:", "\"frame_control.crexx\" 2 1 6 \"next:\"");
    check_output(on, "sigbr l", "\"CLASSIC_SYNTAX\"");
    check_output(on_bound, "sigbrv l", "r7,\"CLASSIC_SYNTAX\"");
    check_output(off, "sighalt \"CLASSIC_SYNTAX\"", NULL);
    for (path = 0; path < 3; path++) {
        check_condition_exit(context, procedure, ITERATE, path);
        check_condition_exit(context, procedure, LEAVE, path);
        check_condition_exit(context, procedure, LEAVE_WITH, path);
        check_condition_exit(context, procedure, FRAME_BRANCH, path);
    }
    fre_cntx(context);
    puts("PASS frame label, branch, handler AST flow and RXAS emission");
    return 0;
}
