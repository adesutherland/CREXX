#ifdef ENABLE_PARSER_MODE

#ifndef restrict
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#elif defined(__GNUC__) || defined(__clang__)
#define restrict __restrict
#elif defined(_MSC_VER)
#define restrict __restrict
#else
#define restrict
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Rename conflicting cREXX AST node types to avoid windows.h collision */
#define ERROR RXCP_ERROR
#define WARNING RXCP_WARNING
#define FLOAT RXCP_FLOAT
#define INTEGER RXCP_INTEGER
#define DECIMAL RXCP_DECIMAL
#define PATTERN RXCP_PATTERN
#define VOID RXCP_VOID

#include "rxcpmain.h"
#include "rxcp_source_tree.h"
#include "rxcp_val.h"
#include "rxcpbgmr.h"
#include "rxcp_exit.h"
#include "utf.h"
#include "platform.h"

#undef ERROR
#undef WARNING
#undef FLOAT
#undef INTEGER
#undef DECIMAL
#undef PATTERN
#undef VOID

#include "dslsyntax_common.h"
#include "dslsyntax_parser.h"
#include "serialization.h"
#include "dslsyntax_log.h"

typedef struct HighlightTokenCursor {
    Context *context;
    Token *token;
} HighlightTokenCursor;

static int retain_context_buffer(Context *context, char *buffer) {
    char **new_buffers;

    if (!context || !buffer) return 0;

    new_buffers = realloc(context->extra_buffers, sizeof(char *) * (context->extra_buffers_count + 1));
    if (!new_buffers) {
        return 0;
    }

    context->extra_buffers = new_buffers;
    context->extra_buffers[context->extra_buffers_count++] = buffer;
    return 1;
}

static void configure_parser_import_locations(Context *context) {
    char *exe_path;
    char *combined_locations;
    char **import_locations;
    size_t length;
    size_t i;
    size_t index;

    if (!context || context->import_locations) return;

    exe_path = exepath();
    if (!exe_path) return;

    length = strlen(exe_path) + 4;
    combined_locations = malloc(length);
    if (!combined_locations) {
        free(exe_path);
        return;
    }
    snprintf(combined_locations, length, ".;%s", exe_path);
    free(exe_path);

    import_locations = malloc(sizeof(char *) * 3);
    if (!import_locations) {
        free(combined_locations);
        return;
    }

    index = 0;
    import_locations[index++] = combined_locations;
    for (i = 0; combined_locations[i]; i++) {
        if (combined_locations[i] == ';') {
            combined_locations[i] = 0;
            import_locations[index++] = combined_locations + i + 1;
        }
    }
    import_locations[index] = 0;

    if (!retain_context_buffer(context, combined_locations)) {
        free(import_locations);
        free(combined_locations);
        return;
    }

    context->import_locations = import_locations;
}

static CB_NodeType map_c_token_to_cb_type(int token_type) {
    switch (token_type) {
        case TK_UNKNOWN:
        case TK_BADCOMMENT: return LEXER_UNKNOWN;
        case TK_EOC:
        case TK_EOL:
        case TK_EOS: return LEXER_STATEMENT_SEPARATOR;
        case TK_MINUSMINUS: return LEXER_COMMENT;
        case TK_VAR_SYMBOL:
        case TK_CLASS_STEM:
        case TK_STEM:
        case TK_STEMVAR:
        case TK_STEMSTRING:
        case TK_STEMNOVAL:
        case TK_STEMINT: return LEXER_IDENTIFIER;
        case TK_CLASS_TYPE: return LEXER_TYPE_IDENTIFIER;
        case TK_LABEL:
        case TK_MULT_LABEL: return LEXER_FUNCTION_IDENTIFIER;
        case TK_IMPORT:
        case TK_NAMESPACE:
        case TK_OPTIONS: return LEXER_PREPROCESSOR;
        case TK_EXIT_PRIMARY:
        case TK_EXIT_TOKEN: return LEXER_KEYWORD;
        case TK_STRING: return LEXER_STRING_LITERAL;
        case TK_DECIMAL:
        case TK_INTEGER:
        case TK_FLOAT: return LEXER_NUMBER_LITERAL;
        case TK_CONCAT: return LEXER_OPERATOR;
        case TK_PLUS:
        case TK_MINUS:
        case TK_HIGH_PRIORITY_MINUS:
        case TK_MULT:
        case TK_DIV:
        case TK_MOD:
        case TK_IDIV:
        case TK_POWER_L:
        case TK_POWER_R: return LEXER_OPERATOR_ARITHMETIC;
        case TK_EQUAL: return LEXER_OPERATOR_ASSIGN;
        case TK_NEQ:
        case TK_GT:
        case TK_LT:
        case TK_GTE:
        case TK_LTE:
        case TK_S_EQ:
        case TK_S_NEQ:
        case TK_S_GT:
        case TK_S_LT:
        case TK_S_GTE:
        case TK_S_LTE:
        case TK_AND:
        case TK_OR:
        case TK_NOT: return LEXER_OPERATOR_LOGICAL;
        case TK_OPEN_BRACKET:
        case TK_OPEN_SBRACKET: return LEXER_LH_EXPR;
        case TK_CLOSE_BRACKET:
        case TK_CLOSE_SBRACKET: return LEXER_RH_EXPR;
        case TK_COMMA:
        case TK_DOT: return LEXER_SEPARATOR;
        default:
            return LEXER_KEYWORD;
    }
}

