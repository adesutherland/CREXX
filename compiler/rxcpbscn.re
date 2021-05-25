/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */
/* Scanner / Lexer              */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpbgmr.h"
#include "rxcpmain.h"

#define   YYCTYPE     char
#define   YYCURSOR    s->cursor
#define   YYMARKER    s->marker
#define   YYCTXMARKER s->ctxmarker

int rexbscan(Context* s) {
    int depth;

    regular:
    if (s->cursor >= s->buff_end) {
        return TK_EOS;
    }
    s->top = s->cursor;

/*!re2c
  re2c:yyfill:enable = 0;

  whitespace = [ \t\v\f]+;
  digit = [0-9];
  letter = [a-zA-Z];
  hex = [a-fA-F0-9];
  int_des = [uUlL]*;
  all = [\000-\377];
  eof = [\000];
  any = all\eof;
  symchr = letter|digit|[.!?_];
  float	= (digit*)[.]digit+([eE][+-]?digit+)?;
  integer = digit+;
  simple = (symchr\(digit|[.]))(symchr\[.])*;
  stem = simple [.];
  symbol = symchr+;
  sqstr = ['] ((any\['\n\r])|(['][']))* ['];
  dqstr = ["] ((any\["\n\r])|(["]["]))* ["];
  str = sqstr|dqstr;
  ob = [ \t]*;
  not = [\\~];
*/

/*!re2c
  "/*" {
    depth = 1;
    goto comment;
  }
  "|" ob "|" { return(TK_CONCAT); }
  "+" { return(TK_PLUS); }
  "-" { return(TK_MINUS); }
  "*" { return(TK_MULT); }
  "/" { return(TK_DIV); }
  "%" { return(TK_IDIV); }
  "/" ob "/" { return(TK_MOD); }
  "*" ob "*" { return(TK_POWER); }

  "=" { return(TK_EQUAL); }
  not ob "=" | "<" ob ">" | ">" ob "<" { return(TK_NEQ); }
  ">" { return(TK_GT); }
  "<" { return(TK_LT); }
  ">" ob "=" | not ob "<" { return(TK_GTE); }
  "<" ob "=" | not ob ">" { return(TK_LTE); }
  "=" ob "=" { return(TK_S_EQ); }
  not ob "=" ob "=" { return(TK_S_NEQ); }
  ">" ob ">" { return(TK_S_GT); }
  "<" ob "<" { return(TK_S_LT); }
  ">" ob ">" ob "=" | not ob "<" ob "<" { return(TK_S_GTE); }
  "<" ob "<" ob "=" | not ob ">" ob ">" { return(TK_S_LTE); }

  "&" { return(TK_AND); }
  "|" { return(TK_OR); }
//  "&" ob "&" { return(TK_OR); } // TODO Check
  not { return(TK_NOT); }
  "," { return(TK_COMMA); }
//  "." { return(TK_STOP); }
  "(" { return(TK_OPEN_BRACKET); }
  ")" { return(TK_CLOSE_BRACKET); }
  ";" { return(TK_EOC); }

//  'ADDRESS' { return(TK_ADDRESS); }
//  'ARG' { return(TK_ARG); }
//  'CALL' { return(TK_CALL); }
  'DO' { return(TK_DO); }
//  'DROP' { return(TK_DROP); }
  'ELSE' { return(TK_ELSE); }
  'END' { return(TK_END); }
//  'EXTERNAL' { return(TK_EXTERNAL); }
//  'EXIT' { return(TK_EXIT); }
  'IF' { return(TK_IF); }
//  'INTERPRET' { return(TK_INTERPRET); }
//  'ITERATE' { return(TK_ITERATE); }
//  'LEAVE' { return(TK_LEAVE); }
//  'NOP' { return(TK_NOP); }
//  'NUMERIC' { return(TK_NUMERIC); }
'OPTIONS' { return(TK_OPTIONS); }
//  'OTHERWISE' { return(TK_OTHERWISE); }
//  'PARSE' { return(TK_PARSE); }
//  'PROCEDURE' { return(TK_PROCEDURE); }
//  'PULL' { return(TK_PULL); }
//  'PUSH' { return(TK_PUSH); }
//  'QUEUE' { return(TK_QUEUE); }
//  'RETURN' { return(TK_RETURN); }
  'SAY' { return(TK_SAY); }
//  'SELECT' { return(TK_SELECT); }
//  'SIGNAL' { return(TK_SIGNAL); }
  'THEN' { return(TK_THEN); }
//  'TRACE' { return(TK_TRACE); }
//  'WHEN' { return(TK_WHEN); }
//  'OFF' { return(TK_OFF); }
//  'ON' { return(TK_ON); }
  'BY' { return(TK_BY); }
//  'DIGITS' { return(TK_DIGITS); }
//  'ENGINEERING' { return(TK_ENGINEERING); }
//  'ERROR' { return(TK_ERROR); }
//  'EXPOSE' { return(TK_EXPOSE); }
//  'FAILURE' { return(TK_FAILURE); }
  'FOR' { return(TK_FOR); }
//  'FOREVER' { return(TK_FOREVER); }
//  'FORM' { return(TK_FORM); }
//  'FUZZ' { return(TK_FUZZ); }
//  'HALT' { return(TK_HALT); }
//  'LINEIN' { return(TK_LINEIN); }
//  'NAME' { return(TK_NAME); }
//  'NOVALUE' { return(TK_NOVALUE); }
//  'SCIENTIFIC' { return(TK_SCIENTIFIC); }
//  'SOURCE' { return(TK_SOURCE); }
//  'SYNTAX' { return(TK_SYNTAX); }
  'TO' { return(TK_TO); }
//  'UNTIL' { return(TK_UNTIL); }
//  'UPPER' { return(TK_UPPER); }
//  'VALUE' { return(TK_VALUE); }
//  'VAR' { return(TK_VAR); }
//  'VERSION' { return(TK_VERSION); }
//  'WHILE' { return(TK_WHILE); }
//  'WITH' { return(TK_WITH); }
  float { return(TK_FLOAT); }
  integer { return(TK_INTEGER); }
  simple { return(TK_VAR_SYMBOL); }
//  stem { return(TK_SYMBOL_STEM); }
  symbol ob ":" { return(TK_LABEL); }
  symbol { return(TK_SYMBOL); }
  str { return(TK_STRING); }
  str [bB] / (all\symchr) { return(TK_STRING); }
  str [xX] / (all\symchr) { return(TK_STRING); }
  eof { return(TK_EOS); }
  whitespace { goto regular; }
  "\r\n" {
     s->line++;
     s->linestart = s->cursor+2;
     return(TK_EOC);
  }
  "\n" {
     s->line++;
     s->linestart = s->cursor+1;
     return(TK_EOC);
  }

  any { printf("unexpected character: %c\n", *s->cursor); return(-1); }
*/

    comment:
/*!re2c
  "*/" {
    if(--depth == 0) goto regular;
    else goto comment;
  }
  "\n" {
    s->line++;
    s->linestart = s->cursor+1;
    goto comment;
  }
  "\r\n" {
    s->line++;
    s->linestart = s->cursor+2;
    goto comment;
  }
  "/*" {
    ++depth;
    goto comment;
  }
  eof {
    printf("EOF before comment closed (comment depth %d): %c\n", depth);
    return(-1);
    }
  any { goto comment; }
*/
}
