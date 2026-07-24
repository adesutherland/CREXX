/* Reporting and monotonic clock support for compile-time VM profiling. */

#include "rxvmprofile.h"
#include "rxvmintp.h"

#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#ifdef __APPLE__
#include <time.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#endif

static const char *const rxvm_profile_transition_names[RXVM_TRANSITION_COUNT] = {
        "sequential_same_frame",
        "branch_same_frame",
        "call_enter_frame",
        "return_leave_frame",
        "interrupt_entry",
        "interrupt_resume",
        "external_entry",
        "terminal"
};

static const char *const rxvm_profile_allocation_names[RXVM_PROFILE_ALLOC_COUNT] = {
        "frame_blocks",
        "standalone_values",
        "attribute_value_blocks",
        "attribute_pointer_storage",
        "string_buffers",
        "binary_buffers"
};

static const char *const rxvm_profile_call_path_names[RXVM_PROFILE_CALL_PATH_COUNT] = {
        "direct_bytecode", "direct_native", "dynamic_bytecode",
        "dynamic_native", "external_root", "signal_bytecode", "signal_native"
};

static const char *const rxvm_profile_census_kind_names[
        RXVM_PROFILE_CENSUS_CALLABLE_KIND_COUNT] = {
        "procedure", "method", "factory", "native", "unknown"
};

static const char *const rxvm_profile_frame_disposition_names[
        RXVM_PROFILE_FRAME_DISPOSITION_COUNT] = {
        "fresh", "reused", "no_child_native", "none_failed"
};

static const char *const rxvm_profile_call_outcome_names[
        RXVM_PROFILE_CALL_OUTCOME_COUNT] = {
        "success", "unresolved", "frame_failed", "invalid"
};

static const char *const rxvm_profile_return_placement_names[
        RXVM_PROFILE_RETURN_PLACEMENT_COUNT] = {
        "void", "ret_reg_move_local", "ret_reg_copy_nonlocal",
        "value_ignored", "immediate", "terminal_external"
};

static const char *const rxvm_profile_dynamic_kind_names[
        RXVM_PROFILE_DYNAMIC_KIND_COUNT] = {
        "srcmethodsel", "srcfprocsel"
};

static const char *const rxvm_profile_dynamic_outcome_names[
        RXVM_PROFILE_DYNAMIC_OUTCOME_COUNT] = {
        "attempt", "success", "failure"
};

static const char *const rxvm_profile_value_operation_names[
        RXVM_PROFILE_VALUE_OPERATION_COUNT] = {
        "copy", "string_copy", "binary_copy", "decimal_copy",
        "integer_copy", "float_copy", "status_copy", "move",
        "clear_contents", "reset_reuse", "destroy", "clear"
};

static const char *const rxvm_profile_value_shape_names[
        RXVM_PROFILE_VALUE_SHAPE_COUNT] = {
        "empty", "scalar", "string", "binary", "decimal", "reference",
        "object", "native", "compound"
};

static const char *const rxvm_profile_frame_phase_names[
        RXVM_PROFILE_FRAME_PHASE_COUNT] = {
        "local_relink", "global_relink", "argument_count_reset",
        "inherited_context", "root_context", "finalize"
};

static const char *const rxvm_profile_frame_source_names[
        RXVM_PROFILE_FRAME_SOURCE_COUNT] = { "fresh", "reused" };

#if defined(_MSC_VER)
#define RXVM_PROFILE_THREAD_LOCAL __declspec(thread)
#else
#define RXVM_PROFILE_THREAD_LOCAL __thread
#endif

static RXVM_PROFILE_THREAD_LOCAL rxvm_profile_state *rxvm_active_allocation_profile;

uint64_t rxvm_profile_now_ns(void) {
#ifdef __APPLE__
    return clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
#elif defined(_WIN32)
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;
    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&frequency);
    if (!frequency.QuadPart) return 0;
    return (uint64_t)((counter.QuadPart / frequency.QuadPart) * 1000000000ULL +
            ((counter.QuadPart % frequency.QuadPart) * 1000000000ULL) /
                    frequency.QuadPart);
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#endif
}

void rxvm_profile_record_allocation(rxvm_profile_allocation_kind kind,
                                    size_t bytes, size_t value_slots) {
    rxvm_profile_add_allocation_at(rxvm_active_allocation_profile, kind,
                                   (uint64_t)bytes, (uint64_t)value_slots);
}

void rxvm_profile_record_frame_activation(int reused, size_t frame_bytes,
                                          size_t value_slots) {
    rxvm_profile_frame_activation_at(rxvm_active_allocation_profile, reused,
                                     (uint64_t)frame_bytes,
                                     (uint64_t)value_slots);
}

void rxvm_profile_record_frame_release(void) {
    rxvm_profile_frame_release_at(rxvm_active_allocation_profile);
}

static rxvm_profile_value_shape rxvm_profile_value_payload_shape(
        const value *payload) {
    int shapes = 0;
    rxvm_profile_value_shape shape = RXVM_PROFILE_VALUE_EMPTY;
    if (!payload) return RXVM_PROFILE_VALUE_EMPTY;
    if (payload->string_length) {
        shape = RXVM_PROFILE_VALUE_STRING;
        shapes++;
    }
    if (payload->binary_length) {
        shape = payload->native_payload_ops
                ? RXVM_PROFILE_VALUE_NATIVE : RXVM_PROFILE_VALUE_BINARY;
        shapes++;
    }
    if (payload->decimal_value_length) {
        shape = RXVM_PROFILE_VALUE_DECIMAL;
        shapes++;
    }
    if (payload->reference_payload || payload->reference_identity) {
        shape = RXVM_PROFILE_VALUE_REFERENCE;
        shapes++;
    }
    if (payload->num_attributes || payload->object_type) {
        shape = RXVM_PROFILE_VALUE_OBJECT;
        shapes++;
    }
    if (shapes > 1) return RXVM_PROFILE_VALUE_COMPOUND;
    if (shapes == 1) return shape;
    if (payload->status.all_type_flags || payload->int_value ||
            payload->float_value)
        return RXVM_PROFILE_VALUE_SCALAR;
    return RXVM_PROFILE_VALUE_EMPTY;
}

static uint64_t rxvm_profile_value_payload_bytes(const value *payload) {
    uint64_t bytes = 0;
    uint64_t attribute_bytes;
    if (!payload) return 0;
    bytes = (uint64_t)payload->string_length;
    if (UINT64_MAX - bytes < (uint64_t)payload->binary_length)
        return UINT64_MAX;
    bytes += (uint64_t)payload->binary_length;
    if (UINT64_MAX - bytes < (uint64_t)payload->decimal_value_length)
        return UINT64_MAX;
    bytes += (uint64_t)payload->decimal_value_length;
    if (payload->num_attributes > UINT64_MAX / sizeof(value *))
        return UINT64_MAX;
    attribute_bytes = (uint64_t)payload->num_attributes * sizeof(value *);
    if (UINT64_MAX - bytes < attribute_bytes) return UINT64_MAX;
    return bytes + attribute_bytes;
}

void rxvm_profile_record_value_typed(rxvm_profile_value_operation operation,
                                     rxvm_profile_value_shape shape,
                                     size_t bytes) {
    rxvm_profile_state *state = rxvm_active_allocation_profile;
    if (!state || !state->enabled) return;
    if (operation < 0 || operation >= RXVM_PROFILE_VALUE_OPERATION_COUNT ||
            shape < 0 || shape >= RXVM_PROFILE_VALUE_SHAPE_COUNT) {
        rxvm_profile_increment(state, &state->invalid_events);
        return;
    }
    rxvm_profile_increment(
            state, &state->value_operations[operation][shape].count);
    rxvm_profile_add_total(
            state, &state->value_operations[operation][shape].bytes,
            (uint64_t)bytes);
    if ((uint64_t)bytes >
            state->value_operations[operation][shape].max_bytes)
        state->value_operations[operation][shape].max_bytes = (uint64_t)bytes;
}

void rxvm_profile_record_value_operation(rxvm_profile_value_operation operation,
                                         const value *payload) {
    uint64_t bytes = rxvm_profile_value_payload_bytes(payload);
    rxvm_profile_record_value_typed(
            operation, rxvm_profile_value_payload_shape(payload),
            (size_t)bytes);
}

uint64_t rxvm_profile_frame_phase_begin(void) {
    rxvm_profile_state *state = rxvm_active_allocation_profile;
    return state && state->enabled && state->timing_enabled
            ? rxvm_profile_now_ns() : 0;
}

void rxvm_profile_record_frame_phase(rxvm_profile_frame_phase phase,
                                     int reused, uint64_t start_ns,
                                     size_t units) {
    rxvm_profile_state *state = rxvm_active_allocation_profile;
    rxvm_profile_frame_source source = reused
            ? RXVM_PROFILE_FRAME_SOURCE_REUSED
            : RXVM_PROFILE_FRAME_SOURCE_FRESH;
    uint64_t end_ns;
    if (!state || !state->enabled) return;
    if (phase < 0 || phase >= RXVM_PROFILE_FRAME_PHASE_COUNT) {
        rxvm_profile_increment(state, &state->invalid_events);
        return;
    }
    end_ns = state->timing_enabled ? rxvm_profile_now_ns() : 0;
    rxvm_profile_add_counter(state, &state->frame_phases[phase][source],
                             rxvm_profile_elapsed(start_ns, end_ns));
    rxvm_profile_add_total(state, &state->frame_phase_units[phase][source],
                           (uint64_t)units);
}

static int rxvm_profile_is_branch_opcode(int opcode) {
    RxOpEffects effects;
    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return 0;
    effects = rxop_effects(opcode);
    return effects.flow == FLOW_JUMP || effects.flow == FLOW_COND;
}

static rxvm_profile_branch_row *rxvm_profile_branch_row_for_active(
        rxvm_profile_state *state) {
    size_t i;
    rxvm_profile_branch_row *rows;
    size_t capacity;
    for (i = 0; i < state->branch_row_count; i++) {
        rxvm_profile_branch_row *row = &state->branch_rows[i];
        if (row->module_id == state->active_module_id &&
                row->instruction_index == state->active_instruction_index &&
                row->opcode == state->active_opcode)
            return row;
    }
    if (state->branch_row_count == state->branch_row_capacity) {
        capacity = state->branch_row_capacity
                ? state->branch_row_capacity * 2 : 64;
        if (capacity < state->branch_row_capacity ||
                capacity > SIZE_MAX / sizeof(*rows)) {
            state->branch_tracking_unavailable = 1;
            return 0;
        }
        rows = (rxvm_profile_branch_row *)realloc(
                state->branch_rows, capacity * sizeof(*rows));
        if (!rows) {
            state->branch_tracking_unavailable = 1;
            return 0;
        }
        state->branch_rows = rows;
        state->branch_row_capacity = capacity;
    }
    rows = state->branch_rows;
    memset(&rows[state->branch_row_count], 0, sizeof(*rows));
    rows[state->branch_row_count].module_id = state->active_module_id;
    rows[state->branch_row_count].instruction_index =
            state->active_instruction_index;
    rows[state->branch_row_count].opcode = state->active_opcode;
    return &rows[state->branch_row_count++];
}

void rxvm_profile_record_branch_at(rxvm_profile_state *state,
                                   size_t target_module_id,
                                   size_t target_instruction_index,
                                   rxvm_transition_reason reason) {
    rxvm_profile_branch_row *row;
    int taken;
    if (!state || !state->enabled ||
            !rxvm_profile_is_branch_opcode(state->active_opcode))
        return;
    row = rxvm_profile_branch_row_for_active(state);
    if (!row) return;
    rxvm_profile_increment(state, &row->executions);
    taken = reason == RXVM_TRANSITION_BRANCH;
    if (taken) {
        rxvm_profile_increment(state, &row->taken);
        if (target_module_id != state->active_module_id)
            rxvm_profile_increment(state, &row->cross_module);
        else if (target_instruction_index <= state->active_instruction_index)
            rxvm_profile_increment(state, &row->backward);
    } else {
        rxvm_profile_increment(state, &row->fallthrough);
    }
}

