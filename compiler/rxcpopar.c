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
 * Options Parser Wrapper
 */

#include <stdlib.h>
#include "rxcpopgr.h"
#include "rxcpmain.h"

int opt_pars(Context *context) {

    int token_type, last_token_type;
    Token *token;
    void *parser;

    /* Create Options parser to work out required language level */
    parser = Opts_Alloc(malloc);
#ifndef NDEBUG
    if (context->debug_mode >= 2) Opts_Trace(stderr, "[OPTIONS] ");
    else Opts_Trace(context->traceFile, "Options parser >> ");
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
        if (context->processedOptions) {
            break;
        }
    }

    /* Deallocate memory */
    Opts_Free(parser, free);
    return(0);
}
