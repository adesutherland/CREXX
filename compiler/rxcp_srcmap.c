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
 * Raw source-map prepass for preprocessed CREXX input.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "rxcpmain.h"
#include "rxcp_srcmap.h"

typedef struct RxcpSrcMapLineText {
    const char *file_name;
    int line;
    char *text;
    size_t text_length;
} RxcpSrcMapLineText;

typedef struct RxcpSrcMapEntry {
    size_t generated_start;
    size_t generated_end;
    const char *file_name;
    int line;
    int column;
    int length;
    const char *line_text;
    size_t line_text_length;
} RxcpSrcMapEntry;

typedef struct RxcpSrcMapSpan {
    size_t generated_start;
    const char *file_name;
    int line;
    int column;
    int length;
    const char *line_text;
    size_t line_text_length;
    int marker_line;
    int marker_column;
    const char *marker_line_start;
} RxcpSrcMapSpan;

typedef struct RxcpSrcMapLineSpan {
    int active;
    size_t generated_start;
    const char *file_name;
    int line;
    int column;
    const char *line_text;
    size_t line_text_length;
} RxcpSrcMapLineSpan;

struct RxcpSrcMap {
    char **file_names;
    size_t file_name_count;
    size_t file_name_capacity;
    RxcpSrcMapLineText *line_texts;
    size_t line_text_count;
    size_t line_text_capacity;
    RxcpSrcMapEntry *entries;
    size_t entry_count;
    size_t entry_capacity;
};

static char *srcmap_strndup(const char *text, size_t length) {
    char *copy;

    copy = malloc(length + 1);
    if (!copy) RX_PANIC_OOM("malloc rxc source map string", length + 1, text);
    if (length) memcpy(copy, text, length);
    copy[length] = 0;
    return copy;
}

static char *srcmap_strdup(const char *text) {
    if (!text) text = "";
    return srcmap_strndup(text, strlen(text));
}

void rxcp_srcmap_raw_mapping_free(RxcpSrcMapRawMapping *mapping) {
    if (!mapping) return;
    free(mapping->cleaned_to_raw_start);
    free(mapping->cleaned_to_raw_end);
    memset(mapping, 0, sizeof(*mapping));
}

static int srcmap_reserve_files(RxcpSrcMap *map) {
    char **new_files;
    size_t new_capacity;

    if (map->file_name_count < map->file_name_capacity) return 1;
    new_capacity = map->file_name_capacity ? map->file_name_capacity * 2 : 4;
    new_files = realloc(map->file_names, sizeof(char *) * new_capacity);
    if (!new_files) return 0;
    map->file_names = new_files;
    map->file_name_capacity = new_capacity;
    return 1;
}

static const char *srcmap_add_file(RxcpSrcMap *map, const char *file_name) {
    char *copy;

    if (!map) return 0;
    if (!srcmap_reserve_files(map)) {
        RX_PANIC_OOM("realloc rxc source map files",
                     sizeof(char *) * (map->file_name_capacity ? map->file_name_capacity * 2 : 4),
                     file_name);
    }
    copy = srcmap_strdup(file_name ? file_name : "<unknown>");
    map->file_names[map->file_name_count++] = copy;
    return copy;
}

static int srcmap_reserve_line_texts(RxcpSrcMap *map) {
    RxcpSrcMapLineText *new_texts;
    size_t new_capacity;

    if (map->line_text_count < map->line_text_capacity) return 1;
    new_capacity = map->line_text_capacity ? map->line_text_capacity * 2 : 8;
    new_texts = realloc(map->line_texts, sizeof(RxcpSrcMapLineText) * new_capacity);
    if (!new_texts) return 0;
    map->line_texts = new_texts;
    map->line_text_capacity = new_capacity;
    return 1;
}

static const RxcpSrcMapLineText *srcmap_find_line_text(RxcpSrcMap *map,
                                                        const char *file_name,
                                                        int line) {
    size_t i;

    if (!map || !file_name || line < 0) return 0;
    for (i = map->line_text_count; i > 0; i--) {
        if (map->line_texts[i - 1].line == line &&
            strcmp(map->line_texts[i - 1].file_name, file_name) == 0) {
            return &map->line_texts[i - 1];
        }
    }
    return 0;
}

