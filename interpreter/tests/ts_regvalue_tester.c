//
// Created by Adrian Sutherland on 28/06/2025.
//
#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "rxbin.h"
#include "rxvalue.h"
#include "rxvmvars.h" // Value functions to be tested
#include "rxvmref.h"
#include "utf.h"      // Your utf8 helper header

/*
 * Verifies the internal consistency of a value's string members.
 * This version allows string_char_pos to be equal to string_chars.
 */
void check_value_state(value* v, const char* file, int line) {
    // 1. Check if char_pos is valid. It can be from 0 to string_chars.
    if (v->string_char_pos > v->string_chars) {
        fprintf(stderr, "VERIFY FAIL (%s:%d): v->string_char_pos (%zu) is out of bounds for v->string_chars (%zu)\n",
                file, line, v->string_char_pos, v->string_chars);
        exit(1);
    }

    // 2. Verify that string_chars is correct by recalculating it.
    size_t calculated_chars = utf8nlen(v->string_value, v->string_length);
    if (v->string_chars != calculated_chars) {
        fprintf(stderr, "VERIFY FAIL (%s:%d): v->string_chars is %zu but should be %zu\n",
                file, line, v->string_chars, calculated_chars);
        exit(1);
    }

    // 3. Verify that string_pos correctly corresponds to string_char_pos
    //    (Only if the char_pos is within the string itself, not at the very end)
    if (v->string_char_pos <= v->string_chars) {
        size_t calculated_pos = 0;
        size_t i = 0;
        for (i = 0; i < v->string_char_pos; ++i) {
            // Protect against reading past the end if state is already corrupt
            if (calculated_pos >= v->string_length) break;
            calculated_pos += utf8codepointcalcsize(v->string_value + calculated_pos);
        }
        if (v->string_pos != calculated_pos) {
            fprintf(stderr, "VERIFY FAIL (%s:%d): v->string_pos (%zu) does not match v->string_char_pos (%zu). Expected pos: %zu\n",
                    file, line, v->string_pos, v->string_char_pos, calculated_pos);
            exit(1);
        }
    }
}

// The macro remains the same
#define CHECK_STATE(v) check_value_state(v, __FILE__, __LINE__)

void check_status_mask(value* v, uint32_t mask, uint32_t expected, const char* file, int line) {
    uint32_t actual = v->status.all_type_flags & mask;
    if (actual != expected) {
        fprintf(stderr, "VERIFY FAIL (%s:%d): status mask 0x%08x is 0x%08x but should be 0x%08x\n",
                file, line, mask, actual, expected);
        exit(1);
    }
}

#define CHECK_STATUS(v, mask, expected) check_status_mask(v, mask, expected, __FILE__, __LINE__)

void check_rc_zero(int rc, const char* expression, const char* file, int line) {
    if (rc != 0) {
        fprintf(stderr, "VERIFY FAIL (%s:%d): %s returned %d\n",
                file, line, expression, rc);
        exit(1);
    }
}

#define CHECK_RC_ZERO(expression) check_rc_zero((expression), #expression, __FILE__, __LINE__)

void check_size_equal(size_t actual, size_t expected, const char* message, const char* file, int line) {
    if (actual != expected) {
        fprintf(stderr, "VERIFY FAIL (%s:%d): %s is %zu but should be %zu\n",
                file, line, message, actual, expected);
        exit(1);
    }
}

#define CHECK_SIZE_EQUAL(actual, expected, message) check_size_equal((actual), (expected), (message), __FILE__, __LINE__)

void check_int_equal(int actual, int expected, const char* message, const char* file, int line) {
    if (actual != expected) {
        fprintf(stderr, "VERIFY FAIL (%s:%d): %s is %d but should be %d\n",
                file, line, message, actual, expected);
        exit(1);
    }
}

#define CHECK_INT_EQUAL(actual, expected, message) check_int_equal((actual), (expected), (message), __FILE__, __LINE__)

void check_pointer_equal(const void* actual, const void* expected, const char* message, const char* file, int line) {
    if (actual != expected) {
        fprintf(stderr, "VERIFY FAIL (%s:%d): %s is %p but should be %p\n",
                file, line, message, actual, expected);
        exit(1);
    }
}

#define CHECK_POINTER_EQUAL(actual, expected, message) check_pointer_equal((actual), (expected), (message), __FILE__, __LINE__)

void check_pointer_not_null(const void* actual, const char* message, const char* file, int line) {
    if (!actual) {
        fprintf(stderr, "VERIFY FAIL (%s:%d): %s is null\n",
                file, line, message);
        exit(1);
    }
}

