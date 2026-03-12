#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxvml.h"
#include "rxbin.h"
#include "rxvmvars.h"
#include "platform.h"
#include "rxcp_exit.h"

static const char* rexx_builtins[] = {
    "ADDRESS", "ASSEMBLER", "ARG", "CALL", "CLASS", "DO", "LOOP", "METHOD", "ELSE", "ERROR", "END", "EXIT",
    "FACTORY", "IF", "IMPORT", "INPUT", "ITERATE", "LEAVE", "NAMESPACE", "OF", "NOP", "NUMERIC", "OPTIONS",
    "OUTPUT", "PROCEDURE", "REGISTER", "RETURN", "SAY", "THEN", "BY", "EXPOSE", "FOR", "FOREVER", "TO",
    "UNTIL", "VOID", "WHILE", "WITH", "PULL", "PUSH", "QUEUE", "SELECT", "SIGNAL", "TRACE", "WHEN", "OFF",
    "ON", "DROP", "EXTERNAL", "INTERPRET", "LINEIN", "NAME", "NOVALUE", "SOURCE", "SYNTAX", "UPPER",
    "VAR", "VERSION", NULL
};
/* ----------------------------------------------------------------------
 * strndup() is POSIX and not available in MSVCRT (MinGW/Windows).
 * Provide a small C99 fallback and map strndup to it on _WIN32
 * to preserve portability without changing call sites.
 * ----------------------------------------------------------------------
 */
#if defined(_WIN32) && !defined(HAVE_STRNDUP)
char *strndup_windows(const char *s, size_t n) {
    if (!s) return NULL;
    size_t len = strnlen(s, n);
    char *out = (char *)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len);
    out[len] = '\0';
    return out;
}
#define strndup strndup_windows     // register strndup to windows
#endif

static int is_builtin_keyword(const char* keyword) {
    int i = 0;
    while (rexx_builtins[i]) {
        if (strcasecmp(rexx_builtins[i], keyword) == 0) return 1;
        i++;
    }
    return 0;
}

void rxcp_free_exits(Context *ctx) {
    Context *root = ctx->master_context ? ctx->master_context : ctx;
    if (ctx != root) return;

    ExitEntry *entry = (ExitEntry *)root->exit_registry;
    while (entry) {
        ExitEntry *next = entry->next;
        if (entry->primary_keyword) free(entry->primary_keyword);
        ExitKeyword *kw = entry->additional_keywords;
        while (kw) {
            ExitKeyword *next_kw = kw->next;
            if (kw->keyword) free(kw->keyword);
            free(kw);
            kw = next_kw;
        }
        if (entry->class_name) free(entry->class_name);
        free(entry);
        entry = next;
    }
    root->exit_registry = NULL;

    ExitAdditionalKeywords *akw = (ExitAdditionalKeywords *)root->exit_additional_keywords;
    while (akw) {
        ExitAdditionalKeywords *next_akw = akw->next;
        if (akw->keyword) free(akw->keyword);
        free(akw);
        akw = next_akw;
    }
    root->exit_additional_keywords = NULL;
}

static rxvml_context* rxcp_init_bridge(Context* ctx);

