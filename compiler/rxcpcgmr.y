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
#include <string.h>
#include "rxcpmain.h"

static char *levelc_diag_token_text(Token *token) {
    if (!token) return strdup("end-of-source");
    if (!token->token_string || token->length <= 0) return strdup("end-of-clause");
    if (token->token_string[0] == '\n' || token->token_string[0] == '\r') return strdup("end-of-clause");
    return rx_strndup(token->token_string, (size_t)token->length);
}

static char *levelc_diag_line_text(Token *token) {
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%d", token ? token->line + 1 : 0);
    return strdup(buffer);
}

static ASTNode *levelc_error_then_or_else(Context *context,
                                          const char *standard_code,
                                          Token *token,
                                          ASTNode *statement) {
    ASTNode *block;

    block = ast_ft(context, INSTRUCTIONS);
    add_ast(block, rxcp_levelc_ast_error(context, standard_code, token));
    if (statement) add_ast(block, statement);
    return block;
}

static ASTNode *levelc_if_missing_then(Context *context, Token *if_token, ASTNode *condition) {
    ASTNode *node;
    char *line;
    char *found;

    node = ast_f(context, IF, if_token);
    if (condition) add_ast(node, condition);

    line = levelc_diag_line_text(if_token);
    found = levelc_diag_token_text(context ? context->current_parser_token : 0);
    add_ast(node, rxcp_levelc_ast_error_insert2(context, "18.1", if_token,
                                                "linenumber", line,
                                                "token", found));
    free(line);
    free(found);
    return node;
}

static ASTNode *levelc_if_bad_condition(Context *context,
                                        Token *if_token,
                                        Token *bad_token,
                                        ASTNode *then_statement) {
    ASTNode *node;

    node = ast_f(context, IF, if_token);
    add_ast(node, rxcp_levelc_ast_error_token(context, "35.1", bad_token));
    if (then_statement) add_ast(node, then_statement);
    return node;
}
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

instruction(I) ::= unexpected_then(T).
{
    I = T;
}

instruction(I) ::= unexpected_else(E).
{
    I = E;
}

instruction(E) ::= CTK_BADCOMMENT(T).
{
    E = rxcp_levelc_ast_error(context, "6.1", T);
}

instruction(E) ::= CTK_UNKNOWN(T).
{
    E = rxcp_levelc_ast_error_token(context, "13.1", T);
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

if_instruction(I) ::= CTK_IF(T) expression(C) CTK_THEN then_instruction(Then) else_clause(Else).
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

if_instruction(I) ::= CTK_IF(T) expression(C). [CTK_IF]
{
    I = levelc_if_missing_then(context, T, C);
}

if_instruction(I) ::= CTK_IF(T) CTK_THEN(Th) then_instruction(Then). [CTK_IF]
{
    I = levelc_if_bad_condition(context, T, Th, Then);
}

if_instruction(I) ::= CTK_IF(T) CTK_THEN(Th). [CTK_IF]
{
    I = levelc_if_bad_condition(context, T, Th, rxcp_levelc_ast_error(context, "14.3", Th));
}

then_instruction(I) ::= instruction(S).
{
    I = S;
}

else_clause(I) ::= CTK_ELSE instruction(S).
{
    I = S;
}

else_clause(I) ::= CTK_ELSE(T).
{
    I = rxcp_levelc_ast_error(context, "14.4", T);
}

recovery_instruction(I) ::= simple_instruction(S).
{
    I = S;
}

recovery_instruction(I) ::= if_instruction(F).
{
    I = F;
}

unexpected_then(E) ::= CTK_THEN(T) recovery_instruction(S).
{
    E = levelc_error_then_or_else(context, "8.1", T, S);
}

unexpected_then(E) ::= CTK_THEN(T).
{
    E = rxcp_levelc_ast_error(context, "8.1", T);
}

unexpected_else(E) ::= CTK_ELSE(T) recovery_instruction(S).
{
    E = levelc_error_then_or_else(context, "8.2", T, S);
}

unexpected_else(E) ::= CTK_ELSE(T).
{
    E = rxcp_levelc_ast_error(context, "8.2", T);
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
