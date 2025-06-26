/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */
/* Scanner / Lexer              */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpbgmr.h"
#include "rxcpmain.h"

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
  decimal = float [d];
  integer = digit+;
  simple = fsymchr symchr*;
  class = [.] simple;
  sqstr = ['] ((any\['\n\r])|(['][']))* ['];
  dqstr = ["] ((any\["\n\r])|(["]["]))* ["];
  str = sqstr|dqstr;
  ob = [ \t]*;
  not = [\\] | not_char;
*/

/*!re2c
    // Line Comments
    [#] {
       if (s->hashcomments) goto skip_line_comment;
       else return(TK_UNKNOWN);
    }
    "//" {
       if (s->slashcomments) goto skip_line_comment;
       else return(TK_MOD);
    }
    "--" {
       if (s->dashcomments) goto skip_line_comment;
       else return(TK_MINUSMINUS);
    }

    // Block Comments
    "/*" {
      depth = 1;
      comment_line = s->line;
      comment_linestart = s->linestart;
      comment_top = s->top;
      goto comment;
    }

    "|" ob "|" { return(TK_CONCAT); }
    "+" { return(TK_PLUS); }
    "-" { return(TK_MINUS); }
    "*" { return(TK_MULT); }
    "/" { return(TK_DIV); }
    "%" { return(TK_IDIV); }
    "?" { return(TK_OPTIONAL); }
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
    "..." { return(TK_ELLIPSIS); }
    "(" { return(TK_OPEN_BRACKET); }
    ")" { return(TK_CLOSE_BRACKET); }
    "[" { return(TK_OPEN_SBRACKET); }
    "]" { return(TK_CLOSE_SBRACKET); }
    ";" { return(TK_EOC); }

    'ADDRESS' { return(TK_ADDRESS); }
    'ASSEMBLER' { return(TK_ASSEMBLER); }
    'ARG' { return(TK_ARG); }
    'CALL' { return(TK_CALL); }
    'DO' { return(TK_DO); }
    'LOOP' { return(TK_LOOP); }
  //  'DROP' { return(TK_DROP); }
    'ELSE' { return(TK_ELSE); }
    'ERROR' { return(TK_ERROR); }
    'END' { return(TK_END); }
  //  'EXTERNAL' { return(TK_EXTERNAL); }
    'EXIT' { return(TK_EXIT); }
    'IF' { return(TK_IF); }
    'IMPORT' { return(TK_IMPORT); }
    'INPUT' { return(TK_INPUT); }
  //  'INTERPRET' { return(TK_INTERPRET); }
    'ITERATE' { return(TK_ITERATE); }
    'LEAVE' { return(TK_LEAVE); }
    'NAMESPACE' { return(TK_NAMESPACE); }
    'NOP' { return(TK_NOP); }
  //  'NUMERIC' { return(TK_NUMERIC); }
  'OPTIONS' { return(TK_OPTIONS); }
  //  'OTHERWISE' { return(TK_OTHERWISE); }
  'OUTPUT' { return(TK_OUTPUT); }
  //  'PARSE' { return(TK_PARSE); }
    'PROCEDURE' { return(TK_PROCEDURE); }
  //  'PULL' { return(TK_PULL); }
  //  'PUSH' { return(TK_PUSH); }
  //  'QUEUE' { return(TK_QUEUE); }
    'RETURN' { return(TK_RETURN); }
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
    'EXPOSE' { return(TK_EXPOSE); }
  //  'FAILURE' { return(TK_FAILURE); }
    'FOR' { return(TK_FOR); }
    'FOREVER' { return(TK_FOREVER); }
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
    'UNTIL' { return(TK_UNTIL); }
  //  'UPPER' { return(TK_UPPER); }
  //  'VALUE' { return(TK_VALUE); }
  //  'VAR' { return(TK_VAR); }
  //  'VERSION' { return(TK_VERSION); }
    'VOID' { return(TK_VOID); }
    'WHILE' { return(TK_WHILE); }
  //  'WITH' { return(TK_WITH); }
    class { return(TK_CLASS); }
    float { return(TK_FLOAT); }
    decimal { return(TK_DECIMAL); }
    integer { return(TK_INTEGER); }
    'ARG' / [.] {
      s->lexer_stem_mode = 1;
      return(TK_ARG_STEM);
    }
    simple / [.] {
      s->lexer_stem_mode = 1;
      return(TK_STEM);
    }
    class / [.] {
      s->lexer_stem_mode = 1;
      return(TK_CLASS_STEM);
    }
    simple { return(TK_VAR_SYMBOL); }
    simple ob ":" { return(TK_LABEL); }
    str { return(TK_STRING); }
    str [bBxX] / (any\(symchr | [.])) { return(TK_STRING); }
    eof { return(TK_EOS); }
    $ { return(TK_EOS); }
    whitespace {
      s->top = s->cursor;
      goto regular;
    }
    eol2 {
       s->line++;
       s->prev_linestart = s->linestart;
       s->linestart = s->cursor+2;
       return(TK_EOL);
    }
    eol1 {
       s->line++;
       s->prev_linestart = s->linestart;
       s->linestart = s->cursor+1;
       return(TK_EOL);
    }
    any {
      return(TK_UNKNOWN);
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
       return(TK_STEMVAR);
    }
    [.] stemsimple {
       s->lexer_stem_mode = 0;
       return(TK_STEMVAR);
    }
    [.] steminteger / [.] {
       s->lexer_stem_mode = 1;
       return(TK_STEMINT);
    }
    [.] steminteger {
       s->lexer_stem_mode = 0;
       return(TK_STEMINT);
    }
    [.] stemstring / [.] {
       s->lexer_stem_mode = 1;
       return(TK_STEMSTRING);
    }
    [.] stemstring {
       s->lexer_stem_mode = 0;
       return(TK_STEMSTRING);
    }
    [.] / [.] {
       s->lexer_stem_mode = 1;
       return(TK_STEMNOVAL);
    }
    [.] {
       s->lexer_stem_mode = 0;
       return(TK_STEMNOVAL);
    }
    $ { return(TK_EOS); }
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
      return(TK_BADCOMMENT);
  }
  $ {
      s->line = comment_line;
      s->prev_linestart = s->linestart;
      s->linestart = comment_linestart;
      s->top = comment_top;
      s->cursor = s->top + 2; /* To get the '/ *' */
      return(TK_BADCOMMENT);
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
    return(TK_EOL);
  }
  eol2 {
    s->line++;
    s->prev_linestart = s->linestart;
    s->linestart = s->cursor+2;
    s->top = s->cursor - 1;
    return(TK_EOL);
  }
  eof { return(TK_EOS); }
  $ { return(TK_EOS); }
  any {
    goto skip_line_comment;
  }
*/
}
