/* Tests for decNumber functions that have been changed or added for the crexx plugin */

/*
 * The following functions have been added or changed:
 * - decNumber * decNumberFromInt64(decNumber *dn, int64_t in) - used in the plugin
 * - decNumber * decNumberFromUInt64(decNumber *dn, uint64_t uin) - not used in the plugin
 * - int64_t decNumberToInt64(const decNumber *dn, decContext *set) - used in the plugin
 * - uint64_t decNumberToUInt64(const decNumber *dn, decContext *set) - not used in the plugin
 * - Int decNumberToInt32(const decNumber *dn, decContext *set) - to be used in 32-bit systems - fixed normalised integer input
 * - uInt decNumberToUInt32(const decNumber *dn, decContext *set) - not used in the plugin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "decNumber.h"

/* Return a decNumber big enough to hold the number */
static decNumber *getNumber(size_t digits) {
    /* Calculate the size of the decNumber */

    /* How many units are necessary, this is the ceil(digits/DECDPUN) */
    size_t units = (digits + DECDPUN - 1) / DECDPUN;
    size_t size;

    /* DECNUMUNITS is the default (minimum) number of units needed - do we need more? */
    if (units > DECNUMUNITS) {
        size = sizeof(decNumber) + ((units - DECNUMUNITS) * sizeof(decNumberUnit));
    }
    else {
        size = sizeof(decNumber);
    }

    return malloc(size);
}

static int do_decNumberFromInt64_test(char* test, int64_t value) {
    int digits = 30;
    char expected[digits + 14];
    char buffer[digits + 14];
    int error = 0;

    sprintf(expected, "%lld", value);
    /* Get a malloced DecNumber */
    decNumber *dn = getNumber(digits);

    printf("%s",test);
    decNumberFromInt64(dn, value);
    decNumberToString(dn, buffer);
    if (strcmp(buffer, expected) != 0) {
        error = 1;
        printf("- ERROR - Expected: %s - Got: %s", expected, buffer);
    }
    printf("\n");
    free(dn);

    return error;
}

// Test decNumber * decNumberFromInt64(decNumber *dn, int64_t in)
int test_decNumberFromInt64() {
    int errors = 0;

    // Tests a positive number
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 with a positive number", 1000);

    // Tests a negative number
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 with a negative number", -1000);

    // Test with a large number > 32 bits
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 with a large number", 0x7FFFFFFFFFFFFFFF);

    // Test with a large negative number > 32 bits
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 with a large negative number", -0x7FFFFFFFFFFFFFFF);

    // Test with 0
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 with 0", 0);

    // Test with INT64_MAX (9223372036854775807)
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 with INT64_MAX", INT64_MAX);

    // Test with INT64_MIN (-9223372036854775808)
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 with INT64_MIN", INT64_MIN);

    // Test with 1
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 with 1", 1);

    // Test with -1
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 with -1", -1);

    // Boundary Conditions Around Limits - Near INT64_MAX Test 9223372036854775806 (one less than INT64_MAX)
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 Boundary Conditions Around Limits - Near INT64_MAX Test 9223372036854775806 (one less than INT64_MAX)", 9223372036854775806);

    // Boundary Conditions Around Limits - Near INT64_MAX Test 9223372036854775800 (close to INT64_MAX but with trailing zeros)
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 Boundary Conditions Around Limits - Near INT64_MAX Test 9223372036854775800 (close to INT64_MAX but with trailing zeros)", 9223372036854775800);

    // Boundary Conditions Around Limits - Near INT64_MIN: Test -9223372036854775807 (one more than INT64_MIN)
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 Boundary Conditions Around Limits - Near INT64_MIN: Test -9223372036854775807 (one more than INT64_MIN)", -9223372036854775807);

    // Boundary Conditions Around Limits - Near INT64_MIN: Test -9223372036854775800 (close to INT64_MIN but with trailing zeros)
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 Boundary Conditions Around Limits - Near INT64_MIN: Test -9223372036854775800 (close to INT64_MIN but with trailing zeros)", -9223372036854775800);

    // Mid-Range Large Values: Positive: 1234567890123456789
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 Mid-Range Large Values: Positive: 1234567890123456789", 1234567890123456789);

    // Mid-Range Large Values: Negative: -1234567890123456789
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 Mid-Range Large Values: Negative: -1234567890123456789", -1234567890123456789);

    // Values that are exactly powers of 10: 1000000000
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 Values that are exactly powers of 10: 1000000000", 1000000000);

    // Values that are exactly powers of 10: 1000000000000000000
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 Values that are exactly powers of 10: 1000000000000000000", 1000000000000000000);

    // Values that are exactly powers of 10: -1000000000
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 Values that are exactly powers of 10: -1000000000", -1000000000);

    // Values that are exactly powers of 10: -1000000000000000000
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 Values that are exactly powers of 10: -1000000000000000000", -1000000000000000000);

    // Values that are exactly powers of 10: 10
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 Values that are exactly powers of 10: 10", 10);

    // Values that are exactly powers of 10: -10
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 Values that are exactly powers of 10: -10", -10);

    // All 9's: 999999999999999999
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 All 9's: 999999999999999999", 999999999999999999);

    // All 9's: -999999999999999999
    errors += do_decNumberFromInt64_test("Test decNumberFromInt64 All 9's: -999999999999999999", -999999999999999999);

    return errors;
}

