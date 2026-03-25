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
 * Fallback structural diagnostics for parse failures without an AST.
 *
 * This is intentionally not a second parser. Its role is limited to
 * terminal parse-failure diagnosis for structural syntax, primarily so the
 * compiler can emit normal user-facing errors instead of "Failure to create
 * AST". If Lemon recovery can preserve a useful partial AST, that recovery
 * should remain in the grammar instead of being moved here.
 */

#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"

typedef enum {
    FB_FRAME_DO,
    FB_FRAME_SELECT
} FallbackFrameType;

typedef struct {
    FallbackFrameType type;
    Token *token;
} FallbackFrame;

static int has_diagnostic(Context *context, Token *token, const char *message) {
    ASTNode *diag = (ASTNode*)context->diagnostics_list;
    size_t message_len = strlen(message);
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

static void append_diagnostic(Context *context, Token *token, const char *message) {
    ASTNode *diag;
    ASTNode *tail;

    if (!context || !message) return;
    if (has_diagnostic(context, token, message)) return;

    diag = ast_ft(context, ERROR);
    ast_copy_str(diag, (char*)message);
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

    tail = (ASTNode*)context->diagnostics_list;
    while (tail->sibling) tail = tail->sibling;
    tail->sibling = diag;
}

static Token *fallback_anchor_token(Context *context) {
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

static int has_select_frame(FallbackFrame *frames, size_t count) {
    size_t i;
    for (i = count; i > 0; i--) {
        if (frames[i - 1].type == FB_FRAME_SELECT) return 1;
    }
    return 0;
}

static int push_frame(FallbackFrame **frames,
                      size_t *count,
                      size_t *capacity,
                      FallbackFrameType type,
                      Token *token) {
    FallbackFrame *new_frames;
    size_t new_capacity;

    if (*count == *capacity) {
        new_capacity = (*capacity == 0) ? 8 : (*capacity * 2);
        new_frames = realloc(*frames, sizeof(FallbackFrame) * new_capacity);
        if (!new_frames) return 0;
        *frames = new_frames;
        *capacity = new_capacity;
    }

    (*frames)[*count].type = type;
    (*frames)[*count].token = token;
    (*count)++;
    return 1;
}

int rxcp_run_fallback_diagnostics(Context *context) {
    ASTNode *diag;
    Token *token;
    Token *pending_if;
    Token *pending_when;
    FallbackFrame *frames;
    size_t frame_count;
    size_t frame_capacity;
    int diagnostics_before;
    int diagnostics_after;
    int token_type;

    if (!context) return 0;

    diagnostics_before = 0;
    diag = (ASTNode*)context->diagnostics_list;
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
                append_diagnostic(context, pending_when, "MISSING_THEN");
                pending_when = 0;
            }
            if (pending_if) {
                append_diagnostic(context, pending_if, "MISSING_THEN");
                pending_if = 0;
            }
        }

        switch (token_type) {
            case TK_DO:
                if (!push_frame(&frames, &frame_count, &frame_capacity, FB_FRAME_DO, token)) {
                    free(frames);
                    return 0;
                }
                break;

            case TK_LOOP:
                if (!push_frame(&frames, &frame_count, &frame_capacity, FB_FRAME_DO, token)) {
                    free(frames);
                    return 0;
                }
                break;

            case TK_SELECT:
                if (!push_frame(&frames, &frame_count, &frame_capacity, FB_FRAME_SELECT, token)) {
                    free(frames);
                    return 0;
                }
                break;

            case TK_END:
                pending_if = 0;
                pending_when = 0;
                if (frame_count > 0) frame_count--;
                else append_diagnostic(context, token, "UNEXPECTED_END");
                break;

            case TK_IF:
                pending_if = token;
                break;

            case TK_WHEN:
                if (!has_select_frame(frames, frame_count)) {
                    append_diagnostic(context, token, "UNEXPECTED_WHEN");
                } else {
                    pending_when = token;
                }
                break;

            case TK_THEN:
                if (pending_when) pending_when = 0;
                else if (pending_if) pending_if = 0;
                else append_diagnostic(context, token, "UNEXPECTED_THEN");
                break;

            case TK_OTHERWISE:
                if (!has_select_frame(frames, frame_count)) {
                    append_diagnostic(context, token, "UNEXPECTED_OTHERWISE");
                }
                break;

            case TK_ELSE:
                break;

            case TK_EOC:
                break;

            case TK_EOS:
                break;

            default:
                break;
        }

        token = token->token_next;
    }

    while (frame_count > 0) {
        frame_count--;
        append_diagnostic(context, frames[frame_count].token, "MISSING_END");
    }

    free(frames);

    diagnostics_after = 0;
    diag = (ASTNode*)context->diagnostics_list;
    while (diag) {
        diagnostics_after++;
        diag = diag->sibling;
    }

    if (diagnostics_after == diagnostics_before) {
        append_diagnostic(context, fallback_anchor_token(context), "PARSE_FAILURE");
        diagnostics_after++;
    }

    return diagnostics_after - diagnostics_before;
}
