/* Actual VM event ownership, scoped initialization and worker reuse. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "rxvm.h"
#include "rxvmintp.h"
#include "rxvmvars.h"
#include "rxastree.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef struct test_state {
    const char *image;
    const char *initializer;
    int failures;
} test_state;

#define CHECK(s, condition) do { if (!(condition)) { \
    fprintf(stderr, "FAIL diagnostic lifecycle line %d: %s\n", __LINE__, #condition); \
    (s)->failures++; } } while (0)

static value *raw_event(test_state *state, value *event) {
    CHECK(state, event->num_attributes >= 5 && event->attributes);
    if (event->num_attributes < 5 || !event->attributes) return NULL;
    CHECK(state, event->attributes[0]->int_value == 1);
    event = event->attributes[4];
    CHECK(state, event && event->num_attributes == 5 && event->attributes);
    return event && event->num_attributes == 5 ? event : NULL;
}

static void check_event(test_state *state, value *event, const char *payload, size_t length) {
    value *raw = raw_event(state, event);
    if (!raw) return;
    CHECK(state, raw->attributes[0]->int_value == RXSIGNAL_OTHER);
    CHECK(state, raw->attributes[1]->int_value > 0 && raw->attributes[2]->int_value > 0);
    CHECK(state, raw->attributes[4]->string_length == length);
    CHECK(state, !length || !memcmp(raw->attributes[4]->string_value, payload, length));
    CHECK(state, raw->native_payload_ops != NULL);
}

static void constructor_payload_ownership(test_state *state, const value *diagnostic) {
    static const rxvm_native_payload_ops user_ops = {
        "signal-diagnostic-test.user-payload", NULL, NULL
    };
    static const unsigned char user_bytes[] = {0x41, 0x00, 0xff, 0x42};
    value signal = {0}, user = {0}, event = {0};
    value *raw, *payload;
    rxinteger module_number = diagnostic->attributes[1]->int_value;
    rxinteger address = diagnostic->attributes[2]->int_value;

    /* The immediate private native signal cell owns a diagnostic beside its
     * visible fields. Only the raw container may inherit that private owner. */
    set_string(&signal, "native\0message", 14);
    signal.int_value = RXSIGNAL_OTHER;
    rxvm_signal_diagnostic_copy(&signal, diagnostic);
    rxvm_invoke_failure_event_with_diagnostic(&event, RXSIGNAL_OTHER,
            module_number, address, &signal, &signal);
    check_event(state, &event, "native\0message", 14);
    raw = raw_event(state, &event);
    if (raw) {
        payload = raw->attributes[4];
        CHECK(state, payload->int_value == RXSIGNAL_OTHER);
        CHECK(state, !payload->native_payload_ops && !payload->binary_length);
        CHECK(state, raw->attributes[1]->int_value == module_number &&
                     raw->attributes[2]->int_value == address);
    }
    CHECK(state, signal.native_payload_ops && signal.int_value == RXSIGNAL_OTHER &&
                 signal.string_length == 14 && !memcmp(signal.string_value, "native\0message", 14));
    clear_value(&event);

    /* A separate user payload owns its own binary bytes. A diagnostic on the
     * raw container must leave that unrelated payload and its copy intact. */
    set_string(&user, "user\0payload", 12);
    user.int_value = 123;
    CHECK(state, set_native_payload(&user, user_bytes, sizeof(user_bytes),
                &user_ops, RXVM_NATIVE_PAYLOAD_FLAG_BITCOPY_SAFE) == 0);
    rxvm_invoke_failure_event_with_diagnostic(&event, RXSIGNAL_OTHER,
            module_number, address, &user, diagnostic);
    check_event(state, &event, "user\0payload", 12);
    raw = raw_event(state, &event);
    if (raw) {
        payload = raw->attributes[4];
        CHECK(state, payload->int_value == 123 && payload->native_payload_ops == &user_ops);
        CHECK(state, payload->binary_length == sizeof(user_bytes));
        if (payload->binary_length == sizeof(user_bytes))
            CHECK(state, !memcmp(payload->binary_value, user_bytes, sizeof(user_bytes)));
        CHECK(state, raw->native_payload_ops != &user_ops);
    }
    clear_value(&event);
    CHECK(state, user.native_payload_ops == &user_ops && user.binary_length == sizeof(user_bytes) &&
                 user.int_value == 123 && user.string_length == 12);
    clear_value(&user);
    clear_value(&signal);
}

