//
// Created by Adrian Sutherland on 28/06/2025.
//
#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "rxbin.h"
#include "rxvalue.h"
#include "rxvmvars.h" // Value functions to be tested
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
    assert(v.string_pos == v.string_length && "Byte position should be at the very end");

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

int main() {
    test_string_positioning();
    test_boundary_conditions();
    test_flawed_optimization_trigger();
    // Add more tests with other strings (empty, all-ASCII, all-multibyte, etc.)

    printf("\nAll tests completed.\n");
    return 0;
}