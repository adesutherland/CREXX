#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
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

static rxvml_context* rxcp_init_bridge(Context* ctx) {
    if (ctx->rxvml_bridge) return (rxvml_context*)ctx->rxvml_bridge;

    /* Use master context if available */
    Context* root = ctx->master_context ? ctx->master_context : ctx;
    if (root->rxvml_bridge) return (rxvml_context*)root->rxvml_bridge;

    rxvml_context* vctx = rxvml_create(root->location, root->debug_mode ? 1 : 0);
    if (!vctx) return NULL;

    const char* mod = getenv("RXCP_EXIT_MODULE");
    if (!mod) mod = "compiler_exit";

    if (rxvml_load_module_file(vctx, mod) <= 0) {
        /* Note: could log error here */
    }

    root->rxvml_bridge = vctx;
    return vctx;
}

int rxcp_exit_bridge_invoke(Context* ctx, ASTNode* node) {
    rxvml_context* vctx = rxcp_init_bridge(ctx);
    if (!vctx) {
        /* Silent Fallback */
        return 0;
    }

    rxvml_value* tok_array = rxcp_marshal_implicit_cmd(vctx, node);
    if (!tok_array) {
        /* Silent Fallback */
        return 0;
    }

    rxvml_value* response = NULL;
    if (rxvml_call_plugin(vctx, "rxcp.exit_dispatch", tok_array, &response) != 0) {
        /* Silent Fallback (e.g. Procedure not found) */
        return 0;
    }

    const char* replacement_code = rxvml_get_replacement_code(vctx, response);
    if (!replacement_code) {
        /* Check for explicit error message in response */
        const char* err_msg = rxvml_get_error_message(vctx, response);
        if (err_msg) {
            mknd_err(node, (char*)err_msg);
            return -1;
        }
        /* No replacement and no error -> silent fallback */
        return 0;
    }

    /* Parse replacement code */
    Context* frag = cntx_f();
    if (!frag) return -1;
    cntx_buf(frag, (char*)replacement_code, strlen(replacement_code));
    if (rexbscan(frag) || rexbpars(frag)) {
        mknd_err(node, "EXIT_BRIDGE_PARSE_FAILED");
        fre_cntx(frag);
        return -1;
    }
    rxcp_bvl(frag);

    if (frag->ast) {
        /* TODO: Source Location Patching */
        /* Grafting */
        /* Copy the new AST into the main context */
        ASTNode* graft = add_dast(node->parent, frag->ast);
        if (graft) {
            ast_del(graft); /* Remove from the end where add_dast put it */
            ast_rpl(node, graft); /* Replace original node with graft */
            ctx->changed = 1;
        }
    }

    fre_cntx(frag);
    return 0;
}
