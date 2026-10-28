/* cREXX License (MIT). Context console routes and native default restoration. */
#include "rxvml.h"
#include "crexxsaa.h"
#include <stdio.h>
#include <string.h>

typedef struct output_state {
    char output[1024], error[1024];
    size_t output_length, error_length;
    int calls, reject;
} output_state;

static int failures;
static output_state legacy;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); ++failures; } } while (0)

static int writer(int stream, const char *bytes, size_t length, void *userdata) {
    output_state *state = userdata;
    char *buffer = stream == RXVML_OUTPUT_STDOUT ? state->output : state->error;
    size_t *used = stream == RXVML_OUTPUT_STDOUT ? &state->output_length : &state->error_length;
    ++state->calls;
    CHECK(stream == RXVML_OUTPUT_STDOUT || stream == RXVML_OUTPUT_STDERR);
    if (state->reject) return 1;
    if (length > sizeof(state->output) - *used) return 1;
    if (length) memcpy(buffer + *used, bytes, length);
    *used += length;
    return 0;
}

static void legacy_writer(const char *bytes, size_t length) {
    (void)writer(RXVML_OUTPUT_STDOUT, bytes, length, &legacy);
}

static void clear_output(output_state *state) {
    memset(state, 0, sizeof(*state));
}

static int equal(const char *actual, size_t length, const char *expected, size_t expected_length) {
    return length == expected_length && (!length || !memcmp(actual, expected, length));
}

static int invoke(rxvml_context *ctx, const char *descriptor, size_t count, rxvml_value **args) {
    rxvml_value *result = NULL;
    rxinteger value = -999;
    int status = rxvml_call_procedure_descriptor(ctx, descriptor, count, args, &result);
    if (status) {
        const char *error = NULL;
        (void)rxvml_last_error(ctx, &error);
        fprintf(stderr, "Output fixture failed: %s\n", error ? error : "unknown");
    }
    CHECK(!status && result);
    if (result) CHECK(!rxvml_to_int(ctx, result, &value));
    rxvml_value_free(result);
    return (int)value;
}

static void say_text(rxvml_context *ctx, const char *bytes, size_t length) {
    rxvml_value *text = rxvml_value_new(ctx);
    CHECK(text && !rxvml_set_str(text, bytes, length));
    CHECK(!invoke(ctx, "rxsig1|context_output.saytext|.int|text=.string", 1, &text));
    rxvml_value_free(text);
}

static void text_write(rxvml_context *ctx, const char *target, const char *bytes) {
    rxvml_value *args[2] = {rxvml_value_new(ctx), rxvml_value_new(ctx)};
    CHECK(args[0] && args[1]);
    CHECK(!rxvml_set_str(args[0], target, strlen(target)));
    CHECK(!rxvml_set_str(args[1], bytes, strlen(bytes)));
    CHECK(!invoke(ctx, "rxsig1|context_output.textwrite|.int|target=.string,text=.string", 2, args));
    rxvml_value_free(args[0]);
    rxvml_value_free(args[1]);
}

static int address_output(rxvml_context *ctx, const rxvml_address_request *request,
                          rxvml_address_response *response, void *userdata) {
    (void)userdata;
    CHECK(!rxvml_address_emit_output(ctx, request, "ADDRESS_PRIVATE"));
    CHECK(!rxvml_address_emit_error(ctx, request, "ADDRESS_ERROR_PRIVATE"));
    response->rc = 0;
    return 0;
}

static rxvml_context *create_context(const char *library, const char *fixture) {
    rxvml_context *ctx = rxvml_create(NULL, 0);
    CHECK(ctx != NULL);
    if (ctx) {
        CHECK(rxvml_load_module_file(ctx, library) > 0);
        CHECK(rxvml_load_module_file(ctx, fixture) > 0);
    }
    return ctx;
}