static const RxcpSrcMapLineText *srcmap_add_line_text(RxcpSrcMap *map,
                                                       const char *file_name,
                                                       int line,
                                                       char *text,
                                                       size_t text_length) {
    RxcpSrcMapLineText *entry;

    if (!map || !file_name || !text || line < 0) return 0;
    if (!srcmap_reserve_line_texts(map)) {
        RX_PANIC_OOM("realloc rxc source map line text",
                     sizeof(RxcpSrcMapLineText) * (map->line_text_capacity ? map->line_text_capacity * 2 : 8),
                     file_name);
    }
    entry = &map->line_texts[map->line_text_count++];
    entry->file_name = file_name;
    entry->line = line;
    entry->text = text;
    entry->text_length = text_length;
    return entry;
}

static int srcmap_reserve_entries(RxcpSrcMap *map) {
    RxcpSrcMapEntry *new_entries;
    size_t new_capacity;

    if (map->entry_count < map->entry_capacity) return 1;
    new_capacity = map->entry_capacity ? map->entry_capacity * 2 : 16;
    new_entries = realloc(map->entries, sizeof(RxcpSrcMapEntry) * new_capacity);
    if (!new_entries) return 0;
    map->entries = new_entries;
    map->entry_capacity = new_capacity;
    return 1;
}

static void srcmap_add_entry(RxcpSrcMap *map, RxcpSrcMapSpan *span, size_t generated_end) {
    RxcpSrcMapEntry *entry;

    if (!map || !span) return;
    if (generated_end <= span->generated_start) return;
    if (!srcmap_reserve_entries(map)) {
        RX_PANIC_OOM("realloc rxc source map entries",
                     sizeof(RxcpSrcMapEntry) * (map->entry_capacity ? map->entry_capacity * 2 : 16),
                     span->file_name);
    }
    entry = &map->entries[map->entry_count++];
    entry->generated_start = span->generated_start;
    entry->generated_end = generated_end;
    entry->file_name = span->file_name;
    entry->line = span->line;
    entry->column = span->column;
    entry->length = span->length;
    entry->line_text = span->line_text;
    entry->line_text_length = span->line_text_length;
}

static void srcmap_add_line_entry(RxcpSrcMap *map,
                                  size_t generated_start,
                                  size_t generated_end,
                                  const char *file_name,
                                  int line,
                                  int column,
                                  const char *line_text,
                                  size_t line_text_length) {
    RxcpSrcMapSpan span;

    if (!map || !file_name || line < 0 || column < 0) return;
    if (generated_end <= generated_start) return;

    memset(&span, 0, sizeof(span));
    span.generated_start = generated_start;
    span.file_name = file_name;
    span.line = line;
    span.column = column;
    span.length = (int)(generated_end - generated_start);
    span.line_text = line_text;
    span.line_text_length = line_text_length;
    srcmap_add_entry(map, &span, generated_end);
}

static void srcmap_open_line_span(RxcpSrcMapLineSpan *span,
                                  size_t generated_start,
                                  const char *file_name,
                                  int line,
                                  int column,
                                  const char *line_text,
                                  size_t line_text_length) {
    if (!span || span->active) return;
    if (!file_name || line < 0 || column < 0) return;
    if (!line_text) return;
    span->active = 1;
    span->generated_start = generated_start;
    span->file_name = file_name;
    span->line = line;
    span->column = column;
    span->line_text = line_text;
    span->line_text_length = line_text_length;
}

static void srcmap_close_line_span(RxcpSrcMap *map,
                                   RxcpSrcMapLineSpan *span,
                                   size_t generated_end) {
    if (!span || !span->active) return;
    srcmap_add_line_entry(map,
                          span->generated_start,
                          generated_end,
                          span->file_name,
                          span->line,
                          span->column,
                          span->line_text,
                          span->line_text_length);
    memset(span, 0, sizeof(*span));
}

static int srcmap_reserve_spans(RxcpSrcMapSpan **spans, size_t *capacity, size_t count) {
    RxcpSrcMapSpan *new_spans;
    size_t new_capacity;

    if (count < *capacity) return 1;
    new_capacity = *capacity ? *capacity * 2 : 8;
    new_spans = realloc(*spans, sizeof(RxcpSrcMapSpan) * new_capacity);
    if (!new_spans) return 0;
    *spans = new_spans;
    *capacity = new_capacity;
    return 1;
}