static int token_span_utf8(Context *context, Token *token, size_t *pos, size_t *len) {
    size_t byte_offset;

    if (!context || !token || !pos || !len) return 0;
    if (!token->token_string || token->length <= 0) return 0;
    if (token->token_string < context->buff_start) return 0;

    byte_offset = (size_t)(token->token_string - context->buff_start);
    *pos = utf8nlen(context->buff_start, byte_offset);
    *len = utf8nlen(token->token_string, token->length);
    return *len > 0;
}

static int source_node_span_utf8(Context *context, SourceNode *node, size_t *pos, size_t *len) {
    size_t start_offset;
    size_t byte_length;

    if (!context || !node || !pos || !len) return 0;
    if (node->source_start && node->source_end &&
        node->source_start >= context->buff_start &&
        node->source_end >= node->source_start) {
        start_offset = (size_t)(node->source_start - context->buff_start);
        byte_length = (size_t)(node->source_end - node->source_start) + 1;
        *pos = utf8nlen(context->buff_start, start_offset);
        *len = utf8nlen(node->source_start, byte_length);
        return *len > 0;
    }

    if (node->token_start && node->token_end &&
        node->token_start->token_string && node->token_end->token_string &&
        node->token_start->token_string >= context->buff_start &&
        node->token_end->token_string >= node->token_start->token_string) {
        start_offset = (size_t)(node->token_start->token_string - context->buff_start);
        byte_length = (size_t)(node->token_end->token_string - node->token_start->token_string) +
                      (size_t)node->token_end->length;
        *pos = utf8nlen(context->buff_start, start_offset);
        *len = utf8nlen(node->token_start->token_string, byte_length);
        return *len > 0;
    }

    if (token_span_utf8(context, node->token, pos, len)) return 1;
    if (node->parent) return source_node_span_utf8(context, node->parent, pos, len);
    return 0;
}

static int source_diagnostic_span_utf8(Context *context, SourceDiagnostic *diag, size_t *pos, size_t *len) {
    size_t start_offset;
    size_t byte_length;

    if (!context || !diag || !pos || !len) return 0;
    if (diag->source_start && diag->source_end &&
        diag->source_start >= context->buff_start &&
        diag->source_end >= diag->source_start) {
        start_offset = (size_t)(diag->source_start - context->buff_start);
        byte_length = (size_t)(diag->source_end - diag->source_start) + 1;
        *pos = utf8nlen(context->buff_start, start_offset);
        *len = utf8nlen(diag->source_start, byte_length);
        return *len > 0;
    }

    if (diag->owner) return source_node_span_utf8(context, diag->owner, pos, len);
    return 0;
}

