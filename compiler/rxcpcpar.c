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
 * Level C Classic REXX tracer parser glue.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpbgmr.h"
#include "rxcpcgmr.h"
#include "rxcpmain.h"
#include "rxcp_util.h"

enum {
    TK_LEVELC_ADDRESS = 50000,
    TK_LEVELC_DROP,
    TK_LEVELC_INTERPRET,
    TK_LEVELC_PARSE,
    TK_LEVELC_PULL,
    TK_LEVELC_PUSH,
    TK_LEVELC_QUEUE,
    TK_LEVELC_TRACE,
    TK_LEVELC_UPPER,
    TK_LEVELC_SOURCE,
    TK_LEVELC_LINEIN,
    TK_LEVELC_VERSION,
    TK_LEVELC_VALUE,
    TK_LEVELC_VAR,
    TK_LEVELC_WITH,
    TK_LEVELC_EXPOSE,
    TK_LEVELC_DIGITS,
    TK_LEVELC_FORM,
    TK_LEVELC_FUZZ,
    TK_LEVELC_ENGINEERING,
    TK_LEVELC_SCIENTIFIC,
    TK_LEVELC_OFF,
    TK_LEVELC_NAME,
    TK_LEVELC_ERROR,
    TK_LEVELC_FAILURE,
    TK_LEVELC_HALT,
    TK_LEVELC_NOTREADY,
    TK_LEVELC_NOVALUE,
    TK_LEVELC_SYNTAX,
    TK_LEVELC_LOSTDIGITS,
    TK_LEVELC_INPUT,
    TK_LEVELC_OUTPUT,
    TK_LEVELC_STREAM,
    TK_LEVELC_STEM,
    TK_LEVELC_NORMAL,
    TK_LEVELC_APPEND,
    TK_LEVELC_REPLACE
};

typedef enum {
    LEVELC_TAIL_NONE = 0,
    LEVELC_TAIL_PARSE,
    LEVELC_TAIL_NUMERIC,
    LEVELC_TAIL_PROCEDURE,
    LEVELC_TAIL_CALL,
    LEVELC_TAIL_SIGNAL,
    LEVELC_TAIL_ADDRESS,
    LEVELC_TAIL_TRACE
} LevelCTailMode;

typedef enum {
    LEVELC_PARSE_TYPE = 0,
    LEVELC_PARSE_VALUE_EXPR,
    LEVELC_PARSE_VAR_TARGET,
    LEVELC_PARSE_TEMPLATE
} LevelCParseStage;

typedef enum {
    LEVELC_TAIL_STAGE_HEAD = 0,
    LEVELC_TAIL_STAGE_AFTER_HEAD,
    LEVELC_TAIL_STAGE_AFTER_FORM,
    LEVELC_TAIL_STAGE_AFTER_ONOFF,
    LEVELC_TAIL_STAGE_AFTER_CONDITION,
    LEVELC_TAIL_STAGE_AFTER_WITH,
    LEVELC_TAIL_STAGE_PLAIN
} LevelCTailStage;

static int levelc_token_is(Token *token, const char *text) {
    int i;

    if (!token || !token->token_string || token->length != (int)strlen(text)) return 0;
    for (i = 0; i < token->length; i++) {
        if (toupper((unsigned char)token->token_string[i]) != toupper((unsigned char)text[i])) return 0;
    }
    return 1;
}

static int levelc_promote_token(Token *token, int token_type, int parser_token) {
    token->token_type = token_type;
    return parser_token;
}

