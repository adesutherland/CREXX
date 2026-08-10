/* Timing/count backend for the internal VM instrumentation contract. */

#ifndef CREXX_RXVMINSTRUMENT_PROFILE_H
#define CREXX_RXVMINSTRUMENT_PROFILE_H

#include "rxvmprofile.h"
#include "rxvmsequence.h"

#ifdef NTHREADED
#define RXVM_PROFILE_VM_MODE "rxbvm"
#else
#define RXVM_PROFILE_VM_MODE "rxvm"
#endif

#define RXVM_INSTRUMENTATION_STATE() \
    rxvm_profile_state vm_profile; rxvm_sequence_state vm_sequence

#define RXVM_INSTRUMENTATION_VM_BEGIN(context_)                                \
    do {                                                                        \
        rxvm_profile_begin(&vm_profile,                                        \
                           (context_)->profile_mode && !(context_)->prepare_only, \
                           (context_));                                        \
        rxvm_sequence_begin(&vm_sequence, (context_),                           \
                (context_)->sequence_count,                                    \
                (context_)->sequence_count && !(context_)->prepare_only);       \
    } while (0)

#define RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(module_, index_, opcode_, handler_inline_) \
    do {                                                                        \
        (void)(module_); (void)(index_);                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_instruction_begin_at(                                 \
                    &vm_profile, (size_t)(module_), (size_t)(index_),           \
                    (int)(opcode_), (int)(handler_inline_),                     \
                    rxvm_profile_timestamp(&vm_profile));                       \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_trace_instruction_at(                                 \
                    &vm_profile, current_frame, pc, meta_map,                   \
                    (size_t)(module_), (size_t)(index_), (int)(opcode_));        \
        if (vm_sequence.enabled)                                                \
            rxvm_sequence_instruction_begin(                                   \
                    &vm_sequence, (size_t)(module_), (size_t)(index_));          \
    } while (0)

#define RXVM_INSTRUMENTATION_INSTRUCTION_RETIRE(target_module_, target_index_, reason_) \
    do {                                                                        \
        (void)(target_module_); (void)(target_index_);                          \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_instruction_retire_at(                                \
                    &vm_profile, (size_t)(target_module_),                      \
                    (size_t)(target_index_), (reason_),                         \
                    rxvm_profile_timestamp(&vm_profile));                       \
        if (vm_sequence.enabled)                                                \
            rxvm_sequence_instruction_retire(&vm_sequence, (reason_));          \
    } while (0)

#define RXVM_INSTRUMENTATION_INSTRUCTION_TERMINAL(module_, index_, reason_)    \
    do {                                                                        \
        (void)(module_); (void)(index_); (void)(reason_);                       \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_instruction_terminal_at(                              \
                    &vm_profile, rxvm_profile_timestamp(&vm_profile));          \
        if (vm_sequence.enabled) rxvm_sequence_break(&vm_sequence);             \
    } while (0)

#define RXVM_INSTRUMENTATION_FRAME_ACTIVATE(frame_, module_, index_, reason_)  \
    do {                                                                        \
        rxvm_transition_reason vm_profile_reason__ = (reason_);                 \
        (void)(module_); (void)(index_);                                        \
        if (vm_profile.enabled) {                                               \
            uint64_t vm_profile_now__ =                                        \
                    (vm_profile_reason__ == RXVM_TRANSITION_EXTERNAL_ENTRY ||   \
                     vm_profile_reason__ == RXVM_TRANSITION_INTERRUPT_ENTRY ||  \
                     (vm_profile_reason__ == RXVM_TRANSITION_RETURN &&          \
                      !vm_profile.instruction_active))                         \
                    ? rxvm_profile_timestamp(&vm_profile)                       \
                    : vm_profile.instruction_start_ns;                          \
            rxvm_profile_frame_activate_at(                                    \
                    &vm_profile, (const void *)(frame_),                        \
                    (frame_)->procedure->profile_id, vm_profile_reason__,       \
                    vm_profile_now__);                                         \
        }                                                                       \
        if (vm_sequence.enabled &&                                             \
                vm_profile_reason__ != RXVM_TRANSITION_SEQUENTIAL)              \
            rxvm_sequence_break(&vm_sequence);                                 \
    } while (0)

#define RXVM_INSTRUMENTATION_NATIVE_BEGIN(procedure_)                          \
    do {                                                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_native_begin_at(                                      \
                    &vm_profile, (procedure_)->profile_id,                      \
                    rxvm_profile_timestamp(&vm_profile));                       \
    } while (0)

#define RXVM_INSTRUMENTATION_NATIVE_END()                                      \
    do {                                                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_native_end_at(                                        \
                    &vm_profile, rxvm_profile_timestamp(&vm_profile));          \
    } while (0)

#define RXVM_INSTRUMENTATION_MODULES_CHANGED(context_)                         \
    do {                                                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_refresh_catalog(&vm_profile, (context_));              \
    } while (0)

#define RXVM_INSTRUMENTATION_TRANSITION(reason_)                               \
    do { vm_profile.current_transition = (reason_); } while (0)