static int do_decNumberFromUInt64_test(char* test, u_int64_t value) {
    int digits = 30;
    char expected[digits + 14];
    char buffer[digits + 14];
    int error = 0;

    sprintf(expected, "%llu", value);
    /* Get a malloced DecNumber */
    decNumber *dn = getNumber(digits);

    printf("%s",test);
    decNumberFromUInt64(dn, value);
    decNumberToString(dn, buffer);
    if (strcmp(buffer, expected) != 0) {
        error = 1;
        printf("- ERROR - Expected: %s - Got: %s", expected, buffer);
    }
    printf("\n");
    free(dn);

    return error;
}

// Test decNumber * decNumberFromUInt64(decNumber *dn, u_int64_t in)
int test_decNumberFromUInt64() {
    int errors = 0;

    // Tests a positive number
    errors += do_decNumberFromUInt64_test("Test decNumberFromUInt64 with a positive number", 1000);

    // Test with a large number > 32 bits
    errors += do_decNumberFromUInt64_test("Test decNumberFromUInt64 with a large unsigned number", 0xFFFFFFFFFFFFFFFF);

    // Test with 0
    errors += do_decNumberFromUInt64_test("Test decNumberFromUInt64 with 0", 0);

    // Test with UINT64_MAX (18446744073709551615)
    errors += do_decNumberFromUInt64_test("Test decNumberFromUInt64 with INT64_MAX", UINT64_MAX);

    // Test with 1
    errors += do_decNumberFromUInt64_test("Test decNumberFromUInt64 with 1", 1);

    // Boundary Conditions Around Limits - Near UINT64_MAX - one less than INT64_MAX - 18446744073709551614
    errors += do_decNumberFromUInt64_test("Test decNumberFromUInt64 Boundary Conditions Around Limits - Near INT64_MAX Test 18446744073709551614 (one less than UINT64_MAX)", 18446744073709551614ULL);

    // Boundary Conditions Around Limits - Near UINT64_MAX Test, close to INT64_MAX but with trailing zeros - 18446744073709551610
    errors += do_decNumberFromUInt64_test("Test decNumberFromUInt64 Boundary Conditions Around Limits - Near UINT64_MAX Test 18446744073709551610 (close to UINT64_MAX but with trailing zeros)", 18446744073709551610ULL);

    // Mid-Range Large Values: Unsigned Positive: 12345678901234567890
    errors += do_decNumberFromUInt64_test("Test decNumberFromUInt64 Mid-Range Large Values: Positive: 12345678901234567890", 12345678901234567890ULL);

    // Values that are exactly powers of 10: 10000000000
    errors += do_decNumberFromUInt64_test("Test decNumberFromUInt64 Values that are exactly powers of 10: 10000000000", 10000000000);

    // Values that are exactly powers of 10: 10000000000000000000
    errors += do_decNumberFromUInt64_test("Test decNumberFromUInt64 Values that are exactly powers of 10: 10000000000000000000", 10000000000000000000ULL);

    // Values that are exactly powers of 10: 10
    errors += do_decNumberFromUInt64_test("Test decNumberFromUInt64 Values that are exactly powers of 10: 10", 10);

    // All 9's: 9999999999999999999
    errors += do_decNumberFromUInt64_test("Test decNumberFromUInt64 All 9's: 9999999999999999999", 9999999999999999999ULL);

    return errors;
}

