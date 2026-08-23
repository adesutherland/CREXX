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

static int token_text_equals_ci(Token *token, const char *text) {
    size_t i;
    size_t length;

    if (!token || !token->token_string || !text) return 0;
    length = strlen(text);
    if (token->length != length) return 0;
    for (i = 0; i < length; i++) {
        if (tolower((unsigned char) token->token_string[i]) !=
            tolower((unsigned char) text[i])) {
            return 0;
        }
    }
    return 1;
}

static int named_operator_equals(Token *token, const char *text) {
    size_t i;
    size_t length;

    if (!token || !token->token_string || !text) return 0;
    length = strlen(text);
    if (token->length != length + 2) return 0;
    if (token->token_string[0] != '<' ||
        token->token_string[token->length - 1] != '>') {
        return 0;
    }
    for (i = 0; i < length; i++) {
        if (tolower((unsigned char) token->token_string[i + 1]) !=
            tolower((unsigned char) text[i])) {
            return 0;
        }
    }
    return 1;
}

static ASTNode *binary_at_operator(Context *context, Token *token, ASTNode *type_node, ASTNode *base, ASTNode *offset);
static ASTNode *binary_for_operator(Context *context, Token *token, ASTNode *left, ASTNode *right);

static ASTNode *intrinsic_path_new(Context *context, Token *token) {
    ASTNode *path;
    path = ast_ft(context, INTRINSIC_PATH);
    add_ast(path, ast_f(context, VAR_SYMBOL, token));
    return path;
}

static ASTNode *intrinsic_path_add(Context *context, ASTNode *path, Token *token) {
    if (!path) path = ast_ft(context, INTRINSIC_PATH);
    add_ast(path, ast_f(context, VAR_SYMBOL, token));
    return path;
}

static ASTNode *intrinsic_types_new(Context *context, ASTNode *type_node) {
    ASTNode *types;
    types = ast_ft(context, INTRINSIC_TYPES);
    if (type_node) add_ast(types, type_node);
    return types;
}

static ASTNode *intrinsic_args_new(Context *context, ASTNode *args) {
    ASTNode *node;
    node = ast_ft(context, INTRINSIC_ARGS);
    if (args) add_ast(node, args);
    return node;
}

static ASTNode *intrinsic_new(Context *context, Token *token, ASTNode *path, ASTNode *types) {
    ASTNode *node;
    node = ast_f(context, INTRINSIC, token);
    if (path) add_ast(node, path);
    else add_ast(node, ast_ft(context, INTRINSIC_PATH));
    if (types) add_ast(node, types);
    return node;
}

static ASTNode *intrinsic_with_args(Context *context, ASTNode *intrinsic, ASTNode *args) {
    if (intrinsic) add_ast(intrinsic, intrinsic_args_new(context, args));
    return intrinsic;
}

static ASTNode *intrinsic_path_node(ASTNode *intrinsic) {
    return ast_chdn(intrinsic, 0);
}

static ASTNode *intrinsic_types_node(ASTNode *intrinsic) {
    ASTNode *child;
    child = ast_chdn(intrinsic, 1);
    return child && child->node_type == INTRINSIC_TYPES ? child : 0;
}

static ASTNode *intrinsic_args_node(ASTNode *intrinsic) {
    ASTNode *child;
    int ix;

    for (ix = 1; (child = ast_chdn(intrinsic, ix)) != 0; ix++) {
        if (child->node_type == INTRINSIC_ARGS) return child;
    }
    return 0;
}

static int intrinsic_part_equals_ci(ASTNode *part, const char *text) {
    const char *start;
    size_t length;
    size_t text_length;
    size_t i;

    if (!part || !part->node_string || !text) return 0;
    start = part->node_string;
    length = part->node_string_length;
    while (length > 0 && *start == '.') {
        start++;
        length--;
    }
    if (length > 0 && *start == ':') {
        while (length > 0 && *start == ':') {
            start++;
            length--;
        }
    }
    text_length = strlen(text);
    if (text_length > 0 && text[0] == '.') {
        text++;
        text_length--;
    }
    if (length != text_length) return 0;
    for (i = 0; i < length; i++) {
        if (tolower((unsigned char) start[i]) !=
            tolower((unsigned char) text[i])) return 0;
    }
    return 1;
}

static int intrinsic_simple_name_equals(ASTNode *intrinsic, const char *text) {
    ASTNode *path;
    path = intrinsic_path_node(intrinsic);
    return path && ast_nchd(path) == 1 && intrinsic_part_equals_ci(ast_chdn(path, 0), text);
}

static int intrinsic_has_generic_types(ASTNode *intrinsic) {
    ASTNode *types;
    types = intrinsic_types_node(intrinsic);
    return types && ast_nchd(types) > 0;
}

static int intrinsic_path_is_name_with_compact_type(ASTNode *intrinsic, const char *name) {
    ASTNode *path;
    ASTNode *first;
    ASTNode *second;

    path = intrinsic_path_node(intrinsic);
    if (!path || ast_nchd(path) != 2) return 0;
    first = ast_chdn(path, 0);
    second = ast_chdn(path, 1);
    return intrinsic_part_equals_ci(first, name) &&
           second && second->node_string_length > 2 &&
           second->node_string[0] == '.' && second->node_string[1] == '.';
}

static ASTNode *intrinsic_compact_type_node(Context *context, ASTNode *intrinsic) {
    ASTNode *path;
    ASTNode *part;
    ASTNode *type_node;

    path = intrinsic_path_node(intrinsic);
    if (!path || ast_nchd(path) < 2) return 0;
    part = ast_chdn(path, ast_nchd(path) - 1);
    if (!part || part->node_string_length <= 2 ||
        part->node_string[0] != '.' || part->node_string[1] != '.') {
        return 0;
    }
    type_node = ast_f(context, CLASS, part->token);
    type_node->node_string = part->node_string + 2;
    type_node->node_string_length = part->node_string_length - 2;
    return type_node;
}

static int intrinsic_arg_count(ASTNode *intrinsic) {
    ASTNode *args;
    ASTNode *child;

    args = intrinsic_args_node(intrinsic);
    if (!args) return -1;
    child = ast_chdn(args, 0);
    if (!child || child->node_type == NOVAL) return 0;
    return ast_nchd(args);
}

static ASTNode *intrinsic_arg(ASTNode *intrinsic, int ix) {
    ASTNode *args;
    args = intrinsic_args_node(intrinsic);
    if (!args) return 0;
    return ast_chdn(args, ix);
}

static ASTNode *intrinsic_lower_one_arg(Context *context, ASTNode *intrinsic, NodeType type, const char *invalid_code) {
    ASTNode *node;
    ASTNode *arg;

    if (intrinsic_arg_count(intrinsic) != 1) {
        return ast_err(context, invalid_code, intrinsic->token);
    }
    arg = intrinsic_arg(intrinsic, 0);
    node = ast_f(context, type, intrinsic->token);
    add_ast(node, arg);
    return node;
}

static ASTNode *intrinsic_lower_compare(Context *context, ASTNode *intrinsic) {
    ASTNode *node;
    ASTNode *type_node;
    ASTNode *args;
    ASTNode *arg;

    if (!intrinsic_path_is_name_with_compact_type(intrinsic, "compare")) {
        return 0;
    }
    if (intrinsic_arg_count(intrinsic) == -1) {
        return ast_err(context, "BINARY_MEMORY_COMPARE_ARGUMENTS", intrinsic->token);
    }

    type_node = intrinsic_compact_type_node(context, intrinsic);
    node = ast_f(context, OP_BINARY_COMPARE, intrinsic->token);
    add_ast(node, type_node);

    args = intrinsic_args_node(intrinsic);
    arg = args ? ast_chdn(args, 0) : 0;
    if (arg) add_ast(node, arg);
    return node;
}

static ASTNode *intrinsic_lower_primary(Context *context, ASTNode *intrinsic) {
    ASTNode *node;
    ASTNode *type_node;

    if (intrinsic_has_generic_types(intrinsic)) {
        return ast_err(context, "INTRINSIC_GENERIC_TYPES_UNSUPPORTED", intrinsic->token);
    }

    if (intrinsic_path_is_name_with_compact_type(intrinsic, "sizeof")) {
        if (intrinsic_arg_count(intrinsic) != -1) {
            return ast_err(context, "INVALID_SIZEOF_SYNTAX", intrinsic->token);
        }
        type_node = intrinsic_compact_type_node(context, intrinsic);
        node = ast_f(context, OP_SIZEOF, intrinsic->token);
        add_ast(node, type_node);
        return node;
    }
    node = intrinsic_lower_compare(context, intrinsic);
    if (node) return node;
    if (intrinsic_simple_name_equals(intrinsic, "compare")) {
        return ast_err(context, "BINARY_MEMORY_COMPARE_TYPE", intrinsic->token);
    }

    if (intrinsic_simple_name_equals(intrinsic, "typeof")) {
        return intrinsic_lower_one_arg(context, intrinsic, OP_TYPEOF, "INVALID_TYPEOF_SYNTAX");
    }
    if (intrinsic_simple_name_equals(intrinsic, "blen")) {
        return intrinsic_lower_one_arg(context, intrinsic, OP_BINARY_LENGTH, "INVALID_BLEN_SYNTAX");
    }
    if (intrinsic_simple_name_equals(intrinsic, "refvalid")) {
        return intrinsic_lower_one_arg(context, intrinsic, OP_REFVALID, "INVALID_REFVALID_SYNTAX");
    }
    if (intrinsic_simple_name_equals(intrinsic, "initialized")) {
        return intrinsic_lower_one_arg(context, intrinsic, OP_INITIALIZED, "INVALID_INITIALIZED_SYNTAX");
    }
    if (intrinsic_simple_name_equals(intrinsic, "argexists")) {
        return intrinsic_lower_one_arg(context, intrinsic, OP_ARG_IX_EXISTS, "INVALID_ARGEXISTS_SYNTAX");
    }
    return ast_err(context, "UNKNOWN_NAMED_OPERATOR", intrinsic->token);
}

