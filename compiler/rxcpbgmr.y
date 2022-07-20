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
#include <stdio.h>
#include <stdlib.h>
#include "rxcpmain.h"
}

%token TK_UNKNOWN TK_BADCOMMENT TK_EOL TK_MINUSMINUS.
%wildcard ANYTHING.

/* Low precedence */
%left ANYTHING.

/* 0 Sets the stack to grow dynamically! */
%stack_size 0

%stack_overflow
{
     /* This should never happen as the stack grows dynamically
        Have to print the error directly - and exit(1) */
     fprintf(stderr,"INTERNAL ERROR: Parser Overflow\n");
     exit(1);
}

%syntax_error {
    /*
    int i;
    int n = YYNTOKEN;
    // Example of getting the offending token
    Token *badToken = yypParser->yytos->minor.yy0;
    prnt_tok(badToken);

    // Example of getting expected tokens
    for (i = 0; i < n; ++i) {
        int a = yy_find_shift_action((YYCODETYPE)i, yypParser->yytos->stateno);
        if (a != YY_ERROR_ACTION) {
            fprintf(stderr, "possible token: %s\n", yyTokenName[i]);
        }
    }
    */
}

/* Program & Structure */
program(P)       ::= rexx_options(R) namespace_list(N) instruction_list(I) TK_EOS.
                     {
                        context->ast = ast_ft(context, REXX_UNIVERSE);
                        P = ast_ft(context, PROGRAM_FILE); add_ast(context->ast,P);
                        add_ast(P,R);
                        add_ast(R,N);
                        add_ast(P,I);
                     }

program(P)       ::= rexx_options(R) instruction_list(I) TK_EOS.
                     {
                        context->ast = ast_ft(context, REXX_UNIVERSE);
                        P = ast_ft(context, PROGRAM_FILE); add_ast(context->ast,P);
                        add_ast(P,R);
                        add_ast(P,I);
                     }


/* This case covers when the EOS was handled by an error (e.g. a missing END) in
 * sub-rules */
program(P)       ::= rexx_options(R) namespace_list(N) instruction_list(I).
                     {
                        context->ast = ast_ft(context, REXX_UNIVERSE);
                        P = ast_ft(context, PROGRAM_FILE); add_ast(context->ast,P);
                        add_ast(P,R);
                        add_ast(R,N);
                        add_ast(P,I);
                     }

program(P)       ::= rexx_options(R) instruction_list(I).
                     {
                        context->ast = ast_ft(context, REXX_UNIVERSE);
                        P = ast_ft(context, PROGRAM_FILE); add_ast(context->ast,P);
                        add_ast(P,R);
                        add_ast(P,I);
                     }

program(P)       ::= rexx_options(R) namespace_list(N) TK_EOS.
                     {
                        context->ast = ast_ft(context, REXX_UNIVERSE);
                        P = ast_ft(context, PROGRAM_FILE); add_ast(context->ast,P);
                        add_ast(P,R);
                        add_ast(R,N);
                     }

program(P)       ::= rexx_options(R) TK_EOS.
                     {
                        context->ast = ast_ft(context, REXX_UNIVERSE);
                        P = ast_ft(context, PROGRAM_FILE); add_ast(context->ast,P);
                        add_ast(P,R);
                     }

program(E)       ::= ANYTHING(T) error.
                     { E = ast_err(context, "MISSING_OPTIONS", T); context->ast = E; }

program(E)       ::=  error.
                     { E = ast_errh(context, "PARSE_FAILURE"); context->ast = E; }

/* Optional EOC */
ncl0             ::= TK_EOC.
ncl0             ::= .

/* Skip junk for syncing and for error reporting */
junk(J)          ::= . { J = 0; }
junk(J)          ::= ANYTHING(A) error.
                     { J = ast_err(context, "EXTRANEOUS", A); }
junk(J)          ::= TK_BADCOMMENT(C).
                     { J = ast_err(context, "BAD_COMMENT", C); }
junk(J)          ::= error.
                     { J = ast_errh(context, "SYNTAX_ERROR"); }

