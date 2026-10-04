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
#ifndef CREXX_TEST_ARG_MODULE
#define CREXX_TEST_ARG_MODULE "levelc_arg_host_entry.rxbin"
#endif

static struct {
    char bytes[512];
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

static int run_case(rxvml_context *ctx, int argc, const char **argv,
                    const char *expected) {
    const char *error = NULL;
    int program_rc = -1;
    memset(&capture, 0, sizeof(capture));
    if (rxvml_run(ctx, argc, argv, &program_rc) != 0) {
        rxvml_last_error(ctx, &error);
        fprintf(stderr, "rxvml_run failed: %s\n", error ? error : "unknown error");
        return 1;
    }
    if (program_rc != 0 || capture.overflow ||
        capture.length != strlen(expected) ||
        memcmp(capture.bytes, expected, capture.length) != 0) {
        fprintf(stderr, "ARG host entry mismatch: program_rc=%d, overflow=%d, length=%zu\n",
                program_rc, capture.overflow, capture.length);
        return 1;
    }
    return 0;
}

int main(void) {
    static const char *two_args[] = {"blue green", "tail"};
    static const char *one_arg[] = {"red"};
    static const char *empty_arg[] = {""};
    rxvml_context *ctx = rxvml_create(NULL, 0);
    int failed = 0;
    if (!ctx) {
        fprintf(stderr, "could not create ARG host context\n");
        return 1;
    }
    if (rxvml_load_module_file(ctx, CREXX_TEST_LIBRARY_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_CLASSLIB_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_RXFNSC_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_ARG_MODULE) <= 0) {
        const char *error = NULL;
        rxvml_last_error(ctx, &error);
        fprintf(stderr, "could not load ARG host fixture: %s\n",
                error ? error : "unknown error");
        failed = 1;
        goto done;
    }
    rxvml_set_context_say_exit_bytes(ctx, capture_say);
    if (run_case(ctx, 2, two_args,
                 "count=2\nexists=1|1\nraw=blue green|tail\nparsed=BLUE GREEN|TAIL\n") ||
        run_case(ctx, 1, one_arg,
                 "count=1\nexists=1|0\nraw=red|\nparsed=RED|\n") ||
        run_case(ctx, 1, empty_arg,
                 "count=1\nexists=1|0\nraw=|\nparsed=|\n") ||
        run_case(ctx, 0, NULL,
                 "count=0\nexists=0|0\nraw=|\nparsed=|\n")) failed = 1;
done:
    rxvml_destroy(ctx);
    return failed;
}
