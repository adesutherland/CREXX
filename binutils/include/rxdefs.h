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
#define FMT_S_R_R "SRR"
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
    RXOP_IMPLICIT_LOCAL_RANGE_AFTER_OP3,
    RXOP_IMPLICIT_CAPTURED_ARGUMENT_INDEX,
    RXOP_IMPLICIT_INLINE_RECIPE,
    RXOP_IMPLICIT_QUALIFIED_GLOBAL
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

/* Activation effects are distinct from optimizer opacity and external side
 * effects.  An unchanged SAY/TIME instruction need not observe the activation
 * that contains it; an unchanged parent-frame or handler-stack operation does.
 * These flags describe adapters/observations, not a second value evaluator. */
typedef enum {
    RXOP_ACTIVATION_NONE = 0,
    RXOP_ACTIVATION_BINDING = 1,
    RXOP_ACTIVATION_ARGUMENTS = 2,
    RXOP_ACTIVATION_CALL = 4,
    RXOP_ACTIVATION_RETURN = 8,
    RXOP_ACTIVATION_NUMERIC = 16,
    RXOP_ACTIVATION_SIGNAL = 32,
    RXOP_ACTIVATION_REFERENCE = 64,
    RXOP_ACTIVATION_LIFETIME = 128,
    RXOP_ACTIVATION_FRAME = 256,
    RXOP_ACTIVATION_MODULE = 512,
    RXOP_ACTIVATION_CODE = 1024,
    RXOP_ACTIVATION_DEBUG = 2048,
    RXOP_ACTIVATION_UNKNOWN = 4096
} RxOpActivationEffect;

typedef enum {
    RXOP_BINDING_NONE = 0,
    RXOP_BINDING_LINK,
    RXOP_BINDING_UNLINK,
    RXOP_BINDING_SWAP,
    RXOP_BINDING_ARGUMENT,
    RXOP_BINDING_COMPOUND,
    RXOP_BINDING_LIFETIME
} RxOpBindingAction;

/* Every source-visible opcode has one structural treatment.  An adapter
 * category does not claim that every selector or state is already supported.
 * Optional component/value facts are not ordinary-clone admission gates. */
typedef enum {
    RXOP_CLONE_MISSING = 0,
    RXOP_CLONE_ORDINARY,
    RXOP_CLONE_BINDING,
    RXOP_CLONE_CALL,
    RXOP_CLONE_RETURN,
    RXOP_CLONE_CONTEXT,
    RXOP_CLONE_RETAINED
} RxOpCloneCategory;

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
    /* VM status flags copied by ACOPY; not the object's type descriptor. */
    RXOP_COMPONENT_ATTRIBUTES = 32,
    RXOP_COMPONENT_REFERENCE = 64,
    /* Host-owned binary payloads have lifetime/finalizer semantics distinct
     * from an ordinary RXAS binary value. */
    RXOP_COMPONENT_NATIVE_PAYLOAD = 128,
    /* Logical child-count state used by SETATTRS/GETATTRS and checked by
     * attribute-addressing instructions. It is distinct from ACOPY status. */
    RXOP_COMPONENT_ATTRIBUTE_COUNT = 256,
    /* Immutable graph/type descriptor pointer. Whole COPY/MOVE transport it;
     * ACOPY and scalar setters preserve the destination descriptor. */
    RXOP_COMPONENT_OBJECT_TYPE = 512,
    RXOP_COMPONENT_ALL = 1023
} RxOpValueComponentMask;

/* Hidden ABI operations describe existing helpers, not serialized opcodes or
 * a client-authored recipe language. NONE is proved absence; UNKNOWN is not. */
