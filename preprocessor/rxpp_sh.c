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

#ifdef ENABLE_PARSER_MODE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dslsyntax_common.h"
#include "dslsyntax_log.h"
#include "dslsyntax_parser.h"
#include "rxcp_highlight_controller.h"
#include "rxpp_sh_config.h"

typedef struct RxppShNameList {
    char **items;
    size_t count;
    size_t capacity;
} RxppShNameList;

typedef struct RxppShDiagnostic {
    CB_NodeType type;
    size_t pos;
    size_t length;
    CB_Severity severity;
    char *message_code;
    char *message;
} RxppShDiagnostic;

typedef struct RxppShDiagnosticList {
    RxppShDiagnostic *items;
    size_t count;
    size_t capacity;
} RxppShDiagnosticList;

typedef enum RxppShSegmentMode {
    RXPP_SH_SEGMENT_NORMAL,
    RXPP_SH_SEGMENT_MACRO_CONSTANTS
} RxppShSegmentMode;

static const char *rxpp_sh_select_maclib(void);

static void rxpp_sh_free_code_buffer(CodeBuffer *cb) {
    if (!cb) return;
    if (cb->ep_rules) {
        cb_free_ep_rules(cb->ep_rules);
        cb->ep_rules = 0;
    }
    free_code_buffer(cb);
}

static int rxpp_sh_is_ident_start(char value) {
    return (value >= 'A' && value <= 'Z') ||
           (value >= 'a' && value <= 'z') ||
           value == '_';
}

static int rxpp_sh_is_ident_part(char value) {
    return rxpp_sh_is_ident_start(value) ||
           (value >= '0' && value <= '9');
}

static int rxpp_sh_is_space_no_newline(char value) {
    return value == ' ' || value == '\t' || value == '\f' || value == '\v';
}

static char rxpp_sh_upper_ascii(char value) {
    if (value >= 'a' && value <= 'z') return (char)(value - 'a' + 'A');
    return value;
}

static int rxpp_sh_span_equals_ci(const char *source, size_t start, size_t end, const char *word) {
    size_t i;
    size_t word_len;

    if (!source || !word || end < start) return 0;
    word_len = strlen(word);
    if (end - start != word_len) return 0;
    for (i = 0; i < word_len; i++) {
        if (rxpp_sh_upper_ascii(source[start + i]) != rxpp_sh_upper_ascii(word[i])) return 0;
    }
    return 1;
}

static char *rxpp_sh_upper_span(const char *source, size_t start, size_t end) {
    char *name;
    size_t i;

    if (!source || end <= start) return 0;
    name = malloc(end - start + 1);
    if (!name) return 0;
    for (i = start; i < end; i++) {
        name[i - start] = rxpp_sh_upper_ascii(source[i]);
    }
    name[end - start] = 0;
    return name;
}

static void rxpp_sh_name_list_free(RxppShNameList *list) {
    size_t i;

    if (!list) return;
    for (i = 0; i < list->count; i++) free(list->items[i]);
    free(list->items);
    list->items = 0;
    list->count = 0;
    list->capacity = 0;
}

static int rxpp_sh_name_list_contains_upper(RxppShNameList *list, const char *name) {
    size_t i;

    if (!list || !name) return 0;
    for (i = 0; i < list->count; i++) {
        if (strcmp(list->items[i], name) == 0) return 1;
    }
    return 0;
}

static int rxpp_sh_name_list_contains_span(RxppShNameList *list, const char *source, size_t start, size_t end) {
    char *name;
    int result;

    name = rxpp_sh_upper_span(source, start, end);
    if (!name) return 0;
    result = rxpp_sh_name_list_contains_upper(list, name);
    free(name);
    return result;
}

static void rxpp_sh_name_list_add_span(RxppShNameList *list, const char *source, size_t start, size_t end) {
    char *name;
    char **items;

    if (!list || !source || end <= start) return;
    name = rxpp_sh_upper_span(source, start, end);
    if (!name) return;
    if (rxpp_sh_name_list_contains_upper(list, name)) {
        free(name);
        return;
    }
    if (list->count == list->capacity) {
        list->capacity = list->capacity ? list->capacity * 2 : 8;
        items = realloc(list->items, list->capacity * sizeof(char *));
        if (!items) {
            free(name);
            list->capacity = list->count;
            return;
        }
        list->items = items;
    }
    list->items[list->count++] = name;
}

static size_t rxpp_sh_skip_ws(const char *source, size_t pos, size_t end) {
    while (pos < end && rxpp_sh_is_space_no_newline(source[pos])) pos++;
    return pos;
}

