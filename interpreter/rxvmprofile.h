/* Compile-time VM timing profiler state and event accounting. */

#ifndef CREXX_RXVMPROFILE_H
#define CREXX_RXVMPROFILE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "rxbin.h"
#include "rxdefs.h"
#include "rxsignal.h"
#include "rxvalue.h"
#include "rxvminstrument.h"

#if defined(_MSC_VER)
#define RXVM_PROFILE_INLINE static __inline
#else
#define RXVM_PROFILE_INLINE static inline
#endif

struct rxvm_context;

typedef struct rxvm_profile_counter {
    uint64_t count;
    uint64_t total_ns;
    uint64_t min_ns;
    uint64_t max_ns;
} rxvm_profile_counter;

typedef struct rxvm_profile_interrupt_counter {
    uint64_t selected;
    uint64_t entries;
    uint64_t resumes;
    uint64_t terminals;
    rxvm_profile_counter mechanics;
} rxvm_profile_interrupt_counter;

typedef enum rxvm_profile_callable_kind {
    RXVM_PROFILE_PROCEDURE = 0,
    RXVM_PROFILE_METHOD,
    RXVM_PROFILE_FACTORY
} rxvm_profile_callable_kind;

typedef struct rxvm_profile_procedure {
    char *module_name;
    char *name;
    char *return_type;
    char *args;
    int native;
    rxvm_profile_callable_kind kind;
    uint64_t calls;
    uint64_t completed;
    uint64_t unwound;
    rxvm_profile_counter elapsed;
    rxvm_profile_counter inclusive_body;
    rxvm_profile_counter self;
    rxvm_profile_counter native_child;
    rxvm_profile_counter entry_overhead;
    rxvm_profile_counter exit_overhead;
    rxvm_profile_counter native_total;
} rxvm_profile_procedure;

typedef enum rxvm_profile_call_path {
    RXVM_PROFILE_CALL_DIRECT_BYTECODE = 0,
    RXVM_PROFILE_CALL_DIRECT_NATIVE,
    RXVM_PROFILE_CALL_DYNAMIC_BYTECODE,
    RXVM_PROFILE_CALL_DYNAMIC_NATIVE,
    RXVM_PROFILE_CALL_EXTERNAL_ROOT,
    RXVM_PROFILE_CALL_SIGNAL_BYTECODE,
    RXVM_PROFILE_CALL_SIGNAL_NATIVE,
    RXVM_PROFILE_CALL_PATH_COUNT
} rxvm_profile_call_path;

typedef enum rxvm_profile_census_callable_kind {
    RXVM_PROFILE_CENSUS_PROCEDURE = 0,
    RXVM_PROFILE_CENSUS_METHOD,
    RXVM_PROFILE_CENSUS_FACTORY,
    RXVM_PROFILE_CENSUS_NATIVE,
    RXVM_PROFILE_CENSUS_UNKNOWN,
    RXVM_PROFILE_CENSUS_CALLABLE_KIND_COUNT
} rxvm_profile_census_callable_kind;

typedef enum rxvm_profile_frame_disposition {
    RXVM_PROFILE_FRAME_FRESH = 0,
    RXVM_PROFILE_FRAME_REUSED,
    RXVM_PROFILE_FRAME_NO_CHILD_NATIVE,
    RXVM_PROFILE_FRAME_NONE_FAILED,
    RXVM_PROFILE_FRAME_DISPOSITION_COUNT,
    RXVM_PROFILE_FRAME_LAST_ACTIVATION = -1
} rxvm_profile_frame_disposition;

typedef enum rxvm_profile_call_outcome {
    RXVM_PROFILE_CALL_SUCCESS = 0,
    RXVM_PROFILE_CALL_UNRESOLVED,
    RXVM_PROFILE_CALL_FRAME_FAILED,
    RXVM_PROFILE_CALL_INVALID,
    RXVM_PROFILE_CALL_OUTCOME_COUNT
} rxvm_profile_call_outcome;

typedef enum rxvm_profile_return_placement {
    RXVM_PROFILE_RETURN_VOID = 0,
    RXVM_PROFILE_RETURN_MOVE_LOCAL,
    RXVM_PROFILE_RETURN_COPY_NONLOCAL,
    RXVM_PROFILE_RETURN_VALUE_IGNORED,
    RXVM_PROFILE_RETURN_IMMEDIATE,
    RXVM_PROFILE_RETURN_TERMINAL_EXTERNAL,
    RXVM_PROFILE_RETURN_PLACEMENT_COUNT
} rxvm_profile_return_placement;