static int do_decNumberToInt64_test(char* test, int64_t value) {
    int digits = 30;
    char expected[digits + 14];
    int error = 0;

    sprintf(expected, "%lld", value);

    // Allocate memory for the context
    decContext* context = malloc(sizeof(decContext));
    decContextDefault(context, DEC_INIT_BASE); // initialize
    context->traps = 0; // no traps
    context->digits = digits; // set precision

    /* Get a malloced DecNumber */
    decNumber *dn = getNumber(digits);

    printf("%s",test);
    decNumberFromInt64(dn, value);
    int64_t result = decNumberToInt64(dn, context);
    if (result != value) {
        error = 1;
        printf("- ERROR - Expected: %lld - Got: %lld", value, result);
    }
    printf("\n");

    printf("%s (after normalisation)",test);
    decNumberFromInt64(dn, value);
    decNumberNormalize(dn, dn, context);
    result = decNumberToInt64(dn, context);
    if (result != value) {
        error = 1;
        printf("- ERROR - Expected: %lld - Got: %lld", value, result);
    }
    printf("\n");

    free(dn);
    free(context);

    return error;
}

// Test int64_t decNumberToInt64(const decNumber *dn, decContext *set)
int test_decNumberToInt64() {
    int errors = 0;

    // Tests a positive number
    errors += do_decNumberToInt64_test("Test decNumberToInt64 with a positive number", 1000);

    // Tests a negative number
    errors += do_decNumberToInt64_test("Test decNumberToInt64 with a negative number", -1000);

    // Test with a large number > 32 bits
    errors += do_decNumberToInt64_test("Test decNumberToInt64 with a large number", 0x7FFFFFFFFFFFFFFF);

    // Test with a large negative number > 32 bits
    errors += do_decNumberToInt64_test("Test decNumberToInt64 with a large negative number", -0x7FFFFFFFFFFFFFFF);

    // Test with 0
    errors += do_decNumberToInt64_test("Test decNumberToInt64 with 0", 0);

    // Test with INT64_MAX (9223372036854775807)
    errors += do_decNumberToInt64_test("Test decNumberToInt64 with INT64_MAX", INT64_MAX);

    // Test with INT64_MIN (-9223372036854775808)
    errors += do_decNumberToInt64_test("Test decNumberToInt64 with INT64_MIN", INT64_MIN);

    // Test with 1
    errors += do_decNumberToInt64_test("Test decNumberToInt64 with 1", 1);

    // Test with -1
    errors += do_decNumberToInt64_test("Test decNumberToInt64 with -1", -1);

    // Boundary Conditions Around Limits - Near INT64_MAX Test 9223372036854775806 (one less than INT64_MAX)
    errors += do_decNumberToInt64_test("Test decNumberToInt64 Boundary Conditions Around Limits - Near INT64_MAX Test 9223372036854775806 (one less than INT64_MAX)", 9223372036854775806);

    // Boundary Conditions Around Limits - Near INT64_MAX Test 9223372036854775800 (close to INT64_MAX but with trailing zeros)
    errors += do_decNumberToInt64_test("Test decNumberToInt64 Boundary Conditions Around Limits - Near INT64_MAX Test 9223372036854775800 (close to INT64_MAX but with trailing zeros)", 9223372036854775800);

    // Boundary Conditions Around Limits - Near INT64_MIN: Test -9223372036854775807
    errors += do_decNumberToInt64_test("Test decNumberToInt64 Boundary Conditions Around Limits - Near INT64_MIN: Test -9223372036854775807", -9223372036854775807);

    // Boundary Conditions Around Limits - Near INT64_MIN: Test -9223372036854775800 (close to INT64_MIN but with trailing zeros)
    errors += do_decNumberToInt64_test("Test decNumberToInt64 Boundary Conditions Around Limits - Near INT64_MIN: Test -9223372036854775800", -9223372036854775800);

    // Mid-Range Large Values: Positive: 1234567890123456789
    errors += do_decNumberToInt64_test("Test decNumberToInt64 Mid-Range Large Values: Positive: 1234567890123456789", 1234567890123456789);

    // Mid-Range Large Values: Negative: -1234567890123456789
    errors += do_decNumberToInt64_test("Test decNumberToInt64 Mid-Range Large Values: Negative: -1234567890123456789", -1234567890123456789);

    // Values that are exactly powers of 10: 1000000000
    errors += do_decNumberToInt64_test("Test decNumberToInt64 Values that are exactly powers of 10: 1000000000", 1000000000);

    // Values that are exactly powers of 10: 1000000000000000000
    errors += do_decNumberToInt64_test("Test decNumberToInt64 Values that are exactly powers of 10: 1000000000000000000", 1000000000000000000);

    // Values that are exactly powers of 10: 10
    errors += do_decNumberToInt64_test("Test decNumberToInt64 Values that are exactly powers of 10: 10", 10);

    // Values that are exactly powers of 10: -10
    errors += do_decNumberToInt64_test("Test decNumberToInt64 Values that are exactly powers of 10: -10", -10);

    // All 9's: 999999999999999999
    errors += do_decNumberToInt64_test("Test decNumberToInt64 All 9's: 999999999999999999", 999999999999999999);

    // Now test a decimal number just bigger than INT64_MAX
    int digits = 30;

    // Allocate memory for the context
    decContext* context = malloc(sizeof(decContext));
    decContextDefault(context, DEC_INIT_BASE); // initialize
    context->traps = 0; // no traps
    context->digits = digits; // set precision

    /* Get a malloced DecNumber */
    decNumber *dn = getNumber(digits);

    // Create the number from a string - INT64_MAX + 1
    decNumberFromString(dn, "9223372036854775808", context);
    // Clear the context flags
    context->status = 0;
    int64_t result = decNumberToInt64(dn, context);
    // Check Context flags
    if (context->status & DEC_Invalid_operation) {
        printf("Test decNumberToInt64 with INT64_MAX + 1 - Got the expected Invalid_operation\n");
    }
    else {
        errors++;
        printf("Test decNumberToInt64 with INT64_MAX + 1 - ERROR, expecting: Invalid_operation - Got: %lld\n", result);
    }
    // Now test after normalisation
    decNumberNormalize(dn, dn, context);
    context->status = 0;
    result = decNumberToInt64(dn, context);
    // Check Context flags
    if (context->status & DEC_Invalid_operation) {
        printf("Test decNumberToInt64 with INT64_MAX + 1 (after normalisation) - Got the expected Invalid_operation\n");
    }
    else {
        errors++;
        printf("Test decNumberToInt64 with INT64_MAX + 1 (after normalisation) - ERROR, expecting: Invalid_operation - Got: %lld\n", result);
    }
    // Free the number and context
    free(dn);
    free(context);

    return errors;
}