typedef enum { RXOP_ABI_NONE, RXOP_ABI_KNOWN, RXOP_ABI_UNKNOWN } RxOpAbiState;
typedef enum {
    RXOP_ABI_ENTRY, RXOP_ABI_PHYSICAL_RETURN, RXOP_ABI_SCOPED_RETURN,
    RXOP_ABI_SCOPE_LEAVE, RXOP_ABI_FRAME_UNWIND,
    RXOP_ABI_ROOT_RETURN, RXOP_ABI_ROOT_SIGRETURN, RXOP_ABI_ROOT_UNWIND,
    RXOP_ABI_EXECUTION_FINISH, RXOP_ABI_FRAME_RETIRE, RXOP_ABI_MODE_COUNT
} RxOpAbiMode;
typedef enum {
    RXOP_ABI_CAPTURE_POINTERS, RXOP_ABI_CLEAR_STORAGE, RXOP_ABI_PUBLISH_SCOPE,
    RXOP_ABI_SET_INTEGER, RXOP_ABI_BIND_PROXY, RXOP_ABI_RETURN_REGISTER,
    RXOP_ABI_RETURN_INTEGER, RXOP_ABI_RETURN_FLOAT, RXOP_ABI_RETURN_STRING,
    RXOP_ABI_RESTORE_POLICY, RXOP_ABI_RESTORE_SLOT, RXOP_ABI_RESTORE_NUMERIC,
    RXOP_ABI_POP_SCOPE, RXOP_ABI_RETIRE_FRAME,
    RXOP_ABI_DEACTIVATE_ROOT, RXOP_ABI_CAPTURE_ROOT_COUNT,
    RXOP_ABI_COPY_ROOT_ARGUMENT, RXOP_ABI_FREE_ROOT_ARGUMENT,
    RXOP_ABI_FREE_ROOT_ARGUMENT_UNBIND, RXOP_ABI_FORGET_ROOT_ARGV_OWNER,
    RXOP_ABI_CAPTURE_ROOT_STATUS, RXOP_ABI_PUBLISH_ROOT_STATUS,
    RXOP_ABI_CLEANUP_EXECUTION_SIGNALS, RXOP_ABI_BEGIN_RECYCLED_FRAME,
    RXOP_ABI_CLEAR_FRAME_CONTROL, RXOP_ABI_DESTROY_STORAGE,
    RXOP_ABI_RELEASE_FRAME_PLUGINS, RXOP_ABI_RELEASE_RECYCLED_FRAME,
    RXOP_ABI_FINISH_RUNTIME_STORAGE, RXOP_ABI_FINISH_RUNTIME_CONTROL,
    RXOP_ABI_RESET_STORAGE, RXOP_ABI_LEAVE_FRAME_SCOPES,
    RXOP_ABI_CLEAR_FRAME_HANDLERS, RXOP_ABI_RELEASE_FRAME_POLICY,
    RXOP_ABI_RELEASE_FRAME_REFERENCES, RXOP_ABI_ABANDON_FRAME_INVOCATION,
    RXOP_ABI_UNPIN_FRAME_INVOCATION, RXOP_ABI_RECYCLE_FRAME
} RxOpAbiOperation;
typedef enum {
    RXOP_ABI_ENTRY_CAPTURE, RXOP_ABI_ENTRY_PRECLEAR, RXOP_ABI_ENTRY_PUBLISH,
    RXOP_ABI_ENTRY_A0, RXOP_ABI_ENTRY_PROXIES, RXOP_ABI_RETURN_RESULT,
    RXOP_ABI_LEAVE_POLICY, RXOP_ABI_LEAVE_CLEAR, RXOP_ABI_LEAVE_RESTORE,
    RXOP_ABI_LEAVE_NUMERIC, RXOP_ABI_LEAVE_POP, RXOP_ABI_PHYSICAL_RETIRE,
    RXOP_ABI_PHYSICAL_NUMERIC, RXOP_ABI_ROOT_DEACTIVATE,
    RXOP_ABI_ROOT_COPYBACK_COUNT, RXOP_ABI_ROOT_COPYBACK,
    RXOP_ABI_ROOT_FREE_COUNT, RXOP_ABI_ROOT_ARGUMENT_FREE,
    RXOP_ABI_ROOT_ARGV_FORGET, RXOP_ABI_ROOT_STATUS_CAPTURE,
    RXOP_ABI_ROOT_STATUS_PUBLISH, RXOP_ABI_FINISH_SIGNALS,
    RXOP_ABI_FINISH_RECYCLER, RXOP_ABI_FINISH_FRAME_CONTROL,
    RXOP_ABI_FINISH_OWNED_CLEAR, RXOP_ABI_FINISH_A0_CLEAR,
    RXOP_ABI_FINISH_PLUGIN_RELEASE, RXOP_ABI_FINISH_FRAME_RELEASE,
    RXOP_ABI_FINISH_STORAGE, RXOP_ABI_FINISH_CONTROL, RXOP_ABI_ROOT_SIGNAL_CLEAR,
    RXOP_ABI_RETIRE_SCOPES, RXOP_ABI_RETIRE_HANDLERS, RXOP_ABI_RETIRE_POLICY,
    RXOP_ABI_RETIRE_REFERENCES, RXOP_ABI_RETIRE_INVOCATION_ABANDON,
    RXOP_ABI_RETIRE_INVOCATION_UNPIN, RXOP_ABI_RETIRE_RECYCLE
} RxOpAbiCheckpoint;
typedef enum {
    RXOP_ABI_ROLE_NONE, RXOP_ABI_RECIPE_CAPTURES, RXOP_ABI_SCOPE_CAPTURES,
    RXOP_ABI_RECIPE_ORIGINAL_BACKING, RXOP_ABI_SCOPE_CONTEXT, RXOP_ABI_PREVIOUS_SCOPE,
    RXOP_ABI_CAPTURED_COUNT, RXOP_ABI_A0_ORIGINAL_BACKING, RXOP_ABI_CAPTURED_ARGUMENT,
    RXOP_ABI_RECIPE_PROXY_SLOT, RXOP_ABI_RETURN_ACTIVE, RXOP_ABI_RETURN_LITERAL,
    RXOP_ABI_CAPTURED_RESULT, RXOP_ABI_SAVED_POLICY, RXOP_ABI_ACTIVE_POLICY,
    RXOP_ABI_SCOPE_ORIGINAL_BACKING, RXOP_ABI_SAVED_SLOT, RXOP_ABI_OWNED_SLOT,
    RXOP_ABI_SAVED_NUMERIC, RXOP_ABI_ACTIVE_NUMERIC, RXOP_ABI_PHYSICAL_FRAME,
    RXOP_ABI_PARENT_NUMERIC, RXOP_ABI_PHYSICAL_ROOT,
    RXOP_ABI_ROOT_RETURN_RECEIVER, RXOP_ABI_ROOT_ACTIVE_ARGUMENT,
    RXOP_ABI_ROOT_ORIGINAL_ARGUMENT, RXOP_ABI_ROOT_ORIGINAL_A0,
    RXOP_ABI_ROOT_COPYBACK_COUNT_VALUE, RXOP_ABI_ROOT_FREE_COUNT_VALUE,
    RXOP_ABI_ROOT_SAVED_NUMBER_ARGS, RXOP_ABI_EXTERNAL_ARGUMENT,
    RXOP_ABI_EXTERNAL_ARGUMENT_COUNT, RXOP_ABI_CLI_ARGV_OWNER,
    RXOP_ABI_ROOT_SAVED_RETURN_STATUS, RXOP_ABI_ROOT_SELECTED_RETURN_STATUS,
    RXOP_ABI_EXECUTION_STATUS, RXOP_ABI_CANONICAL_FRAME_RECYCLERS,
    RXOP_ABI_RECYCLED_FRAME, RXOP_ABI_RECYCLED_OWNED_BACKING,
    RXOP_ABI_RECYCLED_A0_BACKING, RXOP_ABI_EXECUTION_OWNED_STORAGE,
    RXOP_ABI_EXECUTION_CONTROL, RXOP_ABI_ROOT_SIGNAL_DIAGNOSTIC,
    RXOP_ABI_FRAME_SCOPES, RXOP_ABI_FRAME_HANDLERS, RXOP_ABI_FRAME_PRIVATE_POLICY,
    RXOP_ABI_FRAME_REFERENCES, RXOP_ABI_FRAME_INVOCATION,
    RXOP_ABI_PROCEDURE_RECYCLER
} RxOpAbiRole;
typedef enum {
    RXOP_ABI_EXTENT_NONE, RXOP_ABI_TOP, RXOP_ABI_ACTIVE_TREE, RXOP_ABI_FULL_CAPACITY
} RxOpAbiExtent;
typedef enum {
    RXOP_ABI_SUCCESSFUL_ENTRY, RXOP_ABI_COMMITTED_SCOPE, RXOP_ABI_RESULT_PRESENT,
    RXOP_ABI_OWNED_BASE_MOVE_ELSE_COPY, RXOP_ABI_INTERNAL_PARENT, RXOP_ABI_ARGUMENT_PRESENT,
    RXOP_ABI_VALIDATED_STAGE,
    RXOP_ABI_ROOT_FRAME, RXOP_ABI_ROOT_PHYSICAL_RETURN,
    RXOP_ABI_ROOT_EXTERNAL_COPYBACK, RXOP_ABI_ROOT_EXTERNAL_COPIED,
    RXOP_ABI_ROOT_CLI_ARGV, RXOP_ABI_ROOT_RESULT_PRESENT,
    RXOP_ABI_ROOT_OWNED_BASE_MOVE_ELSE_COPY,
    RXOP_ABI_ROOT_STATUS_RECEIVER_OR_SAVED, RXOP_ABI_ROOT_STATUS_ZERO,
    RXOP_ABI_EXECUTION_FINISHED, RXOP_ABI_CANONICAL_RECYCLER_OWNER,
    RXOP_ABI_RECYCLED_FRAME_PRESENT, RXOP_ABI_RETIRING_FRAME,
    RXOP_ABI_FRAME_REFERENCES_MARKED, RXOP_ABI_FRAME_INVOCATION_PRESENT
} RxOpAbiRule;
typedef enum {
    RXOP_ABI_ONCE, RXOP_ABI_EACH_OWNED, RXOP_ABI_OWNED_BEGIN, RXOP_ABI_OWNED_END,
    RXOP_ABI_EACH_PROXY,
    RXOP_ABI_COPYBACK_ARGUMENTS, RXOP_ABI_ROOT_FREE_ARGUMENTS,
    RXOP_ABI_SAVED_ROOT_ARGUMENTS, RXOP_ABI_RECYCLED_BEGIN,
    RXOP_ABI_EACH_RECYCLED_OWNED, RXOP_ABI_RECYCLED_END
} RxOpAbiRepeat;
/* Root modes are conditional native programs, never INTERNAL_PARENT proofs.
 * ROOT_FRAME requires checked native root entry, parent NULL and no invocation
 * completion owner, as established by frame_f on the public entry paths.
 * ROOT_PHYSICAL_RETURN additionally excludes
 * inline-scope RET/SIGRET. ROOT_EXTERNAL_COPIED means ext_proc is nonnull
 * (RXVML run or ordinary descriptor); ROOT_CLI_ARGV is the ext_proc-null argv
 * array path with a remaining argv owner. ROOT_EXTERNAL_COPYBACK also requires
 * ext_args. No rule supplies
 * argument values, receiver presence, clean resources or observer absence.
 *
 * CAPTURE_ROOT_COUNT snapshots the original base-a0 integer with native int
 * conversion, separately before copyback and after its callbacks before free.
 * COPYBACK_ARGUMENTS uses root-copyback-count and tests live ext_argc as an
 * additional bound per iteration. ROOT_FREE_ARGUMENTS uses root-free-count;
 * SAVED_ROOT_ARGUMENTS instead uses immutable frame.number_args. Unknown or
 * out-of-bank counts are unproved, never replaced by another count source.
 * FREE_ROOT_ARGUMENT_UNBIND destroys one original value then nulls its base
 * and active slots before the next iteration; normal free does not unbind.
 *
 * Root register status is captured before result transfer, then selected from
 * the receiver's integer field when present, otherwise the saved field.
 * Result COPY/MOVE callbacks run before DEACTIVATE_ROOT; copyback/free run
 * after deactivation. Root SIGRET resets the current signal diagnostic before
 * deactivation. Abrupt unwind frees before retiring/deactivating root.
 *
 * RECYCLED_BEGIN..RECYCLED_END repeats the entire block in loaded-module,
 * canonical-procedure and recycler LIFO order. It pops a canonical owning
 * recycler before clear_frame's control cleanup, then destroys each original
 * owned-prefix value in order and a0, releases frame-owned decimal/unicode
 * plugins in that order, and releases the frame. Borrowed suffix/globals are
 * not destroyed by those storage rows. FINISH_RUNTIME_STORAGE preserves the
 * existing signal/action/work/interrupt value and diagnostic-owner teardown
 * order; the later CLI argv row destroys only a remaining owner.
 * FINISH_RUNTIME_CONTROL retains
 * debug/profiling, memory/signal/worker teardown and their observations.
 * Retirement retains backing; final destruction is a distinct phase.
 *
 * FRAME_RETIRE expands the existing RETIRE_FRAME helper, not a further
 * operation after retirement. RETIRING_FRAME requires the exact live frame
 * selected for free_frame (ordinary child, root or unwind, independently
 * proved). It leaves inline scopes in LIFO order, clears the saved handler
 * stack with native interrupt-mode restoration, releases only a private COW
 * policy, releases marked original owned-prefix/a0 reference lifetimes and
 * resets the marker, abandons/unpins a present invocation and nulls its owner,
 * then pushes the retained frame to its canonical procedure recycler. None of
 * these rows destroys ordinary retained values or frees decimal/Unicode
 * plugins. clear_frame's later invocation/scopes/handlers/policy/value/plugin
 * destruction is a different program, represented by execution-finish.
 * Scopes, reference cells and invocation windows can run cleanup callbacks;
 * their absence and frame/control observations require separate proof.
 * FRAME_INVOCATION_PRESENT is one native guard before the abandon/unpin
 * group, not a second test between its rows. Unpin rereads the live frame
 * invocation field after abandon and then nulls it; no callback control-state
 * preservation or captured invocation identity is implicit. */