static size_t rxvm_profile_trace_swap_pairs(
        const rxvm_profile_trace_record *record, size_t pairs[][2],
        size_t capacity) {
    static const unsigned char swapn2[][2] = {{1, 2}, {3, 4}};
    static const unsigned char swapn3[][2] = {{1, 2}, {3, 4}, {5, 6}};
    static const unsigned char swapn4[][2] = {
        {1, 2}, {3, 4}, {5, 6}, {7, 8}};
    static const unsigned char settpswap[][2] = {{1, 3}};
    static const unsigned char loadsettpswap[][2] = {{3, 5}};
    static const unsigned char swapsettp[][2] = {{1, 2}};
    static const unsigned char swapsettpswap[][2] = {{1, 2}, {3, 5}};
    static const unsigned char settpswap2[][2] = {{1, 3}, {4, 5}};
    static const unsigned char swapcall[][2] = {{4, 5}};
    static const unsigned char settpswapcall[][2] = {{4, 6}};
    const unsigned char (*operands)[2] = 0;
    size_t count = 0;
    size_t i;
    if (!record || !record->pc || !pairs || !capacity) return 0;
    switch (record->opcode) {
        case OP_SWAP_REG_REG:
            operands = swapn2;
            count = 1;
            break;
        case OP_SWAPN_REG_REG_REG_REG:
            operands = swapn2;
            count = 2;
            break;
        case OP_SWAPN_REG_REG_REG_REG_REG_REG:
            operands = swapn3;
            count = 3;
            break;
        case OP_SWAPN_REG_REG_REG_REG_REG_REG_REG_REG:
            operands = swapn4;
            count = 4;
            break;
        case OP_SETTPSWAP_REG_INT_REG:
            operands = settpswap;
            count = 1;
            break;
        case OP_LOADSETTPSWAP_REG_INT_REG_INT_REG:
            operands = loadsettpswap;
            count = 1;
            break;
        case OP_SWAPSETTP_REG_REG_REG_INT:
            operands = swapsettp;
            count = 1;
            break;
        case OP_SWAPSETTPSWAP_REG_REG_REG_INT_REG:
            operands = swapsettpswap;
            count = 2;
            break;
        case OP_SETTPSWAPSETTPSWAP_REG_INT_REG_REG_REG:
            operands = settpswap2;
            count = 2;
            break;
        case OP_SWAPCALL_REG_FUNC_REG_REG_REG:
            operands = swapcall;
            count = 1;
            break;
        case OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG:
            operands = settpswapcall;
            count = 1;
            break;
        default:
            return 0;
    }
    if (count > capacity) count = capacity;
    for (i = 0; i < count; i++) {
        pairs[i][0] = (record->pc + operands[i][0])->index;
        pairs[i][1] = (record->pc + operands[i][1])->index;
    }
    return count;
}

static rxvm_profile_activation *rxvm_profile_find_activation(
        rxvm_profile_state *state, const void *frame) {
    size_t i;
    if (!state || !frame) return 0;
    for (i = state->activation_count; i > 0; i--) {
        if (state->activations[i - 1].frame == frame)
            return &state->activations[i - 1];
    }
    return 0;
}

static void rxvm_profile_abandon_restoration(
        rxvm_profile_activation *activation) {
    if (!activation) return;
    activation->restoration_mapping_count = 0;
    activation->restoration_trace_count = 0;
    activation->restoration_call_row = SIZE_MAX;
    activation->restoration_pending = 0;
    activation->restoration_ready = 0;
}

static int rxvm_profile_trace_reserve(rxvm_profile_state *state,
                                      rxvm_profile_activation *activation) {
    rxvm_profile_trace_record *replacement;
    size_t capacity;
    if (activation->trace_count < activation->trace_capacity) return 1;
    capacity = activation->trace_capacity ? activation->trace_capacity * 2 : 32;
    replacement = (rxvm_profile_trace_record *)realloc(
            activation->trace, capacity * sizeof(*replacement));
    if (!replacement) {
        state->census_tracking_unavailable = 1;
        rxvm_profile_increment(state, &state->attribution_degraded);
        return 0;
    }
    activation->trace = replacement;
    activation->trace_capacity = capacity;
    return 1;
}

void rxvm_profile_trace_instruction_at(rxvm_profile_state *state,
                                       const void *frame_pointer,
                                       const bin_code *pc,
                                       const Instruction *instruction_map,
                                       size_t module_id,
                                       size_t instruction_index,
                                       int opcode) {
    rxvm_profile_activation *activation;
    rxvm_profile_trace_record *record;
    RxOpEffects previous_effects;
    (void)module_id;
    (void)instruction_index;
    if (!state || !state->enabled || !frame_pointer || !pc ||
            !instruction_map || opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS)
        return;
    activation = rxvm_profile_find_activation(state, frame_pointer);
    if (!activation) return;
    if (activation->trace_count) {
        previous_effects = rxop_effects(
                activation->trace[activation->trace_count - 1].opcode);
        if (previous_effects.flow != FLOW_NEXT ||
                (previous_effects.semantics &
                 (RXOP_SEM_CALL | RXOP_SEM_RETURN)) != 0) {
            if (activation->restoration_ready &&
                    activation->restoration_trace_count)
                rxvm_profile_abandon_restoration(activation);
            activation->trace_count = 0;
        }
    }
    if (!rxvm_profile_trace_reserve(state, activation)) return;
    record = &activation->trace[activation->trace_count++];
    memset(record, 0, sizeof(*record));
    record->opcode = opcode;
    record->pc = pc;
}

static rxvm_profile_census_callable_kind rxvm_profile_census_kind(
        const rxvm_profile_state *state, size_t procedure_id,
        rxvm_profile_call_path path) {
    if (path == RXVM_PROFILE_CALL_DIRECT_NATIVE ||
            path == RXVM_PROFILE_CALL_DYNAMIC_NATIVE ||
            path == RXVM_PROFILE_CALL_SIGNAL_NATIVE)
        return RXVM_PROFILE_CENSUS_NATIVE;
    if (!rxvm_profile_valid_procedure(state, procedure_id))
        return RXVM_PROFILE_CENSUS_UNKNOWN;
    if (state->procedures[procedure_id].kind == RXVM_PROFILE_FACTORY)
        return RXVM_PROFILE_CENSUS_FACTORY;
    if (state->procedures[procedure_id].kind == RXVM_PROFILE_METHOD)
        return RXVM_PROFILE_CENSUS_METHOD;
    return RXVM_PROFILE_CENSUS_PROCEDURE;
}

static size_t rxvm_profile_find_or_add_call_row(
        rxvm_profile_state *state, rxvm_profile_call_path path,
        rxvm_profile_census_callable_kind kind,
        rxvm_profile_frame_disposition disposition,
        rxvm_profile_call_outcome outcome, size_t procedure_id,
        uint64_t arity, int arity_valid, size_t site_module,
        size_t site_index) {
    size_t i;
    rxvm_profile_call_row *row;
    for (i = 0; i < state->call_row_count; i++) {
        row = &state->call_rows[i];
        if (row->path == path && row->callable_kind == kind &&
                row->frame_disposition == disposition &&
                row->outcome == outcome &&
                row->procedure_id == procedure_id && row->arity == arity &&
                row->arity_valid == arity_valid &&
                row->site_module == site_module && row->site_index == site_index)
            return i;
    }
    if (state->call_row_count == state->call_row_capacity) {
        size_t capacity = state->call_row_capacity
                ? state->call_row_capacity * 2 : 64;
        rxvm_profile_call_row *replacement =
                (rxvm_profile_call_row *)realloc(
                        state->call_rows, capacity * sizeof(*replacement));
        if (!replacement) {
            state->census_tracking_unavailable = 1;
            return SIZE_MAX;
        }
        state->call_rows = replacement;
        state->call_row_capacity = capacity;
    }
    row = &state->call_rows[state->call_row_count];
    memset(row, 0, sizeof(*row));
    row->path = path;
    row->callable_kind = kind;
    row->frame_disposition = disposition;
    row->outcome = outcome;
    row->procedure_id = procedure_id;
    row->arity = arity;
    row->arity_valid = arity_valid;
    row->site_module = site_module;
    row->site_index = site_index;
    return state->call_row_count++;
}

static void rxvm_profile_prepare_restoration(
        rxvm_profile_state *state, rxvm_profile_activation *activation,
        size_t row_index, uint64_t setup_swaps, const size_t *swap_pairs,
        size_t register_count, int native_call) {
    size_t *replacement;
    size_t i;
    int displaced = 0;
    if (!setup_swaps) return;
    rxvm_profile_abandon_restoration(activation);
    if (register_count > SIZE_MAX / sizeof(*replacement)) {
        state->census_tracking_unavailable = 1;
        rxvm_profile_increment(state, &state->attribution_degraded);
        return;
    }
    if (activation->restoration_mapping_capacity < register_count) {
        replacement = (size_t *)realloc(
                activation->restoration_mapping,
                register_count * sizeof(*replacement));
        if (!replacement) {
            state->census_tracking_unavailable = 1;
            rxvm_profile_increment(state, &state->attribution_degraded);
            return;
        }
        activation->restoration_mapping = replacement;
        activation->restoration_mapping_capacity = register_count;
    }
    for (i = 0; i < register_count; i++)
        activation->restoration_mapping[i] = i;
    for (i = (size_t)setup_swaps; i > 0; i--) {
        size_t pair_index = (i - 1) * 2;
        size_t first = swap_pairs[pair_index];
        size_t second = swap_pairs[pair_index + 1];
        size_t temporary;
        if (first >= register_count || second >= register_count) {
            state->census_tracking_unavailable = 1;
            rxvm_profile_increment(state, &state->attribution_degraded);
            rxvm_profile_abandon_restoration(activation);
            return;
        }
        temporary = activation->restoration_mapping[first];
        activation->restoration_mapping[first] =
                activation->restoration_mapping[second];
        activation->restoration_mapping[second] = temporary;
    }
    for (i = 0; i < register_count; i++) {
        if (activation->restoration_mapping[i] != i) {
            displaced = 1;
            break;
        }
    }
    if (!displaced) return;
    activation->restoration_mapping_count = register_count;
    activation->restoration_call_row = row_index;
    activation->restoration_pending = 1;
    activation->restoration_ready = native_call;
}

