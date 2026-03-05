/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Shared Compiler Utilities
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "rxcp_util.h"
#include "rxcp_token.h"
#include "rxcpbgmr.h"
#include "rxcp_ast.h"
#include "rxcp_sym.h"
#include "rxcp_ctx.h"
#include "rxcp_val.h"

void error_and_exit(int rc, char* message) {
    fprintf(stderr, "ERROR: %s - try \"rxc -h\"\n", message);
    exit(rc);
}

char* mprintf(const char* format, ...) {
    char *buffer;
    size_t buffer_len;
    size_t needed_len;
    va_list argptr;

    /* Guess a length which is likely to be big enough */
    buffer_len = 100; /* A stab in the dark! */
    buffer = malloc(buffer_len);

    va_start(argptr, format);
    needed_len = vsnprintf(buffer, buffer_len, format, argptr) + 1;
    va_end(argptr);
    if (needed_len > buffer_len) {
        /* Buffer not big enough - do it again */
        buffer_len = needed_len;
        free(buffer);
        buffer = malloc(buffer_len);
        va_start(argptr, format);
        vsnprintf(buffer, buffer_len, format, argptr);
        va_end(argptr);
    }
    return buffer;
}

/* Collect and Prune diagnostics walker */
static walker_result collect_diagnostics_walker(walker_direction direction,
                                                ASTNode* node,
                                                void *payload) {
    Context *context = (Context*)payload;
    if (direction == in) {
        ASTNode *child = node->child;
        ASTNode *prev = NULL;
        while (child) {
            if (child->node_type == ERROR || child->node_type == WARNING) {
                ASTNode *diagnostic = child;

                /* Detach from AST */
                if (prev) prev->sibling = diagnostic->sibling;
                else node->child = diagnostic->sibling;

                /* Add to Side List (lifo) */
                diagnostic->sibling = (ASTNode*)context->diagnostics_list;
                context->diagnostics_list = diagnostic;

                /* Continue with next child (don't update prev) */
                child = prev ? prev->sibling : node->child;
            } else {
                prev = child;
                child = child->sibling;
            }
        }
    }
    return result_normal;
}

/* Public API to prune diagnostics from AST and store in side list for safe emission */
void rxcp_collect_and_prune_diagnostics(Context *context) {
    if (context->ast) {
        ast_wlkr(context->ast, collect_diagnostics_walker, context);
    }
}

#define ADD_CHAR_TO_BUFFER(ch) {out_len++; if (buffer_len) { *(buffer++) = (ch); buffer_len--; }}

