/* Compile-time VM timing profiler state and event accounting. */

#ifndef CREXX_RXVMPROFILE_H
#define CREXX_RXVMPROFILE_H

#include <stdint.h>
#include <stdio.h>
#include "rxdefs.h"
#include "rxsignal.h"
#include "rxvminstrument.h"

#if defined(_MSC_VER)
#define RXVM_PROFILE_INLINE static __inline
#else
#define RXVM_PROFILE_INLINE static inline
#endif

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

typedef struct rxvm_profile_state {
    int enabled;
    int overflowed;
    uint64_t invalid_events;
    uint64_t timer_read_min_ns;
    uint64_t timer_zero_deltas;

    int instruction_active;
    int active_opcode;
    uint64_t instruction_start_ns;
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

    rxvm_profile_counter instructions[OP_MAX_INSTRUCTIONS];
    rxvm_profile_counter transitions[RXVM_TRANSITION_COUNT];
    rxvm_profile_counter interrupt_scans;
    rxvm_profile_counter interrupt_mechanics;
    rxvm_profile_interrupt_counter interrupts[RXSIGNAL_MAX];
} rxvm_profile_state;

typedef const char *(*rxvm_profile_signal_name_fn)(unsigned char signal);

uint64_t rxvm_profile_now_ns(void);
void rxvm_profile_begin(rxvm_profile_state *state, int enabled);
void rxvm_profile_report(const rxvm_profile_state *state,
                         const char *output_path,
                         const char *vm_mode,
                         int result,
                         const Instruction *instruction_map,
                         rxvm_profile_signal_name_fn signal_name);

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

RXVM_PROFILE_INLINE void rxvm_profile_add_counter(rxvm_profile_state *state,
                                                  rxvm_profile_counter *counter,
                                                  uint64_t elapsed_ns) {
    int first = counter->count == 0;
    rxvm_profile_increment(state, &counter->count);
    if (UINT64_MAX - counter->total_ns < elapsed_ns) {
        counter->total_ns = UINT64_MAX;
        state->overflowed = 1;
    } else {
        counter->total_ns += elapsed_ns;
    }
    if (first || elapsed_ns < counter->min_ns) counter->min_ns = elapsed_ns;
    if (first || elapsed_ns > counter->max_ns) counter->max_ns = elapsed_ns;
}

RXVM_PROFILE_INLINE int rxvm_profile_valid_transition(rxvm_transition_reason reason) {
    return reason >= RXVM_TRANSITION_SEQUENTIAL && reason < RXVM_TRANSITION_COUNT;
}

RXVM_PROFILE_INLINE int rxvm_profile_valid_signal(unsigned char signal) {
    return signal > RXSIGNAL_NONE && signal < RXSIGNAL_MAX;
}

RXVM_PROFILE_INLINE void rxvm_profile_finish_transition_at(rxvm_profile_state *state,
                                                           uint64_t now_ns) {
    if (!state->transition_pending) return;
    if (!rxvm_profile_valid_transition(state->pending_transition)) {
        rxvm_profile_increment(state, &state->invalid_events);
        state->pending_transition = RXVM_TRANSITION_SEQUENTIAL;
    }
    rxvm_profile_add_counter(
            state,
            &state->transitions[state->pending_transition],
            rxvm_profile_elapsed(state->transition_start_ns, now_ns));
    state->transition_pending = 0;
}

