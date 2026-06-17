%name LevelCFallbackNoThen
%token_type { PocToken }
%default_type { int }
%extra_argument { PocContext *ctx }
%start_symbol program
%stack_size 0

%include {
#include <stdlib.h>
#include "fallback_poc_shared.h"
}

%fallback ID IF SAY DO.

%syntax_error {
    poc_syntax_error(ctx, TOKEN);
}

%parse_failure {
    poc_parse_failure(ctx);
}

%parse_accept {
    poc_parse_accept(ctx);
}

program ::= statement_list EOS.

statement_list ::= statement_list statement EOC.
statement_list ::= .

statement ::= assignment.
statement ::= say_stmt.
statement ::= if_stmt.

assignment ::= ID(A) EQUAL expr(B). {
    poc_event2(ctx, "assign", A.text, B.text);
}

say_stmt ::= SAY(K) expr(E). {
    poc_event2(ctx, "say", K.text, E.text);
}

if_stmt ::= IF expr THEN statement. {
    poc_event(ctx, "if");
}

%type expr { PocToken }

expr(E) ::= ID(A). {
    E = A;
}