typedef enum rxvm_profile_dynamic_kind {
    RXVM_PROFILE_DYNAMIC_METHOD_SELECTION = 0,
    RXVM_PROFILE_DYNAMIC_FACTORY_SELECTION,
    RXVM_PROFILE_DYNAMIC_KIND_COUNT
} rxvm_profile_dynamic_kind;

typedef enum rxvm_profile_dynamic_outcome {
    RXVM_PROFILE_DYNAMIC_ATTEMPT = 0,
    RXVM_PROFILE_DYNAMIC_SUCCESS,
    RXVM_PROFILE_DYNAMIC_FAILURE,
    RXVM_PROFILE_DYNAMIC_OUTCOME_COUNT
} rxvm_profile_dynamic_outcome;

typedef struct rxvm_profile_trace_record {
    int opcode;
    const bin_code *pc;
    unsigned char attribution;
} rxvm_profile_trace_record;

typedef struct rxvm_profile_call_row {
    rxvm_profile_call_path path;
    rxvm_profile_census_callable_kind callable_kind;
    rxvm_profile_frame_disposition frame_disposition;
    rxvm_profile_call_outcome outcome;
    size_t procedure_id;
    uint64_t arity;
    int arity_valid;
    size_t site_module;
    size_t site_index;
    uint64_t count;
    uint64_t setup_swaps;
    uint64_t normal_restoration_swaps;
    uint64_t defensive_argument_copies;
} rxvm_profile_call_row;

typedef struct rxvm_profile_activation {
    const void *frame;
    size_t procedure_id;
    uint64_t call_start_ns;
    uint64_t body_start_ns;
    uint64_t self_ns;
    uint64_t native_child_ns;
    int body_started;
    rxvm_profile_trace_record *trace;
    size_t trace_count;
    size_t trace_capacity;
    size_t *restoration_mapping;
    size_t restoration_mapping_capacity;
    size_t restoration_mapping_count;
    size_t *restoration_trace_indices;
    size_t restoration_trace_capacity;
    size_t restoration_trace_count;
    size_t restoration_call_row;
    int restoration_pending;
    int restoration_ready;
} rxvm_profile_activation;

typedef struct rxvm_profile_pending_exit {
    int active;
    size_t procedure_id;
    uint64_t call_start_ns;
    uint64_t exit_start_ns;
} rxvm_profile_pending_exit;

typedef enum rxvm_profile_allocation_kind {
    RXVM_PROFILE_ALLOC_FRAME_BLOCK = 0,
    RXVM_PROFILE_ALLOC_VALUE,
    RXVM_PROFILE_ALLOC_ATTRIBUTE_VALUES,
    RXVM_PROFILE_ALLOC_ATTRIBUTE_POINTERS,
    RXVM_PROFILE_ALLOC_STRING_BUFFER,
    RXVM_PROFILE_ALLOC_BINARY_BUFFER,
    RXVM_PROFILE_ALLOC_COUNT
} rxvm_profile_allocation_kind;

typedef struct rxvm_profile_allocation_counter {
    uint64_t count;
    uint64_t bytes;
    uint64_t max_bytes;
} rxvm_profile_allocation_counter;