static size_t rxpp_sh_scan_identifier(const char *source, size_t pos, size_t end) {
    if (pos >= end || !rxpp_sh_is_ident_start(source[pos])) return pos;
    pos++;
    while (pos < end && rxpp_sh_is_ident_part(source[pos])) pos++;
    return pos;
}

static int rxpp_sh_find_directive(const char *source,
                                  size_t line_start,
                                  size_t line_end,
                                  size_t *directive_start,
                                  size_t *directive_end) {
    size_t pos;

    pos = rxpp_sh_skip_ws(source, line_start, line_end);
    if (pos + 2 > line_end || source[pos] != '#' || source[pos + 1] != '#') return 0;
    if (directive_start) *directive_start = pos;
    pos += 2;
    while (pos < line_end && rxpp_sh_is_ident_part(source[pos])) pos++;
    if (directive_end) *directive_end = pos;
    return 1;
}

static int rxpp_sh_directive_is(const char *source,
                                size_t directive_start,
                                size_t directive_end,
                                const char *word) {
    if (directive_end < directive_start + 2) return 0;
    return rxpp_sh_span_equals_ci(source, directive_start + 2, directive_end, word);
}

static int rxpp_sh_is_keyword(const char *source, size_t start, size_t end) {
    static const char *keywords[] = {
            "ADDRESS", "ARG", "BY", "CALL", "CLASS", "DO", "ELSE", "END",
            "EXIT", "EXPOSE", "FACTORY", "FOREVER", "FOR", "IF", "IMPORT",
            "ITERATE", "LEAVE", "METHOD", "NAMESPACE", "NUMERIC", "OPTIONS",
            "OTHERWISE", "PARSE", "PROCEDURE", "PULL", "RETURN", "SAY",
            "SELECT", "SIGNAL", "THEN", "TO", "TRACE", "UNTIL", "WHEN",
            "WHILE", 0
    };
    size_t i;

    for (i = 0; keywords[i]; i++) {
        if (rxpp_sh_span_equals_ci(source, start, end, keywords[i])) return 1;
    }
    return 0;
}

static void rxpp_sh_add_byte_token(CB_ParseTree *tb,
                                   const char *source,
                                   size_t source_len,
                                   CB_NodeType type,
                                   size_t start,
                                   size_t end) {
    CB_Node node;

    if (!tb || !source || end <= start || start >= source_len) return;
    if (end > source_len) end = source_len;
    node = cb_create_node_from_utf8_byte_span(type, source, source_len, start, end - start);
    if (node.length == 0 && end > start) return;
    cb_add_child_node(tb, node);
}

static int rxpp_sh_try_macro_variable(const char *source, size_t pos, size_t end, size_t *span_end) {
    size_t cursor;

    if (pos >= end || source[pos] != '{') return 0;
    cursor = pos + 1;
    if (cursor >= end || !rxpp_sh_is_ident_start(source[cursor])) return 0;
    cursor++;
    while (cursor < end && rxpp_sh_is_ident_part(source[cursor])) cursor++;
    if (cursor >= end || source[cursor] != '}') return 0;
    if (span_end) *span_end = cursor + 1;
    return 1;
}

static size_t rxpp_sh_scan_string(const char *source, size_t pos, size_t end) {
    char quote;

    if (pos >= end || (source[pos] != '"' && source[pos] != '\'')) return pos;
    quote = source[pos++];
    while (pos < end) {
        if (source[pos] == quote) {
            pos++;
            if (pos < end && source[pos] == quote) {
                pos++;
                continue;
            }
            break;
        }
        pos++;
    }
    return pos;
}

static size_t rxpp_sh_scan_block_comment_close(const char *source, size_t pos, size_t end, int *closed) {
    if (closed) *closed = 0;
    while (pos + 1 < end) {
        if (source[pos] == '*' && source[pos + 1] == '/') {
            if (closed) *closed = 1;
            return pos + 2;
        }
        pos++;
    }
    return end;
}

static size_t rxpp_sh_scan_block_comment_line(const char *source, size_t pos, size_t end, int *closed) {
    if (pos + 2 > end) {
        if (closed) *closed = 0;
        return end;
    }
    return rxpp_sh_scan_block_comment_close(source, pos + 2, end, closed);
}

static size_t rxpp_sh_scan_number(const char *source, size_t pos, size_t end) {
    while (pos < end &&
           ((source[pos] >= '0' && source[pos] <= '9') ||
            (source[pos] >= 'A' && source[pos] <= 'F') ||
            (source[pos] >= 'a' && source[pos] <= 'f') ||
            source[pos] == '.' || source[pos] == 'x' || source[pos] == 'X')) {
        pos++;
    }
    return pos;
}

