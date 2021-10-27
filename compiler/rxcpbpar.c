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
    Token *token, *t;
    void *parser;

    /* Create parser and set up tracing */
    parser = RexxBAlloc(malloc);
#ifndef NDEBUG
    RexxBTrace(context->traceFile, "Parser(B) >> ");
#endif
    last_token_type = TK_EOC;
    while ((token_type = rexbscan(context))) {
        // Setup and parse token
        token = token_f(context, token_type);

        // Skip multiple end of clause/line
        if (last_token_type == TK_EOC && token_type == TK_EOC) continue;

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

        RexxB(parser, token_type, token, context);
        last_token_type = token_type;
    }

    /* Deallocate parser */
    RexxBFree(parser, free);
    return (0);
}