static rxvm_context *new_context(test_state *state) {
    rxvm_context *context = rxvm_create();
    CHECK(state, context);
    if (!context) return NULL;
    CHECK(state, rxvm_set_autoload(context, 0) == 0);
    CHECK(state, rxvm_load_file(context, (char *)state->image));
    CHECK(state, rxvm_link(context) == 0);
    CHECK(state, rxvm_prepare(context) == 0);
    CHECK(state, rxvm_initialize(context) == 0);
    return context;
}

static int call_expected(test_state *state, rxvm_context *context, const char *name,
                         value *argument, value *result, int expected_status) {
    size_t found = 0;
    value *arguments[1];
    char *argv[] = {"diagnostic-lifecycle"};
    int status;
    CHECK(state, src_node(context->exposed_proc_tree, (char *)name, &found));
    if (!found) return -1;
    arguments[0] = argument;
    context->ext_proc = (proc_runtime *)(uintptr_t)found;
    context->ext_args = argument ? arguments : NULL;
    context->ext_argc = argument ? 1 : 0;
    context->ext_ret = result;
    status = run(context, 1, argv);
    context->ext_proc = NULL;
    context->ext_args = NULL;
    context->ext_argc = 0;
    context->ext_ret = NULL;
    CHECK(state, status == expected_status);
    CHECK(state, !context->active.pending_interrupts && !context->active.callback_diagnostic &&
                 !context->active.diagnostic_callout);
    return status;
}

static int call(test_state *state, rxvm_context *context, const char *name,
                value *argument, value *result) {
    return call_expected(state, context, name, argument, result, 0);
}

