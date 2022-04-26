// REXX Assembler
// Grammar / Parser

%token_type { Token* }
%default_type { Token* }

%include {
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "rxas.h"
#include "rxasassm.h"
}

%code {
    const char* tk_tp_nm(int type) {
        return yyTokenName[type];
    }
}

%extra_argument { Assembler_Context *context }

// %parse_accept { printf("The parser has completed successfully.\n"); }

%parse_failure {
printf("parse_failure()\n");
    error_f(context, 0, 0, 1, "Error parse failure - completely confused");
}

%start_symbol program

%nonassoc ERROR.

%wildcard ANYTHING.

// Program Structure
program ::= headers functions EOS.
program ::= functions EOS.
program ::= EOS.

// Program error messages
program ::= ANYTHING(T). {err_at(context, T, "Error really confused");}
program ::= headers ANYTHING(T). {err_at(context, T, "totally confused after headers");}
program ::= error. { error_f(context, 0, 0, 1, "Error completely confused");}

// Header directives
headers ::= header.
headers ::= headers header.
header ::= globals NEWLINE.
header ::= global_reg NEWLINE.
header ::= global_meta NEWLINE.
header ::= func_meta NEWLINE.
header ::= file NEWLINE.
header ::= NEWLINE.

// Header error messages
header ::= ANYTHING(T) error NEWLINE. {err_at(context, T, "invalid header directive");}

// Global directive
globals ::= KW_GLOBALS EQUAL INT(G). {rxassetg(context,G);}
globals ::= KW_GLOBALS EQUAL INT ANYTHING(E) error NEWLINE. {err_at(context, E, "expecting newline");}
globals ::= KW_GLOBALS EQUAL(T) error NEWLINE. {err_aftr(context, T, "expecting {integer}");}
globals ::= KW_GLOBALS(T) error NEWLINE. {err_aftr(context, T, "expecting \"={integer}\"");}

// File directive
file ::= KW_FILE STRING(F). { /* rxassetg(context,F); */}
file ::= KW_FILE ANYTHING(T) error NEWLINE. {err_at(context, T, "Expecting {filename}");}
file ::= KW_FILE STRING ANYTHING(T) error NEWLINE. {err_at(context, T, "Expecting NEWLINE");}

// Global register directive
global_reg ::= GREG(R) KW_EXPOSE EQUAL ID(I). {rxasexre(context,R,I);}
global_reg ::= AREG(R) KW_EXPOSE error NEWLINE. {err_at(context, R, "can only expose global registers");}
global_reg ::= RREG(R) KW_EXPOSE error NEWLINE. {err_at(context, R, "can only expose global registers");}
global_reg ::= GREG(T) error NEWLINE. {err_aftr(context, T, "expecting \".expose={id}\"");}
global_reg ::= GREG KW_EXPOSE(T) error NEWLINE. {err_aftr(context, T, "expecting \"={id}\"");}

// Global register metadata
global_meta ::= KW_META ID(V) EQUAL ID(T) GREG(R). {/* rxasmetr(context,V,R,T); */}
global_meta ::= KW_META ID EQUAL ID RREG(R) error NEWLINE. {err_at(context, R, "can only define metadata for global registers here");}
global_meta ::= KW_META ID EQUAL ID AREG(R) error NEWLINE. {err_at(context, R, "can only define metadata for global registers here");}
global_meta ::= KW_META ID EQUAL ID(R) error NEWLINE. {err_aftr(context, R, "expecting \"g{n}\"");}
global_meta ::= KW_META ID EQUAL(R) error NEWLINE. {err_aftr(context, R, "expecting \"{type} g{n}\"");}
global_meta ::= KW_META ID(I) error NEWLINE. {err_aftr(context, I, "expecting \"= {type}  g{n}\"");}
global_meta ::= KW_META ID EQUAL ID STRING(C) error NEWLINE. {err_at(context, C, "cannot define a constant symbol here");}
global_meta ::= KW_META ID(I). {err_at(context, I, "cannot clear metadata here");}
global_meta ::= KW_META(T) error NEWLINE. {err_aftr(context, T, "Expecting {id} or {func}() after .meta");}

// Global Function Meta data
func_meta ::= KW_META FUNC(F) EQUAL ID(T) STRING(L). {/* rxasqlbl(context,N, L, F, T)``; */}
func_meta ::= KW_META FUNC EQUAL ANYTHING(T) error NEWLINE. {err_at(context, T, "Expecting {type} {metadata string}");}
func_meta ::= KW_META FUNC EQUAL ID STRING ANYTHING(T) error NEWLINE. {err_at(context, T, "expecting {newline}");}

// Function list and Declarations and Definitions
functions ::= function.
functions ::= functions function.
function ::= functionDefinition NEWLINE instructions.
function ::= functionDeclaration blank_lines.
function ::= FUNC(T) error NEWLINE. {err_aftr(context, T, "expecting .locals or .expose");}
blank_lines ::= NEWLINE.
blank_lines ::= blank_lines NEWLINE.
functionDefinition ::= FUNC(F) KW_LOCALS EQUAL INT(I). {rxasproc(context,F,I);}
functionDefinition ::= FUNC(F) KW_LOCALS EQUAL INT(I) KW_EXPOSE EQUAL ID(D). {rxasexpc(context,F,I,D);}
functionDeclaration ::= FUNC(F) KW_EXPOSE EQUAL ID(I). {rxasdecl(context,F,I);}

