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
 * Compiler Parser Grammar (Lemon)
 */

%name RexxB 
%token_type { Token* }
%default_type { ASTNode* }
%extra_argument { Context *context }
%start_symbol program

%include {
/* cREXX Compiler                  */
/* (c) Adrian Sutherland 2021-2025 */
/* Grammar                         */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "rxcpmain.h"
}

%token TK_UNKNOWN TK_BADCOMMENT TK_EOL TK_MINUSMINUS TK_DOT TK_EXIT_PRIMARY TK_EXIT_TOKEN.
%wildcard ANYTHING.

/* Low precedence */
%left EXIT_REDUCE.
%right ANYTHING.
%left TK_EOC.
%left IMPLICIT_CONCAT.
%left TK_DOT TK_CLASS_TYPE.

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
junk(J)          ::= TK_BADCOMMENT(C) error.
                     { J = ast_err(context, "BAD_COMMENT", C); }
junk(J)          ::= error.
                     { J = ast_errh(context, "SYNTAX_ERROR"); }

/* Classes / Variables */
class(C)                 ::= TK_CLASS_TYPE(T).
                             { C = ast_f(context, CLASS, T); }
type_def(A)              ::= class(S).
                             { A = S; }
type_def(A)              ::= class(S) array_def_parameters(P).
                             { A = S; if (P) add_ast(A,P); }
type_def(A)              ::= TK_CLASS_STEM(S) stem_def_parts(P).
                             { A = ast_f(context, CLASS, S); if (P) add_ast(A,P); }
array_def_parameters(P)  ::= TK_OPEN_SBRACKET def_expression_list(E) TK_CLOSE_SBRACKET. [TK_VAR_SYMBOL]
                             { P = E; }
stem_def_parts(L)        ::= stem_def_part(S).
                             { L = S; }
stem_def_parts(L)        ::= stem_def_parts(L1) stem_def_part(E).
                             { if (L1) L = L1;
                               else {
                                 L = ast_ft(context, RANGE);
                                 add_ast(L, ast_ft(context, NOVAL));
                                 add_ast(L, ast_ft(context, NOVAL));
                               }
                               add_sbtr(L,E);
                             }
stem_def_part(A)         ::= TK_STEMINT(S).
                             {  /* Remove the leading "." */
                                S->column++; S->length--; S->token_string++;
                                A = ast_ft(context, RANGE);
                                add_ast(A, ast_ft(context, NOVAL));
                                add_ast(A, ast_f(context, INTEGER, S));
                             }
stem_def_part(A)         ::= TK_STEMNOVAL(S).
                             {  /* Remove the leading "." */
                                S->column++; S->length--; S->token_string++;
                                A = ast_ft(context, RANGE);
                                add_ast(A, ast_ft(context, NOVAL));
                                add_ast(A, ast_ft(context, NOVAL));
                             }
stem_def_part(A)         ::= TK_STEMVAR(S).
                             { A = mknd_err(ast_f(context, VAR_SYMBOL,S), "INVALID_IN_ARRAY_DEF");}
stem_def_part(A)         ::= TK_STEMSTRING(S).
                             { A = mknd_err(ast_f(context, VAR_SYMBOL,S), "INVALID_IN_ARRAY_DEF"); }
def_expression_list(L)   ::= .
                             { L = ast_ft(context, RANGE);
                               add_ast(L, ast_ft(context, NOVAL));
                               add_ast(L, ast_ft(context, NOVAL));
                             }
def_expression_list(L)   ::= def_expression(E).
                             { L = E; }
def_expression_list(L)   ::= def_expression_list(L1) TK_COMMA def_expression(E).
                             { ASTNode* _temp;
                               if (L1) L = L1;
                               else {
                                 L = ast_ft(context, RANGE);
                                 add_ast(L, ast_ft(context, NOVAL));
                                 add_ast(L, ast_ft(context, NOVAL));
                               }
                               add_sbtr(L, E);
                             }
def_expression_list(L)   ::= def_expression_list(L1) TK_COMMA.
                             { ASTNode* _temp;
                               if (L1) L = L1;
                               else {
                                 L = ast_ft(context, RANGE);
                                 add_ast(L, ast_ft(context, NOVAL));
                                 add_ast(L, ast_ft(context, NOVAL));
                               }
                               _temp = ast_ft(context, RANGE);
                               add_ast(_temp, ast_ft(context, NOVAL));
                               add_ast(_temp, ast_ft(context, NOVAL));
                               add_sbtr(L, _temp);
                             }
def_expression(D)        ::=   def_value(S).
                             { D = ast_ft(context, RANGE);
                               add_ast(D, ast_ft(context, NOVAL));
                               add_ast(D, S);
                             }
def_expression(D)        ::=   def_value(S1) TK_TO def_value(S2).
                             { D = ast_ft(context, RANGE);
                               add_ast(D, S1);
                               add_ast(D, S2);
                             }
def_value(D)             ::=   TK_INTEGER(S).
                             { D = ast_f(context, INTEGER,S); }
def_value(D)             ::=   TK_MULT(S).
                             { D = ast_f(context, NOVAL,S); }
def_value(D)             ::=   TK_MINUS(O) TK_INTEGER(S).
                             { D = ast_f(context, OP_NEG, O); add_ast(D, ast_f(context, INTEGER,S)); }
def_value(D)             ::=   TK_HIGH_PRIORITY_MINUS(O) TK_INTEGER(S).
                             { D = ast_f(context, OP_NEG, O); add_ast(D, ast_f(context, INTEGER,S)); }
/* Common errors if a user tried to use an expression in an array definition */
def_value(D)             ::=   TK_VAR_SYMBOL(S) error. { D = mknd_err(ast_f(context, VAR_SYMBOL,S), "INVALID_IN_ARRAY_DEF"); }
def_value(D)             ::=   TK_FLOAT(S) error. { D = mknd_err(ast_f(context, FLOAT,S), "INVALID_IN_ARRAY_DEF"); }
def_value(D)             ::=   TK_DECIMAL(S) error. { D = mknd_err(ast_f(context, DECIMAL,S), "INVALID_IN_ARRAY_DEF"); }
def_value(D)             ::=   TK_STRING(S) error. { D = mknd_err(ast_f(context, STRING,S), "INVALID_IN_ARRAY_DEF"); }
def_value(D)             ::=   TK_PLUS(S) error. { D = mknd_err(ast_f(context, OP_ADD,S), "INVALID_IN_ARRAY_DEF");}
def_value(D)             ::=   TK_MINUS(S) error.
                               { D = mknd_err(ast_f(context, OP_MINUS,S), "INVALID_IN_ARRAY_DEF"); }
def_value(D)             ::=   TK_HIGH_PRIORITY_MINUS(S) error.
                               { D = mknd_err(ast_f(context, OP_MINUS,S), "INVALID_IN_ARRAY_DEF");}
def_value(D)             ::=   TK_INTEGER ANYTHING(S) error. { D = ast_err(context, "INVALID_IN_ARRAY_DEF", S); }

