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
#include <limits.h>
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
    atomic_int *peer_abort;
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
        while (atomic_load(&concurrent_callbacks) < 2 &&
               !atomic_load(s->peer_abort) && waits++ < 10000u) {
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
    size_t field_end = prefix_size + sizeof(host.native_declaration);
    rxpa_host_services_v1 *prefix = malloc(prefix_size);
    memset(&host, 0, sizeof(host)); host.struct_size = sizeof(host);
    host.abi_version = RXPA_HOST_SERVICES_ABI_V1; host.native_declaration = unavailable_declaration;
    CHECK(rxpa_host_has_native_declaration(&host));
    CHECK(!rxpa_host_has_native_declaration(NULL));
    for (boundary = prefix_size; boundary < field_end; ++boundary) {
        host.struct_size = boundary; CHECK(!rxpa_host_has_native_declaration(&host));
    }
    host.struct_size = field_end; CHECK(rxpa_host_has_native_declaration(&host));
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
static int unavailable_resource_view(rxpa_attribute_value working_directory) {
    (void)working_directory; return -1;
}
static void check_optional_resource_tail(void) {
    rxpa_host_services_v1 host;
    size_t boundary, prefix_size = offsetof(rxpa_host_services_v1, resource_view);
    size_t field_end = prefix_size + sizeof(host.resource_view);
    rxpa_host_services_v1 *prefix = malloc(prefix_size);
    memset(&host, 0, sizeof(host)); host.struct_size = sizeof(host);
    host.abi_version = RXPA_HOST_SERVICES_ABI_V1; host.resource_view = unavailable_resource_view;
    CHECK(rxpa_host_has_resource_view(&host));
    CHECK(!rxpa_host_has_resource_view(NULL));
    CHECK(!rxpa_host_has_native_declaration(&host));
    for (boundary = prefix_size; boundary < field_end; ++boundary) {
        host.struct_size = boundary; CHECK(!rxpa_host_has_resource_view(&host));
    }
    host.struct_size = field_end; CHECK(rxpa_host_has_resource_view(&host));
    host.abi_version = RXPA_HOST_SERVICES_ABI_V1 + 1u;
    CHECK(!rxpa_host_has_resource_view(&host));
    host.abi_version = RXPA_HOST_SERVICES_ABI_V1;
    CHECK(prefix != NULL);
    if (prefix) {
        memcpy(prefix, &host, prefix_size); prefix->struct_size = prefix_size;
        CHECK(!rxpa_host_has_resource_view(prefix)); free(prefix);
    }
    host.resource_view = NULL; CHECK(!rxpa_host_has_resource_view(&host));
}
static void check_context_initialization(void) {
    rxvm_context context;
    int *empty_callback_signal = NULL;
    rxvm_captured_signal *empty_callback_capture = NULL;
    /* RXVML uses caller-allocated storage; defaults cannot depend on calloc. */
    memset(&context, 0xa5, sizeof(context));
    rxinimod(&context);
    CHECK(context.prepare_only == 0);
    CHECK(!memcmp(&context.active.callback_signal, &empty_callback_signal,
            sizeof(empty_callback_signal)));
    CHECK(!memcmp(&context.active.callback_capture, &empty_callback_capture,
            sizeof(empty_callback_capture)));
    rxfremod(&context);
}
typedef struct reader_image {
    char *bytes;
    size_t length;
    int modules;
} reader_image;

/* The independent count oracle never uses the legacy convenience cursor. */
static int read_counted_image(const char *path, reader_image *image) {
    FILE *file = fopen(path, "rb");
    rxbin_reader reader;
    module_file *module = NULL;
    char *cursor;
    long length;
    int rc;
    memset(image, 0, sizeof(*image));
    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET)) { fclose(file); return 0; }
    image->length = (size_t)length;
    image->bytes = malloc(image->length);
    if (!image->bytes || fread(image->bytes, 1, image->length, file) != image->length) {
        free(image->bytes); image->bytes = NULL; fclose(file); return 0;
    }
    fclose(file);
    cursor = image->bytes;
    rxbin_reader_init_mem(&reader, &cursor, image->bytes + image->length);
    while ((rc = rxbin_reader_next_module(&reader, &module)) == 0) {
        free_module(module); module = NULL;
        if (image->modules == INT_MAX) { rc = -1; break; }
        ++image->modules;
    }
    free_module(module);
    rxbin_reader_close(&reader);
    if (rc != 1 || !image->modules) {
        free(image->bytes); image->bytes = NULL; return 0;
    }
    return 1;
}