static ASTNode *intrinsic_lower_prefix(Context *context, ASTNode *intrinsic, ASTNode *child) {
    ASTNode *node;
    ASTNode *type_node;
    ASTNode *args;
    ASTNode *offset;
    ASTNode *length;
    ASTNode *extra;
    ASTNode *at_node;

    if (intrinsic_has_generic_types(intrinsic)) {
        node = ast_err(context, "INTRINSIC_GENERIC_TYPES_UNSUPPORTED", intrinsic->token);
        add_ast(node, child);
        return node;
    }

    if (intrinsic_path_is_name_with_compact_type(intrinsic, "at")) {
        type_node = intrinsic_compact_type_node(context, intrinsic);
        args = intrinsic_args_node(intrinsic);
        offset = 0;
        length = 0;
        extra = 0;
        if (args && ast_chdn(args, 0)) {
            offset = ast_chdn(args, 0);
            if (offset->node_type != NOVAL) {
                length = offset->sibling;
                offset->sibling = 0;
            } else {
                length = offset->sibling;
                offset->sibling = 0;
                offset = 0;
            }
        }
        if (length) {
            extra = length->sibling;
            length->sibling = 0;
        }
        if (extra) mknd_err1(extra, "UNEXPECTED_ARGUMENT", "position", "3");

        at_node = binary_at_operator(context, intrinsic->token, type_node, child, offset);
        if (length && length->node_type != NOVAL) {
            node = binary_for_operator(context, intrinsic->token, at_node, length);
        } else {
            node = at_node;
            if (length) mknd_err2(length, "ARGUMENT_REQUIRED", "position", "2", "name", "length");
        }
        if (extra) add_ast(node, extra);
        return node;
    }

    if (intrinsic_path_is_name_with_compact_type(intrinsic, "packed")) {
        type_node = intrinsic_compact_type_node(context, intrinsic);
        args = intrinsic_args_node(intrinsic);
        offset = args ? ast_chdn(args, 0) : 0;
        extra = 0;
        if (offset) {
            extra = offset->sibling;
            offset->sibling = 0;
            if (offset->node_type == NOVAL) offset = 0;
        }
        if (extra) {
            ASTNode *unexpected = extra->sibling;
            extra->sibling = 0;
            mknd_err1(extra, "UNEXPECTED_ARGUMENT", "position", "2");
            while (unexpected) {
                ASTNode *next = unexpected->sibling;
                unexpected->sibling = 0;
                add_ast(extra, unexpected);
                unexpected = next;
            }
        }

        node = ast_f(context, OP_PACKED_AT, intrinsic->token);
        add_ast(node, type_node);
        add_ast(node, child);
        if (offset) add_ast(node, offset);
        else mknd_err(node, "PACKED_INDEX_REQUIRED");
        if (extra) add_ast(node, extra);
        return node;
    }

    return ast_err(context, "UNKNOWN_NAMED_OPERATOR", intrinsic->token);
}

static ASTNode *named_prefix_operator(Context *context, Token *token, ASTNode *child) {
    ASTNode *node;

    if (named_operator_equals(token, "not")) {
        node = ast_f(context, OP_BIT_NOT, token);
    } else {
        node = ast_err(context, "UNKNOWN_NAMED_OPERATOR", token);
    }
    add_ast(node, child);
    return node;
}

static ASTNode *binary_at_operator(Context *context, Token *token, ASTNode *type_node, ASTNode *base, ASTNode *offset) {
    ASTNode *node = ast_f(context, OP_BINARY_AT, token);
    add_ast(node, type_node);
    add_ast(node, base);
    if (offset) add_ast(node, offset);
    return node;
}

static ASTNode *binary_for_operator(Context *context, Token *token, ASTNode *left, ASTNode *right) {
    ASTNode *node;

    if (!left || left->node_type != OP_BINARY_AT) {
        node = ast_err(context, "BINARY_MEMORY_AT_REQUIRED", token);
        add_ast(node, left);
        add_ast(node, right);
        return node;
    }
    if (ast_nchd(left) < 3) {
        mknd_err(left, "BINARY_MEMORY_OFFSET_REQUIRED");
    }

    node = ast_f(context, OP_BINARY_FOR, token);
    add_ast(node, left);
    add_ast(node, right);
    return node;
}

static ASTNode *named_binary_operator(Context *context, Token *token, ASTNode *left, ASTNode *right) {
    ASTNode *node;
    ASTNode *not_node;
    NodeType type = TOKEN;

    if (named_operator_equals(token, "and")) type = OP_BIT_AND;
    else if (named_operator_equals(token, "or")) type = OP_BIT_OR;
    else if (named_operator_equals(token, "set")) type = OP_BIT_OR;
    else if (named_operator_equals(token, "xor")) type = OP_BIT_XOR;
    else if (named_operator_equals(token, "idiv")) type = OP_IDIV;
    else if (named_operator_equals(token, "mod")) type = OP_MOD;
    else if (named_operator_equals(token, "rem")) type = OP_MOD;
    else if (named_operator_equals(token, "shl")) type = OP_BIT_SHL;
    else if (named_operator_equals(token, "shr")) type = OP_BIT_SHR;
    else if (named_operator_equals(token, "has")) type = OP_FLAG_HAS;

    if (named_operator_equals(token, "clear")) {
        not_node = ast_f(context, OP_BIT_NOT, token);
        add_ast(not_node, right);
        node = ast_f(context, OP_BIT_AND, token);
        add_ast(node, left);
        add_ast(node, not_node);
        return node;
    }

    if (type == TOKEN) {
        node = ast_err(context, "UNKNOWN_NAMED_OPERATOR", token);
    } else {
        node = ast_f(context, type, token);
    }
    add_ast(node, left);
    add_ast(node, right);
    return node;
}
}

%token TK_UNKNOWN TK_BADCOMMENT TK_EOL TK_MINUSMINUS TK_DOT TK_EXIT_PRIMARY TK_EXIT_TOKEN TK_QUALIFIED_SYMBOL TK_STRING_CONTINUATION TK_INTRINSIC_LT TK_INTRINSIC_PREFIX_LT TK_INTRINSIC_NAME TK_INTRINSIC_GENERIC_OPEN TK_NAMED_OPERATOR TK_NAMED_MULT_OPERATOR TK_NAMED_SHIFT_OPERATOR TK_NAMED_AND_OPERATOR TK_NAMED_XOR_OPERATOR TK_NAMED_OR_OPERATOR TK_TASK TK_PARALLEL TK_USING.
%wildcard ANYTHING.
/* Gate F contextual words fall back to ordinary symbols outside the grammar
 * positions that name task declarations, task targets and parallel blocks. */
%fallback TK_VAR_SYMBOL TK_TASK TK_PARALLEL TK_USING.

/* Low precedence */
%left EXIT_REDUCE.
%right ANYTHING.
%right TK_TASK TK_PARALLEL TK_USING.
%left TK_EOC.
%left TK_END.
%left IMPLICIT_CONCAT.
%left TK_NAMED_OPERATOR TK_NAMED_OR_OPERATOR TK_NAMED_XOR_OPERATOR TK_NAMED_AND_OPERATOR TK_NAMED_SHIFT_OPERATOR.
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
    context->syntax_error_clause_token = context->current_clause_token;
    context->syntax_error_token = TOKEN;
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
type_def(A)              ::= TK_REFERENCE(R) type_def(T).
                             { A = ast_f(context, TYPE_REFERENCE, R); add_ast(A,T); }
type_def(A)              ::= class(S) array_def_parameters(P).
                             { A = S; if (P) add_ast(A,P); }
type_def(A)              ::= TK_CLASS_STEM(S) stem_def_parts(P).
                             { A = ast_f(context, CLASS, S); if (P) add_ast(A,P); }

intrinsic_path(P)        ::= TK_INTRINSIC_NAME(S).
                             { P = intrinsic_path_new(context, S); }
intrinsic_path(P)        ::= intrinsic_path(P0) TK_INTRINSIC_NAME(S).
                             { P = intrinsic_path_add(context, P0, S); }
intrinsic_type_list(T)   ::= class(C).
                             { T = intrinsic_types_new(context, C); }
intrinsic_type_list(T)   ::= intrinsic_type_list(T0) TK_COMMA class(C).
                             { T = T0; add_ast(T, C); }
intrinsic_generics_opt(G) ::= .
                             { G = 0; }
intrinsic_generics_opt(G) ::= TK_INTRINSIC_GENERIC_OPEN intrinsic_type_list(T) TK_CLOSE_SBRACKET.
                             { G = T; }
intrinsic_head(I)        ::= TK_INTRINSIC_LT(L) intrinsic_path(P) intrinsic_generics_opt(G) TK_GT.
                             { I = intrinsic_new(context, L, P, G); }
intrinsic_prefix_head(I) ::= TK_INTRINSIC_PREFIX_LT(L) intrinsic_path(P) intrinsic_generics_opt(G) TK_GT.
                             { I = intrinsic_new(context, L, P, G); }

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
var_symbol(A)          ::= TK_TASK(S). { A = ast_f(context, VAR_SYMBOL, S); }
var_symbol(A)          ::= TK_PARALLEL(S). { A = ast_f(context, VAR_SYMBOL, S); }
var_symbol(A)          ::= TK_USING(S). { A = ast_f(context, VAR_SYMBOL, S); }
var_symbol(A)          ::= TK_VAR_SYMBOL(S) array_parameters(P).
                           { A = ast_f(context, VAR_SYMBOL, S); if (P) add_ast(A,P); }
var_symbol(A)          ::= TK_TASK(S) array_parameters(P).
                           { A = ast_f(context, VAR_SYMBOL, S); if (P) add_ast(A,P); }
var_symbol(A)          ::= TK_PARALLEL(S) array_parameters(P).
                           { A = ast_f(context, VAR_SYMBOL, S); if (P) add_ast(A,P); }
