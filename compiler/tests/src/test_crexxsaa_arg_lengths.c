#include <stdio.h>
#include <string.h>

#include "crexxsaa.h"

#ifndef CREXX_TEST_LOCATION
#define CREXX_TEST_LOCATION "."
#endif
#ifndef CREXX_TEST_LIBRARY_RXBIN
#define CREXX_TEST_LIBRARY_RXBIN "library.rxbin"
#endif
#ifndef CREXX_TEST_RXC
#define CREXX_TEST_RXC "rxc"
#endif
#ifndef CREXX_TEST_RXAS
#define CREXX_TEST_RXAS "rxas"
#endif
#ifndef CREXX_TEST_IMPORT_DIR
#define CREXX_TEST_IMPORT_DIR "."
#endif
#ifndef CREXX_TEST_ARG_SOURCE
#define CREXX_TEST_ARG_SOURCE "crexxsaa_arg_length_hosted.crexx"
#endif
#ifndef CREXX_TEST_ARG_MODULE
#define CREXX_TEST_ARG_MODULE "crexxsaa_arg_length_hosted.rxbin"
#endif
#ifndef CREXX_TEST_ARG_CACHE
#define CREXX_TEST_ARG_CACHE "crexxsaa-arg-length-cache"
#endif

static int run_length_case(crexxsaa_context* ctx, const char* label,
                           const char* path, int source, unsigned flags,
                           const char* data, size_t length,
                           int expected_length) {
    const char* argv[] = {data};
    const size_t lengths[] = {length};
    int program_rc = -1;
    int api_rc = source
        ? crexxsaa_run_source_with_lengths(ctx, path, "arg-lengths", flags,
                                           1, argv, lengths, &program_rc)
        : crexxsaa_run_rxbin_with_lengths(ctx, path, 1, argv, lengths,
                                          &program_rc);
    if (api_rc != 0 || program_rc != expected_length) {
        fprintf(stderr, "%s: API rc=%d, program rc=%d, error=%s\n",
                label, api_rc, program_rc, crexxsaa_last_error(ctx));
        return 1;
    }
    return 0;
}

static int reject_length_case(crexxsaa_context* ctx, const char* label,
                              int source, const char* data,
                              const size_t* lengths, const char* error_part) {
    const char* argv[] = {data};
    int program_rc = -1;
    int api_rc = source
        ? crexxsaa_run_source_with_lengths(ctx, CREXX_TEST_ARG_SOURCE,
                                           "arg-lengths", 0, 1, argv, lengths,
                                           &program_rc)
        : crexxsaa_run_rxbin_with_lengths(ctx, CREXX_TEST_ARG_MODULE,
                                          1, argv, lengths, &program_rc);
    if (api_rc == 0 || program_rc != 0 ||
        !strstr(crexxsaa_last_error(ctx), error_part)) {
        fprintf(stderr, "%s: invalid input accepted or wrong error: rc=%d, program=%d, %s\n",
                label, api_rc, program_rc, crexxsaa_last_error(ctx));
        return 1;
    }
    return 0;
}

int main(void) {
    static const char nul_data[] = {'a', '\0', 'b'};
    static const char invalid_utf8[] = {'a', (char)0xff};
    static const size_t invalid_length[] = {sizeof(invalid_utf8)};
    static const size_t missing_data_length[] = {1};
    static const char* legacy_args[] = {"abcd"};
    crexxsaa_context* ctx = NULL;
    int program_rc = -1;
    int failures = 0;

    if (crexxsaa_create(CREXX_TEST_LOCATION, CREXX_TEST_LIBRARY_RXBIN,
                        &ctx) != 0 || !ctx) {
        fprintf(stderr, "failed to create CREXXSAA length context\n");
        return 1;
    }
    if (crexxsaa_set_compiler(ctx, CREXX_TEST_RXC, CREXX_TEST_RXAS,
                              CREXX_TEST_IMPORT_DIR) != 0 ||
        crexxsaa_set_cache_dir(ctx, CREXX_TEST_ARG_CACHE) != 0 ||
        crexxsaa_clear_cache(CREXX_TEST_ARG_CACHE) != 0) {
        fprintf(stderr, "failed to configure CREXXSAA length test: %s\n",
                crexxsaa_last_error(ctx));
        failures = 1;
        goto done;
    }

    failures += run_length_case(ctx, "uncached source NUL",
                                CREXX_TEST_ARG_SOURCE, 1,
                                CREXXSAA_CACHE_DISABLE, nul_data,
                                sizeof(nul_data), 3);
    failures += run_length_case(ctx, "source cache miss NUL",
                                CREXX_TEST_ARG_SOURCE, 1, 0, nul_data,
                                sizeof(nul_data), 3);
    failures += run_length_case(ctx, "source cache hit Unicode",
                                CREXX_TEST_ARG_SOURCE, 1, 0, "é🙂",
                                strlen("é🙂"), 2);
    failures += run_length_case(ctx, "direct RXBIN NUL",
                                CREXX_TEST_ARG_MODULE, 0, 0, nul_data,
                                sizeof(nul_data), 3);
    failures += reject_length_case(ctx, "source invalid UTF8", 1,
                                   invalid_utf8,
                                   invalid_length, "Invalid UTF-8");
    failures += reject_length_case(ctx, "RXBIN missing data", 0,
                                   NULL, missing_data_length,
                                   "Missing rxvml run argument data");
    failures += reject_length_case(ctx, "source missing lengths", 1,
                                   "abc", NULL,
                                   "Missing CREXXSAA run argument lengths");
    if (crexxsaa_run_source(ctx, CREXX_TEST_ARG_SOURCE, "arg-lengths", 0,
                            1, legacy_args, &program_rc) != 0 ||
        program_rc != 4) {
        fprintf(stderr, "legacy source run failed: program=%d, %s\n",
                program_rc, crexxsaa_last_error(ctx));
        failures++;
    }

done:
    if (crexxsaa_clear_cache(CREXX_TEST_ARG_CACHE) != 0) {
        fprintf(stderr, "failed to clear ARG length test cache\n");
        failures++;
    }
    crexxsaa_destroy(ctx);
    return failures ? 1 : 0;
}