static void rxvm_profile_attribute_call_window(
        rxvm_profile_state *state, rxvm_profile_activation *activation,
        const stack_frame *frame, size_t argument_base, uint64_t arity,
        size_t row_index, int native_call) {
    unsigned char *needed;
    size_t *restoration_swaps;
    uint64_t setup_swaps = 0;
    uint64_t copies = 0;
    size_t i;
    if (!activation || !frame || !arity) return;
    if (arity > SIZE_MAX || argument_base > frame->number_locals ||
            (size_t)arity > frame->number_locals - argument_base) {
        state->census_tracking_unavailable = 1;
        rxvm_profile_increment(state, &state->attribution_degraded);
        return;
    }
    needed = (unsigned char *)calloc(frame->number_locals, 1);
    if (activation->trace_count > SIZE_MAX / (8 * sizeof(*restoration_swaps))) {
        free(needed);
        state->census_tracking_unavailable = 1;
        rxvm_profile_increment(state, &state->attribution_degraded);
        return;
    }
    restoration_swaps = activation->trace_count
            ? (size_t *)malloc(
                    activation->trace_count * 8 * sizeof(*restoration_swaps))
            : 0;
    if (!needed || (activation->trace_count && !restoration_swaps)) {
        free(needed);
        free(restoration_swaps);
        state->census_tracking_unavailable = 1;
        rxvm_profile_increment(state, &state->attribution_degraded);
        return;
    }
    for (i = 0; i < (size_t)arity; i++) needed[argument_base + i] = 1;
    for (i = activation->trace_count; i > 0; i--) {
        rxvm_profile_trace_record *record = &activation->trace[i - 1];
        RxOpEffects effects = rxop_effects(record->opcode);
        size_t swap_pairs[4][2];
        size_t swap_pair_count;
        size_t swap_pair;
        int setup_swap_attributed = 0;
        size_t operand;
        size_t operand_count;
        swap_pair_count = rxvm_profile_trace_swap_pairs(
                record, swap_pairs, sizeof(swap_pairs) / sizeof(swap_pairs[0]));
        if (!(record->attribution &
              (RXVM_PROFILE_ATTR_SETUP_SWAP |
               RXVM_PROFILE_ATTR_RESTORE_SWAP))) {
            for (swap_pair = swap_pair_count; swap_pair > 0; swap_pair--) {
                size_t first = swap_pairs[swap_pair - 1][0];
                size_t second = swap_pairs[swap_pair - 1][1];
                if (first >= frame->number_locals ||
                        second >= frame->number_locals ||
                        (!needed[first] && !needed[second]))
                    continue;
                unsigned char temporary = needed[first];
                setup_swaps++;
                restoration_swaps[(size_t)(setup_swaps - 1) * 2] = first;
                restoration_swaps[
                        (size_t)(setup_swaps - 1) * 2 + 1] = second;
                needed[first] = needed[second];
                needed[second] = temporary;
                setup_swap_attributed = 1;
            }
            if (setup_swap_attributed)
                record->attribution |= RXVM_PROFILE_ATTR_SETUP_SWAP;
        }
        if (i == activation->trace_count &&
                (effects.semantics & RXOP_SEM_CALL) != 0)
            continue;
        if (swap_pair_count) {
            /* The swap effects above replace the generic kill handling. */
        } else if (record->opcode == OP_COPY_REG_REG && record->pc &&
                   (record->pc + 1)->index < frame->number_locals &&
                   needed[(record->pc + 1)->index]) {
            if (!(record->attribution & RXVM_PROFILE_ATTR_ARGUMENT_COPY)) {
                record->attribution |= RXVM_PROFILE_ATTR_ARGUMENT_COPY;
                copies++;
            }
            needed[(record->pc + 1)->index] = 0;
        } else if (effects.state == RXOP_EFFECT_CLASSIFIED) {
            operand_count = rxop_format_operand_count(rxbin_opcode_format(record->opcode));
            for (operand = 0; record->pc && operand < operand_count; operand++) {
                size_t reg = (record->pc + operand + 1)->index;
                if (rxop_effect_kills_operand(&effects, operand) &&
                        rxop_format_operand_type(rxbin_opcode_format(record->opcode), operand) == OP_REG &&
                        reg < frame->number_locals)
                    needed[reg] = 0;
            }
        } else {
            break;
        }
        if (effects.flow != FLOW_NEXT ||
                (effects.semantics & (RXOP_SEM_CALL | RXOP_SEM_RETURN)) != 0)
            break;
    }
    rxvm_profile_add_total(state, &state->setup_swaps, setup_swaps);
    rxvm_profile_add_total(state, &state->defensive_argument_copies, copies);
    if (row_index < state->call_row_count) {
        rxvm_profile_add_total(state,
                &state->call_rows[row_index].setup_swaps, setup_swaps);
        rxvm_profile_add_total(state,
                &state->call_rows[row_index].defensive_argument_copies, copies);
    }
    rxvm_profile_prepare_restoration(state, activation, row_index, setup_swaps,
                                     restoration_swaps, frame->number_locals,
                                     native_call);
    free(needed);
    free(restoration_swaps);
}

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
                                 int has_argument_window) {
    rxvm_profile_census_callable_kind kind;
    rxvm_profile_activation *activation;
    size_t row_index;
    uint64_t exact_arity = arity >= 0 ? (uint64_t)arity : 0;
    int arity_valid = arity >= 0;
    if (!state || !state->enabled) return;
    if (path < 0 || path >= RXVM_PROFILE_CALL_PATH_COUNT ||
            outcome < 0 || outcome >= RXVM_PROFILE_CALL_OUTCOME_COUNT) {
        rxvm_profile_increment(state, &state->invalid_events);
        state->census_tracking_unavailable = 1;
        return;
    }
    if (disposition == RXVM_PROFILE_FRAME_LAST_ACTIVATION) {
        if (state->last_frame_disposition_valid) {
            disposition = state->last_frame_disposition;
            state->last_frame_disposition_valid = 0;
        } else {
            disposition = RXVM_PROFILE_FRAME_NONE_FAILED;
            state->census_tracking_unavailable = 1;
        }
    }
    if (disposition < 0 || disposition >= RXVM_PROFILE_FRAME_DISPOSITION_COUNT)
        disposition = RXVM_PROFILE_FRAME_NONE_FAILED;
    if (!arity_valid) {
        outcome = RXVM_PROFILE_CALL_INVALID;
        state->census_tracking_unavailable = 1;
    }
    kind = rxvm_profile_census_kind(state, procedure_id, path);
    row_index = rxvm_profile_find_or_add_call_row(
            state, path, kind, disposition, outcome, procedure_id,
            exact_arity, arity_valid, site_module, site_index);
    rxvm_profile_increment(state, &state->call_path_totals[path]);
    rxvm_profile_increment(state, &state->callable_kind_totals[kind]);
    rxvm_profile_increment(state, &state->frame_disposition_totals[disposition]);
    rxvm_profile_increment(state, &state->call_outcome_totals[outcome]);
    if (row_index != SIZE_MAX)
        rxvm_profile_increment(state, &state->call_rows[row_index].count);
    if (outcome != RXVM_PROFILE_CALL_SUCCESS || !has_argument_window ||
            !arity_valid || !exact_arity || row_index == SIZE_MAX)
        return;
    activation = rxvm_profile_find_activation(state, caller_frame);
    rxvm_profile_attribute_call_window(
            state, activation, (const stack_frame *)caller_frame,
            argument_base, exact_arity, row_index,
            disposition == RXVM_PROFILE_FRAME_NO_CHILD_NATIVE);
}

void rxvm_profile_record_return_at(rxvm_profile_state *state,
                                   rxvm_profile_return_placement placement) {
    if (!state || !state->enabled) return;
    if (placement < 0 || placement >= RXVM_PROFILE_RETURN_PLACEMENT_COUNT) {
        rxvm_profile_increment(state, &state->invalid_events);
        state->census_tracking_unavailable = 1;
        return;
    }
    rxvm_profile_increment(state, &state->return_placements[placement]);
}

void rxvm_profile_record_dynamic_at(rxvm_profile_state *state,
                                    rxvm_profile_dynamic_kind kind,
                                    rxvm_profile_dynamic_outcome outcome) {
    if (!state || !state->enabled) return;
    if (kind < 0 || kind >= RXVM_PROFILE_DYNAMIC_KIND_COUNT ||
            outcome < 0 || outcome >= RXVM_PROFILE_DYNAMIC_OUTCOME_COUNT) {
        rxvm_profile_increment(state, &state->invalid_events);
        state->census_tracking_unavailable = 1;
        return;
    }
    rxvm_profile_increment(state, &state->dynamic_resolution[kind][outcome]);
}

void rxvm_profile_record_swap_at(rxvm_profile_state *state,
                                 const void *frame,
                                 size_t register_1,
                                 size_t register_2) {
    rxvm_profile_activation *activation;
    rxvm_profile_trace_record *record;
    size_t *replacement;
    size_t temporary;
    size_t i;
    int restored = 1;
    if (!state || !state->enabled) return;
    rxvm_profile_increment(state, &state->swap_operations);
    activation = rxvm_profile_find_activation(state, frame);
    if (!activation || !activation->restoration_pending ||
            !activation->restoration_ready)
        return;
    if (register_1 >= activation->restoration_mapping_count ||
            register_2 >= activation->restoration_mapping_count) {
        state->census_tracking_unavailable = 1;
        rxvm_profile_increment(state, &state->attribution_degraded);
        rxvm_profile_abandon_restoration(activation);
        return;
    }
    if (!activation->trace_count) {
        state->census_tracking_unavailable = 1;
        rxvm_profile_increment(state, &state->attribution_degraded);
        rxvm_profile_abandon_restoration(activation);
        return;
    }
    if (activation->restoration_trace_count ==
            activation->restoration_trace_capacity) {
        size_t capacity;
        if (activation->restoration_trace_capacity > SIZE_MAX / 2) {
            state->census_tracking_unavailable = 1;
            rxvm_profile_increment(state, &state->attribution_degraded);
            rxvm_profile_abandon_restoration(activation);
            return;
        }
        capacity = activation->restoration_trace_capacity
                ? activation->restoration_trace_capacity * 2 : 8;
        replacement = (size_t *)realloc(
                activation->restoration_trace_indices,
                capacity * sizeof(*replacement));
        if (!replacement) {
            state->census_tracking_unavailable = 1;
            rxvm_profile_increment(state, &state->attribution_degraded);
            rxvm_profile_abandon_restoration(activation);
            return;
        }
        activation->restoration_trace_indices = replacement;
        activation->restoration_trace_capacity = capacity;
    }
    activation->restoration_trace_indices[
            activation->restoration_trace_count++] =
            activation->trace_count - 1;
    temporary = activation->restoration_mapping[register_1];
    activation->restoration_mapping[register_1] =
            activation->restoration_mapping[register_2];
    activation->restoration_mapping[register_2] = temporary;
    for (i = 0; i < activation->restoration_mapping_count; i++) {
        if (activation->restoration_mapping[i] != i) {
            restored = 0;
            break;
        }
    }
    if (!restored) return;
    rxvm_profile_add_total(state, &state->normal_restoration_swaps,
                           activation->restoration_trace_count);
    if (activation->restoration_call_row < state->call_row_count)
        rxvm_profile_add_total(
                state,
                &state->call_rows[activation->restoration_call_row]
                     .normal_restoration_swaps,
                activation->restoration_trace_count);
    for (i = 0; i < activation->restoration_trace_count; i++) {
        size_t trace_index = activation->restoration_trace_indices[i];
        if (trace_index < activation->trace_count) {
            record = &activation->trace[trace_index];
            record->attribution |= RXVM_PROFILE_ATTR_RESTORE_SWAP;
        }
    }
    rxvm_profile_abandon_restoration(activation);
}

void rxvm_profile_record_signal_unwind_at(rxvm_profile_state *state,
                                          uint64_t frames_discarded,
                                          uint64_t windows_restored,
                                          uint64_t slots_restored,
                                          int restoration_failed) {
    if (!state || !state->enabled) return;
    rxvm_profile_increment(state, &state->signal_unwind_events);
    rxvm_profile_add_total(state, &state->signal_bytecode_frames_discarded,
                           frames_discarded);
    rxvm_profile_add_total(state, &state->signal_argument_windows_restored,
                           windows_restored);
    rxvm_profile_add_total(state, &state->signal_argument_slots_restored,
                           slots_restored);
    if (restoration_failed) {
        rxvm_profile_increment(state, &state->signal_restoration_failures);
        state->census_tracking_unavailable = 1;
    }
    if (frames_discarded && state->activation_count) {
        rxvm_profile_activation *target =
                &state->activations[state->activation_count - 1];
        rxvm_profile_abandon_restoration(target);
    }
}

void rxvm_profile_record_signal_native_restore_at(rxvm_profile_state *state,
                                                  int window_observed,
                                                  uint64_t slots_restored,
                                                  int restoration_failed) {
    if (!state || !state->enabled) return;
    if (window_observed)
        rxvm_profile_increment(state, &state->signal_native_windows_restored);
    rxvm_profile_add_total(state, &state->signal_native_slots_restored,
                           slots_restored);
    if (restoration_failed) {
        rxvm_profile_increment(state, &state->signal_restoration_failures);
        state->census_tracking_unavailable = 1;
    }
    if (window_observed && state->activation_count) {
        rxvm_profile_activation *activation =
                &state->activations[state->activation_count - 1];
        rxvm_profile_abandon_restoration(activation);
    }
}

static char *rxvm_profile_copy_string(const char *source, size_t length) {
    char *copy;
    if (!source) return 0;
    copy = (char *)malloc(length + 1);
    if (!copy) return 0;
    memcpy(copy, source, length);
    copy[length] = 0;
    return copy;
}

