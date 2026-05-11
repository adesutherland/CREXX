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

%token CTK_UNKNOWN CTK_BADCOMMENT CTK_EOS CTK_EOC CTK_VAR_SYMBOL CTK_LABEL CTK_INTEGER CTK_STRING.
%token CTK_EQUAL CTK_SAY CTK_IF CTK_THEN CTK_ELSE CTK_SELECT CTK_WHEN CTK_OTHERWISE CTK_DO CTK_END.
%token CTK_TO CTK_BY CTK_FOR CTK_WHILE CTK_UNTIL CTK_FOREVER CTK_LEAVE CTK_ITERATE.

%nonassoc CTK_IF.
%nonassoc CTK_SELECT.
%nonassoc CTK_WHEN.
%nonassoc CTK_OTHERWISE.
%nonassoc CTK_DO.
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

do_repetition(R) ::= CTK_VAR_SYMBOL(V) CTK_EQUAL(T) expression(E).
{
    R = levelc_repeat_assignment(context, T, V, E, 0);
}

do_repetition(R) ::= CTK_VAR_SYMBOL(V) CTK_EQUAL(T) expression(E) do_control_list(L).
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
