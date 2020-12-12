%token_type { Token* }

%default_type { ASTNode* }

// Token only used in AST
%nonassoc TK_NULL TK_VREF TK_LIST TK_FUNCTION TK_REP TK_PARSEERROR.

%include {
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "compiler.h"
}

%code {
    const char* token_type_name(int type) {
        return yyTokenName[type];
    }
}

%extra_argument { Scanner *context }

%parse_accept { printf("The parser has completed successfully.\n"); }

%syntax_error {
        int i, n;
        fprintf(stderr, "Error line %d - expecting", context->line+1);
        n = sizeof(yyTokenName) / sizeof(yyTokenName[0]);
        for (i = 0; i < n; ++i) {
                int a = yy_find_shift_action((YYCODETYPE)yymajor, (YYCODETYPE)i);
                if (a < YYNSTATE + YYNRULE) {
                        fprintf(stderr," %s", yyTokenName[i]);
                }
        }
        fprintf(stderr, "\n");
}

%parse_failure { fprintf(stderr, "Parse failure\n"); }

%start_symbol program

// Symbols can be keywords
%fallback TK_SYMBOL
          TK_ADDRESS TK_ARG TK_BY TK_CALL TK_DIGITS TK_DO TK_DROP  TK_END
          TK_ENGINEERING TK_ERROR TK_EXIT TK_EXPOSE TK_EXTERNAL TK_FAILURE TK_FOR
          TK_FOREVER TK_FORM TK_FUZZ TK_HALT  TK_INTERPRET TK_ITERATE TK_LEAVE
          TK_NAME TK_NOP TK_NOVALUE TK_NUMERIC TK_OFF TK_ON TK_OPTIONS TK_OTHERWISE
          TK_PARSE TK_PROCEDURE TK_PULL TK_PUSH TK_QUEUE TK_RETURN TK_SAY TK_SCIENTIFIC
          TK_SELECT TK_SIGNAL TK_SOURCE TK_STOP TK_SYNTAX TK_TO TK_TRACE TK_UNTIL
          TK_UPPER TK_VALUE TK_VAR TK_VERSION TK_WHEN TK_WHILE TK_WITH.

// TK_IF TK_THEN TK_ELSE


// Make TK_EQUAL (for assignments) have higher priority
%nonassoc TK_SYMBOL TK_SYMBOL_STEM TK_SYMBOL_COMPOUND.
%nonassoc TK_EQUAL.

// EXPRESSIONS
term(T)                ::= var_symbol(V). [TK_SYMBOL] { T = V; }
term(T)                ::= TK_CONST(C). { T = ast_f(C); }
term(T)                ::= TK_STRING(S). { T = ast_f(S); }
term(T)                ::= function(F). [TK_SYMBOL] { T = F; }

bracket(B)             ::= term(T). { B = T; }
bracket(B)             ::= TK_BOPEN expression(E) TK_BCLOSE. { B = E; }

prefix_expression(P)   ::= bracket(B). { P = B; }
prefix_expression(P)   ::= TK_PLUS(T) prefix_expression(E). { P = ast_f(T); add_ast(P,E); }
prefix_expression(P)   ::= TK_MINUS(T) prefix_expression(E). { P = ast_f(T); add_ast(P,E); }
prefix_expression(P)   ::= TK_NOT(T) prefix_expression(E). { P = ast_f(T); add_ast(P,E); }

