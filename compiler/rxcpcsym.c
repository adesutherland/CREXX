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

char *rxcp_levelc_upper_symbol_from_token(Token *token, int strip_label_colon) {
    char *name;
    int length;
    int i;

    if (!token || !token->token_string || token->length <= 0) return 0;
    length = token->length;
    if (strip_label_colon && length > 0 && token->token_string[length - 1] == ':') {
        length--;
    }
    if (length <= 0) return 0;

    name = malloc((size_t)length + 1);
    if (!name) return 0;
    for (i = 0; i < length; i++) {
        name[i] = (char)toupper((unsigned char)token->token_string[i]);
    }
    name[length] = '\0';
    return name;
}

int rxcp_levelc_is_ansi_bif_name(const char *name) {
    int i;

    if (!name || !*name) return 0;
    for (i = 0; levelc_ansi_bif_names[i]; i++) {
        if (levelc_name_equals(name, levelc_ansi_bif_names[i])) return 1;
    }
    return 0;
}
