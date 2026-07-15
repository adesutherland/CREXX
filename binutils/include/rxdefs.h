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

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Operand Formats */
typedef enum {
    FMT_EMPTY,
    FMT_B,
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
    FMT_R_B,
    FMT_R_B_B,
    FMT_R_B_R,
    FMT_R_B_S,
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
    FMT_R_R_B,
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

/* Explicit operand positions used by the opcode-effects inventory. */
typedef enum {
    RXOP_OP_NONE = 0,
    RXOP_OP_1 = 1,
    RXOP_OP_2 = 2,
    RXOP_OP_12 = 3,
    RXOP_OP_3 = 4,
    RXOP_OP_13 = 5,
    RXOP_OP_23 = 6,
    RXOP_OP_ALL = 7
} RxOpOperandMask;

typedef enum {
    RXOP_EFFECT_CLASSIFIED = 0,
    RXOP_EFFECT_CONSERVATIVE,
    RXOP_EFFECT_RESERVED,
    RXOP_EFFECT_INTERNAL
} RxOpEffectState;

typedef enum {
    RXOP_IMPLICIT_NONE = 0,
    RXOP_IMPLICIT_LOCAL_COPY,
    RXOP_IMPLICIT_LOCAL_TARGET,
    RXOP_IMPLICIT_LOCAL_R0_READ_WRITE,
    RXOP_IMPLICIT_LOCAL_R1_READ_WRITE,
    RXOP_IMPLICIT_LOCAL_R2_READ_WRITE,
    RXOP_IMPLICIT_ARGUMENT_INDEX,
    RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3
} RxOpImplicitEffect;

typedef enum {
    RXOP_SEM_NONE = 0,
    RXOP_SEM_MAY_THROW = 1,
    RXOP_SEM_CALL = 2,
    RXOP_SEM_DYNAMIC_CALL = 4,
    RXOP_SEM_RETURN = 8,
    RXOP_SEM_ALIAS_CREATE = 16,
    RXOP_SEM_ALIAS_RELEASE = 32,
    RXOP_SEM_REFERENCE_CREATE = 64,
    RXOP_SEM_REFERENCE_READ = 128,
    RXOP_SEM_REFERENCE_WRITE = 256,
    RXOP_SEM_REFERENCE_RELEASE = 512,
    RXOP_SEM_LIFETIME_END = 1024,
    RXOP_SEM_INDIRECT_WRITE = 2048,
    RXOP_SEM_INDIRECT_BRANCH = 4096,
    RXOP_SEM_OPAQUE = 8192
} RxOpSemanticFlags;

typedef struct {
    int opcode;
    RxOpEffectState state;
    unsigned int reads;
    unsigned int writes;
    unsigned int kills;
    unsigned int branch_targets;
    RxOpImplicitEffect implicit;
    unsigned int semantics;
    FlowType flow;
    int optimizer_barrier;
} RxOpEffects;

typedef struct {
    const char* mnemonic;
    int         opcode; /* Using int to avoid dependency loop or forward decl issues, or strictly Opcode */
    OpFormat    format;
    FlowType    flow;
    int         flags;
    const char* description;
} OpInfo;

static inline int rxop_is_source_mnemonic(const char *mnemonic) {
    if (!mnemonic) return 0;
    if (strncmp(mnemonic, "RESERVED_", 9) == 0) return 0;
    if (strcmp(mnemonic, "INULL") == 0) return 0;
    if (strcmp(mnemonic, "INTERRUPT") == 0) return 0;
    if (strcmp(mnemonic, "IUNKNOWN") == 0) return 0;
    return 1;
}

extern const OpInfo op_table[];

/* Opcode Enum using X-Macro */
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) OP_##NAME = OPCODE,
typedef enum {
    #include "rxops.h"
    OP_MAX_INSTRUCTIONS
} Opcode;
#undef X

RxOpEffects rxop_effects(int opcode);
size_t rxop_effect_count(void);

void *src_inst(const char* name, OperandType op1, OperandType op2, OperandType op3);

#endif // RXDEFS_H
