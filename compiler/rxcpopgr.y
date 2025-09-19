/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */
/* Grammar                      */

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
}

/* Program Structure */
program            ::= rexx_options after_the_options.
program            ::= after_the_options.
program            ::= error.
                   {
                        if (context->level == UNKNOWN) context->level = LEVELC;
                        context->processedOptions = 1;
                   }

rexx_options       ::= TK_OPTIONS TK_EOC.
                   {
                        if (context->level == UNKNOWN) context->level = LEVELC;
                        context->processedOptions = 1;
                   }

rexx_options       ::= TK_OPTIONS option_list TK_EOC.
                   {
                        if (context->level == UNKNOWN) context->level = LEVELC;
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
                        if (context->level == UNKNOWN) context->level = LEVELC;
                        context->processedOptions = 1;
                   }

after_the_options  ::= after_the_options ANYTHING.