typedef struct {
    RxOpAbiOperation operation;
    RxOpAbiCheckpoint checkpoint;
    RxOpAbiRole source;
    RxOpAbiRole destination;
    RxOpAbiExtent source_extent;
    RxOpAbiExtent destination_extent;
    RxOpAbiRule rule;
    RxOpAbiRepeat repeat;
} RxOpAbiStep;
typedef struct {
    RxOpAbiState state;
    const RxOpAbiStep *steps;
    size_t count;
} RxOpAbiContract;

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

/* Where the primary signal name is obtained. STATIC_NAMES uses the contract's
 * canonical '|' separated name set. Literal and register operands are
 * zero-based RXAS operand positions resolved for each instruction instance;
 * any static_names are additional possible validation signals in their union. */
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
    RXOP_SIGNAL_PROP_ASYNC_ENTRY = 16,
    /* Pending-bit mutation is independent of the structural handler table. */
    RXOP_SIGNAL_PROP_PENDING_WRITE = 32
} RxOpSignalProperties;

/* Normal-path signal control effect. POLICY_WRITE selects a structural table
 * transfer; PENDING_WRITE selects pending-bit control without changing that
 * table. Failure phase locates an effect on exceptional edges. */
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
    /* Complete static set for STATIC_NAMES; optional additional alternatives
     * for operand-selected names, never a replacement for the primary name. */
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

