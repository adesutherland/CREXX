/* cREXX Phase 0 (PoC) Compiler   */
/* (c) Adrian Sutherland 2021     */
/* Scanner / Lexer for OPTIONS    */
#include "rxcpopgr.h"
#include "rxcpmain.h"

#define   YYCTYPE     unsigned char
#define   YYCURSOR    s->cursor
#define   YYLIMIT     s->buff_end
#define   YYMARKER    s->marker
#define   YYCTXMARKER s->ctxmarker

int opt_scan(Context* s) {
    int depth;

/*!re2c
    re2c:yyfill:enable = 0;
    re2c:eof = 0;
*/

    /* Character Encoding Specifics  */
    /*!include:re2c "encoding.re" */

    regular:

/*!re2c
    re2c:yyfill:enable = 0;
    re2c:eof = 0;

    eol2 = "\r\n";
    eol1 = [\r] | [\n];
    eof = [\000] ;
    any = [^] \ eof ;
    digit = [0-9];
    symchr = letter|digit|[._];
    symbol = symchr*;
*/

/*!re2c
    "/*" {
        depth = 1;
        goto comment;
    }
    'OPTIONS' { return(TK_OPTIONS); }
    'LEVELA' { return(TK_LEVELA); }
    'LEVELB' { return(TK_LEVELB); }
    'LEVELC' { return(TK_LEVELC); }
    'LEVELD' { return(TK_LEVELD); }
    'LEVELG' { return(TK_LEVELG); }
    'LEVELL' { return(TK_LEVELL); }
    'HASHCOMMENTS' { return(TK_HASHCOMMENTS); }
    'DASHCOMMENTS' { return(TK_DASHCOMMENTS); }
    'SLASHCOMMENTS' { return(TK_SLASHCOMMENTS); }
    'NOHASHCOMMENTS' { return(TK_NOHASHCOMMENTS); }
    'NODASHCOMMENTS' { return(TK_NODASHCOMMENTS); }
    'NOSLASHCOMMENTS' { return(TK_NOSLASHCOMMENTS); }
    symbol { return(TK_SYMBOL); }
    eof { return(TK_EOS); }
    $ { return(TK_EOS); }
    whitespace {
        s->top = s->cursor;
        goto regular;
    }
    ";" { return(TK_EOC); }

    // Accept all line comment formats
    [#] {
       goto skip_line_comment;
    }
    "//" {
       goto skip_line_comment;
    }
    "--" {
       goto skip_line_comment;
    }

    eol2 {
        s->line++;
        s->linestart = s->cursor+2;
        return(TK_EOC);
    }
    eol1 {
        s->line++;
        s->linestart = s->cursor+1;
        return(TK_EOC);
    }
    any { return TK_UNKNOWN; }
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
        s->linestart = s->cursor+1;
        goto comment;
    }
    eol2 {
        s->line++;
        s->linestart = s->cursor+2;
        goto comment;
    }
    "/*" {
        ++depth;
        goto comment;
    }
    eof { return(TK_EOS); }
    $ { return(TK_EOS); }
    any { goto comment; }
*/

skip_line_comment:
/*!re2c
  eol1 {
    s->line++;
    s->prev_linestart = s->linestart;
    s->linestart = s->cursor+1;
    s->top = s->cursor - 1;
    return(TK_EOC);
  }
  eol2 {
    s->line++;
    s->prev_linestart = s->linestart;
    s->linestart = s->cursor+2;
    s->top = s->cursor - 1;
    return(TK_EOC);
  }
  eof { return(TK_EOS); }
  $ { return(TK_EOS); }
  any {
    goto skip_line_comment;
  }
*/
}
