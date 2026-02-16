#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxvml.h"
#include "rxbin.h"
#include "rxvmvars.h"
#include "platform.h"

static const char* token_type_to_string(int type) {
    switch (type) {
        case TK_VAR_SYMBOL: return "IDENTIFIER";
        case TK_STRING:     return "STRING_LITERAL";
        case TK_INTEGER:    return "INT_LITERAL";
        case TK_FLOAT:      return "FLOAT_LITERAL";
        case TK_DECIMAL:    return "DECIMAL_LITERAL";
        case TK_PLUS:
        case TK_MINUS:
        case TK_MULT:
        case TK_DIV:
        case TK_IDIV:
        case TK_MOD:
        case TK_POWER_L:
        case TK_POWER_R:    return "OPERATOR";
        case TK_EQUAL:      return "ASSIGNMENT";
        case TK_IF:
        case TK_THEN:
        case TK_ELSE:
        case TK_DO:
        case TK_END:
        case TK_SAY:
        case TK_RETURN:
        case TK_EXIT:
        case TK_CALL:
        case TK_PROCEDURE:
        case TK_NAMESPACE:
        case TK_IMPORT:
        case TK_EXPOSE:
        case TK_OPTIONS:    return "KEYWORD";
        case TK_COMMA:      return "COMMA";
        case TK_OPEN_BRACKET:
        case TK_CLOSE_BRACKET:
        case TK_OPEN_SBRACKET:
        case TK_CLOSE_SBRACKET: return "BRACKET";
        case TK_LABEL:      return "LABEL";
        default:            return "OTHER";
    }
}

static const char* node_value_type_to_string(ValueType type) {
    switch (type) {
        case TP_INTEGER: return "INT";
        case TP_STRING:  return "STRING";
        case TP_FLOAT:   return "FLOAT";
        case TP_DECIMAL: return "DECIMAL";
        case TP_BOOLEAN: return "BOOLEAN";
        default:         return "VOID";
    }
}

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

static void collect_tokens(rxvml_context *ctx, ASTNode *node, rxvml_value *token_array, ASTNode **node_map, size_t *count) {
    if (!node) return;

    if (node->node_type == OP_CONCAT || node->node_type == OP_SCONCAT) {
        ASTNode *child = node->child;
        while (child) {
            collect_tokens(ctx, child, token_array, node_map, count);
            child = child->sibling;
        }
    } else {
        /* Design Mapping:
           Register 1: ID (.int) -> attributes[0]
           Register 2: Type (.string) -> attributes[1]
           Register 3: Value Type (.string) -> attributes[2]
           Register 4: Value/Text (.string) -> attributes[3]
           Register 5: Line (.int) -> attributes[4]
           Register 6: Column (.int) -> attributes[5]
        */

        rxvml_value *tok_obj = NULL;
        if (rxvml_call_plugin(ctx, "rxcp.token.§factory", NULL, &tok_obj) != 0 || !tok_obj) {
            tok_obj = rxvml_array_new(ctx, 15);
        }
        if (!tok_obj) return;

        /* Ensure enough attributes */
        if (((value*)tok_obj)->num_attributes < 15) {
            set_num_attributes((value*)tok_obj, 15);
        }

        /* Populate registers */
        int id = node->token ? node->token->token_number : 0;
        set_int(tok_obj->attributes[0], id);

        const char* type_str = token_type_to_string(node->token ? node->token->token_type : 0);
        set_string(tok_obj->attributes[1], (char*)type_str, strlen(type_str));

        const char* val_type_str = node_value_type_to_string(node->value_type);
        set_string(tok_obj->attributes[2], (char*)val_type_str, strlen(val_type_str));

        if (node->token) {
            set_string(tok_obj->attributes[3], node->token->token_string, node->token->length);
        } else {
            set_string(tok_obj->attributes[3], node->node_string, node->node_string_length);
        }

        set_int(tok_obj->attributes[4], node->line);
        set_int(tok_obj->attributes[5], node->column);

        if (rxvml_get_debug_mode(ctx) >= 2) {
            fprintf(stderr, "DEBUG_EXIT: Marshaled token %zu: type=%s, text=\"%.*s\"\n",
                    *count + 1, type_str, (int)(node->token ? node->token->length : node->node_string_length),
                    node->token ? node->token->token_string : node->node_string);
        }
        rxvml_array_set(ctx, token_array, *count + 1, tok_obj);
        if (node_map) node_map[*count] = node;
        (*count)++;
        rxvml_value_free(tok_obj);
    }
}

rxvml_value* rxcp_marshal_implicit_cmd(rxvml_context *ctx, ASTNode *cmd_node, ASTNode ***node_map_out, size_t *num_tokens_out) {
    size_t num_tokens = 0;
    size_t count = 0;
    rxvml_value *token_array;
    ASTNode *command_expression;

    if (!cmd_node) return NULL;

    command_expression = cmd_node->child;

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

    ASTNode **node_map = malloc(sizeof(ASTNode*) * num_tokens);
    collect_tokens(ctx, command_expression, token_array, node_map, &count);

    if (node_map_out) *node_map_out = node_map;
    if (num_tokens_out) *num_tokens_out = num_tokens;

    return token_array;
}

