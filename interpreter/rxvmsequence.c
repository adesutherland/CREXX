/* Dynamic RXSEQ site counter and versioned extractor. */

#include "rxvmsequence.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxseqfile.h"
#include "rxvmintp.h"

static void rxvm_sequence_free(rxvm_sequence_state *state) {
    size_t i;
    if (!state) return;
    for (i = 0; i < state->module_count; i++) free(state->modules[i].counts);
    free(state->modules);
    state->modules = 0;
    state->module_count = 0;
}

void rxvm_sequence_begin(rxvm_sequence_state *state,
                         struct rxvm_context *context,
                         unsigned int length,
                         int enabled) {
    size_t i;
    memset(state, 0, sizeof(*state));
    if (!enabled) return;
    if (length < 2 || length > 4) {
        fprintf(stderr, "ERROR: sequence count must be 2, 3, or 4\n");
        return;
    }
    state->length = length;
    state->module_count = context->num_modules;
    state->modules = calloc(state->module_count, sizeof(*state->modules));
    if (!state->modules && state->module_count) {
        fprintf(stderr, "ERROR: cannot allocate RXSEQ module counters\n");
        return;
    }
    for (i = 0; i < state->module_count; i++) {
        size_t size = context->modules[i]->segment.inst_size;
        state->modules[i].instruction_size = size;
        state->modules[i].counts = calloc(size, sizeof(uint64_t));
        if (!state->modules[i].counts && size) {
            fprintf(stderr, "ERROR: cannot allocate RXSEQ instruction counters\n");
            rxvm_sequence_free(state);
            return;
        }
    }
    state->previous_transition = RXVM_TRANSITION_EXTERNAL_ENTRY;
    state->enabled = 1;
}

void rxvm_sequence_break(rxvm_sequence_state *state) {
    if (!state || !state->enabled) return;
    state->window_used = 0;
    state->have_previous = 0;
    state->previous_transition = RXVM_TRANSITION_EXTERNAL_ENTRY;
}

void rxvm_sequence_instruction_begin(rxvm_sequence_state *state,
                                     size_t module_number,
                                     size_t instruction_index) {
    rxvm_sequence_module *module;
    unsigned int i;
    size_t start;
    if (!state || !state->enabled) return;
    if (!module_number || module_number > state->module_count) {
        rxvm_sequence_break(state);
        return;
    }
    module = &state->modules[module_number - 1];
    if (instruction_index >= module->instruction_size) {
        rxvm_sequence_break(state);
        return;
    }
    if (!state->have_previous ||
            state->previous_transition != RXVM_TRANSITION_SEQUENTIAL ||
            state->window_module != module_number) {
        state->window_used = 0;
    }
    if (state->window_used == state->length) {
        for (i = 1; i < state->window_used; i++)
            state->window[i - 1] = state->window[i];
        state->window_used--;
    }
    state->window[state->window_used++] = instruction_index;
    state->window_module = module_number;
    state->have_previous = 1;
    state->previous_transition = RXVM_TRANSITION_SEQUENTIAL;
    if (state->window_used != state->length) return;
    start = state->window[0];
    if (module->counts[start] == UINT64_MAX) {
        state->overflowed = 1;
    } else {
        if (!module->counts[start]) state->site_count++;
        module->counts[start]++;
    }
}

void rxvm_sequence_instruction_retire(rxvm_sequence_state *state,
                                      rxvm_transition_reason reason) {
    if (!state || !state->enabled) return;
    state->previous_transition = reason;
}

void rxvm_sequence_report(rxvm_sequence_state *state,
                          const struct rxvm_context *context,
                          const char *output_path,
                          int result) {
    FILE *out;
    size_t i;
    int write_ok = 1;
    if (!state || !state->enabled) return;
    if (!output_path || !*output_path) {
        fprintf(stderr, "ERROR: RXSEQ extraction requires an output path\n");
        rxvm_sequence_free(state);
        state->enabled = 0;
        return;
    }
    out = fopen(output_path, "wb");
    if (!out) {
        fprintf(stderr, "ERROR: cannot write RXSEQ output %s\n", output_path);
        rxvm_sequence_free(state);
        state->enabled = 0;
        return;
    }
    write_ok = rxseq_write_bytes(out, rxseq_file_magic, 8) &&
            rxseq_write_u32(out, RXSEQ_FORMAT_VERSION) &&
            rxseq_write_u32(out, RXSEQ_HEADER_SIZE) &&
            rxseq_write_u32(out, state->length) &&
            rxseq_write_u32(out, (uint32_t)result) &&
            rxseq_write_u32(out, state->overflowed ? RXSEQ_FLAG_OVERFLOW : 0) &&
            rxseq_write_u32(out, 0) &&
            rxseq_write_u64(out, (uint64_t)state->module_count) &&
            rxseq_write_u64(out, state->site_count);
    for (i = 0; i < state->module_count; i++) {
        const module *mod = context->modules[i];
        const char *name = mod->name ? mod->name : "";
        size_t name_size = strlen(name);
        write_ok = write_ok &&
                rxseq_write_varuint(out, (uint64_t)i + 1) &&
                rxseq_write_u64(out, rxseq_hash_module_file(mod->file)) &&
                rxseq_write_varuint(out, (uint64_t)mod->segment.inst_size) &&
                rxseq_write_varuint(out, (uint64_t)name_size) &&
                rxseq_write_bytes(out, name, name_size);
    }
    for (i = 0; i < state->module_count; i++) {
        size_t index;
        for (index = 0; index < state->modules[i].instruction_size; index++) {
            uint64_t count = state->modules[i].counts[index];
            if (count) {
                write_ok = write_ok &&
                        rxseq_write_varuint(out, (uint64_t)i + 1) &&
                        rxseq_write_varuint(out, (uint64_t)index) &&
                        rxseq_write_varuint(out, count);
            }
        }
    }
    if (!write_ok)
        fprintf(stderr, "ERROR: failed while writing RXSEQ output %s\n", output_path);
    if (fclose(out) != 0)
        fprintf(stderr, "ERROR: failed while closing RXSEQ output %s\n", output_path);
    rxvm_sequence_free(state);
    state->enabled = 0;
}