static size_t encode_print(char* buffer, size_t buffer_len, const char* string, size_t length) {
    size_t out_len = 0;
    if (!length) {
        if (buffer_len) *buffer = 0;
        return 0;
    }
    while (length) {
        switch (*string) {
            case '\\':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\\')
                break;
            case '\n':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('n')
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('t')
                break;
            case '\a':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('a')
                break;
            case '\b':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('b')
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('f')
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('r')
                break;
            case '\v':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('v')
                break;
            case '\'':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\'')
                break;
            case '\"':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\"')
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('0')
                break;
            case '\?':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('?')
                break;
            default:
                ADD_CHAR_TO_BUFFER(*string)
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    else if (out_len > 0) *(buffer - 1) = 0;
    return out_len;
}

char* encdstrg(const char* string, size_t length) {
    char *buffer;
    size_t buffer_len;
    size_t needed_len;

    /* Guess a length which is likely to be big enough */
    buffer_len = (length * 2) + 1;
    buffer = malloc(buffer_len);

    needed_len = encode_print(buffer, buffer_len, string, length) + 1;
    if (needed_len > buffer_len) {
        /* Buffer not big enough - do it again */
        buffer_len = needed_len;
        free(buffer);
        buffer = malloc(buffer_len);
        encode_print(buffer, buffer_len, string, length);
    }
    return buffer;
}

static size_t encode_comment(char* buffer, size_t buffer_len, const char* string, size_t length) {
    size_t out_len = 0;
    while (length) {
        switch (*string) {
            case '\n':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('n')
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('t')
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('f')
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('r')
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('0')
                break;
            default:
                ADD_CHAR_TO_BUFFER(*string)
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    else if (out_len > 0) *(buffer - 1) = 0;
    return out_len;
}

char* encode_comment_malloc(const char* string, size_t length) {
    char *buffer;
    size_t buffer_len;
    size_t needed_len;

    /* Guess a length which is likely to be big enough */
    buffer_len = (length * 2) + 1;
    buffer = malloc(buffer_len);

    needed_len = encode_comment(buffer, buffer_len, string, length) + 1;
    if (needed_len > buffer_len) {
        /* Buffer not big enough - do it again */
        buffer_len = needed_len;
        free(buffer);
        buffer = malloc(buffer_len);
        encode_comment(buffer, buffer_len, string, length);
    }
    return buffer;
}

static size_t encode_line_source(char* buffer, size_t buffer_len, const char* string, size_t length) {
    size_t out_len = 0;
    while (length) {
        switch (*string) {
            case '\n':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('n')
                break;
            case '\"':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\"')
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('t')
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('f')
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('r')
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('0')
                break;
            default:
                ADD_CHAR_TO_BUFFER(*string)
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    else if (out_len > 0) *(buffer - 1) = 0;
    return out_len;
}

char* encode_line_source_malloc(const char* string, size_t length) {
    char *buffer;
    size_t buffer_len;
    size_t needed_len;

    /* Guess a length which is likely to be big enough */
    buffer_len = (length * 2) + 1;
    buffer = malloc(buffer_len);

    needed_len = encode_line_source(buffer, buffer_len, string, length) + 1;
    if (needed_len > buffer_len) {
        /* Buffer not big enough - do it again */
        buffer_len = needed_len;
        free(buffer);
        buffer = malloc(buffer_len);
        encode_line_source(buffer, buffer_len, string, length);
    }
    return buffer;
}

#undef ADD_CHAR_TO_BUFFER

int tokenis(Token *token, const char* value) {
    char text_buffer[15];
    int val;
    int i;
    if (!token || !token->token_string || !value) return 0;

    val = (int)strlen(value);
    if (val > 14) val = 14;
    strncpy(text_buffer, token->token_string, val);
    // lowercase the buffer
    for (i = 0; i < val; i++) {
        text_buffer[i] = (char)tolower(text_buffer[i]);
    }
    text_buffer[val] = 0;

    if (strcmp(text_buffer, value) == 0) return 1;
    return 0;
}

int hexchar2int(char hexbyte) {
    int val = -1;

    // transform hex character to the 4bit equivalent number
    if (hexbyte >= '0' && hexbyte <= '9') val = hexbyte - '0';
    else if (hexbyte >= 'a' && hexbyte <='f') val = hexbyte - 'a' + 10;
    else if (hexbyte >= 'A' && hexbyte <='F') val = hexbyte - 'A' + 10;

    return val;
}

int binchar2int(const char* bin) {
    int result = 0;

    if (bin[0] == '1') result += 8;
    if (bin[1] == '1') result += 4;
    if (bin[2] == '1') result += 2;
    if (bin[3] == '1') result++;

    return result;
}

char* escape_character(unsigned char c) {
    static char buffer[5];
    buffer[0] = '\\';
    buffer[2] = 0;
    /* Encode C style */
    switch (c) {
        case '\\':
            buffer[1] = '\\';
            break;
        case '\n':
            buffer[1] = 'n';
            break;
        case '\t':
            buffer[1] = 't';
            break;
        case '\a':
            buffer[1] = 'a';
            break;
        case '\b':
            buffer[1] = 'b';
            break;
        case '\f':
            buffer[1] = 'f';
            break;
        case '\r':
            buffer[1] = 'r';
            break;
        case '\v':
            buffer[1] = 'v';
            break;
        case '\'':
            buffer[1] = '\'';
            break;
        case '\"':
            buffer[1] = '\"';
            break;
        case 0:
            buffer[1] = '0';
            break;
        case '\?':
            buffer[1] = '\?';
            break;
        default:
            /* Should we escape this character? */
            if (isprint(c)) {
                buffer[0] = (char)c;
                buffer[1] = 0;
            } else {
                /* Escape as a hex character */
                buffer[1] = 'x';
                snprintf(buffer + 2, 3, "%02x", c);
            }
            break;
    }
    return buffer;
}

void prt_unex(FILE* output, const char *ptr, int len) {
    int i;
    if (!ptr) return;
    for (i = 0; i < len; i++, ptr++) {
        switch (*ptr) {
            case '\0':
                fprintf(output, "\\0");
                break;
            case '\a':
                fprintf(output, "\\a");
                break;
            case '\b':
                fprintf(output, "\\b");
                break;
            case '\f':
                fprintf(output, "\\f");
                break;
            case '\n':
                fprintf(output, "\\n");
                break;
            case '\r':
                fprintf(output, "\\r");
                break;
            case '\t':
                fprintf(output, "\\t");
                break;
            case '\v':
                fprintf(output, "\\v");
                break;
            case '\\':
                fprintf(output, "\\\\");
                break;
            case '\?':
                fprintf(output, "\\?");
                break;
            case '\'':
                fprintf(output, "\\'");
                break;
            case '\"':
                fprintf(output, "\\\"");
                break;
            default:
                /* Should we escape this character? */
                if (isprint((unsigned char)*ptr)) {
                    fprintf(output, "%c", *ptr);
                }
                else {
                    /* Escape as a hex character */
                    fprintf(output, "\\x%02x", (unsigned char)*ptr);
                }
        }
    }
}

const char* token_to_string(int token_id) {
    switch (token_id) {
        case TK_UNKNOWN: return "TK_UNKNOWN";
        case TK_BADCOMMENT: return "TK_BADCOMMENT";
        case TK_EOL: return "TK_EOL";
        case TK_MINUSMINUS: return "TK_MINUSMINUS";
        case TK_EOS: return "TK_EOS";
        case TK_EOC: return "TK_EOC";
        case TK_VAR_SYMBOL: return "TK_VAR_SYMBOL";
        case TK_INTEGER: return "TK_INTEGER";
        case TK_FLOAT: return "TK_FLOAT";
        case TK_DECIMAL: return "TK_DECIMAL";
        case TK_STRING: return "TK_STRING";
        case TK_PLUS: return "TK_PLUS";
        case TK_MINUS: return "TK_MINUS";
        case TK_MULT: return "TK_MULT";
        case TK_DIV: return "TK_DIV";
        case TK_IDIV: return "TK_IDIV";
        case TK_MOD: return "TK_MOD";
        case TK_POWER_L: return "TK_POWER_L";
        case TK_POWER_R: return "TK_POWER_R";
        case TK_CONCAT: return "TK_CONCAT";
        case TK_AND: return "TK_AND";
        case TK_OR: return "TK_OR";
        case TK_NOT: return "TK_NOT";
        case TK_EQUAL: return "TK_EQUAL";
        case TK_NEQ: return "TK_NEQ";
        case TK_GT: return "TK_GT";
        case TK_LT: return "TK_LT";
        case TK_GTE: return "TK_GTE";
        case TK_LTE: return "TK_LTE";
        case TK_S_EQ: return "TK_S_EQ";
        case TK_S_NEQ: return "TK_S_NEQ";
        case TK_S_GT: return "TK_S_GT";
        case TK_S_LT: return "TK_S_LT";
        case TK_S_GTE: return "TK_S_GTE";
        case TK_S_LTE: return "TK_S_LTE";
        case TK_IF: return "TK_IF";
        case TK_THEN: return "TK_THEN";
        case TK_ELSE: return "TK_ELSE";
        case TK_DO: return "TK_DO";
        case TK_END: return "TK_END";
        case TK_SELECT: return "TK_SELECT";
        case TK_WHEN: return "TK_WHEN";
        case TK_OTHERWISE: return "TK_OTHERWISE";
        case TK_LOOP: return "TK_LOOP";
        case TK_WHILE: return "TK_WHILE";
        case TK_UNTIL: return "TK_UNTIL";
        case TK_FOR: return "TK_FOR";
        case TK_BY: return "TK_BY";
        case TK_TO: return "TK_TO";
        case TK_LEAVE: return "TK_LEAVE";
        case TK_ITERATE: return "TK_ITERATE";
        case TK_SAY: return "TK_SAY";
        case TK_RETURN: return "TK_RETURN";
        case TK_EXIT: return "TK_EXIT";
        case TK_PROCEDURE: return "TK_PROCEDURE";
        case TK_EXPOSE: return "TK_EXPOSE";
        case TK_CALL: return "TK_CALL";
        case TK_OPTIONS: return "TK_OPTIONS";
        case TK_NAMESPACE: return "TK_NAMESPACE";
        case TK_IMPORT: return "TK_IMPORT";
        case TK_CLASS_TYPE: return "TK_CLASS_TYPE";
        case TK_OPEN_BRACKET: return "TK_OPEN_BRACKET";
        case TK_CLOSE_BRACKET: return "TK_CLOSE_BRACKET";
        case TK_OPEN_SBRACKET: return "TK_OPEN_SBRACKET";
        case TK_CLOSE_SBRACKET: return "TK_CLOSE_SBRACKET";
        case TK_COMMA: return "TK_COMMA";
        case TK_STEM: return "TK_STEM";
        case TK_ARG_STEM: return "TK_ARG_STEM";
        case TK_CLASS_STEM: return "TK_CLASS_STEM";
        case TK_STEMVAR: return "TK_STEMVAR";
        case TK_STEMINT: return "TK_STEMINT";
        case TK_STEMSTRING: return "TK_STEMSTRING";
        case TK_STEMNOVAL: return "TK_STEMNOVAL";
        case TK_CLASS: return "TK_CLASS";
        case TK_FACTORY: return "TK_FACTORY";
        case TK_METHOD: return "TK_METHOD";
        case TK_WITH: return "TK_WITH";
        case TK_REGISTER: return "TK_REGISTER";
        case TK_OF: return "TK_OF";
        case TK_MULT_LABEL: return "TK_MULT_LABEL";
        case TK_DOT: return "TK_DOT";
        case TK_EXIT_PRIMARY: return "TK_EXIT_PRIMARY";
        case TK_EXIT_TOKEN: return "TK_EXIT_TOKEN";
        case TK_ARG: return "TK_ARG";
        case TK_ADDRESS: return "TK_ADDRESS";
        case TK_OUTPUT: return "TK_OUTPUT";
        case TK_ERROR: return "TK_ERROR";
        case TK_INPUT: return "TK_INPUT";
        case TK_ASSEMBLER: return "TK_ASSEMBLER";
        case TK_VOID: return "TK_VOID";
        case TK_ELLIPSIS: return "TK_ELLIPSIS";
        case TK_OPTIONAL: return "TK_OPTIONAL";
        case TK_NUMERIC: return "TK_NUMERIC";
        case EXIT_OWNED: return "EXIT_OWNED";
        default: return "UNKNOWN";
    }
}

ASTNode* ast_fndn(Context* ctx, ASTNode* node, NodeType type) {
    if (!node) return NULL;
    if (node->node_type == type) return node;
    ASTNode* found = ast_fndn(ctx, node->child, type);
    if (found) return found;
    return ast_fndn(ctx, node->sibling, type);
}

typedef struct SourceMapEntry {
    size_t start;
    size_t end;
    int line;
    int column;
} SourceMapEntry;

typedef struct SourceMap {
    SourceMapEntry *entries;
    size_t count;
    size_t capacity;
} SourceMap;

static void add_source_map_entry(SourceMap *map, size_t start, size_t end, int line, int column) {
    if (map->count == map->capacity) {
        map->capacity = map->capacity == 0 ? 8 : map->capacity * 2;
        map->entries = realloc(map->entries, sizeof(SourceMapEntry) * map->capacity);
    }
    map->entries[map->count].start = start;
    map->entries[map->count].end = end;
    map->entries[map->count].line = line;
    map->entries[map->count].column = column;
    map->count++;
}

static walker_result fragment_fixup_walker(walker_direction direction, ASTNode *node, void *payload) {
    SourceMap *map = (SourceMap *)payload;
    if (direction == in) {
        if (node->node_string && !node->free_node_string) {
            char *s = malloc(node->node_string_length + 1);
            memcpy(s, node->node_string, node->node_string_length);
            s[node->node_string_length] = 0;
            node->node_string = s;
            node->free_node_string = 1;
        }
        /* Update line/column from map if possible */
        if (map && node->token) {
            size_t pos = node->token->token_string - node->context->buff_start;
            size_t i;
            for (i = 0; i < map->count; i++) {
                if (pos >= map->entries[i].start && pos < map->entries[i].end) {
                    node->line = map->entries[i].line;
                    node->column = map->entries[i].column;
                    if (node->token) {
                        node->token->line = map->entries[i].line;
                        node->token->column = map->entries[i].column;
                    }
                    break;
                }
            }
        }
    }
    return result_normal;
}

int ast_grft_interpolated(Context *ctx, ASTNode *target_node, const char *rexx_code, ASTNode **node_map, size_t num_tokens) {
    SourceMap map = {NULL, 0, 0};
    size_t code_len = strlen(rexx_code);
    char *interpolated = malloc(code_len * 2 + 1024); /* Conservative buffer */
    size_t int_pos = 0;
    size_t i = 0;

    /* Add "options levelb\n" offset */
    const char* prefix = "options levelb\n";
    size_t prefix_len = strlen(prefix);
    strcpy(interpolated, prefix);
    int_pos = prefix_len;

    while (i < code_len) {
        if (rexx_code[i] == '{' && isdigit(rexx_code[i+1])) {
            size_t start = i;
            i++;
            int n = 0;
            while (isdigit(rexx_code[i])) {
                n = n * 10 + (rexx_code[i] - '0');
                i++;
            }
            if (rexx_code[i] == '}' && n > 0 && (size_t)n <= num_tokens && node_map[n-1]) {
                i++;
                ASTNode *token_node = node_map[n-1];
                /* Get raw text of the token */
                const char *text = NULL;
                size_t text_len = 0;
                if (token_node->token) {
                    text = token_node->token->token_string;
                    text_len = token_node->token->length;
                } else if (token_node->node_string) {
                    text = token_node->node_string;
                    text_len = token_node->node_string_length;
                }

                if (text) {
                    add_source_map_entry(&map, int_pos, int_pos + text_len, token_node->line, token_node->column);
                    memcpy(interpolated + int_pos, text, text_len);
                    int_pos += text_len;
                }
            } else {
                /* Not a valid {n} or n out of range, copy literally */
                interpolated[int_pos++] = '{';
                i = start + 1;
            }
        } else {
            interpolated[int_pos++] = rexx_code[i++];
        }
    }
    interpolated[int_pos] = 0;

    Context* frag = cntx_f();
    if (!frag) {
        free(interpolated);
        if (map.entries) free(map.entries);
        return -1;
    }

    frag->in_exit_bridge = 1;
    frag->master_context = ctx->master_context;
    frag->location = ctx->location;
    frag->file_name = "exit_fragment";
    frag->level = ctx->level;
    frag->debug_mode = ctx->debug_mode;
    frag->disable_exits = ctx->disable_exits;
    frag->floats_decimal = ctx->floats_decimal;
    frag->floats_binary = ctx->floats_binary;
    frag->numeric_standard = ctx->numeric_standard;
    frag->comments_hash = ctx->comments_hash;
    frag->comments_slash = ctx->comments_slash;
    frag->comments_dash = ctx->comments_dash;

    if (ctx->debug_mode >= 2) fprintf(stderr, "DEBUG_GRFT: Parsing replacement code: %s\n", interpolated);
    cntx_buf(frag, interpolated, int_pos);
    if (rexbpars(frag)) {
        if (ctx->debug_mode >= 2) fprintf(stderr, "DEBUG_GRFT: Parsing FAILED\n");
        mknd_err(target_node, "EXIT_BRIDGE_PARSE_FAILED");
        fre_cntx(frag);
        if (map.entries) free(map.entries);
        return -1;
    }

    /* Normalise fragment */
    ast_wlkr(frag->ast, ast_structure_fixup_walker, (void *) frag);
    ast_wlkr(frag->ast, source_location_walker, (void *) frag);
    ast_wlkr(frag->ast, syntax_validation_walker, (void *) frag);
    ast_wlkr(frag->ast, rxcp_fixup_walker, (void *) frag);

    if (frag->floats_decimal)  {
        ast_wlkr(frag->ast, float2decimal_walker, (void *) frag);
    } else if (frag->floats_binary) {
        ast_wlkr(frag->ast, decimal2float_walker, (void *) frag);
    }

    /* Fix up fragment: copy strings and apply source map */
    ast_wlkr(frag->ast, fragment_fixup_walker, &map);

    if (frag->ast) {
        ASTNode *search = ast_fndn(ctx, frag->ast, INSTRUCTIONS);
        if (search && search->child) {
            ASTNode *exit_owned = ast_f(ctx, EXIT_OWNED, target_node->token);
            exit_owned->parent = target_node->parent;
            exit_owned->scope = NULL;

            if (target_node->parent) {
                if (target_node->parent->child == target_node) target_node->parent->child = exit_owned;
                else {
                    ASTNode *p = target_node->parent->child;
                    while (p && p->sibling != target_node) p = p->sibling;
                    if (p) p->sibling = exit_owned;
                }
            }
            exit_owned->sibling = target_node->sibling;

            ASTNode *instr = search->child;
            while (instr) {
                ASTNode *next_instr = instr->sibling;
                add_dast(exit_owned, instr);
                instr = next_instr;
            }
            ctx->changed_flags |= FLAG_UTIL;
        }
        frag->ast = NULL;
    }

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

    fre_cntx(frag);
    if (map.entries) free(map.entries);
    return 0;
}

int ast_grft(Context *ctx, ASTNode *target_node, const char *rexx_code) {
    return ast_grft_interpolated(ctx, target_node, rexx_code, NULL, 0);
}

const char* node_type_to_string(NodeType type) {
    switch (type) {
        case ABS_POS: return "ABS_POS";
        case ADDRESS: return "ADDRESS";
        case IMPLICIT_CMD: return "IMPLICIT_CMD";
        case ARG: return "ARG";
        case ARGS: return "ARGS";
        case ASSEMBLER: return "ASSEMBLER";
        case ASSIGN: return "ASSIGN";
        case BY: return "BY";
        case CALL: return "CALL";
        case CLASS: return "CLASS";
        case LITERAL: return "LITERAL";
        case CONST_SYMBOL: return "CONST_SYMBOL";
        case DEC_DIGITS: return "DEC_DIGITS";
        case DEC_FORM: return "DEC_FORM";
        case DEC_FUZZ: return "DEC_FUZZ";
        case DEC_CASE: return "DEC_CASE";
        case DEC_STANDARD: return "DEC_STANDARD";
        case DEFINE: return "DEFINE";
        case DO: return "DO";
        case ENVIRONMENT: return "ENVIRONMENT";
        case ERROR: return "ERROR";
        case EXPOSED: return "EXPOSED";
        case EXIT: return "EXIT";
        case FOR: return "FOR";
        case FUNCTION: return "FUNCTION";
        case FUNC_SYMBOL: return "FUNC_SYMBOL";
        case IF: return "IF";
        case IMPORT: return "IMPORT";
        case IMPORTED_FILE: return "IMPORTED_FILE";
        case INSTRUCTIONS: return "INSTRUCTIONS";
        case ITERATE: return "ITERATE";
        case LABEL: return "LABEL";
        case LEAVE: return "LEAVE";
        case FLOAT: return "FLOAT";
        case INTEGER: return "INTEGER";
        case OP_MAKE_ARRAY: return "OP_MAKE_ARRAY";
        case DECIMAL: return "DECIMAL";
        case NAMESPACE: return "NAMESPACE";
        case NOP: return "NOP";
        case NOVAL: return "NOVAL";
        case OP_ADD: return "OP_ADD";
        case OP_MINUS: return "OP_MINUS";
        case OP_AND: return "OP_AND";
        case OP_ARGS: return "OP_ARGS";
        case OP_ARG_VALUE: return "OP_ARG_VALUE";
        case OP_ARG_EXISTS: return "OP_ARG_EXISTS";
        case OP_ARG_IX_EXISTS: return "OP_ARG_IX_EXISTS";
        case OP_CONCAT: return "OP_CONCAT";
        case OP_MULT: return "OP_MULT";
        case OP_DIV: return "OP_DIV";
        case OP_IDIV: return "OP_IDIV";
        case OP_MOD: return "OP_MOD";
        case OP_OR: return "OP_OR";
        case OP_POWER: return "OP_POWER";
        case OP_NOT: return "OP_NOT";
        case OP_NEG: return "OP_NEG";
        case OP_PLUS: return "OP_PLUS";
        case OP_COMPARE_EQUAL: return "OP_COMPARE_EQUAL";
        case OP_COMPARE_NEQ: return "OP_COMPARE_NEQ";
        case OP_COMPARE_GT: return "OP_COMPARE_GT";
        case OP_COMPARE_LT: return "OP_COMPARE_LT";
        case OP_COMPARE_GTE: return "OP_COMPARE_GTE";
        case OP_COMPARE_LTE: return "OP_COMPARE_LTE";
        case OP_COMPARE_S_EQ: return "OP_COMPARE_S_EQ";
        case OP_COMPARE_S_NEQ: return "OP_COMPARE_S_NEQ";
        case OP_COMPARE_S_GT: return "OP_COMPARE_S_GT";
        case OP_COMPARE_S_LT: return "OP_COMPARE_S_LT";
        case OP_COMPARE_S_GTE: return "OP_COMPARE_S_GTE";
        case OP_COMPARE_S_LTE: return "OP_COMPARE_S_LTE";
        case OP_SCONCAT: return "OP_SCONCAT";
        case OPTIONS: return "OPTIONS";
        case PARSE: return "PARSE";
        case PATTERN: return "PATTERN";
        case PROCEDURE: return "PROCEDURE";
        case PROGRAM_FILE: return "PROGRAM_FILE";
        case PULL: return "PULL";
        case REL_POS: return "REL_POS";
        case RANGE: return "RANGE";
        case REPEAT: return "REPEAT";
        case REDIRECT_IN: return "REDIRECT_IN";
        case REDIRECT_OUT: return "REDIRECT_OUT";
        case REDIRECT_ERROR: return "REDIRECT_ERROR";
        case REDIRECT_EXPOSE: return "REDIRECT_EXPOSE";
        case RETURN: return "RETURN";
        case REXX_OPTIONS: return "REXX_OPTIONS";
        case REXX_UNIVERSE: return "REXX_UNIVERSE";
        case SAY: return "SAY";
        case SIGN: return "SIGN";
        case STRING: return "STRING";
        case BINARY: return "BINARY";
        case TARGET: return "TARGET";
        case TEMPLATES: return "TEMPLATES";
        case TO: return "TO";
        case TOKEN: return "TOKEN";
        case UPPER: return "UPPER";
        case VAR_REFERENCE: return "VAR_REFERENCE";
        case VAR_SYMBOL: return "VAR_SYMBOL";
        case VAR_TARGET: return "VAR_TARGET";
        case VOID: return "VOID";
        case VARG: return "VARG";
        case VARG_REFERENCE: return "VARG_REFERENCE";
        case CONSTANT: return "CONSTANT";
        case WARNING: return "WARNING";
        case WHILE: return "WHILE";
        case UNTIL: return "UNTIL";
        case FACTORY: return "FACTORY";
        case METHOD: return "METHOD";
        case WITH: return "WITH";
        case NODE_REGISTER: return "NODE_REGISTER";
        case OF: return "OF";
        case CLASS_DEF: return "CLASS_DEF";
        case MEMBER_CALL: return "MEMBER_CALL";
        case FACTORY_CALL: return "FACTORY_CALL";
        case EXIT_OWNED: return "EXIT_OWNED";
        case EXIT_EXTENDED: return "EXIT_EXTENDED";
        case EXIT_TOKEN: return "EXIT_TOKEN";
    }
    return "UNKNOWN";
}
