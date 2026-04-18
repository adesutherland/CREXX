#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxvml.h"
#include "rxbin.h"
#include "rxvmvars.h"
#include "platform.h"
#include "rxcp_exit.h"
#include "rxcp_val.h"

static const char* rexx_builtins[] = {
    "ADDRESS", "ASSEMBLER", "ARG", "CALL", "CLASS", "DO", "LOOP", "METHOD", "ELSE", "ERROR", "END", "EXIT",
    "FACTORY", "IF", "IMPORT", "INPUT", "ITERATE", "LEAVE", "NAMESPACE", "OF", "NOP", "NUMERIC", "OPTIONS",
    "OUTPUT", "PARSE", "PROCEDURE", "REGISTER", "RETURN", "SAY", "THEN", "BY", "EXPOSE", "FOR", "FOREVER", "TO",
    "UNTIL", "VOID", "WHILE", "WITH", "PULL", "PUSH", "QUEUE", "SELECT", "SIGNAL", "TRACE", "WHEN", "OFF",
    "ON", "DROP", "EXTERNAL", "INTERPRET", "LINEIN", "NAME", "NOVALUE", "SOURCE", "SYNTAX", "UPPER",
    "VAR", "VERSION", NULL
};

typedef struct CertifiedExitSpec {
    const char *primary_keyword;
    const char *module_name;
    const char *class_name;
    unsigned int flags;
    const char *required_imports;
} CertifiedExitSpec;

static const CertifiedExitSpec certified_exit_specs[] = {
    { "ADDRESS", "rxcexits", "rxcpexits.addressexit",
      RXCP_EXIT_FLAG_CERTIFIED | RXCP_EXIT_FLAG_RESERVED_KEYWORD | RXCP_EXIT_FLAG_IMPLICIT_COMMAND,
      NULL },
    { "PARSE", "rxcexits", "rxcpexits.parseexit",
      RXCP_EXIT_FLAG_CERTIFIED | RXCP_EXIT_FLAG_RESERVED_KEYWORD,
      "rxfnsb" },
    { NULL, NULL, NULL, 0, NULL }
};

static int is_builtin_keyword(const char* keyword) {
    int i = 0;
    while (rexx_builtins[i]) {
        if (strcasecmp(rexx_builtins[i], keyword) == 0) return 1;
        i++;
    }
    return 0;
}

static const CertifiedExitSpec *rxcp_find_certified_exit_by_keyword(const char *keyword) {
    int i;

    for (i = 0; certified_exit_specs[i].primary_keyword; i++) {
        if (strcasecmp(certified_exit_specs[i].primary_keyword, keyword) == 0) {
            return &certified_exit_specs[i];
        }
    }
    return NULL;
}

static const CertifiedExitSpec *rxcp_find_certified_exit_by_class(const char *class_name) {
    int i;

    for (i = 0; certified_exit_specs[i].class_name; i++) {
        if (strcmp(certified_exit_specs[i].class_name, class_name) == 0) {
            return &certified_exit_specs[i];
        }
    }
    return NULL;
}

const char *rxcp_match_certified_exit_primary(const char *keyword, size_t len) {
    int i;

    if (!keyword || len == 0) return NULL;

    for (i = 0; certified_exit_specs[i].primary_keyword; i++) {
        const char *primary = certified_exit_specs[i].primary_keyword;
        if (len == strlen(primary) && strncasecmp(primary, keyword, len) == 0) {
            return primary;
        }
    }
    return NULL;
}

