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

static ASTNode *levelc_missing_then(Context *context,
                                    const char *standard_code,
                                    Token *keyword_token,
                                    ASTNode *condition) {
    ASTNode *node;
    char *line;
    char *found;

    node = ast_f(context, TOKEN, keyword_token);
    if (condition) add_ast(node, condition);

    line = levelc_diag_line_text(keyword_token);
    found = levelc_diag_token_text(context ? context->current_parser_token : 0);
    add_ast(node, rxcp_levelc_ast_error_insert2(context, standard_code, keyword_token,
                                                "linenumber", line,
                                                "token", found));
    free(line);
    free(found);
    return node;
}

static ASTNode *levelc_select_missing_when(Context *context,
                                           Token *select_token,
                                           Token *found_token) {
    ASTNode *node;
    char *line;
    char *found;

    node = ast_f(context, SELECT, select_token);
    line = levelc_diag_line_text(select_token);
    found = levelc_diag_token_text(found_token);
    add_ast(node, rxcp_levelc_ast_error_insert2(context, "7.1", found_token ? found_token : select_token,
                                                "linenumber", line,
                                                "token", found));
    free(line);
    free(found);
    return node;
}

static ASTNode *levelc_unexpected_keyword(Context *context,
                                          const char *standard_code,
                                          Token *token,
                                          ASTNode *statement) {
    ASTNode *block;

    block = ast_ft(context, INSTRUCTIONS);
    add_ast(block, rxcp_levelc_ast_error(context, standard_code, token));
    if (statement) add_ast(block, statement);
    return block;
}

static ASTNode *levelc_end_clause_node(Context *context, Token *end_token, Token *symbol_token) {
    ASTNode *node;

    node = ast_f(context, TOKEN, end_token);
    if (symbol_token) add_ast(node, ast_f(context, VAR_SYMBOL, symbol_token));
    return node;
}

static ASTNode *levelc_missing_expression_rhs(Context *context, NodeType operator_type, Token *operator_token, ASTNode *left) {
    ASTNode *node;
    char *found;

    node = ast_f(context, operator_type, operator_token);
    if (left) add_ast(node, left);
    found = levelc_diag_token_text(context ? context->current_parser_token : 0);
    add_ast(node, rxcp_levelc_ast_error_insert(context, "35.1", operator_token,
                                               "token", found));
    free(found);
    return node;
}

static ASTNode *levelc_bad_expression_start(Context *context, Token *bad_token, ASTNode *tail) {
    ASTNode *node;

    node = rxcp_levelc_ast_error_token(context, "35.1", bad_token);
    if (tail) add_ast(node, tail);
    return node;
}

static ASTNode *levelc_unmatched_open_bracket(Context *context, Token *open_token, ASTNode *expression) {
    ASTNode *node;

    node = ast_f(context, BLOCK_EXPR, open_token);
    if (expression) add_ast(node, expression);
    add_ast(node, rxcp_levelc_ast_error(context, "36", open_token));
    return node;
}

static Token *levelc_end_clause_token(ASTNode *node) {
    return node ? node->token : 0;
}

static ASTNode *levelc_repeat_count(Context *context, ASTNode *expression) {
    ASTNode *repeat;
    ASTNode *count;

    repeat = ast_ft(context, REPEAT);
    count = ast_ft(context, FOR);
    add_ast(count, expression);
    add_ast(repeat, count);
    return repeat;
}

static ASTNode *levelc_repeat_assignment(Context *context,
                                         Token *assign_token,
                                         Token *target_token,
                                         ASTNode *initial_expression,
                                         ASTNode *items) {
    ASTNode *repeat;
    ASTNode *assign;
    ASTNode *target;

    repeat = ast_ft(context, REPEAT);
    assign = ast_f(context, ASSIGN, assign_token);
    target = ast_f(context, VAR_TARGET, target_token);
    add_ast(assign, target);
    add_ast(assign, initial_expression);
    add_ast(repeat, assign);
    if (items) add_ast(repeat, items);
    return repeat;
}

static ASTNode *levelc_condition_node(Context *context,
                                      NodeType node_type,
                                      Token *token,
                                      ASTNode *expression) {
    ASTNode *node;

    node = ast_f(context, node_type, token);
    add_ast(node, expression);
    return node;
}

static ASTNode *levelc_forever_node(Context *context, Token *token) {
    return ast_f(context, REPEAT, token);
}

static ASTNode *levelc_forever_bad_tail(Context *context,
                                        Token *forever_token,
                                        Token *bad_token,
                                        ASTNode *tail_expression) {
    ASTNode *node;
    char *found;

    node = ast_f(context, REPEAT, forever_token);
    found = levelc_diag_token_text(bad_token);
    add_ast(node, rxcp_levelc_ast_error_insert2(context, "25.16", bad_token,
                                                "keywords", "WHILE UNTIL",
                                                "token", found));
    if (tail_expression) add_ast(node, tail_expression);
    free(found);
    return node;
}

