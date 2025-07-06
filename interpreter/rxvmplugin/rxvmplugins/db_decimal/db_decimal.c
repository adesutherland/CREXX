/*
 * Description: This file contains the implementation of the rxvmplugin plugin using
 *              platform-specific long double precision floating point numbers.
*/

#define RXVM_PLUGIN dbnumber

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "rxvmplugin.h"

/* Private Context */
typedef struct dbcontext {
    size_t digits;
    size_t max_digits;
} dbcontext;

#include <math.h>  // Ensure math.h is included before using math_errhandling

/* Check if the compiler supports C99 or later */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* C99 or later */
#if defined(MATH_ERRNO) && (math_errhandling & MATH_ERRNO)
#define USE_ERRNO 1
#else
#define USE_ERRNO 0
#endif
#else
/* C90 or earlier */
#define USE_ERRNO 0
#endif

// Macro to clear errno if it is used
#if USE_ERRNO
    #define CLEAR_ERRNO errno = 0
#else
    #define CLEAR_ERRNO
#endif

// Note that signal_string is set to a static buffer in the check_signal function (or NULL)
static void check_signal(decplugin *plugin, long double value) {
    plugin->base.signal_number = 0;
    plugin->base.signal_string = NULL;

    // Detect the long double errors
#if USE_ERRNO
    if (errno == ERANGE) {
        plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
        plugin->base.signal_string = "Overflow/Underflow";
        errno = 0; // Clear errno
        return;
    }
    if (errno == EDOM) {
        plugin->base.signal_number = RXSIGNAL_DIVISION_BY_ZERO;
        plugin->base.signal_string = "Divide by zero or domain error";
        errno = 0; // Clear errno
        return;
    }
#else
    if (value == HUGE_VALL || value == -HUGE_VALL) {
        plugin->base.signal_number = RXSIGNAL_DIVISION_BY_ZERO;
        plugin->base.signal_string = "Divide by zero";
        return;
    }
    if (value == LDBL_MAX || value == -LDBL_MAX) {
        plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
        plugin->base.signal_string = "Overflow/Underflow";
        return;
    }
    if (value == LDBL_MIN || value == -LDBL_MIN) {
        plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
        plugin->base.signal_string = "Overflow/Underflow";
        return;
    }
    if (isnan(value)) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string = "Invalid operation";
        return;
    }
    if (value == 0.0 && signbit(value)) {
        plugin->base.signal_number = RXSIGNAL_DIVISION_BY_ZERO;
        plugin->base.signal_string = "Negative zero";
        return;
    }
#endif
}

// Precompute 10^decimal_digits to avoid repeated computation
static const long double powers_of_10[] = {
    1.0L, 10.0L, 100.0L, 1000.0L, 10000.0L,
    100000.0L, 1000000.0L, 10000000.0L, 100000000.0L,
    1000000000.0L, 10000000000.0L, 100000000000.0L,
    1000000000000.0L, 10000000000000.0L, 100000000000000.0L,
    1000000000000000.0L, 10000000000000000.0L,
    100000000000000000.0L, 1000000000000000000.0L
};