var_symbol(A)          ::= TK_USING(S) array_parameters(P).
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
label(A)               ::= TK_LABEL(S). [TK_TASK]
                           { A = ast_f(context, LABEL, S); }
label(A)               ::= TK_RESERVED_LABEL(S).
                           { A = mknd_err(ast_f(context, VAR_SYMBOL, S), "KEYWORD"); }

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
literal(L)               ::= TK_SIGNAL(N).
                         { L = ast_f(context, LITERAL, N); }
literal(L)               ::= TK_ON(N).
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
single_instruction(I)  ::= array_mutation(B). { I = B; }
single_instruction(I)  ::= define(B). { I = B; }
single_instruction(I)  ::= constant_def(B). { I = B; }
single_instruction(I)  ::= exit_extended(B). { I = B; }
single_instruction(I)  ::= command(B). { I = B; }
single_instruction(I)  ::= keyword_instruction(B). { I = B; }

array_mutation(I) ::= TK_ARRAY_APPEND(T) var_symbol(A) TK_WITH expression(V).
    { I = ast_f(context, ARRAY_APPEND, T); add_ast(I,A); add_ast(I,V); }

array_mutation(I) ::= TK_ARRAY_INSERT(T) var_symbol(A) TK_WITH expression(V) TK_ARRAY_AT expression(X).
    { I = ast_f(context, ARRAY_INSERT, T); add_ast(I,A); add_ast(I,V); add_ast(I,X); }

array_mutation(I) ::= TK_ARRAY_REMOVE(T) var_symbol(A) TK_ARRAY_AT expression(X).
    { I = ast_f(context, ARRAY_REMOVE, T); add_ast(I,A); add_ast(I,X); }

array_mutation(I) ::= TK_ARRAY_REMOVE(T) var_symbol(A) TK_ARRAY_AT expression(X) TK_FOR expression(C).
    { I = ast_f(context, ARRAY_REMOVE, T); add_ast(I,A); add_ast(I,X); add_ast(I,C); }

array_mutation(I) ::= TK_ARRAY_REMOVE(T) var_symbol(A) TK_ARRAY_AT expression(F) TK_TO expression(L).
    { I = ast_f(context, ARRAY_REMOVE_RANGE, T); add_ast(I,A); add_ast(I,F); add_ast(I,L); }

array_mutation(I) ::= TK_ARRAY_CLEAR(T) var_symbol(A).
    { I = ast_f(context, ARRAY_CLEAR, T); add_ast(I,A); }

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