void rxcp_init_exits(Context *ctx) {
    Context *root = ctx->master_context ? ctx->master_context : ctx;

    rxvml_context* vctx = rxcp_init_bridge(ctx);
    if (!vctx) return;

    rxvml_class_info* classes = NULL;
    size_t class_count = 0;
    rxvml_discover_classes(vctx, "rxcpexits", &classes, &class_count);

    if (classes) {
        size_t i;
        for (i = 0; i < class_count; i++) {
            /* Skip internal token class */
            if (strstr(classes[i].class_name, ".token")) continue;

            rxvml_value* obj = NULL;
            /* Use a dummy node number 0 for initialization */
            rxvml_value* nid_val = rxvml_value_new(vctx);
            rxvml_set_int(nid_val, 0);

            if (rxvml_call_factory(vctx, classes[i].class_name, 1, &nid_val, &obj) == 0 && obj) {
                rxvml_value* pk_val = NULL;
                if (rxvml_call_method(vctx, obj, classes[i].class_name, "get_primary_keyword", 0, NULL, &pk_val) == 0 && pk_val) {
                    const char* pk = NULL;
                    size_t pk_len = 0;
                    rxvml_to_str(vctx, pk_val, &pk, &pk_len);

                    if (pk && pk_len > 0) {
                        /* Check for conflicts */
                        if (is_builtin_keyword(pk)) {
                            fprintf(stderr, "INTERNAL EXIT ERROR: Exit '%s' uses a Rexx built-in primary keyword '%s'\n", classes[i].class_name, pk);
                            exit(-1);
                        }
                        if (rxcp_is_exit_primary(ctx, pk, pk_len)) {
                            /* Already registered, maybe from a previous iteration of fixed-point loop */
                            rxvml_value_free(pk_val);
                            rxvml_value_free(obj);
                            rxvml_value_free(nid_val);
                            continue;
                        }

                        /* Register exit */
                        ExitEntry *entry = calloc(1, sizeof(ExitEntry));
                        entry->primary_keyword = strndup(pk, pk_len);
                        entry->class_name = strdup(classes[i].class_name);
                        entry->next = (ExitEntry *)root->exit_registry;
                        root->exit_registry = entry;

                        /* Get additional keywords */
                        rxvml_value* ak_val = NULL;
                        if (rxvml_call_method(vctx, obj, classes[i].class_name, "get_additional_keywords", 0, NULL, &ak_val) == 0 && ak_val) {
                            const char* ak_str = NULL;
                            size_t ak_len = 0;
                            rxvml_to_str(vctx, ak_val, &ak_str, &ak_len);
                            if (ak_str && ak_len > 0) {
                                char *work = strndup(ak_str, ak_len);
                                char *p = strtok(work, " ");
                                while (p) {
                                    ExitKeyword *kw = calloc(1, sizeof(ExitKeyword));
                                    kw->keyword = strdup(p);
                                    kw->next = entry->additional_keywords;
                                    entry->additional_keywords = kw;

                                    /* Also add to global list if not already there */
                                    if (!rxcp_is_exit_additional(ctx, p, strlen(p))) {
                                        ExitAdditionalKeywords *akw = calloc(1, sizeof(ExitAdditionalKeywords));
                                        akw->keyword = strdup(p);
                                        akw->next = (ExitAdditionalKeywords *)root->exit_additional_keywords;
                                        root->exit_additional_keywords = akw;
                                    }

                                    p = strtok(NULL, " ");
                                }
                                free(work);
                            }
                            rxvml_value_free(ak_val);
                        }
                    }
                    rxvml_value_free(pk_val);
                    rxvml_value_free(obj);
                } else {
                     /* No primary keyword, maybe it's not a parser exit */
                     rxvml_value_free(obj);
                }
            }
            rxvml_value_free(nid_val);
        }
        free(classes);
    }
}

int rxcp_is_exit_primary(Context *ctx, const char *keyword, size_t len) {
    Context *root = ctx->master_context ? ctx->master_context : ctx;
    ExitEntry *entry = (ExitEntry *)root->exit_registry;
    while (entry) {
        if (len == strlen(entry->primary_keyword) && strncasecmp(entry->primary_keyword, keyword, len) == 0) return 1;
        entry = entry->next;
    }
    return 0;
}

int rxcp_is_exit_additional(Context *ctx, const char *keyword, size_t len) {
    Context *root = ctx->master_context ? ctx->master_context : ctx;
    ExitAdditionalKeywords *akw = (ExitAdditionalKeywords *)root->exit_additional_keywords;
    while (akw) {
        if (len == strlen(akw->keyword) && strncasecmp(akw->keyword, keyword, len) == 0) return 1;
        akw = akw->next;
    }
    return 0;
}

static const char* token_type_to_string(int type) {
    switch (type) {
        case TK_VAR_SYMBOL: return "identifier";
        case TK_STRING:     return "string_literal";
        case TK_INTEGER:    return "int_literal";
        case TK_FLOAT:      return "float_literal";
        case TK_DECIMAL:    return "decimal_literal";
        case TK_PLUS:
        case TK_MINUS:
        case TK_MULT:
        case TK_DIV:
        case TK_IDIV:
        case TK_MOD:
        case TK_POWER_L:
        case TK_POWER_R:    return "operator";
        case TK_EQUAL:      return "assignment";
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
        case TK_OPTIONS:    return "keyword";
        case TK_EXIT_PRIMARY: return "exit_primary";
        case TK_EXIT_TOKEN:   return "exit_keyword";
        case TK_COMMA:      return "comma";
        case TK_OPEN_BRACKET:
        case TK_CLOSE_BRACKET:
        case TK_OPEN_SBRACKET:
        case TK_CLOSE_SBRACKET: return "bracket";
        case TK_LABEL:      return "label";
        default:            return "other";
    }
}


