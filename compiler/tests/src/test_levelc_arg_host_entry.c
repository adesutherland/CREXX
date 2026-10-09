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

static int run_case_bytes(rxvml_context *ctx, int argc, const char **argv,
                          const size_t *lengths, const char *expected,
                          size_t expected_length) {
    const char *error = NULL;
    int program_rc = -1;
    int run_rc;
    memset(&capture, 0, sizeof(capture));
    run_rc = lengths
        ? rxvml_run_with_lengths(ctx, argc, argv, lengths, &program_rc)
        : rxvml_run(ctx, argc, argv, &program_rc);
    if (run_rc != 0) {
        rxvml_last_error(ctx, &error);
        fprintf(stderr, "rxvml_run failed: %s\n", error ? error : "unknown error");
        return 1;
    }
    if (program_rc != 0 || capture.overflow ||
        capture.length != expected_length ||
        memcmp(capture.bytes, expected, capture.length) != 0) {
        fprintf(stderr, "ARG host entry mismatch: program_rc=%d, overflow=%d, length=%zu\n",
                program_rc, capture.overflow, capture.length);
        return 1;
    }
    return 0;
}

static int run_case(rxvml_context *ctx, int argc, const char **argv,
                    const char *expected) {
    return run_case_bytes(ctx, argc, argv, NULL, expected, strlen(expected));
}

int main(void) {
    static const char *two_args[] = {"blue green", "tail"};
    static const char *unicode_args[] = {"é🙂", "ÿ"};
    static const char *one_arg[] = {"red"};
    static const char *empty_arg[] = {""};
    static const char nul_arg[] = {'a', '\0', 'b'};
    static const char *nul_args[] = {nul_arg};
    static const size_t nul_lengths[] = {sizeof(nul_arg)};
    static const char nul_expected[] =
        "count=1\nexists=1|0\nraw=a\0b|\nparsed=A\0B|\n";
    static const char invalid_utf8[] = {'a', (char)0xff};
    static const char *invalid_args[] = {invalid_utf8};
    static const size_t invalid_lengths[] = {sizeof(invalid_utf8)};
    static const char *missing_args[] = {NULL};
    rxvml_context *ctx = rxvml_create(CREXX_TEST_BINDIR, 0);
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
        run_case(ctx, 2, unicode_args,
                 "count=2\nexists=1|1\nraw=é🙂|ÿ\nparsed=É🙂|Ÿ\n") ||
        run_case(ctx, 1, one_arg,
                 "count=1\nexists=1|0\nraw=red|\nparsed=RED|\n") ||
        run_case(ctx, 1, empty_arg,
                 "count=1\nexists=1|0\nraw=|\nparsed=|\n") ||
        run_case_bytes(ctx, 1, nul_args, nul_lengths,
                       nul_expected, sizeof(nul_expected) - 1) ||
        run_case(ctx, 0, NULL,
                 "count=0\nexists=0|0\nraw=|\nparsed=|\n")) failed = 1;
    if (!failed) {
        const char *error = NULL;
        int program_rc = -1;
        if (rxvml_run_with_lengths(ctx, 1, invalid_args, invalid_lengths,
                                   &program_rc) == 0 ||
            rxvml_last_error(ctx, &error) == 0 || !error ||
            !strstr(error, "Invalid UTF-8")) {
            fprintf(stderr, "invalid UTF-8 host ARG was accepted\n");
            failed = 1;
        }
        error = NULL;
        if (rxvml_run_with_lengths(ctx, 1, missing_args, invalid_lengths,
                                   &program_rc) == 0 ||
            rxvml_last_error(ctx, &error) == 0 || !error ||
            !strstr(error, "Missing rxvml run argument data")) {
            fprintf(stderr, "missing host ARG data was accepted\n");
            failed = 1;
        }
        error = NULL;
        if (rxvml_run_with_lengths(ctx, 1, one_arg, NULL,
                                   &program_rc) == 0 ||
            rxvml_last_error(ctx, &error) == 0 || !error ||
            !strstr(error, "Missing rxvml run argument lengths")) {
            fprintf(stderr, "missing host ARG lengths were accepted\n");
            failed = 1;
        }
        if (run_case(ctx, 1, one_arg,
                     "count=1\nexists=1|0\nraw=red|\nparsed=RED|\n")) failed = 1;
    }
done:
    rxvml_destroy(ctx);
    return failed;
}
