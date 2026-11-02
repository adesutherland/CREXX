/* cREXX License (MIT). Private host injection into recycled a0 versus
 * counted-call capture. Ordinary retired frames have no such payload. */
#include "rxvm.h"
#include "rxvmintp.h"
#include "rxvml.h"
#include <stdio.h>
#include <string.h>

static int failures, finalizers;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); ++failures; \
} } while (0)

typedef struct count_payload { value *caller_count; } count_payload;

static void finalize_count(void *item) {
    const rxvm_native_payload_ops *ops = NULL;
    count_payload payload;
    size_t length = 0;
    void *bytes = rxvml_get_native_payload(item, &length, &ops, NULL);
    CHECK(bytes && length == sizeof(payload));
    if (!bytes || length != sizeof(payload)) return;
    memcpy(&payload, bytes, sizeof(payload));
    CHECK(payload.caller_count != NULL);
    if (payload.caller_count) rxvml_set_int((rxvml_value *)payload.caller_count, 0);
    ++finalizers;
}
static const rxvm_native_payload_ops count_ops = {
    "call-count-snapshot.payload", NULL, finalize_count
};

static proc_runtime *find_target(module *owner) {
    size_t index;
    for (index = 0; index < owner->procedure_count; index++)
        if (!strcmp(owner->procedures[index].name, "target")) return &owner->procedures[index];
    return NULL;
}

typedef struct call_probe {
    module *owner;
    proc_runtime *target;
    stack_frame *recycled; /* Borrowed only while the bytecode root is live. */
    int writes;
    int writer_active;
} call_probe;

/* A leaf host callback, not interpreter reentry. Both checkpoints occur in
 * one run: root return drains the recycler before rxvm_call returns to C. */
static int probe_writer(int stream, const char *bytes, size_t length, void *userdata) {
    call_probe *probe = userdata;
    size_t a0_index = (size_t)probe->target->locals + (size_t)probe->owner->segment.globals;
    CHECK(!probe->writer_active);
    probe->writer_active = 1;
    CHECK(stream == RXVML_OUTPUT_STDOUT);
    if (probe->writes == 0 && length == 3 && !memcmp(bytes, "arm", 3)) {
        count_payload payload;
        probe->recycled = *probe->target->frame_free_list;
        CHECK(probe->recycled != NULL);
        if (probe->recycled) {
            CHECK(probe->recycled->number_args == 1);
            CHECK(probe->recycled->baselocals[a0_index + 1] == probe->owner->globals[1]);
            payload.caller_count = probe->owner->globals[0];
            CHECK(rxvml_set_native_payload((rxvml_value *)probe->recycled->baselocals[a0_index],
                        &payload, sizeof(payload), &count_ops, 0) == 0);
        }
    } else if (probe->writes == 1 && length == 6 && !memcmp(bytes, "verify", 6)) {
        CHECK(probe->recycled && *probe->target->frame_free_list == probe->recycled);
        if (probe->recycled) {
            CHECK(probe->recycled->number_args == 1);
            CHECK(probe->recycled->baselocals[a0_index + 1] == probe->owner->globals[4]);
        }
    } else CHECK(0);
    ++probe->writes;
    probe->writer_active = 0;
    return 0;
}

static void check_case(const char *path, const char *entry, int unused_result) {
    rxvm_context *context = rxvm_create();
    module *owner;
    proc_runtime *target;
    call_probe probe;
    int status;
    CHECK(context != NULL);
    if (!context) return;
    CHECK(rxvm_load_file(context, (char *)path) != 0);
    CHECK(rxvm_initialize(context) == 0);
    if (context->num_modules != 1u) { CHECK(context->num_modules == 1u); rxvm_destroy(context); return; }
    owner = context->modules[0]; target = find_target(owner);
    CHECK(target && target->frame_free_list);
    if (!target || !target->frame_free_list) { rxvm_destroy(context); return; }
    rxvml_set_int((rxvml_value *)owner->globals[0], 1);
    rxvml_set_int((rxvml_value *)owner->globals[1], 17);
    rxvml_set_int((rxvml_value *)owner->globals[4], 29);
    rxvml_set_int((rxvml_value *)owner->globals[5], unused_result);
    rxvml_set_int((rxvml_value *)owner->globals[6], (rxinteger)(uintptr_t)target);
    memset(&probe, 0, sizeof(probe)); probe.owner = owner; probe.target = target;
    context->active.output_writer = probe_writer;
    context->active.output_userdata = &probe;
    finalizers = 0;
    status = rxvm_call(context, (char *)entry, 0, NULL);
    if (status) fprintf(stderr, "count capture %s status=%d\n", entry, status);
    CHECK(status == 0);
    CHECK(probe.writes == 2 && !probe.writer_active);
    /* run() has drained all recycled frames. Keep only detached value/count
     * assertions here, never dereference the in-run frame witness. */
    probe.recycled = NULL;
    CHECK(finalizers == 1);
    CHECK(owner->globals[0]->int_value == 0);
    CHECK(owner->globals[2]->int_value == 1);
    CHECK(owner->globals[3]->int_value == 29);
    CHECK(owner->globals[7]->int_value == (unused_result ? 77 : 29));
    CHECK(*target->frame_free_list == NULL);
    context->active.output_writer = NULL;
    context->active.output_userdata = NULL;
    rxvm_destroy(context);
    CHECK(finalizers == 1);
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    check_case(argv[1], "callcapture.direct", 0);
    check_case(argv[1], "callcapture.direct", 1);
    check_case(argv[1], "callcapture.dynamic", 0);
    check_case(argv[1], "callcapture.dynamic", 1);
    check_case(argv[1], "callcapture.fixed", 0);
    printf("COUNTED_CALL_SNAPSHOT|failures=%d\n", failures);
    return failures ? 1 : 0;
}
