#include <assert.h>
#include <string.h>
#include "rxvmprofile.h"

static void test_balanced_procedure_accounting(void) {
    rxvm_profile_state state;
    rxvm_profile_procedure procedures[3];
    rxvm_profile_activation activations[4];
    int root_frame;
    int child_frame;

    memset(&state, 0, sizeof(state));
    memset(procedures, 0, sizeof(procedures));
    memset(activations, 0, sizeof(activations));
    state.enabled = 1;
    state.procedures = procedures;
    state.procedure_count = 3;
    state.procedure_capacity = 3;
    state.activations = activations;
    state.activation_capacity = 4;

    rxvm_profile_frame_activate_at(&state, &root_frame, 0,
                                   RXVM_TRANSITION_EXTERNAL_ENTRY, 0);
    rxvm_profile_instruction_begin_at(&state, OP_CALL_FUNC, 10);
    rxvm_profile_frame_activate_at(&state, &child_frame, 1,
                                   RXVM_TRANSITION_CALL, 10);
    rxvm_profile_instruction_retire_at(&state, RXVM_TRANSITION_CALL, 20);

    rxvm_profile_instruction_begin_at(&state, OP_LOAD_REG_INT, 30);
    rxvm_profile_instruction_retire_at(&state, RXVM_TRANSITION_SEQUENTIAL, 40);
    rxvm_profile_instruction_begin_at(&state, OP_RET, 45);
    rxvm_profile_frame_activate_at(&state, &root_frame, 0,
                                   RXVM_TRANSITION_RETURN, 45);
    rxvm_profile_instruction_retire_at(&state, RXVM_TRANSITION_RETURN, 50);

    rxvm_profile_instruction_begin_at(&state, OP_CALL_REG_FUNC, 60);
    rxvm_profile_native_begin_at(&state, 2, 62);
    rxvm_profile_native_end_at(&state, 72);
    rxvm_profile_instruction_retire_at(&state, RXVM_TRANSITION_SEQUENTIAL, 82);
    rxvm_profile_instruction_begin_at(&state, OP_RET, 85);
    rxvm_profile_instruction_terminal_at(&state, 95);

    assert(procedures[0].calls == 1);
    assert(procedures[0].completed == 1);
    assert(procedures[0].entry_overhead.total_ns == 10);
    assert(procedures[0].inclusive_body.total_ns == 75);
    assert(procedures[0].self.total_ns == 15);
    assert(procedures[0].exit_overhead.total_ns == 10);
    assert(procedures[0].elapsed.total_ns == 95);

    assert(procedures[1].calls == 1);
    assert(procedures[1].completed == 1);
    assert(procedures[1].entry_overhead.total_ns == 20);
    assert(procedures[1].inclusive_body.total_ns == 15);
    assert(procedures[1].self.total_ns == 15);
    assert(procedures[1].exit_overhead.total_ns == 15);
    assert(procedures[1].elapsed.total_ns == 50);

    assert(procedures[2].calls == 1);
    assert(procedures[2].completed == 1);
    assert(procedures[2].native_total.total_ns == 10);
    assert(procedures[2].elapsed.total_ns == 10);
    assert(state.invalid_events == 0);
}

static void test_unwound_procedure_accounting(void) {
    rxvm_profile_state state;
    rxvm_profile_procedure procedures[2];
    rxvm_profile_activation activations[2];
    int root_frame;
    int child_frame;

    memset(&state, 0, sizeof(state));
    memset(procedures, 0, sizeof(procedures));
    memset(activations, 0, sizeof(activations));
    state.enabled = 1;
    state.procedures = procedures;
    state.procedure_count = 2;
    state.procedure_capacity = 2;
    state.activations = activations;
    state.activation_capacity = 2;

    rxvm_profile_frame_activate_at(&state, &root_frame, 0,
                                   RXVM_TRANSITION_EXTERNAL_ENTRY, 0);
    rxvm_profile_instruction_begin_at(&state, OP_CALL_FUNC, 10);
    rxvm_profile_frame_activate_at(&state, &child_frame, 1,
                                   RXVM_TRANSITION_CALL, 10);
    rxvm_profile_instruction_retire_at(&state, RXVM_TRANSITION_CALL, 20);
    rxvm_profile_instruction_begin_at(&state, OP_LOAD_REG_INT, 30);
    rxvm_profile_frame_unwind_at(&state, &child_frame, 40);
    rxvm_profile_instruction_terminal_at(&state, 40);

    assert(procedures[1].calls == 1);
    assert(procedures[1].completed == 0);
    assert(procedures[1].unwound == 1);
    assert(procedures[1].inclusive_body.total_ns == 10);
    assert(procedures[1].self.total_ns == 10);
    assert(procedures[1].elapsed.total_ns == 30);
    assert(procedures[0].unwound == 1);
    assert(state.invalid_events == 0);
}

