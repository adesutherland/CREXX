/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

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
