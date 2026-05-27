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
 * Emitter Core: Fragment management, File I/O, Promotion Matrix
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxcp_source_tree.h"
#include "rxbin.h"

/* Type promotion matrix for numeric operators */
const char* emit_promotion[9][9] = {
/*                  TP_UNKNOWN,TP_VOID,TP_BOOLEAN,TP_INTEGER,TP_FLOAT, TP_DECIMAL, TP_STRING,TP_BINARY,TP_OBJECT */

/* TP_UNKNOWN */   {0,         0,      0,         0,         0,      0,         0,        0,        0},
/* TP_VOID    */   {0,         0,      0,         0,         0,      0,         0,        0,        0},
/* TP_BOOLEAN */   {0,         0,      0,         "btoi",    "btof", "btod",    "btos",   0,        0},
/* TP_INTEGER */   {0,         0,      0,         0,         "itof", "itod",    "itos",   0,        0},
/* TP_FLOAT   */   {0,         0,      "ftob",    "ftoi",    0,      "ftod",    "ftos",   0,        0},
/* TP_DECIMAL */   {0,         0,      "dtob",    "dtoi",    "dtof", 0,         "dtos",   0,        0},
/* TP_STRING  */   {0,         0,      "stoi",    "stoi",    "stof", "stod",    0,        "stobin", 0},
/* TP_BINARY  */   {0,         0,      0,         0,         0,      0,         "bintos", 0,        0},
/* TP_OBJECT  */   {0,         0,      0,         0,         0,      0,         0,        0,        0},
};

static unsigned int next_source_step_id = 1;

void reset_metaline_source_file(const char *file_name) {
    (void)file_name;
    next_source_step_id = 1;
}

static int source_span_is_emit_safe(const char *start, const char *end) {
    size_t length;

    if (!start || !end) return 0;
    if (end < start) return end + 1 == start;
    length = (size_t)(end - start) + 1;
    if (length > 2048) return 0;
    return memchr(start, 0, length) == NULL;
}

static char *empty_metaline(void) {
    char *result;

    result = malloc(1);
    if (result) result[0] = 0;
    return result;
}

static unsigned int source_step_flags(char provenance, int compiler_added, SourceNode *source_node) {
    unsigned int flags;

    flags = RXBIN_SOURCE_AUTHORED;
    switch ((ASTSourceProvenance)provenance) {
        case AST_SOURCE_EXACT:
            flags |= RXBIN_SOURCE_EXACT;
            break;
        case AST_SOURCE_INHERITED:
            flags |= RXBIN_SOURCE_INHERITED;
            break;
        case AST_SOURCE_SYNTHETIC:
            flags &= ~RXBIN_SOURCE_AUTHORED;
            flags |= RXBIN_SOURCE_SYNTHETIC | RXBIN_SOURCE_GENERATED;
            break;
        case AST_SOURCE_COMPOSITE:
            flags |= RXBIN_SOURCE_COMPOSITE;
            break;
        case AST_SOURCE_NONE:
        default:
            break;
    }
    if (compiler_added) flags |= RXBIN_SOURCE_GENERATED;
    if (source_node && source_node->owned_source_text) flags |= RXBIN_SOURCE_INLINED;
    return flags;
}

static int pointer_in_range(const char *ptr, const char *range_start, const char *range_end) {
    return ptr && range_start && range_end && ptr >= range_start && ptr <= range_end;
}

static int context_source_range(Context *context, const char *ptr, const char **range_start, const char **range_end) {
    if (!context || !context->buff_start || !context->buff_end || !ptr) return 0;
    if (ptr < context->buff_start || ptr > context->buff_end) return 0;
    if (range_start) *range_start = context->buff_start;
    if (range_end) *range_end = context->buff_end;
    return 1;
}

static int owned_source_range(SourceNode *source_node, const char *ptr, const char **range_start, const char **range_end) {
    size_t length;

    if (!source_node || !source_node->owned_source_text || !ptr) return 0;
    length = strlen(source_node->owned_source_text);
    if (!pointer_in_range(ptr, source_node->owned_source_text, source_node->owned_source_text + length)) return 0;
    if (range_start) *range_start = source_node->owned_source_text;
    if (range_end) *range_end = source_node->owned_source_text + length;
    return 1;
}

