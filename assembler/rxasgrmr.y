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
// Grammar / Parser

%token_type { Assembler_Token* }
%default_type { Assembler_Token* }
%name Rxasm

%include {
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "rxas.h"
#include "rxasassm.h"
}

%code {
    const char* rxas_tpn(int type) {
        return yyTokenName[type];
    }
}

%extra_argument { Assembler_Context *context }

%parse_failure {
    rxaserrf(context, 0, 0, 1, "Error unexpected parse failure (1)");
}

%start_symbol program

%nonassoc ERROR.

%wildcard ANYTHING.

// Program Structure
program ::= headers functions EOS.
program ::= functions EOS.
program ::= headers EOS.

// Program error messages
program ::= ANYTHING(T) NEWLINE EOS. { rxaserat(context, T, "Error unexpected parse failure (2)"); }
program ::= headers ANYTHING(T) NEWLINE EOS. { rxaserat(context, T, "Error unexpected parse failure after headers");  }

// Header directives
headers ::= header.
headers ::= headers header.
header ::= globals.
header ::= global_reg.
header ::= global_meta.
header ::= file.
header ::= NEWLINE.

// Header error messages
header ::= ANYTHING(T) error NEWLINE. {rxaserat(context, T, "Invalid header directive");}

// Global directive
globals ::= KW_GLOBALS EQUAL INT(G) NEWLINE. {rxassetg(context,G);}
globals ::= KW_GLOBALS(T) error NEWLINE. {rxaseaft(context, T, "Expecting \"={integer}\"");}

// File directive
file ::= KW_SRCFILE EQUAL STRING(F) NEWLINE. {rxasqmfl(context,F);}
file ::= KW_SRCFILE(T) error NEWLINE. {rxaserat(context, T, "Expecting .srcfile = {filename}");}

// Global register directive
global_reg ::= GREG(R) KW_EXPOSE EQUAL ID(I) NEWLINE. {rxasexre(context,R,I);}
global_reg ::= AREG(R) KW_EXPOSE error NEWLINE. {rxaserat(context, R, "Can only expose global registers");}
global_reg ::= RREG(R) KW_EXPOSE error NEWLINE. {rxaserat(context, R, "Can only expose global registers");}
global_reg ::= GREG(T) error NEWLINE. {rxaseaft(context, T, "Expecting \".expose={id}\"");}
global_reg ::= GREG KW_EXPOSE(T) error NEWLINE. {rxaseaft(context, T, "Expecting \"={id}\"");}

// Global metadata
global_meta ::= KW_META STRING EQUAL STRING STRING reg(E) NEWLINE. {rxaserat(context, E, "Cannot set register metadata here");}
global_meta ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) NEWLINE. {rxasqmfu(context,V,OP,T,F,A);}
global_meta ::= KW_META STRING(V) EQUAL STRING(OP) STRING(P) NEWLINE. {rxasqmil(context,V,OP,P);}
global_meta ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) KW_CLASS NEWLINE. {rxasqmclss(context,V,OP,T);}
global_meta ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) KW_ATTR INT(R) NEWLINE. {rxasqmattr(context,V,OP,T,R);}
global_meta ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) KW_INTERFACE NEWLINE. {rxasqmintf(context,V,OP,T);}
global_meta ::= KW_META STRING(V) EQUAL STRING(I) KW_IMPLEMENTS NEWLINE. {rxasqmimpl(context,V,I);}
global_meta ::= KW_META STRING(O) EQUAL STRING(K) STRING(M) STRING(T) STRING(A) KW_MEMBER NEWLINE. {rxasqmmemb(context,O,K,M,T,A);}
global_meta ::= KW_META STRING(E) NEWLINE. {rxaserat(context, E, "Cannot clear metadata here");}
global_meta ::= KW_META STRING(E) EQUAL STRING STRING STRING NEWLINE. {rxaserat(context, E, "Cannot set constant metadata here");}
global_meta ::= KW_META(T) error NEWLINE. {rxaseaft(context, T, "Expecting {string} = {meta definition}");}

