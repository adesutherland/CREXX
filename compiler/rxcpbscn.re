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
 * Compiler Scanner (re2c)
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpbgmr.h"
#include "rxcpmain.h"
#include "rxcp_util.h"

#define   YYCTYPE     unsigned char
#define   YYCURSOR    s->cursor
#define   YYLIMIT     s->buff_end
#define   YYMARKER    s->marker
#define   YYCTXMARKER s->ctxmarker

int rexbscan(Context* s) {
  int depth;
  char *comment_top;
  int comment_line;
  char* comment_linestart;

#define RET(id) do { \
    int tid = (id); \
    if (s->debug_mode >= 2) { \
        fprintf(stderr, "[LEX] Line %d: Token %d (%s) Value: '%.*s'\n", \
            s->line, tid, token_to_string(tid), (int)(s->cursor - s->top), s->top); \
    } \
    return tid; \
} while(0)

/*!re2c
  re2c:yyfill:enable = 0;
  re2c:eof = 0;
*/

/* Character Encoding Specifics  */
/*!include:re2c "encoding.re" */

  if (s->lexer_stem_mode) goto lexer_stem_mode;

  regular:

/*!re2c
  re2c:yyfill:enable = 0;
  re2c:eof = 0;

  eol2 = "\r\n";
  eol1 = [\r] | [\n];
  eof = [\000] ;
  any = [^] \ eof ;
  digit = [0-9];
  hex = [a-fA-F0-9];
  int_des = [uUlL]*;
  fsymchr = letter|[_];    // First Symbol Character
  symchr = fsymchr|digit;  // Subsequent Symbol Character

  // floating literals
  fsig = digit* "." digit+ | digit+ ".";
  fexp = [eE] [+-]? digit+;
  float = (fsig fexp? | digit+ fexp);
  integer = digit+;
  decimal = float [d] | integer [d];
  simple = fsymchr symchr*;
  class = [.] simple;
  sqstr = ['] ((any\['\n\r])|(['][']))* ['];
  dqstr = ["] ((any\["\n\r])|(["]["]))* ["];
  str = sqstr|dqstr;
  ob = [ \t]*;
  not = not_char;
*/