typedef struct concurrent_case {
    const char *library, *fixture;
    const reader_image *combined;
    int identity, memory, native_modules, library_modules;
    atomic_int *error_phase, *peer_abort;
} concurrent_case;

static int wait_reader_phase(concurrent_case *test, int phase) {
    unsigned waits = 0;
    while (atomic_load(test->error_phase) < phase &&
           !atomic_load(test->peer_abort) && waits++ < 10000u) {
#ifdef _WIN32
        Sleep(1);
#else
        struct timespec pause = {0, 1000000};
        (void)nanosleep(&pause, NULL);
#endif
    }
    return atomic_load(test->error_phase) >= phase;
}

/* Phased operations make error clobbering deterministic, without racing reads. */
static void check_reader_error_isolation(concurrent_case *test) {
    rxbin_reader reader;
    module_file *module = NULL;
    char *cursor;
    if (test->identity == 101) {
        char truncated[] = "cReXx007";
        const char *borrowed;
        char saved[512];
        cursor = truncated;
        rxbin_reader_init_mem(&reader, &cursor, truncated + 8);
        CHECK(rxbin_reader_next_module(&reader, &module) == -1 && !module);
        free_module(module); module = NULL;
        rxbin_reader_close(&reader);
        borrowed = rxbin_last_error();
        CHECK(borrowed && strstr(borrowed, "truncated header"));
        (void)snprintf(saved, sizeof(saved), "%s", borrowed ? borrowed : "");
        atomic_store(test->error_phase, 1);
        CHECK(wait_reader_phase(test, 2));
        CHECK(borrowed && strcmp(borrowed, saved) == 0 && rxbin_last_error() == borrowed);
        atomic_store(test->error_phase, 3);
    } else {
        if (!wait_reader_phase(test, 1)) {
            CHECK(0); atomic_store(test->peer_abort, 1); return;
        }
        cursor = test->combined->bytes;
        rxbin_reader_init_mem(&reader, &cursor, cursor + test->combined->length);
        CHECK(rxbin_reader_next_module(&reader, &module) == 0 && module);
        free_module(module);
        /* Early close must release the remaining, undelivered linked modules. */
        rxbin_reader_close(&reader);
        CHECK(rxbin_last_error() == NULL);
        atomic_store(test->error_phase, 2);
        CHECK(wait_reader_phase(test, 3));
    }
}

