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

/*
 * Operand signatures.
 *
 * An opcode's format is a NUL-terminated sequence of operand-kind codes, one
 * code per operand.  It deliberately has no small format-specific ceiling:
 * consumers must iterate the signature rather than copying it into a fixed
 * array.  The bytecode stream retains its existing INT_MAX cell-count bound.
 * The named forms below preserve the readable spelling used by the existing
 * instruction database; new instructions may use either a named form or a
 * signature literal.
 */
typedef const char *OpFormat;

#define FMT_EMPTY ""
#define FMT_B "B"
#define FMT_C "C"
#define FMT_F "F"
#define FMT_I "I"
#define FMT_I_I "II"
#define FMT_I_I_I "III"
#define FMT_I_I_R "IIR"
#define FMT_I_R "IR"
#define FMT_I_R_R "IRR"
#define FMT_L "L"
#define FMT_L_L_R "LLR"
#define FMT_L_P_S "LPS"
#define FMT_L_R "LR"
#define FMT_L_R_I "LRI"
#define FMT_L_R_R "LRR"
#define FMT_L_R_S "LRS"
#define FMT_L_S "LS"
#define FMT_P "P"
#define FMT_P_S "PS"
#define FMT_R "R"
#define FMT_R_B "RB"
#define FMT_R_B_B "RBB"
#define FMT_R_B_R "RBR"
#define FMT_R_B_S "RBS"
#define FMT_R_C "RC"
#define FMT_R_D "RD"
#define FMT_R_D_R "RDR"
#define FMT_R_F "RF"
#define FMT_R_F_I "RFI"
#define FMT_R_F_R "RFR"
#define FMT_R_I "RI"
#define FMT_R_I_I "RII"
#define FMT_R_I_R "RIR"
#define FMT_R_P "RP"
#define FMT_R_P_R "RPR"
#define FMT_R_R "RR"
#define FMT_R_R_B "RRB"
#define FMT_R_R_D "RRD"
#define FMT_R_R_F "RRF"
#define FMT_R_R_I "RRI"
#define FMT_R_R_R "RRR"
#define FMT_R_R_S "RRS"
#define FMT_R_S "RS"
#define FMT_R_S_I "RSI"
#define FMT_R_S_R "RSR"
#define FMT_R_S_S "RSS"
#define FMT_S "S"
#define FMT_S_R "SR"
#define FMT_S_S "SS"
#define FMT_S_S_R "SSR"

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

static inline size_t rxop_format_operand_count(OpFormat format) {
    return format ? strlen(format) : 0;
}

static inline OperandType rxop_format_operand_type(OpFormat format,
                                                   size_t operand_index) {
    char code;
    if (!format || operand_index >= strlen(format)) return OP_NONE;
    code = format[operand_index];
    switch (code) {
        case 'B': return OP_BINARY;
        case 'C': return OP_CHAR;
        case 'D': return OP_DECIMAL;
        case 'F': return OP_FLOAT;
        case 'I': return OP_INT;
        case 'L': return OP_ID;
        case 'P': return OP_FUNC;
        case 'R': return OP_REG;
        case 'S': return OP_STRING;
        default: return OP_NONE;
    }
}

static inline int rxop_format_matches(OpFormat format,
                                      const OperandType *operands,
                                      size_t operand_count) {
    size_t i;
    if (rxop_format_operand_count(format) != operand_count) return 0;
    for (i = 0; i < operand_count; i++) {
        if (!operands || rxop_format_operand_type(format, i) != operands[i]) return 0;
    }
    return 1;
}