static string_constant *rxvm_profile_string_constant(module *mod,
                                                      size_t offset) {
    string_constant *constant;
    if (!mod || offset >= mod->segment.const_size) return 0;
    constant = (string_constant *)(mod->segment.const_pool + offset);
    return constant->base.type == STRING_CONST ? constant : 0;
}

static const char *rxvm_profile_module_label(const char *module_name) {
    const char *slash;
    const char *backslash;
    if (!module_name) return "<module>";
    slash = strrchr(module_name, '/');
    backslash = strrchr(module_name, '\\');
    if (!slash || (backslash && backslash > slash)) slash = backslash;
    return slash ? slash + 1 : module_name;
}

static size_t rxvm_profile_find_procedure(const rxvm_profile_state *state,
                                          const char *name,
                                          const char *return_type,
                                          const char *args) {
    size_t i;
    for (i = 0; i < state->procedure_count; i++) {
        const rxvm_profile_procedure *procedure = &state->procedures[i];
        if (strcmp(procedure->name, name) == 0 &&
                strcmp(procedure->return_type, return_type) == 0 &&
                strcmp(procedure->args, args) == 0) return i;
    }
    return SIZE_MAX;
}

static size_t rxvm_profile_add_procedure(rxvm_profile_state *state,
                                         const char *module_name,
                                         const char *name,
                                         const char *return_type,
                                         const char *args,
                                         int native,
                                         rxvm_profile_callable_kind kind) {
    rxvm_profile_procedure *procedure;
    size_t found = rxvm_profile_find_procedure(state, name, return_type, args);
    if (found != SIZE_MAX) {
        if (native) state->procedures[found].native = 1;
        if (kind > state->procedures[found].kind)
            state->procedures[found].kind = kind;
        return found;
    }
    if (state->procedure_count == state->procedure_capacity) {
        size_t new_capacity = state->procedure_capacity
                ? state->procedure_capacity * 2 : 64;
        rxvm_profile_procedure *new_procedures =
                (rxvm_profile_procedure *)realloc(
                        state->procedures,
                        new_capacity * sizeof(rxvm_profile_procedure));
        if (!new_procedures) return SIZE_MAX;
        state->procedures = new_procedures;
        state->procedure_capacity = new_capacity;
    }
    procedure = &state->procedures[state->procedure_count];
    memset(procedure, 0, sizeof(*procedure));
    procedure->module_name = rxvm_profile_copy_string(
            module_name ? module_name : "", strlen(module_name ? module_name : ""));
    procedure->name = rxvm_profile_copy_string(name, strlen(name));
    procedure->return_type = rxvm_profile_copy_string(return_type,
                                                       strlen(return_type));
    procedure->args = rxvm_profile_copy_string(args, strlen(args));
    if (!procedure->module_name || !procedure->name ||
            !procedure->return_type || !procedure->args) {
        free(procedure->module_name);
        free(procedure->name);
        free(procedure->return_type);
        free(procedure->args);
        memset(procedure, 0, sizeof(*procedure));
        return SIZE_MAX;
    }
    procedure->native = native;
    procedure->kind = kind;
    return state->procedure_count++;
}

#ifdef CREXX_VM_PROFILING
static rxvm_profile_callable_kind rxvm_profile_symbol_kind(
        const struct rxvm_context *context, const char *symbol) {
    size_t module_index;
    size_t symbol_length = strlen(symbol);
    if (strstr(symbol, ".§factory") != 0) return RXVM_PROFILE_FACTORY;
    for (module_index = 0; module_index < context->num_modules; module_index++) {
        module *mod = context->modules[module_index];
        int meta_index = mod ? mod->meta_head : -1;
        while (meta_index != -1) {
            meta_entry *meta = (meta_entry *)(mod->segment.const_pool + meta_index);
            if (meta->base.type == META_CLASS) {
                meta_class_constant *class_meta = (meta_class_constant *)meta;
                string_constant *class_name = rxvm_profile_string_constant(
                        mod, class_meta->symbol);
                if (class_name && symbol_length > class_name->string_len &&
                        strncmp(symbol, class_name->string,
                                class_name->string_len) == 0 &&
                        symbol[class_name->string_len] == '.') {
                    return RXVM_PROFILE_METHOD;
                }
            }
            meta_index = meta->next;
        }
    }
    return RXVM_PROFILE_PROCEDURE;
}
#endif

void rxvm_profile_refresh_catalog(rxvm_profile_state *state,
                                  struct rxvm_context *context) {
#ifdef CREXX_VM_PROFILING
    size_t module_index;
    if (!state || !state->enabled || !context) return;

    for (module_index = 0; module_index < context->num_modules; module_index++) {
        module *mod = context->modules[module_index];
        int meta_index;
        size_t procedure_index;
        if (!mod) continue;
        for (procedure_index = 0; procedure_index < mod->procedure_count;
             procedure_index++) {
            mod->procedures[procedure_index].profile_id = SIZE_MAX;
        }

        meta_index = mod->meta_head;
        while (meta_index != -1) {
            meta_entry *meta = (meta_entry *)(mod->segment.const_pool + meta_index);
            if (meta->base.type == META_FUNC) {
                meta_func_constant *func = (meta_func_constant *)meta;
                proc_runtime *runtime = rxvm_get_module_runtime_procedure(mod,
                                                                          func->func);
                string_constant *symbol = rxvm_profile_string_constant(mod,
                                                                        func->symbol);
                string_constant *return_type = rxvm_profile_string_constant(
                        mod, func->type);
                string_constant *args = rxvm_profile_string_constant(mod,
                                                                      func->args);
                if (runtime && symbol && return_type && args) {
                    char *name_copy = rxvm_profile_copy_string(symbol->string,
                                                               symbol->string_len);
                    char *type_copy = rxvm_profile_copy_string(return_type->string,
                                                               return_type->string_len);
                    char *args_copy = rxvm_profile_copy_string(args->string,
                                                               args->string_len);
                    size_t profile_id = SIZE_MAX;
                    if (name_copy && type_copy && args_copy) {
                        const char *runtime_module = mod->name;
                        if (runtime->binarySpace && runtime->binarySpace->module)
                            runtime_module = runtime->binarySpace->module->name;
                        profile_id = rxvm_profile_add_procedure(
                                state, runtime_module, name_copy, type_copy,
                                args_copy, runtime->binarySpace == 0,
                                rxvm_profile_symbol_kind(context, name_copy));
                    }
                    free(name_copy);
                    free(type_copy);
                    free(args_copy);
                    if (profile_id != SIZE_MAX) runtime->profile_id = profile_id;
                    else state->procedure_tracking_unavailable = 1;
                }
            }
            meta_index = meta->next;
        }

        for (procedure_index = 0; procedure_index < mod->procedure_count;
             procedure_index++) {
            proc_runtime *runtime = &mod->procedures[procedure_index];
            if (runtime->profile_id == SIZE_MAX) {
                char fallback[512];
                size_t profile_id;
                snprintf(fallback, sizeof(fallback), "%s::%s",
                         rxvm_profile_module_label(mod->name),
                         runtime->name ? runtime->name : "<procedure>");
                profile_id = rxvm_profile_add_procedure(
                        state, mod->name, fallback, "", "",
                        runtime->binarySpace == 0, RXVM_PROFILE_PROCEDURE);
                if (profile_id == SIZE_MAX)
                    state->procedure_tracking_unavailable = 1;
                else
                    runtime->profile_id = profile_id;
            }
        }
    }
#else
    (void)state;
    (void)context;
#endif
}

void rxvm_profile_begin(rxvm_profile_state *state, int enabled,
                        struct rxvm_context *context) {
    uint64_t minimum = UINT64_MAX;
    int i;

    memset(state, 0, sizeof(*state));
    state->enabled = enabled != 0;
    state->timing_enabled = context && context->profile_mode == 1;
    state->current_transition = RXVM_TRANSITION_SEQUENTIAL;
    state->instruction_activation_index = SIZE_MAX;
    state->native_procedure_id = SIZE_MAX;
    state->context = context;
    if (!state->enabled) return;

    state->activations = (rxvm_profile_activation *)calloc(
            64, sizeof(rxvm_profile_activation));
    if (state->activations) state->activation_capacity = 64;
    else state->procedure_tracking_unavailable = 1;
    rxvm_profile_refresh_catalog(state, context);

    if (state->timing_enabled) {
        for (i = 0; i < 1000; i++) {
            uint64_t start = rxvm_profile_now_ns();
            uint64_t end = rxvm_profile_now_ns();
            uint64_t elapsed = rxvm_profile_elapsed(start, end);
            if (!elapsed) {
                state->timer_zero_deltas++;
            } else if (elapsed < minimum) {
                minimum = elapsed;
            }
        }
    }
    state->timer_read_min_ns = minimum == UINT64_MAX ? 0 : minimum;
    state->previous_allocation_profile = rxvm_active_allocation_profile;
    rxvm_active_allocation_profile = state;
}

void rxvm_profile_destroy(rxvm_profile_state *state) {
    size_t i;
    if (!state) return;
    for (i = 0; i < state->procedure_count; i++) {
        free(state->procedures[i].module_name);
        free(state->procedures[i].name);
        free(state->procedures[i].return_type);
        free(state->procedures[i].args);
    }
    free(state->procedures);
    for (i = 0; i < state->activation_count; i++) {
        free(state->activations[i].trace);
        free(state->activations[i].restoration_mapping);
        free(state->activations[i].restoration_trace_indices);
    }
    free(state->activations);
    free(state->call_rows);
    free(state->branch_rows);
    state->procedures = 0;
    state->activations = 0;
    state->call_rows = 0;
    state->branch_rows = 0;
    state->procedure_count = 0;
    state->procedure_capacity = 0;
    state->activation_count = 0;
    state->activation_capacity = 0;
    state->call_row_count = 0;
    state->call_row_capacity = 0;
    state->branch_row_count = 0;
    state->branch_row_capacity = 0;
    if (rxvm_active_allocation_profile == state)
        rxvm_active_allocation_profile = state->previous_allocation_profile;
    state->previous_allocation_profile = 0;
}

static uint64_t rxvm_profile_average(const rxvm_profile_counter *counter) {
    return counter->count ? counter->total_ns / counter->count : 0;
}

static uint64_t rxvm_profile_total_instruction_ns(const rxvm_profile_state *state) {
    uint64_t total = 0;
    int i;
    for (i = 0; i < OP_MAX_INSTRUCTIONS; i++) {
        if (UINT64_MAX - total < state->instructions[i].total_ns) return UINT64_MAX;
        total += state->instructions[i].total_ns;
    }
    return total;
}

static uint64_t rxvm_profile_total_instruction_count(const rxvm_profile_state *state) {
    uint64_t total = 0;
    int i;
    for (i = 0; i < OP_MAX_INSTRUCTIONS; i++) {
        if (UINT64_MAX - total < state->instructions[i].count) return UINT64_MAX;
        total += state->instructions[i].count;
    }
    return total;
}

static uint64_t rxvm_profile_total_transition_ns(const rxvm_profile_state *state) {
    uint64_t total = 0;
    int i;
    for (i = 0; i < RXVM_TRANSITION_COUNT; i++) {
        if (UINT64_MAX - total < state->transitions[i].total_ns) return UINT64_MAX;
        total += state->transitions[i].total_ns;
    }
    return total;
}

static double rxvm_profile_percent(uint64_t part, uint64_t total) {
    if (!total) return 0.0;
    return (double)((long double)part * 100.0L / (long double)total);
}

static int rxvm_profile_csv_path(const char *path) {
    size_t length;
    if (!path) return 0;
    length = strlen(path);
    if (length < 4) return 0;
    return path[length - 4] == '.' &&
            tolower((unsigned char)path[length - 3]) == 'c' &&
            tolower((unsigned char)path[length - 2]) == 's' &&
            tolower((unsigned char)path[length - 1]) == 'v';
}

