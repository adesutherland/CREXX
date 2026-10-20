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

/* Equal-position closure is deliberately different from the frozen v2 word
 * fallback: a v3 section retains the whole raw remainder, then moves back. */
static void section_equal(void) {
    unsigned char plan[] = {
        0x50,2,12,0,4,0,2,0,0,0,0,0,
        3,0,1,0,0,0,0,0,0,0, 1,1,
        3,0,1,0,0,0,0,0,0,0, 1,1
    };
    rxvm_parse_cursor *cursor;
    const char *text;
    size_t length;
    int version, status;
    for (version = 2; version <= 3; ++version) {
        plan[1] = (unsigned char)version;
        cursor = rxvm_parse_create("one two three", 13, plan, sizeof(plan), &status);
        CHECK(cursor && status == 0);
        if (!cursor) continue;
        CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_FIELD);
        CHECK(rxvm_parse_text(cursor, &text, &length) == 0);
        CHECK(length == (version == 2 ? 3u : 13u));
        CHECK(!memcmp(text, "one two three", length));
        CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_FIELD);
        CHECK(rxvm_parse_text(cursor, &text, &length) == 0 && length == 13);
        CHECK(!memcmp(text, "one two three", length));
        CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_DONE);
        rxvm_parse_destroy(cursor);
    }
}

