/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Isolated fixed-34 DECIMAL-01 comparator using the vendored decQuad API. */

#define RXVM_PLUGIN decquadnumber

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "rxvmplugin.h"
#include "decQuad.h"

enum { DECQUAD_CANDIDATE_STRING = 80 };

typedef struct decquad_plugin_context {
    decContext arithmetic;
    size_t digits;
    char signal[96];
} decquad_plugin_context;

static decquad_plugin_context *private_context(decplugin *plugin) {
    return (decquad_plugin_context *)plugin->base.private_context;
}

static decContext *arithmetic_context(decplugin *plugin) {
    return &private_context(plugin)->arithmetic;
}

static void clear_signal(decplugin *plugin) {
    plugin->base.signal_number = RXSIGNAL_NONE;
    plugin->base.signal_string = NULL;
    arithmetic_context(plugin)->status = 0;
}

static void check_status(decplugin *plugin) {
    uint32_t errors = arithmetic_context(plugin)->status & DEC_Errors;
    decquad_plugin_context *state = private_context(plugin);

    plugin->base.signal_number = RXSIGNAL_NONE;
    plugin->base.signal_string = NULL;
    if (!errors) return;
    if (errors & DEC_Insufficient_storage) {
        RX_PANIC_OOM("decQuad candidate operation", 0u, "decimal value");
    }
    if (errors & DEC_Division_by_zero) {
        plugin->base.signal_number = RXSIGNAL_DIVISION_BY_ZERO;
    }
    else if (errors & (DEC_Overflow | DEC_Underflow)) {
        plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
    }
    else if (errors & (DEC_Conversion_syntax | DEC_Invalid_operation |
                       DEC_Division_impossible | DEC_Division_undefined |
                       DEC_Invalid_context)) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
    }
    else {
        plugin->base.signal_number = RXSIGNAL_ERROR;
    }
    (void)snprintf(state->signal, sizeof(state->signal),
                   "decQuad status 0x%08" PRIx32, errors);
    plugin->base.signal_string = state->signal;
}

static decQuad *ensure_capacity(decplugin *plugin, value *number) {
    decQuad *quad = (decQuad *)plugin->reserve_decimal(number, sizeof(decQuad));
    if (!quad) {
        RX_PANIC_OOM("reserve decQuad candidate sidecar", sizeof(decQuad),
                     "decimal value");
    }
    return quad;
}

static const decQuad *quad_value(const value *number) {
    return (const decQuad *)number->decimal_value;
}

static size_t context_digits(decplugin *plugin) {
    return private_context(plugin)->digits;
}

static int round_quad(decplugin *plugin, decQuad *number, size_t digits) {
    decContext *context = arithmetic_context(plugin);
    decQuad quantum;
    decQuad rounded;
    char quantum_text[32];
    int32_t adjusted;
    int32_t quantum_exponent;

    if (!decQuadIsFinite(number) || decQuadIsZero(number) ||
        digits >= DECQUAD_Pmax || decQuadDigits(number) <= digits) return 0;
    adjusted = decQuadGetExponent(number) + (int32_t)decQuadDigits(number) - 1;
    quantum_exponent = adjusted - (int32_t)digits + 1;
    (void)snprintf(quantum_text, sizeof(quantum_text), "1E%+" PRId32,
                   quantum_exponent);
    decQuadFromString(&quantum, quantum_text, context);
    if (context->status & DEC_Errors) return 1;
    decQuadQuantize(&rounded, number, &quantum, context);
    if (context->status & DEC_Errors) return 1;
    decQuadCopy(number, &rounded);
    return 0;
}

static void syncNumericContext(decplugin *plugin) {
    static numeric_context defaults = {
        DEFAULT_NUMERIC_DIGITS, DEFAULT_NUMERIC_FUZZ, DEFAULT_NUMERIC_FORM,
        DEFAULT_NUMERIC_CASE, NUMERIC_STANDARD_COMMON
    };
    numeric_context *numeric = plugin->num_context ? plugin->num_context
                                                   : &defaults;
    decContext *context = arithmetic_context(plugin);

    clear_signal(plugin);
    if (numeric->digits == 0u || numeric->digits > DECQUAD_Pmax) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string =
            "decQuad candidate supports numeric digits 1 through 34";
        return;
    }
    private_context(plugin)->digits = (size_t)numeric->digits;
    context->digits = DECQUAD_Pmax;
    context->clamp = numeric->standard == NUMERIC_STANDARD_COMMON ? 1 : 0;
    context->round = numeric->standard == NUMERIC_STANDARD_COMMON
        ? DEC_ROUND_HALF_EVEN : DEC_ROUND_HALF_UP;
}