static int do_decNumberToUInt64_test(char* test, u_int64_t value) {
    int digits = 30;
    char expected[digits + 14];
    int error = 0;

    sprintf(expected, "%llu", value);

    // Allocate memory for the context
    decContext* context = malloc(sizeof(decContext));
    decContextDefault(context, DEC_INIT_BASE); // initialize
    context->traps = 0; // no traps
    context->digits = digits; // set precision

    /* Get a malloced DecNumber */
    decNumber *dn = getNumber(digits);

    printf("%s",test);
    decNumberFromUInt64(dn, value);
    u_int64_t result = decNumberToUInt64(dn, context);
    if (result != value) {
        error = 1;
        printf("- ERROR - Expected: %llu - Got: %llu", value, result);
    }
    printf("\n");

    printf("%s (after normalisation)",test);
    decNumberFromUInt64(dn, value);
    decNumberNormalize(dn, dn, context);
    result = decNumberToUInt64(dn, context);
    if (result != value) {
        error = 1;
        printf("- ERROR - Expected: %llu - Got: %llu", value, result);
    }
    printf("\n");

    free(dn);
    free(context);

    return error;
}

// Test uint64_t decNumberToUInt64(const decNumber *dn, decContext *set)
int test_decNumberToUInt64() {
    int errors = 0;

    // Tests a positive number
    errors += do_decNumberToUInt64_test("Test decNumberToUInt64 with a positive number", 1000);

    // Test with a large number > 32 bits
    errors += do_decNumberToUInt64_test("Test decNumberToUInt64 with a large unsigned number", 0xFFFFFFFFFFFFFFFF);

    // Test with 0
    errors += do_decNumberToUInt64_test("Test decNumberToUInt64 with 0", 0);

    // Test with UINT64_MAX (18446744073709551615)
    errors += do_decNumberToUInt64_test("Test decNumberToUInt64 with UINT64_MAX", UINT64_MAX);

    // Test with 1
    errors += do_decNumberToUInt64_test("Test decNumberToUInt64 with 1", 1);

    // Boundary Conditions Around Limits - Near UINT64_MAX - one less than UINT64_MAX - 18446744073709551614
    errors += do_decNumberToUInt64_test("Test decNumberToUInt64 Boundary Conditions Around Limits - Near INT64_MAX Test 18446744073709551614 (one less than UINT64_MAX)", 18446744073709551614ULL);

    // Boundary Conditions Around Limits - Near UINT64_MAX Test (close to UINT64_MAX but with trailing zeros) - 18446744073709551610
    errors += do_decNumberToUInt64_test("Test decNumberToUInt64 Boundary Conditions Around Limits - Near UINT64_MAX Test 18446744073709551610 (close to UINT64_MAX but with trailing zeros)", 18446744073709551610ULL);

    // Mid-Range Large Values: Unsigned Positive: 12345678901234567890
    errors += do_decNumberToUInt64_test("Test decNumberToUInt64 Mid-Range Large Values: Positive: 12345678901234567890", 12345678901234567890ULL);

    // Values that are exactly powers of 10: 10000000000
    errors += do_decNumberToUInt64_test("Test decNumberToUInt64 Values that are exactly powers of 10: 10000000000", 10000000000);

    // Values that are exactly powers of 10: 100000000000000
    errors += do_decNumberToUInt64_test("Test decNumberToUInt64 Values that are exactly powers of 10: 100000000000000", 100000000000000);

    // Values that are exactly powers of 10: 10
    errors += do_decNumberToUInt64_test("Test decNumberToUInt64 Values that are exactly powers of 10: 10", 10);

    // All 9's: 9999999999999999999
    errors += do_decNumberToUInt64_test("Test decNumberToUInt64 All 9's: 9999999999999999999", 9999999999999999999ULL);

    // Now test a decimal number just bigger than UINT64_MAX
    int digits = 30;

    // Allocate memory for the context
    decContext* context = malloc(sizeof(decContext));
    decContextDefault(context, DEC_INIT_BASE); // initialize
    context->traps = 0; // no traps
    context->digits = digits; // set precision

    /* Get a malloced DecNumber */
    decNumber *dn = getNumber(digits);

    // Create the number from a string - UINT64_MAX + 1
    decNumberFromString(dn, "18446744073709551616", context);
    // Clear the context flags
    context->status = 0;
    u_int64_t result = decNumberToUInt64(dn, context);
    // Check Context flags
    if (context->status & DEC_Invalid_operation) {
        printf("Test decNumberToUInt64 with UINT64_MAX + 1 - Got the expected Invalid_operation\n");
    }
    else {
        errors++;
        printf("Test decNumberToUInt64 with UINT64_MAX + 1 - ERROR, expecting: Invalid_operation - Got: %lld\n", result);
    }
    // Now test after normalisation
    decNumberNormalize(dn, dn, context);
    context->status = 0;
    result = decNumberToUInt64(dn, context);
    // Check Context flags
    if (context->status & DEC_Invalid_operation) {
        printf("Test decNumberToUInt64 with UINT64_MAX + 1 (after normalisation) - Got the expected Invalid_operation\n");
    }
    else {
        errors++;
        printf("Test decNumberToUInt64 with UINT64_MAX + 1 (after normalisation) - ERROR, expecting: Invalid_operation - Got: %lld\n", result);
    }
    // Free the number and context
    free(dn);
    free(context);

    return errors;
}

