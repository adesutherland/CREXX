#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
    unsigned int required_flags;
} CertifiedExitSpec;

typedef struct ExitHelperRecord {
    ASTNode *file_node;
    char *helper_id;
    char *source_text;
    struct ExitHelperRecord *next;
} ExitHelperRecord;

static const CertifiedExitSpec certified_exit_specs[] = {
    { "ADDRESS",
      RXCP_EXIT_FLAG_CERTIFIED | RXCP_EXIT_FLAG_RESERVED_KEYWORD | RXCP_EXIT_FLAG_IMPLICIT_COMMAND },
    { "PARSE",
      RXCP_EXIT_FLAG_CERTIFIED | RXCP_EXIT_FLAG_RESERVED_KEYWORD },
    { NULL, 0 }
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
                                           unsigned int flags) {
    Context *root = ctx->master_context ? ctx->master_context : ctx;
    ExitEntry *entry = calloc(1, sizeof(ExitEntry));

    if (!entry) return NULL;

    entry->primary_keyword = rx_strndup(primary_keyword, primary_len);
    entry->class_name = strdup(class_name);
    entry->flags = flags;
    entry->next = (ExitEntry *)root->exit_registry;
    root->exit_registry = entry;

    return entry;
}

static void rxcp_register_additional_keyword(Context *ctx,
                                             ExitEntry *entry,
                                             const char *keyword,
                                             size_t keyword_len) {
    Context *root = ctx->master_context ? ctx->master_context : ctx;
    ExitKeyword *kw;
    char *keyword_copy;

    if (!entry || !keyword || keyword_len == 0) return;

    keyword_copy = rx_strndup(keyword, keyword_len);
    kw = calloc(1, sizeof(ExitKeyword));
    if (!kw || !keyword_copy) {
        if (kw) free(kw);
        if (keyword_copy) free(keyword_copy);
        return;
    }

    kw->keyword = keyword_copy;
    kw->next = entry->additional_keywords;
    entry->additional_keywords = kw;

    if (!rxcp_is_exit_additional(ctx, keyword_copy, strlen(keyword_copy))) {
        ExitAdditionalKeywords *akw = calloc(1, sizeof(ExitAdditionalKeywords));
        if (akw) {
            akw->keyword = strdup(keyword_copy);
            akw->next = (ExitAdditionalKeywords *)root->exit_additional_keywords;
            root->exit_additional_keywords = akw;
        }
    }
}