typedef struct rxvm_profile_state {
    int enabled;
    int overflowed;
    int procedure_tracking_unavailable;
    int census_tracking_unavailable;
    uint64_t invalid_events;
    uint64_t timer_read_min_ns;
    uint64_t timer_zero_deltas;

    int instruction_active;
    int active_opcode;
    uint64_t instruction_start_ns;
    size_t instruction_activation_index;
    uint64_t native_elapsed_in_instruction;
    rxvm_transition_reason current_transition;

    int transition_pending;
    rxvm_transition_reason pending_transition;
    uint64_t transition_start_ns;

    uint64_t interrupt_polls;
    int interrupt_scan_active;
    uint64_t interrupt_scan_start_ns;
    uint64_t interrupt_scans_without_selection;

    int interrupt_mechanics_active;
    unsigned char interrupt_mechanics_signal;
    uint64_t interrupt_mechanics_start_ns;

    rxvm_profile_procedure *procedures;
    size_t procedure_count;
    size_t procedure_capacity;
    rxvm_profile_activation *activations;
    size_t activation_count;
    size_t activation_capacity;
    rxvm_profile_pending_exit pending_exit;

    int native_active;
    size_t native_procedure_id;
    uint64_t native_start_ns;

    struct rxvm_profile_state *previous_allocation_profile;
    rxvm_profile_allocation_counter allocations[RXVM_PROFILE_ALLOC_COUNT];
    uint64_t value_slots;
    uint64_t value_slot_bytes;
    uint64_t max_value_slots_per_block;
    uint64_t frame_activations;
    uint64_t frame_reuses;
    uint64_t active_frames;
    uint64_t frame_high_water;
    int last_frame_disposition_valid;
    rxvm_profile_frame_disposition last_frame_disposition;

    struct rxvm_context *context;
    rxvm_profile_call_row *call_rows;
    size_t call_row_count;
    size_t call_row_capacity;
    uint64_t call_path_totals[RXVM_PROFILE_CALL_PATH_COUNT];
    uint64_t callable_kind_totals[RXVM_PROFILE_CENSUS_CALLABLE_KIND_COUNT];
    uint64_t frame_disposition_totals[RXVM_PROFILE_FRAME_DISPOSITION_COUNT];
    uint64_t call_outcome_totals[RXVM_PROFILE_CALL_OUTCOME_COUNT];
    uint64_t return_placements[RXVM_PROFILE_RETURN_PLACEMENT_COUNT];
    uint64_t dynamic_resolution[RXVM_PROFILE_DYNAMIC_KIND_COUNT]
                               [RXVM_PROFILE_DYNAMIC_OUTCOME_COUNT];
    uint64_t setup_swaps;
    uint64_t normal_restoration_swaps;
    uint64_t defensive_argument_copies;
    uint64_t attribution_degraded;
    uint64_t signal_unwind_events;
    uint64_t signal_bytecode_frames_discarded;
    uint64_t signal_argument_windows_restored;
    uint64_t signal_argument_slots_restored;
    uint64_t signal_native_windows_restored;
    uint64_t signal_native_slots_restored;
    uint64_t signal_restoration_failures;

    rxvm_profile_counter instructions[OP_MAX_INSTRUCTIONS];
    rxvm_profile_counter transitions[RXVM_TRANSITION_COUNT];
    rxvm_profile_counter interrupt_scans;
    rxvm_profile_counter interrupt_mechanics;
    rxvm_profile_interrupt_counter interrupts[RXSIGNAL_MAX];
} rxvm_profile_state;

typedef const char *(*rxvm_profile_signal_name_fn)(unsigned char signal);

uint64_t rxvm_profile_now_ns(void);
void rxvm_profile_begin(rxvm_profile_state *state, int enabled,
                        struct rxvm_context *context);
void rxvm_profile_refresh_catalog(rxvm_profile_state *state,
                                  struct rxvm_context *context);
void rxvm_profile_destroy(rxvm_profile_state *state);
void rxvm_profile_report(const rxvm_profile_state *state,
                         const char *output_path,
                         const char *vm_mode,
                         int result,
                         const Instruction *instruction_map,
                         rxvm_profile_signal_name_fn signal_name);
void rxvm_profile_record_allocation(rxvm_profile_allocation_kind kind,
                                    size_t bytes, size_t value_slots);
void rxvm_profile_record_frame_activation(int reused, size_t frame_bytes,
                                          size_t value_slots);
void rxvm_profile_record_frame_release(void);
void rxvm_profile_trace_instruction_at(rxvm_profile_state *state,
                                       const void *frame,
                                       const bin_code *pc,
                                       const Instruction *instruction_map,
                                       size_t module_id,
                                       size_t instruction_index,
                                       int opcode);
void rxvm_profile_record_call_at(rxvm_profile_state *state,
                                 rxvm_profile_call_path path,
                                 size_t procedure_id,
                                 int64_t arity,
                                 rxvm_profile_frame_disposition disposition,
                                 rxvm_profile_call_outcome outcome,
                                 const void *caller_frame,
                                 size_t site_module,
                                 size_t site_index,
                                 size_t argument_base,
                                 int has_argument_window);
void rxvm_profile_record_return_at(rxvm_profile_state *state,
                                   rxvm_profile_return_placement placement);
void rxvm_profile_record_dynamic_at(rxvm_profile_state *state,
                                    rxvm_profile_dynamic_kind kind,
                                    rxvm_profile_dynamic_outcome outcome);
void rxvm_profile_record_swap_at(rxvm_profile_state *state,
                                 const void *frame,
                                 size_t register_1,
                                 size_t register_2);
void rxvm_profile_record_signal_unwind_at(rxvm_profile_state *state,
                                          uint64_t frames_discarded,
                                          uint64_t windows_restored,
                                          uint64_t slots_restored,
                                          int restoration_failed);
void rxvm_profile_record_signal_native_restore_at(rxvm_profile_state *state,
                                                  int window_observed,
                                                  uint64_t slots_restored,
                                                  int restoration_failed);

RXVM_PROFILE_INLINE uint64_t rxvm_profile_elapsed(uint64_t start_ns,
                                                  uint64_t end_ns) {
    return end_ns >= start_ns ? end_ns - start_ns : 0;
}