static void count_tokens(ASTNode *node, size_t *count) {
    if (!node) return;
    if (node->node_type == ERROR || node->node_type == WARNING) return;

    if (node->node_type == OP_CONCAT || node->node_type == OP_SCONCAT || node->node_type == EXIT_EXTENDED) {
        ASTNode *child = node->child;
        while (child) {
            count_tokens(child, count);
            child = child->sibling;
        }
    } else {
        (*count)++;
    }
}

static void marshal_single_token(rxvml_context *ctx, ASTNode *node, rxvml_value *token_array, ASTNode **node_map, size_t *count) {
    /* Layout (rxcp_intern.token factory compatible):
       Arg 1: Type (.int)
       Arg 2: Subtype (.int)
       Arg 3: Text (.string)
       Arg 4: Line (.int)
       Arg 5: Column (.int)
       Arg 6: Length (.int)
       Arg 7: File (.string)
       Arg 8: Node Type (.int)
       Arg 9: Value Type (.int)
    */

    rxvml_value* args[10];
    int i;
    args[0] = rxvml_value_new(ctx);
    int t_type = node->token ? node->token->token_type : 0;
    rxvml_set_int(args[0], t_type);
    args[1] = rxvml_value_new(ctx);
    rxvml_set_int(args[1], 0); /* subtype */
    args[2] = rxvml_value_new(ctx);
    rxvml_set_str(args[2], node->token ? node->token->token_string : node->node_string, node->token ? node->token->length : node->node_string_length);
    args[3] = rxvml_value_new(ctx);
    rxvml_set_int(args[3], node->line);
    args[4] = rxvml_value_new(ctx);
    rxvml_set_int(args[4], node->column);
    args[5] = rxvml_value_new(ctx);
    rxvml_set_int(args[5], node->token ? node->token->length : node->node_string_length);
    args[6] = rxvml_value_new(ctx);
    rxvml_set_str(args[6], "", 0); /* file */
    args[7] = rxvml_value_new(ctx);
    rxvml_set_int(args[7], node->node_type);
    args[8] = rxvml_value_new(ctx);
    char *vt_str = ast_n2tp(node);
    rxvml_set_str(args[8], vt_str, strlen(vt_str));
    free(vt_str);
    args[9] = rxvml_value_new(ctx);
    rxvml_set_str(args[9], token_type_to_string(t_type), strlen(token_type_to_string(t_type)));

    rxvml_value *tok_obj = NULL;
    if (rxvml_call_factory(ctx, "rxcp.token", 10, args, &tok_obj) == 0 && tok_obj) {
        rxvml_array_set(ctx, token_array, *count + 1, tok_obj);
        if (node_map) node_map[*count] = node;
        (*count)++;
        rxvml_value_free(tok_obj);
    }

    for (i = 0; i < 10; i++) {
        rxvml_value_free(args[i]);
    }
}

static void collect_tokens(rxvml_context *ctx, ASTNode *node, rxvml_value *token_array, ASTNode **node_map, size_t *count) {
    if (!node) return;
    if (node->node_type == ERROR || node->node_type == WARNING) return;

    if (node->node_type == OP_CONCAT || node->node_type == OP_SCONCAT || node->node_type == EXIT_EXTENDED) {
        ASTNode *child = node->child;
        while (child) {
            collect_tokens(ctx, child, token_array, node_map, count);
            child = child->sibling;
        }
    } else {
        marshal_single_token(ctx, node, token_array, node_map, count);
    }
}

