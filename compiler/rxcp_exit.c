#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxvml.h"
#include "platform.h"

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
            d.text = node->node_string;
            d.text_len = node->node_string_length;
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
            if (rxvml_get_debug_mode(ctx) >= 2) {
                fprintf(stderr, "DEBUG_EXIT: Marshaled token %zu: type=%d, text=\"%.*s\"\n",
                        *count, d.type, (int)d.text_len, d.text);
            }
            rxvml_array_set(ctx, token_array, *count, tok_obj);
        }
    }
}

rxvml_value* rxcp_marshal_implicit_cmd(rxvml_context *ctx, ASTNode *cmd_node) {
    size_t num_tokens = 0;
    size_t count = 0;
    rxvml_value *token_array;
    ASTNode *command_expression = cmd_node->child;

    if (!cmd_node) return NULL;

    if (cmd_node->node_type == ADDRESS) {
        // First child is environment, second is command
        if (command_expression) command_expression = command_expression->sibling;
    }

    if (!command_expression) return NULL;

    /* Count tokens in the expression */
    count_tokens(command_expression, &num_tokens);

    if (cmd_node->context->debug_mode >= 2) {
        fprintf(stderr, "DEBUG_EXIT: Marshaling %zu tokens for node type %d\n", num_tokens, cmd_node->node_type);
    }

    token_array = rxvml_array_new(ctx, num_tokens);
    if (!token_array) return NULL;

    collect_tokens(ctx, command_expression, token_array, &count);

    return token_array;
}

static void rxcp_say_exit(char* message) {
    fprintf(stderr, "%s", message);
    fflush(stderr);
}

