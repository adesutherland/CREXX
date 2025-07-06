//
// Created by Adrian Sutherland on 16/09/2024.
//
// Decimal Test 2
// Test functions we have added to decnumber or which
// are non-trivial functions in the mc_decimal plugin

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "rxvmplugin_framework.h"

decplugin *plugin;

// Do test
int aTestFromToInt(char* expected, int64_t int_input) {
    int64_t int_output;
    char* output;
    value a;
    int errors = 0;

    a.decimal_value = NULL;

    /* Make a string buffer to hold the result as a string */
    output = malloc(plugin->getRequiredStringSize(plugin));

    printf("\nTesting with %s\n", expected);
    plugin->decimalFromInt(plugin, &a, int_input);
    plugin->decimalToString(plugin, &a, output);
    if (strcmp(output,expected) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("expected %s got %s for from int\n", expected, output);
    plugin->decimalToInt(plugin, &a, &int_output);
    if (int_output != int_input) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("expected %lld got %lld for to int\n", int_input, int_output);

    free(output);

    return errors;
}

// Do test
int aTestBeyondLimits(char* input) {
    int64_t int_output;
    value a;
    int errors = 0;

    a.decimal_value = NULL;

    printf("\nTesting beyond limits with %s\n", input);
    plugin->decimalFromString(plugin, &a, input);
    plugin->decimalToInt(plugin, &a, &int_output);
    if (!plugin->base.signal_number) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("expected signal and got - %d \"%s\"\n", plugin->base.signal_number, plugin->base.signal_string);

    return errors;
}

static int test_int_tofrom_conversions() {
    char* input;
    int64_t int_input;
    int errors = 0;

    printf("\nTesting integer conversions\n");

    // Test with zero
    errors += aTestFromToInt("0", 0);

    // Test with INT64_MAX (9223372036854775807)
    errors += aTestFromToInt("9223372036854775807", INT64_MAX);

    // Test with INT64_MIN (-9223372036854775808)
    errors += aTestFromToInt("-9223372036854775808", INT64_MIN);

    // Test with 1
    errors += aTestFromToInt("1", 1);

    // Test with -1
    errors += aTestFromToInt("-1", -1);

    // Test with a positive number between 32 and 64 bits
    errors += aTestFromToInt("4294967295", 4294967295ll);

    // Test with a negative number between 32 and 64 bits
    errors += aTestFromToInt("-4294967295", -4294967295ll);

    // Test with a positive number less than 32 bits
    errors += aTestFromToInt("255", 255);

    // Test with a negative number less than 32 bits
    errors += aTestFromToInt("-255", -255);

    /* Boundary Conditions Around Limits - Near INT64_MAX Test 9223372036854775806 (one less than INT64_MAX). */
    errors += aTestFromToInt("9223372036854775806", 9223372036854775806);

    /* Boundary Conditions Around Limits - Near INT64_MAX Test 9223372036854775800 (close to INT64_MAX but with trailing zeros).*/
    errors += aTestFromToInt("9223372036854775800", 9223372036854775800);

    /* Boundary Conditions Around Limits - Near INT64_MIN: Test -9223372036854775807 (one more than INT64_MIN). */
    errors += aTestFromToInt("-9223372036854775807", -9223372036854775807);

    /* Boundary Conditions Around Limits - Near INT64_MIN: Test -9223372036854775800 (close to INT64_MIN but with trailing zeros). */
    errors += aTestFromToInt("-9223372036854775800", -9223372036854775800);

    /* Values Just Beyond Limits: 9223372036854775808 - should trigger Invalid_operation. */
    errors += aTestBeyondLimits("9223372036854775808");

    /* Values Just Beyond Limits: -9223372036854775809 - should trigger Invalid_operation. */
    errors += aTestBeyondLimits("-9223372036854775809");

    /* Mid-Range Large Values: Positive: 1234567890123456789 */
    errors += aTestFromToInt("1234567890123456789", 1234567890123456789);

    /* Mid-Range Large Values: Negative: -1234567890123456789 */
    errors += aTestFromToInt("-1234567890123456789", -1234567890123456789);

    /* Values that are exactly powers of 10: 1000000000 */
    errors += aTestFromToInt("1000000000", 1000000000);

    /* Values that are exactly powers of 10: 1000000000000000000 */
    errors += aTestFromToInt("1000000000000000000", 1000000000000000000);

    /* Values that are exactly powers of 10: -1000000000 */
    errors += aTestFromToInt("-1000000000", -1000000000);

    /* Values that are exactly powers of 10: -1000000000000000000 */
    errors += aTestFromToInt("-1000000000000000000", -1000000000000000000);

    /* Values that are exactly powers of 10: 10 */
    errors += aTestFromToInt("10", 10);

    /* Values that are exactly powers of 10: -10 */
    errors += aTestFromToInt("-10", -10);

    /* All 9's: 999999999999999999 */
    errors += aTestFromToInt("999999999999999999", 999999999999999999);

    /* All 9's: -999999999999999999 */
    errors += aTestFromToInt("-999999999999999999", -999999999999999999);

    /* Loop and Generate a few random 19-digit numbers and test both positive and negative forms */
    input = malloc(40);
    int i;
    for (i = 0; i < 10; i++) {
        int_input = rand(); // NOLINT(cert-msc50-cpp)
        int_input = int_input * rand(); // NOLINT(cert-msc50-cpp)
        int_input = int_input * rand(); // NOLINT(cert-msc50-cpp)
        int_input = int_input * rand(); // NOLINT(cert-msc50-cpp)
        int_input = int_input * rand(); // NOLINT(cert-msc50-cpp)
        int_input = int_input * rand(); // NOLINT(cert-msc50-cpp)
        int_input = int_input * rand(); // NOLINT(cert-msc50-cpp)
        int_input = int_input * rand(); // NOLINT(cert-msc50-cpp)
        int_input = int_input * rand(); // NOLINT(cert-msc50-cpp)
        int_input = int_input * rand(); // NOLINT(cert-msc50-cpp)
        int_input = int_input * rand(); // NOLINT(cert-msc50-cpp)
        int_input = int_input * rand(); // NOLINT(cert-msc50-cpp)
        int_input = int_input * rand(); // NOLINT(cert-msc50-cpp)

        sprintf(input, "%lld", int_input);
        errors += aTestFromToInt(input, int_input);
        sprintf(input, "%lld", -int_input);
        errors += aTestFromToInt(input, -int_input);
    }
    free(input);

    return errors;
} // test_int_conversion

// Tests for decimalFromDouble
int test_decimalFromDouble() {
    value result;
    int errors = 0;
    double inputs[] = { NAN, INFINITY, -INFINITY, 0.0, -0.0, 1.23456789, -1.23456789 };
    const char *descriptions[] = { "nan", "inf", "-inf", "0", "-0", "1.23456789", "-1.23456789" };
    int i;
    result.decimal_value = NULL;

    printf("\nTesting decimalFromDouble()\n");

    for (i = 0; i < 7; i++) {
        plugin->decimalFromDouble(plugin, &result, inputs[i]);
        char buffer[32];
        plugin->decimalToString(plugin, &result, buffer);
        if (strcmp(descriptions[i], buffer) != 0) {
            printf("Error - ");
            errors++;
        }
        else printf("OK - ");
        printf("Input: %s, decNumber: %s\n", descriptions[i], buffer);
    }
    return errors;
}

// Tests for decimalToDouble
int test_decimalToDouble() {
    value input;
    double result;
    char buffer[32];
    int errors = 0;
    input.decimal_value = NULL;

    printf("\nTesting decimalToDouble()\n");

    // Test NaN
    plugin->decimalFromDouble(plugin, &input, NAN);
    plugin->decimalToDouble(plugin, &input, &result);
    sprintf(buffer, "%f", result);
    if (strcmp("nan", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: nan, Result: %s\n", buffer);

    // Test Infinity
    plugin->decimalFromDouble(plugin, &input, INFINITY);
    plugin->decimalToDouble(plugin, &input, &result);
    sprintf(buffer, "%f", result);
    if (strcmp("inf", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: inf, Result: %s\n", buffer);

    // Test -Infinity
    plugin->decimalFromDouble(plugin, &input, -INFINITY);
    plugin->decimalToDouble(plugin, &input, &result);
    sprintf(buffer, "%f", result);
    if (strcmp("-inf", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: -inf, Result: %s\n", buffer);

    // Test 0.0
    plugin->decimalFromDouble(plugin, &input, 0.0);
    plugin->decimalToDouble(plugin, &input, &result);
    sprintf(buffer, "%f", result);
    if (strcmp("0.000000", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 0.0, Result: %s\n", buffer);

    // Test -0.0
    plugin->decimalFromDouble(plugin, &input, -0.0);
    plugin->decimalToDouble(plugin, &input, &result);
    sprintf(buffer, "%f", result);
    if (strcmp("-0.000000", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: -0.0, Result: %s\n", buffer);

    // Test 1.23456789
    plugin->decimalFromDouble(plugin, &input, 1.23456789);
    plugin->decimalToDouble(plugin, &input, &result);
    sprintf(buffer, "%f", result);
    if (strcmp("1.234568", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 1.23456789, Result: %s\n", buffer);

    return errors;
}

// Tests for decimalToString and decimalFromString
int test_decimalToString_decimalFromString() {
    value input;
    char buffer[32];
    int errors = 0;
    input.decimal_value = NULL;

    printf("\nTesting decimalToString() and decimalFromString()\n");

    // Test NaN
    plugin->decimalFromString(plugin, &input, "nan");
    plugin->decimalToString(plugin, &input, buffer);
    if (strcmp("nan", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: nan, Result: %s\n", buffer);

    // Test Infinity
    plugin->decimalFromString(plugin, &input, "inf");
    plugin->decimalToString(plugin, &input, buffer);
    if (strcmp("inf", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: inf, Result: %s\n", buffer);

    // Test -Infinity
    plugin->decimalFromString(plugin, &input, "-inf");
    plugin->decimalToString(plugin, &input, buffer);
    if (strcmp("-inf", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: -inf, Result: %s\n", buffer);

    // Test 0.0
    plugin->decimalFromString(plugin, &input, "0");
    plugin->decimalToString(plugin, &input, buffer);
    if (strcmp("0", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 0.0, Result: %s\n", buffer);

    // Test -0.0
    plugin->decimalFromString(plugin, &input, "-0");
    plugin->decimalToString(plugin, &input, buffer);
    if (strcmp("-0", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: -0.0, Result: %s\n", buffer);

    // Test Simple number 123456789
    plugin->decimalFromString(plugin, &input, "123456789");
    plugin->decimalToString(plugin, &input, buffer);
    if (strcmp("123456789", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 123456789, Result: %s\n", buffer);

    // Test 1.23456789
    plugin->decimalFromString(plugin, &input, "1.23456789");
    plugin->decimalToString(plugin, &input, buffer);
    if (strcmp("1.23456789", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 1.23456789, Result: %s\n", buffer);

    // Test 123e3 -> 123000
    plugin->decimalFromString(plugin, &input, "123e3");
    plugin->decimalToString(plugin, &input, buffer);
    if (strcmp("123000", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 123e3 -> 123000, Result: %s\n", buffer);

    // Test 123e-3 -> 0.123
    plugin->decimalFromString(plugin, &input, "123e-3");
    plugin->decimalToString(plugin, &input, buffer);
    if (strcmp("0.123", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 123e-3 -> 0.123, Result: %s\n", buffer);

    // Test 123456789000000000000000000000 -> 1.23456789E+29
    plugin->decimalFromString(plugin, &input, "123456789000000000000000000000");
    plugin->decimalToString(plugin, &input, buffer);
    if (strcmp("1.23456789E+29", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 123456789000000000000000000000 -> 1.23456789E+29, Result: %s\n", buffer);

    return errors;
}

// Test decimalToInteger (considering signals)
int test_moreDecimalToInteger() {
    value input;
    int64_t result;
    int errors = 0;
    input.decimal_value = NULL;

    printf("\nTesting decimalToInteger()\n");

    // Test NaN
    plugin->decimalFromString(plugin, &input, "nan");
    plugin->decimalToInt(plugin, &input, &result);
    if (!plugin->base.signal_number) {
        printf("Error - no signal - ");
        errors++;
    }
    else printf("OK - signal - ");
    printf("decNumber: nan, Result: %lld\n", result);

    // Test Infinity
    plugin->decimalFromString(plugin, &input, "inf");
    plugin->decimalToInt(plugin, &input, &result);
    if (!plugin->base.signal_number) {
        printf("Error - no signal - ");
        errors++;
    }
    else printf("OK - signal - ");
    printf("decNumber: inf, Result: %lld\n", result);

    // Test -Infinity
    plugin->decimalFromString(plugin, &input, "-inf");
    plugin->decimalToInt(plugin, &input, &result);
    if (!plugin->base.signal_number) {
        printf("Error - no signal - ");
        errors++;
    }
    else printf("OK - signal - ");
    printf("decNumber: -inf, Result: %lld\n", result);

    // Test 0.0
    plugin->decimalFromString(plugin, &input, "0");
    plugin->decimalToInt(plugin, &input, &result);
    if (result != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 0.0, Result: %lld\n", result);

    // Test -0.0
    plugin->decimalFromString(plugin, &input, "-0");
    plugin->decimalToInt(plugin, &input, &result);
    if (result != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: -0.0, Result: %lld\n", result);

    // Test 1.23456789
    plugin->decimalFromString(plugin, &input, "1.23456789");
    plugin->decimalToInt(plugin, &input, &result);
    if (!plugin->base.signal_number) { // (result != 1) {
        printf("Error - no signal: 1.23456789, Result: %lld\n", result);
        errors++;
    }
    else printf("OK - signal: decNumber: 1.23456789, Result: %lld\n", result);

    // Test 1234567890123456789
    plugin->decimalFromString(plugin, &input, "1234567890123456789");
    plugin->decimalToInt(plugin, &input, &result);
    if (result != 1234567890123456789) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 1234567890123456789, Result: %lld\n", result);

    return errors;
}

/* Test basic decimal functions (add, subtract, multiply, divide) - these are just basic tests as they have low risk */
int test_basic_decimal_functions() {
    value a, b, result;
    int errors = 0;
    char buffer[32];
    a.decimal_value = NULL;
    b.decimal_value = NULL;
    result.decimal_value = NULL;

    printf("\nTesting basic decimal functions\n");

    // Test 1 + 1 = 2
    plugin->decimalFromString(plugin, &a, "1");
    plugin->decimalFromString(plugin, &b, "1");
    plugin->decimalAdd(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("2", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1 + 1 = %s\n", buffer);

    // Test 1 - 1 = 0
    plugin->decimalFromString(plugin, &a, "1");
    plugin->decimalFromString(plugin, &b, "1");
    plugin->decimalSub(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("0", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1 - 1 = %s\n", buffer);

    // Test 2 * 2 = 4
    plugin->decimalFromString(plugin, &a, "2");
    plugin->decimalFromString(plugin, &b, "2");
    plugin->decimalMul(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("4", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("2 * 2 = %s\n", buffer);

    // Test 4 / 2 = 2
    plugin->decimalFromString(plugin, &a, "4");
    plugin->decimalFromString(plugin, &b, "2");
    plugin->decimalDiv(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("2", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("4 / 2 = %s\n", buffer);

    return errors;
}

/* Test Comparison Function */
int test_decimalCompare() {
    value a, b;
    int result;
    int errors = 0;
    a.decimal_value = NULL;
    b.decimal_value = NULL;

    printf("\nTesting decimalCompare()\n");

    // Test 1 == 1
    plugin->decimalFromString(plugin, &a, "1");
    plugin->decimalFromString(plugin, &b, "1");
    result = plugin->decimalCompare(plugin, &a, &b);
    if (result != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1 == 1\n");

    // Test 1 < 2
    plugin->decimalFromString(plugin, &a, "1");
    plugin->decimalFromString(plugin, &b, "2");
    result = plugin->decimalCompare(plugin, &a, &b);
    if (result != -1) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1 < 2\n");

    // Test 2 > 1
    plugin->decimalFromString(plugin, &a, "2");
    plugin->decimalFromString(plugin, &b, "1");
    result = plugin->decimalCompare(plugin, &a, &b);
    if (result != 1) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("2 > 1\n");

    // Test with a NaN
    plugin->decimalFromString(plugin, &a, "nan");
    plugin->decimalFromString(plugin, &b, "1");
    // Clear the signal
    plugin->base.signal_number = 0;
    plugin->decimalCompare(plugin, &a, &b);
    if (!plugin->base.signal_number) {
        printf("Error - NaN -> No signal\n");
        errors++;
    }
    else printf("OK - NaN -> Signal\n");

    // Test with an Infinity
    plugin->decimalFromString(plugin, &a, "inf");
    plugin->decimalFromString(plugin, &b, "1");
    // Clear the signal
    plugin->base.signal_number = 0;
    result = plugin->decimalCompare(plugin, &a, &b);
    if (plugin->base.signal_number) {
        printf("Error - inf > 1 -> Unexpected signal\n");
        errors++;
    }
    if (result != 1) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("inf > 1\n");

    return errors;
}

/* Test decimalNeg */
int test_decimalNeg() {
    value a, result;
    int errors = 0;
    char buffer[32];
    a.decimal_value = NULL;
    result.decimal_value = NULL;

    printf("\nTesting decimalNeg()\n");

    // Test -1
    plugin->decimalFromString(plugin, &a, "1");
    plugin->decimalNeg(plugin, &result, &a);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("-1", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1 -> %s\n", buffer);

    // Test 1
    plugin->decimalFromString(plugin, &a, "-1");
    plugin->decimalNeg(plugin, &result, &a);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("1", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("-1 -> %s\n", buffer);

    // Test 0
    plugin->decimalFromString(plugin, &a, "0");
    plugin->decimalNeg(plugin, &result, &a);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("0", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("0 -> %s\n", buffer);

    // Test with the result being the same as the input
    plugin->decimalFromString(plugin, &a, "1");
    plugin->decimalNeg(plugin, &a, &a);
    plugin->decimalToString(plugin, &a, buffer);
    if (strcmp("-1", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1 -> %s (inplace)\n", buffer);

    return errors;
}

/* Test decimalPow */
int test_decimalPow() {
    value a, b, result;
    int errors = 0;
    char buffer[32];
    a.decimal_value = NULL;
    b.decimal_value = NULL;
    result.decimal_value = NULL;

    printf("\nTesting decimalPow()\n");

    // Test 2^3 = 8
    plugin->decimalFromString(plugin, &a, "2");
    plugin->decimalFromString(plugin, &b, "3");
    plugin->decimalPow(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("8", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("2^3 = %s\n", buffer);

    // Test 2^0 = 1
    plugin->decimalFromString(plugin, &a, "2");
    plugin->decimalFromString(plugin, &b, "0");
    plugin->decimalPow(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("1", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("2^0 = %s\n", buffer);

    // Test 2^1 = 2
    plugin->decimalFromString(plugin, &a, "2");
    plugin->decimalFromString(plugin, &b, "1");
    plugin->decimalPow(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("2", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("2^1 = %s\n", buffer);

    // Test 2^-1 = 0.5
    plugin->decimalFromString(plugin, &a, "2");
    plugin->decimalFromString(plugin, &b, "-1");
    plugin->decimalPow(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("0.5", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("2^-1 = %s\n", buffer);

    return errors;
}

/* Test Comparison String Function */
int test_decimalCompareString() {
    value a;
    int result;
    int errors = 0;
    a.decimal_value = NULL;

    printf("\nTesting decimalCompareString()\n");

    // Test 1 == 1
    plugin->decimalFromString(plugin, &a, "1");
    result = plugin->decimalCompareString(plugin, &a, "1");
    if (result != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1 == 1\n");

    // Test 1 < 2
    plugin->decimalFromString(plugin, &a, "1");
    result = plugin->decimalCompareString(plugin, &a, "2");
    if (result != -1) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1 < 2\n");

    // Test 2 > 1
    plugin->decimalFromString(plugin, &a, "2");
    result = plugin->decimalCompareString(plugin, &a, "1");
    if (result != 1) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("2 > 1\n");

    // Test with a NaN
    plugin->decimalFromString(plugin, &a, "nan");
    // Clear the signal
    plugin->base.signal_number = 0;
    plugin->decimalCompareString(plugin, &a, "1");
    if (!plugin->base.signal_number) {
        printf("Error - NaN -> No signal\n");
        errors++;
    }
    else printf("OK - NaN -> Signal\n");

    // Test with an Infinity
    plugin->decimalFromString(plugin, &a, "inf");
    // Clear the signal
    plugin->base.signal_number = 0;
    result = plugin->decimalCompareString(plugin, &a, "1");
    if (plugin->base.signal_number) {
        printf("Error - inf > 1 -> Unexpected signal\n");
        errors++;
    }
    if (result != 1) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("inf > 1\n");

    return errors;
}

/* Test decimalExtract
 * This function extracts the coefficient and exponent from a decimal number.
 * It is bespoke to the mc_decimal plugin and therefore is high risk and needs
 * comprehensive testing including edge cases
 */

/* Utility function for a single test of decimalExtract */
int testDecimalExtract(char* input, char* expected_coefficient, int64_t expected_exponent) {
    value a;
    int64_t exponent;
    char *coefficient = malloc(plugin->getDigits(plugin) + 14);
    a.decimal_value = NULL;
    int errors = 0;

    plugin->decimalFromString(plugin, &a, input);
    plugin->decimalExtract(plugin, coefficient, &exponent, &a);
    if (strcmp(expected_coefficient, coefficient) != 0 || exponent != expected_exponent) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    if (exponent) printf("%s -> %se%lld\n", input, coefficient, exponent);
    else printf("%s -> %s\n", input, coefficient);

    free(a.decimal_value);
    free(coefficient);
    return errors;
}

/* Test decimalExtract - test cases */
int test_decimalExtract() {

    int errors = 0;

    printf("\nTesting decimalExtract()\n");

    // Test zero, nan, inf, -inf
    errors += testDecimalExtract("0", "0", 0);
    errors += testDecimalExtract("nan", "nan", 0);
    errors += testDecimalExtract("inf", "inf", 0);
    errors += testDecimalExtract("-inf", "-inf", 0);
    // Test Integer and negative integer
    errors += testDecimalExtract("1234567890123456789", "1.234567890123456789", 18);
    errors += testDecimalExtract("-1234567890123456789", "-1.234567890123456789", 18);
    // Test Normalized floating point numbers (positive and negative)
    errors += testDecimalExtract("1.23456789", "1.23456789", 0);
    errors += testDecimalExtract("-1.23456789", "-1.23456789", 0);
    // Test Normalized floating point numbers with exponent (positive and negative)
    errors += testDecimalExtract("1.23456789e3", "1.23456789", 3);
    errors += testDecimalExtract("-1.23456789e3", "-1.23456789", 3);
    // Test Normalized floating point numbers with negative exponent (positive and negative)
    errors += testDecimalExtract("1.23456789e-3", "1.23456789", -3);
    errors += testDecimalExtract("-1.23456789e-3", "-1.23456789", -3);
    // Test removal of leading zeros
    errors += testDecimalExtract("0000123456789", "1.23456789", 8);
    // Test removal of trailing zeros
    errors += testDecimalExtract("123456789000000000000000000000", "1.23456789", 29);
    // Test Large numbers
    plugin->setDigits(plugin, 50);
    errors += testDecimalExtract("123456789012345678901234567890", "1.2345678901234567890123456789", 29);
    errors += testDecimalExtract("-123456789012345678901234567890", "-1.2345678901234567890123456789", 29);
    // Test Large numbers with exponent
    errors += testDecimalExtract("123456789012345678901234567890e3", "1.2345678901234567890123456789", 32);
    errors += testDecimalExtract("-123456789012345678901234567890e3", "-1.2345678901234567890123456789", 32);
    // Test Large numbers with negative exponent
    errors += testDecimalExtract("123456789012345678901234567890e-3", "1.2345678901234567890123456789", 26);
    errors += testDecimalExtract("-123456789012345678901234567890e-3", "-1.2345678901234567890123456789", 26);
    // Test Large numbers with negative exponent and leading zeros
    errors += testDecimalExtract("1234567890123456789012345678901e-30", "1.234567890123456789012345678901", 0);
    errors += testDecimalExtract("-1234567890123456789012345678901e-30", "-1.234567890123456789012345678901", 0);
    // Test Large numbers with negative exponent and trailing zeros
    errors += testDecimalExtract("123456789012345678901234567890e-3", "1.2345678901234567890123456789", 26);
    errors += testDecimalExtract("-123456789012345678901234567890e-3", "-1.2345678901234567890123456789", 26);
    // Test Large numbers with negative exponent and leading and trailing zeros
    errors += testDecimalExtract("123456789012345678901234567890e-30", "1.2345678901234567890123456789", -1);
    errors += testDecimalExtract("-123456789012345678901234567890e-30", "-1.2345678901234567890123456789", -1);
    /* Test Large numbers with smaller digits setting */
    plugin->setDigits(plugin, 20);
    errors += testDecimalExtract("123456789012345678901234567890", "1.234567890123456789", 29);
    errors += testDecimalExtract("-123456789012345678901234567890", "-1.234567890123456789", 29);
    // Test Large numbers with exponent
    errors += testDecimalExtract("123456789012345678901234567890e3", "1.234567890123456789", 32);
    errors += testDecimalExtract("-123456789012345678901234567890e3", "-1.234567890123456789", 32);
    // Test Large numbers with negative exponent
    errors += testDecimalExtract("123456789012345678901234567890e-3", "1.234567890123456789", 26);
    errors += testDecimalExtract("-123456789012345678901234567890e-3", "-1.234567890123456789", 26);
    // Test Large numbers with negative exponent and leading zeros
    errors += testDecimalExtract("123456789012345678901234567890e-30", "1.234567890123456789", -1);
    errors += testDecimalExtract("-123456789012345678901234567890e-30", "-1.234567890123456789", -1);
    // Test Large numbers with negative exponent and trailing zeros
    errors += testDecimalExtract("123456789012345678901234567890e-3", "1.234567890123456789", 26);
    errors += testDecimalExtract("-123456789012345678901234567890e-3", "-1.234567890123456789", 26);
    // Test Large numbers with negative exponent and leading and trailing zeros
    errors += testDecimalExtract("123456789012345678901234567890e-30", "1.234567890123456789", -1);
    errors += testDecimalExtract("-123456789012345678901234567890e-30", "-1.234567890123456789", -1);
    // Test factors of 10
    errors += testDecimalExtract("1000000000", "1", 9);
    errors += testDecimalExtract("1000000000000000000", "1", 18);
    errors += testDecimalExtract("-1000000000", "-1", 9);
    errors += testDecimalExtract("-1000000000000000000", "-1", 18);
    errors += testDecimalExtract("10", "1", 1);
    errors += testDecimalExtract("-10", "-1", 1);
    errors += testDecimalExtract("0.00001", "1", -5);
    // Test all 9s
    errors += testDecimalExtract("999999999999999999", "9.99999999999999999", 17);
    errors += testDecimalExtract("-999999999999999999", "-9.99999999999999999", 17);
    // Finally, Test Floating point numbers (positive and negative) with over 100 digits
    plugin->setDigits(plugin, 200);
    errors += testDecimalExtract("1234567890123456789012345678900123456789001234567890012345678900123456789001234567890012345678900123456789001234567890012345678900123456789001234567890e-300",
        "1.23456789012345678901234567890012345678900123456789001234567890012345678900123456789001234567890012345678900123456789001234567890012345678900123456789",
        -150);

    return errors;
}

// Test is zero
int test_isZero() {
    value input;
    int result;
    int errors = 0;
    input.decimal_value = NULL;

    printf("\nTesting isZero()\n");

    // Test 0
    plugin->decimalFromString(plugin, &input, "0");
    result = plugin->decimalIsZero(plugin, &input);
    if (result != 1) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 0 -> isZero: %d\n", result);

    // Test -0
    plugin->decimalFromString(plugin, &input, "-0");
    result = plugin->decimalIsZero(plugin, &input);
    if (result != 1) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: -0 -> isZero: %d\n", result);

    // Test 1
    plugin->decimalFromString(plugin, &input, "1");
    result = plugin->decimalIsZero(plugin, &input);
    if (result != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 1 -> isZero: %d\n", result);

    // Test -1
    plugin->decimalFromString(plugin, &input, "-1");
    result = plugin->decimalIsZero(plugin, &input);
    if (result != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: -1 -> isZero: %d\n", result);

    // Test with an exponent
    plugin->decimalFromString(plugin, &input, "1e-10");
    result = plugin->decimalIsZero(plugin, &input);
    if (result != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 1e-10 -> isZero: %d\n", result);

    return errors;
}

// Test decimalTruncate - which truncates (round down - towards zero) to an integer
int test_decimalTruncate() {
    value input, result;
    int errors = 0;
    char buffer[32];
    input.decimal_value = NULL;
    result.decimal_value = NULL;

    printf("\nTesting decimalTruncate()\n");

    // Test 1.23456789 -> 1
    plugin->decimalFromString(plugin, &input, "1.23456789");
    plugin->decimalTruncate(plugin, &result, &input);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("1", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1.23456789 -> %s\n", buffer);

    // Test -1.23456789 -> -1
    plugin->decimalFromString(plugin, &input, "-1.23456789");
    plugin->decimalTruncate(plugin, &result, &input);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("-1", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("-1.23456789 -> %s\n", buffer);

    // Test 1234567890123456789.987654321 -> 1234567890123456789
    plugin->decimalFromString(plugin, &input, "1234567890123456789.987654321");
    plugin->decimalTruncate(plugin, &result, &input);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("1234567890123456789", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1234567890123456789.987654321 -> %s\n", buffer);

    // Test with an exponent
    plugin->decimalFromString(plugin, &input, "1234567890123456789.987654321e2");
    plugin->decimalTruncate(plugin, &result, &input);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("123456789012345678998", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1234567890123456789.987654321e2 -> %s\n", buffer);

    return errors;
}

// Test decimalRound - which rounds (round half up even) to an integer
int test_decimalRound() {
    value input, result;
    int errors = 0;
    char buffer[32];
    input.decimal_value = NULL;
    result.decimal_value = NULL;

    printf("\nTesting decimalRound()\n");

    // Test 1.23456789 -> 1
    plugin->decimalFromString(plugin, &input, "1.23456789");
    plugin->decimalRound(plugin, &result, &input);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("1", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1.23456789 -> %s\n", buffer);

    // Test -1.23456789 -> -1
    plugin->decimalFromString(plugin, &input, "-1.23456789");
    plugin->decimalRound(plugin, &result, &input);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("-1", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("-1.23456789 -> %s\n", buffer);

    // Test 1234567890123456789.987654321 -> 1234567890123456790
    plugin->decimalFromString(plugin, &input, "1234567890123456789.987654321");
    plugin->decimalRound(plugin, &result, &input);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("1234567890123456790", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1234567890123456789.987654321 -> %s\n", buffer);

    // Test with an exponent
    plugin->decimalFromString(plugin, &input, "1234567890123456789.987654321e2");
    plugin->decimalRound(plugin, &result, &input);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("123456789012345678999", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1234567890123456789.987654321e2 -> %s\n", buffer);

    return errors;
}

// Main function
int main(int argc, char *argv[]) {
    int errors = 0;

    // Load the plugin
    printf("mc_decimal_test2 - Loading Dynamic Plugin\n");
    if (load_rxvmplugin(".", "rxvm_mc_decimal") != 0) {
        printf("Unable to load the rxvmplugin plugin\n");
        return 1;
    }

    plugin = (decplugin*)get_rxvmplugin(RXVM_PLUGIN_DECIMAL);
    if (!plugin) {
        printf("No default rxvmplugin plugin\n");
        return 1;
    }

    // Set the number of digits in the rxvmplugin context
    plugin->setDigits(plugin, 20); // Big enough for 64-bit int

    errors += test_int_tofrom_conversions();
    errors += test_moreDecimalToInteger();
    errors += test_decimalFromDouble();
    errors += test_decimalToDouble();
    errors += test_decimalToString_decimalFromString();
    errors += test_basic_decimal_functions();
    errors += test_decimalCompare();
    errors += test_decimalCompareString();
    errors += test_decimalNeg();
    errors += test_decimalPow();
    errors += test_decimalExtract();
    errors += test_isZero();
    errors += test_decimalTruncate();
    errors += test_decimalRound();

    plugin->base.free((rxvm_plugin*)plugin);

    return errors;
} // main
