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

#include "compiler.h"
#include <stdio.h>
}

/* Program Structure */
program            ::= rexx_options after_the_options.
program            ::= after_the_options.
program            ::= error.
                   { if (context->level == UNKNOWN) context->level = LEVELC; }

rexx_options       ::= TK_OPTIONS TK_EOC.
                   { if (context->level == UNKNOWN) context->level = LEVELC; }

rexx_options       ::= TK_OPTIONS option_list TK_EOC.
                   { if (context->level == UNKNOWN) context->level = LEVELC; }

option_list        ::= option.
option_list        ::= option_list option.

option             ::= TK_LEVELA. { context->level = LEVELA; }
option             ::= TK_LEVELB. { context->level = LEVELB; }
option             ::= TK_LEVELC. { context->level = LEVELC; }
option             ::= TK_LEVELD. { context->level = LEVELD; }
option             ::= TK_LEVELG. { context->level = LEVELG; }
option             ::= TK_LEVELL. { context->level = LEVELL; }
option             ::= TK_SYMBOL. /* For some other language processor */

after_the_options  ::= ANYTHING.
                   { if (context->level == UNKNOWN) context->level = LEVELC; }

after_the_options  ::= after_the_options ANYTHING.
