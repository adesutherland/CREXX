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
 * THE SOFTWARE IS PROVIDED "AS IS".
 */

/**
 * Level C Classic REXX symbol helpers.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifndef NUTF8
#include "utf.h"
#endif
#include "rxcp_token.h"
#include "rxcpcsym.h"

static const char *levelc_ansi_bif_names[] = {
    "ABBREV",
    "ABS",
    "ADDRESS",
    "ARG",
    "B2X",
    "BITAND",
    "BITOR",
    "BITXOR",
    "C2D",
    "C2X",
    "CENTER",
    "CENTRE",
    "CHANGESTR",
    "CHARIN",
    "CHAROUT",
    "CHARS",
    "COMPARE",
    "CONDITION",
    "COPIES",
    "COUNTSTR",
    "DATATYPE",
    "DATE",
    "DELSTR",
    "DELWORD",
    "DIGITS",
    "D2C",
    "D2X",
    "ERRORTEXT",
    "FORM",
    "FORMAT",
    "FUZZ",
    "INSERT",
    "LASTPOS",
    "LEFT",
    "LENGTH",
    "LINEIN",
    "LINEOUT",
    "LINES",
    "MAX",
    "MIN",
    "OVERLAY",
    "POS",
    "QUALIFY",
    "QUEUED",
    "RANDOM",
    "REVERSE",
    "RIGHT",
    "SIGN",
    "SOURCELINE",
    "SPACE",
    "STREAM",
    "STRIP",
    "SUBSTR",
    "SUBWORD",
    "SYMBOL",
    "TIME",
    "TRACE",
    "TRANSLATE",
    "TRUNC",
    "VALUE",
    "VERIFY",
    "WORD",
    "WORDINDEX",
    "WORDLENGTH",
    "WORDPOS",
    "WORDS",
    "XRANGE",
    "X2B",
    "X2C",
    "X2D",
    0
};

static int levelc_name_equals(const char *left, const char *right) {
    while (*left && *right) {
        if (toupper((unsigned char)*left) != toupper((unsigned char)*right)) return 0;
        left++;
        right++;
    }
    return *left == '\0' && *right == '\0';
}

char *rxcp_levelc_upper_text(const char *text, size_t length) {
    char *name;
    if (!text || memchr(text, '\0', length)) return NULL;
    name = malloc(length + 1);
    if (!name) return NULL;
    memcpy(name, text, length);
    name[length] = '\0';
#ifdef NUTF8
    for (size_t i = 0; i < length; i++) {
        name[i] = (char)toupper((unsigned char)name[i]);
    }
#else
    size_t chars = 0;
    char *current = name;
    char *end = name + length;
    if (utf8nvalid_count(name, length, &chars)) {
        free(name);
        return NULL;
    }
    while (current < end) {
        utf8_int32_t codepoint;
        utf8_int32_t mapped;
        char *next = utf8codepoint(current, &codepoint);
        mapped = utf8uprcodepoint(codepoint);
        if (mapped != codepoint &&
            utf8catcodepoint(current, mapped, (size_t)(next - current)) != next) {
            free(name);
            return NULL;
        }
        current = next;
    }
#endif
    return name;
}

char *rxcp_levelc_upper_symbol_from_token(Token *token, int strip_label_colon) {
    size_t length;

    if (!token || !token->token_string || token->length <= 0) return NULL;
    length = (size_t)token->length;
    if (strip_label_colon && length > 0 && token->token_string[length - 1] == ':')
        length--;
    if (length == 0) return NULL;
    return rxcp_levelc_upper_text(token->token_string, length);
}

int rxcp_levelc_is_ansi_bif_name(const char *name) {
    int i;

    if (!name || !*name) return 0;
    for (i = 0; levelc_ansi_bif_names[i]; i++) {
        if (levelc_name_equals(name, levelc_ansi_bif_names[i])) return 1;
    }
    return 0;
}
