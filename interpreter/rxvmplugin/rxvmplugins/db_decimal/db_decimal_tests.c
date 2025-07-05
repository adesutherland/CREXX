//
// Created by Adrian Sutherland on 16/09/2024.
//
// Decimal Test 2
// Test functions we have added to decnumber or which
// are non-trivial functions in the mc_decimal plugin

#include <float.h>
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

    printf("\nTesting with %ld\n", int_input);
    plugin->decimalFromInt(plugin, &a, int_input);

    plugin->decimalToString(plugin, &a, output);
    if (strcmp(output,expected) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("expected %s got %s for from int\n", expected, output);
    plugin->decimalToInt(plugin, &a, &int_output);
    if (int_input > 9223372036854775804 || int_input < -9223372036854775804) {
        // Bigger than the max int value that can be convered exactly (without rounding)
        if (!plugin->base.signal_number) {
            printf("Error - expected signal got %lld for int\n", int_output);
            errors++;
        }
        else printf("OK - expected signal for int\n");
    }
    else {
        if (int_output != int_input) {
            printf("Error - ");
            errors++;
        }
        else printf("OK - ");
        printf("expected %lld got %lld for to int\n", int_input, int_output);
    }
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

    // Note digits = 16 hence the rounding

    // Test with INT64_MAX (9223372036854775807)
    errors += aTestFromToInt("9.22337203685477581E+18", INT64_MAX);

    // Test with INT64_MIN (-9223372036854775808)
    errors += aTestFromToInt("-9.22337203685477581E+18", INT64_MIN);

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
    errors += aTestFromToInt("9.22337203685477581E+18", 9223372036854775806);

    /* Boundary Conditions Around Limits - Near INT64_MAX Test 9223372036854775800 (close to INT64_MAX but with trailing zeros).*/
    errors += aTestFromToInt("9.2233720368547758E+18", 9223372036854775800);

    /* Boundary Conditions Around Limits - Near INT64_MIN: Test -9223372036854775807 (one more than INT64_MIN). */
    errors += aTestFromToInt("-9.22337203685477581E+18", -9223372036854775807);

    /* Boundary Conditions Around Limits - Near INT64_MIN: Test -9223372036854775800 (close to INT64_MIN but with trailing zeros). */
    errors += aTestFromToInt("-9.2233720368547758E+18", -9223372036854775800);

    /* Values Just Beyond Limits: 9223372036854775808 - should trigger Invalid_operation. */
    errors += aTestBeyondLimits("9223372036854775808");

    /* Values Just Beyond Limits: -9223372036854775809 - should trigger Invalid_operation. */
    errors += aTestBeyondLimits("-9223372036854775809");

    /* Mid-Range Large Values: Positive: 123456789012345678 */
    errors += aTestFromToInt("123456789012345678", 123456789012345678);

    /* Mid-Range Large Values: Negative: -123456789012345678 */
    errors += aTestFromToInt("-123456789012345678", -123456789012345678);

    /* Values that are exactly powers of 10: 1000000000 */
    errors += aTestFromToInt("1000000000", 1000000000);

    /* Values that are exactly powers of 10: 100000000000000000 */
    errors += aTestFromToInt("100000000000000000", 100000000000000000);

    /* Values that are exactly powers of 10: -1000000000 */
    errors += aTestFromToInt("-1000000000", -1000000000);

    /* Values that are exactly powers of 10: -100000000000000000 */
    errors += aTestFromToInt("-100000000000000000", -100000000000000000);

    /* Values that are exactly powers of 10: 10 */
    errors += aTestFromToInt("10", 10);

    /* Values that are exactly powers of 10: -10 */
    errors += aTestFromToInt("-10", -10);

    /* All 9's: 999999999999999999 */
    errors += aTestFromToInt("999999999999999999", 999999999999999999);

    /* All 9's: -999999999999999999 */
    errors += aTestFromToInt("-999999999999999999", -999999999999999999);

    /* Loop and Generate a few random 18-digit numbers and test both positive and negative forms */
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
        // Make sure it is only 18 digits
        int_input = int_input % 1000000000000000000;

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
    if (result != 1234567890123456790) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("decNumber: 1234567890123456789 -> 1234567890123456790, Result: %lld (i.e. rounded to 18 digits)\n", result);

    return errors;
}