RXVM_PROFILE_INLINE void rxvm_profile_finish_scan_at(rxvm_profile_state *state,
                                                     uint64_t now_ns,
                                                     int selected) {
    if (!state->interrupt_scan_active) return;
    rxvm_profile_add_counter(
            state,
            &state->interrupt_scans,
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

RXVM_PROFILE_INLINE void rxvm_profile_instruction_begin_at(rxvm_profile_state *state,
                                                           int opcode,
                                                           uint64_t now_ns) {
    if (!state->enabled) return;
    rxvm_profile_finish_mechanics_at(state, now_ns);
    rxvm_profile_finish_transition_at(state, now_ns);
    if (state->instruction_active)
        rxvm_profile_increment(state, &state->invalid_events);
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) {
        rxvm_profile_increment(state, &state->invalid_events);
        opcode = OP_IUNKNOWN;
    }
    state->instruction_active = 1;
    state->active_opcode = opcode;
    state->instruction_start_ns = now_ns;
    state->current_transition = RXVM_TRANSITION_SEQUENTIAL;
}

RXVM_PROFILE_INLINE void rxvm_profile_instruction_retire_at(
        rxvm_profile_state *state,
        rxvm_transition_reason reason,
        uint64_t now_ns) {
    if (!state->enabled || !state->instruction_active) return;
    rxvm_profile_add_counter(
            state,
            &state->instructions[state->active_opcode],
            rxvm_profile_elapsed(state->instruction_start_ns, now_ns));
    state->instruction_active = 0;
    state->transition_pending = 1;
    state->pending_transition = rxvm_profile_valid_transition(reason)
            ? reason : RXVM_TRANSITION_SEQUENTIAL;
    state->transition_start_ns = now_ns;
}

RXVM_PROFILE_INLINE void rxvm_profile_instruction_terminal_at(
        rxvm_profile_state *state,
        uint64_t now_ns) {
    if (!state->enabled || !state->instruction_active) return;
    rxvm_profile_add_counter(
            state,
            &state->instructions[state->active_opcode],
            rxvm_profile_elapsed(state->instruction_start_ns, now_ns));
    state->instruction_active = 0;
    rxvm_profile_add_counter(state,
                             &state->transitions[RXVM_TRANSITION_TERMINAL], 0);
}

RXVM_PROFILE_INLINE void rxvm_profile_frame_activate_at(
        rxvm_profile_state *state,
        rxvm_transition_reason reason,
        uint64_t now_ns) {
    if (!state->enabled) return;
    if (reason == RXVM_TRANSITION_EXTERNAL_ENTRY &&
            !state->instruction_active && !state->transition_pending) {
        state->transition_pending = 1;
        state->pending_transition = RXVM_TRANSITION_EXTERNAL_ENTRY;
        state->transition_start_ns = now_ns;
    }
}

RXVM_PROFILE_INLINE void rxvm_profile_interrupt_poll(rxvm_profile_state *state) {
    if (state->enabled) rxvm_profile_increment(state, &state->interrupt_polls);
}

RXVM_PROFILE_INLINE void rxvm_profile_interrupt_scan_begin_at(
        rxvm_profile_state *state,
        uint64_t now_ns) {
    if (!state->enabled) return;
    rxvm_profile_finish_mechanics_at(state, now_ns);
    if (state->interrupt_scan_active)
        rxvm_profile_increment(state, &state->invalid_events);
    state->interrupt_scan_active = 1;
    state->interrupt_scan_start_ns = now_ns;
}

RXVM_PROFILE_INLINE void rxvm_profile_interrupt_select_at(
        rxvm_profile_state *state,
        unsigned char signal,
        uint64_t now_ns) {
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
    if (state->transition_pending) {
        state->pending_transition = RXVM_TRANSITION_INTERRUPT_ENTRY;
    }
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
        rxvm_profile_state *state,
        unsigned char signal,
        uint64_t now_ns) {
    if (!state->enabled) return;
    rxvm_profile_finish_scan_at(state, now_ns, 0);
    rxvm_profile_finish_mechanics_at(state, now_ns);
    if (rxvm_profile_valid_signal(signal)) {
        rxvm_profile_increment(state, &state->interrupts[signal].resumes);
    }
}

RXVM_PROFILE_INLINE void rxvm_profile_interrupt_terminal_at(
        rxvm_profile_state *state,
        unsigned char signal,
        uint64_t now_ns) {
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
