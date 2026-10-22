/* cREXX License (MIT). Exact ADDRESS spans and execution-owner interrupts. */
#include "rxvml.h"
#include "rxvmintp.h"
#include "rxaddress_layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdatomic.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

/* The owning exact view accepts empty VM text without materializing a terminator. */
int rxvm_string_view(rxpa_attribute_value, const char **, size_t *);
static atomic_int failures;
static atomic_int concurrent_callbacks;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); ++failures; } } while (0)
static const char input[] = {'a', 0, 'b'};
static const char output[] = {'x', 0, 'y'};
static const char diagnostic[] = {'d', 0, 'g'};
static rxvml_context *signal_context;
static volatile sig_atomic_t signal_result;
typedef struct state {
    rxvml_context *other;
    int releases, command_calls, function_calls, foreign_result;
    int identity, concurrent;
    rxvml_address_binding_spans update;
} state;
static int bytes(rxvml_text_span span, const char *data, size_t length) {
    return span.length == length && (!length || !memcmp(span.data, data, length));
}
static void release_response(void *userdata) { ++((state *)userdata)->releases; }
static void check_text(rxvml_context *ctx, rxvml_value *value, const char *data, size_t length) {
    const char *actual = NULL; size_t actual_length = 0;
    (void)ctx;
    CHECK(value && !rxvm_string_view((rxpa_attribute_value)value, &actual, &actual_length));
    CHECK(actual_length == length && (!length || (actual && !memcmp(actual, data, length))));
}
static void check_attribute_text(rxvml_context *ctx, rxvml_value *object, size_t index,
        const char *data, size_t length) {
    rxvml_value *value = rxvml_get_attribute(ctx, object, index);
    check_text(ctx, value, data, length); rxvml_value_free(value);
}
static rxinteger attribute_int(rxvml_context *ctx, rxvml_value *object, size_t index) {
    rxinteger number = -999;
    rxvml_value *value = rxvml_get_attribute(ctx, object, index);
    CHECK(value && !rxvml_to_int(ctx, value, &number)); rxvml_value_free(value); return number;
}
static void check_diagnostic(rxvml_context *ctx, rxvml_value *object) {
    rxvml_value *list = rxvml_get_attribute(ctx, object, 3);
    CHECK(attribute_int(ctx, object, 2) == 1);
    check_attribute_text(ctx, list, 1, diagnostic, sizeof(diagnostic)); rxvml_value_free(list);
}
static rxvml_value *call(rxvml_context *ctx, const char *descriptor, size_t count, rxvml_value **args) {
    rxvml_value *result = NULL;
    int status = rxvml_call_procedure_descriptor(ctx, descriptor, count, args, &result);
    if (status) { const char *error = NULL; rxvml_last_error(ctx, &error); fprintf(stderr, "Call failed: %s\n", error ? error : "unknown"); }
    CHECK(!status && result); return result;
}
static int command_callback(rxvml_context *ctx, const rxvml_address_request_spans *request,
        rxvml_address_response_spans *response, void *userdata) {
    state *s = userdata; rxvml_value *text, *nested, *args[1];
    ++s->command_calls;
    if (s->concurrent) {
        unsigned int waits = 0;
        atomic_fetch_add(&concurrent_callbacks, 1);
        while (atomic_load(&concurrent_callbacks) < 2 && waits++ < 10000u) {
#ifdef _WIN32
            Sleep(1);
#else
            struct timespec pause = {0, 1000000};
            (void)nanosleep(&pause, NULL);
#endif
        }
        CHECK(atomic_load(&concurrent_callbacks) == 2);
    }
    CHECK(request->struct_size == sizeof(*request));
    CHECK(!strcmp(request->environment_name, "SPANS"));
    CHECK(bytes(request->command, input, sizeof(input)));
    CHECK(request->binding_count == 1);
    CHECK(!strcmp(request->bindings[0].internal_name, "buffer"));
    CHECK(bytes(request->bindings[0].value, input, sizeof(input)));
    text = rxvml_value_new(ctx); CHECK(text && !rxvml_set_str(text, output, sizeof(output))); args[0] = text;
    nested = call(ctx, "rxsig1|rxvml_spans.identity|.string|text=.string", 1, args);
    check_text(ctx, nested, output, sizeof(output)); rxvml_value_free(nested); rxvml_value_free(text);
    CHECK(bytes(request->command, input, sizeof(input)));
    CHECK(bytes(request->bindings[0].value, input, sizeof(input)));
    memset(&s->update, 0, sizeof(s->update));
    s->update.kind = "var"; s->update.internal_name = "buffer"; s->update.external_alias = "alias";
    s->update.flags = ""; s->update.value.data = output; s->update.value.length = sizeof(output);
    response->updated_binding_count = 1; response->updated_bindings = &s->update;
    response->diagnostic.data = diagnostic; response->diagnostic.length = sizeof(diagnostic);
    response->release = release_response; response->release_userdata = s;
    response->rc = 12 + s->identity;
    return 0;
}
#ifdef _WIN32
static DWORD WINAPI foreign_interrupt(void *userdata) {
#else
static void *foreign_interrupt(void *userdata) {
#endif
    state *s = userdata;
    s->foreign_result = rxvml_request_interrupt(signal_context);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}
#ifndef _WIN32
static void owner_signal(int number) {
    (void)number; signal_result = rxvml_request_interrupt(signal_context);
}
#endif
static int function_callback(rxvml_context *ctx, const rxvml_address_function_request_spans *request,
        rxvml_address_function_response_spans *response, void *userdata) {
    state *s = userdata;
    ++s->function_calls;
    response->rc = s->identity;
    CHECK(request->struct_size == sizeof(*request));
    CHECK(request->argc == 1);
    response->release = release_response; response->release_userdata = s;
    if (!strcmp(request->function_name, "interrupt") || !strcmp(request->function_name, "osinterrupt")) {
        rxvm_context *active = rxvm_active_context_current();
        CHECK(active && active->active.pending_interrupts);
        CHECK(rxvml_request_interrupt(s->other) == -1);
        signal_context = ctx;
#ifdef _WIN32
        { HANDLE thread = CreateThread(NULL, 0, foreign_interrupt, s, 0, NULL);
          CHECK(thread != NULL); if (thread) { CHECK(WaitForSingleObject(thread, 10000) == WAIT_OBJECT_0); CloseHandle(thread); } }
#else
        { pthread_t thread; int status = pthread_create(&thread, NULL, foreign_interrupt, s);
          CHECK(!status); if (!status) CHECK(!pthread_join(thread, NULL)); }
#endif
        CHECK(s->foreign_result == -1);
        CHECK(!(*active->active.pending_interrupts & rxsignal_mask(RXSIGNAL_POSIX_INT)));
#ifndef _WIN32
        if (!strcmp(request->function_name, "osinterrupt")) {
            struct sigaction action, previous;
            memset(&action, 0, sizeof(action)); action.sa_handler = owner_signal; sigemptyset(&action.sa_mask);
            CHECK(!sigaction(SIGUSR1, &action, &previous));
            signal_result = -99;
            CHECK(!pthread_kill(pthread_self(), SIGUSR1));
            CHECK(signal_result == 0);
            CHECK(!sigaction(SIGUSR1, &previous, NULL));
        } else
#endif
        CHECK(!rxvml_request_interrupt(ctx));
        CHECK(*active->active.pending_interrupts & rxsignal_mask(RXSIGNAL_POSIX_INT));
        return 0;
    }
    CHECK(bytes(request->args[0], input, sizeof(input)));
    response->has_result = strcmp(request->function_name, "absent") != 0;
    if (!strcmp(request->function_name, "invalid")) {
        response->result.data = NULL; response->result.length = 1;
    } else if (!strcmp(request->function_name, "limit")) {
        response->result.data = output; response->result.length = (size_t)-1;
    } else if (strcmp(request->function_name, "empty")) {
        response->result.data = output; response->result.length = sizeof(output);
    }
    response->diagnostic.data = diagnostic; response->diagnostic.length = sizeof(diagnostic);
    return 0;
}
static int legacy_function(rxvml_context *ctx, const rxvml_address_function_request *request,
        rxvml_address_function_response *response, void *userdata) {
    (void)ctx; (void)request; (void)userdata; response->result = ""; return 0;
}
static int unavailable_declaration(const char *artifact, const char *callable,
        size_t bytes_limit, size_t work_limit, rxpa_native_declaration_v1 *declaration,
        const char **error, size_t *bytes_used, size_t *work_used) {
    (void)artifact; (void)callable; (void)bytes_limit; (void)work_limit;
    (void)declaration; (void)error; (void)bytes_used; (void)work_used; return 4;
}
static void check_optional_native_tail(void) {
    rxpa_host_services_v1 host;
    size_t boundary, prefix_size = offsetof(rxpa_host_services_v1, native_declaration);
    rxpa_host_services_v1 *prefix = malloc(prefix_size);
    memset(&host, 0, sizeof(host)); host.struct_size = sizeof(host);
    host.abi_version = RXPA_HOST_SERVICES_ABI_V1; host.native_declaration = unavailable_declaration;
    CHECK(rxpa_host_has_native_declaration(&host));
    CHECK(!rxpa_host_has_native_declaration(NULL));
    for (boundary = prefix_size; boundary < sizeof(host); ++boundary) {
        host.struct_size = boundary; CHECK(!rxpa_host_has_native_declaration(&host));
    }
    host.struct_size = sizeof(host); host.abi_version = RXPA_HOST_SERVICES_ABI_V1 + 1u;
    CHECK(!rxpa_host_has_native_declaration(&host));
    host.abi_version = RXPA_HOST_SERVICES_ABI_V1;
    CHECK(prefix != NULL);
    if (prefix) {
        memcpy(prefix, &host, prefix_size); prefix->struct_size = prefix_size;
        CHECK(!rxpa_host_has_native_declaration(prefix)); free(prefix);
    }
    host.native_declaration = NULL; CHECK(!rxpa_host_has_native_declaration(&host));
}
typedef struct concurrent_case { const char *library, *fixture; int identity; } concurrent_case;
#ifdef _WIN32
static DWORD WINAPI independent_context(void *userdata) {
#else
static void *independent_context(void *userdata) {
#endif
    concurrent_case *test = userdata;
    state s;
    rxvml_context *ctx;
    rxvml_value *text, *name, *result, *args[2];
    memset(&s, 0, sizeof(s)); s.concurrent = 1; s.identity = test->identity;
    ctx = rxvml_create(NULL, 0); CHECK(ctx != NULL);
    if (ctx) {
        CHECK(rxvml_load_module_file(ctx, test->library) > 0);
        CHECK(rxvml_load_module_file(ctx, test->fixture) > 0);
        CHECK(!rxvml_address_register_callback_environment_spans(ctx, "SPANS", "independent", command_callback, function_callback, &s));
        text = rxvml_value_new(ctx); name = rxvml_value_new(ctx);
        CHECK(!rxvml_set_str(text, input, sizeof(input))); CHECK(!rxvml_set_str(name, "result", 6));
        args[0] = text; args[1] = text;
        result = call(ctx, "rxsig1|rxvml_spans.command|._rxsysb..addressresponse|text=.string,binding=.string", 2, args);
        if (result) { CHECK(attribute_int(ctx, result, 4) == 12 + test->identity); rxvml_value_free(result); }
        args[0] = name;
        result = call(ctx, "rxsig1|rxvml_spans.function|._rxsysb..addressfunctionresponse|name=.string,text=.string", 2, args);
        if (result) {
            CHECK(attribute_int(ctx, result, 4) == test->identity);
            check_attribute_text(ctx, result, 5, output, sizeof(output)); rxvml_value_free(result);
        }
        CHECK(s.command_calls == 1 && s.function_calls == 1 && s.releases == 2);
        rxvml_value_free(text); rxvml_value_free(name); rxvml_destroy(ctx);
    }
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}
static void check_independent_contexts(const char *library, const char *fixture) {
    concurrent_case cases[2] = {{library, fixture, 101}, {library, fixture, 202}};
    size_t index;
#ifdef _WIN32
    HANDLE threads[2];
    for (index = 0; index < 2; ++index) { threads[index] = CreateThread(NULL, 0, independent_context, &cases[index], 0, NULL); CHECK(threads[index] != NULL); }
    for (index = 0; index < 2; ++index) if (threads[index]) { CHECK(WaitForSingleObject(threads[index], 30000) == WAIT_OBJECT_0); CloseHandle(threads[index]); }
#else
    pthread_t threads[2]; int started[2];
    for (index = 0; index < 2; ++index) { started[index] = pthread_create(&threads[index], NULL, independent_context, &cases[index]); CHECK(!started[index]); }
    for (index = 0; index < 2; ++index) if (!started[index]) CHECK(!pthread_join(threads[index], NULL));
#endif
}
static int halt_command(rxvml_context *ctx, const rxvml_address_request_spans *request,
        rxvml_address_response_spans *response, void *userdata) {
    int *calls = userdata;
    ++*calls;
    CHECK(bytes(request->command, "halt", 4));
    CHECK(!rxvml_request_interrupt(ctx));
    response->rc = 0;
    return 0;
}
static void check_compiled_halt(const char *image) {
    char *provider_root, *separator, *backslash;
    size_t length = strlen(image);
    rxvml_context *ctx;
    int calls = 0, program_rc = -1, run_rc;
    provider_root = malloc(length + sizeof("./providers")); CHECK(provider_root != NULL);
    if (!provider_root) return;
    memcpy(provider_root, image, length + 1u);
    separator = strrchr(provider_root, '/'); backslash = strrchr(provider_root, '\\');
    if (backslash && (!separator || backslash > separator)) separator = backslash;
    if (separator) separator[1] = 0;
    else { provider_root[0] = '.'; provider_root[1] = '/'; provider_root[2] = 0; }
    strcat(provider_root, "providers");
    ctx = rxvml_create(provider_root, 0); free(provider_root); CHECK(ctx != NULL);
    if (!ctx) return;
    CHECK(rxvml_load_module_file(ctx, image) > 0);
    CHECK(!rxvml_address_register_callback_environment_spans(ctx, "SPANS", "classic-halt", halt_command, NULL, &calls));
    run_rc = rxvml_run(ctx, 0, NULL, &program_rc);
    if (run_rc) { const char *error = NULL; rxvml_last_error(ctx, &error); fprintf(stderr, "Classic HALT run failed: %s\n", error ? error : "unknown"); }
    CHECK(!run_rc && program_rc == 71 && calls == 1);
    CHECK(rxvml_request_interrupt(ctx) == -1);
    rxvml_destroy(ctx);
}
int main(int argc, char **argv) {
    state s; rxvml_context *ctx;
    rxvml_value *text, *name, *result, *args[2], *list, *binding;
    size_t index;
    const char *names[] = {"result", "empty", "absent", "invalid", "limit"};
    const char *function_descriptor = "rxsig1|rxvml_spans.function|._rxsysb..addressfunctionresponse|name=.string,text=.string";
    if (argc == 4) {
        check_compiled_halt(argv[3]);
        if (!failures) puts("PASS: compiled Classic HALT ingress");
        return failures ? 1 : 0;
    }
    if (argc != 3) return 2;
    check_optional_native_tail();
    memset(&s, 0, sizeof(s)); ctx = rxvml_create(NULL, 0); s.other = rxvml_create(NULL, 0);
    CHECK(ctx && s.other); if (!ctx || !s.other) return 1;
    CHECK(rxvml_request_interrupt(ctx) == -1);
    CHECK(rxvml_load_module_file(ctx, argv[1]) > 0 && rxvml_load_module_file(ctx, argv[2]) > 0);
    CHECK(!rxvml_address_register_callback_environment_spans(ctx, "SPANS", "span-test", command_callback, function_callback, &s));
    text = rxvml_value_new(ctx); name = rxvml_value_new(ctx);
    CHECK(!rxvml_set_str(text, input, sizeof(input))); args[0] = text; args[1] = text;
    result = call(ctx, "rxsig1|rxvml_spans.command|._rxsysb..addressresponse|text=.string,binding=.string", 2, args);
    if (result) {
        CHECK(attribute_int(ctx, result, 4) == 12); check_diagnostic(ctx, result);
        CHECK(attribute_int(ctx, result, 5) == 1); list = rxvml_get_attribute(ctx, result, 6);
        binding = rxvml_get_attribute(ctx, list, 1); check_attribute_text(ctx, binding, 5, output, sizeof(output));
        rxvml_value_free(binding); rxvml_value_free(list); rxvml_value_free(result);
    }
    args[0] = name; args[1] = text;
    for (index = 0; index < sizeof(names) / sizeof(*names); ++index) {
        CHECK(!rxvml_set_str(name, names[index], strlen(names[index])));
        if (index >= 3) {
            result = NULL;
            CHECK(rxvml_call_procedure_descriptor(ctx, function_descriptor, 2, args, &result) != 0);
            CHECK(result == NULL);
        } else {
            result = call(ctx, function_descriptor, 2, args);
            if (result) {
                CHECK(attribute_int(ctx, result, 6) == (index != 2));
                check_attribute_text(ctx, result, 5, index ? "" : output, index ? 0 : sizeof(output));
                check_diagnostic(ctx, result); rxvml_value_free(result);
            }
        }
        CHECK(s.releases == (int)index + 2);
    }
    for (index = 0; index < 2; ++index) {
#ifdef _WIN32
        if (index) break;
#endif
        const char *kind = index ? "osinterrupt" : "interrupt";
        CHECK(!rxvml_set_str(name, kind, strlen(kind)));
        result = call(ctx, "rxsig1|rxvml_spans.interrupt|.int|name=.string", 1, args);
        if (result) { rxinteger number = 0; CHECK(!rxvml_to_int(ctx, result, &number) && number == 71); rxvml_value_free(result); }
    }
    result = call(ctx, "rxsig1|rxvml_spans.clocks|.int|", 0, NULL);
    if (result) { rxinteger number = -1; CHECK(!rxvml_to_int(ctx, result, &number) && !number); rxvml_value_free(result); }
    /* Re-registration uses the same owner and restores legacy always-present behavior. */
    CHECK(!rxvml_address_register_callback_environment(ctx, "SPANS", "legacy-test", NULL, legacy_function, NULL));
    CHECK(!rxvml_set_str(name, "legacy", 6)); result = call(ctx, function_descriptor, 2, args);
    if (result) { CHECK(attribute_int(ctx, result, 6) == 1); check_attribute_text(ctx, result, 5, "", 0); rxvml_value_free(result); }
    CHECK(rxvml_request_interrupt(ctx) == -1);
    CHECK(s.command_calls == 1);
    rxvml_value_free(name); rxvml_value_free(text); rxvml_destroy(s.other); rxvml_destroy(ctx);
    check_independent_contexts(argv[1], argv[2]);
    if (!failures) puts("PASS: exact ADDRESS spans and owner interrupts");
    return failures ? 1 : 0;
}
