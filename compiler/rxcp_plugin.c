#include <stdlib.h>
#include <string.h>
#include "rxcp_ast.h"
#include "rxcp_sym.h"
#include "rxvml.h"

/* Forward declaration if needed */
static void count_tokens(ASTNode *node, size_t *count) {
    if (!node) return;
    if (node->node_type == OP_CONCAT || node->node_type == OP_SCONCAT) {
        ASTNode *child = node->child;
        while (child) {
            count_tokens(child, count);
            child = child->sibling;
        }
    } else {
        (*count)++;
    }
}

static void collect_tokens(rxvml_context *ctx, ASTNode *node, rxvml_value *token_array, size_t *count) {
    if (!node) return;

    if (node->node_type == OP_CONCAT || node->node_type == OP_SCONCAT) {
        ASTNode *child = node->child;
        while (child) {
            collect_tokens(ctx, child, token_array, count);
            child = child->sibling;
        }
    } else {
        rxvml_token_desc d;
        memset(&d, 0, sizeof(d));

        if (node->token) {
            d.type = node->token->token_type;
            d.subtype = node->token->token_subtype;
            d.text = node->token->token_string;
            d.text_len = (size_t)node->token->length;
            d.line = node->token->line;
            d.column = node->token->column;
            d.length = node->token->length;
        } else {
            d.line = node->line;
            d.column = node->column;
        }

        d.file = node->file_name;
        d.node_type = (int)node->node_type;
        d.node_number = node->node_number;
        d.ord_low = node->low_ordinal;
        d.ord_high = node->high_ordinal;

        if (node->symbolNode && node->symbolNode->symbol) {
            d.sym_name = node->symbolNode->symbol->name;
            d.sym_type = (int)node->symbolNode->symbol->type;
        }

        rxvml_value *tok_obj = rxvml_make_token(ctx, &d);
        if (tok_obj) {
            (*count)++;
            rxvml_array_set(ctx, token_array, *count, tok_obj);
        }
    }
}

rxvml_value* rxcp_marshal_implicit_cmd(rxvml_context *ctx, ASTNode *cmd_node) {
    size_t num_tokens = 0;
    size_t count = 0;
    rxvml_value *token_array;

    if (!cmd_node) return NULL;

    /* Count tokens in the expression (the first child of IMPLICIT_CMD) */
    count_tokens(cmd_node->child, &num_tokens);

    token_array = rxvml_array_new(ctx, num_tokens);
    if (!token_array) return NULL;

    collect_tokens(ctx, cmd_node->child, token_array, &count);

    return token_array;
}
