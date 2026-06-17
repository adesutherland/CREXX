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

#ifndef RXCPCSYM_H
#define RXCPCSYM_H

#include "rxcp_types.h"

enum {
    TK_LEVELC_ADDRESS = 50000,
    TK_LEVELC_DROP,
    TK_LEVELC_INTERPRET,
    TK_LEVELC_PARSE,
    TK_LEVELC_PULL,
    TK_LEVELC_PUSH,
    TK_LEVELC_QUEUE,
    TK_LEVELC_TRACE,
    TK_LEVELC_UPPER,
    TK_LEVELC_SOURCE,
    TK_LEVELC_LINEIN,
    TK_LEVELC_VERSION,
    TK_LEVELC_VALUE,
    TK_LEVELC_VAR,
    TK_LEVELC_WITH,
    TK_LEVELC_EXPOSE,
    TK_LEVELC_DIGITS,
    TK_LEVELC_FORM,
    TK_LEVELC_FUZZ,
    TK_LEVELC_ENGINEERING,
    TK_LEVELC_SCIENTIFIC,
    TK_LEVELC_OFF,
    TK_LEVELC_NAME,
    TK_LEVELC_ERROR,
    TK_LEVELC_FAILURE,
    TK_LEVELC_HALT,
    TK_LEVELC_NOTREADY,
    TK_LEVELC_NOVALUE,
    TK_LEVELC_SYNTAX,
    TK_LEVELC_LOSTDIGITS,
    TK_LEVELC_INPUT,
    TK_LEVELC_OUTPUT,
    TK_LEVELC_STREAM,
    TK_LEVELC_STEM,
    TK_LEVELC_NORMAL,
    TK_LEVELC_APPEND,
    TK_LEVELC_REPLACE
};

char *rxcp_levelc_upper_symbol_from_token(Token *token, int strip_label_colon);
int rxcp_levelc_is_ansi_bif_name(const char *name);

#endif