static const char *srcmap_line_end(const char *line_start, const char *buffer_end) {
    const char *cursor;

    cursor = line_start;
    while (cursor < buffer_end && *cursor != '\n' && *cursor != '\r' && *cursor != 0) cursor++;
    return cursor;
}

static void srcmap_append_diagnostic(Context *context,
                                     const char *code,
                                     const char *reason,
                                     int line,
                                     int column,
                                     const char *line_start,
                                     const char *buffer_end) {
    RxcpDiagnostic *diagnostic;
    ASTNode *diag;
    ASTNode *tail;
    const char *line_end;

    if (!context || !code) return;
    diagnostic = rxcp_diag_create(code);
    if (diagnostic && reason) rxcp_diag_add_param(diagnostic, "reason", reason);

    diag = ast_ft(context, ERROR);
    ast_set_diagnostic(diag, diagnostic);
    diag->file_name = context->file_name;
    diag->line = line;
    diag->column = column;
    diag->source_start = (char *)(line_start ? line_start : context->buff_start);
    if (diag->source_start) {
        line_end = srcmap_line_end(diag->source_start, buffer_end ? buffer_end : context->buff_end);
        if (line_end > diag->source_start) diag->source_end = (char *)(line_end - 1);
        else diag->source_end = diag->source_start;
    }

    if (!context->diagnostics_list) {
        context->diagnostics_list = diag;
        return;
    }

    tail = (ASTNode *)context->diagnostics_list;
    while (tail->sibling) tail = tail->sibling;
    tail->sibling = diag;
}

static void srcmap_advance_raw(const char *buffer,
                               size_t length,
                               size_t start,
                               size_t end,
                               int *line,
                               int *column,
                               const char **line_start) {
    size_t i;

    for (i = start; i < end && i < length; i++) {
        if (buffer[i] == '\n') {
            (*line)++;
            *column = 0;
            if (line_start) *line_start = buffer + i + 1;
        } else {
            (*column)++;
        }
    }
}

static int srcmap_parse_quoted(const char *buffer,
                               size_t length,
                               size_t quote_pos,
                               size_t *end_out,
                               char **value_out,
                               size_t *value_length_out,
                               const char **reason_out) {
    char *value;
    size_t read_pos;
    size_t write_pos;
    size_t capacity;

    if (reason_out) *reason_out = 0;
    if (quote_pos >= length || buffer[quote_pos] != '"') {
        if (reason_out) *reason_out = "expected quoted payload";
        return 0;
    }

    capacity = length - quote_pos;
    value = malloc(capacity ? capacity : 1);
    if (!value) RX_PANIC_OOM("malloc rxc source map quoted payload", capacity, buffer + quote_pos);

    read_pos = quote_pos + 1;
    write_pos = 0;
    while (read_pos < length) {
        char ch;

        ch = buffer[read_pos];
        if (ch == '\n' || ch == '\r') {
            free(value);
            if (reason_out) *reason_out = "newline in quoted payload";
            return 0;
        }
        if (ch == '"') {
            if (read_pos + 1 < length && buffer[read_pos + 1] == '"') {
                value[write_pos++] = '"';
                read_pos += 2;
                continue;
            }
            value[write_pos] = 0;
            if (end_out) *end_out = read_pos + 1;
            if (value_out) *value_out = value;
            else free(value);
            if (value_length_out) *value_length_out = write_pos;
            return 1;
        }
        if (ch == '@' && read_pos + 1 < length && buffer[read_pos + 1] == '@') {
            value[write_pos++] = '@';
            read_pos += 2;
            continue;
        }
        value[write_pos++] = ch;
        read_pos++;
    }

    free(value);
    if (reason_out) *reason_out = "unterminated quoted payload";
    return 0;
}

