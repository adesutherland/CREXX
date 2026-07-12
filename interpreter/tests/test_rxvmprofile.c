#include <assert.h>
#include <string.h>
#include "rxvmprofile.h"

int main(void) {
    rxvm_profile_state state;

    memset(&state, 0, sizeof(state));
    state.enabled = 1;
    state.current_transition = RXVM_TRANSITION_SEQUENTIAL;

    rxvm_profile_frame_activate_at(&state, RXVM_TRANSITION_EXTERNAL_ENTRY, 5);
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
    assert(state.invalid_events == 0);
    assert(!state.overflowed);

    return 0;
}