static ExitEntry *rxcp_find_exit_entry(Context *ctx, const char *keyword, size_t len) {
    Context *root = ctx->master_context ? ctx->master_context : ctx;
    ExitEntry *entry = (ExitEntry *)root->exit_registry;

    while (entry) {
        if (len == strlen(entry->primary_keyword) &&
            strncasecmp(entry->primary_keyword, keyword, len) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

static ExitEntry *rxcp_register_exit_entry(Context *ctx,
                                           const char *primary_keyword,
                                           size_t primary_len,
                                           const char *class_name,
                                           const char *required_imports,
                                           unsigned int flags) {
    Context *root = ctx->master_context ? ctx->master_context : ctx;
    ExitEntry *entry = calloc(1, sizeof(ExitEntry));

    if (!entry) return NULL;

    entry->primary_keyword = rx_strndup(primary_keyword, primary_len);
    entry->class_name = strdup(class_name);
    if (required_imports && required_imports[0]) {
        entry->required_imports = strdup(required_imports);
    }
    entry->flags = flags;
    entry->next = (ExitEntry *)root->exit_registry;
    root->exit_registry = entry;

    return entry;
}

static void rxcp_register_additional_keywords(Context *ctx,
                                              ExitEntry *entry,
                                              const char *keywords,
                                              size_t keywords_len) {
    Context *root = ctx->master_context ? ctx->master_context : ctx;
    char *work;
    char *p;

    if (!entry || !keywords || keywords_len == 0) return;

    work = rx_strndup(keywords, keywords_len);
    p = strtok(work, " ");
    while (p) {
        ExitKeyword *kw = calloc(1, sizeof(ExitKeyword));

        kw->keyword = strdup(p);
        kw->next = entry->additional_keywords;
        entry->additional_keywords = kw;

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

static void rxcp_assert_certified_exits_registered(Context *ctx) {
    int i;

    for (i = 0; certified_exit_specs[i].primary_keyword; i++) {
        const CertifiedExitSpec *spec = &certified_exit_specs[i];
        if (!rxcp_is_exit_primary(ctx, spec->primary_keyword, strlen(spec->primary_keyword))) {
            fprintf(stderr,
                    "INTERNAL EXIT ERROR: Certified exit '%s' from module '%s' was not registered\n",
                    spec->primary_keyword,
                    spec->module_name);
            exit(-1);
        }
    }
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
        if (entry->required_imports) free(entry->required_imports);
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
            const CertifiedExitSpec *certified = NULL;
            unsigned int flags = 0;
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
                        certified = rxcp_find_certified_exit_by_keyword(pk);
                        if (!certified) {
                            certified = rxcp_find_certified_exit_by_class(classes[i].class_name);
                        }
                        if (certified) {
                            flags = certified->flags;
                        }

                        /* Check for conflicts */
                        if (is_builtin_keyword(pk)) {
                            if (!certified || strcmp(classes[i].class_name, certified->class_name) != 0) {
                                fprintf(stderr,
                                        "INTERNAL EXIT ERROR: Exit '%s' uses a Rexx built-in primary keyword '%s'\n",
                                        classes[i].class_name,
                                        pk);
                                exit(-1);
                            }
                        }
                        if (rxcp_is_exit_primary(ctx, pk, pk_len)) {
                            /* Already registered, maybe from a previous iteration of fixed-point loop */
                            rxvml_value_free(pk_val);
                            rxvml_value_free(obj);
                            rxvml_value_free(nid_val);
                            continue;
                        }

                        /* Register exit */
                        ExitEntry *entry = rxcp_register_exit_entry(ctx,
                                                                   pk,
                                                                   pk_len,
                                                                   classes[i].class_name,
                                                                   certified ? certified->required_imports : NULL,
                                                                   flags);

                        /* Get additional keywords */
                        rxvml_value* ak_val = NULL;
                        if (rxvml_call_method(vctx, obj, classes[i].class_name, "get_additional_keywords", 0, NULL, &ak_val) == 0 && ak_val) {
                            const char* ak_str = NULL;
                            size_t ak_len = 0;
                            rxvml_to_str(vctx, ak_val, &ak_str, &ak_len);
                            if (ak_str && ak_len > 0) {
                                rxcp_register_additional_keywords(ctx, entry, ak_str, ak_len);
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

    rxcp_assert_certified_exits_registered(ctx);
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

unsigned int rxcp_get_exit_flags(Context *ctx, const char *keyword, size_t len) {
    ExitEntry *entry = rxcp_find_exit_entry(ctx, keyword, len);
    return entry ? entry->flags : 0;
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

static int rxcp_token_value_dims(ASTNode *node) {
    if (!node) return 0;
    if (node->value_dims > 0) return (int)node->value_dims;
    if (node->symbolNode && node->symbolNode->symbol) {
        return node->symbolNode->symbol->value_dims;
    }
    return 0;
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

static int rxcp_node_has_error(ASTNode *node) {
    ASTNode *child;

    if (!node) return 0;
    if (node->node_type == ERROR) return 1;

    child = node->child;
    while (child) {
        if (rxcp_node_has_error(child)) return 1;
        child = child->sibling;
    }
    return 0;
}

static void rxcp_preserve_replaced_node_diagnostics(Context *ctx, ASTNode *node) {
    ASTNode *child;
    ASTNode *prev;

    if (!ctx || !node) return;

    prev = NULL;
    child = node->child;
    while (child) {
        if (child->node_type == ERROR || child->node_type == WARNING) {
            ASTNode *diagnostic;

            diagnostic = child;
            if (prev) prev->sibling = diagnostic->sibling;
            else node->child = diagnostic->sibling;
            child = diagnostic->sibling;

            diagnostic->sibling = (ASTNode *)ctx->diagnostics_list;
            ctx->diagnostics_list = diagnostic;
            continue;
        }

        rxcp_preserve_replaced_node_diagnostics(ctx, child);
        prev = child;
        child = child->sibling;
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
       Arg 10: Type String (.string)
       Arg 11: Value Dims (.int)
    */

    rxvml_value* args[11];
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
    args[10] = rxvml_value_new(ctx);
    rxvml_set_int(args[10], rxcp_token_value_dims(node));

    rxvml_value *tok_obj = NULL;
    if (rxvml_call_factory(ctx, "rxcp.token", 11, args, &tok_obj) == 0 && tok_obj) {
        rxvml_array_set(ctx, token_array, *count + 1, tok_obj);
        if (node_map) node_map[*count] = node;
        (*count)++;
        rxvml_value_free(tok_obj);
    }

    for (i = 0; i < 11; i++) {
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
    ASTNode **node_map;
    ASTNode *c;
    rxvml_value *count_val;

    if (!cmd_node) return NULL;

    if (cmd_node->node_type == EXIT_EXTENDED) {
        num_tokens = 1;
        c = cmd_node->child;
        while (c) {
            count_tokens(c, &num_tokens);
            c = c->sibling;
        }

        token_array = rxvml_array_new(ctx, num_tokens);
        if (!token_array) return NULL;

        count_val = rxvml_value_new(ctx);
        rxvml_set_int(count_val, num_tokens);
        rxvml_array_set(ctx, token_array, 0, count_val);
        rxvml_value_free(count_val);

        node_map = malloc(sizeof(ASTNode*) * num_tokens);
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

    if (!command_expression) return NULL;

    /* Count tokens in the expression */
    count_tokens(command_expression, &num_tokens);

    token_array = rxvml_array_new(ctx, num_tokens);
    if (!token_array) return NULL;

    count_val = rxvml_value_new(ctx);
    rxvml_set_int(count_val, num_tokens);
    rxvml_array_set(ctx, token_array, 0, count_val);
    rxvml_value_free(count_val);

    node_map = malloc(sizeof(ASTNode*) * num_tokens);
    collect_tokens(ctx, command_expression, token_array, node_map, &count);

    if (node_map_out) *node_map_out = node_map;
    if (num_tokens_out) *num_tokens_out = num_tokens;

    return token_array;
}

static ExitEntry *rxcp_find_implicit_command_exit(Context *ctx) {
    Context *root;
    ExitEntry *entry;

    root = ctx->master_context ? ctx->master_context : ctx;
    entry = (ExitEntry *)root->exit_registry;
    while (entry) {
        if (entry->flags & RXCP_EXIT_FLAG_IMPLICIT_COMMAND) return entry;
        entry = entry->next;
    }
    return NULL;
}

static ExitEntry *rxcp_resolve_exit_entry(Context *ctx,
                                          ASTNode *node,
                                          ASTNode **node_map,
                                          size_t num_tokens,
                                          const char **dispatch_keyword_out,
                                          size_t *dispatch_keyword_len_out) {
    const char *dispatch_keyword;
    size_t dispatch_keyword_len;
    ExitEntry *entry;
    ASTNode *first_node;

    dispatch_keyword = NULL;
    dispatch_keyword_len = 0;
    entry = NULL;

    if (node->node_type == EXIT_EXTENDED && node->token) {
        dispatch_keyword = node->token->token_string;
        dispatch_keyword_len = node->token->length;
        entry = rxcp_find_exit_entry(ctx, dispatch_keyword, dispatch_keyword_len);
    } else if (node->node_type == IMPLICIT_CMD && num_tokens > 0 && node_map) {
        first_node = node_map[0];
        if (first_node) {
            dispatch_keyword = first_node->token ? first_node->token->token_string : first_node->node_string;
            dispatch_keyword_len = first_node->token ? first_node->token->length : first_node->node_string_length;
            if (dispatch_keyword && dispatch_keyword_len > 0) {
                entry = rxcp_find_exit_entry(ctx, dispatch_keyword, dispatch_keyword_len);
            }
        }

        if (!entry) {
            entry = rxcp_find_implicit_command_exit(ctx);
            if (entry) {
                dispatch_keyword = entry->primary_keyword;
                dispatch_keyword_len = strlen(entry->primary_keyword);
            }
        }
    }

    if (dispatch_keyword_out) *dispatch_keyword_out = dispatch_keyword;
    if (dispatch_keyword_len_out) *dispatch_keyword_len_out = dispatch_keyword_len;
    return entry;
}

static int rxcp_apply_legacy_hoists(Context *ctx,
                                    ASTNode *node,
                                    const char *indices,
                                    size_t indices_len,
                                    ASTNode **node_map,
                                    size_t num_tokens) {
    char *copy;
    char *p;

    if (!indices || indices_len == 0) return 0;

    copy = rx_strndup(indices, indices_len);
    p = strtok(copy, " ");
    while (p) {
        int idx = atoi(p);
        if (idx > 0 && (size_t)idx <= num_tokens && node_map) {
            ASTNode *target = node_map[idx - 1];
            if (target) {
                char *var_name;

                var_name = NULL;
                if (target->token && target->token->token_string && target->token->length > 0) {
                    var_name = rx_strndup(target->token->token_string, target->token->length);
                } else if (target->node_string) {
                    var_name = strdup(target->node_string);
                }
                if (var_name) {
                    ast_hoist_var(ctx, node, var_name, 0);
                    free(var_name);
                }
            }
        }
        p = strtok(NULL, " ");
    }
    free(copy);
    return 0;
}

static int rxcp_read_plan_status(rxvml_context *vctx,
                                 rxvml_value *plan,
                                 char **status_out,
                                 size_t *status_len_out) {
    rxvml_value *status_val;
    const char *status;
    size_t status_len;

    status_val = NULL;
    status = NULL;
    status_len = 0;

    if (rxvml_call_method(vctx, plan, "rxcp.exitplan", "get_status", 0, NULL, &status_val) == 0 && status_val) {
        rxvml_to_str(vctx, status_val, &status, &status_len);
        rxvml_value_free(status_val);
    }

    if (status_out) *status_out = status ? rx_strndup(status, status_len) : NULL;
    if (status_len_out) *status_len_out = status_len;
    return status ? 0 : -1;
}

static int rxcp_response_is_exit_plan(rxvml_context *vctx, rxvml_value *response) {
    char *status;
    size_t status_len;
    int rc;

    if (rxvml_num_attributes(response) < 16) return 0;

    status = NULL;
    status_len = 0;
    rc = rxcp_read_plan_status(vctx, response, &status, &status_len);
    if (status) free(status);
    return rc == 0;
}

static int rxcp_apply_plan_bindings(Context *ctx,
                                    ASTNode *node,
                                    rxvml_context *vctx,
                                    rxvml_value *plan) {
    rxvml_value *count_val;
    rxinteger count;
    rxinteger i;

    count_val = NULL;
    count = 0;

    if (rxvml_call_method(vctx, plan, "rxcp.exitplan", "get_binding_count", 0, NULL, &count_val) != 0 || !count_val) {
        return 0;
    }
    rxvml_to_int(vctx, count_val, &count);
    rxvml_value_free(count_val);

    for (i = 1; i <= count; i++) {
        rxvml_value *kind_val;
        rxvml_value *name_val;
        rxvml_value *type_val;
        rxvml_value *dims_val;
        const char *kind;
        const char *name;
        const char *type_name;
        size_t kind_len;
        size_t name_len;
        size_t type_len;
        rxinteger dims;
        rxvml_value *arg;

        kind_val = NULL;
        name_val = NULL;
        type_val = NULL;
        dims_val = NULL;
        kind = NULL;
        name = NULL;
        type_name = NULL;
        kind_len = 0;
        name_len = 0;
        type_len = 0;
        dims = 0;
        arg = rxvml_value_new(vctx);
        rxvml_set_int(arg, i);

        if (rxvml_call_method(vctx, plan, "rxcp.exitplan", "get_binding_kind", 1, &arg, &kind_val) == 0 && kind_val) {
            rxvml_to_str(vctx, kind_val, &kind, &kind_len);
        }
        if (rxvml_call_method(vctx, plan, "rxcp.exitplan", "get_binding_internal_name", 1, &arg, &name_val) == 0 && name_val) {
            rxvml_to_str(vctx, name_val, &name, &name_len);
        }
        if (rxvml_call_method(vctx, plan, "rxcp.exitplan", "get_binding_value_type", 1, &arg, &type_val) == 0 && type_val) {
            rxvml_to_str(vctx, type_val, &type_name, &type_len);
        }
        if (rxvml_call_method(vctx, plan, "rxcp.exitplan", "get_binding_dimensions", 1, &arg, &dims_val) == 0 && dims_val) {
            rxvml_to_int(vctx, dims_val, &dims);
        }
        rxvml_value_free(arg);

        if (name && name_len > 0 &&
            (!kind || kind_len == 0 || (kind_len == 3 && strncasecmp(kind, "var", 3) == 0))) {
            char *var_name;

            var_name = rx_strndup(name, name_len);
            ast_hoist_var_typed(ctx,
                                node,
                                var_name,
                                0,
                                (type_name && type_len > 0) ? type_name : ".unknown",
                                dims > 0 ? (size_t)dims : 0);
            free(var_name);
        }

        if (dims_val) rxvml_value_free(dims_val);
        if (type_val) rxvml_value_free(type_val);
        if (name_val) rxvml_value_free(name_val);
        if (kind_val) rxvml_value_free(kind_val);
    }
    return 0;
}

static int rxcp_apply_plan_keywords(ExitEntry *entry,
                                    ASTNode **node_map,
                                    size_t num_tokens,
                                    rxvml_context *vctx,
                                    rxvml_value *plan) {
    rxvml_value *count_val;
    rxinteger count;
    rxinteger i;

    if (!entry || !(entry->flags & RXCP_EXIT_FLAG_CERTIFIED)) return 0;

    count_val = NULL;
    count = 0;
    if (rxvml_call_method(vctx, plan, "rxcp.exitplan", "get_keyword_count", 0, NULL, &count_val) != 0 || !count_val) {
        return 0;
    }
    rxvml_to_int(vctx, count_val, &count);
    rxvml_value_free(count_val);

    for (i = 1; i <= count; i++) {
        rxvml_value *idx_val;
        rxinteger idx;
        rxvml_value *arg;

        idx_val = NULL;
        idx = 0;
        arg = rxvml_value_new(vctx);
        rxvml_set_int(arg, i);

        if (rxvml_call_method(vctx, plan, "rxcp.exitplan", "get_keyword_token_index", 1, &arg, &idx_val) == 0 && idx_val) {
            rxvml_to_int(vctx, idx_val, &idx);
        }
        rxvml_value_free(arg);

        if (idx > 0 && (size_t)idx <= num_tokens && node_map) {
            ASTNode *target = node_map[idx - 1];
            if (target && target->token && target->token->token_type == TK_VAR_SYMBOL) {
                target->token->token_type = TK_EXIT_TOKEN;
            }
        }

        if (idx_val) rxvml_value_free(idx_val);
    }
    return 0;
}

static ASTNode *rxcp_find_file_container(ASTNode *node) {
    while (node) {
        if (node->node_type == PROGRAM_FILE || node->node_type == IMPORTED_FILE) {
            return node;
        }
        node = node->parent;
    }
    return NULL;
}

static int rxcp_file_has_import(ASTNode *file_node, const char *import_name) {
    ASTNode *child;

    if (!file_node || !import_name || !import_name[0]) return 0;

    child = file_node->child;
    while (child) {
        if (child->node_type == IMPORT &&
            child->child &&
            is_node_string(child->child, import_name)) {
            return 1;
        }
        child = child->sibling;
    }
    return 0;
}

static int rxcp_insert_import(Context *ctx, ASTNode *node, const char *import_name) {
    ASTNode *file_node;
    ASTNode *import_node;
    ASTNode *literal_node;

    if (!ctx || !node || !import_name || !import_name[0]) return 0;

    file_node = rxcp_find_file_container(node);
    if (!file_node || rxcp_file_has_import(file_node, import_name)) return 0;

    import_node = ast_ft(ctx, IMPORT);
    literal_node = ast_ft(ctx, LITERAL);
    ast_copy_str(literal_node, (char *)import_name);
    add_ast(import_node, literal_node);

    import_node->sibling = file_node->child;
    file_node->child = import_node;
    import_node->parent = file_node;

    return 1;
}

static int rxcp_apply_required_imports(Context *ctx, ASTNode *node, const char *required_imports) {
    const char *cursor;
    int inserted;

    if (!required_imports || !required_imports[0]) return 0;

    inserted = 0;
    cursor = required_imports;
    while (*cursor) {
        const char *start;
        size_t len;
        char *import_name;

        while (*cursor && isspace((unsigned char)*cursor)) cursor++;
        if (!*cursor) break;

        start = cursor;
        while (*cursor && !isspace((unsigned char)*cursor)) cursor++;
        len = (size_t)(cursor - start);
        if (len == 0) continue;

        import_name = rx_strndup(start, len);
        if (rxcp_insert_import(ctx, node, import_name)) {
            inserted = 1;
        }
        free(import_name);
    }

    if (inserted) ctx->changed_flags |= FLAG_EXIT;
    return inserted;
}

static int rxcp_apply_exit_plan(Context *ctx,
                                ASTNode *node,
                                ExitEntry *entry,
                                rxvml_context *vctx,
                                rxvml_value *plan,
                                ASTNode **node_map,
                                size_t num_tokens) {
    char *status;
    size_t status_len;

    status = NULL;
    status_len = 0;

    if (rxcp_read_plan_status(vctx, plan, &status, &status_len) != 0) {
        mknd_err(node, "EXIT_BRIDGE_BAD_PLAN");
        return -1;
    }

    if (status_len >= 5 && strncmp(status, "ERROR", 5) == 0) {
        rxvml_value *val;
        rxinteger error_token_idx;
        const char *err_msg;
        size_t err_msg_len;
        ASTNode *err_node;
        char *err_msg_copy;

        val = NULL;
        error_token_idx = 0;
        err_msg = NULL;
        err_msg_len = 0;
        err_node = node;
        err_msg_copy = NULL;

        if (rxvml_call_method(vctx, plan, "rxcp.exitplan", "get_error_token", 0, NULL, &val) == 0 && val) {
            rxvml_to_int(vctx, val, &error_token_idx);
            rxvml_value_free(val);
            val = NULL;
        }

        if (rxvml_call_method(vctx, plan, "rxcp.exitplan", "get_error_message", 0, NULL, &val) == 0 && val) {
            if (rxvml_to_str(vctx, val, &err_msg, &err_msg_len) == 0 && err_msg) {
                err_msg_copy = rx_strndup(err_msg, err_msg_len);
            }
            rxvml_value_free(val);
            val = NULL;
        }

        if (error_token_idx > 0 && (size_t)error_token_idx <= num_tokens && node_map) {
            err_node = node_map[error_token_idx - 1];
        }

        if (err_msg_copy) {
            mknd_err(err_node, "%s", err_msg_copy);
            free(err_msg_copy);
        } else {
            mknd_err(err_node, "EXIT_BRIDGE_ERROR");
        }
        free(status);
        return -1;
    }

    if (status_len >= 7 && strncmp(status, "PENDING", 7) == 0) {
        ctx->changed_flags |= FLAG_EXIT;
        free(status);
        return 1;
    }

    rxcp_apply_plan_bindings(ctx, node, vctx, plan);
    rxcp_apply_plan_keywords(entry, node_map, num_tokens, vctx, plan);
    free(status);
    return 0;
}

static int rxcp_apply_certified_exit_imports(Context *ctx, ASTNode *node, const char *class_name) {
    const CertifiedExitSpec *spec;

    if (!class_name) return 0;

    spec = rxcp_find_certified_exit_by_class(class_name);
    if (!spec || !(spec->flags & RXCP_EXIT_FLAG_CERTIFIED)) return 0;

    return rxcp_apply_required_imports(ctx, node, spec->required_imports);
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

    {
        int rc;
        const char *mod = getenv("RXCP_EXIT_MODULE");

        rc = rxvml_load_module_file(vctx, "rxcexits");
        if (rc <= 0) {
            fprintf(stderr, "INTERNAL EXIT ERROR: Failed to load certified exit module 'rxcexits'\n");
            rxvml_destroy(vctx);
            exit(-1);
        }

        if (mod && strcmp(mod, "rxcexits") != 0) {
            rc = rxvml_load_module_file(vctx, mod);
            if (rc <= 0) {
                fprintf(stderr,
                        "EXIT_MODULE_LOAD_ERROR: Failed to load '%s'; continuing with certified exits only\n",
                        mod);
            }
        }
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
            rxcp_apply_certified_exit_imports(ctx, node, class_name);
            rxcp_preserve_replaced_node_diagnostics(ctx, node);
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
    rxvml_value* response;
    ASTNode **node_map;
    size_t num_tokens;
    int handled;
    const char *dispatch_keyword;
    size_t dispatch_keyword_len;
    ExitEntry *entry;

    /* Optimization: If the node already has an error, skip redundant invocation.
     * Warnings must not suppress certified-exit lowering. */
    if (rxcp_node_has_error(node)) return 0;

    if (ctx->in_exit_bridge) return 0;
    ctx->in_exit_bridge = 1;
    response = NULL;
    node_map = NULL;
    num_tokens = 0;
    handled = 0;
    dispatch_keyword = NULL;
    dispatch_keyword_len = 0;
    entry = NULL;

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
        rxvml_value* obj;

        obj = rxvml_reg_get(vctx, node->exit_obj_reg, class_name);
        if (obj) {
            if (rxvml_call_method(vctx, obj, class_name, "process", 1, &tok_array, &response) == 0) {
                handled = rxcp_exit_handle_response(ctx, node, vctx, obj, class_name, response, node_map, num_tokens);
                if (response) rxvml_value_free(response);
                response = NULL;
            }
        }
    }

    if (!handled) {
        entry = rxcp_resolve_exit_entry(ctx, node, node_map, num_tokens, &dispatch_keyword, &dispatch_keyword_len);
    }

    if (!handled && entry) {
        rxvml_value* nid_val;
        rxvml_value* obj;
        int release_obj;

        nid_val = rxvml_value_new(vctx);
        obj = NULL;
        release_obj = 1;

        rxvml_set_int(nid_val, node->node_number);
        if (rxvml_call_factory(vctx, entry->class_name, 1, &nid_val, &obj) == 0 && obj) {
            if (rxvml_call_method(vctx, obj, entry->class_name, "process", 1, &tok_array, &response) == 0) {
                handled = rxcp_exit_handle_response(ctx, node, vctx, obj, entry->class_name, response, node_map, num_tokens);
                if (handled > 0) {
                    node->exit_obj_reg = rxvml_reg_alloc(vctx, obj, entry->class_name);
                    release_obj = 0;
                }
            }
            if (response) rxvml_value_free(response);
            response = NULL;
            if (release_obj) rxvml_value_free(obj);
        }
        rxvml_value_free(nid_val);
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

int rxcp_exit_bridge_pre_invoke(Context *ctx, ASTNode *node) {
    rxvml_context* vctx;
    rxvml_value* tok_array;
    rxvml_value* response;
    ASTNode **node_map;
    size_t num_tokens;
    ExitEntry *entry;
    const char *keyword;
    size_t key_len;

    if (ctx->in_exit_bridge) return 0;
    ctx->in_exit_bridge = 1;
    response = NULL;
    node_map = NULL;
    num_tokens = 0;
    entry = NULL;
    keyword = NULL;
    key_len = 0;

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

    entry = rxcp_resolve_exit_entry(ctx, node, node_map, num_tokens, &keyword, &key_len);

    if (entry) {
        rxvml_value* obj;
        char class_name[256];

        obj = NULL;
        class_name[0] = 0;

        if (node->exit_obj_reg != -1) {
            obj = rxvml_reg_get(vctx, node->exit_obj_reg, class_name);
        }

        if (!obj) {
            rxvml_value* nid_val;

            nid_val = rxvml_value_new(vctx);
            rxvml_set_int(nid_val, node->node_number);

            if (rxvml_call_factory(vctx, entry->class_name, 1, &nid_val, &obj) == 0 && obj) {
                int reg_idx;

                reg_idx = rxvml_reg_alloc(vctx, obj, entry->class_name);
                if (reg_idx >= 0) {
                    node->exit_obj_reg = reg_idx;
                    strncpy(class_name, entry->class_name, sizeof(class_name) - 1);
                    class_name[sizeof(class_name) - 1] = 0;
                } else {
                    rxvml_value_free(obj);
                    obj = NULL;
                }
            }
            rxvml_value_free(nid_val);
        }

        if (obj) {
            if (!class_name[0]) {
                strncpy(class_name, entry->class_name, sizeof(class_name) - 1);
                class_name[sizeof(class_name) - 1] = 0;
            }

            if (rxvml_call_method(vctx, obj, class_name, "pre_process", 1, &tok_array, &response) == 0 && response) {
                const char* status;
                size_t status_len;

                status = NULL;
                status_len = 0;

                if (rxcp_response_is_exit_plan(vctx, response)) {
                    rxcp_apply_exit_plan(ctx, node, entry, vctx, response, node_map, num_tokens);
                } else if (rxvml_to_str(vctx, response, &status, &status_len) == 0 && status) {
                    rxcp_apply_legacy_hoists(ctx, node, status, status_len, node_map, num_tokens);
                }
                rxvml_value_free(response);
                response = NULL;
            }
        }
    }

    rxvml_value_free(tok_array);
    if (node_map) free(node_map);
    ctx->in_exit_bridge = 0;
    return 1;
}
