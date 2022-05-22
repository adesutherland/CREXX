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

%parse_failure {
    error_f(context, 0, 0, 1, "Error unexpected parse failure (1)");
}

%start_symbol program

%nonassoc ERROR.

%wildcard ANYTHING.

// Program Structure
program ::= headers functions EOS.
program ::= functions EOS.
program ::= NEWLINE EOS. { error_f(context, 0, 0, 1, "Error empty file"); }

// Program error messages
program ::= ANYTHING(T) NEWLINE EOS. { err_at(context, T, "Error unexpected parse failure (2)"); }
program ::= headers ANYTHING(T) NEWLINE EOS. { err_at(context, T, "Error unexpected parse failure after headers");  }

// Header directives
headers ::= header.
headers ::= headers header.
header ::= globals.
header ::= global_reg.
header ::= global_meta.
header ::= file.
header ::= NEWLINE.

// Header error messages
header ::= ANYTHING(T) error NEWLINE. {err_at(context, T, "Invalid header directive");}

// Global directive
globals ::= KW_GLOBALS EQUAL INT(G) NEWLINE. {rxassetg(context,G);}
globals ::= KW_GLOBALS(T) error NEWLINE. {err_aftr(context, T, "Expecting \"={integer}\"");}

// File directive
file ::= KW_SRCFILE EQUAL STRING(F) NEWLINE. {rxasqmfl(context,F);}
file ::= KW_SRCFILE(T) error NEWLINE. {err_at(context, T, "Expecting .srcfile = {filename}");}

// Global register directive
global_reg ::= GREG(R) KW_EXPOSE EQUAL ID(I) NEWLINE. {rxasexre(context,R,I);}
global_reg ::= AREG(R) KW_EXPOSE error NEWLINE. {err_at(context, R, "Can only expose global registers");}
global_reg ::= RREG(R) KW_EXPOSE error NEWLINE. {err_at(context, R, "Can only expose global registers");}
global_reg ::= GREG(T) error NEWLINE. {err_aftr(context, T, "Expecting \".expose={id}\"");}
global_reg ::= GREG KW_EXPOSE(T) error NEWLINE. {err_aftr(context, T, "Expecting \"={id}\"");}

// Global metadata
global_meta ::= KW_META STRING EQUAL STRING STRING reg(E) NEWLINE. {err_at(context, E, "Cannot set register metadata here");}
global_meta ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) STRING(I) NEWLINE. {rxasqmfu(context,V,OP,T,F,A,I);}
global_meta ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) NEWLINE. {rxasqmfu(context,V,OP,T,F,A,0);}
global_meta ::= KW_META STRING(E) NEWLINE. {err_at(context, E, "Cannot clear metadata here");}
global_meta ::= KW_META STRING(E) EQUAL STRING STRING STRING NEWLINE. {err_at(context, E, "Cannot set constant metadata here");}
global_meta ::= KW_META(T) error NEWLINE. {err_aftr(context, T, "Expecting {string} = {meta definition}");}

// Function list and Declarations and Definitions
functions ::= function.
functions ::= functions function.
function ::= functionDefinition NEWLINE instructions.
function ::= functionDeclaration NEWLINE decl_instructions.
function ::= functionDeclaration NEWLINE.
function ::= FUNC(T) error NEWLINE. {err_aftr(context, T, "Expecting .locals or .expose");}
functionDefinition ::= FUNC(F) KW_LOCALS EQUAL INT(I). {rxasproc(context,F,I);}
functionDefinition ::= FUNC(F) KW_LOCALS EQUAL INT(I) KW_EXPOSE EQUAL ID(D). {rxasexpc(context,F,I,D);}
functionDeclaration ::= FUNC(F) KW_EXPOSE EQUAL ID(I). {rxasdecl(context,F,I);}

// Function Declaration error messages
functionDeclaration ::= FUNC KW_EXPOSE(T) error NEWLINE.
                        {err_aftr(context, T, "Expecting \"={id}\"");}
functionDeclaration ::= FUNC KW_EXPOSE EQUAL ID ANYTHING(T) error NEWLINE.
                        {err_at(context, T, "Expecting {newline}");}

// Function Definition error messages
functionDefinition ::= FUNC KW_LOCALS(T) error NEWLINE.
                        {err_aftr(context, T, "Expecting \"={int}\"");}
functionDefinition ::= FUNC KW_LOCALS EQUAL INT ANYTHING(T) error NEWLINE.
                        {err_at(context, T, "Expecting {newline} or .expose");}
functionDefinition ::= FUNC KW_LOCALS EQUAL INT KW_EXPOSE(T) error NEWLINE.
                        {err_aftr(context, T, "Expecting \"={id}\"");}

