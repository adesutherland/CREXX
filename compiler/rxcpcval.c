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
 * Level C source-tree preparation for DSLSH tracer support.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpbgmr.h"
#include "rxcpmain.h"
#include "rxcp_source_tree.h"

walker_result source_location_walker(walker_direction direction,
                                     ASTNode* node,
                                     void *payload);

static int levelc_string_suffix(Token *token, char *suffix, int *raw_length) {
    char quote;
    char last;

    if (suffix) *suffix = '\0';
    if (raw_length) *raw_length = 0;
    if (!token || token->token_type != TK_STRING || !token->token_string ||
        token->length < 3) {
        return 0;
    }

    quote = token->token_string[0];
    if (quote != '\'' && quote != '"') return 0;
    last = token->token_string[token->length - 1];
    if ((last != 'x' && last != 'X' && last != 'b' && last != 'B') ||
        token->token_string[token->length - 2] != quote) {
        return 0;
    }

    if (suffix) *suffix = (char)toupper((unsigned char)last);
    if (raw_length) *raw_length = token->length - 3;
    return 1;
}

static int levelc_hex_digit(char ch) {
    return (ch >= '0' && ch <= '9') ||
           (ch >= 'a' && ch <= 'f') ||
           (ch >= 'A' && ch <= 'F');
}

static int levelc_binary_digit(char ch) {
    return ch == '0' || ch == '1';
}

static int levelc_first_invalid_string_char(Token *token, char suffix, char *invalid) {
    char *raw;
    int raw_length;
    int i;

    if (invalid) *invalid = '\0';
    if (!levelc_string_suffix(token, 0, &raw_length)) return 0;
    raw = token->token_string + 1;
    for (i = 0; i < raw_length; i++) {
        if (raw[i] == ' ') continue;
        if (suffix == 'X') {
            if (!levelc_hex_digit(raw[i])) {
                if (invalid) *invalid = raw[i];
                return 1;
            }
        } else if (!levelc_binary_digit(raw[i])) {
            if (invalid) *invalid = raw[i];
            return 1;
        }
    }
    return 0;
}

static void levelc_replace_error_text(ASTNode *node, char *message) {
    if (!node || !message) return;
    ast_sstr(node, message, strlen(message));
}

static void levelc_rewrite_legacy_string_errors(ASTNode *node) {
    ASTNode *child;
    char suffix;
    char invalid;
    char text[2];
    char *message;

    while (node) {
        if (node->node_type == STRING &&
            levelc_string_suffix(node->token, &suffix, 0) &&
            levelc_first_invalid_string_char(node->token, suffix, &invalid)) {
            child = node->child;
            while (child) {
                if (child->node_type == ERROR && child->node_string &&
                    ((suffix == 'X' &&
                      child->node_string_length == strlen("INVALID_HEX") &&
                      strcmp(child->node_string, "INVALID_HEX") == 0) ||
                     (suffix == 'B' &&
                      child->node_string_length == strlen("INVALID_BIN") &&
                      strcmp(child->node_string, "INVALID_BIN") == 0))) {
                    text[0] = invalid;
                    text[1] = '\0';
                    message = rxcp_levelc_diag_format(suffix == 'X' ? "15.3" : "15.4",
                                                       "char", text);
                    levelc_replace_error_text(child, message);
                }
                child = child->sibling;
            }
        }
        if (node->child) levelc_rewrite_legacy_string_errors(node->child);
        node = node->sibling;
    }
}

void rxcp_levelc_prepare_source_ast(Context *context) {
    if (!context || !context->ast || context->source_tree) return;

    ast_wlkr(context->ast, source_location_walker, (void *)context);
    levelc_rewrite_legacy_string_errors(context->ast);
    source_tree_build(context, context->ast);
    rxcp_levelc_validate_control_diagnostics(context);
}
