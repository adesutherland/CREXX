/* Compile-time test backend for the internal VM instrumentation contract. */

#ifndef CREXX_RXVMINSTRUMENT_TEST_H
#define CREXX_RXVMINSTRUMENT_TEST_H

#define CREXX_VM_INSTRUMENTATION_TEST 1

typedef struct rxvm_test_instrumentation_state {
    size_t instruction_begins;
    size_t instruction_retires;
    size_t instruction_terminals;
    size_t frame_activations;
    size_t branches;
    size_t calls;
    size_t returns;
    size_t interrupt_polls;
    size_t interrupt_scans;
    size_t interrupt_selections;
    size_t interrupt_entries;
    size_t interrupt_resumes;
    size_t interrupt_terminals;
    size_t begin_module;
    size_t begin_index;
    rxvm_transition_reason transition;
    int instruction_active;
    int failed;
} rxvm_test_instrumentation_state;

#define RXVM_INSTRUMENTATION_STATE()                                            \
    rxvm_test_instrumentation_state vm_instrumentation = {0}

#define RXVM_INSTRUMENTATION_VM_BEGIN(context_)                                 \
    do {                                                                        \
        (void)(context_);                                                       \
        vm_instrumentation.transition = RXVM_TRANSITION_SEQUENTIAL;             \
    } while (0)

#define RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(module_, index_, opcode_, handler_inline_) \
    do {                                                                        \
        size_t vm_module__ = (size_t)(module_);                                 \
        size_t vm_index__ = (size_t)(index_);                                   \
        (void)(opcode_);                                                        \
        (void)(handler_inline_);                                                \
        if (!vm_module__ || vm_instrumentation.instruction_active)              \
            vm_instrumentation.failed = 1;                                      \
        vm_instrumentation.begin_module = vm_module__;                          \
        vm_instrumentation.begin_index = vm_index__;                            \
        vm_instrumentation.instruction_active = 1;                              \
        vm_instrumentation.instruction_begins++;                                \
        vm_instrumentation.transition = RXVM_TRANSITION_SEQUENTIAL;             \
    } while (0)

#define RXVM_INSTRUMENTATION_INSTRUCTION_RETIRE(target_module_, target_index_, reason_) \
    do {                                                                        \
        size_t vm_target_module__ = (size_t)(target_module_);                   \
        (void)(target_index_);                                                  \
        if (vm_instrumentation.instruction_active) {                            \
            if (!vm_instrumentation.begin_module || !vm_target_module__)        \
                vm_instrumentation.failed = 1;                                  \
            vm_instrumentation.instruction_active = 0;                          \
            vm_instrumentation.instruction_retires++;                           \
            if ((reason_) == RXVM_TRANSITION_BRANCH) vm_instrumentation.branches++; \
            else if ((reason_) == RXVM_TRANSITION_CALL) vm_instrumentation.calls++; \
            else if ((reason_) == RXVM_TRANSITION_RETURN) vm_instrumentation.returns++; \
        }                                                                       \
    } while (0)

#define RXVM_INSTRUMENTATION_INSTRUCTION_TERMINAL(module_, index_, reason_)      \
    do {                                                                        \
        (void)(module_); (void)(index_); (void)(reason_);                       \
        if (vm_instrumentation.instruction_active) {                            \
            vm_instrumentation.instruction_active = 0;                          \
            vm_instrumentation.instruction_terminals++;                         \
        }                                                                       \
    } while (0)

#define RXVM_INSTRUMENTATION_FRAME_ACTIVATE(frame_, module_, index_, reason_)   \
    do {                                                                        \
        (void)(frame_); (void)(index_);                                         \
        if (!(module_)) vm_instrumentation.failed = 1;                          \
        vm_instrumentation.frame_activations++;                                 \
        if ((reason_) == RXVM_TRANSITION_CALL) vm_instrumentation.calls++;      \
        else if ((reason_) == RXVM_TRANSITION_RETURN) vm_instrumentation.returns++; \
    } while (0)

#define RXVM_INSTRUMENTATION_NATIVE_BEGIN(procedure_) do { (void)(procedure_); } while (0)
#define RXVM_INSTRUMENTATION_NATIVE_END() ((void)0)
#define RXVM_INSTRUMENTATION_MODULES_CHANGED(context_) do { (void)(context_); } while (0)

#define RXVM_INSTRUMENTATION_TRANSITION(reason_)                                \
    do { vm_instrumentation.transition = (reason_); } while (0)