/* Assignments trying to assign to a keywords.
 * Keep TK_END out of this recovery set: END must remain a structural
 * terminator for DO and ON SIGNAL bodies. Keyword-instruction recovery handles
 * stray END diagnostics without making END a valid assignment target.
 */
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
assignment(G)     ::= TK_AS(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_IS(K) TK_EQUAL(T) expression(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
assignment(G)     ::= TK_TYPEOF(K) TK_EQUAL(T) expression(E).
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
define(G)     ::= TK_AS(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_IS(K) TK_EQUAL(T) type_def(E).
      { G = ast_f(context, ASSIGN, T); add_ast(G,mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD")); add_ast(G,E);  }
define(G)     ::= TK_TYPEOF(K) TK_EQUAL(T) type_def(E).
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

constant_def(I) ::= TK_CONSTANT(K) var_symbol(V) TK_EQUAL expression(E).
    {
        I = ast_f(context, CONSTANT_DEF, K); add_ast(I,V); add_ast(I,E);
        V->node_type = VAR_TARGET;
    }

assignment(I) ::=  var_symbol(V) TK_EQUAL(T) expression(E). [TK_VAR_SYMBOL]
    {
        I = ast_f(context, ASSIGN, T); add_ast(I,V); add_ast(I,E);
        V->node_type = VAR_TARGET;
    }
assignment(I) ::=  binary_memory_access(V) TK_EQUAL(T) expression(E). [TK_VAR_SYMBOL]
    {
        I = ast_f(context, ASSIGN, T); add_ast(I,V); add_ast(I,E);
    }

command(I)             ::= command_expression(E).
                       { I = ast_ft(context, IMPLICIT_CMD); add_ast(I,E); }

keyword_instruction(I) ::= assembler(K). { I = K; }
keyword_instruction(I) ::= arg(K). { I = K; }
keyword_instruction(I) ::= call(K). { I = K; }
keyword_instruction(I) ::= iterate(K). { I = K; }
keyword_instruction(I) ::= leave(K). { I = K; }
keyword_instruction(I) ::= nop(K). { I = K; }
//keyword_instruction(I) ::= parse(K). { I = K; }
keyword_instruction(I) ::= procedure(K). { I = K; }
keyword_instruction(I) ::= task_def(K). { I = K; }
//keyword_instruction(I) ::= pull(K). { I = K; }
keyword_instruction(I) ::= return(K). { I = K; }
keyword_instruction(I) ::= exit(K). { I = K; }
keyword_instruction(I) ::= say(K). { I = K; }
keyword_instruction(I) ::= numeric(K). { I = K; }
keyword_instruction(I) ::= factory_def(K). { I = K; }
keyword_instruction(I) ::= match_def(K). { I = K; }
keyword_instruction(I) ::= method_def(K). { I = K; }
keyword_instruction(I) ::= class_def(K). { I = K; }
keyword_instruction(I) ::= interface_def(K). { I = K; }

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
group(I) ::= parallel_do(K) TK_EOC. { I = K; }
group(I) ::= parallel_do(K). { I = K; }
group(I) ::= do(K) TK_EOC. { I = K; }
group(I) ::= do(K). { I = K; }
group(I) ::= if(K). { I = K; }
group(I) ::= select(K) TK_EOC. { I = K; }
group(I) ::= select(K). { I = K; }

/* Groups */

/* Gate F structured parallel group. A missing USING child denotes the
 * execution-local default scope selected by the lowering pass. */
parallel_using_opt(U) ::= . { U = NULL; }
parallel_using_opt(U) ::= TK_USING expression(E). { U = E; }
parallel_do(G) ::= TK_DO(T) TK_PARALLEL parallel_using_opt(U) TK_EOC instruction_list(I) TK_END.
          { G = ast_f(context, PARALLEL_DO, T); if (U) add_ast(G,U); add_ast(G,I); }
parallel_do(G) ::= TK_DO(T) TK_PARALLEL parallel_using_opt(U) TK_EOC TK_END.
          { G = ast_f(context, PARALLEL_DO, T); if (U) add_ast(G,U); add_ast(G,ast_ft(context, INSTRUCTIONS)); }

/* Simple DO Group */
simple_do(G) ::= TK_DO(T) TK_EOC instruction_list(I) signal_handler_list(H) TK_END.
          { G = ast_f(context, SIGNAL_BLOCK, T); add_ast(G,I); add_ast(G,H); }
simple_do(G) ::= TK_DO(T) TK_EOC signal_handler_list(H) TK_END.
          { G = ast_f(context, SIGNAL_BLOCK, T); add_ast(G,ast_ft(context, NOP)); add_ast(G,H); }
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

signal_handler_list(L) ::= signal_handler(H).
          { L = H; }
signal_handler_list(L) ::= signal_handler_list(L1) signal_handler(H).
          { L = L1; add_sbtr(L,H); }

/* Handler bodies reuse the ordinary instruction grammar. TK_ON is not an
 * ordinary instruction starter, so the next ON SIGNAL naturally terminates the
 * previous handler body.
 */
signal_handler(H) ::= TK_ON(O) TK_SIGNAL signal_names(N) signal_as_opt(A) TK_EOC instruction_list(I). [TK_END]
          { H = ast_f(context, SIGNAL_HANDLER, O); add_ast(H,N); add_ast(H,A); add_ast(H,I); }

signal_names(N) ::= .
          { N = ast_ft(context, SIGNAL_NAMES); }
signal_names(N) ::= signal_name_list(L).
          { N = ast_ft(context, SIGNAL_NAMES); add_ast(N,L); }

signal_name_list(L) ::= signal_name(N).
          { L = N; }
signal_name_list(L) ::= signal_name_list(L1) TK_COMMA signal_name(N).
          { L = L1; add_sbtr(L,N); }

signal_name(N) ::= TK_VAR_SYMBOL(S).
          { N = ast_f(context, SIGNAL_NAME, S); }
signal_name(N) ::= TK_QUALIFIED_SYMBOL(S).
          { N = ast_f(context, SIGNAL_NAME, S); }

signal_as_opt(A) ::= .
          { A = ast_ft(context, NOP); }
signal_as_opt(A) ::= TK_AS TK_VAR_SYMBOL(S).
          { A = ast_f(context, VAR_TARGET, S); }

/* DO Group */

tk_doloop(D)  ::= TK_DO(T). [TK_PARALLEL]
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

/* SELECT Group */
%nonassoc TK_SELECT.
%nonassoc TK_WHEN.
%nonassoc TK_OTHERWISE.

select(S) ::= TK_SELECT(K) ncl0 when_list(W) TK_END.
              { S = ast_f(context, SELECT, K); add_ast(S,W); }
select(S) ::= TK_SELECT(K) ncl0 when_list(W) TK_OTHERWISE(O) ncl0 instruction_list(I) TK_END.
              { S = ast_f(context, SELECT, K); add_ast(S,W); ASTNode *oth = ast_f(context, OTHERWISE, O); if (I && I->node_type == INSTRUCTIONS && I->child && I->child->sibling == NULL) add_ast(oth, I->child); else add_ast(oth, I); add_ast(S,oth); }
select(S) ::= TK_SELECT(K) ncl0 when_list(W) TK_OTHERWISE(O) ncl0 TK_END.
              { S = ast_f(context, SELECT, K); add_ast(S,W); ASTNode *oth = ast_f(context, OTHERWISE, O); add_ast(oth,ast_ft(context, NOP)); add_ast(S,oth); }

select(S) ::= TK_SELECT(K) ncl0 TK_END(E).
              { S = ast_f(context, SELECT, K); add_ast(S, ast_err(context, "MISSING_WHEN", E)); }
select(S) ::= TK_SELECT(K) ncl0 TK_OTHERWISE(O) ncl0 instruction_list(I) TK_END.
              { S = ast_f(context, SELECT, K); add_ast(S, ast_err(context, "MISSING_WHEN", O)); ASTNode *oth = ast_f(context, OTHERWISE, O); if (I && I->node_type == INSTRUCTIONS && I->child && I->child->sibling == NULL) add_ast(oth, I->child); else add_ast(oth, I); add_ast(S,oth); }
select(S) ::= TK_SELECT(K) ncl0 TK_OTHERWISE(O) ncl0 TK_END.
              { S = ast_f(context, SELECT, K); add_ast(S, ast_err(context, "MISSING_WHEN", O)); ASTNode *oth = ast_f(context, OTHERWISE, O); add_ast(oth,ast_ft(context, NOP)); add_ast(S,oth); }

select(S) ::= TK_SELECT(K) ncl0 when_list(W) TK_EOS.
              { S = ast_f(context, SELECT, K); add_ast(S,W); add_ast(S,ast_err(context, "MISSING_END", K)); }
select(S) ::= TK_SELECT(K) ncl0 when_list(W) TK_OTHERWISE(O) ncl0 instruction_list(I) TK_EOS.
              { S = ast_f(context, SELECT, K); add_ast(S,W); ASTNode *oth = ast_f(context, OTHERWISE, O); if (I && I->node_type == INSTRUCTIONS && I->child && I->child->sibling == NULL) add_ast(oth, I->child); else add_ast(oth, I); add_ast(S,oth); add_ast(S,ast_err(context, "MISSING_END", K)); }
select(S) ::= TK_SELECT(K) ncl0 when_list(W) TK_OTHERWISE(O) ncl0 TK_EOS.
              { S = ast_f(context, SELECT, K); add_ast(S,W); ASTNode *oth = ast_f(context, OTHERWISE, O); add_ast(oth,ast_ft(context, NOP)); add_ast(S,oth); add_ast(S,ast_err(context, "MISSING_END", K)); }

select(S) ::= TK_SELECT(K) expression(E) ncl0 when_list(W) TK_END.
              { S = ast_f(context, SWITCH, K); add_ast(S,E); add_ast(S,W); }
select(S) ::= TK_SELECT(K) expression(E) ncl0 when_list(W) TK_OTHERWISE(O) ncl0 instruction_list(I) TK_END.
              { S = ast_f(context, SWITCH, K); add_ast(S,E); add_ast(S,W); ASTNode *oth = ast_f(context, OTHERWISE, O); if (I && I->node_type == INSTRUCTIONS && I->child && I->child->sibling == NULL) add_ast(oth, I->child); else add_ast(oth, I); add_ast(S,oth); }
select(S) ::= TK_SELECT(K) expression(E) ncl0 when_list(W) TK_OTHERWISE(O) ncl0 TK_END.
              { S = ast_f(context, SWITCH, K); add_ast(S,E); add_ast(S,W); ASTNode *oth = ast_f(context, OTHERWISE, O); add_ast(oth,ast_ft(context, NOP)); add_ast(S,oth); }

select(S) ::= TK_SELECT(K) expression(E) ncl0 TK_END(U).
              { S = ast_f(context, SWITCH, K); add_ast(S,E); add_ast(S, ast_err(context, "MISSING_WHEN", U)); }
select(S) ::= TK_SELECT(K) expression(E) ncl0 TK_OTHERWISE(O) ncl0 instruction_list(I) TK_END.
              { S = ast_f(context, SWITCH, K); add_ast(S,E); add_ast(S, ast_err(context, "MISSING_WHEN", O)); ASTNode *oth = ast_f(context, OTHERWISE, O); if (I && I->node_type == INSTRUCTIONS && I->child && I->child->sibling == NULL) add_ast(oth, I->child); else add_ast(oth, I); add_ast(S,oth); }
select(S) ::= TK_SELECT(K) expression(E) ncl0 TK_OTHERWISE(O) ncl0 TK_END.
              { S = ast_f(context, SWITCH, K); add_ast(S,E); add_ast(S, ast_err(context, "MISSING_WHEN", O)); ASTNode *oth = ast_f(context, OTHERWISE, O); add_ast(oth,ast_ft(context, NOP)); add_ast(S,oth); }

select(S) ::= TK_SELECT(K) expression(E) ncl0 when_list(W) TK_EOS.
              { S = ast_f(context, SWITCH, K); add_ast(S,E); add_ast(S,W); add_ast(S,ast_err(context, "MISSING_END", K)); }
select(S) ::= TK_SELECT(K) expression(E) ncl0 when_list(W) TK_OTHERWISE(O) ncl0 instruction_list(I) TK_EOS.
              { S = ast_f(context, SWITCH, K); add_ast(S,E); add_ast(S,W); ASTNode *oth = ast_f(context, OTHERWISE, O); if (I && I->node_type == INSTRUCTIONS && I->child && I->child->sibling == NULL) add_ast(oth, I->child); else add_ast(oth, I); add_ast(S,oth); add_ast(S,ast_err(context, "MISSING_END", K)); }
select(S) ::= TK_SELECT(K) expression(E) ncl0 when_list(W) TK_OTHERWISE(O) ncl0 TK_EOS.
              { S = ast_f(context, SWITCH, K); add_ast(S,E); add_ast(S,W); ASTNode *oth = ast_f(context, OTHERWISE, O); add_ast(oth,ast_ft(context, NOP)); add_ast(S,oth); add_ast(S,ast_err(context, "MISSING_END", K)); }

when_list(L) ::= when_clause(W).
                 { L = ast_ft(context, INSTRUCTIONS); add_ast(L,W); }
when_list(L) ::= when_list(L1) when_clause(W).
                 { L = L1; add_ast(L,W); }

when_clause(W) ::= TK_WHEN(K) expression(E) ncl0 then(T).
                   { W = ast_f(context, WHEN, K); add_ast(W,E); add_ast(W,T); }
when_clause(W) ::= TK_WHEN(K) expression(E) ncl0 ANYTHING(A).
                   { W = ast_f(context, WHEN, K); add_ast(W,E); add_ast(W,ast_err(context, "MISSING_THEN", A)); }
when_clause(W) ::= TK_WHEN(K) expression(E) ncl0 TK_EOS.
                   { W = ast_f(context, WHEN, K); add_ast(W,E); add_ast(W,ast_err(context, "MISSING_THEN", K)); }

/* Procedure / Args */
task_def(P)         ::= TK_LABEL(L) TK_TASK opt_method_return_type(T).
                      { P = ast_f(context, TASK_DECL, L); P->is_task_callable = 1;
                        if (T) add_ast(P,T); else add_ast(P,ast_ft(context, VOID)); }
procedure(P)      ::= TK_LABEL(L) TK_INITIALISER.
                      { P = ast_f(context, PROCEDURE, L); P->is_initializer = 1;
                        add_ast(P,ast_ft(context, VOID)); }
procedure(P)      ::= TK_LABEL(L) TK_INITIALISER expose(E).
                      { P = ast_f(context, PROCEDURE, L); P->is_initializer = 1;
                        add_ast(P,ast_ft(context, VOID)); add_ast(P,E); }
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
procedure(P)      ::= TK_RESERVED_LABEL(L) TK_PROCEDURE.
                      { P = mknd_err(ast_f(context, VAR_SYMBOL, L), "KEYWORD"); }
procedure(P)      ::= TK_RESERVED_LABEL(L) TK_PROCEDURE TK_EQUAL type_def.
                      { P = mknd_err(ast_f(context, VAR_SYMBOL, L), "KEYWORD"); }
procedure(P)      ::= TK_RESERVED_LABEL(L) TK_PROCEDURE TK_EQUAL TK_VOID.
                      { P = mknd_err(ast_f(context, VAR_SYMBOL, L), "KEYWORD"); }
procedure(P)      ::= TK_RESERVED_LABEL(L) TK_PROCEDURE expose.
                      { P = mknd_err(ast_f(context, VAR_SYMBOL, L), "KEYWORD"); }
procedure(P)      ::= TK_RESERVED_LABEL(L) TK_PROCEDURE TK_EQUAL type_def expose.
                      { P = mknd_err(ast_f(context, VAR_SYMBOL, L), "KEYWORD"); }
procedure(P)      ::= TK_RESERVED_LABEL(L) TK_PROCEDURE TK_EQUAL TK_VOID expose.
                      { P = mknd_err(ast_f(context, VAR_SYMBOL, L), "KEYWORD"); }

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
assembler_instruction(I) ::= assembler_op(OP) assembler_args(A).
    { I = OP; add_ast(I,A); }
assembler_args(A) ::= assembler_arg(V).
    { A = V; }
assembler_args(A) ::= assembler_args(L) TK_COMMA assembler_arg(V).
    { A = L; add_sbtr(A,V); }
assembler_op(OP)         ::= TK_VAR_SYMBOL(S).
                             { OP = ast_f(context, ASSEMBLER, S); }
assembler_op(OP)         ::= TK_SIGNAL(S).
                             { OP = ast_f(context, ASSEMBLER, S); }
assembler_op(OP)         ::= TK_SAY(S). /* SAY is also a REXX keyword */
                         { OP = ast_f(context, ASSEMBLER, S); }
assembler_op(OP)         ::= TK_EXIT(S). /* EXIT is also a REXX keyword */
                         { OP = ast_f(context, ASSEMBLER, S); }
assembler_arg(A)         ::= var_symbol(B).
                         { A = B; }
assembler_arg(A)         ::= TK_SELF(S).
                         { A = ast_f(context, VAR_SYMBOL, S); ast_sstr(A, strdup("\xc2\xa7" "this"), 6); }
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
assembler_arg(A)         ::= continued_string(S).
                         { A = ast_fstr_chain(context,S); }

/* Iterate */
iterate(I) ::= TK_ITERATE(T) var_symbol(S).
    { I = ast_f(context, ITERATE, T); add_ast(I,S); }

iterate(I) ::= TK_ITERATE(T).
    { I = ast_f(context, ITERATE, T); }

/* Leave */
leave(I) ::= TK_LEAVE(T) TK_WITH expression(E).
    { I = ast_f(context, LEAVE_WITH, T); add_ast(I,E); }

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
%left TK_STRING TK_STRING_CONTINUATION TK_FLOAT TK_DECIMAL TK_INTEGER TK_VAR_SYMBOL TK_QUALIFIED_SYMBOL.
%left TK_CLASS_FACTORY.
%left TK_OPEN_BRACKET.
%nonassoc TK_EQUAL.

%type continued_string {Token*}
continued_string(C)   ::= TK_STRING(S) TK_STRING_CONTINUATION(N).
                          { S->token_subtype = N->token_number; C = S; }
continued_string(C)   ::= continued_string(S) TK_STRING_CONTINUATION(N).
                          { S->token_subtype = N->token_number; C = S; }

function_name(N)       ::= TK_VAR_SYMBOL(S).
                           { N = ast_f(context, FUNCTION, S); }
function_name(N)       ::= TK_TASK(S).
                           { N = ast_f(context, FUNCTION, S); }
function_name(N)       ::= TK_PARALLEL(S).
                           { N = ast_f(context, FUNCTION, S); }
function_name(N)       ::= TK_USING(S).
                           { N = ast_f(context, FUNCTION, S); }
function_name(N)       ::= TK_QUALIFIED_SYMBOL(S).
                           { N = ast_f(context, FUNCTION, S); }
function_name(N)       ::= TK_STRING(S).
                           { N = ast_f(context, FUNCTION, S); }
function_name(N)       ::= TK_ARG(S).
                           { N = mknd_err(ast_f(context, FUNCTION, S), "KEYWORD"); }
function_name(N)       ::= TK_AS(S).
                           { N = mknd_err(ast_f(context, FUNCTION, S), "KEYWORD"); }
function_name(N)       ::= TK_IS(S).
                           { N = mknd_err(ast_f(context, FUNCTION, S), "KEYWORD"); }
function_name(N)       ::= TK_TYPEOF(S).
                           { N = mknd_err(ast_f(context, FUNCTION, S), "KEYWORD"); }
call(I) ::= TK_CALL(T) function_name(F) expression_list(E).
        { I = ast_f(context, CALL, T); add_ast(I,F); if (E) add_ast(F,E); }
call(I) ::= TK_CALL(T) TK_VAR_SYMBOL(S) function_parameters(P). [TK_VAR_SYMBOL]
        { I = ast_f(context, CALL, T); ASTNode *F = ast_f(context, FUNCTION, S); add_ast(I, F); if (P) add_ast(F, P); }
call(I) ::= TK_CALL(T) TK_QUALIFIED_SYMBOL(S) function_parameters(P). [TK_QUALIFIED_SYMBOL]
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
call(I) ::= TK_CALL(T) TK_CLASS_FACTORY(S) function_parameters(PP).
        {
           size_t split = 0;
           size_t i;
           ASTNode *F = ast_f(context, FACTORY_CALL, S);

           for (i = 1; i < F->node_string_length; i++) {
               if (F->node_string[i] == '.') split = i;
           }

           if (split) {
               F->association = ast_f(context, VAR_SYMBOL, S);
               F->association->node_string = S->token_string + split + 1;
               F->association->node_string_length = S->length - split - 1;
               F->node_string++;
               F->node_string_length = split - 1;
           }

           if (PP) add_ast(F, PP);
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
term(F)                ::= TK_TASK(T) TK_VAR_SYMBOL(S).
                           { F = ast_f(context, TASK_TARGET, T); add_ast(F,ast_f(context, FUNC_SYMBOL,S)); }
term(F)                ::= TK_TASK(T) TK_QUALIFIED_SYMBOL(S).
                           { F = ast_f(context, TASK_TARGET, T); add_ast(F,ast_f(context, FUNC_SYMBOL,S)); }
term(F)                ::= TK_TASK(T) TK_CLASS_FACTORY(S) function_parameters(PP).
                           {
                               ASTNode *factory = ast_f(context, FACTORY_CALL, S);
                               if (PP) add_ast(factory, PP);
                               F = ast_f(context, TASK_TARGET, T);
                               add_ast(F, factory);
                           }
term(F)                ::= TK_TASK(T) TK_CLASS_TYPE(S) function_parameters(PP).
                           {
                               ASTNode *factory = ast_f(context, FACTORY_CALL, S);
                               if (factory->node_string && factory->node_string[0] == '.') {
                                   factory->node_string++;
                                   factory->node_string_length--;
                               }
                               if (PP) add_ast(factory, PP);
                               F = ast_f(context, TASK_TARGET, T);
                               add_ast(F, factory);
                           }
term(F)                ::= TK_TASK(S) function_parameters(P).
                           { F = ast_f(context, FUNCTION, S); if (P) add_ast(F,P); }
term(F)                ::= TK_PARALLEL(S) function_parameters(P).
                           { F = ast_f(context, FUNCTION, S); if (P) add_ast(F,P); }
term(F)                ::= TK_USING(S) function_parameters(P).
                           { F = ast_f(context, FUNCTION, S); if (P) add_ast(F,P); }
term(F)                ::= TK_VAR_SYMBOL(S) function_parameters(P).
                           {
                               F = ast_f(context, token_text_equals_ci(S, "initialized") ? OP_INITIALIZED : FUNCTION, S);
                               if (P) add_ast(F,P);
                           }
term(F)                ::= TK_VAR_SYMBOL(S) TK_OPEN_BRACKET TK_CLASS_TYPE(C) TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           {
                               ASTNode *type_node = ast_f(context, CLASS, C);
                               F = ast_f(context, token_text_equals_ci(S, "initialized") ? OP_INITIALIZED : FUNCTION, S);
                               add_ast(F, type_node);
                           }
term(F)                ::= TK_QUALIFIED_SYMBOL(S) function_parameters(P).
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
term(F)                ::= TK_CLASS_FACTORY(S) function_parameters(PP).
                           {
                               size_t split = 0;
                               size_t i;
                               F = ast_f(context, FACTORY_CALL, S);
                               for (i = 1; i < F->node_string_length; i++) {
                                   if (F->node_string[i] == '.') split = i;
                               }
                               if (split) {
                                   F->association = ast_f(context, VAR_SYMBOL, S);
                                   F->association->node_string = S->token_string + split + 1;
                                   F->association->node_string_length = S->length - split - 1;
                                   F->node_string++;
                                   F->node_string_length = split - 1;
                               }
                               if (PP) add_ast(F, PP);
                           }
term(F)                ::= TK_STRING(S) function_parameters(P).
                           { F = ast_f(context, FUNCTION, S); if (P) add_ast(F,P); }
function_parameters(P) ::= TK_OPEN_BRACKET expression_list(E) TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           { P = E; }
term(A)                ::= var_symbol(B). [TK_VAR_SYMBOL]
                         { A = B; }
term(A)                ::= TK_SELF(S).
                         { A = ast_f(context, VAR_SYMBOL, S); ast_sstr(A, strdup("\xc2\xa7" "this"), 6); }
term(A)                ::= TK_FLOAT(S).
                         { A = ast_f(context, FLOAT,S); }
term(A)                ::= TK_DECIMAL(S).
                         { A = ast_fdec(context,S); }
term(A)                ::= TK_INTEGER(S).
                         { A = ast_f(context, INTEGER,S); }
term(A)                ::= TK_STRING(S).
                         { A = ast_fstr(context,S); }
term(A)                ::= continued_string(S).
                         { A = ast_fstr_chain(context,S); }
term(F)                ::= intrinsic_head(I). [TK_VAR_SYMBOL]
                         { F = intrinsic_lower_primary(context, I); }
term(F)                ::= intrinsic_head(I) function_parameters(P). [TK_VAR_SYMBOL]
                         { F = intrinsic_lower_primary(context, intrinsic_with_args(context, I, P)); }

/* Special Operator - ARG */
term(F)                ::= TK_ARG(A) TK_OPEN_BRACKET TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           { F = ast_err(context, "LEGACY_ARG_CALL_SYNTAX", A); }
term(F)                ::= TK_ARG(A) TK_OPEN_BRACKET expression_in_list(E) TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           { (void)E; F = ast_err(context, "LEGACY_ARG_CALL_SYNTAX", A); }
term(F)                ::= TK_ARG(A) TK_OPEN_BRACKET expression_in_list(EX) TK_COMMA TK_STRING(OP) TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           {
                              (void)EX; (void)OP;
                              F = ast_err(context, "LEGACY_ARG_CALL_SYNTAX", A);
                           }
term(F)                ::= TK_ARG(A) TK_OPEN_BRACKET expression_in_list(EX) TK_COMMA TK_VAR_SYMBOL(OP) TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           {
                              (void)EX; (void)OP;
                              F = ast_err(context, "LEGACY_ARG_CALL_SYNTAX", A);
                           }
term(F)                ::= TK_ARG TK_OPEN_BRACKET(A) error TK_CLOSE_BRACKET.
                           { F = ast_err(context, "LEGACY_ARG_CALL_SYNTAX", A); }
term(F)                ::= TK_ARG TK_OPEN_BRACKET ANYTHING(A).
                           { F = ast_err(context, "LEGACY_ARG_CALL_SYNTAX", A); }

/* Type inspection intrinsic */
term(F)                ::= TK_TYPEOF(A) TK_OPEN_BRACKET expression(E) TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           { (void)E; F = ast_err(context, "LEGACY_TYPEOF_SYNTAX", A); }
term(F)                ::= TK_TYPEOF TK_OPEN_BRACKET(A) error TK_CLOSE_BRACKET.
                           { F = ast_err(context, "LEGACY_TYPEOF_SYNTAX", A); }
term(F)                ::= TK_TYPEOF TK_OPEN_BRACKET ANYTHING(A).
                           { F = ast_err(context, "LEGACY_TYPEOF_SYNTAX", A); }

/* Reference intrinsics */
term(F)                ::= TK_REFVALID(A) TK_OPEN_BRACKET expression(E) TK_CLOSE_BRACKET. [TK_VAR_SYMBOL]
                           { (void)E; F = ast_err(context, "LEGACY_REFVALID_SYNTAX", A); }
term(F)                ::= TK_REFVALID TK_OPEN_BRACKET(A) error TK_CLOSE_BRACKET.
                           { F = ast_err(context, "LEGACY_REFVALID_SYNTAX", A); }
term(F)                ::= TK_REFVALID TK_OPEN_BRACKET ANYTHING(A).
                           { F = ast_err(context, "LEGACY_REFVALID_SYNTAX", A); }

/* Special Operator - ? */
term(F)                ::= TK_OPTIONAL TK_VAR_SYMBOL(S). [TK_VAR_SYMBOL]
                           { F = ast_f(context, OP_ARG_EXISTS, S); }

/* Special Operator - arg pseudo array/stem */
term(A)                ::= TK_ARG(S) array_parameters(P). [TK_VAR_SYMBOL]
                           { A = ast_f(context, OP_ARG_VALUE, S); if (P) add_ast(A,P); }
term(A)                ::= TK_ARG_STEM(S) stemparts(P). [TK_VAR_SYMBOL]
                           { (void)P; A = ast_err(context, "LEGACY_ARG_STEM_SYNTAX", S); }

/* These Keywords can be trapped at error terms - e.g. they are not instructions */
term(E)                 ::= TK_OPTIONS(K). [ANYTHING] { E = mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"); }
term(E)                 ::= TK_NAMESPACE(K). [ANYTHING] { E = mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"); }
term(E)                 ::= TK_IMPORT(K). [ANYTHING] { E = mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"); }
term(E)                 ::= TK_VOID(K). [ANYTHING] { E = mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"); }
term(E)                 ::= TK_OPTIONAL(K). [ANYTHING] { E = mknd_err(ast_f(context, VAR_SYMBOL,K), "KEYWORD"); }

binary_memory_operand(A) ::= term(T).
                         { A = T; }
binary_memory_operand(A) ::= TK_OPEN_BRACKET expression(B) TK_CLOSE_BRACKET.
                         { A = B; }
binary_memory_access(A) ::= intrinsic_prefix_head(I) function_parameters(P) binary_memory_operand(B). [TK_NAMED_OPERATOR]
                         { A = intrinsic_lower_prefix(context, intrinsic_with_args(context, I, P), B); }

bracket(A)           ::= term(T).
                         { A = T; }
bracket(A)           ::= TK_OPEN_BRACKET expression(B) TK_CLOSE_BRACKET.
                         { A = B; }
bracket(A)           ::= binary_memory_access(B). [TK_NAMED_OPERATOR]
                         { A = B; }
block_expr(A)        ::= TK_DO(T) TK_EOC instruction_list(I) TK_END.
                         { A = ast_f(context, BLOCK_EXPR, T); add_ast(A, I); }
block_expr(A)        ::= TK_DO(T) TK_EOC TK_END.
                         { A = ast_f(context, BLOCK_EXPR, T); add_ast(A, ast_ft(context, INSTRUCTIONS)); }
parallel_block_expr(A) ::= TK_DO(T) TK_PARALLEL parallel_using_opt(U) TK_EOC instruction_list(I) TK_END.
                         { A = ast_f(context, PARALLEL_BLOCK_EXPR, T); if (U) add_ast(A,U); add_ast(A,I); }
parallel_block_expr(A) ::= TK_DO(T) TK_PARALLEL parallel_using_opt(U) TK_EOC TK_END.
                         { A = ast_f(context, PARALLEL_BLOCK_EXPR, T); if (U) add_ast(A,U); add_ast(A,ast_ft(context, INSTRUCTIONS)); }
bracket(A)           ::= block_expr(B).
                         { A = B; }
bracket(A)           ::= parallel_block_expr(B).
                         { A = B; }
/* Standalone class factory call as a primary */
bracket(F)           ::= TK_CLASS_TYPE(S) TK_CLASS_TYPE(M) function_parameters(P).
                           {
                               F = ast_f(context, FACTORY_CALL, S);
                               if (P) add_ast(F,P);
                               if (F->node_string && F->node_string[0] == '.') {
                                   F->node_string++;
                                   F->node_string_length--;
                               }
                               F->association = ast_f(context, VAR_SYMBOL, M);
                               if (F->association->node_string && F->association->node_string[0] == '.') {
                                   F->association->node_string++;
                                   F->association->node_string_length--;
                               }
                           }
bracket(F)           ::= TK_CLASS_TYPE(S) function_parameters(P).
                           {
                               F = ast_f(context, FACTORY_CALL, S);
                               if (P) add_ast(F,P);
                               if (F->node_string && F->node_string[0] == '.') {
                                   F->node_string++;
                                   F->node_string_length--;
                               }
                           }

/* Command expressions use a left-spine variant so a bare TK_DO at statement
 * start remains reserved for statement DO blocks, while nested expressions can
 * still use BLOCK_EXPR through the regular expression grammar. */
command_bracket(A)   ::= term(T).
                         { A = T; }
command_bracket(A)   ::= TK_OPEN_BRACKET expression(B) TK_CLOSE_BRACKET.
                         { A = B; }
command_bracket(A)   ::= binary_memory_access(B). [TK_NAMED_OPERATOR]
                         { A = B; }
command_bracket(F)   ::= TK_CLASS_TYPE(S) TK_CLASS_TYPE(M) function_parameters(P).
                           {
                               F = ast_f(context, FACTORY_CALL, S);
                               if (P) add_ast(F,P);
                               if (F->node_string && F->node_string[0] == '.') {
                                   F->node_string++;
                                   F->node_string_length--;
                               }
                               F->association = ast_f(context, VAR_SYMBOL, M);
                               if (F->association->node_string && F->association->node_string[0] == '.') {
                                   F->association->node_string++;
                                   F->association->node_string_length--;
                               }
                           }
command_bracket(F)   ::= TK_CLASS_TYPE(S) function_parameters(P).
                           {
                               F = ast_f(context, FACTORY_CALL, S);
                               if (P) add_ast(F,P);
                               if (F->node_string && F->node_string[0] == '.') {
                                   F->node_string++;
                                   F->node_string_length--;
                               }
                           }
command_postfix(P)   ::= command_bracket(B).
                         { P = B; }
command_postfix(A)   ::= command_postfix(B) TK_CLASS_TYPE(S) function_parameters(PP). [TK_CLASS_TYPE]
                         { A = ast_f(context, MEMBER_CALL, S);
                           if (A->node_string && A->node_string[0] == '.') {
                               A->node_string++;
                               A->node_string_length--; }
                           add_ast(A,B); if (PP) add_ast(A,PP); }
command_postfix(A)   ::= command_postfix(B) TK_AS(O) type_def(T). [TK_CLASS_TYPE]
                         { A = ast_f(context, OP_TYPE_CAST, O);
                           add_ast(A, B);
                           add_ast(A, T); }
command_prefix_expression(P) ::= command_postfix(B). [ANYTHING] { P = B; }
command_prefix_expression(A) ::= TK_NOT(O) prefix_expression(C).
                         { A = ast_f(context, OP_NOT, O); add_ast(A,C); }
command_prefix_expression(A) ::= TK_NAMED_OPERATOR(O) prefix_expression(C).
                         { A = named_prefix_operator(context, O, C); }
command_prefix_expression(A) ::= TK_PLUS(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_PLUS, O); add_ast(A,C); }
command_prefix_expression(A) ::= TK_HIGH_PRIORITY_MINUS(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_NEG, O); add_ast(A,C); }
command_prefix_expression(A) ::= TK_REFERENCE(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_REFERENCE, O); add_ast(A,C); }
command_prefix_expression(A) ::= TK_DEREFERENCE(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_DEREFERENCE, O); add_ast(A,C); }
command_prefix_expression(A) ::= TK_SNAPSHOT(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_SNAPSHOT, O); add_ast(A,C); }
command_power_expression_L(A) ::= command_power_expression_L(B) TK_POWER_L(O) prefix_expression(C).
                          { A = ast_f(context, OP_POWER, O); add_ast(A,B); add_ast(A,C); }
command_power_expression_L(P) ::= command_prefix_expression(E).  { P = E; }
command_power_expression_R(A) ::= command_power_expression_L(B) TK_POWER_R(O) power_expression_R(C).
                          { A = ast_f(context, OP_POWER, O); add_ast(A,B); add_ast(A,C); }
command_power_expression_R(P) ::= command_power_expression_L(E).  { P = E; }
command_low_prefix_expression(P) ::= command_power_expression_R(E).
                  { P = E; }
command_low_prefix_expression(A) ::= TK_MINUS(O) power_expression_R(C).
                  { A = ast_f(context, OP_NEG, O); add_ast(A,C); }
command_multiplication(P)    ::= command_low_prefix_expression(E).
                         { P = E; }
command_multiplication(A)    ::= command_multiplication(B) TK_MULT(O) low_prefix_expression(C).
                         { A = ast_f(context, OP_MULT, O); add_ast(A,B); add_ast(A,C); }
command_multiplication(A)    ::= command_multiplication(B) TK_DIV(O) low_prefix_expression(C).
                         { A = ast_f(context, OP_DIV, O); add_ast(A,B); add_ast(A,C); }
command_multiplication(A)    ::= command_multiplication(B) TK_IDIV(O) low_prefix_expression(C).
                         { A = ast_f(context, OP_IDIV, O); add_ast(A,B); add_ast(A,C); }
command_multiplication(A)    ::= command_multiplication(B) TK_MOD(O) low_prefix_expression(C).
                         { A = ast_f(context, OP_MOD, O); add_ast(A,B); add_ast(A,C); }
command_multiplication(A)    ::= command_multiplication(B) TK_NAMED_MULT_OPERATOR(O) low_prefix_expression(C).
                         { A = named_binary_operator(context, O, B, C); }
command_addition(P)          ::= command_multiplication(E).
                         { P = E; }
command_addition(A)          ::= command_addition(B) TK_PLUS(O) multiplication(C).
                         { A = ast_f(context, OP_ADD, O); add_ast(A,B); add_ast(A,C); }
command_addition(A)          ::= command_addition(B) TK_MINUS(O) multiplication(C).
                         { A = ast_f(context, OP_MINUS, O); add_ast(A,B); add_ast(A,C); }
command_addition(A)          ::= command_addition(B) TK_HIGH_PRIORITY_MINUS(O) multiplication(C).
                         { A = ast_f(context, OP_MINUS, O); add_ast(A,B); add_ast(A,C); }
command_shift_operator(P)    ::= command_addition(E).
                         { P = E; }
command_shift_operator(A)    ::= command_shift_operator(B) TK_NAMED_SHIFT_OPERATOR(O) addition(C).
                         { A = named_binary_operator(context, O, B, C); }
command_bit_and_operator(P)  ::= command_shift_operator(E).
                         { P = E; }
command_bit_and_operator(A)  ::= command_bit_and_operator(B) TK_NAMED_AND_OPERATOR(O) shift_operator(C).
                         { A = named_binary_operator(context, O, B, C); }
command_bit_xor_operator(P)  ::= command_bit_and_operator(E).
                         { P = E; }
command_bit_xor_operator(A)  ::= command_bit_xor_operator(B) TK_NAMED_XOR_OPERATOR(O) bit_and_operator(C).
                         { A = named_binary_operator(context, O, B, C); }
command_bit_or_operator(P)   ::= command_bit_xor_operator(E).
                         { P = E; }
command_bit_or_operator(A)   ::= command_bit_or_operator(B) TK_NAMED_OR_OPERATOR(O) bit_xor_operator(C).
                         { A = named_binary_operator(context, O, B, C); }
command_named_operator(P)    ::= command_bit_or_operator(E).
                         { P = E; }
command_named_operator(A)    ::= command_named_operator(B) TK_NAMED_OPERATOR(O) bit_or_operator(C).
                         { A = named_binary_operator(context, O, B, C); }
command_concatenation(P)     ::= command_named_operator(E). [IMPLICIT_CONCAT]
                         { P = E; }
command_concatenation(A)     ::= command_concatenation(B) TK_CONCAT(O) named_operator(C). [IMPLICIT_CONCAT]
                         { A = ast_f(context, OP_CONCAT, O); add_ast(A,B); add_ast(A,C); }
command_concatenation(A)     ::= command_concatenation(B) named_operator_c(C). [IMPLICIT_CONCAT]
                         { A = ast_ft(context, OP_SCONCAT); add_ast(A,B); add_ast(A,C); }
command_comparison(P)        ::= command_concatenation(E).
                         { P = E; }
command_comparison(A)        ::= command_comparison(B) TK_EQUAL(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_EQUAL, O); add_ast(A,B); add_ast(A,C); }
command_comparison(A)        ::= command_comparison(B) TK_NEQ(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_NEQ, O); add_ast(A,B); add_ast(A,C); }
command_comparison(A)        ::= command_comparison(B) TK_GT(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_GT, O); add_ast(A,B); add_ast(A,C); }
command_comparison(A)        ::= command_comparison(B) TK_LT(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_LT, O); add_ast(A,B); add_ast(A,C); }
command_comparison(A)        ::= command_comparison(B) TK_GTE(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_GTE, O); add_ast(A,B); add_ast(A,C); }
command_comparison(A)        ::= command_comparison(B) TK_LTE(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_LTE, O); add_ast(A,B); add_ast(A,C); }
command_comparison(A)        ::= command_comparison(B) TK_S_EQ(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_S_EQ, O); add_ast(A,B); add_ast(A,C); }
command_comparison(A)        ::= command_comparison(B) TK_S_NEQ(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_S_NEQ, O); add_ast(A,B); add_ast(A,C); }
command_comparison(A)        ::= command_comparison(B) TK_S_GT(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_S_GT, O); add_ast(A,B); add_ast(A,C); }
command_comparison(A)        ::= command_comparison(B) TK_S_LT(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_S_LT, O); add_ast(A,B); add_ast(A,C); }
command_comparison(A)        ::= command_comparison(B) TK_S_GTE(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_S_GTE, O); add_ast(A,B); add_ast(A,C); }
command_comparison(A)        ::= command_comparison(B) TK_S_LTE(O) concatenation(C).
                         { A = ast_f(context, OP_COMPARE_S_LTE, O); add_ast(A,B); add_ast(A,C); }