static size_t getDigits(decplugin *plugin) {
    return context_digits(plugin);
}

static size_t getRequiredStringSize(decplugin *plugin, const value *number) {
    (void)plugin;
    (void)number;
    return DECQUAD_CANDIDATE_STRING;
}

static void decimalFromString(decplugin *plugin, value *result,
                              const char *string) {
    decQuad *output = ensure_capacity(plugin, result);
    decQuad signed_zero;
    clear_signal(plugin);
    decQuadFromString(output, string, arithmetic_context(plugin));
    if (!(arithmetic_context(plugin)->status & DEC_Errors))
        (void)round_quad(plugin, output, context_digits(plugin));
    /* decQuad canonicalizes textual negative zero; retain the provider
     * contract's signed-zero observation explicitly. */
    if (!(arithmetic_context(plugin)->status & DEC_Errors) &&
        string[0] == '-' && decQuadIsZero(output) &&
        !decQuadIsSigned(output)) {
        decQuadCopyNegate(&signed_zero, output);
        decQuadCopy(output, &signed_zero);
    }
    check_status(plugin);
}

static void decimalExtract(decplugin *plugin, char *coefficient,
                           rxinteger *exponent, value *decimal) {
    const decQuad *number = quad_value(decimal);
    uint8_t bcd[DECQUAD_Pmax];
    size_t first = 0u;
    size_t last = DECQUAD_Pmax - 1u;
    char *out = coefficient;

    (void)plugin;
    *exponent = 0;
    if (decQuadIsNaN(number)) {
        strcpy(coefficient, "nan");
        return;
    }
    if (decQuadIsInfinite(number)) {
        strcpy(coefficient, decQuadIsNegative(number) ? "-inf" : "inf");
        return;
    }
    if (decQuadIsZero(number)) {
        strcpy(coefficient, "0");
        return;
    }

    (void)decQuadGetCoefficient(number, bcd);
    while (first < DECQUAD_Pmax && bcd[first] == 0u) ++first;
    while (last > first && bcd[last] == 0u) --last;
    *exponent = (rxinteger)decQuadGetExponent(number) +
                (rxinteger)(DECQUAD_Pmax - first) - 1;
    if (decQuadIsNegative(number)) *out++ = '-';
    *out++ = (char)('0' + bcd[first]);
    if (last > first) {
        *out++ = '.';
        while (++first <= last) *out++ = (char)('0' + bcd[first]);
    }
    *out = '\0';
}

static void init_local_string(decplugin *plugin, value *local) {
    memset(local, 0, sizeof(*local));
    if (!plugin->reserve_string(local, getRequiredStringSize(plugin, NULL))) {
        RX_PANIC_OOM("reserve decQuad formatting sidecar",
                     getRequiredStringSize(plugin, NULL), "decimal formatting");
    }
}

static void decimalToString(decplugin *plugin, const value *number,
                            char *string) {
    static numeric_context defaults = {
        DEFAULT_NUMERIC_DIGITS, DEFAULT_NUMERIC_FUZZ, DEFAULT_NUMERIC_FORM,
        DEFAULT_NUMERIC_CASE, NUMERIC_STANDARD_COMMON
    };
    numeric_context *numeric = plugin->num_context ? plugin->num_context
                                                   : &defaults;
    const decQuad *quad;
    value coefficient_value;
    value exponent_value;
    value formatted_value;

    clear_signal(plugin);
    if (!number->decimal_value || rxvm_value_decimal_length(number) == 0u) {
        strcpy(string, "nan");
        return;
    }
    quad = quad_value(number);
    if (decQuadIsNaN(quad)) { strcpy(string, "nan"); return; }
    if (decQuadIsInfinite(quad)) {
        strcpy(string, decQuadIsNegative(quad) ? "-inf" : "inf");
        return;
    }
    if (decQuadIsZero(quad)) {
        strcpy(string, decQuadIsSigned(quad) ? "-0" : "0");
        return;
    }

    init_local_string(plugin, &coefficient_value);
    memset(&exponent_value, 0, sizeof(exponent_value));
    init_local_string(plugin, &formatted_value);
    decimalExtract(plugin, coefficient_value.string_value,
                   &exponent_value.int_value, (value *)number);
    coefficient_value.string_length = strlen(coefficient_value.string_value);
    plugin->format_number_components(numeric, &coefficient_value,
                                     &exponent_value, &formatted_value);
    strcpy(string, formatted_value.string_value);
    plugin->release_value_storage(&coefficient_value);
    plugin->release_value_storage(&formatted_value);
    clear_signal(plugin);
}