/* Variables / Labels */
var_symbol(A)    ::= TK_VAR_SYMBOL(S). { A = ast_f(context, VAR_SYMBOL, S); }
label(A)         ::= TK_LABEL(S). { A = ast_f(context, LABEL, S); }

/* Language Options */
rexx_options(I)    ::= TK_OPTIONS(T) junk(J) TK_EOC.
                   { I = ast_f(context, REXX_OPTIONS, T); add_ast(I,J); }
rexx_options(I)    ::= TK_OPTIONS(T) option_list(L) junk(J) TK_EOC.
                   { I = ast_f(context, REXX_OPTIONS, T); add_ast(I,L); add_ast(I,J); }
option_list(L)     ::= option(L1).
                   { L = L1; }
option_list(L)     ::= option_list(L1) junk(J) option(L2).
                   { L = L1; add_sbtr(L,J); add_sbtr(L,L2); }
option(C)          ::= TK_VAR_SYMBOL(S).
                   { C = ast_f(context, LITERAL, S); }

/* Namespace Instructions */
namespace_list(I)        ::= namespace_instruction(L).
//                         { I = ast_ft(context, INSTRUCTIONS); add_ast(I,L); }
                         { I = L; }
namespace_list(I)        ::= namespace_list(I1) namespace_instruction(L).
                         { I = I1; add_sbtr(I,L); }
namespace_instruction(I) ::= TK_NAMESPACE(K) TK_SYMBOL_COMPOUND(N) junk(J) TK_EOC.
                         { I = ast_f(context, NAMESPACE, K); add_ast(I, ast_f(context, LITERAL, N)); add_ast(I,J); }
namespace_instruction(I) ::= TK_NAMESPACE(K) TK_VAR_SYMBOL(N) junk(J) TK_EOC.
                         { I = ast_f(context, NAMESPACE, K); add_ast(I, ast_f(context, LITERAL, N)); add_ast(I,J); }
namespace_instruction(I) ::= TK_IMPORT(K) TK_SYMBOL_COMPOUND(N) junk(J) TK_EOC.
                         { I = ast_f(context, IMPORT, K); add_ast(I,ast_f(context, LITERAL, N)); add_ast(I,J); }
namespace_instruction(I) ::= TK_IMPORT(K) TK_VAR_SYMBOL(N) junk(J) TK_EOC.
                         { I = ast_f(context, IMPORT, K); add_ast(I,ast_f(context, LITERAL, N)); add_ast(I,J); }
namespace_instruction(I) ::= TK_NAMESPACE(E) TK_EOC.
                         { I = ast_err(context, "BAD_NAMESPACE_SYNTAX", E); }
namespace_instruction(I) ::= TK_IMPORT(E) TK_EOC.
                         { I = ast_err(context, "BAD_IMPORT_SYNTAX", E); }
namespace_instruction(I) ::= TK_NAMESPACE ANYTHING(E) error TK_EOC.
                         { I = ast_err(context, "BAD_NAMESPACE_SYNTAX", E); }
namespace_instruction(I) ::= TK_IMPORT ANYTHING(E) error TK_EOC.
                         { I = ast_err(context, "BAD_IMPORT_SYNTAX", E); }

/* Program file body */
instruction_list(I)  ::= labeled_instruction(L).
                         { I = ast_ft(context, INSTRUCTIONS); add_ast(I,L); }
instruction_list(I)  ::= instruction_list(I1) labeled_instruction(L).
                         { I = I1; add_ast(I,L); }

labeled_instruction(I) ::= group(B). { I = B; }

labeled_instruction(I) ::= single_instruction(B) junk(J) TK_EOC.
                           { I = B; add_sbtr(I,J); }
labeled_instruction(I) ::= label(B). { I = B; }
labeled_instruction(I) ::= TK_EOC. { I = 0; }
labeled_instruction(E) ::= TK_BADCOMMENT(C).
                           { E = ast_err(context, "BAD_COMMENT", C); }
labeled_instruction(E) ::= error.
                           { E = ast_errh(context, "SYNTAX_ERROR"); }

instruction(I)         ::= group(B). { I = B; }
instruction(I)         ::= single_instruction(B) junk(J) TK_EOC.
                           { I = B; add_sbtr(I,J); }