static CB_NodeType rxpp_sh_operator_type(const char *source, size_t pos) {
    switch (source[pos]) {
        case '=':
            return LEXER_OPERATOR_ASSIGN;
        case '+':
        case '-':
        case '*':
        case '/':
            return LEXER_OPERATOR_ARITHMETIC;
        case '<':
        case '>':
        case '\\':
        case '&':
        case '|':
            return LEXER_OPERATOR_LOGICAL;
        case '(':
            return LEXER_LH_EXPR;
        case ')':
            return LEXER_RH_EXPR;
        case ';':
            return LEXER_STATEMENT_SEPARATOR;
        case ',':
        case '.':
        case ':':
        case '{':
        case '}':
        case '[':
        case ']':
            return LEXER_SEPARATOR;
        default:
            return LEXER_TOKEN;
    }
}

static char *rxpp_sh_shell_quote(const char *value) {
    char *quoted;
    char *out;
    size_t len;
    const char *cursor;

    if (!value) value = "";
    len = 2;
    cursor = value;
    while (*cursor) {
        len += (*cursor == '\'') ? 4 : 1;
        cursor++;
    }

    quoted = malloc(len + 1);
    if (!quoted) return 0;

    out = quoted;
    *out++ = '\'';
    cursor = value;
    while (*cursor) {
        if (*cursor == '\'') {
            *out++ = '\'';
            *out++ = '\\';
            *out++ = '\'';
            *out++ = '\'';
        } else {
            *out++ = *cursor;
        }
        cursor++;
    }
    *out++ = '\'';
    *out = 0;
    return quoted;
}

static int rxpp_sh_make_temp_path(const char *prefix, char *path, size_t path_size) {
    const char *tmpdir;
    int fd;

    tmpdir = getenv("TMPDIR");
    if (!tmpdir || tmpdir[0] == 0) tmpdir = "/tmp";
    if (snprintf(path, path_size, "%s/%s.XXXXXX", tmpdir, prefix) >= (int)path_size) return -1;

    fd = mkstemp(path);
    if (fd < 0) return -1;
    close(fd);
    return 0;
}

static int rxpp_sh_write_file(const char *path, const char *text) {
    FILE *file;
    size_t length;

    file = fopen(path, "wb");
    if (!file) return -1;
    length = text ? strlen(text) : 0;
    if (length && fwrite(text, 1, length, file) != length) {
        fclose(file);
        return -1;
    }
    if (fclose(file) != 0) return -1;
    return 0;
}