static void decimalFromInt(decplugin *plugin, value *result, rxinteger input) {
    char text[32];
    (void)snprintf(text, sizeof(text), "%" PRId64, (int64_t)input);
    decimalFromString(plugin, result, text);
}

static int quad_to_integer(decplugin *plugin, const decQuad *number,
                           rxinteger *result) {
    uint8_t bcd[DECQUAD_Pmax];
    size_t first = 0u;
    size_t significant;
    int32_t adjusted;
    int32_t zeroes;
    uint64_t magnitude = 0u;
    uint64_t limit = decQuadIsNegative(number)
        ? (uint64_t)INT64_MAX + 1u : (uint64_t)INT64_MAX;
    size_t i;

    *result = 0;
    if (!decQuadIsFinite(number) || !decQuadIsInteger(number)) return 1;
    if (decQuadIsZero(number)) return 0;
    (void)decQuadGetCoefficient(number, bcd);
    while (first < DECQUAD_Pmax && bcd[first] == 0u) ++first;
    significant = DECQUAD_Pmax - first;
    adjusted = decQuadGetExponent(number) + (int32_t)significant - 1;
    zeroes = adjusted - (int32_t)significant + 1;
    if (zeroes < 0) {
        significant = (size_t)((int32_t)significant + zeroes);
        zeroes = 0;
    }
    for (i = 0; i < significant; ++i) {
        unsigned digit = bcd[first + i];
        if (magnitude > (limit - digit) / 10u) return 1;
        magnitude = magnitude * 10u + digit;
    }
    while (zeroes-- > 0) {
        if (magnitude > limit / 10u) return 1;
        magnitude *= 10u;
    }
    if (decQuadIsNegative(number)) {
        *result = magnitude == (uint64_t)INT64_MAX + 1u
            ? (rxinteger)INT64_MIN : -(rxinteger)magnitude;
    }
    else *result = (rxinteger)magnitude;
    (void)plugin;
    return 0;
}

static void decimalToInt(decplugin *plugin, const value *number,
                         rxinteger *result) {
    clear_signal(plugin);
    if (quad_to_integer(plugin, quad_value(number), result)) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string =
            "decQuad value is not an in-range integer";
    }
}

static void decimalFromDouble(decplugin *plugin, value *result, double input) {
    char text[32];
    if (isnan(input)) strcpy(text, "nan");
    else if (isinf(input)) strcpy(text, signbit(input) ? "-inf" : "inf");
    else if (input == 0.0 && signbit(input)) strcpy(text, "-0");
    else (void)snprintf(text, sizeof(text), "%.16g", input);
    decimalFromString(plugin, result, text);
}

static void decimalToDouble(decplugin *plugin, const value *number,
                            double *result) {
    char text[DECQUAD_String];
    clear_signal(plugin);
    decQuadToString(quad_value(number), text);
    *result = strtod(text, NULL);
}

typedef decQuad *(*binary_operation)(decQuad *, const decQuad *,
                                     const decQuad *, decContext *);

static void apply_binary(decplugin *plugin, value *result, const value *op1,
                         const value *op2, binary_operation operation) {
    decQuad output;
    clear_signal(plugin);
    operation(&output, quad_value(op1), quad_value(op2),
              arithmetic_context(plugin));
    if (!(arithmetic_context(plugin)->status & DEC_Errors))
        (void)round_quad(plugin, &output, context_digits(plugin));
    decQuadCopy(ensure_capacity(plugin, result), &output);
    check_status(plugin);
}

