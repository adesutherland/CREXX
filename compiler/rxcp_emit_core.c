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

/* Type promotion matrix for numeric operators */
const char* emit_promotion[9][9] = {
/*                  TP_UNKNOWN,TP_VOID,TP_BOOLEAN,TP_INTEGER,TP_FLOAT, TP_DECIMAL, TP_STRING,TP_BINARY,TP_OBJECT */

/* TP_UNKNOWN */   {0,         0,      0,         0,         0,      0,         0,        0,        0},
/* TP_VOID    */   {0,         0,      0,         0,         0,      0,         0,        0,        0},
/* TP_BOOLEAN */   {0,         0,      0,         "btoi",    "btof", "btod",    "btos",   0,        0},
/* TP_INTEGER */   {0,         0,      0,         0,         "itof", "itod",    "itos",   0,        0},
/* TP_FLOAT   */   {0,         0,      "ftob",    "ftoi",    0,      "ftod",    "ftos",   0,        0},
/* TP_DECIMAL */   {0,         0,      "dtob",    "dtoi",    "dtof", 0,         "dtos",   0,        0},
/* TP_STRING  */   {0,         0,      "stoi",    "stoi",    "stof", "stod",    0,        0,        0},
/* TP_BINARY  */   {0,         0,      0,         0,         0,      0,         0,        0,        0},
/* TP_OBJECT  */   {0,         0,      0,         0,         0,      0,         0,        0,        0},
};

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

/* Returns the meta .src line in a malloced buffer */
char* get_metaline(ASTNode *node) {
    char *result, *src;
    int line, column;
    char *source_start, *source_end;

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

    if (!source_start) {
        result = malloc(1);
        result[0] = 0;
    }
    else {
        src = encode_line_source_malloc(source_start,
                                        (int) (source_end - source_start) + 1);
        result = mprintf("   .src %d:%d=\"%s\"\n", line + 1, column + 1, src);
        free(src);
    }
    return result;
}

/* Returns the meta .src line in a malloced buffer */
char* get_metaline_range(ASTNode *from, ASTNode *to) {
    char *result, *src;
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

    if (!from_source_start || !to_source_end) {
        result = malloc(1);
        result[0] = 0;
    }
    else {
        src = encode_line_source_malloc(from_source_start,
                                        (int) (to_source_end - from_source_start) + 1);
        result = mprintf("   .src %d:%d=\"%s\"\n", from_line + 1, from_column, src);
        free(src);
    }
    return result;
}

/* Returns the meta .src line in a malloced buffer */
char* get_metaline_between(ASTNode *from, ASTNode *to) {
    char *result, *src;
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

    if (!start || !end) {
        result = malloc(1);
        result[0] = 0;
    }
    else {
        src = encode_line_source_malloc(start->token_string,
                                        (int) (end->token_string - start->token_string) + end->length);
        result = mprintf("   .src %d:%d=\"%s\"\n", start->line + 1, start->column + 1, src);
        free(src);
    }
    return result;
}

/* Returns the meta .src line in a malloced buffer */
char* get_metaline_token_after(ASTNode *node) {
    char *result, *src;
    Token *start = 0;

    if (node->token_end) start = node->token_end->token_next;

    /* Get rid of leading newline */
    if (start && start->token_type == TK_EOC) {
        /* Skip the newline */
        start = start->token_next;
    }

    if (!start) {
        result = malloc(1);
        result[0] = 0;
    }
    else {
        src = encode_line_source_malloc(start->token_string,
                                        start->length);
        result = mprintf("   .src %d:%d=\"%s\"\n", start->line + 1, start->column + 1, src);
        free(src);
    }
    return result;
}

/* Returns the meta .src line in a malloced buffer */
char* get_metaline_clause(ASTNode *node) {
    char *result, *src;
    Token *start = 0;
    Token *end = 0;

    if (node->token_start) start = node->token_start;

    if (!start) {
        result = malloc(1);
        result[0] = 0;
    }
    else {
        end = start;
        while (end->token_next->token_type != TK_EOC && end->token_next->token_type != TK_EOS)
            end = end->token_next;

        src = encode_line_source_malloc(start->token_string,
                                        (int) (end->token_string - start->token_string) + end->length);
        result = mprintf("   .src %d:%d=\"%s\"\n", start->line + 1, start->column + 1, src);
        free(src);
    }
    return result;
}

/* Returns the meta .src line in a malloced buffer */
char* get_metaline_token_at(ASTNode *node) {
    char *result, *src;
    Token *start = 0;

    if (node->token_start) start = node->token_start;

    /* Get rid of leading newline - unlikely that there will be one! */
    if (start && start->token_type == TK_EOC) {
        /* Skip the newline */
        start = start->token_next;
    }

    if (!start) {
        result = malloc(1);
        result[0] = 0;
    }
    else {
        src = encode_line_source_malloc(start->token_string,
                                        start->length);
        result = mprintf("   .src %d:%d=\"%s\"\n", start->line + 1, start->column + 1, src);
        free(src);
    }
    return result;
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
            if (node->node_string[i] == '.' || node->node_string[i] == 'e') {
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
        default:
            return "";
    }
}