static void lifecycle(test_state *state) {
    rxvm_context *first = new_context(state), *second = new_context(state);
    value a = {0}, b = {0}, copied = {0}, relayed = {0}, fresh = {0};
    value checked = {0}, detached = {0}, detached_copy = {0}, empty = {0};
    value *raw_a, *raw_b, *raw_relay;
    if (!first || !second) goto done;
    CHECK(state, call(state, first, "signal_diag.save_a", NULL, &a) == 0);
    check_event(state, &a, "A\0tail", 6);
    copy_value(&copied, &a);
    raw_a = raw_event(state, &a);
    CHECK(state, call(state, first, "signal_diag.save_b", NULL, &b) == 0);
    check_event(state, &b, "B", 1);
    raw_b = raw_event(state, &b);
    if (raw_a && raw_b)
        CHECK(state, raw_a->attributes[2]->int_value != raw_b->attributes[2]->int_value);
    clear_value(&a);
    CHECK(state, call(state, second, "signal_diag.relay", &copied, &relayed) == 0);
    check_event(state, &relayed, "A\0tail", 6);
    raw_a = raw_event(state, &copied); raw_relay = raw_event(state, &relayed);
    if (raw_a && raw_relay) {
        CHECK(state, raw_a->attributes[1]->int_value == raw_relay->attributes[1]->int_value);
        CHECK(state, raw_a->attributes[2]->int_value == raw_relay->attributes[2]->int_value);
        rxvm_invoke_failure_event_with_diagnostic(&checked, RXSIGNAL_OTHER,
                raw_a->attributes[1]->int_value, raw_a->attributes[2]->int_value,
                raw_a->attributes[4], raw_a);
        check_event(state, &checked, "A\0tail", 6);
        constructor_payload_ownership(state, raw_a);
        /* Descriptor publication must detach even inside a worker binding,
         * then restore that binding for the caller's remaining allocations. */
        {
            rxvm_memory_worker *previous = rxvm_memory_enter(first->worker.memory_worker);
            rxvm_signal_diagnostic_copy(&detached, raw_a);
            CHECK(state, rxvm_memory_current_worker() == first->worker.memory_worker);
            rxvm_memory_leave(previous);
        }
        CHECK(state, detached.native_payload_ops && detached.binary_value);
        CHECK(state, rxvm_memory_owner(detached.binary_value) == NULL);
    }
    CHECK(state, call(state, first, "signal_diag.fresh", &copied, &fresh) == 0);
    check_event(state, &fresh, "fresh", 5);
    CHECK(state, call(state, first, "signal_diag.empty", NULL, &empty) == 0);
    check_event(state, &empty, "empty", 5);
#ifdef CREXX_VM_INSTRUMENTATION_TEST
    raw_a = raw_event(state, &fresh);
    if (raw_a) CHECK(state, rxvm_signal_diagnostic_test_state(raw_a) == 1);
    raw_a = raw_event(state, &empty);
    if (raw_a) CHECK(state, rxvm_signal_diagnostic_test_state(raw_a) == 1);
#endif
    /* A scoped initializer failure publishes a full event without a panic. */
    {
        value sink = {0};
        module *failed = rxvm_load_file(first, (char *)state->initializer);
        CHECK(state, failed);
        first->active.invocation_fault_sink = &sink;
        CHECK(state, rxvm_initialize(first) != 0);
        first->active.invocation_fault_sink = NULL;
        CHECK(state, !first->active.invocation_fault_entry);
        if (failed) CHECK(state, failed->initializer_state == RXVM_INIT_FAILED);
        check_event(state, &sink, "initializer\0event", 17);
        clear_value(&sink);
        /* A failed late module cannot contaminate a previously ready module. */
        clear_value(&fresh);
        CHECK(state, call(state, first, "signal_diag.fresh", &copied, &fresh) == 0);
        check_event(state, &fresh, "fresh", 5);
    }
done:
    clear_value(&a); clear_value(&b); clear_value(&copied); clear_value(&relayed);
    clear_value(&fresh); clear_value(&checked); clear_value(&empty);
    if (first) rxvm_destroy(first);
    if (second) rxvm_destroy(second);
    CHECK(state, rxvm_memory_current_worker() == NULL);
    /* No source-context pointer may be needed by a later copy or release. */
    if (detached.native_payload_ops) {
        rxvm_memory_worker *previous = rxvm_memory_current_worker();
        rxvm_signal_diagnostic_copy(&detached_copy, &detached);
        CHECK(state, rxvm_memory_current_worker() == previous);
        CHECK(state, detached_copy.native_payload_ops == detached.native_payload_ops &&
                     detached_copy.binary_value && detached_copy.binary_length == detached.binary_length);
        CHECK(state, rxvm_memory_owner(detached.binary_value) == NULL &&
                     rxvm_memory_owner(detached_copy.binary_value) == NULL);
    }
    clear_value(&detached);
    clear_value(&detached_copy);
}

