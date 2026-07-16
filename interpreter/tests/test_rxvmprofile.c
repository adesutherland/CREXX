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
    assert(procedures[0].native_child.total_ns == 10);
    assert(procedures[0].exit_overhead.total_ns == 10);
    assert(procedures[0].elapsed.total_ns == 95);

    assert(procedures[1].calls == 1);
    assert(procedures[1].completed == 1);
    assert(procedures[1].entry_overhead.total_ns == 20);
    assert(procedures[1].inclusive_body.total_ns == 15);
    assert(procedures[1].self.total_ns == 15);
    assert(procedures[1].native_child.total_ns == 0);
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

static void test_allocation_and_frame_accounting(void) {
    rxvm_profile_state state;

    memset(&state, 0, sizeof(state));
    state.enabled = 1;

    rxvm_profile_add_allocation_at(
            &state, RXVM_PROFILE_ALLOC_VALUE, 2 * sizeof(value), 2);
    rxvm_profile_add_allocation_at(
            &state, RXVM_PROFILE_ALLOC_STRING_BUFFER, 64, 0);
    rxvm_profile_frame_activation_at(&state, 0, 512, 3);
    rxvm_profile_frame_activation_at(&state, 1, 0, 0);

    assert(state.allocations[RXVM_PROFILE_ALLOC_VALUE].count == 1);
    assert(state.allocations[RXVM_PROFILE_ALLOC_VALUE].bytes ==
           2 * sizeof(value));
    assert(state.allocations[RXVM_PROFILE_ALLOC_STRING_BUFFER].max_bytes == 64);
    assert(state.allocations[RXVM_PROFILE_ALLOC_FRAME_BLOCK].count == 1);
    assert(state.allocations[RXVM_PROFILE_ALLOC_FRAME_BLOCK].bytes == 512);
    assert(state.value_slots == 5);
    assert(state.value_slot_bytes == 5 * sizeof(value));
    assert(state.max_value_slots_per_block == 3);
    assert(state.frame_activations == 2);
    assert(state.frame_reuses == 1);
    assert(state.frame_high_water == 2);
    assert(state.active_frames == 2);

    rxvm_profile_frame_release_at(&state);
    rxvm_profile_frame_release_at(&state);
    assert(state.active_frames == 0);
    assert(state.invalid_events == 0);

    state.allocations[RXVM_PROFILE_ALLOC_BINARY_BUFFER].count = UINT64_MAX;
    rxvm_profile_add_allocation_at(
            &state, RXVM_PROFILE_ALLOC_BINARY_BUFFER, 32, 0);
    assert(state.overflowed);
}