instruction(E)         ::= TK_BADCOMMENT(C).
                           { E = ast_err(context, "BAD_COMMENT", C); }
instruction(E)         ::= error.
                           { E = ast_errh(context, "SYNTAX_ERROR"); }

single_instruction(I)  ::= assignment(B). { I = B; }
single_instruction(I)  ::= command(B). { I = B; }
single_instruction(I)  ::= keyword_instruction(B). { I = B; }

assignment(I) ::=  var_symbol(V) TK_EQUAL(T) expression(E). [TK_VAR_SYMBOL]
    {
        I = ast_f(context, ASSIGN, T); add_ast(I,V); add_ast(I,E);
        V->node_type = VAR_TARGET;
    }

assignment(I) ::=  TK_FLOAT(T) TK_EQUAL expression(E).
    { I = ast_f(context, ASSIGN, T); add_ast(I,ast_err(context, "31.1", T));
      add_ast(I,E); }

assignment(I) ::=  TK_INTEGER(T) TK_EQUAL expression(E).
    { I = ast_f(context, ASSIGN, T); add_ast(I,ast_err(context, "31.1", T));
      add_ast(I,E); }

command(I)             ::= expression(E).
                       { I = ast_ft(context, ADDRESS); add_ast(I,E); }

keyword_instruction(I) ::= assembler(K). { I = K; }
//keyword_instruction(I) ::= address(K). { I = K; }
keyword_instruction(I) ::= arg(K). { I = K; }
keyword_instruction(I) ::= call(K). { I = K; }
keyword_instruction(I) ::= iterate(K). { I = K; }
keyword_instruction(I) ::= leave(K). { I = K; }
keyword_instruction(I) ::= nop(K). { I = K; }
//keyword_instruction(I) ::= parse(K). { I = K; }
keyword_instruction(I) ::= procedure(K). { I = K; }
//keyword_instruction(I) ::= pull(K). { I = K; }
keyword_instruction(I) ::= return(K). { I = K; }
keyword_instruction(I) ::= say(K). { I = K; }
/* Note the "error" tokens here (esp for TK_END) - seem to fix a conflict error - I am not
   sure if the error virtual token is only enabled when in error recovery node. If so this
   would explain it, and be a great (undocumented) feature */
keyword_instruction(I) ::= TK_THEN(T) error. { I = ast_err(context, "8.1", T); }
keyword_instruction(I) ::= TK_ELSE(T) error. { I = ast_err(context, "8.2", T); }
keyword_instruction(I) ::= TK_WHEN(T) error. { I = ast_err(context, "9.1", T); }
keyword_instruction(I) ::= TK_OTHERWISE(T) error. { I = ast_err(context, "9.2", T); }
keyword_instruction(I) ::= TK_END(T) error. { I = ast_err(context, "10.1", T); }
keyword_instruction(I) ::= TK_NAMESPACE(T) ANYTHING error.
                           { I = ast_err(context, "BAD_NAMESPACE", T); }
keyword_instruction(I) ::= TK_IMPORT(T) ANYTHING error.
                           { I = ast_err(context, "BAD_IMPORT", T); }
keyword_instruction(I) ::= TK_NAMESPACE(T) error.
                           { I = ast_err(context, "BAD_NAMESPACE", T); }
keyword_instruction(I) ::= TK_IMPORT(T) error.
                           { I = ast_err(context, "BAD_IMPORT", T); }

group(I) ::= simple_do(K) junk(J) TK_EOC. { I = K; add_sbtr(I,J); }
group(I) ::= do(K) junk(J) TK_EOC. { I = K; add_sbtr(I,J); }
group(I) ::= if(K). { I = K; }

/* Groups */

/* Simple DO Group */
simple_do(G) ::= TK_DO TK_EOC instruction_list(I) TK_END.
          { G = I; }
simple_do(G) ::= TK_DO TK_EOC TK_END.
          { G = ast_ft(context, NOP); }
simple_do(G) ::= TK_DO error.
          { G = ast_errh(context, "14.1"); }