static int ast_node_span_utf8(Context *context, ASTNode *node, size_t *pos, size_t *len) {
    size_t start_offset;
    size_t byte_length;

    if (!context || !node || !pos || !len) return 0;
    if (node->source_start && node->source_end &&
        node->source_start >= context->buff_start &&
        node->source_end >= node->source_start) {
        start_offset = (size_t)(node->source_start - context->buff_start);
        byte_length = (size_t)(node->source_end - node->source_start) + 1;
        *pos = utf8nlen(context->buff_start, start_offset);
        *len = utf8nlen(node->source_start, byte_length);
        return *len > 0;
    }

    if (node->token_start && node->token_end &&
        node->token_start->token_string && node->token_end->token_string &&
        node->token_start->token_string >= context->buff_start &&
        node->token_end->token_string >= node->token_start->token_string) {
        start_offset = (size_t)(node->token_start->token_string - context->buff_start);
        byte_length = (size_t)(node->token_end->token_string - node->token_start->token_string) +
                      (size_t)node->token_end->length;
        *pos = utf8nlen(context->buff_start, start_offset);
        *len = utf8nlen(node->token_start->token_string, byte_length);
        return *len > 0;
    }

    if (token_span_utf8(context, node->token, pos, len)) return 1;
    if (node->parent) return ast_node_span_utf8(context, node->parent, pos, len);
    return 0;
}

static int source_container_type(SourceNode *node, CB_NodeType *type) {
    if (!node || !type) return 0;

    switch (node->node_type) {
        case CLASS_DEF:
            *type = PARSE_TREE_STRUCTURE;
            return 1;
        case PROCEDURE:
        case METHOD:
        case FACTORY:
            *type = PARSE_TREE_FUNCTION;
            return 1;
        case INSTRUCTIONS:
        case DO:
        case SELECT:
        case WHEN:
        case OTHERWISE:
            *type = PARSE_TREE_CODEBLOCK;
            return 1;
        case IF:
        case ASSIGN:
        case CALL:
        case RETURN:
        case EXIT:
        case ADDRESS:
        case IMPLICIT_CMD:
        case SAY:
        case PULL:
        case PARSE:
            *type = PARSE_TREE_STATEMENT;
            return 1;
        case BLOCK_EXPR:
        case OP_ADD:
        case OP_MINUS:
        case OP_MULT:
        case OP_DIV:
        case OP_IDIV:
        case OP_MOD:
        case OP_POWER:
        case OP_CONCAT:
        case OP_SCONCAT:
        case OP_AND:
        case OP_OR:
        case OP_COMPARE_EQUAL:
        case OP_COMPARE_NEQ:
        case OP_COMPARE_GT:
        case OP_COMPARE_LT:
        case OP_COMPARE_GTE:
        case OP_COMPARE_LTE:
        case OP_COMPARE_S_EQ:
        case OP_COMPARE_S_NEQ:
        case OP_COMPARE_S_GT:
        case OP_COMPARE_S_LT:
        case OP_COMPARE_S_GTE:
        case OP_COMPARE_S_LTE:
            *type = PARSE_TREE_EXPR;
            return 1;
        default:
            return 0;
    }
}

static void emit_projected_token(CB_ParseTree *tb, Token *token, size_t pos, size_t len) {
    CB_NodeType type;
    int has_embedded_separator;

    if (!tb || !token || len == 0) return;
    has_embedded_separator = token->token_string && token->token_string[0] == '.';

    switch (token->token_type) {
        case TK_STEMVAR:
            if (has_embedded_separator && len > 1) {
                cb_add_child_node(tb, cb_create_node(LEXER_SEPARATOR, pos, 1));
                cb_add_child_node(tb, cb_create_node(LEXER_IDENTIFIER, pos + 1, len - 1));
                return;
            }
            cb_add_child_node(tb, cb_create_node(LEXER_IDENTIFIER, pos, len));
            return;
        case TK_STEMINT:
            if (has_embedded_separator && len > 1) {
                cb_add_child_node(tb, cb_create_node(LEXER_SEPARATOR, pos, 1));
                cb_add_child_node(tb, cb_create_node(LEXER_NUMBER_LITERAL, pos + 1, len - 1));
                return;
            }
            cb_add_child_node(tb, cb_create_node(LEXER_NUMBER_LITERAL, pos, len));
            return;
        case TK_STEMSTRING:
            if (has_embedded_separator && len > 1) {
                cb_add_child_node(tb, cb_create_node(LEXER_SEPARATOR, pos, 1));
                cb_add_child_node(tb, cb_create_node(LEXER_IDENTIFIER, pos + 1, len - 1));
                return;
            }
            cb_add_child_node(tb, cb_create_node(LEXER_IDENTIFIER, pos, len));
            return;
        case TK_STEMNOVAL:
            cb_add_child_node(tb, cb_create_node(LEXER_SEPARATOR, pos, len));
            return;
    }

    type = map_c_token_to_cb_type(token->token_type);
    cb_add_child_node(tb, cb_create_node(type, pos, len));
}