static int levelc_parser_token_for_raw(int token_type) {
    switch (token_type) {
        case TK_BADCOMMENT: return CTK_BADCOMMENT;
        case TK_EOS: return CTK_EOS;
        case TK_EOC:
        case TK_EOL: return CTK_EOC;
        case TK_VAR_SYMBOL: return CTK_VAR_SYMBOL;
        case TK_LABEL: return CTK_LABEL;
        case TK_INTEGER: return CTK_INTEGER;
        case TK_STRING: return CTK_STRING;
        case TK_DOT: return CTK_DOT;
        case TK_EQUAL: return CTK_EQUAL;
        case TK_COMMA: return CTK_COMMA;
        case TK_OPEN_BRACKET: return CTK_OPEN_BRACKET;
        case TK_CLOSE_BRACKET: return CTK_CLOSE_BRACKET;
        case TK_PLUS: return CTK_PLUS;
        case TK_MINUS: return CTK_MINUS;
        case TK_HIGH_PRIORITY_MINUS: return CTK_HIGH_PRIORITY_MINUS;
        case TK_NOT: return CTK_NOT;
        case TK_CONCAT: return CTK_CONCAT;
        case TK_MULT: return CTK_MULT;
        case TK_DIV: return CTK_DIV;
        case TK_IDIV: return CTK_IDIV;
        case TK_MOD: return CTK_MOD;
        case TK_POWER_L:
        case TK_POWER_R: return CTK_POWER;
        case TK_NEQ: return CTK_NEQ;
        case TK_GT: return CTK_GT;
        case TK_LT: return CTK_LT;
        case TK_GTE: return CTK_GTE;
        case TK_LTE: return CTK_LTE;
        case TK_S_EQ: return CTK_S_EQ;
        case TK_S_NEQ: return CTK_S_NEQ;
        case TK_S_GT: return CTK_S_GT;
        case TK_S_LT: return CTK_S_LT;
        case TK_S_GTE: return CTK_S_GTE;
        case TK_S_LTE: return CTK_S_LTE;
        case TK_AND: return CTK_AND;
        case TK_OR: return CTK_OR;
        case TK_XOR: return CTK_XOR;
        default: return CTK_UNKNOWN;
    }
}

static int levelc_promote_clause_keyword(Token *token) {
    if (levelc_token_is(token, "ADDRESS")) {
        return levelc_promote_token(token, TK_LEVELC_ADDRESS, CTK_ADDRESS);
    }
    if (levelc_token_is(token, "ARG")) {
        return levelc_promote_token(token, TK_ARG, CTK_ARG);
    }
    if (levelc_token_is(token, "CALL")) {
        return levelc_promote_token(token, TK_CALL, CTK_CALL);
    }
    if (levelc_token_is(token, "DROP")) {
        return levelc_promote_token(token, TK_LEVELC_DROP, CTK_DROP);
    }
    if (levelc_token_is(token, "EXIT")) {
        return levelc_promote_token(token, TK_EXIT, CTK_EXIT);
    }
    if (levelc_token_is(token, "INTERPRET")) {
        return levelc_promote_token(token, TK_LEVELC_INTERPRET, CTK_INTERPRET);
    }
    if (levelc_token_is(token, "NOP")) {
        return levelc_promote_token(token, TK_NOP, CTK_NOP);
    }
    if (levelc_token_is(token, "NUMERIC")) {
        return levelc_promote_token(token, TK_NUMERIC, CTK_NUMERIC);
    }
    if (levelc_token_is(token, "OPTIONS")) {
        return levelc_promote_token(token, TK_OPTIONS, CTK_OPTIONS);
    }
    if (levelc_token_is(token, "PARSE")) {
        return levelc_promote_token(token, TK_LEVELC_PARSE, CTK_PARSE);
    }
    if (levelc_token_is(token, "PROCEDURE")) {
        return levelc_promote_token(token, TK_PROCEDURE, CTK_PROCEDURE);
    }
    if (levelc_token_is(token, "PULL")) {
        return levelc_promote_token(token, TK_LEVELC_PULL, CTK_PULL);
    }
    if (levelc_token_is(token, "PUSH")) {
        return levelc_promote_token(token, TK_LEVELC_PUSH, CTK_PUSH);
    }
    if (levelc_token_is(token, "QUEUE")) {
        return levelc_promote_token(token, TK_LEVELC_QUEUE, CTK_QUEUE);
    }
    if (levelc_token_is(token, "RETURN")) {
        return levelc_promote_token(token, TK_RETURN, CTK_RETURN);
    }
    if (levelc_token_is(token, "SAY")) {
        return levelc_promote_token(token, TK_SAY, CTK_SAY);
    }
    if (levelc_token_is(token, "SIGNAL")) {
        return levelc_promote_token(token, TK_SIGNAL, CTK_SIGNAL);
    }
    if (levelc_token_is(token, "TRACE")) {
        return levelc_promote_token(token, TK_LEVELC_TRACE, CTK_TRACE);
    }
    if (levelc_token_is(token, "IF")) {
        return levelc_promote_token(token, TK_IF, CTK_IF);
    }
    if (levelc_token_is(token, "SELECT")) {
        return levelc_promote_token(token, TK_SELECT, CTK_SELECT);
    }
    if (levelc_token_is(token, "WHEN")) {
        return levelc_promote_token(token, TK_WHEN, CTK_WHEN);
    }
    if (levelc_token_is(token, "THEN")) {
        return levelc_promote_token(token, TK_THEN, CTK_THEN);
    }
    if (levelc_token_is(token, "ELSE")) {
        return levelc_promote_token(token, TK_ELSE, CTK_ELSE);
    }
    if (levelc_token_is(token, "OTHERWISE")) {
        return levelc_promote_token(token, TK_OTHERWISE, CTK_OTHERWISE);
    }
    if (levelc_token_is(token, "DO")) {
        return levelc_promote_token(token, TK_DO, CTK_DO);
    }
    if (levelc_token_is(token, "END")) {
        return levelc_promote_token(token, TK_END, CTK_END);
    }
    if (levelc_token_is(token, "LEAVE")) {
        return levelc_promote_token(token, TK_LEAVE, CTK_LEAVE);
    }
    if (levelc_token_is(token, "ITERATE")) {
        return levelc_promote_token(token, TK_ITERATE, CTK_ITERATE);
    }
    return CTK_VAR_SYMBOL;
}