static int source_line_bounds(const char *ptr,
                              const char *range_start,
                              const char *range_end,
                              const char **line_start_out,
                              const char **line_end_out) {
    const char *line_start;
    const char *line_end;

    if (!ptr || !range_start || !range_end || ptr < range_start || ptr > range_end) return 0;
    if (ptr == range_end && ptr > range_start) ptr--;

    line_start = ptr;
    while (line_start > range_start &&
           line_start[-1] != '\n' &&
           line_start[-1] != '\r') {
        line_start--;
    }

    line_end = ptr;
    while (line_end < range_end &&
           *line_end != '\n' &&
           *line_end != '\r' &&
           *line_end != 0) {
        line_end++;
    }

    if (line_start_out) *line_start_out = line_start;
    if (line_end_out) *line_end_out = line_end;
    return 1;
}

static int source_anchor_range(Context *context,
                               SourceNode *source_node,
                               const char *source_start,
                               const char **range_start,
                               const char **range_end) {
    if (owned_source_range(source_node, source_start, range_start, range_end)) return 1;
    if (context_source_range(context, source_start, range_start, range_end)) return 1;
    if (source_node && context_source_range(source_node->context, source_start, range_start, range_end)) return 1;
    return 0;
}

static char *source_step_metaline(Context *context,
                                  SourceNode *source_node,
                                  const char *file_name,
                                  int line,
                                  int column,
                                  const char *source_start,
                                  const char *source_end,
                                  unsigned int flags) {
    const char *range_start;
    const char *range_end;
    const char *line_start;
    const char *line_end;
    const char *active_start;
    const char *active_end;
    size_t line_length;
    unsigned int step_id;
    int active_start_column;
    int active_end_column;
    char *encoded_file;
    char *encoded_line;
    char *result;

    if (!source_start || !source_end || line < 0) return empty_metaline();
    if (!source_span_is_emit_safe(source_start, source_end)) return empty_metaline();

    range_start = 0;
    range_end = 0;
    line_start = source_start;
    line_end = source_end >= source_start ? source_end + 1 : source_start;
    active_start = source_start;
    active_end = source_end >= source_start ? source_end : source_start;

    if (source_anchor_range(context, source_node, source_start, &range_start, &range_end) &&
        source_line_bounds(source_start, range_start, range_end, &line_start, &line_end)) {
        if (active_start < line_start) active_start = line_start;
        if (active_start > line_end) active_start = line_end;
        if (active_end < active_start) active_end = active_start;
        if (active_end > line_end) active_end = line_end;
        active_start_column = (int)(active_start - line_start) + 1;
        active_end_column = source_end >= source_start ? (int)(active_end - line_start) + 2 : active_start_column;
    } else {
        if (column < 0) column = 0;
        active_start_column = 1;
        active_end_column = source_end >= source_start ? (int)(line_end - line_start) + 1 : active_start_column;
    }

    line_length = (size_t)(line_end - line_start);
    if (line_length == 0) return empty_metaline();
    if (line_length > 4096 || memchr(line_start, 0, line_length)) return empty_metaline();

    if (active_end_column < active_start_column) active_end_column = active_start_column;
    if (active_end_column > (int)line_length + 1) active_end_column = (int)line_length + 1;
    if (active_start_column < 1) active_start_column = 1;
    if (active_start_column > (int)line_length + 1) active_start_column = (int)line_length + 1;

    encoded_file = encode_line_source_malloc(file_name ? file_name : "", file_name ? strlen(file_name) : 0);
    encoded_line = encode_line_source_malloc(line_start, (int)line_length);
    if (!encoded_file || !encoded_line) {
        if (encoded_file) free(encoded_file);
        if (encoded_line) free(encoded_line);
        return empty_metaline();
    }

    if (!flags) flags = RXBIN_SOURCE_AUTHORED;
    step_id = next_source_step_id++;
    result = mprintf("   .srcstep %u %u %u \"%s\" %d %d %d \"%s\"\n",
                     step_id,
                     step_id,
                     flags,
                     encoded_file,
                     line + 1,
                     active_start_column,
                     active_end_column,
                     encoded_line);
    free(encoded_file);
    free(encoded_line);
    return result;
}