static void rxcp_say_exit(char* message) {
    fprintf(stderr, "%s", message);
    fflush(stderr);
}

static rxvml_context* rxcp_init_bridge(Context* ctx) {
    if (getenv("RXCP_DISABLE_EXIT")) return NULL;
    if (ctx->rxvml_bridge) return (rxvml_context*)ctx->rxvml_bridge;

    /* Use master context if available */
    Context* root = ctx->master_context ? ctx->master_context : ctx;
    if (root->rxvml_bridge) return (rxvml_context*)root->rxvml_bridge;

    if (root->debug_mode >= 2) {
        fprintf(stderr, "DEBUG_EXIT: Initializing compiler exit bridge, location=%s\n", root->location ? root->location : "NULL");
    }

    rxvml_context* vctx = rxvml_create(root->location, root->debug_mode);
    if (!vctx) {
        fprintf(stderr, "DEBUG_EXIT: Failed to create bridge VM context\n");
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
        fprintf(stderr, "DEBUG_EXIT: Failed to load %s into bridge VM (rc=%d)\n", mod, rc);
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

static walker_result graft_fixup_walker(walker_direction direction, ASTNode *node, void *payload) {
    if (direction == in) {
        node->scope = (Scope*)payload;
    }
    return result_normal;
}

static int rxcp_exit_handle_response(Context* ctx, ASTNode* node, rxvml_context* vctx, rxvml_value* obj, const char* class_name, rxvml_value* response, ASTNode **node_map, size_t num_tokens) {
    const char* status = NULL;
    size_t status_len = 0;
    if (rxvml_to_str(vctx, response, &status, &status_len) != 0 || status_len == 0) {
        /* Fallback to status property if return value is empty (attribute 4 / register 5) */
        if (((value*)obj)->num_attributes > 4) {
            rxvml_to_str(vctx, ((value*)obj)->attributes[4], &status, &status_len);
        }
    }

    if (!status) return 0;

    if (strncmp(status, "REJECT", status_len) == 0) {
        return 0;
    }

    if (strncmp(status, "ACCEPT", status_len) == 0) {
        return 1;
    }

    if (strncmp(status, "PENDING", status_len) == 0) {
        ctx->changed = 1; /* Ensure another pass */
        return 1;
    }

    if (strncmp(status, "ERROR", status_len) == 0) {
        rxinteger error_token_idx = 0;
        char* err_msg_copy = NULL;
        const char* err_msg = NULL;
        size_t err_msg_len = 0;

        /* get_error_token - attribute 2 (register 3) */
        if (((value*)obj)->num_attributes > 2) {
            rxvml_to_int(vctx, ((value*)obj)->attributes[2], &error_token_idx);
        }

        /* get_error_message - attribute 3 (register 4) */
        if (((value*)obj)->num_attributes > 3) {
            if (rxvml_to_str(vctx, ((value*)obj)->attributes[3], &err_msg, &err_msg_len) == 0 && err_msg) {
                err_msg_copy = malloc(err_msg_len + 1);
                memcpy(err_msg_copy, err_msg, err_msg_len);
                err_msg_copy[err_msg_len] = 0;
            }
        }

        ASTNode *err_node = node;
        if (error_token_idx > 0 && (size_t)error_token_idx <= num_tokens && node_map) {
            err_node = node_map[error_token_idx - 1];
        }

        if (err_msg_copy) {
            mknd_err(err_node, err_msg_copy);
            free(err_msg_copy);
        } else {
            mknd_err(err_node, "EXIT_BRIDGE_ERROR");
        }
        return -1;
    }

    if (strncmp(status, "REPLACE", status_len) == 0) {
        const char* replacement_code = NULL;
        size_t replacement_len = 0;
        char* replacement_copy = NULL;

        /* get_replacement - attribute 1 (register 2) */
        if (((value*)obj)->num_attributes > 1) {
            rxvml_to_str(vctx, ((value*)obj)->attributes[1], &replacement_code, &replacement_len);
        }

        if (replacement_code) {
            replacement_copy = malloc(replacement_len + 1);
            memcpy(replacement_copy, replacement_code, replacement_len);
            replacement_copy[replacement_len] = 0;
        }

        if (!replacement_copy) {
            mknd_err(node, "EXIT_BRIDGE_REPLACE_MISSING_TEXT");
            return -1;
        }

        if (ctx->debug_mode >= 2) {
            fprintf(stderr, "DEBUG_EXIT: Replacement text: %s\n", replacement_copy);
        }

        /* Parse replacement code */
        Context* frag = cntx_f();
        if (!frag) {
            free(replacement_copy);
            return -1;
        }
        frag->in_exit_bridge = 1;
        frag->master_context = ctx->master_context;
        frag->location = ctx->location;
        frag->file_name = "exit_fragment";
        frag->level = LEVELB;
        frag->debug_mode = ctx->debug_mode;

        cntx_buf(frag, replacement_copy, strlen(replacement_copy));
        if (rexbpars(frag)) {
            mknd_err(node, "EXIT_BRIDGE_PARSE_FAILED");
            fre_cntx(frag);
            return -1;
        }

        if (ctx->debug_mode >= 2) {
            fprintf(stderr, "DEBUG_EXIT: Fragment AST:\n");
            rxcp_print_ast_recursive(frag->ast, 0);
        }

        ast_wlkr(frag->ast, fragment_fixup_walker, NULL);

        if (frag->ast) {
            /* Find the instructions list in the parsed fragment */
            ASTNode *search = find_node(frag->ast, INSTRUCTIONS);

            if (search && search->child) {
                /* Replace the node with the instructions from fragment */
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
                    /* Link the end of the new chain to the original siblings */
                    prev_graft->sibling = node->sibling;

                    /* Link the parent to the head of the new chain */
                    if (node->parent->child == node) {
                        node->parent->child = first_graft;
                    } else {
                        ASTNode *p = node->parent->child;
                        while (p && p->sibling != node) p = p->sibling;
                        if (p) p->sibling = first_graft;
                    }
                    /* Fixup scope of grafted nodes */
                    ast_wlkr(first_graft, graft_fixup_walker, node->scope);

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

        /* Transfer buffer ownership to ctx */
        if (frag->buff_start) {
            ctx->extra_buffers_count++;
            ctx->extra_buffers = realloc(ctx->extra_buffers, sizeof(char*) * ctx->extra_buffers_count);
            ctx->extra_buffers[ctx->extra_buffers_count - 1] = frag->buff_start;
            frag->buff_start = NULL;
        }

        fre_cntx(frag);
        return -1; /* Node was replaced */
    }

    return 0;
}

int rxcp_exit_bridge_invoke(Context* ctx, ASTNode* node) {
    rxvml_context* vctx;
    rxvml_value* tok_array;
    rxvml_value* response = NULL;
    ASTNode **node_map = NULL;
    size_t num_tokens = 0;
    int handled = 0;

    if (ctx->in_exit_bridge) return 0;
    ctx->in_exit_bridge = 1;

    vctx = rxcp_init_bridge(ctx);
    if (!vctx) {
        ctx->in_exit_bridge = 0;
        return 0;
    }

    tok_array = rxcp_marshal_implicit_cmd(vctx, node, &node_map, &num_tokens);
    if (!tok_array) {
        ctx->in_exit_bridge = 0;
        return 0;
    }

    /* 1. Check Attachment */
    if (node->exit_obj_reg != -1) {
        char class_name[256];
        rxvml_value* obj = rxvml_reg_get(vctx, node->exit_obj_reg, class_name);
        if (obj) {
            if (ctx->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Using attached exit object (class=%s)\n", class_name);
            if (rxvml_call_method(vctx, obj, class_name, "process", 1, &tok_array, &response) == 0) {
                handled = rxcp_exit_handle_response(ctx, node, vctx, obj, class_name, response, node_map, num_tokens);
                fprintf(stderr, "DEBUG_EXIT: Attached process handled=%d\n", handled);
                if (response) rxvml_value_free(response);
            } else {
                fprintf(stderr, "DEBUG_EXIT: Attached process call failed\n");
            }
        }
    }

    /* 2. Discovery Loop (if not handled) */
    if (!handled) {
        rxvml_class_info* classes = NULL;
        size_t class_count = 0;
        rxvml_discover_classes(vctx, "rxcp", &classes, &class_count);

        if (classes) {
            size_t i;
            for (i = 0; i < class_count; i++) {
                rxvml_value* obj = NULL;
                rxvml_value* nid_val = value_f();
                set_int(nid_val, node->node_number);

                if (rxvml_call_plugin(vctx, classes[i].factory_proc, nid_val, &obj) == 0 && obj) {
                    if (rxvml_call_method(vctx, obj, classes[i].class_name, "process", 1, &tok_array, &response) == 0) {
                        handled = rxcp_exit_handle_response(ctx, node, vctx, obj, classes[i].class_name, response, node_map, num_tokens);
                        if (handled != 0) {
                            if (handled > 0) {
                                /* Accept/Pending: Attach object */
                                node->exit_obj_reg = rxvml_reg_alloc(vctx, obj, classes[i].class_name);
                            } else {
                                /* Error/Replace: Destroy object as it's no longer needed for this node */
                                rxvml_value_free(obj);
                            }
                            if (response) rxvml_value_free(response);
                            break;
                        } else {
                            /* Reject: Destroy object */
                            rxvml_value_free(obj);
                            if (response) rxvml_value_free(response);
                            response = NULL;
                        }
                    } else {
                        rxvml_value_free(obj);
                    }
                }
            }
            free(classes);
        }
    }

    rxvml_value_free(tok_array);
    if (node_map) free(node_map);
    ctx->in_exit_bridge = 0;
    return handled;
}
