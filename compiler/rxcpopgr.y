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
 * Options Parser Grammar (Lemon)
 */

/* The concept here is that we parse the program for OPTIONS, and as soon
 * context->level (explicitly or as a default) then proceccing stops. In sum,
 * we are just getting the language level
 */

%name Opts_

%token_type { Token* }
%default_type { ASTNode* }
%extra_argument { Context *context }
%start_symbol program
%nonassoc TK_UNKNOWN TK_EOS.
%wildcard ANYTHING.

%include {
/* cREXX Phase 0 (PoC 2) Compiler */
/* (c) Adrian Sutherland 2021     */
/* OPTIONS Grammar                */

#include <assert.h>
#include <stdio.h>
#include "rxcpmain.h"

static RexxLevel cli_or_default_level(Context *context) {
    if (context->cli_default_level != UNKNOWN) return context->cli_default_level;
    return LEVELC;
}
}

/* Program Structure */
program            ::= rexx_options after_the_options.
program            ::= after_the_options.
program            ::= error.
                   {
                        if (context->level == UNKNOWN) context->level = cli_or_default_level(context);
                        context->processedOptions = 1;
                   }

rexx_options       ::= TK_OPTIONS TK_EOC.
                   {
                        context->source_has_options = 1;
                        if (context->level == UNKNOWN) context->level = cli_or_default_level(context);
                        context->processedOptions = 1;
                   }

rexx_options       ::= TK_OPTIONS option_list TK_EOC.
                   {
                        context->source_has_options = 1;
                        if (context->level == UNKNOWN) context->level = cli_or_default_level(context);
                        context->processedOptions = 1;
                   }

option_list        ::= option.
option_list        ::= option_list option.

option             ::= TK_LEVELA. { context->level = LEVELA; }
option             ::= TK_LEVELB. { context->level = LEVELB; }
option             ::= TK_LEVELC. { context->level = LEVELC; }
option             ::= TK_LEVELD. { context->level = LEVELD; }
option             ::= TK_LEVELG. { context->level = LEVELG; }
option             ::= TK_LEVELL. { context->level = LEVELL; }
option             ::= TK_COMMENTS_HASH. { context->comments_hash = 1; }
option             ::= TK_COMMENTS_DASH. { context->comments_dash = 1; }
option             ::= TK_COMMENTS_SLASH. { context->comments_slash = 1; }
option             ::= TK_COMMENTS_NOHASH. { context->comments_hash = 0; }
option             ::= TK_COMMENTS_NODASH. { context->comments_dash = 0; }
option             ::= TK_COMMENTS_NOSLASH. { context->comments_slash = 0; }
option             ::= TK_NUMERIC_COMMON. { context->numeric_standard = 0; } // Common
option             ::= TK_NUMERIC_CLASSIC. { context->numeric_standard = 1; } // Classic
option             ::= TK_SYMBOL. /* For some other language processor */

after_the_options  ::= ANYTHING.
                   {
                        if (context->level == UNKNOWN) context->level = cli_or_default_level(context);
                        context->processedOptions = 1;
                   }

after_the_options  ::= after_the_options ANYTHING.