/* Output Marshalling Functions */
OutputFragment *output_f(){
    OutputFragment *output = malloc(sizeof(OutputFragment));
    output->output = 0;
    output->after = NULL;
    output->before = NULL;
    return output;
}

OutputFragment *output_fs(char* text) {
    OutputFragment *output = malloc(sizeof(OutputFragment));
    output->after = NULL;
    output->before = NULL;
    output->output = malloc(strlen(text) + 1);
    strcpy(output->output, text);
    return output;
}

void f_output(OutputFragment *output) {
    if (!output) return;
    /* Note: We do not iterate through the 'after' chain here because fragments are shared
     * between multiple ASTNodes. Iterating and freeing the entire chain would lead to
     * double-frees when other nodes pointing to fragments in the same chain are later freed.
     * The AST free walker calls f_output for every node, so every fragment that is
     * associated with a node will eventually be freed exactly once. */
    if (output->output) free(output->output);
    free(output);
}

void output_insert_before(OutputFragment* existing, OutputFragment* before) {
    before->before = existing->before;
    before->after = existing;
    existing->before = before;
}

void output_insert_after(OutputFragment* existing, OutputFragment* after) {
    after->after = existing->after;
    after->before = existing;
    existing->after = after;
}

void output_concat(OutputFragment* before, OutputFragment* after) {
    if (before) {
        while (before->after) before = before->after;
    }
    if (after) {
        while (after->before) after = after->before;
    }
    if (before) before->after = after;
    if (after) after->before = before;
}

void output_append_text(OutputFragment* before, char* after) {
    while (before->after) before = before->after;
    if (before->output) {
        before->output = realloc(before->output, strlen(before->output) + strlen(after) + 1);
        strcat(before->output, after);
    }
    else {
        before->output = malloc(strlen(after) + 1);
        strcpy(before->output, after);
    }
}

void output_prepend_text(char* before, OutputFragment* after) {
    char* buffer;
    while (after->before) after = after->before;
    if (after->output) {
        buffer = malloc(strlen(after->output) + strlen(before) + 1);
        strcpy(buffer, before);
        strcat(buffer, after->output);
        free(after->output);
        after->output = buffer;
    }
    else {
        after->output = malloc(strlen(before) + 1);
        strcpy(after->output, before);
    }
}

void print_output(FILE* file, OutputFragment* existing) {
    while (existing) {
        if (existing->output) fputs(existing->output, file);
        existing = existing->after;
    }
}

static char *get_source_node_metaline(SourceNode *node) {
    int line;
    int column;
    char *source_start;
    char *source_end;

    line = -1;
    column = -1;
    source_start = 0;
    source_end = 0;

    if (!node) {
        return empty_metaline();
    }

    line = node->line;
    column = node->column;
    source_start = node->source_start;
    source_end = node->source_end;

    if (node->token) {
        if (line == -1) line = node->token->line;
        if (column == -1) column = node->token->column;
        if (!source_start) source_start = node->token->token_string;
        if (!source_end) source_end = node->token->token_string + node->token->length - 1;
    }

    return source_step_metaline(node->context,
                                node,
                                node->file_name,
                                line,
                                column,
                                source_start,
                                source_end,
                                source_step_flags(AST_SOURCE_EXACT, 0, node));
}

static void append_metaline_buffer(char **buffer, size_t *buffer_len, char *line) {
    size_t line_len;

    if (!buffer || !buffer_len || !line) return;
    line_len = strlen(line);
    if (line_len == 0) {
        free(line);
        return;
    }

    *buffer = realloc(*buffer, *buffer_len + line_len);
    memcpy(*buffer + (*buffer_len - 1), line, line_len + 1);
    *buffer_len += line_len;
    free(line);
}

