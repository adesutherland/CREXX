/* cREXX License (MIT). Logical RETURN assignment and real-frame retirement. */
#include "rxvm.h"
#include "rxvmintp.h"
#include "rxvml.h"
#include "rxvmvars.h"
#include "rxastree.h"
#include <stdio.h>
#include <string.h>

void rxvm_addfunc(rxpa_libfunc, char *, char *, char *, char *);

static int failures, copies[8], finalizers[8];
static rxvm_context *fixture_context;
static stack_frame *witness;
static int seeds, checkpoints, unwinding;
static int hook_reference;

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); ++failures; \
} } while (0)

static void copy_payload(void *, void *);
static void finalize_payload(void *);
static const rxvm_native_payload_ops payload_ops = {
    "return-retirement.payload", copy_payload, finalize_payload
};

static unsigned int payload_id(value *item) {
    size_t length = 0;
    const rxvm_native_payload_ops *ops = NULL;
    unsigned int id = 0;
    void *bytes = get_native_payload(item, &length, &ops, NULL);
    CHECK(bytes && ops == &payload_ops && length == sizeof(id));
    if (bytes && ops == &payload_ops && length == sizeof(id)) memcpy(&id, bytes, sizeof(id));
    CHECK(id > 0 && id < 8);
    return id < 8 ? id : 0;
}

static void mark(value *item, unsigned int id) {
    CHECK(set_native_payload(item, &id, sizeof(id), &payload_ops, 0) == 0);
}

static void copy_payload(void *destination, void *source) {
    unsigned int id = payload_id(source);
    ++copies[id];
    mark(destination, id);
    if (hook_reference) {
        rxvm_reference_cell *cell = rxvm_reference_identity_for_context(
                &fixture_context->references, fixture_context->modules[0]->globals[8],
                RXVM_REF_GLOBAL, fixture_context->modules[0], 0, "native COPY target");
        CHECK(cell != NULL);
        if (cell) rxvm_reference_value_set_payload(destination, cell);
    }
}

static void finalize_payload(void *item) {
    ++finalizers[payload_id(item)];
}

/* Private host injection into actual active frame backing. This makes inactive
 * capacity and a0 finalizers independently observable without compiler choices. */
static void seed(rxinteger count, rxpa_attribute_value *arguments,
                 rxpa_attribute_value result, rxpa_attribute_value signal) {
    rxvm_diagnostic_callout_scope *scope = fixture_context->active.diagnostic_callout;
    stack_frame *frame = scope ? scope->frame : NULL;
    value *owned, *a0, *borrowed;
    size_t a0_index;
    (void)arguments;
    CHECK(count == 0 && frame && frame->parent);
    if (!frame) { set_int(signal, RXSIGNAL_FAILURE); return; }
    CHECK(!strcmp(frame->procedure->name, "target"));
    if (seeds) CHECK(frame == witness);
    witness = frame;
    CHECK(rxvm_private_binding_owner(frame->procedure)->inlined_private_count == 1);
    owned = frame->baselocals[0];
    a0_index = (size_t)frame->procedure->locals +
               (size_t)frame->procedure->binarySpace->globals;
    a0 = frame->baselocals[a0_index];
    borrowed = frame->baselocals[a0_index + 1];
    CHECK(a0->int_value == 1 && borrowed == fixture_context->modules[0]->globals[0]);
    CHECK(frame->baselocals[8] == fixture_context->modules[0]->globals[9]);
    CHECK(payload_id(borrowed) == 6 && payload_id(frame->baselocals[8]) == 7);

    set_string(owned, "preserved", 9);
    owned->int_value = 41;
    owned->float_value = 2.5;
    set_num_attributes(owned, 3);
    set_num_attributes(owned, 1);
    CHECK(owned->max_num_attributes > owned->num_attributes);
    mark(owned, 1);
    mark(owned->unlinked_attributes[0], 2);
    set_num_attributes(owned->unlinked_attributes[0], 1);
    mark(owned->unlinked_attributes[0]->unlinked_attributes[0], 3);
    mark(owned->unlinked_attributes[owned->max_num_attributes - 1], 4);
    mark(a0, 5);
    owned->attributes[0] = borrowed;
    ++seeds;
    set_int(result, 0);
    set_int(signal, 0);
}

