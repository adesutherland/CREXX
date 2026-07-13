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
    state->current_transition = RXVM_TRANSITION_SEQUENTIAL;
    state->instruction_activation_index = SIZE_MAX;
    state->native_procedure_id = SIZE_MAX;
    if (!state->enabled) return;

    state->activations = (rxvm_profile_activation *)calloc(
            64, sizeof(rxvm_profile_activation));
    if (state->activations) state->activation_capacity = 64;
    else state->procedure_tracking_unavailable = 1;
    rxvm_profile_refresh_catalog(state, context);

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
    free(state->activations);
    state->procedures = 0;
    state->activations = 0;
    state->procedure_count = 0;
    state->procedure_capacity = 0;
    state->activation_count = 0;
    state->activation_capacity = 0;
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

    fprintf(out, "section,name,value,id,count,total_ns,average_ns,min_ns,max_ns,percent,selected,entries,resumes,terminals,module,kind,completed,unwound,return_type,args\n");
    fprintf(out, "summary,schema_version,2,,0,0,0,0,0,0,,,,,,,,,,\n");
    fprintf(out, "summary,vm_mode,%s,,0,0,0,0,0,0,,,,,,,,,,\n", vm_mode);
    fprintf(out, "summary,result,%d,,0,0,0,0,0,0,,,,,,,,,,\n", result);
    fprintf(out, "summary,timer_read_min_ns,%" PRIu64 ",,1,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",0,,,,,,,,,,\n",
            state->timer_read_min_ns,
            state->timer_read_min_ns, state->timer_read_min_ns,
            state->timer_read_min_ns, state->timer_read_min_ns);
    fprintf(out, "summary,timer_zero_deltas,%" PRIu64 ",,0,0,0,0,0,0,,,,,,,,,,\n",
            state->timer_zero_deltas);
    fprintf(out, "summary,interrupt_polls,%" PRIu64 ",,0,0,0,0,0,0,,,,,,,,,,\n",
            state->interrupt_polls);
    fprintf(out, "summary,invalid_events,%" PRIu64 ",,0,0,0,0,0,0,,,,,,,,,,\n",
            state->invalid_events);
    fprintf(out, "summary,counter_overflow,%d,,0,0,0,0,0,0,,,,,,,,,,\n",
            state->overflowed);
    fprintf(out, "summary,procedure_tracking_unavailable,%d,,0,0,0,0,0,0,,,,,,,,,,\n",
            state->procedure_tracking_unavailable);

    rxvm_profile_sort_instruction_indices(state, indices, &used);
    for (position = 0; position < used; position++) {
        int opcode = indices[position];
        const rxvm_profile_counter *counter = &state->instructions[opcode];
        fprintf(out,
                "instruction,%s,,%d,%" PRIu64 ",%" PRIu64 ",%" PRIu64
                ",%" PRIu64 ",%" PRIu64 ",%.6f,,,,,,,,,,\n",
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
                ",%" PRIu64 ",%" PRIu64 ",%.6f,,,,,,,,,,\n",
                rxvm_profile_transition_names[i], i, counter->count,
                counter->total_ns, rxvm_profile_average(counter),
                counter->min_ns, counter->max_ns,
                rxvm_profile_percent(counter->total_ns, transition_total));
    }

    fprintf(out,
            "interrupt,scan_all,,,%" PRIu64 ",%" PRIu64 ",%" PRIu64
            ",%" PRIu64 ",%" PRIu64 ",0,,,,,,,,,,\n",
            state->interrupt_scans.count, state->interrupt_scans.total_ns,
            rxvm_profile_average(&state->interrupt_scans),
            state->interrupt_scans.min_ns, state->interrupt_scans.max_ns);
    fprintf(out,
            "interrupt,scan_without_selection,,,%" PRIu64 ",0,0,0,0,0,,,,,,,,,,\n",
            state->interrupt_scans_without_selection);
    fprintf(out,
            "interrupt,mechanics_all,,,%" PRIu64 ",%" PRIu64 ",%" PRIu64
            ",%" PRIu64 ",%" PRIu64 ",0,,,,,,,,,,\n",
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
                ",%" PRIu64 ",%" PRIu64 ",,,,,,\n",
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
                        out, procedure, "entry_overhead",
                        &procedure->entry_overhead);
                rxvm_profile_write_procedure_csv_row(
                        out, procedure, "exit_overhead",
                        &procedure->exit_overhead);
            }
        }
        free(procedure_indices);
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
            "; counter overflow=%s; procedure tracking=%s\n",
            state->interrupt_polls, state->invalid_events,
            state->overflowed ? "yes" : "no",
            state->procedure_tracking_unavailable ? "degraded" : "complete");

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
                "%-60s %-9s %9s %9s %9s %14s %12s %14s %14s %8s\n",
                "callable", "kind", "calls", "complete", "unwound",
                "total ns", "average ns", "body ns", "self ns", "self %");
        for (procedure_position = 0; procedure_position < procedure_used;
             procedure_position++) {
            const rxvm_profile_procedure *procedure =
                    &state->procedures[procedure_indices[procedure_position]];
            if (procedure->native) {
                fprintf(out,
                        "%-60s %-9s %9" PRIu64 " %9" PRIu64 " %9" PRIu64
                        " %14" PRIu64 " %12" PRIu64 " %14s %14s %8s\n",
                        procedure->name, "native", procedure->calls,
                        procedure->completed, procedure->unwound,
                        procedure->native_total.total_ns,
                        rxvm_profile_average(&procedure->native_total),
                        "-", "-", "-");
            } else {
                fprintf(out,
                        "%-60s %-9s %9" PRIu64 " %9" PRIu64 " %9" PRIu64
                        " %14" PRIu64 " %12" PRIu64 " %14" PRIu64
                        " %14" PRIu64
                        " %7.2f%%\n",
                        procedure->name,
                        rxvm_profile_callable_kind_name(procedure),
                        procedure->calls, procedure->completed,
                        procedure->unwound,
                        procedure->elapsed.total_ns,
                        rxvm_profile_average(&procedure->elapsed),
                        procedure->inclusive_body.total_ns,
                        procedure->self.total_ns,
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