#ifdef _WIN32
static DWORD WINAPI independent_context(void *userdata) {
#else
static void *independent_context(void *userdata) {
#endif
    concurrent_case *test = userdata;
    state s;
    rxvml_context *ctx;
    rxvml_value *text, *name, *result, *args[2];
    int registration_rc, total;
    memset(&s, 0, sizeof(s)); s.concurrent = 1; s.identity = test->identity;
    s.peer_abort = test->peer_abort;
    if (!test->memory) check_reader_error_isolation(test);
    ctx = rxvml_create(NULL, 0); CHECK(ctx != NULL);
    if (!ctx) atomic_store(test->peer_abort, 1);
    if (ctx) {
        if (test->memory) {
            total = rxvml_load_module_buffer(ctx, test->combined->bytes, test->combined->length);
        } else {
            total = rxvml_load_module_file(ctx, test->library);
            CHECK(total == test->native_modules + test->library_modules);
            if (total != test->native_modules + test->library_modules)
                atomic_store(test->peer_abort, 1);
            total = rxvml_load_module_file(ctx, test->fixture);
        }
        CHECK(total == test->native_modules + test->combined->modules);
        if (total != test->native_modules + test->combined->modules)
            atomic_store(test->peer_abort, 1);
        registration_rc = rxvml_address_register_callback_environment_spans(
                ctx, "SPANS", "independent", command_callback, function_callback, &s);
        if (registration_rc) {
            const char *error = NULL;
            rxvml_last_error(ctx, &error);
            fprintf(stderr, "Independent context %d ADDRESS registration failed: %s\n",
                    test->identity, error ? error : "unknown");
        }
        CHECK(!registration_rc);
        if (registration_rc) atomic_store(test->peer_abort, 1);
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
    reader_image library_image = {0}, fixture_image = {0}, combined = {0};
    concurrent_case cases[2];
    rxvml_context *probe;
    atomic_int error_phase, peer_abort;
    int native_modules, loaded, mode;
    size_t index;
    CHECK(read_counted_image(library, &library_image));
    CHECK(read_counted_image(fixture, &fixture_image));
    if (!library_image.bytes || !fixture_image.bytes) goto done;
    CHECK(library_image.length <= SIZE_MAX - fixture_image.length &&
          library_image.modules <= INT_MAX - fixture_image.modules);
    if (library_image.length > SIZE_MAX - fixture_image.length ||
        library_image.modules > INT_MAX - fixture_image.modules) goto done;
    combined.length = library_image.length + fixture_image.length;
    combined.modules = library_image.modules + fixture_image.modules;
    combined.bytes = malloc(combined.length); CHECK(combined.bytes);
    if (!combined.bytes) goto done;
    memcpy(combined.bytes, library_image.bytes, library_image.length);
    memcpy(combined.bytes + library_image.length, fixture_image.bytes, fixture_image.length);
    /* Derive the native prefix without assuming a fixed bridge module count. */
    probe = rxvml_create(NULL, 0); CHECK(probe);
    if (!probe) goto done;
    loaded = rxvml_load_module_buffer(probe, fixture_image.bytes, fixture_image.length);
    native_modules = loaded - fixture_image.modules;
    CHECK(loaded > 0 && native_modules >= 0);
    rxvml_destroy(probe);
    if (loaded <= 0 || native_modules < 0) goto done;
    CHECK(combined.modules <= INT_MAX - native_modules);
    if (combined.modules > INT_MAX - native_modules) goto done;
    for (mode = 0; mode < 2; ++mode) {
        atomic_init(&error_phase, 0); atomic_init(&peer_abort, 0);
        atomic_store(&concurrent_callbacks, 0);
        for (index = 0; index < 2; ++index) {
            cases[index].library = library; cases[index].fixture = fixture;
            cases[index].combined = &combined; cases[index].identity = index ? 202 : 101;
            cases[index].memory = mode; cases[index].native_modules = native_modules;
            cases[index].library_modules = library_image.modules;
            cases[index].error_phase = &error_phase; cases[index].peer_abort = &peer_abort;
        }
#ifdef _WIN32
        HANDLE threads[2];
        for (index = 0; index < 2; ++index) { threads[index] = CreateThread(NULL, 0, independent_context, &cases[index], 0, NULL); CHECK(threads[index] != NULL); if (!threads[index]) atomic_store(&peer_abort, 1); }
        for (index = 0; index < 2; ++index) if (threads[index]) {
            DWORD waited = WaitForSingleObject(threads[index], 30000);
            CHECK(waited == WAIT_OBJECT_0);
            if (waited != WAIT_OBJECT_0) {
                atomic_store(&peer_abort, 1);
                /* Shared inputs cannot be freed while an owner still runs. */
                waited = WaitForSingleObject(threads[index], INFINITE);
                if (waited != WAIT_OBJECT_0) { CHECK(0); exit(1); }
            }
            CloseHandle(threads[index]);
        }
#else
        pthread_t threads[2]; int started[2];
        for (index = 0; index < 2; ++index) { started[index] = pthread_create(&threads[index], NULL, independent_context, &cases[index]); CHECK(!started[index]); if (started[index]) atomic_store(&peer_abort, 1); }
        for (index = 0; index < 2; ++index) if (!started[index]) {
            int joined = pthread_join(threads[index], NULL);
            CHECK(!joined);
            if (joined) exit(1);
        }
#endif
    }
done:
    free(combined.bytes); free(fixture_image.bytes); free(library_image.bytes);
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
static void check_captured_signal_diagnostics(const char *library, const char *fixture) {
    rxvml_context *first = rxvml_create(NULL, 0);
    rxvml_context *second = rxvml_create(NULL, 0);
    rxvml_value *first_name, *second_name, *result = NULL, *args[1];
    const char *first_error = NULL, *second_error = NULL;
    const char *location;
    char saved[512], saved_second[512];
    int first_module, second_module;
    long long module_number = -1, failure_pc = -1, missing_pc = -1;
    const char *descriptor = "rxsig1|rxvml_spans.forcedsignal|.int|name=.string";

    CHECK(first && second);
    if (!first || !second) {
        if (first) rxvml_destroy(first);
        if (second) rxvml_destroy(second);
        return;
    }
    CHECK(rxvml_load_module_file(first, library) > 0);
    first_module = rxvml_load_module_file(first, fixture); CHECK(first_module > 0);
    CHECK(rxvml_load_module_file(second, library) > 0);
    second_module = rxvml_load_module_file(second, fixture); CHECK(second_module > 0);
    first_name = rxvml_value_new(first); second_name = rxvml_value_new(second);
    CHECK(first_name && second_name);
    if (!first_name || !second_name) {
        if (first_name) rxvml_value_free(first_name);
        if (second_name) rxvml_value_free(second_name);
        rxvml_destroy(first); rxvml_destroy(second);
        return;
    }
    CHECK(!rxvml_set_str(first_name, "FAILURE", 7)); args[0] = first_name;
    CHECK(rxvml_call_procedure_descriptor(first, descriptor, 1, args, &result) == -1);
    CHECK(result == NULL && rxvml_last_error(first, &first_error) == -1);
    CHECK(first_error && strstr(first_error, "code=2 name=FAILURE") &&
          strstr(first_error, "procedure=rxvml_spans.forcedsignal") &&
          strstr(first_error, "message=forced RXVML diagnostic"));
    location = first_error ? strstr(first_error, " module=") : NULL;
    CHECK(location && sscanf(location, " module=%lld pc=%lld", &module_number, &failure_pc) == 2);
    CHECK(module_number == first_module && failure_pc >= 0);
    (void)snprintf(saved, sizeof(saved), "%s", first_error ? first_error : "");
    if (result) { rxvml_value_free(result); result = NULL; }

    CHECK(!rxvml_set_str(second_name, "OUT_OF_RANGE", 12)); args[0] = second_name;
    CHECK(rxvml_call_procedure_descriptor(second, descriptor, 1, args, &result) == -1);
    CHECK(result == NULL && rxvml_last_error(second, &second_error) == -1);
    CHECK(second_error && strstr(second_error, "code=8 name=OUT_OF_RANGE") &&
          strstr(second_error, "procedure=rxvml_spans.forcedsignal") &&
          strstr(second_error, "message=forced RXVML diagnostic"));
    module_number = -1;
    location = second_error ? strstr(second_error, " module=") : NULL;
    CHECK(location && sscanf(location, " module=%lld pc=%lld", &module_number, &missing_pc) == 2);
    CHECK(module_number == second_module && missing_pc >= 0);
    (void)snprintf(saved_second, sizeof(saved_second), "%s", second_error ? second_error : "");
    if (result) { rxvml_value_free(result); result = NULL; }
    CHECK(first_error && strcmp(first_error, saved) == 0);
    CHECK(rxvml_last_error(first, &first_error) == -1 && first_error && strcmp(first_error, saved) == 0);

    CHECK(!rxvml_set_str(first_name, "FUNCTION_NOT_FOUND", 18)); args[0] = first_name;
    CHECK(rxvml_call_procedure_descriptor(first, descriptor, 1, args, &result) == -1);
    CHECK(result == NULL && rxvml_last_error(first, &first_error) == -1);
    CHECK(first_error && strstr(first_error, "code=11 name=FUNCTION_NOT_FOUND") &&
          strstr(first_error, "procedure=rxvml_spans.forcedsignal") &&
          strstr(first_error, "message=rxvml_spans.missing_inner_callee"));
    module_number = -1; missing_pc = -1;
    location = first_error ? strstr(first_error, " module=") : NULL;
    CHECK(location && sscanf(location, " module=%lld pc=%lld", &module_number, &missing_pc) == 2);
    CHECK(module_number == first_module && missing_pc >= 0 && missing_pc != failure_pc);
    CHECK(second_error && strcmp(second_error, saved_second) == 0);
    if (result) { rxvml_value_free(result); result = NULL; }

    args[0] = first_name;
    result = call(first, "rxsig1|rxvml_spans.identity|.string|text=.string", 1, args);
    if (result) {
        check_text(first, result, "FUNCTION_NOT_FOUND", 18);
        rxvml_value_free(result);
    }
    rxvml_value_free(first_name); rxvml_value_free(second_name);
    rxvml_destroy(first); rxvml_destroy(second);
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
    check_captured_signal_diagnostics(argv[1], argv[2]);
    check_context_initialization();
    check_optional_native_tail();
    check_optional_resource_tail();
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