var_symbol(A)          ::= TK_VAR_SYMBOL(S). { A = ast_f(context, VAR_SYMBOL, S); }
var_symbol(A)          ::= TK_VAR_SYMBOL(S) array_parameters(P).
                           { A = ast_f(context, VAR_SYMBOL, S); if (P) add_ast(A,P); }
var_symbol(A)          ::= TK_STEM(S) stemparts(P). [TK_VAR_SYMBOL]
                           { A = ast_f(context, VAR_SYMBOL, S); if (P) add_ast(A,P); }
array_parameters(P)    ::= TK_OPEN_SBRACKET expression_list(E) TK_CLOSE_SBRACKET. [TK_VAR_SYMBOL]
                           { P = E; }
stemparts(L)           ::= stempart(S).
                           { L = S; }
stemparts(L)           ::= stemparts(L1) stempart(E).
                           { if (L1) L = L1; else L = ast_ft(context, NOVAL); add_sbtr(L,E);}
stempart(A)            ::= TK_STEMVAR(S).
                           {  /* Remove the leading "." */
                              S->column++; S->length--; S->token_string++;
                              A = ast_f(context, VAR_SYMBOL, S);
                           }
stempart(A)            ::= TK_STEMINT(S).
                           {  /* Remove the leading "." */
                              S->column++; S->length--; S->token_string++;
                              A = ast_f(context, INTEGER, S);
                           }
stempart(A)            ::= TK_STEMSTRING(S).
                           {  /* Remove the leading "." */
                              S->column++; S->length--; S->token_string++;
                              A = ast_f(context, STRING, S);
                           }
stempart(A)            ::= TK_STEMNOVAL(S).
                           {  /* Remove the leading "." */
                              S->column++; S->length--; S->token_string++;
                              A = ast_f(context, NOVAL, S);
                           }

/* Labels */
label(A)               ::= TK_LABEL(S).
                           { A = ast_f(context, LABEL, S); }

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
literal(L)               ::= TK_VAR_SYMBOL(N).
                         { L = ast_f(context, LITERAL, N); }
namespace_list(I)        ::= namespace_instruction(L).
                         { I = L; }
namespace_list(I)        ::= namespace_list(I1) namespace_instruction(L).
                         { I = I1; add_sbtr(I,L); }
namespace_instruction(I) ::= TK_NAMESPACE(K) literal(N) junk(J) TK_EOC.
                         { I = ast_f(context, NAMESPACE, K); add_ast(I, N); add_ast(I,J); }
namespace_instruction(I) ::= TK_NAMESPACE(K) literal(N) expose(E) junk(J) TK_EOC.
                         { I = ast_f(context, NAMESPACE, K); add_ast(I,N); add_ast(I,E); add_ast(I,J); }
namespace_instruction(I) ::= TK_IMPORT(K) literal(N) junk(J) TK_EOC.
                         { I = ast_f(context, IMPORT, K); add_ast(I,N); add_ast(I,J); }
namespace_instruction(I) ::= TK_NAMESPACE(E) TK_EOC.
                         { I = mknd_err(ast_f(context, NAMESPACE,E), "BAD_NAMESPACE_SYNTAX"); }
namespace_instruction(I) ::= TK_IMPORT(E) TK_EOC.
                         { I = mknd_err(ast_f(context, NAMESPACE,E), "BAD_IMPORT_SYNTAX"); }
namespace_instruction(I) ::= TK_NAMESPACE ANYTHING(E) error TK_EOC.
                         { I = ast_err(context, "BAD_NAMESPACE_SYNTAX", E); }
namespace_instruction(I) ::= TK_IMPORT ANYTHING(E) error TK_EOC.
                         { I = ast_err(context, "BAD_IMPORT_SYNTAX", E); }
expose(I)                ::= TK_EXPOSE(K) expose_list(L).
                         { I = ast_f(context, EXPOSED, K); add_ast(I,L); }
expose_list(I)           ::= literal(L).
                         { I = L; }
expose_list(I)           ::= expose_list(I1) literal(L).
                         { I = I1; add_sbtr(I,L); }

/* Program file body */
instruction_list(I)  ::= labeled_instruction(L).
                         { I = ast_ft(context, INSTRUCTIONS); add_ast(I,L); }
instruction_list(I)  ::= instruction_list(I1) labeled_instruction(L).
                         { I = I1; add_ast(I,L); }

labeled_instruction(I) ::= group(B). { I = B; }

labeled_instruction(I) ::= single_instruction(B) junk(J) TK_EOC.
                           { I = B; add_sbtr(I,J); }
labeled_instruction(I) ::= label(B). { I = B; }
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
single_instruction(I)  ::= define(B). { I = B; }
single_instruction(I)  ::= exit_extended(B). { I = B; }
single_instruction(I)  ::= command(B). { I = B; }
single_instruction(I)  ::= keyword_instruction(B). { I = B; }

exit_extended(I) ::= TK_EXIT_PRIMARY(P) exit_tokens(L). [EXIT_REDUCE]
{
    I = ast_f(context, EXIT_EXTENDED, P);
    add_ast(I, L);
}

exit_tokens(L) ::= exit_tokens(L1) exit_token(T). [ANYTHING]
{
    if (L1) {
        L = L1;
        add_sbtr(L, T);
    } else {
        L = T;
    }
}
exit_tokens(L) ::= . { L = 0; }

exit_token(T) ::= TK_EXIT_TOKEN(K). { T = ast_f(context, EXIT_TOKEN, K); }
exit_token(T) ::= ANYTHING(A).       { T = ast_f(context, EXIT_TOKEN, A); }