// Rounding a long double to a given number of significant digits
long double round_decimal(long double value, size_t significant_digits) {
    // Check for overflow
    if (value == HUGE_VALL || value == LDBL_MAX) {
        return HUGE_VALL;
    }
    if (value == -HUGE_VALL || value == -LDBL_MAX) {
        return -HUGE_VALL;
    }
    // Special case for 0 or -0
    if (value == 0.0L) {
        return value;
    }

    // Determine the exponent (base 10) of the value - we need to be careful about binary rounding
    // Get a provisional exponent from log10
    long double abs_value = fabsl(value);
    long double log_val = log10l(abs_value);
    int exponent = (int) floorl(log_val);

    // Compare back
    // Calculate 10^exponent using powers_of_10[] if the exponent is between -18 and 18, otherwise use powl()
    long double power;
    if (exponent == 0) {
        power = 1.0L;
    }
    else if (exponent > 0 && exponent <= 18) {
        power = powers_of_10[exponent];
    }
    else if (exponent < 0 && exponent >= -18) {
        power = 1.0L / powers_of_10[-exponent];
    }
    else {
        power = powl(10.0L, (long double)exponent);
    }
    // If abs_value is actually less than that power, it means a log10 overshot
    if (abs_value < power) {
        exponent--;
    }
    int exponent_shift = (int)(significant_digits) - 1 - exponent;

    // Fast path: no rounding needed because we already have the digits requested
    if (exponent_shift == 0) {
        return value;
    }

    /* - Clamping seens unecessary
    // Clamping to avoid overflow
#define CLAMP 4900
    if (exponent_shift > CLAMP) {
        // If exponent_shift > CLAMP the result effectively becomes 0 after scaling, so we just return 0.0 with the sign
        return (value > 0.0L) ? 0.0L : -0.0L;
    } else if (exponent_shift < -CLAMP) {
        // If exponent_shift < -CLAMPresult effectively is the unchanged 'value'
        return value;
    }
    */

    // Calculate a scaling factor:
    // We shift the decimal so 'roundl' will act on 'significant_digits' digits
    // Calculate multiplier = 10^exponent using powers_of_10[] if the exponent is between -18 and 18, otherwise use powl()
    long double multiplier;
    if (exponent_shift > 0) {
        if (exponent_shift <= 18) {
            multiplier = powers_of_10[exponent_shift];
        }
        else {
            multiplier = powl(10.0L, (long double)exponent_shift);
        }
    }
    else {
        if (exponent_shift >= -18) {
            multiplier = 1.0L / powers_of_10[-exponent_shift];
        }
        else {
            multiplier = 1.0L / powl(10.0L, (long double)-exponent_shift);
        }
    }

    // Scale the value
    long double scaled = value * multiplier;

    // Round to the nearest integer
    long double rounded = roundl(scaled);

    // Scale back to the original magnitude
    return rounded / multiplier;
}

/* Ensure that the decNumber is big enough to hold the number */
static void EnsureCapacity(value *number) {
    size_t size = sizeof(long double);
    /* If the value has a rxvmplugin buffer already */
    if (number->decimal_value) {

        /* If the size is bigger than the current size, reallocate the memory */
        if (size > number->decimal_buffer_length) {
            /* Allocate the new memory */
            void *new_value = realloc(number->decimal_value, size);

            /* If the reallocation was unsuccessful, panic as per crexx standard behavior */
            if (new_value == NULL) {
                fprintf(stderr, "PANIC: Unable to reallocate memory for decNumber\n");
                exit(1);
            }
            number->decimal_value = new_value;
            number->decimal_buffer_length = size;
        }
        number->decimal_value_length = size;
    }
    else {
        /* Allocate the memory for the decNumber */
        number->decimal_value = malloc(size);
        number->decimal_buffer_length = size;
        number->decimal_value_length = size;
    }
}

/* Definitions the rxvmplugin functions */

/* Get the number of digits in the rxvmplugin context */
static size_t getDigits(decplugin *plugin) {
    return ((dbcontext*)(plugin->base.private_context))->digits;
}

/* Set the number of digits in the rxvmplugin context */
static void setDigits(decplugin *plugin, size_t digits) {
    if (digits > ((dbcontext*)(plugin->base.private_context))->max_digits)
        digits = ((dbcontext*)(plugin->base.private_context))->max_digits; /* Max for this double implementation */
    ((dbcontext*)(plugin->base.private_context))->digits = digits;
}

/* Get the required string size for the rxvmplugin context */
static size_t getRequiredStringSize(decplugin *plugin) {
    return ((dbcontext*)(plugin->base.private_context))->digits + 14;
}

/* Convert a string to a rxvmplugin number */
static void decimalFromString(decplugin *plugin, value *result, const char *string) {
    char *endptr;
    EnsureCapacity(result);
    long double *number = result->decimal_value;
    CLEAR_ERRNO;

    // Handle NaN
    if (strcmp(string, "nan") == 0) {
        *number = NAN;
        return;
    }

    // Handle Infinity
    if (strcmp(string, "inf") == 0) {
        *number = INFINITY;
        return;
    }

    if (strcmp(string, "-inf") == 0) {
        *number = -INFINITY;
        return;
    }

    // Convert to double
    *number = strtold(string, &endptr);
    if (endptr == string) *number = NAN;
    else *number = round_decimal(*number, ((dbcontext*)(plugin->base.private_context))->digits);
    check_signal(plugin, *number);
}

/* Convert a rxvmplugin number to a string */
/* The string must be allocated by the caller and should be at least
 * getRequiredStringSize() bytes */
