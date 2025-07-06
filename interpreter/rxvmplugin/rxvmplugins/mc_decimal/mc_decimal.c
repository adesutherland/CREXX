/*
 * Description: This file contains the implementation of the rxvmplugin plugin using the ICU
 *              decNumber library.
 *
 * Created by Adrian Sutherland on 15/09/2024.
 *
 * -------------------------------------------------------------------------------
 * The following parameters have been tuned in rxvmplugin decNumber for CREXX:
 * DECDPUN = 8       # For 64-bit (Overriding 3)
 * DECNUMDIGITS = 64 # Default (Overriding 100)
 * DECBUFFER = 64    # Internal Buffer size avoiding malloc (Overriding 36)
 * DECUSE64 = 1      # 64-bit (Default)
 * DECEXTFLAG  = 1   # Extended set of flags (Default)
 * DECSUBSET = 0     # No subset - possibly higher performance (Default)
 * -------------------------------------------------------------------------------
*/

#define RXVM_PLUGIN decnumber

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "rxvmplugin.h"
#include "decNumber.h"
#include "decNumberLocal.h"

// Note that signal_string is set to a static buffer in the check_signal function (or NULL)
static void check_signal(decplugin *plugin) {
    plugin->base.signal_number = 0;
    plugin->base.signal_string = NULL;
    static char buffer[100]; // Buffer for the signal string
    decContext *dec_context = ((decContext*)(plugin->base.private_context));

    if (dec_context->status & DEC_Errors) {

        switch (dec_context->status) {
            case DEC_Invalid_operation:
                plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
                break;
            case DEC_Division_by_zero:
                plugin->base.signal_number = RXSIGNAL_DIVISION_BY_ZERO;
                break;
            case DEC_Overflow:
            case DEC_Underflow:
                plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
                break;
            default:
                plugin->base.signal_number = RXSIGNAL_ERROR;
        }
        strcpy(buffer, "decNumber: ");
        strcat(buffer, decContextStatusToString(dec_context));
        plugin->base.signal_string = buffer;
    }

    dec_context->status = 0; // clear status
}

/* Calculate the size of the decNumber */
static size_t getRequiredDecNumberSize(size_t digits) {
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
    return size;
}

