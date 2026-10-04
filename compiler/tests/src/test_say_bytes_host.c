#include <stdio.h>
#include <string.h>

#include "rxvml.h"

#ifndef CREXX_TEST_LIBRARY_PATH
#define CREXX_TEST_LIBRARY_PATH "library"
#endif
#ifndef CREXX_TEST_SAY_BYTES_MODULE
#define CREXX_TEST_SAY_BYTES_MODULE "say_bytes_host.rxbin"
#endif

typedef struct say_capture {
    char bytes[64];
    size_t length;
    int calls;
} say_capture;

static say_capture first;
static say_capture second;

static void capture(say_capture *target, const char *text, size_t length) {
    target->calls++;
    if (length > sizeof(target->bytes) - target->length) return;
    memcpy(target->bytes + target->length, text, length);
    target->length += length;
}

static void first_bytes(const char *text, size_t length) {
    capture(&first, text, length);
}

static void second_bytes(const char *text, size_t length) {
    capture(&second, text, length);
}

static int emit(rxvml_context *ctx, const char *text, size_t length) {
    rxvml_value *argument = rxvml_value_new(ctx);
    rxvml_value *arguments[1];
    rxvml_value *result = NULL;
    int status;
    if (!argument || rxvml_set_str(argument, text, length) != 0) {
        if (argument) rxvml_value_free(argument);
        return -1;
    }
    arguments[0] = argument;
    status = rxvml_call_procedure_descriptor(
        ctx, "rxsig1|say_bytes_host.emit|.void|text=.string",
        1, arguments, &result);
    if (result) rxvml_value_free(result);
    rxvml_value_free(argument);
    return status;
}

static rxvml_context *loaded_context(void) {
    rxvml_context *ctx = rxvml_create(NULL, 0);
    if (!ctx) return NULL;
    if (rxvml_load_module_file(ctx, CREXX_TEST_LIBRARY_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_SAY_BYTES_MODULE) <= 0) {
        rxvml_destroy(ctx);
        return NULL;
    }
    return ctx;
}

int main(void) {
    const char with_nul[] = {'A', 0, 'B'};
    const char utf8[] = {'\xC3', '\xA9'};
    const char expected_first[] = {'A', 0, 'B', '\n'};
    const char expected_second[] = {'\xC3', '\xA9', '\n'};
    rxvml_context *a = loaded_context();
    rxvml_context *b = loaded_context();
    int failed = 0;

    if (!a || !b) {
        fprintf(stderr, "could not load SAY host fixture\n");
        failed = 1;
        goto done;
    }
    rxvml_set_context_say_exit_bytes(a, first_bytes);
    rxvml_set_context_say_exit_bytes(b, second_bytes);
    if (emit(a, with_nul, sizeof(with_nul)) != 0 ||
        emit(b, utf8, sizeof(utf8)) != 0 ||
        first.calls != 1 || first.length != sizeof(expected_first) ||
        memcmp(first.bytes, expected_first, sizeof(expected_first)) != 0 ||
        second.calls != 1 || second.length != sizeof(expected_second) ||
        memcmp(second.bytes, expected_second, sizeof(expected_second)) != 0) {
        fprintf(stderr, "length-aware callbacks lost bytes or crossed contexts\n");
        failed = 1;
        goto done;
    }

done:
    if (a) rxvml_destroy(a);
    if (b) rxvml_destroy(b);
    return failed;
}