simple_do(G) ::= TK_DO ANYTHING(E) error.
          { G = ast_err(context, "14.1", E); }
simple_do(G) ::= TK_DO(E) TK_EOS.
          { G = ast_err(context, "35.1", E); }
simple_do(G) ::= TK_DO(E) TK_EOC instruction_list(I) TK_EOS.
          { G = I; add_ast(G,ast_err(context, "14.1", E)); }
simple_do(G) ::= TK_DO TK_EOC instruction_list(I) ANYTHING(E).
          { G = I; add_ast(G,ast_err(context, "35.1", E)); }

/* DO Group */

tk_doloop(D)  ::= TK_DO(T).
                  { D = ast_f(context, DO, T); }
tk_doloop(D)  ::= TK_LOOP(T).
                  { D = ast_f(context, DO, T); }
do(G)         ::= tk_doloop(T) dorep(R) TK_EOC instruction_list(I) TK_END.
                  { G = T; add_ast(G,R); add_ast(G,I); }
do(G)         ::= tk_doloop(T) dorep(R) TK_EOC TK_END.
                  { G = T; add_ast(G,R); add_ast(G,ast_ft(context, NOP)); }
do(G)         ::= tk_doloop(T) dorep(R) docond(D) TK_EOC instruction_list(I) TK_END.
                  { G = T; add_ast(G,R); add_ast(R,D); add_ast(G,I); }
do(G)         ::= tk_doloop(T) dorep(R) docond(D) TK_EOC TK_END.
                  { G = T; add_ast(G,R); add_ast(R,D); add_ast(G,ast_ft(context, NOP)); }
do(G)         ::= tk_doloop(T) docond(D) TK_EOC instruction_list(I) TK_END.
                  { G = T; ASTNode* R = ast_ft(context, REPEAT);
                    add_ast(G,R); add_ast(R,D); add_ast(G,I); }
do(G)         ::= tk_doloop(T) docond(D) TK_EOC TK_END.
                  { G = T; ASTNode* R = ast_ft(context, REPEAT);
                    add_ast(G,R); add_ast(R,D); add_ast(G,ast_ft(context, NOP)); }
do(G)         ::= tk_doloop(T) doforever(F) TK_EOC instruction_list(I) TK_END.
                  { G = T; add_ast(G,F); add_ast(G,I); }
do(G)         ::= tk_doloop(T) doforever(F) TK_EOC TK_END.
                  { G = T; add_ast(G,F); add_ast(G,ast_ft(context, NOP)); }
do(G)         ::= tk_doloop dorep error.
                  { G = ast_errh(context, "27.1"); }
do(G)         ::= tk_doloop dorep ANYTHING(E) error.
                  { G = ast_err(context, "27.1", E); }
do(G)         ::= tk_doloop(E) dorep TK_EOC instruction_list(I) TK_EOS.
                  { G = I; mknd_err(E, "14.1"); add_ast(G,E); }
do(G)         ::= tk_doloop(E) dorep TK_EOC TK_EOS.
                  { G = ast_ft(context, NOP); mknd_err(E, "14.1"); add_ast(G,E); }
do(G)         ::= tk_doloop dorep TK_EOC instruction_list(I) ANYTHING(E).
                  { G = I; add_ast(G,ast_err(context, "35.1", E)); }
do(G)         ::= tk_doloop dorep TK_EOC ANYTHING(E).
                  { G = ast_ft(context, NOP); add_ast(G,ast_err(context, "35.1", E)); }
dorep(R)      ::= expression(E).
                  { R = ast_ft(context, REPEAT);
                  ASTNode* F = ast_ft(context, FOR); add_ast(R,F); add_ast(F,E); }
dorep(R)      ::= assignment(A).
                  { R = ast_ft(context, REPEAT); add_ast(R,A); }
dorep(R)      ::= assignment(A) dorep_list(L).
                  { R = ast_ft(context, REPEAT); add_ast(R,A); add_ast(R,L); }
dorep_list(L) ::= dorep_item(L1).
                  { L = L1; }
dorep_list(L) ::= dorep_list(L1) dorep_item(L2).
                  { L = L1; add_sbtr(L,L2); }