/* Called while the root still keeps the retired frame/recycler alive. */
static int writer(int stream, const char *bytes, size_t length, void *userdata) {
    value *owned, *result;
    size_t a0_index, id;
    (void)userdata;
    CHECK(stream == RXVML_OUTPUT_STDOUT && length == 7 && !memcmp(bytes, "retired", 7));
    CHECK(witness && seeds == checkpoints + 1);
    if (!witness) return 0;
    CHECK(*witness->procedure->frame_free_list == witness);
    owned = witness->baselocals[0];
    result = fixture_context->modules[0]->globals[1];
    a0_index = (size_t)witness->procedure->locals +
               (size_t)witness->procedure->binarySpace->globals;
    CHECK(owned->native_payload_ops == NULL && owned->int_value == 41 && owned->float_value == 2.5);
    CHECK(owned->string_length == 9 && !memcmp(owned->string_value, "preserved", 9));
    CHECK(owned->num_attributes == 1 && owned->attributes[0] == fixture_context->modules[0]->globals[0]);
    CHECK(owned->unlinked_attributes[0]->native_payload_ops == NULL);
    CHECK(owned->unlinked_attributes[0]->unlinked_attributes[0]->native_payload_ops == NULL);
    CHECK(owned->unlinked_attributes[owned->max_num_attributes - 1]->native_payload_ops == NULL);
    CHECK(witness->baselocals[a0_index]->native_payload_ops == NULL);
    for (id = 2; id <= 5; ++id) CHECK(finalizers[id] == checkpoints + 1);
    CHECK(finalizers[7] == 0);
    if (unwinding == 1) {
        CHECK(finalizers[1] == checkpoints + 1 && finalizers[6] == 0);
        CHECK(copies[1] == 0 && copies[6] == 0);
    } else if (unwinding == 2) {
        CHECK(finalizers[1] == checkpoints + 1 && finalizers[6] == checkpoints);
        CHECK(copies[1] == 0 && copies[6] == checkpoints + 1);
        CHECK(payload_id(result) == 6 && result->num_attributes == 0);
        CHECK(witness->locals[0] == fixture_context->modules[0]->globals[0]);
    } else {
        CHECK(finalizers[1] == checkpoints * 2 + 1 && finalizers[6] == checkpoints);
        CHECK(copies[1] == checkpoints + 1 && copies[6] == checkpoints + 1);
        CHECK(payload_id(result) == 1 && result->num_attributes == 1);
        CHECK(result->attributes[0] == result->unlinked_attributes[0]);
        CHECK(result->attributes[0] != owned->attributes[0] && payload_id(result->attributes[0]) == 6);
    }
    ++checkpoints;
    return 0;
}

static rxvm_context *load(const char *path) {
    rxvm_context *context = rxvm_create();
    CHECK(context != NULL);
    if (!context) return NULL;
    CHECK(rxvm_load_file(context, (char *)path) != 0);
    if (rxldmodp(context) <= 0 || rxvm_link(context) != 0 || rxvm_prepare(context) != 0 ||
        rxvm_initialize(context) != 0 || context->num_modules < 2 ||
        !context->modules[0]->file ||
        strcmp(context->modules[0]->file->name, "tests_return_retirement.rxas") != 0) {
        CHECK(0);
        rxvm_destroy(context);
        return NULL;
    }
    return context;
}

static void check_references(const char *path, const char *entry) {
    rxvm_context *context = load(path);
    module *owner;
    if (!context) return;
    owner = context->modules[0];
    CHECK(rxvm_call(context, (char *)entry, 0, NULL) == 0);
    if (!strcmp(entry, "returnretirement.identity")) {
        CHECK(owner->globals[4]->int_value == 0 && owner->globals[5]->int_value == 1);
        CHECK(owner->globals[1]->int_value == 41 && owner->globals[8]->int_value == 41);
        CHECK(owner->globals[3]->reference_payload == owner->globals[1]->reference_identity);
        CHECK(owner->globals[2]->reference_payload &&
              !rxvm_reference_cell_is_valid(owner->globals[2]->reference_payload));
    } else if (!strcmp(entry, "returnretirement.localref")) {
        CHECK(owner->globals[4]->int_value == 0);
        CHECK(context->references.active_count == 0);
    } else {
        if (!strcmp(entry, "returnretirement.globalref")) CHECK(owner->globals[4]->int_value == 1);
        CHECK(owner->globals[8]->reference_identity &&
              owner->globals[8]->reference_identity->retain_count == 1);
        CHECK(context->references.active_count == 1);
    }
    rxvm_destroy(context);
}

static void check_borrowed_return(const char *path) {
    rxvm_context *context = load(path);
    module *owner;
    if (!context) return;
    owner = context->modules[0];
    memset(copies, 0, sizeof(copies)); memset(finalizers, 0, sizeof(finalizers));
    mark(owner->globals[0], 6);
    CHECK(rxvm_call(context, "returnretirement.borrowed", 0, NULL) == 0);
    CHECK(payload_id(owner->globals[0]) == 6 && payload_id(owner->globals[1]) == 6);
    CHECK(copies[6] == 1 && finalizers[6] == 0);
    clear_value(owner->globals[1]);
    CHECK(finalizers[6] == 1 && payload_id(owner->globals[0]) == 6);
    rxvm_destroy(context);
    CHECK(finalizers[6] == 2);
}

static void check_nested_reference(const char *path) {
    rxvm_context *context = load(path);
    module *owner;
    rxvm_reference_cell *cell;
    if (!context) return;
    owner = context->modules[0];
    cell = rxvm_reference_identity_for_context(&context->references,
            owner->globals[8], RXVM_REF_GLOBAL, owner, 0, "borrowed aggregate target");
    CHECK(cell != NULL);
    if (cell) {
        set_num_attributes(owner->globals[0], 1);
        rxvm_reference_value_set_payload(owner->globals[0]->attributes[0], cell);
        CHECK(cell->retain_count == 2);
        CHECK(rxvm_call(context, "returnretirement.nestedref", 0, NULL) == 0);
        CHECK(rxvm_reference_cell_is_valid(cell) && cell->retain_count == 2);
    }
    rxvm_destroy(context);
}

