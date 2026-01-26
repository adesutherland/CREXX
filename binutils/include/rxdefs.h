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

/* Instruction Flags */
typedef enum {
    FLG_NONE = 0,
    FLG_DEPRECATED = 1
} OpFlags;

typedef struct {
    const char* mnemonic;
    int         opcode; /* Using int to avoid dependency loop or forward decl issues, or strictly Opcode */
    OpFormat    format;
    FlowType    flow;
    int         flags;
    const char* description;
} OpInfo;

/* Opcode Enum using X-Macro */
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) OP_##NAME = OPCODE,
typedef enum {
    #include "rxops.h"
    OP_MAX_INSTRUCTIONS
} Opcode;
#undef X

#endif // RXDEFS_H
