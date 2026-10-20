/* cREXX License (MIT). Native boundary/failure-injection regression.
 * Compile the exact provider source privately so its existing host callbacks
 * can fail deterministically. Production builds expose no test hook/API. The
 * normal B fixture separately tests the real core catalogue and type service. */
#include <stdlib.h>
#include <stddef.h>
static int fail_allocation, fail_payload;
static void *test_provider_calloc(size_t count, size_t size);
#define calloc test_provider_calloc
#define rxvm_setnativepayload parse_test_payload
#define rxvm_addfunc_for_plugin parse_test_addfunc
#define rxvm_addclass parse_test_addclass
#define rxvm_addmember parse_test_addmember
#define rxvm_register_static_plugin_manifest_v2 parse_test_manifest
#define CREXX_VM_SINGLE_THREADED 1
#define rxvm_parse_cursor_init parse_test_catalogue_init
#include "../../interpreter/rxvmparse_cursor.c"
#undef CREXX_VM_SINGLE_THREADED
#undef calloc
#undef rxvm_setnativepayload
#include "rxvmintp.h"
#include "rxvmvars.h"
#include <stdio.h>
#include <stdint.h>

extern int rxvm_string_view(rxpa_attribute_value, const char **, size_t *);
extern int rxvm_string_set(rxpa_attribute_value, const char *, size_t);
static int fail_type, fail_text, fail_view, failures, checks;
static int registered_classes, registered_members, registered_functions, registered_manifests;
#define CHECK(test) do { ++checks; if (!(test)) { \
    fprintf(stderr, "FAIL: line %d: %s\n", __LINE__, #test); ++failures; } } while (0)

static void *test_provider_calloc(size_t count, size_t size) {
    if (fail_allocation) { fail_allocation = 0; return NULL; }
    return calloc(count, size);
}
extern int rxvm_setnativepayload(rxpa_attribute_value, const void *, size_t,
                                const rxpa_native_payload_ops *, unsigned int);
int parse_test_payload(rxpa_attribute_value value, const void *data, size_t length,
                       const rxpa_native_payload_ops *ops, unsigned int flags) {
    if (fail_payload) { fail_payload = 0; return -1; }
    return rxvm_setnativepayload(value, data, length, ops, flags);
}
void parse_test_addfunc(const char *provider, rxpa_libfunc fn, char *name,
                        char *option, char *type, char *args) {
    CHECK(!strcmp(provider, "rxparse") && fn && !strcmp(option, "b"));
    CHECK(!strncmp(name, "rxparse.cursor.", 15) && type && args);
    ++registered_functions;
}
void parse_test_addclass(char *name, char *option, char *type) {
    CHECK(!strcmp(name, "rxparse.cursor") && !strcmp(option, "b") && !strcmp(type, ".unknown"));
    ++registered_classes;
}
void parse_test_addmember(char *owner, char *kind, char *member, char *type, char *args) {
    CHECK(!strcmp(owner, "rxparse.cursor") && kind && member && type && args);
    ++registered_members;
}
void parse_test_manifest(const rxpa_plugin_manifest_v2 *manifest) {
    CHECK(!strcmp(manifest->plugin_id, "rxparse") && manifest->session_create_with_host);
    ++registered_manifests;
}

static int test_type(rxpa_attribute_value destination, const char *name) {
    (void)destination;
    CHECK(strcmp(name, "rxparse.cursor") == 0);
    return fail_type ? -1 : 0;
}
static int test_view(rxpa_attribute_value value, const char **data, size_t *length) {
    return fail_view ? -1 : rxvm_string_view(value, data, length);
}
static int test_text(rxpa_attribute_value value, const char *data, size_t length) {
    return fail_text ? -1 : rxvm_string_set(value, data, length);
}
static const rxpa_host_services_v1 test_host = {
    sizeof(rxpa_host_services_v1), RXPA_HOST_SERVICES_ABI_V1,
    test_view, test_type, test_text
};
static const unsigned char one_field[] = {
    0x50, 1, 8, 0, 1, 0, 1, 0, 1, 1
};
static const unsigned char dynamic_field[] = {
    0x50, 2, 12, 0, 2, 0, 1, 0, 1, 0, 0, 0, 7, 0, 0, 0, 1, 1
};

static void provider_failures(void) {
    value source, plan, result, alias, output, signal, operand, forged;
    rxpa_attribute_value arguments[2];
    parse_session *first, *second;
    parse_resource *owner;
    value_init(&source); value_init(&plan); value_init(&result); value_init(&alias);
    value_init(&output); value_init(&signal); value_init(&operand); value_init(&forged);
    CHECK(set_string_validated(&source, "a\0b", 3) == 0);
    CHECK(set_binary(&plan, one_field, sizeof(one_field)) == 0);
    first = create_session(&test_host); second = create_session(&test_host);
    CHECK(first && second);
    if (!first || !second) goto cleanup;
    current = first;
    arguments[0] = &source; arguments[1] = &plan;
    fail_allocation = 1;
    make_cursor(2, arguments, &result, &signal);
    CHECK(signal.int_value == SIGNAL_FAILURE && resource(&result) == NULL && first->refs == 1);
    fail_payload = 1;
    make_cursor(2, arguments, &result, &signal);
    CHECK(signal.int_value == SIGNAL_FAILURE && resource(&result) == NULL && first->refs == 1);
    fail_type = 1;
    make_cursor(2, arguments, &result, &signal);
    CHECK(signal.int_value == SIGNAL_FAILURE && resource(&result) == NULL);
    CHECK(first->refs == 1);
    fail_type = 0;
    make_cursor(2, arguments, &result, &signal);
    CHECK(signal.int_value == SIGNAL_NONE && resource(&result));
    owner = resource(&result);
    if (!owner) goto cleanup;
    CHECK(first->refs == 2);
    copy_cursor(&alias, &result);
    CHECK(resource(&alias) == owner && owner->refs == 2);
    arguments[0] = &result;
    cursor_next(1, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_NONE && output.int_value == RXVM_PARSE_FIELD);
    fail_text = 1;
    cursor_text(1, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_FAILURE);
    CHECK(rxvm_parse_item(owner->cursor) == 1 && rxvm_parse_slot(owner->cursor) == 1);
    fail_text = 0;
    cursor_next(1, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_INVALID_ARGUMENTS);
    cursor_close(1, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_NONE && rxvm_parse_item(owner->cursor) == 0);
    arguments[0] = &alias;
    cursor_close(1, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_NONE);
    current = second;
    cursor_next(1, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_INVALID_ARGUMENTS);
    arguments[0] = &forged;
    cursor_next(1, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_INVALID_ARGUMENTS);
    current = first;
    clear_value(&alias); clear_value(&result);
    CHECK(first->refs == 1);
    CHECK(set_binary(&plan, dynamic_field, sizeof(dynamic_field)) == 0);
    arguments[0] = &source; arguments[1] = &plan;
    make_cursor(2, arguments, &result, &signal);
    CHECK(signal.int_value == SIGNAL_NONE);
    arguments[0] = &result;
    cursor_next(1, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_NONE && output.int_value == RXVM_PARSE_INPUT);
    CHECK(set_string_validated(&operand, ":", 1) == 0);
    arguments[1] = &operand;
    fail_view = 1;
    cursor_supply(2, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_UNICODE_ERROR);
    fail_view = 0;
    cursor_next(1, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_INVALID_ARGUMENTS);
    clear_value(&result);
    CHECK(first->refs == 1);
    arguments[0] = &source; arguments[1] = &plan;
    make_cursor(2, arguments, &result, &signal);
    arguments[0] = &result;
    cursor_next(1, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_NONE && output.int_value == RXVM_PARSE_INPUT);
    arguments[1] = NULL;
    cursor_supply(2, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_INVALID_ARGUMENTS);
    cursor_next(1, arguments, &output, &signal);
    CHECK(signal.int_value == SIGNAL_INVALID_ARGUMENTS);
cleanup:
    current = NULL;
    clear_value(&forged); clear_value(&operand); clear_value(&signal); clear_value(&output);
    clear_value(&alias); clear_value(&result); clear_value(&plan); clear_value(&source);
    release_session(first); release_session(second);
}

static void kernel_edges(void) {
    rxvm_parse_cursor *cursor;
    int status;
    unsigned char bad[sizeof(one_field)];
    memcpy(bad, one_field, sizeof(bad)); bad[4] = 255; bad[5] = 255;
    cursor = rxvm_parse_create("x", 1, bad, sizeof(bad), &status);
    CHECK(!cursor && status == RXVM_PARSE_INVALID);
    cursor = rxvm_parse_create("x", SIZE_MAX, one_field, sizeof(one_field), &status);
    CHECK(!cursor && status == RXVM_PARSE_RANGE);
    cursor = rxvm_parse_create("x", (size_t)PTRDIFF_MAX, one_field, sizeof(one_field), &status);
    CHECK(!cursor && status == RXVM_PARSE_RANGE);
    cursor = rxvm_parse_create("x", 1, one_field, SIZE_MAX, &status);
    CHECK(!cursor && status == RXVM_PARSE_RANGE);
#ifndef NUTF8
    cursor = rxvm_parse_create("\xff", 1, one_field, sizeof(one_field), &status);
    CHECK(!cursor && status == RXVM_PARSE_UNICODE);
#endif
    cursor = rxvm_parse_create("", 0, one_field, sizeof(one_field), &status);
    CHECK(cursor && status == 0);
    if (cursor) {
        CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_FIELD);
        CHECK(rxvm_parse_item(cursor) == 1 && rxvm_parse_slot(cursor) == 1);
        rxvm_parse_close(cursor); rxvm_parse_close(cursor); rxvm_parse_destroy(cursor);
    }
}
int main(void) {
    parse_test_catalogue_init(); parse_test_catalogue_init();
    CHECK(registered_classes == 1 && registered_members == 7 &&
          registered_functions == 7 && registered_manifests == 1);
    provider_failures(); kernel_edges();
    printf("PARSE_CURSOR_NATIVE|checks=%d|failures=%d\n", checks, failures);
    return failures ? 1 : 0;
}