char* get_reporting_metalines(ASTNode *node) {
    char *result;
    char *line;
    size_t buffer_len;
    size_t i;

    result = malloc(1);
    result[0] = 0;
    buffer_len = 1;

    if (!node) return result;

    if (node->emit_primary_reporting_anchor && node->source_node) {
        line = get_source_node_metaline(node->source_node);
        append_metaline_buffer(&result, &buffer_len, line);
    }

    for (i = 0; i < node->reporting_source_count; i++) {
        line = get_source_node_metaline(node->reporting_source_nodes[i]);
        append_metaline_buffer(&result, &buffer_len, line);
    }

    return result;
}

/* Returns the source-step metadata line in a malloced buffer */
char* get_metaline(ASTNode *node) {
    int line, column;
    char *source_start, *source_end;

    if (node->is_compiler_added &&
        node->source_provenance == AST_SOURCE_SYNTHETIC &&
        node->node_type == INSTRUCTIONS) {
        return empty_metaline();
    }

    line = node->line;
    column = node->column;
    source_start = node->source_start;
    source_end = node->source_end;

    /* Try and set error position if not already set */
    if (node->token) {
        if (line == -1) line = node->token->line;
        if (column == -1) column = node->token->column;
        if (!source_start) source_start = node->token->token_string;
        if (!source_end) source_end = node->token->token_string + node->token->length - 1;
    }

    return source_step_metaline(node->context,
                                node->source_node,
                                node->file_name,
                                line,
                                column,
                                source_start,
                                source_end,
                                source_step_flags(node->source_provenance, node->is_compiler_added, node->source_node));
}

/* Returns the source-step metadata line in a malloced buffer */
char* get_metaline_range(ASTNode *from, ASTNode *to) {
    int from_line, from_column;
    char *from_source_start, *from_source_end;
    int to_line, to_column;
    char *to_source_start, *to_source_end;

    from_line = from->line;
    from_column = from->column;
    from_source_start = from->source_start;
    from_source_end = from->source_end;
    to_line = to->line;
    to_column = to->column;
    to_source_start = to->source_start;
    to_source_end = to->source_end;

    /* Try and set error position if not already set */
    if (from->token_start) {
        if (from_line == -1) from_line = from->token_start->line;
        if (from_column == -1) from_column = from->token_start->column;
        if (!from_source_start) from_source_start = from->token_start->token_string;
        if (!from_source_end) from_source_end = from->token_start->token_string + from->token_start->length - 1;
    }
    if (to->token_end) {
        if (to_line == -1) to_line = to->token_end->line;
        if (to_column == -1) to_column = to->token_end->column;
        if (!to_source_start) to_source_start = to->token_end->token_string;
        if (!to_source_end) to_source_end = to->token_end->token_string + to->token_end->length - 1;
    }

    (void)from_source_end;
    (void)to_line;
    (void)to_column;
    (void)to_source_start;
    return source_step_metaline(from->context,
                                from->source_node,
                                from->file_name,
                                from_line,
                                from_column,
                                from_source_start,
                                to_source_end,
                                source_step_flags(from->source_provenance, from->is_compiler_added, from->source_node));
}

/* Returns the source-step metadata line in a malloced buffer */
char* get_metaline_between(ASTNode *from, ASTNode *to) {
    Token *start = 0;
    Token *end = 0;

    if (from->token_end) start = from->token_end->token_next;
    if (to->token_start) end = to->token_start->token_prev;

    /* Get rid of leading and training newlines */
    if (start == end) {
        if (start->token_type == TK_EOC) start = 0; /* Weird empty string condition */
    }
    else {
        if (start->token_type == TK_EOC) {
            /* Skip the newline */
            start = start->token_next;
        }
        if (start != end) {
            if (end->token_type == TK_EOC) {
                /* Skip the newline */
                end = end->token_prev;
            }
        }
    }

    if (!start || !end ||
        !source_span_is_emit_safe(start->token_string, end->token_string + end->length - 1)) {
        return empty_metaline();
    }
    return source_step_metaline(from->context,
                                from->source_node,
                                from->file_name,
                                start->line,
                                start->column,
                                start->token_string,
                                end->token_string + end->length - 1,
                                source_step_flags(from->source_provenance, from->is_compiler_added, from->source_node));
}