#define RXVM_INSTRUMENTATION_CURRENT_TRANSITION() vm_instrumentation.transition

#define RXVM_INSTRUMENTATION_INTERRUPT_POLL()                             \
    do { vm_instrumentation.interrupt_polls++; } while (0)
#define RXVM_INSTRUMENTATION_INTERRUPT_SCAN_BEGIN(module_, index_)        \
    do { (void)(module_); (void)(index_); vm_instrumentation.interrupt_scans++; } while (0)
#define RXVM_INSTRUMENTATION_INTERRUPT_SELECT(signal_, module_, index_)         \
    do { (void)(signal_); (void)(module_); (void)(index_); vm_instrumentation.interrupt_selections++; } while (0)
#define RXVM_INSTRUMENTATION_INTERRUPT_ENTRY(signal_, module_, index_)          \
    do { (void)(signal_); (void)(module_); (void)(index_); vm_instrumentation.interrupt_entries++; } while (0)
#define RXVM_INSTRUMENTATION_INTERRUPT_RESUME(signal_, module_, index_)         \
    do { (void)(signal_); (void)(module_); (void)(index_); vm_instrumentation.interrupt_resumes++; } while (0)
#define RXVM_INSTRUMENTATION_INTERRUPT_TERMINAL(signal_, module_, index_)       \
    do { (void)(signal_); (void)(module_); (void)(index_); vm_instrumentation.interrupt_terminals++; } while (0)

#define RXVM_INSTRUMENTATION_VM_END(context_, result_)                          \
    do {                                                                        \
        (void)(context_);                                                       \
        if (vm_instrumentation.instruction_active ||                            \
            vm_instrumentation.instruction_begins !=                            \
                vm_instrumentation.instruction_retires +                        \
                vm_instrumentation.instruction_terminals ||                     \
            (vm_instrumentation.instruction_begins &&                           \
             !vm_instrumentation.frame_activations))                            \
            vm_instrumentation.failed = 1;                                      \
        if (!vm_instrumentation.instruction_begins) break;                      \
        if (vm_instrumentation.failed) {                                        \
            fprintf(stderr, "VM instrumentation: FAIL begins=%zu retires=%zu terminals=%zu frames=%zu\n", \
                    vm_instrumentation.instruction_begins,                      \
                    vm_instrumentation.instruction_retires,                     \
                    vm_instrumentation.instruction_terminals,                   \
                    vm_instrumentation.frame_activations);                      \
            (result_) = RXSIGNAL_FAILURE;                                       \
        } else {                                                                \
            fprintf(stderr, "VM instrumentation: PASS begins=%zu retires=%zu terminals=%zu frames=%zu branches=%zu calls=%zu returns=%zu interrupts=%zu/%zu/%zu/%zu polls=%zu scans=%zu\n", \
                    vm_instrumentation.instruction_begins,                      \
                    vm_instrumentation.instruction_retires,                     \
                    vm_instrumentation.instruction_terminals,                   \
                    vm_instrumentation.frame_activations,                       \
                    vm_instrumentation.branches, vm_instrumentation.calls,      \
                    vm_instrumentation.returns,                                 \
                    vm_instrumentation.interrupt_selections,                    \
                    vm_instrumentation.interrupt_entries,                       \
                    vm_instrumentation.interrupt_resumes,                       \
                    vm_instrumentation.interrupt_terminals,                     \
                    vm_instrumentation.interrupt_polls,                         \
                    vm_instrumentation.interrupt_scans);                        \
        }                                                                       \
    } while (0)

#define RXVM_INSTRUMENTATION_CALL(path_, procedure_, arity_, disposition_, outcome_, caller_, module_, index_, argument_base_, has_window_) ((void)0)
#define RXVM_INSTRUMENTATION_RETURN(placement_) ((void)0)
#define RXVM_INSTRUMENTATION_DYNAMIC(kind_, outcome_) ((void)0)
#define RXVM_INSTRUMENTATION_SWAP(frame_, register_1_, register_2_) ((void)0)
#define RXVM_INSTRUMENTATION_SIGNAL_UNWIND(frames_, windows_, slots_, failed_) ((void)0)
#define RXVM_INSTRUMENTATION_SIGNAL_NATIVE_RESTORE(observed_, slots_, failed_) ((void)0)
#define RXVM_INSTRUMENTATION_VALUE_TYPED(operation_, shape_, bytes_) ((void)0)

#endif
