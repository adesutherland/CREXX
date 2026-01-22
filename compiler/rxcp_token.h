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
 * Token Definitions
 */

#ifndef CREXX_RXCP_TOKEN_H
#define CREXX_RXCP_TOKEN_H

#include "rxcp_types.h"

struct Token {
    int token_type;
    int token_number;
    int token_subtype;
    Token *token_next;
    Token *token_prev;
    int line, column, length;
    char* token_string;
};

/* Token Functions */
Token* token_f(Context* context, int type);
/* Split a token - returns the first token (token->token_next) points to the next twin; */
/* the first token has len characters, the second twin as the remaining characters.       */
/* The caller can then change the tokens' types as needed.                              */
Token *tok_splt(Context *context, Token *token, int len);
/* Remove the last (tail) token */
void token_r(Context *context);
void free_tok(Context* context);
void prnt_tok(Token* token);
const char* tk_tp_nm(int type); /* Get Token Type Name */

#endif //CREXX_RXCP_TOKEN_H