#define CHECK_POINTER_NOT_NULL(actual, message) check_pointer_not_null((actual), (message), __FILE__, __LINE__)

void check_binary_bytes(value* v, const unsigned char* expected, size_t length, const char* file, int line) {
    if (v->binary_length != length) {
        fprintf(stderr, "VERIFY FAIL (%s:%d): binary_length is %zu but should be %zu\n",
                file, line, v->binary_length, length);
        exit(1);
    }
    if (length && memcmp(v->binary_value, expected, length) != 0) {
        fprintf(stderr, "VERIFY FAIL (%s:%d): binary bytes did not match expected data\n",
                file, line);
        exit(1);
    }
}

#define CHECK_BINARY(v, expected, length) check_binary_bytes(v, expected, length, __FILE__, __LINE__)

void test_string_positioning() {
    value v;
    value_init(&v);

    const char* test_str = "Hello © world with α, β, γ and 𐍈."; // Mixed 1, 2, and 4-byte chars
    printf("--- Testing String: \"%s\" ---\n", test_str);

    set_null_string(&v, test_str);
    CHECK_STATE(&v);

    if (v.string_chars == 0) return;

    printf("Testing forward iteration...\n");
    for (size_t i = 0; i < v.string_chars; ++i) {
        string_set_byte_pos(&v, i);
        CHECK_STATE(&v);
    }

    printf("Testing backward iteration...\n");
    for (size_t i = v.string_chars - 1; ; --i) {
        string_set_byte_pos(&v, i);
        CHECK_STATE(&v);
        if (i == 0) break;
    }

    printf("Testing random jumps...\n");
    string_set_byte_pos(&v, 5);  CHECK_STATE(&v);
    string_set_byte_pos(&v, 15); CHECK_STATE(&v);
    string_set_byte_pos(&v, 2);  CHECK_STATE(&v);
    string_set_byte_pos(&v, v.string_chars - 1); CHECK_STATE(&v);
    string_set_byte_pos(&v, 0);  CHECK_STATE(&v);

    printf("Testing string modifications...\n");
    value v2;
    value_init(&v2);
    set_null_string(&v2, " APPENDED");
    string_append(&v, &v2);
    CHECK_STATE(&v); // CRITICAL: Check state after modification

    // Re-run position tests on the modified string
    for (size_t i = 0; i < v.string_chars; ++i) {
        string_set_byte_pos(&v, i);
        CHECK_STATE(&v);
    }

    clear_value(&v);
    clear_value(&v2);
    printf("--- Test Passed ---\n");
}

/*
 * This test specifically probes boundary conditions that can corrupt the value's state.
 */
void test_boundary_conditions() {
    printf("\n--- Running Boundary Condition Tests ---\n");
    value v;
    value_init(&v);

    // A test string with a multi-byte character near the end.
    // string: "abc€" -> string_length=6, string_chars=4
    const char* test_str = "abc€";
    set_null_string(&v, test_str);
    printf("Testing with string: \"%s\" (length: %zu, chars: %zu)\n",
           test_str, v.string_length, v.string_chars);
    CHECK_STATE(&v);

    // Test 1: Seek to the position right after the last character (char_pos == string_chars).
    // This should be a valid operation.
    printf("  Testing seek to end position (pos %zu)...\n", v.string_chars);
    string_set_byte_pos(&v, v.string_chars);
    CHECK_STATE(&v);
    CHECK_SIZE_EQUAL(v.string_pos, v.string_length, "string_pos at end position");

    // Test 2: Seek ONE position beyond the end.
    // THIS IS THE KEY TEST. The original code will read past the buffer, corrupting its
    // internal state (e.g., setting v.string_char_pos to 5). The subsequent CHECK_STATE will fail.
    printf("  Testing seek one-past-end (pos %zu)...\n", v.string_chars + 1);
    string_set_byte_pos(&v, v.string_chars + 1);
    printf("  State after one-past-end seek: string_pos=%zu, string_char_pos=%zu\n", v.string_pos, v.string_char_pos);
    CHECK_STATE(&v); // <-- EXPECTED TO FAIL ON ORIGINAL CODE

    // Reset for the next test
    value_zero(&v);
    set_null_string(&v, test_str);

    // Test 3: Seek MANY positions beyond the end.
    printf("  Testing seek many-past-end (pos %zu)...\n", v.string_chars + 5);
    string_set_byte_pos(&v, v.string_chars + 5);
    printf("  State after many-past-end seek: string_pos=%zu, string_char_pos=%zu\n", v.string_pos, v.string_char_pos);
    CHECK_STATE(&v); // <-- EXPECTED TO FAIL ON ORIGINAL CODE

    clear_value(&v);
    printf("--- Boundary Tests Finished ---\n");
}