static int do_decNumberToInt32_test(char* test, int32_t value) {
    int digits = 30;
    char expected[digits + 14];
    int error = 0;

    sprintf(expected, "%d", value);

    // Allocate memory for the context
    decContext* context = malloc(sizeof(decContext));
    decContextDefault(context, DEC_INIT_BASE); // initialize
    context->traps = 0; // no traps
    context->digits = digits; // set precision

    /* Get a malloced DecNumber */
    decNumber *dn = getNumber(digits);

    printf("%s",test);
    decNumberFromInt32(dn, value);
    int32_t result = decNumberToInt32(dn, context);
    if (result != value) {
        error = 1;
        printf("- ERROR - Expected: %d - Got: %d", value, result);
    }
    printf("\n");

    printf("%s (after normalisation)",test);
    decNumberFromInt32(dn, value);
    decNumberNormalize(dn, dn, context);
    result = decNumberToInt32(dn, context);
    if (result != value) {
        error = 1;
        printf("- ERROR - Expected: %d - Got: %d", value, result);
    }
    printf("\n");

    free(dn);
    free(context);

    return error;
}

// Test int32_t decNumberToInt32(const decNumber *dn, decContext *set)
int test_decNumberToInt32() {
    int errors = 0;

    // Tests a positive number
    errors += do_decNumberToInt32_test("Test decNumberToInt32 with a positive number", 1000);

    // Tests a negative number
    errors += do_decNumberToInt32_test("Test decNumberToInt32 with a negative number", -1000);

    // Test with a large number > 32 bits
    errors += do_decNumberToInt32_test("Test decNumberToInt32 with a large number", 0x7FFFFFFF);

    // Test with a large negative number > 32 bits
    errors += do_decNumberToInt32_test("Test decNumberToInt32 with a large negative number", -0x7FFFFFFF);

    // Test with 0
    errors += do_decNumberToInt32_test("Test decNumberToInt32 with 0", 0);

    // Test with INT32_MAX (2147483647)
    errors += do_decNumberToInt32_test("Test decNumberToInt32 with INT32_MAX", INT32_MAX);

    // Test with INT32_MIN (-2147483648)
    errors += do_decNumberToInt32_test("Test decNumberToInt32 with INT32_MIN", INT32_MIN);

    // Test with 1
    errors += do_decNumberToInt32_test("Test decNumberToInt32 with 1", 1);

    // Test with -1
    errors += do_decNumberToInt32_test("Test decNumberToInt32 with -1", -1);

    // Boundary Conditions Around Limits - Near INT32_MAX Test 2147483646 (one less than INT32_MAX)
    errors += do_decNumberToInt32_test("Test decNumberToInt32 Boundary Conditions Around Limits - Near INT32_MAX Test 2147483646 (one less than INT32_MAX)", 2147483646);

    // Boundary Conditions Around Limits - Near INT32_MAX Test 2147483640 (close to INT32_MAX but with trailing zeros)
    errors += do_decNumberToInt32_test("Test decNumberToInt32 Boundary Conditions Around Limits - Near INT32_MAX Test 2147483640 (close to INT32_MAX but with trailing zeros)", 2147483640);

    // Boundary Conditions Around Limits - Near INT32_MIN: Test -2147483647
    errors += do_decNumberToInt32_test("Test decNumberToInt32 Boundary Conditions Around Limits - Near INT32_MIN: Test -2147483647", -2147483647);

    // Boundary Conditions Around Limits - Near INT32_MIN: Test -2147483640 (close to INT32_MIN but with trailing zeros)
    errors += do_decNumberToInt32_test("Test decNumberToInt32 Boundary Conditions Around Limits - Near INT32_MIN: Test -2147483640", -2147483640);

    // Mid-Range Large Values: Positive: 1234567890
    errors += do_decNumberToInt32_test("Test decNumberToInt32 Mid-Range Large Values: Positive: 1234567890", 1234567890);

    // Mid-Range Large Values: Negative: -1234567890
    errors += do_decNumberToInt32_test("Test decNumberToInt32 Mid-Range Large Values: Negative: -1234567890", -1234567890);

    // Values that are exactly powers of 10: 1000000000
    errors += do_decNumberToInt32_test("Test decNumberToInt32 Values that are exactly powers of 10: 1000000000", 1000000000);

    // Values that are exactly powers of 10: 10000000
    errors += do_decNumberToInt32_test("Test decNumberToInt32 Values that are exactly powers of 10: 10000000", 10000000);

    // Values that are exactly powers of 10: 10
    errors += do_decNumberToInt32_test("Test decNumberToInt32 Values that are exactly powers of 10: 10", 10);

    // Values that are exactly powers of 10: -10
    errors += do_decNumberToInt32_test("Test decNumberToInt32 Values that are exactly powers of 10: -10", -10);

    // All 9's: 9999999
    errors += do_decNumberToInt32_test("Test decNumberToInt32 All 9's: 9999999", 9999999);

    // Now test a decimal number just bigger than INT32_MAX
    int digits = 30;

    // Allocate memory for the context
    decContext* context = malloc(sizeof(decContext));
    decContextDefault(context, DEC_INIT_BASE); // initialize
    context->traps = 0; // no traps
    context->digits = digits; // set precision

    /* Get a malloced DecNumber */
    decNumber *dn = getNumber(digits);

    // Create the number from a string - INT32_MAX + 1 (2147483647 + 1)
    decNumberFromString(dn, "2147483648", context);
    // Clear the context flags
    context->status = 0;
    int32_t result = decNumberToInt32(dn, context);
    // Check Context flags
    if (context->status & DEC_Invalid_operation) {
        printf("Test decNumberToInt32 with INT32_MAX + 1 - Got the expected Invalid_operation\n");
    }
    else {
        errors++;
        printf("Test decNumberToInt32 with INT32_MAX + 1 - ERROR, expecting: Invalid_operation - Got: %d\n", result);
    }
    // Now test after normalisation
    decNumberNormalize(dn, dn, context);
    context->status = 0;
    result = decNumberToInt32(dn, context);
    // Check Context flags
    if (context->status & DEC_Invalid_operation) {
        printf("Test decNumberToInt32 with INT32_MAX + 1 (after normalisation) - Got the expected Invalid_operation\n");
    }
    else {
        errors++;
        printf("Test decNumberToInt32 with INT32_MAX + 1 (after normalisation) - ERROR, expecting: Invalid_operation - Got: %d\n", result);
    }
    // Free the number and context
    free(dn);
    free(context);

    return errors;
}

