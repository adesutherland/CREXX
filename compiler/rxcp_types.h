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
 * Common Compiler Type Definitions
 */

#ifndef CREXX_RXCP_TYPES_H
#define CREXX_RXCP_TYPES_H

#define rxversion "crexx-dev-260110"

#include <stdio.h>
#include "platform.h"
#include "rxvalue.h"

/* Forward declarations */
typedef struct ASTNode ASTNode;
typedef struct Token Token;
typedef struct Scope Scope;
typedef struct Symbol Symbol;
typedef struct SymbolNode SymbolNode;
typedef struct Context Context;
typedef struct OutputFragment OutputFragment;
typedef struct importable_file importable_file;
typedef struct imported_func imported_func;

typedef enum ScopeType {
    SCOPE_UNIVERSE,     /* Rexx Universe (top-level) */
    SCOPE_NAMESPACE,    /* Namespace (from PROGRAM_FILE/IMPORTED_FILE/NAMESPACE) */
    SCOPE_CLASS,        /* Class definition */
    SCOPE_PROCEDURE,    /* Procedure, Method, or Factory */
    SCOPE_LOCAL         /* Local scope for code blocks like DO loops, etc. */
} ScopeType;

typedef enum RexxLevel {
    UNKNOWN, LEVELA, LEVELB, LEVELC, LEVELD, LEVELG, LEVELL
} RexxLevel;

typedef enum ValueType {
    TP_UNKNOWN, TP_VOID, TP_BOOLEAN, TP_INTEGER, TP_FLOAT, TP_DECIMAL, TP_STRING, TP_BINARY, TP_OBJECT
} ValueType;

typedef enum NodeType {
    ABS_POS=1, ADDRESS, IMPLICIT_CMD, ARG, ARGS, ASSEMBLER, ASSIGN, BY, CALL, CLASS, LITERAL, CONST_SYMBOL,
    DEC_DIGITS, DEC_FORM, DEC_FUZZ, DEC_CASE, DEC_STANDARD, DEFINE,
    DO, ENVIRONMENT, ERROR, EXPOSED, EXIT, FOR, FUNCTION, FUNC_SYMBOL, IF, IMPORT, IMPORTED_FILE, INSTRUCTIONS, ITERATE, LABEL, LEAVE,
    FLOAT, INTEGER, OP_MAKE_ARRAY, DECIMAL,
    NAMESPACE, NOP, NOVAL, OP_ADD, OP_MINUS, OP_AND, OP_ARGS, OP_ARG_VALUE, OP_ARG_EXISTS, OP_ARG_IX_EXISTS,
    OP_CONCAT, OP_MULT, OP_DIV, OP_IDIV,
    OP_MOD, OP_OR, OP_POWER, OP_NOT, OP_NEG, OP_PLUS,
    OP_COMPARE_EQUAL, OP_COMPARE_NEQ, OP_COMPARE_GT, OP_COMPARE_LT,
    OP_COMPARE_GTE, OP_COMPARE_LTE, OP_COMPARE_S_EQ, OP_COMPARE_S_NEQ,
    OP_COMPARE_S_GT, OP_COMPARE_S_LT, OP_COMPARE_S_GTE, OP_COMPARE_S_LTE,
    OP_SCONCAT, OPTIONS, PARSE, PATTERN, PROCEDURE, PROGRAM_FILE, PULL, REL_POS, RANGE, REPEAT,
    REDIRECT_IN, REDIRECT_OUT, REDIRECT_ERROR, REDIRECT_EXPOSE,
    RETURN, REXX_OPTIONS, REXX_UNIVERSE, SAY, SIGN, STRING, BINARY, TARGET, TEMPLATES, TO, TOKEN, UPPER,
    VAR_REFERENCE, VAR_SYMBOL, VAR_TARGET, VOID,
    VARG, VARG_REFERENCE, CONSTANT, WARNING, WHILE, UNTIL,
    FACTORY, METHOD, WITH, NODE_REGISTER, OF, CLASS_DEF, MEMBER_CALL, FACTORY_CALL,
    BLOCK_EXPR, LEAVE_WITH, EXIT_EXTENDED, EXIT_TOKEN, SELECT, SWITCH, WHEN, OTHERWISE
} NodeType;

typedef enum SymbolType {
    UNKNOWN_SYMBOL=0, CONSTANT_SYMBOL, VARIABLE_SYMBOL, FUNCTION_SYMBOL, CLASS_SYMBOL, NAMESPACE_SYMBOL
} SymbolType;

typedef enum SymbolStatus {
    SYM_STATUS_UNRESOLVED,     /* Initial state: Name encountered but definition/linkage not yet attempted. */
    SYM_STATUS_LOCAL_DEF,      /* Explicitly defined in the current unit (e.g., PROCEDURE, CLASS). */
    SYM_STATUS_LOCAL_VAR,      /* Inferred as a local variable (e.g., via assignment or typed declaration). */
    SYM_STATUS_RESOLVED_GLOBAL, /* Successfully linked to a global definition (Imported Function/Variable or BIF). */
    SYM_STATUS_PENDING_STUB,   /* Name identified as a required external class stub not yet loaded. */
    SYM_STATUS_AMBIGUOUS,      /* Resolution failed due to multiple conflicting definitions. */
    SYM_STATUS_NOT_FOUND       /* Final state: Name could not be resolved after all attempts. */
} SymbolStatus;

typedef enum walker_direction { in, out } walker_direction;
typedef enum walker_result {
    result_normal, result_abort, result_error, request_skip
} walker_result;

typedef walker_result (*walker_handler)(walker_direction direction,
                                        ASTNode* visited_node, void *payload);

typedef void (*symbol_worker)(Symbol *symbol, void *payload);

typedef enum file_type {
    REXX_FILE, RXBIN_FILE, RXAS_FILE, NATIVE_FILE
} file_type;

#endif //CREXX_RXCP_TYPES_H