typedef struct section_plan {
    unsigned char bytes[1024];
    size_t length;
    unsigned int count, stored;
} section_plan;
static void plan_start(section_plan *p, unsigned int dynamics) {
    memset(p, 0, sizeof(*p));
    p->bytes[0] = 0x50; p->bytes[1] = 3; p->bytes[2] = 12;
    p->bytes[8] = (unsigned char)dynamics;
    p->length = 12;
}
static void plan_item(section_plan *p, unsigned int kind, unsigned int flags) {
    CHECK(p->length + 2u <= sizeof(p->bytes));
    p->bytes[p->length++] = (unsigned char)kind;
    p->bytes[p->length++] = (unsigned char)flags;
    ++p->count; if (kind == 1 && flags == 1) ++p->stored;
    p->bytes[4] = (unsigned char)p->count;
    p->bytes[5] = (unsigned char)(p->count >> 8);
    p->bytes[6] = (unsigned char)p->stored;
    p->bytes[7] = (unsigned char)(p->stored >> 8);
}
static void plan_number(section_plan *p, unsigned int kind, uint64_t movement) {
    size_t i;
    plan_item(p, kind, 0);
    CHECK(p->length + 8u <= sizeof(p->bytes));
    for (i = 0; i < 8; ++i) p->bytes[p->length++] = (unsigned char)(movement >> (8u * i));
}
static void plan_literal(section_plan *p, const char *text, size_t bytes, size_t chars) {
    size_t i;
    plan_item(p, 2, 0);
    CHECK(p->length + 8u + bytes <= sizeof(p->bytes));
    for (i = 0; i < 4; ++i) p->bytes[p->length++] = (unsigned char)(bytes >> (8u * i));
    for (i = 0; i < 4; ++i) p->bytes[p->length++] = (unsigned char)(chars >> (8u * i));
    memcpy(p->bytes + p->length, text, bytes); p->length += bytes;
}
static void plan_input(section_plan *p, unsigned int kind, unsigned int index) {
    plan_item(p, kind, 0);
    p->bytes[p->length++] = (unsigned char)index;
    p->bytes[p->length++] = (unsigned char)(index >> 8);
}
static void sections_expect(const section_plan *p, const char *source,
                            const char *const *expected, size_t count) {
    rxvm_parse_cursor *cursor;
    const char *text;
    size_t length, i;
    int status;
    cursor = rxvm_parse_create(source, strlen(source), p->bytes, p->length, &status);
    CHECK(cursor && status == 0);
    if (!cursor) return;
    for (i = 0; i < count; ++i) {
        CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_FIELD);
        CHECK(rxvm_parse_slot(cursor) == i + 1u);
        status = rxvm_parse_text(cursor, &text, &length);
        CHECK(status == 0 && length == strlen(expected[i]));
        if (!status && length == strlen(expected[i])) CHECK(!memcmp(text, expected[i], length));
    }
    CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_DONE);
    CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_DONE);
    rxvm_parse_destroy(cursor);
}
static void section_transitions(void) {
    section_plan p;
    unsigned int kind;
    const char *whole[] = {"one two three", "one two three"};
    const char *empty[] = {"one two three", ""};
    const char *back_absolute[] = {"cd ef", "ab cd ef"};
    const char *back_relative[] = {"cd ef", " cd ef"};
    const char *anchored[] = {"aa", ":bb cc", "a:bb cc"};
    const char *zero[] = {"aa", ":bb cc", ":bb cc"};
    const char *control[] = {"aa", "bb cc"};
    const char *missing[] = {"abc", "c"};
    const char *huge[] = {"abc", "", "bc"};
    const char *consecutive[] = {"st", "a", "r", "s"};
    const char *unicode[] = {"a", "b", "c"};
    const char *spaces[] = {"  raw text  "};
    const char *space_anchor[] = {"", " ", "aa bb cc "};
    const char *wide_anchor[] = {"ab", "<=", ">cd"};
    const char *scalar_anchor[] = {"\xce\xb1", ":", "\xf0\x9f\x99\x82 z"};
    const char *at_end[] = {"ab", ""};
    for (kind = 3; kind <= 5; ++kind) {
        plan_start(&p, 0); plan_item(&p, 1, 1); plan_number(&p, kind, 0); plan_item(&p, 1, 1);
        sections_expect(&p, "one two three", whole, 2);
    }
    plan_start(&p, 0); plan_item(&p, 1, 1); plan_literal(&p, "", 0, 0); plan_item(&p, 1, 1);
    sections_expect(&p, "one two three", empty, 2);
    for (kind = 3; kind <= 5; kind += 2) {
        plan_start(&p, 0); plan_number(&p, 3, 4); plan_item(&p, 1, 1);
        plan_number(&p, kind, 1); plan_item(&p, 1, 1);
        sections_expect(&p, "ab cd ef", kind == 3 ? back_absolute : back_relative, 2);
    }
    plan_start(&p, 0); plan_item(&p, 1, 1); plan_literal(&p, ":", 1, 1);
    plan_item(&p, 1, 1); plan_number(&p, 5, 1); plan_item(&p, 1, 1);
    sections_expect(&p, "aa:bb cc", anchored, 3);
    for (kind = 4; kind <= 5; ++kind) {
        plan_start(&p, 0); plan_item(&p, 1, 1); plan_literal(&p, ":", 1, 1);
        plan_item(&p, 1, 1); plan_number(&p, kind, 0); plan_item(&p, 1, 1);
        sections_expect(&p, "aa:bb cc", zero, 3);
    }
    plan_start(&p, 0); plan_item(&p, 1, 1); plan_literal(&p, ":", 1, 1);
    plan_number(&p, 4, 1); plan_item(&p, 1, 1);
    sections_expect(&p, "aa:bb cc", control, 2);
    for (kind = 0; kind < 2; ++kind) {
        plan_start(&p, 0); plan_item(&p, 1, 1); plan_literal(&p, kind ? "!" : "", kind, kind);
        plan_number(&p, 5, 1); plan_item(&p, 1, 1);
        sections_expect(&p, "abc", missing, 2);
    }
    plan_start(&p, 0); plan_item(&p, 1, 1); plan_number(&p, 4, INT64_MAX);
    plan_item(&p, 1, 1); plan_number(&p, 5, 2); plan_item(&p, 1, 1);
    sections_expect(&p, "abc", huge, 3);
    plan_start(&p, 0); plan_number(&p, 3, 2); plan_item(&p, 1, 1); plan_number(&p, 3, 4);
    plan_number(&p, 3, 1); plan_item(&p, 1, 1); plan_number(&p, 3, 2); plan_number(&p, 3, 4);
    plan_item(&p, 1, 1); plan_number(&p, 3, 5); plan_number(&p, 3, 11); plan_item(&p, 1, 1);
    sections_expect(&p, "astronomers", consecutive, 4);
    plan_start(&p, 0); plan_item(&p, 1, 1); plan_literal(&p, "\xe6\x9d\xb1\xe4\xba\xac", 6, 2);
    plan_item(&p, 1, 1); plan_literal(&p, "\xe6\x9d\xb1\xe4\xba\xac", 6, 2); plan_item(&p, 1, 1);
    sections_expect(&p, "a\xe6\x9d\xb1\xe4\xba\xac" "b\xe6\x9d\xb1\xe4\xba\xac" "c", unicode, 3);
    plan_start(&p, 0); plan_item(&p, 1, 1);
    sections_expect(&p, "  raw text  ", spaces, 1);
    plan_start(&p, 0); plan_item(&p, 1, 1); plan_literal(&p, " ", 1, 1);
    plan_item(&p, 1, 1); plan_number(&p, 4, 1); plan_item(&p, 1, 1);
    sections_expect(&p, " aa bb cc ", space_anchor, 3);
    plan_start(&p, 0); plan_item(&p, 1, 1); plan_literal(&p, "<=>", 3, 3);
    plan_item(&p, 1, 1); plan_number(&p, 4, 2); plan_item(&p, 1, 1);
    sections_expect(&p, "ab<=>cd", wide_anchor, 3);
    plan_start(&p, 0); plan_item(&p, 1, 1); plan_literal(&p, ":", 1, 1);
    plan_item(&p, 1, 1); plan_number(&p, 4, 1); plan_item(&p, 1, 1);
    sections_expect(&p, "\xce\xb1:\xf0\x9f\x99\x82 z", scalar_anchor, 3);
    plan_start(&p, 0); plan_item(&p, 1, 1); plan_literal(&p, ":", 1, 1); plan_item(&p, 1, 1);
    sections_expect(&p, "ab:", at_end, 2);
}
static void section_inputs(void) {
    section_plan p;
    rxvm_parse_cursor *cursor;
    const char *text;
    size_t length;
    int status;
    plan_start(&p, 2); plan_input(&p, 7, 0); plan_number(&p, 4, 1);
    plan_item(&p, 1, 1); plan_input(&p, 7, 0);
    cursor = rxvm_parse_create("aa:bb/cc", 8, p.bytes, p.length, &status);
    CHECK(cursor && status == 0);
    if (cursor) {
        CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_INPUT && rxvm_parse_item(cursor) == 1);
        CHECK(rxvm_parse_supply(cursor, ":", 1) == 0);
        CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_INPUT && rxvm_parse_item(cursor) == 4);
        CHECK(rxvm_parse_supply(cursor, "/", 1) == 0);
        CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_FIELD && rxvm_parse_item(cursor) == 3);
        CHECK(rxvm_parse_text(cursor, &text, &length) == 0 && length == 2 && !memcmp(text, "bb", 2));
        CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_DONE); rxvm_parse_destroy(cursor);
    }
    /* All controls, including those after the final field, execute once. */
    plan_start(&p, 1); plan_input(&p, 8, 0); plan_input(&p, 9, 0); plan_input(&p, 10, 0);
    cursor = rxvm_parse_create("abc", 3, p.bytes, p.length, &status);
    CHECK(cursor && status == 0);
    if (cursor) {
        unsigned int item;
        for (item = 1; item <= 3; ++item) {
            CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_INPUT && rxvm_parse_item(cursor) == item);
            CHECK(rxvm_parse_supply(cursor, "1", 1) == 0);
        }
        CHECK(rxvm_parse_next(cursor) == RXVM_PARSE_DONE); rxvm_parse_destroy(cursor);
    }
}
static void section_invalid(const unsigned char *bytes, size_t length, int expected) {
    int status;
    rxvm_parse_cursor *cursor = rxvm_parse_create("abc", 3, bytes, length, &status);
    CHECK(!cursor && status == expected);
    rxvm_parse_destroy(cursor);
}
static void section_validation(void) {
    section_plan p;
    unsigned char changed[1024];
    size_t i;
    static const unsigned int header_offsets[] = {0,1,2,4,6,10,11};
    plan_start(&p, 0); plan_item(&p, 1, 1);
    for (i = 0; i < sizeof(header_offsets) / sizeof(header_offsets[0]); ++i) {
        memcpy(changed, p.bytes, p.length);
        changed[header_offsets[i]] = 255;
        section_invalid(changed, p.length, RXVM_PARSE_INVALID);
    }
    for (i = 0; i < p.length; ++i) section_invalid(p.bytes, i, RXVM_PARSE_INVALID);
    memcpy(changed, p.bytes, p.length); changed[p.length] = 0;
    section_invalid(changed, p.length + 1u, RXVM_PARSE_INVALID);
    for (i = 0; i <= 11; ++i) {
        if (i != 0 && i != 6 && i != 11) continue;
        memcpy(changed, p.bytes, p.length); changed[12] = (unsigned char)i;
        section_invalid(changed, p.length, RXVM_PARSE_INVALID);
    }
    memcpy(changed, p.bytes, p.length); changed[13] = 2;
    section_invalid(changed, p.length, RXVM_PARSE_INVALID);
    plan_item(&p, 1, 0); section_invalid(p.bytes, p.length, RXVM_PARSE_INVALID);
    for (i = 3; i <= 5; ++i) {
        plan_start(&p, 0); plan_number(&p, (unsigned int)i, 1);
        p.bytes[13] = 1; section_invalid(p.bytes, p.length, RXVM_PARSE_INVALID);
        p.bytes[13] = 0; memset(p.bytes + 14, 255, 8);
        section_invalid(p.bytes, p.length, RXVM_PARSE_INVALID);
    }
    for (i = 7; i <= 10; ++i) {
        plan_start(&p, 1); plan_input(&p, (unsigned int)i, 0);
        p.bytes[13] = 1; section_invalid(p.bytes, p.length, RXVM_PARSE_INVALID);
        p.bytes[13] = 0; p.bytes[14] = 1;
        section_invalid(p.bytes, p.length, RXVM_PARSE_INVALID);
        p.bytes[8] = 0; p.bytes[14] = 0;
        section_invalid(p.bytes, p.length, RXVM_PARSE_INVALID);
    }
    plan_start(&p, 0); plan_literal(&p, "x", 1, 2);
    section_invalid(p.bytes, p.length, RXVM_PARSE_INVALID);
    plan_start(&p, 0); plan_literal(&p, "\xff", 1, 1);
    section_invalid(p.bytes, p.length, RXVM_PARSE_UNICODE);
}
int main(void) {
    parse_test_catalogue_init(); parse_test_catalogue_init();
    CHECK(registered_classes == 1 && registered_members == 7 &&
          registered_functions == 7 && registered_manifests == 1);
    provider_failures(); kernel_edges(); section_equal();
    section_transitions(); section_inputs(); section_validation();
    printf("PARSE_CURSOR_NATIVE|checks=%d|failures=%d\n", checks, failures);
    return failures ? 1 : 0;
}