/* Ensure that the decNumber is big enough to hold the number */
static void EnsureCapacity(value *number, size_t digits) {
    /* Calculate the size of the decNumber */
    size_t size = getRequiredDecNumberSize(digits);

    /* If the value has a rxvmplugin buffer already */
    if (number->decimal_value) {

        /* If the size is bigger than the current size, reallocate the memory */
        if (size > number->decimal_buffer_length) {
            /* Allocate the new memory */
            decNumber *new_value = realloc(number->decimal_value, size);

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
    return ((decContext*)(plugin->base.private_context))->digits;
}

/* Set the number of digits in the rxvmplugin context */
static void setDigits(decplugin *plugin, size_t digits) {
    ((decContext*)(plugin->base.private_context))->digits = (int32_t)digits;
}

/* Get the required string size for the rxvmplugin context */
static size_t getRequiredStringSize(decplugin *plugin) {
    return ((decContext*)(plugin->base.private_context))->digits + 14;
}

static void decNumberToCREXXString(decplugin *plugin, decNumber *number, char *buffer) {

    // The standard decNumber library prints numbers in simple form or sci/eng form based on the exponent. If the
    // exponent is 0 it will print the number in simple form, otherwise it will print it in sci/eng form. This means
    // that if the number is normalised (1.xxxEyy) it will be printed in sci/eng form, even if the exponent is small
    // (e.g. 1E4 will not print as 10000). This is not ideal for the rxvmplugin plugin as it is expected to print the number in simple form
    // if the exponent is within a certain range. This function will print the number in simple form if the exponent is
    // within a certain range, otherwise it will print it in scientific form.
    decContext *context = (decContext*)(plugin->base.private_context);

    #define LOWER_THRESHOLD -5

    // If a longer buffer is needed, it will be dynamically allocated
    #define CONVERSION_BUFFER_LEN 100
    char conversion_buffer[CONVERSION_BUFFER_LEN];

    // Determine the range to decide on fixed-point or exponential notation
    int32_t exponent;
    int32_t precision = context->digits;
    // Define thresholds based on %g specifier logic
    int32_t upper_threshold = precision - 1;

    // The decimal library prints exponent == 0 in simple form as a "fast path" so let's handle that first
    if (number->exponent == 0) {
        exponent = number->digits - 1;
        if (exponent >= LOWER_THRESHOLD && exponent <= upper_threshold) {
            // Number is in the range for simple format
            decNumberToString(number, buffer);
            return;
        }
    }

    // Normalise the number to simplify the exponent and coefficient
    decNumberNormalize(number, number, context);
    exponent = number->exponent;

    // Decide on format based on thresholds
    if (exponent >= LOWER_THRESHOLD && exponent <= upper_threshold) {

        // Get a buffer big enough to hold the normalised number
        size_t bufferSize = context->digits + 14;
        if (bufferSize > CONVERSION_BUFFER_LEN) {
            char *temp_buffer = malloc(bufferSize);
            decNumberToString(number, temp_buffer);
            plugin->number_to_simple_format(temp_buffer, buffer);
            free(temp_buffer);
        }
        else {
            decNumberToString(number, conversion_buffer);
            plugin->number_to_simple_format(conversion_buffer, buffer);
        }
    }

    else {
        // Exponential notation
        decNumberToString(number, buffer);
    }
}

/* Convert a string to a rxvmplugin number */
static void decimalFromString(decplugin *plugin, value *result, const char *string) {
    decContext *context = (decContext*)(plugin->base.private_context);
    EnsureCapacity(result, context->digits);
    decNumberFromString(result->decimal_value, string, context);
    check_signal(plugin);
}

/* Convert a rxvmplugin number to a string */
/* The string must be allocated by the caller and should be at least
 * getRequiredStringSize() bytes */
static void decimalToString(decplugin *plugin, const value *number, char *string) {
    decNumber *dn = number->decimal_value;

    if (!dn) {
        strcpy(string, "nan");
        return;
    }

    // Handle Infinity
    if (decNumberIsInfinite(dn)) {
        if (dn->bits & DECNEG) {
            strcpy(string, "-inf");
        } else {
            strcpy(string, "inf");
        }
    }

    // Handle NaN
    else if (decNumberIsNaN(dn)) {
        strcpy(string, "nan");
    }

    else {
        decNumberToCREXXString(plugin, dn, string);
    }
    check_signal(plugin);
}

/* Convert an int to a rxvmplugin number */
void decimalFromInt(decplugin *plugin, value *result, const rxinteger value) {
    decContext *context = (decContext*)(plugin->base.private_context);
    EnsureCapacity(result, context->digits);
    /* Is rxinteger 32 bit? */
    if (IS_RXINTEGER_32BIT) {
        decNumberFromInt32(result->decimal_value, value);
    }
    else {
        decNumberFromInt64(result->decimal_value, value);
    }
    check_signal(plugin);
}

/* Convert a rxvmplugin number to an int */
void decimalToInt(decplugin *plugin, const value *number, rxinteger *integer) {
    decContext *context = (decContext*)(plugin->base.private_context);
    /* Is rxinteger 32 bit? */
    if (IS_RXINTEGER_32BIT) {
        *integer = decNumberToInt32(number->decimal_value, context);
    }
    else {
        *integer = decNumberToInt64(number->decimal_value, context);
    }
    check_signal(plugin);
}

/* Convert a double to a rxvmplugin number */
void decimalFromDouble(decplugin *plugin, value *result, double input) {
    char buffer[32]; // Enough to hold a double in scientific notation
    decContext *context = (decContext*)(plugin->base.private_context);
    EnsureCapacity(result, context->digits);
    decNumber *dn = result->decimal_value;

    // Handle NaN
    if (isnan(input)) {
        decNumberZero(result->decimal_value);
        dn->bits |= DECNAN;
        return;
    }

    // Handle Infinity
    if (isinf(input)) {
        decNumberZero(result->decimal_value);
        dn->bits |= DECINF;
        if (signbit(input)) dn->bits |= DECNEG;
        return;
    }

    // Handle zero (including signed zero)
    if (input == 0.0) {
        decNumberZero(dn);
        if (signbit(input)) dn->bits |= DECNEG;
        return;
    }

    snprintf(buffer, sizeof(buffer), "%.16g", input); // 16 digits for double precision
    decNumberFromString(result->decimal_value, buffer, context);
    check_signal(plugin);
}

/* Convert a rxvmplugin number to a double */
void decimalToDouble(decplugin *plugin, const value *input, double *result) {
    decNumber *dn = input->decimal_value;

    // Handle NaN
    if (dn->bits & DECNAN) {
        *result = NAN;
        return;
    }

    // Handle Infinity
    if (dn->bits & DECINF) {
        *result = (dn->bits & DECNEG) ? -INFINITY : INFINITY;
        return;
    }

    // Handle zero (including signed zero)
    if (decNumberIsZero(dn)) {
        *result = (dn->bits & DECNEG) ? -0.0 : 0.0;
        return;
    }

    // For finite values, convert to string and then to double
    size_t bufferSize = getRequiredStringSize(plugin);
    char *buffer = malloc(bufferSize);
    if (buffer == NULL) {
        fprintf(stderr, "PANIC: Failed to allocate memory for double conversion\n");
        exit(1);
    }

    decNumberToString(dn, buffer);
    *result = strtod(buffer, NULL);
    free(buffer);
    check_signal(plugin);
}

/* Add two rxvmplugin numbers */
static void decimalAdd(decplugin *plugin, value *result, const value *op1, const value *op2) {
    EnsureCapacity(result, ((decContext*)(plugin->base.private_context))->digits);
    decNumberAdd(result->decimal_value, op1->decimal_value, op2->decimal_value, (decContext*)(plugin->base.private_context));
    check_signal(plugin);
}

/* Subtract two rxvmplugin numbers */
static void decimalSub(decplugin *plugin, value *result, const value *op1, const value *op2) {
    EnsureCapacity(result, ((decContext*)(plugin->base.private_context))->digits);
    decNumberSubtract(result->decimal_value, op1->decimal_value, op2->decimal_value, (decContext*)(plugin->base.private_context));
    check_signal(plugin);
}

/* Multiply two rxvmplugin numbers */
static void decimalMul(decplugin *plugin, value *result, const value *op1, const value *op2) {
    EnsureCapacity(result, ((decContext*)(plugin->base.private_context))->digits);
    decNumberMultiply(result->decimal_value, op1->decimal_value, op2->decimal_value, (decContext*)(plugin->base.private_context));
    check_signal(plugin);
}

/* Divide two rxvmplugin numbers */
static void decimalDiv(decplugin *plugin, value *result, const value *op1, const value *op2) {
    EnsureCapacity(result, ((decContext*)(plugin->base.private_context))->digits);
    decNumberDivide(result->decimal_value, op1->decimal_value, op2->decimal_value, (decContext*)(plugin->base.private_context));
    check_signal(plugin);
}

/* Power two rxvmplugin numbers */
static void decimalPow(decplugin *plugin, value *result, const value *op1, const value *op2) {
    EnsureCapacity(result, ((decContext*)(plugin->base.private_context))->digits);
    decNumberPower(result->decimal_value, op1->decimal_value, op2->decimal_value, (decContext*)(plugin->base.private_context));
    check_signal(plugin);
}

/* Change the sign of an rxvmplugin number */
static void decimalNeg(decplugin *plugin, value *result, const value *op1) {
    if (op1 != result) EnsureCapacity(result, ((decContext*)(plugin->base.private_context))->digits);
    decNumberMinus(result->decimal_value, op1->decimal_value, (decContext*)(plugin->base.private_context));
    check_signal(plugin);
}

/* Compare two rxvmplugin numbers returning -1, 0, 1 for less than, equal, greater than */
static int decimalCompare(decplugin *plugin, const value *op1, const value *op2) {
    decContext *context = (decContext*)(plugin->base.private_context);
    decNumber result;
    decNumberCompare(&result, op1->decimal_value, op2->decimal_value, context);
    int cmp = decNumberToInt32(&result, context);
    check_signal(plugin);
    return cmp;
}

/* Compare an rxvmplugin number to a string representation of a number returning -1, 0, 1 for less than, equal, greater than */
static int decimalCompareString(decplugin *plugin, const value *op1, const char *op2) {
    decContext *context = (decContext*)(plugin->base.private_context);
    decNumber *op2dn;
    decNumber buffer;
    // Allocate memory for the decNumber representation of the string
    size_t digits = plugin->getDigits(plugin);
    if (digits > DECNUMDIGITS) {
        op2dn = malloc(getRequiredDecNumberSize(digits));
    }
    else {
        op2dn = &buffer;
    }
    decNumberFromString(op2dn, op2, context);
    decNumber result;
    decNumberCompare(&result, op1->decimal_value, op2dn, context);
    int cmp = decNumberToInt32(&result, context);
    check_signal(plugin);
    if (op2dn != &buffer) {
        free(op2dn);
    };
    return cmp;
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
    const Unit *up;
    char *c=coefficient;
    uInt u, pow;
    Int cut;

    /* Handle NaN / Infinity */
    decNumber *dn = decimal->decimal_value;
    if (decNumberIsNaN(dn)) {
        strcpy(coefficient, "nan");
        *exponent = 0;
        return;
    }
    if (decNumberIsInfinite(dn)) {
        if (dn->bits & DECNEG) {
            strcpy(coefficient, "-inf");
        } else {
            strcpy(coefficient, "inf");
        }
        *exponent = 0;
        return;
    }

    /* Handle zero */
    if (decNumberIsZero(dn)) {
        strcpy(coefficient, "0");
        *exponent = 0;
        return;
    }

    /* Handle a number with zero digits - probable not needed but just in case! */
    if (dn->digits == 0) {
        strcpy(coefficient, "0");
        *exponent = 0;
        return;
    }

    /* Get the exponent */
    *exponent = dn->exponent + dn->digits - 1;

    /* Get the coefficient */

    /* Negative number? */
    if (dn->bits & DECNEG) *c++='-';

    /* Process the first digit */
    up=dn->lsu+D2U(dn->digits)-1;    // First Unit
    cut = MSUDIGITS(dn->digits) - 1; // Digits in the first Unit
    u=*up;
    TODIGIT(u, cut, c, pow);
    c++;
    cut--;

    /* If more than 1 digit add a decimal point */
    if (dn->digits > 1) {
        *c++='.';
    }

    /* Process the rest of the first Unit */
    for (; cut>=0; c++, cut--) {
        TODIGIT(u, cut, c, pow);
    }

    /* Process the rest of the Units */
    cut=DECDPUN-1; /* next Unit is full */
    up--;          /* next Unit */
    for (;up>=dn->lsu; up--) {
        u=*up;
        for (; cut>=0; c++, cut--) { /* Process all digits in the Unit */
            TODIGIT(u, cut, c, pow);
        }
        cut=DECDPUN-1; /* next Units are full */
    }
    *c='\0';

    /* Trim trailing zeros */
    trim_numeric_trailing_zeros(coefficient);
}

/* Is zero */
static int decimalIsZero(decplugin *plugin, const value *number) {
    decNumber *dn = number->decimal_value;
    return (decNumberIsZero(dn));
}

/* Truncate the decimal value to an integer */
static void decimalTruncate(decplugin *plugin, value *result, const value *op1) {
    EnsureCapacity(result, ((decContext*)(plugin->base.private_context))->digits);
    enum rounding rounding_value = decContextGetRounding((decContext*)(plugin->base.private_context));
    decContextSetRounding((decContext*)(plugin->base.private_context), DEC_ROUND_DOWN); // Set rounding to truncate
    decNumberToIntegralValue(result->decimal_value, op1->decimal_value, (decContext*)(plugin->base.private_context));
    decContextSetRounding((decContext*)(plugin->base.private_context), rounding_value); // Restore original rounding
    check_signal(plugin);
}

/* Round the decimal value to the nearest integer */
static void decimalRound(decplugin *plugin, value *result, const value *op1) {
    EnsureCapacity(result, ((decContext*)(plugin->base.private_context))->digits);
    enum rounding rounding_value = decContextGetRounding((decContext*)(plugin->base.private_context));
    decContextSetRounding((decContext*)(plugin->base.private_context), DEC_ROUND_HALF_EVEN); // Set rounding to round
    decNumberToIntegralValue(result->decimal_value, op1->decimal_value, (decContext*)(plugin->base.private_context));
    decContextSetRounding((decContext*)(plugin->base.private_context), rounding_value); // Restore original rounding
    check_signal(plugin);
}

/* Function to destroy a rxvmplugin plugin */
static void destroy_decplugin(rxvm_plugin *plugin) {
    free(plugin->private_context);
    free(plugin);
}

/* Function to create a new rxvmplugin plugin */
static rxvm_plugin *new_decplugin() {
    /* Allocate memory for the context */
    decContext* context = malloc(sizeof(decContext));
    decContextDefault(context, DEC_INIT_BASE); // initialize
    context->traps = 0; // no traps
    context->digits = DECNUMDIGITS; // set precision

    /* Allocate memory for the plugin */
    decplugin *plugin = malloc(sizeof(decplugin));
    plugin->base.type = RXVM_PLUGIN_DECIMAL;
    plugin->base.private_context = context;
    plugin->base.name = "mcdecimal";
    plugin->base.version = "Plugin:0.1 Library:3.64";
    plugin->base.description = "Decimal Plugin using Mike Cowlishaw's decNumber library";
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