RXVM_PROFILE_INLINE void rxvm_profile_increment(rxvm_profile_state *state,
                                                uint64_t *value) {
    if (*value == UINT64_MAX) {
        state->overflowed = 1;
    } else {
        (*value)++;
    }
}

RXVM_PROFILE_INLINE void rxvm_profile_add_total(rxvm_profile_state *state,
                                                uint64_t *value,
                                                uint64_t amount) {
    if (UINT64_MAX - *value < amount) {
        *value = UINT64_MAX;
        state->overflowed = 1;
    } else {
        *value += amount;
    }
}

RXVM_PROFILE_INLINE void rxvm_profile_add_counter(rxvm_profile_state *state,
                                                  rxvm_profile_counter *counter,
                                                  uint64_t elapsed_ns) {
    int first = counter->count == 0;
    rxvm_profile_increment(state, &counter->count);
    rxvm_profile_add_total(state, &counter->total_ns, elapsed_ns);
    if (first || elapsed_ns < counter->min_ns) counter->min_ns = elapsed_ns;
    if (first || elapsed_ns > counter->max_ns) counter->max_ns = elapsed_ns;
}

RXVM_PROFILE_INLINE int rxvm_profile_valid_allocation_kind(
        rxvm_profile_allocation_kind kind) {
    return kind >= RXVM_PROFILE_ALLOC_FRAME_BLOCK &&
           kind < RXVM_PROFILE_ALLOC_COUNT;
}

RXVM_PROFILE_INLINE void rxvm_profile_add_allocation_at(
        rxvm_profile_state *state, rxvm_profile_allocation_kind kind,
        uint64_t bytes, uint64_t value_slots) {
    rxvm_profile_allocation_counter *counter;
    uint64_t value_bytes;
    if (!state || !state->enabled ||
            !rxvm_profile_valid_allocation_kind(kind)) return;
    counter = &state->allocations[kind];
    rxvm_profile_increment(state, &counter->count);
    rxvm_profile_add_total(state, &counter->bytes, bytes);
    if (bytes > counter->max_bytes) counter->max_bytes = bytes;
    if (!value_slots) return;
    if (value_slots > state->max_value_slots_per_block)
        state->max_value_slots_per_block = value_slots;
    rxvm_profile_add_total(state, &state->value_slots, value_slots);
    if (value_slots > UINT64_MAX / sizeof(value)) {
        value_bytes = UINT64_MAX;
        state->overflowed = 1;
    } else {
        value_bytes = value_slots * sizeof(value);
    }
    rxvm_profile_add_total(state, &state->value_slot_bytes, value_bytes);
}

RXVM_PROFILE_INLINE void rxvm_profile_frame_activation_at(
        rxvm_profile_state *state, int reused, uint64_t frame_bytes,
        uint64_t value_slots) {
    if (!state || !state->enabled) return;
    rxvm_profile_increment(state, &state->frame_activations);
    rxvm_profile_increment(state, &state->active_frames);
    if (state->active_frames > state->frame_high_water)
        state->frame_high_water = state->active_frames;
    if (reused) {
        rxvm_profile_increment(state, &state->frame_reuses);
        state->last_frame_disposition = RXVM_PROFILE_FRAME_REUSED;
    } else {
        rxvm_profile_add_allocation_at(
                state, RXVM_PROFILE_ALLOC_FRAME_BLOCK, frame_bytes,
                value_slots);
        state->last_frame_disposition = RXVM_PROFILE_FRAME_FRESH;
    }
    state->last_frame_disposition_valid = 1;
}

RXVM_PROFILE_INLINE void rxvm_profile_frame_release_at(
        rxvm_profile_state *state) {
    if (!state || !state->enabled) return;
    if (!state->active_frames) {
        rxvm_profile_increment(state, &state->invalid_events);
        return;
    }
    state->active_frames--;
}

RXVM_PROFILE_INLINE int rxvm_profile_valid_transition(rxvm_transition_reason reason) {
    return reason >= RXVM_TRANSITION_SEQUENTIAL && reason < RXVM_TRANSITION_COUNT;
}

RXVM_PROFILE_INLINE int rxvm_profile_valid_signal(unsigned char signal) {
    return signal > RXSIGNAL_NONE && signal < RXSIGNAL_MAX;
}

RXVM_PROFILE_INLINE int rxvm_profile_valid_procedure(
        const rxvm_profile_state *state, size_t procedure_id) {
    return procedure_id < state->procedure_count;
}

RXVM_PROFILE_INLINE void rxvm_profile_add_activation_self(
        rxvm_profile_state *state, size_t activation_index, uint64_t elapsed_ns) {
    if (activation_index >= state->activation_count) return;
    rxvm_profile_add_total(state,
                           &state->activations[activation_index].self_ns,
                           elapsed_ns);
}