static void decimalToString(decplugin *plugin, const value *number, char *string) {
    long double value = *(long double*)number->decimal_value;
    // Handle special cases
    if (isnan(value)) {
        strcpy(string, "nan");
        return;
    }
    if (value == HUGE_VALL || value == LDBL_MAX) {
        strcpy(string, "inf");
        return;
    }
    if (value == -HUGE_VALL || value == -LDBL_MAX) {
        strcpy(string, "-inf");
        return;
    }
    if (value == 0.0) {
        // Handle signed zero
        if (signbit(value)) {
            strcpy(string, "-0");
        }
        else {
            strcpy(string, "0");
        }
        return;
    }
    sprintf(string, "%.*LG", (int)((dbcontext*)(plugin->base.private_context))->digits, value);
}

/* Convert an int to a rxvmplugin number */
void decimalFromInt(decplugin *plugin, value *result, const rxinteger value) {
    EnsureCapacity(result);
    long double *number = result->decimal_value;
    *number = (long double)value;
    *number = round_decimal(*number, ((dbcontext*)(plugin->base.private_context))->digits);
    check_signal(plugin, *number);
}

/* Convert a rxvmplugin number to an int */

/* Needed forward declaration */
static void decimalExtract(decplugin *plugin, char *coefficient, rxinteger *exponent, value *decimal);

/* Precompute 10^decimal_digits to avoid repeated computation */
static const rxinteger int_powers_of_10[] = {
    1, 10, 100, 1000, 10000,
    100000, 1000000, 10000000, 100000000,
    1000000000, 10000000000, 100000000000,
    1000000000000, 10000000000000, 100000000000000,
    1000000000000000, 10000000000000000,
    100000000000000000, 1000000000000000000
};

void decimalToInt(decplugin *plugin, const value *number, rxinteger *int_value) {
    char coefficient[64];
    rxinteger exponent;
    char* c = coefficient;
    size_t sig_digits;
    long double decimalValue = *(long double*)number->decimal_value;
    *int_value = 0;
    char is_neg = 0;
    uint64_t uvalue = 0;

    // Handle special cases
    if (isnan(decimalValue)) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string = "Source is NaN";
        return;
    }
    if (decimalValue == HUGE_VALL || decimalValue == LDBL_MAX || decimalValue == -HUGE_VALL || decimalValue == -LDBL_MAX ||
        decimalValue > (long double)INT64_MAX || decimalValue < (long double)INT64_MIN) {
        plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
        plugin->base.signal_string = "Overflow/Underflow";
        return;
    }

    // Extract coefficient and exponent
    decimalExtract(plugin, coefficient, &exponent, (value*)number);

    // Process the coefficient to get the number of digits
    if (*c == '-') {
        // Skip the negative sign
        c++;
        is_neg = 1;
    }
    if (exponent > 18) {
        // The value is too large to fit in an int64_t
        plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
        plugin->base.signal_string = "Overflow/Underflow";
        return;
    }
    if (!isdigit(c[0])) {
        // Not a number - e.g. "nan" or "inf"
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string = "Source is not a number";
        return;
    }

    // Accumulate the digits
    sig_digits = 0;
    for (; *c; c++) {
        if (!isdigit(*c)) continue;
        uvalue = uvalue * 10 + (*c - '0');
        sig_digits++;
    }

    // Apply the rule: exponent >= (sig_digits - 1)
    if (exponent >= (sig_digits - 1)) {
        // The value is an integer
        exponent -= ((rxinteger)sig_digits -1);
        // Note we are using unsigned ints here because of a edge case where INT64_MAX is exceeded because of rounding
        uvalue *= int_powers_of_10[exponent];
        if (is_neg) {
            if (-uvalue < INT64_MIN) {
                plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
                plugin->base.signal_string = "Overflow/Underflow";
            }
            else {
                *int_value = -(rxinteger)uvalue;
            }
        }
        else {
            if (uvalue > INT64_MAX) {
                plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
                plugin->base.signal_string = "Overflow/Underflow";
            }
            else {
                *int_value = (rxinteger)uvalue;
            }
        }
    } else {
        // The value is not an integer
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string = "Source is not an integer";
    }
}

