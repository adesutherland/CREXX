#include <stdio.h>
#include <string.h>

#include "rxvml.h"

#ifndef CREXX_TEST_LIBRARY_PATH
#define CREXX_TEST_LIBRARY_PATH "library"
#endif
#ifndef CREXX_TEST_CLASSLIB_PATH
#define CREXX_TEST_CLASSLIB_PATH "classlib"
#endif
#ifndef CREXX_TEST_RXFNSC_PATH
#define CREXX_TEST_RXFNSC_PATH "rxfnsc"
#endif
#ifndef CREXX_TEST_SAY_MODULE
#define CREXX_TEST_SAY_MODULE "levelc_say_host_output.rxbin"
#endif

static struct {
    char bytes[128];
    size_t length;
    int calls;
    int overflow;
} capture;

static void capture_say(const char *text, size_t length) {
    capture.calls++;
    if (length > sizeof(capture.bytes) - capture.length) {
        capture.overflow = 1;
        return;
    }
    memcpy(capture.bytes + capture.length, text, length);
    capture.length += length;
}

int main(void) {
    static const char expected[] =
        "A\0B\xC2\x80\xC3\xBF\n"
        "\xE6\xBC\xA2\xF0\x9F\x99\x82\n"
        "\xC2\x80\xC3\xBF\n"
        "\xC3\x83\xC2\xA9\n"
        "\xC3\xA9\n"
        "\n";
    const char *error = NULL;
    int program_rc = -1;
    int failed = 0;
    rxvml_context *ctx = rxvml_create(NULL, 0);
    if (!ctx) {
        fprintf(stderr, "could not create SAY host context\n");
        return 1;
    }
    if (rxvml_load_module_file(ctx, CREXX_TEST_LIBRARY_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_CLASSLIB_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_RXFNSC_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_SAY_MODULE) <= 0) {
        rxvml_last_error(ctx, &error);
        fprintf(stderr, "could not load SAY host fixture: %s\n",
                error ? error : "unknown error");
        failed = 1;
        goto done;
    }
    rxvml_set_context_say_exit_bytes(ctx, capture_say);
    if (rxvml_run(ctx, 0, NULL, &program_rc) != 0) {
        rxvml_last_error(ctx, &error);
        fprintf(stderr, "SAY host run failed: %s\n", error ? error : "unknown error");
        failed = 1;
        goto done;
    }
    if (program_rc != 0 || capture.overflow || capture.calls != 6 ||
        capture.length != sizeof(expected) - 1 ||
        memcmp(capture.bytes, expected, sizeof(expected) - 1) != 0) {
        fprintf(stderr, "Level C SAY host bytes differ: rc=%d calls=%d length=%zu\n",
                program_rc, capture.calls, capture.length);
        failed = 1;
    }
done:
    rxvml_destroy(ctx);
    return failed;
}
