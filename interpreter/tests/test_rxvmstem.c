/*
 * Focused storage, failure-atomicity, corruption and lifetime tests for the
 * private NR-15 native stem representation.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "rxbin.h"
#include "rxvalue.h"

static int test_prep_binary_buffer(value *v, size_t length);
static int test_reserve_binary_buffer(value *v, size_t length);
static int test_try_set_num_attributes(value *v, size_t count);
static void *test_stem_malloc(size_t length);
static void test_stem_free(void *buffer);

#define RXSTEM_PREP_BINARY_BUFFER test_prep_binary_buffer
#define RXSTEM_RESERVE_BINARY_BUFFER test_reserve_binary_buffer
#define RXSTEM_TRY_SET_NUM_ATTRIBUTES test_try_set_num_attributes
#define RXSTEM_MALLOC test_stem_malloc
#define RXSTEM_FREE test_stem_free
#include "rxvmstem.h"

enum test_failure_kind {
    TEST_FAIL_NONE = 0,
    TEST_FAIL_PREP_BINARY,
    TEST_FAIL_RESERVE_BINARY,
    TEST_FAIL_ATTRIBUTES,
    TEST_FAIL_STRING
};

static enum test_failure_kind next_failure = TEST_FAIL_NONE;

static int consume_failure(enum test_failure_kind kind) {
    if (next_failure != kind) return 0;
    next_failure = TEST_FAIL_NONE;
    return 1;
}

static int test_prep_binary_buffer(value *v, size_t length) {
    if (consume_failure(TEST_FAIL_PREP_BINARY)) return -1;
    return prep_binary_buffer(v, length);
}

static int test_reserve_binary_buffer(value *v, size_t length) {
    if (consume_failure(TEST_FAIL_RESERVE_BINARY)) return -1;
    return reserve_binary_buffer(v, length);
}

static int test_try_set_num_attributes(value *v, size_t count) {
    if (consume_failure(TEST_FAIL_ATTRIBUTES)) return -1;
    return try_set_num_attributes(v, count);
}

static void *test_stem_malloc(size_t length) {
    if (consume_failure(TEST_FAIL_STRING)) return 0;
    return malloc(length);
}

static void test_stem_free(void *buffer) {
    free(buffer);
}

static void fail(const char *expression, const char *file, int line) {
    fprintf(stderr, "FAIL %s:%d: %s\n", file, line, expression);
    exit(1);
}

#define CHECK(expression) \
    do { if (!(expression)) fail(#expression, __FILE__, __LINE__); } while (0)

static void set_text(value *v, const char *text) {
    set_null_string(v, text);
}

static int text_is(const value *v, const char *text) {
    size_t length = strlen(text);
    return v->string_length == length &&
           (!length || memcmp(v->string_value, text, length) == 0);
}

static int stem_get_text(value *stem, const char *key_text,
                         const char *expected) {
    value key;
    value out;
    rxstem_key_parts parts;
    int result;

    value_init(&key);
    value_init(&out);
    set_text(&key, key_text);
    parts = rxstem_one_part(&key);
    result = rxstem_get_parts(&out, stem, &parts);
    CHECK(result == RXSTEM_OK);
    result = text_is(&out, expected);
    clear_value(&out);
    clear_value(&key);
    return result;
}

static int stem_set_text(value *stem, const char *key_text,
                         const char *value_text) {
    value key;
    value new_value;
    rxstem_key_parts parts;
    int result;

    value_init(&key);
    value_init(&new_value);
    set_text(&key, key_text);
    set_text(&new_value, value_text);
    parts = rxstem_one_part(&key);
    result = rxstem_set_parts(stem, &parts, &new_value);
    clear_value(&new_value);
    clear_value(&key);
    return result;
}

static void fill_long_text(char *buffer, size_t size, char seed) {
    size_t i;
    CHECK(size > 1);
    for (i = 0; i + 1 < size; i++) buffer[i] = (char)(seed + (i % 13));
    buffer[size - 1] = 0;
}

static void test_basic_segmented_and_alias_semantics(void) {
    value stem;
    value key;
    value right;
    value data;
    value out;
    rxstem_key_parts parts;
    rxinteger size;

    value_init(&stem);
    value_init(&key);
    value_init(&right);
    value_init(&data);
    value_init(&out);

    CHECK(rxstem_init(&stem) == RXSTEM_OK);
    CHECK(rxstem_init(&stem) == RXSTEM_CORRUPT);
    CHECK(rxstem_size(&size, &stem) == RXSTEM_OK && size == 0);

    set_text(&key, "left");
    set_text(&right, "right");
    set_text(&data, "joined");
    parts = rxstem_two_parts(&key, &right);
    CHECK(rxstem_set_parts(&stem, &parts, &data) == RXSTEM_OK);
    CHECK(rxstem_get_parts(&out, &stem, &parts) == RXSTEM_OK);
    CHECK(text_is(&out, "joined"));
    CHECK(stem_get_text(&stem, "left.right", "joined"));

    set_text(&key, "alias");
    parts = rxstem_one_part(&key);
    CHECK(rxstem_set_parts(&stem, &parts, &key) == RXSTEM_OK);
    CHECK(rxstem_get_parts(&key, &stem, &parts) == RXSTEM_OK);
    CHECK(text_is(&key, "alias"));

    parts = rxstem_one_part(&key);
    CHECK(rxstem_get_parts(&stem, &stem, &parts) == RXSTEM_OK);
    CHECK(text_is(&stem, "alias"));
    CHECK(rxstem_size(&size, &stem) == RXSTEM_OK && size == 2);

    clear_value(&out);
    clear_value(&data);
    clear_value(&right);
    clear_value(&key);
    clear_value(&stem);
}

static void test_failure_atomicity(void) {
    value stem;
    value key;
    value data;
    value out;
    value old_default;
    rxstem_key_parts parts;
    rxstem_header before;
    rxstem_header after;
    char key_name[32];
    char long_text[160];
    int i;

    value_init(&stem);
    next_failure = TEST_FAIL_PREP_BINARY;
    CHECK(rxstem_init(&stem) == RXSTEM_OUT_OF_MEMORY);
    CHECK(stem.binary_length == 0 && stem.num_attributes == 0);
    CHECK(next_failure == TEST_FAIL_NONE);

    next_failure = TEST_FAIL_ATTRIBUTES;
    CHECK(rxstem_init(&stem) == RXSTEM_OUT_OF_MEMORY);
    CHECK(stem.binary_length == 0 && stem.num_attributes == 0);
    CHECK(rxstem_init(&stem) == RXSTEM_OK);

    for (i = 0; i < RXSTEM_INITIAL_CAPACITY; i++) {
        snprintf(key_name, sizeof(key_name), "key-%02d", i);
        CHECK(stem_set_text(&stem, key_name, "value") == RXSTEM_OK);
    }
    CHECK(rxstem_load(&stem, &before) == RXSTEM_OK);
    CHECK(before.count == RXSTEM_INITIAL_CAPACITY);

    value_init(&key);
    value_init(&data);
    value_init(&out);
    value_init(&old_default);
    set_text(&key, "reserve-failure");
    set_text(&data, "new");
    parts = rxstem_one_part(&key);
    next_failure = TEST_FAIL_RESERVE_BINARY;
    CHECK(rxstem_set_parts(&stem, &parts, &data) == RXSTEM_OUT_OF_MEMORY);
    CHECK(rxstem_load(&stem, &after) == RXSTEM_OK);
    CHECK(after.count == before.count && after.capacity == before.capacity);
    CHECK(stem_get_text(&stem, "reserve-failure", ""));

    next_failure = TEST_FAIL_ATTRIBUTES;
    CHECK(rxstem_set_parts(&stem, &parts, &data) == RXSTEM_OUT_OF_MEMORY);
    CHECK(rxstem_load(&stem, &after) == RXSTEM_OK);
    CHECK(after.count == before.count);
    CHECK(stem_get_text(&stem, "reserve-failure", ""));

    fill_long_text(long_text, sizeof(long_text), 'a');
    set_text(&key, long_text);
    parts = rxstem_one_part(&key);
    next_failure = TEST_FAIL_STRING;
    CHECK(rxstem_set_parts(&stem, &parts, &data) == RXSTEM_OUT_OF_MEMORY);
    CHECK(rxstem_load(&stem, &after) == RXSTEM_OK);
    CHECK(after.count == before.count);

    set_text(&key, "long-value-insert");
    set_text(&data, long_text);
    parts = rxstem_one_part(&key);
    next_failure = TEST_FAIL_STRING;
    CHECK(rxstem_set_parts(&stem, &parts, &data) == RXSTEM_OUT_OF_MEMORY);
    CHECK(rxstem_load(&stem, &after) == RXSTEM_OK);
    CHECK(after.count == before.count);
    CHECK(stem_get_text(&stem, "long-value-insert", ""));

    CHECK(stem_set_text(&stem, "stable", "old") == RXSTEM_OK);
    set_text(&key, "stable");
    parts = rxstem_one_part(&key);
    next_failure = TEST_FAIL_STRING;
    CHECK(rxstem_set_parts(&stem, &parts, &data) == RXSTEM_OUT_OF_MEMORY);
    CHECK(stem_get_text(&stem, "stable", "old"));

    set_text(&old_default, "default-before");
    CHECK(rxstem_reset(&stem, &old_default) == RXSTEM_OK);
    CHECK(rxstem_load(&stem, &before) == RXSTEM_OK);
    next_failure = TEST_FAIL_STRING;
    CHECK(rxstem_reset(&stem, &data) == RXSTEM_OUT_OF_MEMORY);
    CHECK(rxstem_load(&stem, &after) == RXSTEM_OK);
    CHECK(after.generation == before.generation);
    CHECK(stem_get_text(&stem, "missing", "default-before"));

    CHECK(rxstem_set_parts(&stem, &parts, &data) == RXSTEM_OK);
    set_text(&out, "destination-before");
    next_failure = TEST_FAIL_STRING;
    CHECK(rxstem_get_parts(&out, &stem, &parts) == RXSTEM_OUT_OF_MEMORY);
    CHECK(text_is(&out, "destination-before"));
    CHECK(rxstem_get_parts(&out, &stem, &parts) == RXSTEM_OK);
    CHECK(text_is(&out, long_text));

    clear_value(&old_default);
    clear_value(&out);
    clear_value(&data);
    clear_value(&key);
    clear_value(&stem);
}

static void test_get_failure_contract(void) {
    value stem;
    value left;
    value right;
    value data;
    value out;
    rxstem_key_parts parts;
    rxstem_header before;
    rxstem_header after;
    char long_text[160];

    value_init(&stem);
    value_init(&left);
    value_init(&right);
    value_init(&data);
    value_init(&out);

    /* Lazy private initialization can fail, but a GET failure changes neither
     * the logical empty stem nor its destination. */
    set_text(&left, "missing");
    set_text(&out, "destination-before");
    parts = rxstem_one_part(&left);
    next_failure = TEST_FAIL_PREP_BINARY;
    CHECK(rxstem_get_parts(&out, &stem, &parts) == RXSTEM_OUT_OF_MEMORY);
    CHECK(stem.binary_length == 0 && stem.num_attributes == 0);
    CHECK(text_is(&out, "destination-before"));

    /* The segmented handler shares the same failure-atomic value copy.  An
     * allocation failure must preserve both the destination and logical stem
     * state, including its generation and entry count. */
    CHECK(rxstem_init(&stem) == RXSTEM_OK);
    set_text(&left, "left");
    set_text(&right, "right");
    fill_long_text(long_text, sizeof(long_text), 'm');
    set_text(&data, long_text);
    parts = rxstem_two_parts(&left, &right);
    CHECK(rxstem_set_parts(&stem, &parts, &data) == RXSTEM_OK);
    CHECK(rxstem_load(&stem, &before) == RXSTEM_OK);
    next_failure = TEST_FAIL_STRING;
    CHECK(rxstem_get_parts(&out, &stem, &parts) == RXSTEM_OUT_OF_MEMORY);
    CHECK(text_is(&out, "destination-before"));
    CHECK(rxstem_load(&stem, &after) == RXSTEM_OK);
    CHECK(after.generation == before.generation &&
          after.count == before.count && after.capacity == before.capacity);
    CHECK(rxstem_get_parts(&out, &stem, &parts) == RXSTEM_OK);
    CHECK(text_is(&out, long_text));

    clear_value(&out);
    clear_value(&data);
    clear_value(&right);
    clear_value(&left);
    clear_value(&stem);
}