static int levelc_promote_parse_keyword(Token *token, LevelCParseStage stage) {
    if (stage == LEVELC_PARSE_TYPE) {
        if (levelc_token_is(token, "UPPER")) {
            return levelc_promote_token(token, TK_LEVELC_UPPER, CTK_UPPER);
        }
        if (levelc_token_is(token, "ARG")) {
            return levelc_promote_token(token, TK_ARG, CTK_ARG);
        }
        if (levelc_token_is(token, "PULL")) {
            return levelc_promote_token(token, TK_LEVELC_PULL, CTK_PULL);
        }
        if (levelc_token_is(token, "SOURCE")) {
            return levelc_promote_token(token, TK_LEVELC_SOURCE, CTK_SOURCE);
        }
        if (levelc_token_is(token, "LINEIN")) {
            return levelc_promote_token(token, TK_LEVELC_LINEIN, CTK_LINEIN);
        }
        if (levelc_token_is(token, "VERSION")) {
            return levelc_promote_token(token, TK_LEVELC_VERSION, CTK_VERSION);
        }
        if (levelc_token_is(token, "VALUE")) {
            return levelc_promote_token(token, TK_LEVELC_VALUE, CTK_VALUE);
        }
        if (levelc_token_is(token, "VAR")) {
            return levelc_promote_token(token, TK_LEVELC_VAR, CTK_VAR);
        }
    }
    if (stage == LEVELC_PARSE_VALUE_EXPR && levelc_token_is(token, "WITH")) {
        return levelc_promote_token(token, TK_LEVELC_WITH, CTK_WITH);
    }
    return CTK_VAR_SYMBOL;
}

static int levelc_promote_condition_keyword(Token *token, int signal_conditions) {
    if (levelc_token_is(token, "ERROR")) {
        return levelc_promote_token(token, TK_LEVELC_ERROR, CTK_ERROR);
    }
    if (levelc_token_is(token, "FAILURE")) {
        return levelc_promote_token(token, TK_LEVELC_FAILURE, CTK_FAILURE);
    }
    if (levelc_token_is(token, "HALT")) {
        return levelc_promote_token(token, TK_LEVELC_HALT, CTK_HALT);
    }
    if (levelc_token_is(token, "NOTREADY")) {
        return levelc_promote_token(token, TK_LEVELC_NOTREADY, CTK_NOTREADY);
    }
    if (signal_conditions && levelc_token_is(token, "NOVALUE")) {
        return levelc_promote_token(token, TK_LEVELC_NOVALUE, CTK_NOVALUE);
    }
    if (signal_conditions && levelc_token_is(token, "SYNTAX")) {
        return levelc_promote_token(token, TK_LEVELC_SYNTAX, CTK_SYNTAX);
    }
    if (signal_conditions && levelc_token_is(token, "LOSTDIGITS")) {
        return levelc_promote_token(token, TK_LEVELC_LOSTDIGITS, CTK_LOSTDIGITS);
    }
    return CTK_VAR_SYMBOL;
}

