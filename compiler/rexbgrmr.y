/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */
/* Grammar                      */

%name RexxB
%token_type { Token* }
%default_type { ASTNode* }
%extra_argument { Context *context }
%start_symbol program

%include {
/* cREXX Phase 0 (PoC 2) Compiler */
/* (c) Adrian Sutherland 2021   */
/* Grammar                      */

#include <assert.h>
#include "compiler.h"
}

%token TK_UNKNOWN.
%wildcard ANYTHING.

/* Program & Structure */
program(P)       ::= rexx_options(R) instruction_list(I) TK_EOS.
                     {
                        P = ast_ft(context, PROGRAM_FILE); context->ast = P;
                        add_ast(P,R);
                        add_ast(P,I);
                     }

/* This case covers when the EOS was handled by an error (e.g. a missing END) in
 * sub-rules */
program(P)       ::= rexx_options(R) instruction_list(I).
                     {
                        P = ast_ft(context, PROGRAM_FILE); context->ast = P;
                        add_ast(P,R);
                        add_ast(P,I);
                     }

program(P)       ::= rexx_options(R) TK_EOS.
                     {
                        P = ast_ft(context, PROGRAM_FILE); context->ast = P;
                        add_ast(P,R);
                     }

program(E)       ::= ANYTHING(T) error.
                     { E = ast_error(context, "49.1", T); context->ast = E; }

program(E)       ::=  error.
                     { E = ast_error_here(context, "49.2"); }

/* Optional EOC */
ncl0             ::= TK_EOC.
ncl0             ::= .

/* Junk sync - also for error reporting */
junk(J)          ::= . { J = 0; }
junk(J)          ::= junk_list(L). { J = L; }
junk_list(L)     ::= ANYTHING(L1).
                   { L = ast_error(context, "49.3", L1); }
junk_list(L)     ::= junk_list(L1) ANYTHING.
                   { L = L1; } /* We are only reporting the first error */

/* Variables / Labels */
var_symbol(A)    ::= TK_VAR_SYMBOL(S). { A = ast_f(context, VAR_SYMBOL, S); }
label(A)         ::= TK_LABEL(S). { A = ast_f(context, LABEL, S); }

/* Language Options */
rexx_options(I)    ::= TK_OPTIONS TK_EOC.
                   { I = 0; }
rexx_options(I)    ::= TK_OPTIONS(T) option_list(L) TK_EOC.
                   { I = ast_f(context, REXX_OPTIONS, T); add_ast(I,L); }
option_list(L)     ::= option(L1).
                   { L = L1; }
option_list(L)     ::= option_list(L1) option(L2).
                   { L = L1; add_sibling_ast(L,L2); }
option(C)          ::= TK_SYMBOL(S).
                   { C = ast_f(context, CONST_SYMBOL, S); }
option(C)          ::= TK_VAR_SYMBOL(S).
                   { C = ast_f(context, CONST_SYMBOL, S); }
option(E)          ::= error.
                   { E = ast_error_here(context, "100.1"); }

instruction_list(I)  ::= labeled_instruction(L).
                         { I = ast_ft(context, INSTRUCTIONS); add_ast(I,L); }
instruction_list(I)  ::= instruction_list(I1) labeled_instruction(L).
                         { I = I1; add_ast(I,L); }

labeled_instruction(I) ::= group(B). { I = B; }
labeled_instruction(I) ::= single_instruction(B) junk(J) TK_EOC.
                           { I = B; add_ast(I, J); }
labeled_instruction(I) ::= label(B). { I = B; }
labeled_instruction(I) ::= TK_EOC. { I = 0; }

instruction(I)         ::= group(B). { I = B; }
instruction(I)         ::= single_instruction(B) junk(J) TK_EOC.
                           { I = B; add_ast(I, J); }

single_instruction(I)  ::= assignment(B). { I = B; }
single_instruction(I)  ::= command(B). { I = B; }
single_instruction(I)  ::= keyword_instruction(B). { I = B; }
single_instruction(E)  ::= error.
                          { E = ast_error_here(context, "49.4"); }

assignment(I) ::=  var_symbol(V) TK_EQUAL(T) expression(E). [TK_VAR_SYMBOL]
    {
        I = ast_f(context, ASSIGN, T); add_ast(I,V); add_ast(I,E);
        V->node_type = VAR_TARGET;
    }

assignment(I) ::=  TK_FLOAT(T) TK_EQUAL expression(E).
    { I = ast_f(context, ASSIGN, T); add_ast(I,ast_error(context, "31.1", T));
      add_ast(I,E); }

assignment(I) ::=  TK_INTEGER(T) TK_EQUAL expression(E).
    { I = ast_f(context, ASSIGN, T); add_ast(I,ast_error(context, "31.1", T));
      add_ast(I,E); }


