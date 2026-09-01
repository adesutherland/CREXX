/* Dynamic, sequential instruction-window extraction for profiling builds. */

#ifndef CREXX_RXVMSEQUENCE_H
#define CREXX_RXVMSEQUENCE_H

#include <stddef.h>
#include <stdint.h>

#include "rxvminstrument.h"

struct rxvm_context;

typedef struct rxvm_sequence_module {
    uint64_t *counts;
    size_t instruction_size;
} rxvm_sequence_module;

typedef struct rxvm_sequence_state {
    int enabled;
    int overflowed;
    uint64_t site_count;
    unsigned int length;
    size_t module_count;
    rxvm_sequence_module *modules;
    size_t window[4];
    unsigned int window_used;
    size_t window_module;
    rxvm_transition_reason previous_transition;
    int have_previous;
} rxvm_sequence_state;

void rxvm_sequence_begin(rxvm_sequence_state *state,
                         struct rxvm_context *context,
                         unsigned int length,
                         int enabled);
void rxvm_sequence_instruction_begin(rxvm_sequence_state *state,
                                     size_t module_number,
                                     size_t instruction_index);
void rxvm_sequence_instruction_retire(rxvm_sequence_state *state,
                                      rxvm_transition_reason reason);
void rxvm_sequence_break(rxvm_sequence_state *state);
void rxvm_sequence_report(rxvm_sequence_state *state,
                          const struct rxvm_context *context,
                          const char *output_path,
                          int result);

#endif