static void emit_tokens_until(CB_ParseTree *tb, HighlightTokenCursor *cursor, size_t limit_pos) {
    size_t pos;
    size_t len;

    while (cursor && cursor->token) {
        if (!token_span_utf8(cursor->context, cursor->token, &pos, &len)) {
            cursor->token = cursor->token->token_next;
            continue;
        }
        if (pos >= limit_pos) break;
        if (pos + len > limit_pos) break;

        emit_projected_token(tb, cursor->token, pos, len);
        cursor->token = cursor->token->token_next;
    }
}

static void emit_source_projection(CB_ParseTree *tb,
                                   Context *context,
                                   SourceNode *node,
                                   HighlightTokenCursor *cursor) {
    SourceNode *child;
    size_t child_pos;
    size_t child_len;
    CB_NodeType child_type;
    CB_Node child_node;

    child = node;
    while (child) {
        if (child->node_type != RXCP_ERROR && child->node_type != RXCP_WARNING &&
            source_container_type(child, &child_type) &&
            source_node_span_utf8(context, child, &child_pos, &child_len)) {
            emit_tokens_until(tb, cursor, child_pos);
            child_node = cb_create_node(child_type, child_pos, child_len);
            cb_add_child_node(tb, child_node);
            cb_set_current_parent_to_last_node(tb);
            emit_source_projection(tb, context, child->child, cursor);
            emit_tokens_until(tb, cursor, child_pos + child_len);
            cb_set_current_parent_to_grandparent(tb);
        } else if (child->child) {
            emit_source_projection(tb, context, child->child, cursor);
        }
        child = child->sibling;
    }
}

static char *format_source_diagnostic_message(SourceDiagnostic *diag) {
    char *message;

    if (!diag) return strdup("Syntax Error");
    if (!diag->is_internal) {
        return strdup(diag->message ? diag->message : "Syntax Error");
    }

    if (diag->severity == SOURCE_DIAG_WARNING) {
        message = mprintf("Internal generated-code warning: %s",
                          diag->message ? diag->message : "Warning");
    } else {
        message = mprintf("Internal generated-code error: %s",
                          diag->message ? diag->message : "Syntax Error");
    }
    return message;
}

static char *format_ast_diagnostic_message(ASTNode *diag) {
    char *message;

    if (!diag) return strdup("Syntax Error");
    if (!diag->is_internal_diagnostic) {
        return strdup(diag->node_string ? diag->node_string : "Syntax Error");
    }

    if (diag->node_type == RXCP_WARNING) {
        message = mprintf("Internal generated-code warning: %s",
                          diag->node_string ? diag->node_string : "Warning");
    } else {
        message = mprintf("Internal generated-code error: %s",
                          diag->node_string ? diag->node_string : "Syntax Error");
    }
    return message;
}

static void emit_diagnostics_from_source_state(CB_ParseTree *tb, Context *context) {
    SourceDiagnostic *diag;
    size_t pos;
    size_t len;
    CB_Node diag_node;

    diag = context ? context->source_diagnostics_list : 0;
    while (diag) {
        if (source_diagnostic_span_utf8(context, diag, &pos, &len)) {
            diag_node = cb_create_node(SYNTAX_ERROR, pos, len);
            diag_node.severity = (diag->severity == SOURCE_DIAG_WARNING) ? CB_WARNING : CB_ERROR;
            diag_node.message = format_source_diagnostic_message(diag);
            cb_set_current_parent_to_root_node(tb);
            cb_add_child_node(tb, diag_node);
        }
        diag = diag->next_in_context;
    }
}