RXVM_PROFILE_INLINE void rxvm_profile_finish_pending_exit_at(
        rxvm_profile_state *state, uint64_t now_ns) {
    rxvm_profile_procedure *procedure;
    if (!state->pending_exit.active) return;
    if (!rxvm_profile_valid_procedure(state, state->pending_exit.procedure_id)) {
        rxvm_profile_increment(state, &state->invalid_events);
        state->pending_exit.active = 0;
        return;
    }
    procedure = &state->procedures[state->pending_exit.procedure_id];
    rxvm_profile_add_counter(
            state, &procedure->exit_overhead,
            rxvm_profile_elapsed(state->pending_exit.exit_start_ns, now_ns));
    rxvm_profile_add_counter(
            state, &procedure->elapsed,
            rxvm_profile_elapsed(state->pending_exit.call_start_ns, now_ns));
    state->pending_exit.active = 0;
}

RXVM_PROFILE_INLINE void rxvm_profile_finish_transition_at(rxvm_profile_state *state,
                                                           uint64_t now_ns) {
    uint64_t elapsed_ns;
    if (!state->transition_pending) return;
    if (!rxvm_profile_valid_transition(state->pending_transition)) {
        rxvm_profile_increment(state, &state->invalid_events);
        state->pending_transition = RXVM_TRANSITION_SEQUENTIAL;
    }
    elapsed_ns = rxvm_profile_elapsed(state->transition_start_ns, now_ns);
    rxvm_profile_add_counter(
            state, &state->transitions[state->pending_transition], elapsed_ns);
    if (state->pending_transition == RXVM_TRANSITION_SEQUENTIAL ||
            state->pending_transition == RXVM_TRANSITION_BRANCH) {
        if (state->activation_count)
            rxvm_profile_add_activation_self(state, state->activation_count - 1,
                                             elapsed_ns);
    }
    state->transition_pending = 0;
}

RXVM_PROFILE_INLINE void rxvm_profile_finish_scan_at(rxvm_profile_state *state,
                                                     uint64_t now_ns,
                                                     int selected) {
    if (!state->interrupt_scan_active) return;
    rxvm_profile_add_counter(
            state, &state->interrupt_scans,
            rxvm_profile_elapsed(state->interrupt_scan_start_ns, now_ns));
    if (!selected) rxvm_profile_increment(state, &state->interrupt_scans_without_selection);
    state->interrupt_scan_active = 0;
}

RXVM_PROFILE_INLINE void rxvm_profile_finish_mechanics_at(rxvm_profile_state *state,
                                                          uint64_t now_ns) {
    uint64_t elapsed_ns;
    unsigned char signal;
    if (!state->interrupt_mechanics_active) return;
    elapsed_ns = rxvm_profile_elapsed(state->interrupt_mechanics_start_ns, now_ns);
    signal = state->interrupt_mechanics_signal;
    rxvm_profile_add_counter(state, &state->interrupt_mechanics, elapsed_ns);
    if (rxvm_profile_valid_signal(signal)) {
        rxvm_profile_add_counter(state, &state->interrupts[signal].mechanics,
                                 elapsed_ns);
    }
    state->interrupt_mechanics_active = 0;
}

RXVM_PROFILE_INLINE int rxvm_profile_push_activation(
        rxvm_profile_state *state, const void *frame, size_t procedure_id,
        uint64_t call_start_ns) {
    rxvm_profile_activation *activation;
    if (!rxvm_profile_valid_procedure(state, procedure_id)) {
        rxvm_profile_increment(state, &state->invalid_events);
        return 0;
    }
    if (state->activation_count == state->activation_capacity) {
        size_t new_capacity = state->activation_capacity
                ? state->activation_capacity * 2 : 64;
        rxvm_profile_activation *new_activations =
                (rxvm_profile_activation *)realloc(
                        state->activations,
                        new_capacity * sizeof(rxvm_profile_activation));
        if (!new_activations) {
            state->procedure_tracking_unavailable = 1;
            return 0;
        }
        state->activations = new_activations;
        state->activation_capacity = new_capacity;
    }
    activation = &state->activations[state->activation_count++];
    activation->frame = frame;
    activation->procedure_id = procedure_id;
    activation->call_start_ns = call_start_ns;
    activation->body_start_ns = 0;
    activation->self_ns = 0;
    activation->native_child_ns = 0;
    activation->body_started = 0;
    activation->trace = 0;
    activation->trace_count = 0;
    activation->trace_capacity = 0;
    activation->restoration_mapping = 0;
    activation->restoration_mapping_capacity = 0;
    activation->restoration_mapping_count = 0;
    activation->restoration_trace_indices = 0;
    activation->restoration_trace_capacity = 0;
    activation->restoration_trace_count = 0;
    activation->restoration_call_row = SIZE_MAX;
    activation->restoration_pending = 0;
    activation->restoration_ready = 0;
    rxvm_profile_increment(state, &state->procedures[procedure_id].calls);
    return 1;
}