/*!re2c
    // Line Comments
    [#] {
       if (s->comments_hash) goto skip_line_comment;
       else RET(TK_UNKNOWN);
    }
    "//" {
       if (s->comments_slash) goto skip_line_comment;
       else RET(s->numeric_standard ? TK_MOD : TK_UNKNOWN); // numeric_standard: 1 = Classic Standard, 0 = Common Standard
    }
    "--" {
       if (s->comments_dash) goto skip_line_comment;
       else RET(TK_MINUSMINUS);
    }

    // Block Comments
    "/*" {
      depth = 1;
      comment_line = s->line;
      comment_linestart = s->linestart;
      comment_top = s->top;
      goto comment;
    }

    "|" ob "|" { RET(TK_CONCAT); }
    "+" { RET(TK_PLUS); }
    "-" { RET(s->numeric_standard ? TK_HIGH_PRIORITY_MINUS : TK_MINUS); } // numeric_standard: 1 = Classic Standard, 0 = Common Standard
    "*" { RET(TK_MULT); }
    "/" { RET(TK_DIV); }
    "%" { RET(s->numeric_standard ? TK_IDIV : TK_MOD); } // numeric_standard: 1 = Classic Standard, 0 = Common Standard
    "?" { RET(TK_OPTIONAL); }
    "/" ob "/" { RET(s->numeric_standard ? TK_MOD : TK_UNKNOWN); } // numeric_standard: 1 = Classic Standard, 0 = Common Standard
    "*" ob "*" { RET(s->numeric_standard ? TK_POWER_L : TK_POWER_R); } // numeric_standard: 1 = Classic Standard, 0 = Common Standard

    "=" { RET(TK_EQUAL); }
    not ob "=" | "<" ob ">" | ">" ob "<" { RET(TK_NEQ); }
    ">" { RET(TK_GT); }
    "<" { RET(TK_LT); }
    ">" ob "=" | not ob "<" { RET(TK_GTE); }
    "<" ob "=" | not ob ">" { RET(TK_LTE); }
    "=" ob "=" { RET(TK_S_EQ); }
    not ob "=" ob "=" { RET(TK_S_NEQ); }
    ">" ob ">" { RET(TK_S_GT); }
    "<" ob "<" { RET(TK_S_LT); }
    ">" ob ">" ob "=" | not ob "<" ob "<" { RET(TK_S_GTE); }
    "<" ob "<" ob "=" | not ob ">" ob ">" { RET(TK_S_LTE); }

    "&" { RET(TK_AND); }
    "|" { RET(TK_OR); }
  //  "&" ob "&" { RET(TK_OR); } // TODO Check
    not { RET(TK_NOT); }         // Note that '/' will be treated as divide as it is listed first
    "," { RET(TK_COMMA); }
    "..." { RET(TK_ELLIPSIS); }
    "(" { RET(TK_OPEN_BRACKET); }
    ")" { RET(TK_CLOSE_BRACKET); }
    "[" { RET(TK_OPEN_SBRACKET); }
    "]" { RET(TK_CLOSE_SBRACKET); }
    ";" { RET(TK_EOC); }

    'ADDRESS' { RET(TK_ADDRESS); }
    'ASSEMBLER' { RET(TK_ASSEMBLER); }
    'ARG' { RET(TK_ARG); }
    'CALL' { RET(TK_CALL); }
    'CLASS' { RET(TK_CLASS); }
    'DO' { RET(TK_DO); }
    'LOOP' { RET(TK_LOOP); }
    'METHOD' { RET(TK_METHOD); }
  //  'DROP' { RET(TK_DROP); }
    'ELSE' { RET(TK_ELSE); }
    'ERROR' { RET(TK_ERROR); }
    'END' { RET(TK_END); }
  //  'EXTERNAL' { RET(TK_EXTERNAL); }
    'EXIT' { RET(TK_EXIT); }
    'FACTORY' { RET(TK_FACTORY); }
    'IF' { RET(TK_IF); }
    'IMPORT' { RET(TK_IMPORT); }
    'INPUT' { RET(TK_INPUT); }
  //  'INTERPRET' { RET(TK_INTERPRET); }
    'ITERATE' { RET(TK_ITERATE); }
    'LEAVE' { RET(TK_LEAVE); }
    'NAMESPACE' { RET(TK_NAMESPACE); }
    'OF' { RET(TK_OF); }
    'NOP' { RET(TK_NOP); }
    'NUMERIC' { RET(TK_NUMERIC); }
  'OPTIONS' { RET(TK_OPTIONS); }
    'OTHERWISE' { RET(TK_OTHERWISE); }
  'OUTPUT' { RET(TK_OUTPUT); }
  //  'PARSE' { RET(TK_PARSE); }
    'PROCEDURE' { RET(TK_PROCEDURE); }
    'REGISTER' / [.] {
      s->lexer_stem_mode = 1;
      RET(TK_REGISTER);
    }
    'REGISTER' { RET(TK_REGISTER); }
  //  'PULL' { RET(TK_PULL); }
  //  'PUSH' { RET(TK_PUSH); }
  //  'QUEUE' { RET(TK_QUEUE); }
    'RETURN' { RET(TK_RETURN); }
    'SAY' { RET(TK_SAY); }
    'SELECT' { RET(TK_SELECT); }
  //  'SIGNAL' { RET(TK_SIGNAL); }
    'THEN' { RET(TK_THEN); }
  //  'TRACE' { RET(TK_TRACE); }
    'WHEN' { RET(TK_WHEN); }
  //  'OFF' { RET(TK_OFF); }
  //  'ON' { RET(TK_ON); }
    'BY' { RET(TK_BY); }
  //  'ERROR' { RET(TK_ERROR); }
    'EXPOSE' { RET(TK_EXPOSE); }
  //  'FAILURE' { RET(TK_FAILURE); }
    'FOR' { RET(TK_FOR); }
    'FOREVER' { RET(TK_FOREVER); }
  //  'HALT' { RET(TK_HALT); }
  //  'LINEIN' { RET(TK_LINEIN); }
  //  'NAME' { RET(TK_NAME); }
  //  'NOVALUE' { RET(TK_NOVALUE); }
  //  'SOURCE' { RET(TK_SOURCE); }
  //  'SYNTAX' { RET(TK_SYNTAX); }
    'TO' { RET(TK_TO); }
    'UNTIL' { RET(TK_UNTIL); }
  //  'UPPER' { RET(TK_UPPER); }
  //  'VAR' { RET(TK_VAR); }
  //  'VERSION' { RET(TK_VERSION); }
    'VOID' { RET(TK_VOID); }
    'WHILE' { RET(TK_WHILE); }
    'WITH' { RET(TK_WITH); }
    class { RET(TK_CLASS_TYPE); }
    float { RET(TK_FLOAT); }
    decimal { RET(TK_DECIMAL); }
    integer { RET(TK_INTEGER); }
    'ARG' / [.] {
      s->lexer_stem_mode = 1;
      RET(TK_ARG_STEM);
    }
    simple / [.] {
      s->lexer_stem_mode = 1;
      RET(TK_STEM);
    }
    class / [.] {
      s->lexer_stem_mode = 1;
      RET(TK_CLASS_STEM);
    }
    simple { RET(TK_VAR_SYMBOL); }
    simple ob ":" { RET(TK_LABEL); }
    "*" ob ":" { RET(TK_MULT_LABEL); }
    str { RET(TK_STRING); }
    str [bBxX] / (any\(symchr | [.])) { RET(TK_STRING); }
    eof { RET(TK_EOS); }
    $ { RET(TK_EOS); }
    whitespace {
      s->top = s->cursor;
      goto regular;
    }
    eol2 {
       s->line++;
       s->prev_linestart = s->linestart;
       s->linestart = s->cursor+2;
       RET(TK_EOL);
    }
    eol1 {
       s->line++;
       s->prev_linestart = s->linestart;
       s->linestart = s->cursor+1;
       RET(TK_EOL);
    }
    "." { RET(TK_DOT); }
    any {
      RET(TK_UNKNOWN);
    }
  */

  lexer_stem_mode:
/*!re2c
    stemdigit = [0-9];
    stemfsymchr = letter|[_];    // First Symbol Character
    stemsymchr = stemfsymchr|stemdigit;  // Subsequent Symbol Character

    steminteger = stemdigit+;
    stemsimple = stemfsymchr stemsymchr*;
    stemstring = stemsymchr+;

    [.] stemsimple / [.] {
       s->lexer_stem_mode = 1;
       RET(TK_STEMVAR);
    }
    [.] stemsimple {
       s->lexer_stem_mode = 0;
       RET(TK_STEMVAR);
    }
    [.] steminteger / [.] {
       s->lexer_stem_mode = 1;
       RET(TK_STEMINT);
    }
    [.] steminteger {
       s->lexer_stem_mode = 0;
       RET(TK_STEMINT);
    }
    [.] stemstring / [.] {
       s->lexer_stem_mode = 1;
       RET(TK_STEMSTRING);
    }
    [.] stemstring {
       s->lexer_stem_mode = 0;
       RET(TK_STEMSTRING);
    }
    [.] / [.] {
       s->lexer_stem_mode = 1;
       RET(TK_STEMNOVAL);
    }
    [.] {
       s->lexer_stem_mode = 0;
       RET(TK_STEMNOVAL);
    }
    $ { RET(TK_EOS); }
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
    s->linestart = s->cursor+1;
    goto comment;
  }
  eol2 {
    s->line++;
    s->prev_linestart = s->linestart;
    s->linestart = s->cursor+2;
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
      s->cursor = s->top + 2; /* To get the '/ *' */
      RET(TK_BADCOMMENT);
  }
  $ {
      s->line = comment_line;
      s->prev_linestart = s->linestart;
      s->linestart = comment_linestart;
      s->top = comment_top;
      s->cursor = s->top + 2; /* To get the '/ *' */
      RET(TK_BADCOMMENT);
  }
  any { goto comment; }
*/

skip_line_comment:
/*!re2c
  eol1 {
    s->line++;
    s->prev_linestart = s->linestart;
    s->linestart = s->cursor+1;
    s->top = s->cursor - 1;
    RET(TK_EOL);
  }
  eol2 {
    s->line++;
    s->prev_linestart = s->linestart;
    s->linestart = s->cursor+2;
    s->top = s->cursor - 1;
    RET(TK_EOL);
  }
  eof { RET(TK_EOS); }
  $ { RET(TK_EOS); }
  any {
    goto skip_line_comment;
  }
*/
}
