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

static int static_option_word(int type) {
    switch (type) {
        case TK_LEVELA: case TK_LEVELB: case TK_LEVELC:
        case TK_LEVELD: case TK_LEVELG: case TK_LEVELL:
        case TK_COMMENTS_HASH: case TK_COMMENTS_DASH:
        case TK_COMMENTS_SLASH: case TK_COMMENTS_NOHASH:
        case TK_COMMENTS_NODASH: case TK_COMMENTS_NOSLASH:
        case TK_NUMERIC_COMMON: case TK_NUMERIC_CLASSIC:
        case TK_SYMBOL:
            return 1;
        default:
            return 0;
    }
}

int opt_pars(Context *context) {
    int token_type;
    int static_clause = 1;
    int hash_explicit = 0;
    Token *token;
    Token *options = NULL;
    Token *cursor;
    void *parser = NULL;

    /* Inspect the complete first clause before applying any static words.
     * OPTIONS expressions containing punctuation are executable source,
     * not compiler directives (for example OPTIONS levelc()). */
    while ((token_type = opt_scan(context))) {
        token = token_f(context, token_type);
        if (!options) {
            if (token_type == TK_EOC) continue;
            if (token_type != TK_OPTIONS) break;
            options = token;
            continue;
        }
        if (token_type == TK_EOC || token_type == TK_EOS) {
            break;
        }
        if (!static_option_word(token_type)) static_clause = 0;
    }

    if (options) {
        context->source_has_options = 1;
        if (static_clause) {
            parser = Opts_Alloc(malloc);
#ifndef NDEBUG
            if (context->debug_mode >= 2) Opts_Trace(stderr, "[OPTIONS] ");
            else Opts_Trace(context->traceFile, "Options parser >> ");
#endif
            for (cursor = options; cursor && cursor != token; cursor = cursor->token_next) {
                if (cursor->token_type == TK_SYMBOL &&
                    token_text_equals_ci(cursor, "srcmap")) {
                    context->source_has_srcmap = 1;
                }
                if (cursor->token_type == TK_COMMENTS_HASH ||
                    cursor->token_type == TK_COMMENTS_NOHASH) {
                    hash_explicit = 1;
                }
                Opts_(parser, cursor->token_type, cursor, context);
            }
            Opts_(parser, TK_EOC, token, context);
            Opts_(parser, TK_EOS, token, context);
            Opts_(parser, 0, NULL, context);
            Opts_Free(parser, free);
        }
    }

    if (context->level == UNKNOWN) context->level = header_cli_or_default_level(context);
    if (context->level == LEVELC && !hash_explicit)
        context->comments_hash = 0;
    context->processedOptions = 1;
    return 0;
}

int rxcp_scan_source_header(const char *location, const char *file_name, RexxLevel cli_default_level,
                            RexxLevel *level_out, char **namespace_out) {
    Context *context;
    Token *token;
    int token_type;
    int clause_kind;
    int at_clause_start;
    int expect_namespace_name;
    int option_clause_static;
    RexxLevel option_clause_level;
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
    if (fclose(context->file_pointer) != 0) {
        free(buff_start);
        buff_start = 0;
    }
    context->file_pointer = 0;
    if (!buff_start) {
        fprintf(stderr, "Can't read input file header %s\n", file_name);
        fre_cntx(context);
        return -1;
    }

    cntx_buf(context, buff_start, bytes);
    context->file_name = (char *) filename(file_name);

    clause_kind = 0;
    at_clause_start = 1;
    expect_namespace_name = 0;
    option_clause_static = 1;
    option_clause_level = UNKNOWN;

    while ((token_type = opt_scan(context))) {
        token = token_f(context, token_type);
        if (!token) break;

        if (token_type == TK_EOC) {
            if (clause_kind == 1 && option_clause_static &&
                option_clause_level != UNKNOWN) context->level = option_clause_level;
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
                option_clause_static = 1;
                option_clause_level = UNKNOWN;
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
            if (!static_option_word(token_type)) option_clause_static = 0;
            switch (token_type) {
                case TK_LEVELA: option_clause_level = LEVELA; break;
                case TK_LEVELB: option_clause_level = LEVELB; break;
                case TK_LEVELC: option_clause_level = LEVELC; break;
                case TK_LEVELD: option_clause_level = LEVELD; break;
                case TK_LEVELG: option_clause_level = LEVELG; break;
                case TK_LEVELL: option_clause_level = LEVELL; break;
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

    if (clause_kind == 1 && option_clause_static &&
        option_clause_level != UNKNOWN) context->level = option_clause_level;

    if (level_out) {
        *level_out = context->level == UNKNOWN ? header_cli_or_default_level(context) : context->level;
    }

    fre_cntx(context);
    return 0;
}