assignment(I) ::=  TK_CONST_SYMBOL(T) TK_EQUAL expression(E).
    { I = ast_f(context, ASSIGN, T); add_ast(I,ast_error(context, "31.2", T));
      add_ast(I,E); }

command(I)             ::= expression(E).
                       { I = ast_ft(context, ADDRESS); add_ast(I,E); }

//keyword_instruction(I) ::= address(K). { I = K; }
//keyword_instruction(I) ::= arg(K). { I = K; }
//keyword_instruction(I) ::= call(K). { I = K; }
//keyword_instruction(I) ::= iterate(K). { I = K; }
//keyword_instruction(I) ::= leave(K). { I = K; }
//keyword_instruction(I) ::= nop(K). { I = K; }
//keyword_instruction(I) ::= parse(K). { I = K; }
//keyword_instruction(I) ::= procedure(K). { I = K; }
//keyword_instruction(I) ::= pull(K). { I = K; }
//keyword_instruction(I) ::= return(K). { I = K; }
keyword_instruction(I) ::= say(K). { I = K; }
keyword_instruction(I) ::= TK_THEN(T) error. { I = ast_error(context, "8.1", T); }
keyword_instruction(I) ::= TK_ELSE(T) error. { I = ast_error(context, "8.2", T); }
keyword_instruction(I) ::= TK_WHEN(T) error. { I = ast_error(context, "9.1", T); }
keyword_instruction(I) ::= TK_OTHERWISE(T) error. { I = ast_error(context, "9.2", T); }
keyword_instruction(I) ::= TK_END(T) error. { I = ast_error(context, "10.1", T); }

group(I) ::= simple_do(K). { I = K; }
//group(I) ::= do(K). { I = K; }
group(I) ::= if(K). { I = K; }

/* Groups */

/* Simple DO Group */
simple_do(G) ::= TK_DO TK_EOC instruction_list(I) TK_END TK_EOC.
          { G = I; }
simple_do(G) ::= TK_DO ANYTHING(E).
          { G = ast_error(context, "14.1", E); }
simple_do(G) ::= TK_DO(E) TK_EOS.
          { G = ast_error(context, "35.1", E); }
simple_do(G) ::= TK_DO(E) TK_EOC instruction_list(I) TK_EOS.
          { G = I; add_ast(G,ast_error(context, "14.1", E)); }
simple_do(G) ::= TK_DO TK_EOC instruction_list(I) ANYTHING(E).
          { G = I; add_ast(G,ast_error(context, "35.1", E)); }

/* DO Group */
/*
    do ::= 'DO' r:dorep (ncl / t:. error -> (ERROR[27.1] t)) i:instruction_list? do_ending
       -> (DO r i);

    do_ending ::= 'END' VAR_SYMBOL? ncl
		 / TK_EOS -> ERROR[14.1]
		 / t:. resync -> (ERROR[35.1] t);

    dorep ::= ( a:assignment {? t:dot b:dob f:dof} )
          -> (REPEAT a t? b? f?);
    dot ::= 'TO' e:expression -> (TO e);
    dob ::= 'BY' e:expression -> (BY e);
    dof ::= 'FOR' e:expression -> (FOR e);
*/

/* IF Group */
%nonassoc TK_IF.
%nonassoc TK_ELSE.
if(I) ::= TK_IF(K) expression(E) ncl0 then(T) else(F).
          { I = ast_f(context, IF, K); add_ast(I,E); add_ast(I,T); add_ast(I,F); }
if(I) ::= TK_IF(K) expression(E) ncl0 then(T).
          { I = ast_f(context, IF, K); add_ast(I,E); add_ast(I,T); }
if(I) ::= TK_IF expression ncl0 ANYTHING(E).
          { I = ast_error(context, "18.1", E); }

then(T) ::= TK_THEN ncl0 instruction(I).
            { T = I; }
then(T) ::= TK_THEN(E) ncl0 TK_EOS.
            { T = ast_error(context, "14.3", E); }
then(T) ::= TK_THEN ncl0 TK_END(E).
            { T = ast_error(context, "10.5", E); }

else(T) ::= TK_ELSE ncl0 instruction(I).
            { T = I; }
else(T) ::= TK_ELSE(E) ncl0 TK_EOS.
            { T = ast_error(context, "14.4", E); }
else(T) ::= TK_ELSE ncl0 TK_END(E).
            { T = ast_error(context, "10.6", E); }

/* Instructions */