/* Per explicit operand: successful shared setters release old reference and
 * native payloads before assigning the scalar/string/binary component. FIELD
 * writes preserve those unrelated resources. UNKNOWN includes hidden/indexed
 * writes, alias-conditional helpers and unreviewed writers; it proves no clear
 * or absent callback. Whole resets, copies and hidden ABI rows stay separate. */
typedef enum {
    RXOP_VALUE_WRITER_NONE = 0,
    RXOP_VALUE_WRITER_FIELD,
    RXOP_VALUE_WRITER_SETTER,
    RXOP_VALUE_WRITER_UNKNOWN
} RxOpValueWriter;

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
    RxOpConstEvaluator const_evaluator;
    /* 0 no write, f reviewed field/copy, s shared setter, ? unknown/special. */
    const char *value_writers_signature;
    /* Complete inventory of non-resource callback routes for this row. */
    int callback_inventory_complete;
    FlowType flow;
    int optimizer_barrier;
} RxOpEffects;

/* Canonical target-tool contracts. Operand selectors are one-based, with 0
 * denoting absence. Recipes use these selectors, never register numbers.
 * A recipe is an ordered list of primitive operations separated by ';'.
 * Its preparation and pointer capture are one instruction boundary: consumers
 * must not introduce an asynchronous observation between these operations. */
typedef enum {
    RXOP_CALL_NONE = 0,
    RXOP_CALL_DIRECT,
    RXOP_CALL_DYNAMIC,
    RXOP_CALL_FACTORY
} RxOpCallKind;

typedef struct {
    RxOpCallKind kind;
    unsigned int target_operand;
    unsigned int result_operand;
    unsigned int count_operand;
    unsigned int window_operand;
    const char *actual_operands;
    const char *prefix;
    int capture_after_prefix;
    int atomic_prefix;
} RxOpCallContract;

/* Existing CALL failure routes, separated by their native checkpoint. This
 * inventory does not replace the aggregate signal contract or prove that an
 * initializer, provider or hook is absent. Only FRAME_ALLOCATION describes
 * infrastructure allocation pressure; WINDOW and FRAME_LAYOUT are validation
 * obligations. Dispatch-time asynchronous delivery is a separate CFG cause.
 * Rows describe local causes, not cumulative writes from earlier call phases. */
typedef enum {
    RXOP_CALL_CAUSES_NONE = 0,
    RXOP_CALL_CAUSES_KNOWN,
    RXOP_CALL_CAUSES_UNKNOWN
} RxOpCallCauseState;

typedef enum {
    RXOP_CALL_CAUSE_PREPARATION_CALLBACK = 0,
    RXOP_CALL_CAUSE_TARGET,
    RXOP_CALL_CAUSE_TARGET_KIND,
    RXOP_CALL_CAUSE_INITIALIZER,
    RXOP_CALL_CAUSE_WINDOW,
    RXOP_CALL_CAUSE_FRAME_LAYOUT,
    RXOP_CALL_CAUSE_FRAME_ALLOCATION,
    RXOP_CALL_CAUSE_FRAME_CALLBACK,
    RXOP_CALL_CAUSE_PROVIDER,
    RXOP_CALL_CAUSE_CONTEXT_SYNC
} RxOpCallCauseKind;

typedef enum {
    RXOP_CALL_POINT_PREPARATION = 0,
    RXOP_CALL_POINT_TARGET,
    RXOP_CALL_POINT_INITIALIZER,
    RXOP_CALL_POINT_AFTER_INITIALIZER,
    RXOP_CALL_POINT_FRAME_ALLOCATION,
    RXOP_CALL_POINT_FRAME_PREPARATION,
    RXOP_CALL_POINT_PROVIDER
} RxOpCallCheckpoint;

