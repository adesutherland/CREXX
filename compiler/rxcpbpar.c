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
 * Compiler Parser Wrapper
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpbgmr.h"
#include "rxcpmain.h"
#include "rxcp_util.h"
#include "rxcp_exit.h"

static int rxcp_is_instruction_lead_token(int last_token_type) {
    return last_token_type == TK_EOC ||
           last_token_type == TK_THEN ||
           last_token_type == TK_ELSE ||
           last_token_type == TK_OTHERWISE;
}

static int rxcp_token_text_equals(Token *token, const char *text) {
    int i;
    int length;

    if (!token || !token->token_string || !text) return 0;
    length = (int)strlen(text);
    if (token->length != length) return 0;
    for (i = 0; i < length; i++) {
        if (tolower((unsigned char)token->token_string[i]) != text[i]) return 0;
    }
    return 1;
}

static int rxcp_array_statement_head_type(Token *token) {
    if (rxcp_token_text_equals(token, "append")) return TK_ARRAY_APPEND;
    if (rxcp_token_text_equals(token, "clear")) return TK_ARRAY_CLEAR;
    if (rxcp_token_text_equals(token, "insert")) return TK_ARRAY_INSERT;
    if (rxcp_token_text_equals(token, "remove")) return TK_ARRAY_REMOVE;
    return 0;
}

static int rxcp_can_start_array_statement_target(Token *token) {
    return token && token->token_type == TK_VAR_SYMBOL;
}

static void rxcp_update_array_statement_depth(int token_type, int *depth) {
    if (!depth) return;
    switch (token_type) {
        case TK_OPEN_BRACKET:
        case TK_OPEN_SBRACKET:
            (*depth)++;
            break;
        case TK_CLOSE_BRACKET:
        case TK_CLOSE_SBRACKET:
            if (*depth > 0) (*depth)--;
            break;
        default:
            break;
    }
}

static const char *rxcp_cli_level_name(Context *context) {
    switch (context->cli_default_level) {
        case LEVELA: return "levela";
        case LEVELB: return "levelb";
        case LEVELC: return "levelc";
        case LEVELD: return "leveld";
        case LEVELG: return "levelg";
        case LEVELL: return "levell";
        default: return 0;
    }
}

static Token *rxcp_synthetic_token(Context *context, int type, const char *text) {
    Token *token;

    token = token_f(context, type);
    if (text) {
        token->token_string = (char*)text;
        token->length = (int)strlen(text);
        token->line = 0;
        token->column = 1;
    }

    return token;
}

static Token *rxcp_next_parser_token(Context *context, int *cli_option_stage) {
    const char *level_name;

    if (cli_option_stage && *cli_option_stage) {
        level_name = rxcp_cli_level_name(context);
        switch (*cli_option_stage) {
            case 1:
                *cli_option_stage = 2;
                return rxcp_synthetic_token(context, TK_OPTIONS, "options");
            case 2:
                *cli_option_stage = 3;
                return rxcp_synthetic_token(context, TK_VAR_SYMBOL, level_name);
            case 3:
                *cli_option_stage = 0;
                return rxcp_synthetic_token(context, TK_EOC, 0);
            default:
                *cli_option_stage = 0;
                break;
        }
    }

    return token_f(context, rexbscan(context));
}

static Token *rxcp_take_parser_token(Context *context, int *cli_option_stage, Token **deferred_token) {
    Token *token;

    if (deferred_token && *deferred_token) {
        token = *deferred_token;
        *deferred_token = 0;
        return token;
    }

    return rxcp_next_parser_token(context, cli_option_stage);
}

