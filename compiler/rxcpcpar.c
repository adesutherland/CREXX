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
 * Level C Classic REXX tracer parser glue.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpbgmr.h"
#include "rxcpcgmr.h"
#include "rxcpmain.h"
#include "rxcp_util.h"

static int levelc_token_is(Token *token, const char *text) {
    int i;

    if (!token || !token->token_string || token->length != (int)strlen(text)) return 0;
    for (i = 0; i < token->length; i++) {
        if (toupper((unsigned char)token->token_string[i]) != toupper((unsigned char)text[i])) return 0;
    }
    return 1;
}

static int levelc_parser_token_for_raw(int token_type) {
    switch (token_type) {
        case TK_BADCOMMENT: return CTK_BADCOMMENT;
        case TK_EOS: return CTK_EOS;
        case TK_EOC:
        case TK_EOL: return CTK_EOC;
        case TK_VAR_SYMBOL: return CTK_VAR_SYMBOL;
        case TK_LABEL: return CTK_LABEL;
        case TK_INTEGER: return CTK_INTEGER;
        case TK_STRING: return CTK_STRING;
        case TK_EQUAL: return CTK_EQUAL;
        default: return CTK_UNKNOWN;
    }
}

static int levelc_promote_clause_keyword(Token *token) {
    if (levelc_token_is(token, "SAY")) {
        token->token_type = TK_SAY;
        return CTK_SAY;
    }
    if (levelc_token_is(token, "IF")) {
        token->token_type = TK_IF;
        return CTK_IF;
    }
    if (levelc_token_is(token, "SELECT")) {
        token->token_type = TK_SELECT;
        return CTK_SELECT;
    }
    if (levelc_token_is(token, "WHEN")) {
        token->token_type = TK_WHEN;
        return CTK_WHEN;
    }
    if (levelc_token_is(token, "THEN")) {
        token->token_type = TK_THEN;
        return CTK_THEN;
    }
    if (levelc_token_is(token, "ELSE")) {
        token->token_type = TK_ELSE;
        return CTK_ELSE;
    }
    if (levelc_token_is(token, "OTHERWISE")) {
        token->token_type = TK_OTHERWISE;
        return CTK_OTHERWISE;
    }
    if (levelc_token_is(token, "DO")) {
        token->token_type = TK_DO;
        return CTK_DO;
    }
    if (levelc_token_is(token, "END")) {
        token->token_type = TK_END;
        return CTK_END;
    }
    return CTK_VAR_SYMBOL;
}

int rexcpars(Context *context) {
    Token *token;
    Token *peek_token;
    Token *t;
    void *parser;
    int token_type;
    int parser_token;
    int last_parser_token;
    int clause_start;
    int if_condition;
    int when_condition;
    int pending_else;

    parser = RexxCAlloc(malloc);
#ifndef NDEBUG
    if (context->debug_mode >= 2) RexxCTrace(stderr, "[PARSER-C] ");
    else RexxCTrace(context->traceFile, "Parser(C) >> ");
#endif

    peek_token = token_f(context, rexcscan(context));
    last_parser_token = CTK_EOC;
    clause_start = 1;
    if_condition = 0;
    when_condition = 0;
    pending_else = 0;

    while (1) {
        token = peek_token;
        if (token->token_type == TK_EOL) token->token_type = TK_EOC;
        token_type = token->token_type;

        if (token_type == TK_EOS || token_type == TK_BADCOMMENT) {
            if (last_parser_token != CTK_EOC) {
                RexxC(parser, CTK_EOC, 0, context);
            }
            if (token_type == TK_BADCOMMENT) {
                RexxC(parser, CTK_BADCOMMENT, token, context);
                t = token_f(context, TK_EOC);
                RexxC(parser, CTK_EOC, t, context);
            }
            t = token_f(context, TK_EOS);
            RexxC(parser, CTK_EOS, t, context);
            RexxC(parser, 0, NULL, context);
            break;
        }

        peek_token = token_f(context, rexcscan(context));

        if (token_type == TK_COMMA && peek_token->token_type == TK_EOL) {
            token_r(context);
            token_r(context);
            peek_token = token_f(context, rexcscan(context));
            continue;
        }

        if (token_type == TK_EOC &&
            (last_parser_token == CTK_THEN || last_parser_token == CTK_ELSE ||
             last_parser_token == CTK_OTHERWISE) &&
            peek_token->token_type != TK_EOS) {
            continue;
        }

        if (token_type == TK_EOC &&
            pending_else &&
            peek_token->token_type == TK_VAR_SYMBOL &&
            levelc_token_is(peek_token, "ELSE")) {
            continue;
        }

        if (token_type == TK_EOC && last_parser_token == CTK_EOC) continue;

        parser_token = levelc_parser_token_for_raw(token_type);

        if (token_type == TK_VAR_SYMBOL) {
            if (if_condition && levelc_token_is(token, "THEN")) {
                token->token_type = TK_THEN;
                parser_token = CTK_THEN;
                if_condition = 0;
                pending_else++;
                clause_start = 1;
            }
            else if (when_condition && levelc_token_is(token, "THEN")) {
                token->token_type = TK_THEN;
                parser_token = CTK_THEN;
                when_condition = 0;
                clause_start = 1;
            }
            else if (pending_else && levelc_token_is(token, "ELSE") && peek_token->token_type != TK_EQUAL) {
                token->token_type = TK_ELSE;
                parser_token = CTK_ELSE;
                pending_else--;
                clause_start = 1;
            }
            else if (clause_start && peek_token->token_type != TK_EQUAL) {
                parser_token = levelc_promote_clause_keyword(token);
                if (parser_token == CTK_IF) {
                    if_condition = 1;
                    clause_start = 0;
                }
                else if (parser_token == CTK_WHEN) {
                    when_condition = 1;
                    clause_start = 0;
                }
                else if (parser_token == CTK_SAY) {
                    clause_start = 0;
                }
            }
        }

        if (parser_token == CTK_EOC || parser_token == CTK_LABEL) {
            clause_start = 1;
            if_condition = 0;
            when_condition = 0;
        }
        else if (parser_token != CTK_THEN &&
                 parser_token != CTK_ELSE &&
                 parser_token != CTK_OTHERWISE) {
            clause_start = 0;
        }

        if (parser_token != CTK_EOC && last_parser_token == CTK_EOC) {
            context->current_clause_token = token;
        }
        context->current_parser_token = token;
        context->next_parser_token = peek_token;
        if (context->debug_mode >= 2) {
            fprintf(stderr, "[GLUE-C] Line %d: Passing raw %d (%s) as parser token %d\n",
                    context->line, token_type, token_to_string(token_type), parser_token);
        }
        RexxC(parser, parser_token, token, context);
        last_parser_token = parser_token;
    }

    RexxCFree(parser, free);
    if (!context->ast) rxcp_levelc_run_fallback_diagnostics(context);
    return 0;
}
