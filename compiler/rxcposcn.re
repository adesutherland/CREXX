/* cREXX Phase 0 (PoC) Compiler   */
/* (c) Adrian Sutherland 2021     */
/* Scanner / Lexer for OPTIONS    */
#include "rxcpopgr.h"
#include "rxcpmain.h"

#define   YYCTYPE     char
#define   YYCURSOR    s->cursor
#define   YYMARKER    s->marker
#define   YYCTXMARKER s->ctxmarker

int opt_scan(Context* s) {
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
    all = [\000-\377];
    eof = [\000];
    any = all\eof;
    symchr = letter|digit|[.!?_];
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
    symbol { return(TK_SYMBOL); }
    eof { return(TK_EOS); }
    whitespace { goto regular; }
    ";" { return(TK_EOC); }
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
    any { return TK_UNKNOWN; }
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
    eof { return(TK_EOS); }
    any { goto comment; }
*/
}