power_expression(P)    ::= prefix_expression(E). { P = E; }
power_expression(P)    ::= power_expression(E1) TK_POWER(T) prefix_expression(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

multiplication(P)      ::= power_expression(E). { P = E; }
multiplication(P)      ::= multiplication(E1) TK_MULT(T) power_expression(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
multiplication(P)      ::= multiplication(E1) TK_DIV(T) power_expression(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
multiplication(P)      ::= multiplication(E1) TK_IDIV(T) power_expression(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
multiplication(P)      ::= multiplication(E1) TK_REMAIN(T) power_expression(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

addition(P)            ::= multiplication(E). { P = E; }
addition(P)            ::= addition(E1) TK_PLUS(T) multiplication(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
addition(P)            ::= addition(E1) TK_MINUS(T) multiplication(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

concatenation(P)       ::= addition(E). { P = E; }
concatenation(P)       ::= concatenation(E1) term(E2).
                           { P = ast_ft(TK_CONCAT); add_ast(P,E1); add_ast(P,E2); }
concatenation(P)       ::= concatenation(E1) TK_CONCAT(T) addition(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

comparison(P)          ::= concatenation(E). { P = E; }
comparison(P)          ::= comparison(E1) TK_EQUAL(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_NOT_EQUAL(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_GT(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_LT(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_GE(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_LE(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_EQUAL_EQUAL(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_NOT_EQUAL_EQUAL(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_GT_STRICT(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_LT_STRICT(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_GE_STRICT(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
comparison(P)          ::= comparison(E1) TK_LE_STRICT(T) concatenation(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

or_expression(P)       ::= comparison(E). { P = E; }
or_expression(P)       ::= or_expression(E1) TK_OR(T) comparison(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}
or_expression(P)       ::= or_expression(E1) TK_XOR(T) comparison(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

and_expression(P)      ::= or_expression(E). { P = E; }
and_expression(P)      ::= and_expression(E1) TK_AND(T) or_expression(E2).
                           { P = ast_f(T); add_ast(P,E1); add_ast(P,E2);}

expression(P)          ::= and_expression(E). { P = E; }

valueexp(P)            ::= TK_VALUE(T) expression(E). { P = ast_f(T); add_ast(P,E); }

expression0(P)         ::= . { P = ast_ft(TK_NULL); }
expression0(P)         ::= expression(E). { P = E; }

expression_list(L)     ::= . { L = ast_ft(TK_LIST); }
expression_list(L)     ::= expression_list1(E). { L = E; }
expression_list1(L)    ::= expression(E). { L = ast_ft(TK_LIST); add_ast(L,E); }
expression_list1(L)    ::= expression_list1(L1) TK_COMMA expression(E). { L = L1; add_ast(L,E); }

// VARIABLE & SYMBOLS
var_symbol(T)          ::= TK_SYMBOL(S). { T = ast_f(S); }
var_symbol(T)          ::= TK_SYMBOL_STEM(S). { T = ast_f(S); }
var_symbol(T)          ::= TK_SYMBOL_COMPOUND(S). { T = ast_f(S); }
var_symbol_list1(L)    ::= var_symbol(V). { L = ast_ft(TK_LIST); add_ast(L,V); }
var_symbol_list1(L)    ::= var_symbol_list1(L1) var_symbol(V). { L = L1; add_ast(L,V); }

variable(V)            ::= vref(R). { V = R; }
variable(V)            ::= var_symbol(S). { V = S; }
vref(V)                ::= TK_BOPEN var_symbol(R) TK_BCLOSE. { V = ast_ft(TK_VREF); add_ast(V,R); }
variable_list(L)       ::= variable(V). { L = ast_ft(TK_LIST); add_ast(L,V); }
variable_list(L)       ::= variable_list(L1) variable(V). { L = L1; add_ast(L,V); }

// FUNCTIONS
function(F)            ::= taken_constant(T) function_parameters(P).
                           { F = ast_ft(TK_FUNCTION); add_ast(F,T); add_ast(F,P); }
taken_constant(T)      ::= TK_SYMBOL(S). { T = ast_f(S); }
taken_constant(T)      ::= TK_SYMBOL_COMPOUND(S). { T = ast_f(S); }
taken_constant(T)      ::= TK_STRING(S). { T = ast_f(S); }
function_parameters(P) ::= TK_BOPEN expression_list(E) TK_BCLOSE. { P = E; }

// TEMPLATES
template_list(L)       ::= . { L = ast_ft(TK_LIST); }
template_list(L)       ::= templates(T). { L = ast_ft(TK_LIST); add_ast(L,T); }
template_list(L)       ::= template_list(L1) TK_COMMA templates(T). { L = L1; add_ast(L,T); }
templates(L)           ::= template(T). { L = ast_ft(TK_LIST); add_ast(L,T); }
templates(L)           ::= templates(L1) template(T). { L = L1; add_ast(L,T); }
template(T)            ::= var_symbol(V). { T = V; }
template(T)            ::= TK_STOP(TK). { T = ast_f(TK); }
template(T)            ::= TK_STRING(TK). { T = ast_f(TK); }
template(T)            ::= vref(V). { T = V; }
template(T)            ::= TK_CONST(TK). { T = ast_f(TK); }
template(T)            ::= TK_EQUAL(TK) position(P). { T = ast_f(TK); add_ast(T,P); }
template(T)            ::= TK_PLUS(TK) position(P). { T = ast_f(TK); add_ast(T,P); }
template(T)            ::= TK_MINUS(TK) position(P). { T = ast_f(TK); add_ast(T,P); }
position(P)            ::= TK_CONST(TK). { P = ast_f(TK); }
position(P)            ::= vref(V). { P = V; }

// INSTRUCTIONS
address(I)             ::= TK_ADDRESS(T). { I = ast_f(T); }
address(I)             ::= TK_ADDRESS(T) taken_constant(A1). { I = ast_f(T); add_ast(I,A1); }
address(I)             ::= TK_ADDRESS(T) taken_constant(A1) expression(A2).
                           { I = ast_f(T); add_ast(I,A1); add_ast(I,A2); }
address(I)             ::= TK_ADDRESS(T) valueexp(P). { I = ast_f(T); add_ast(I,P); }
arg(I)                 ::= TK_ARG(T) template_list(P). { I = ast_f(T); add_ast(I,P); }

call(I)                ::= TK_CALL(T) callon_spec(P). { I = ast_f(T); add_ast(I,P); }
call(I)                ::= TK_CALL(T) taken_constant(P) expression_list(E).
                           { I = ast_f(T); add_ast(I,P); add_ast(I,E); }
callon_spec(I)         ::= TK_ON(T) callable_condition(P). { I = ast_f(T); add_ast(I,P); }
callon_spec(I)         ::= TK_ON(T) callable_condition(P1) TK_NAME taken_constant(P2).
                          { I = ast_f(T); add_ast(I,P1); add_ast(I,P2); }
callon_spec(I)         ::= TK_OFF(T) callable_condition(P). { I = ast_f(T); add_ast(I,P); }
callable_condition(I)  ::= TK_ERROR(T). { I = ast_f(T); }
callable_condition(I)  ::= TK_FAILURE(T). { I = ast_f(T); }
callable_condition(I)  ::= TK_HALT(T). { I = ast_f(T); }

drop(I)                ::= TK_DROP(T) variable_list(P). { I = ast_f(T); add_ast(I,P); }

exit(I)                ::= TK_EXIT(T) expression0(P). { I = ast_f(T); add_ast(I,P); }
interpret(I)           ::= TK_INTERPRET(T) expression(P). { I = ast_f(T); add_ast(I,P); }
iterate(I)             ::= TK_ITERATE(T). { I = ast_f(T); }
iterate(I)             ::= TK_ITERATE(T) var_symbol(P). { I = ast_f(T); add_ast(I,P); }
leave(I)               ::= TK_LEAVE(T). { I = ast_f(T); }
leave(I)               ::= TK_LEAVE(T) var_symbol(P). { I = ast_f(T); add_ast(I,P); }
nop(I)                 ::= TK_NOP(T). { I = ast_f(T); }

numeric(I)             ::= TK_NUMERIC(T) TK_DIGITS(T2) expression0(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }
numeric(I)             ::= TK_NUMERIC(T) TK_FORM(P). { I = ast_f(T); add_ast(I,ast_f(P)); }
numeric(I)             ::= TK_NUMERIC(T) TK_FORM TK_ENGINEERING(P). { I = ast_f(T); add_ast(I,ast_f(P)); }
numeric(I)             ::= TK_NUMERIC(T) TK_FORM TK_SCIENTIFIC(P). { I = ast_f(T); add_ast(I,ast_f(P)); }
numeric(I)             ::= TK_NUMERIC(T) TK_FORM(T2) valueexp(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }
numeric(I)             ::= TK_NUMERIC(T) TK_FORM(T2) expression(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }
numeric(I)             ::= TK_NUMERIC(T) TK_FUZZ(T2) expression0(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }

options(I)             ::= TK_OPTIONS(T) expression(P). { I = ast_f(T); add_ast(I,P); }

parse(I)               ::= TK_PARSE(T) parse_type(PT) template_list(L).
                           { I = ast_f(T); add_ast(I,PT); add_ast(I,L);}
parse(I)               ::= TK_PARSE(T) TK_UPPER(U) parse_type(PT) template_list(L).
                           { I = ast_f(T); add_ast(I,PT); add_ast(I,L); add_ast(I,ast_f(U)); }
parse_type(I)          ::= TK_ARG(T). { I = ast_f(T); }
parse_type(I)          ::= TK_EXTERNAL(T). { I = ast_f(T); }
parse_type(I)          ::= TK_NUMERIC(T). { I = ast_f(T); }
parse_type(I)          ::= TK_PULL(T). { I = ast_f(T); }
parse_type(I)          ::= TK_SOURCE(T). { I = ast_f(T); }
parse_type(I)          ::= TK_VERSION(T). { I = ast_f(T); }
parse_type(I)          ::= TK_LINEIN(T). { I = ast_f(T); }
parse_type(I)          ::= TK_VALUE(T) expression0(E) TK_WITH.
                           { I = ast_f(T); add_ast(I,E); }
parse_type(I)          ::= TK_VAR(T) var_symbol(V).
                           { I = ast_f(T); add_ast(I,V); }

procedure(I)           ::= TK_PROCEDURE(T). { I = ast_f(T); }
procedure(I)           ::= TK_PROCEDURE(T) TK_EXPOSE(T2) variable_list(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }
pull(I)                ::= TK_PULL(T) template_list(V). { I = ast_f(T); add_ast(I,V); }
push(I)                ::= TK_PUSH(T) expression0(V). { I = ast_f(T); add_ast(I,V); }
queue(I)               ::= TK_QUEUE(T) expression0(V). { I = ast_f(T); add_ast(I,V); }
return(I)              ::= TK_RETURN(T) expression0(V). { I = ast_f(T); add_ast(I,V); }
say(I)                 ::= TK_SAY(T) expression0(V). { I = ast_f(T); add_ast(I,V); }
signal(I)              ::= TK_SIGNAL(T) valueexp(V). { I = ast_f(T); add_ast(I,V); }
signal(I)              ::= TK_SIGNAL(T) taken_constant(V). { I = ast_f(T); add_ast(I,V); }
signal(I)              ::= TK_SIGNAL(T) TK_ON(T2) condition(P) TK_NAME taken_constant(N).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); add_ast(I, N); }
signal(I)              ::= TK_SIGNAL(T) TK_ON(T2) condition(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }
signal(I)              ::= TK_SIGNAL(T) TK_OFF(T2) condition(P).
                           { I = ast_f(T); add_ast(add_ast(I, ast_f(T2)), P); }
condition(I)           ::= callable_condition(C). { I = C; }
condition(I)           ::= TK_NOVALUE(T). { I = ast_f(T); }
condition(I)           ::= TK_SYNTAX(T). { I = ast_f(T); }
trace(I)               ::= TK_TRACE(T) taken_constant(P). { I = ast_f(T); add_ast(I,P); }
trace(I)               ::= TK_TRACE(T) valueexp(P). { I = ast_f(T); add_ast(I,P); }
trace(I)               ::= TK_TRACE(T) TK_ERROR(T2). { I = ast_f(T); add_ast(I,ast_f(T2)); }
trace(I)               ::= TK_TRACE(T) TK_FAILURE(T2). { I = ast_f(T); add_ast(I,ast_f(T2)); }
trace(I)               ::= TK_TRACE(T) TK_OFF(T2). { I = ast_f(T); add_ast(I,ast_f(T2)); }
upper(I)               ::= TK_UPPER(T) var_symbol_list1(P). { I = ast_f(T); add_ast(I,P); }

do(I)                  ::= TK_DO(T) do_rep(A) do_cond(B) delim program_instructions(C) TK_END.
                           { I = ast_f(T); add_ast(I,A); add_ast(I,B); add_ast(I,C); add_ast(I,ast_ft(TK_NULL)); }
do(I)                  ::= TK_DO(T) do_rep(A) do_cond(B) delim program_instructions(C) TK_END var_symbol(D).
                           { I = ast_f(T); add_ast(I,A); add_ast(I,B); add_ast(I,C); add_ast(I,D); }
do_rep(I)              ::= . { I = ast_ft(TK_NULL); }
do_rep(I)              ::= assignment(A) do_cnt(B). { I = ast_ft(TK_REP); add_ast(I,A); add_ast(I,B); }
do_rep(I)              ::= assignment(A). { I = ast_ft(TK_REP); add_ast(I,A); }
do_rep(I)              ::= TK_FOREVER(A). { I = ast_ft(TK_REP); add_ast(I,ast_f(A)); }
do_rep(I)              ::= expression(A). { I = ast_ft(TK_REP); add_ast(I,A); }
cnt(I)                 ::= dob(A). { I = A; }
cnt(I)                 ::= dof(A). { I = A; }
cnt(I)                 ::= dot(A). { I = A; }
do_cnt(I)              ::= cnt(A). { I = ast_ft(TK_LIST); add_ast(I,A); }
do_cnt(I)              ::= do_cnt(I1) cnt(A). { I = I1; add_ast(I,A); }
dot(I)                 ::= TK_TO(T) expression(A). { I = ast_f(T); add_ast(I,A); }
dob(I)                 ::= TK_BY(T) expression(A). { I = ast_f(T); add_ast(I,A); }
dof(I)                 ::= TK_FOR(T) expression(A). { I = ast_f(T); add_ast(I,A); }
do_cond(I)             ::= . { I = ast_ft(TK_NULL); }
do_cond(I)             ::= TK_WHILE(T) expression(A). { I = ast_f(T); add_ast(I,A); }
do_cond(I)             ::= TK_UNTIL(T) expression(A). { I = ast_f(T); add_ast(I,A); }
if(I)                  ::= TK_IF(T) expression(E) delim0
                           TK_THEN ncl0 instruction(S1).
                           { I = ast_f(T); add_ast(I,E); add_ast(I,S1);}
else(I)                ::= TK_ELSE(T) ncl0 instruction(S1).
                           { I = ast_f(T); add_ast(I,S1); }
select(I)              ::= TK_SELECT(T) delim0 when_list(A) TK_END.
                           { I = ast_f(T); add_ast(I,A); add_ast(I,ast_ft(TK_NULL)); }
select(I)              ::= TK_SELECT(T) delim0 when_list(A) otherwise(B) TK_END.
                           { I = ast_f(T); add_ast(I,A); add_ast(I,B); }
when_list(I)           ::= when(A). { I = ast_ft(TK_LIST); add_ast(I,A); }
when_list(I)           ::= when_list(I1) when(A). { I = I1; add_ast(I,A); }
when(I)                ::= TK_WHEN(T) expression(E) delim0
                           TK_THEN ncl0 instruction(S) ncl0.
                           { I = ast_f(T); add_ast(I,E); add_ast(I,S); }
otherwise(I)           ::= TK_OTHERWISE(T) program_instructions(P). { I = ast_f(T); add_ast(I,P); }
command(I)             ::= expression(E). { I = ast_ft(TK_ADDRESS); add_ast(I,E); }

keyword_instruction(I) ::= address(K). { I = K; }
keyword_instruction(I) ::= arg(K). { I = K; }
keyword_instruction(I) ::= call(K). { I = K; }
keyword_instruction(I) ::= drop(K). { I = K; }
keyword_instruction(I) ::= exit(K). { I = K; }
keyword_instruction(I) ::= interpret(K). { I = K; }
keyword_instruction(I) ::= iterate(K). { I = K; }
keyword_instruction(I) ::= leave(K). { I = K; }
keyword_instruction(I) ::= nop(K). { I = K; }
keyword_instruction(I) ::= numeric(K). { I = K; }
keyword_instruction(I) ::= options(K). { I = K; }
keyword_instruction(I) ::= parse(K). { I = K; }
keyword_instruction(I) ::= procedure(K). { I = K; }
keyword_instruction(I) ::= pull(K). { I = K; }
keyword_instruction(I) ::= push(K). { I = K; }
keyword_instruction(I) ::= queue(K). { I = K; }
keyword_instruction(I) ::= return(K). { I = K; }
keyword_instruction(I) ::= say(K). { I = K; }
keyword_instruction(I) ::= signal(K). { I = K; }
keyword_instruction(I) ::= trace(K). { I = K; }
keyword_instruction(I) ::= upper(K). { I = K; }

assignment(I)          ::= var_symbol(V) TK_EQUAL(T) expression(E). [TK_EQUAL]
                           { I = ast_f(T); add_ast(I,V); add_ast(I,E); }
assignment(I)          ::= TK_SYMBOL_COMPOUND(V) TK_EQUAL(T) expression(E). [TK_EQUAL]
                           { I = ast_f(T); add_ast(I,ast_f(V)); add_ast(I,E); }

instruction(I)         ::= do(K). { I = K; }
instruction(I)         ::= if(K). { I = K; }
instruction(I)         ::= else(K). { I = K; }
instruction(I)         ::= select(K). { I = K; }
instruction(I)         ::= assignment(K). { I = K; }
instruction(I)         ::= keyword_instruction(K). { I = K; }
instruction(I)         ::= command(K). { I = K; }
instruction(I)         ::= error. { I = ast_ft(TK_PARSEERROR); }

// PROGRAM STRUCTURE
ncl0                   ::= .
ncl0                   ::= ncl.
ncl                    ::= ncl null_clause.
ncl                    ::= null_clause.
null_clause            ::= label.
null_clause            ::= delim.

label(I)               ::= TK_LABEL(T). { I = ast_f(T); }

delim                  ::= TK_EOC.
delim                  ::= TK_EOL.
delim0                 ::= .
delim0                 ::= delim.


program_instruction(I) ::= label(K). { I = K; }
program_instruction(I) ::= instruction(K) delim. { I = K; }

program_instructions(I)::= program_instruction(A). { I = ast_ft(TK_LIST); add_ast(I,A); }
program_instructions(I)::= delim. { I = ast_ft(TK_LIST); }
program_instructions(I)::= program_instructions(I1) program_instruction(A). { I = I1; add_ast(I,A); }
program_instructions(I)::= program_instructions(I1) delim. { I = I1; }

program(P)             ::= TK_EOF. { P = ast_ft(TK_NULL); context->ast = P; }
program(P)             ::= program_instructions(I) TK_EOF. { P = I;  context->ast = P; }