/* Test add */
int test_add() {
    value a, b, result;
    int errors = 0;
    char buffer[32];
    a.decimal_value = NULL;
    b.decimal_value = NULL;
    result.decimal_value = NULL;

    plugin->setDigits(plugin, 18);
    plugin->base.signal_number = 0;

    printf("\nTesting Add\n");

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

    // Test with two floating point numbers
    plugin->decimalFromString(plugin, &a, "1.23456789");
    plugin->decimalFromString(plugin, &b, "1.23456789");
    plugin->decimalAdd(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("2.46913578", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1.23456789 + 1.23456789 = %s\n", buffer);

    // Test plus with large numbers (no loss of precision)
    plugin->decimalFromString(plugin, &a, "123456789012345678");
    plugin->decimalFromString(plugin, &b, "987654321098765432");
    plugin->decimalAdd(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("1.11111111011111111E+18", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("123456789012345678 + 987654321098765432 = %s\n", buffer);

    // Test plus with large numbers (loss of precision)
    plugin->decimalFromString(plugin, &a, "1234567890123456789");
    plugin->decimalFromString(plugin, &b, "9876543210987654321");
    plugin->decimalAdd(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("1.11111111011111111E+19", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1234567890123456789 + 9876543210987654321 = %s (rounded to 18 digits)\n", buffer);

    // Test with a long double overflow
    // Make a string 2/3rds of max long double
    char bignum[32];
    sprintf(bignum, "%Lg", LDBL_MAX / 3 * 2);
    plugin->base.signal_number = 0;
    plugin->decimalFromString(plugin, &a, bignum);
    plugin->decimalFromString(plugin, &b, bignum);
    // Check no signal in setting up the numbers
    if (plugin->base.signal_number != 0) {
        printf("ERROR IN SETUP - %s", plugin->base.signal_string);
        errors++;
    }
    plugin->decimalAdd(plugin, &result, &a, &b);
    if (plugin->base.signal_number != 0) {
        printf("OK SIGNAL - ");
    }
    else {
        printf("ERROR NO SIGNAL - ");
        errors++;
    }
    printf("%Lg + %Lg -> %Lg\n", *(long double*)a.decimal_value, *(long double*)b.decimal_value, *(long double*)result.decimal_value);

    // Test with an Infinity (should return an Infinity)
    plugin->decimalFromString(plugin, &a, "inf");
    plugin->decimalFromString(plugin, &b, "1");
    plugin->decimalAdd(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("inf", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("inf + 1 = %s\n", buffer);

    // Test with a NaN
    plugin->decimalFromString(plugin, &a, "nan");
    plugin->decimalFromString(plugin, &b, "1");
    // Clear the signal
    plugin->base.signal_number = 0;
    plugin->decimalAdd(plugin, &result, &a, &b);
    if (!plugin->base.signal_number) {
        printf("Error - NaN -> No signal\n");
        errors++;
    }
    else printf("OK - NaN -> Signal\n");

    // Test rounding when digits is set to 5
    plugin->setDigits(plugin, 5);
    plugin->decimalFromString(plugin, &a, "1.23456789"); // Rounded to 5 significant digits = 1.2346
    plugin->decimalFromString(plugin, &b, "1.23456789"); // Rounded to 5 significant digits = 1.2346
    plugin->decimalAdd(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("2.4692", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else {
        // Double-check with long double comparison
        double long val = *(double long*)result.decimal_value;
        if (val != 2.4692L) {
            printf("Error (float comparison) - ");
            errors++;
        }
        else printf("OK - ");
    }
    printf("1.23456789 + 1.23456789 = %s (using 5 digits maths)\n", buffer);

    // Put the digits back to 18
    plugin->setDigits(plugin, 18);
    return errors;
}

/* Test subtract */
int test_subtract() {
    value a, b, result;
    int errors = 0;
    char buffer[32];
    a.decimal_value = NULL;
    b.decimal_value = NULL;
    result.decimal_value = NULL;

    plugin->setDigits(plugin, 18);
    plugin->base.signal_number = 0;

    printf("\nTesting subtract\n");

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

    // Test with two floating point numbers
    plugin->decimalFromString(plugin, &a, "9.87654321");
    plugin->decimalFromString(plugin, &b, "1.23456789");
    plugin->decimalSub(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("8.64197532", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("9.87654321 - 1.23456789 = %s\n", buffer);

    // Test minus with large numbers (no loss of precision)
    plugin->decimalFromString(plugin, &a, "987654321098765432");
    plugin->decimalFromString(plugin, &b, "123456789012345678");
    plugin->decimalSub(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("864197532086419754", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("987654321098765432 - 123456789012345678 = %s\n", buffer);

    // Test minus with large numbers (loss of precision)
    plugin->decimalFromString(plugin, &a, "9876543210987654321");
    plugin->decimalFromString(plugin, &b, "1234567890123456789");
    plugin->decimalSub(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("8.64197532086419753E+18", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("9876543210987654321 - 1234567890123456789 = %s (rounded to 18 digits)\n", buffer);

    // Test with a long double overflow
    // Make a string 2/3rds of max long double
    char bignum[32];
    sprintf(bignum, "%Lg", LDBL_MAX / 3 * 2);
    plugin->base.signal_number = 0;
    plugin->decimalFromString(plugin, &a, bignum);
    sprintf(bignum, "-%Lg", LDBL_MAX / 3 * 2);
    plugin->decimalFromString(plugin, &b, bignum);
    // Check no signal in setting up the numbers
    if (plugin->base.signal_number != 0) {
        printf("ERROR IN SETUP - %s", plugin->base.signal_string);
        errors++;
    }
    plugin->decimalSub(plugin, &result, &a, &b);
    if (plugin->base.signal_number != 0) {
        printf("OK SIGNAL - ");
    }
    else {
        printf("ERROR NO SIGNAL - ");
        errors++;
    }
    printf("%Lg - %Lg -> %Lg\n", *(long double*)a.decimal_value, *(long double*)b.decimal_value, *(long double*)result.decimal_value);

    // Test with an Infinity (should return an Infinity)
    plugin->decimalFromString(plugin, &a, "inf");
    plugin->decimalFromString(plugin, &b, "1");
    plugin->decimalSub(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("inf", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("inf - 1 = %s\n", buffer);

    // Test with a NaN
    plugin->decimalFromString(plugin, &a, "nan");
    plugin->decimalFromString(plugin, &b, "1");
    // Clear the signal
    plugin->base.signal_number = 0;
    plugin->decimalSub(plugin, &result, &a, &b);
    if (!plugin->base.signal_number) {
        printf("Error - NaN -> No signal\n");
        errors++;
    }
    else printf("OK - NaN -> Signal\n");

    // Test rounding when digits is set to 5
    plugin->setDigits(plugin, 5);
    plugin->decimalFromString(plugin, &a, "9.87654321"); // Rounded to 5 significant digits = 9.8765
    plugin->decimalFromString(plugin, &b, "1.23456789"); // Rounded to 5 significant digits = 1.2346
    plugin->decimalSub(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("8.6419", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else {
        // Double-check with long double comparison
        double long val = *(double long*)result.decimal_value;
        if (val != 8.6419L) {
            printf("Error (float comparison) - ");
            errors++;
        }
        else printf("OK - ");
    }
    printf("9.87654321 - 1.23456789 = %s (using 5 digits maths)\n", buffer);

    // Put the digits back to 18
    plugin->setDigits(plugin, 18);

    return errors;
}

/* Test multiply */
int test_multiply() {
    value a, b, result;
    int errors = 0;
    char buffer[32];
    a.decimal_value = NULL;
    b.decimal_value = NULL;
    result.decimal_value = NULL;

    plugin->setDigits(plugin, 18);
    plugin->base.signal_number = 0;

    printf("\nTesting multiply\n");

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

    // Test with two floating point numbers
    plugin->decimalFromString(plugin, &a, "1.23456789");
    plugin->decimalFromString(plugin, &b, "1.23456789");
    plugin->decimalMul(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("1.5241578750190521", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1.23456789 * 1.23456789 = %s\n", buffer);

    // Test multiply with large numbers (no loss of precision)
    plugin->decimalFromString(plugin, &a, "123456789");
    plugin->decimalFromString(plugin, &b, "987654321");
    plugin->decimalMul(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("121932631112635269", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("123456789012345678 * 987654321098765432 = %s\n", buffer);

    // Test multiply with large numbers (loss of precision)
    plugin->decimalFromString(plugin, &a, "1234567890123456789");
    plugin->decimalFromString(plugin, &b, "9876543210987654321");
    plugin->decimalMul(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("1.21932631137021795E+37", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1234567890123456789 * 9876543210987654321 = %s (rounded to 18 digits)\n", buffer);

    // Test with a long double overflow
    // Make a string 2/3rds of max long double
    char bignum[32];
    sprintf(bignum, "%Lg", LDBL_MAX / 3 * 2);
    plugin->base.signal_number = 0;
    plugin->decimalFromString(plugin, &a, bignum);
    plugin->decimalFromString(plugin, &b, bignum);
    // Check no signal in setting up the numbers
    if (plugin->base.signal_number != 0) {
        printf("ERROR IN SETUP - %s", plugin->base.signal_string);
        errors++;
    }
    plugin->decimalMul(plugin, &result, &a, &b);
    if (plugin->base.signal_number != 0) {
        printf("OK SIGNAL - ");
    }
    else {
        printf("ERROR NO SIGNAL - ");
        errors++;
    }
    printf("%Lg * %Lg -> %Lg\n", *(long double*)a.decimal_value, *(long double*)b.decimal_value, *(long double*)result.decimal_value);

    // Test with an Infinity (should return an Infinity)
    plugin->decimalFromString(plugin, &a, "inf");
    plugin->decimalFromString(plugin, &b, "1");
    plugin->decimalMul(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("inf", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("inf * 1 = %s\n", buffer);

    // Test with a NaN
    plugin->decimalFromString(plugin, &a, "nan");
    plugin->decimalFromString(plugin, &b, "1");
    // Clear the signal
    plugin->base.signal_number = 0;
    plugin->decimalMul(plugin, &result, &a, &b);
    if (!plugin->base.signal_number) {
        printf("Error - NaN -> No signal\n");
        errors++;
    }
    else printf("OK - NaN -> Signal\n");

    // Test rounding when digits is set to 5
    plugin->setDigits(plugin, 5);
    plugin->decimalFromString(plugin, &a, "1.23456789"); // Rounded to 5 significant digits = 1.2346
    plugin->decimalFromString(plugin, &b, "1.23456789"); // Rounded to 5 significant digits = 1.2346
    plugin->decimalMul(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("1.5242", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else {
        // Double-check with long double comparison
        double long val = *(double long*)result.decimal_value;
        if (val != 1.5242L) {
            printf("Error (float comparison) - ");
            errors++;
        }
        else printf("OK - ");
    }
    printf("1.23456789 * 1.23456789 = %s (using 5 digits maths)\n", buffer);

    // Put the digits back to 18
    plugin->setDigits(plugin, 18);

    return errors;
}

/* Test divide */
int test_divide() {
    value a, b, result;
    int errors = 0;
    char buffer[32];
    a.decimal_value = NULL;
    b.decimal_value = NULL;
    result.decimal_value = NULL;

    plugin->setDigits(plugin, 18);
    plugin->base.signal_number = 0;

    printf("\nTesting divide\n");

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

    // Test with two floating point numbers
    plugin->decimalFromString(plugin, &a, "9.87654321");
    plugin->decimalFromString(plugin, &b, "1.23456789");
    plugin->decimalDiv(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("8.00000007290000066", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("9.87654321 / 1.23456789 = %s\n", buffer);

    // Test divide with large numbers (no loss of precision)
    plugin->decimalFromString(plugin, &a, "321572632265607");
    plugin->decimalFromString(plugin, &b, "8866444222000");
    plugin->decimalDiv(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("36.2685", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("987654321098765432 / 12345678901234567 = %s\n", buffer);

    // Test divide with large numbers (loss of precision)
    plugin->decimalFromString(plugin, &a, "9876543210987654321");
    plugin->decimalFromString(plugin, &b, "1234567890123456789");
    plugin->decimalDiv(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("8.00000007290000066", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("9876543210987654321 / 1234567890123456789 = %s (rounded to 18 digits)\n", buffer);

    // Test with a long double overflow
    // Make a string 2/3rds of max long double
    char bignum[32];
    sprintf(bignum, "%Lg", LDBL_MAX / 3 * 2);
    plugin->base.signal_number = 0;
    plugin->decimalFromString(plugin, &a, bignum);
    plugin->decimalFromString(plugin, &b, "1.2344E-100");
    // Check no signal in setting up the numbers
    if (plugin->base.signal_number != 0) {
        printf("ERROR IN SETUP - %s", plugin->base.signal_string);
        errors++;
    }
    plugin->decimalDiv(plugin, &result, &a, &b);
    if (plugin->base.signal_number != 0) {
        printf("OK SIGNAL - ");
    }
    else {
        printf("ERROR NO SIGNAL - ");
        errors++;
    }
    printf("%Lg / %Lg -> %Lg\n", *(long double*)a.decimal_value, *(long double*)b.decimal_value, *(long double*)result.decimal_value);

    // Test with an Infinity (should return an Infinity)
    plugin->decimalFromString(plugin, &a, "inf");
    plugin->decimalFromString(plugin, &b, "1");
    plugin->decimalDiv(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("inf", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("inf / 1 = %s\n", buffer);

    // Test with a NaN
    plugin->decimalFromString(plugin, &a, "nan");
    plugin->decimalFromString(plugin, &b, "1");
    // Clear the signal
    plugin->base.signal_number = 0;
    plugin->decimalDiv(plugin, &result, &a, &b);
    if (!plugin->base.signal_number) {
        printf("Error - NaN -> No signal\n");
        errors++;
    }
    else printf("OK - NaN -> Signal\n");

    // Test rounding when digits is set to 5
    plugin->setDigits(plugin, 5);
    plugin->decimalFromString(plugin, &a, "9.87654321"); // Rounded to 5 significant digits = 9.8765
    plugin->decimalFromString(plugin, &b, "1.23456789"); // Rounded to 5 significant digits = 1.2346
    plugin->decimalDiv(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("7.9998", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else {
        // Double-check with long double comparison
        double long val = *(double long*)result.decimal_value;
        if (val != 7.9998L) {
            printf("Error (float comparison) - ");
            errors++;
        }
        else printf("OK - ");
    }
    printf("9.87654321 / 1.23456789 = %s (using 5 digits maths)\n", buffer);

    // Put the digits back to 18
    plugin->setDigits(plugin, 18);

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

    // Test with digits set to 5 and a number that is different after 5 digits
    plugin->decimalFromString(plugin, &a, "1.11111");
    plugin->decimalFromString(plugin, &b, "1.11112");
    plugin->setDigits(plugin, 5); // Compare should be done with 5 digits
    result = plugin->decimalCompare(plugin, &a, &b);
    if (result != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1.11111 == 1.11112 (5 digits)\n");

    // Put the digits back to 18
    plugin->setDigits(plugin, 18);

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

    // Test with a NaN
    plugin->decimalFromString(plugin, &a, "nan");
    // Clear the signal
    plugin->base.signal_number = 0;
    plugin->decimalNeg(plugin, &result, &a);
    if (!plugin->base.signal_number) {
        printf("Error - NaN -> No signal\n");
        errors++;
    }
    else printf("OK - NaN -> Signal\n");

    // Test with an Infinity
    plugin->decimalFromString(plugin, &a, "inf");
    plugin->decimalNeg(plugin, &result, &a);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("-inf", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("inf -> %s\n", buffer);

    // Test with negative Infinity
    plugin->decimalFromString(plugin, &a, "-inf");
    plugin->decimalNeg(plugin, &result, &a);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("inf", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("-inf -> %s\n", buffer);

    // Test with a floating point number
    plugin->decimalFromString(plugin, &a, "1.23456789");
    plugin->decimalNeg(plugin, &result, &a);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("-1.23456789", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1.23456789 -> %s\n", buffer);

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

    // Test with a NaN
    plugin->decimalFromString(plugin, &a, "nan");
    plugin->decimalFromString(plugin, &b, "1");
    // Clear the signal
    plugin->base.signal_number = 0;
    plugin->decimalPow(plugin, &result, &a, &b);
    if (!plugin->base.signal_number) {
        printf("Error - NaN -> No signal\n");
        errors++;
    }
    else printf("OK - NaN -> Signal\n");

    // Test with an Infinity
    plugin->decimalFromString(plugin, &a, "inf");
    plugin->decimalFromString(plugin, &b, "1");
    plugin->decimalPow(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("inf", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("inf^1 = %s\n", buffer);

    // Test with a negative Infinity
    plugin->decimalFromString(plugin, &a, "-inf");
    plugin->decimalFromString(plugin, &b, "1");
    plugin->decimalPow(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("-inf", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("-inf^1 = %s\n", buffer);

    // Test with floats - no loss of precision
    plugin->decimalFromString(plugin, &a, "2.5");
    plugin->decimalFromString(plugin, &b, "3");
    plugin->decimalPow(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("15.625", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("2.5^3 = %s\n", buffer);

    // Test with floats - loss of precision - 5 digits
    plugin->setDigits(plugin, 5);
    plugin->decimalFromString(plugin, &a, "2.5");
    plugin->decimalFromString(plugin, &b, "3.5");
    plugin->decimalPow(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, buffer);
    if (strcmp("24.705", buffer) != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("2.5^3.5 = %s (5 digits)\n", buffer);

    // Put the digits back to 18
    plugin->setDigits(plugin, 18);

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

    // Test with digits set to 5 and a number that is different after 5 digits
    plugin->decimalFromString(plugin, &a, "1.11111");
    plugin->setDigits(plugin, 5); // Compare should be done with 5 digits
    result = plugin->decimalCompareString(plugin, &a, "1.11112");
    if (result != 0) {
        printf("Error - ");
        errors++;
    }
    else printf("OK - ");
    printf("1.11111 == 1.11112 (5 digits)\n");

    // Put the digits back to 18
    plugin->setDigits(plugin, 18);

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
    errors += testDecimalExtract("123456789012345678", "1.23456789012345678", 17);
    errors += testDecimalExtract("-123456789012345678", "-1.23456789012345678", 17);
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

    return errors;
}

// Main function
int main(int argc, char *argv[]) {
    int errors = 0;

    // Load the plugin
  //  printf("db_decimal_test - Loading Dynamic Plugin\n");
  //  if (load_rxvmplugin(".", "rxvm_db_decimal") != 0) {
  //      printf("Unable to load the rxvmplugin plugin\n");
  //      return 1;
  //  }
    printf("db_decimal_test - Loading Manual Static Plugin\n");
    CALL_PLUGIN_INITIALIZER(dbnumber);

    plugin = (decplugin*)get_rxvmplugin(RXVM_PLUGIN_DECIMAL);
    if (!plugin) {
        printf("No default rxvmplugin plugin\n");
        return 1;
    }

    // Set the number of digits in the rxvmplugin context
    plugin->setDigits(plugin, 18);

    errors += test_int_tofrom_conversions();
    errors += test_moreDecimalToInteger();
    errors += test_decimalFromDouble();
    errors += test_decimalToDouble();
    errors += test_decimalToString_decimalFromString();
    errors += test_add();
    errors += test_subtract();
    errors += test_multiply();
    errors += test_divide();
    errors += test_decimalCompare();
    errors += test_decimalCompareString();
    errors += test_decimalNeg();
    errors += test_decimalPow();
    errors += test_decimalExtract();

    plugin->base.free((rxvm_plugin*)plugin);

    return errors;
} // main