static void test_corruption_overflow_and_bounds(void) {
    value stem;
    value key;
    value out;
    value default_value;
    rxstem_key_parts parts;
    rxstem_header header;
    rxstem_entry entry;
    uint32_t bucket;
    size_t length;

    value_init(&stem);
    value_init(&key);
    value_init(&out);
    value_init(&default_value);
    CHECK(rxstem_init(&stem) == RXSTEM_OK);
    CHECK(stem_set_text(&stem, "one", "first") == RXSTEM_OK);

    rxstem_store_u32((unsigned char *)stem.binary_value, 0);
    CHECK(rxstem_load(&stem, &header) == RXSTEM_CORRUPT);
    rxstem_store_u32((unsigned char *)stem.binary_value, RXSTEM_MAGIC);

    CHECK(rxstem_load(&stem, &header) == RXSTEM_OK);
    rxstem_store_u32((unsigned char *)stem.binary_value + 16,
                     header.capacity + 1);
    CHECK(rxstem_load(&stem, &header) == RXSTEM_CORRUPT);
    rxstem_store_u32((unsigned char *)stem.binary_value + 16, 1);

    set_text(&key, "cycle-probe");
    parts = rxstem_one_part(&key);
    bucket = rxstem_hash_parts(&parts) & 255u;
    rxstem_bucket_set(&stem, bucket, 1);
    rxstem_entry_load(&stem, 0, &entry);
    entry.next = 1;
    rxstem_entry_store(&stem, 0, &entry);
    CHECK(rxstem_get_parts(&out, &stem, &parts) == RXSTEM_CORRUPT);
    clear_value(&stem);

    value_init(&stem);
    CHECK(rxstem_init(&stem) == RXSTEM_OK);
    CHECK(stem_set_text(&stem, "one", "first") == RXSTEM_OK);
    set_num_attributes(stem.attributes[RXSTEM_KEYS_ATTRIBUTE], 0);
    CHECK(rxstem_load(&stem, &header) == RXSTEM_CORRUPT);
    clear_value(&stem);

    value_init(&stem);
    CHECK(rxstem_init(&stem) == RXSTEM_OK);
    CHECK(rxstem_key_at(&out, &stem, 0) == RXSTEM_INVALID_INDEX);
    CHECK(rxstem_value_at(&out, &stem, 1) == RXSTEM_INVALID_INDEX);

    CHECK(rxstem_load(&stem, &header) == RXSTEM_OK);
    header.generation = UINT64_MAX;
    rxstem_store_header(&stem, &header);
    set_text(&default_value, "must-not-commit");
    CHECK(rxstem_reset(&stem, &default_value) == RXSTEM_OVERFLOW);
    CHECK(stem_get_text(&stem, "missing", ""));

    header.capacity = UINT32_C(0x80000000);
    CHECK(rxstem_ensure_capacity(&stem, &header, UINT32_MAX) ==
          RXSTEM_OVERFLOW);

    parts.left = "x";
    parts.left_length = SIZE_MAX;
    parts.right = "y";
    parts.right_length = 1;
    parts.segmented = 1;
    CHECK(rxstem_parts_length(&parts, &length) == RXSTEM_OVERFLOW);

#if SIZE_MAX > UINT32_MAX
    parts.left = "x";
    parts.left_length = (size_t)UINT32_MAX + 1;
    parts.right = 0;
    parts.right_length = 0;
    parts.segmented = 0;
    CHECK(rxstem_set_parts(&stem, &parts, &default_value) == RXSTEM_OVERFLOW);
#endif

    clear_value(&default_value);
    clear_value(&out);
    clear_value(&key);
    clear_value(&stem);
}