/*
 * This test is designed to trigger the flawed optimization logic by jumping
 * from a position near the end of the string to an invalid position.
 */
void test_flawed_optimization_trigger() {
    printf("\n--- Running Flawed Optimization Trigger Tests ---\n");
    value v;
    value_init(&v);

    // Use a longer string to make the optimization more likely to be triggered.
    const char* test_str = "a long string to test the seeking logic"; // All ASCII
    set_null_string(&v, test_str);
    printf("Testing with string of length %zu\n", v.string_chars);
    CHECK_STATE(&v);

    // Position the cursor near the end of the string.
    size_t near_end_pos = v.string_chars - 3;
    string_set_byte_pos(&v, near_end_pos);
    printf("  Positioned at char %zu. Seeking to invalid pos %zu...\n", near_end_pos, v.string_chars + 1);
    CHECK_STATE(&v);

    // Now, from this high position, jump to an invalid position.
    // `diff` will be small and positive.
    // The buggy check `v->string_chars - 1 - new_string_char_pos < diff` will
    // underflow and likely evaluate to `false`, causing the code to seek forward
    // from the current position and run off the end of the buffer.
    string_set_byte_pos(&v, v.string_chars + 1);

    printf("  State after flawed seek: string_pos=%zu, string_char_pos=%zu\n", v.string_pos, v.string_char_pos);
    CHECK_STATE(&v); // <-- EXPECTED TO FAIL ON ORIGINAL CODE

    clear_value(&v);
    printf("--- Flawed Optimization Tests Finished ---\n");
}

void test_utf_status_flags() {
#ifndef NUTF8
    const uint32_t utf_known = RXFLAG_VM_UTF8_VALID | RXFLAG_VM_UTF8_COUNT_VALID;
    value valid;
    value copy;
    value appended;
    value concatenated;
    value bad;
    char invalid_bytes[] = { 'o', 'k', (char)0xff };

    printf("\n--- Running UTF Status Flag Tests ---\n");

    value_init(&valid);
    value_init(&copy);
    value_init(&appended);
    value_init(&concatenated);
    value_init(&bad);

    set_null_string(&valid, "Hello ©");
    CHECK_STATUS(&valid, utf_known, utf_known);
    CHECK_STATE(&valid);

    copy_string_value(&copy, &valid);
    CHECK_STATUS(&copy, utf_known, utf_known);
    CHECK_STATE(&copy);

    set_null_string(&appended, "prefix ");
    string_append(&appended, &valid);
    CHECK_STATUS(&appended, utf_known, utf_known);
    CHECK_STATE(&appended);

    string_concat(&concatenated, &valid, &copy);
    CHECK_STATUS(&concatenated, utf_known, utf_known);
    CHECK_STATE(&concatenated);

    set_string(&bad, invalid_bytes, sizeof(invalid_bytes));
    CHECK_STATUS(&bad, utf_known, 0);

    string_append(&bad, &valid);
    CHECK_STATUS(&bad, utf_known, 0);

    clear_value(&valid);
    clear_value(&copy);
    clear_value(&appended);
    clear_value(&concatenated);
    clear_value(&bad);

    printf("--- UTF Status Flag Tests Finished ---\n");
#endif
}