static ASTNode *levelc_do_bad_keyword(Context *context,
                                      Token *bad_token,
                                      ASTNode *tail_expression) {
    ASTNode *node;

    node = rxcp_levelc_ast_error_token(context, "27.1", bad_token);
    if (tail_expression) add_ast(node, tail_expression);
    return node;
}
}

%token CTK_UNKNOWN CTK_BADCOMMENT CTK_EOS CTK_EOC CTK_MISSING_EXPR CTK_MISSING_RPAREN.
%token CTK_VAR_SYMBOL CTK_DO_CONTROL_SYMBOL CTK_LABEL CTK_INTEGER CTK_STRING.
%token CTK_EQUAL CTK_ADDRESS CTK_ARG CTK_CALL CTK_DROP CTK_EXIT CTK_INTERPRET CTK_NOP CTK_NUMERIC.
%token CTK_OPTIONS CTK_PROCEDURE CTK_PULL CTK_PUSH CTK_QUEUE CTK_RETURN CTK_SAY CTK_SIGNAL CTK_TRACE.
%token CTK_IF CTK_THEN CTK_ELSE CTK_SELECT CTK_WHEN CTK_OTHERWISE CTK_DO CTK_END.
%token CTK_TO CTK_BY CTK_FOR CTK_WHILE CTK_UNTIL CTK_FOREVER CTK_LEAVE CTK_ITERATE.
%token CTK_COMMA CTK_OPEN_BRACKET CTK_CLOSE_BRACKET.
%token CTK_PLUS CTK_MINUS CTK_HIGH_PRIORITY_MINUS CTK_NOT.
%token CTK_CONCAT CTK_MULT CTK_DIV CTK_IDIV CTK_MOD CTK_POWER.
%token CTK_NEQ CTK_GT CTK_LT CTK_GTE CTK_LTE.
%token CTK_S_EQ CTK_S_NEQ CTK_S_GT CTK_S_LT CTK_S_GTE CTK_S_LTE.
%token CTK_AND CTK_OR CTK_XOR.

%nonassoc CTK_IF.
%nonassoc CTK_SELECT.
%nonassoc CTK_WHEN.
%nonassoc CTK_OTHERWISE.
%nonassoc CTK_DO.
%nonassoc CTK_ELSE.

%type bad_expression_start {Token*}

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

top_instruction(I) ::= CTK_END(T) CTK_EOC.
{
    I = rxcp_levelc_ast_error(context, "10.1", T);
}

top_instruction(I) ::= CTK_END(T) CTK_VAR_SYMBOL CTK_EOC.
{
    I = rxcp_levelc_ast_error(context, "10.1", T);
}

instruction(I) ::= simple_instruction(S).
{
    I = S;
}

instruction(I) ::= if_instruction(F).
{
    I = F;
}

instruction(I) ::= select_instruction(S).
{
    I = S;
}

instruction(I) ::= do_instruction(D).
{
    I = D;
}

instruction(I) ::= unexpected_then(T).
{
    I = T;
}

instruction(I) ::= unexpected_else(E).
{
    I = E;
}

instruction(I) ::= unexpected_when(W).
{
    I = W;
}

instruction(I) ::= unexpected_otherwise(O).
{
    I = O;
}

instruction(I) ::= CTK_TO(T).
{
    I = rxcp_levelc_ast_error_token(context, "27.1", T);
}

instruction(I) ::= CTK_BY(T).
{
    I = rxcp_levelc_ast_error_token(context, "27.1", T);
}

