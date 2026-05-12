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
 * Level C Classic REXX tracer scanner.
 *
 * Scanner rule: words are neutral symbols. The Level C parser glue owns
 * contextual keyword promotion/demotion.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpbgmr.h"
#include "rxcpmain.h"
#include "rxcp_util.h"

#define YYCTYPE     unsigned char
#define YYCURSOR    s->cursor
#define YYLIMIT     s->buff_end
#define YYMARKER    s->marker
#define YYCTXMARKER s->ctxmarker

int rexcscan(Context* s) {
  int depth;
  char *comment_top;
  int comment_line;
  char* comment_linestart;

#define RET(id) do { \
    int tid = (id); \
    if (s->debug_mode >= 2) { \
        fprintf(stderr, "[LEX-C] Line %d: Token %d (%s) Value: '%.*s'\n", \
            s->line, tid, token_to_string(tid), (int)(s->cursor - s->top), s->top); \
    } \
    return tid; \
} while(0)

/*!re2c
  re2c:yyfill:enable = 0;
  re2c:eof = 0;
*/

/* Character Encoding Specifics */
/*!include:re2c "encoding.re" */

regular:

/*!re2c
  re2c:yyfill:enable = 0;
  re2c:eof = 0;

  eol2 = "\r\n";
  eol1 = [\r] | [\n];
  eof = [\000];
  any = [^] \ eof;
  digit = [0-9];
  not = "\\";
  lcfsymchr = letter | [_!?];
  lcsymchr = lcfsymchr | digit | [.];
  lcsimple = lcfsymchr lcsymchr*;
  integer = digit+;
  sqstr = ['] ((any\['\n\r])|(['][']))* ['];
  dqstr = ["] ((any\["\n\r])|(["]["]))* ["];
  badsqstr = ['] ((any\['\n\r])|(['][']))*;
  baddqstr = ["] ((any\["\n\r])|(["]["]))*;
  str = sqstr | dqstr;
  ob = [ \t]*;
  lcspace = [ \t\v\f]+;
*/

/*!re2c
    "/*" {
      depth = 1;
      comment_line = s->line;
      comment_linestart = s->linestart;
      comment_top = s->top;
      goto comment;
    }

    "|" ob "|" { RET(TK_CONCAT); }
    "*" ob "*" { RET(TK_POWER_L); }
    "/" ob "/" { RET(TK_MOD); }
    not ob "=" ob "=" { RET(TK_S_NEQ); }
    ">" ob ">" ob "=" | not ob "<" ob "<" { RET(TK_S_GTE); }
    "<" ob "<" ob "=" | not ob ">" ob ">" { RET(TK_S_LTE); }
    "=" ob "=" { RET(TK_S_EQ); }
    not ob "=" | "<" ob ">" | ">" ob "<" { RET(TK_NEQ); }
    ">" ob "=" | not ob "<" { RET(TK_GTE); }
    "<" ob "=" | not ob ">" { RET(TK_LTE); }
    ">" ob ">" { RET(TK_S_GT); }
    "<" ob "<" { RET(TK_S_LT); }
    "&" ob "&" { RET(TK_XOR); }

    "," { RET(TK_COMMA); }
    ";" { RET(TK_EOC); }
    "(" { RET(TK_OPEN_BRACKET); }
    ")" { RET(TK_CLOSE_BRACKET); }
    "=" { RET(TK_EQUAL); }
    "+" { RET(TK_PLUS); }
    "-" { RET(TK_HIGH_PRIORITY_MINUS); }
    "*" { RET(TK_MULT); }
    "/" { RET(TK_DIV); }
    "%" { RET(TK_IDIV); }
    ">" { RET(TK_GT); }
    "<" { RET(TK_LT); }
    "&" { RET(TK_AND); }
    "|" { RET(TK_OR); }
    not { RET(TK_NOT); }

    lcsimple ob ":" { RET(TK_LABEL); }
    integer { RET(TK_INTEGER); }
    str { RET(TK_STRING); }
    str [bBxX] / (any\(lcfsymchr | digit | [.])) { RET(TK_STRING); }
    badsqstr { RET(TK_UNKNOWN); }
    baddqstr { RET(TK_UNKNOWN); }
    lcsimple { RET(TK_VAR_SYMBOL); }

    eol2 {
       s->line++;
       s->prev_linestart = s->linestart;
       s->linestart = s->cursor + 2;
       RET(TK_EOL);
    }
    eol1 {
       s->line++;
       s->prev_linestart = s->linestart;
       s->linestart = s->cursor + 1;
       RET(TK_EOL);
    }
    eof { RET(TK_EOS); }
    $ { RET(TK_EOS); }
    lcspace {
      s->top = s->cursor;
      goto regular;
    }
    "." { RET(TK_DOT); }
    any { RET(TK_UNKNOWN); }
*/

comment:
/*!re2c
  "*/" {
    if(--depth == 0) {
        s->top = s->cursor;
        goto regular;
    }
    else goto comment;
  }
  eol1 {
    s->line++;
    s->prev_linestart = s->linestart;
    s->linestart = s->cursor + 1;
    goto comment;
  }
  eol2 {
    s->line++;
    s->prev_linestart = s->linestart;
    s->linestart = s->cursor + 2;
    goto comment;
  }
  "/*" {
    ++depth;
    goto comment;
  }
  eof {
      s->line = comment_line;
      s->prev_linestart = s->linestart;
      s->linestart = comment_linestart;
      s->top = comment_top;
      s->cursor = s->top + 2;
      RET(TK_BADCOMMENT);
  }
  $ {
      s->line = comment_line;
      s->prev_linestart = s->linestart;
      s->linestart = comment_linestart;
      s->top = comment_top;
      s->cursor = s->top + 2;
      RET(TK_BADCOMMENT);
  }
  any { goto comment; }
*/
}