// Function list and Declarations and Definitions
functions ::= function.
functions ::= functions function.
function ::= functionDefinition NEWLINE instructions.
function ::= functionDeclaration NEWLINE decl_instructions.
function ::= functionDeclaration NEWLINE.
function ::= FUNC(T) error NEWLINE. {rxaseaft(context, T, "Expecting .locals or .expose");}
functionDefinition ::= FUNC(F) KW_LOCALS EQUAL INT(I). {rxasproc(context,F,I);}
functionDefinition ::= FUNC(F) KW_LOCALS EQUAL INT(I) KW_EXPOSE EQUAL ID(D). {rxasexpc(context,F,I,D);}
functionDeclaration ::= FUNC(F) KW_EXPOSE EQUAL ID(I). {rxasdecl(context,F,I);}

// Function Declaration error messages
functionDeclaration ::= FUNC KW_EXPOSE(T) error NEWLINE.
                        {rxaseaft(context, T, "Expecting \"={id}\"");}
functionDeclaration ::= FUNC KW_EXPOSE EQUAL ID ANYTHING(T) error NEWLINE.
                        {rxaserat(context, T, "Expecting {newline}");}

// Function Definition error messages
functionDefinition ::= FUNC KW_LOCALS(T) error NEWLINE.
                        {rxaseaft(context, T, "Expecting \"={int}\"");}
functionDefinition ::= FUNC KW_LOCALS EQUAL INT ANYTHING(T) error NEWLINE.
                        {rxaserat(context, T, "Expecting {newline} or .expose");}
functionDefinition ::= FUNC KW_LOCALS EQUAL INT KW_EXPOSE(T) error NEWLINE.
                        {rxaseaft(context, T, "Expecting \"={id}\"");}

// Instructions in a function/procedure
instructions ::= instruction.
instructions ::= instructions instruction.
instruction ::= instr NEWLINE.
instruction ::= LABEL(L). {rxasqlbl(context,L);}
instruction ::= KW_SRC INT(L) COLON INT(C) EQUAL STRING(S) NEWLINE. {rxasqmsr(context, L, C, S);}
instruction ::= KW_SRCSTEP INT(ST) INT(CL) INT(FL) STRING(F) INT(L) INT(SC) INT(EC) STRING(S) NEWLINE.
                {rxasqmstp(context, ST, CL, FL, F, L, SC, EC, S);}
instruction ::= KW_TRACEEVENT STRING(K) INT(M) STRING(VS) STRING(VT) STRING(RT) INT(VR) INT(ST) INT(CL) INT(FL) STRING(SYM) STRING(RN) NEWLINE.
                {rxasqmte(context, K, M, VS, VT, RT, VR, ST, CL, FL, SYM, RN);}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) reg(R) NEWLINE. {rxasqmre(context,V,OP,T,R);}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) NEWLINE. {rxasqmfu(context,V,OP,T,F,A);}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(P) NEWLINE. {rxasqmil(context,V,OP,P);}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) STRING(C) NEWLINE. {rxasqmct(context,V,OP,T,C);}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) KW_CLASS NEWLINE. {rxasqmclss(context,V,OP,T);}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) KW_ATTR INT(R) NEWLINE. {rxasqmattr(context,V,OP,T,R);}
instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) KW_INTERFACE NEWLINE. {rxasqmintf(context,V,OP,T);}
instruction ::= KW_META STRING(V) EQUAL STRING(I) KW_IMPLEMENTS NEWLINE. {rxasqmimpl(context,V,I);}
instruction ::= KW_META STRING(O) EQUAL STRING(K) STRING(M) STRING(T) STRING(A) KW_MEMBER NEWLINE. {rxasqmmemb(context,O,K,M,T,A);}
instruction ::= KW_META STRING(V) NEWLINE. {rxasqmcl(context,V);}
instruction ::= KW_SRCFILE EQUAL STRING(F) NEWLINE. {rxasqmfl(context,F);}
instruction ::= NEWLINE.