static int srcmap_parse_signed_number(const char *buffer,
                                      size_t length,
                                      size_t start,
                                      size_t *end_out,
                                      int *value_out,
                                      int *is_relative_out) {
    int sign;
    int is_relative;
    long value;
    size_t pos;
    int saw_digit;

    pos = start;
    sign = 1;
    is_relative = 0;
    if (pos < length && (buffer[pos] == '+' || buffer[pos] == '-')) {
        is_relative = 1;
        if (buffer[pos] == '-') sign = -1;
        pos++;
    }

    value = 0;
    saw_digit = 0;
    while (pos < length && isdigit((unsigned char)buffer[pos])) {
        saw_digit = 1;
        value = value * 10 + (buffer[pos] - '0');
        pos++;
    }

    if (!saw_digit) return 0;
    if (end_out) *end_out = pos;
    if (value_out) *value_out = (int)(value * sign);
    if (is_relative_out) *is_relative_out = is_relative;
    return 1;
}

static int srcmap_parse_length(const char *buffer,
                               size_t length,
                               size_t start,
                               size_t *end_out,
                               int *value_out) {
    long value;
    size_t pos;
    int saw_digit;

    pos = start;
    value = 0;
    saw_digit = 0;
    while (pos < length && isdigit((unsigned char)buffer[pos])) {
        saw_digit = 1;
        value = value * 10 + (buffer[pos] - '0');
        pos++;
    }

    if (!saw_digit) return 0;
    if (end_out) *end_out = pos;
    if (value_out) *value_out = (int)value;
    return 1;
}

static int srcmap_decode_line(Context *context,
                              RxcpSrcMap *map,
                              const char *buffer,
                              size_t length,
                              size_t marker_pos,
                              size_t number_end,
                              int number_value,
                              int is_relative,
                              const char *current_file,
                              int *current_line,
                              int *current_column,
                              int *line_column,
                              const char **current_line_text,
                              size_t *current_line_text_length,
                              size_t *directive_end,
                              int marker_line,
                              int marker_column,
                              const char *marker_line_start) {
    const RxcpSrcMapLineText *line_text;
    char *decoded_text;
    const char *reason;
    size_t decoded_length;
    size_t end;
    int new_line;

    if (is_relative) new_line = *current_line + number_value;
    else new_line = number_value - 1;

    if (new_line < 0) {
        srcmap_append_diagnostic(context,
                                 "SRCMAP_MALFORMED",
                                 "line number is before the start of the source",
                                 marker_line,
                                 marker_column,
                                 marker_line_start,
                                 buffer + length);
        return 0;
    }

    *current_line = new_line;
    *current_line_text = 0;
    *current_line_text_length = 0;
    *current_column = 0;
    if (line_column) *line_column = 0;
    end = number_end + 1;

    if (end < length && buffer[end] == '"') {
        decoded_text = 0;
        decoded_length = 0;
        reason = 0;
        if (!srcmap_parse_quoted(buffer, length, end, &end, &decoded_text, &decoded_length, &reason)) {
            srcmap_append_diagnostic(context,
                                     "SRCMAP_MALFORMED",
                                     reason ? reason : "invalid quoted line text",
                                     marker_line,
                                     marker_column,
                                     marker_line_start,
                                     buffer + length);
            return 0;
        }
        line_text = srcmap_add_line_text(map, current_file, *current_line, decoded_text, decoded_length);
        if (line_text) {
            *current_line_text = line_text->text;
            *current_line_text_length = line_text->text_length;
        }
    } else {
        line_text = srcmap_find_line_text(map, current_file, *current_line);
        if (line_text) {
            *current_line_text = line_text->text;
            *current_line_text_length = line_text->text_length;
        }
    }

    if (directive_end) *directive_end = end;
    return 1;
}

static int srcmap_decode_column(Context *context,
                                const char *buffer,
                                size_t length,
                                size_t number_end,
                                int number_value,
                                int is_relative,
                                int *current_column,
                                int *line_column,
                                size_t *directive_end,
                                int marker_line,
                                int marker_column,
                                const char *marker_line_start) {
    int new_column;

    if (is_relative) new_column = *current_column + number_value;
    else new_column = number_value - 1;

    if (new_column < 0) {
        srcmap_append_diagnostic(context,
                                 "SRCMAP_MALFORMED",
                                 "column number is before the start of the source",
                                 marker_line,
                                 marker_column,
                                 marker_line_start,
                                 buffer + length);
        return 0;
    }

    *current_column = new_column;
    if (line_column) *line_column = new_column;
    if (directive_end) *directive_end = number_end + 1;
    return 1;
}