static void decimalAdd(decplugin *plugin, value *result, const value *op1,
                       const value *op2) {
    apply_binary(plugin, result, op1, op2, decQuadAdd);
}

static void decimalSub(decplugin *plugin, value *result, const value *op1,
                       const value *op2) {
    apply_binary(plugin, result, op1, op2, decQuadSubtract);
}

static void decimalMul(decplugin *plugin, value *result, const value *op1,
                       const value *op2) {
    apply_binary(plugin, result, op1, op2, decQuadMultiply);
}

static void decimalDiv(decplugin *plugin, value *result, const value *op1,
                       const value *op2) {
    apply_binary(plugin, result, op1, op2, decQuadDivide);
}

static void decimalPow(decplugin *plugin, value *result, const value *op1,
                       const value *op2) {
    rxinteger exponent;
    uint64_t remaining;
    decQuad base;
    decQuad output;
    decQuad temporary;
    decQuad one;
    int negative;

    clear_signal(plugin);
    if (quad_to_integer(plugin, quad_value(op2), &exponent) ||
        exponent > 100000 || exponent < -100000) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
        plugin->base.signal_string =
            "decQuad candidate power supports bounded integral exponents only";
        return;
    }
    negative = exponent < 0;
    remaining = negative ? (uint64_t)(-(exponent + 1)) + 1u
                         : (uint64_t)exponent;
    decQuadCopy(&base, quad_value(op1));
    decQuadFromInt32(&output, 1);
    while (remaining != 0u &&
           !(arithmetic_context(plugin)->status & DEC_Errors)) {
        if (remaining & 1u) {
            decQuadMultiply(&temporary, &output, &base,
                            arithmetic_context(plugin));
            if (!(arithmetic_context(plugin)->status & DEC_Errors))
                (void)round_quad(plugin, &temporary, context_digits(plugin));
            decQuadCopy(&output, &temporary);
        }
        remaining >>= 1u;
        if (remaining != 0u) {
            decQuadMultiply(&temporary, &base, &base,
                            arithmetic_context(plugin));
            if (!(arithmetic_context(plugin)->status & DEC_Errors))
                (void)round_quad(plugin, &temporary, context_digits(plugin));
            decQuadCopy(&base, &temporary);
        }
    }
    if (negative && !(arithmetic_context(plugin)->status & DEC_Errors)) {
        decQuadFromInt32(&one, 1);
        decQuadDivide(&temporary, &one, &output, arithmetic_context(plugin));
        if (!(arithmetic_context(plugin)->status & DEC_Errors))
            (void)round_quad(plugin, &temporary, context_digits(plugin));
        decQuadCopy(&output, &temporary);
    }
    decQuadCopy(ensure_capacity(plugin, result), &output);
    check_status(plugin);
}

static void decimalNeg(decplugin *plugin, value *result, const value *op1) {
    decQuad output;
    clear_signal(plugin);
    if (decQuadIsZero(quad_value(op1))) decQuadZero(&output);
    else decQuadMinus(&output, quad_value(op1), arithmetic_context(plugin));
    decQuadCopy(ensure_capacity(plugin, result), &output);
    check_status(plugin);
}

static size_t comparison_digits(decplugin *plugin) {
    numeric_context *numeric = plugin->num_context;
    size_t digits = context_digits(plugin);
    if (numeric && numeric->fuzz > 0 && (size_t)numeric->fuzz < digits)
        digits -= (size_t)numeric->fuzz;
    return digits == 0u ? 1u : digits;
}

static int compare_quads(decplugin *plugin, const decQuad *left,
                         const decQuad *right) {
    decQuad rounded_left;
    decQuad rounded_right;
    decQuad comparison;
    size_t digits = comparison_digits(plugin);

    clear_signal(plugin);
    if (digits != context_digits(plugin)) {
        decQuadCopy(&rounded_left, left);
        decQuadCopy(&rounded_right, right);
        (void)round_quad(plugin, &rounded_left, digits);
        (void)round_quad(plugin, &rounded_right, digits);
        left = &rounded_left;
        right = &rounded_right;
    }
    decQuadCompareSignal(&comparison, left, right,
                         arithmetic_context(plugin));
    check_status(plugin);
    if (plugin->base.signal_number || decQuadIsNaN(&comparison)) return 0;
    if (decQuadIsZero(&comparison)) return 0;
    return decQuadIsNegative(&comparison) ? -1 : 1;
}

