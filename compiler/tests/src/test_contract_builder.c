/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcp_contract.h"
#include "rxcp_val.h"
#include "rxcp_source_tree.h"
int main(void) {
    Context *root = cntx_f();
    Context *c;
    ASTNode *decl;
    root->master_context = root;
    root->file_name = "contract-test";
    c = rxcp_contract_context(root, "provider.rxbin", "_rxsysb");
    decl = rxcp_contract_callable(c, PROCEDURE, "_close", ".int",
                                 "name=.string,?nomsg=.int", 0);
    assert(decl);
    add_ast(ast_chld(c->ast->child, INSTRUCTIONS, 0), decl);
    decl = rxcp_contract_callable(c, PROCEDURE, "pos", ".int",
                                 "needle=.string,haystack=.string,start=1", 0);
    assert(decl && decl->sibling && ast_nchd(decl->sibling) == 3);
    assert(ast_chdn(decl->sibling, 2)->is_opt_arg);
    add_ast(ast_chld(c->ast->child, INSTRUCTIONS, 0), decl);
    assert(decl->is_metadata_declaration && decl->line == -1);
    assert(c->defer_imported_inline_attachments);
    {
        Context *provider = cntx_f();
        ASTNode *original = ast_ft(provider, FACTORY);
        char *source = strdup("    *: factory\n");
        provider->file_name = "source-provider.crexx";
        cntx_buf(provider, source, strlen(source));
        original->file_name = provider->file_name;
        original->line = 4; original->column = 4;
        original->source_start = source + 4; original->source_end = source + 13;
        rxcp_contract_source_anchor(decl, original);
        fre_cntx(provider);
        assert(decl->line == 4 && decl->column == 4);
        assert(strcmp(decl->file_name, "source-provider.crexx") == 0);
        assert(strcmp(decl->source_node->owned_source_text, "    *: factory") == 0);
    }
    assert(rxcp_contract_type(c, "reference .rxcp..tokens[-2 to 3, *]") != NULL);
    assert(rxcp_contract_type(c, ".") == NULL);
    assert(rxcp_contract_callable(c, PROCEDURE, "bad", ".int", "value=.", 0) == NULL);
    rxcp_contract_finish(c);
    assert(prnterrs(c) == 0);
    {
        Context *consumer = cntx_f();
        Context *generated = cntx_f();
        ASTNode *copy;
        ASTNode *synthetic = ast_ft(generated, NOP);
        consumer->file_name = "consumer.crexx";
        consumer->ast = ast_ft(consumer, REXX_UNIVERSE);
        source_tree_build(consumer, consumer->ast);
        copy = ast_dup(consumer, decl);
        generated->file_name = "generated.crexx";
        generated->ast = ast_ft(generated, REXX_UNIVERSE);
        ast_copy_source_anchor(synthetic, decl, AST_SOURCE_SYNTHETIC);
        add_ast(generated->ast, synthetic);
        add_ast(consumer->ast, copy);
        assert(copy->source_node != decl->source_node);
        assert(copy->source_node->context == consumer);
        assert(synthetic->source_node->context == generated);
        assert(synthetic->source_provenance == AST_SOURCE_SYNTHETIC);
        copy->value_type = TP_OBJECT;
        free(copy->value_class);
        copy->value_class = strdup("item");
        source_tree_sync_semantics(consumer);
        assert(consumer->source_tree->child == NULL); /* Anchor is detached. */
        assert(copy->source_node->semantics);
        assert(strcmp(copy->source_node->semantics->value_class, "item") == 0);
        source_tree_clear_semantics(consumer);
        assert(copy->source_node->semantics == NULL);
        source_tree_sync_semantics(consumer); /* Teardown must release it too. */
        assert(copy->source_node->semantics);
        fre_cntx(c);
        assert(strcmp(copy->source_node->owned_source_text, "    *: factory") == 0);
        assert(strcmp(copy->file_name, "source-provider.crexx") == 0);
        fre_cntx(consumer);
        assert(strcmp(synthetic->file_name, "source-provider.crexx") == 0);
        fre_cntx(generated);
    }
    fre_cntx(root);
    puts("Neutral declaration types, flags and authored RXAS literal defaults pass");
    return 0;
}