dorep_item(R) ::= TK_TO(T) expression(E).
                  { R = ast_f(context, TO, T); add_ast(R,E); }
dorep_item(R) ::= TK_BY(T) expression(E).
                  { R = ast_f(context, BY, T); add_ast(R,E); }
dorep_item(R) ::= TK_FOR(T) expression(E).
                  { R = ast_f(context, FOR, T); add_ast(R,E); }

doforever(R)  ::= TK_FOREVER(T).
                  { R = ast_f(context, REPEAT, T); }

docond(R) ::= TK_WHILE(T) expression(E).
                  { R = ast_f(context, WHILE, T); add_ast(R,E); }
docond(R) ::= TK_UNTIL(T) expression(E).
                  { R = ast_f(context, UNTIL, T); add_ast(R,E); }


/* IF Group */
%nonassoc TK_IF.
%nonassoc TK_ELSE.
if(I) ::= TK_IF(K) expression(E) ncl0 then(T) else(F).
          { I = ast_f(context, IF, K); add_ast(I,E); add_ast(I,T); add_ast(I,F); }
if(I) ::= TK_IF(K) expression(E) ncl0 then(T).
          { I = ast_f(context, IF, K); add_ast(I,E); add_ast(I,T); }
if(I) ::= TK_IF expression ncl0 ANYTHING(E).
          { I = ast_err(context, "18.1", E); }

then(T) ::= TK_THEN ncl0 instruction(I).
            { T = I; }
then(T) ::= TK_THEN(E) ncl0 TK_EOS.
            { T = ast_err(context, "14.3", E); }
then(T) ::= TK_THEN ncl0 TK_END(E).
            { T = ast_err(context, "10.5", E); }

else(T) ::= TK_ELSE ncl0 instruction(I).
            { T = I; }
else(T) ::= TK_ELSE(E) ncl0 TK_EOS.
            { T = ast_err(context, "14.4", E); }
else(T) ::= TK_ELSE ncl0 TK_END(E).
            { T = ast_err(context, "10.6", E); }

/* Procedure / Args */
procedure(P)      ::= TK_LABEL(L) TK_PROCEDURE TK_EQUAL class(C).
                      { P = ast_f(context, PROCEDURE, L); add_ast(P,C); }
procedure(P)      ::= TK_LABEL(L) TK_PROCEDURE TK_EQUAL TK_VOID(V).
                      { P = ast_f(context, PROCEDURE, L); add_ast(P,ast_f(context, VOID, V)); }
procedure(P)      ::= TK_LABEL(L) TK_PROCEDURE.
                      { P = ast_f(context, PROCEDURE, L); add_ast(P,ast_ft(context, VOID)); }
arg(P)            ::= TK_ARG arg_list(A).
                      { P = A;}
/* Classes */
class(C)          ::= TK_CLASS(T).
                      { C = ast_f(context, CLASS, T); }

/* Argument Templates */
arg_list(L)       ::= . { L = ast_ft(context, ARGS); }
arg_list(L)       ::= argument(T). { L = ast_ft(context, ARGS); add_ast(L,T); }
arg_list(L)       ::= arg_list(L1) TK_COMMA argument(T). { L = L1; add_ast(L,T); }
argument(T)       ::= TK_EXPOSE var_symbol(V) TK_EQUAL class(C).
                      { V->node_type = VAR_REFERENCE; T = ast_ft(context, ARG); add_ast(T,V); add_ast(T,C); }
argument(T)       ::= var_symbol(V) TK_EQUAL class(C).
                      { V->node_type = VAR_TARGET; T = ast_ft(context, ARG); add_ast(T,V); add_ast(T,C); }
argument(T)       ::= TK_EXPOSE var_symbol(V) TK_EQUAL expression(E).
                      { V->node_type = VAR_REFERENCE; T = ast_ft(context, ARG); add_ast(T,V); add_ast(T,E); }
argument(T)       ::= var_symbol(V) TK_EQUAL expression(E).
                      { V->node_type = VAR_TARGET; T = ast_ft(context, ARG); add_ast(T,V); add_ast(T,E); }

/* Instructions */