void test_binary_buffers() {
    value v;
    value copy;
    value other;
    value concat;
    value slice;
    unsigned char initial[] = { 0x00, 0x41, 0xff };
    unsigned char smaller[] = { 0x10, 0x20 };
    unsigned char extra[] = { 0x30, 0x40, 0x50 };
    unsigned char appended[] = { 0x10, 0x20, 0x30, 0x40, 0x50 };
    unsigned char doubled[] = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x10, 0x20, 0x30, 0x40, 0x50 };
    unsigned char prefix[] = { 0xaa };
    unsigned char prefix_appended[] = { 0xaa, 0x10, 0x20, 0x30, 0x40, 0x50 };
    unsigned char combined[] = { 0xaa, 0x10, 0x20, 0x30, 0x40, 0x50, 0x10, 0x20, 0x30, 0x40, 0x50 };
    unsigned char sliced[] = { 0x10, 0x20, 0x30 };
    size_t capacity;

    printf("\n--- Running Binary Buffer Tests ---\n");

    value_init(&v);
    value_init(&copy);
    value_init(&other);
    value_init(&concat);
    value_init(&slice);

    CHECK_RC_ZERO(set_binary(&v, initial, sizeof(initial)));
    CHECK_BINARY(&v, initial, sizeof(initial));
    CHECK_SIZE_EQUAL(v.binary_pos, 0, "binary_pos after binary write");
    capacity = v.binary_buffer_length;

    v.binary_pos = 2;
    CHECK_RC_ZERO(set_binary(&v, smaller, sizeof(smaller)));
    CHECK_BINARY(&v, smaller, sizeof(smaller));
    CHECK_SIZE_EQUAL(v.binary_pos, 0, "binary_pos after replacing binary");
    CHECK_SIZE_EQUAL(v.binary_buffer_length, capacity, "binary_buffer_length after smaller binary write");

    v.binary_pos = 1;
    CHECK_RC_ZERO(append_binary(&v, extra, sizeof(extra)));
    CHECK_BINARY(&v, appended, sizeof(appended));
    CHECK_SIZE_EQUAL(v.binary_pos, 1, "binary_pos after binary append");

    copy_value(&copy, &v);
    CHECK_BINARY(&copy, appended, sizeof(appended));
    CHECK_SIZE_EQUAL(copy.binary_pos, 1, "binary_pos after binary copy");

    CHECK_RC_ZERO(append_binary_value(&v, &v));
    CHECK_BINARY(&v, doubled, sizeof(doubled));
    CHECK_SIZE_EQUAL(v.binary_pos, 1, "binary_pos after self append");

    CHECK_RC_ZERO(set_binary(&other, prefix, sizeof(prefix)));
    CHECK_RC_ZERO(concat_binary(&concat, &other, &v));
    CHECK_BINARY(&concat, combined, sizeof(combined));
    CHECK_SIZE_EQUAL(concat.binary_pos, 0, "binary_pos after binary concat");

    other.binary_pos = 1;
    CHECK_RC_ZERO(concat_binary(&other, &other, &copy));
    CHECK_BINARY(&other, prefix_appended, sizeof(prefix_appended));
    CHECK_SIZE_EQUAL(other.binary_pos, 0, "binary_pos after in-place binary concat");

    CHECK_RC_ZERO(slice_binary(&slice, &concat, 1, 3));
    CHECK_BINARY(&slice, sliced, sizeof(sliced));
    CHECK_SIZE_EQUAL(slice.binary_pos, 0, "binary_pos after binary slice");

    concat.binary_pos = 4;
    CHECK_RC_ZERO(slice_binary(&concat, &concat, 50, 8));
    CHECK_SIZE_EQUAL(concat.binary_length, 0, "binary_length after out-of-range slice");
    CHECK_SIZE_EQUAL(concat.binary_pos, 0, "binary_pos after out-of-range slice");

    CHECK_RC_ZERO(set_binary(&other, 0, 0));
    CHECK_RC_ZERO(concat_binary(&other, &other, &other));
    CHECK_SIZE_EQUAL(other.binary_length, 0, "binary_length after empty self concat");
    CHECK_SIZE_EQUAL(other.binary_pos, 0, "binary_pos after empty self concat");

    clear_value(&v);
    clear_value(&copy);
    clear_value(&other);
    clear_value(&concat);
    clear_value(&slice);

    printf("--- Binary Buffer Tests Finished ---\n");
}

