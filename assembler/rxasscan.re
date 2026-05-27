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

// REXX Assembler
// Scanner

#include <stdio.h>
#include "rxasgrmr.h"
#include "rxas.h"

#define   YYCTYPE     unsigned char
#define   YYCURSOR    s->cursor
#define   YYMARKER    s->marker
#define   YYCTXMARKER s->ctxmarker

int rx_scan(Assembler_Context* s, char *buff_end) {
    int depth;

/*!re2c
    re2c:yyfill:enable = 0;
*/
    regular:

    /* Character Encoding Specifics  */
    /*!include:re2c "encoding.re" */

/*!re2c
    digit = [0-9];
    eol2 = "\r\n";
    eol1 = [\r] | [\n];
    eof = [\000] ;
    any = [^] \ eof ;

    slit = ["] ( (any\["\n\r]) | ( [\\]["] ) )* ["];
    clit = (['] (any\['\n\r]) [']) | ("\'\\" (any\[\n\r]) [']);

    // floating literals
    fsig = digit* "." digit+ | digit+ ".";
    fexp = [eE] [+-]? digit+;
    float = [-+]? (fsig fexp? | digit+ fexp);

    // binary literals
    hex =  ("0x" | "0X")([0-9a-fA-F][0-9a-fA-F])*;

    integer = [-+]? digit+;
    rreg = 'r' digit+;
    greg = 'g' digit+;
    areg = 'a' digit+;
    id = (letter | [_\xc2\xa7]) (letter | digit | [_\-.#\xc2\xa7])*;

    "/*" {
      depth = 1;
      goto comment;
    }
    eol1 {
       s->line++;
       s->linestart = s->cursor;
       return(NEWLINE);
    }
    eol2 {
       s->line++;
       s->linestart = s->cursor;
       return(NEWLINE);
    }

    "*" [^\r\n]* { goto regular; }

    float "d" {return(DECIMAL);}
    float {return(FLOAT);}
    integer "d" {return(DECIMAL);}
    integer {return(INT);}
    hex {return(HEX);}
    slit {return(STRING);}
    clit {return(CHAR);}
    rreg {return(RREG);}
    greg {return(GREG);}
    areg {return(AREG);}
    id "()" {return(FUNC);}
    id ":" {return(LABEL);}
    id {return(ID);}
    '=' {return(EQUAL);}
    ':' {return(COLON);}
    ',' {return(COMMA);}
    '.globals' { return(KW_GLOBALS); }
    '.locals' { return(KW_LOCALS); }
    '.expose' { return(KW_EXPOSE); }
    '.srcstep' { return(KW_SRCSTEP); }
    '.src' { return(KW_SRC); }
    '.srcfile' { return(KW_SRCFILE); }
    '.meta' { return(KW_META); }
    '.class' { return(KW_CLASS); }
    '.attr' { return(KW_ATTR); }
    '.interface' { return(KW_INTERFACE); }
    '.implements' { return(KW_IMPLEMENTS); }
    '.member' { return(KW_MEMBER); }
    eof { return(EOS); }
    whitespace {
      s->top = s->cursor;
      goto regular;
    }
    * { return(ERROR); }
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
  "\r\n" {
    s->line++;
    s->linestart = s->cursor;
    goto comment;
  }
  [\r] | [\n] {
    s->line++;
    s->linestart = s->cursor;
    goto comment;
  }

"/*" {
    ++depth;
    goto comment;
  }
  eof {
    rxaserrf(s, s->line+1, 0, 1, "Error EOS before comment end");
    return(EOS);
  }
  * { goto comment; }
*/
}