RXVM_PROFILE_INLINE void rxvm_profile_close_top_at(
        rxvm_profile_state *state, uint64_t body_end_ns, uint64_t elapsed_end_ns,
        int completed, int defer_exit) {
    rxvm_profile_activation activation;
    rxvm_profile_procedure *procedure;
    if (!state->activation_count) {
        rxvm_profile_increment(state, &state->invalid_events);
        return;
    }
    activation = state->activations[state->activation_count - 1];
    state->activation_count--;
    free(activation.trace);
    free(activation.restoration_mapping);
    free(activation.restoration_trace_indices);
    if (!rxvm_profile_valid_procedure(state, activation.procedure_id)) {
        rxvm_profile_increment(state, &state->invalid_events);
        return;
    }
    procedure = &state->procedures[activation.procedure_id];
    if (activation.body_started) {
        rxvm_profile_add_counter(
                state, &procedure->inclusive_body,
                rxvm_profile_elapsed(activation.body_start_ns, body_end_ns));
        rxvm_profile_add_counter(state, &procedure->self, activation.self_ns);
        rxvm_profile_add_counter(state, &procedure->native_child,
                                 activation.native_child_ns);
    }
    if (completed) rxvm_profile_increment(state, &procedure->completed);
    else rxvm_profile_increment(state, &procedure->unwound);

    if (defer_exit) {
        rxvm_profile_finish_pending_exit_at(state, elapsed_end_ns);
        state->pending_exit.active = 1;
        state->pending_exit.procedure_id = activation.procedure_id;
        state->pending_exit.call_start_ns = activation.call_start_ns;
        state->pending_exit.exit_start_ns = body_end_ns;
    } else {
        rxvm_profile_add_counter(
                state, &procedure->exit_overhead,
                rxvm_profile_elapsed(body_end_ns, elapsed_end_ns));
        rxvm_profile_add_counter(
                state, &procedure->elapsed,
                rxvm_profile_elapsed(activation.call_start_ns, elapsed_end_ns));
    }
}

RXVM_PROFILE_INLINE void rxvm_profile_instruction_begin_at(rxvm_profile_state *state,
                                                           int opcode,
                                                           uint64_t now_ns) {
    rxvm_profile_activation *activation;
    if (!state->enabled) return;
    rxvm_profile_finish_mechanics_at(state, now_ns);
    rxvm_profile_finish_transition_at(state, now_ns);
    rxvm_profile_finish_pending_exit_at(state, now_ns);
    if (state->instruction_active)
        rxvm_profile_increment(state, &state->invalid_events);
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) {
        rxvm_profile_increment(state, &state->invalid_events);
        opcode = OP_IUNKNOWN;
    }
    state->instruction_active = 1;
    state->active_opcode = opcode;
    state->instruction_start_ns = now_ns;
    state->native_elapsed_in_instruction = 0;
    state->instruction_activation_index = SIZE_MAX;
    if (state->activation_count) {
        state->instruction_activation_index = state->activation_count - 1;
        activation = &state->activations[state->instruction_activation_index];
        if (!activation->body_started) {
            activation->body_started = 1;
            activation->body_start_ns = now_ns;
            rxvm_profile_add_counter(
                    state,
                    &state->procedures[activation->procedure_id].entry_overhead,
                    rxvm_profile_elapsed(activation->call_start_ns, now_ns));
        }
    }
    state->current_transition = RXVM_TRANSITION_SEQUENTIAL;
}

RXVM_PROFILE_INLINE void rxvm_profile_instruction_retire_at(
        rxvm_profile_state *state, rxvm_transition_reason reason,
        uint64_t now_ns) {
    uint64_t elapsed_ns;
    if (!state->enabled || !state->instruction_active) return;
    elapsed_ns = rxvm_profile_elapsed(state->instruction_start_ns, now_ns);
    rxvm_profile_add_counter(state, &state->instructions[state->active_opcode],
                             elapsed_ns);
    if (reason != RXVM_TRANSITION_CALL && reason != RXVM_TRANSITION_RETURN &&
            reason != RXVM_TRANSITION_INTERRUPT_ENTRY &&
            reason != RXVM_TRANSITION_INTERRUPT_RESUME &&
            state->instruction_activation_index < state->activation_count) {
        uint64_t self_ns = elapsed_ns >= state->native_elapsed_in_instruction
                ? elapsed_ns - state->native_elapsed_in_instruction : 0;
        rxvm_profile_add_activation_self(state,
                                         state->instruction_activation_index,
                                         self_ns);
    }
    state->instruction_active = 0;
    state->transition_pending = 1;
    state->pending_transition = rxvm_profile_valid_transition(reason)
            ? reason : RXVM_TRANSITION_SEQUENTIAL;
    state->transition_start_ns = now_ns;
}

