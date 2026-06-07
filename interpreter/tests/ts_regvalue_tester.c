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

void test_reference_context_allocator() {
    rxvm_reference_context refs;
    value target;
    value other;
    rxvm_reference_cell *cell;
    rxvm_reference_cell *reused_cell;
    uint64_t first_id;

    printf("\n--- Running Reference Context Allocator Tests ---\n");

    rxvm_reference_context_init(&refs);
    value_init(&target);
    value_init(&other);

    cell = rxvm_reference_identity_for_context(&refs, &target, RXVM_REF_LOCAL, 0, 60, "ctx-target");
    CHECK_POINTER_NOT_NULL(cell, "context identity cell");
    CHECK_SIZE_EQUAL(cell->id, 1, "first context reference id");
    CHECK_POINTER_EQUAL(cell->context, &refs, "reference context pointer");
    CHECK_SIZE_EQUAL(refs.active_count, 1, "active count after context create");
    CHECK_SIZE_EQUAL(refs.free_count, 0, "free count after context create");
    CHECK_POINTER_EQUAL(rxvm_reference_context_find(&refs, cell->id), cell, "context root lookup");
    first_id = cell->id;

    rxvm_reference_cell_retain(cell);
    rxvm_reference_identity_release(&target);
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(cell), 0, "context cell invalid after identity release");
    CHECK_SIZE_EQUAL(refs.active_count, 1, "retained invalid cell remains active");
    CHECK_SIZE_EQUAL(refs.free_count, 0, "retained invalid cell not on free list");

    rxvm_reference_cell_release(cell);
    CHECK_SIZE_EQUAL(refs.active_count, 0, "active count after last release");
    CHECK_SIZE_EQUAL(refs.free_count, 1, "free count after last release");
    CHECK_POINTER_EQUAL(rxvm_reference_context_find(&refs, first_id), 0, "released cell removed from root lookup");

    reused_cell = rxvm_reference_identity_for_context(&refs, &other, RXVM_REF_GLOBAL, 0, 61, "ctx-other");
    CHECK_POINTER_EQUAL(reused_cell, cell, "context free-list cell reused");
    CHECK_SIZE_EQUAL(reused_cell->id, 2, "reused cell gets fresh context id");
    CHECK_SIZE_EQUAL(refs.active_count, 1, "active count after reuse");
    CHECK_SIZE_EQUAL(refs.free_count, 0, "free count after reuse");

    clear_value(&other);
    CHECK_SIZE_EQUAL(refs.active_count, 0, "active count after clearing reused target");
    CHECK_SIZE_EQUAL(refs.free_count, 1, "free count after clearing reused target");

    rxvm_reference_context_free(&refs);
    CHECK_SIZE_EQUAL(refs.active_count, 0, "active count after context free");
    CHECK_SIZE_EQUAL(refs.free_count, 0, "free count after context free");

    clear_value(&target);
    printf("--- Reference Context Allocator Tests Finished ---\n");
}

void test_reference_payload_copy_and_clear() {
    value target;
    value ref;
    value copy;
    rxvm_reference_cell *cell;

    printf("\n--- Running Reference Payload Copy/Clear Tests ---\n");

    value_init(&target);
    value_init(&ref);
    value_init(&copy);

    cell = rxvm_reference_identity_for(&target, RXVM_REF_LOCAL, 0, 20, "payload-target");
    CHECK_POINTER_NOT_NULL(cell, "payload target identity");
    rxvm_reference_value_set_payload(&ref, cell);
    CHECK_POINTER_EQUAL(ref.reference_payload, cell, "reference payload pointer");
    CHECK_SIZE_EQUAL(cell->retain_count, 2, "retain_count after setting payload");

    copy_value(&copy, &ref);
    CHECK_POINTER_EQUAL(copy.reference_payload, cell, "copied reference payload pointer");
    CHECK_POINTER_EQUAL(copy.reference_identity, 0, "copy does not inherit source identity");
    CHECK_SIZE_EQUAL(cell->retain_count, 3, "retain_count after copying payload");

    value_zero(&ref);
    CHECK_POINTER_EQUAL(ref.reference_payload, 0, "payload released by value_zero");
    CHECK_SIZE_EQUAL(cell->retain_count, 2, "retain_count after zeroing payload value");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(cell), 1, "target still valid after payload zero");

    clear_value(&copy);
    CHECK_POINTER_EQUAL(copy.reference_payload, 0, "payload released by clear_value");
    CHECK_SIZE_EQUAL(cell->retain_count, 1, "retain_count after clearing payload copy");

    rxvm_reference_value_set_payload(&ref, cell);
    CHECK_SIZE_EQUAL(cell->retain_count, 2, "retain_count after resetting payload");
    set_null_string(&ref, "plain-value");
    CHECK_POINTER_EQUAL(ref.reference_payload, 0, "string assignment releases reference payload");
    CHECK_SIZE_EQUAL(cell->retain_count, 1, "retain_count after scalar assignment");

    rxvm_reference_cell_retain(cell);
    clear_value(&target);
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(cell), 0, "target identity invalidated by clear_value");
    CHECK_SIZE_EQUAL(cell->retain_count, 1, "retain_count after clearing target storage");
    rxvm_reference_cell_release(cell);

    clear_value(&ref);
    printf("--- Reference Payload Copy/Clear Tests Finished ---\n");
}