typedef enum {
    RXOP_CALL_CONDITION_RESULT_CLEAR = 0,
    RXOP_CALL_CONDITION_TARGET_UNRESOLVED,
    RXOP_CALL_CONDITION_FIXED_TARGET_NATIVE,
    RXOP_CALL_CONDITION_BYTECODE_INITIALIZER_NEEDED,
    RXOP_CALL_CONDITION_BYTECODE_WINDOW_INVALID,
    RXOP_CALL_CONDITION_BYTECODE_FRAME_LAYOUT_INVALID,
    RXOP_CALL_CONDITION_BYTECODE_ALLOCATION_FAILED,
    RXOP_CALL_CONDITION_BYTECODE_FRAME_HOOK,
    RXOP_CALL_CONDITION_PROVIDER_INVOKED,
    RXOP_CALL_CONDITION_BYTECODE_PROVIDER_INVOKED,
    RXOP_CALL_CONDITION_BYTECODE_DECIMAL_SYNC
} RxOpCallCauseCondition;

typedef struct {
    RxOpCallCauseKind kind;
    RxOpSignalSource source;
    const char *static_names;
    RxOpCallCheckpoint checkpoint;
    RxOpCallCauseCondition condition;
    unsigned callbacks; /* Exact native callback route for this cause. */
} RxOpCallCause;

typedef struct {
    RxOpCallCauseState state;
    const RxOpCallCause *causes;
    size_t count;
} RxOpCallCauseContract;

/* Native frame_f mode, checked beside SAFE_RECYCLED_STACKFRAMES in
 * rxvmintp.c. Changing recycled owned-local cleanup requires updating both
 * the VM and this bytecode-path contract; it must not silently gain purity. */
#define RXOP_CALL_FRAME_REUSED_LOCALS_CLEAR 0

typedef enum {
    RXOP_CALL_FRAME_NONE = 0, RXOP_CALL_FRAME_KNOWN, RXOP_CALL_FRAME_UNKNOWN
} RxOpCallFrameState;
typedef enum {
    RXOP_CALL_FRAME_FRESH_NONE = 0, RXOP_CALL_FRAME_VALUE_INIT,
    RXOP_CALL_FRAME_FRESH_UNKNOWN
} RxOpCallFrameFreshMode;
typedef enum {
    RXOP_CALL_FRAME_CLEAR_NONE = 0, RXOP_CALL_FRAME_REUSED_FRAME,
    RXOP_CALL_FRAME_CLEAR_UNKNOWN
} RxOpCallFrameClearWhen;
typedef enum {
    RXOP_CALL_FRAME_LOCAL_NONE = 0, RXOP_CALL_FRAME_RELINK_ORIGINAL,
    RXOP_CALL_FRAME_LOCAL_UNKNOWN
} RxOpCallFrameLocalMode;

/* Only the bytecode activation path is described, after target/initializer
 * validation. Fresh owned backing (including a0) uses value_init. Reuse
 * relinks owned locals, resets original-owned a0 with value_zero over its
 * active tree, then publishes integer arity to that same backing. Roles and
 * extents share the hidden-ABI vocabulary; NONE/UNKNOWN state grants no such
 * sequence. This is not hook absence, clean recycler history, a borrowed or
 * global storage fact, a callback value definition or an observer premise. */
typedef struct {
    RxOpCallFrameState state;
    RxOpCallFrameFreshMode fresh_mode;
    RxOpAbiRole clear_role;
    RxOpAbiExtent clear_extent;
    RxOpCallFrameClearWhen clear_when;
    RxOpCallFrameLocalMode local_mode;
    RxOpAbiRole count_role;
} RxOpCallFrameContract;

/* These fields describe successful publication of an ordinary bytecode CALL
 * child, after frame_f and argument installation: parent is nonnull, target
 * is bytecode and not program-entry, and this is neither a checked invocation
 * child nor a signal-handler/external/root entry. They are entry facts, not a
 * whole-body ownership theorem. Fresh and reused frames receive the same
 * control resets; inherited plugin pointers are nonowning (loaded_here = 0),
 * policy is parent-borrowed until COW, and is_interrupt is inherited, not zero.
 * Counted/mapped calls additionally publish caller_arg_base; fixed/zero-arg
 * forms leave its UINT32_MAX sentinel. No pointer identity or payload is given. */
typedef enum {
    RXOP_FRAME_ENTRY_PLUGINS, RXOP_FRAME_ENTRY_PLUGIN_OWNERS,
    RXOP_FRAME_ENTRY_INVOCATION, RXOP_FRAME_ENTRY_SCOPE,
    RXOP_FRAME_ENTRY_HANDLERS, RXOP_FRAME_ENTRY_POLICY,
    RXOP_FRAME_ENTRY_REFERENCES, RXOP_FRAME_ENTRY_INTERRUPT,
    RXOP_FRAME_ENTRY_ACTION, RXOP_FRAME_ENTRY_CALLER_WINDOW,
    RXOP_FRAME_ENTRY_FIELD_COUNT
} RxOpFrameControlEntryField;
typedef enum {
    RXOP_FRAME_ENTRY_NONE, RXOP_FRAME_ENTRY_PARENT_BORROWED,
    RXOP_FRAME_ENTRY_EMPTY, RXOP_FRAME_ENTRY_PARENT_BORROWED_COW,
    RXOP_FRAME_ENTRY_UNMARKED, RXOP_FRAME_ENTRY_PARENT_INHERITED,
    RXOP_FRAME_ENTRY_OFF, RXOP_FRAME_ENTRY_MAPPED_WINDOW,
    RXOP_FRAME_ENTRY_UNKNOWN
} RxOpFrameControlEntryValue;
typedef enum {
    RXOP_FRAME_CONTROL_UNKNOWN = 0, RXOP_FRAME_CONTROL_KNOWN
} RxOpFrameControlState;
typedef enum {
    RXOP_FRAME_CONTROL_NONE = 0,
    RXOP_FRAME_CONTROL_PLUGIN_OWNERS = 1,
    RXOP_FRAME_CONTROL_POLICY = 2,
    RXOP_FRAME_CONTROL_HANDLERS = 4,
    RXOP_FRAME_CONTROL_SCOPE = 8,
    RXOP_FRAME_CONTROL_REFERENCES = 16,
    RXOP_FRAME_CONTROL_ANCESTOR_REFERENCES = 32,
    RXOP_FRAME_CONTROL_CHILD = 64,
    RXOP_FRAME_CONTROL_RETIRE = 128,
    RXOP_FRAME_CONTROL_FOREIGN = 256,
    RXOP_FRAME_CONTROL_SIGNAL = 512,
    RXOP_FRAME_CONTROL_UNCLASSIFIED = 1024
} RxOpFrameControlMask;
/* A known mask is the complete conservative intrinsic control-effect
 * inventory, not proof of preservation through a callback or callee. FOREIGN
 * includes resource/host/provider/plugin/ABI callbacks (also opaque native
 * helpers), CHILD requires target/transitive closure, and SIGNAL retains
 * intrinsic delivery paths. Ambient dispatch/reentry and every CFG exit remain
 * separate even when SIGNAL is absent. MKREF may mark any live ancestor;
 * plugins/loaded_here cannot be inferred unchanged across a FOREIGN route.
 * Invalid/incomplete rows are UNKNOWN plus UNCLASSIFIED, never an empty mask.
 * Named-mask lookup returning zero for an unknown name proves no absence. */
