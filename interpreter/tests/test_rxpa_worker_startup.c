/* S3-D01: real attached-worker startup, nested execution and callback rejection.
 * Reuse the RXPA catalogue fixtures and portable synchronization helpers. */
#define main rxpa_concurrency_main
#include "test_rxpa_concurrency.c"
#undef main
#include "rxvm.h"
#include "rxvmexecutor.h"
#include "rxvmprogram.h"

#define REQUIRE(condition) do { if (!(condition)) { \
    fprintf(stderr, "FAIL: startup line %d: %s\n", __LINE__, #condition); \
    exit(1); } } while (0)

static rxvm_context *startup_source;
static const rxvm_program_generation *startup_generation;
static int callback_rejected;

static void startup_from_callback(rxinteger argc, rxpa_attribute_value *argv,
                                  rxpa_attribute_value ret,
                                  rxpa_attribute_value signal) {
    rxvm_executor_result result;
    rxvm_executor *executor;
    (void)argc; (void)argv; (void)ret; (void)signal;
    executor = rxvm_executor_create_attached(startup_source->worker.runtime,
            startup_generation, NULL, 1, 2, &result);
    REQUIRE(executor == NULL && result == RXVM_EXECUTOR_WORKER_START_FAILED);
    callback_rejected++;
}

int main(int argc, char **argv) {
    rxvm_runtime *runtime, *wrong_runtime;
    rxvm_context *nested;
    rxvm_executor *executor;
    rxvm_executor_result result;
    proc_runtime *legacy;
    int callback_mode;
    REQUIRE(argc == 3);
    callback_mode = strcmp(argv[1], "callback") == 0;
    register_binding_probes();
    runtime = rxvm_runtime_create();
    REQUIRE(runtime != NULL);
    startup_source = rxvm_context_create_in_runtime(runtime);
    nested = rxvm_context_create_in_runtime(runtime);
    REQUIRE(startup_source && nested);
    REQUIRE(rxldmodp(startup_source) > 0);
    REQUIRE(rxvm_load_file(startup_source, argv[2]) != 0);
    REQUIRE(rxvm_program_generation_seal(startup_source, &startup_generation)
            == RXVM_PROGRAM_OK);
    legacy = context_find_procedure(startup_source, "e3b.binding_legacy");
    REQUIRE(legacy != NULL);

    /* A -> B -> A: B has no legacy provider, but A is still active twice.
     * Parking only the innermost execution_leave cannot quiesce A. */
    rxpa_compatibility_execution_enter(&startup_source->rxpa_compatibility);
    rxpa_compatibility_execution_enter(&nested->rxpa_compatibility);
    rxpa_compatibility_execution_enter(&startup_source->rxpa_compatibility);
    if (callback_mode) {
        legacy->start = (size_t)startup_from_callback;
        rxvm_call_native_procedure(legacy, 0, NULL, NULL, NULL);
        REQUIRE(callback_rejected == 1);
        /* Protected payload/initializer paths must reject even when the
         * active procedure is process-reentrant. */
        rxpa_compatibility_enter();
        startup_from_callback(0, NULL, NULL, NULL);
        rxpa_compatibility_leave();
        REQUIRE(callback_rejected == 2);
    }

    /* Fail after worker creation, then prove that execution was restored. */
    wrong_runtime = rxvm_runtime_create();
    REQUIRE(wrong_runtime != NULL);
    executor = rxvm_executor_create_attached(wrong_runtime, startup_generation,
                                             NULL, 1, 2, &result);
    REQUIRE(!executor && result == RXVM_EXECUTOR_WORKER_START_FAILED);
    REQUIRE(rxvm_runtime_destroy(wrong_runtime) == 0);

    /* Before S3-D01 this blocks: the worker needs the parent's cold transition. */
    executor = rxvm_executor_create_attached(runtime, startup_generation,
                                             NULL, 2, 4, &result);
    REQUIRE(executor && result == RXVM_EXECUTOR_OK);
    REQUIRE(startup_source->rxpa_compatibility.execution_depth == 2);
    REQUIRE(nested->rxpa_compatibility.execution_depth == 1);
    REQUIRE(legacy->native_invoker == rxvm_callfunc);
    REQUIRE(context_procedure_invoker(startup_source, "e3b.binding_reentrant")
            == rxvm_callfunc_direct);
    REQUIRE(rxvm_executor_destroy(executor) == 0);
    if (callback_mode) {
        /* Sticky locked mode also rejects a callback-held recursive lock. */
        rxvm_call_native_procedure(legacy, 0, NULL, NULL, NULL);
        REQUIRE(callback_rejected == 3);
    }

    rxpa_compatibility_execution_leave(&startup_source->rxpa_compatibility);
    rxpa_compatibility_execution_leave(&nested->rxpa_compatibility);
    rxpa_compatibility_execution_leave(&startup_source->rxpa_compatibility);
    /* A second startup after balanced leave/entry proves there is no stale
     * suspension state or thread-list member left by the failed operation. */
    rxpa_compatibility_execution_enter(&startup_source->rxpa_compatibility);
    executor = rxvm_executor_create_attached(runtime, startup_generation,
                                             NULL, 1, 2, &result);
    REQUIRE(executor && result == RXVM_EXECUTOR_OK);
    REQUIRE(rxvm_executor_destroy(executor) == 0);
    rxpa_compatibility_execution_leave(&startup_source->rxpa_compatibility);
    rxvm_destroy(nested);
    rxvm_destroy(startup_source);
    REQUIRE(rxvm_runtime_destroy(runtime) == 0);
    puts("PASS: RXPA attached worker startup, nested ownership and cleanup");
    return 0;
}