void test_reference_cells() {
    value local;
    value attribute;
    rxvm_reference_cell *cell;

    printf("\n--- Running Reference Cell Tests ---\n");

    value_init(&local);
    value_init(&attribute);
    CHECK_POINTER_EQUAL(local.reference_identity, 0, "new value reference_identity");
    CHECK_POINTER_EQUAL(local.reference_payload, 0, "new value reference_payload");

    cell = rxvm_reference_cell_create(RXVM_REF_LOCAL, &local, 0, 1, "local");
    CHECK_POINTER_NOT_NULL(cell, "created reference cell");
    CHECK_SIZE_EQUAL(cell->retain_count, 1, "initial retain_count");
    CHECK_INT_EQUAL(cell->state, RXVM_REF_VALID, "initial reference state");
    CHECK_INT_EQUAL(cell->owner_kind, RXVM_REF_LOCAL, "initial owner kind");
    CHECK_POINTER_EQUAL(rxvm_reference_cell_target(cell), &local, "initial reference target");
    CHECK_SIZE_EQUAL(cell->owner_generation, 1, "initial owner generation");

    rxvm_reference_cell_retain(cell);
    CHECK_SIZE_EQUAL(cell->retain_count, 2, "retain_count after retain");

    rxvm_reference_cell_invalidate(cell);
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(cell), 0, "invalidated reference validity");
    CHECK_POINTER_EQUAL(rxvm_reference_cell_target(cell), 0, "invalidated reference target");
    CHECK_SIZE_EQUAL(cell->retain_count, 2, "retain_count after invalidate");

    rxvm_reference_cell_retarget(cell, RXVM_REF_ATTRIBUTE, &attribute, &local, 2, "attr");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(cell), 1, "retargeted reference validity");
    CHECK_INT_EQUAL(cell->owner_kind, RXVM_REF_ATTRIBUTE, "retargeted owner kind");
    CHECK_POINTER_EQUAL(rxvm_reference_cell_target(cell), &attribute, "retargeted reference target");
    CHECK_POINTER_EQUAL(cell->owner, &local, "retargeted owner");
    CHECK_SIZE_EQUAL(cell->owner_generation, 2, "retargeted owner generation");

    rxvm_reference_cell_release(cell);
    CHECK_SIZE_EQUAL(cell->retain_count, 1, "retain_count after release");
    rxvm_reference_cell_release(cell);

    clear_value(&local);
    clear_value(&attribute);
    printf("--- Reference Cell Tests Finished ---\n");
}

void test_reference_identity_helpers() {
    value target;
    value other;
    rxvm_reference_cell *cell;
    rxvm_reference_cell *same_cell;
    uint64_t original_id;

    printf("\n--- Running Reference Identity Helper Tests ---\n");

    value_init(&target);
    value_init(&other);

    cell = rxvm_reference_identity_for(&target, RXVM_REF_ARGUMENT, 0, 10, "arg");
    CHECK_POINTER_NOT_NULL(cell, "created identity cell");
    CHECK_POINTER_EQUAL(target.reference_identity, cell, "target identity pointer");
    CHECK_POINTER_EQUAL(rxvm_reference_cell_target(cell), &target, "identity target");
    CHECK_SIZE_EQUAL(cell->retain_count, 1, "identity retain_count");
    CHECK_INT_EQUAL(cell->owner_kind, RXVM_REF_ARGUMENT, "identity owner kind");
    original_id = cell->id;

    same_cell = rxvm_reference_identity_for(&target, RXVM_REF_GLOBAL, &other, 11, "global");
    CHECK_POINTER_EQUAL(same_cell, cell, "canonical identity cell");
    CHECK_SIZE_EQUAL(cell->id, original_id, "canonical identity id");
    CHECK_INT_EQUAL(cell->owner_kind, RXVM_REF_GLOBAL, "retargeted identity owner kind");
    CHECK_POINTER_EQUAL(cell->owner, &other, "retargeted identity owner");
    CHECK_POINTER_EQUAL(rxvm_reference_cell_target(cell), &target, "retargeted identity target");

    rxvm_reference_cell_retain(cell);
    CHECK_SIZE_EQUAL(cell->retain_count, 2, "identity retain_count after payload retain");

    rxvm_reference_identity_invalidate(&target);
    CHECK_POINTER_EQUAL(target.reference_identity, cell, "identity pointer after invalidate");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(cell), 0, "identity validity after invalidate");

    same_cell = rxvm_reference_identity_for(&target, RXVM_REF_LOCAL, 0, 12, "local");
    CHECK_POINTER_EQUAL(same_cell, cell, "reused identity cell after invalidate");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(cell), 1, "identity validity after reuse");
    CHECK_POINTER_EQUAL(rxvm_reference_cell_target(cell), &target, "identity target after reuse");

    rxvm_reference_identity_release(&target);
    CHECK_POINTER_EQUAL(target.reference_identity, 0, "identity pointer after release");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(cell), 0, "identity validity after release");
    CHECK_SIZE_EQUAL(cell->retain_count, 1, "identity retain_count after storage release");
    rxvm_reference_cell_release(cell);

    clear_value(&target);
    clear_value(&other);
    printf("--- Reference Identity Helper Tests Finished ---\n");
}

int main() {
    test_string_positioning();
    test_boundary_conditions();
    test_flawed_optimization_trigger();
    test_utf_status_flags();
    test_binary_buffers();
    test_reference_cells();
    test_reference_identity_helpers();
    // Add more tests with other strings (empty, all-ASCII, all-multibyte, etc.)

    printf("\nAll tests completed.\n");
    return 0;
}