/*
### ADDRESS
    address ::= 'ADDRESS' e:taken_constant c:expression? -> (ADDRESS ENVIRONMENT[e] c);

### Arg
    arg ::= 'ARG' t:template_list?
        -> (PARSE (OPTIONS UPPER?) ARG t?)

### Call
    call ::= 'CALL' (f:taken_constant / ( (. -> ERROR[19.2]) resync) ) e:expression_list?
         -> (CALL CONST_SYMBOL[f] e);
    expression_list ::= expr (',' expr)*;

### Iterate
    iterate ::= 'ITERATE' ( v:VAR_SYMBOL / (. -> ERROR[20.2]) resync) )?
            -> (ITERATE v?)

### Leave
    leave ::= 'LEAVE' ( v:VAR_SYMBOL / (. -> ERROR[20.2]) resync) )?
          -> (LEAVE v?);

### Nop
    nop ::= 'NOP';

### Parse
    parse ::= ('PARSE' (in:parse_type / (. -> ERROR[25.12]) resync)) out:template_list?)
             -> (PARSE OPTIONS in out)

           / ('PARSE' 'op:UPPER' (in:parse_type / (. -> ERROR[25.13]) resync)) out:template_list?)
             -> (PARSE (OPTIONS op) in out);

    parse_type ::= parse_key;
    parse_key ::= 'ARG'->ARG / 'PULL'->PULL;

### Procedure
    procedure ::= LABEL 'PROCEDURE' ncl
                 ( !(TK_EOS / procedure) i:labeled_instruction )*
              -> (PROCEDURE LABEL (INSTRUCTIONS i));


### Pull
    pull ::= 'PULL' t:template_list?
         -> (PARSE (OPTIONS UPPER?) PULL t?);

### Return
    return ::= 'RETURN' e:expression?
           -> (RETURN e?);
*/

/* Say */
say(I) ::= TK_SAY(T) expression(E).
    { I = ast_f(context, SAY, T); add_ast(I,E); }

/*
### Parse Templates
    template_list ::= t:template (',' t:template)*
                  -> (TEMPLATES t+);
    template ::= (trigger / target / ((. -> ERROR[38.1]) resync)+;
    target ::= (VAR_SYMBOL / '.')
           -> TARGET;
    trigger ::= pattern / positional;
    pattern ::= STRING / vrefp
            -> PATTERN;
    vrefp ::= '('
                ( VAR_SYMBOL / ((. -> ERROR[19.7]) resync) )
                ( ')' / ((. -> ERROR[46.1]) resync) );
    positional ::= absolute_positional / relative_positional;
    absolute_positional ::= (NUMBER / '=' position)
                        -> ABS_POS;
    position ::= NUMBER / vrefp / ((. -> ERROR[38.2]) resync);
    relative_positional ::= s:('+' / '-') position
                        -> (REL_POS SIGN[s] position);
*/

// EXPRESSIONS
// precedence to disambiguate assignment vs equality
%left TK_CONST_SYMBOL TK_STRING TK_FLOAT TK_INTEGER TK_VAR_SYMBOL.
%nonassoc TK_EQUAL.

/*
    function ::= (f:taken_constant '(' p:expression_list? (')') -> (FUNCTION[f] p)
              / ((e:. -> (ERROR["36"] e)) resync);
*/


term(A)              ::= var_symbol(B). [TK_VAR_SYMBOL]
                         { A = B; }
term(A)              ::= TK_CONST_SYMBOL(S).
                         { A = ast_f(context, CONST_SYMBOL, S); }
term(A)              ::= TK_FLOAT(S).
                         { A = ast_f(context, FLOAT,S); }
term(A)              ::= TK_INTEGER(S).
                         { A = ast_f(context, INTEGER,S); }
term(A)              ::= TK_STRING(S). { A = ast_f(context, STRING,S); }
bracket(A)           ::= term(T).
                         { A = T; }
bracket(A)           ::= TK_OPEN_BRACKET expression(B) TK_CLOSE_BRACKET.
                         { A = B; }

/* These are the normal expression form in unambiguous form */
prefix_expression(P) ::= bracket(B). { P = B; }
prefix_expression(A) ::= TK_NOT(O) prefix_expression(C).
                         { A = ast_f(context, OP_PREFIX, O); add_ast(A,C); }
prefix_expression(A) ::= TK_PLUS(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_PREFIX, O); add_ast(A,C); }
prefix_expression(A) ::= TK_MINUS(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_PREFIX, O); add_ast(A,C); }
power_expression(P)  ::= prefix_expression(E).
                         { P = E; }
power_expression(A)  ::= power_expression(B) TK_POWER(O) prefix_expression(C).
                         { A = ast_f(context, OP_POWER, O); add_ast(A,B); add_ast(A,C); }
multiplication(P)    ::= power_expression(E).
                         { P = E; }
multiplication(A)    ::= multiplication(B) TK_MULT(O) power_expression(C).
                         { A = ast_f(context, OP_MULT, O); add_ast(A,B); add_ast(A,C); }
multiplication(A)    ::= multiplication(B) TK_DIV(O) power_expression(C).
                         { A = ast_f(context, OP_DIV, O); add_ast(A,B); add_ast(A,C); }