/* Returns the source-step metadata line in a malloced buffer */
char* get_metaline_token_after(ASTNode *node) {
    Token *start = 0;

    if (node->token_end) start = node->token_end->token_next;

    /* Get rid of leading newline */
    if (start && start->token_type == TK_EOC) {
        /* Skip the newline */
        start = start->token_next;
    }

    if (!start || !source_span_is_emit_safe(start->token_string, start->token_string + start->length - 1)) {
        return empty_metaline();
    }
    return source_step_metaline(node->context,
                                node->source_node,
                                node->file_name,
                                start->line,
                                start->column,
                                start->token_string,
                                start->token_string + start->length - 1,
                                source_step_flags(node->source_provenance, node->is_compiler_added, node->source_node));
}

/* Returns the source-step metadata line in a malloced buffer */
char* get_metaline_clause(ASTNode *node) {
    Token *start = 0;
    Token *end = 0;

    if (node->token_start) start = node->token_start;

    if (!start || !source_span_is_emit_safe(start->token_string, start->token_string + start->length - 1)) {
        return empty_metaline();
    }
    else {
        end = start;
        while (end->token_next->token_type != TK_EOC && end->token_next->token_type != TK_EOS)
            end = end->token_next;

        if (!source_span_is_emit_safe(start->token_string, end->token_string + end->length - 1)) {
            return empty_metaline();
        } else {
            return source_step_metaline(node->context,
                                        node->source_node,
                                        node->file_name,
                                        start->line,
                                        start->column,
                                        start->token_string,
                                        end->token_string + end->length - 1,
                                        source_step_flags(node->source_provenance, node->is_compiler_added, node->source_node));
        }
    }
}

/* Returns the source-step metadata line in a malloced buffer */
char* get_metaline_token_at(ASTNode *node) {
    Token *start = 0;

    if (node->token_start) start = node->token_start;

    /* Get rid of leading newline - unlikely that there will be one! */
    if (start && start->token_type == TK_EOC) {
        /* Skip the newline */
        start = start->token_next;
    }

    if (!start) {
        return empty_metaline();
    }
    return source_step_metaline(node->context,
                                node->source_node,
                                node->file_name,
                                start->line,
                                start->column,
                                start->token_string,
                                start->token_string + start->length - 1,
                                source_step_flags(node->source_provenance, node->is_compiler_added, node->source_node));
}

/* Get Comment from a node (in a malloced buffer) */
char* get_comment(ASTNode *node, char* prefix) {
    char *result, *src;
    if (!node->source_start) {
        result = malloc(1);
        result[0] = 0;
    }
    else {
        src = encode_comment_malloc(node->source_start,
                                    (int) (node->source_end - node->source_start) + 1);
        if (prefix)
            result = mprintf("   * Line %d: %s %s\n", node->line + 1,
                             prefix, src);
        else
            result = mprintf("   * Line %d: %s\n", node->line + 1, src);
        free(src);
    }
    return result;
}

/* Comment without quoting node text (in a malloced buffer) */
char* get_comment_line_number_only(ASTNode *node, char* comment_text) {
    if (comment_text)
        return mprintf("   * Line %d: %s\n", node->line + 1, comment_text);
    else
        return mprintf("   * Line %d:\n", node->line + 1);
}

void type_promotion(ASTNode *node) {
    char *temp;

    if (emit_promotion[node->value_type][node->target_type]) { /* Check that there is a promotion */
        temp = mprintf("   %s %c%d\n",
                       emit_promotion[node->value_type][node->target_type],
                       node->register_type,
                       node->register_num);
        output_append_text(node->output, temp);
        free(temp);
    }
}

