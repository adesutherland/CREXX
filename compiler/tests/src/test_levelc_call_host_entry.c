#include <stdio.h>
#include <string.h>

#include "rxvml.h"

#ifndef CREXX_TEST_BINDIR
#define CREXX_TEST_BINDIR "."
#endif

#ifndef CREXX_TEST_LIBRARY_PATH
#define CREXX_TEST_LIBRARY_PATH "library"
#endif
#ifndef CREXX_TEST_CLASSLIB_PATH
#define CREXX_TEST_CLASSLIB_PATH "classlib"
#endif
#ifndef CREXX_TEST_RXFNSC_PATH
#define CREXX_TEST_RXFNSC_PATH "rxfnsc"
#endif
#ifndef CREXX_TEST_CALL_PROVIDER
#define CREXX_TEST_CALL_PROVIDER "levelc_call_host_provider.rxbin"
#endif
#ifndef CREXX_TEST_CALL_ENTRY
#define CREXX_TEST_CALL_ENTRY "levelc_call_host_entry.rxbin"
#endif

static struct {
    char bytes[256];
    size_t length;
    int overflow;
} capture;

static void capture_say(const char *text, size_t length) {
    if (length > sizeof(capture.bytes) - capture.length) {
        capture.overflow = 1;
        return;
    }
    memcpy(capture.bytes + capture.length, text, length);
    capture.length += length;
}

static int run_case(rxvml_context *ctx, const char *arg, size_t arg_length,
                    const char *expected, size_t expected_length) {
    const char *argv[] = {arg};
    const size_t lengths[] = {arg_length};
    const char *error = NULL;
    int program_rc = -1;
    memset(&capture, 0, sizeof(capture));
    if (rxvml_run_with_lengths(ctx, 1, argv, lengths, &program_rc) != 0) {
        rxvml_last_error(ctx, &error);
        fprintf(stderr, "Level C CALL host run failed: %s\n",
                error ? error : "unknown error");
        return 1;
    }
    if (program_rc != 0 || capture.overflow ||
        capture.length != expected_length ||
        memcmp(capture.bytes, expected, expected_length) != 0) {
        fprintf(stderr, "Level C CALL host result mismatch: rc=%d, overflow=%d, length=%zu\n",
                program_rc, capture.overflow, capture.length);
        return 1;
    }
    return 0;
}

int main(void) {
    static const char embedded_nul[] = {'a', '\0', 'b'};
    static const char expected_nul[] =
        "provider=A\0B\ncaller=A\0B:ok\n";
    rxvml_context *ctx = rxvml_create(CREXX_TEST_BINDIR, 0);
    const char *error = NULL;
    int failed = 0;
    if (!ctx) return 1;
    if (rxvml_load_module_file(ctx, CREXX_TEST_LIBRARY_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_CLASSLIB_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_RXFNSC_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_CALL_PROVIDER) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_CALL_ENTRY) <= 0) {
        rxvml_last_error(ctx, &error);
        fprintf(stderr, "Level C CALL host modules failed to load: %s\n",
                error ? error : "unknown error");
        failed = 1;
    } else {
        rxvml_set_context_say_exit_bytes(ctx, capture_say);
        failed = run_case(ctx, "é🙂", strlen("é🙂"),
                          "provider=É🙂\ncaller=É🙂:ok\n",
                          strlen("provider=É🙂\ncaller=É🙂:ok\n")) ||
                 run_case(ctx, embedded_nul, sizeof(embedded_nul),
                          expected_nul, sizeof(expected_nul) - 1);
    }
    rxvml_destroy(ctx);
    return failed;
}
