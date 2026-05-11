/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 */

/**
 * Level C Classic REXX diagnostic helpers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpbgmr.h"
#include "rxcpmain.h"

typedef enum {
    LEVELC_FB_FRAME_DO,
    LEVELC_FB_FRAME_SELECT
} LevelCFallbackFrameType;

typedef struct {
    LevelCFallbackFrameType type;
    Token *token;
} LevelCFallbackFrame;

static char *levelc_escape_diag_value(const char *value) {
    char *escaped;
    char *out;
    size_t length;
    size_t i;
    size_t escaped_length;
    unsigned char ch;
    char hex[5];

    if (!value) value = "";
    length = strlen(value);
    escaped_length = 0;
    for (i = 0; i < length; i++) {
        ch = (unsigned char)value[i];
        if (ch == '\\' || ch == '"') escaped_length += 2;
        else if (ch == '\n' || ch == '\r' || ch == '\t') escaped_length += 2;
        else if (ch < 32) escaped_length += 4;
        else escaped_length++;
    }

    escaped = malloc(escaped_length + 1);
    if (!escaped) return strdup("");
    out = escaped;
    for (i = 0; i < length; i++) {
        ch = (unsigned char)value[i];
        if (ch == '\\' || ch == '"') {
            *out++ = '\\';
            *out++ = (char)ch;
        } else if (ch == '\n') {
            *out++ = '\\';
            *out++ = 'n';
        } else if (ch == '\r') {
            *out++ = '\\';
            *out++ = 'r';
        } else if (ch == '\t') {
            *out++ = '\\';
            *out++ = 't';
        } else if (ch < 32) {
            snprintf(hex, sizeof(hex), "\\x%02X", ch);
            memcpy(out, hex, 4);
            out += 4;
        } else {
            *out++ = (char)ch;
        }
    }
    *out = '\0';
    return escaped;
}

static char *levelc_token_value(Token *token) {
    if (!token || !token->token_string || token->length <= 0) return strdup("");
    return rx_strndup(token->token_string, (size_t)token->length);
}

static char *levelc_fallback_token_text(Token *token) {
    if (!token) return strdup("end-of-source");
    if (token->token_type == TK_EOS) return strdup("end-of-source");
    if (token->token_type == TK_EOC ||
        token->token_type == TK_EOL ||
        !token->token_string ||
        token->length <= 0 ||
        token->token_string[0] == '\n' ||
        token->token_string[0] == '\r') {
        return strdup("end-of-clause");
    }
    return rx_strndup(token->token_string, (size_t)token->length);
}

static char *levelc_fallback_line_text(Token *token) {
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%d", token ? token->line + 1 : 0);
    return strdup(buffer);
}

char *rxcp_levelc_diag_format(const char *standard_code,
                              const char *insert_name,
                              const char *insert_value) {
    return rxcp_levelc_diag_format2(standard_code, insert_name, insert_value, 0, 0);
}

char *rxcp_levelc_diag_format2(const char *standard_code,
                               const char *insert_name1,
                               const char *insert_value1,
                               const char *insert_name2,
                               const char *insert_value2) {
    char *escaped;
    char *escaped2;
    char *message;

    if (!standard_code) standard_code = "0";
    if (!insert_name1 || !*insert_name1) return mprintf("RXC-LC-%s", standard_code);

    escaped = levelc_escape_diag_value(insert_value1);
    if (!insert_name2 || !*insert_name2) {
        message = mprintf("RXC-LC-%s %s=\"%s\"", standard_code, insert_name1, escaped ? escaped : "");
        if (escaped) free(escaped);
        return message;
    }

    escaped2 = levelc_escape_diag_value(insert_value2);
    message = mprintf("RXC-LC-%s %s=\"%s\" %s=\"%s\"",
                      standard_code,
                      insert_name1,
                      escaped ? escaped : "",
                      insert_name2,
                      escaped2 ? escaped2 : "");
    if (escaped) free(escaped);
    if (escaped2) free(escaped2);
    return message;
}

ASTNode *rxcp_levelc_ast_error_insert(Context *context,
                                      const char *standard_code,
                                      Token *token,
                                      const char *insert_name,
                                      const char *insert_value) {
    return rxcp_levelc_ast_error_insert2(context, standard_code, token,
                                         insert_name, insert_value, 0, 0);
}

ASTNode *rxcp_levelc_ast_error_insert2(Context *context,
                                       const char *standard_code,
                                       Token *token,
                                       const char *insert_name1,
                                       const char *insert_value1,
                                       const char *insert_name2,
                                       const char *insert_value2) {
    ASTNode *node;
    char *message;

    node = ast_f(context, TOKEN, token);
    message = rxcp_levelc_diag_format2(standard_code, insert_name1, insert_value1,
                                       insert_name2, insert_value2);
    mknd_err(node, "%s", message ? message : "RXC-LC-0");
    if (message) free(message);
    return node;
}

ASTNode *rxcp_levelc_ast_error(Context *context,
                               const char *standard_code,
                               Token *token) {
    return rxcp_levelc_ast_error_insert(context, standard_code, token, 0, 0);
}

ASTNode *rxcp_levelc_ast_error_token(Context *context,
                                     const char *standard_code,
                                     Token *token) {
    ASTNode *node;
    char *value;

    value = levelc_token_value(token);
    node = rxcp_levelc_ast_error_insert(context, standard_code, token, "token", value);
    if (value) free(value);
    return node;
}

ASTNode *rxcp_levelc_add_error(ASTNode *node,
                               const char *standard_code,
                               const char *insert_name,
                               const char *insert_value) {
    return rxcp_levelc_add_error2(node, standard_code, insert_name, insert_value, 0, 0);
}

ASTNode *rxcp_levelc_add_error2(ASTNode *node,
                                const char *standard_code,
                                const char *insert_name1,
                                const char *insert_value1,
                                const char *insert_name2,
                                const char *insert_value2) {
    char *message;

    if (!node) return 0;
    message = rxcp_levelc_diag_format2(standard_code, insert_name1, insert_value1,
                                       insert_name2, insert_value2);
    mknd_err(node, "%s", message ? message : "RXC-LC-0");
    if (message) free(message);
    return node;
}

static int levelc_has_detached_diagnostic(Context *context,
                                          Token *token,
                                          const char *message) {
    ASTNode *diag;
    size_t message_len;

    if (!context || !message) return 0;
    message_len = strlen(message);
    diag = (ASTNode *)context->diagnostics_list;
    while (diag) {
        if (diag->node_type == ERROR &&
            diag->token == token &&
            diag->node_string_length == message_len &&
            strncmp(diag->node_string, message, message_len) == 0) {
            return 1;
        }
        diag = diag->sibling;
    }
    return 0;
}

static void levelc_append_detached_diagnostic(Context *context,
                                              Token *token,
                                              const char *message) {
    ASTNode *diag;
    ASTNode *tail;

    if (!context || !message) return;
    if (levelc_has_detached_diagnostic(context, token, message)) return;

    diag = ast_ft(context, ERROR);
    ast_copy_str(diag, (char *)message);
    diag->token = token;
    diag->file_name = context->file_name;
    if (token) {
        diag->line = token->line;
        diag->column = token->column;
        diag->source_start = token->token_string;
        diag->source_end = token->token_string + token->length - 1;
    }

    if (!context->diagnostics_list) {
        context->diagnostics_list = diag;
        return;
    }

    tail = (ASTNode *)context->diagnostics_list;
    while (tail->sibling) tail = tail->sibling;
    tail->sibling = diag;
}

static void levelc_append_code(Context *context, Token *token, const char *standard_code) {
    char *message;

    message = rxcp_levelc_diag_format(standard_code, 0, 0);
    levelc_append_detached_diagnostic(context, token, message);
    free(message);
}

static void levelc_append_code_token(Context *context,
                                     Token *token,
                                     const char *standard_code) {
    char *token_text;
    char *message;

    token_text = levelc_fallback_token_text(token);
    message = rxcp_levelc_diag_format(standard_code, "token", token_text);
    levelc_append_detached_diagnostic(context, token, message);
    free(token_text);
    free(message);
}

static void levelc_append_missing_then(Context *context,
                                       Token *keyword,
                                       Token *found,
                                       const char *standard_code) {
    char *line;
    char *token_text;
    char *message;

    line = levelc_fallback_line_text(keyword);
    token_text = levelc_fallback_token_text(found);
    message = rxcp_levelc_diag_format2(standard_code,
                                       "linenumber", line,
                                       "token", token_text);
    levelc_append_detached_diagnostic(context, keyword, message);
    free(line);
    free(token_text);
    free(message);
}

static Token *levelc_fallback_anchor_token(Context *context) {
    Token *token;

    if (!context) return 0;
    token = context->token_tail;
    while (token &&
           (token->token_type == TK_EOC ||
            token->token_type == TK_EOS ||
            token->token_type == TK_BADCOMMENT)) {
        token = token->token_prev;
    }
    return token;
}

static int levelc_has_select_frame(LevelCFallbackFrame *frames, size_t count) {
    size_t i;

    for (i = count; i > 0; i--) {
        if (frames[i - 1].type == LEVELC_FB_FRAME_SELECT) return 1;
    }
    return 0;
}

static int levelc_push_frame(LevelCFallbackFrame **frames,
                             size_t *count,
                             size_t *capacity,
                             LevelCFallbackFrameType type,
                             Token *token) {
    LevelCFallbackFrame *new_frames;
    size_t new_capacity;

    if (*count == *capacity) {
        new_capacity = (*capacity == 0) ? 8 : (*capacity * 2);
        new_frames = realloc(*frames, sizeof(LevelCFallbackFrame) * new_capacity);
        if (!new_frames) return 0;
        *frames = new_frames;
        *capacity = new_capacity;
    }

    (*frames)[*count].type = type;
    (*frames)[*count].token = token;
    (*count)++;
    return 1;
}

int rxcp_levelc_run_fallback_diagnostics(Context *context) {
    ASTNode *diag;
    Token *token;
    Token *pending_if;
    Token *pending_when;
    LevelCFallbackFrame *frames;
    size_t frame_count;
    size_t frame_capacity;
    int diagnostics_before;
    int diagnostics_after;
    int token_type;

    if (!context) return 0;

    diagnostics_before = 0;
    diag = (ASTNode *)context->diagnostics_list;
    while (diag) {
        diagnostics_before++;
        diag = diag->sibling;
    }

    pending_if = 0;
    pending_when = 0;
    frames = 0;
    frame_count = 0;
    frame_capacity = 0;

    token = context->token_head;
    while (token) {
        token_type = token->token_type;
        if (token_type == TK_END ||
            token_type == TK_ELSE ||
            token_type == TK_OTHERWISE ||
            token_type == TK_WHEN ||
            token_type == TK_EOS) {
            if (pending_when) {
                levelc_append_missing_then(context, pending_when, token, "18.2");
                pending_when = 0;
            }
            if (pending_if) {
                levelc_append_missing_then(context, pending_if, token, "18.1");
                pending_if = 0;
            }
        }

        switch (token_type) {
            case TK_DO:
            case TK_LOOP:
                if (!levelc_push_frame(&frames, &frame_count, &frame_capacity,
                                       LEVELC_FB_FRAME_DO, token)) {
                    free(frames);
                    return 0;
                }
                break;

            case TK_SELECT:
                if (!levelc_push_frame(&frames, &frame_count, &frame_capacity,
                                       LEVELC_FB_FRAME_SELECT, token)) {
                    free(frames);
                    return 0;
                }
                break;

            case TK_END:
                pending_if = 0;
                pending_when = 0;
                if (frame_count > 0) frame_count--;
                else levelc_append_code(context, token, "10.1");
                break;

            case TK_IF:
                pending_if = token;
                break;

            case TK_WHEN:
                if (!levelc_has_select_frame(frames, frame_count)) {
                    levelc_append_code(context, token, "9.1");
                } else {
                    pending_when = token;
                }
                break;

            case TK_THEN:
                if (pending_when) pending_when = 0;
                else if (pending_if) pending_if = 0;
                else levelc_append_code(context, token, "8.1");
                break;

            case TK_OTHERWISE:
                if (!levelc_has_select_frame(frames, frame_count)) {
                    levelc_append_code(context, token, "9.2");
                }
                break;

            default:
                break;
        }

        token = token->token_next;
    }

    while (frame_count > 0) {
        frame_count--;
        levelc_append_code(context,
                           frames[frame_count].token,
                           frames[frame_count].type == LEVELC_FB_FRAME_SELECT ? "14.2" : "14.1");
    }

    free(frames);

    diagnostics_after = 0;
    diag = (ASTNode *)context->diagnostics_list;
    while (diag) {
        diagnostics_after++;
        diag = diag->sibling;
    }

    if (diagnostics_after == diagnostics_before) {
        levelc_append_code_token(context, levelc_fallback_anchor_token(context), "21.1");
        diagnostics_after++;
    }

    return diagnostics_after - diagnostics_before;
}
