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
#include "rxcpmain.h"

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
