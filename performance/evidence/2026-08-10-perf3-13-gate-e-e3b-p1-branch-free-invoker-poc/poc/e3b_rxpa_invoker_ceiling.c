/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Disposable PERF3-13 E3b-P1 machine-level proof. This file deliberately does
 * not change proc_runtime or the production handlers. It compares the existing
 * direct adapter with the proposed preselected invoker shape and validates the
 * cold, quiescent legacy-binding transition.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crexxpa.h"
#include "rxvmintp.h"

#if defined(_MSC_VER)
#define POC_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define POC_NOINLINE __attribute__((noinline))
#else
#define POC_NOINLINE
#endif

#define POC_MAX_PROCEDURES 16u

typedef void (*poc_invoker)(void *function, int args, value **argv,
                            value *ret, value *signal);

typedef enum poc_legacy_mode {
    POC_LEGACY_EXCLUSIVE = 0,
    POC_LEGACY_LOCKED = 1
} poc_legacy_mode;

typedef struct poc_procedure {
    void *function;
    poc_invoker volatile invoker;
    volatile uint32_t capabilities;
    int legacy;
} poc_procedure;

typedef struct poc_coordinator {
    poc_procedure *procedures[POC_MAX_PROCEDURES];
    size_t procedure_count;
    size_t executor_count;
    size_t legacy_executor_count;
    size_t transition_count;
    size_t rebound_count;
    poc_legacy_mode legacy_mode;
} poc_coordinator;

static volatile unsigned long long poc_call_count;

static void poc_tick(rxinteger numargs, rxpa_attribute_value *args,
                     rxpa_attribute_value result,
                     rxpa_attribute_value signal) {
    (void)numargs;
    (void)args;
    (void)result;
    (void)signal;
    poc_call_count++;
}

static int poc_is_reentrant(uint32_t capabilities) {
    return (capabilities & RXPA_PLUGIN_CAP_PROCESS_REENTRANT) != 0u;
}

static void poc_bind_procedure(poc_coordinator *coordinator,
                               poc_procedure *procedure,
                               uint32_t capabilities) {
    procedure->function = (void *)poc_tick;
    procedure->capabilities = capabilities;
    procedure->legacy = !poc_is_reentrant(capabilities);
    procedure->invoker = procedure->legacy &&
                         coordinator->legacy_mode == POC_LEGACY_LOCKED
                             ? rxvm_callfunc
                             : rxvm_callfunc_direct;
}

static int poc_load_procedure(poc_coordinator *coordinator,
                              poc_procedure *procedure,
                              uint32_t capabilities) {
    if (coordinator->procedure_count == POC_MAX_PROCEDURES) return -1;
    poc_bind_procedure(coordinator, procedure, capabilities);
    coordinator->procedures[coordinator->procedure_count++] = procedure;
    return 0;
}

/*
 * The integrated implementation must establish this quiescent precondition
 * through the VM worker lifecycle. The isolated proof rejects a transition
 * when the caller cannot establish it; it adds no per-call announcement.
 */
static int poc_transition_legacy_to_locked(poc_coordinator *coordinator,
                                           int executors_quiescent) {
    size_t index;
    if (coordinator->legacy_mode == POC_LEGACY_LOCKED) return 0;
    if (!executors_quiescent) return -1;

    for (index = 0; index < coordinator->procedure_count; index++) {
        poc_procedure *procedure = coordinator->procedures[index];
        if (!procedure->legacy) continue;
        procedure->invoker = rxvm_callfunc;
        coordinator->rebound_count++;
    }
    coordinator->legacy_mode = POC_LEGACY_LOCKED;
    coordinator->transition_count++;
    return 0;
}

static int poc_register_executor(poc_coordinator *coordinator,
                                 int legacy_capable,
                                 int executors_quiescent) {
    if (legacy_capable && coordinator->legacy_executor_count == 1u &&
        poc_transition_legacy_to_locked(coordinator,
                                        executors_quiescent) != 0) {
        return -1;
    }
    coordinator->executor_count++;
    if (legacy_capable) coordinator->legacy_executor_count++;
    return 0;
}

static POC_NOINLINE void poc_run_raw_direct(unsigned long long iterations) {
    unsigned long long index;
    for (index = 0; index < iterations; index++) {
        rxvm_callfunc_direct((void *)poc_tick, 0, NULL, NULL, NULL);
    }
}

static POC_NOINLINE void poc_run_selected(poc_procedure *procedure,
                                          unsigned long long iterations) {
    unsigned long long index;
    for (index = 0; index < iterations; index++) {
        poc_invoker invoker = procedure->invoker;
        invoker(procedure->function, 0, NULL, NULL, NULL);
    }
}

static POC_NOINLINE void poc_run_branch(poc_procedure *procedure,
                                        unsigned long long iterations) {
    unsigned long long index;
    for (index = 0; index < iterations; index++) {
        uint32_t capabilities = procedure->capabilities;
        if (poc_is_reentrant(capabilities)) {
            rxvm_callfunc_direct(procedure->function, 0, NULL, NULL, NULL);
        } else {
            rxvm_callfunc(procedure->function, 0, NULL, NULL, NULL);
        }
    }
}