typedef struct {
    RxOpFrameControlState state;
    unsigned int mask;
} RxOpFrameControlContract;

/* Current published VM interfaces never install frame-owned decimal/Unicode
 * instances: frame_f resets loaded_here on fresh/reused entry, roots borrow
 * worker-instance-set plugins, and ordinary children inherit nonowning links.
 * RXPA/RXVML expose no stack-frame/loaded_here setter; factory registration,
 * options, plugin values and numeric context retain their separate owners.
 * This invariant covers supported published routes, not privately fabricated
 * frame state or direct internal-structure writes. It promises neither plugin
 * state immutability, numeric-sync purity nor callback/observation absence.
 * A future supported frame-owner installation must revise this contract. */
typedef enum {
    RXOP_FRAME_PLUGIN_OWNERSHIP_UNKNOWN = 0,
    RXOP_FRAME_PLUGIN_OWNERSHIP_NONE_PUBLISHED_INTERFACE_V1
} RxOpFramePluginOwnership;

/* Selection resolves a procedure pointer without invoking its body. The legacy
 * FACTORY call view is derived from this descriptor for source compatibility. */
typedef enum {
    RXOP_SELECTION_NONE = 0,
    RXOP_SELECTION_METHOD,
    RXOP_SELECTION_FACTORY
} RxOpSelectionKind;

typedef struct {
    RxOpSelectionKind kind;
    unsigned int result_operand;
    unsigned int descriptor_operand;
    unsigned int receiver_operand;
    unsigned int count_operand;
    unsigned int window_operand;
} RxOpSelectionContract;

typedef enum {
    RXOP_RELOC_NONE = 0,
    RXOP_RELOC_REGISTER,
    RXOP_RELOC_LABEL,
    RXOP_RELOC_PROCEDURE,
    RXOP_RELOC_STRING,
    RXOP_RELOC_BINARY,
    RXOP_RELOC_DECIMAL,
    RXOP_RELOC_LOCAL_INDEX,
    RXOP_RELOC_JUMP_TABLE,
    RXOP_RELOC_UNKNOWN,
    RXOP_RELOC_INLINE_RECIPE,
    RXOP_RELOC_MODULE
} RxOpRelocationKind;

/* Numeric settings are independent domains. Plugin/locale/external facts
 * are dependencies, not a claim that a successful operation changes numeric
 * settings. The existing RXOP_CONTEXT_NUMERIC API remains a coarse view. */
typedef enum {
    RXOP_CONTEXT_READ_NONE = 0,
    RXOP_CONTEXT_DIGITS = 1,
    RXOP_CONTEXT_FUZZ = 2,
    RXOP_CONTEXT_FORM = 4,
    RXOP_CONTEXT_CASE = 8,
    RXOP_CONTEXT_STANDARD = 16,
    RXOP_CONTEXT_ALL_NUMERIC = 31,
    RXOP_CONTEXT_PLUGIN = 32,
    RXOP_CONTEXT_LOCALE = 64,
    RXOP_CONTEXT_EXTERNAL = 128,
    RXOP_CONTEXT_HANDLER = 256,
    RXOP_CONTEXT_UNKNOWN = 512
} RxOpContextAccessMask;

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
RxOpValueWriter rxop_value_writer(int opcode, size_t operand_index);
size_t rxop_effect_count(void);
int rxop_effect_reads_operand(const RxOpEffects *effects, size_t operand_index);
int rxop_effect_writes_operand(const RxOpEffects *effects, size_t operand_index);
int rxop_effect_kills_operand(const RxOpEffects *effects, size_t operand_index);
int rxop_effect_branch_target_operand(const RxOpEffects *effects, size_t operand_index);
/* Normal edges use declared target-operand order, followed by lexical
 * fallthrough when present. -1 means ordinary FLOW_NEXT; -2 means unknown or
 * indirect control. Exceptional label references are not normal successors. */
int rxop_normal_successors(int opcode);
int rxop_normal_fallthrough(int opcode);
RxOpBindingAction rxop_binding_action(int opcode);
unsigned int rxop_activation_effects(int opcode);
RxOpCloneCategory rxop_clone_category(int opcode);
unsigned int rxop_component_reads(int opcode, size_t operand_index);
unsigned int rxop_component_writes(int opcode, size_t operand_index);
/* Whether optional cleanup has a known component-write projection. False
 * means unknown projection, not no write. A true result with NONE components
 * can describe a pointer-slot write that preserves its pointee. True does not
 * mean an unconditional definition on exceptional/resumed paths or authorize
 * deleting the opcode. */