/* Convert a double to a rxvmplugin number */
void decimalFromDouble(decplugin *plugin, value *result, double input) {
    // CLEAR_ERRNO;
    // EnsureCapacity(result);
    // // Convert from double to long double
    // long double* target = result->decimal_value;
    // *target = (long double)input;
    // size_t digits = ((dbcontext*)(plugin->base.private_context))->digits;
    // if (digits > DBL_DIG) { // digits should not be bigger than the maximum precision of a double
    //     digits = DBL_DIG;
    // }
    // *target = round_decimal(*target, digits);
    // check_signal(plugin, *target);

    char buffer[32]; // Enough to hold a double in scientific notation.
    // Note that transforming via a string is the most feasible way to do this while maintaining precision;
    // printf uses advanced formatting logic (like Grisu or Dragon4) that decides the minimal decimal digits needed
    // so that parsing the result returns exactly the same binary double which we can't do here.
    // Otherwise we get lots of trailing "9s" in the result because of double precision limitations.
    // Special case for 0 and -0
    if (input == 0.0) {
        EnsureCapacity(result);
        long double *number = result->decimal_value;
        if (signbit(input)) {
            *number = -0.0;
        }
        else {
            *number = 0.0;
        }
    }
    else {
        snprintf(buffer, sizeof(buffer), "%.16g", input); // 16 digits for double precision
        decimalFromString(plugin, result, buffer);
    }
}

/* Convert a rxvmplugin number to a double */
void decimalToDouble(decplugin *plugin, const value *input, double *result) {
    CLEAR_ERRNO;
    *result = (double)*(long double*)input->decimal_value;
    /* Check if *result is a valid number - note that check_signal() works for long double but not double ... */
    if (isnan(*result)) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string = "Source is NaN";
    }
    if (*result == HUGE_VAL || *result == -HUGE_VAL) {
        plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
        plugin->base.signal_string = "Overflow/Underflow";
    }
    check_signal(plugin, *result);
}

/* Add two rxvmplugin numbers */
static void decimalAdd(decplugin *plugin, value *result, const value *op1, const value *op2) {
    CLEAR_ERRNO;
    EnsureCapacity(result);
    long double number;

    number = *(long double*)op1->decimal_value + *(long double*)op2->decimal_value;
    number = round_decimal(number, ((dbcontext*)(plugin->base.private_context))->digits);
    check_signal(plugin, number);
    *((long double*)result->decimal_value) = number;
}

/* Subtract two rxvmplugin numbers */
static void decimalSub(decplugin *plugin, value *result, const value *op1, const value *op2) {
    CLEAR_ERRNO;
    EnsureCapacity(result);
    long double number;
    number = *(long double*)op1->decimal_value - *(long double*)op2->decimal_value;
    number = round_decimal(number, ((dbcontext*)(plugin->base.private_context))->digits);
    check_signal(plugin, number);
    *((long double*)result->decimal_value) = number;
}

/* Multiply two rxvmplugin numbers */
static void decimalMul(decplugin *plugin, value *result, const value *op1, const value *op2) {
    CLEAR_ERRNO;
    EnsureCapacity(result);
    long double number;
    number = *(long double*)op1->decimal_value * *(long double*)op2->decimal_value;
    number = round_decimal(number, ((dbcontext*)(plugin->base.private_context))->digits);
    check_signal(plugin, number);
    *((long double*)result->decimal_value) = number;
}

/* Divide two rxvmplugin numbers */
static void decimalDiv(decplugin *plugin, value *result, const value *op1, const value *op2) {
    CLEAR_ERRNO;
    EnsureCapacity(result);
    long double number;
    number = *(long double*)op1->decimal_value / *(long double*)op2->decimal_value;
    number = round_decimal(number, ((dbcontext*)(plugin->base.private_context))->digits);
    check_signal(plugin, number);
    *((long double*)result->decimal_value) = number;
}

/* Power two rxvmplugin numbers */
static void decimalPow(decplugin *plugin, value *result, const value *op1, const value *op2) {
    CLEAR_ERRNO;
    EnsureCapacity(result);
    long double number;
    number = powl(*(long double*)op1->decimal_value, *(long double*)op2->decimal_value);
    number = round_decimal(number, ((dbcontext*)(plugin->base.private_context))->digits);
    check_signal(plugin, number);
    *((long double*)result->decimal_value) = number;
}