static const char *rxvm_profile_signal_name(unsigned char signal,
                                            rxvm_profile_signal_name_fn name_fn,
                                            char *buffer,
                                            size_t buffer_size) {
    const char *name = name_fn ? name_fn(signal) : 0;
    if (name) return name;
    snprintf(buffer, buffer_size, "SIGNAL_%u", (unsigned int)signal);
    return buffer;
}

static void rxvm_profile_sort_instruction_indices(const rxvm_profile_state *state,
                                                  int *indices,
                                                  int *used) {
    int count = 0;
    int i;
    for (i = 0; i < OP_MAX_INSTRUCTIONS; i++) {
        int position;
        if (!state->instructions[i].count) continue;
        position = count;
        while (position > 0) {
            int previous = indices[position - 1];
            if (state->instructions[previous].total_ns >
                    state->instructions[i].total_ns) break;
            if (state->instructions[previous].total_ns ==
                    state->instructions[i].total_ns && previous < i) break;
            indices[position] = previous;
            position--;
        }
        indices[position] = i;
        count++;
    }
    *used = count;
}

static uint64_t rxvm_profile_procedure_sort_total(
        const rxvm_profile_procedure *procedure) {
    return procedure->native ? procedure->native_total.total_ns
                             : procedure->elapsed.total_ns;
}

static const char *rxvm_profile_callable_kind_name(
        const rxvm_profile_procedure *procedure) {
    if (procedure->native) return "native";
    if (procedure->kind == RXVM_PROFILE_FACTORY) return "factory";
    if (procedure->kind == RXVM_PROFILE_METHOD) return "method";
    return "procedure";
}

static size_t *rxvm_profile_sorted_procedure_indices(
        const rxvm_profile_state *state, size_t *used) {
    size_t *indices;
    size_t count = 0;
    size_t i;
    *used = 0;
    if (!state->procedure_count) return 0;
    indices = (size_t *)malloc(state->procedure_count * sizeof(size_t));
    if (!indices) return 0;
    for (i = 0; i < state->procedure_count; i++) {
        size_t position;
        uint64_t total;
        if (!state->procedures[i].calls) continue;
        total = rxvm_profile_procedure_sort_total(&state->procedures[i]);
        position = count;
        while (position > 0) {
            size_t previous = indices[position - 1];
            uint64_t previous_total = rxvm_profile_procedure_sort_total(
                    &state->procedures[previous]);
            if (previous_total > total) break;
            if (previous_total == total &&
                    strcmp(state->procedures[previous].name,
                           state->procedures[i].name) < 0) break;
            indices[position] = previous;
            position--;
        }
        indices[position] = i;
        count++;
    }
    *used = count;
    return indices;
}

static void rxvm_profile_csv_string(FILE *out, const char *value) {
    const char *cursor = value ? value : "";
    fputc('"', out);
    while (*cursor) {
        if (*cursor == '"') fputc('"', out);
        fputc(*cursor++, out);
    }
    fputc('"', out);
}

static void rxvm_profile_write_procedure_csv_row(
        FILE *out, const rxvm_profile_procedure *procedure,
        const char *metric, const rxvm_profile_counter *counter) {
    fprintf(out, "procedure,");
    rxvm_profile_csv_string(out, procedure->name);
    fputc(',', out);
    rxvm_profile_csv_string(out, metric);
    fprintf(out, ",,%" PRIu64 ",%" PRIu64 ",%" PRIu64
                 ",%" PRIu64 ",%" PRIu64 ",0,,,,,",
            counter->count, counter->total_ns,
            rxvm_profile_average(counter), counter->min_ns, counter->max_ns);
    rxvm_profile_csv_string(out, procedure->module_name);
    fprintf(out, ",%s,%" PRIu64 ",%" PRIu64 ",",
            rxvm_profile_callable_kind_name(procedure),
            procedure->completed, procedure->unwound);
    rxvm_profile_csv_string(out, procedure->return_type);
    fputc(',', out);
    rxvm_profile_csv_string(out, procedure->args);
    fprintf(out, ",,,,\n");
}

static const char *rxvm_profile_counter_status(const rxvm_profile_state *state) {
    return state->overflowed ? "overflowed" : "complete";
}

static void rxvm_profile_write_allocation_csv_row(
        FILE *out, const char *name, uint64_t count, uint64_t bytes,
        uint64_t max_bytes, uint64_t high_water, const char *status) {
    int i;
    fprintf(out, "allocation,%s,,,%" PRIu64 ",0,0,0,0,0",
            name, count);
    for (i = 0; i < 10; i++) fputc(',', out);
    fprintf(out, ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%s\n",
            bytes, max_bytes, high_water, status);
}

static const char *rxvm_profile_census_status(
        const rxvm_profile_state *state) {
    return state->overflowed || state->census_tracking_unavailable
            ? "degraded" : "complete";
}

static void rxvm_profile_write_named_count_csv_row(
        FILE *out, const char *section, const char *name, const char *value,
        uint64_t count, const char *status) {
    fprintf(out, "%s,", section);
    rxvm_profile_csv_string(out, name);
    fputc(',', out);
    rxvm_profile_csv_string(out, value);
    fprintf(out, ",,%" PRIu64 ",0,0,0,0,0", count);
    fprintf(out, ",,,,,,,,,,,,,,");
    rxvm_profile_csv_string(out, status);
    fputc('\n', out);
}

static const char *rxvm_profile_module_name(
        const rxvm_profile_state *state, size_t module_id) {
    if (state->context && module_id > 0 &&
            module_id <= state->context->num_modules &&
            state->context->modules[module_id - 1] &&
            state->context->modules[module_id - 1]->name)
        return state->context->modules[module_id - 1]->name;
    return "";
}

static void rxvm_profile_write_status_csv_row(
        FILE *out, const char *domain, const char *status,
        uint64_t degraded_events) {
    fprintf(out, "status,");
    rxvm_profile_csv_string(out, domain);
    fputc(',', out);
    rxvm_profile_csv_string(out, status);
    fprintf(out, ",,%" PRIu64 ",0,0,0,0,0,,,,,,,,,,,,,,", degraded_events);
    rxvm_profile_csv_string(out, status);
    fputc('\n', out);
}

static void rxvm_profile_write_value_csv_row(
        FILE *out, const char *operation, const char *shape,
        const rxvm_profile_allocation_counter *counter,
        const char *status) {
    int i;
    fprintf(out, "value_operation,");
    rxvm_profile_csv_string(out, operation);
    fputc(',', out);
    rxvm_profile_csv_string(out, shape);
    fprintf(out, ",,%" PRIu64 ",0,0,0,0,0", counter->count);
    for (i = 0; i < 10; i++) fputc(',', out);
    fprintf(out, ",%" PRIu64 ",%" PRIu64 ",,",
            counter->bytes, counter->max_bytes);
    rxvm_profile_csv_string(out, status);
    fputc('\n', out);
}

static void rxvm_profile_write_frame_phase_csv_row(
        FILE *out, const char *phase, const char *source,
        const rxvm_profile_counter *counter, uint64_t units,
        const char *status) {
    fprintf(out, "frame_entry,");
    rxvm_profile_csv_string(out, phase);
    fputc(',', out);
    rxvm_profile_csv_string(out, source);
    fprintf(out, ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64
                 ",%" PRIu64 ",%" PRIu64 ",0,,,,,,,,,,,,,,",
            units, counter->count, counter->total_ns,
            rxvm_profile_average(counter), counter->min_ns,
            counter->max_ns);
    rxvm_profile_csv_string(out, status);
    fputc('\n', out);
}

static void rxvm_profile_write_branch_csv_row(
        FILE *out, const rxvm_profile_state *state,
        const rxvm_profile_branch_row *row,
        const Instruction *instruction_map, const char *status) {
    fprintf(out, "branch,");
    rxvm_profile_csv_string(out, instruction_map[row->opcode].instruction);
    fprintf(out, ",,%zu,%" PRIu64 ",0,0,0,0,0,%" PRIu64
                 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",",
            row->instruction_index, row->executions, row->taken,
            row->fallthrough, row->backward, row->cross_module);
    rxvm_profile_csv_string(out, rxvm_profile_module_name(state, row->module_id));
    fprintf(out, ",branch_site,,,,,,,,");
    rxvm_profile_csv_string(out, status);
    fputc('\n', out);
}

static const char *rxvm_profile_call_target(
        const rxvm_profile_state *state,
        const rxvm_profile_call_row *row) {
    if (row->procedure_id < state->procedure_count)
        return state->procedures[row->procedure_id].name;
    return "<unresolved>";
}

static const char *rxvm_profile_call_site_module(
        const rxvm_profile_state *state,
        const rxvm_profile_call_row *row) {
    if (state->context && row->site_module > 0 &&
            row->site_module <= state->context->num_modules &&
            state->context->modules[row->site_module - 1] &&
            state->context->modules[row->site_module - 1]->name)
        return state->context->modules[row->site_module - 1]->name;
    return "";
}

static void rxvm_profile_write_call_csv_row(
        FILE *out, const rxvm_profile_state *state,
        const rxvm_profile_call_row *row) {
    fprintf(out, "call,");
    rxvm_profile_csv_string(out, rxvm_profile_call_target(state, row));
    fputc(',', out);
    rxvm_profile_csv_string(out, rxvm_profile_call_path_names[row->path]);
    fprintf(out, ",%zu,%" PRIu64 ",0,0,0,0,0,",
            row->site_index, row->count);
    if (row->arity_valid) fprintf(out, "%" PRIu64, row->arity);
    fprintf(out, ",,,,");
    rxvm_profile_csv_string(out, rxvm_profile_call_site_module(state, row));
    fputc(',', out);
    rxvm_profile_csv_string(
            out, rxvm_profile_census_kind_names[row->callable_kind]);
    fprintf(out, ",,,");
    rxvm_profile_csv_string(
            out, rxvm_profile_frame_disposition_names[row->frame_disposition]);
    fputc(',', out);
    rxvm_profile_csv_string(out,
            rxvm_profile_call_outcome_names[row->outcome]);
    fprintf(out, ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",",
            row->setup_swaps, row->normal_restoration_swaps,
            row->defensive_argument_copies);
    rxvm_profile_csv_string(out, rxvm_profile_census_status(state));
    fputc('\n', out);
}

