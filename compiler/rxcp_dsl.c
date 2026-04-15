#ifdef ENABLE_PARSER_MODE

#ifndef restrict
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* restrict is a keyword */
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
#include "rxcp_highlight_controller.h"
#include "rxcp_val.h"
#include "rxcpbgmr.h"
#include "rxcp_exit.h"
#include "utf.h"

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

/* Map cREXX token types to DSL platform node types */
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
        case TK_STRING: return LEXER_STRING_LITERAL;
        case TK_DECIMAL:
        case TK_INTEGER:
        case TK_FLOAT: return LEXER_NUMBER_LITERAL;
        case TK_PLUS:
        case TK_MINUS:
        case TK_HIGH_PRIORITY_MINUS:
        case TK_MULT:
        case TK_DIV:
        case TK_MOD:
        case TK_IDIV:
        case TK_POWER_L:
        case TK_POWER_R: return LEXER_OPERATOR_ARITHMETIC;
        case TK_EQUAL:
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
            /* Keywords and builtins */
            return LEXER_KEYWORD;
    }
}

/* AST parsing function for the CodeBuffer event loop */
void rxc_dsl_parser_func(CodeBuffer *cb) {
    if (!cb) return;

    LOG("rxc_dsl_parser_func: Extracting source...");
    char *source_code = get_code_buffer_source(cb);
    if (!source_code) {
        LOG("rxc_dsl_parser_func: source_code is NULL");
        return;
    }

    LOG("rxc_dsl_parser_func: Setting up context...");
    /* Setup compiler context */
    Context *context = cntx_f();
    context->master_context = context;
    context->file_name = strdup("dsl_buffer.rexx");
    context->debug_mode = 0;
    context->stop_after_parse = 1;
    context->optimise = 0;

    /* Currently defaulting to LEVELB for standard cREXX */
    context->level = LEVELB;

    LOG("rxc_dsl_parser_func: Feeding buffer...");
    /* Feed the buffer to context */
    cntx_buf(context, source_code, strlen(source_code));

    LOG("rxc_dsl_parser_func: Running parsing and validation...");
    /* Initialize and Parse */
    rxcp_init_exits(context);
    rexbpars(context);

    if (context->ast) {
        LOG("rxc_dsl_parser_func: Running rxcp_val...");
        /* Run validation pass to link symbols, infer types and report structural issues */
        rxcp_val(context);
    }

    LOG("rxc_dsl_parser_func: Building token buffer...");
    /* Build Token Buffer */
    CB_ParseTree *tb = cb_create_token_buffer();

    LOG("rxc_dsl_parser_func: Iterating tokens...");
    /* 1. Iterate linear token stream and push to token buffer */
    Token *t = context->token_head;
    size_t last_end = 0;
    while (t) {
        /* Filter out synthetic or empty tokens */
        if (t->token_string && t->length > 0 && t->token_string >= context->buff_start) {
            size_t byte_offset = (size_t)(t->token_string - context->buff_start);
            size_t pos = utf8nlen(context->buff_start, byte_offset);
            size_t len = utf8nlen(t->token_string, t->length);
            
            if (pos >= last_end) {
                CB_NodeType type = map_c_token_to_cb_type(t->token_type);
                cb_set_current_parent_to_root_node(tb);
                CB_Node token_node = cb_create_node(type, pos, len);
                cb_add_child_node(tb, token_node);
                last_end = pos + len;
            }
        }
        t = t->token_next;
    }

    LOG("rxc_dsl_parser_func: Processing diagnostics...");
    /* 2. Attach Diagnostics */
    ASTNode *diag = (ASTNode*)context->diagnostics_list;
    while (diag) {
        if (!diag->is_duplicate_warning) {
            size_t pos = 0;
            size_t len = 1;
            
            if (diag->token_start && diag->token_start->token_string >= context->buff_start) {
                size_t byte_offset = (size_t)(diag->token_start->token_string - context->buff_start);
                pos = utf8nlen(context->buff_start, byte_offset);
                if (diag->token_end && diag->token_end->token_string >= diag->token_start->token_string) {
                    size_t end_byte_offset = (size_t)(diag->token_end->token_string - context->buff_start);
                    size_t end_pos = utf8nlen(context->buff_start, end_byte_offset);
                    size_t end_len = utf8nlen(diag->token_end->token_string, diag->token_end->length);
                    len = (end_pos - pos) + end_len;
                } else {
                    len = utf8nlen(diag->token_start->token_string, diag->token_start->length);
                }
            } else if (diag->token && diag->token->token_string >= context->buff_start) {
                size_t byte_offset = (size_t)(diag->token->token_string - context->buff_start);
                pos = utf8nlen(context->buff_start, byte_offset);
                len = utf8nlen(diag->token->token_string, diag->token->length);
            }

            cb_set_current_parent_to_root_node(tb);
            CB_Node diag_node = cb_create_node(LEXER_UNKNOWN, pos, len);
            
            /* Error message text is stored in node_string for errors/warnings */
            if (diag->node_string) {
                diag_node.message = strdup(diag->node_string);
            } else {
                diag_node.message = strdup("Syntax Error");
            }
            
            diag_node.severity = (diag->node_type == RXCP_WARNING) ? CB_WARNING : CB_ERROR;
            cb_add_child_node(tb, diag_node);
        }
        diag = diag->sibling;
    }

    LOG("rxc_dsl_parser_func: Finalizing tree...");
    /* Order and finalize tree */
    cb_order_tree(tb);
    
    LOG("rxc_dsl_parser_func: Adding missing tokens...");
    /* Fill gaps */
    cb_add_missing_tokens(tb, cb, cb_default_get_token_callback, NULL);
    
    LOG("rxc_dsl_parser_func: Tweaking positions and validating...");
    cb_tweak_tree_positions(tb);
    cb_validate_tree(tb);

    LOG("rxc_dsl_parser_func: Cleaning up...");
    /* Clean up compiler context */
    if (context->file_name) free(context->file_name);
    fre_cntx(context);

    /* Assign the parsed token tree back to the CodeBuffer */
    cb->parse_tree = tb;
}

int rxc_parser_mode_main(int stdio_mode, int port, const char *file_name, int debug_mode) {
    if (debug_mode) {
        cb_log_init("rxc_parser.log");
        LOG("cREXX Parser Server starting... (debug=%d)", debug_mode);
    }

    const char *crexx_config = 
        "[.rexx]\n"
        "keywords=say,if,then,else,do,end,procedure,expose,return,exit,pull,parse,arg\n"
        "operators=+,-,*,/,=,<,>,(,),{,},,,;\n"
        "line_comment=--\n"
        "block_start=/*\n"
        "block_end=*/\n"
        "quotes=\"\n";
    cb_set_ep_config_string(crexx_config);

    CodeBuffer *cb = create_code_buffer(NULL, rxc_highlight_controller_parse);
    if (!cb) {
        if (debug_mode) LOG("Failed to create CodeBuffer.");
        return 1;
    }

    if (stdio_mode) {
        if (debug_mode) LOG("Starting stdio server...");
        cb_start_stdio_server(cb);
    } else {
        if (debug_mode) LOG("Starting socket server on port %d...", port);
        cb_start_server(cb, "127.0.0.1", port);
    }

    free_code_buffer(cb);
    if (debug_mode) cb_log_close();

    return 0;
}

#endif