void test_reference_copy_preserves_destination_identity() {
    value source;
    value dest;
    rxvm_reference_cell *source_identity;
    rxvm_reference_cell *dest_identity;

    printf("\n--- Running Reference Copy Destination Identity Tests ---\n");

    value_init(&source);
    value_init(&dest);

    set_null_string(&source, "source");
    source_identity = rxvm_reference_identity_for(&source, RXVM_REF_LOCAL, 0, 30, "source");
    dest_identity = rxvm_reference_identity_for(&dest, RXVM_REF_LOCAL, 0, 31, "dest");

    CHECK_POINTER_NOT_NULL(source_identity, "source identity");
    CHECK_POINTER_NOT_NULL(dest_identity, "dest identity");
    CHECK_POINTER_EQUAL(dest.reference_identity, dest_identity, "destination identity before copy");

    copy_value(&dest, &source);
    CHECK_POINTER_EQUAL(dest.reference_identity, dest_identity, "destination identity preserved by copy");
    CHECK_POINTER_EQUAL(rxvm_reference_cell_target(dest_identity), &dest, "destination identity target after copy");
    CHECK_POINTER_EQUAL(source.reference_identity, source_identity, "source identity preserved by copy");
    CHECK_POINTER_EQUAL(rxvm_reference_cell_target(source_identity), &source, "source identity target after copy");
    CHECK_POINTER_EQUAL(dest.reference_payload, source.reference_payload, "ordinary copy has no reference payload");
    CHECK_SIZE_EQUAL(dest.string_length, source.string_length, "copied string length");

    clear_value(&source);
    clear_value(&dest);
    printf("--- Reference Copy Destination Identity Tests Finished ---\n");
}

void test_reference_move_transfers_identity_and_payload() {
    value target;
    value source;
    value dest;
    rxvm_reference_cell *payload_cell;
    rxvm_reference_cell *source_identity;
    rxvm_reference_cell *dest_identity;

    printf("\n--- Running Reference Move Identity/Payload Tests ---\n");

    value_init(&target);
    value_init(&source);
    value_init(&dest);

    payload_cell = rxvm_reference_identity_for(&target, RXVM_REF_GLOBAL, 0, 40, "payload");
    source_identity = rxvm_reference_identity_for(&source, RXVM_REF_LOCAL, 0, 41, "source");
    dest_identity = rxvm_reference_identity_for(&dest, RXVM_REF_LOCAL, 0, 42, "dest");
    rxvm_reference_cell_retain(dest_identity);
    rxvm_reference_value_set_payload(&source, payload_cell);

    CHECK_SIZE_EQUAL(payload_cell->retain_count, 2, "payload retain_count before move");
    CHECK_POINTER_EQUAL(source.reference_identity, source_identity, "source identity before move");
    CHECK_POINTER_EQUAL(dest.reference_identity, dest_identity, "dest identity before move");

    move_value(&dest, &source);
    CHECK_POINTER_EQUAL(source.reference_identity, 0, "source identity cleared by move");
    CHECK_POINTER_EQUAL(source.reference_payload, 0, "source payload cleared by move");
    CHECK_POINTER_EQUAL(dest.reference_identity, source_identity, "source identity transferred to dest");
    CHECK_POINTER_EQUAL(rxvm_reference_cell_target(source_identity), &dest, "transferred identity retargeted to dest");
    CHECK_POINTER_EQUAL(dest.reference_payload, payload_cell, "payload moved to dest");
    CHECK_SIZE_EQUAL(payload_cell->retain_count, 2, "payload retain_count unchanged by move");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(dest_identity), 0, "old dest identity invalidated by move");
    CHECK_SIZE_EQUAL(dest_identity->retain_count, 1, "old dest identity retained only by test");
    rxvm_reference_cell_release(dest_identity);

    clear_value(&dest);
    CHECK_SIZE_EQUAL(payload_cell->retain_count, 1, "payload retain_count after clearing moved dest");
    clear_value(&target);
    clear_value(&source);
    printf("--- Reference Move Identity/Payload Tests Finished ---\n");
}

