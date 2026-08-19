#include <stdio.h>
#include <stdlib.h>

#include "rxvm.h"
#include "rxvmintp.h"

static int failures;

#define CHECK(condition, message)                                             \
    do {                                                                      \
        if (!(condition)) {                                                   \
            fprintf(stderr, "FAIL: %s\n", (message));                       \
            failures++;                                                       \
        }                                                                     \
    } while (0)

static rxvm_context *new_context(void) {
    rxvm_context *context = rxvm_create();

    CHECK(context != 0, "create VM context");
    return context;
}

static void load(rxvm_context *context, const char *path) {
    CHECK(context && rxvm_load_file(context, (char *)path) != 0,
          "load initializer fixture");
}

static int call_int(rxvm_context *context, const char *name) {
    return rxvm_call(context, (char *)name, 0, 0);
}

static void check_dependency_order(const char *root_path,
                                   const char *dependency_path) {
    rxvm_context *context = new_context();

    if (!context) return;
    load(context, root_path);
    load(context, dependency_path);
    CHECK(context->num_modules == 2u, "load two independent modules");
    CHECK(context->modules[0]->initializer_state == RXVM_INIT_UNINITIALIZED &&
              context->modules[1]->initializer_state == RXVM_INIT_UNINITIALIZED,
          "loading and linking do not run initializers");
    CHECK(rxvm_link(context) == 0, "link initializer dependencies");
    CHECK(rxvm_prepare(context) == 0, "prepare initializer dependencies");
    CHECK(context->modules[0]->initializer_state == RXVM_INIT_UNINITIALIZED &&
              context->modules[1]->initializer_state == RXVM_INIT_UNINITIALIZED,
          "preparation does not run initializers");
    CHECK(rxvm_initialize(context) == 0, "initialize dependency graph");
    CHECK(call_int(context, "initroot.value") == 345,
          "root observes dependency initialization before its own body");
    CHECK(call_int(context, "initdep.value") == 345,
          "dependency initializers retain declaration order");
    CHECK(rxvm_initialize(context) == 0, "repeat initialization is idempotent");
    CHECK(call_int(context, "initdep.value") == 345,
          "repeat initialization does not rerun module initializers");
    rxvm_destroy(context);
}

static void check_linked_image(const char *linked_path) {
    rxvm_context *context = new_context();

    if (!context) return;
    load(context, linked_path);
    CHECK(context->num_modules == 2u,
          "linked image preserves individual module ownership");
    CHECK(rxvm_initialize(context) == 0, "initialize linked image");
    CHECK(call_int(context, "initroot.value") == 345,
          "linked image preserves every initializer");
    CHECK(call_int(context, "initdep.value") == 345,
          "linked initializer order is stable");
    rxvm_destroy(context);
}

static void check_late_load(const char *dependency_path,
                            const char *late_path) {
    rxvm_context *context = new_context();

    if (!context) return;
    load(context, dependency_path);
    CHECK(rxvm_initialize(context) == 0, "initialize original module set");
    CHECK(call_int(context, "initdep.value") == 34,
          "original module has initialized state");
    load(context, late_path);
    CHECK(context->modules[1]->initializer_state == RXVM_INIT_UNINITIALIZED,
          "late-loaded module begins uninitialized");
    CHECK(call_int(context, "initlate.value") == 7,
          "first execution after a late load initializes the new module");
    CHECK(call_int(context, "initdep.value") == 34,
          "late load does not rerun an existing module initializer");
    rxvm_destroy(context);
}

static void check_cycle_failure(const char *a_path, const char *b_path) {
    rxvm_context *context = new_context();

    if (!context) return;
    load(context, a_path);
    load(context, b_path);
    CHECK(rxvm_initialize(context) != 0, "initializer cycle fails");
    CHECK(context->modules[0]->initializer_state == RXVM_INIT_FAILED &&
              context->modules[1]->initializer_state == RXVM_INIT_FAILED,
          "every module in the observed cycle is failed");
    CHECK(call_int(context, "initcyclea.ping") != 0,
          "a procedure in a failed module cannot be called");
    CHECK(call_int(context, "initcycleb.ping") != 0,
          "the other failed module cannot be called either");
    CHECK(rxvm_initialize(context) != 0,
          "failed initializers are not retried automatically");
    CHECK(context->modules[0]->initializer_state == RXVM_INIT_FAILED &&
              context->modules[1]->initializer_state == RXVM_INIT_FAILED,
          "failed initializer state is stable");
    rxvm_destroy(context);
}

static void check_signal_failure(const char *failure_path) {
    rxvm_context *context = new_context();

    if (!context) return;
    load(context, failure_path);
    CHECK(rxvm_initialize(context) != 0,
          "an unhandled initializer signal fails initialization");
    CHECK(context->modules[0]->initializer_state == RXVM_INIT_FAILED,
          "an unhandled signal poisons the mutable module instance");
    CHECK(call_int(context, "initfail.ping") != 0,
          "a procedure in the signal-failed module cannot be called");
    CHECK(rxvm_initialize(context) != 0,
          "a signal-failed initializer is not retried");
    rxvm_destroy(context);
}

static void check_late_failure_isolation(const char *dependency_path,
                                         const char *a_path,
                                         const char *b_path) {
    rxvm_context *context = new_context();

    if (!context) return;
    load(context, dependency_path);
    CHECK(rxvm_initialize(context) == 0, "initialize published module prefix");
    CHECK(call_int(context, "initdep.value") == 34,
          "published module is callable before late load");
    load(context, a_path);
    load(context, b_path);
    CHECK(rxvm_initialize(context) != 0,
          "a cyclic late-loaded module set fails initialization");
    CHECK(context->initialized_module_count == 1u,
          "failed late initialization does not publish new modules");
    CHECK(call_int(context, "initdep.value") == 34,
          "previously published module remains callable after late failure");
    CHECK(call_int(context, "initcyclea.ping") != 0,
          "failed late-loaded module remains uncallable");
    rxvm_destroy(context);
}

int main(int argc, char **argv) {
    if (argc != 8) {
        fprintf(stderr,
                "usage: %s root dependency linked late cycle-a cycle-b failure\n",
                argv[0]);
        return 2;
    }

    check_dependency_order(argv[1], argv[2]);
    check_dependency_order(argv[1], argv[2]);
    check_linked_image(argv[3]);
    check_late_load(argv[2], argv[4]);
    check_cycle_failure(argv[5], argv[6]);
    check_late_failure_isolation(argv[2], argv[5], argv[6]);
    check_signal_failure(argv[7]);

    if (failures) {
        fprintf(stderr, "FAIL: %d module initializer checks\n", failures);
        return 1;
    }
    puts("PASS: module initializer lifecycle");
    return 0;
}