static void test_call_census_accounting(void) {
    rxvm_profile_state state;
    rxvm_profile_procedure procedures[3];
    size_t i;

    memset(&state, 0, sizeof(state));
    memset(procedures, 0, sizeof(procedures));
    state.enabled = 1;
    state.procedures = procedures;
    state.procedure_count = 3;
    state.procedure_capacity = 3;
    procedures[0].kind = RXVM_PROFILE_PROCEDURE;
    procedures[1].kind = RXVM_PROFILE_METHOD;
    procedures[2].kind = RXVM_PROFILE_FACTORY;

    rxvm_profile_record_call_at(
            &state, RXVM_PROFILE_CALL_DIRECT_BYTECODE, 0, 0,
            RXVM_PROFILE_FRAME_FRESH, RXVM_PROFILE_CALL_SUCCESS,
            0, 1, 10, 0, 0);
    rxvm_profile_record_call_at(
            &state, RXVM_PROFILE_CALL_DIRECT_BYTECODE, 0, 0,
            RXVM_PROFILE_FRAME_FRESH, RXVM_PROFILE_CALL_SUCCESS,
            0, 1, 10, 0, 0);
    assert(state.call_row_count == 1);
    assert(state.call_rows[0].count == 2);

    rxvm_profile_record_call_at(
            &state, RXVM_PROFILE_CALL_DYNAMIC_BYTECODE, 1, 2,
            RXVM_PROFILE_FRAME_REUSED, RXVM_PROFILE_CALL_SUCCESS,
            0, 1, 20, 0, 0);
    rxvm_profile_record_call_at(
            &state, RXVM_PROFILE_CALL_DYNAMIC_NATIVE, 0, 1,
            RXVM_PROFILE_FRAME_NO_CHILD_NATIVE, RXVM_PROFILE_CALL_SUCCESS,
            0, 1, 30, 0, 0);
    rxvm_profile_record_call_at(
            &state, RXVM_PROFILE_CALL_EXTERNAL_ROOT, 0, 1,
            RXVM_PROFILE_FRAME_FRESH, RXVM_PROFILE_CALL_SUCCESS,
            0, 1, 0, 0, 0);
    rxvm_profile_record_call_at(
            &state, RXVM_PROFILE_CALL_SIGNAL_BYTECODE, 2, 1,
            RXVM_PROFILE_FRAME_FRESH, RXVM_PROFILE_CALL_SUCCESS,
            0, 1, 40, 0, 0);
    rxvm_profile_record_call_at(
            &state, RXVM_PROFILE_CALL_SIGNAL_NATIVE, 0, 1,
            RXVM_PROFILE_FRAME_NO_CHILD_NATIVE, RXVM_PROFILE_CALL_SUCCESS,
            0, 1, 50, 0, 0);
    rxvm_profile_record_call_at(
            &state, RXVM_PROFILE_CALL_DIRECT_NATIVE, SIZE_MAX, 0,
            RXVM_PROFILE_FRAME_NONE_FAILED, RXVM_PROFILE_CALL_UNRESOLVED,
            0, 1, 60, 0, 0);
    rxvm_profile_record_call_at(
            &state, RXVM_PROFILE_CALL_DIRECT_BYTECODE, 0, 1,
            RXVM_PROFILE_FRAME_NONE_FAILED, RXVM_PROFILE_CALL_FRAME_FAILED,
            0, 1, 70, 0, 0);
    rxvm_profile_record_call_at(
            &state, RXVM_PROFILE_CALL_DIRECT_BYTECODE, 0, -1,
            RXVM_PROFILE_FRAME_NONE_FAILED, RXVM_PROFILE_CALL_SUCCESS,
            0, 1, 80, 0, 0);
    rxvm_profile_record_call_at(
            &state, RXVM_PROFILE_CALL_DIRECT_BYTECODE, SIZE_MAX, 0,
            RXVM_PROFILE_FRAME_NONE_FAILED, RXVM_PROFILE_CALL_UNRESOLVED,
            0, 1, 90, 0, 0);

    assert(state.call_path_totals[RXVM_PROFILE_CALL_DIRECT_BYTECODE] == 5);
    for (i = 0; i < RXVM_PROFILE_CALL_PATH_COUNT; i++)
        assert(state.call_path_totals[i] > 0);
    assert(state.callable_kind_totals[RXVM_PROFILE_CENSUS_PROCEDURE] == 5);
    assert(state.callable_kind_totals[RXVM_PROFILE_CENSUS_METHOD] == 1);
    assert(state.callable_kind_totals[RXVM_PROFILE_CENSUS_FACTORY] == 1);
    assert(state.callable_kind_totals[RXVM_PROFILE_CENSUS_NATIVE] == 3);
    assert(state.callable_kind_totals[RXVM_PROFILE_CENSUS_UNKNOWN] == 1);
    assert(state.frame_disposition_totals[RXVM_PROFILE_FRAME_FRESH] == 4);
    assert(state.frame_disposition_totals[RXVM_PROFILE_FRAME_REUSED] == 1);
    assert(state.frame_disposition_totals[
                   RXVM_PROFILE_FRAME_NO_CHILD_NATIVE] == 2);
    assert(state.frame_disposition_totals[RXVM_PROFILE_FRAME_NONE_FAILED] == 4);
    assert(state.call_outcome_totals[RXVM_PROFILE_CALL_SUCCESS] == 7);
    assert(state.call_outcome_totals[RXVM_PROFILE_CALL_UNRESOLVED] == 2);
    assert(state.call_outcome_totals[RXVM_PROFILE_CALL_FRAME_FAILED] == 1);
    assert(state.call_outcome_totals[RXVM_PROFILE_CALL_INVALID] == 1);
    assert(state.census_tracking_unavailable);

    for (i = 0; i < RXVM_PROFILE_RETURN_PLACEMENT_COUNT; i++)
        rxvm_profile_record_return_at(
                &state, (rxvm_profile_return_placement)i);
    for (i = 0; i < RXVM_PROFILE_RETURN_PLACEMENT_COUNT; i++)
        assert(state.return_placements[i] == 1);

    for (i = 0; i < RXVM_PROFILE_DYNAMIC_KIND_COUNT; i++) {
        size_t outcome;
        for (outcome = 0; outcome < RXVM_PROFILE_DYNAMIC_OUTCOME_COUNT;
             outcome++)
            rxvm_profile_record_dynamic_at(
                    &state, (rxvm_profile_dynamic_kind)i,
                    (rxvm_profile_dynamic_outcome)outcome);
    }
    for (i = 0; i < RXVM_PROFILE_DYNAMIC_KIND_COUNT; i++) {
        size_t outcome;
        for (outcome = 0; outcome < RXVM_PROFILE_DYNAMIC_OUTCOME_COUNT;
             outcome++)
            assert(state.dynamic_resolution[i][outcome] == 1);
    }

    rxvm_profile_record_signal_unwind_at(&state, 2, 1, 3, 0);
    rxvm_profile_record_signal_native_restore_at(&state, 1, 2, 0);
    assert(state.signal_unwind_events == 1);
    assert(state.signal_bytecode_frames_discarded == 2);
    assert(state.signal_argument_windows_restored == 1);
    assert(state.signal_argument_slots_restored == 3);
    assert(state.signal_native_windows_restored == 1);
    assert(state.signal_native_slots_restored == 2);
    assert(state.signal_restoration_failures == 0);
    free(state.call_rows);
}