static void rxvm_profile_write_csv(FILE *out,
                                   const rxvm_profile_state *state,
                                   const char *vm_mode,
                                   int result,
                                   const Instruction *instruction_map,
                                   rxvm_profile_signal_name_fn signal_name) {
    uint64_t instruction_total = rxvm_profile_total_instruction_ns(state);
    uint64_t transition_total = rxvm_profile_total_transition_ns(state);
    int indices[OP_MAX_INSTRUCTIONS];
    int used = 0;
    int position;
    int i;

    fprintf(out, "section,name,value,id,count,total_ns,average_ns,min_ns,max_ns,percent,selected,entries,resumes,terminals,module,kind,completed,unwound,return_type,args,bytes,max_bytes,high_water,status\n");
    fprintf(out, "summary,schema_version,5,,0,0,0,0,0,0,,,,,,,,,,,,,,\n");
    fprintf(out, "summary,vm_mode,%s,,0,0,0,0,0,0,,,,,,,,,,,,,,\n", vm_mode);
    fprintf(out, "summary,profile_mode,%s,,0,0,0,0,0,0,,,,,,,,,,,,,,\n",
            state->timing_enabled ? "timing" : "counts");
    fprintf(out, "summary,result,%d,,0,0,0,0,0,0,,,,,,,,,,,,,,\n", result);
    fprintf(out, "summary,timer_read_min_ns,%" PRIu64 ",,1,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",0,,,,,,,,,,,,,,\n",
            state->timer_read_min_ns,
            state->timer_read_min_ns, state->timer_read_min_ns,
            state->timer_read_min_ns, state->timer_read_min_ns);
    fprintf(out, "summary,timer_zero_deltas,%" PRIu64 ",,0,0,0,0,0,0,,,,,,,,,,,,,,\n",
            state->timer_zero_deltas);
    fprintf(out, "summary,interrupt_polls,%" PRIu64 ",,0,0,0,0,0,0,,,,,,,,,,,,,,\n",
            state->interrupt_polls);
    fprintf(out, "summary,invalid_events,%" PRIu64 ",,0,0,0,0,0,0,,,,,,,,,,,,,,\n",
            state->invalid_events);
    fprintf(out, "summary,counter_overflow,%d,,0,0,0,0,0,0,,,,,,,,,,,,,,\n",
            state->overflowed);
    fprintf(out, "summary,procedure_tracking_unavailable,%d,,0,0,0,0,0,0,,,,,,,,,,,,,,\n",
            state->procedure_tracking_unavailable);
    fprintf(out, "summary,allocation_tracking_unavailable,0,,0,0,0,0,0,0,,,,,,,,,,,,,,\n");
    fprintf(out, "summary,census_tracking_unavailable,%d,,0,0,0,0,0,0,,,,,,,,,,,,,,\n",
            state->census_tracking_unavailable);
    fprintf(out, "summary,branch_tracking_unavailable,%d,,0,0,0,0,0,0,,,,,,,,,,,,,,\n",
            state->branch_tracking_unavailable);

    rxvm_profile_write_status_csv_row(
            out, "instructions", rxvm_profile_counter_status(state),
            state->overflowed ? 1 : 0);
    rxvm_profile_write_status_csv_row(
            out, "procedures",
            state->overflowed || state->procedure_tracking_unavailable
                    ? "degraded" : "complete",
            (uint64_t)(state->overflowed != 0) +
                    (uint64_t)(state->procedure_tracking_unavailable != 0));
    rxvm_profile_write_status_csv_row(
            out, "allocations",
            state->overflowed || state->active_frames
                    ? "degraded" : "complete",
            (uint64_t)(state->overflowed != 0) + state->active_frames);
    rxvm_profile_write_status_csv_row(
            out, "call_census", rxvm_profile_census_status(state),
            (uint64_t)(state->overflowed != 0) +
                    (uint64_t)(state->census_tracking_unavailable != 0));
    rxvm_profile_write_status_csv_row(
            out, "frame_entry", rxvm_profile_counter_status(state),
            state->overflowed ? 1 : 0);
    rxvm_profile_write_status_csv_row(
            out, "value_operations", rxvm_profile_counter_status(state),
            state->overflowed ? 1 : 0);
    rxvm_profile_write_status_csv_row(
            out, "branch_sites",
            state->overflowed || state->branch_tracking_unavailable
                    ? "degraded" : "complete",
            (uint64_t)(state->overflowed != 0) +
                    (uint64_t)(state->branch_tracking_unavailable != 0));

    rxvm_profile_sort_instruction_indices(state, indices, &used);
    for (position = 0; position < used; position++) {
        int opcode = indices[position];
        const rxvm_profile_counter *counter = &state->instructions[opcode];
        fprintf(out,
                "instruction,%s,,%d,%" PRIu64 ",%" PRIu64 ",%" PRIu64
                ",%" PRIu64 ",%" PRIu64 ",%.6f,,,,,,,,,,,,,,\n",
                instruction_map[opcode].instruction, opcode, counter->count,
                counter->total_ns, rxvm_profile_average(counter),
                counter->min_ns, counter->max_ns,
                rxvm_profile_percent(counter->total_ns, instruction_total));
    }

    for (i = 0; i < RXVM_TRANSITION_COUNT; i++) {
        const rxvm_profile_counter *counter = &state->transitions[i];
        if (!counter->count) continue;
        fprintf(out,
                "transition,%s,,%d,%" PRIu64 ",%" PRIu64 ",%" PRIu64
                ",%" PRIu64 ",%" PRIu64 ",%.6f,,,,,,,,,,,,,,\n",
                rxvm_profile_transition_names[i], i, counter->count,
                counter->total_ns, rxvm_profile_average(counter),
                counter->min_ns, counter->max_ns,
                rxvm_profile_percent(counter->total_ns, transition_total));
    }

    fprintf(out,
            "interrupt,scan_all,,,%" PRIu64 ",%" PRIu64 ",%" PRIu64
            ",%" PRIu64 ",%" PRIu64 ",0,,,,,,,,,,,,,,\n",
            state->interrupt_scans.count, state->interrupt_scans.total_ns,
            rxvm_profile_average(&state->interrupt_scans),
            state->interrupt_scans.min_ns, state->interrupt_scans.max_ns);
    fprintf(out,
            "interrupt,scan_without_selection,,,%" PRIu64 ",0,0,0,0,0,,,,,,,,,,,,,,\n",
            state->interrupt_scans_without_selection);
    fprintf(out,
            "interrupt,mechanics_all,,,%" PRIu64 ",%" PRIu64 ",%" PRIu64
            ",%" PRIu64 ",%" PRIu64 ",0,,,,,,,,,,,,,,\n",
            state->interrupt_mechanics.count,
            state->interrupt_mechanics.total_ns,
            rxvm_profile_average(&state->interrupt_mechanics),
            state->interrupt_mechanics.min_ns,
            state->interrupt_mechanics.max_ns);

    for (i = 1; i < RXSIGNAL_MAX; i++) {
        char fallback[24];
        const rxvm_profile_interrupt_counter *counter = &state->interrupts[i];
        if (!counter->selected && !counter->entries && !counter->resumes &&
                !counter->terminals && !counter->mechanics.count) continue;
        fprintf(out,
                "interrupt,%s,,%d,%" PRIu64 ",%" PRIu64 ",%" PRIu64
                ",%" PRIu64 ",%" PRIu64 ",0,%" PRIu64 ",%" PRIu64
                ",%" PRIu64 ",%" PRIu64 ",,,,,,,,,,\n",
                rxvm_profile_signal_name((unsigned char)i, signal_name,
                                         fallback, sizeof(fallback)),
                i, counter->mechanics.count, counter->mechanics.total_ns,
                rxvm_profile_average(&counter->mechanics),
                counter->mechanics.min_ns, counter->mechanics.max_ns,
                counter->selected, counter->entries, counter->resumes,
                counter->terminals);
    }

    {
        size_t procedure_used = 0;
        size_t *procedure_indices = rxvm_profile_sorted_procedure_indices(
                state, &procedure_used);
        size_t procedure_position;
        for (procedure_position = 0; procedure_position < procedure_used;
             procedure_position++) {
            const rxvm_profile_procedure *procedure =
                    &state->procedures[procedure_indices[procedure_position]];
            if (procedure->native) {
                rxvm_profile_write_procedure_csv_row(
                        out, procedure, "native_total", &procedure->native_total);
            } else {
                rxvm_profile_write_procedure_csv_row(
                        out, procedure, "elapsed", &procedure->elapsed);
                rxvm_profile_write_procedure_csv_row(
                        out, procedure, "inclusive_body",
                        &procedure->inclusive_body);
                rxvm_profile_write_procedure_csv_row(
                        out, procedure, "self", &procedure->self);
                rxvm_profile_write_procedure_csv_row(
                        out, procedure, "native_child",
                        &procedure->native_child);
                rxvm_profile_write_procedure_csv_row(
                        out, procedure, "entry_overhead",
                        &procedure->entry_overhead);
                rxvm_profile_write_procedure_csv_row(
                        out, procedure, "exit_overhead",
                        &procedure->exit_overhead);
            }
        }
        free(procedure_indices);
    }

    for (i = 0; i < RXVM_PROFILE_ALLOC_COUNT; i++) {
        const rxvm_profile_allocation_counter *counter = &state->allocations[i];
        rxvm_profile_write_allocation_csv_row(
                out, rxvm_profile_allocation_names[i], counter->count,
                counter->bytes, counter->max_bytes, 0,
                rxvm_profile_counter_status(state));
    }
    rxvm_profile_write_allocation_csv_row(
            out, "value_slots", state->value_slots, state->value_slot_bytes,
            state->max_value_slots_per_block * sizeof(value), 0,
            rxvm_profile_counter_status(state));
    rxvm_profile_write_allocation_csv_row(
            out, "frame_activations", state->frame_activations, 0, 0,
            state->frame_high_water,
            state->active_frames ? "degraded" : rxvm_profile_counter_status(state));
    rxvm_profile_write_allocation_csv_row(
            out, "frame_reuses", state->frame_reuses, 0, 0, 0,
            rxvm_profile_counter_status(state));

    for (i = 0; i < RXVM_PROFILE_VALUE_OPERATION_COUNT; i++) {
        int shape;
        for (shape = 0; shape < RXVM_PROFILE_VALUE_SHAPE_COUNT; shape++) {
            const rxvm_profile_allocation_counter *counter =
                    &state->value_operations[i][shape];
            if (!counter->count) continue;
            rxvm_profile_write_value_csv_row(
                    out, rxvm_profile_value_operation_names[i],
                    rxvm_profile_value_shape_names[shape], counter,
                    rxvm_profile_counter_status(state));
        }
    }
    for (i = 0; i < RXVM_PROFILE_FRAME_PHASE_COUNT; i++) {
        int source;
        for (source = 0; source < RXVM_PROFILE_FRAME_SOURCE_COUNT; source++) {
            const rxvm_profile_counter *counter =
                    &state->frame_phases[i][source];
            if (!counter->count) continue;
            rxvm_profile_write_frame_phase_csv_row(
                    out, rxvm_profile_frame_phase_names[i],
                    rxvm_profile_frame_source_names[source], counter,
                    state->frame_phase_units[i][source],
                    rxvm_profile_counter_status(state));
        }
    }
    {
        size_t branch_index;
        const char *branch_status =
                state->overflowed || state->branch_tracking_unavailable
                        ? "degraded" : "complete";
        for (branch_index = 0; branch_index < state->branch_row_count;
             branch_index++)
            rxvm_profile_write_branch_csv_row(
                    out, state, &state->branch_rows[branch_index],
                    instruction_map, branch_status);
    }

    for (i = 0; i < RXVM_PROFILE_CALL_PATH_COUNT; i++)
        rxvm_profile_write_named_count_csv_row(
                out, "census", "call_path",
                rxvm_profile_call_path_names[i],
                state->call_path_totals[i],
                rxvm_profile_census_status(state));
    {
        size_t arity_row;
        int invalid_emitted = 0;
        for (arity_row = 0; arity_row < state->call_row_count; arity_row++) {
            const rxvm_profile_call_row *row = &state->call_rows[arity_row];
            uint64_t count = 0;
            size_t earlier;
            size_t other;
            char exact_arity[32];
            if (!row->arity_valid) {
                if (invalid_emitted) continue;
                invalid_emitted = 1;
                for (other = arity_row; other < state->call_row_count; other++)
                    if (!state->call_rows[other].arity_valid)
                        count = UINT64_MAX - count <
                                        state->call_rows[other].count
                                ? UINT64_MAX
                                : count + state->call_rows[other].count;
                rxvm_profile_write_named_count_csv_row(
                        out, "census", "arity", "invalid", count,
                        rxvm_profile_census_status(state));
                continue;
            }
            for (earlier = 0; earlier < arity_row; earlier++)
                if (state->call_rows[earlier].arity_valid &&
                        state->call_rows[earlier].arity == row->arity)
                    break;
            if (earlier != arity_row) continue;
            for (other = arity_row; other < state->call_row_count; other++)
                if (state->call_rows[other].arity_valid &&
                        state->call_rows[other].arity == row->arity)
                    count = UINT64_MAX - count <
                                    state->call_rows[other].count
                            ? UINT64_MAX
                            : count + state->call_rows[other].count;
            snprintf(exact_arity, sizeof(exact_arity), "%" PRIu64,
                     row->arity);
            rxvm_profile_write_named_count_csv_row(
                    out, "census", "arity", exact_arity, count,
                    rxvm_profile_census_status(state));
        }
    }
    for (i = 0; i < RXVM_PROFILE_CENSUS_CALLABLE_KIND_COUNT; i++)
        rxvm_profile_write_named_count_csv_row(
                out, "census", "callable_kind",
                rxvm_profile_census_kind_names[i],
                state->callable_kind_totals[i],
                rxvm_profile_census_status(state));
    for (i = 0; i < RXVM_PROFILE_FRAME_DISPOSITION_COUNT; i++)
        rxvm_profile_write_named_count_csv_row(
                out, "census", "frame_disposition",
                rxvm_profile_frame_disposition_names[i],
                state->frame_disposition_totals[i],
                rxvm_profile_census_status(state));
    for (i = 0; i < RXVM_PROFILE_CALL_OUTCOME_COUNT; i++)
        rxvm_profile_write_named_count_csv_row(
                out, "census", "call_outcome",
                rxvm_profile_call_outcome_names[i],
                state->call_outcome_totals[i],
                rxvm_profile_census_status(state));
    for (i = 0; i < RXVM_PROFILE_RETURN_PLACEMENT_COUNT; i++)
        rxvm_profile_write_named_count_csv_row(
                out, "return", "placement",
                rxvm_profile_return_placement_names[i],
                state->return_placements[i],
                rxvm_profile_census_status(state));
    for (i = 0; i < RXVM_PROFILE_DYNAMIC_KIND_COUNT; i++) {
        int outcome;
        for (outcome = 0; outcome < RXVM_PROFILE_DYNAMIC_OUTCOME_COUNT;
             outcome++)
            rxvm_profile_write_named_count_csv_row(
                    out, "dynamic",
                    rxvm_profile_dynamic_kind_names[i],
                    rxvm_profile_dynamic_outcome_names[outcome],
                    state->dynamic_resolution[i][outcome],
                    rxvm_profile_census_status(state));
    }
    {
        uint64_t swap_total = state->swap_operations;
        uint64_t copy_total =
                state->instructions[OP_COPY_REG_REG].count;
        uint64_t classified_swaps = state->setup_swaps;
        uint64_t unclassified_swaps;
        uint64_t unclassified_copies;
        if (UINT64_MAX - classified_swaps < state->normal_restoration_swaps)
            classified_swaps = UINT64_MAX;
        else
            classified_swaps += state->normal_restoration_swaps;
        unclassified_swaps = classified_swaps <= swap_total
                ? swap_total - classified_swaps : 0;
        unclassified_copies =
                state->defensive_argument_copies <= copy_total
                ? copy_total - state->defensive_argument_copies : 0;
        rxvm_profile_write_named_count_csv_row(
                out, "mechanics", "call_window", "setup_swaps",
                state->setup_swaps, rxvm_profile_census_status(state));
        rxvm_profile_write_named_count_csv_row(
                out, "mechanics", "call_window", "normal_restoration_swaps",
                state->normal_restoration_swaps,
                rxvm_profile_census_status(state));
        rxvm_profile_write_named_count_csv_row(
                out, "mechanics", "call_window", "unclassified_swaps",
                unclassified_swaps, rxvm_profile_census_status(state));
        rxvm_profile_write_named_count_csv_row(
                out, "mechanics", "call_window", "defensive_argument_copies",
                state->defensive_argument_copies,
                rxvm_profile_census_status(state));
        rxvm_profile_write_named_count_csv_row(
                out, "mechanics", "call_window", "unclassified_copies",
                unclassified_copies, rxvm_profile_census_status(state));
        rxvm_profile_write_named_count_csv_row(
                out, "mechanics", "call_window", "attribution_degraded",
                state->attribution_degraded,
                rxvm_profile_census_status(state));
    }
    rxvm_profile_write_named_count_csv_row(
            out, "unwind", "signal", "events",
            state->signal_unwind_events, rxvm_profile_census_status(state));
    rxvm_profile_write_named_count_csv_row(
            out, "unwind", "signal", "bytecode_frames_discarded",
            state->signal_bytecode_frames_discarded,
            rxvm_profile_census_status(state));
    rxvm_profile_write_named_count_csv_row(
            out, "unwind", "signal", "argument_windows_restored",
            state->signal_argument_windows_restored,
            rxvm_profile_census_status(state));
    rxvm_profile_write_named_count_csv_row(
            out, "unwind", "signal", "argument_slots_restored",
            state->signal_argument_slots_restored,
            rxvm_profile_census_status(state));
    rxvm_profile_write_named_count_csv_row(
            out, "unwind", "signal", "interrupted_native_windows_restored",
            state->signal_native_windows_restored,
            rxvm_profile_census_status(state));
    rxvm_profile_write_named_count_csv_row(
            out, "unwind", "signal", "interrupted_native_slots_restored",
            state->signal_native_slots_restored,
            rxvm_profile_census_status(state));
    rxvm_profile_write_named_count_csv_row(
            out, "unwind", "signal", "restoration_failures",
            state->signal_restoration_failures,
            rxvm_profile_census_status(state));
    {
        size_t call_index;
        for (call_index = 0; call_index < state->call_row_count; call_index++)
            rxvm_profile_write_call_csv_row(
                    out, state, &state->call_rows[call_index]);
    }
}

