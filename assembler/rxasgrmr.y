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
    const char* token_type_name(int type) {
        return yyTokenName[type];
    }
}

%extra_argument { Assembler_Context *context }

// %parse_accept { printf("The parser has completed successfully.\n"); }

%parse_failure { fprintf(stderr, "0:0 - Error super completely confused - aborted!\n"); }

%start_symbol program

%nonassoc ERROR.

%wildcard ANYTHING.

// Program Structure
program ::= headers functions EOS.
program ::= functions EOS.
program ::= EOS.

// Program error messages
program ::= ANYTHING(T). {err_at(context, T, "really confused - aborted!");}
program ::= headers ANYTHING(T). {err_at(context, T, "totally confused - aborted!");}
program ::= error. { error_f(context, 0, 0, 1, "Error completely confused - aborted!");}

// Header directives
headers ::= header.
headers ::= headers header.
header ::= globals NEWLINE.
header ::= NEWLINE.

// Header error messages
header ::= ANYTHING(T) error NEWLINE. {err_at(context, T, "invalid header directive");}

// Global directive
globals ::= KW_GLOBALS EQUAL INT(G). {rxas_setglobals(context,G);}

// Global directive error messages
globals ::= KW_GLOBALS EQUAL(T) error. {err_aftr(context, T, "expecting integer");}
globals ::= KW_GLOBALS(T) error. {err_aftr(context, T, "expecting \"={integer}\"");}

// Function list an declaration
functions ::= function.
functions ::= functions function.
function ::= functionDeclaration NEWLINE instructions.
functionDeclaration ::= FUNC(F) KW_LOCALS EQUAL INT(I). {rxas_proc(context,F,I);}

// Function declaration error messages
functionDeclaration ::= FUNC(T) error. {err_aftr(context, T, "expecting \".locals={integer}\"");}
functionDeclaration ::= FUNC KW_LOCALS EQUAL INT ANYTHING(T) error.
                        {err_at(context, T, "expecting {newline}");}

// Instructions in a function/procedure
instructions ::= instruction.
instructions ::= instructions instruction.
instruction ::= instr NEWLINE.
instruction ::= LABEL(L). {rxas_label(context,L);}
instruction ::= NEWLINE.

// Instruction error messages
instruction ::= ANYTHING(T) error NEWLINE. {err_at(context, T, "invalid label, opcode or directive");}

// operation/instruction
instr ::= ID(IN). {rxas_gen0(context,IN);}
instr ::= ID(IN) operand(OP1). {rxas_gen1(context,IN,OP1);}
instr ::= ID(IN) operand(OP1) COMMA operand(OP2). {rxas_gen2(context,IN,OP1,OP2);}
instr ::= ID(IN) operand(OP1) COMMA operand(OP2) COMMA operand(OP3).
          {rxas_gen3(context,IN,OP1,OP2,OP3);}

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
operand ::= REG.
operand ::= FUNC.
operand ::= INT.
operand ::= FLOAT.
operand ::= CHAR.
operand ::= STRING.
