/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "rxcpbgmr.h"
#include "rxcpmain.h"

int rexbpars(Context *context) {

    char *buff, *buff_end;
    size_t bytes;
    int token_type, last_token_type;
    Token *token, *t, *peek_token;
    void *parser;

    /* Create parser and set up tracing */
    parser = RexxBAlloc(malloc);
#ifndef NDEBUG
    RexxBTrace(context->traceFile, "Parser(B) >> ");
#endif

    peek_token = token_f(context, rexbscan(context));
    last_token_type = TK_EOC;
    while (1) {
        token = peek_token;
        if (token->token_type == TK_EOL) token->token_type = TK_EOC;
        token_type = token->token_type;

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

        peek_token = token_f(context, rexbscan(context));

        // Line Continuation
        if (token_type == TK_COMMA && peek_token->token_type == TK_EOL) {
            token_r(context);  /* Discard tokens , and EOC tokens */
            token_r(context);
            peek_token = token_f(context, rexbscan(context));
            continue;
        }

        // Skip multiple end of clause/line
        if (last_token_type == TK_EOC && token_type == TK_EOC) continue;


        /* Special Processing */
        if (token_type == TK_MINUSMINUS) {
            /* TODO Check for Check for C operator mode - when/if implemented!    */
            /* The TK_MINUSMINUS token is generated by -- in nodashcomments mode, */
            /* it needs to be converted to two TK_MINUS tokens                    */
            t = tok_splt(context, token, 1);
            token = t->token_next; /* In case tok_splt() changes token pointer - it doesn't currently */
            token_type = TK_MINUS;
            t->token_type = token_type;
            token->token_type = token_type;
            RexxB(parser, token_type, t, context);
        }

        RexxB(parser, token_type, token, context);
        last_token_type = token_type;
    }

    /* Deallocate parser */
    RexxBFree(parser, free);
    return (0);
}