static void check_hook_reference(const char *path) {
    rxvm_context *context = load(path);
    module *owner;
    if (!context) return;
    owner = context->modules[0];
    memset(copies, 0, sizeof(copies)); memset(finalizers, 0, sizeof(finalizers));
    fixture_context = context; hook_reference = 1;
    mark(owner->globals[0], 1);
    CHECK(rxvm_call(context, "returnretirement.hookref", 0, NULL) == 0);
    CHECK(copies[1] == 1 && finalizers[1] == 1);
    CHECK(owner->globals[8]->reference_identity &&
          owner->globals[8]->reference_identity->retain_count == 1);
    hook_reference = 0;
    rxvm_destroy(context); fixture_context = NULL;
    CHECK(finalizers[1] == 2);
}

static void check_retirement(const char *path, int outward) {
    rxvm_context *context = load(path);
    module *owner;
    size_t id;
    if (!context) return;
    owner = context->modules[0];
    memset(copies, 0, sizeof(copies)); memset(finalizers, 0, sizeof(finalizers));
    fixture_context = context; witness = NULL; seeds = checkpoints = 0; unwinding = outward;
    set_int(owner->globals[6], outward);
    mark(owner->globals[0], 6); mark(owner->globals[9], 7);
    context->active.output_writer = writer;
    CHECK(rxvm_call(context, outward == 1 ? "returnretirement.unwind" : "returnretirement.normal", 0, NULL) == 0);
    witness = NULL; /* run() drained the physical frames before returning. */
    CHECK(seeds == 2 && checkpoints == 2);
    CHECK(finalizers[1] == (outward ? 2 : 4));
    for (id = 2; id <= 5; ++id) CHECK(finalizers[id] == 2);
    CHECK(finalizers[6] == (outward == 1 ? 0 : 2) && finalizers[7] == 0);
    CHECK(payload_id(owner->globals[0]) == 6 && payload_id(owner->globals[9]) == 7);
    context->active.output_writer = NULL;
    rxvm_destroy(context); fixture_context = NULL;
    CHECK(finalizers[1] == (outward ? 2 : 4));
    for (id = 2; id <= 5; ++id) CHECK(finalizers[id] == 2);
    CHECK(finalizers[6] == (outward == 1 ? 1 : 3) && finalizers[7] == 1);
}

static void check_terminal_retirement(const char *path) {
    rxvm_context *context = load(path);
    module *owner;
    size_t id;
    if (!context) return;
    owner = context->modules[0];
    memset(copies, 0, sizeof(copies)); memset(finalizers, 0, sizeof(finalizers));
    fixture_context = context; witness = NULL; seeds = checkpoints = 0;
    mark(owner->globals[0], 6); mark(owner->globals[9], 7);
    /* SILENT_HALT returns zero but must retire the interrupted physical frame
     * and abandon the root continuation. No finalizer timing oracle is used. */
    CHECK(rxvm_call(context, "returnretirement.terminal", 0, NULL) == 0);
    witness = NULL;
    CHECK(seeds == 1 && checkpoints == 0 && owner->globals[8]->int_value == 0);
    for (id = 1; id <= 5; ++id) CHECK(finalizers[id] == 1);
    CHECK(finalizers[6] == 0 && finalizers[7] == 0);
    CHECK(payload_id(owner->globals[0]) == 6 && payload_id(owner->globals[9]) == 7);
    /* The same embedding context admits a clean subsequent VM run. */
    CHECK(rxvm_call(context, "returnretirement.globalref", 0, NULL) == 0);
    CHECK(owner->globals[4]->int_value == 1 && owner->globals[8]->reference_identity &&
          owner->globals[8]->reference_identity->retain_count == 1);
    for (id = 1; id <= 5; ++id) CHECK(finalizers[id] == 1);
    rxvm_destroy(context); fixture_context = NULL;
    for (id = 1; id <= 5; ++id) CHECK(finalizers[id] == 1);
    CHECK(finalizers[6] == 1 && finalizers[7] == 1);
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    rxvm_addfunc(seed, "returnretirement.seed", "b", ".void", "");
    check_references(argv[1], "returnretirement.identity");
    check_references(argv[1], "returnretirement.localref");
    check_references(argv[1], "returnretirement.globalref");
    check_references(argv[1], "returnretirement.relayref");
    check_nested_reference(argv[1]);
    check_hook_reference(argv[1]);
    check_borrowed_return(argv[1]);
    check_retirement(argv[1], 0);
    check_retirement(argv[1], 1);
    check_retirement(argv[1], 2);
    check_terminal_retirement(argv[1]);
    printf("RETURN_RETIREMENT|failures=%d\n", failures);
    return failures ? 1 : 0;
}
