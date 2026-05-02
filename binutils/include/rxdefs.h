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

#ifndef RXDEFS_H
#define RXDEFS_H

#include <stdint.h>

/* Operand Formats */
typedef enum {
    FMT_EMPTY,
    FMT_C,
    FMT_F,
    FMT_I,
    FMT_I_I,
    FMT_I_I_I,
    FMT_I_I_R,
    FMT_I_R,
    FMT_I_R_R,
    FMT_L,
    FMT_L_L_R,
    FMT_L_P_S,
    FMT_L_R,
    FMT_L_R_I,
    FMT_L_R_R,
    FMT_L_R_S,
    FMT_L_S,
    FMT_P,
    FMT_P_S,
    FMT_R,
    FMT_R_C,
    FMT_R_D,
    FMT_R_D_R,
    FMT_R_F,
    FMT_R_F_I,
    FMT_R_F_R,
    FMT_R_I,
    FMT_R_I_I,
    FMT_R_I_R,
    FMT_R_P,
    FMT_R_P_R,
    FMT_R_R,
    FMT_R_R_D,
    FMT_R_R_F,
    FMT_R_R_I,
    FMT_R_R_R,
    FMT_R_R_S,
    FMT_R_S,
    FMT_R_S_I,
    FMT_R_S_R,
    FMT_R_S_S,
    FMT_S,
    FMT_S_R,
    FMT_S_S,
    FMT_S_S_R
} OpFormat;

/* Flow Control Types */
typedef enum {
    FLOW_NEXT,
    FLOW_JUMP,
    FLOW_COND,
    FLOW_TERM
} FlowType;

/* Operand Types (Legacy machine/rxvminst.h) */
typedef enum
{
    OP_NONE = 0,
    OP_ID = 1,
    OP_REG = 2,
    OP_FUNC = 3,
    OP_INT = 4,
    OP_FLOAT = 5,
    OP_CHAR = 6,
    OP_STRING = 7,
    OP_DECIMAL = 8,
    OP_BINARY = 9
} OperandType;

/* Instruction Definition (Legacy machine/rxvminst.h) */
typedef struct Instruction
{
    int opcode;
    char *instruction;
    char *desc;
    int operands;
    OperandType op1_type;
    OperandType op2_type;
    OperandType op3_type;
} Instruction;

/* Instruction Flags */
typedef enum {
    FLG_NONE = 0,
    FLG_DEPRECATED = 1,
    FLG_OPT_BARRIER = 2,
    FLG_IMPLICIT_REG_USE = 4
} OpFlags;

typedef struct {
    const char* mnemonic;
    int         opcode; /* Using int to avoid dependency loop or forward decl issues, or strictly Opcode */
    OpFormat    format;
    FlowType    flow;
    int         flags;
    const char* description;
} OpInfo;

extern const OpInfo op_table[];

/* Opcode Enum using X-Macro */
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) OP_##NAME = OPCODE,
typedef enum {
    #include "rxops.h"
    OP_MAX_INSTRUCTIONS
} Opcode;
#undef X

void *src_inst(const char* name, OperandType op1, OperandType op2, OperandType op3);

#endif // RXDEFS_H