static void test_copy_move_and_destruction(void) {
    value stem;
    value copy;
    value moved;
    value new_default;
    char key[32];
    char data[96];
    int i;

    value_init(&stem);
    value_init(&copy);
    value_init(&moved);
    value_init(&new_default);
    CHECK(rxstem_init(&stem) == RXSTEM_OK);

    for (i = 0; i < 48; i++) {
        snprintf(key, sizeof(key), "entry-%02d", i);
        fill_long_text(data, sizeof(data), (char)('A' + (i % 8)));
        CHECK(stem_set_text(&stem, key, data) == RXSTEM_OK);
    }

    copy_value(&copy, &stem);
    CHECK(stem_get_text(&copy, "entry-00", data) == 0);
    fill_long_text(data, sizeof(data), 'A');
    CHECK(stem_get_text(&copy, "entry-00", data));

    CHECK(stem_set_text(&stem, "entry-00", "changed") == RXSTEM_OK);
    CHECK(stem_get_text(&stem, "entry-00", "changed"));
    CHECK(stem_get_text(&copy, "entry-00", data));

    set_text(&new_default, "new-default");
    CHECK(rxstem_reset(&stem, &new_default) == RXSTEM_OK);
    CHECK(stem_get_text(&stem, "entry-01", "new-default"));
    CHECK(stem_get_text(&copy, "entry-01", data) == 0);

    move_value(&moved, &copy);
    CHECK(copy.binary_length == 0 && copy.num_attributes == 0);
    CHECK(stem_get_text(&moved, "entry-00", data));

    clear_value(&new_default);
    clear_value(&moved);
    clear_value(&copy);
    clear_value(&stem);
}

int main(void) {
    test_basic_segmented_and_alias_semantics();
    test_failure_atomicity();
    test_get_failure_contract();
    test_corruption_overflow_and_bounds();
    test_copy_move_and_destruction();
    CHECK(next_failure == TEST_FAIL_NONE);
    puts("PASS: native stem storage and failure paths");
    return 0;
}