static int srcmap_decode_span(Context *context,
                              const char *buffer,
                              size_t length,
                              size_t output_pos,
                              size_t number_end,
                              int number_value,
                              int is_relative,
                              int current_column,
                              const char *current_file,
                              int current_line,
                              const char *current_line_text,
                              size_t current_line_text_length,
                              RxcpSrcMapSpan **spans,
                              size_t *span_count,
                              size_t *span_capacity,
                              size_t *directive_end,
                              int marker_line,
                              int marker_column,
                              const char *marker_line_start) {
    RxcpSrcMapSpan *span;
    size_t length_end;
    int source_column;
    int source_length;

    if (is_relative) source_column = current_column + number_value;
    else source_column = number_value - 1;

    if (source_column < 0) {
        srcmap_append_diagnostic(context,
                                 "SRCMAP_MALFORMED",
                                 "span column is before the start of the source",
                                 marker_line,
                                 marker_column,
                                 marker_line_start,
                                 buffer + length);
        return 0;
    }

    if (number_end >= length || buffer[number_end] != '+' ||
        !srcmap_parse_length(buffer, length, number_end + 1, &length_end, &source_length)) {
        srcmap_append_diagnostic(context,
                                 "SRCMAP_MALFORMED",
                                 "span length is missing",
                                 marker_line,
                                 marker_column,
                                 marker_line_start,
                                 buffer + length);
        return 0;
    }

    if (source_length <= 0) {
        srcmap_append_diagnostic(context,
                                 "SRCMAP_MALFORMED",
                                 "span length must be positive",
                                 marker_line,
                                 marker_column,
                                 marker_line_start,
                                 buffer + length);
        return 0;
    }

    if (length_end >= length || buffer[length_end] != '{') {
        srcmap_append_diagnostic(context,
                                 "SRCMAP_MALFORMED",
                                 "span directive is missing opening brace",
                                 marker_line,
                                 marker_column,
                                 marker_line_start,
                                 buffer + length);
        return 0;
    }

    if (!srcmap_reserve_spans(spans, span_capacity, *span_count)) {
        RX_PANIC_OOM("realloc rxc source map span stack",
                     sizeof(RxcpSrcMapSpan) * (*span_capacity ? *span_capacity * 2 : 8),
                     current_file);
    }
    span = &(*spans)[(*span_count)++];
    span->generated_start = output_pos;
    span->file_name = current_file;
    span->line = current_line;
    span->column = source_column;
    span->length = source_length;
    span->line_text = current_line_text;
    span->line_text_length = current_line_text_length;
    span->marker_line = marker_line;
    span->marker_column = marker_column;
    span->marker_line_start = marker_line_start;
    if (directive_end) *directive_end = length_end + 1;
    return 1;
}