static int levelc_promote_tail_keyword(Token *token,
                                       LevelCTailMode mode,
                                       LevelCTailStage stage) {
    if (mode == LEVELC_TAIL_PARSE) return CTK_VAR_SYMBOL;

    if (mode == LEVELC_TAIL_NUMERIC) {
        if (stage == LEVELC_TAIL_STAGE_HEAD) {
            if (levelc_token_is(token, "DIGITS")) {
                return levelc_promote_token(token, TK_LEVELC_DIGITS, CTK_DIGITS);
            }
            if (levelc_token_is(token, "FORM")) {
                return levelc_promote_token(token, TK_LEVELC_FORM, CTK_FORM);
            }
            if (levelc_token_is(token, "FUZZ")) {
                return levelc_promote_token(token, TK_LEVELC_FUZZ, CTK_FUZZ);
            }
        }
        if (stage == LEVELC_TAIL_STAGE_AFTER_FORM) {
            if (levelc_token_is(token, "ENGINEERING")) {
                return levelc_promote_token(token, TK_LEVELC_ENGINEERING, CTK_ENGINEERING);
            }
            if (levelc_token_is(token, "SCIENTIFIC")) {
                return levelc_promote_token(token, TK_LEVELC_SCIENTIFIC, CTK_SCIENTIFIC);
            }
            if (levelc_token_is(token, "VALUE")) {
                return levelc_promote_token(token, TK_LEVELC_VALUE, CTK_VALUE);
            }
        }
        return CTK_VAR_SYMBOL;
    }

    if (mode == LEVELC_TAIL_PROCEDURE) {
        if (stage == LEVELC_TAIL_STAGE_HEAD && levelc_token_is(token, "EXPOSE")) {
            return levelc_promote_token(token, TK_LEVELC_EXPOSE, CTK_EXPOSE);
        }
        return CTK_VAR_SYMBOL;
    }

    if (mode == LEVELC_TAIL_CALL || mode == LEVELC_TAIL_SIGNAL) {
        if (stage == LEVELC_TAIL_STAGE_HEAD) {
            if (levelc_token_is(token, "ON")) {
                return levelc_promote_token(token, TK_ON, CTK_ON);
            }
            if (levelc_token_is(token, "OFF")) {
                return levelc_promote_token(token, TK_LEVELC_OFF, CTK_OFF);
            }
            if (mode == LEVELC_TAIL_SIGNAL && levelc_token_is(token, "VALUE")) {
                return levelc_promote_token(token, TK_LEVELC_VALUE, CTK_VALUE);
            }
        }
        if (stage == LEVELC_TAIL_STAGE_AFTER_ONOFF) {
            return levelc_promote_condition_keyword(token, mode == LEVELC_TAIL_SIGNAL);
        }
        if (stage == LEVELC_TAIL_STAGE_AFTER_CONDITION && levelc_token_is(token, "NAME")) {
            return levelc_promote_token(token, TK_LEVELC_NAME, CTK_NAME);
        }
        return CTK_VAR_SYMBOL;
    }

    if (mode == LEVELC_TAIL_ADDRESS) {
        if (stage == LEVELC_TAIL_STAGE_HEAD && levelc_token_is(token, "VALUE")) {
            return levelc_promote_token(token, TK_LEVELC_VALUE, CTK_VALUE);
        }
        if (levelc_token_is(token, "WITH")) {
            return levelc_promote_token(token, TK_LEVELC_WITH, CTK_WITH);
        }
        if (stage == LEVELC_TAIL_STAGE_AFTER_WITH) {
            if (levelc_token_is(token, "INPUT")) {
                return levelc_promote_token(token, TK_LEVELC_INPUT, CTK_INPUT);
            }
            if (levelc_token_is(token, "OUTPUT")) {
                return levelc_promote_token(token, TK_LEVELC_OUTPUT, CTK_OUTPUT);
            }
            if (levelc_token_is(token, "ERROR")) {
                return levelc_promote_token(token, TK_LEVELC_ERROR, CTK_ERROR);
            }
            if (levelc_token_is(token, "STREAM")) {
                return levelc_promote_token(token, TK_LEVELC_STREAM, CTK_STREAM);
            }
            if (levelc_token_is(token, "STEM")) {
                return levelc_promote_token(token, TK_LEVELC_STEM, CTK_STEM);
            }
            if (levelc_token_is(token, "NORMAL")) {
                return levelc_promote_token(token, TK_LEVELC_NORMAL, CTK_NORMAL);
            }
            if (levelc_token_is(token, "APPEND")) {
                return levelc_promote_token(token, TK_LEVELC_APPEND, CTK_APPEND);
            }
            if (levelc_token_is(token, "REPLACE")) {
                return levelc_promote_token(token, TK_LEVELC_REPLACE, CTK_REPLACE);
            }
        }
        return CTK_VAR_SYMBOL;
    }

    if (mode == LEVELC_TAIL_TRACE && stage == LEVELC_TAIL_STAGE_HEAD &&
        levelc_token_is(token, "VALUE")) {
        return levelc_promote_token(token, TK_LEVELC_VALUE, CTK_VALUE);
    }

    return CTK_VAR_SYMBOL;
}

