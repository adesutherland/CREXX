// REXX Assembler
// Scanner

#include <stdio.h>
#include "rxasgrmr.h"
#include "rxas.h"

int scan(Assembler_Context* s, char *buff_end) {
    int depth;

    regular:
    if (s->cursor >= buff_end) {
        return EOS;
    }
    s->top = s->cursor;

/*!re2c
  re2c:yyfill:enable = 0;

  whitespace = [ \t\v\f]+;
  digit = [0-9];
  letter = [a-zA-Z];
  any = [\000-\377];
  eof = [\000];
  slit = ["] ( (any\["\n\r]) | ( [\\]["] ) )* ["];
  clit = (['] (any\['\n\r]) [']) | ("\'\\" (any\['\n\r]) [']);
  float = [-+]? (digit* "." digit+ | digit+ ".");
  integer = [-+]? digit+;
  reg = ('r' | 'a' | 'g') digit+;
  id = letter (letter | digit)*;

  "/*" {
    depth = 1;
    goto comment;
  }
  "\n" {
     s->line++;
     s->linestart = s->cursor;
     return(NEWLINE);
  }
  "\r\n" {
     s->line++;
     s->linestart = s->cursor+1;
     return(NEWLINE);
  }
  "*" [^\r\n]* { goto regular; }

  float {return(FLOAT);}
  integer {return(INT);}
  slit {return(STRING);}
  clit {return(CHAR);}
  reg {return(REG);}
  id "()" {return(FUNC);}
  id ":" {return(LABEL);}
  id {return(ID);}
  '=' {return(EQUAL);}
  ',' {return(COMMA);}
  '.globals' { return(KW_GLOBALS); }
  '.locals' { return(KW_LOCALS); }
  eof { return(EOS); }
  whitespace { goto regular; }
  any { return(ERROR); }
*/

    comment:
/*!re2c
  "*/" {
    if(--depth == 0) goto regular;
    else goto comment;
  }
  "\n" {
    s->line++;
    s->linestart = s->cursor;
    goto comment;
  }
  "\r\n" {
    s->line++;
    s->linestart = s->cursor+1;
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
  any { goto comment; }
*/
}