/*
### ADDRESS
    address ::= 'ADDRESS' e:taken_constant c:expression? -> (ADDRESS ENVIRONMENT[e] c);
*/

/* Assembler */
assembler(I) ::= TK_ASSEMBLER assembler_instruction(A).
             { I = A; }
assembler(I) ::= TK_ASSEMBLER TK_EOC assembler_instruction(A).
             { I = A; }
assembler(G) ::= TK_ASSEMBLER TK_DO TK_EOC assembler_list(I) TK_END.
             { G = I; }
assembler(G) ::= TK_ASSEMBLER TK_DO TK_EOC TK_END.
             { G = 0; }
assembler(G) ::= TK_ASSEMBLER TK_DO ANYTHING(E).
             { G = ast_err(context, "BAD_ASSEMBLER", E); }
assembler(G) ::= TK_ASSEMBLER TK_DO(E) TK_EOS.
             { G = ast_err(context, "ASSEMBLER_NO_END", E); }
assembler(G) ::= TK_ASSEMBLER TK_DO(E) TK_EOC assembler_list(I) TK_EOS.
             { G = I; add_ast(G,ast_err(context, "ASSEMBLER_NO_END", E)); }
assembler(G) ::= TK_ASSEMBLER TK_DO TK_EOC assembler_list(I) ANYTHING(E).
             { G = I; add_ast(G,ast_err(context, "BAD_ASSEMBLER", E)); }
assembler(G) ::= TK_ASSEMBLER TK_EOC TK_DO TK_EOC assembler_list(I) TK_END.
             { G = I; }
assembler(G) ::= TK_ASSEMBLER TK_EOC TK_DO TK_EOC TK_END.
             { G = 0; }
assembler(G) ::= TK_ASSEMBLER TK_EOC TK_DO ANYTHING(E).
             { G = ast_err(context, "BAD_ASSEMBLER", E); }
assembler(G) ::= TK_ASSEMBLER TK_EOC TK_DO(E) TK_EOS.
             { G = ast_err(context, "ASSEMBLER_NO_END", E); }
assembler(G) ::= TK_ASSEMBLER TK_EOC TK_DO(E) TK_EOC assembler_list(I) TK_EOS.
             { G = I; add_ast(G,ast_err(context, "ASSEMBLER_NO_END", E)); }
assembler(G) ::= TK_ASSEMBLER TK_EOC TK_DO TK_EOC assembler_list(I) ANYTHING(E).
             { G = I; add_ast(G,ast_err(context, "BAD_ASSEMBLER", E)); }
assembler_list(I)  ::= assembler_instruction(L) TK_EOC.
                       { I = L; }
assembler_list(I)  ::= assembler_list(I1) assembler_instruction(L) TK_EOC.
                       { I = I1; add_sbtr(I,L); }
assembler_instruction(I) ::= assembler_op(OP).
    { I = OP; }
assembler_instruction(I) ::= assembler_op(OP) assembler_arg(A1).
    { I = OP; add_ast(I,A1); }
assembler_instruction(I) ::= assembler_op(OP) assembler_arg(A1) TK_COMMA assembler_arg(A2).
    { I = OP; add_ast(I,A1); add_ast(I,A2); }
assembler_instruction(I) ::= assembler_op(OP) assembler_arg(A1) TK_COMMA assembler_arg(A2) TK_COMMA assembler_arg(A3).
    { I = OP; add_ast(I,A1); add_ast(I,A2); add_ast(I,A3);}
assembler_op(OP)         ::= TK_VAR_SYMBOL(S).
                         { OP = ast_f(context, ASSEMBLER, S); }
assembler_op(OP)         ::= TK_SAY(S). /* SAY is also a REXX keyword */
                         { OP = ast_f(context, ASSEMBLER, S); }
assembler_arg(A)         ::= var_symbol(B).
                         { A = B; }
assembler_arg(A)         ::= TK_FLOAT(S).
                         { A = ast_f(context, FLOAT,S); }
assembler_arg(A)         ::= TK_INTEGER(S).
                         { A = ast_f(context, INTEGER,S); }