static void test_call_census_overflow(void) {
    rxvm_profile_state state;
    rxvm_profile_procedure procedure;

    memset(&state, 0, sizeof(state));
    memset(&procedure, 0, sizeof(procedure));
    state.enabled = 1;
    state.procedures = &procedure;
    state.procedure_count = 1;
    state.procedure_capacity = 1;
    state.call_path_totals[RXVM_PROFILE_CALL_DIRECT_BYTECODE] = UINT64_MAX;
    rxvm_profile_record_call_at(
            &state, RXVM_PROFILE_CALL_DIRECT_BYTECODE, 0, 0,
            RXVM_PROFILE_FRAME_FRESH, RXVM_PROFILE_CALL_SUCCESS,
            0, 1, 1, 0, 0);
    assert(state.overflowed);
    assert(state.call_path_totals[RXVM_PROFILE_CALL_DIRECT_BYTECODE] ==
           UINT64_MAX);
    free(state.call_rows);
}

static void test_restoration_sequence_accounting(void) {
    rxvm_profile_state state;
    rxvm_profile_activation activation;
    rxvm_profile_call_row row;
    rxvm_profile_trace_record trace[3];
    size_t mapping[4] = {0, 3, 1, 2};
    size_t trace_indices[3];
    int frame;

    memset(&state, 0, sizeof(state));
    memset(&activation, 0, sizeof(activation));
    memset(&row, 0, sizeof(row));
    memset(trace, 0, sizeof(trace));
    state.enabled = 1;
    state.activations = &activation;
    state.activation_count = 1;
    state.activation_capacity = 1;
    state.call_rows = &row;
    state.call_row_count = 1;
    state.call_row_capacity = 1;
    activation.frame = &frame;
    activation.trace = trace;
    activation.trace_capacity = 3;
    activation.trace_count = 1;
    activation.restoration_mapping = mapping;
    activation.restoration_mapping_capacity = 4;
    activation.restoration_mapping_count = 4;
    activation.restoration_trace_indices = trace_indices;
    activation.restoration_trace_capacity = 3;
    activation.restoration_call_row = 0;
    activation.restoration_pending = 1;
    activation.restoration_ready = 1;
    trace[0].opcode = OP_SWAP_REG_REG;
    trace[1].opcode = OP_SWAP_REG_REG;
    trace[2].opcode = OP_SWAP_REG_REG;

    rxvm_profile_record_swap_at(&state, &frame, 1, 2);
    assert(state.normal_restoration_swaps == 0);
    assert(row.normal_restoration_swaps == 0);
    assert(activation.restoration_pending);

    activation.trace_count = 2;
    rxvm_profile_record_swap_at(&state, &frame, 2, 3);
    assert(state.normal_restoration_swaps == 2);
    assert(row.normal_restoration_swaps == 2);
    assert(!activation.restoration_pending);
    assert((trace[0].attribution & RXVM_PROFILE_ATTR_RESTORE_SWAP) != 0);
    assert((trace[1].attribution & RXVM_PROFILE_ATTR_RESTORE_SWAP) != 0);

    activation.trace_count = 3;
    rxvm_profile_record_swap_at(&state, &frame, 3, 1);
    assert(state.normal_restoration_swaps == 2);
    assert(row.normal_restoration_swaps == 2);
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
    assert(procedures[0].native_child.total_ns == 0);
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
    test_allocation_and_frame_accounting();
    test_call_census_accounting();
    test_call_census_overflow();
    test_restoration_sequence_accounting();

    return 0;
}