static int levelc_promote_do_keyword(Token *token, int first_header_token) {
    if (levelc_token_is(token, "TO")) {
        token->token_type = TK_TO;
        return CTK_TO;
    }
    if (levelc_token_is(token, "BY")) {
        token->token_type = TK_BY;
        return CTK_BY;
    }
    if (levelc_token_is(token, "FOR")) {
        token->token_type = TK_FOR;
        return CTK_FOR;
    }
    if (levelc_token_is(token, "WHILE")) {
        token->token_type = TK_WHILE;
        return CTK_WHILE;
    }
    if (levelc_token_is(token, "UNTIL")) {
        token->token_type = TK_UNTIL;
        return CTK_UNTIL;
    }
    if (first_header_token && levelc_token_is(token, "FOREVER")) {
        token->token_type = TK_FOREVER;
        return CTK_FOREVER;
    }
    return CTK_VAR_SYMBOL;
}

static int levelc_missing_expression_boundary(int parser_token) {
    return parser_token == CTK_EOC ||
           parser_token == CTK_EOS ||
           parser_token == CTK_CLOSE_BRACKET ||
           parser_token == CTK_WITH;
}

static int levelc_token_expects_expression_rhs(int parser_token) {
    switch (parser_token) {
        case CTK_EQUAL:
        case CTK_PLUS:
        case CTK_MINUS:
        case CTK_HIGH_PRIORITY_MINUS:
        case CTK_NOT:
        case CTK_CONCAT:
        case CTK_MULT:
        case CTK_DIV:
        case CTK_IDIV:
        case CTK_MOD:
        case CTK_POWER:
        case CTK_NEQ:
        case CTK_GT:
        case CTK_LT:
        case CTK_GTE:
        case CTK_LTE:
        case CTK_S_EQ:
        case CTK_S_NEQ:
        case CTK_S_GT:
        case CTK_S_LT:
        case CTK_S_GTE:
        case CTK_S_LTE:
        case CTK_AND:
        case CTK_OR:
        case CTK_XOR:
            return 1;
        default:
            return 0;
    }
}