assembler_arg(A)         ::= TK_STRING(S).
                         { A = ast_fstr(context,S); }

/*
### Arg
    arg ::= 'ARG' t:template_list?
        -> (PARSE (OPTIONS UPPER?) ARG t?)

### Call
    call ::= 'CALL' (f:taken_constant / ( (. -> ERROR[19.2]) resync) ) e:expression_list?
         -> (CALL CONST_SYMBOL[f] e);
    expression_list ::= expr (',' expr)*;
*/

/* Iterate */
iterate(I) ::= TK_ITERATE(T) var_symbol(S).
    { I = ast_f(context, ITERATE, T); add_ast(I,S); }

iterate(I) ::= TK_ITERATE(T).
    { I = ast_f(context, ITERATE, T); }

/* Leave */
leave(I) ::= TK_LEAVE(T) var_symbol(S).
    { I = ast_f(context, LEAVE, T); add_ast(I,S); }

leave(I) ::= TK_LEAVE(T).
    { I = ast_f(context, LEAVE, T); }

/*
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
*/

/* Return */
return(I) ::= TK_RETURN(T) expression(E).
    { I = ast_f(context, RETURN, T); add_ast(I,E); }

return(I) ::= TK_RETURN(T).
    { I = ast_f(context, RETURN, T); }

/* Say */
say(I) ::= TK_SAY(T) expression(E).
    { I = ast_f(context, SAY, T); add_ast(I,E); }
say(I) ::= TK_SAY(T).
    { I = ast_f(context, SAY, T); add_ast(I,ast_ft(context, STRING)); }

/* Nop */
nop(I) ::= TK_NOP(T).
    { I = ast_f(context, NOP, T); }

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
%left TK_STRING TK_FLOAT TK_INTEGER TK_VAR_SYMBOL TK_SYMBOL_COMPOUND.
%left TK_OPEN_BRACKET.
%nonassoc TK_EQUAL.

function_name(N)       ::= TK_VAR_SYMBOL(S).
                           { N = ast_f(context, FUNCTION, S); }
function_name(N)       ::= TK_SYMBOL_COMPOUND(S).
                           { N = ast_f(context, FUNCTION, S); }
function_name(N)       ::= TK_STRING(S).
                           { N = ast_f(context, FUNCTION, S); }
call(I) ::= TK_CALL(T) function_name(F) expression_list(E).
        { I = ast_f(context, CALL, T); add_ast(I,F); if (E) add_ast(F,E); }
call(I) ::= TK_CALL(T) ANYTHING(E).
        { I = ast_f(context, CALL, T); add_ast(I,ast_err(context, "19.2", E)); }

/* Expression Lists */
expression_list(L)     ::= .
                         { L = 0; }
expression_list(L)     ::= expression_in_list(E).
                         { L = E; }
expression_list(L)     ::= expression_list(L1) TK_COMMA expression_in_list(E).
                         { if (L1) L = L1; else L = ast_ft(context, NOVAL); add_sbtr(L,E);}
expression_list(L)     ::= expression_list(L1) TK_COMMA.
                         { if (L1) L = L1; else L = ast_ft(context, NOVAL); add_sbtr(L, ast_ft(context, NOVAL)); }
term(F)                ::= TK_VAR_SYMBOL(S) function_parameters(P).
                           { F = ast_f(context, FUNCTION, S); if (P) add_ast(F,P); }
term(F)                ::= TK_SYMBOL_COMPOUND(S) function_parameters(P).
                           { F = ast_f(context, FUNCTION, S); if (P) add_ast(F,P); }
term(F)                ::= TK_STRING(S) function_parameters(P).
                           { F = ast_f(context, FUNCTION, S); if (P) add_ast(F,P); }
function_parameters(P) ::= TK_OPEN_BRACKET expression_list(E) TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           { P = E; }
term(A)                ::= var_symbol(B). [TK_VAR_SYMBOL]
                         { A = B; }
term(A)                ::= TK_FLOAT(S).
                         { A = ast_f(context, FLOAT,S); }
term(A)                ::= TK_INTEGER(S).
                         { A = ast_f(context, INTEGER,S); }
