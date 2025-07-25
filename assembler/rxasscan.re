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

    // hex literals
    hex =  ("0x" | "OX")[0-9a-fA-F][0-9a-fA-F]([0-9a-fA-F][0-9a-fA-F])*;

    integer = [-+]? digit+;
    rreg = 'r' digit+;
    greg = 'g' digit+;
    areg = 'a' digit+;
    id = (letter | [_]) (letter | digit | [_\-.#])*;

    "/*" {
      depth = 1;
      goto comment;
    }
    eol1 {
       s->line++;
       s->linestart = s->cursor+1;
       return(NEWLINE);
    }
    eol2 {
       s->line++;
       s->linestart = s->cursor+2;
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
    '.src' { return(KW_SRC); }
    '.srcfile' { return(KW_SRCFILE); }
    '.meta' { return(KW_META); }
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
    s->linestart = s->cursor+1;
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
