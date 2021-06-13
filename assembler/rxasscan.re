// REXX Assembler
// Scanner

#include <stdio.h>
#include "rxasgrmr.h"
#include "rxas.h"

#define   YYCTYPE     unsigned char
#define   YYCURSOR    s->cursor
#define   YYMARKER    s->marker
#define   YYCTXMARKER s->ctxmarker

int scan(Assembler_Context* s, char *buff_end) {
    int depth;

    regular:
/*!re2c
    re2c:yyfill:enable = 0;

    whitespace = [ \t\v\f]+;
    digit = [0-9];
    letter = [a-zA-Z];
    any = [\x01-\xFF];
    eof = [\x00];
    slit = ["] ( (any\["\n\r]) | ( [\\]["] ) )* ["];
    clit = (['] (any\['\n\r]) [']) | ("\'\\" (any\['\n\r]) [']);
    float = [-+]? (digit* "." digit+ | digit+ ".");
    integer = [-+]? digit+;
    rreg = 'r' digit+;
    greg = 'g' digit+;
    areg = 'a' digit+;
    id = (letter | [_]) (letter | digit | [_-.])*;

    "/*" {
      depth = 1;
      goto comment;
    }
    "\r\n" {
       s->line++;
       s->linestart = s->cursor+1;
       return(NEWLINE);
    }
    [\r] | [\n] {
       s->line++;
       s->linestart = s->cursor;
       return(NEWLINE);
    }
    "*" [^\r\n]* { goto regular; }

    float {return(FLOAT);}
    integer {return(INT);}
    slit {return(STRING);}
    clit {return(CHAR);}
    rreg {return(RREG);}
    greg {return(GREG);}
    areg {return(AREG);}
    id "()" {return(FUNC);}
    id ":" {return(LABEL);}
    id {return(ID);}
    '=' {return(EQUAL);}
    ',' {return(COMMA);}
    '.globals' { return(KW_GLOBALS); }
    '.locals' { return(KW_LOCALS); }
    '.expose' { return(KW_EXPOSE); }
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
    error_f(s, s->line+1, 0, 1, "Error EOS before comment end");
    return(EOS);
  }
  * { goto comment; }
*/
}