/* Change the sign of an rxvmplugin number */
static void decimalNeg(decplugin *plugin, value *result, const value *op1) {
    CLEAR_ERRNO;
    if (result->decimal_value != op1->decimal_value)
        EnsureCapacity(result);
    long double *op = op1->decimal_value;
    long double number;
    if (*op == 0.0L) {
        number = 0.0L;
    } else {
        number = -(*op);
        number = round_decimal(number, ((dbcontext*)(plugin->base.private_context))->digits);
        check_signal(plugin, number);
    }
    *((long double*)result->decimal_value) = number;
}

/* Compare two rxvmplugin numbers returning -1, 0, 1 for less than, equal, greater than */
static int decimalCompare(decplugin *plugin, const value *op1, const value *op2) {
    long double number1 = *(long double*)op1->decimal_value;
    long double number2 = *(long double*)op2->decimal_value;
    if (isnan(number1)) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string = "Invalid operation";
        return 0;
    }
    if (isnan(number2)) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string = "Invalid operation";
        return 0;
    }

    // Round numbers to digits
    number1 = round_decimal(number1, ((dbcontext*)(plugin->base.private_context))->digits);
    number2 = round_decimal(number2, ((dbcontext*)(plugin->base.private_context))->digits);

    if (number1 < number2) return -1;
    if (number1 > number2) return 1;

    return 0;
}

/* Compare an rxvmplugin number to a string representation of a number returning -1, 0, 1 for less than, equal, greater than */
static int decimalCompareString(decplugin *plugin, const value *op1, const char *op2) {
    char *endptr;
    long double number1 = *(long double*)op1->decimal_value;
    if (isnan(number1)) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string = "Invalid operation";
        return 0;
    }

    errno = 0;  // Reset errno before the call
    long double number2 = strtold(op2, &endptr);
    if (endptr == op2) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string = "Invalid operation";
        return 0;
    }
    if (errno == ERANGE) {
        if (number2 == HUGE_VALL || number2 == -HUGE_VALL) {
            plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
            plugin->base.signal_string = "Overflow";
        } else {
            plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
            plugin->base.signal_string = "Underflow";
        }
        return 0;
    }
    if (number2 == HUGE_VALL || number2 == -HUGE_VALL) {
        plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
        plugin->base.signal_string = "Overflow/Underflow";
        return 0;
    }
    if (isnan(number2)) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string = "Invalid operation";
        return 0;
    }
    // Round numbers to digits
    number1 = round_decimal(number1, ((dbcontext*)(plugin->base.private_context))->digits);
    number2 = round_decimal(number2, ((dbcontext*)(plugin->base.private_context))->digits);
    if (number1 < number2) return -1;
    if (number1 > number2) return 1;
    return 0;
}

// Static utility function to trim trailing zeros from a number format and including possibly the decimal point
static void trim_numeric_trailing_zeros(char *str) {
    size_t len = strlen(str);
    if (len == 0)
        return;

    // Find the decimal point
    char *dot = strchr(str, '.');
    if (!dot)
        return; // No decimal point, nothing to trim

    // Start from the end of the string
    char *end = str + len - 1;

    // Remove trailing zeros
    while (end > dot && *end == '0') {
        *end = '\0';
        end--;
    }

    // If the last character is a decimal point, remove it
    if (end == dot) {
        *end = '\0';
    }
}

/* Extract the coefficient, exponent and sign from a rxvmplugin number.
 * - `decimal` contains the float value.
 * - `coefficient' will store the coefficient as a string (or nan, inf, -inf).
 *    It must be allocated by the caller and should be at least getRequiredStringSize()
 *    bytes (too big but simple for the caller)
 * - `exponent` will store the exponent as an integer.
 * Normalized, round to context precision/digits, trim trailing zeros.
 */