void test_reference_attribute_storage_lifecycle() {
    value array;
    value *first;
    value *second;
    value *third;
    rxvm_reference_cell *first_cell;
    rxvm_reference_cell *second_cell;
    rxvm_reference_cell *third_cell;

    printf("\n--- Running Reference Attribute Storage Lifecycle Tests ---\n");

    value_init(&array);
    set_num_attributes(&array, 3);
    first = array.attributes[0];
    second = array.attributes[1];
    third = array.attributes[2];

    first_cell = rxvm_reference_identity_for(first, RXVM_REF_ATTRIBUTE, &array, 50, "first");
    second_cell = rxvm_reference_identity_for(second, RXVM_REF_ATTRIBUTE, &array, 50, "second");
    third_cell = rxvm_reference_identity_for(third, RXVM_REF_ATTRIBUTE, &array, 50, "third");
    rxvm_reference_cell_retain(first_cell);
    rxvm_reference_cell_retain(second_cell);
    rxvm_reference_cell_retain(third_cell);

    delete_attributes(&array, 0, 1);
    CHECK_SIZE_EQUAL(array.num_attributes, 2, "attribute count after delete");
    CHECK_POINTER_EQUAL(array.attributes[0], second, "second physical storage moved to first logical slot");
    CHECK_POINTER_EQUAL(array.attributes[1], third, "third physical storage moved to second logical slot");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(first_cell), 0, "deleted first storage invalidated");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(second_cell), 1, "second storage remains valid after delete");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(third_cell), 1, "third storage remains valid after delete");
    CHECK_POINTER_EQUAL(rxvm_reference_cell_target(second_cell), second, "second reference follows physical storage");
    CHECK_POINTER_EQUAL(rxvm_reference_cell_target(third_cell), third, "third reference follows physical storage");

    set_num_attributes(&array, 1);
    CHECK_SIZE_EQUAL(array.num_attributes, 1, "attribute count after shrink");
    CHECK_POINTER_EQUAL(array.attributes[0], second, "remaining physical storage after shrink");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(second_cell), 1, "remaining storage stays valid after shrink");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(third_cell), 0, "shrunk-away storage invalidated");

    clear_value(&array);
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(second_cell), 0, "remaining storage invalidated by parent clear");
    CHECK_SIZE_EQUAL(first_cell->retain_count, 1, "first test retain after parent clear");
    CHECK_SIZE_EQUAL(second_cell->retain_count, 1, "second test retain after parent clear");
    CHECK_SIZE_EQUAL(third_cell->retain_count, 1, "third test retain after parent clear");
    rxvm_reference_cell_release(first_cell);
    rxvm_reference_cell_release(second_cell);
    rxvm_reference_cell_release(third_cell);

    printf("--- Reference Attribute Storage Lifecycle Tests Finished ---\n");
}