RXVM_PROFILE_INLINE void rxvm_profile_instruction_terminal_at(
        rxvm_profile_state *state, uint64_t now_ns) {
    int completed_return = 0;
    if (!state->enabled) return;
    if (state->instruction_active) {
        uint64_t elapsed_ns = rxvm_profile_elapsed(state->instruction_start_ns,
                                                   now_ns);
        rxvm_profile_add_counter(state, &state->instructions[state->active_opcode],
                                 elapsed_ns);
        completed_return = state->active_opcode == OP_RET ||
                state->active_opcode == OP_RET_REG ||
                state->active_opcode == OP_RET_INT ||
                state->active_opcode == OP_RET_FLOAT ||
                state->active_opcode == OP_RET_STRING;
        if (!completed_return &&
                state->instruction_activation_index < state->activation_count) {
            uint64_t self_ns = elapsed_ns >= state->native_elapsed_in_instruction
                    ? elapsed_ns - state->native_elapsed_in_instruction : 0;
            rxvm_profile_add_activation_self(
                    state, state->instruction_activation_index, self_ns);
        }
        state->instruction_active = 0;
        rxvm_profile_add_counter(state,
                                 &state->transitions[RXVM_TRANSITION_TERMINAL], 0);
    }
    rxvm_profile_finish_pending_exit_at(state, now_ns);
    if (completed_return && state->activation_count) {
        rxvm_profile_close_top_at(state, state->instruction_start_ns, now_ns,
                                  1, 0);
    }
    while (state->activation_count)
        rxvm_profile_close_top_at(state, now_ns, now_ns, 0, 0);
}

RXVM_PROFILE_INLINE void rxvm_profile_frame_activate_at(
        rxvm_profile_state *state, const void *frame, size_t procedure_id,
        rxvm_transition_reason reason, uint64_t now_ns) {
    if (!state->enabled) return;
    if (reason == RXVM_TRANSITION_RETURN) {
        if (state->activation_count) {
            uint64_t body_end_ns = state->instruction_active
                    ? state->instruction_start_ns : now_ns;
            rxvm_profile_close_top_at(state, body_end_ns, now_ns, 1, 1);
        }
        if (state->activation_count &&
                state->activations[state->activation_count - 1].restoration_pending) {
            state->activations[state->activation_count - 1].restoration_ready = 1;
        }
        return;
    }
    if (reason == RXVM_TRANSITION_EXTERNAL_ENTRY &&
            !state->instruction_active && !state->transition_pending) {
        state->transition_pending = 1;
        state->pending_transition = RXVM_TRANSITION_EXTERNAL_ENTRY;
        state->transition_start_ns = now_ns;
    }
    if (reason == RXVM_TRANSITION_INTERRUPT_ENTRY && state->activation_count &&
            state->activations[state->activation_count - 1].frame == frame) {
        return;
    }
    if (reason == RXVM_TRANSITION_CALL ||
            reason == RXVM_TRANSITION_EXTERNAL_ENTRY ||
            reason == RXVM_TRANSITION_INTERRUPT_ENTRY) {
        uint64_t call_start_ns = state->instruction_active
                ? state->instruction_start_ns : now_ns;
        rxvm_profile_push_activation(state, frame, procedure_id, call_start_ns);
    }
}

RXVM_PROFILE_INLINE void rxvm_profile_frame_unwind_at(
        rxvm_profile_state *state, const void *frame, uint64_t now_ns) {
    if (!state->enabled || !frame) return;
    rxvm_profile_finish_pending_exit_at(state, now_ns);
    if (state->instruction_active && state->activation_count &&
            state->instruction_activation_index == state->activation_count - 1) {
        uint64_t elapsed_ns = rxvm_profile_elapsed(state->instruction_start_ns,
                                                   now_ns);
        uint64_t self_ns = elapsed_ns >= state->native_elapsed_in_instruction
                ? elapsed_ns - state->native_elapsed_in_instruction : 0;
        rxvm_profile_add_activation_self(state,
                                         state->instruction_activation_index,
                                         self_ns);
        state->instruction_activation_index = SIZE_MAX;
    }
    while (state->activation_count) {
        const void *top_frame = state->activations[state->activation_count - 1].frame;
        rxvm_profile_close_top_at(state, now_ns, now_ns, 0, 0);
        if (top_frame == frame) return;
    }
    rxvm_profile_increment(state, &state->invalid_events);
}

