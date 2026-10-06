#include <stdio.h>

#include "rxvml.h"

#ifndef CREXX_TEST_BINDIR
#define CREXX_TEST_BINDIR "."
#endif
#ifndef CREXX_TEST_LIBRARY_PATH
#define CREXX_TEST_LIBRARY_PATH "library.rxbin"
#endif
#ifndef CREXX_TEST_CLASSLIB_PATH
#define CREXX_TEST_CLASSLIB_PATH "classlib.rxbin"
#endif
#ifndef CREXX_TEST_RXFNSC_PATH
#define CREXX_TEST_RXFNSC_PATH "rxfnsc.rxbin"
#endif
#ifndef CREXX_TEST_COMPLETION_MODULE
#define CREXX_TEST_COMPLETION_MODULE "levelc_return_status7.rxbin"
#endif
#ifndef CREXX_TEST_INSTRUCTION
#define CREXX_TEST_INSTRUCTION "RETURN"
#endif

int main(void) {
    rxvml_context *ctx = rxvml_create(CREXX_TEST_BINDIR, 0);
    const char *error = NULL;
    int program_rc = -1;
    int failed = 1;
    if (!ctx) return 1;
    if (rxvml_load_module_file(ctx, CREXX_TEST_LIBRARY_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_CLASSLIB_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_RXFNSC_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_COMPLETION_MODULE) <= 0) {
        rxvml_last_error(ctx, &error);
        fprintf(stderr, "%s host module load failed: %s\n",
                CREXX_TEST_INSTRUCTION, error ? error : "unknown error");
        goto cleanup;
    }
    if (rxvml_run(ctx, 0, NULL, &program_rc) != 0) {
        rxvml_last_error(ctx, &error);
        fprintf(stderr, "%s host run failed: %s\n",
                CREXX_TEST_INSTRUCTION, error ? error : "unknown error");
        goto cleanup;
    }
    if (program_rc != 7) {
        fprintf(stderr, "%s host expected status 7, got %d\n",
                CREXX_TEST_INSTRUCTION, program_rc);
    } else {
        failed = 0;
    }

cleanup:
    rxvml_destroy(ctx);
    return failed;
}
