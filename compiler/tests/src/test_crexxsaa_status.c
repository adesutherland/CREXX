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

#ifndef CREXX_TEST_CREXXSAA_EXIT7_SOURCE
#define CREXX_TEST_CREXXSAA_EXIT7_SOURCE "crexxsaa_exit7_hosted.crexx"
#endif

#ifndef CREXX_TEST_CREXXSAA_RETURN7_SOURCE
#define CREXX_TEST_CREXXSAA_RETURN7_SOURCE "crexxsaa_return7_hosted.crexx"
#endif

static int the_callback(
    const crexxsaa_address_request* request,
    crexxsaa_address_response* response,
    void* userdata) {

    (void)userdata;
    if (!request || !response) return -1;
    response->rc = strcmp(request->command ? request->command : "", "qquit") == 0 ? 0 : 99;
    response->condition_name = NULL;
    response->diagnostic = NULL;
    return 0;
}

static int run_source_case(const char* label, const char* source_path, int expected_program_rc) {
    crexxsaa_context* ctx = NULL;
    int program_rc = -1;
    int run_rc;
    int status = 1;

    if (crexxsaa_create(CREXX_TEST_LOCATION, CREXX_TEST_LIBRARY_RXBIN, &ctx) != 0 || !ctx) {
        fprintf(stderr, "%s: failed to create CREXXSAA context\n", label);
        return 1;
    }

    if (crexxsaa_set_compiler(ctx, CREXX_TEST_RXC, CREXX_TEST_RXAS, CREXX_TEST_IMPORT_DIR) != 0) {
        fprintf(stderr, "%s: failed to configure compiler: %s\n", label, crexxsaa_last_error(ctx));
        goto cleanup;
    }

    if (crexxsaa_register_address_environment(ctx, "THE", the_callback, NULL) != 0) {
        fprintf(stderr, "%s: failed to register THE environment: %s\n", label, crexxsaa_last_error(ctx));
        goto cleanup;
    }

    run_rc = crexxsaa_run_source(
        ctx,
        source_path,
        "status",
        CREXXSAA_CACHE_DISABLE,
        0,
        NULL,
        &program_rc);

    if (run_rc != 0) {
        fprintf(stderr, "%s: run failed with API rc %d: %s\n", label, run_rc, crexxsaa_last_error(ctx));
        goto cleanup;
    }

    if (program_rc != expected_program_rc) {
        fprintf(stderr, "%s: expected program rc %d, got %d\n", label, expected_program_rc, program_rc);
        goto cleanup;
    }

    status = 0;

cleanup:
    crexxsaa_destroy(ctx);
    return status;
}

int main(void) {
    int failures = 0;

    failures += run_source_case("top-level exit", CREXX_TEST_CREXXSAA_EXIT7_SOURCE, 7);
    failures += run_source_case("integer main return", CREXX_TEST_CREXXSAA_RETURN7_SOURCE, 7);

    return failures ? 1 : 0;
}
