/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

#include <stdlib.h>
#include "opt_grmr.h"
#include "compiler.h"

int opt_pars(Context *context) {

    int token_type, last_token_type;
    Token *token;
    void *parser;

    /* Create Options parser to work out required language level */
    parser = Opts_Alloc(malloc);
#ifndef NDEBUG
    Opts_Trace(context->traceFile, "Options parser >> ");
#endif
    last_token_type = TK_EOC;
    while((token_type = opt_scan(context))) {
        // Setup and parse token
        token = token_f(context, token_type);

        // Skip multiple end of clause/line
        if (last_token_type == TK_EOC && token_type == TK_EOC) continue;
        last_token_type = token_type;

        // EOS Special Processing
        if(token_type == TK_EOS) {
            // Send an EOC
            token = token_f(context, TK_EOC);
            Opts_(parser, TK_EOC, token, context);

            // Send EOS
            token = token_f(context, token_type);
            Opts_(parser, token_type, token, context);

            // Send a null
            Opts_(parser, 0, NULL, context);
            break;
        }

        Opts_(parser, token_type, token, context);

        // Check if we are done
        if (context->level != UNKNOWN) {
            break;
        }
    }

    /* Deallocate memory */
    Opts_Free(parser, free);
    return(0);
}