instruction(I) ::= CTK_FOR(T).
{
    I = rxcp_levelc_ast_error_token(context, "27.1", T);
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

simple_instruction(S) ::= address_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= arg_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= call_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= drop_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= exit_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= interpret_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= nop_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= numeric_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= options_instruction(O).
{
    S = O;
}

simple_instruction(S) ::= procedure_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= pull_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= push_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= queue_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= return_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= signal_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= trace_instruction(I).
{
    S = I;
}

simple_instruction(S) ::= assignment(A).
{
    S = A;
}

simple_instruction(S) ::= leave_instruction(L).
{
    S = L;
}

simple_instruction(S) ::= iterate_instruction(I).
{
    S = I;
}

address_instruction(I) ::= CTK_ADDRESS(T) simple_tail(L).
{
    I = ast_f(context, LEVELC_ADDRESS, T);
    if (L) add_ast(I, L);
}

arg_instruction(I) ::= CTK_ARG(T) simple_tail(L).
{
    I = ast_f(context, LEVELC_ARG, T);
    if (L) add_ast(I, L);
}

call_instruction(I) ::= CTK_CALL(T) simple_tail(L).
{
    I = ast_f(context, CALL, T);
    if (L) add_ast(I, L);
}

drop_instruction(I) ::= CTK_DROP(T) simple_tail(L).
{
    I = ast_f(context, LEVELC_DROP, T);
    if (L) add_ast(I, L);
}

exit_instruction(I) ::= CTK_EXIT(T) expression(E).
{
    I = ast_f(context, EXIT, T);
    add_ast(I, E);
}

exit_instruction(I) ::= CTK_EXIT(T).
{
    I = ast_f(context, EXIT, T);
}

interpret_instruction(I) ::= CTK_INTERPRET(T) expression(E).
{
    I = ast_f(context, LEVELC_INTERPRET, T);
    add_ast(I, E);
}

interpret_instruction(I) ::= CTK_INTERPRET(T).
{
    I = ast_f(context, LEVELC_INTERPRET, T);
    add_ast(I, rxcp_levelc_ast_error(context, "35.1", T));
}

nop_instruction(I) ::= CTK_NOP(T).
{
    I = ast_f(context, NOP, T);
}

numeric_instruction(I) ::= CTK_NUMERIC(T) simple_tail(L).
{
    I = ast_f(context, LEVELC_NUMERIC, T);
    if (L) add_ast(I, L);
}

procedure_instruction(I) ::= CTK_PROCEDURE(T) simple_tail(L).
{
    I = ast_f(context, LEVELC_PROCEDURE, T);
    if (L) add_ast(I, L);
}

pull_instruction(I) ::= CTK_PULL(T) simple_tail(L).
{
    I = ast_f(context, PULL, T);
    if (L) add_ast(I, L);
}

push_instruction(I) ::= CTK_PUSH(T) expression(E).
{
    I = ast_f(context, LEVELC_PUSH, T);
    add_ast(I, E);
}

push_instruction(I) ::= CTK_PUSH(T).
{
    I = ast_f(context, LEVELC_PUSH, T);
}

queue_instruction(I) ::= CTK_QUEUE(T) expression(E).
{
    I = ast_f(context, LEVELC_QUEUE, T);
    add_ast(I, E);
}

queue_instruction(I) ::= CTK_QUEUE(T).
{
    I = ast_f(context, LEVELC_QUEUE, T);
}

return_instruction(I) ::= CTK_RETURN(T) expression(E).
{
    I = ast_f(context, RETURN, T);
    add_ast(I, E);
}

return_instruction(I) ::= CTK_RETURN(T).
{
    I = ast_f(context, RETURN, T);
}

signal_instruction(I) ::= CTK_SIGNAL(T) simple_tail(L).
{
    I = ast_f(context, LEVELC_SIGNAL, T);
    if (L) add_ast(I, L);
}

trace_instruction(I) ::= CTK_TRACE(T) simple_tail(L).
{
    I = ast_f(context, LEVELC_TRACE, T);
    if (L) add_ast(I, L);
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

say_instruction(S) ::= CTK_SAY(T) CTK_CLOSE_BRACKET(R).
{
    S = ast_f(context, SAY, T);
    add_ast(S, rxcp_levelc_ast_error(context, "37.2", R));
}

options_instruction(O) ::= CTK_OPTIONS(T) option_tail(L).
{
    O = ast_f(context, REXX_OPTIONS, T);
    if (L) add_ast(O, L);
}

option_tail(L) ::= option_tail(L0) CTK_VAR_SYMBOL(S).
{
    L = L0 ? L0 : ast_ft(context, OPTIONS);
    add_ast(L, ast_f(context, LITERAL, S));
}

option_tail(L) ::= .
{
    L = 0;
}

simple_tail(L) ::= simple_tail(L0) simple_tail_atom(A).
{
    L = L0 ? L0 : ast_ft(context, ARGS);
    add_ast(L, A);
}

simple_tail(L) ::= .
{
    L = 0;
}

simple_tail_atom(A) ::= CTK_VAR_SYMBOL(T).
{
    A = ast_f(context, LITERAL, T);
}

simple_tail_atom(A) ::= CTK_INTEGER(T).
{
    A = ast_f(context, INTEGER, T);
}

simple_tail_atom(A) ::= CTK_STRING(T).
{
    A = ast_f(context, STRING, T);
}

simple_tail_atom(A) ::= CTK_COMMA(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_OPEN_BRACKET(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_CLOSE_BRACKET(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_EQUAL(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_PLUS(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_MINUS(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_HIGH_PRIORITY_MINUS(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_NOT(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_CONCAT(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_MULT(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_DIV(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_IDIV(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_MOD(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_POWER(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_NEQ(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_GT(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_LT(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_GTE(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_LTE(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_S_EQ(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_S_NEQ(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_S_GT(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_S_LT(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_S_GTE(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_S_LTE(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_AND(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_OR(T).
{
    A = ast_f(context, TOKEN, T);
}

simple_tail_atom(A) ::= CTK_XOR(T).
{
    A = ast_f(context, TOKEN, T);
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

then_instruction(I) ::= end_clause(E).
{
    I = rxcp_levelc_ast_error(context, "10.5", levelc_end_clause_token(E));
}

select_instruction(S) ::= CTK_SELECT(T) CTK_EOC select_body(B) end_clause.
{
    S = ast_f(context, SELECT, T);
    add_ast(S, B);
}

select_instruction(S) ::= CTK_SELECT(T) CTK_EOC end_clause(E).
{
    S = levelc_select_missing_when(context, T, levelc_end_clause_token(E));
}

select_instruction(S) ::= CTK_SELECT(T) CTK_EOC CTK_OTHERWISE(O) CTK_EOC select_inner_list(L) end_clause.
{
    ASTNode *other;

    S = levelc_select_missing_when(context, T, O);
    other = ast_f(context, OTHERWISE, O);
    add_ast(other, L);
    add_ast(S, other);
}

select_instruction(S) ::= CTK_SELECT(T) CTK_EOC CTK_OTHERWISE(O) recovery_instruction(I) CTK_EOC select_inner_list(L) end_clause.
{
    ASTNode *other;

    S = levelc_select_missing_when(context, T, O);
    other = ast_f(context, OTHERWISE, O);
    add_ast(other, I);
    add_ast(other, L);
    add_ast(S, other);
}

select_body(B) ::= when_list(W).
{
    B = W;
}

select_body(B) ::= when_list(W) otherwise_clause(O).
{
    B = W;
    add_ast(B, O);
}

when_list(L) ::= when_clause(W).
{
    L = ast_ft(context, INSTRUCTIONS);
    add_ast(L, W);
}

when_list(L) ::= when_list(L0) when_clause(W).
{
    L = L0;
    add_ast(L, W);
}

when_clause(W) ::= CTK_WHEN(T) expression(C) CTK_THEN then_instruction(Then) CTK_EOC.
{
    W = ast_f(context, WHEN, T);
    add_ast(W, C);
    add_ast(W, Then);
}

when_clause(W) ::= CTK_WHEN(T) expression(C) CTK_THEN CTK_EOC.
{
    W = ast_f(context, WHEN, T);
    add_ast(W, C);
    add_ast(W, rxcp_levelc_ast_error(context, "14.3", T));
}

when_clause(W) ::= CTK_WHEN(T) expression(C) CTK_EOC.
{
    W = levelc_missing_then(context, "18.2", T, C);
}

otherwise_clause(O) ::= CTK_OTHERWISE(T) CTK_EOC select_inner_list(L).
{
    O = ast_f(context, OTHERWISE, T);
    add_ast(O, L);
}

otherwise_clause(O) ::= CTK_OTHERWISE(T) recovery_instruction(I) CTK_EOC select_inner_list(L).
{
    O = ast_f(context, OTHERWISE, T);
    add_ast(O, I);
    add_ast(O, L);
}

select_inner_list(L) ::= .
{
    L = ast_ft(context, INSTRUCTIONS);
}

select_inner_list(L) ::= select_inner_list(L0) select_inner_top(I).
{
    L = L0;
    if (I) add_ast(L, I);
}

select_inner_top(I) ::= select_inner_instruction(S) CTK_EOC.
{
    I = S;
}

select_inner_top(I) ::= CTK_LABEL(T).
{
    I = ast_f(context, LABEL, T);
}

select_inner_top(I) ::= CTK_EOC.
{
    I = 0;
}

select_inner_instruction(I) ::= simple_instruction(S).
{
    I = S;
}

select_inner_instruction(I) ::= if_instruction(F).
{
    I = F;
}

select_inner_instruction(I) ::= select_instruction(S).
{
    I = S;
}

select_inner_instruction(I) ::= do_instruction(D).
{
    I = D;
}

select_inner_instruction(I) ::= unexpected_then(T).
{
    I = T;
}

select_inner_instruction(I) ::= unexpected_else(E).
{
    I = E;
}

select_inner_instruction(I) ::= unexpected_when(W).
{
    I = W;
}

select_inner_instruction(I) ::= unexpected_otherwise(O).
{
    I = O;
}

select_inner_instruction(E) ::= CTK_BADCOMMENT(T).
{
    E = rxcp_levelc_ast_error(context, "6.1", T);
}

select_inner_instruction(E) ::= CTK_UNKNOWN(T).
{
    E = rxcp_levelc_ast_error_token(context, "13.1", T);
}

else_clause(I) ::= CTK_ELSE instruction(S).
{
    I = S;
}

else_clause(I) ::= CTK_ELSE end_clause(E).
{
    I = rxcp_levelc_ast_error(context, "10.6", levelc_end_clause_token(E));
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

recovery_instruction(I) ::= do_instruction(D).
{
    I = D;
}

do_instruction(D) ::= CTK_DO(T) do_header(H) CTK_EOC select_inner_list(L) end_clause.
{
    D = ast_f(context, DO, T);
    if (H) add_ast(D, H);
    add_ast(D, L);
}

do_header(H) ::= .
{
    H = 0;
}

do_header(H) ::= do_repetition(R).
{
    H = R;
}

do_header(H) ::= do_condition(C).
{
    H = C;
}

do_header(H) ::= do_invalid_keyword(K).
{
    H = levelc_do_bad_keyword(context, K ? K->token : 0, 0);
}

do_header(H) ::= do_invalid_keyword(K) expression(E).
{
    H = levelc_do_bad_keyword(context, K ? K->token : 0, E);
}

do_header(H) ::= do_repetition(R) do_condition(C).
{
    H = R;
    add_sbtr(H, C);
}

do_header(H) ::= CTK_FOREVER(T).
{
    H = levelc_forever_node(context, T);
}

do_header(H) ::= CTK_FOREVER(T) do_condition(C).
{
    H = levelc_forever_node(context, T);
    add_sbtr(H, C);
}

do_header(H) ::= CTK_FOREVER(T) do_forever_invalid(B).
{
    H = levelc_forever_bad_tail(context, T, B ? B->token : T, 0);
}

do_header(H) ::= CTK_FOREVER(T) do_forever_invalid(B) expression(E).
{
    H = levelc_forever_bad_tail(context, T, B ? B->token : T, E);
}

do_repetition(R) ::= expression(E).
{
    R = levelc_repeat_count(context, E);
}

do_repetition(R) ::= CTK_DO_CONTROL_SYMBOL(V) CTK_EQUAL(T) expression(E).
{
    R = levelc_repeat_assignment(context, T, V, E, 0);
}

do_repetition(R) ::= CTK_DO_CONTROL_SYMBOL(V) CTK_EQUAL(T) expression(E) do_control_list(L).
{
    R = levelc_repeat_assignment(context, T, V, E, L);
}

do_control_list(L) ::= do_control_item(I).
{
    L = I;
}

do_control_list(L) ::= do_control_list(L0) do_control_item(I).
{
    L = L0;
    add_sbtr(L, I);
}

do_control_item(I) ::= CTK_TO(T) expression(E).
{
    I = levelc_condition_node(context, TO, T, E);
}

do_control_item(I) ::= CTK_BY(T) expression(E).
{
    I = levelc_condition_node(context, BY, T, E);
}

do_control_item(I) ::= CTK_FOR(T) expression(E).
{
    I = levelc_condition_node(context, FOR, T, E);
}

do_condition(C) ::= CTK_WHILE(T) expression(E).
{
    C = levelc_condition_node(context, WHILE, T, E);
}

do_condition(C) ::= CTK_UNTIL(T) expression(E).
{
    C = levelc_condition_node(context, UNTIL, T, E);
}

do_forever_invalid(B) ::= CTK_TO(T).
{
    B = ast_f(context, TOKEN, T);
}

do_forever_invalid(B) ::= CTK_BY(T).
{
    B = ast_f(context, TOKEN, T);
}

do_forever_invalid(B) ::= CTK_FOR(T).
{
    B = ast_f(context, TOKEN, T);
}

do_forever_invalid(B) ::= CTK_VAR_SYMBOL(T).
{
    B = ast_f(context, TOKEN, T);
}

do_invalid_keyword(K) ::= CTK_TO(T).
{
    K = ast_f(context, TOKEN, T);
}

do_invalid_keyword(K) ::= CTK_BY(T).
{
    K = ast_f(context, TOKEN, T);
}

do_invalid_keyword(K) ::= CTK_FOR(T).
{
    K = ast_f(context, TOKEN, T);
}

end_clause(E) ::= CTK_END(T).
{
    E = levelc_end_clause_node(context, T, 0);
}

end_clause(E) ::= CTK_END(T) CTK_VAR_SYMBOL(S).
{
    E = levelc_end_clause_node(context, T, S);
}

leave_instruction(L) ::= CTK_LEAVE(T).
{
    L = ast_f(context, LEAVE, T);
}

leave_instruction(L) ::= CTK_LEAVE(T) CTK_VAR_SYMBOL(S).
{
    L = ast_f(context, LEAVE, T);
    add_ast(L, ast_f(context, VAR_SYMBOL, S));
}

iterate_instruction(I) ::= CTK_ITERATE(T).
{
    I = ast_f(context, ITERATE, T);
}

iterate_instruction(I) ::= CTK_ITERATE(T) CTK_VAR_SYMBOL(S).
{
    I = ast_f(context, ITERATE, T);
    add_ast(I, ast_f(context, VAR_SYMBOL, S));
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

unexpected_when(E) ::= CTK_WHEN(T) expression(C) CTK_THEN recovery_instruction(S).
{
    ASTNode *when_node;

    when_node = ast_f(context, WHEN, T);
    add_ast(when_node, C);
    add_ast(when_node, S);
    E = levelc_unexpected_keyword(context, "9.1", T, when_node);
}

unexpected_when(E) ::= CTK_WHEN(T).
{
    E = rxcp_levelc_ast_error(context, "9.1", T);
}

unexpected_otherwise(E) ::= CTK_OTHERWISE(T) recovery_instruction(S).
{
    E = levelc_unexpected_keyword(context, "9.2", T, S);
}

unexpected_otherwise(E) ::= CTK_OTHERWISE(T).
{
    E = rxcp_levelc_ast_error(context, "9.2", T);
}

primary_expr(T) ::= CTK_VAR_SYMBOL(S).
{
    T = ast_f(context, VAR_SYMBOL, S);
}

primary_expr(T) ::= CTK_INTEGER(S).
{
    T = ast_f(context, INTEGER, S);
}

primary_expr(T) ::= CTK_STRING(S).
{
    T = ast_fstr(context, S);
}

primary_expr(T) ::= CTK_OPEN_BRACKET expression(E) CTK_CLOSE_BRACKET.
{
    T = E;
}

primary_expr(T) ::= CTK_OPEN_BRACKET(O) expression(E) missing_close_bracket.
{
    T = levelc_unmatched_open_bracket(context, O, E);
}

primary_expr(T) ::= CTK_OPEN_BRACKET(O) missing_close_bracket.
{
    T = levelc_unmatched_open_bracket(context, O, 0);
}

primary_expr(T) ::= CTK_COMMA(S).
{
    T = rxcp_levelc_ast_error(context, "37.1", S);
}

primary_expr(T) ::= bad_expression_start(S).
{
    T = levelc_bad_expression_start(context, S, 0);
}

prefix_expr(E) ::= primary_expr(P).
{
    E = P;
}

prefix_expr(E) ::= CTK_NOT(T) prefix_expr(P).
{
    E = ast_f(context, OP_NOT, T);
    add_ast(E, P);
}

prefix_expr(E) ::= CTK_NOT(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_NOT, T, 0);
}

prefix_expr(E) ::= CTK_PLUS(T) prefix_expr(P).
{
    E = ast_f(context, OP_PLUS, T);
    add_ast(E, P);
}

prefix_expr(E) ::= CTK_PLUS(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_PLUS, T, 0);
}

prefix_expr(E) ::= CTK_HIGH_PRIORITY_MINUS(T) prefix_expr(P).
{
    E = ast_f(context, OP_NEG, T);
    add_ast(E, P);
}

prefix_expr(E) ::= CTK_HIGH_PRIORITY_MINUS(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_NEG, T, 0);
}

power_expr(E) ::= power_expr(L) CTK_POWER(T) prefix_expr(R).
{
    E = ast_f(context, OP_POWER, T);
    add_ast(E, L);
    add_ast(E, R);
}

power_expr(E) ::= power_expr(L) CTK_POWER(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_POWER, T, L);
}

power_expr(E) ::= prefix_expr(P).
{
    E = P;
}

low_prefix_expr(E) ::= power_expr(P).
{
    E = P;
}

low_prefix_expr(E) ::= CTK_MINUS(T) power_expr(P).
{
    E = ast_f(context, OP_NEG, T);
    add_ast(E, P);
}

low_prefix_expr(E) ::= CTK_MINUS(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_NEG, T, 0);
}

multiplication(E) ::= low_prefix_expr(P).
{
    E = P;
}

multiplication(E) ::= multiplication(L) CTK_MULT(T) low_prefix_expr(R).
{
    E = ast_f(context, OP_MULT, T);
    add_ast(E, L);
    add_ast(E, R);
}

multiplication(E) ::= multiplication(L) CTK_MULT(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_MULT, T, L);
}

multiplication(E) ::= multiplication(L) CTK_DIV(T) low_prefix_expr(R).
{
    E = ast_f(context, OP_DIV, T);
    add_ast(E, L);
    add_ast(E, R);
}

multiplication(E) ::= multiplication(L) CTK_DIV(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_DIV, T, L);
}

multiplication(E) ::= multiplication(L) CTK_IDIV(T) low_prefix_expr(R).
{
    E = ast_f(context, OP_IDIV, T);
    add_ast(E, L);
    add_ast(E, R);
}

multiplication(E) ::= multiplication(L) CTK_IDIV(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_IDIV, T, L);
}

multiplication(E) ::= multiplication(L) CTK_MOD(T) low_prefix_expr(R).
{
    E = ast_f(context, OP_MOD, T);
    add_ast(E, L);
    add_ast(E, R);
}

multiplication(E) ::= multiplication(L) CTK_MOD(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_MOD, T, L);
}

addition(E) ::= multiplication(P).
{
    E = P;
}

addition(E) ::= addition(L) CTK_PLUS(T) multiplication(R).
{
    E = ast_f(context, OP_ADD, T);
    add_ast(E, L);
    add_ast(E, R);
}

addition(E) ::= addition(L) CTK_PLUS(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_ADD, T, L);
}

addition(E) ::= addition(L) CTK_MINUS(T) multiplication(R).
{
    E = ast_f(context, OP_MINUS, T);
    add_ast(E, L);
    add_ast(E, R);
}

addition(E) ::= addition(L) CTK_MINUS(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_MINUS, T, L);
}

addition(E) ::= addition(L) CTK_HIGH_PRIORITY_MINUS(T) multiplication(R).
{
    E = ast_f(context, OP_MINUS, T);
    add_ast(E, L);
    add_ast(E, R);
}

addition(E) ::= addition(L) CTK_HIGH_PRIORITY_MINUS(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_MINUS, T, L);
}

primary_expr_c(T) ::= CTK_VAR_SYMBOL(S).
{
    T = ast_f(context, VAR_SYMBOL, S);
}

primary_expr_c(T) ::= CTK_INTEGER(S).
{
    T = ast_f(context, INTEGER, S);
}

primary_expr_c(T) ::= CTK_STRING(S).
{
    T = ast_fstr(context, S);
}

primary_expr_c(T) ::= CTK_OPEN_BRACKET expression(E) CTK_CLOSE_BRACKET.
{
    T = E;
}

primary_expr_c(T) ::= CTK_COMMA(S).
{
    T = rxcp_levelc_ast_error(context, "37.1", S);
}

prefix_expr_c(E) ::= primary_expr_c(P).
{
    E = P;
}

prefix_expr_c(E) ::= CTK_NOT(T) prefix_expr_c(P).
{
    E = ast_f(context, OP_NOT, T);
    add_ast(E, P);
}

power_expr_c(E) ::= power_expr_c(L) CTK_POWER(T) prefix_expr_c(R).
{
    E = ast_f(context, OP_POWER, T);
    add_ast(E, L);
    add_ast(E, R);
}

power_expr_c(E) ::= prefix_expr_c(P).
{
    E = P;
}

multiplication_c(E) ::= power_expr_c(P).
{
    E = P;
}

multiplication_c(E) ::= multiplication_c(L) CTK_MULT(T) power_expr_c(R).
{
    E = ast_f(context, OP_MULT, T);
    add_ast(E, L);
    add_ast(E, R);
}

multiplication_c(E) ::= multiplication_c(L) CTK_DIV(T) power_expr_c(R).
{
    E = ast_f(context, OP_DIV, T);
    add_ast(E, L);
    add_ast(E, R);
}

multiplication_c(E) ::= multiplication_c(L) CTK_IDIV(T) power_expr_c(R).
{
    E = ast_f(context, OP_IDIV, T);
    add_ast(E, L);
    add_ast(E, R);
}

multiplication_c(E) ::= multiplication_c(L) CTK_MOD(T) power_expr_c(R).
{
    E = ast_f(context, OP_MOD, T);
    add_ast(E, L);
    add_ast(E, R);
}

addition_c(E) ::= multiplication_c(P).
{
    E = P;
}

addition_c(E) ::= addition_c(L) CTK_PLUS(T) multiplication_c(R).
{
    E = ast_f(context, OP_ADD, T);
    add_ast(E, L);
    add_ast(E, R);
}

addition_c(E) ::= addition_c(L) CTK_MINUS(T) multiplication_c(R).
{
    E = ast_f(context, OP_MINUS, T);
    add_ast(E, L);
    add_ast(E, R);
}

addition_c(E) ::= addition_c(L) CTK_HIGH_PRIORITY_MINUS(T) multiplication_c(R).
{
    E = ast_f(context, OP_MINUS, T);
    add_ast(E, L);
    add_ast(E, R);
}

concat_expr(E) ::= addition(P).
{
    E = P;
}

concat_expr(E) ::= concat_expr(L) CTK_CONCAT(T) addition(R).
{
    E = ast_f(context, OP_CONCAT, T);
    add_ast(E, L);
    add_ast(E, R);
}

concat_expr(E) ::= concat_expr(L) CTK_CONCAT(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_CONCAT, T, L);
}

concat_expr(E) ::= concat_expr(L) addition_c(R).
{
    E = ast_ft(context, OP_SCONCAT);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= concat_expr(P).
{
    E = P;
}

comparison(E) ::= comparison(L) CTK_EQUAL(T) concat_expr(R).
{
    E = ast_f(context, OP_COMPARE_EQUAL, T);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= comparison(L) CTK_EQUAL(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_COMPARE_EQUAL, T, L);
}

comparison(E) ::= comparison(L) CTK_NEQ(T) concat_expr(R).
{
    E = ast_f(context, OP_COMPARE_NEQ, T);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= comparison(L) CTK_NEQ(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_COMPARE_NEQ, T, L);
}

comparison(E) ::= comparison(L) CTK_GT(T) concat_expr(R).
{
    E = ast_f(context, OP_COMPARE_GT, T);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= comparison(L) CTK_GT(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_COMPARE_GT, T, L);
}

comparison(E) ::= comparison(L) CTK_LT(T) concat_expr(R).
{
    E = ast_f(context, OP_COMPARE_LT, T);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= comparison(L) CTK_LT(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_COMPARE_LT, T, L);
}

comparison(E) ::= comparison(L) CTK_GTE(T) concat_expr(R).
{
    E = ast_f(context, OP_COMPARE_GTE, T);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= comparison(L) CTK_GTE(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_COMPARE_GTE, T, L);
}

comparison(E) ::= comparison(L) CTK_LTE(T) concat_expr(R).
{
    E = ast_f(context, OP_COMPARE_LTE, T);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= comparison(L) CTK_LTE(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_COMPARE_LTE, T, L);
}

comparison(E) ::= comparison(L) CTK_S_EQ(T) concat_expr(R).
{
    E = ast_f(context, OP_COMPARE_S_EQ, T);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= comparison(L) CTK_S_EQ(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_COMPARE_S_EQ, T, L);
}

comparison(E) ::= comparison(L) CTK_S_NEQ(T) concat_expr(R).
{
    E = ast_f(context, OP_COMPARE_S_NEQ, T);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= comparison(L) CTK_S_NEQ(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_COMPARE_S_NEQ, T, L);
}

comparison(E) ::= comparison(L) CTK_S_GT(T) concat_expr(R).
{
    E = ast_f(context, OP_COMPARE_S_GT, T);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= comparison(L) CTK_S_GT(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_COMPARE_S_GT, T, L);
}

comparison(E) ::= comparison(L) CTK_S_LT(T) concat_expr(R).
{
    E = ast_f(context, OP_COMPARE_S_LT, T);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= comparison(L) CTK_S_LT(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_COMPARE_S_LT, T, L);
}

comparison(E) ::= comparison(L) CTK_S_GTE(T) concat_expr(R).
{
    E = ast_f(context, OP_COMPARE_S_GTE, T);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= comparison(L) CTK_S_GTE(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_COMPARE_S_GTE, T, L);
}

comparison(E) ::= comparison(L) CTK_S_LTE(T) concat_expr(R).
{
    E = ast_f(context, OP_COMPARE_S_LTE, T);
    add_ast(E, L);
    add_ast(E, R);
}

comparison(E) ::= comparison(L) CTK_S_LTE(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_COMPARE_S_LTE, T, L);
}

missing_expression_rhs ::= CTK_MISSING_EXPR.

and_expr(E) ::= comparison(P).
{
    E = P;
}

and_expr(E) ::= and_expr(L) CTK_AND(T) comparison(R).
{
    E = ast_f(context, OP_AND, T);
    add_ast(E, L);
    add_ast(E, R);
}

and_expr(E) ::= and_expr(L) CTK_AND(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_AND, T, L);
}

or_expr(E) ::= and_expr(P).
{
    E = P;
}

or_expr(E) ::= or_expr(L) CTK_OR(T) and_expr(R).
{
    E = ast_f(context, OP_OR, T);
    add_ast(E, L);
    add_ast(E, R);
}

or_expr(E) ::= or_expr(L) CTK_OR(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_OR, T, L);
}

or_expr(E) ::= or_expr(L) CTK_XOR(T) and_expr(R).
{
    E = ast_f(context, OP_XOR, T);
    add_ast(E, L);
    add_ast(E, R);
}

or_expr(E) ::= or_expr(L) CTK_XOR(T) missing_expression_rhs.
{
    E = levelc_missing_expression_rhs(context, OP_XOR, T, L);
}

expression(E) ::= or_expr(P).
{
    E = P;
}

missing_close_bracket ::= CTK_MISSING_RPAREN.

bad_expression_start(T) ::= CTK_CONCAT(S). { T = S; }
bad_expression_start(T) ::= CTK_MULT(S). { T = S; }
bad_expression_start(T) ::= CTK_DIV(S). { T = S; }
bad_expression_start(T) ::= CTK_IDIV(S). { T = S; }
bad_expression_start(T) ::= CTK_MOD(S). { T = S; }
bad_expression_start(T) ::= CTK_POWER(S). { T = S; }
bad_expression_start(T) ::= CTK_EQUAL(S). { T = S; }
bad_expression_start(T) ::= CTK_NEQ(S). { T = S; }
bad_expression_start(T) ::= CTK_GT(S). { T = S; }
bad_expression_start(T) ::= CTK_LT(S). { T = S; }
bad_expression_start(T) ::= CTK_GTE(S). { T = S; }
bad_expression_start(T) ::= CTK_LTE(S). { T = S; }
bad_expression_start(T) ::= CTK_S_EQ(S). { T = S; }
bad_expression_start(T) ::= CTK_S_NEQ(S). { T = S; }
bad_expression_start(T) ::= CTK_S_GT(S). { T = S; }
bad_expression_start(T) ::= CTK_S_LT(S). { T = S; }
bad_expression_start(T) ::= CTK_S_GTE(S). { T = S; }
bad_expression_start(T) ::= CTK_S_LTE(S). { T = S; }
bad_expression_start(T) ::= CTK_AND(S). { T = S; }
bad_expression_start(T) ::= CTK_OR(S). { T = S; }
bad_expression_start(T) ::= CTK_XOR(S). { T = S; }
