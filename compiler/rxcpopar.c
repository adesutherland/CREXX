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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpopgr.h"
#include "rxcpmain.h"

static RexxLevel header_cli_or_default_level(Context *context) {
    if (context && context->cli_default_level != UNKNOWN) return context->cli_default_level;
    return LEVELC;
}

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

static int token_text_equals_ci(const Token *token, const char *text) {
    size_t i;
    size_t len;

    if (!token || !token->token_string || !text) return 0;
    len = strlen(text);
    if ((size_t) token->length != len) return 0;

    for (i = 0; i < len; i++) {
        if (tolower((unsigned char) token->token_string[i]) !=
            tolower((unsigned char) text[i])) {
            return 0;
        }
    }

    return 1;
}

int rxcp_scan_source_header(const char *location, const char *file_name, RexxLevel cli_default_level,
                            RexxLevel *level_out, char **namespace_out) {
    Context *context;
    Token *token;
    int token_type;
    int clause_kind;
    int at_clause_start;
    int expect_namespace_name;
    size_t bytes;
    char *buff_start;

    if (level_out) *level_out = cli_default_level != UNKNOWN ? cli_default_level : LEVELC;
    if (namespace_out) *namespace_out = 0;
    if (!file_name) return -1;

    context = cntx_f();
    context->cli_default_level = cli_default_level;
    context->file_pointer = openfile((char *) file_name, "", (char *) location, "r");
    if (!context->file_pointer) {
        fre_cntx(context);
        return -1;
    }

    buff_start = file2buf(context->file_pointer, &bytes);
    fclose(context->file_pointer);
    context->file_pointer = 0;
    if (!buff_start) {
        fre_cntx(context);
        return -1;
    }

    cntx_buf(context, buff_start, bytes);
    context->file_name = (char *) filename(file_name);

    clause_kind = 0;
    at_clause_start = 1;
    expect_namespace_name = 0;

    while ((token_type = opt_scan(context))) {
        token = token_f(context, token_type);
        if (!token) break;

        if (token_type == TK_EOC) {
            at_clause_start = 1;
            clause_kind = 0;
            expect_namespace_name = 0;
            continue;
        }

        if (token_type == TK_EOS) break;

        if (at_clause_start) {
            at_clause_start = 0;
            if (token_type == TK_OPTIONS) {
                clause_kind = 1;
                continue;
            }
            if (token_type == TK_SYMBOL && token_text_equals_ci(token, "namespace")) {
                clause_kind = 2;
                expect_namespace_name = 1;
                continue;
            }
            if (token_type == TK_SYMBOL && token_text_equals_ci(token, "import")) {
                clause_kind = 3;
                continue;
            }
            break;
        }

        if (clause_kind == 1) {
            switch (token_type) {
                case TK_LEVELA: context->level = LEVELA; break;
                case TK_LEVELB: context->level = LEVELB; break;
                case TK_LEVELC: context->level = LEVELC; break;
                case TK_LEVELD: context->level = LEVELD; break;
                case TK_LEVELG: context->level = LEVELG; break;
                case TK_LEVELL: context->level = LEVELL; break;
                default: break;
            }
            continue;
        }

        if (clause_kind == 2 && expect_namespace_name && token_type == TK_SYMBOL) {
            if (namespace_out && !*namespace_out) {
                *namespace_out = rx_strndup(token->token_string, (size_t) token->length);
            }
            expect_namespace_name = 0;
            continue;
        }
    }

    if (level_out) {
        *level_out = context->level == UNKNOWN ? header_cli_or_default_level(context) : context->level;
    }

    fre_cntx(context);
    return 0;
}