command_comparison(A)        ::= command_comparison(B) TK_IS(O) type_def(T).
                         { A = ast_f(context, OP_TYPE_IS, O); add_ast(A,B); add_ast(A,T); }
command_or_expression(P)     ::= command_comparison(E).
                         { P = E; }
command_or_expression(A)     ::= command_or_expression(B) TK_OR(O) comparison(C).
                         { A = ast_f(context, OP_OR, O); add_ast(A,B); add_ast(A,C); }
command_or_expression(A)     ::= command_or_expression(B) TK_XOR(O) comparison(C).
                         { A = ast_f(context, OP_XOR, O); add_ast(A,B); add_ast(A,C); }
command_and_expression(P)    ::= command_or_expression(E).
                         { P = E; }
command_and_expression(A)    ::= command_and_expression(B) TK_AND(O) or_expression(C).
                         { A = ast_f(context, OP_AND, O); add_ast(A,B); add_ast(A,C); }
command_expression(P)        ::= command_and_expression(E). { P = E; }
command_expression(E)        ::= TK_COMMA(U) error. { E = ast_err(context, "BADEXPR", U); }
command_expression(E)        ::= TK_CLOSE_BRACKET(U) error. { E = ast_err(context, "BADEXPR", U); }

/* These are the normal expression form in unambiguous form */
postfix(P)           ::= bracket(B).
                         { P = B; }