#define RXVM_INSTRUMENTATION_CURRENT_TRANSITION() vm_profile.current_transition

#define RXVM_INSTRUMENTATION_INTERRUPT_POLL()                                 \
    do { rxvm_profile_interrupt_poll(&vm_profile); } while (0)

#define RXVM_INSTRUMENTATION_INTERRUPT_SCAN_BEGIN(module_, index_)             \
    do {                                                                        \
        (void)(module_); (void)(index_);                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_interrupt_scan_begin_at(                              \
                    &vm_profile, rxvm_profile_timestamp(&vm_profile));          \
    } while (0)

#define RXVM_INSTRUMENTATION_INTERRUPT_SELECT(signal_, module_, index_)        \
    do {                                                                        \
        (void)(module_); (void)(index_);                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_interrupt_select_at(                                  \
                    &vm_profile, (unsigned char)(signal_),                      \
                    rxvm_profile_timestamp(&vm_profile));                       \
        if (vm_sequence.enabled) rxvm_sequence_break(&vm_sequence);             \
    } while (0)

#define RXVM_INSTRUMENTATION_INTERRUPT_ENTRY(signal_, module_, index_)         \
    do {                                                                        \
        (void)(module_); (void)(index_);                                        \
        rxvm_profile_interrupt_entry(&vm_profile, (unsigned char)(signal_));    \
    } while (0)

#define RXVM_INSTRUMENTATION_INTERRUPT_RESUME(signal_, module_, index_)        \
    do {                                                                        \
        (void)(module_); (void)(index_);                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_interrupt_resume_at(                                  \
                    &vm_profile, (unsigned char)(signal_),                      \
                    rxvm_profile_timestamp(&vm_profile));                       \
    } while (0)

#define RXVM_INSTRUMENTATION_INTERRUPT_TERMINAL(signal_, module_, index_)      \
    do {                                                                        \
        (void)(module_); (void)(index_);                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_interrupt_terminal_at(                                \
                    &vm_profile, (unsigned char)(signal_),                      \
                    rxvm_profile_timestamp(&vm_profile));                       \
    } while (0)

#define RXVM_INSTRUMENTATION_CALL(path_, procedure_, arity_, disposition_, outcome_, caller_, module_, index_, argument_base_, has_window_) \
    do {                                                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_record_call_at(                                       \
                    &vm_profile, (path_),                                      \
                    (procedure_) ? (procedure_)->profile_id : SIZE_MAX,         \
                    (int64_t)(arity_), (disposition_), (outcome_),              \
                    (const void *)(caller_), (size_t)(module_),                 \
                    (size_t)(index_), (size_t)(argument_base_),                 \
                    (has_window_));                                             \
    } while (0)

#define RXVM_INSTRUMENTATION_RETURN(placement_)                                \
    do {                                                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_record_return_at(&vm_profile, (placement_));           \
    } while (0)

#define RXVM_INSTRUMENTATION_DYNAMIC(kind_, outcome_)                          \
    do {                                                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_record_dynamic_at(&vm_profile, (kind_), (outcome_));   \
    } while (0)

#define RXVM_INSTRUMENTATION_SWAP(frame_, register_1_, register_2_)            \
    do {                                                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_record_swap_at(                                       \
                    &vm_profile, (const void *)(frame_),                        \
                    (size_t)(register_1_), (size_t)(register_2_));               \
    } while (0)

#define RXVM_INSTRUMENTATION_SIGNAL_UNWIND(frames_, windows_, slots_, failed_) \
    do {                                                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_record_signal_unwind_at(                              \
                    &vm_profile, (uint64_t)(frames_), (uint64_t)(windows_),      \
                    (uint64_t)(slots_), (failed_));                             \
    } while (0)

#define RXVM_INSTRUMENTATION_SIGNAL_NATIVE_RESTORE(observed_, slots_, failed_) \
    do {                                                                        \
        if (vm_profile.enabled)                                                 \
            rxvm_profile_record_signal_native_restore_at(                      \
                    &vm_profile, (observed_), (uint64_t)(slots_), (failed_));   \
    } while (0)

#define RXVM_INSTRUMENTATION_VALUE_TYPED(operation_, shape_, bytes_)           \
    do {                                                                        \
        rxvm_profile_record_value_typed((operation_), (shape_),                \
                                        (size_t)(bytes_));                      \
    } while (0)

#define RXVM_INSTRUMENTATION_VM_END(context_, result_)                         \
    do {                                                                        \
        rxvm_profile_finish_value_census(&vm_profile, (context_));              \
        rxvm_profile_report(&vm_profile, (context_)->profile_output,            \
                            RXVM_PROFILE_VM_MODE, (result_), meta_map,          \
                            interrupt_to_string);                               \
        rxvm_sequence_report(&vm_sequence, (context_),                          \
                             (context_)->sequence_output, (result_));           \
        rxvm_profile_destroy(&vm_profile);                                      \
    } while (0)

#endif
