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

int main(void) {
    Context context;
    ASTNode *root, *namespace_node, *procedure, *body, *branch, *label, *on, *on_bound, *off;
    ASTNode *binding;
    char source[] = "signal on syntax\nnext:\nsignal off syntax\n";
    memset(&context, 0, sizeof(context));
    context.file_name = "frame_control.crexx";
    context.buff_start = source;
    context.buff_end = source + sizeof(source) - 1;
    root = ast_ft(&context, REXX_UNIVERSE);
    namespace_node = ast_ft(&context, NAMESPACE);
    procedure = ast_ft(&context, PROCEDURE);
    body = ast_ft(&context, INSTRUCTIONS);
    branch = ast_ft(&context, FRAME_BRANCH);
    label = ast_ft(&context, FRAME_LABEL);
    on = ast_ft(&context, FRAME_HANDLER_ON);
    on_bound = ast_ft(&context, FRAME_HANDLER_ON);
    off = ast_ft(&context, FRAME_HANDLER_OFF);
    binding = ast_ft(&context, VAR_TARGET);
    ast_copy_str(binding, "caught_signal");
    binding->register_type = 'r';
    binding->register_num = 7;
    add_ast(on_bound, binding);
    context.ast = root;
    add_ast(root, namespace_node);
    add_ast(namespace_node, procedure);
    scp_f(&context, NULL, root, NULL, SCOPE_UNIVERSE);
    scp_f(&context, root->scope, namespace_node, NULL, SCOPE_NAMESPACE);
    scp_f(&context, namespace_node->scope, procedure, NULL, SCOPE_PROCEDURE);
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
    assert(rxcp_flow_analyze(&context, 0));
    assert(rxcp_flow_analyze(&context, 1));
    check_output(branch, "br l", NULL);
    check_output(label, "frame:", "\"frame_control.crexx\" 2 1 6 \"next:\"");
    check_output(on, "sigbr l", "\"CLASSIC_SYNTAX\"");
    check_output(on_bound, "sigbrv l", "r7,\"CLASSIC_SYNTAX\"");
    check_output(off, "sighalt \"CLASSIC_SYNTAX\"", NULL);
    rxcp_flow_free(&context);
    puts("PASS frame label, branch, handler AST flow and RXAS emission");
    return 0;
}