void test_reference_attribute_trim_policy() {
    value array;
    value *old_first;
    value *old_removed;
    rxvm_reference_cell *first_cell;
    rxvm_reference_cell *removed_cell;
    size_t large_capacity;

    printf("\n--- Running Reference Attribute Trim Policy Tests ---\n");

    value_init(&array);
    set_num_attributes(&array, 128);
    large_capacity = array.max_num_attributes;
    old_first = array.attributes[0];
    old_removed = array.attributes[64];

    first_cell = rxvm_reference_identity_for(old_first, RXVM_REF_ATTRIBUTE, &array, 70, "trim-first");
    removed_cell = rxvm_reference_identity_for(old_removed, RXVM_REF_ATTRIBUTE, &array, 70, "trim-removed");
    rxvm_reference_cell_retain(first_cell);
    rxvm_reference_cell_retain(removed_cell);

    set_num_attributes(&array, 4);
    CHECK_SIZE_EQUAL(array.num_attributes, 4, "trimmed attribute count");
    CHECK_INT_EQUAL(array.max_num_attributes < large_capacity, 1, "extreme shrink trims capacity");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(first_cell), 1, "active storage remains valid after trim");
    CHECK_POINTER_EQUAL(rxvm_reference_cell_target(first_cell), array.attributes[0], "active reference retargeted after trim");
    CHECK_INT_EQUAL(rxvm_reference_cell_target(first_cell) != old_first, 1, "active reference moved to compacted storage");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(removed_cell), 0, "removed storage invalidated by shrink");

    clear_value(&array);
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(first_cell), 0, "trimmed active storage invalidated by clear");
    CHECK_SIZE_EQUAL(first_cell->retain_count, 1, "first trim test retain after clear");
    CHECK_SIZE_EQUAL(removed_cell->retain_count, 1, "removed trim test retain after clear");
    rxvm_reference_cell_release(first_cell);
    rxvm_reference_cell_release(removed_cell);

    printf("--- Reference Attribute Trim Policy Tests Finished ---\n");
}

void test_reference_lifetime_release_helper() {
    value root;
    value target;
    rxvm_reference_cell *root_cell;
    rxvm_reference_cell *attr_cell;
    rxvm_reference_cell *payload_cell;

    printf("\n--- Running Reference Lifetime Release Helper Tests ---\n");

    value_init(&root);
    value_init(&target);
    set_num_attributes(&root, 1);

    root_cell = rxvm_reference_identity_for(&root, RXVM_REF_LOCAL, 0, 80, "lifetime-root");
    attr_cell = rxvm_reference_identity_for(root.attributes[0], RXVM_REF_ATTRIBUTE, &root, 80, "lifetime-attr");
    payload_cell = rxvm_reference_identity_for(&target, RXVM_REF_LOCAL, 0, 80, "lifetime-payload");

    rxvm_reference_cell_retain(root_cell);
    rxvm_reference_cell_retain(attr_cell);
    rxvm_reference_cell_retain(payload_cell);
    rxvm_reference_value_set_payload(root.attributes[0], payload_cell);

    CHECK_SIZE_EQUAL(payload_cell->retain_count, 3, "payload retain before lifetime release");
    release_value_reference_lifetime(&root);

    CHECK_POINTER_EQUAL(root.reference_identity, 0, "root identity released by lifetime release");
    CHECK_POINTER_EQUAL(root.attributes[0]->reference_identity, 0, "attribute identity released by lifetime release");
    CHECK_POINTER_EQUAL(root.attributes[0]->reference_payload, 0, "attribute payload released by lifetime release");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(root_cell), 0, "root cell invalid after lifetime release");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(attr_cell), 0, "attribute cell invalid after lifetime release");
    CHECK_INT_EQUAL(rxvm_reference_cell_is_valid(payload_cell), 1, "payload target remains valid after lifetime release");
    CHECK_SIZE_EQUAL(payload_cell->retain_count, 2, "payload retain after lifetime release");

    clear_value(&root);
    clear_value(&target);
    rxvm_reference_cell_release(root_cell);
    rxvm_reference_cell_release(attr_cell);
    rxvm_reference_cell_release(payload_cell);

    printf("--- Reference Lifetime Release Helper Tests Finished ---\n");
}

int main() {
    test_string_positioning();
    test_boundary_conditions();
    test_flawed_optimization_trigger();
    test_utf_status_flags();
    test_binary_buffers();
    test_reference_cells();
    test_reference_identity_helpers();
    test_reference_context_allocator();
    test_reference_payload_copy_and_clear();
    test_reference_copy_preserves_destination_identity();
    test_reference_move_transfers_identity_and_payload();
    test_reference_attribute_storage_lifecycle();
    test_reference_attribute_trim_policy();
    test_reference_lifetime_release_helper();
    // Add more tests with other strings (empty, all-ASCII, all-multibyte, etc.)

    printf("\nAll tests completed.\n");
    return 0;
}
