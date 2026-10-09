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
#ifndef NUTF8
#include "crexx_levelc_unicode_cases.h"
#endif

static const char *levelc_ansi_bif_names[] = {
    "UNICODEVERSION",
    "TONFD",
    "TONFC",
    "TONFKD",
    "TONFKC",
    "ISNFD",
    "ISNFC",
    "ISNFKD",
    "ISNFKC",
    "TOUPPERCASE",
    "TOLOWERCASE",
    "TOCASEFOLD",
    "TOSIMPLECASEFOLD",
    "TOTURKICCASEFOLD",
    "TOTURKICSIMPLECASEFOLD",
    "GRAPHEMECOUNT",
    "GRAPHEMEREVERSE",
    "GRAPHEMESUBSTR",
    "GRAPHEMEPOS",
    "ENCODE",
    "DECODE",
    "ISDECODABLE",
    "ISENCODINGSUPPORTED",
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
#ifdef NUTF8
    name = malloc(length + 1);
    if (!name) return NULL;
    for (size_t i = 0; i < length; i++) name[i] = (char)toupper((unsigned char)text[i]);
    name[length] = '\0';
#else
    size_t chars = 0;
    const char *current = text;
    const char *end = text + length;
    char *destination;
    if (utf8nvalid_count(text, length, &chars) || chars > ((size_t)-1 - 1) / 4) return NULL;
    name = malloc(chars * 4 + 1);
    if (!name) return NULL;
    destination = name;
    while (current < end) {
        utf8_int32_t codepoint;
        utf8_int32_t mapped;
        size_t low = 0, high = sizeof(levelc_upper_cases) / sizeof(levelc_upper_cases[0]);
        current = utf8codepoint(current, &codepoint);
        mapped = codepoint;
        while (low < high) {
            size_t middle = low + (high - low) / 2;
            unsigned int candidate = levelc_upper_cases[middle][0];
            if ((unsigned int)codepoint < candidate) high = middle;
            else if ((unsigned int)codepoint > candidate) low = middle + 1;
            else { mapped = (utf8_int32_t)levelc_upper_cases[middle][1]; break; }
        }
        destination = utf8catcodepoint(destination, mapped, 4);
    }
    *destination = '\0';
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