static int srcmap_decode_directive(Context *context,
                                   RxcpSrcMap *map,
                                   const char *buffer,
                                   size_t length,
                                   size_t marker_pos,
                                   size_t output_pos,
                                   const char **current_file,
                                   int *current_line,
                                   int *current_column,
                                   int *line_column,
                                   const char **current_line_text,
                                   size_t *current_line_text_length,
                                   RxcpSrcMapSpan **spans,
                                   size_t *span_count,
                                   size_t *span_capacity,
                                   size_t *directive_end,
                                   char *escaped_char_out,
                                   int marker_line,
                                   int marker_column,
                                   const char *marker_line_start) {
    char *decoded_file;
    const char *reason;
    size_t decoded_length;
    size_t number_end;
    int number_value;
    int is_relative;

    if (marker_pos + 1 >= length) {
        srcmap_append_diagnostic(context,
                                 "SRCMAP_MALFORMED",
                                 "marker at end of file",
                                 marker_line,
                                 marker_column,
                                 marker_line_start,
                                 buffer + length);
        return 0;
    }

    if (buffer[marker_pos + 1] == '@') {
        *escaped_char_out = '@';
        *directive_end = marker_pos + 2;
        return 1;
    }

    if (buffer[marker_pos + 1] == '}') {
        if (*span_count == 0) {
            srcmap_append_diagnostic(context,
                                     "SRCMAP_UNBALANCED",
                                     "closing span has no matching opening span",
                                     marker_line,
                                     marker_column,
                                     marker_line_start,
                                     buffer + length);
            return 0;
        }
        (*span_count)--;
        srcmap_add_entry(map, &(*spans)[*span_count], output_pos);
        *directive_end = marker_pos + 2;
        return 1;
    }

    if (buffer[marker_pos + 1] == '"') {
        decoded_file = 0;
        decoded_length = 0;
        reason = 0;
        if (!srcmap_parse_quoted(buffer, length, marker_pos + 1, directive_end,
                                 &decoded_file, &decoded_length, &reason)) {
            srcmap_append_diagnostic(context,
                                     "SRCMAP_MALFORMED",
                                     reason ? reason : "invalid quoted file name",
                                     marker_line,
                                     marker_column,
                                     marker_line_start,
                                     buffer + length);
            return 0;
        }
        *current_file = srcmap_add_file(map, decoded_file);
        free(decoded_file);
        *current_line_text = 0;
        *current_line_text_length = 0;
        return 1;
    }

    if (!srcmap_parse_signed_number(buffer, length, marker_pos + 1,
                                    &number_end, &number_value, &is_relative)) {
        srcmap_append_diagnostic(context,
                                 "SRCMAP_MALFORMED",
                                 "unescaped marker is not a source-map directive",
                                 marker_line,
                                 marker_column,
                                 marker_line_start,
                                 buffer + length);
        return 0;
    }

    if (number_end >= length) {
        srcmap_append_diagnostic(context,
                                 "SRCMAP_MALFORMED",
                                 "directive is incomplete",
                                 marker_line,
                                 marker_column,
                                 marker_line_start,
                                 buffer + length);
        return 0;
    }

    if (buffer[number_end] == 'l') {
        return srcmap_decode_line(context,
                                  map,
                                  buffer,
                                  length,
                                  marker_pos,
                                  number_end,
                                  number_value,
                                  is_relative,
                                  *current_file,
                                  current_line,
                                  current_column,
                                  line_column,
                                  current_line_text,
                                  current_line_text_length,
                                  directive_end,
                                  marker_line,
                                  marker_column,
                                  marker_line_start);
    }

    if (buffer[number_end] == 'c') {
        return srcmap_decode_column(context,
                                    buffer,
                                    length,
                                    number_end,
                                    number_value,
                                    is_relative,
                                    current_column,
                                    line_column,
                                    directive_end,
                                    marker_line,
                                    marker_column,
                                    marker_line_start);
    }

    if (buffer[number_end] == '+') {
        return srcmap_decode_span(context,
                                  buffer,
                                  length,
                                  output_pos,
                                  number_end,
                                  number_value,
                                  is_relative,
                                  *current_column,
                                  *current_file,
                                  *current_line,
                                  *current_line_text,
                                  *current_line_text_length,
                                  spans,
                                  span_count,
                                  span_capacity,
                                  directive_end,
                                  marker_line,
                                  marker_column,
                                  marker_line_start);
    }

    srcmap_append_diagnostic(context,
                             "SRCMAP_MALFORMED",
                             "unknown source-map directive",
                             marker_line,
                             marker_column,
                             marker_line_start,
                             buffer + length);
    return 0;
}

int rxcp_srcmap_preprocess(Context *context, char **cleaned_out, size_t *cleaned_len_out) {
    return rxcp_srcmap_preprocess_with_raw_map(context, cleaned_out, cleaned_len_out, 0);
}