// Instructions in a function/procedure
instructions ::= instruction.
instructions ::= instructions instruction.
instruction ::= instr NEWLINE.
instruction ::= LABEL(L). {rxasqlbl(context,L);}
instruction ::= KW_SRC INT(L) COLON INT(C) EQUAL STRING(S) NEWLINE. {rxasqmsr(context, L, C, S);}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) reg(R) NEWLINE. {rxasqmre(context,V,OP,T,R);}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) STRING(I) NEWLINE. {rxasqmfu(context,V,OP,T,F,A,I);}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) NEWLINE. {rxasqmfu(context,V,OP,T,F,A,0);}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) STRING(C) NEWLINE. {rxasqmct(context,V,OP,T,C);}
instruction ::= KW_META STRING(V) NEWLINE. {rxasqmcl(context,V);}
instruction ::= KW_SRCFILE EQUAL STRING(F) NEWLINE. {rxasqmfl(context,F);}
instruction ::= NEWLINE.

// Instruction error messages
instruction ::= KW_SRCFILE(T) error NEWLINE. {err_at(context, T, "Expecting .srcfile = {filename}");}
instruction ::= KW_META(T) error NEWLINE. {err_aftr(context, T, "Expecting {string} = {meta definition}");}
instruction ::= ANYTHING(T) error NEWLINE. {err_at(context, T, "Invalid label, opcode or directive");}
instruction ::= KW_SRC(T) error NEWLINE. {err_at(context, T, "Expecting .src {line}:{col} = \"{source}\"");}

// Instructions in a function declaration
decl_instructions ::= decl_instruction.
decl_instructions ::= decl_instructions decl_instruction.
decl_instruction ::= KW_SRCFILE EQUAL STRING(F) NEWLINE. {rxasqmfl(context,F);}
decl_instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) STRING(I) NEWLINE. {rxasqmfu(context,V,OP,T,F,A,I);}
decl_instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) NEWLINE. {rxasqmfu(context,V,OP,T,F,A,0);}
decl_instruction ::= NEWLINE.

// Declaration instruction error messages
decl_instruction ::= KW_SRCFILE(T) error NEWLINE. {err_at(context, T, "Expecting .srcfile = {filename}");}
decl_instruction ::= ANYTHING(T) error NEWLINE. {err_at(context, T, "Invalid label, opcode or directive");}
decl_instruction ::= KW_SRC(T) error NEWLINE. {err_at(context, T, "Expecting .src {line}:{col} = \"{source}\"");}
decl_instruction ::= KW_SRC(E) INT COLON INT EQUAL STRING NEWLINE. {err_at(context, E, "Cannot define source line here");}
decl_instruction ::= KW_META STRING EQUAL STRING STRING reg(E) NEWLINE. {err_at(context, E, "Cannot set register metadata here");}
decl_instruction ::= KW_META STRING(E) NEWLINE. {err_at(context, E, "Cannot clear metadata here");}
decl_instruction ::= KW_META STRING(E) EQUAL STRING STRING STRING NEWLINE. {err_at(context, E, "Cannot set constant metadata here");}
decl_instruction ::= KW_META(T) error NEWLINE. {err_aftr(context, T, "Expecting {string} = {meta definition}");}

// operation/instruction
instr ::= ID(IN). {rxasque0(context,IN);}
instr ::= ID(IN) operand(OP1). {rxasque1(context,IN,OP1);}
instr ::= ID(IN) operand(OP1) COMMA operand(OP2). {rxasque2(context,IN,OP1,OP2);}
instr ::= ID(IN) operand(OP1) COMMA operand(OP2) COMMA operand(OP3).
          {rxasque3(context,IN,OP1,OP2,OP3);}

// instr error messages
instr ::= ID ANYTHING(T) error NEWLINE. {err_at(context, T, "Expecting {operand} or {newline}");}
instr ::= ID operand ANYTHING(T) error NEWLINE. {err_at(context, T, "Expecting {operand} or {newline}");}
instr ::= ID operand COMMA(T) ANYTHING error NEWLINE. {err_aftr(context, T, "Expecting {operand}");}
instr ::= ID operand COMMA operand ANYTHING(T) error NEWLINE.
          {err_at(context, T, "Expecting {operand} or {newline}");}
instr ::= ID operand COMMA operand COMMA(T) ANYTHING error NEWLINE.
          {err_aftr(context, T, "Expecting {operand}");}
instr ::= ID operand COMMA operand COMMA operand ANYTHING(T) error NEWLINE.
          {err_at(context, T, "Expecting {newline} - max 3 operands");}

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