rxvml_value* rxcp_marshal_implicit_cmd(rxvml_context *ctx, ASTNode *cmd_node, ASTNode ***node_map_out, size_t *num_tokens_out) {
    size_t num_tokens = 0;
    size_t count = 0;
    rxvml_value *token_array;
    ASTNode *command_expression;

    if (!cmd_node) return NULL;

    if (cmd_node->node_type == EXIT_EXTENDED) {
        num_tokens = 1;
        ASTNode *c = cmd_node->child;
        while (c) {
            count_tokens(c, &num_tokens);
            c = c->sibling;
        }

        token_array = rxvml_array_new(ctx, num_tokens);
        if (!token_array) return NULL;

        rxvml_value* count_val = rxvml_value_new(ctx);
        rxvml_set_int(count_val, num_tokens);
        rxvml_array_set(ctx, token_array, 0, count_val);
        rxvml_value_free(count_val);

        ASTNode **node_map = malloc(sizeof(ASTNode*) * num_tokens);
        marshal_single_token(ctx, cmd_node, token_array, node_map, &count);
        c = cmd_node->child;
        while (c) {
            collect_tokens(ctx, c, token_array, node_map, &count);
            c = c->sibling;
        }

        if (node_map_out) *node_map_out = node_map;
        if (num_tokens_out) *num_tokens_out = num_tokens;
        return token_array;
    }

    command_expression = cmd_node->child;

    if (cmd_node->node_type == ADDRESS) {
        // First child is environment, second is command
        if (command_expression) command_expression = command_expression->sibling;
    }

    if (!command_expression) return NULL;

    /* Count tokens in the expression */
    count_tokens(command_expression, &num_tokens);

    token_array = rxvml_array_new(ctx, num_tokens);
    if (!token_array) return NULL;

    rxvml_value* count_val = rxvml_value_new(ctx);
    rxvml_set_int(count_val, num_tokens);
    rxvml_array_set(ctx, token_array, 0, count_val);
    rxvml_value_free(count_val);

    ASTNode **node_map = malloc(sizeof(ASTNode*) * num_tokens);
    collect_tokens(ctx, command_expression, token_array, node_map, &count);

    if (node_map_out) *node_map_out = node_map;
    if (num_tokens_out) *num_tokens_out = num_tokens;

    return token_array;
}

static void rxcp_say_exit(char* message) {
    fprintf(stdout, "%s", message);
    fflush(stdout);
}

static rxvml_context* rxcp_init_bridge(Context* ctx) {
    /* Use master context if available */
    Context* root = ctx->master_context ? ctx->master_context : ctx;

    if (root->disable_exits) return NULL;
    if (ctx->rxvml_bridge) return (rxvml_context*)ctx->rxvml_bridge;

    if (root->rxvml_bridge) return (rxvml_context*)root->rxvml_bridge;

    char *combined_loc = NULL;
    size_t len = 0;
    int i;
    if (root->location) len += strlen(root->location) + 1;
    if (root->import_locations) {
        for (i = 0; root->import_locations[i]; i++) {
            len += strlen(root->import_locations[i]) + 1;
        }
    }

    if (len > 0) {
        combined_loc = malloc(len + 1);
        combined_loc[0] = 0;
        if (root->location) {
            strcat(combined_loc, root->location);
            if (root->import_locations && root->import_locations[0]) strcat(combined_loc, ";");
        }
        if (root->import_locations) {
            for (i = 0; root->import_locations[i]; i++) {
                strcat(combined_loc, root->import_locations[i]);
                if (root->import_locations[i+1]) strcat(combined_loc, ";");
            }
        }
    }

    rxvml_context* vctx = rxvml_create(combined_loc, 0);
    if (combined_loc) free(combined_loc);

    if (!vctx) return NULL;

    /* Set say exit to print to stderr */
    rxvml_set_say_exit(rxcp_say_exit);

    if (rxvml_load_module_file(vctx, "library") <= 0) {
        rxvml_destroy(vctx);
        return NULL;
    }

    const char* mod = getenv("RXCP_EXIT_MODULE");
    if (!mod) mod = "rxcexits";

    int rc = rxvml_load_module_file(vctx, mod);

    if (rc <= 0) {
        rxvml_destroy(vctx);
        return NULL;
    }

    root->rxvml_bridge = vctx;
    return vctx;
}