int main(void) {
    rxvm_profile_state state;
    rxvm_profile_procedure procedures[2];
    rxvm_profile_activation activations[4];
    int root_frame;

    memset(&state, 0, sizeof(state));
    memset(procedures, 0, sizeof(procedures));
    memset(activations, 0, sizeof(activations));
    state.enabled = 1;
    state.current_transition = RXVM_TRANSITION_SEQUENTIAL;
    state.procedures = procedures;
    state.procedure_count = 2;
    state.procedure_capacity = 2;
    state.activations = activations;
    state.activation_capacity = 4;

    rxvm_profile_frame_activate_at(&state, &root_frame, 0,
                                   RXVM_TRANSITION_EXTERNAL_ENTRY, 5);
    rxvm_profile_instruction_begin_at(&state, OP_LOAD_REG_INT, 10);
    rxvm_profile_instruction_retire_at(&state, RXVM_TRANSITION_SEQUENTIAL, 30);
    rxvm_profile_interrupt_poll(&state);

    rxvm_profile_instruction_begin_at(&state, OP_COPY_REG_REG, 40);
    rxvm_profile_instruction_retire_at(&state, RXVM_TRANSITION_CALL, 70);
    rxvm_profile_interrupt_poll(&state);

    rxvm_profile_instruction_begin_at(&state, OP_RET, 90);
    rxvm_profile_instruction_terminal_at(&state, 100);

    rxvm_profile_instruction_begin_at(&state, OP_LOAD_REG_INT, 110);
    rxvm_profile_instruction_retire_at(&state, RXVM_TRANSITION_SEQUENTIAL, 120);
    rxvm_profile_interrupt_poll(&state);
    rxvm_profile_interrupt_scan_begin_at(&state, 121);
    rxvm_profile_interrupt_select_at(&state, RXSIGNAL_ERROR, 126);
    rxvm_profile_interrupt_entry(&state, RXSIGNAL_ERROR);
    rxvm_profile_instruction_begin_at(&state, OP_COPY_REG_REG, 140);
    rxvm_profile_instruction_retire_at(&state, RXVM_TRANSITION_INTERRUPT_RESUME, 150);
    rxvm_profile_interrupt_poll(&state);
    rxvm_profile_interrupt_resume_at(&state, RXSIGNAL_ERROR, 152);
    rxvm_profile_instruction_begin_at(&state, OP_RET, 160);
    rxvm_profile_instruction_terminal_at(&state, 170);

    rxvm_profile_instruction_begin_at(&state, OP_LOAD_REG_INT, 180);
    rxvm_profile_instruction_retire_at(&state, RXVM_TRANSITION_SEQUENTIAL, 190);
    rxvm_profile_interrupt_poll(&state);
    rxvm_profile_interrupt_scan_begin_at(&state, 191);
    rxvm_profile_interrupt_select_at(&state, RXSIGNAL_OTHER, 196);
    rxvm_profile_interrupt_entry(&state, RXSIGNAL_OTHER);
    rxvm_profile_interrupt_terminal_at(&state, RXSIGNAL_OTHER, 205);

    rxvm_profile_native_begin_at(&state, 1, 210);
    rxvm_profile_native_end_at(&state, 250);

    assert(state.instructions[OP_LOAD_REG_INT].count == 3);
    assert(state.instructions[OP_LOAD_REG_INT].total_ns == 40);
    assert(state.instructions[OP_COPY_REG_REG].count == 2);
    assert(state.instructions[OP_COPY_REG_REG].total_ns == 40);
    assert(state.instructions[OP_RET].count == 2);
    assert(state.instructions[OP_RET].total_ns == 20);

    assert(state.transitions[RXVM_TRANSITION_EXTERNAL_ENTRY].count == 1);
    assert(state.transitions[RXVM_TRANSITION_EXTERNAL_ENTRY].total_ns == 5);
    assert(state.transitions[RXVM_TRANSITION_SEQUENTIAL].count == 1);
    assert(state.transitions[RXVM_TRANSITION_SEQUENTIAL].total_ns == 10);
    assert(state.transitions[RXVM_TRANSITION_CALL].count == 1);
    assert(state.transitions[RXVM_TRANSITION_CALL].total_ns == 20);
    assert(state.transitions[RXVM_TRANSITION_INTERRUPT_ENTRY].count == 2);
    assert(state.transitions[RXVM_TRANSITION_INTERRUPT_ENTRY].total_ns == 35);
    assert(state.transitions[RXVM_TRANSITION_INTERRUPT_RESUME].count == 1);
    assert(state.transitions[RXVM_TRANSITION_INTERRUPT_RESUME].total_ns == 10);
    assert(state.transitions[RXVM_TRANSITION_TERMINAL].count == 2);

    assert(state.interrupt_polls == 5);
    assert(state.interrupt_scans.count == 2);
    assert(state.interrupt_scans.total_ns == 10);
    assert(state.interrupts[RXSIGNAL_ERROR].selected == 1);
    assert(state.interrupts[RXSIGNAL_ERROR].entries == 1);
    assert(state.interrupts[RXSIGNAL_ERROR].resumes == 1);
    assert(state.interrupts[RXSIGNAL_ERROR].mechanics.count == 1);
    assert(state.interrupts[RXSIGNAL_ERROR].mechanics.total_ns == 14);
    assert(state.interrupts[RXSIGNAL_OTHER].selected == 1);
    assert(state.interrupts[RXSIGNAL_OTHER].entries == 1);
    assert(state.interrupts[RXSIGNAL_OTHER].terminals == 1);
    assert(state.interrupts[RXSIGNAL_OTHER].mechanics.count == 1);
    assert(state.interrupts[RXSIGNAL_OTHER].mechanics.total_ns == 9);
    assert(procedures[0].calls == 1);
    assert(procedures[0].completed == 1);
    assert(procedures[0].unwound == 0);
    assert(procedures[0].entry_overhead.total_ns == 5);
    assert(procedures[0].inclusive_body.total_ns == 80);
    assert(procedures[0].self.total_ns == 30);
    assert(procedures[0].exit_overhead.total_ns == 10);
    assert(procedures[0].elapsed.total_ns == 95);
    assert(procedures[1].calls == 1);
    assert(procedures[1].completed == 1);
    assert(procedures[1].native_total.count == 1);
    assert(procedures[1].native_total.total_ns == 40);
    assert(procedures[1].elapsed.total_ns == 40);
    assert(state.invalid_events == 0);
    assert(!state.overflowed);

    test_balanced_procedure_accounting();
    test_unwound_procedure_accounting();

    return 0;
}