static char *rxpp_sh_read_file(const char *path) {
    FILE *file;
    long length;
    char *buffer;
    size_t read_len;

    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    length = ftell(file);
    if (length < 0) {
        fclose(file);
        return 0;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }

    buffer = malloc((size_t)length + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    read_len = fread(buffer, 1, (size_t)length, file);
    fclose(file);
    if (read_len != (size_t)length) {
        free(buffer);
        return 0;
    }
    buffer[length] = 0;
    return buffer;
}

static void rxpp_sh_collect_macro_names(const char *source, size_t source_len, RxppShNameList *names) {
    size_t line_start;
    size_t line_end;
    size_t directive_start;
    size_t directive_end;
    size_t pos;
    size_t name_start;
    size_t name_end;

    if (!source || !names) return;

    line_start = 0;
    while (line_start < source_len) {
        line_end = line_start;
        while (line_end < source_len && source[line_end] != '\n' && source[line_end] != '\r') line_end++;

        if (rxpp_sh_find_directive(source, line_start, line_end, &directive_start, &directive_end) &&
            (rxpp_sh_directive_is(source, directive_start, directive_end, "DEFINE") ||
             rxpp_sh_directive_is(source, directive_start, directive_end, "MACRO"))) {
            pos = rxpp_sh_skip_ws(source, directive_end, line_end);
            name_start = pos;
            name_end = rxpp_sh_scan_identifier(source, name_start, line_end);

            if (rxpp_sh_directive_is(source, directive_start, directive_end, "DEFINE") &&
                rxpp_sh_span_equals_ci(source, name_start, name_end, "CMD")) {
                pos = rxpp_sh_skip_ws(source, name_end, line_end);
                name_start = pos;
                name_end = rxpp_sh_scan_identifier(source, name_start, line_end);
            }

            if (name_end > name_start) rxpp_sh_name_list_add_span(names, source, name_start, name_end);
        }

        line_start = line_end;
        if (line_start < source_len && source[line_start] == '\r') line_start++;
        if (line_start < source_len && source[line_start] == '\n') line_start++;
    }
}

static void rxpp_sh_collect_configured_maclib_macro_names(RxppShNameList *names) {
    const char *maclib_path;
    char *maclib_source;

    if (!names) return;
    maclib_path = rxpp_sh_select_maclib();
    maclib_source = rxpp_sh_read_file(maclib_path);
    if (!maclib_source) return;
    rxpp_sh_collect_macro_names(maclib_source, strlen(maclib_source), names);
    free(maclib_source);
}

static void rxpp_sh_emit_segment_tokens(CB_ParseTree *tb,
                                        const char *source,
                                        size_t source_len,
                                        size_t start,
                                        size_t end,
                                        RxppShNameList *macro_names,
                                        RxppShSegmentMode mode,
                                        int *in_block_comment) {
    size_t pos;
    size_t token_end;
    CB_NodeType type;
    int closed;

    pos = start;
    while (pos < end) {
        if (in_block_comment && *in_block_comment) {
            token_end = rxpp_sh_scan_block_comment_close(source, pos, end, &closed);
            rxpp_sh_add_byte_token(tb, source, source_len, LEXER_COMMENT, pos, token_end);
            pos = token_end;
            if (!closed) return;
            *in_block_comment = 0;
            continue;
        }

        if (rxpp_sh_is_space_no_newline(source[pos])) {
            pos++;
            continue;
        }

        if (source[pos] == '-' && pos + 1 < end && source[pos + 1] == '-') {
            rxpp_sh_add_byte_token(tb, source, source_len, LEXER_COMMENT, pos, end);
            return;
        }
        if (source[pos] == '#' && pos + 1 < end && source[pos + 1] == '#') {
            rxpp_sh_add_byte_token(tb, source, source_len, LEXER_COMMENT, pos, end);
            return;
        }
        if (source[pos] == '/' && pos + 1 < end && source[pos + 1] == '*') {
            token_end = rxpp_sh_scan_block_comment_line(source, pos, end, &closed);
            rxpp_sh_add_byte_token(tb, source, source_len, LEXER_COMMENT, pos, token_end);
            pos = token_end;
            if (in_block_comment && !closed) *in_block_comment = 1;
            continue;
        }
        if (source[pos] == '"' || source[pos] == '\'') {
            token_end = rxpp_sh_scan_string(source, pos, end);
            rxpp_sh_add_byte_token(tb, source, source_len, LEXER_STRING_LITERAL, pos, token_end);
            pos = token_end;
            continue;
        }
        if (rxpp_sh_try_macro_variable(source, pos, end, &token_end)) {
            rxpp_sh_add_byte_token(tb, source, source_len, LEXER_MACRO_VARIABLE, pos, token_end);
            pos = token_end;
            continue;
        }
        if (source[pos] >= '0' && source[pos] <= '9') {
            token_end = rxpp_sh_scan_number(source, pos, end);
            rxpp_sh_add_byte_token(tb, source, source_len, LEXER_NUMBER_LITERAL, pos, token_end);
            pos = token_end;
            continue;
        }
        if (rxpp_sh_is_ident_start(source[pos])) {
            token_end = rxpp_sh_scan_identifier(source, pos, end);
            if (mode == RXPP_SH_SEGMENT_MACRO_CONSTANTS) {
                type = LEXER_MACRO_CONSTANT;
            } else if (rxpp_sh_name_list_contains_span(macro_names, source, pos, token_end)) {
                type = LEXER_MACRO_IDENTIFIER;
            } else if (rxpp_sh_is_keyword(source, pos, token_end)) {
                type = LEXER_KEYWORD;
            } else {
                type = LEXER_IDENTIFIER;
            }
            rxpp_sh_add_byte_token(tb, source, source_len, type, pos, token_end);
            pos = token_end;
            continue;
        }

        type = rxpp_sh_operator_type(source, pos);
        rxpp_sh_add_byte_token(tb, source, source_len, type, pos, pos + 1);
        pos++;
    }
}

static void rxpp_sh_emit_directive_line(CB_ParseTree *tb,
                                        const char *source,
                                        size_t source_len,
                                        size_t line_start,
                                        size_t line_end,
                                        size_t directive_start,
                                        size_t directive_end,
                                        RxppShNameList *macro_names,
                                        int *in_block_comment) {
    size_t pos;
    size_t token_end;
    int is_define;
    int is_macro;

    is_define = rxpp_sh_directive_is(source, directive_start, directive_end, "DEFINE");
    is_macro = rxpp_sh_directive_is(source, directive_start, directive_end, "MACRO");

    (void)line_start;
    rxpp_sh_add_byte_token(tb, source, source_len, LEXER_PREPROCESSOR, directive_start, directive_end);

    if (rxpp_sh_directive_is(source, directive_start, directive_end, "INCLUDE") ||
        rxpp_sh_directive_is(source, directive_start, directive_end, "USE")) {
        pos = rxpp_sh_skip_ws(source, directive_end, line_end);
        if (pos < line_end) rxpp_sh_add_byte_token(tb, source, source_len, LEXER_STRING_LITERAL, pos, line_end);
        return;
    }

    if (is_define || is_macro) {
        pos = rxpp_sh_skip_ws(source, directive_end, line_end);
        token_end = rxpp_sh_scan_identifier(source, pos, line_end);
        if (is_define && rxpp_sh_span_equals_ci(source, pos, token_end, "CMD")) {
            rxpp_sh_add_byte_token(tb, source, source_len, LEXER_MACRO_CONSTANT, pos, token_end);
            pos = rxpp_sh_skip_ws(source, token_end, line_end);
            token_end = rxpp_sh_scan_identifier(source, pos, line_end);
        }
        if (token_end > pos) {
            rxpp_sh_add_byte_token(tb, source, source_len, LEXER_MACRO_IDENTIFIER, pos, token_end);
            pos = token_end;
        }
        rxpp_sh_emit_segment_tokens(tb,
                                    source,
                                    source_len,
                                    pos,
                                    line_end,
                                    macro_names,
                                    RXPP_SH_SEGMENT_NORMAL,
                                    in_block_comment);
        return;
    }

    if (rxpp_sh_directive_is(source, directive_start, directive_end, "SET") ||
        rxpp_sh_directive_is(source, directive_start, directive_end, "UNSET") ||
        rxpp_sh_directive_is(source, directive_start, directive_end, "IF") ||
        rxpp_sh_directive_is(source, directive_start, directive_end, "IFN") ||
        rxpp_sh_directive_is(source, directive_start, directive_end, "CFLAG") ||
        rxpp_sh_directive_is(source, directive_start, directive_end, "CFLAGS") ||
        rxpp_sh_directive_is(source, directive_start, directive_end, "ARRAY") ||
        rxpp_sh_directive_is(source, directive_start, directive_end, "GLOBAL") ||
        rxpp_sh_directive_is(source, directive_start, directive_end, "STEM") ||
        rxpp_sh_directive_is(source, directive_start, directive_end, "DATA")) {
        rxpp_sh_emit_segment_tokens(tb,
                                    source,
                                    source_len,
                                    directive_end,
                                    line_end,
                                    macro_names,
                                    RXPP_SH_SEGMENT_MACRO_CONSTANTS,
                                    in_block_comment);
        return;
    }

    rxpp_sh_emit_segment_tokens(tb,
                                source,
                                source_len,
                                directive_end,
                                line_end,
                                macro_names,
                                RXPP_SH_SEGMENT_NORMAL,
                                in_block_comment);
}

static void rxpp_sh_emit_source_tokens(CB_ParseTree *tb,
                                       CodeBuffer *cb,
                                       const char *source,
                                       RxppShNameList *macro_names) {
    size_t source_len;
    size_t line_start;
    size_t line_end;
    size_t directive_start;
    size_t directive_end;
    int in_block_comment;

    (void)cb;
    if (!tb || !source) return;

    source_len = strlen(source);
    line_start = 0;
    in_block_comment = 0;
    while (line_start < source_len) {
        line_end = line_start;
        while (line_end < source_len && source[line_end] != '\n' && source[line_end] != '\r') line_end++;

        if (!in_block_comment &&
            rxpp_sh_find_directive(source, line_start, line_end, &directive_start, &directive_end)) {
            rxpp_sh_emit_directive_line(tb,
                                        source,
                                        source_len,
                                        line_start,
                                        line_end,
                                        directive_start,
                                        directive_end,
                                        macro_names,
                                        &in_block_comment);
        } else {
            rxpp_sh_emit_segment_tokens(tb,
                                        source,
                                        source_len,
                                        line_start,
                                        line_end,
                                        macro_names,
                                        RXPP_SH_SEGMENT_NORMAL,
                                        &in_block_comment);
        }

        line_start = line_end;
        if (line_start < source_len && source[line_start] == '\r') line_start++;
        if (line_start < source_len && source[line_start] == '\n') line_start++;
    }
}

static void rxpp_sh_diagnostic_list_free(RxppShDiagnosticList *list) {
    size_t i;

    if (!list) return;
    for (i = 0; i < list->count; i++) {
        free(list->items[i].message_code);
        free(list->items[i].message);
    }
    free(list->items);
    list->items = 0;
    list->count = 0;
    list->capacity = 0;
}

static void rxpp_sh_diagnostic_list_add(RxppShDiagnosticList *list, CB_Node *node) {
    RxppShDiagnostic *items;

    if (!list || !node) return;
    if (node->type != SYNTAX_ERROR && node->type != INTERNAL_ERROR) return;
    if (list->count == list->capacity) {
        list->capacity = list->capacity ? list->capacity * 2 : 4;
        items = realloc(list->items, list->capacity * sizeof(RxppShDiagnostic));
        if (!items) {
            list->capacity = list->count;
            return;
        }
        list->items = items;
    }
    memset(&list->items[list->count], 0, sizeof(RxppShDiagnostic));
    list->items[list->count].type = node->type;
    list->items[list->count].pos = node->pos;
    list->items[list->count].length = node->length;
    list->items[list->count].severity = node->severity;
    list->items[list->count].message_code = node->message_code ? strdup(node->message_code) : 0;
    list->items[list->count].message = node->message ? strdup(node->message) : 0;
    list->count++;
}

static void rxpp_sh_collect_diagnostics_node(CB_Node *node, size_t depth, void *user_data) {
    (void)depth;
    rxpp_sh_diagnostic_list_add((RxppShDiagnosticList *)user_data, node);
}

static int rxpp_sh_spans_overlap(size_t left_pos, size_t left_len, size_t right_pos, size_t right_len) {
    size_t left_end;
    size_t right_end;

    if (left_len == 0 || right_len == 0) return left_pos == right_pos;
    left_end = left_pos + left_len;
    right_end = right_pos + right_len;
    return left_pos < right_end && right_pos < left_end;
}

static const RxppShDiagnostic *rxpp_sh_select_diagnostic_for_node(CB_Node *node,
                                                                  RxppShDiagnosticList *diagnostics) {
    const RxppShDiagnostic *best;
    size_t i;

    if (!node || !diagnostics) return 0;
    best = 0;
    for (i = 0; i < diagnostics->count; i++) {
        RxppShDiagnostic *diag = &diagnostics->items[i];
        if (!rxpp_sh_spans_overlap(node->pos, node->length, diag->pos, diag->length)) continue;
        if (!best ||
            diag->severity > best->severity ||
            (diag->severity == best->severity && diag->length < best->length) ||
            (diag->severity == best->severity && diag->length == best->length && diag->pos < best->pos)) {
            best = diag;
        }
    }
    return best;
}

static void rxpp_sh_overlay_diagnostic_on_leaf(CB_Node *node, size_t depth, void *user_data) {
    RxppShDiagnosticList *diagnostics;
    const RxppShDiagnostic *diag;

    (void)depth;
    diagnostics = (RxppShDiagnosticList *)user_data;
    if (!node || node->child) return;
    if (node->type == SYNTAX_ERROR || node->type == INTERNAL_ERROR) return;

    diag = rxpp_sh_select_diagnostic_for_node(node, diagnostics);
    if (!diag) return;
    if (node->severity > diag->severity) return;
    if (node->severity == diag->severity && node->message) return;

    node->severity = diag->severity;
    if (node->message_code) {
        free(node->message_code);
        node->message_code = 0;
    }
    if (node->message) {
        free(node->message);
        node->message = 0;
    }
    node->message_code = diag->message_code ? strdup(diag->message_code) : 0;
    node->message = diag->message ? strdup(diag->message) : 0;
}

static void rxpp_sh_overlay_diagnostics(CB_ParseTree *tb, RxppShDiagnosticList *diagnostics) {
    if (!tb || !diagnostics || diagnostics->count == 0) return;
    cb_walk_tree_top_down(tb, rxpp_sh_overlay_diagnostic_on_leaf, diagnostics);
}

static void rxpp_sh_add_diagnostic_nodes(CB_ParseTree *tb, RxppShDiagnosticList *diagnostics) {
    size_t i;

    if (!tb || !diagnostics) return;
    for (i = 0; i < diagnostics->count; i++) {
        CB_Node node = cb_create_node(diagnostics->items[i].type,
                                      diagnostics->items[i].pos,
                                      diagnostics->items[i].length);
        node.severity = diagnostics->items[i].severity;
        node.message_code = diagnostics->items[i].message_code;
        node.message = diagnostics->items[i].message;
        cb_add_child_node(tb, node);
    }
}

static CB_ParseTree *rxpp_sh_build_tree(CodeBuffer *cb,
                                        const char *source,
                                        RxppShDiagnosticList *diagnostics) {
    CB_ParseTree *tb;
    CB_Node root_node;
    RxppShNameList macro_names;

    if (!cb || !source) return 0;

    memset(&macro_names, 0, sizeof(macro_names));
    rxpp_sh_collect_macro_names(source, strlen(source), &macro_names);
    rxpp_sh_collect_configured_maclib_macro_names(&macro_names);

    tb = cb_create_token_buffer();
    root_node = cb_create_node(PARSE_TREE_FILE, 0, get_code_buffer_length(cb));
    cb_add_child_node(tb, root_node);
    cb_set_current_parent_to_root_node(tb);

    rxpp_sh_emit_source_tokens(tb, cb, source, &macro_names);
    rxpp_sh_add_diagnostic_nodes(tb, diagnostics);

    cb_order_tree(tb);
    cb_add_missing_tokens(tb, cb, cb_default_get_token_callback, 0);
    cb_tweak_tree_positions(tb);
    rxpp_sh_overlay_diagnostics(tb, diagnostics);
    cb_validate_tree(tb);

    rxpp_sh_name_list_free(&macro_names);
    return tb;
}

static int rxpp_sh_file_is_readable(const char *path) {
    return path && path[0] && access(path, R_OK) == 0;
}

static int rxpp_sh_file_is_executable(const char *path) {
    return path && path[0] && access(path, X_OK) == 0;
}

static const char *rxpp_sh_select_rxpp(void) {
    const char *rxpp_bin;

    rxpp_bin = getenv("RXPP_SH_RXPP");
    if (rxpp_bin && rxpp_bin[0]) return rxpp_bin;
    if (rxpp_sh_file_is_executable(RXPP_SH_BUILD_RXPP)) return RXPP_SH_BUILD_RXPP;
    return "rxpp";
}

static const char *rxpp_sh_select_maclib(void) {
    const char *maclib_path;

    maclib_path = getenv("RXPP_SH_MACLIB");
    if (maclib_path && maclib_path[0]) return maclib_path;
    if (rxpp_sh_file_is_readable(RXPP_SH_BUILD_MACLIB)) return RXPP_SH_BUILD_MACLIB;
    if (rxpp_sh_file_is_readable(RXPP_SH_SOURCE_MACLIB)) return RXPP_SH_SOURCE_MACLIB;
    return "maclib.rexx";
}

static int rxpp_sh_run_rxpp(const char *input_path, const char *output_path) {
    const char *rxpp_bin;
    const char *maclib_path;
    char *rxpp_q;
    char *input_q;
    char *output_q;
    char *maclib_q;
    char *command;
    size_t command_len;
    int result;

    rxpp_bin = rxpp_sh_select_rxpp();
    maclib_path = rxpp_sh_select_maclib();

    rxpp_q = rxpp_sh_shell_quote(rxpp_bin);
    input_q = rxpp_sh_shell_quote(input_path);
    output_q = rxpp_sh_shell_quote(output_path);
    maclib_q = rxpp_sh_shell_quote(maclib_path);
    if (!rxpp_q || !input_q || !output_q || !maclib_q) {
        free(rxpp_q);
        free(input_q);
        free(output_q);
        free(maclib_q);
        return -1;
    }

    command_len = strlen(rxpp_q) + strlen(input_q) + strlen(output_q) + strlen(maclib_q) + 96;
    command = malloc(command_len);
    if (!command) {
        free(rxpp_q);
        free(input_q);
        free(output_q);
        free(maclib_q);
        return -1;
    }
    snprintf(command,
             command_len,
             "%s rxprecomp -I %s -o %s -m %s >/dev/null 2>&1",
             rxpp_q,
             input_q,
             output_q,
             maclib_q);

    result = system(command);

    free(command);
    free(rxpp_q);
    free(input_q);
    free(output_q);
    free(maclib_q);

    return result == 0 ? 0 : -1;
}

static void rxpp_sh_set_error_tree(CodeBuffer *cb, const char *message) {
    CB_ParseTree *tb;
    CB_Node root_node;
    CB_Node error_node;
    size_t length;

    if (!cb) return;

    length = get_code_buffer_length(cb);
    tb = cb_create_token_buffer();
    root_node = cb_create_node(PARSE_TREE_FILE, 0, length);
    cb_add_child_node(tb, root_node);
    cb_set_current_parent_to_root_node(tb);

    error_node = cb_create_node(SYNTAX_ERROR, 0, length ? 1 : 0);
    error_node.severity = CB_ERROR;
    error_node.message = strdup(message ? message : "RXPP parser wrapper failed.");
    cb_add_child_node(tb, error_node);

    cb_order_tree(tb);
    cb_add_missing_tokens(tb, cb, cb_default_get_token_callback, 0);
    cb_tweak_tree_positions(tb);
    cb_validate_tree(tb);

    cb->parse_tree = tb;
}

static void rxpp_sh_parse(CodeBuffer *cb) {
    char input_path[1024];
    char output_path[1024];
    char *source_code;
    char *generated_code;
    CB_ParseTree *diagnostic_tree;
    RxppShDiagnosticList diagnostics;
    int have_input_path;
    int have_output_path;

    if (!cb) return;

    input_path[0] = 0;
    output_path[0] = 0;
    generated_code = 0;
    diagnostic_tree = 0;
    memset(&diagnostics, 0, sizeof(diagnostics));
    have_input_path = 0;
    have_output_path = 0;

    source_code = get_code_buffer_source(cb);
    if (!source_code) return;

    if (rxpp_sh_make_temp_path("rxpp-sh-input", input_path, sizeof(input_path)) != 0) {
        free(source_code);
        rxpp_sh_set_error_tree(cb, "RXPP parser wrapper could not allocate an input temp file.");
        return;
    }
    have_input_path = 1;

    if (rxpp_sh_make_temp_path("rxpp-sh-output", output_path, sizeof(output_path)) != 0) {
        free(source_code);
        if (have_input_path) unlink(input_path);
        rxpp_sh_set_error_tree(cb, "RXPP parser wrapper could not allocate an output temp file.");
        return;
    }
    have_output_path = 1;

    if (rxpp_sh_write_file(input_path, source_code) != 0) {
        free(source_code);
        if (have_input_path) unlink(input_path);
        if (have_output_path) unlink(output_path);
        rxpp_sh_set_error_tree(cb, "RXPP parser wrapper could not write the editor buffer.");
        return;
    }

    if (rxpp_sh_run_rxpp(input_path, output_path) != 0) {
        free(source_code);
        if (have_input_path) unlink(input_path);
        if (have_output_path) unlink(output_path);
        rxpp_sh_set_error_tree(cb, "RXPP preprocessing failed.");
        return;
    }

    generated_code = rxpp_sh_read_file(output_path);
    if (!generated_code) {
        free(source_code);
        if (have_input_path) unlink(input_path);
        if (have_output_path) unlink(output_path);
        rxpp_sh_set_error_tree(cb, "RXPP parser wrapper could not read generated CREXX.");
        return;
    }

    if (cb->parse_tree) {
        cb_free_token_buffer(cb->parse_tree);
        cb->parse_tree = 0;
    }
    rxc_highlight_controller_parse_mapped_source(cb, generated_code, output_path, input_path);
    diagnostic_tree = cb->parse_tree;
    cb->parse_tree = 0;
    if (diagnostic_tree) cb_walk_tree_top_down(diagnostic_tree, rxpp_sh_collect_diagnostics_node, &diagnostics);

    cb->parse_tree = rxpp_sh_build_tree(cb, source_code, &diagnostics);

    if (diagnostic_tree) cb_free_token_buffer(diagnostic_tree);
    rxpp_sh_diagnostic_list_free(&diagnostics);
    free(source_code);
    free(generated_code);
    if (have_input_path) unlink(input_path);
    if (have_output_path) unlink(output_path);
}

int main(int argc, char **argv) {
    CodeBuffer *cb;
    int i;
    int debug_mode;
    const char *rxpp_config;

    debug_mode = 0;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) debug_mode = 1;
        if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: rxpp-sh [--syntaxhighlight]\n");
            printf("Environment overrides: RXPP_SH_RXPP=/path/to/rxpp RXPP_SH_MACLIB=/path/to/maclib.rexx\n");
            return 0;
        }
    }

    if (debug_mode) cb_log_init("rxpp-sh.log");

    rxpp_config =
            "[.rxpp]\n"
            "keywords=options,import,say,if,then,else,select,when,otherwise,do,end,to,by,for,while,until,forever,leave,iterate,procedure,expose,return,exit,pull,parse,arg\n"
            "operators=+,-,*,/,=,<,>,(,),{,},,,;\n"
            "line_comment=--\n"
            "block_start=/*\n"
            "block_end=*/\n"
            "quotes=\"\n"
            "prefix_tokens=##:preprocessor\n"
            "span_tokens={:}:macro_variable\n";
    cb_set_ep_config_string(rxpp_config);

    cb = create_code_buffer(0, rxpp_sh_parse);
    if (!cb) {
        if (debug_mode) cb_log_close();
        return 1;
    }

    cb_start_stdio_server(cb);

    rxpp_sh_free_code_buffer(cb);
    if (debug_mode) cb_log_close();
    return 0;
}

#else

#include <stdio.h>

int main(void) {
    fprintf(stderr, "rxpp-sh requires ENABLE_PARSER_MODE.\n");
    return 1;
}

#endif
