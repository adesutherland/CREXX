/* Public reentry cannot borrow an initializer's internal call lineage. */
#include <stdio.h>
#include <string.h>

#include "rxvm.h"
#include "rxvmintp.h"
#include "rxvml.h"

static int failures;

#define CHECK(condition, message) do {                                        \
    if (!(condition)) {                                                       \
        fprintf(stderr, "FAIL: %s\n", (message));                             \
        failures++;                                                            \
    }                                                                         \
} while (0)

typedef struct lineage_probe {
    rxvm_context *context;
    module *owner;
    int writes;
} lineage_probe;

static int probe_writer(int stream, const char *bytes, size_t length,
                        void *userdata) {
    lineage_probe *probe = userdata;
    proc_runtime *saved_proc = probe->context->ext_proc;
    value *saved_ret = probe->context->ext_ret;

    CHECK(stream == RXVML_OUTPUT_STDOUT, "initializer writes stdout");
    CHECK(length == 5u && !memcmp(bytes, "probe", 5u),
          "initializer reaches reentry checkpoint");
    CHECK(probe->writes++ == 0, "initializer checkpoint runs exactly once");
    CHECK(probe->owner->initializer_state == RXVM_INIT_INITIALIZING,
          "module is still initializing at callback");
    CHECK(probe->owner->globals[0]->int_value == 1,
          "internal helper ran before public reentry");
    CHECK(rxvm_call(probe->context, "lineage.helper", 0, NULL) != 0,
          "public reentry to initializing module rejects");
    CHECK(probe->owner->globals[0]->int_value == 1,
          "rejected public reentry has no body effect");
    CHECK(probe->context->ext_proc == saved_proc &&
          probe->context->ext_ret == saved_ret,
          "rejected public reentry preserves outer trampoline");
    return 0;
}

int main(int argc, char **argv) {
    rxvm_context *context;
    lineage_probe probe;

    if (argc != 2) return 2;
    context = rxvm_create();
    CHECK(context != NULL, "create VM context");
    if (!context) return 1;
    CHECK(rxvm_load_file(context, argv[1]) != 0, "load lineage fixture");
    if (context->num_modules != 1u) {
        CHECK(context->num_modules == 1u, "one module loaded");
        rxvm_destroy(context);
        return 1;
    }
    memset(&probe, 0, sizeof(probe));
    probe.context = context;
    probe.owner = context->modules[0];
    context->active.output_writer = probe_writer;
    context->active.output_userdata = &probe;
    CHECK(rxvm_initialize(context) == 0,
          "initializer continues after rejected public reentry");
    CHECK(probe.writes == 1, "callback ran once");
    CHECK(probe.owner->initializer_state == RXVM_INIT_READY,
          "initializer publishes READY after successful completion");
    CHECK(probe.owner->globals[0]->int_value == 2,
          "second internal helper ran after callback");
    CHECK(rxvm_initialize(context) == 0,
          "repeated initialization succeeds without rerunning body");
    CHECK(probe.owner->globals[0]->int_value == 2 && probe.writes == 1,
          "repeated initialization leaves completed state unchanged");
    CHECK(rxvm_call(context, "lineage.helper", 0, NULL) == 0,
          "public helper entry succeeds after READY");
    CHECK(probe.owner->globals[0]->int_value == 3,
          "post-READY public call executes once");
    context->active.output_writer = NULL;
    context->active.output_userdata = NULL;
    rxvm_destroy(context);
    if (failures) return 1;
    puts("PASS: initializer lineage and public reentry");
    return 0;
}