#ifdef CREXX_VM_INSTRUMENTATION_TEST
static void allocation_failures(test_state *state) {
    rxvm_context *context = new_context(state);
    value a = {0}, copied = {0};
    value *raw;
    size_t initial = rxvm_signal_diagnostic_test_live_owners();
    size_t budget;
    rxinteger expected_module = 0, expected_address = 0;
    int captured = 0;
    if (!context) return;
    /* Walk every real allocation/publication failure until this tiny capture
     * first succeeds. This bound protects the fixture; it never truncates a
     * production snapshot or assumes an implementation allocation count. */
    for (budget = 0; budget < 128; ++budget) {
        rxvm_signal_diagnostic_test_fail_after(budget);
        CHECK(state, call(state, context, "signal_diag.save_a", NULL, &a) == 0);
        rxvm_signal_diagnostic_test_reset();
        check_event(state, &a, "A\0tail", 6);
        raw = raw_event(state, &a);
        if (raw) {
            int kind = rxvm_signal_diagnostic_test_state(raw);
            if (!budget) {
                expected_module = raw->attributes[1]->int_value;
                expected_address = raw->attributes[2]->int_value;
            }
            CHECK(state, raw->attributes[1]->int_value == expected_module);
            CHECK(state, raw->attributes[2]->int_value == expected_address);
            CHECK(state, kind == 3 || kind == 2);
            captured = kind == 2;
        }
        clear_value(&a);
        CHECK(state, rxvm_signal_diagnostic_test_live_owners() == initial);
        if (captured) break;
    }
    CHECK(state, captured && budget > 0);
    CHECK(state, call(state, context, "signal_diag.save_a", NULL, &a) == 0);
    raw = raw_event(state, &a);
    if (raw) {
        CHECK(state, rxvm_signal_diagnostic_test_state(raw) == 2);
        CHECK(state, rxvm_signal_diagnostic_test_count(raw) > 0);
    }
    rxvm_signal_diagnostic_test_fail_after(0);
    copy_value(&copied, &a);
    rxvm_signal_diagnostic_test_reset();
    check_event(state, &copied, "A\0tail", 6);
    raw = raw_event(state, &copied);
    if (raw) CHECK(state, rxvm_signal_diagnostic_test_state(raw) == 3);
    raw = raw_event(state, &a);
    if (raw) CHECK(state, rxvm_signal_diagnostic_test_state(raw) == 2);
    clear_value(&copied); clear_value(&a);
    rxvm_destroy(context);
    CHECK(state, rxvm_signal_diagnostic_test_live_owners() == initial);
}
#endif

#ifdef _WIN32
static DWORD WINAPI worker(void *argument) { lifecycle(argument); return 0; }
#else
static void *worker(void *argument) { lifecycle(argument); return NULL; }
#endif

int main(int argc, char **argv) {
    test_state state, workers[2];
    int i;
    if (argc != 3 && argc != 4) return 2;
    state.image = argv[1]; state.initializer = argv[2]; state.failures = 0;
#ifdef CREXX_VM_INSTRUMENTATION_TEST
    if (argc == 4 && !strcmp(argv[3], "unavailable")) {
        rxvm_context *context = new_context(&state);
        value result = {0};
        if (!context) return 1;
        rxvm_signal_diagnostic_test_fail_after(0);
        call_expected(&state, context, "signal_diag.unhandled", NULL, &result, RXSIGNAL_OTHER);
        rxvm_signal_diagnostic_test_reset();
        clear_value(&result);
        CHECK(&state, call(&state, context, "signal_diag.save_a", NULL, &result) == 0);
        check_event(&state, &result, "A\0tail", 6);
        clear_value(&result); rxvm_destroy(context);
        if (state.failures) return 1;
        puts("PASS: unavailable diagnostic preserves original signal and recovery");
        return 0;
    }
#endif
    if (argc != 3) return 2;
    lifecycle(&state);
#ifdef CREXX_VM_INSTRUMENTATION_TEST
    allocation_failures(&state);
#endif
    for (i = 0; i < 2; i++) workers[i] = state, workers[i].failures = 0;
#ifdef _WIN32
    {
        HANDLE threads[2];
        for (i = 0; i < 2; i++) threads[i] = CreateThread(NULL, 0, worker, &workers[i], 0, NULL);
        for (i = 0; i < 2; i++) {
            CHECK(&state, threads[i] != NULL);
            if (threads[i]) { WaitForSingleObject(threads[i], INFINITE); CloseHandle(threads[i]); }
        }
    }
#else
    {
        pthread_t threads[2]; int started[2];
        for (i = 0; i < 2; i++) started[i] = pthread_create(&threads[i], NULL, worker, &workers[i]);
        for (i = 0; i < 2; i++) {
            CHECK(&state, started[i] == 0);
            if (!started[i]) CHECK(&state, pthread_join(threads[i], NULL) == 0);
        }
    }
#endif
    for (i = 0; i < 2; i++) state.failures += workers[i].failures;
    if (state.failures) return 1;
    puts("PASS: signal diagnostic ownership, scoped initialization and worker reuse");
    return 0;
}