multiplication(A)    ::= multiplication(B) TK_IDIV(O) power_expression(C).
                         { A = ast_f(context, OP_IDIV, O); add_ast(A,B); add_ast(A,C); }
multiplication(A)    ::= multiplication(B) TK_MOD(O) power_expression(C).
                         { A = ast_f(context, OP_MOD, O); add_ast(A,B); add_ast(A,C); }
addition(P)          ::= multiplication(E).
                         { P = E; }
addition(A)          ::= addition(B) TK_PLUS(O) multiplication(C).
                         { A = ast_f(context, OP_ADD, O); add_ast(A,B); add_ast(A,C); }
addition(A)          ::= addition(B) TK_MINUS(O) multiplication(C).
                         { A = ast_f(context, OP_MINUS, O); add_ast(A,B); add_ast(A,C); }

/* These are for expressions "after" a concat defined by a whitespace to avoid
 * ambiguous issues with prefix operators (i.e. these miss out the +/- prefixes)
 */
prefix_expression_c(P) ::= bracket(B). { P = B; }

prefix_expression_c(A) ::= TK_NOT(O) prefix_expression_c(C).
                         { A = ast_f(context, OP_PREFIX, O); add_ast(A,C); }
power_expression_c(P)::= prefix_expression_c(E).
                         { P = E; }
power_expression_c(A)::= power_expression_c(B) TK_POWER(O) prefix_expression_c(C).
                         { A = ast_f(context, OP_POWER, O); add_ast(A,B); add_ast(A,C); }
multiplication_c(P)  ::= power_expression_c(E).
                         { P = E; }
multiplication_c(A)  ::= multiplication_c(B) TK_MULT(O) power_expression_c(C).
                         { A = ast_f(context, OP_MULT, O); add_ast(A,B); add_ast(A,C); }
multiplication_c(A)  ::= multiplication_c(B) TK_DIV(O) power_expression_c(C).
                         { A = ast_f(context, OP_DIV, O); add_ast(A,B); add_ast(A,C); }
multiplication_c(A)  ::= multiplication_c(B) TK_IDIV(O) power_expression_c(C).
                         { A = ast_f(context, OP_IDIV, O); add_ast(A,B); add_ast(A,C); }
multiplication_c(A)  ::= multiplication_c(B) TK_MOD(O) power_expression_c(C).
                         { A = ast_f(context, OP_MOD, O); add_ast(A,B); add_ast(A,C); }
addition_c(P)        ::= multiplication_c(E).
                         { P = E; }
addition_c(A)        ::= addition_c(B) TK_PLUS(O) multiplication_c(C).
                         { A = ast_f(context, OP_ADD, O); add_ast(A,B); add_ast(A,C); }
addition_c(A)        ::= addition_c(B) TK_MINUS(O) multiplication_c(C).
                         { A = ast_f(context, OP_MINUS, O); add_ast(A,B); add_ast(A,C); }

/* Back to normal expressions in the usual unambiguous form */
concatenation(P)     ::= addition(E).
                         { P = E; }
concatenation(A)     ::= concatenation(B) TK_CONCAT(O) addition(C).
                         { A = ast_f(context, OP_CONCAT, O); add_ast(A,B); add_ast(A,C); }
concatenation(A)     ::= concatenation(B) addition_c(C).  /* Note the addition_c */
                         { A = ast_ft(context, OP_SCONCAT); add_ast(A,B); add_ast(A,C); }
comparison(P)        ::= concatenation(E).
                         { P = E; }
comparison(A)        ::= comparison(B) TK_EQUAL(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_NEQ(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_GT(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_LT(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_GTE(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_LTE(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_S_EQ(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_S_NEQ(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_S_GT(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_S_LT(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_S_GTE(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_S_LTE(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE, O); add_ast(A,B); add_ast(A,C); }
or_expression(P)     ::= comparison(E).
                         { P = E; }
or_expression(A)     ::= or_expression(B) TK_OR(O) comparison(C).
                         { A = ast_f(context, OP_OR, O); add_ast(A,B); add_ast(A,C); }
and_expression(P)    ::= or_expression(E).
                         { P = E; }
and_expression(A)    ::= and_expression(B) TK_AND(O) or_expression(C).
                         { A = ast_f(context, OP_AND, O); add_ast(A,B); add_ast(A,C); }
expression(P)        ::= and_expression(E).
                         { P = E; }

/* Support Standard REXX Errors */
expression(E)        ::= TK_COMMA(S).
                         { E = ast_error(context, "37.1", S); }
expression(E)        ::= TK_CLOSE_BRACKET(S).
                         { E = ast_error(context, "37.2", S); }

/*
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

//options(I)             ::= TK_OPTIONS(T) expression(P). { I = ast_f(T); add_ast(I,P); }

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
*/