static void decimalExtract(decplugin *plugin, char *coefficient, rxinteger *exponent, value *decimal) {

    *exponent = 0;
    long double value = *(long double*)decimal->decimal_value;

    // Handle special cases
    if (isnan(value)) {
        strcpy(coefficient, "nan");
        return;
    }
    if (value == HUGE_VALL || value == LDBL_MAX) {
        strcpy(coefficient, "inf");
        return;

    }
    if (value == -HUGE_VALL || value == -LDBL_MAX) {
        strcpy(coefficient, "-inf");
        return;
    }
    if (value == 0.0L) {
        // Handle zero
        strcpy(coefficient, "0");
        return;
    }

    // Determine if negative
    int is_negative = value < 0.0L;
    long double abs_value = fabsl(value);

    // Calculate decimal exponent
    int64_t exp = (int64_t)floorl(log10l(abs_value));

    // Normalize the coefficient to [1.0, 10.0)
    long double coeff = abs_value / powl(10.0L, (long double)exp);

    // Adjust if coeff is exactly 10.0 due to floating-point inaccuracies
    if (coeff >= 10.0L) {
        coeff /= 10.0L;
        exp += 1;
    }
    // Adjust if coeff is smaller than 1 due to floating-point inaccuracies
    if (coeff < 1.0L) {
        coeff *= 10.0L;
        exp -= 1;
    }

    *exponent = exp;

    // Format the coefficient string with precision upto digits fractional digits
    sprintf(coefficient, is_negative ? "-%.*Lf" : "%.*Lf", (int)((dbcontext*)(plugin->base.private_context))->digits - 1, coeff);

    /* Trim trailing zeros */
    trim_numeric_trailing_zeros(coefficient);
}

/* Is zero? */
static int decimalIsZero(decplugin *plugin, const value *number) {
    long double value = *(long double*)number->decimal_value;
    return (value == 0.0L || value == -0.0L);
}

/* Truncate the decimal value to an integer */
static void decimalTruncate(decplugin *plugin, value *result, const value *op1) {
    CLEAR_ERRNO;
    EnsureCapacity(result);
    long double number = *(long double*)op1->decimal_value;
    // Truncate the decimal part
    number = truncl(number);
    number = round_decimal(number, ((dbcontext*)(plugin->base.private_context))->digits);
    check_signal(plugin, number);
    *((long double*)result->decimal_value) = number;
}

/* Round the decimal value to the nearest integer */
static void decimalRound(decplugin *plugin, value *result, const value *op1) {
    CLEAR_ERRNO;
    EnsureCapacity(result);
    long double number = *(long double*)op1->decimal_value;
    // Round to the nearest integer
    number = roundl(number);
    number = round_decimal(number, ((dbcontext*)(plugin->base.private_context))->digits);
    check_signal(plugin, number);
    *((long double*)result->decimal_value) = number;
}


/* Function to destroy a rxvmplugin plugin */
static void destroy_decplugin(rxvm_plugin *plugin) {
    free(plugin->private_context);
    free(plugin);
}

/* Function to create a new rxvmplugin plugin */
static rxvm_plugin *new_decplugin() {
    /* Allocate memory for the context */
    dbcontext* context = malloc(sizeof(dbcontext)); // NOLINT

    context->digits = LDBL_DIG; // set precision
    context->max_digits = LDBL_DIG; // set max precision

    /* Allocate memory for the plugin */
    decplugin *plugin = malloc(sizeof(decplugin));
    plugin->base.type = RXVM_PLUGIN_DECIMAL;
    plugin->base.private_context = context;
    plugin->base.name = "dbdecimal";
    plugin->base.version = "0.1";
    plugin->base.description = "Decimal Plugin based on long double";
    plugin->base.free = destroy_decplugin;
    plugin->getDigits = getDigits;
    plugin->setDigits = setDigits;
    plugin->getRequiredStringSize = getRequiredStringSize;
    plugin->decimalFromString = decimalFromString;
    plugin->decimalToString = decimalToString;
    plugin->decimalFromInt = decimalFromInt;
    plugin->decimalToInt = decimalToInt;
    plugin->decimalFromDouble = decimalFromDouble;
    plugin->decimalToDouble = decimalToDouble;
    plugin->decimalAdd = decimalAdd;
    plugin->decimalSub = decimalSub;
    plugin->decimalMul = decimalMul;
    plugin->decimalDiv = decimalDiv;
    plugin->decimalPow = decimalPow;
    plugin->decimalNeg = decimalNeg;
    plugin->decimalCompare = decimalCompare;
    plugin->decimalCompareString = decimalCompareString;
    plugin->decimalExtract = decimalExtract;
    plugin->decimalIsZero = decimalIsZero;
    plugin->decimalTruncate = decimalTruncate;
    plugin->decimalRound = decimalRound;

    return (rxvm_plugin*)plugin;
}

// Register the rxvmplugin plugin factory
REGISTER_PLUGIN(new_decplugin)