int main(int argc, char **argv) {
    static const char exact[] = {'A', 0, (char)0xc3, (char)0xa9};
    static const char expected[] = {'A', 0, (char)0xc3, (char)0xa9, '\n'};
    char long_text[150];
    output_state a = {0}, b = {0}, facade = {0};
    rxvml_context *first, *second;
    crexxsaa_context *saa = NULL;
    rxvml_address_request request = {0};
    int kind, program_rc = -1;
    if (argc != 3) return 2;
    CHECK(rxvml_set_context_output_writer(NULL, writer, &a) == -1);
    CHECK(crexxsaa_set_output_writer(NULL, writer, &a) == -1);
    first = create_context(argv[1], argv[2]);
    second = create_context(argv[1], argv[2]);
    if (!first || !second) goto cleanup;
    CHECK(!rxvml_set_context_output_writer(first, writer, &a));
    CHECK(!rxvml_set_context_output_writer(second, writer, &b));
    say_text(first, exact, sizeof(exact));
    say_text(second, "SECOND_PRIVATE", 14);
    CHECK(equal(a.output, a.output_length, expected, sizeof(expected)));
    CHECK(equal(b.output, b.output_length, "SECOND_PRIVATE\n", 15));

    memset(long_text, 'x', sizeof(long_text));
    clear_output(&a);
    say_text(first, long_text, sizeof(long_text));
    CHECK(a.output_length == sizeof(long_text) + 1u);
    CHECK(!memcmp(a.output, long_text, sizeof(long_text)) &&
          a.output[sizeof(long_text)] == '\n');

    clear_output(&a);
    text_write(first, "stdout", "TEXT_PRIVATE");
    text_write(first, "stderr", "ERROR_PRIVATE");
    CHECK(!invoke(first, "rxsig1|context_output.codepoint|.int|", 0, NULL));
    CHECK(!invoke(first, "rxsig1|context_output.tracewrite|.int|", 0, NULL));
    CHECK(equal(a.output, a.output_length, "TEXT_PRIVATE", 12));
    CHECK(equal(a.error, a.error_length, "ERROR_PRIVATEZTRACE_PRIVATE\n", 28));

    clear_output(&a);
    rxvml_set_context_say_exit_bytes(first, legacy_writer);
    say_text(first, "LEGACY_PRIVATE", 14);
    CHECK(a.calls == 0 && equal(legacy.output, legacy.output_length, "LEGACY_PRIVATE\n", 15));
    rxvml_set_context_say_exit_bytes(first, NULL);

    a.reject = 1;
    for (kind = 0; kind < 3; ++kind) {
        rxvml_value *argument = rxvml_value_new(first);
        CHECK(argument != NULL);
        rxvml_set_int(argument, kind);
        CHECK(invoke(first, "rxsig1|context_output.failure|.int|kind=.int", 1, &argument) == 71);
        rxvml_value_free(argument);
    }
    CHECK(rxvml_address_emit_output(first, &request, "FAILED_PRIVATE") == -1);
    CHECK(rxvml_address_emit_error(first, &request, "FAILED_PRIVATE") == -1);
    CHECK(!rxvml_address_register_callback_environment(first, "CONSOLE", NULL, address_output, NULL, NULL));
    CHECK(!invoke(first, "rxsig1|context_output.capture|.int|", 0, NULL));

    clear_output(&a);
    CHECK(!rxvml_address_emit_output(first, &request, "ADDRESS_PRIVATE"));
    CHECK(!rxvml_address_emit_error(first, &request, "ADDRESS_ERROR_PRIVATE"));
    CHECK(equal(a.output, a.output_length, "ADDRESS_PRIVATE", 15));
    CHECK(equal(a.error, a.error_length, "ADDRESS_ERROR_PRIVATE", 21));
    clear_output(&a);
    CHECK(!invoke(first, "rxsig1|context_output.crexxwrite|.int|", 0, NULL));
    CHECK(equal(a.output, a.output_length, "CREXX_PRIVATE\n", 14));

    CHECK(!rxvml_set_context_output_writer(first, NULL, NULL));
    say_text(first, "DEFAULT_OUT", 11);
    text_write(first, "stderr", "DEFAULT_ERR\n");
    CHECK(a.calls > 0);
    CHECK(equal(a.output, a.output_length, "CREXX_PRIVATE\n", 14));
    clear_output(&b);
    say_text(second, "SECOND_PRIVATE", 14);
    CHECK(equal(b.output, b.output_length, "SECOND_PRIVATE\n", 15));

    CHECK(!crexxsaa_create(NULL, argv[1], &saa));
    if (saa) {
        CHECK(!crexxsaa_set_output_writer(saa, writer, &facade));
        CHECK(!crexxsaa_run_rxbin(saa, argv[2], 0, NULL, &program_rc));
        CHECK(program_rc == 0);
        CHECK(equal(facade.output, facade.output_length, "SAA_PRIVATE\n", 12));
        CHECK(equal(facade.error, facade.error_length, "SAA_ERROR_PRIVATE", 17));
        CHECK(!crexxsaa_set_output_writer(saa, NULL, NULL));
    }
cleanup:
    crexxsaa_destroy(saa);
    if (second) rxvml_destroy(second);
    if (first) rxvml_destroy(first);
    return failures ? 1 : 0;
}
