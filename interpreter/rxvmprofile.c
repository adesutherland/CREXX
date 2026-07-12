/* Reporting and monotonic clock support for compile-time VM profiling. */

#include "rxvmprofile.h"

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

void rxvm_profile_begin(rxvm_profile_state *state, int enabled) {
    uint64_t minimum = UINT64_MAX;
    int i;

    memset(state, 0, sizeof(*state));
    state->enabled = enabled != 0;
    state->current_transition = RXVM_TRANSITION_SEQUENTIAL;
    if (!state->enabled) return;

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
    state->timer_read_min_ns = minimum == UINT64_MAX ? 0 : minimum;
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

    fprintf(out, "section,name,value,id,count,total_ns,average_ns,min_ns,max_ns,percent,selected,entries,resumes,terminals\n");
    fprintf(out, "summary,vm_mode,%s,,0,0,0,0,0,0,,,,\n", vm_mode);
    fprintf(out, "summary,result,%d,,0,0,0,0,0,0,,,,\n", result);
    fprintf(out, "summary,timer_read_min_ns,%" PRIu64 ",,1,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",0,,,,\n",
            state->timer_read_min_ns,
            state->timer_read_min_ns, state->timer_read_min_ns,
            state->timer_read_min_ns, state->timer_read_min_ns);
    fprintf(out, "summary,timer_zero_deltas,%" PRIu64 ",,0,0,0,0,0,0,,,,\n",
            state->timer_zero_deltas);
    fprintf(out, "summary,interrupt_polls,%" PRIu64 ",,0,0,0,0,0,0,,,,\n",
            state->interrupt_polls);
    fprintf(out, "summary,invalid_events,%" PRIu64 ",,0,0,0,0,0,0,,,,\n",
            state->invalid_events);
    fprintf(out, "summary,counter_overflow,%d,,0,0,0,0,0,0,,,,\n",
            state->overflowed);

    rxvm_profile_sort_instruction_indices(state, indices, &used);
    for (position = 0; position < used; position++) {
        int opcode = indices[position];
        const rxvm_profile_counter *counter = &state->instructions[opcode];
        fprintf(out,
                "instruction,%s,,%d,%" PRIu64 ",%" PRIu64 ",%" PRIu64
                ",%" PRIu64 ",%" PRIu64 ",%.6f,,,,\n",
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
                ",%" PRIu64 ",%" PRIu64 ",%.6f,,,,\n",
                rxvm_profile_transition_names[i], i, counter->count,
                counter->total_ns, rxvm_profile_average(counter),
                counter->min_ns, counter->max_ns,
                rxvm_profile_percent(counter->total_ns, transition_total));
    }

    fprintf(out,
            "interrupt,scan_all,,,%" PRIu64 ",%" PRIu64 ",%" PRIu64
            ",%" PRIu64 ",%" PRIu64 ",0,,,,\n",
            state->interrupt_scans.count, state->interrupt_scans.total_ns,
            rxvm_profile_average(&state->interrupt_scans),
            state->interrupt_scans.min_ns, state->interrupt_scans.max_ns);
    fprintf(out,
            "interrupt,scan_without_selection,,,%" PRIu64 ",0,0,0,0,0,,,,\n",
            state->interrupt_scans_without_selection);
    fprintf(out,
            "interrupt,mechanics_all,,,%" PRIu64 ",%" PRIu64 ",%" PRIu64
            ",%" PRIu64 ",%" PRIu64 ",0,,,,\n",
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
                ",%" PRIu64 ",%" PRIu64 "\n",
                rxvm_profile_signal_name((unsigned char)i, signal_name,
                                         fallback, sizeof(fallback)),
                i, counter->mechanics.count, counter->mechanics.total_ns,
                rxvm_profile_average(&counter->mechanics),
                counter->mechanics.min_ns, counter->mechanics.max_ns,
                counter->selected, counter->entries, counter->resumes,
                counter->terminals);
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
            "Clock: monotonic wall time; raw instrumented timings; minimum positive adjacent timer read=%" PRIu64
            " ns; zero calibration deltas=%" PRIu64 "/1000\n",
            state->timer_read_min_ns, state->timer_zero_deltas);
    fprintf(out,
            "Hot-loop interrupt polls=%" PRIu64 "; invalid events=%" PRIu64
            "; counter overflow=%s\n",
            state->interrupt_polls, state->invalid_events,
            state->overflowed ? "yes" : "no");

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