static int do_decNumberToUInt32_test(char* test, u_int32_t value) {
    int digits = 30;
    char expected[digits + 14];
    int error = 0;

    sprintf(expected, "%u", value);

    // Allocate memory for the context
    decContext* context = malloc(sizeof(decContext));
    decContextDefault(context, DEC_INIT_BASE); // initialize
    context->traps = 0; // no traps
    context->digits = digits; // set precision

    /* Get a malloced DecNumber */
    decNumber *dn = getNumber(digits);

    printf("%s",test);
    decNumberFromUInt32(dn, value);
    u_int32_t result = decNumberToUInt32(dn, context);
    if (result != value) {
        error = 1;
        printf("- ERROR - Expected: %u - Got: %u", value, result);
    }
    printf("\n");

    printf("%s (after normalisation)",test);
    decNumberFromUInt32(dn, value);
    decNumberNormalize(dn, dn, context);
    result = decNumberToUInt32(dn, context);
    if (result != value) {
        error = 1;
        printf("- ERROR - Expected: %u - Got: %u", value, result);
    }
    printf("\n");

    free(dn);
    free(context);

    return error;
}

// Test uint32_t decNumberToUInt32(const decNumber *dn, decContext *set)
int test_decNumberToUInt32() {
    int errors = 0;

    // Tests a positive number
    errors += do_decNumberToUInt32_test("Test decNumberToUInt32 with a positive number", 1000);

    // Test with a large number > 32 bits
    errors += do_decNumberToUInt32_test("Test decNumberToUInt32 with a large unsigned number", 0xFFFFFFFF);

    // Test with 0
    errors += do_decNumberToUInt32_test("Test decNumberToUInt32 with 0", 0);

    // Test with UINT32_MAX (4294967295)
    errors += do_decNumberToUInt32_test("Test decNumberToUInt32 with UINT32_MAX", UINT32_MAX);

    // Test with 1
    errors += do_decNumberToUInt32_test("Test decNumberToUInt32 with 1", 1);

    // Boundary Conditions Around Limits - Near UINT32_MAX - one less than UINT32_MAX - 4294967294
    errors += do_decNumberToUInt32_test("Test decNumberToUInt32 Boundary Conditions Around Limits - Near UINT32_MAX Test 4294967294 (one less than UINT32_MAX)", 4294967294);

    // Boundary Conditions Around Limits - Near UINT32_MAX Test (close to UINT32_MAX but with trailing zeros) - 4294967290
    errors += do_decNumberToUInt32_test("Test decNumberToUInt32 Boundary Conditions Around Limits - Near UINT32_MAX Test 4294967290 (close to UINT32_MAX but with trailing zeros)", 4294967290);

    // Mid-Range Large Values: Unsigned Positive: 1234567890
    errors += do_decNumberToUInt32_test("Test decNumberToUInt32 Mid-Range Large Values: Positive: 1234567890", 1234567890);

    // Values that are exactly powers of 10: 1000000000
    errors += do_decNumberToUInt32_test("Test decNumberToUInt32 Values that are exactly powers of 10: 1000000000", 1000000000);

    // Values that are exactly powers of 10: 10000000
    errors += do_decNumberToUInt32_test("Test decNumberToUInt32 Values that are exactly powers of 10: 10000000", 10000000);

    // Values that are exactly powers of 10: 10
    errors += do_decNumberToUInt32_test("Test decNumberToUInt32 Values that are exactly powers of 10: 10", 10);

    // All 9's: 9999999
    errors += do_decNumberToUInt32_test("Test decNumberToUInt32 All 9's: 9999999", 9999999);

    // Now test a decimal number just bigger than UINT32_MAX
    int digits = 30;

    // Allocate memory for the context
    decContext* context = malloc(sizeof(decContext));
    decContextDefault(context, DEC_INIT_BASE); // initialize
    context->traps = 0; // no traps
    context->digits = digits; // set precision

    /* Get a malloced DecNumber */
    decNumber *dn = getNumber(digits);

    // Create the number from a string - UINT32_MAX + 1 (4294967295 + 1)
    decNumberFromString(dn, "4294967296", context);
    // Clear the context flags
    context->status = 0;
    u_int32_t result = decNumberToUInt32(dn, context);
    // Check Context flags
    if (context->status & DEC_Invalid_operation) {
        printf("Test decNumberToUInt32 with UINT32_MAX + 1 - Got the expected Invalid_operation\n");
    }
    else {
        errors++;
        printf("Test decNumberToUInt32 with UINT32_MAX + 1 - ERROR, expecting: Invalid_operation - Got: %u\n", result);
    }
    // Now test after normalisation
    decNumberNormalize(dn, dn, context);
    context->status = 0;
    result = decNumberToUInt32(dn, context);
    // Check Context flags
    if (context->status & DEC_Invalid_operation) {
        printf("Test decNumberToUInt32 with UINT32_MAX + 1 (after normalisation) - Got the expected Invalid_operation\n");
    }
    else {
        errors++;
        printf("Test decNumberToUInt32 with UINT32_MAX + 1 (after normalisation) - ERROR, expecting: Invalid_operation - Got: %u\n", result);
    }
    // Free the number and context
    free(dn);
    free(context);

    return errors;
}

// Main test function
int main() {
    int errors = 0;

    errors += test_decNumberFromInt64();
    errors += test_decNumberFromUInt64();
    errors += test_decNumberToInt64();
    errors += test_decNumberToUInt64();
    errors += test_decNumberToInt32();
    errors += test_decNumberToUInt32();

    return errors;
}
