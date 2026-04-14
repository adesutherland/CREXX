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

#include "rxas.h"
#include "rxasassm.h"
#include "rxdefs.h"

#include "dslsyntax_common.h"
#include "dslsyntax_parser.h"
#include "serialization.h"
#include "dslsyntax_log.h"

#include <ctype.h>

extern const OpInfo op_table[];

static int is_mnemonic(const char *s) {
    int i;
    if (!s) return 0;
    for (i = 0; op_table[i].mnemonic != NULL; i++) {
        const char *m = op_table[i].mnemonic;
        int j = 0;
        while (s[j] && m[j] && toupper((unsigned char)s[j]) == m[j]) j++;
        if (s[j] == 0 && (m[j] == 0 || m[j] == '_')) return 1;
    }
    return 0;
}

static CB_NodeType map_c_token_to_cb_type(Assembler_Token *t) {
    if (!t) return LEXER_UNKNOWN;
    switch (t->token_type) {
        case ERROR: return LEXER_UNKNOWN;
        case ANYTHING: return LEXER_UNKNOWN;
        case EOS:
        case NEWLINE: return LEXER_STATEMENT_SEPARATOR;
        case KW_GLOBALS:
        case KW_EXPOSE:
        case KW_META:
        case KW_CLASS:
        case KW_ATTR:
        case KW_LOCALS:
        case KW_SRCFILE:
        case KW_SRC: return LEXER_PREPROCESSOR; // Directives -> Magenta
        case EQUAL: return LEXER_OPERATOR_ASSIGN;
        case COLON: return LEXER_OPERATOR;
        case COMMA: return LEXER_SEPARATOR;
        case INT:
        case FLOAT:
        case HEX:
        case DECIMAL: return LEXER_NUMBER_LITERAL; // Literals -> Yellow
        case STRING:
        case CHAR: return LEXER_STRING_LITERAL;
        case GREG:
        case AREG:
        case RREG: return LEXER_CONSTANT_IDENTIFIER; // Registers -> Green
        case ID:
            {
                char buf[256];
                int len = (int)t->length;
                if (len >= (int)sizeof(buf)) len = sizeof(buf) - 1;
                strncpy(buf, (const char*)t->token_source, (size_t)len);
                buf[len] = '\0';
                if (is_mnemonic(buf)) return LEXER_KEYWORD;      // Mnemonics -> Blue
                else return LEXER_IDENTIFIER;                    // Label refs -> Light Blue
            }
        case FUNC: 
        case LABEL: return LEXER_IDENTIFIER; // Labels Definitions -> Light Blue
        default:
            return LEXER_KEYWORD;
    }
}