int rxcp_srcmap_preprocess_with_raw_map(Context *context,
                                        char **cleaned_out,
                                        size_t *cleaned_len_out,
                                        RxcpSrcMapRawMapping *mapping_out) {
    RxcpSrcMap *map;
    RxcpSrcMapSpan *spans;
    RxcpSrcMapLineSpan line_span;
    const char *current_file;
    const char *current_line_text;
    const char *marker_line_start;
    const char *buffer;
    char *cleaned;
    size_t length;
    size_t read_pos;
    size_t write_pos;
    size_t span_count;
    size_t span_capacity;
    size_t directive_end;
    size_t marker_pos;
    size_t current_line_text_length;
    size_t *cleaned_to_raw_start;
    size_t *cleaned_to_raw_end;
    int current_line;
    int current_column;
    int line_column;
    int raw_line;
    int raw_column;
    int marker_line;
    int marker_column;
    char escaped_char;

    if (cleaned_out) *cleaned_out = 0;
    if (cleaned_len_out) *cleaned_len_out = 0;
    if (mapping_out) memset(mapping_out, 0, sizeof(*mapping_out));
    if (!context || !context->buff_start || !context->buff_end) return -1;

    buffer = context->buff_start;
    length = (size_t)(context->buff_end - context->buff_start);
    map = calloc(1, sizeof(RxcpSrcMap));
    if (!map) RX_PANIC_OOM("calloc rxc source map", sizeof(RxcpSrcMap), context->file_name);
    current_file = srcmap_add_file(map, context->file_name ? context->file_name : "<unknown>");
    current_line = 0;
    current_column = 0;
    line_column = 0;
    current_line_text = 0;
    current_line_text_length = 0;
    spans = 0;
    span_count = 0;
    span_capacity = 0;

    cleaned = malloc(length + 1);
    if (!cleaned) RX_PANIC_OOM("malloc rxc source map cleaned buffer", length + 1, context->file_name);
    cleaned_to_raw_start = 0;
    cleaned_to_raw_end = 0;
    if (mapping_out) {
        cleaned_to_raw_start = malloc(sizeof(size_t) * (length + 1));
        cleaned_to_raw_end = malloc(sizeof(size_t) * (length + 1));
        if (!cleaned_to_raw_start || !cleaned_to_raw_end) {
            free(cleaned_to_raw_start);
            free(cleaned_to_raw_end);
            free(cleaned);
            rxcp_srcmap_free(map);
            RX_PANIC_OOM("malloc rxc source map raw mapping", sizeof(size_t) * (length + 1), context->file_name);
        }
    }

    read_pos = 0;
    write_pos = 0;
    memset(&line_span, 0, sizeof(line_span));
    raw_line = 0;
    raw_column = 0;
    marker_line_start = buffer;

    while (read_pos < length) {
        if (buffer[read_pos] != '@') {
            if (buffer[read_pos] == '\n' || buffer[read_pos] == '\r') {
                srcmap_close_line_span(map, &line_span, write_pos);
            } else if (span_count == 0) {
                srcmap_open_line_span(&line_span,
                                      write_pos,
                                      current_file,
                                      current_line,
                                      line_column,
                                      current_line_text,
                                      current_line_text_length);
            }
            if (mapping_out) {
                cleaned_to_raw_start[write_pos] = read_pos;
                cleaned_to_raw_end[write_pos] = read_pos + 1;
            }
            cleaned[write_pos++] = buffer[read_pos];
            if (buffer[read_pos] == '\n') {
                raw_line++;
                raw_column = 0;
                line_column = 0;
                marker_line_start = buffer + read_pos + 1;
            } else {
                raw_column++;
                if (buffer[read_pos] != '\r') line_column++;
            }
            read_pos++;
            continue;
        }

        srcmap_close_line_span(map, &line_span, write_pos);
        marker_line = raw_line;
        marker_column = raw_column;
        marker_pos = read_pos;
        directive_end = read_pos;
        escaped_char = 0;
        if (!srcmap_decode_directive(context,
                                     map,
                                     buffer,
                                     length,
                                     read_pos,
                                     write_pos,
                                     &current_file,
                                     &current_line,
                                     &current_column,
                                     &line_column,
                                     &current_line_text,
                                     &current_line_text_length,
                                     &spans,
                                     &span_count,
                                     &span_capacity,
                                     &directive_end,
                                     &escaped_char,
                                     marker_line,
                                     marker_column,
                                     marker_line_start)) {
            free(spans);
            free(cleaned_to_raw_start);
            free(cleaned_to_raw_end);
            free(cleaned);
            rxcp_srcmap_free(map);
            return -1;
        }

        srcmap_advance_raw(buffer,
                           length,
                           read_pos,
                           directive_end,
                           &raw_line,
                           &raw_column,
                           &marker_line_start);
        read_pos = directive_end;

        if (escaped_char) {
            if (span_count == 0) {
                srcmap_open_line_span(&line_span,
                                      write_pos,
                                      current_file,
                                      current_line,
                                      line_column,
                                      current_line_text,
                                      current_line_text_length);
            }
            if (mapping_out) {
                cleaned_to_raw_start[write_pos] = marker_pos;
                cleaned_to_raw_end[write_pos] = directive_end;
            }
            cleaned[write_pos++] = escaped_char;
            line_column++;
        }
    }
    srcmap_close_line_span(map, &line_span, write_pos);

    if (span_count > 0) {
        RxcpSrcMapSpan *span;

        span = &spans[span_count - 1];
        srcmap_append_diagnostic(context,
                                 "SRCMAP_UNBALANCED",
                                 "opening span has no matching closing span",
                                 span->marker_line,
                                 span->marker_column,
                                 span->marker_line_start,
                                 buffer + length);
        free(spans);
        free(cleaned_to_raw_start);
        free(cleaned_to_raw_end);
        free(cleaned);
        rxcp_srcmap_free(map);
        return -1;
    }

    cleaned[write_pos] = 0;
    if (mapping_out) {
        cleaned_to_raw_start[write_pos] = length;
        cleaned_to_raw_end[write_pos] = length;
    }
    free(spans);
    if (context->srcmap) rxcp_srcmap_free(context->srcmap);
    context->srcmap = map;
    if (cleaned_out) *cleaned_out = cleaned;
    if (cleaned_len_out) *cleaned_len_out = write_pos;
    if (mapping_out) {
        mapping_out->cleaned_to_raw_start = cleaned_to_raw_start;
        mapping_out->cleaned_to_raw_end = cleaned_to_raw_end;
        mapping_out->cleaned_len = write_pos;
        mapping_out->raw_len = length;
    }
    return 0;
}