postfix(A)           ::= postfix(B) TK_CLASS_TYPE(S) function_parameters(PP). [TK_CLASS_TYPE]
                         { A = ast_f(context, MEMBER_CALL, S);
                           if (A->node_string && A->node_string[0] == '.') {
                               A->node_string++;
                               A->node_string_length--; }
                           add_ast(A,B); if (PP) add_ast(A,PP); }
postfix(A)           ::= postfix(B) TK_AS(O) type_def(T). [TK_CLASS_TYPE]
                         { A = ast_f(context, OP_TYPE_CAST, O);
                           add_ast(A, B);
                           add_ast(A, T); }

prefix_expression(P) ::= postfix(B). [ANYTHING] { P = B; }
prefix_expression(A) ::= TK_NOT(O) prefix_expression(C).
                         { A = ast_f(context, OP_NOT, O); add_ast(A,C); }
prefix_expression(A) ::= TK_NAMED_OPERATOR(O) prefix_expression(C).
                         { A = named_prefix_operator(context, O, C); }
prefix_expression(A) ::= TK_PLUS(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_PLUS, O); add_ast(A,C); }
prefix_expression(A) ::= TK_HIGH_PRIORITY_MINUS(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_NEG, O); add_ast(A,C); }
prefix_expression(A) ::= TK_REFERENCE(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_REFERENCE, O); add_ast(A,C); }
prefix_expression(A) ::= TK_DEREFERENCE(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_DEREFERENCE, O); add_ast(A,C); }
prefix_expression(A) ::= TK_SNAPSHOT(O) prefix_expression(C). [TK_NOT]
                         { A = ast_f(context, OP_SNAPSHOT, O); add_ast(A,C); }
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
multiplication(A)    ::= multiplication(B) TK_NAMED_MULT_OPERATOR(O) low_prefix_expression(C).
                         { A = named_binary_operator(context, O, B, C); }
