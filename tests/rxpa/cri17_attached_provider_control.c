#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxvm.h"
#include "rxvmexecutor.h"
#include "rxvmintp.h"
#include "rxvmprogram.h"

static int failures;

#define CHECK(condition, message)                                             \
    do {                                                                      \
        if (!(condition)) {                                                   \
            fprintf(stderr, "FAIL: %s\n", (message));                        \
            failures++;                                                       \
        }                                                                     \
    } while (0)

static char *copy_string(const char *source) {
    size_t length = strlen(source);
    char *copy = (char *)malloc(length + 1u);
    if (copy) memcpy(copy, source, length + 1u);
    return copy;
}

int main(int argc, char **argv) {
    rxvm_runtime *runtime = 0;
    rxvm_context *source = 0;
    const rxvm_program_generation *generation = 0;
    rxvm_executor *executor = 0;
    rxvm_executor_result result = RXVM_EXECUTOR_INVALID;
    size_t leaks = 0u;

    if (argc != 5) {
        fprintf(stderr,
                "usage: %s PROGRAM_RXBIN PROVIDER_DIRECTORY LIBRARY_RXBIN "
                "CLASSLIB_RXBIN\n",
                argv[0]);
        return 2;
    }
    runtime = rxvm_runtime_create();
    CHECK(runtime != 0, "create CRI-17 shared runtime");
    if (!runtime) goto cleanup;
    source = rxvm_context_create_in_runtime(runtime);
    CHECK(source != 0, "create CRI-17 controller context");
    if (!source) goto cleanup;
    source->provider_location = copy_string(argv[2]);
    CHECK(source->provider_location != 0, "copy controller provider path");
    CHECK(rxvm_load_file(source, argv[3]) != 0,
          "load packaged standard library bytecode");
    CHECK(rxvm_load_file(source, argv[4]) != 0,
          "load packaged class library bytecode");
    CHECK(rxvm_load_file(source, argv[1]) != 0,
          "load CRI-17 provider-bearing bytecode");
    CHECK(rxldmodp(source) >= 0, "load controller static provider catalogue");
    CHECK(rxvm_link(source) == 0 && rxvm_prepare(source) == 0,
          "resolve and prepare controller provider");
    CHECK(rxvm_program_generation_seal(source, &generation) == RXVM_PROGRAM_OK &&
              generation != 0,
          "seal bytecode after the controller loaded a native provider");
    if (failures) goto cleanup;

    executor = rxvm_executor_create_attached(
            runtime, generation, "", 1u, 2u, &result);
    CHECK(executor == 0,
          "missing attached-worker provider path fails pool construction");
    CHECK(result == RXVM_EXECUTOR_PROVIDER_RESOLUTION_FAILED,
          "missing attached-worker provider reports provider resolution failure");
    if (executor) {
        (void)rxvm_executor_destroy(executor);
        executor = 0;
    }

    executor = rxvm_executor_create_attached(
            runtime, generation, argv[2], 2u, 2u, &result);
    CHECK(executor != 0 && result == RXVM_EXECUTOR_OK,
          "attached workers resolve the declared dynamic provider");
    CHECK(rxvm_call(source, "cri17_attached_test.provider_created", 0, 0) == 3,
          "controller and two workers own three distinct provider sessions");
    if (executor) {
        CHECK(rxvm_executor_destroy(executor) == 0u,
              "attached provider executor tears down without leaks");
        executor = 0;
    }
    CHECK(rxvm_call(source, "cri17_attached_test.provider_destroyed", 0, 0) == 2 &&
              rxvm_call(source, "cri17_attached_test.provider_live", 0, 0) == 1,
          "worker provider sessions are destroyed before controller teardown");

cleanup:
    if (executor) (void)rxvm_executor_destroy(executor);
    if (source) rxvm_destroy(source);
    if (runtime) {
        leaks = rxvm_runtime_destroy(runtime);
        CHECK(leaks == 0u, "CRI-17 shared runtime tears down without leaks");
    }
    if (failures) {
        fprintf(stderr, "CRI17_ATTACHED_PROVIDER result=FAIL failures=%d\n",
                failures);
        return 1;
    }
    printf("CRI17_ATTACHED_PROVIDER result=PASS\n");
    return 0;
}