RXVM_PROFILE_INLINE void rxvm_profile_native_begin_at(
        rxvm_profile_state *state, size_t procedure_id, uint64_t now_ns) {
    if (!state->enabled) return;
    if (state->native_active) {
        rxvm_profile_increment(state, &state->invalid_events);
        return;
    }
    if (!rxvm_profile_valid_procedure(state, procedure_id)) {
        rxvm_profile_increment(state, &state->invalid_events);
        return;
    }
    state->native_active = 1;
    state->native_procedure_id = procedure_id;
    state->native_start_ns = now_ns;
    rxvm_profile_increment(state, &state->procedures[procedure_id].calls);
}

RXVM_PROFILE_INLINE void rxvm_profile_native_end_at(rxvm_profile_state *state,
                                                    uint64_t now_ns) {
    uint64_t elapsed_ns;
    rxvm_profile_procedure *procedure;
    if (!state->enabled || !state->native_active) return;
    elapsed_ns = rxvm_profile_elapsed(state->native_start_ns, now_ns);
    procedure = &state->procedures[state->native_procedure_id];
    rxvm_profile_increment(state, &procedure->completed);
    rxvm_profile_add_counter(state, &procedure->native_total, elapsed_ns);
    rxvm_profile_add_counter(state, &procedure->elapsed, elapsed_ns);
    if (state->instruction_active)
        rxvm_profile_add_total(state, &state->native_elapsed_in_instruction,
                               elapsed_ns);
    if (state->activation_count)
        rxvm_profile_add_total(
                state,
                &state->activations[state->activation_count - 1].native_child_ns,
                elapsed_ns);
    state->native_active = 0;
}

RXVM_PROFILE_INLINE void rxvm_profile_interrupt_poll(rxvm_profile_state *state) {
    if (state->enabled) rxvm_profile_increment(state, &state->interrupt_polls);
}

RXVM_PROFILE_INLINE void rxvm_profile_interrupt_scan_begin_at(
        rxvm_profile_state *state, uint64_t now_ns) {
    if (!state->enabled) return;
    rxvm_profile_finish_mechanics_at(state, now_ns);
    if (state->interrupt_scan_active)
        rxvm_profile_increment(state, &state->invalid_events);
    state->interrupt_scan_active = 1;
    state->interrupt_scan_start_ns = now_ns;
}

RXVM_PROFILE_INLINE void rxvm_profile_interrupt_select_at(
        rxvm_profile_state *state, unsigned char signal, uint64_t now_ns) {
    if (!state->enabled) return;
    rxvm_profile_finish_scan_at(state, now_ns, 1);
    if (rxvm_profile_valid_signal(signal)) {
        rxvm_profile_increment(state, &state->interrupts[signal].selected);
    } else {
        rxvm_profile_increment(state, &state->invalid_events);
    }
    state->interrupt_mechanics_active = 1;
    state->interrupt_mechanics_signal = signal;
    state->interrupt_mechanics_start_ns = now_ns;
    if (state->transition_pending)
        state->pending_transition = RXVM_TRANSITION_INTERRUPT_ENTRY;
}

RXVM_PROFILE_INLINE void rxvm_profile_interrupt_entry(rxvm_profile_state *state,
                                                      unsigned char signal) {
    if (!state->enabled) return;
    if (rxvm_profile_valid_signal(signal)) {
        rxvm_profile_increment(state, &state->interrupts[signal].entries);
    } else {
        rxvm_profile_increment(state, &state->invalid_events);
    }
}

RXVM_PROFILE_INLINE void rxvm_profile_interrupt_resume_at(
        rxvm_profile_state *state, unsigned char signal, uint64_t now_ns) {
    if (!state->enabled) return;
    rxvm_profile_finish_scan_at(state, now_ns, 0);
    rxvm_profile_finish_mechanics_at(state, now_ns);
    if (rxvm_profile_valid_signal(signal))
        rxvm_profile_increment(state, &state->interrupts[signal].resumes);
}

RXVM_PROFILE_INLINE void rxvm_profile_interrupt_terminal_at(
        rxvm_profile_state *state, unsigned char signal, uint64_t now_ns) {
    if (!state->enabled) return;
    rxvm_profile_finish_scan_at(state, now_ns, 0);
    rxvm_profile_finish_mechanics_at(state, now_ns);
    if (rxvm_profile_valid_signal(signal)) {
        rxvm_profile_increment(state, &state->interrupts[signal].terminals);
    } else {
        rxvm_profile_increment(state, &state->invalid_events);
    }
    rxvm_profile_finish_transition_at(state, now_ns);
}

#undef RXVM_PROFILE_INLINE

#endif