static void emit_diagnostics_from_detached_ast(CB_ParseTree *tb, Context *context, ASTNode *diag) {
    size_t pos;
    size_t len;
    CB_Node diag_node;

    while (diag) {
        if ((diag->node_type == RXCP_ERROR || diag->node_type == RXCP_WARNING) &&
            ast_node_span_utf8(context, diag, &pos, &len)) {
            diag_node = cb_create_node(SYNTAX_ERROR, pos, len);
            diag_node.severity = (diag->node_type == RXCP_WARNING) ? CB_WARNING : CB_ERROR;
            diag_node.message = format_ast_diagnostic_message(diag);
            cb_set_current_parent_to_root_node(tb);
            cb_add_child_node(tb, diag_node);
        }
        diag = diag->sibling;
    }
}

static CB_Node compiler_get_token_callback(void *user_data,
                                           size_t pos,
                                           size_t length,
                                           CodeBufferCharacter* token_chars) {
    HighlightTokenCursor *cursor;
    Token *token;
    size_t token_pos;
    size_t token_len;

    cursor = (HighlightTokenCursor *)user_data;
    token = cursor ? cursor->context->token_head : 0;
    while (token) {
        if (token_span_utf8(cursor->context, token, &token_pos, &token_len) && token_pos == pos) {
            return cb_create_node(map_c_token_to_cb_type(token->token_type), pos, token_len);
        }
        token = token->token_next;
    }

    if (token_chars && length > 0 && token_chars[0].codepoints > 0) {
        switch (token_chars[0].character[0]) {
            case '.':
            case ',':
                return cb_create_node(LEXER_SEPARATOR, pos, 1);
            case '(':
            case '[':
                return cb_create_node(LEXER_LH_EXPR, pos, 1);
            case ')':
            case ']':
                return cb_create_node(LEXER_RH_EXPR, pos, 1);
            case '=':
                return cb_create_node(LEXER_OPERATOR_ASSIGN, pos, 1);
            case '+':
            case '-':
            case '*':
            case '/':
                return cb_create_node(LEXER_OPERATOR_ARITHMETIC, pos, 1);
        }
    }

    return cb_default_get_token_callback(user_data, pos, length, token_chars);
}

static void emit_flat_tokens(CB_ParseTree *tb, Context *context) {
    Token *token;
    size_t pos;
    size_t len;

    token = context->token_head;
    while (token) {
        if (token_span_utf8(context, token, &pos, &len)) {
            cb_add_child_node(tb, cb_create_node(map_c_token_to_cb_type(token->token_type), pos, len));
        }
        token = token->token_next;
    }
}

void rxc_highlight_controller_parse(CodeBuffer *cb) {
    char *source_code;
    size_t source_len;
    Context *context;
    CB_ParseTree *tb;
    CB_Node root_node;
    HighlightTokenCursor cursor;

    if (!cb) return;

    source_code = get_code_buffer_source(cb);
    if (!source_code) return;
    source_len = strlen(source_code);

    context = cntx_f();
    context->master_context = context;
    context->file_name = strdup("dsl_buffer.rexx");
    context->debug_mode = 0;
    context->stop_after_parse = 1;
    context->optimise = 0;
    context->level = LEVELB;

    cntx_buf(context, source_code, strlen(source_code));
    configure_parser_import_locations(context);

    rxcp_init_exits(context);
    rexbpars(context);

    if (!context->ast) {
        rxcp_run_fallback_diagnostics(context);
    } else {
        rxcp_prepare_source_ast(context);
        source_tree_sync_diagnostics(context);
    }

    tb = cb_create_token_buffer();
    root_node = cb_create_node(PARSE_TREE_FILE, 0, source_len);
    cb_add_child_node(tb, root_node);
    cb_set_current_parent_to_root_node(tb);

    if (context->source_tree) {
        cursor.context = context;
        cursor.token = context->token_head;
        emit_source_projection(tb, context, context->source_tree->child, &cursor);
        emit_tokens_until(tb, &cursor, source_len);
        emit_diagnostics_from_source_state(tb, context);
    } else {
        emit_flat_tokens(tb, context);
        emit_diagnostics_from_detached_ast(tb, context, (ASTNode *)context->diagnostics_list);
    }

    cb_order_tree(tb);
    cursor.context = context;
    cursor.token = context->token_head;
    cb_add_missing_tokens(tb, cb, compiler_get_token_callback, &cursor);
    cb_tweak_tree_positions(tb);
    cb_validate_tree(tb);

    if (context->file_name) free(context->file_name);
    fre_cntx(context);
    cb->parse_tree = tb;
}

#endif