addition(P)          ::= multiplication(E).
                         { P = E; }
addition(A)          ::= addition(B) TK_PLUS(O) multiplication(C).
                         { A = ast_f(context, OP_ADD, O); add_ast(A,B); add_ast(A,C); }
addition(A)          ::= addition(B) TK_MINUS(O) multiplication(C).
                         { A = ast_f(context, OP_MINUS, O); add_ast(A,B); add_ast(A,C); }
addition(A)          ::= addition(B) TK_HIGH_PRIORITY_MINUS(O) multiplication(C).
                         { A = ast_f(context, OP_MINUS, O); add_ast(A,B); add_ast(A,C); }

shift_operator(P)    ::= addition(E).
                         { P = E; }
shift_operator(A)    ::= shift_operator(B) TK_NAMED_SHIFT_OPERATOR(O) addition(C).
                         { A = named_binary_operator(context, O, B, C); }
bit_and_operator(P)  ::= shift_operator(E).
                         { P = E; }
bit_and_operator(A)  ::= bit_and_operator(B) TK_NAMED_AND_OPERATOR(O) shift_operator(C).
                         { A = named_binary_operator(context, O, B, C); }
bit_xor_operator(P)  ::= bit_and_operator(E).
                         { P = E; }
bit_xor_operator(A)  ::= bit_xor_operator(B) TK_NAMED_XOR_OPERATOR(O) bit_and_operator(C).
                         { A = named_binary_operator(context, O, B, C); }
bit_or_operator(P)   ::= bit_xor_operator(E).
                         { P = E; }
bit_or_operator(A)   ::= bit_or_operator(B) TK_NAMED_OR_OPERATOR(O) bit_xor_operator(C).
                         { A = named_binary_operator(context, O, B, C); }