/* Assignments trying to assign to a keywords */
assignment(G)     ::= TK_DO(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_LOOP(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_CLASS_TYPE(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_TO(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_EXPOSE(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_THEN(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_ELSE(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_WHEN(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_OTHERWISE(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_SELECT(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_END(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_BY(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_FOR(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_FOREVER(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_WHILE(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_UNTIL(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_IF(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_ARG(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_ADDRESS(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_OUTPUT(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_ERROR(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_INPUT(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_ASSEMBLER(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_SAY(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_ITERATE(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_LEAVE(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_RETURN(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_NOP(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_CALL(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_EXIT_PRIMARY(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_EXIT_TOKEN(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }

/* Defines trying to assign to a keywords */
define(G)     ::= TK_DO(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_LOOP(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_CLASS_TYPE(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_TO(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_EXPOSE(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_THEN(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_ELSE(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_WHEN(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_OTHERWISE(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_SELECT(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_END(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_BY(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_FOR(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_FOREVER(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_WHILE(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_UNTIL(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_IF(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_ARG(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_ADDRESS(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_OUTPUT(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_ERROR(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_INPUT(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_ASSEMBLER(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_SAY(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_ITERATE(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_LEAVE(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_RETURN(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_NOP(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_CALL(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }

/* Assignments trying to assign from a keywords */
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_DO(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_LOOP(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_CLASS_TYPE(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_TO(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_EXPOSE(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_THEN(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_ELSE(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_WHEN(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_OTHERWISE(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_SELECT(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_END(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_BY(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_FOR(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_FOREVER(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_WHILE(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_UNTIL(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_IF(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_ADDRESS(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_OUTPUT(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_ERROR(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_INPUT(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_ASSEMBLER(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_SAY(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_ITERATE(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_LEAVE(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_RETURN(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_NOP(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_CALL(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"));}
assignment(G)     ::= var_symbol(V) TK_EQUAL(T) TK_NUMERIC(K) error.
    { G = ast_f(context, ASSIGN, T); add_ast(G,V); V->node_type = VAR_TARGET; add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); }

/* Assignments / Defines with invalid LHS */
assignment(G) ::=  TK_FLOAT(K) TK_EQUAL(T) expression(E).
    { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, FLOAT,K), "INVALID_LHS")); add_ast(G,E);  }
assignment(G) ::=  TK_DECIMAL(K) TK_EQUAL(T) expression(E).
    { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, DECIMAL,K), "INVALID_LHS")); add_ast(G,E);  }
assignment(G) ::=  TK_INTEGER(K) TK_EQUAL(T) expression(E).
    { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, INTEGER,K), "INVALID_LHS")); add_ast(G,E);  }
define(G) ::=  TK_FLOAT(K) TK_EQUAL(T) type_def(E).
    { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, FLOAT,K), "INVALID_LHS")); add_ast(G,E);  }
define(G) ::=  TK_DECIMAL(K)TK_EQUAL(T) type_def(E).
    { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, DECIMAL,K), "INVALID_LHS")); add_ast(G,E);  }
define(G) ::=  TK_INTEGER(K) TK_EQUAL(T) type_def(E).
    { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, INTEGER,K), "INVALID_LHS")); add_ast(G,E);  }

/* Correct Define and Assignment */
define(I) ::=  var_symbol(V) TK_EQUAL(T) type_def(E) opt_with(W).
    {
        I = ast_f(context, DEFINE, T); add_ast(I,V); add_ast(I,E);
        if (W) add_ast(I,W);
        V->node_type = VAR_TARGET;
    }

assignment(I) ::=  var_symbol(V) TK_EQUAL(T) expression(E). [TK_VAR_SYMBOL]
    {
        I = ast_f(context, ASSIGN, T); add_ast(I,V); add_ast(I,E);
        V->node_type = VAR_TARGET;
    }

command(I)             ::= expression(E).
                       { I = ast_ft(context, IMPLICIT_CMD); add_ast(I,E); }

keyword_instruction(I) ::= assembler(K). { I = K; }
keyword_instruction(I) ::= address(K). { I = K; }
keyword_instruction(I) ::= arg(K). { I = K; }
keyword_instruction(I) ::= call(K). { I = K; }
keyword_instruction(I) ::= iterate(K). { I = K; }
keyword_instruction(I) ::= leave(K). { I = K; }
keyword_instruction(I) ::= nop(K). { I = K; }
//keyword_instruction(I) ::= parse(K). { I = K; }
keyword_instruction(I) ::= procedure(K). { I = K; }
//keyword_instruction(I) ::= pull(K). { I = K; }
keyword_instruction(I) ::= return(K). { I = K; }
keyword_instruction(I) ::= exit(K). { I = K; }
keyword_instruction(I) ::= say(K). { I = K; }
keyword_instruction(I) ::= numeric(K). { I = K; }
keyword_instruction(I) ::= factory_def(K). { I = K; }
keyword_instruction(I) ::= method_def(K). { I = K; }
keyword_instruction(I) ::= class_def(K). { I = K; }

/* Note the "error" tokens here (esp for TK_END) - seem to fix a conflict error - I am not
   sure if the error virtual token is only enabled when in error recovery node. If so this
   would explain it, and be a great (undocumented) feature */
keyword_instruction(I) ::= TK_THEN(T) error. { I = ast_err(context, "UNEXPECTED_THEN", T); }
keyword_instruction(I) ::= TK_ELSE(T) error. { I = ast_err(context, "UNEXPECTED_ELSE", T); }
keyword_instruction(I) ::= TK_WHEN(T) error. { I = ast_err(context, "UNEXPECTED_WHEN", T); }
keyword_instruction(I) ::= TK_OTHERWISE(T) error. { I = ast_err(context, "UNEXPECTED_OTHERWISE", T); }
keyword_instruction(I) ::= TK_END(T) error. { I = ast_err(context, "UNEXPECTED_END", T); }
keyword_instruction(I) ::= TK_NAMESPACE(T) ANYTHING error.
                           { I = ast_err(context, "BAD_NAMESPACE", T); }
keyword_instruction(I) ::= TK_IMPORT(T) ANYTHING error.
                           { I = ast_err(context, "BAD_IMPORT", T); }
keyword_instruction(I) ::= TK_NAMESPACE(T) error.
                           { I = ast_err(context, "BAD_NAMESPACE", T); }
keyword_instruction(I) ::= TK_IMPORT(T) error.
                           { I = ast_err(context, "BAD_IMPORT", T); }

group(I) ::= simple_do(K) TK_EOC. { I = K; }
group(I) ::= simple_do(K). { I = K; }
group(I) ::= do(K) TK_EOC. { I = K; }
group(I) ::= do(K). { I = K; }
group(I) ::= if(K). { I = K; }

/* Groups */

/* Simple DO Group */
simple_do(G) ::= TK_DO TK_EOC instruction_list(I) TK_END.
          { G = I; }
simple_do(G) ::= TK_DO TK_EOC TK_END.
          { G = ast_ft(context, NOP); }
simple_do(G) ::= TK_DO error.
          { G = ast_errh(context, "INCOMPLETE_DO"); }
simple_do(G) ::= TK_DO ANYTHING(E) error.
          { G = ast_err(context, "INCOMPLETE_DO", E); }
simple_do(G) ::= TK_DO(E) TK_EOS.
          { G = ast_err(context, "INCOMPLETE_DO", E); }
simple_do(G) ::= TK_DO(E) TK_EOC instruction_list(I) TK_EOS.
          { G = I; add_ast(G,ast_err(context, "INCOMPLETE_DO", E)); }
simple_do(G) ::= TK_DO TK_EOC instruction_list(I) ANYTHING(E).
          { G = I; add_ast(G,ast_err(context, "INCOMPLETE_DO", E)); }

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
                  { G = ast_errh(context, "INVALID_DO"); }
do(G)         ::= tk_doloop dorep ANYTHING(E) error.
                  { G = ast_err(context, "INVALID_DO", E); }
do(G)         ::= tk_doloop(E) dorep TK_EOC instruction_list(I) TK_EOS.
                  { G = I; mknd_err(E, "MISSING_END"); add_ast(G,E); }
do(G)         ::= tk_doloop(E) dorep TK_EOC TK_EOS.
                  { G = ast_ft(context, NOP); mknd_err(E, "INCOMPLETE_DO"); add_ast(G,E); }
do(G)         ::= tk_doloop dorep TK_EOC instruction_list(I) ANYTHING(E).
                  { G = I; add_ast(G,ast_err(context, "INVALID_EXPRESSION", E)); }
do(G)         ::= tk_doloop dorep TK_EOC ANYTHING(E).
                  { G = ast_ft(context, NOP); add_ast(G,ast_err(context, "INVALID_EXPRESSION", E)); }
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
          { I = ast_err(context, "MISSING_THEN", E); }

then(T) ::= TK_THEN ncl0 instruction(I).
            { T = I; }
then(T) ::= TK_THEN(E) ncl0 TK_EOS.
            { T = ast_err(context, "MISSING_END", E); }
then(T) ::= TK_THEN ncl0 TK_END(E).
            { T = ast_err(context, "UNEXPECTED_END", E); }

else(T) ::= TK_ELSE ncl0 instruction(I).
            { T = I; }
else(T) ::= TK_ELSE(E) ncl0 TK_EOS.
            { T = ast_err(context, "MISSING_END", E); }
else(T) ::= TK_ELSE ncl0 TK_END(E).
            { T = ast_err(context, "UNEXPECTED_END", E); }

/* Procedure / Args */
procedure(P)      ::= TK_LABEL(L) TK_PROCEDURE TK_EQUAL type_def(C).
                      { P = ast_f(context, PROCEDURE, L); add_ast(P,C); }
procedure(P)      ::= TK_LABEL(L) TK_PROCEDURE TK_EQUAL TK_VOID(V).
                      { P = ast_f(context, PROCEDURE, L); add_ast(P,ast_f(context, VOID, V)); }
procedure(P)      ::= TK_LABEL(L) TK_PROCEDURE.
                      { P = ast_f(context, PROCEDURE, L); add_ast(P,ast_ft(context, VOID)); }
procedure(P)      ::= TK_LABEL(L) TK_PROCEDURE TK_EQUAL type_def(C) expose(E).
                      { P = ast_f(context, PROCEDURE, L); add_ast(P,C); add_ast(P,E);}
procedure(P)      ::= TK_LABEL(L) TK_PROCEDURE TK_EQUAL TK_VOID(V) expose(E).
                      { P = ast_f(context, PROCEDURE, L); add_ast(P,ast_f(context, VOID, V)); add_ast(P,E);}
procedure(P)      ::= TK_LABEL(L) TK_PROCEDURE expose(E).
                      { P = ast_f(context, PROCEDURE, L); add_ast(P,ast_ft(context, VOID)); add_ast(P,E);}

arg(P)            ::= TK_ARG arg_list(A).
                      { P = A;}

/* Argument Templates */
arg_list(L)       ::= . { L = ast_ft(context, ARGS); }
arg_list(L)       ::= argument(T). { L = ast_ft(context, ARGS); add_ast(L,T); }
arg_list(L)       ::= arg_list(L1) TK_COMMA argument(T). { L = L1; add_ast(L,T); }
/* Without Optional Flag (?) */
argument(T)       ::= TK_EXPOSE var_symbol(V) TK_EQUAL expression(E).
                      { T = ast_ft(context, ARG); V->node_type = VAR_REFERENCE; T->is_ref_arg = 1; T->is_opt_arg = 1;
                        add_ast(T,V); add_ast(T,E); }
argument(T)       ::= var_symbol(V) TK_EQUAL expression(E).
                      { T = ast_ft(context, ARG); V->node_type = VAR_TARGET; T->is_ref_arg = 0; T->is_opt_arg = 1;
                        add_ast(T,V); add_ast(T,E); }
argument(T)       ::= var_symbol(V) TK_EQUAL type_def(E).
                      { T = ast_ft(context, ARG); V->node_type = VAR_TARGET; T->is_ref_arg = 0; T->is_opt_arg = 0;
                        add_ast(T,V); add_ast(T,E); }
argument(T)       ::= TK_EXPOSE var_symbol(V) TK_EQUAL type_def(E).
                      { T = ast_ft(context, ARG); V->node_type = VAR_REFERENCE; T->is_ref_arg = 1; T->is_opt_arg = 0;
                        add_ast(T,V); add_ast(T,E); }
argument(T)       ::= TK_ELLIPSIS(V) TK_EQUAL type_def(E).
                      { T = ast_ft(context, ARG); add_ast(T,ast_f(context, VARG, V)); add_ast(T,E); T->is_ref_arg = 0;
                        T->is_opt_arg = 0; T->is_varg = 1; }
argument(T)       ::= TK_EXPOSE TK_ELLIPSIS(V) TK_EQUAL type_def(E).
                      { T = ast_ft(context, ARG); add_ast(T,ast_f(context, VARG_REFERENCE, V)); add_ast(T,E);
                        T->is_ref_arg = 1; T->is_opt_arg = 0; T->is_varg = 1; }

/* With Optional (?) Flag */
argument(T)       ::= TK_EXPOSE TK_OPTIONAL var_symbol(V) TK_EQUAL expression(E).
                      { T = ast_ft(context, ARG); V->node_type = VAR_REFERENCE; T->is_ref_arg = 1; T->is_opt_arg = 1;
                        add_ast(T,V); add_ast(T,E); }
argument(T)       ::= TK_OPTIONAL var_symbol(V) TK_EQUAL expression(E).
                      { T = ast_ft(context, ARG); V->node_type = VAR_TARGET; T->is_ref_arg = 0; T->is_opt_arg = 1;
                        add_ast(T,V); add_ast(T,E); }
argument(T)       ::= TK_OPTIONAL var_symbol(V) TK_EQUAL type_def(E).
                      { T = ast_ft(context, ARG); V->node_type = VAR_TARGET; T->is_ref_arg = 0; T->is_opt_arg = 1;
                        add_ast(T,V); add_ast(T,E); }
argument(T)       ::= TK_EXPOSE TK_OPTIONAL var_symbol(V) TK_EQUAL type_def(E).
                      { T = ast_ft(context, ARG); V->node_type = VAR_REFERENCE; T->is_ref_arg = 1; T->is_opt_arg = 1;
                        add_ast(T,V); add_ast(T,E); }
/* Errors */
argument(E)         ::= error.
                      { E = ast_errh(context, "SYNTAX_ERROR"); }
argument(E)         ::= TK_VAR_SYMBOL(S).
                      { E = ast_err(context, "MISSING_TYPE", S); }
argument(E)         ::= TK_EXPOSE TK_VAR_SYMBOL(S).
                      { E = ast_err(context, "MISSING_TYPE", S); }
argument(E)         ::= TK_CLASS_TYPE(S).
                      { E = ast_err(context, "MISSING_TYPE", S); }
argument(E)         ::= TK_EXPOSE TK_CLASS_TYPE(S).
                      { E = ast_err(context, "MISSING_TYPE", S); }
argument(E)         ::= TK_STEM(S).
                      { E = ast_err(context, "MISSING_TYPE", S); }
argument(E)         ::= TK_EXPOSE TK_STEM(S).
                      { E = ast_err(context, "MISSING_TYPE", S); }
argument(E)         ::= TK_CLASS_STEM(S).
                      { E = ast_err(context, "MISSING_TYPE", S); }
argument(E)         ::= TK_EXPOSE TK_CLASS_STEM(S).
                      { E = ast_err(context, "MISSING_TYPE", S); }
argument(E)         ::= TK_OPTIONAL(S) TK_ELLIPSIS TK_EQUAL type_def.
                      { E = ast_err(context, "OPTIONAL_ELLIPSIS", S); }
argument(E)         ::= TK_EXPOSE TK_OPTIONAL(S) TK_ELLIPSIS TK_EQUAL type_def.
                      { E = ast_err(context, "OPTIONAL_ELLIPSIS", S); }
argument(E)         ::= TK_ELLIPSIS TK_EQUAL(X) expression.
                      { E = ast_err(context, "MUST_EQUAL_TYPE", X); }
argument(E)         ::= TK_EXPOSE TK_ELLIPSIS TK_EQUAL(X) expression.
                      { E = ast_err(context, "MUST_EQUAL_TYPE", X); }
argument(E)         ::= TK_ELLIPSIS(S).
                      { E = ast_err(context, "MISSING_TYPE", S); }

/* Instructions */

/* ADDRESS */
address(A)   ::= TK_ADDRESS literal(S) expression(E).
             { A = ast_ft(context, ADDRESS); add_ast(A,S); add_ast(A,E); }
address(A)   ::= TK_ADDRESS literal(S) expression(E) redirect_list(R).
             { A = ast_ft(context, ADDRESS); add_ast(A,S); add_ast(A,E); add_ast(A,R); }
redirect_list(L) ::= redirect(E).
                 { L = E; }
redirect_list(L) ::= redirect_list(L1) redirect(L2).
                 { L = L1; add_sbtr(L,L2); }
redirect(C)      ::= TK_OUTPUT var_symbol(S).
                 { C = ast_ft(context, REDIRECT_OUT); add_ast(C, S); }
redirect(C)      ::= TK_ERROR var_symbol(S).
                 { C = ast_ft(context, REDIRECT_ERROR); add_ast(C, S); }
redirect(C)      ::= TK_INPUT expression(S).
                 { C = ast_ft(context, REDIRECT_IN); add_ast(C, S); }
redirect(C)      ::= TK_EXPOSE expose_list_as_var(S).
                 { C = ast_ft(context, REDIRECT_EXPOSE); add_ast(C, S); }
expose_list_as_var(I) ::= var_symbol(L).
                      { I = L; }
expose_list_as_var(I) ::= expose_list_as_var(I1) var_symbol(L).
                      { I = I1; add_sbtr(I,L); }

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
assembler_op(OP)         ::= TK_EXIT(S). /* EXIT is also a REXX keyword */
                         { OP = ast_f(context, ASSEMBLER, S); }
assembler_arg(A)         ::= var_symbol(B).
                         { A = B; }
assembler_arg(A)         ::= TK_VAR_SYMBOL(S) TK_OPEN_BRACKET TK_CLOSE_BRACKET.
                         { A = ast_f(context, FUNC_SYMBOL, S); }
assembler_arg(A)         ::= TK_FLOAT(S).
                         { A = ast_f(context, FLOAT,S); }
assembler_arg(A)         ::= TK_DECIMAL(S).
                         { A = ast_f(context, DECIMAL,S); }
assembler_arg(A)         ::= TK_MINUS(O) TK_FLOAT(S).
                         { A = ast_f(context, OP_NEG, O); add_ast(A, ast_f(context, FLOAT,S)); }
assembler_arg(A)         ::= TK_HIGH_PRIORITY_MINUS(O) TK_FLOAT(S).
                         { A = ast_f(context, OP_NEG, O); add_ast(A, ast_f(context, FLOAT,S)); }
assembler_arg(A)         ::= TK_MINUS(O) TK_DECIMAL(S).
                         { A = ast_f(context, OP_NEG, O); add_ast(A, ast_f(context, DECIMAL,S)); }
assembler_arg(A)         ::= TK_HIGH_PRIORITY_MINUS(O) TK_DECIMAL(S).
                         { A = ast_f(context, OP_NEG, O); add_ast(A, ast_f(context, DECIMAL,S)); }
assembler_arg(A)         ::= TK_INTEGER(S).
                         { A = ast_f(context, INTEGER,S); }
assembler_arg(A)         ::= TK_MINUS(O) TK_INTEGER(S).
                         { A = ast_f(context, OP_NEG, O); add_ast(A, ast_f(context, INTEGER,S)); }
assembler_arg(A)         ::= TK_HIGH_PRIORITY_MINUS(O) TK_INTEGER(S).
                         { A = ast_f(context, OP_NEG, O); add_ast(A, ast_f(context, INTEGER,S)); }
assembler_arg(A)         ::= TK_STRING(S).
                         { A = ast_fstr(context,S); }

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


### Pull
    pull ::= 'PULL' t:template_list?
         -> (PARSE (OPTIONS UPPER?) PULL t?);
*/

/* Numeric */
numeric(I) ::= TK_NUMERIC TK_VAR_SYMBOL(T) TK_INTEGER(S).
    {
    if (tokenis(T,"digits"))
        { I = ast_f(context, DEC_DIGITS, T); add_ast(I, ast_f(context, INTEGER,S)); }
    else if (tokenis(T,"form"))
        { I = mknd_err(ast_f(context, TK_INTEGER,S), "INVALID_DECIMAL_FORM_VALUE"); }
    else if (tokenis(T,"fuzz"))
        { I = ast_f(context, DEC_FUZZ, T); add_ast(I, ast_f(context, INTEGER,S)); }
    else if (tokenis(T,"case"))
        { I = mknd_err(ast_f(context, TK_INTEGER,S), "INVALID_DECIMAL_CASE_VALUE"); }
    else if (tokenis(T,"standard"))
        { I = mknd_err(ast_f(context, TK_INTEGER,S), "INVALID_DECIMAL_STANDARD_VALUE"); }
    else
        { I = mknd_err(ast_f(context, LITERAL,T), "INVALID_NUMERIC_OPTION"); }
    }

numeric(I) ::= TK_NUMERIC TK_VAR_SYMBOL(T) TK_VAR_SYMBOL(S).
    {
    if (tokenis(S,"inherited"))
        {
        if (tokenis(T,"digits"))
            { I = ast_f(context, DEC_DIGITS, T); }
        else if (tokenis(T,"form"))
            { I = ast_f(context, DEC_FORM, T); }
        else if (tokenis(T,"fuzz"))
            { I = ast_f(context, DEC_FUZZ, T); }
        else if (tokenis(T,"case"))
            { I = ast_f(context, DEC_CASE, T); }
        else if (tokenis(T,"standard"))
            { I = ast_f(context, DEC_STANDARD, T); }
        else
            { I = mknd_err(ast_f(context, LITERAL,T), "INVALID_NUMERIC_OPTION"); }
        }
    else {
        if (tokenis(T,"digits"))
            { I = mknd_err(ast_f(context, TK_INTEGER,S), "INVALID_DECIMAL_DIGITS_RANGE"); }
        else if (tokenis(T,"form"))
            { I = ast_f(context, DEC_FORM, T); add_ast(I,ast_f(context, LITERAL, S)); }
        else if (tokenis(T,"fuzz"))
            { I = mknd_err(ast_f(context, TK_INTEGER,S), "INVALID_DECIMAL_FUZZ_RANGE"); }
        else if (tokenis(T,"case"))
            { I = ast_f(context, DEC_CASE, T); add_ast(I,ast_f(context, LITERAL, S)); }
        else if (tokenis(T,"standard"))
            { I = ast_f(context, DEC_STANDARD, T); add_ast(I,ast_f(context, LITERAL, S)); }
        else
            { I = mknd_err(ast_f(context, LITERAL,T), "INVALID_NUMERIC_OPTION"); }
        }
    }

numeric(I) ::= TK_NUMERIC TK_VAR_SYMBOL(T) TK_MINUS(S) TK_INTEGER.
    {
    if (tokenis(T,"digits"))
        { I = mknd_err(ast_f(context, OP_MINUS,S), "INVALID_DECIMAL_DIGITS_RANGE"); }
    else if (tokenis(T,"form"))
        { I = mknd_err(ast_f(context, OP_MINUS,S), "INVALID_DECIMAL_FORM_VALUE"); }
    else if (tokenis(T,"fuzz"))
        { I = mknd_err(ast_f(context, OP_MINUS,S), "INVALID_DECIMAL_FUZZ_RANGE"); }
    else if (tokenis(T,"case"))
        { I = mknd_err(ast_f(context, OP_MINUS,S), "INVALID_DECIMAL_CASE_VALUE"); }
    else
        { I = mknd_err(ast_f(context, LITERAL,T), "INVALID_NUMERIC_OPTION"); }
    }

numeric(I) ::= TK_NUMERIC TK_VAR_SYMBOL(T) TK_HIGH_PRIORITY_MINUS(S) TK_INTEGER.
    {
    if (tokenis(T,"digits"))
        { I = mknd_err(ast_f(context, OP_MINUS,S), "INVALID_DECIMAL_DIGITS_RANGE"); }
    else if (tokenis(T,"form"))
        { I = mknd_err(ast_f(context, OP_MINUS,S), "INVALID_DECIMAL_FORM_VALUE"); }
    else if (tokenis(T,"fuzz"))
        { I = mknd_err(ast_f(context, OP_MINUS,S), "INVALID_DECIMAL_FUZZ_RANGE"); }
    else if (tokenis(T,"case"))
        { I = mknd_err(ast_f(context, OP_MINUS,S), "INVALID_DECIMAL_CASE_VALUE"); }
    else
        { I = mknd_err(ast_f(context, LITERAL,T), "INVALID_NUMERIC_OPTION"); }
    }

numeric(I) ::= TK_NUMERIC(N) TK_VAR_SYMBOL(T).
    {
      if (tokenis(T, "inherited")) {
          ASTNode* _digits = ast_f(context, DEC_DIGITS, N);
          ASTNode* _fuzz = ast_fstk(context, _digits); _fuzz->node_type = DEC_FUZZ;
          ASTNode* _forms = ast_fstk(context, _digits); _forms->node_type = DEC_FORM;
          ASTNode* _case = ast_fstk(context, _digits); _case->node_type = DEC_CASE;
          ASTNode* _standard = ast_fstk(context, _digits); _standard->node_type = DEC_STANDARD;
          add_sbtr( _digits, _standard );
          add_sbtr( _digits, _fuzz );
          add_sbtr( _digits, _forms );
          add_sbtr( _digits, _case );
          I = _digits;
      }
      else
         { I = mknd_err(ast_f(context, LITERAL,T), "INVALID_NUMERIC_OPTION"); }
    }

/* Return */
return(I) ::= TK_RETURN(T) expression(E).
    { I = ast_f(context, RETURN, T); add_ast(I,E); }

return(I) ::= TK_RETURN(T).
    { I = ast_f(context, RETURN, T); }

/* EXIT */
exit(I) ::= TK_EXIT(T) expression(E).
    { I = ast_f(context, EXIT, T); add_ast(I,E); }

exit(I) ::= TK_EXIT(T).
    { I = ast_f(context, EXIT, T); }

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
%left TK_STRING TK_FLOAT TK_DECIMAL TK_INTEGER TK_VAR_SYMBOL.
%left TK_OPEN_BRACKET.
%nonassoc TK_EQUAL.

function_name(N)       ::= TK_VAR_SYMBOL(S).
                           { N = ast_f(context, FUNCTION, S); }
function_name(N)       ::= TK_STRING(S).
                           { N = ast_f(context, FUNCTION, S); }
call(I) ::= TK_CALL(T) function_name(F) expression_list(E).
        { I = ast_f(context, CALL, T); add_ast(I,F); if (E) add_ast(F,E); }
call(I) ::= TK_CALL(T) TK_VAR_SYMBOL(S) function_parameters(P). [TK_VAR_SYMBOL]
        { I = ast_f(context, CALL, T); ASTNode *F = ast_f(context, FUNCTION, S); add_ast(I, F); if (P) add_ast(F, P); }
/* Support member calls: CALL obj.method(args...) */
call(I) ::= TK_CALL(T) TK_STEM(S) stemparts(P) function_parameters(PP).
        {
           ASTNode *F;
           ASTNode *last = P;
           ASTNode *prev = NULL;
           while (last->sibling) { prev = last; last = last->sibling; }
           if (prev) { prev->sibling = NULL; } else { P = NULL; }
           F = ast_f(context, MEMBER_CALL, last->token);
           if (F->node_string && F->node_string[0] == '.') {
               F->node_string++;
               F->node_string_length--;
           }
           {
               ASTNode *lhs = ast_f(context, VAR_SYMBOL, S);
               if (P) add_ast(lhs, P);
               add_ast(F, lhs);
               if (PP) add_ast(F, PP);
           }
           I = ast_f(context, CALL, T);
           add_ast(I, F);
        }
call(I) ::= TK_CALL(T) ANYTHING(E).
        { I = ast_f(context, CALL, T); add_ast(I,ast_err(context, "EXPECTED_PROCEDURE", E)); }

/* Expression Lists */
expression_list(L)     ::= .
                         { L = ast_ft(context, NOVAL); }
expression_list(L)     ::= expression_in_list(E).
                         { L = E; }
expression_list(L)     ::= expression_list(L1) TK_COMMA expression_in_list(E).
                         { if (L1) L = L1; else L = ast_ft(context, NOVAL); add_sbtr(L,E);}
expression_list(L)     ::= expression_list(L1) TK_COMMA.
                         { if (L1) L = L1; else L = ast_ft(context, NOVAL); add_sbtr(L, ast_ft(context, NOVAL)); }

/* Expression terminal nodes */
term(F)                ::= TK_VAR_SYMBOL(S) function_parameters(P).
                           { F = ast_f(context, FUNCTION, S); if (P) add_ast(F,P); }
term(F)                ::= TK_STEM(S) stemparts(P) function_parameters(PP).
                           {
                               ASTNode *last = P;
                               ASTNode *prev = NULL;
                               while (last->sibling) {
                                   prev = last;
                                   last = last->sibling;
                               }

                               if (prev) {
                                   prev->sibling = NULL;
                               } else {
                                   P = NULL;
                               }

                               F = ast_f(context, MEMBER_CALL, last->token);
                               if (F->node_string && F->node_string[0] == '.') {
                                   F->node_string++;
                                   F->node_string_length--;
                               }

                               ASTNode *lhs = ast_f(context, VAR_SYMBOL, S);
                               if (P) add_ast(lhs, P);
                               add_ast(F, lhs);
                               if (PP) add_ast(F, PP);
                           }
term(F)                ::= TK_STRING(S) function_parameters(P).
                           { F = ast_f(context, FUNCTION, S); if (P) add_ast(F,P); }
function_parameters(P) ::= TK_OPEN_BRACKET expression_list(E) TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           { P = E; }
term(A)                ::= var_symbol(B). [TK_VAR_SYMBOL]
                         { A = B; }
term(A)                ::= TK_FLOAT(S).
                         { A = ast_f(context, FLOAT,S); }
term(A)                ::= TK_DECIMAL(S).
                         { A = ast_fdec(context,S); }
term(A)                ::= TK_INTEGER(S).
                         { A = ast_f(context, INTEGER,S); }
term(A)                ::= TK_STRING(S).
                         { A = ast_fstr(context,S); }

/* Special Operator - ARG */
term(F)                ::= TK_ARG(A) TK_OPEN_BRACKET TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           { F = ast_f(context, OP_ARGS, A); }
term(F)                ::= TK_ARG(A) TK_OPEN_BRACKET expression_in_list(E) TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           { F = ast_f(context, OP_ARG_VALUE, A); add_ast(F, E);}
term(F)                ::= TK_ARG(A) TK_OPEN_BRACKET expression_in_list(EX) TK_COMMA TK_STRING(OP) TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           {
                              if (OP->length>2 && toupper(OP->token_string[1]) == 'E') {
                                 F = ast_f(context, OP_ARG_IX_EXISTS, A); add_ast(F, EX);
                              }
                              else if (OP->length>2 && toupper(OP->token_string[1]) == 'O') {
                                 F = ast_ft(context, OP_NOT); add_ast(add_ast(F, ast_f(context, OP_ARG_IX_EXISTS, A)), EX);
                              }
                              else F = mknd_err(ast_fstr(context,OP), "INVALID_ARG_OPTION");
                           }
term(F)                ::= TK_ARG(A) TK_OPEN_BRACKET expression_in_list(EX) TK_COMMA TK_VAR_SYMBOL(OP) TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           {
                              if (OP->length>0 && toupper(OP->token_string[0]) == 'E') {
                                 F = ast_f(context, OP_ARG_IX_EXISTS, A); add_ast(F, EX);
                              }
                              else if (OP->length>0 && toupper(OP->token_string[0]) == 'O') {
                                 F = ast_ft(context, OP_NOT); add_ast(add_ast(F, ast_f(context, OP_ARG_IX_EXISTS, A)), EX);
                              }
                              else F = ast_err(context, "INVALID_ARG_OPTION", OP);
                           }
term(F)                ::= TK_ARG TK_OPEN_BRACKET(A) error TK_CLOSE_BRACKET.
                           { F = ast_err(context, "INVALID_ARG_SYNTAX", A); }
term(F)                ::= TK_ARG TK_OPEN_BRACKET ANYTHING(A).
                           { F = ast_err(context, "INVALID_ARG_SYNTAX", A); }

/* Special Operator - ? */
term(F)                ::= TK_OPTIONAL TK_VAR_SYMBOL(S). [TK_VAR_SYMBOL]
                           { F = ast_f(context, OP_ARG_EXISTS, S); }

/* Special Operator - arg pseudo array/stem */
term(A)                ::= TK_ARG(S) array_parameters(P). [TK_VAR_SYMBOL]
                           { A = ast_f(context, OP_ARG_VALUE, S); if (P) add_ast(A,P); }
term(A)                ::= TK_ARG_STEM(S) stemparts(P). [TK_VAR_SYMBOL]
                           { A = ast_f(context, OP_ARG_VALUE, S); if (P) add_ast(A,P); }

/* These Keywords can be trapped at error terms - e.g. they are not instructions */
term(E)                 ::= TK_OPTIONS(K). [ANYTHING] { E = mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"); }
term(E)                 ::= TK_NAMESPACE(K). [ANYTHING] { E = mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"); }
term(E)                 ::= TK_IMPORT(K). [ANYTHING] { E = mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"); }
term(E)                 ::= TK_VOID(K). [ANYTHING] { E = mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"); }
term(E)                 ::= TK_OPTIONAL(K). [ANYTHING] { E = mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"); }

bracket(A)           ::= term(T).
                         { A = T; }
bracket(A)           ::= TK_OPEN_BRACKET expression(B) TK_CLOSE_BRACKET.
                         { A = B; }
/* Standalone class factory call as a primary */
bracket(F)           ::= TK_CLASS_TYPE(S) function_parameters(P).
                           {
                               F = ast_f(context, FACTORY_CALL, S);
                               if (P) add_ast(F,P);
                               if (F->node_string && F->node_string[0] == '.') {
                                   F->node_string++;
                                   F->node_string_length--;
                               }
                           }

/* These are the normal expression form in unambiguous form */
postfix(P)           ::= bracket(B).
                         { P = B; }
postfix(A)           ::= postfix(B) TK_CLASS_TYPE(S) function_parameters(PP). [TK_CLASS_TYPE]
                         { A = ast_f(context, MEMBER_CALL, S);
                           if (A->node_string && A->node_string[0] == '.') {
                               A->node_string++;
                               A->node_string_length--; }
                           add_ast(A,B); if (PP) add_ast(A,PP); }

prefix_expression(P) ::= postfix(B). [ANYTHING] { P = B; }
prefix_expression(A) ::= TK_NOT(O) prefix_expression(C).
                         { A = ast_f(context, OP_NOT, O); add_ast(A,C); }
prefix_expression(A) ::= TK_PLUS(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_PLUS, O); add_ast(A,C); }
prefix_expression(A) ::= TK_HIGH_PRIORITY_MINUS(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_NEG, O); add_ast(A,C); }
/*
 * power_expression contains rules for both left and right-associative power operators.
 * The lexer ensures only one of TK_POWER_L or TK_POWER_R is present in the
 * token stream for any given file, so there is no ambiguity.
 */
// Rule for the Left-associative power operator - NUMERIC_CLASSIC
power_expression_L(A) ::= power_expression_L(B) TK_POWER_L(O) prefix_expression(C).
                          { A = ast_f(context, OP_POWER, O); add_ast(A,B); add_ast(A,C); }
power_expression_L(P) ::= prefix_expression(E).  { P = E; }
// Rule for the Right-associative power operator - NUMERIC_COMMON
power_expression_R(A) ::= power_expression_L(B) TK_POWER_R(O) power_expression_R(C).
                          { A = ast_f(context, OP_POWER, O); add_ast(A,B); add_ast(A,C); }
power_expression_R(P) ::= power_expression_L(E).  { P = E; }

// Low precedence prefix expression
low_prefix_expression(P) ::= power_expression_R(E).
                  { P = E; }
low_prefix_expression(A) ::= TK_MINUS(O) power_expression_R(C).
                  { A = ast_f(context, OP_NEG, O); add_ast(A,C); }
multiplication(P)    ::= low_prefix_expression(E).
                         { P = E; }
multiplication(A)    ::= multiplication(B) TK_MULT(O) low_prefix_expression(C).
                         { A = ast_f(context, OP_MULT, O); add_ast(A,B); add_ast(A,C); }
multiplication(A)    ::= multiplication(B) TK_DIV(O) low_prefix_expression(C).
                         { A = ast_f(context, OP_DIV, O); add_ast(A,B); add_ast(A,C); }
multiplication(A)    ::= multiplication(B) TK_IDIV(O) low_prefix_expression(C).
                         { A = ast_f(context, OP_IDIV, O); add_ast(A,B); add_ast(A,C); }
multiplication(A)    ::= multiplication(B) TK_MOD(O) low_prefix_expression(C).
                         { A = ast_f(context, OP_MOD, O); add_ast(A,B); add_ast(A,C); }
addition(P)          ::= multiplication(E).
                         { P = E; }
addition(A)          ::= addition(B) TK_PLUS(O) multiplication(C).
                         { A = ast_f(context, OP_ADD, O); add_ast(A,B); add_ast(A,C); }
addition(A)          ::= addition(B) TK_MINUS(O) multiplication(C).
                         { A = ast_f(context, OP_MINUS, O); add_ast(A,B); add_ast(A,C); }
addition(A)          ::= addition(B) TK_HIGH_PRIORITY_MINUS(O) multiplication(C).
                         { A = ast_f(context, OP_MINUS, O); add_ast(A,B); add_ast(A,C); }

/* These are for expressions "after" a concat defined by a whitespace to avoid
 * ambiguous issues with prefix operators (i.e. these miss out the +/- prefixes)
 */
postfix_c(P)         ::= bracket(B).
                         { P = B; }
postfix_c(A)         ::= postfix_c(B) TK_CLASS_TYPE(S) function_parameters(PP). [TK_CLASS_TYPE]
                         { A = ast_f(context, MEMBER_CALL, S);
                           if (A->node_string && A->node_string[0] == '.') {
                               A->node_string++;
                               A->node_string_length--; }
                           add_ast(A,B); if (PP) add_ast(A,PP); }

prefix_expression_c(P) ::= postfix_c(B). [ANYTHING] { P = B; }

prefix_expression_c(A) ::= TK_NOT(O) prefix_expression_c(C).
                         { A = ast_f(context, OP_NOT, O); add_ast(A,C); }

// Rule for the Left-associative power operator - NUMERIC_CLASSIC
power_expression_L_c(A) ::= power_expression_L_c(B) TK_POWER_L(O) prefix_expression_c(C).
                          { A = ast_f(context, OP_POWER, O); add_ast(A,B); add_ast(A,C); }
power_expression_L_c(P) ::= prefix_expression_c(E).  { P = E; }
// Rule for the Right-associative power operator - NUMERIC_COMMON
power_expression_R_c(A) ::= power_expression_L_c(B) TK_POWER_R(O) power_expression_R_c(C).
                          { A = ast_f(context, OP_POWER, O); add_ast(A,B); add_ast(A,C); }
power_expression_R_c(P) ::= power_expression_L_c(E).  { P = E; }

multiplication_c(P)  ::= power_expression_R_c(E).
                         { P = E; }
multiplication_c(A)  ::= multiplication_c(B) TK_MULT(O) power_expression_R_c(C).
                         { A = ast_f(context, OP_MULT, O); add_ast(A,B); add_ast(A,C); }
multiplication_c(A)  ::= multiplication_c(B) TK_DIV(O) power_expression_R_c(C).
                         { A = ast_f(context, OP_DIV, O); add_ast(A,B); add_ast(A,C); }
multiplication_c(A)  ::= multiplication_c(B) TK_IDIV(O) power_expression_R_c(C).
                         { A = ast_f(context, OP_IDIV, O); add_ast(A,B); add_ast(A,C); }
multiplication_c(A)  ::= multiplication_c(B) TK_MOD(O) power_expression_R_c(C).
                         { A = ast_f(context, OP_MOD, O); add_ast(A,B); add_ast(A,C); }
addition_c(P)        ::= multiplication_c(E).
                         { P = E; }
addition_c(A)        ::= addition_c(B) TK_PLUS(O) multiplication_c(C).
                         { A = ast_f(context, OP_ADD, O); add_ast(A,B); add_ast(A,C); }
addition_c(A)        ::= addition_c(B) TK_MINUS(O) multiplication_c(C).
                         { A = ast_f(context, OP_MINUS, O); add_ast(A,B); add_ast(A,C); }
addition_c(A)        ::= addition_c(B) TK_HIGH_PRIORITY_MINUS(O) multiplication_c(C).
                         { A = ast_f(context, OP_MINUS, O); add_ast(A,B); add_ast(A,C); }

/* Back to normal expressions in the usual unambiguous form */
concatenation(P)     ::= addition(E).
                         { P = E; }
concatenation(A)     ::= concatenation(B) TK_CONCAT(O) addition(C).
                         { A = ast_f(context, OP_CONCAT, O); add_ast(A,B); add_ast(A,C); }
concatenation(A)     ::= concatenation(B) addition_c(C). [IMPLICIT_CONCAT] /* Note the addition_c */
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
and_expression(E)  ::= TK_POWER_L(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_POWER_R(U) error. { E = ast_err(context, "BADEXPR", U); }
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

/* expressions in a list cannot expression() errors above because of parsing conflicts */
expression_in_list(P) ::= and_expression(E). { P = E; }

/* Finally set the nodes with the highest precedence */
%left TK_BADCOMMENT.

/* Classes / Factories / Methods */

class_def(C) ::= TK_LABEL(L) TK_CLASS opt_of(O).
{
  C = ast_f(context, CLASS_DEF, L);
  if (O) add_ast(C, O);
}

opt_of(O) ::= . { O = NULL; }
opt_of(O) ::= TK_OF type_def(T). { O = T; }

factory_def(F) ::= TK_MULT_LABEL(L) TK_FACTORY.
{
  F = ast_f(context, FACTORY, L);
  add_ast(F, ast_ft(context, VOID));
}

method_def(M) ::= TK_LABEL(L) TK_METHOD opt_method_return_type(T).
{
  M = ast_f(context, METHOD, L);
  if (T) add_ast(M, T);
  else add_ast(M, ast_ft(context, VOID));
}

opt_method_return_type(T) ::= . { T = NULL; }
opt_method_return_type(T) ::= TK_EQUAL type_def(D). { T = D; }
opt_method_return_type(T) ::= TK_EQUAL TK_VOID(V). { T = ast_f(context, VOID, V); }

/* Attribute Mapping */
opt_with(W) ::= . { W = NULL; }
opt_with(W) ::= TK_WITH register_mapping(R). { W = R; }

register_mapping(R) ::= TK_REGISTER(K) register_index(I) opt_register_attribute(A).
{
  R = ast_f(context, NODE_REGISTER, K);
  add_ast(R, I);
  if (A) add_ast(R, A);
}

register_index(I) ::= TK_STEMINT(T).
{
  /* Remove leading dot from .index */
  T->token_string++;
  T->length--;
  I = ast_f(context, INTEGER, T);
}

opt_register_attribute(A) ::= . { A = NULL; }
opt_register_attribute(A) ::= TK_STEMVAR(T).
{
  /* Remove leading dot from .attribute */
  T->token_string++;
  T->length--;
  A = ast_f(context, VAR_SYMBOL, T);
}