static int poc_run_transition_test(void) {
    poc_coordinator coordinator;
    poc_procedure reentrant;
    poc_procedure legacy;
    poc_procedure late_legacy;
    poc_invoker reentrant_binding;

    memset(&coordinator, 0, sizeof(coordinator));
    memset(&reentrant, 0, sizeof(reentrant));
    memset(&legacy, 0, sizeof(legacy));
    memset(&late_legacy, 0, sizeof(late_legacy));

    if (poc_load_procedure(&coordinator, &reentrant,
                           RXPA_PLUGIN_CAP_PROCESS_REENTRANT) != 0 ||
        poc_load_procedure(&coordinator, &legacy, 0u) != 0) {
        return 1;
    }
    reentrant_binding = reentrant.invoker;

    if (poc_register_executor(&coordinator, 1, 1) != 0 ||
        legacy.invoker != rxvm_callfunc_direct ||
        coordinator.legacy_mode != POC_LEGACY_EXCLUSIVE) {
        return 1;
    }

    /* A reentrant-only executor must not disturb either binding. */
    if (poc_register_executor(&coordinator, 0, 1) != 0 ||
        coordinator.transition_count != 0u ||
        reentrant.invoker != reentrant_binding ||
        legacy.invoker != rxvm_callfunc_direct) {
        return 1;
    }

    /* The second legacy-capable executor must not publish without quiescence. */
    if (poc_register_executor(&coordinator, 1, 0) == 0 ||
        coordinator.legacy_mode != POC_LEGACY_EXCLUSIVE ||
        legacy.invoker != rxvm_callfunc_direct) {
        return 1;
    }

    /* Retry the rejected publication at the established safe point. */
    if (poc_register_executor(&coordinator, 1, 1) != 0 ||
        coordinator.legacy_mode != POC_LEGACY_LOCKED ||
        coordinator.transition_count != 1u ||
        coordinator.rebound_count != 1u ||
        legacy.invoker != rxvm_callfunc ||
        reentrant.invoker != reentrant_binding) {
        return 1;
    }

    /* A legacy procedure loaded after the sticky transition binds locked. */
    if (poc_load_procedure(&coordinator, &late_legacy, 0u) != 0 ||
        late_legacy.invoker != rxvm_callfunc ||
        coordinator.transition_count != 1u) {
        return 1;
    }

    poc_call_count = 0u;
    poc_run_selected(&reentrant, 1u);
    poc_run_selected(&legacy, 1u);
    poc_run_selected(&late_legacy, 1u);
    if (poc_call_count != 3u) return 1;

    puts("PASS: E3b RXPA invoker transition proof");
    return 0;
}

static int poc_run_invocation_mode(const char *mode,
                                   unsigned long long iterations) {
    poc_coordinator coordinator;
    poc_procedure procedure;

    memset(&coordinator, 0, sizeof(coordinator));
    memset(&procedure, 0, sizeof(procedure));
    poc_call_count = 0u;

    if (strcmp(mode, "raw-direct") == 0) {
        poc_run_raw_direct(iterations);
    } else if (strcmp(mode, "selected-direct") == 0) {
        if (poc_load_procedure(&coordinator, &procedure,
                               RXPA_PLUGIN_CAP_PROCESS_REENTRANT) != 0) {
            return 1;
        }
        poc_run_selected(&procedure, iterations);
    } else if (strcmp(mode, "branch-direct") == 0) {
        poc_bind_procedure(&coordinator, &procedure,
                           RXPA_PLUGIN_CAP_PROCESS_REENTRANT);
        poc_run_branch(&procedure, iterations);
    } else if (strcmp(mode, "selected-locked") == 0) {
        coordinator.legacy_mode = POC_LEGACY_LOCKED;
        if (poc_load_procedure(&coordinator, &procedure, 0u) != 0) return 1;
        poc_run_selected(&procedure, iterations);
    } else if (strcmp(mode, "branch-locked") == 0) {
        poc_bind_procedure(&coordinator, &procedure, 0u);
        poc_run_branch(&procedure, iterations);
    } else {
        fprintf(stderr, "Unknown mode: %s\n", mode);
        return 1;
    }

    if (poc_call_count != iterations) {
        fprintf(stderr, "Invocation count mismatch: expected %llu, got %llu\n",
                iterations, poc_call_count);
        return 1;
    }
    printf("PASS: E3b RXPA invoker ceiling MODE=%s CALLS=%llu\n",
           mode, iterations);
    return 0;
}

static int poc_selftest(void) {
    static const char *modes[] = {
        "raw-direct",
        "selected-direct",
        "branch-direct",
        "selected-locked",
        "branch-locked"
    };
    size_t index;
    for (index = 0; index < sizeof(modes) / sizeof(modes[0]); index++) {
        if (poc_run_invocation_mode(modes[index], 4u) != 0) return 1;
    }
    return poc_run_transition_test();
}

static int poc_parse_iterations(const char *text,
                                unsigned long long *iterations) {
    char *end = NULL;
    unsigned long long parsed;
    errno = 0;
    parsed = strtoull(text, &end, 10);
    if (errno != 0 || !end || *end != '\0' || parsed == 0u) return -1;
    *iterations = parsed;
    return 0;
}

int main(int argc, char **argv) {
    unsigned long long iterations;
    if (argc == 2 && strcmp(argv[1], "selftest") == 0) return poc_selftest();
    if (argc == 2 && strcmp(argv[1], "transition") == 0) {
        return poc_run_transition_test();
    }
    if (argc != 3 || poc_parse_iterations(argv[2], &iterations) != 0) {
        fprintf(stderr,
                "Usage: %s selftest|transition|MODE ITERATIONS\n"
                "MODE: raw-direct, selected-direct, branch-direct, "
                "selected-locked, branch-locked\n",
                argv[0]);
        return 2;
    }
    return poc_run_invocation_mode(argv[1], iterations);
}