static void rxvm_profile_write_table(FILE *out,
                                     const rxvm_profile_state *state,
                                     const char *vm_mode,
                                     int result,
                                     const Instruction *instruction_map,
                                     rxvm_profile_signal_name_fn signal_name) {
    uint64_t instruction_total = rxvm_profile_total_instruction_ns(state);
    uint64_t transition_total = rxvm_profile_total_transition_ns(state);
    int indices[OP_MAX_INSTRUCTIONS];
    int used = 0;
    int position;
    int i;

    fprintf(out, "\nVM PROFILE (%s) result=%d\n", vm_mode, result);
    fprintf(out,
            "Mode: %s; clock: monotonic wall time; raw instrumented timings; minimum positive adjacent timer read=%" PRIu64
            " ns; zero calibration deltas=%" PRIu64 "/1000\n",
            state->timing_enabled ? "timing" : "counts (timing fields zero)",
            state->timer_read_min_ns, state->timer_zero_deltas);
    fprintf(out,
            "Hot-loop interrupt polls=%" PRIu64 "; invalid events=%" PRIu64
            "; counter overflow=%s; procedure tracking=%s; allocation tracking=%s; census tracking=%s\n",
            state->interrupt_polls, state->invalid_events,
            state->overflowed ? "yes" : "no",
            state->procedure_tracking_unavailable ? "degraded" : "complete",
            state->active_frames ? "degraded" : "complete",
            rxvm_profile_census_status(state));

    fprintf(out, "\nInstructions (entry to retire/terminal)\n");
    fprintf(out, "%-30s %7s %14s %14s %12s %12s %8s\n",
            "opcode", "count", "total ns", "average ns", "min ns", "max ns", "% time");
    rxvm_profile_sort_instruction_indices(state, indices, &used);
    for (position = 0; position < used; position++) {
        int opcode = indices[position];
        const rxvm_profile_counter *counter = &state->instructions[opcode];
        fprintf(out,
                "%-30s %7" PRIu64 " %14" PRIu64 " %14" PRIu64
                " %12" PRIu64 " %12" PRIu64 " %7.2f%%\n",
                instruction_map[opcode].instruction, counter->count,
                counter->total_ns, rxvm_profile_average(counter),
                counter->min_ns, counter->max_ns,
                rxvm_profile_percent(counter->total_ns, instruction_total));
    }

    fprintf(out, "\nTransitions (retire to next entry; interrupt rows can overlap sub-phase rows below)\n");
    fprintf(out, "%-30s %7s %14s %14s %12s %12s %8s\n",
            "kind", "count", "total ns", "average ns", "min ns", "max ns", "% time");
    for (i = 0; i < RXVM_TRANSITION_COUNT; i++) {
        const rxvm_profile_counter *counter = &state->transitions[i];
        if (!counter->count) continue;
        fprintf(out,
                "%-30s %7" PRIu64 " %14" PRIu64 " %14" PRIu64
                " %12" PRIu64 " %12" PRIu64 " %7.2f%%\n",
                rxvm_profile_transition_names[i], counter->count,
                counter->total_ns, rxvm_profile_average(counter),
                counter->min_ns, counter->max_ns,
                rxvm_profile_percent(counter->total_ns, transition_total));
    }

    {
        size_t procedure_used = 0;
        size_t *procedure_indices = rxvm_profile_sorted_procedure_indices(
                state, &procedure_used);
        size_t procedure_position;

        fprintf(out, "\nProcedures and methods (inclusive body overlaps nested calls)\n");
        fprintf(out,
                "%-60s %-9s %9s %9s %9s %14s %12s %14s %14s %14s %8s\n",
                "callable", "kind", "calls", "complete", "unwound",
                "total ns", "average ns", "body ns", "self ns",
                "native child", "self %");
        for (procedure_position = 0; procedure_position < procedure_used;
             procedure_position++) {
            const rxvm_profile_procedure *procedure =
                    &state->procedures[procedure_indices[procedure_position]];
            if (procedure->native) {
                fprintf(out,
                        "%-60s %-9s %9" PRIu64 " %9" PRIu64 " %9" PRIu64
                        " %14" PRIu64 " %12" PRIu64 " %14s %14s %14s %8s\n",
                        procedure->name, "native", procedure->calls,
                        procedure->completed, procedure->unwound,
                        procedure->native_total.total_ns,
                        rxvm_profile_average(&procedure->native_total),
                        "-", "-", "-", "-");
            } else {
                fprintf(out,
                        "%-60s %-9s %9" PRIu64 " %9" PRIu64 " %9" PRIu64
                        " %14" PRIu64 " %12" PRIu64 " %14" PRIu64
                        " %14" PRIu64 " %14" PRIu64
                        " %7.2f%%\n",
                        procedure->name,
                        rxvm_profile_callable_kind_name(procedure),
                        procedure->calls, procedure->completed,
                        procedure->unwound,
                        procedure->elapsed.total_ns,
                        rxvm_profile_average(&procedure->elapsed),
                        procedure->inclusive_body.total_ns,
                        procedure->self.total_ns,
                        procedure->native_child.total_ns,
                        rxvm_profile_percent(procedure->self.total_ns,
                                             procedure->inclusive_body.total_ns));
            }
        }

        fprintf(out, "\nCall mechanics (VM entry/exit work; native calls expose total time only)\n");
        fprintf(out, "%-60s %9s %14s %12s %9s %14s %12s %14s\n",
                "callable", "entries", "entry ns", "entry avg", "exits",
                "exit ns", "exit avg", "overhead ns");
        for (procedure_position = 0; procedure_position < procedure_used;
             procedure_position++) {
            const rxvm_profile_procedure *procedure =
                    &state->procedures[procedure_indices[procedure_position]];
            uint64_t overhead;
            if (procedure->native) continue;
            overhead = procedure->entry_overhead.total_ns;
            if (UINT64_MAX - overhead < procedure->exit_overhead.total_ns)
                overhead = UINT64_MAX;
            else
                overhead += procedure->exit_overhead.total_ns;
            fprintf(out,
                    "%-60s %9" PRIu64 " %14" PRIu64 " %12" PRIu64
                    " %9" PRIu64 " %14" PRIu64 " %12" PRIu64
                    " %14" PRIu64 "\n",
                    procedure->name, procedure->entry_overhead.count,
                    procedure->entry_overhead.total_ns,
                    rxvm_profile_average(&procedure->entry_overhead),
                    procedure->exit_overhead.count,
                    procedure->exit_overhead.total_ns,
                    rxvm_profile_average(&procedure->exit_overhead), overhead);
        }
        free(procedure_indices);
    }

    fprintf(out, "\nRuntime allocation and value/frame storage\n");
    fprintf(out, "%-30s %12s %16s %16s %16s\n",
            "counter", "requests", "requested bytes", "max request", "high water");
    for (i = 0; i < RXVM_PROFILE_ALLOC_COUNT; i++) {
        const rxvm_profile_allocation_counter *counter = &state->allocations[i];
        fprintf(out, "%-30s %12" PRIu64 " %16" PRIu64 " %16" PRIu64 " %16s\n",
                rxvm_profile_allocation_names[i], counter->count,
                counter->bytes, counter->max_bytes, "-");
    }
    fprintf(out, "%-30s %12" PRIu64 " %16" PRIu64 " %16" PRIu64 " %16s\n",
            "value_slots", state->value_slots, state->value_slot_bytes,
            state->max_value_slots_per_block * sizeof(value), "-");
    fprintf(out, "%-30s %12" PRIu64 " %16s %16s %16" PRIu64 "\n",
            "frame_activations", state->frame_activations, "-", "-",
            state->frame_high_water);
    fprintf(out, "%-30s %12" PRIu64 " %16s %16s %16s\n",
            "frame_reuses", state->frame_reuses, "-", "-", "-");

    fprintf(out, "\nValue operations (payload bytes; nested helper calls are separate rows)\n");
    fprintf(out, "%-22s %-12s %12s %16s %16s\n",
            "operation", "shape", "count", "payload bytes", "max payload");
    for (i = 0; i < RXVM_PROFILE_VALUE_OPERATION_COUNT; i++) {
        int shape;
        for (shape = 0; shape < RXVM_PROFILE_VALUE_SHAPE_COUNT; shape++) {
            const rxvm_profile_allocation_counter *counter =
                    &state->value_operations[i][shape];
            if (!counter->count) continue;
            fprintf(out, "%-22s %-12s %12" PRIu64 " %16" PRIu64
                         " %16" PRIu64 "\n",
                    rxvm_profile_value_operation_names[i],
                    rxvm_profile_value_shape_names[shape], counter->count,
                    counter->bytes, counter->max_bytes);
        }
    }

    fprintf(out, "\nFrame-entry phases (units are registers or phase events)\n");
    fprintf(out, "%-24s %-8s %10s %12s %16s %14s\n",
            "phase", "source", "entries", "units", "total ns", "average ns");
    for (i = 0; i < RXVM_PROFILE_FRAME_PHASE_COUNT; i++) {
        int source;
        for (source = 0; source < RXVM_PROFILE_FRAME_SOURCE_COUNT; source++) {
            const rxvm_profile_counter *counter =
                    &state->frame_phases[i][source];
            if (!counter->count) continue;
            fprintf(out, "%-24s %-8s %10" PRIu64 " %12" PRIu64
                         " %16" PRIu64 " %14" PRIu64 "\n",
                    rxvm_profile_frame_phase_names[i],
                    rxvm_profile_frame_source_names[source], counter->count,
                    state->frame_phase_units[i][source], counter->total_ns,
                    rxvm_profile_average(counter));
        }
    }

    fprintf(out, "\nBranch sites (backward means taken to same-module index <= site)\n");
    fprintf(out, "%-26s %10s %10s %10s %10s %10s %s\n",
            "opcode", "site", "exec", "taken", "fallthru", "backward",
            "module");
    {
        size_t branch_index;
        for (branch_index = 0; branch_index < state->branch_row_count;
             branch_index++) {
            const rxvm_profile_branch_row *row =
                    &state->branch_rows[branch_index];
            fprintf(out, "%-26s %10zu %10" PRIu64 " %10" PRIu64
                         " %10" PRIu64 " %10" PRIu64 " %s\n",
                    instruction_map[row->opcode].instruction,
                    row->instruction_index, row->executions, row->taken,
                    row->fallthrough, row->backward,
                    rxvm_profile_module_name(state, row->module_id));
        }
    }

    fprintf(out, "\nCall-path census (dynamic observations; zero categories retained in CSV)\n");
    fprintf(out, "%-30s %12s\n", "call path", "attempts");
    for (i = 0; i < RXVM_PROFILE_CALL_PATH_COUNT; i++)
        fprintf(out, "%-30s %12" PRIu64 "\n",
                rxvm_profile_call_path_names[i],
                state->call_path_totals[i]);
    fprintf(out, "%-30s %12s\n", "callable kind", "attempts");
    for (i = 0; i < RXVM_PROFILE_CENSUS_CALLABLE_KIND_COUNT; i++)
        fprintf(out, "%-30s %12" PRIu64 "\n",
                rxvm_profile_census_kind_names[i],
                state->callable_kind_totals[i]);
    fprintf(out, "%-30s %12s\n", "frame disposition", "attempts");
    for (i = 0; i < RXVM_PROFILE_FRAME_DISPOSITION_COUNT; i++)
        fprintf(out, "%-30s %12" PRIu64 "\n",
                rxvm_profile_frame_disposition_names[i],
                state->frame_disposition_totals[i]);
    fprintf(out, "%-30s %12s\n", "call outcome", "attempts");
    for (i = 0; i < RXVM_PROFILE_CALL_OUTCOME_COUNT; i++)
        fprintf(out, "%-30s %12" PRIu64 "\n",
                rxvm_profile_call_outcome_names[i],
                state->call_outcome_totals[i]);

    fprintf(out, "\nObserved exact call rows\n");
    fprintf(out,
            "%-34s %-18s %7s %-10s %-16s %-12s %10s %8s %8s %8s\n",
            "target", "path", "arity", "kind", "frame", "outcome",
            "calls", "setup", "restore", "copies");
    {
        size_t call_index;
        for (call_index = 0; call_index < state->call_row_count; call_index++) {
            const rxvm_profile_call_row *row = &state->call_rows[call_index];
            char arity[32];
            if (row->arity_valid)
                snprintf(arity, sizeof(arity), "%" PRIu64, row->arity);
            else
                snprintf(arity, sizeof(arity), "%s", "invalid");
            fprintf(out,
                    "%-34.34s %-18s %7s %-10s %-16s %-12s %10" PRIu64
                    " %8" PRIu64 " %8" PRIu64 " %8" PRIu64 "\n",
                    rxvm_profile_call_target(state, row),
                    rxvm_profile_call_path_names[row->path], arity,
                    rxvm_profile_census_kind_names[row->callable_kind],
                    rxvm_profile_frame_disposition_names[
                            row->frame_disposition],
                    rxvm_profile_call_outcome_names[row->outcome],
                    row->count, row->setup_swaps,
                    row->normal_restoration_swaps,
                    row->defensive_argument_copies);
        }
    }

    fprintf(out, "\nReturn placement (dynamic opcode decisions)\n");
    fprintf(out, "%-30s %12s\n", "placement", "count");
    for (i = 0; i < RXVM_PROFILE_RETURN_PLACEMENT_COUNT; i++)
        fprintf(out, "%-30s %12" PRIu64 "\n",
                rxvm_profile_return_placement_names[i],
                state->return_placements[i]);

    fprintf(out, "\nDynamic selection (selection is separate from DCALL)\n");
    fprintf(out, "%-30s %12s %12s %12s\n",
            "selector", "attempt", "success", "failure");
    for (i = 0; i < RXVM_PROFILE_DYNAMIC_KIND_COUNT; i++)
        fprintf(out, "%-30s %12" PRIu64 " %12" PRIu64 " %12" PRIu64 "\n",
                rxvm_profile_dynamic_kind_names[i],
                state->dynamic_resolution[i][RXVM_PROFILE_DYNAMIC_ATTEMPT],
                state->dynamic_resolution[i][RXVM_PROFILE_DYNAMIC_SUCCESS],
                state->dynamic_resolution[i][RXVM_PROFILE_DYNAMIC_FAILURE]);

    {
        uint64_t swap_total = state->swap_operations;
        uint64_t copy_total = state->instructions[OP_COPY_REG_REG].count;
        uint64_t classified_swaps = state->setup_swaps;
        uint64_t unclassified_swaps;
        uint64_t unclassified_copies;
        if (UINT64_MAX - classified_swaps < state->normal_restoration_swaps)
            classified_swaps = UINT64_MAX;
        else
            classified_swaps += state->normal_restoration_swaps;
        unclassified_swaps = classified_swaps <= swap_total
                ? swap_total - classified_swaps : 0;
        unclassified_copies =
                state->defensive_argument_copies <= copy_total
                ? copy_total - state->defensive_argument_copies : 0;
        fprintf(out, "\nCall-window attribution (NR-04 effects-backed dynamic slice)\n");
        fprintf(out, "%-34s %12s\n", "mechanic", "count");
        fprintf(out, "%-34s %12" PRIu64 "\n", "setup swaps",
                state->setup_swaps);
        fprintf(out, "%-34s %12" PRIu64 "\n", "normal restoration swaps",
                state->normal_restoration_swaps);
        fprintf(out, "%-34s %12" PRIu64 "\n", "unclassified swaps",
                unclassified_swaps);
        fprintf(out, "%-34s %12" PRIu64 "\n", "defensive argument copies",
                state->defensive_argument_copies);
        fprintf(out, "%-34s %12" PRIu64 "\n", "unclassified copies",
                unclassified_copies);
        fprintf(out, "%-34s %12" PRIu64 "\n", "degraded attributions",
                state->attribution_degraded);
    }

    fprintf(out, "\nSignal-unwind call-window restoration\n");
    fprintf(out, "%-42s %12s\n", "counter", "count");
    fprintf(out, "%-42s %12" PRIu64 "\n", "events",
            state->signal_unwind_events);
    fprintf(out, "%-42s %12" PRIu64 "\n", "bytecode frames discarded",
            state->signal_bytecode_frames_discarded);
    fprintf(out, "%-42s %12" PRIu64 "\n", "argument windows restored",
            state->signal_argument_windows_restored);
    fprintf(out, "%-42s %12" PRIu64 "\n", "argument slots restored",
            state->signal_argument_slots_restored);
    fprintf(out, "%-42s %12" PRIu64 "\n",
            "interrupted native windows restored",
            state->signal_native_windows_restored);
    fprintf(out, "%-42s %12" PRIu64 "\n",
            "interrupted native slots restored",
            state->signal_native_slots_restored);
    fprintf(out, "%-42s %12" PRIu64 "\n", "restoration failures",
            state->signal_restoration_failures);

    fprintf(out, "\nInterrupt sub-phases\n");
    fprintf(out,
            "scan: count=%" PRIu64 " total=%" PRIu64 " ns average=%" PRIu64
            " ns without-selection=%" PRIu64 "\n",
            state->interrupt_scans.count, state->interrupt_scans.total_ns,
            rxvm_profile_average(&state->interrupt_scans),
            state->interrupt_scans_without_selection);
    fprintf(out,
            "mechanics: count=%" PRIu64 " total=%" PRIu64 " ns average=%" PRIu64 " ns\n",
            state->interrupt_mechanics.count,
            state->interrupt_mechanics.total_ns,
            rxvm_profile_average(&state->interrupt_mechanics));
    fprintf(out, "%-24s %9s %9s %9s %9s %14s %14s\n",
            "signal", "selected", "entries", "resumes", "terminal",
            "mechanics ns", "average ns");
    for (i = 1; i < RXSIGNAL_MAX; i++) {
        char fallback[24];
        const rxvm_profile_interrupt_counter *counter = &state->interrupts[i];
        if (!counter->selected && !counter->entries && !counter->resumes &&
                !counter->terminals && !counter->mechanics.count) continue;
        fprintf(out,
                "%-24s %9" PRIu64 " %9" PRIu64 " %9" PRIu64 " %9" PRIu64
                " %14" PRIu64 " %14" PRIu64 "\n",
                rxvm_profile_signal_name((unsigned char)i, signal_name,
                                         fallback, sizeof(fallback)),
                counter->selected, counter->entries, counter->resumes,
                counter->terminals, counter->mechanics.total_ns,
                rxvm_profile_average(&counter->mechanics));
    }
}

void rxvm_profile_report(const rxvm_profile_state *state,
                         const char *output_path,
                         const char *vm_mode,
                         int result,
                         const Instruction *instruction_map,
                         rxvm_profile_signal_name_fn signal_name) {
    FILE *out = stderr;
    int close_output = 0;

    if (!state->enabled || !rxvm_profile_total_instruction_count(state)) return;
    if (output_path) {
        out = fopen(output_path, "w");
        if (!out) {
            fprintf(stderr, "ERROR: unable to open VM profile output '%s'\n",
                    output_path);
            out = stderr;
        } else {
            close_output = 1;
        }
    }

    if (rxvm_profile_csv_path(output_path)) {
        rxvm_profile_write_csv(out, state, vm_mode, result, instruction_map,
                               signal_name);
    } else {
        rxvm_profile_write_table(out, state, vm_mode, result, instruction_map,
                                 signal_name);
    }

    if (close_output) fclose(out);
}