// Instruction error messages
instruction ::= KW_SRCFILE(T) error NEWLINE. {rxaserat(context, T, "Expecting .srcfile = {filename}");}
instruction ::= KW_META(T) error NEWLINE. {rxaseaft(context, T, "Expecting {string} = {meta definition}");}
instruction ::= ANYTHING(T) error NEWLINE. {rxaserat(context, T, "Invalid label, opcode or directive");}
instruction ::= KW_SRC(T) error NEWLINE. {rxaserat(context, T, "Expecting .src {line}:{col} = \"{source}\"");}
instruction ::= KW_SRCSTEP(T) error NEWLINE. {rxaserat(context, T, "Expecting .srcstep {step} {clause} {flags} \"{file}\" {line} {start-col} {end-col} \"{source line}\"");}
instruction ::= KW_TRACEEVENT(T) error NEWLINE. {rxaserat(context, T, "Expecting .traceevent \"{kind}\" {mode-mask} \"{value-source}\" \"{value-type}\" \"{register-type}\" {value-ref} {source-step} {clause} {flags} \"{symbol}\" \"{resolved-name}\"");}

// Instructions in a function declaration
decl_instructions ::= decl_instruction.
decl_instructions ::= decl_instructions decl_instruction.
decl_instruction ::= KW_SRCFILE EQUAL STRING(F) NEWLINE. {rxasqmfl(context,F);}
decl_instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(T) FUNC(F) STRING(A) NEWLINE. {rxasqmfu(context,V,OP,T,F,A);}
decl_instruction ::= KW_META STRING(V) EQUAL STRING(OP) STRING(P) NEWLINE. {rxasqmil(context,V,OP,P);}
decl_instruction ::= NEWLINE.

// Declaration instruction error messages
decl_instruction ::= KW_SRCFILE(T) error NEWLINE. {rxaserat(context, T, "Expecting .srcfile = {filename}");}
decl_instruction ::= ANYTHING(T) error NEWLINE. {rxaserat(context, T, "Invalid label, opcode or directive");}
decl_instruction ::= KW_SRC(T) error NEWLINE. {rxaserat(context, T, "Expecting .src {line}:{col} = \"{source}\"");}
decl_instruction ::= KW_SRC(E) INT COLON INT EQUAL STRING NEWLINE. {rxaserat(context, E, "Cannot define source line here");}
decl_instruction ::= KW_SRCSTEP(T) error NEWLINE. {rxaserat(context, T, "Cannot define source step here");}
decl_instruction ::= KW_TRACEEVENT(T) error NEWLINE. {rxaserat(context, T, "Cannot define trace event here");}
decl_instruction ::= KW_META STRING EQUAL STRING STRING reg(E) NEWLINE. {rxaserat(context, E, "Cannot set register metadata here");}
decl_instruction ::= KW_META STRING(E) NEWLINE. {rxaserat(context, E, "Cannot clear metadata here");}
decl_instruction ::= KW_META STRING(E) EQUAL STRING STRING STRING NEWLINE. {rxaserat(context, E, "Cannot set constant metadata here");}
decl_instruction ::= KW_META(T) error NEWLINE. {rxaseaft(context, T, "Expecting {string} = {meta definition}");}

// operation/instruction
instr ::= ID(IN). {rxasque0(context,IN);}
instr ::= ID(IN) operand(OP1). {rxasque1(context,IN,OP1);}
instr ::= ID(IN) operand(OP1) COMMA operand(OP2). {rxasque2(context,IN,OP1,OP2);}
instr ::= ID(IN) operand(OP1) COMMA operand(OP2) COMMA operand(OP3).
          {rxasque3(context,IN,OP1,OP2,OP3);}

// instr error messages
instr ::= ID ANYTHING(T) error NEWLINE. {rxaserat(context, T, "Expecting {operand} or {newline}");}
instr ::= ID operand ANYTHING(T) error NEWLINE. {rxaserat(context, T, "Expecting {operand} or {newline}");}
instr ::= ID operand COMMA(T) ANYTHING error NEWLINE. {rxaseaft(context, T, "Expecting {operand}");}
instr ::= ID operand COMMA operand ANYTHING(T) error NEWLINE.
          {rxaserat(context, T, "Expecting {operand} or {newline}");}
instr ::= ID operand COMMA operand COMMA(T) ANYTHING error NEWLINE.
          {rxaseaft(context, T, "Expecting {operand}");}
instr ::= ID operand COMMA operand COMMA operand ANYTHING(T) error NEWLINE.
          {rxaserat(context, T, "Expecting {newline} - max 3 operands");}

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
operand ::= HEX.
operand ::= DECIMAL.