void rxas_dsl_parser_func(CodeBuffer *cb) {
    if (!cb) return;

    cb_log("rxas_dsl_parser_func: Extracting source...\n");
    char *source_code = get_code_buffer_source(cb);
    if (!source_code) {
        cb_log("rxas_dsl_parser_func: source_code is NULL\n");
        return;
    }
    size_t source_len = strlen(source_code);
    cb_log("rxas_dsl_parser_func: Source extracted, length=%zu\n", source_len);

    cb_log("rxas_dsl_parser_func: Setting up context...\n");
    Assembler_Context scanner;
    memset(&scanner, 0, sizeof(Assembler_Context));

    scanner.debug_mode = 0;
    scanner.quiet = 1;
    scanner.traceFile = 0;
    scanner.optimise = 0;
    scanner.file_name = "dsl_buffer.rxas";
    scanner.output_file_name = "dsl_buffer.rxbin";
    scanner.location = ".";
    
    scanner.buff = source_code;
    scanner.buff_end = source_code + source_len;
    
    cb_log("rxas_dsl_parser_func: Init buff... (buff=%p, end=%p)\n", scanner.buff, scanner.buff_end);
    if (rxasinbf(&scanner) != 0) {
        cb_log("rxas_dsl_parser_func: rxasinbf failed\n");
        // Still build a dummy tree so we don't crash
    }
    scanner.top = scanner.buff;
    scanner.cursor = scanner.buff;
    scanner.marker = scanner.buff;
    scanner.ctxmarker = scanner.buff;
    scanner.linestart = scanner.buff;
    scanner.line = 1;
    scanner.token_counter = 0;

    if (!scanner.parser) {
        cb_log("rxas_dsl_parser_func: Failed to allocate parser in rxasinbf\n");
        CB_ParseTree *tb = cb_create_token_buffer();
        CB_Node root_node = cb_create_node(PARSE_TREE_FILE, 0, source_len);
        cb_add_child_node(tb, root_node);
        cb_set_current_parent_to_root_node(tb);
        cb->parse_tree = tb;
        return;
    }

    cb_log("rxas_dsl_parser_func: Running parsing...\n");
    rxaspars(&scanner);

    cb_log("rxas_dsl_parser_func: Building token buffer...\n");
    CB_ParseTree *tb = cb_create_token_buffer();
    
    // Create root node
    CB_Node root_node = cb_create_node(PARSE_TREE_FILE, 0, source_len);
    cb_add_child_node(tb, root_node);
    cb_set_current_parent_to_root_node(tb);

        if (tb->root == NULL) {
        cb_log("rxas_dsl_parser_func: tb->root is NULL after adding root_node!\n");
    }

    cb_log("rxas_dsl_parser_func: Iterating tokens...\n");
    Assembler_Token *t = scanner.token_head;
    while (t) {
        if (t->token_source && t->length > 0 && t->token_source >= scanner.buff) {
            size_t pos = (size_t)(t->token_source - scanner.buff);
            size_t len = t->length;
            
            if (pos >= source_len) {
                cb_log("rxas_dsl_parser_func: WARNING: Ignoring out of bounds token! pos=%zu, len=%zu, source_len=%zu\n", pos, len, source_len);
                t = t->token_next;
                continue;
            }
            
            if (pos + len > source_len) {
                cb_log("rxas_dsl_parser_func: WARNING: Truncating out of bounds token! pos=%zu, len=%zu, source_len=%zu\n", pos, len, source_len);
                len = source_len - pos;
            }

            CB_NodeType type = map_c_token_to_cb_type(t);
            CB_Node token_node = cb_create_node(type, pos, len);
            cb_add_child_node(tb, token_node);
        }
        t = t->token_next;
    }

    cb_log("rxas_dsl_parser_func: Processing diagnostics...\n");
    /* First, setup next_error pointers correctly (rxasperr style) */
    Assembler_Error *e = scanner.error_tail;
    Assembler_Error *p;
    while (e) {
        p = e->prev_error;
        if (p) {
            p->next_error = e;
            e = p;
        }
        else break;
    }
    /* e is now the first error (head) */

    while (e) {
        cb_log("rxas_dsl_parser_func: Diagnostic: %d:%d - %s (sev=%d)\n", 
               e->line, e->column, e->message, e->severity);
        
        /* Find absolute position */
        char *line_ptr = scanner.buff;
        int current_line = 1;
        while (current_line < e->line && line_ptr < scanner.buff_end) {
            char *next_nl = strchr(line_ptr, '\n');
            if (next_nl) {
                line_ptr = next_nl + 1;
                current_line++;
            } else {
                break;
            }
        }
        
        size_t pos = 0;
        size_t len = 1; // Default
        if (current_line == e->line) {
            pos = (size_t)(line_ptr - scanner.buff) + (e->column > 0 ? e->column - 1 : 0);
            
            /* Robustness: ensure pos is within bounds */
            if (pos >= source_len) pos = (source_len > 0) ? source_len - 1 : 0;

            /* Try to find a token that covers this position to get a better length */
            Assembler_Token *tok = scanner.token_head;
            while (tok) {
                if (tok->token_source && tok->length > 0) {
                    size_t t_pos = (size_t)(tok->token_source - scanner.buff);
                    if (pos >= t_pos && pos < t_pos + tok->length) {
                        /* Error is within this token, use its full range */
                        pos = t_pos;
                        len = tok->length;
                        break;
                    }
                }
                tok = tok->token_next;
            }
        }

        CB_Node diag_node = cb_create_node(SYNTAX_ERROR, pos, len);
        /* Map rxas severity (usually 1 for error) to middleware severity */
        diag_node.severity = (e->severity >= 1) ? CB_ERROR : CB_INFORMATION;
        diag_node.message = strdup(e->message);
        cb_add_child_node(tb, diag_node);
        
        e = e->next_error;
    }

    cb_log("rxas_dsl_parser_func: Finalizing tree...\n");
    cb_set_current_parent_to_root_node(tb);
    cb_order_tree(tb);
    
    cb_log("rxas_dsl_parser_func: Adding missing tokens...\n");
    cb_add_missing_tokens(tb, cb, cb_default_get_token_callback, NULL);
    
    cb_log("rxas_dsl_parser_func: Tweaking positions and validating...\n");
    cb_tweak_tree_positions(tb);
    cb_validate_tree(tb);

    cb_log("rxas_dsl_parser_func: Cleaning up...\n");
    scanner.buff = NULL; // Prevent rxasclrc from freeing source_code
    rxasclrc(&scanner);
    free(source_code); // We must free it here!

    cb_log("rxas_dsl_parser_func: AST before return:\n");
    // cb_print_token_buffer(cb, tb); // REMOVED: Corrupts stdio protocol by printing to stdout

    cb->parse_tree = tb;
}

int rxas_parser_mode_main(int stdio_mode, int port, const char *file_name, int debug_mode) {
    if (debug_mode || stdio_mode) {
        cb_log_init("rxas_parser.log");
        cb_log("cREXX Assembler Server starting... (debug=%d, stdio=%d)\n", debug_mode, stdio_mode);
    }

    const char *rxas_config = 
        "[.rxas]\n"
        "keywords=.globals,.expose,.meta,.class,.attr,.locals,.srcfile,.src\n"
        "operators=:\n"
        "line_comment=*\n"
        "quotes=\"\n";
    cb_set_ep_config_string(rxas_config);

    CodeBuffer *cb = create_code_buffer(NULL, rxas_dsl_parser_func);
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