static int rxas_hex_value(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

static void append_hex_byte(char *buffer, size_t *pos, unsigned char byte) {
    static const char hex[] = "0123456789abcdef";
    buffer[(*pos)++] = hex[(byte >> 4) & 0x0f];
    buffer[(*pos)++] = hex[byte & 0x0f];
}

static char *format_rxas_escaped_string_as_binary(ASTNode *node) {
    char *buffer;
    size_t i;
    size_t out;
    unsigned char byte;
    char esc;
    int hi;
    int lo;

    buffer = malloc(3 + (node->node_string_length * 2));
    buffer[0] = '0';
    buffer[1] = 'x';
    out = 2;

    for (i = 0; i < node->node_string_length; i++) {
        byte = (unsigned char)node->node_string[i];
        if (byte == '\\' && i + 1 < node->node_string_length) {
            esc = node->node_string[++i];
            switch (esc) {
                case '\\': byte = '\\'; break;
                case 'n': byte = '\n'; break;
                case 't': byte = '\t'; break;
                case 'a': byte = '\a'; break;
                case 'b': byte = '\b'; break;
                case 'f': byte = '\f'; break;
                case 'r': byte = '\r'; break;
                case 'v': byte = '\v'; break;
                case '\'': byte = '\''; break;
                case '\"': byte = '\"'; break;
                case '0': byte = '\0'; break;
                case '?': byte = '\?'; break;
                case 'x':
                    if (i + 2 < node->node_string_length) {
                        hi = rxas_hex_value(node->node_string[i + 1]);
                        lo = rxas_hex_value(node->node_string[i + 2]);
                        if (hi != -1 && lo != -1) {
                            byte = (unsigned char)((hi << 4) | lo);
                            i += 2;
                            break;
                        }
                    }
                    append_hex_byte(buffer, &out, '\\');
                    byte = (unsigned char)esc;
                    break;
                default:
                    append_hex_byte(buffer, &out, '\\');
                    byte = (unsigned char)esc;
                    break;
            }
        }
        append_hex_byte(buffer, &out, byte);
    }

    buffer[out] = 0;
    return buffer;
}

/* Formats a constant value returned as a malloced buffer */
char* format_constant(ValueType type, ASTNode* node) {
    char *buffer;
    int flag;
    size_t i;

    if (type == TP_STRING) {
        buffer = mprintf("\"%.*s\"",
                         node->node_string_length,
                         node->node_string);
    }
    else if (type == TP_FLOAT) {
        /* Need to make sure the float literal has an ".0" (for the assembler) */
        flag = 1; /* Assume we should add .0 */
        for (i=0; i<node->node_string_length; i++) {
            if (node->node_string[i] == '.' || node->node_string[i] == 'e' || node->node_string[i] == 'E') {
                /* Already in a float format */
                flag = 0; /* don't add .0 */
                break;
            }
        }
        if (flag)
            buffer = mprintf("%.*s.0",
                             node->node_string_length,
                             node->node_string);
        else
            buffer = mprintf("%.*s",
                             node->node_string_length,
                             node->node_string);
    }
    else if (type == TP_DECIMAL) {
        buffer = mprintf("%.*sd",
                        node->node_string_length,
                        node->node_string);
    }
    else if (type == TP_BINARY) {
        if (node->node_type == BINARY || node->value_type == TP_BINARY) {
            buffer = mprintf("%.*s",
                             node->node_string_length,
                             node->node_string);
        }
        else {
            buffer = format_rxas_escaped_string_as_binary(node);
        }
    }
    else {
        /* Integer */
        buffer = mprintf("%.*s",
                         node->node_string_length,
                         node->node_string);
    }
    return buffer;
}

char* type_to_prefix(ValueType value_type) {
    switch (value_type) {
        case TP_BOOLEAN:
        case TP_INTEGER:
            return "i";
        case TP_STRING:
            return "s";
        case TP_FLOAT:
            return "f";
        case TP_DECIMAL:
            return "d";
        case TP_BINARY:
            return "";
        default:
            return "";
    }
}

/* Tests if a node is a constant */
int is_constant(ASTNode* node) {
    switch (node->node_type) {
        case CONSTANT: /* This is what the optimiser changes all constants to */
        case CONST_SYMBOL: /* Should not be being used in the AST at this stage - but for safety */
        case STRING: /* This and the following will exist if the optimiser has not been run */
        case FLOAT:
        case DECIMAL:
        case BINARY:
        case INTEGER:
        case CLASS:
            if (node->value_type == node->target_type)
                return 1; /* No type promotion - so a constant */
            else
                return 0; /* Type promotion needed - so not a constant */
        default:
            return 0;
    }
}
