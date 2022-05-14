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
    error_f(context, 0, 0, 1, "Error unexpected parse failure (1)");
}

%start_symbol program

%nonassoc ERROR.

%wildcard ANYTHING.

// Program Structure
program ::= headers functions EOS.
program ::= functions EOS.
program ::= EOS.

// Program error messages
program ::= ANYTHING(T) error EOS. {err_at(context, T, "Error unexpected parse failure (2)");}
program ::= headers ANYTHING(T) error EOS. {err_at(context, T, "Error unexpected parse failure (3)");}
program ::= error EOS. { error_f(context, 0, 0, 1, "Error unexpected parse failure (4)");}

// Header directives
headers ::= header.
headers ::= headers header.
header ::= globals.
header ::= global_reg.
header ::= global_meta.
header ::= file.
header ::= NEWLINE.

// Header error messages
header ::= ANYTHING(T) error NEWLINE. {err_at(context, T, "invalid header directive");}

// Global directive
globals ::= KW_GLOBALS EQUAL INT(G) NEWLINE. {rxassetg(context,G);}
globals ::= KW_GLOBALS(T) error NEWLINE. {err_aftr(context, T, "expecting \"={integer}\"");}

// File directive
file ::= KW_SRCFILE EQUAL STRING(F) NEWLINE. { /* rxasmfil(context,F); */}
file ::= KW_SRCFILE(T) error NEWLINE. {err_at(context, T, "Expecting .srcfile = {filename}");}

// Global register directive
global_reg ::= KW_EXPOSE GREG(R) EQUAL ID(I) NEWLINE. {rxasexre(context,R,I);}
global_reg ::= KW_EXPOSE AREG(R) error NEWLINE. {err_at(context, R, "can only expose global registers");}
global_reg ::= KW_EXPOSE RREG(R) error NEWLINE. {err_at(context, R, "can only expose global registers");}
global_reg ::= KW_EXPOSE(T) error NEWLINE. {err_aftr(context, T, "expecting \".expose={id}\"");}

// Global metadata
global_meta ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) GREG(R) NEWLINE. {/* rxasmetr(context,V,OP,T,R); */}
global_meta ::= KW_META STRING EQUAL STRING STRING AREG(R) NEWLINE. {err_at(context, R, "can only define global registers here");}
global_meta ::= KW_META STRING EQUAL STRING STRING RREG(R) NEWLINE. {err_at(context, R, "can only define global registers here");}
global_meta ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) STRING(I) NEWLINE. {/* rxasmetf(context,V,OP,T,F,A,I); */}
global_meta ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) NEWLINE. {/* rxasmetf(context,V,OP,T,F,A,0); */}
global_meta ::= KW_META STRING(V) NEWLINE. {/* rxasmetd(context,V); */}
global_meta ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) STRING(C) NEWLINE. {/* rxasmetc(context,V,OP,T,C); */}
global_meta ::= KW_META(T) error NEWLINE. {err_aftr(context, T, "{string} = {meta definition}");}

// Function list and Declarations and Definitions
functions ::= function.
functions ::= functions function.
function ::= functionDefinition NEWLINE instructions.
function ::= functionDeclaration NEWLINE decl_instructions.
function ::= FUNC(T) error NEWLINE. {err_aftr(context, T, "expecting .locals or .expose");}
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
instruction ::= KW_SRC INT(L) COLON INT(C) EQUAL STRING(S) NEWLINE. {/* rxasmets(context, L, C, S); */}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) reg(R) NEWLINE. {/* rxasmetr(context,V,OP,T,R); */}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) STRING(I) NEWLINE. {/* rxasmetf(context,V,OP,T,F,A,I); */}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) NEWLINE. {/* rxasmetf(context,V,OP,T,F,A,0); */}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) STRING(C) NEWLINE. {/* rxasmetc(context,V,OP,T,C); */}
instruction ::= KW_META STRING(V) NEWLINE. {/* rxasmetd(context,V); */}
instruction ::= KW_SRCFILE EQUAL STRING(F) NEWLINE. { /* rxasmfil(context,F); */}
instruction ::= NEWLINE.

// Instruction error messages
instruction ::= KW_SRCFILE(T) error NEWLINE. {err_at(context, T, "Expecting .srcfile = {filename}");}
instruction ::= KW_META(T) error NEWLINE. {err_aftr(context, T, "{string} = {meta definition}");}
instruction ::= ANYTHING(T) error NEWLINE. {err_at(context, T, "invalid label, opcode or directive");}
instruction ::= KW_SRC(T) error NEWLINE. {err_at(context, T, "Expecting .src {line}:{col} = \"{source}\"");}

// Instructions in a function declaration
decl_instructions ::= decl_instruction.
decl_instructions ::= decl_instructions decl_instruction.
decl_instruction ::= KW_SRCFILE EQUAL STRING(F) NEWLINE. { /* rxasmfil(context,F); */}
decl_instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) STRING(I) NEWLINE. {/* rxasmetf(context,V,OP,T,F,A,I); */}
decl_instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) NEWLINE. {/* rxasmetf(context,V,OP,T,F,A,0); */}
decl_instruction ::= KW_META STRING(V) NEWLINE. {/* rxasmetd(context,V); */}
decl_instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) STRING(C) NEWLINE. {/* rxasmetc(context,V,OP,T,C); */}
decl_instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) GREG(R) NEWLINE. {/* rxasmetr(context,V,OP,T,R); */}
decl_instruction ::= NEWLINE.

// Declaration instruction error messages
decl_instruction ::= KW_SRCFILE(T) error NEWLINE. {err_at(context, T, "Expecting .srcfile = {filename}");}
decl_instruction ::= KW_META(T) error NEWLINE. {err_aftr(context, T, "{string} = {meta definition}");}
decl_instruction ::= ANYTHING(T) error NEWLINE. {err_at(context, T, "invalid label, opcode or directive");}
decl_instruction ::= KW_SRC(T) error NEWLINE. {err_at(context, T, "Expecting .src {line}:{col} = \"{source}\"");}
decl_instruction ::= KW_SRC(E) INT COLON INT EQUAL STRING NEWLINE. {err_at(context, E, "cannot define source line here");}
decl_instruction ::= KW_META STRING EQUAL STRING STRING AREG(R) NEWLINE. {err_at(context, R, "can only define global registers here");}
decl_instruction ::= KW_META STRING EQUAL STRING STRING RREG(R) NEWLINE. {err_at(context, R, "can only define global registers here");}

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