static rxvml_context* rxcp_init_bridge(Context* ctx) {
    if (ctx->rxvml_bridge) return (rxvml_context*)ctx->rxvml_bridge;

    /* Use master context if available */
    Context* root = ctx->master_context ? ctx->master_context : ctx;
    if (root->rxvml_bridge) return (rxvml_context*)root->rxvml_bridge;

    if (root->debug_mode >= 2) {
        fprintf(stderr, "DEBUG_EXIT: Initializing compiler exit bridge, location=%s\n", root->location ? root->location : "NULL");
    }

    rxvml_context* vctx = rxvml_create(root->location, root->debug_mode);
    if (!vctx) {
        if (root->debug_mode) fprintf(stderr, "DEBUG_EXIT: Failed to create bridge VM context\n");
        return NULL;
    }

    /* Set say exit to print to stderr */
    rxvml_set_say_exit(rxcp_say_exit);

    /* Load library.rxbin */
    if (root->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Loading library.rxbin into bridge VM\n");
    if (rxvml_load_module_file(vctx, "library") <= 0) {
        char *exe_path = exepath();
        if (exe_path && *exe_path) {
            char path[MAXFILEPATH];
            snprintf(path, sizeof(path), "%s/library", exe_path);
            if (root->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Fallback: Loading library from %s\n", path);
            rxvml_load_module_file(vctx, path);
        }
        if (exe_path) free(exe_path);
    }

    const char* mod = getenv("RXCP_EXIT_MODULE");
    if (!mod) mod = "compiler_exit";

    if (root->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Loading %s into bridge VM\n", mod);
    int rc = rxvml_load_module_file(vctx, mod);
    if (rc <= 0) {
        char *exe_path = exepath();
        if (exe_path && *exe_path) {
            char path[MAXFILEPATH];
            snprintf(path, sizeof(path), "%s/%s", exe_path, mod);
            if (root->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Fallback: Loading %s from %s\n", mod, path);
            rc = rxvml_load_module_file(vctx, path);
        }
        if (exe_path) free(exe_path);
    }

    if (rc <= 0) {
        if (root->debug_mode) fprintf(stderr, "DEBUG_EXIT: Failed to load %s into bridge VM (rc=%d)\n", mod, rc);
    }

    root->rxvml_bridge = vctx;
    return vctx;
}

static ASTNode* find_node(ASTNode* node, NodeType type) {
    if (!node) return NULL;
    if (node->node_type == type) return node;
    ASTNode* found = find_node(node->child, type);
    if (found) return found;
    return find_node(node->sibling, type);
}

static walker_result fragment_fixup_walker(walker_direction direction, ASTNode *node, void *payload) {
    if (direction == in) {
        if (node->node_string && !node->free_node_string) {
            char *s = malloc(node->node_string_length + 1);
            memcpy(s, node->node_string, node->node_string_length);
            s[node->node_string_length] = 0;
            node->node_string = s;
            node->free_node_string = 1;
        }
    }
    return result_normal;
}

int rxcp_exit_bridge_invoke(Context* ctx, ASTNode* node) {
    if (ctx->debug_mode) fprintf(stderr, "DEBUG_EXIT: rxcp_exit_bridge_invoke called for node type %d\n", node->node_type);
    if (ctx->in_exit_bridge) return 0;
    ctx->in_exit_bridge = 1;

    rxvml_context* vctx = rxcp_init_bridge(ctx);
    if (!vctx) {
        ctx->in_exit_bridge = 0;
        return 0;
    }

    rxvml_value* tok_array = rxcp_marshal_implicit_cmd(vctx, node);
    if (!tok_array) {
        ctx->in_exit_bridge = 0;
        return 0;
    }

    rxvml_value* response = NULL;
    if (ctx->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Calling rxcp.exit_dispatch in bridge VM\n");
    int rc = rxvml_call_plugin(vctx, "rxcp.exit_dispatch", tok_array, &response);
    if (rc != 0) {
        if (ctx->debug_mode) fprintf(stderr, "DEBUG_EXIT: Call to rxcp.exit_dispatch failed (rc=%d)\n", rc);
        ctx->in_exit_bridge = 0;
        return 0;
    }

    const char* replacement_code = rxvml_get_replacement_code(vctx, response);
    if (ctx->debug_mode >= 2) {
        fprintf(stderr, "DEBUG_EXIT: Replacement code: %s\n", replacement_code ? replacement_code : "NULL");
    }
    if (!replacement_code) {
        const char* err_msg = rxvml_get_error_message(vctx, response);
        if (err_msg) {
            mknd_err(node, (char*)err_msg);
            ctx->in_exit_bridge = 0;
            return -1;
        }
        ctx->in_exit_bridge = 0;
        return 0;
    }

    /* Parse replacement code */
    Context* frag = cntx_f();
    if (!frag) {
        ctx->in_exit_bridge = 0;
        return -1;
    }
    frag->in_exit_bridge = 1;
    frag->master_context = ctx->master_context;
    frag->location = ctx->location;
    frag->file_name = "exit_fragment";
    frag->level = LEVELB;
    frag->debug_mode = ctx->debug_mode;

    char* code_copy = strdup(replacement_code);
    cntx_buf(frag, code_copy, strlen(code_copy));
    if (rexbpars(frag)) {
        mknd_err(node, "EXIT_BRIDGE_PARSE_FAILED");
        fre_cntx(frag);
        ctx->in_exit_bridge = 0;
        return -1;
    }
    ast_wlkr(frag->ast, fragment_fixup_walker, NULL);

    if (frag->ast) {
        /* Find the instructions list in the parsed fragment */
        ASTNode *search = find_node(frag->ast, INSTRUCTIONS);

        if (search && search->child) {
            /* Replace the IMPLICIT_CMD with the instructions from fragment */
            ASTNode *instr = search->child;
            ASTNode *first_graft = NULL;
            ASTNode *prev_graft = NULL;

            while (instr) {
                ASTNode *next_instr = instr->sibling;
                ASTNode *graft = add_dast(node->parent, instr);
                if (graft) {
                    ast_del(graft); /* Detach from end of parent's child list */
                    if (prev_graft) {
                        /* Link to previous graft */
                        prev_graft->sibling = graft;
                        graft->parent = node->parent;
                    } else {
                        first_graft = graft;
                    }
                    prev_graft = graft;
                }
                instr = next_instr;
            }

            if (first_graft) {
                /* Ensure the end of the new chain is linked to original siblings */
                prev_graft->sibling = node->sibling;
                /* Replace the original node with the head of our new chain */
                ast_rpl(node, first_graft);
                ctx->changed = 1;
            }
        }
        /* Avoid double free when frag is destroyed */
        frag->ast = NULL;
    }

    /* Move tokens from frag to ctx to keep them alive */
    if (frag->token_head) {
        if (ctx->token_tail) {
            ctx->token_tail->token_next = frag->token_head;
            ctx->token_tail = frag->token_tail;
        } else {
            ctx->token_head = frag->token_head;
            ctx->token_tail = frag->token_tail;
        }
        ctx->token_counter += frag->token_counter;
        frag->token_head = frag->token_tail = NULL;
        frag->token_counter = 0;
    }

    fre_cntx(frag);
    ctx->in_exit_bridge = 0;
    return 0;
}