static int rxcp_exit_handle_response(Context* ctx, ASTNode* node, rxvml_context* vctx, rxvml_value* obj, const char* class_name, rxvml_value* response, ASTNode **node_map, size_t num_tokens) {
    const char* status = NULL;
    size_t status_len = 0;
    rxvml_value* status_val = NULL;

    if (obj && (rxvml_to_str(vctx, response, &status, &status_len) != 0 || status_len == 0)) {
        /* Fallback to get_status() method if return value is empty */
        if (rxvml_call_method(vctx, obj, class_name, "get_status", 0, NULL, &status_val) == 0 && status_val) {
            rxvml_to_str(vctx, status_val, &status, &status_len);
        }
    }

    if (!status) {
        if (status_val) rxvml_value_free(status_val);
        return 0;
    }

    if (strncmp(status, "REJECT", status_len) == 0) {
        if (status_val) rxvml_value_free(status_val);
        return 0;
    }

    if (strncmp(status, "ACCEPT", status_len) == 0) {
        if (status_val) rxvml_value_free(status_val);
        return 1;
    }

    if (strncmp(status, "PENDING", status_len) == 0) {
        ctx->changed_flags |= FLAG_EXIT; /* Ensure another pass */
        if (status_val) rxvml_value_free(status_val);
        return 1;
    }

    if (status_len >= 5 && strncmp(status, "ERROR", 5) == 0) {
        rxinteger error_token_idx = 0;
        char* err_msg_copy = NULL;
        const char* err_msg = NULL;
        size_t err_msg_len = 0;
        rxvml_value *val = NULL;

        /* get_error_token */
        if (rxvml_call_method(vctx, obj, class_name, "get_error_token", 0, NULL, &val) == 0 && val) {
            int rc = rxvml_to_int(vctx, val, &error_token_idx);
            if (rc != 0) {
                const char* err_tok_str = NULL;
                size_t len = 0;
                if (rxvml_to_str(vctx, val, &err_tok_str, &len) == 0 && err_tok_str) {
                    error_token_idx = atol(err_tok_str);
                    rc = 0;
                }
            }
            rxvml_value_free(val);
            val = NULL;
        }

        /* get_error_message */
        if (rxvml_call_method(vctx, obj, class_name, "get_error_message", 0, NULL, &val) == 0 && val) {
            if (rxvml_to_str(vctx, val, &err_msg, &err_msg_len) == 0 && err_msg) {
                err_msg_copy = malloc(err_msg_len + 1);
                memcpy(err_msg_copy, err_msg, err_msg_len);
                err_msg_copy[err_msg_len] = 0;
            }
            rxvml_value_free(val);
            val = NULL;
        }

        ASTNode *err_node = node;
        if (error_token_idx > 0 && (size_t)error_token_idx <= num_tokens && node_map) {
            err_node = node_map[error_token_idx - 1];
        }
        fprintf(stderr, "DEBUG_FINAL: error_token_idx=%lld err_node_type=%d src='%.*s'\n",
            (long long)error_token_idx, err_node->node_type,
            (int)(err_node->source_end - err_node->source_start + 1), err_node->source_start);

        if (err_msg_copy) {
            mknd_err(err_node, "%s", err_msg_copy);
            free(err_msg_copy);
        } else {
            mknd_err(err_node, "EXIT_BRIDGE_ERROR");
        }
        if (status_val) rxvml_value_free(status_val);
        return -1;
    }

    char* replacement_copy = NULL;
    rxvml_value *val = NULL;

    if (status_len >= 7 && strncmp(status, "REPLACE", 7) == 0) {
        const char* replacement_code = NULL;
        size_t replacement_len = 0;

        /* get_replacement */
        if (rxvml_call_method(vctx, obj, class_name, "get_replacement", 0, NULL, &val) == 0 && val) {
            rxvml_to_str(vctx, val, &replacement_code, &replacement_len);
        }

        if (replacement_code) {
            int rc = ast_grft_interpolated(ctx, node, replacement_code, node_map, num_tokens);
            /* Mark context changed to force recompilation pass (locals/regs recalculation) */
            ctx->changed_flags |= FLAG_EXIT;
            if (val) rxvml_value_free(val);
            if (status_val) rxvml_value_free(status_val);
            return rc < 0 ? -1 : -1; /* Node was replaced */
        }

        if (val) rxvml_value_free(val);
        if (status_val) rxvml_value_free(status_val);
        mknd_err(node, "EXIT_BRIDGE_REPLACE_MISSING_TEXT");
        return -1;
    }

    if (status_val) rxvml_value_free(status_val);
    return 0;
}