int rxop_component_write_is_known(int opcode, size_t operand_index);
int rxop_compare_branch_fusion(int compare_opcode, int branch_opcode,
                               RxOpCompareBranchFusion *fusion);
/* True when a two-register copy is guaranteed to perform no work, signal, or
 * observable value/effect update if both operands denote the same physical
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
RxOpCallContract rxop_call_contract(int opcode);
int rxop_call_invokes(int opcode);
RxOpCallCauseContract rxop_call_cause_contract(int opcode);
const char *rxop_call_cause_state_name(RxOpCallCauseState value);
const char *rxop_call_cause_kind_name(RxOpCallCauseKind value);
const char *rxop_call_checkpoint_name(RxOpCallCheckpoint value);
const char *rxop_call_cause_condition_name(RxOpCallCauseCondition value);
RxOpCallFrameContract rxop_call_frame_contract(int opcode);
const char *rxop_call_frame_state_name(RxOpCallFrameState value);
const char *rxop_call_frame_fresh_mode_name(RxOpCallFrameFreshMode value);
const char *rxop_call_frame_clear_when_name(RxOpCallFrameClearWhen value);
const char *rxop_call_frame_local_mode_name(RxOpCallFrameLocalMode value);
RxOpCallFrameState rxop_call_frame_control_state(int opcode);
RxOpFrameControlEntryValue rxop_call_frame_control_entry(int opcode, RxOpFrameControlEntryField field);
const char *rxop_frame_control_entry_field_name(RxOpFrameControlEntryField field);
const char *rxop_frame_control_entry_value_name(RxOpFrameControlEntryValue value);
RxOpFrameControlContract rxop_frame_control_contract(int opcode);
const char *rxop_frame_control_state_name(RxOpFrameControlState state);
unsigned int rxop_frame_control_mask_named(const char *name);
RxOpFramePluginOwnership rxop_frame_plugin_ownership(void);
const char *rxop_frame_plugin_ownership_name(RxOpFramePluginOwnership state);
/* Descriptor-equivalent fixed layout; zero means unavailable. Counted calls
 * do not become fixed-actual calls across an initializer capture checkpoint. */
int rxop_call_variant(int opcode, size_t count);
unsigned int rxop_selection_window_components(int opcode);
/* Positive callback routes and their conditional resource witnesses. A zero
 * selector/host predicate is not a global theorem about native payload hooks.
 * Capacity callbacks require physical ownership proof, not logical count0. */
/* A zero obligation is a reviewed complete no-hook inventory. Unknown is a
 * missing inventory, not evidence that an opaque opcode necessarily calls out.
 * Resource inputs include copy-source hooks. Capacity requires physical extent
 * evidence and is not discharged by an active attribute count of zero. */
typedef enum {
    RXOP_CALLBACK_NONE = 0,
    RXOP_CALLBACK_INPUT = 1u,
    RXOP_CALLBACK_OUTPUT = 2u,
    RXOP_CALLBACK_CAPACITY = 4u,
    RXOP_CALLBACK_SELECTION = 8u,
    RXOP_CALLBACK_HOST = 16u,
    RXOP_CALLBACK_ABI = 32u,
    RXOP_CALLBACK_DEPARTURE = 64u,
    RXOP_CALLBACK_INVOCATION = 128u,
    RXOP_CALLBACK_UNKNOWN = 256u,
    /* Conditional on an active decimal provider. ABI 3 makes synchronization
     * value-only and non-reentrant; numeric writes remain separate effects. */
    RXOP_CALLBACK_PLUGIN_SYNC = 512u
} RxOpCallbackObligation;
unsigned int rxop_callback_obligations(int opcode);
/* Reachable-value surface of the complete public callback route, not its
 * complete observation/effect set. BYTES_SNAPSHOT passes temporary copied
 * bytes, never an operand value handle or suspended frame coordinates.
 * With a separate original-owned/nonescape/alias proof this can bound access
 * to one private backing. Reentry, retained handles/provider state, pending
 * signals, context/value havoc, I/O failure and physical/private observers
 * remain separate obligations. Arbitrary payload callbacks stay UNKNOWN. */
typedef enum {
    RXOP_CALLBACK_VALUES_NONE = 0,
    RXOP_CALLBACK_VALUES_BYTES_SNAPSHOT,
    RXOP_CALLBACK_VALUES_UNKNOWN
} RxOpCallbackValueSurface;
RxOpCallbackValueSurface rxop_callback_value_surface(int opcode);
const char *rxop_callback_value_surface_name(RxOpCallbackValueSurface value);
/* Native execution control includes possible callback-raised interrupts;
 * rxop_signal_contract remains the intrinsic instruction contract. */
RxOpSignalContract rxop_execution_signal_contract(int opcode);
/* One-based explicit receiver for a complete single shared setter normal
 * postwrite profile; zero means unknown. The receiver and computed argument
 * are captured before its resource hook. Only native-payload absence is
 * guaranteed after the final reset; a hook may reinstall a reference. */
unsigned int rxop_callback_postwrite_operand(int opcode);
/* A normally returning resource-only public callback preserves the suspended
 * frame's pointer tables and structural signal-policy table. This describes
 * only the callback: explicit instruction effects remain, and pointees,
 * pending interrupts, numeric/plugin context and delivered handlers do not get
 * this premise. */
int rxop_callback_normal_preserves_frame(int opcode);
/* The complete reviewed callback route preserves the suspended frame's
 * structural signal-policy table, including before queued interrupt dispatch.
 * Public resource-only and bytes-output routes qualify. Explicit instruction
 * policy effects, pending signals, nested execution, Rexx values/bindings,
 * physical-frame observations and output failure remain independent. This is
 * weaker than callback_normal_preserves_frame; zero means unproved. */
