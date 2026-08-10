/*
 * Internal VM instrumentation contract.
 *
 * Backends are selected at compile time.  The default backend intentionally
 * preprocesses to no code: ordinary VM builds pay no branch, callback, state,
 * or argument-evaluation cost for these extension points.
 */

#ifndef CREXX_RXVMINSTRUMENT_H
#define CREXX_RXVMINSTRUMENT_H

typedef enum rxvm_transition_reason {
    RXVM_TRANSITION_SEQUENTIAL = 0,
    RXVM_TRANSITION_BRANCH,
    RXVM_TRANSITION_CALL,
    RXVM_TRANSITION_RETURN,
    RXVM_TRANSITION_INTERRUPT_ENTRY,
    RXVM_TRANSITION_INTERRUPT_RESUME,
    RXVM_TRANSITION_EXTERNAL_ENTRY,
    RXVM_TRANSITION_TERMINAL,
    RXVM_TRANSITION_COUNT
} rxvm_transition_reason;

#ifndef RXVM_INSTRUMENTATION_BACKEND

#define RXVM_INSTRUMENTATION_STATE()
#define RXVM_INSTRUMENTATION_VM_BEGIN(context_) ((void)0)
#define RXVM_INSTRUMENTATION_VM_END(context_, result_) ((void)0)
#define RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(module_, index_, opcode_, handler_inline_) ((void)0)
#define RXVM_INSTRUMENTATION_INSTRUCTION_RETIRE(target_module_, target_index_, reason_) ((void)0)
#define RXVM_INSTRUMENTATION_INSTRUCTION_TERMINAL(module_, index_, reason_) ((void)0)
#define RXVM_INSTRUMENTATION_FRAME_ACTIVATE(frame_, module_, index_, reason_) ((void)0)
#define RXVM_INSTRUMENTATION_NATIVE_BEGIN(procedure_) ((void)0)
#define RXVM_INSTRUMENTATION_NATIVE_END() ((void)0)
#define RXVM_INSTRUMENTATION_MODULES_CHANGED(context_) ((void)0)
#define RXVM_INSTRUMENTATION_TRANSITION(reason_) ((void)0)
#define RXVM_INSTRUMENTATION_CURRENT_TRANSITION() RXVM_TRANSITION_SEQUENTIAL
#define RXVM_INSTRUMENTATION_INTERRUPT_POLL() ((void)0)
#define RXVM_INSTRUMENTATION_INTERRUPT_SCAN_BEGIN(module_, index_) ((void)0)
#define RXVM_INSTRUMENTATION_INTERRUPT_SELECT(signal_, module_, index_) ((void)0)
#define RXVM_INSTRUMENTATION_INTERRUPT_ENTRY(signal_, module_, index_) ((void)0)
#define RXVM_INSTRUMENTATION_INTERRUPT_RESUME(signal_, module_, index_) ((void)0)
#define RXVM_INSTRUMENTATION_INTERRUPT_TERMINAL(signal_, module_, index_) ((void)0)
#define RXVM_INSTRUMENTATION_CALL(path_, procedure_, arity_, disposition_, outcome_, caller_, module_, index_, argument_base_, has_window_) ((void)0)
#define RXVM_INSTRUMENTATION_RETURN(placement_) ((void)0)
#define RXVM_INSTRUMENTATION_DYNAMIC(kind_, outcome_) ((void)0)
#define RXVM_INSTRUMENTATION_SWAP(frame_, register_1_, register_2_) ((void)0)
#define RXVM_INSTRUMENTATION_SIGNAL_UNWIND(frames_, windows_, slots_, failed_) ((void)0)
#define RXVM_INSTRUMENTATION_SIGNAL_NATIVE_RESTORE(observed_, slots_, failed_) ((void)0)
#define RXVM_INSTRUMENTATION_VALUE_TYPED(operation_, shape_, bytes_) ((void)0)

#else
#include RXVM_INSTRUMENTATION_BACKEND
#endif

#endif
