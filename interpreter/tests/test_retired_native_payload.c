/* cREXX License (MIT). Finalizer ownership in retired value backing. */
#include "rxvm.h"
#include "platform.h"
#include "rxvmintp.h"
#include "rxvmvars.h"
#include "rxastree.h"
#include <stdio.h>

static int failures;
static int finalizers;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); ++failures; \
} } while (0)

static void finalize_payload(void *item) {
    size_t length = 0;
    const rxvm_native_payload_ops *ops = NULL;
    const unsigned char *bytes = get_native_payload((value *)item, &length, &ops, NULL);
    CHECK(bytes && length == 1 && *bytes == 0x5a);
    ++finalizers;
}

static const rxvm_native_payload_ops payload_ops = {
    "retired-frame-test.payload", NULL, finalize_payload
};

static void mark(value *target) {
    const unsigned char byte = 0x5a;
    CHECK(set_native_payload(target, &byte, 1, &payload_ops, 0) == 0);
}

int main(void) {
    value owned = {0}, transferred = {0}, destination = {0}, borrowed = {0};
    value *inactive;

    set_num_attributes(&owned, 1);
    CHECK(owned.num_attributes == 1 && owned.unlinked_attributes);
    if (!owned.unlinked_attributes) return 1;
    set_num_attributes(owned.unlinked_attributes[0], 1);
    mark(owned.unlinked_attributes[0]->unlinked_attributes[0]);
    inactive = owned.unlinked_attributes[owned.max_num_attributes - 1];
    CHECK(inactive != NULL && inactive != owned.unlinked_attributes[0]);
    if (inactive) mark(inactive);
    mark(&owned);

    /* Active links can be borrowed. Only unlinked backing belongs to owned. */
    mark(&borrowed);
    owned.attributes[0] = &borrowed;
    finalize_retired_native_payloads(&owned);
    CHECK(finalizers == 3);
    CHECK(borrowed.native_payload_ops == &payload_ops);
    CHECK(owned.native_payload_ops == NULL);
    CHECK(owned.unlinked_attributes[0]->unlinked_attributes[0]->native_payload_ops == NULL);
    CHECK(inactive && inactive->native_payload_ops == NULL);
    finalize_retired_native_payloads(&owned);
    CHECK(finalizers == 3);
    owned.attributes[0] = owned.unlinked_attributes[0];
    clear_value(&owned);
    CHECK(finalizers == 3);

    mark(&transferred);
    move_value(&destination, &transferred);
    finalize_retired_native_payloads(&transferred);
    CHECK(finalizers == 3 && transferred.native_payload_ops == NULL);
    CHECK(destination.native_payload_ops == &payload_ops);
    clear_value(&destination);
    CHECK(finalizers == 4);
    clear_value(&transferred);
    clear_value(&borrowed);
    CHECK(finalizers == 5);

    printf("RETIRED_NATIVE_PAYLOAD|failures=%d\n", failures);
    return failures ? 1 : 0;
}
