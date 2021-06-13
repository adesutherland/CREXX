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
header ::= NEWLINE.

// Header error messages
header ::= ANYTHING(T) error NEWLINE. {err_at(context, T, "invalid header directive");}

// Global directive
globals ::= KW_GLOBALS EQUAL INT(G). {rxassetg(context,G);}

// Global directive error messages
globals ::= KW_GLOBALS EQUAL(T) error. {err_aftr(context, T, "expecting integer");}
globals ::= KW_GLOBALS(T) error. {err_aftr(context, T, "expecting \"={integer}\"");}

// Global register directive
global_reg ::= GREG(R) KW_EXPOSE EQUAL ID(I). {rxasexre(context,R,I);}

// Global register error messages
global_reg ::= AREG(R) KW_EXPOSE error. {err_at(context, R, "can only expose global registers");}
global_reg ::= RREG(R) KW_EXPOSE error. {err_at(context, R, "can only expose global registers");}
global_reg ::= GREG(T) error. {err_aftr(context, T, "expecting \".expose={id}\"");}
global_reg ::= GREG KW_EXPOSE(T) error. {err_aftr(context, T, "expecting \"={id}\"");}

// Function list and Declarations and Definitions
functions ::= function.
functions ::= functions function.
function ::= functionDefinition NEWLINE instructions.
function ::= functionDeclaration blank_lines.
function ::= FUNC(T) error. {err_aftr(context, T, "expecting .locals or .expose");}
blank_lines ::= NEWLINE.
blank_lines ::= blank_lines NEWLINE.
functionDefinition ::= FUNC(F) KW_LOCALS EQUAL INT(I). {rxasproc(context,F,I);}
functionDefinition ::= FUNC(F) KW_LOCALS EQUAL INT(I) KW_EXPOSE EQUAL ID(D). {rxasexpc(context,F,I,D);}
functionDeclaration ::= FUNC(F) KW_EXPOSE EQUAL ID(I). {rxasdecl(context,F,I);}

// Function Declaration error messages
functionDeclaration ::= FUNC KW_EXPOSE(T) error.
                        {err_aftr(context, T, "expecting \"={id}\"");}
functionDeclaration ::= FUNC KW_EXPOSE EQUAL ID ANYTHING(T) error.
                        {err_at(context, T, "expecting {newline}");}

// Function Definition error messages
functionDefinition ::= FUNC KW_LOCALS(T) error.
                        {err_aftr(context, T, "expecting \"={int}\"");}
functionDefinition ::= FUNC KW_LOCALS EQUAL INT ANYTHING(T) error.
                        {err_at(context, T, "expecting {newline} or .expose");}
functionDefinition ::= FUNC KW_LOCALS EQUAL INT KW_EXPOSE(T) error.
                        {err_aftr(context, T, "expecting \"={id}\"");}

// Instructions in a function/procedure
instructions ::= instruction.
instructions ::= instructions instruction.
instruction ::= instr NEWLINE.
instruction ::= LABEL(L). {rxaslabl(context,L);}
instruction ::= NEWLINE.

// Instruction error messages
instruction ::= ANYTHING(T) error NEWLINE. {err_at(context, T, "invalid label, opcode or directive");}

// operation/instruction
instr ::= ID(IN). {rxasgen0(context,IN);}
instr ::= ID(IN) operand(OP1). {rxasgen1(context,IN,OP1);}
instr ::= ID(IN) operand(OP1) COMMA operand(OP2). {rxasgen2(context,IN,OP1,OP2);}
instr ::= ID(IN) operand(OP1) COMMA operand(OP2) COMMA operand(OP3).
          {rxasgen3(context,IN,OP1,OP2,OP3);}

// instr error messages
instr ::= ID ANYTHING(T) error. {err_at(context, T, "expecting {operand} or {newline}");}
instr ::= ID operand ANYTHING(T) error. {err_at(context, T, "expecting {operand} or {newline}");}
instr ::= ID operand COMMA(T) ANYTHING error. {err_aftr(context, T, "expecting {operand}");}
instr ::= ID operand COMMA operand ANYTHING(T) error.
          {err_at(context, T, "expecting {operand} or {newline}");}
instr ::= ID operand COMMA operand COMMA(T) ANYTHING error.
          {err_aftr(context, T, "expecting {operand}");}
instr ::= ID operand COMMA operand COMMA operand ANYTHING(T) error.
          {err_at(context, T, "expecting {newline} - max 3 operands");}

// Operand Types
operand ::= ID.
operand ::= RREG.
operand ::= AREG.
operand ::= GREG.
operand ::= FUNC.
operand ::= INT.
operand ::= FLOAT.
operand ::= CHAR.
operand ::= STRING.