named_operator(P)    ::= bit_or_operator(E).
                         { P = E; }
named_operator(A)    ::= named_operator(B) TK_NAMED_OPERATOR(O) bit_or_operator(C).
                         { A = named_binary_operator(context, O, B, C); }

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
postfix_c(A)         ::= postfix_c(B) TK_AS(O) type_def(T). [TK_CLASS_TYPE]
                         { A = ast_f(context, OP_TYPE_CAST, O);
                           add_ast(A, B);
                           add_ast(A, T); }

prefix_expression_c(P) ::= postfix_c(B). [ANYTHING] { P = B; }

prefix_expression_c(A) ::= TK_NOT(O) prefix_expression_c(C).
                         { A = ast_f(context, OP_NOT, O); add_ast(A,C); }
prefix_expression_c(A) ::= TK_NAMED_OPERATOR(O) prefix_expression_c(C).
                         { A = named_prefix_operator(context, O, C); }
prefix_expression_c(A) ::= TK_REFERENCE(O) prefix_expression_c(C). [TK_NOT]
                         { A = ast_f(context, OP_REFERENCE, O); add_ast(A,C); }
prefix_expression_c(A) ::= TK_DEREFERENCE(O) prefix_expression_c(C). [TK_NOT]
                         { A = ast_f(context, OP_DEREFERENCE, O); add_ast(A,C); }
prefix_expression_c(A) ::= TK_SNAPSHOT(O) prefix_expression_c(C). [TK_NOT]
                         { A = ast_f(context, OP_SNAPSHOT, O); add_ast(A,C); }

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
multiplication_c(A)  ::= multiplication_c(B) TK_NAMED_MULT_OPERATOR(O) power_expression_R_c(C).
                         { A = named_binary_operator(context, O, B, C); }
addition_c(P)        ::= multiplication_c(E).
                         { P = E; }
addition_c(A)        ::= addition_c(B) TK_PLUS(O) multiplication_c(C).
                         { A = ast_f(context, OP_ADD, O); add_ast(A,B); add_ast(A,C); }
addition_c(A)        ::= addition_c(B) TK_MINUS(O) multiplication_c(C).
                         { A = ast_f(context, OP_MINUS, O); add_ast(A,B); add_ast(A,C); }
addition_c(A)        ::= addition_c(B) TK_HIGH_PRIORITY_MINUS(O) multiplication_c(C).
                         { A = ast_f(context, OP_MINUS, O); add_ast(A,B); add_ast(A,C); }

shift_operator_c(P)  ::= addition_c(E).
                         { P = E; }
shift_operator_c(A)  ::= shift_operator_c(B) TK_NAMED_SHIFT_OPERATOR(O) addition_c(C).
                         { A = named_binary_operator(context, O, B, C); }
bit_and_operator_c(P) ::= shift_operator_c(E).
                         { P = E; }
bit_and_operator_c(A) ::= bit_and_operator_c(B) TK_NAMED_AND_OPERATOR(O) shift_operator_c(C).
                         { A = named_binary_operator(context, O, B, C); }
bit_xor_operator_c(P) ::= bit_and_operator_c(E).
                         { P = E; }
bit_xor_operator_c(A) ::= bit_xor_operator_c(B) TK_NAMED_XOR_OPERATOR(O) bit_and_operator_c(C).
                         { A = named_binary_operator(context, O, B, C); }
bit_or_operator_c(P) ::= bit_xor_operator_c(E).
                         { P = E; }
bit_or_operator_c(A) ::= bit_or_operator_c(B) TK_NAMED_OR_OPERATOR(O) bit_xor_operator_c(C).
                         { A = named_binary_operator(context, O, B, C); }
named_operator_c(P)  ::= bit_or_operator_c(E).
                         { P = E; }
named_operator_c(A)  ::= named_operator_c(B) TK_NAMED_OPERATOR(O) bit_or_operator_c(C).
                         { A = named_binary_operator(context, O, B, C); }

/* Back to normal expressions in the usual unambiguous form */
concatenation(P)     ::= named_operator(E). [IMPLICIT_CONCAT]
                         { P = E; }
concatenation(A)     ::= concatenation(B) TK_CONCAT(O) named_operator(C). [IMPLICIT_CONCAT]
                         { A = ast_f(context, OP_CONCAT, O); add_ast(A,B); add_ast(A,C); }
concatenation(A)     ::= concatenation(B) named_operator_c(C). [IMPLICIT_CONCAT] /* Note the addition_c */
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
comparison(A)        ::= comparison(B) TK_IS(O) type_def(T).
                         { A = ast_f(context, OP_TYPE_IS, O); add_ast(A,B); add_ast(A,T); }
or_expression(P)     ::= comparison(E).
                         { P = E; }
or_expression(A)     ::= or_expression(B) TK_OR(O) comparison(C).
                         { A = ast_f(context, OP_OR, O); add_ast(A,B); add_ast(A,C); }
or_expression(A)     ::= or_expression(B) TK_XOR(O) comparison(C).
                         { A = ast_f(context, OP_XOR, O); add_ast(A,B); add_ast(A,C); }
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
and_expression(E)  ::= TK_XOR(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_NAMED_OPERATOR(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_NAMED_MULT_OPERATOR(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_NAMED_SHIFT_OPERATOR(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_NAMED_AND_OPERATOR(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_NAMED_XOR_OPERATOR(U) error. { E = ast_err(context, "BADEXPR", U); }
and_expression(E)  ::= TK_NAMED_OR_OPERATOR(U) error. { E = ast_err(context, "BADEXPR", U); }

expression(P)  ::= and_expression(E). { P = E; }
expression(E)  ::= TK_COMMA(U) error. { E = ast_err(context, "BADEXPR", U); }
expression(E)  ::= TK_CLOSE_BRACKET(U) error. { E = ast_err(context, "BADEXPR", U); }

/* expressions in a list cannot expression() errors above because of parsing conflicts */
expression_in_list(P) ::= and_expression(E). { P = E; }

/* Finally set the nodes with the highest precedence */
%left TK_BADCOMMENT.

/* Classes / Factories / Methods */

class_def(C) ::= TK_LABEL(L) TK_CLASS opt_of(O) opt_implements(I).
{
  C = ast_f(context, CLASS_DEF, L);
  if (O) add_ast(C, O);
  if (I) add_ast(C, I);
}
class_def(C) ::= TK_RESERVED_LABEL(L) TK_CLASS opt_of opt_implements.
{
  C = mknd_err(ast_f(context, VAR_SYMBOL, L), "KEYWORD");
}

opt_of(O) ::= . { O = NULL; }
opt_of(O) ::= TK_OF type_def(T). { O = T; }

opt_implements(I) ::= . { I = NULL; }
opt_implements(I) ::= TK_IMPLEMENTS implements_list(L). { I = L; }

implements_list(I) ::= type_def(T).
{
  I = ast_ft(context, IMPLEMENTS);
  add_ast(I, T);
}
implements_list(I) ::= implements_list(L) type_def(T).
{
  I = L;
  add_ast(I, T);
}

interface_def(C) ::= TK_LABEL(L) TK_INTERFACE.
{
  C = ast_f(context, INTERFACE_DEF, L);
}
interface_def(C) ::= TK_RESERVED_LABEL(L) TK_INTERFACE.
{
  C = mknd_err(ast_f(context, VAR_SYMBOL, L), "KEYWORD");
}

factory_def(F) ::= TK_MULT_LABEL(L) TK_FACTORY opt_method_return_type(T).
{
  F = ast_f(context, FACTORY, L);
  add_ast(F, ast_ft(context, VOID));
  if (T) mknd_err(F, "FACTORY_RETURN_TYPE_NOT_ALLOWED");
}

factory_def(F) ::= TK_LABEL(L) TK_FACTORY opt_method_return_type(T).
{
  F = ast_f(context, FACTORY, L);
  add_ast(F, ast_ft(context, VOID));
  if (T) mknd_err(F, "FACTORY_RETURN_TYPE_NOT_ALLOWED");
}
factory_def(F) ::= TK_RESERVED_LABEL(L) TK_FACTORY opt_method_return_type.
{
  F = mknd_err(ast_f(context, VAR_SYMBOL, L), "KEYWORD");
}

match_def(M) ::= TK_MULT_LABEL(L) TK_MATCH.
{
  M = ast_f(context, MATCH, L);
  add_ast(M, ast_ftt(context, CLASS, ".int"));
}

match_def(M) ::= TK_LABEL(L) TK_MATCH.
{
  M = ast_f(context, MATCH, L);
  add_ast(M, ast_ftt(context, CLASS, ".int"));
}
match_def(M) ::= TK_RESERVED_LABEL(L) TK_MATCH.
{
  M = mknd_err(ast_f(context, VAR_SYMBOL, L), "KEYWORD");
}

method_def(M) ::= TK_LABEL(L) TK_METHOD opt_method_return_type(T).
{
  M = ast_f(context, METHOD, L);
  if (T) add_ast(M, T);
  else add_ast(M, ast_ft(context, VOID));
}
method_def(M) ::= TK_RESERVED_LABEL(L) TK_METHOD opt_method_return_type.
{
  M = mknd_err(ast_f(context, VAR_SYMBOL, L), "KEYWORD");
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
opt_register_attribute(A) ::= TK_STEMVAR(T) TK_STEMVAR(P).
{
  char *combined;
  size_t length;

  /* Remove leading dot from .attribute.partition */
  T->token_string++;
  T->length--;
  P->token_string++;
  P->length--;
  A = ast_f(context, VAR_SYMBOL, T);
  length = (size_t)T->length + 1 + (size_t)P->length;
  combined = malloc(length + 1);
  if (combined) {
    memcpy(combined, T->token_string, (size_t)T->length);
    combined[T->length] = '.';
    memcpy(combined + T->length + 1, P->token_string, (size_t)P->length);
    combined[length] = 0;
    ast_sstr(A, combined, length);
  }
}