int rexcpars(Context *context) {
    Token *token;
    Token *peek_token;
    Token *t;
    void *parser;
    int token_type;
    int parser_token;
    int last_parser_token;
    int clause_start;
    int if_condition;
    int when_condition;
    int pending_else;
    int do_header;
    int do_header_first;
    int do_condition_expr;
    int paren_depth;
    LevelCTailMode tail_mode;
    LevelCParseStage parse_stage;
    LevelCTailStage tail_stage;

    context->numeric_standard = 1;
    parser = RexxCAlloc(malloc);
#ifndef NDEBUG
    if (context->debug_mode >= 2) RexxCTrace(stderr, "[PARSER-C] ");
    else RexxCTrace(context->traceFile, "Parser(C) >> ");
#endif

    peek_token = token_f(context, rexcscan(context));
    last_parser_token = CTK_EOC;
    clause_start = 1;
    if_condition = 0;
    when_condition = 0;
    pending_else = 0;
    do_header = 0;
    do_header_first = 0;
    do_condition_expr = 0;
    paren_depth = 0;
    tail_mode = LEVELC_TAIL_NONE;
    parse_stage = LEVELC_PARSE_TYPE;
    tail_stage = LEVELC_TAIL_STAGE_HEAD;

    while (1) {
        token = peek_token;
        if (token->token_type == TK_EOL) token->token_type = TK_EOC;
        token_type = token->token_type;

        if (token_type == TK_EOS || token_type == TK_BADCOMMENT) {
            if (last_parser_token != CTK_EOC) {
                context->current_parser_token = token;
                context->next_parser_token = token;
                if (levelc_missing_expression_boundary(CTK_EOC) &&
                    levelc_token_expects_expression_rhs(last_parser_token)) {
                    RexxC(parser, CTK_MISSING_EXPR, token, context);
                }
                while (paren_depth > 0) {
                    RexxC(parser, CTK_MISSING_RPAREN, token, context);
                    paren_depth--;
                }
                RexxC(parser, CTK_EOC, 0, context);
            }
            if (token_type == TK_BADCOMMENT) {
                RexxC(parser, CTK_BADCOMMENT, token, context);
                t = token_f(context, TK_EOC);
                RexxC(parser, CTK_EOC, t, context);
            }
            t = token_f(context, TK_EOS);
            RexxC(parser, CTK_EOS, t, context);
            RexxC(parser, 0, NULL, context);
            break;
        }

        peek_token = token_f(context, rexcscan(context));

        if (token_type == TK_COMMA && peek_token->token_type == TK_EOL) {
            token_r(context);
            token_r(context);
            peek_token = token_f(context, rexcscan(context));
            continue;
        }

        if (token_type == TK_EOC &&
            (last_parser_token == CTK_THEN || last_parser_token == CTK_ELSE ||
             last_parser_token == CTK_OTHERWISE) &&
            peek_token->token_type != TK_EOS) {
            continue;
        }

        if (token_type == TK_EOC &&
            pending_else &&
            peek_token->token_type == TK_VAR_SYMBOL &&
            levelc_token_is(peek_token, "ELSE")) {
            continue;
        }

        if (token_type == TK_EOC && last_parser_token == CTK_EOC) continue;

        parser_token = levelc_parser_token_for_raw(token_type);

        if (token_type == TK_VAR_SYMBOL) {
            if (if_condition && levelc_token_is(token, "THEN")) {
                token->token_type = TK_THEN;
                parser_token = CTK_THEN;
                if_condition = 0;
                pending_else++;
                clause_start = 1;
            }
            else if (when_condition && levelc_token_is(token, "THEN")) {
                token->token_type = TK_THEN;
                parser_token = CTK_THEN;
                when_condition = 0;
                clause_start = 1;
            }
            else if (pending_else && levelc_token_is(token, "ELSE") && peek_token->token_type != TK_EQUAL) {
                token->token_type = TK_ELSE;
                parser_token = CTK_ELSE;
                pending_else--;
                clause_start = 1;
            }
            else if (tail_mode == LEVELC_TAIL_PARSE) {
                parser_token = levelc_promote_parse_keyword(token, parse_stage);
            }
            else if (tail_mode != LEVELC_TAIL_NONE) {
                parser_token = levelc_promote_tail_keyword(token, tail_mode, tail_stage);
            }
            else if (do_header && do_header_first && !do_condition_expr && peek_token->token_type == TK_EQUAL) {
                parser_token = CTK_DO_CONTROL_SYMBOL;
            }
            else if (do_header && !do_condition_expr && peek_token->token_type != TK_EQUAL) {
                parser_token = levelc_promote_do_keyword(token, do_header_first);
            }
            else if (clause_start && peek_token->token_type != TK_EQUAL) {
                parser_token = levelc_promote_clause_keyword(token);
                if (parser_token == CTK_IF) {
                    if_condition = 1;
                    clause_start = 0;
                }
                else if (parser_token == CTK_WHEN) {
                    when_condition = 1;
                    clause_start = 0;
                }
                else if (parser_token == CTK_SAY) {
                    clause_start = 0;
                }
                else if (parser_token == CTK_VAR_SYMBOL) {
                    parser_token = CTK_COMMAND_SYMBOL;
                }
            }
        }

        if (parser_token == CTK_EOC || parser_token == CTK_LABEL) {
            clause_start = 1;
            if_condition = 0;
            when_condition = 0;
            do_header = 0;
            do_header_first = 0;
            do_condition_expr = 0;
            tail_mode = LEVELC_TAIL_NONE;
            parse_stage = LEVELC_PARSE_TYPE;
            tail_stage = LEVELC_TAIL_STAGE_HEAD;
        }
        else if (parser_token != CTK_THEN &&
                 parser_token != CTK_ELSE &&
                 parser_token != CTK_OTHERWISE) {
            clause_start = 0;
        }

        if (parser_token == CTK_DO) {
            do_header = 1;
            do_header_first = 1;
            do_condition_expr = 0;
        }
        else if (parser_token == CTK_PARSE) {
            tail_mode = LEVELC_TAIL_PARSE;
            parse_stage = LEVELC_PARSE_TYPE;
            tail_stage = LEVELC_TAIL_STAGE_HEAD;
        }
        else if (parser_token == CTK_NUMERIC) {
            tail_mode = LEVELC_TAIL_NUMERIC;
            tail_stage = LEVELC_TAIL_STAGE_HEAD;
        }
        else if (parser_token == CTK_PROCEDURE) {
            tail_mode = LEVELC_TAIL_PROCEDURE;
            tail_stage = LEVELC_TAIL_STAGE_HEAD;
        }
        else if (parser_token == CTK_CALL) {
            tail_mode = LEVELC_TAIL_CALL;
            tail_stage = LEVELC_TAIL_STAGE_HEAD;
        }
        else if (parser_token == CTK_SIGNAL) {
            tail_mode = LEVELC_TAIL_SIGNAL;
            tail_stage = LEVELC_TAIL_STAGE_HEAD;
        }
        else if (parser_token == CTK_ADDRESS) {
            tail_mode = LEVELC_TAIL_ADDRESS;
            tail_stage = LEVELC_TAIL_STAGE_HEAD;
        }
        else if (parser_token == CTK_TRACE) {
            tail_mode = LEVELC_TAIL_TRACE;
            tail_stage = LEVELC_TAIL_STAGE_HEAD;
        }
        else if (tail_mode == LEVELC_TAIL_PARSE) {
            if (parse_stage == LEVELC_PARSE_TYPE) {
                if (parser_token == CTK_VALUE) parse_stage = LEVELC_PARSE_VALUE_EXPR;
                else if (parser_token == CTK_VAR) parse_stage = LEVELC_PARSE_VAR_TARGET;
                else if (parser_token == CTK_ARG ||
                         parser_token == CTK_PULL ||
                         parser_token == CTK_SOURCE ||
                         parser_token == CTK_LINEIN ||
                         parser_token == CTK_VERSION) {
                    parse_stage = LEVELC_PARSE_TEMPLATE;
                }
            }
            else if (parse_stage == LEVELC_PARSE_VALUE_EXPR && parser_token == CTK_WITH) {
                parse_stage = LEVELC_PARSE_TEMPLATE;
            }
            else if (parse_stage == LEVELC_PARSE_VAR_TARGET && parser_token == CTK_VAR_SYMBOL) {
                parse_stage = LEVELC_PARSE_TEMPLATE;
            }
        }
        else if (tail_mode == LEVELC_TAIL_NUMERIC) {
            if (tail_stage == LEVELC_TAIL_STAGE_HEAD) {
                if (parser_token == CTK_FORM) tail_stage = LEVELC_TAIL_STAGE_AFTER_FORM;
                else if (parser_token == CTK_DIGITS || parser_token == CTK_FUZZ) tail_stage = LEVELC_TAIL_STAGE_PLAIN;
                else if (parser_token != CTK_NUMERIC) tail_stage = LEVELC_TAIL_STAGE_PLAIN;
            }
            else if (tail_stage == LEVELC_TAIL_STAGE_AFTER_FORM && parser_token != CTK_FORM) {
                tail_stage = LEVELC_TAIL_STAGE_PLAIN;
            }
        }
        else if (tail_mode == LEVELC_TAIL_PROCEDURE) {
            if (tail_stage == LEVELC_TAIL_STAGE_HEAD && parser_token != CTK_PROCEDURE) {
                tail_stage = parser_token == CTK_EXPOSE ? LEVELC_TAIL_STAGE_AFTER_HEAD : LEVELC_TAIL_STAGE_PLAIN;
            }
        }
        else if (tail_mode == LEVELC_TAIL_CALL || tail_mode == LEVELC_TAIL_SIGNAL) {
            if (tail_stage == LEVELC_TAIL_STAGE_HEAD) {
                if (parser_token == CTK_ON || parser_token == CTK_OFF) tail_stage = LEVELC_TAIL_STAGE_AFTER_ONOFF;
                else if (parser_token != CTK_CALL && parser_token != CTK_SIGNAL) tail_stage = LEVELC_TAIL_STAGE_PLAIN;
            }
            else if (tail_stage == LEVELC_TAIL_STAGE_AFTER_ONOFF) {
                tail_stage = LEVELC_TAIL_STAGE_AFTER_CONDITION;
            }
            else if (tail_stage == LEVELC_TAIL_STAGE_AFTER_CONDITION && parser_token == CTK_NAME) {
                tail_stage = LEVELC_TAIL_STAGE_PLAIN;
            }
        }
        else if (tail_mode == LEVELC_TAIL_ADDRESS) {
            if (parser_token == CTK_WITH) tail_stage = LEVELC_TAIL_STAGE_AFTER_WITH;
            else if (tail_stage == LEVELC_TAIL_STAGE_HEAD && parser_token != CTK_ADDRESS) {
                tail_stage = LEVELC_TAIL_STAGE_AFTER_HEAD;
            }
        }
        else if (tail_mode == LEVELC_TAIL_TRACE) {
            if (tail_stage == LEVELC_TAIL_STAGE_HEAD && parser_token != CTK_TRACE) {
                tail_stage = LEVELC_TAIL_STAGE_PLAIN;
            }
        }
        else if (parser_token == CTK_WHILE || parser_token == CTK_UNTIL) {
            do_condition_expr = 1;
        }
        else if (do_header && parser_token != CTK_EOC) {
            do_header_first = 0;
        }

        if (parser_token != CTK_EOC && last_parser_token == CTK_EOC) {
            context->current_clause_token = token;
        }
        context->current_parser_token = token;
        context->next_parser_token = peek_token;
        if (context->debug_mode >= 2) {
            fprintf(stderr, "[GLUE-C] Line %d: Passing raw %d (%s) as parser token %d\n",
                    context->line, token_type, token_to_string(token_type), parser_token);
        }
        if (levelc_missing_expression_boundary(parser_token) &&
            levelc_token_expects_expression_rhs(last_parser_token)) {
            RexxC(parser, CTK_MISSING_EXPR, token, context);
        }
        if ((parser_token == CTK_EOC || parser_token == CTK_EOS) && paren_depth > 0) {
            while (paren_depth > 0) {
                RexxC(parser, CTK_MISSING_RPAREN, token, context);
                paren_depth--;
            }
        }
        RexxC(parser, parser_token, token, context);
        if (parser_token == CTK_OPEN_BRACKET) {
            paren_depth++;
        }
        else if (parser_token == CTK_CLOSE_BRACKET && paren_depth > 0) {
            paren_depth--;
        }
        last_parser_token = parser_token;
    }

    RexxCFree(parser, free);
    if (!context->ast) rxcp_levelc_run_fallback_diagnostics(context);
    return 0;
}