int rxop_callback_preserves_signal_policy(int opcode);


int rxop_selection_callbacks(int opcode);
int rxop_host_callbacks(int opcode);
int rxop_context_sync_callbacks(int opcode);
int rxop_abi_callbacks(int opcode);
int rxop_departure_callbacks(int opcode);
/* Complete intrinsic Rexx-value reads/writes are described by the typed
 * operand inventory. This excludes separately retained host callbacks,
 * external output, signals and their reentry effects. False is unknown,
 * never an empty footprint; true does not remove SEM_OPAQUE barriers. */
int rxop_intrinsic_value_footprint_is_known(int opcode);
/* Physical-return rows require an ordinary internal parent. External roots,
 * interrupt-action/invocation completion and unmapped unwind remain unknown.
 * Separate root/finish modes describe their conditional native programs;
 * selecting those rows does not prove resource or observation absence and
 * never turns a raw root into INTERNAL_PARENT. See the root-role/repeat
 * contract beside RxOpAbiStep for entry variants and count snapshots.
 * OWNED_BEGIN..OWNED_END is one repeated body: clear/restore slot0, then
 * clear/restore slot1, etc. Never execute two whole-array phases. A result
 * transfer's equal-pointer no-op does not suppress any later leave row.
 * Resource extents are conservative maximum obligations; callers may refine
 * them only with the matching native operation's checked point facts. */
RxOpAbiContract rxop_abi_contract(int opcode, RxOpAbiMode mode);
const char *rxop_abi_state_name(RxOpAbiState value);
const char *rxop_abi_mode_name(RxOpAbiMode value);
const char *rxop_abi_operation_name(RxOpAbiOperation value);
const char *rxop_abi_checkpoint_name(RxOpAbiCheckpoint value);
const char *rxop_abi_role_name(RxOpAbiRole value);
const char *rxop_abi_extent_name(RxOpAbiExtent value);
const char *rxop_abi_rule_name(RxOpAbiRule value);
const char *rxop_abi_repeat_name(RxOpAbiRepeat value);
unsigned int rxop_callback_inputs(int opcode, size_t operand_index);
unsigned int rxop_callback_outputs(int opcode, size_t operand_index);
int rxop_callback_capacity(int opcode, size_t operand_index);
unsigned int rxop_call_target_components(int opcode);
/* Conservative possible non-output observability, for discard proof only.
 * It is not a structural cloning/admission classification. */
int rxop_residual_observable(int opcode);
/* Current INLINE_SCOPES feature contract, with logical detached diagnostics.
 * This is not a no-scope diagnostic or older-runtime compatibility promise. */
int rxop_scoped_diagnostics(void);
RxOpSelectionContract rxop_selection_contract(int opcode);
/* Ordered non-call binding/compound preparation; an empty string is a proved
 * absence only for a classified instruction without binding effects. */
const char *rxop_binding_recipe(int opcode);
const char *rxop_implicit_recipe(int opcode);
/* Return kind: 0 nonreturn, 1 void, 2 owned-or-borrowed register value,
 * 3 integer component, 4 float component, 5 string component. */
int rxop_return_kind(int opcode);
unsigned int rxop_return_source_operand(int opcode);
/* Prior result facts invalidated by the return setter, before installing its
 * transferred/literal component. Masks are analysis metadata, not wire ABI.
 * Register COPY/MOVE supplies all result domains from its source separately. */
unsigned int rxop_return_result_component_invalidations(int opcode);
uint32_t rxop_return_result_flag_invalidations(int opcode);
unsigned int rxop_scope_continuation_operand(int opcode);
unsigned int rxop_recipe_operand(int opcode);
/* Exact unfused counterpart for fixed implicit selector forms, or -1. */
int rxop_explicit_variant(int opcode);
RxOpRelocationKind rxop_operand_relocation(int opcode, size_t operand_index);
unsigned int rxop_context_read_mask(int opcode);
unsigned int rxop_context_write_mask(int opcode);
unsigned int rxop_observation_mask(int opcode);
/* Complete conservative may-access masks. These include all published value
 * components/flags that an operation can touch; they never assert a must-write
 * or value-independent kill. Precise optional projections remain separate. */
unsigned int rxop_component_may_reads(int opcode, size_t operand_index);
unsigned int rxop_observed_components(int opcode, size_t operand_index);
uint32_t rxop_observed_flags(int opcode, size_t operand_index);
int rxop_observation_is_known(int opcode, size_t operand_index);
unsigned int rxop_observed_flags_mask_operand(int opcode);
unsigned int rxop_component_may_writes(int opcode, size_t operand_index);
uint32_t rxop_flag_may_reads(int opcode, size_t operand_index);
uint32_t rxop_flag_may_writes(int opcode, size_t operand_index);
/* Optional value facts on a successful normal edge. A definition requires
 * availability of its declared source reads. A transport propagates source
 * availability and never invents a definition for an unavailable component. */
unsigned int rxop_definition_components(int opcode, size_t operand_index);
unsigned int rxop_transport_source_operand(int opcode);
unsigned int rxop_transport_components(int opcode);
unsigned int rxop_literal_operand(int opcode);
unsigned int rxop_literal_components(int opcode);
RxOpSignalContract rxop_signal_contract(int opcode);
size_t rxop_signal_contract_count(void);
int rxop_can_signal(int opcode);
int rxop_signal_failure_writes_operand(const RxOpSignalContract *contract,
                                       size_t operand_index);
RxOpSignalPhase rxop_signal_phase(int opcode);

void *src_inst(const char* name, OperandType op1, OperandType op2, OperandType op3);
void *src_instv(const char *name, const OperandType *operands, size_t operand_count);

#endif // RXDEFS_H