static void rxcp_assert_certified_exits_registered(Context *ctx) {
    int i;

    for (i = 0; certified_exit_specs[i].primary_keyword; i++) {
        const CertifiedExitSpec *spec = &certified_exit_specs[i];
        if (!rxcp_is_exit_primary(ctx, spec->primary_keyword, strlen(spec->primary_keyword))) {
            fprintf(stderr,
                    "INTERNAL EXIT ERROR: Certified exit '%s' was not registered\n",
                    spec->primary_keyword);
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
        ExitImport *import_plan = entry->default_imports;
        while (import_plan) {
            ExitImport *next_import = import_plan->next;
            if (import_plan->namespace_name) free(import_plan->namespace_name);
            if (import_plan->provenance) free(import_plan->provenance);
            if (import_plan->flags) free(import_plan->flags);
            free(import_plan);
            import_plan = next_import;
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

    ExitHelperRecord *helper_record = (ExitHelperRecord *)root->exit_helper_registry;
    while (helper_record) {
        ExitHelperRecord *next_helper = helper_record->next;
        if (helper_record->helper_id) free(helper_record->helper_id);
        if (helper_record->source_text) free(helper_record->source_text);
        free(helper_record);
        helper_record = next_helper;
    }
    root->exit_helper_registry = NULL;
}

static rxvml_context* rxcp_init_bridge(Context* ctx);

static int rxcp_get_method_string_dup(rxvml_context *vctx,
                                      rxvml_value *obj,
                                      const char *class_name,
                                      const char *method_name,
                                      char **value_out,
                                      size_t *value_len_out) {
    rxvml_value *value;
    const char *text;
    size_t text_len;

    value = NULL;
    text = NULL;
    text_len = 0;

    if (value_out) *value_out = NULL;
    if (value_len_out) *value_len_out = 0;

    if (rxvml_call_method(vctx, obj, class_name, method_name, 0, NULL, &value) != 0 || !value) {
        return -1;
    }

    if (rxvml_to_str(vctx, value, &text, &text_len) != 0 || !text) {
        rxvml_value_free(value);
        return -1;
    }

    if (value_out) *value_out = rx_strndup(text, text_len);
    if (value_len_out) *value_len_out = text_len;
    rxvml_value_free(value);
    return 0;
}

static int rxcp_get_method_int(rxvml_context *vctx,
                               rxvml_value *obj,
                               const char *class_name,
                               const char *method_name,
                               rxinteger *value_out) {
    rxvml_value *value;

    value = NULL;
    if (value_out) *value_out = 0;

    if (rxvml_call_method(vctx, obj, class_name, method_name, 0, NULL, &value) != 0 || !value) {
        return -1;
    }

    if (rxvml_to_int(vctx, value, value_out) != 0) {
        rxvml_value_free(value);
        return -1;
    }

    rxvml_value_free(value);
    return 0;
}

static int rxcp_call_indexed_method(rxvml_context *vctx,
                                    rxvml_value *obj,
                                    const char *class_name,
                                    const char *method_name,
                                    rxinteger index,
                                    rxvml_value **value_out) {
    rxvml_value *arg;
    int rc;

    if (value_out) *value_out = NULL;

    arg = rxvml_value_new(vctx);
    if (!arg) return -1;
    rxvml_set_int(arg, index);
    rc = rxvml_call_method(vctx, obj, class_name, method_name, 1, &arg, value_out);
    rxvml_value_free(arg);
    return rc;
}

static void rxcp_add_entry_import(ExitEntry *entry,
                                  const char *namespace_name,
                                  const char *provenance,
                                  const char *flags) {
    ExitImport *import_plan;

    if (!entry || !namespace_name || !namespace_name[0]) return;

    import_plan = entry->default_imports;
    while (import_plan) {
        if (strcasecmp(import_plan->namespace_name, namespace_name) == 0) return;
        import_plan = import_plan->next;
    }

    import_plan = calloc(1, sizeof(ExitImport));
    if (!import_plan) return;

    import_plan->namespace_name = strdup(namespace_name);
    import_plan->provenance = provenance && provenance[0] ? strdup(provenance) : strdup("descriptor");
    import_plan->flags = flags && flags[0] ? strdup(flags) : strdup("");
    import_plan->next = entry->default_imports;
    entry->default_imports = import_plan;
}

static unsigned int rxcp_parse_descriptor_flags(rxvml_context *vctx,
                                                rxvml_value *descriptor,
                                                const char *primary_keyword,
                                                const char *class_name) {
    const CertifiedExitSpec *certified;
    unsigned int flags;
    rxinteger flag_count;
    rxinteger i;

    certified = rxcp_find_certified_exit_by_keyword(primary_keyword);
    flags = 0;
    flag_count = 0;

    if (rxcp_get_method_int(vctx, descriptor, "rxcp.exitdescriptor", "get_flag_count", &flag_count) != 0) {
        return 0;
    }

    for (i = 1; i <= flag_count; i++) {
        rxvml_value *flag_value;
        const char *flag_text;
        size_t flag_len;

        flag_value = NULL;
        flag_text = NULL;
        flag_len = 0;

        if (rxcp_call_indexed_method(vctx, descriptor, "rxcp.exitdescriptor", "get_flag", i, &flag_value) != 0 || !flag_value) {
            continue;
        }

        if (rxvml_to_str(vctx, flag_value, &flag_text, &flag_len) == 0 && flag_text && flag_len > 0) {
            if (strncasecmp(flag_text, "certified", flag_len) == 0) flags |= RXCP_EXIT_FLAG_CERTIFIED;
            else if (strncasecmp(flag_text, "reserved_keyword", flag_len) == 0) flags |= RXCP_EXIT_FLAG_RESERVED_KEYWORD;
            else if (strncasecmp(flag_text, "implicit_command", flag_len) == 0) flags |= RXCP_EXIT_FLAG_IMPLICIT_COMMAND;
        }

        rxvml_value_free(flag_value);
    }

    if (certified) {
        if ((flags & certified->required_flags) != certified->required_flags) {
            fprintf(stderr,
                    "INTERNAL EXIT ERROR: Exit '%s' for '%s' is missing required certified flags\n",
                    class_name,
                    primary_keyword);
            exit(-1);
        }
    } else if (flags & (RXCP_EXIT_FLAG_CERTIFIED | RXCP_EXIT_FLAG_RESERVED_KEYWORD | RXCP_EXIT_FLAG_IMPLICIT_COMMAND)) {
        fprintf(stderr,
                "INTERNAL EXIT ERROR: Exit '%s' uses restricted flags for non-certified primary '%s'\n",
                class_name,
                primary_keyword);
        exit(-1);
    }

    return flags;
}

static void rxcp_register_descriptor_imports(rxvml_context *vctx,
                                             rxvml_value *descriptor,
                                             ExitEntry *entry) {
    rxinteger import_count;
    rxinteger i;

    import_count = 0;
    if (rxcp_get_method_int(vctx, descriptor, "rxcp.exitdescriptor", "get_default_import_count", &import_count) != 0) {
        return;
    }

    for (i = 1; i <= import_count; i++) {
        rxvml_value *import_value;
        char *namespace_name;
        char *provenance;
        char *flags;

        import_value = NULL;
        namespace_name = NULL;
        provenance = NULL;
        flags = NULL;

        if (rxcp_call_indexed_method(vctx, descriptor, "rxcp.exitdescriptor", "get_default_import", i, &import_value) != 0 || !import_value) {
            continue;
        }

        rxcp_get_method_string_dup(vctx, import_value, "rxcp.importplan", "get_namespace_name", &namespace_name, NULL);
        rxcp_get_method_string_dup(vctx, import_value, "rxcp.importplan", "get_provenance", &provenance, NULL);
        rxcp_get_method_string_dup(vctx, import_value, "rxcp.importplan", "get_flags", &flags, NULL);
        rxcp_add_entry_import(entry, namespace_name, provenance, flags);

        if (flags) free(flags);
        if (provenance) free(provenance);
        if (namespace_name) free(namespace_name);
        rxvml_value_free(import_value);
    }
}

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
            unsigned int flags;
            rxvml_value* obj;
            rxvml_value* nid_val;
            rxvml_value* descriptor;
            char *primary_keyword;
            size_t primary_len;
            rxinteger protocol_version;

            if (strstr(classes[i].class_name, ".token")) continue;

            flags = 0;
            obj = NULL;
            descriptor = NULL;
            primary_keyword = NULL;
            primary_len = 0;
            protocol_version = 0;

            nid_val = rxvml_value_new(vctx);
            rxvml_set_int(nid_val, 0);

            if (rxvml_call_factory(vctx, classes[i].class_name, 1, &nid_val, &obj) != 0 || !obj) {
                rxvml_value_free(nid_val);
                continue;
            }

            if (rxvml_call_method(vctx, obj, classes[i].class_name, "describe", 0, NULL, &descriptor) != 0 || !descriptor) {
                fprintf(stderr,
                        "INTERNAL EXIT ERROR: Exit '%s' does not implement describe()\n",
                        classes[i].class_name);
                exit(-1);
            }

            if (rxcp_get_method_int(vctx, descriptor, "rxcp.exitdescriptor", "get_protocol_version", &protocol_version) != 0 ||
                protocol_version != RXCP_EXIT_PROTOCOL_VERSION) {
                fprintf(stderr,
                        "INTERNAL EXIT ERROR: Exit '%s' uses unsupported protocol version\n",
                        classes[i].class_name);
                exit(-1);
            }

            if (rxcp_get_method_string_dup(vctx, descriptor, "rxcp.exitdescriptor", "get_primary_keyword", &primary_keyword, &primary_len) != 0 ||
                !primary_keyword || primary_len == 0) {
                fprintf(stderr,
                        "INTERNAL EXIT ERROR: Exit '%s' returned an empty primary keyword\n",
                        classes[i].class_name);
                exit(-1);
            }

            certified = rxcp_find_certified_exit_by_keyword(primary_keyword);
            flags = rxcp_parse_descriptor_flags(vctx, descriptor, primary_keyword, classes[i].class_name);

            if (is_builtin_keyword(primary_keyword) && !certified) {
                fprintf(stderr,
                        "INTERNAL EXIT ERROR: Exit '%s' uses a Rexx built-in primary keyword '%s'\n",
                        classes[i].class_name,
                        primary_keyword);
                exit(-1);
            }

            if (rxcp_is_exit_primary(ctx, primary_keyword, primary_len)) {
                free(primary_keyword);
                rxvml_value_free(descriptor);
                rxvml_value_free(obj);
                rxvml_value_free(nid_val);
                continue;
            }

            {
                ExitEntry *entry;
                rxinteger additional_count;
                rxinteger j;

                entry = rxcp_register_exit_entry(ctx,
                                                 primary_keyword,
                                                 primary_len,
                                                 classes[i].class_name,
                                                 flags);
                rxcp_register_descriptor_imports(vctx, descriptor, entry);

                additional_count = 0;
                if (rxcp_get_method_int(vctx, descriptor, "rxcp.exitdescriptor", "get_additional_keyword_count", &additional_count) == 0) {
                    for (j = 1; j <= additional_count; j++) {
                        rxvml_value *keyword_value;
                        const char *keyword_text;
                        size_t keyword_len;

                        keyword_value = NULL;
                        keyword_text = NULL;
                        keyword_len = 0;

                        if (rxcp_call_indexed_method(vctx, descriptor, "rxcp.exitdescriptor", "get_additional_keyword", j, &keyword_value) != 0 || !keyword_value) {
                            continue;
                        }

                        if (rxvml_to_str(vctx, keyword_value, &keyword_text, &keyword_len) == 0 && keyword_text && keyword_len > 0) {
                            rxcp_register_additional_keyword(ctx, entry, keyword_text, keyword_len);
                        }
                        rxvml_value_free(keyword_value);
                    }
                }
            }

            free(primary_keyword);
            rxvml_value_free(descriptor);
            rxvml_value_free(obj);
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

static int rxcp_is_simple_integer_text(const char *text, size_t len) {
    size_t i;

    if (!text || len == 0) return 0;
    for (i = 0; i < len; i++) {
        if (!isdigit((unsigned char)text[i])) return 0;
    }
    return 1;
}

static int rxcp_is_simple_decimal_text(const char *text, size_t len) {
    size_t i;
    int seen_digit;
    int seen_dot;

    if (!text || len == 0) return 0;
    seen_digit = 0;
    seen_dot = 0;
    for (i = 0; i < len; i++) {
        if (isdigit((unsigned char)text[i])) {
            seen_digit = 1;
            continue;
        }
        if (text[i] == '.' && !seen_dot) {
            seen_dot = 1;
            continue;
        }
        return 0;
    }
    return seen_digit && seen_dot;
}

static int rxcp_is_simple_operator_text(const char *text, size_t len) {
    if (!text || len == 0) return 0;
    if (len == 1 && strchr("+-*/%=<>\\", text[0])) return 1;
    if (len == 2) {
        if ((text[0] == '*' && text[1] == '*') ||
            (text[0] == '/' && text[1] == '/') ||
            (text[0] == '<' && text[1] == '=') ||
            (text[0] == '>' && text[1] == '=') ||
            (text[0] == '<' && text[1] == '>')) {
            return 1;
        }
    }
    return 0;
}

static const char *rxcp_classify_exit_token_text(const char *text, size_t len) {
    if (!text || len == 0) return "other";
    if (len == 1) {
        if (text[0] == ',') return "comma";
        if (text[0] == '(' || text[0] == ')' || text[0] == '[' || text[0] == ']') return "bracket";
    }
    if ((text[0] == '\'' || text[0] == '"') && len >= 2 && text[len - 1] == text[0]) return "string_literal";
    if (rxcp_is_simple_integer_text(text, len)) return "int_literal";
    if (rxcp_is_simple_decimal_text(text, len)) return "decimal_literal";
    if (rxcp_is_simple_operator_text(text, len)) return "operator";
    return "identifier";
}

static const char* token_type_to_string(int type, const char *text, size_t text_len) {
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
        case TK_EXIT_TOKEN:   return rxcp_classify_exit_token_text(text, text_len);
        case TK_COMMA:      return "comma";
        case TK_OPEN_BRACKET:
        case TK_CLOSE_BRACKET:
        case TK_OPEN_SBRACKET:
        case TK_CLOSE_SBRACKET: return "bracket";
        case TK_LABEL:      return "label";
        default:            return "other";
    }
}

static const char *rxcp_node_token_type_string(ASTNode *node) {
    const char *text;
    size_t text_len;
    int token_type;

    if (!node) return "other";

    text = node->token ? node->token->token_string : node->node_string;
    text_len = node->token ? (size_t)node->token->length : (size_t)node->node_string_length;
    token_type = node->token ? node->token->token_type : 0;

    switch (node->node_type) {
        case EXIT_EXTENDED:
            return "exit_primary";
        case EXIT_TOKEN:
            return rxcp_classify_exit_token_text(text, text_len);
        case VAR_SYMBOL:
        case VAR_REFERENCE:
        case VAR_TARGET:
        case CONST_SYMBOL:
            return "identifier";
        default:
            break;
    }

    return token_type_to_string(token_type, text, text_len);
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
    {
        const char *type_string = rxcp_node_token_type_string(node);
        rxvml_set_str(args[9], type_string, strlen(type_string));
    }
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

static int rxcp_read_status(rxvml_context *vctx,
                            rxvml_value *obj,
                            const char *class_name,
                            char **status_out,
                            size_t *status_len_out) {
    return rxcp_get_method_string_dup(vctx, obj, class_name, "get_status", status_out, status_len_out);
}

static ASTNode *rxcp_node_from_token_index(ASTNode *fallback,
                                           ASTNode **node_map,
                                           size_t num_tokens,
                                           rxinteger token_index) {
    if (token_index > 0 && (size_t)token_index <= num_tokens && node_map) {
        ASTNode *mapped = node_map[token_index - 1];
        if (mapped) return mapped;
    }
    return fallback;
}

static char *rxcp_format_diagnostic_text(const char *severity,
                                         const char *code,
                                         const char *message) {
    int is_note;

    is_note = severity && strcasecmp(severity, "note") == 0;

    if (is_note) {
        if (code && code[0] && message && message[0]) return mprintf("NOTE %s, \"%s\"", code, message);
        if (code && code[0]) return mprintf("NOTE %s", code);
        if (message && message[0]) return mprintf("NOTE, \"%s\"", message);
        return strdup("NOTE");
    }

    if (code && code[0] && message && message[0]) return mprintf("%s, \"%s\"", code, message);
    if (code && code[0]) return strdup(code);
    if (message && message[0]) return strdup(message);
    return strdup("EXIT_BRIDGE_ERROR");
}

static int rxcp_apply_diagnostic_object(Context *ctx,
                                        ASTNode *node,
                                        rxvml_context *vctx,
                                        rxvml_value *diagnostic,
                                        ASTNode **node_map,
                                        size_t num_tokens) {
    char *severity;
    char *code;
    char *message;
    rxinteger token_index;
    ASTNode *diag_node;
    char *formatted;
    int saved_in_exit_bridge;

    severity = NULL;
    code = NULL;
    message = NULL;
    token_index = 0;

    rxcp_get_method_string_dup(vctx, diagnostic, "rxcp.exitdiagnostic", "get_severity", &severity, NULL);
    rxcp_get_method_string_dup(vctx, diagnostic, "rxcp.exitdiagnostic", "get_code", &code, NULL);
    rxcp_get_method_string_dup(vctx, diagnostic, "rxcp.exitdiagnostic", "get_message", &message, NULL);
    rxcp_get_method_int(vctx, diagnostic, "rxcp.exitdiagnostic", "get_token_index", &token_index);

    diag_node = rxcp_node_from_token_index(node, node_map, num_tokens, token_index);
    formatted = rxcp_format_diagnostic_text(severity ? severity : "error", code ? code : "", message ? message : "");
    saved_in_exit_bridge = ctx ? ctx->in_exit_bridge : 0;

    /* Exit-authored diagnostics describe user source unless they are explicit
     * bridge failures emitted elsewhere. Do not mark them as generated-code
     * internals just because they were transported through the bridge. */
    if (ctx) ctx->in_exit_bridge = 0;

    if (severity && (strcasecmp(severity, "warning") == 0 || strcasecmp(severity, "note") == 0)) {
        mknd_war(diag_node, "%s", formatted);
    } else {
        mknd_err_unique(diag_node, "%s", formatted);
        if (ctx) ctx->in_exit_bridge = saved_in_exit_bridge;
        free(formatted);
        if (message) free(message);
        if (code) free(code);
        if (severity) free(severity);
        return -1;
    }

    if (ctx) ctx->in_exit_bridge = saved_in_exit_bridge;
    free(formatted);
    if (message) free(message);
    if (code) free(code);
    if (severity) free(severity);
    return 0;
}

static int rxcp_apply_diagnostics(Context *ctx,
                                  ASTNode *node,
                                  rxvml_context *vctx,
                                  rxvml_value *owner,
                                  const char *class_name,
                                  ASTNode **node_map,
                                  size_t num_tokens) {
    rxinteger diagnostic_count;
    rxinteger i;
    int saw_error;

    diagnostic_count = 0;
    saw_error = 0;
    if (rxcp_get_method_int(vctx, owner, class_name, "get_diagnostic_count", &diagnostic_count) != 0) {
        return 0;
    }

    for (i = 1; i <= diagnostic_count; i++) {
        rxvml_value *diagnostic;
        int rc;

        diagnostic = NULL;
        if (rxcp_call_indexed_method(vctx, owner, class_name, "get_diagnostic", i, &diagnostic) != 0 || !diagnostic) {
            continue;
        }

        rc = rxcp_apply_diagnostic_object(ctx, node, vctx, diagnostic, node_map, num_tokens);
        if (rc < 0) saw_error = 1;
        rxvml_value_free(diagnostic);
    }

    return saw_error ? -1 : 0;
}

static void rxcp_log_notes(Context *ctx,
                           rxvml_context *vctx,
                           rxvml_value *owner,
                           const char *class_name,
                           const char *primary_keyword,
                           ASTNode *node) {
    rxinteger note_count;
    rxinteger i;

    if (!ctx || ctx->debug_mode < 2) return;

    note_count = 0;
    if (rxcp_get_method_int(vctx, owner, class_name, "get_note_count", &note_count) != 0) {
        return;
    }

    for (i = 1; i <= note_count; i++) {
        rxvml_value *note_value;
        const char *note_text;
        size_t note_len;

        note_value = NULL;
        note_text = NULL;
        note_len = 0;
        if (rxcp_call_indexed_method(vctx, owner, class_name, "get_note", i, &note_value) != 0 || !note_value) {
            continue;
        }

        if (rxvml_to_str(vctx, note_value, &note_text, &note_len) == 0 && note_text && note_len > 0) {
            fprintf(stderr,
                    "DEBUG_EXIT_NOTE: %s @ %d:%d - %.*s\n",
                    primary_keyword ? primary_keyword : "<unknown>",
                    node ? node->line : -1,
                    node ? node->column : -1,
                    (int)note_len,
                    note_text);
        }
        rxvml_value_free(note_value);
    }
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

static int rxcp_apply_import_object(Context *ctx,
                                    ASTNode *node,
                                    rxvml_context *vctx,
                                    rxvml_value *import_plan) {
    char *namespace_name;
    int inserted;

    namespace_name = NULL;
    inserted = 0;
    if (rxcp_get_method_string_dup(vctx, import_plan, "rxcp.importplan", "get_namespace_name", &namespace_name, NULL) == 0 &&
        namespace_name && namespace_name[0]) {
        inserted = rxcp_insert_import(ctx, node, namespace_name);
        if (inserted) ctx->changed_flags |= FLAG_EXIT;
    }
    if (namespace_name) free(namespace_name);
    return inserted;
}

static int rxcp_apply_entry_default_imports(Context *ctx, ASTNode *node, ExitEntry *entry) {
    ExitImport *import_plan;
    int inserted;

    inserted = 0;
    if (!entry) return 0;

    import_plan = entry->default_imports;
    while (import_plan) {
        if (import_plan->namespace_name && import_plan->namespace_name[0] &&
            rxcp_insert_import(ctx, node, import_plan->namespace_name)) {
            inserted = 1;
        }
        import_plan = import_plan->next;
    }

    if (inserted) ctx->changed_flags |= FLAG_EXIT;
    return inserted;
}

static int rxcp_apply_plan_bindings(Context *ctx,
                                    ASTNode *node,
                                    rxvml_context *vctx,
                                    rxvml_value *plan) {
    rxinteger binding_count;
    rxinteger i;

    binding_count = 0;
    if (rxcp_get_method_int(vctx, plan, "rxcp.exitplan", "get_binding_count", &binding_count) != 0) {
        return 0;
    }

    for (i = 1; i <= binding_count; i++) {
        rxvml_value *binding;
        char *kind;
        char *internal_name;
        char *type_name;
        rxinteger dims;

        binding = NULL;
        kind = NULL;
        internal_name = NULL;
        type_name = NULL;
        dims = 0;

        if (rxcp_call_indexed_method(vctx, plan, "rxcp.exitplan", "get_binding", i, &binding) != 0 || !binding) {
            continue;
        }

        rxcp_get_method_string_dup(vctx, binding, "rxcp.bindingplan", "get_kind", &kind, NULL);
        rxcp_get_method_string_dup(vctx, binding, "rxcp.bindingplan", "get_internal_name", &internal_name, NULL);
        rxcp_get_method_string_dup(vctx, binding, "rxcp.bindingplan", "get_value_type", &type_name, NULL);
        rxcp_get_method_int(vctx, binding, "rxcp.bindingplan", "get_dimensions", &dims);

        if (internal_name && internal_name[0] &&
            (!kind || !kind[0] || strcasecmp(kind, "var") == 0)) {
            ast_hoist_var_typed(ctx,
                                node,
                                internal_name,
                                0,
                                (type_name && type_name[0]) ? type_name : ".unknown",
                                dims > 0 ? (size_t)dims : 0);
        }

        if (type_name) free(type_name);
        if (internal_name) free(internal_name);
        if (kind) free(kind);
        rxvml_value_free(binding);
    }
    return 0;
}

static int rxcp_apply_plan_keywords(ASTNode **node_map,
                                    size_t num_tokens,
                                    rxvml_context *vctx,
                                    rxvml_value *plan) {
    rxinteger keyword_count;
    rxinteger i;

    keyword_count = 0;
    if (rxcp_get_method_int(vctx, plan, "rxcp.exitplan", "get_keyword_count", &keyword_count) != 0) {
        return 0;
    }

    for (i = 1; i <= keyword_count; i++) {
        rxvml_value *keyword;
        rxinteger token_index;
        ASTNode *target;

        keyword = NULL;
        token_index = 0;

        if (rxcp_call_indexed_method(vctx, plan, "rxcp.exitplan", "get_keyword", i, &keyword) != 0 || !keyword) {
            continue;
        }

        rxcp_get_method_int(vctx, keyword, "rxcp.keywordclaim", "get_token_index", &token_index);
        target = rxcp_node_from_token_index(NULL, node_map, num_tokens, token_index);
        if (target && target->token && target->token->token_type == TK_VAR_SYMBOL) {
            target->token->token_type = TK_EXIT_TOKEN;
        }

        rxvml_value_free(keyword);
    }
    return 0;
}

static int rxcp_apply_plan_imports(Context *ctx,
                                   ASTNode *node,
                                   rxvml_context *vctx,
                                   rxvml_value *plan) {
    rxinteger import_count;
    rxinteger i;
    int inserted;

    import_count = 0;
    inserted = 0;
    if (rxcp_get_method_int(vctx, plan, "rxcp.exitplan", "get_import_count", &import_count) != 0) {
        return 0;
    }

    for (i = 1; i <= import_count; i++) {
        rxvml_value *import_plan;

        import_plan = NULL;
        if (rxcp_call_indexed_method(vctx, plan, "rxcp.exitplan", "get_import", i, &import_plan) != 0 || !import_plan) {
            continue;
        }

        if (rxcp_apply_import_object(ctx, node, vctx, import_plan)) inserted = 1;
        rxvml_value_free(import_plan);
    }
    return inserted;
}

static void rxcp_merge_fragment_context(Context *ctx, Context *frag) {
    if (!ctx || !frag) return;

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
    }

    if (frag->buff_start) {
        ctx->extra_buffers_count++;
        ctx->extra_buffers = realloc(ctx->extra_buffers, sizeof(char*) * ctx->extra_buffers_count);
        ctx->extra_buffers[ctx->extra_buffers_count - 1] = frag->buff_start;
        frag->buff_start = NULL;
    }
}

static void rxcp_mark_helper_subtree(ASTNode *node, ASTNode *source_anchor) {
    ASTNode *current;

    current = node;
    while (current) {
        ast_copy_source_anchor(current, source_anchor, AST_SOURCE_SYNTHETIC);
        current->is_compiler_added = 1;
        current->skip_exit_dispatch = 1;
        if (current->child) rxcp_mark_helper_subtree(current->child, source_anchor);
        current = current->sibling;
    }
}

static char *rxcp_join_helper_source(rxvml_context *vctx, rxvml_value *helper_plan) {
    rxinteger line_count;
    rxinteger i;
    char **lines;
    size_t total_len;
    char *buffer;
    size_t pos;

    line_count = 0;
    if (rxcp_get_method_int(vctx, helper_plan, "rxcp.helperplan", "get_line_count", &line_count) != 0 || line_count <= 0) {
        return NULL;
    }

    lines = calloc((size_t)line_count, sizeof(char *));
    if (!lines) return NULL;

    total_len = 0;
    for (i = 1; i <= line_count; i++) {
        rxvml_value *line_value;
        const char *line_text;
        size_t line_len;

        line_value = NULL;
        line_text = NULL;
        line_len = 0;

        if (rxcp_call_indexed_method(vctx, helper_plan, "rxcp.helperplan", "get_line", i, &line_value) == 0 && line_value &&
            rxvml_to_str(vctx, line_value, &line_text, &line_len) == 0 && line_text) {
            lines[i - 1] = rx_strndup(line_text, line_len);
            total_len += line_len + 1;
        }
        if (line_value) rxvml_value_free(line_value);
    }

    if (total_len == 0) {
        free(lines);
        return NULL;
    }

    buffer = malloc(total_len + 1);
    if (!buffer) {
        for (i = 0; i < line_count; i++) {
            if (lines[i]) free(lines[i]);
        }
        free(lines);
        return NULL;
    }

    pos = 0;
    for (i = 0; i < line_count; i++) {
        size_t len = lines[i] ? strlen(lines[i]) : 0;
        if (len > 0) {
            memcpy(buffer + pos, lines[i], len);
            pos += len;
        }
        buffer[pos++] = '\n';
        if (lines[i]) free(lines[i]);
    }
    buffer[pos] = 0;
    free(lines);
    return buffer;
}

static int rxcp_check_helper_source(Context *ctx,
                                    ASTNode *file_node,
                                    const char *helper_id,
                                    const char *source_text) {
    Context *root;
    ExitHelperRecord *record;

    root = ctx->master_context ? ctx->master_context : ctx;
    record = (ExitHelperRecord *)root->exit_helper_registry;
    while (record) {
        if (record->file_node == file_node && strcmp(record->helper_id, helper_id) == 0) {
            return strcmp(record->source_text, source_text) == 0 ? 0 : -1;
        }
        record = record->next;
    }

    return 1;
}

static int rxcp_store_helper_source(Context *ctx,
                                    ASTNode *file_node,
                                    const char *helper_id,
                                    const char *source_text) {
    Context *root;
    ExitHelperRecord *record;

    root = ctx->master_context ? ctx->master_context : ctx;

    record = calloc(1, sizeof(ExitHelperRecord));
    if (!record) return -1;
    record->file_node = file_node;
    record->helper_id = strdup(helper_id);
    record->source_text = strdup(source_text);
    record->next = (ExitHelperRecord *)root->exit_helper_registry;
    root->exit_helper_registry = record;
    return 1;
}

static int rxcp_append_helper(Context *ctx,
                              ASTNode *node,
                              const char *helper_id,
                              const char *helper_source) {
    Context *frag;
    ASTNode *file_node;
    ASTNode *program_file;
    ASTNode *child;
    ASTNode *helper_proc;
    int registration_rc;
    char *prefixed_source;

    if (!ctx || !node || !helper_id || !helper_id[0] || !helper_source || !helper_source[0]) return -1;

    file_node = rxcp_find_file_container(node);
    if (!file_node) return -1;

    registration_rc = rxcp_check_helper_source(ctx, file_node, helper_id, helper_source);
    if (registration_rc == 0) return 0;
    if (registration_rc < 0) {
        mknd_err_unique(node, "EXIT_BRIDGE_HELPER_CONFLICT, \"%s\"", helper_id);
        return -1;
    }

    prefixed_source = mprintf("options levelb\n%s", helper_source);
    frag = cntx_f();
    if (!frag) {
        mknd_err_unique(node, "EXIT_BRIDGE_HELPER_CONTEXT_FAILED, \"%s\"", helper_id);
        free(prefixed_source);
        return -1;
    }

    frag->in_exit_bridge = 1;
    frag->master_context = ctx->master_context;
    frag->location = ctx->location;
    frag->file_name = "exit_helper_fragment";
    frag->level = ctx->level;
    frag->debug_mode = ctx->debug_mode;
    frag->disable_exits = ctx->disable_exits;
    frag->floats_decimal = ctx->floats_decimal;
    frag->floats_binary = ctx->floats_binary;
    frag->numeric_standard = ctx->numeric_standard;
    frag->comments_hash = ctx->comments_hash;
    frag->comments_slash = ctx->comments_slash;
    frag->comments_dash = ctx->comments_dash;

    cntx_buf(frag, prefixed_source, strlen(prefixed_source));
    if (rexbpars(frag)) {
        mknd_err_unique(node, "EXIT_BRIDGE_HELPER_PARSE_FAILED, \"%s\"", helper_id);
        fre_cntx(frag);
        return -1;
    }

    ast_wlkr(frag->ast, ast_structure_fixup_walker, (void *) frag);
    ast_wlkr(frag->ast, source_location_walker, (void *) frag);
    ast_wlkr(frag->ast, syntax_validation_walker, (void *) frag);
    ast_wlkr(frag->ast, rxcp_fixup_walker, (void *) frag);

    if (frag->floats_decimal) {
        ast_wlkr(frag->ast, float2decimal_walker, (void *) frag);
    } else if (frag->floats_binary) {
        ast_wlkr(frag->ast, decimal2float_walker, (void *) frag);
    }

    program_file = ast_fndn(frag, frag->ast, PROGRAM_FILE);
    helper_proc = NULL;
    if (program_file) {
        child = program_file->child;
        while (child) {
            if (child->node_type == PROCEDURE) {
                if (helper_proc) {
                    mknd_err_unique(node, "EXIT_BRIDGE_HELPER_MULTI_DEF, \"%s\"", helper_id);
                    fre_cntx(frag);
                    return -1;
                }
                helper_proc = child;
            } else if (child->node_type == CLASS_DEF || child->node_type == METHOD || child->node_type == FACTORY) {
                mknd_err_unique(node, "EXIT_BRIDGE_HELPER_BAD_SHAPE, \"%s\"", helper_id);
                fre_cntx(frag);
                return -1;
            }
            child = child->sibling;
        }
    }

    if (!helper_proc) {
        mknd_err_unique(node, "EXIT_BRIDGE_HELPER_MISSING_DEF, \"%s\"", helper_id);
        fre_cntx(frag);
        return -1;
    }

    helper_proc->sibling = NULL;
    rxcp_mark_helper_subtree(helper_proc, node);
    add_dast(file_node, helper_proc);
    if (rxcp_store_helper_source(ctx, file_node, helper_id, helper_source) < 0) {
        mknd_err_unique(node, "EXIT_BRIDGE_HELPER_REGISTRY_FAILED, \"%s\"", helper_id);
        fre_cntx(frag);
        return -1;
    }
    rxcp_merge_fragment_context(ctx, frag);
    ctx->changed_flags |= FLAG_EXIT;

    fre_cntx(frag);
    return 1;
}

static int rxcp_apply_plan_helpers(Context *ctx,
                                   ASTNode *node,
                                   rxvml_context *vctx,
                                   rxvml_value *plan) {
    rxinteger helper_count;
    rxinteger i;

    helper_count = 0;
    if (rxcp_get_method_int(vctx, plan, "rxcp.exitplan", "get_helper_count", &helper_count) != 0) {
        return 0;
    }

    for (i = 1; i <= helper_count; i++) {
        rxvml_value *helper_plan;
        char *helper_id;
        char *scope;
        char *source_text;

        helper_plan = NULL;
        helper_id = NULL;
        scope = NULL;
        source_text = NULL;

        if (rxcp_call_indexed_method(vctx, plan, "rxcp.exitplan", "get_helper", i, &helper_plan) != 0 || !helper_plan) {
            continue;
        }

        rxcp_get_method_string_dup(vctx, helper_plan, "rxcp.helperplan", "get_helper_id", &helper_id, NULL);
        rxcp_get_method_string_dup(vctx, helper_plan, "rxcp.helperplan", "get_scope", &scope, NULL);
        source_text = rxcp_join_helper_source(vctx, helper_plan);

        if (!scope || strcasecmp(scope, "file_tail") != 0) {
            mknd_err_unique(node, "EXIT_BRIDGE_UNSUPPORTED_HELPER_SCOPE, \"%s\"", scope ? scope : "");
            if (source_text) free(source_text);
            if (scope) free(scope);
            if (helper_id) free(helper_id);
            rxvml_value_free(helper_plan);
            return -1;
        }

        if (!helper_id || !helper_id[0] || !source_text || !source_text[0]) {
            mknd_err_unique(node, "EXIT_BRIDGE_BAD_HELPER");
            if (source_text) free(source_text);
            if (scope) free(scope);
            if (helper_id) free(helper_id);
            rxvml_value_free(helper_plan);
            return -1;
        }

        if (rxcp_append_helper(ctx, node, helper_id, source_text) < 0) {
            free(source_text);
            if (scope) free(scope);
            if (helper_id) free(helper_id);
            rxvml_value_free(helper_plan);
            return -1;
        }

        free(source_text);
        if (scope) free(scope);
        if (helper_id) free(helper_id);
        rxvml_value_free(helper_plan);
    }

    return 0;
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
    int diag_rc;

    status = NULL;
    status_len = 0;
    if (rxcp_read_status(vctx, plan, "rxcp.exitplan", &status, &status_len) != 0 || !status) {
        mknd_err_unique(node, "EXIT_BRIDGE_BAD_PLAN");
        return -1;
    }

    rxcp_apply_entry_default_imports(ctx, node, entry);
    rxcp_apply_plan_imports(ctx, node, vctx, plan);
    rxcp_apply_plan_bindings(ctx, node, vctx, plan);
    rxcp_apply_plan_keywords(node_map, num_tokens, vctx, plan);
    if (rxcp_apply_plan_helpers(ctx, node, vctx, plan) < 0) {
        free(status);
        return -1;
    }

    rxcp_log_notes(ctx, vctx, plan, "rxcp.exitplan", entry ? entry->primary_keyword : NULL, node);
    diag_rc = rxcp_apply_diagnostics(ctx, node, vctx, plan, "rxcp.exitplan", node_map, num_tokens);

    if (status_len >= 7 && strncmp(status, "PENDING", 7) == 0) {
        ctx->changed_flags |= FLAG_EXIT;
        free(status);
        return diag_rc < 0 ? -1 : 1;
    }

    if (status_len >= 5 && strncmp(status, "ERROR", 5) == 0) {
        if (diag_rc >= 0) mknd_err_unique(node, "EXIT_BRIDGE_ERROR");
        free(status);
        return -1;
    }

    free(status);
    return diag_rc;
}

static void rxcp_say_exit(char* message) {
    fprintf(stdout, "%s", message);
    fflush(stdout);
}

static int rxcp_report_bridge_method_failure(ASTNode *node,
                                             rxvml_context *vctx,
                                             const char *code,
                                             const char *class_name,
                                             const char *method_name) {
    const char *vm_error;

    vm_error = NULL;
    rxvml_last_error(vctx, &vm_error);

    if (vm_error && vm_error[0]) {
        mknd_err_unique(node,
                        "%s, \"%s.%s: %s\"",
                        code,
                        class_name ? class_name : "<unknown>",
                        method_name ? method_name : "<unknown>",
                        vm_error);
    } else {
        mknd_err_unique(node,
                        "%s, \"%s.%s\"",
                        code,
                        class_name ? class_name : "<unknown>",
                        method_name ? method_name : "<unknown>");
    }

    return -1;
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


static char *rxcp_join_result_lines(rxvml_context *vctx, rxvml_value *result) {
    rxinteger line_count;
    rxinteger i;
    char **lines;
    size_t total_len;
    char *buffer;
    size_t pos;

    line_count = 0;
    if (rxcp_get_method_int(vctx, result, "rxcp.exitresult", "get_replacement_line_count", &line_count) != 0 || line_count <= 0) {
        return NULL;
    }

    lines = calloc((size_t)line_count, sizeof(char *));
    if (!lines) return NULL;

    total_len = 0;
    for (i = 1; i <= line_count; i++) {
        rxvml_value *line_value;
        const char *line_text;
        size_t line_len;

        line_value = NULL;
        line_text = NULL;
        line_len = 0;
        if (rxcp_call_indexed_method(vctx, result, "rxcp.exitresult", "get_replacement_line", i, &line_value) == 0 && line_value &&
            rxvml_to_str(vctx, line_value, &line_text, &line_len) == 0 && line_text) {
            lines[i - 1] = rx_strndup(line_text, line_len);
            total_len += line_len;
            if (i < line_count) total_len++;
        }
        if (line_value) rxvml_value_free(line_value);
    }

    if (total_len == 0) {
        free(lines);
        return NULL;
    }

    buffer = malloc(total_len + 1);
    if (!buffer) {
        for (i = 0; i < line_count; i++) {
            if (lines[i]) free(lines[i]);
        }
        free(lines);
        return NULL;
    }

    pos = 0;
    for (i = 0; i < line_count; i++) {
        size_t len = lines[i] ? strlen(lines[i]) : 0;
        if (len > 0) {
            memcpy(buffer + pos, lines[i], len);
            pos += len;
        }
        if (i + 1 < line_count) buffer[pos++] = '\n';
        if (lines[i]) free(lines[i]);
    }
    buffer[pos] = 0;
    free(lines);
    return buffer;
}

static int rxcp_exit_handle_response(Context* ctx,
                                     ASTNode* node,
                                     rxvml_context* vctx,
                                     ExitEntry *entry,
                                     rxvml_value* response,
                                     ASTNode **node_map,
                                     size_t num_tokens) {
    char *status;
    size_t status_len;
    int diag_rc;

    status = NULL;
    status_len = 0;
    if (rxcp_read_status(vctx, response, "rxcp.exitresult", &status, &status_len) != 0 || !status) {
        mknd_err_unique(node, "EXIT_BRIDGE_BAD_RESULT");
        return -1;
    }

    rxcp_log_notes(ctx, vctx, response, "rxcp.exitresult", entry ? entry->primary_keyword : NULL, node);
    diag_rc = rxcp_apply_diagnostics(ctx, node, vctx, response, "rxcp.exitresult", node_map, num_tokens);

    if (strncmp(status, "REJECT", status_len) == 0) {
        free(status);
        return 0;
    }

    if (strncmp(status, "ACCEPT", status_len) == 0) {
        free(status);
        return diag_rc < 0 ? -1 : 1;
    }

    if (strncmp(status, "PENDING", status_len) == 0) {
        ctx->changed_flags |= FLAG_EXIT;
        free(status);
        return diag_rc < 0 ? -1 : 1;
    }

    if (status_len >= 5 && strncmp(status, "ERROR", 5) == 0) {
        if (diag_rc >= 0) mknd_err_unique(node, "EXIT_BRIDGE_ERROR");
        free(status);
        return -1;
    }

    if (status_len >= 7 && strncmp(status, "REPLACE", 7) == 0) {
        char *replacement_code;
        int rc;

        replacement_code = rxcp_join_result_lines(vctx, response);
        if (!replacement_code || !replacement_code[0]) {
            if (replacement_code) free(replacement_code);
            free(status);
            mknd_err_unique(node, "EXIT_BRIDGE_REPLACE_MISSING_TEXT");
            return -1;
        }

        if (diag_rc < 0) {
            free(replacement_code);
            free(status);
            return -1;
        }

        rxcp_preserve_replaced_node_diagnostics(ctx, node);
        rc = ast_grft_interpolated(ctx, node, replacement_code, node_map, num_tokens);
        ctx->changed_flags |= FLAG_EXIT;
        free(replacement_code);
        free(status);
        return rc < 0 ? -1 : -1;
    }

    free(status);
    return diag_rc;
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
            if (rxvml_call_method(vctx, obj, class_name, "process", 1, &tok_array, &response) == 0 && response) {
                handled = rxcp_exit_handle_response(ctx, node, vctx, NULL, response, node_map, num_tokens);
                if (response) rxvml_value_free(response);
                response = NULL;
            } else {
                if (response) rxvml_value_free(response);
                response = NULL;
                handled = rxcp_report_bridge_method_failure(node,
                                                           vctx,
                                                           "EXIT_BRIDGE_PROCESS_FAILED",
                                                           class_name,
                                                           "process");
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
            if (rxvml_call_method(vctx, obj, entry->class_name, "process", 1, &tok_array, &response) == 0 && response) {
                handled = rxcp_exit_handle_response(ctx, node, vctx, entry, response, node_map, num_tokens);
                if (handled > 0) {
                    node->exit_obj_reg = rxvml_reg_alloc(vctx, obj, entry->class_name);
                    release_obj = 0;
                }
            } else {
                if (response) rxvml_value_free(response);
                response = NULL;
                handled = rxcp_report_bridge_method_failure(node,
                                                           vctx,
                                                           "EXIT_BRIDGE_PROCESS_FAILED",
                                                           entry->class_name,
                                                           "process");
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

int rxcp_exit_bridge_plan_invoke(Context *ctx, ASTNode *node) {
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
                if (rxcp_apply_exit_plan(ctx, node, entry, vctx, response, node_map, num_tokens) < 0) {
                    rxvml_value_free(response);
                    rxvml_value_free(tok_array);
                    if (node_map) free(node_map);
                    ctx->in_exit_bridge = 0;
                    return -1;
                }
                rxvml_value_free(response);
                response = NULL;
            } else {
                if (response) rxvml_value_free(response);
                response = NULL;
                rxcp_report_bridge_method_failure(node,
                                                  vctx,
                                                  "EXIT_BRIDGE_PREPROCESS_FAILED",
                                                  class_name,
                                                  "pre_process");
                rxvml_value_free(tok_array);
                if (node_map) free(node_map);
                ctx->in_exit_bridge = 0;
                return -1;
            }
        }
    }

    rxvml_value_free(tok_array);
    if (node_map) free(node_map);
    ctx->in_exit_bridge = 0;
    return 1;
}