int rxcp_exit_bridge_invoke(Context *ctx, ASTNode *node) {
    rxvml_context* vctx;
    rxvml_value* tok_array;
    rxvml_value* response = NULL;
    ASTNode **node_map = NULL;
    size_t num_tokens = 0;
    int handled = 0;
    Context *root = ctx->master_context ? ctx->master_context : ctx;

    /* Optimization: If the node already has an error, skip redundant invocation */
    if (ast_hase(node)) return 0;

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
            if (rxvml_call_method(vctx, obj, class_name, "process", 1, &tok_array, &response) == 0) {
            handled = rxcp_exit_handle_response(ctx, node, vctx, obj, class_name, response, node_map, num_tokens);
            if (response) rxvml_value_free(response);
        }
        }
    }

    /* 2. Direct Dispatch for EXIT_EXTENDED */
    if (!handled && node->node_type == EXIT_EXTENDED) {
        ExitEntry *entry = (ExitEntry *)root->exit_registry;
        while (entry) {
            if (node->token->length == strlen(entry->primary_keyword) &&
                strncasecmp(entry->primary_keyword, node->token->token_string, node->token->length) == 0) {

                rxvml_value* nid_val = rxvml_value_new(vctx);
                rxvml_set_int(nid_val, node->node_number);
                rxvml_value* obj = NULL;

                if (rxvml_call_factory(vctx, entry->class_name, 1, &nid_val, &obj) == 0 && obj) {
                    if (rxvml_call_method(vctx, obj, entry->class_name, "process", 1, &tok_array, &response) == 0) {
                        handled = rxcp_exit_handle_response(ctx, node, vctx, obj, entry->class_name, response, node_map, num_tokens);
                        if (handled != 0) {
                            if (handled > 0) {
                                 node->exit_obj_reg = rxvml_reg_alloc(vctx, obj, entry->class_name);
                            } else {
                                 rxvml_value_free(obj);
                            }
                            if (response) rxvml_value_free(response);
                            rxvml_value_free(nid_val);
                            break;
                        }
                    }
                    rxvml_value_free(obj);
                }
                rxvml_value_free(nid_val);
                if (response) rxvml_value_free(response);
                break;
            }
            entry = entry->next;
        }
    }

    /* 3. Registry Lookup for IMPLICIT_CMD (if not handled) */
    if (!handled && node->node_type == IMPLICIT_CMD && num_tokens > 0) {
        ASTNode *first_node = node_map[0];
        const char *first_word = first_node->token ? first_node->token->token_string : first_node->node_string;
        size_t first_len = first_node->token ? first_node->token->length : first_node->node_string_length;

        if (first_word) {
            ExitEntry *entry = (ExitEntry *)root->exit_registry;
            while (entry) {
                if (first_len == strlen(entry->primary_keyword) &&
                    strncasecmp(entry->primary_keyword, first_word, first_len) == 0) {

                    rxvml_value* nid_val = rxvml_value_new(vctx);
                    rxvml_set_int(nid_val, node->node_number);
                    rxvml_value* obj = NULL;

                    if (rxvml_call_factory(vctx, entry->class_name, 1, &nid_val, &obj) == 0 && obj) {
                        if (rxvml_call_method(vctx, obj, entry->class_name, "process", 1, &tok_array, &response) == 0) {
                            handled = rxcp_exit_handle_response(ctx, node, vctx, obj, entry->class_name, response, node_map, num_tokens);
                            if (handled != 0) {
                                if (handled > 0) {
                                     node->exit_obj_reg = rxvml_reg_alloc(vctx, obj, entry->class_name);
                                } else {
                                     rxvml_value_free(obj);
                                }
                                if (response) rxvml_value_free(response);
                                rxvml_value_free(nid_val);
                                break;
                            }
                        }
                        rxvml_value_free(obj);
                    }
                    rxvml_value_free(nid_val);
                    if (response) rxvml_value_free(response);
                }
                entry = entry->next;
            }
        }
    }

    if (!handled && node->node_type == EXIT_EXTENDED) {
        mknd_err(node, "EXIT_BRIDGE_DISPATCH_FAILED");
        handled = -1;
    }

    rxvml_value_free(tok_array);
    if (node_map) free(node_map);
    ctx->in_exit_bridge = 0;
    return handled;
}
