/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 */

/**
 * Level C Classic REXX tracer grammar.
 *
 * This grammar is intentionally small. It proves the Level C-only parser path
 * and DSLSH source-tree path without enabling Level C compilation.
 */

%name RexxC
%token_type { Token* }
%default_type { ASTNode* }
%extra_argument { Context *context }
%start_symbol program

%include {
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "rxcpmain.h"
}

%token CTK_UNKNOWN CTK_BADCOMMENT CTK_EOS CTK_EOC CTK_VAR_SYMBOL CTK_LABEL CTK_INTEGER CTK_STRING.
%token CTK_EQUAL CTK_SAY CTK_IF CTK_THEN CTK_ELSE.

%nonassoc CTK_IF.
%nonassoc CTK_ELSE.

%stack_size 0

%syntax_error {
    context->syntax_error_clause_token = context->current_clause_token;
    context->syntax_error_token = TOKEN;
}

program(P) ::= top_instruction_list(I) CTK_EOS.
{
    ASTNode *file;

    context->ast = ast_ft(context, REXX_UNIVERSE);
    P = context->ast;
    file = ast_ft(context, PROGRAM_FILE);
    add_ast(context->ast, file);
    add_ast(file, I);
}

top_instruction_list(L) ::= top_instruction_list(L0) top_instruction(I).
{
    L = L0;
    if (I) add_ast(L, I);
}

top_instruction_list(L) ::= .
{
    L = ast_ft(context, INSTRUCTIONS);
}

top_instruction(I) ::= instruction(S) CTK_EOC.
{
    I = S;
}

top_instruction(I) ::= CTK_LABEL(T).
{
    I = ast_f(context, LABEL, T);
}

top_instruction(I) ::= CTK_EOC.
{
    I = 0;
}

instruction(I) ::= simple_instruction(S).
{
    I = S;
}

instruction(I) ::= if_instruction(F).
{
    I = F;
}

instruction(E) ::= CTK_BADCOMMENT(T).
{
    E = ast_err(context, "BAD_COMMENT", T);
}

instruction(E) ::= CTK_UNKNOWN(T).
{
    E = ast_err(context, "SYNTAX_ERROR", T);
}

simple_instruction(S) ::= say_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= assignment(A).
{
    S = A;
}

say_instruction(S) ::= CTK_SAY(T) expression(E).
{
    S = ast_f(context, SAY, T);
    add_ast(S, E);
}

say_instruction(S) ::= CTK_SAY(T).
{
    S = ast_f(context, SAY, T);
}

assignment(A) ::= CTK_VAR_SYMBOL(V) CTK_EQUAL(T) expression(E).
{
    ASTNode *target;

    A = ast_f(context, ASSIGN, T);
    target = ast_f(context, VAR_TARGET, V);
    add_ast(A, target);
    add_ast(A, E);
}

if_instruction(I) ::= CTK_IF(T) expression(C) CTK_THEN then_instruction(Then) CTK_ELSE instruction(Else).
{
    I = ast_f(context, IF, T);
    add_ast(I, C);
    add_ast(I, Then);
    add_ast(I, Else);
}

if_instruction(I) ::= CTK_IF(T) expression(C) CTK_THEN then_instruction(Then). [CTK_IF]
{
    I = ast_f(context, IF, T);
    add_ast(I, C);
    add_ast(I, Then);
}

then_instruction(I) ::= instruction(S).
{
    I = S;
}

expression(E) ::= expression(L) term(R).
{
    E = ast_ft(context, OP_SCONCAT);
    add_ast(E, L);
    add_ast(E, R);
}

expression(E) ::= term(T).
{
    E = T;
}

term(T) ::= CTK_VAR_SYMBOL(S).
{
    T = ast_f(context, VAR_SYMBOL, S);
}

term(T) ::= CTK_INTEGER(S).
{
    T = ast_f(context, INTEGER, S);
}

term(T) ::= CTK_STRING(S).
{
    T = ast_fstr(context, S);
}
