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
 * Options Scanner (re2c)
 */

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
    'COMMENTS_HASH' { return(TK_COMMENTS_HASH); }
    'COMMENTS_DASH' { return(TK_COMMENTS_DASH); }
    'COMMENTS_SLASH' { return(TK_COMMENTS_SLASH); }
    'COMMENTS_NOHASH' { return(TK_COMMENTS_NOHASH); }
    'COMMENTS_NODASH' { return(TK_COMMENTS_NODASH); }
    'COMMENTS_NOSLASH' { return(TK_COMMENTS_NOSLASH); }
    'NUMERIC_COMMON' { return(TK_NUMERIC_COMMON); }
    'NUMERIC_CLASSIC' { return(TK_NUMERIC_CLASSIC); }
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