// Function Declaration error messages
functionDeclaration ::= FUNC KW_EXPOSE(T) error NEWLINE.
                        {err_aftr(context, T, "expecting \"={id}\"");}
functionDeclaration ::= FUNC KW_EXPOSE EQUAL ID ANYTHING(T) error NEWLINE.
                        {err_at(context, T, "expecting {newline}");}

// Function Definition error messages
functionDefinition ::= FUNC KW_LOCALS(T) error NEWLINE.
                        {err_aftr(context, T, "expecting \"={int}\"");}
functionDefinition ::= FUNC KW_LOCALS EQUAL INT ANYTHING(T) error NEWLINE.
                        {err_at(context, T, "expecting {newline} or .expose");}
functionDefinition ::= FUNC KW_LOCALS EQUAL INT KW_EXPOSE(T) error NEWLINE.
                        {err_aftr(context, T, "expecting \"={id}\"");}

// Instructions in a function/procedure
instructions ::= instruction.
instructions ::= instructions instruction.
instruction ::= instr NEWLINE.
instruction ::= LABEL(L). {rxasqlbl(context,L);}
instruction ::= KW_LINE INT(N) STRING(L) NEWLINE. {/* rxasqlbl(context,N, L); */}
instruction ::= KW_META ID(V) EQUAL ID(T) reg(R) NEWLINE. {/* rxasmetr(context,V,R,T); */}
instruction ::= KW_META ID(V) EQUAL ID(T) STRING(S) NEWLINE. {/* rxasmetr(context,V,T,S); */}
instruction ::= KW_META ID(V) NEWLINE. {/* rxasmetr(context,V,R,S); */}
instruction ::= KW_META FUNC(F) EQUAL ID(T) STRING(L) NEWLINE. {/* rxasqlbl(context,N, L, F, T)``; */}
instruction ::= KW_FILE STRING(F) NEWLINE. { /* rxassetg(context,F); */}
instruction ::= NEWLINE.

// Instruction error messages
instruction ::= ANYTHING(T) error NEWLINE. {err_at(context, T, "invalid label, opcode or directive");}
instruction ::= KW_LINE ANYTHING(T) error NEWLINE. {err_at(context, T, "Expecting line number after .line");}
instruction ::= KW_LINE INT ANYTHING(T) error NEWLINE. {err_at(context, T, "Expecting line contents after .line");}
instruction ::= KW_LINE INT STRING ANYTHING(T) error NEWLINE. {err_at(context, T, "expecting {newline}");}
instruction ::= KW_FILE ANYTHING(T) error NEWLINE. {err_at(context, T, "Expecting {filename}");}
instruction ::= KW_FILE STRING ANYTHING(T) error NEWLINE. {err_at(context, T, "Expecting NEWLINE");}
instruction ::= KW_META ID EQUAL ID(R) error NEWLINE. {err_aftr(context, R, "expecting \"r{n}\"");}
instruction ::= KW_META ID EQUAL(R) error NEWLINE. {err_aftr(context, R, "expecting \"{type} r{n}\"");}
instruction ::= KW_META ID(I) error NEWLINE. {err_aftr(context, I, "expecting \"= {type} r{n}\"");}
instruction ::= KW_META ID EQUAL ID STRING ANYTHING(T) error NEWLINE. {err_at(context, T, "expecting {newline}");}
instruction ::= KW_META(T) error NEWLINE. {err_aftr(context, T, "Expecting {id} or {func}() after .meta");}
instruction ::= KW_META FUNC EQUAL ANYTHING(T) error NEWLINE. {err_at(context, T, "Expecting {type} {metadata string}");}
instruction ::= KW_META FUNC EQUAL ID STRING ANYTHING(T) error NEWLINE. {err_at(context, T, "expecting {newline}");}

// operation/instruction
instr ::= ID(IN). {rxasque0(context,IN);}
instr ::= ID(IN) operand(OP1). {rxasque1(context,IN,OP1);}
instr ::= ID(IN) operand(OP1) COMMA operand(OP2). {rxasque2(context,IN,OP1,OP2);}
instr ::= ID(IN) operand(OP1) COMMA operand(OP2) COMMA operand(OP3).
          {rxasque3(context,IN,OP1,OP2,OP3);}

// instr error messages
instr ::= ID ANYTHING(T) error NEWLINE. {err_at(context, T, "expecting {operand} or {newline}");}
instr ::= ID operand ANYTHING(T) error NEWLINE. {err_at(context, T, "expecting {operand} or {newline}");}
instr ::= ID operand COMMA(T) ANYTHING error NEWLINE. {err_aftr(context, T, "expecting {operand}");}
instr ::= ID operand COMMA operand ANYTHING(T) error NEWLINE.
          {err_at(context, T, "expecting {operand} or {newline}");}
instr ::= ID operand COMMA operand COMMA(T) ANYTHING error NEWLINE.
          {err_aftr(context, T, "expecting {operand}");}
instr ::= ID operand COMMA operand COMMA operand ANYTHING(T) error NEWLINE.
          {err_at(context, T, "expecting {newline} - max 3 operands");}

// Register Types
reg ::= RREG.
reg ::= AREG.
reg ::= GREG.

// Operand Types
operand ::= ID.
operand ::= reg.
operand ::= FUNC.
operand ::= INT.
operand ::= FLOAT.
operand ::= CHAR.
operand ::= STRING.