static int srcmap_offset_from_line_column(Context *context,
                                          int generated_line,
                                          int generated_column,
                                          size_t *offset_out) {
    const char *cursor;
    int line;
    int column;

    if (!context || !context->buff_start || !context->buff_end ||
        generated_line < 0 || generated_column < 0 || !offset_out) {
        return 0;
    }

    cursor = context->buff_start;
    line = 0;
    column = 0;
    while (cursor < context->buff_end) {
        if (line == generated_line && column == generated_column) {
            *offset_out = (size_t)(cursor - context->buff_start);
            return 1;
        }
        if (*cursor == '\n') {
            line++;
            column = 0;
        } else {
            column++;
        }
        cursor++;
    }

    if (line == generated_line && column == generated_column) {
        *offset_out = (size_t)(cursor - context->buff_start);
        return 1;
    }
    return 0;
}

int rxcp_srcmap_lookup(Context *context,
                       const char *generated_ptr,
                       int generated_line,
                       int generated_column,
                       RxcpSrcMapLocation *location_out) {
    RxcpSrcMap *map;
    RxcpSrcMapEntry *best;
    size_t offset;
    size_t i;
    size_t best_width;

    if (location_out) memset(location_out, 0, sizeof(RxcpSrcMapLocation));
    if (!context || !context->srcmap || !location_out) return 0;
    map = context->srcmap;
    if (!map->entry_count) return 0;

    if (generated_ptr &&
        generated_ptr >= context->buff_start &&
        generated_ptr <= context->buff_end) {
        offset = (size_t)(generated_ptr - context->buff_start);
    } else {
        if (!srcmap_offset_from_line_column(context, generated_line, generated_column, &offset)) return 0;
    }

    best = 0;
    best_width = 0;
    for (i = 0; i < map->entry_count; i++) {
        RxcpSrcMapEntry *entry;
        size_t width;

        entry = &map->entries[i];
        if (offset < entry->generated_start || offset >= entry->generated_end) continue;
        width = entry->generated_end - entry->generated_start;
        if (!best || width <= best_width) {
            best = entry;
            best_width = width;
        }
    }

    if (!best) return 0;
    location_out->file_name = best->file_name;
    location_out->line = best->line;
    location_out->column = best->column;
    location_out->length = best->length;
    location_out->line_text = best->line_text;
    location_out->line_text_length = best->line_text_length;
    return 1;
}

void rxcp_srcmap_free(RxcpSrcMap *map) {
    size_t i;

    if (!map) return;
    if (map->file_names) {
        for (i = 0; i < map->file_name_count; i++) free(map->file_names[i]);
        free(map->file_names);
    }
    if (map->line_texts) {
        for (i = 0; i < map->line_text_count; i++) free(map->line_texts[i].text);
        free(map->line_texts);
    }
    if (map->entries) free(map->entries);
    free(map);
}