/* Instruction Definition (Legacy machine/rxvminst.h) */
typedef struct Instruction
{
    int opcode;
    char *instruction;
    char *desc;
    size_t operands;
    OpFormat format;
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
    /* Value 1 was the retired RXOP_SEM_MAY_THROW bit. */
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

/* Canonical value components used by flow consumers.  These describe which
 * representation inside a value is observed or changed; they are deliberately
 * separate from register-slot reads/writes and from serialized opcodes. */
typedef enum {
    RXOP_COMPONENT_NONE = 0,
    RXOP_COMPONENT_INTEGER = 1,
    RXOP_COMPONENT_FLOAT = 2,
    RXOP_COMPONENT_STRING = 4,
    RXOP_COMPONENT_DECIMAL = 8,
    RXOP_COMPONENT_BINARY = 16,
    /* VM value status/type flags copied by ACOPY. */
    RXOP_COMPONENT_ATTRIBUTES = 32,
    RXOP_COMPONENT_REFERENCE = 64,
    /* Host-owned binary payloads have lifetime/finalizer semantics distinct
     * from an ordinary RXAS binary value. */
    RXOP_COMPONENT_NATIVE_PAYLOAD = 128,
    /* Logical child-count state used by SETATTRS/GETATTRS and checked by
     * attribute-addressing instructions. It is distinct from ACOPY status. */
    RXOP_COMPONENT_ATTRIBUTE_COUNT = 256,
    RXOP_COMPONENT_ALL = 511
} RxOpValueComponentMask;

typedef enum {
    RXOP_DERIVATION_NONE = 0,
    RXOP_DERIVATION_INTEGER_TO_FLOAT,
    RXOP_DERIVATION_INTEGER_TO_STRING,
    RXOP_DERIVATION_FLOAT_TO_STRING,
    RXOP_DERIVATION_DECIMAL_TO_STRING,
    RXOP_DERIVATION_INTEGER_TO_DECIMAL,
    RXOP_DERIVATION_BOOLEAN_TO_INTEGER,
    RXOP_DERIVATION_BOOLEAN_TO_DECIMAL,
    RXOP_DERIVATION_BOOLEAN_TO_FLOAT,
    RXOP_DERIVATION_BOOLEAN_TO_STRING,
    RXOP_DERIVATION_FLOAT_TO_INTEGER,
    RXOP_DERIVATION_FLOAT_TO_BOOLEAN,
    RXOP_DERIVATION_INTEGER_TO_BOOLEAN,
    RXOP_DERIVATION_STRING_TO_BOOLEAN,
    RXOP_DERIVATION_STRING_TO_FLOAT,
    RXOP_DERIVATION_STRING_TO_INTEGER,
    RXOP_DERIVATION_STRING_TO_DECIMAL,
    RXOP_DERIVATION_DECIMAL_TO_INTEGER,
    RXOP_DERIVATION_DECIMAL_TO_BOOLEAN,
    RXOP_DERIVATION_FLOAT_TO_DECIMAL,
    RXOP_DERIVATION_DECIMAL_TO_FLOAT
} RxOpValueDerivation;

typedef enum {
    RXOP_CONTEXT_NONE = 0,
    RXOP_CONTEXT_NUMERIC = 1
} RxOpContextMask;

/* Signal phase is an explicit contract. Consumers must fail closed on UNKNOWN;
 * known opcodes distinguish pre-write, post-write and named partial state. */
typedef enum {
    RXOP_SIGNAL_PHASE_NONE = 0,
    RXOP_SIGNAL_PHASE_BEFORE_WRITES,
    RXOP_SIGNAL_PHASE_AFTER_WRITES,
    RXOP_SIGNAL_PHASE_PARTIAL_WRITES,
    RXOP_SIGNAL_PHASE_UNKNOWN
} RxOpSignalPhase;

typedef enum {
    RXOP_SIGNAL_STATE_NONE = 0,
    RXOP_SIGNAL_STATE_KNOWN,
    RXOP_SIGNAL_STATE_UNKNOWN
} RxOpSignalState;

/* Where the signal name is obtained.  STATIC_NAMES uses the contract's
 * canonical '|' separated name set.  Literal and register operands are
 * zero-based RXAS operand positions resolved for each instruction instance. */
typedef enum {
    RXOP_SIGNAL_SOURCE_NONE = 0,
    RXOP_SIGNAL_SOURCE_STATIC_NAMES,
    RXOP_SIGNAL_SOURCE_LITERAL_OPERAND,
    RXOP_SIGNAL_SOURCE_REGISTER_OPERAND,
    RXOP_SIGNAL_SOURCE_PLUGIN,
    RXOP_SIGNAL_SOURCE_UNKNOWN,
    /* Signal propagated by a direct, dynamic or fused CALL.  The exact name
     * is callee/plugin supplied, but the caller-side failure phase is known. */
    RXOP_SIGNAL_SOURCE_PROPAGATED_CALL
} RxOpSignalSource;

typedef enum {
    RXOP_SIGNAL_CONT_NONE = 0,
    RXOP_SIGNAL_CONT_NORMAL = 1,
    RXOP_SIGNAL_CONT_SKIP = 2,
    RXOP_SIGNAL_CONT_HANDLER = 4,
    RXOP_SIGNAL_CONT_UNWIND = 8,
    RXOP_SIGNAL_CONT_TERMINAL = 16,
    RXOP_SIGNAL_CONT_ALL = 31
} RxOpSignalContinuations;

typedef enum {
    RXOP_SIGNAL_DEP_NONE = 0,
    RXOP_SIGNAL_DEP_NUMERIC_CONTEXT = 1,
    RXOP_SIGNAL_DEP_HANDLER_POLICY = 2,
    RXOP_SIGNAL_DEP_PLUGIN = 4,
    RXOP_SIGNAL_DEP_LOCALE = 8,
    RXOP_SIGNAL_DEP_EXTERNAL_STATE = 16,
    RXOP_SIGNAL_DEP_UNKNOWN = 32
} RxOpSignalDependencies;

typedef enum {
    RXOP_SIGNAL_PROP_NONE = 0,
    RXOP_SIGNAL_PROP_ADDRESS_OBSERVABLE = 1,
    RXOP_SIGNAL_PROP_PAYLOAD_OBSERVABLE = 2,
    RXOP_SIGNAL_PROP_SUCCESS_STABLE = 4,
    RXOP_SIGNAL_PROP_POLICY_WRITE = 8,
    RXOP_SIGNAL_PROP_ASYNC_ENTRY = 16
} RxOpSignalProperties;

/* Normal-path handler-policy transfer. Failure phase determines whether this
 * transfer is visible on skip/handler/failure edges. */
typedef enum {
    RXOP_POLICY_EFFECT_NONE = 0,
    RXOP_POLICY_EFFECT_BREAKPOINT_ENABLE_HANDLER,
    RXOP_POLICY_EFFECT_BREAKPOINT_ENABLE_EXISTING,
    RXOP_POLICY_EFFECT_BREAKPOINT_DISABLE,
    RXOP_POLICY_EFFECT_IGNORE,
    RXOP_POLICY_EFFECT_HALT,
    RXOP_POLICY_EFFECT_SILENT_HALT,
    RXOP_POLICY_EFFECT_BRANCH,
    RXOP_POLICY_EFFECT_CALL,
    RXOP_POLICY_EFFECT_CALL_BRANCH,
    RXOP_POLICY_EFFECT_RETURN,
    RXOP_POLICY_EFFECT_CALL_ACTION,
    RXOP_POLICY_EFFECT_PUSH,
    RXOP_POLICY_EFFECT_POP,
    RXOP_POLICY_EFFECT_BRANCH_VALUE,
    RXOP_POLICY_EFFECT_UNKNOWN
} RxOpSignalPolicyEffect;

typedef struct {
    int opcode;
    RxOpSignalState state;
    RxOpSignalPhase phase;
    RxOpSignalSource source;
    /* SIZE_MAX when the name does not come from an RXAS operand. */
    size_t source_operand;
    const char *static_names;
    /* Register operands whose state may be visible on failure. */
    unsigned int failure_writes;
    const char *failure_writes_signature;
    /* Aggregate component mask for the failure-visible register writes. */
    unsigned int failure_component_writes;
    unsigned int failure_context_writes;
    unsigned int dependencies;
    unsigned int continuations;
    unsigned int properties;
    RxOpSignalPolicyEffect policy_effect;
    RxOpSignalSource policy_source;
    size_t policy_source_operand;
    const char *policy_static_name;
} RxOpSignalContract;

/* Compile-time evaluators are semantic implementations shared by any callable
 * body that uses the instruction; they are not BIF identities.  NONE is the
 * fail-closed default for every instruction not individually proved. */
typedef enum {
    RXOP_CONST_EVAL_NONE = 0,
    RXOP_CONST_EVAL_STRLEN,
    RXOP_CONST_EVAL_SETSTRPOS,
    RXOP_CONST_EVAL_GETSTRPOS,
    RXOP_CONST_EVAL_STRCHAR_AT,
    RXOP_CONST_EVAL_SUBSTRING,
    RXOP_CONST_EVAL_PADSTR,
    RXOP_CONST_EVAL_FNDBLNK,
    RXOP_CONST_EVAL_FNDNBLNK,
    RXOP_CONST_EVAL_SCOPY,
    RXOP_CONST_EVAL_APPEND,
    RXOP_CONST_EVAL_STRLOWER,
    RXOP_CONST_EVAL_STRUPPER
} RxOpConstEvaluator;

typedef struct {
    int opcode;
    RxOpEffectState state;
    unsigned int reads;
    unsigned int writes;
    unsigned int kills;
    unsigned int branch_targets;
    /* Optional one-character-per-operand bit strings for wide instructions. */
    const char *reads_signature;
    const char *writes_signature;
    const char *kills_signature;
    const char *branch_targets_signature;
    RxOpImplicitEffect implicit;
    unsigned int semantics;
    /* Value-internal cursor state is separately observable through RXAS and
     * therefore cannot be hidden inside the coarse payload read/write masks. */
    unsigned int cursor_reads;
    unsigned int cursor_writes;
    RxOpConstEvaluator const_evaluator;
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

/* Canonical equivalence between a materialized integer comparison followed by
 * BRT/BRF and the corresponding non-materializing conditional branch.  Source
 * operands name indices in the comparison instruction; int/register forms may
 * therefore be normalized by swapping them. */
typedef struct RxOpCompareBranchFusion {
    int compare_opcode;
    int branch_opcode;
    int fused_opcode;
    size_t left_source_operand;
    size_t right_source_operand;
} RxOpCompareBranchFusion;

RxOpEffects rxop_effects(int opcode);
size_t rxop_effect_count(void);
int rxop_effect_reads_operand(const RxOpEffects *effects, size_t operand_index);
int rxop_effect_writes_operand(const RxOpEffects *effects, size_t operand_index);
int rxop_effect_kills_operand(const RxOpEffects *effects, size_t operand_index);
int rxop_effect_branch_target_operand(const RxOpEffects *effects, size_t operand_index);
int rxop_effect_reads_cursor(const RxOpEffects *effects, size_t operand_index);
int rxop_effect_writes_cursor(const RxOpEffects *effects, size_t operand_index);
unsigned int rxop_component_reads(int opcode, size_t operand_index);
unsigned int rxop_component_writes(int opcode, size_t operand_index);
int rxop_compare_branch_fusion(int compare_opcode, int branch_opcode,
                               RxOpCompareBranchFusion *fusion);
/* True when a two-register copy is guaranteed to perform no work, signal, or
 * observable cursor/effect update if both operands denote the same physical
 * register storage. */
int rxop_same_storage_copy_is_noop(int opcode);
/* Components proved absent after a successful operand write.  This is kept
 * separate from writes because one opcode may assign a scalar component while
 * clearing reference or native-payload lifetime state. */
unsigned int rxop_component_clears(int opcode, size_t operand_index);
RxOpValueDerivation rxop_value_derivation(int opcode);
size_t rxop_derivation_source_operand(int opcode);
unsigned int rxop_derivation_source_component(int opcode);
unsigned int rxop_derivation_context_reads(int opcode);
unsigned int rxop_context_writes(int opcode);
RxOpSignalContract rxop_signal_contract(int opcode);
size_t rxop_signal_contract_count(void);
int rxop_can_signal(int opcode);
int rxop_signal_failure_writes_operand(const RxOpSignalContract *contract,
                                       size_t operand_index);
RxOpSignalPhase rxop_signal_phase(int opcode);

void *src_inst(const char* name, OperandType op1, OperandType op2, OperandType op3);
void *src_instv(const char *name, const OperandType *operands, size_t operand_count);

#endif // RXDEFS_H
