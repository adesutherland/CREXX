#include <stdlib.h>
#include <string.h>
#include "rxcp_ast.h"
#include "rxcp_sym.h"
#include "rxvml.h"

/* Forward declaration if needed */
rxvml_value* rxcp_marshal_implicit_cmd(rxvml_context *ctx, ASTNode *cmd_node) {
    ASTNode *child;
    size_t num_children = 0;
    size_t i;
    rxvml_value *token_array;
    rxvml_value *tok_obj;

    if (!cmd_node) return NULL;

    /* Count children */
    child = cmd_node->child;
    while (child) {
        num_children++;
        child = child->sibling;
    }

    token_array = rxvml_array_new(ctx, num_children);
    if (!token_array) return NULL;

    child = cmd_node->child;
    for (i = 0; i < num_children && child; i++) {
        rxvml_token_desc d;
        memset(&d, 0, sizeof(d));

        if (child->token) {
            d.type = child->token->token_type;
            d.subtype = child->token->token_subtype;
            d.text = child->token->token_string;
            d.text_len = (size_t)child->token->length;
            d.line = child->token->line;
            d.column = child->token->column;
            d.length = child->token->length;
        } else {
            /* Fallback to node info if token is missing */
            d.line = child->line;
            d.column = child->column;
        }

        d.file = child->file_name;
        d.node_type = (int)child->node_type;
        d.node_number = child->node_number;
        d.ord_low = child->low_ordinal;
        d.ord_high = child->high_ordinal;

        if (child->symbolNode && child->symbolNode->symbol) {
            d.sym_name = child->symbolNode->symbol->name;
            d.sym_type = (int)child->symbolNode->symbol->type;
        }

        tok_obj = rxvml_make_token(ctx, &d);
        if (tok_obj) {
            rxvml_array_set(ctx, token_array, i + 1, tok_obj);
        }

        child = child->sibling;
    }

    return token_array;
}