static int decimalCompare(decplugin *plugin, const value *op1,
                          const value *op2) {
    return compare_quads(plugin, quad_value(op1), quad_value(op2));
}

static int decimalCompareString(decplugin *plugin, const value *op1,
                                const char *op2) {
    decQuad right;
    clear_signal(plugin);
    decQuadFromString(&right, op2, arithmetic_context(plugin));
    if (!(arithmetic_context(plugin)->status & DEC_Errors))
        (void)round_quad(plugin, &right, context_digits(plugin));
    if (arithmetic_context(plugin)->status & DEC_Errors) {
        check_status(plugin);
        return 0;
    }
    return compare_quads(plugin, quad_value(op1), &right);
}

static int decimalIsZero(decplugin *plugin, const value *number) {
    (void)plugin;
    return decQuadIsZero(quad_value(number));
}

static void decimalTruncate(decplugin *plugin, value *result,
                            const value *op1) {
    decQuad output;
    clear_signal(plugin);
    decQuadToIntegralValue(&output, quad_value(op1),
                           arithmetic_context(plugin), DEC_ROUND_DOWN);
    decQuadCopy(ensure_capacity(plugin, result), &output);
    check_status(plugin);
}

static void decimalRound(decplugin *plugin, value *result,
                         const value *op1) {
    decQuad output;
    clear_signal(plugin);
    decQuadToIntegralValue(&output, quad_value(op1),
                           arithmetic_context(plugin),
                           arithmetic_context(plugin)->round);
    decQuadCopy(ensure_capacity(plugin, result), &output);
    check_status(plugin);
}

static void destroy_decplugin(rxvm_plugin *base) {
    free(base->private_context);
    free(base);
}

static rxvm_plugin *new_decplugin(void) {
    decquad_plugin_context *context =
        (decquad_plugin_context *)calloc(1u, sizeof(*context));
    decplugin *plugin = (decplugin *)calloc(1u, sizeof(*plugin));
    if (!context || !plugin) {
        free(context);
        free(plugin);
        RX_PANIC_OOM("allocate decQuad candidate plugin",
                     sizeof(*context) + sizeof(*plugin), "decimal plugin");
    }
    decContextDefault(&context->arithmetic, DEC_INIT_DECQUAD);
    context->arithmetic.traps = 0;

    plugin->base.type = RXVM_PLUGIN_DECIMAL;
    plugin->base.name = "decquad-candidate";
    plugin->base.version = "Plugin:0.1 Library:3.68-decQuad";
    plugin->base.description =
        "Non-default fixed-34 DECIMAL-01 comparator using decQuad";
    plugin->base.private_context = context;
    plugin->base.free = destroy_decplugin;
    plugin->syncNumericContext = syncNumericContext;
    plugin->getDigits = getDigits;
    plugin->getRequiredStringSize = getRequiredStringSize;
    plugin->decimalFromString = decimalFromString;
    plugin->decimalToString = decimalToString;
    plugin->decimalFromInt = decimalFromInt;
    plugin->decimalToInt = decimalToInt;
    plugin->decimalFromDouble = decimalFromDouble;
    plugin->decimalToDouble = decimalToDouble;
    plugin->decimalExtract = decimalExtract;
    plugin->decimalAdd = decimalAdd;
    plugin->decimalSub = decimalSub;
    plugin->decimalMul = decimalMul;
    plugin->decimalDiv = decimalDiv;
    plugin->decimalPow = decimalPow;
    plugin->decimalNeg = decimalNeg;
    plugin->decimalCompare = decimalCompare;
    plugin->decimalCompareString = decimalCompareString;
    plugin->decimalIsZero = decimalIsZero;
    plugin->decimalTruncate = decimalTruncate;
    plugin->decimalRound = decimalRound;
    syncNumericContext(plugin);
    return (rxvm_plugin *)plugin;
}

REGISTER_PLUGIN(new_decplugin)