term(A)                ::= TK_STRING(S). { A = ast_fstr(context,S); }

bracket(A)           ::= term(T).
                         { A = T; }
bracket(A)           ::= TK_OPEN_BRACKET expression(B) TK_CLOSE_BRACKET.
                         { A = B; }

/* These are the normal expression form in unambiguous form */
prefix_expression(P) ::= bracket(B). { P = B; }
prefix_expression(A) ::= TK_NOT(O) prefix_expression(C).
                         { A = ast_f(context, OP_NOT, O); add_ast(A,C); }
prefix_expression(A) ::= TK_PLUS(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_PLUS, O); add_ast(A,C); }
prefix_expression(A) ::= TK_MINUS(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_NEG, O); add_ast(A,C); }
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
                         { A = ast_f(context, OP_NOT, O); add_ast(A,C); }
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
                         { A = ast_f(context, OP_COMPARE_EQUAL, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_NEQ(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_NEQ, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_GT(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_GT, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_LT(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_LT, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_GTE(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_GTE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_LTE(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_LTE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_S_EQ(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_S_EQ, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_S_NEQ(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_S_NEQ, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_S_GT(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_S_GT, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_S_LT(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_S_LT, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_S_GTE(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_S_GTE, O); add_ast(A,B); add_ast(A,C); }
comparison(A)        ::= comparison(B) TK_S_LTE(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_S_LTE, O); add_ast(A,B); add_ast(A,C); }
or_expression(P)     ::= comparison(E).
                         { P = E; }
or_expression(A)     ::= or_expression(B) TK_OR(O) comparison(C).
                         { A = ast_f(context, OP_OR, O); add_ast(A,B); add_ast(A,C); }
and_expression(P)    ::= or_expression(E).
                         { P = E; }
and_expression(A)    ::= and_expression(B) TK_AND(O) or_expression(C).
                         { A = ast_f(context, OP_AND, O); add_ast(A,B); add_ast(A,C); }

/* Errors */
and_expression(E)  ::= TK_UNKNOWN(U) error. { E = ast_err(context, "BADCHAR", U); }
and_expression(E)  ::= TK_CONCAT(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_MULT(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_DIV(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_IDIV(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_MOD(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_POWER(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_EQUAL(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_NEQ(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_GT(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_LT(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_GTE(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_LTE(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_S_EQ(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_S_NEQ(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_S_GT(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_S_LT(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_S_GTE(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_S_LTE(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_AND(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_OR(U) error. { E = ast_err(context, "BADEXPR", U); }

expression(P)  ::= and_expression(E). { P = E; }
expression(E)  ::= TK_COMMA(U) error. { E = ast_err(context, "BADEXPR", U); }
expression(E)  ::= TK_CLOSE_BRACKET(U) error. { E = ast_err(context, "BADEXPR", U); }

/* expressions in a list cannot expresssion() errors above because of parsing conflicta */
expression_in_list(P) ::= and_expression(E). { P = E; }

/* Finally set the node with the highest precedence */
%left TK_EOC.

/* ******************************* END *************************************
   ******************************* END *************************************
   ******************************* END *************************************
   ******************************* END *************************************
   ******************************* END *************************************

valueexp(P)            ::= TK_VALUE(T) expression(E). { P = ast_f(T); add_ast(P,E); }

expression0(P)         ::= . { P = ast_ft(TK_NULL); }
expression0(P)         ::= expression(E). { P = E; }

expression_list(L)     ::= . { L = ast_ft(TK_LIST); }
expression_list(L)     ::= expression_list1(E). { L = E; }
expression_list1(L)    ::= expression(E). { L = ast_ft(TK_LIST); add_ast(L,E); }
expression_list1(L)    ::= expression_list1(L1) TK_COMMA expression(E). { L = L1; add_ast(L,E); }

// VARIABLE & SYMBOLS
var_symbol(T)          ::= TK_VAR_SYMBOL(S). { T = ast_f(S); }
var_symbol(T)          ::= TK_SYMBOL_STEM(S). { T = ast_f(S);
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
taken_constant(T)      ::= TK_VAR_SYMBOL(S). { T = ast_f(S); }
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