int rexbpars(Context *context) {

    char *buff, *buff_end;
    size_t bytes;
    int token_type, last_token_type;
    Token *token, *t, *peek_token, *deferred_token;
    void *parser;
    int cli_option_stage;
    int array_statement_active;
    int array_statement_depth;

    /* Create parser and set up tracing */
    parser = RexxBAlloc(malloc);
#ifndef NDEBUG
    if (context->debug_mode >= 2) RexxBTrace(stderr, "[PARSER] ");
    else RexxBTrace(context->traceFile, "Parser(B) >> ");
#endif

    cli_option_stage = 0;
    if (context->cli_default_level != UNKNOWN && !context->source_has_options) {
        cli_option_stage = 1;
    }

    deferred_token = 0;
    peek_token = rxcp_take_parser_token(context, &cli_option_stage, &deferred_token);
    last_token_type = TK_EOC;
    int in_exit_instruction = 0;
    array_statement_active = 0;
    array_statement_depth = 0;
    while (1) {
        const char *disabled_certified_primary;
        int promoted_array_head;

        token = peek_token;
        if (token->token_type == TK_EOL) token->token_type = TK_EOC;
        token_type = token->token_type;

        if (token_type == TK_EOC) {
            in_exit_instruction = 0;
            array_statement_active = 0;
            array_statement_depth = 0;
        }

        if (token_type != TK_EOS && token_type != TK_BADCOMMENT) {
            peek_token = rxcp_take_parser_token(context, &cli_option_stage, &deferred_token);
        }

        if ((token_type == TK_STRING || token_type == TK_STRING_CONTINUATION) &&
            peek_token->token_type == TK_EOL) {
            Token *newline_token;
            Token *after_newline_token;

            newline_token = peek_token;
            after_newline_token = rxcp_take_parser_token(context, &cli_option_stage, &deferred_token);
            if (after_newline_token->token_type == TK_STRING) {
                after_newline_token->token_type = TK_STRING_CONTINUATION;
                peek_token = after_newline_token;
            } else {
                deferred_token = after_newline_token;
                peek_token = newline_token;
            }
        }

        // Promotion
        promoted_array_head = 0;
        disabled_certified_primary = NULL;
        if (context->disable_exits && token->token_string) {
            disabled_certified_primary = rxcp_match_certified_exit_primary(token->token_string, token->length);
        }
        if (rxcp_is_instruction_lead_token(last_token_type) &&
            token_type == TK_VAR_SYMBOL &&
            rxcp_can_start_array_statement_target(peek_token) &&
            rxcp_array_statement_head_type(token)) {
            token_type = rxcp_array_statement_head_type(token);
            token->token_type = token_type;
            array_statement_active = 1;
            array_statement_depth = 0;
            promoted_array_head = 1;
        } else if (array_statement_active &&
                   array_statement_depth == 0 &&
                   token_type == TK_VAR_SYMBOL &&
                   rxcp_token_text_equals(token, "at")) {
            token_type = TK_ARRAY_AT;
            token->token_type = TK_ARRAY_AT;
        }

        if (!promoted_array_head &&
            rxcp_is_instruction_lead_token(last_token_type) &&
            token_type != TK_EOC &&
            token_type != TK_EOS &&
            token->token_string &&
            (rxcp_is_exit_primary(context, token->token_string, token->length) ||
             disabled_certified_primary)) {
            token_type = TK_EXIT_PRIMARY;
            token->token_type = TK_EXIT_PRIMARY;
            in_exit_instruction = 1;
        } else if (token_type == TK_VAR_SYMBOL) {
            if (in_exit_instruction && rxcp_is_exit_additional(context, token->token_string, token->length)) {
                token_type = TK_EXIT_TOKEN;
                token->token_type = TK_EXIT_TOKEN;
            }
        }

        if (array_statement_active) {
            rxcp_update_array_statement_depth(token_type, &array_statement_depth);
        }

        // EOS Special Processing
        if (token_type == TK_EOS || token_type == TK_BADCOMMENT) {
            // Send an EOC
            if (last_token_type != TK_EOC) {
                RexxB(parser, TK_EOC, 0, context);
            }

            if (token_type == TK_BADCOMMENT) {
                RexxB(parser, TK_BADCOMMENT, token, context);

                t = token_f(context, TK_EOC);
                RexxB(parser, TK_EOC, t, context);
            }

            // Send EOS
            t = token_f(context, TK_EOS);
            RexxB(parser, TK_EOS, t, context);

            // Send a null
            RexxB(parser, 0, NULL, context);
            break;
        }

        // Line Continuation
        if (token_type == TK_COMMA && peek_token->token_type == TK_EOL) {
            token_r(context);  /* Discard tokens , and EOC tokens */
            token_r(context);
            peek_token = rxcp_take_parser_token(context, &cli_option_stage, &deferred_token);
            continue;
        }

        // Skip multiple end of clause/line
        if (last_token_type == TK_EOC && token_type == TK_EOC) continue;


        /* Special Processing */
        if (token_type == TK_MINUSMINUS) {
            /* TODO Check for Check for C operator mode - when/if implemented!    */
            /* The TK_MINUSMINUS token is generated by -- in comments_nodash mode, */
            /* it needs to be converted to two TK_MINUS tokens                    */
            t = tok_splt(context, token, 1);
            token = t->token_next; /* In case tok_splt() changes token pointer - it doesn't currently */
            token_type = TK_MINUS;
            t->token_type = token_type;
            token->token_type = token_type;
            RexxB(parser, token_type, t, context);
        }

        if (token_type != TK_EOC && last_token_type == TK_EOC) {
            context->current_clause_token = token;
        }
        context->current_parser_token = token;
        context->next_parser_token = peek_token;
        if (context->debug_mode >= 2) fprintf(stderr, "[GLUE] Line %d: Passing Token %d (%s) to Parser\n", context->line, token_type, token_to_string(token_type));
        RexxB(parser, token_type, token, context);
        last_token_type = token_type;
    }

    /* Deallocate parser */
    RexxBFree(parser, free);
    return (0);
}
