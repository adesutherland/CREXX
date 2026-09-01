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

/* Isolated DECIMAL-01 candidate adapter for libmpdec 4.0.1. */

#define RXVM_PLUGIN mpdnumber

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "rxvmplugin.h"
#include "mpdecimal.h"

typedef struct mpd_payload {
    uint8_t flags;
    mpd_ssize_t exp;
    mpd_ssize_t digits;
    mpd_ssize_t len;
    mpd_ssize_t alloc;
    mpd_uint_t data[];
} mpd_payload;

typedef struct mpd_plugin_context {
    mpd_context_t arithmetic;
    size_t operation_words;
    char signal[MPD_MAX_FLAG_STRING + 16];
} mpd_plugin_context;

static mpd_plugin_context *private_context(decplugin *plugin) {
    return (mpd_plugin_context *)plugin->base.private_context;
}

static mpd_context_t *arithmetic_context(decplugin *plugin) {
    return &private_context(plugin)->arithmetic;
}

static void clear_signal(decplugin *plugin) {
    plugin->base.signal_number = RXSIGNAL_NONE;
    plugin->base.signal_string = NULL;
}

static void check_status(decplugin *plugin, uint32_t status) {
    uint32_t errors = status & (MPD_Errors | MPD_Overflow | MPD_Underflow);
    mpd_plugin_context *state = private_context(plugin);

    clear_signal(plugin);
    if (!errors) return;

    if (errors & MPD_Malloc_error) {
        RX_PANIC_OOM("libmpdec candidate operation", 0u, "decimal value");
    }
    if (errors & (MPD_Conversion_syntax | MPD_Invalid_operation |
                  MPD_Division_impossible | MPD_Division_undefined |
                  MPD_Invalid_context | MPD_Fpu_error)) {
        plugin->base.signal_number = RXSIGNAL_CONVERSION_ERROR;
    }
    else if (errors & MPD_Division_by_zero) {
        plugin->base.signal_number = RXSIGNAL_DIVISION_BY_ZERO;
    }
    else if (errors & (MPD_Overflow | MPD_Underflow)) {
        plugin->base.signal_number = RXSIGNAL_OVERFLOW_UNDERFLOW;
    }
    else {
        plugin->base.signal_number = RXSIGNAL_ERROR;
    }

    strcpy(state->signal, "libmpdec: ");
    (void)mpd_snprint_flags(state->signal + strlen(state->signal),
                            MPD_MAX_FLAG_STRING, errors);
    plugin->base.signal_string = state->signal;
}

static size_t words_for_digits(size_t digits) {
    size_t words = (digits + (size_t)MPD_RDIGITS - 1u) /
                   (size_t)MPD_RDIGITS;
    if (words < (size_t)MPD_MINALLOC) words = (size_t)MPD_MINALLOC;
    return words;
}

static size_t payload_size(size_t words) {
    if (words > (SIZE_MAX - offsetof(mpd_payload, data)) /
                sizeof(mpd_uint_t)) {
        RX_PANIC_OOM("size libmpdec candidate payload", words,
                     "decimal coefficient words");
    }
    return offsetof(mpd_payload, data) + words * sizeof(mpd_uint_t);
}

static mpd_payload *ensure_words(decplugin *plugin, value *number,
                                 size_t words) {
    size_t old_length = rxvm_value_decimal_length(number);
    size_t size;
    mpd_payload *payload;

    if (words < (size_t)MPD_MINALLOC) words = (size_t)MPD_MINALLOC;
    size = payload_size(words);
    payload = (mpd_payload *)plugin->reserve_decimal(number, size);
    if (!payload) {
        RX_PANIC_OOM("reserve libmpdec candidate sidecar payload", size,
                     "decimal value");
    }
    if (old_length == 0u) {
        memset(payload, 0, size);
        payload->digits = 1;
        payload->len = 1;
    }
    payload->alloc = (mpd_ssize_t)words;
    return payload;
}

static mpd_t view_value(const value *number) {
    const mpd_payload *payload = (const mpd_payload *)number->decimal_value;
    mpd_t view;

    view.flags = (uint8_t)(payload->flags | MPD_STATIC | MPD_STATIC_DATA);
    view.exp = payload->exp;
    view.digits = payload->digits;
    view.len = payload->len;
    view.alloc = payload->alloc;
    view.data = (mpd_uint_t *)payload->data;
    return view;
}

static mpd_t view_result(value *number) {
    return view_value(number);
}

static void commit_view(decplugin *plugin, value *number, mpd_t *view) {
    mpd_payload *payload = (mpd_payload *)number->decimal_value;
    mpd_uint_t *dynamic_data = NULL;
    mpd_ssize_t len = view->len;
    uint8_t flags = (uint8_t)(view->flags & ~(MPD_STATIC | MPD_DATAFLAGS));
    mpd_ssize_t exp = view->exp;
    mpd_ssize_t digits = view->digits;

    if (view->data != payload->data) {
        dynamic_data = view->data;
        payload = ensure_words(plugin, number, (size_t)view->alloc);
        if (len > 0) {
            memcpy(payload->data, dynamic_data,
                   (size_t)len * sizeof(mpd_uint_t));
        }
        mpd_free(dynamic_data);
    }
    payload->flags = flags;
    payload->exp = exp;
    payload->digits = digits;
    payload->len = len;
}

static size_t operation_words(decplugin *plugin) {
    return private_context(plugin)->operation_words;
}

static void syncNumericContext(decplugin *plugin) {
    static numeric_context defaults = {
        DEFAULT_NUMERIC_DIGITS, DEFAULT_NUMERIC_FUZZ, DEFAULT_NUMERIC_FORM,
        DEFAULT_NUMERIC_CASE, NUMERIC_STANDARD_COMMON
    };
    numeric_context *numeric = plugin->num_context ? plugin->num_context
                                                   : &defaults;
    mpd_context_t *context = arithmetic_context(plugin);

    context->prec = (mpd_ssize_t)numeric->digits;
    context->emax = 999999999;
    context->emin = -999999999;
    context->traps = 0;
    context->status = 0;
    context->newtrap = 0;
    private_context(plugin)->operation_words =
        words_for_digits((size_t)numeric->digits * 2u + 2u);
    if (numeric->standard == NUMERIC_STANDARD_COMMON) {
        context->clamp = MPD_CLAMP_IEEE_754;
        context->round = MPD_ROUND_HALF_EVEN;
    }
    else {
        context->clamp = MPD_CLAMP_DEFAULT;
        context->round = MPD_ROUND_HALF_UP;
    }
    clear_signal(plugin);
}

static size_t getDigits(decplugin *plugin) {
    return (size_t)arithmetic_context(plugin)->prec;
}

static size_t getRequiredStringSize(decplugin *plugin, const value *number) {
    (void)number;
    return getDigits(plugin) + 32u;
}

static void decimalFromString(decplugin *plugin, value *result,
                              const char *string) {
    size_t input_words = words_for_digits(strlen(string) + 1u);
    size_t context_words = operation_words(plugin);
    uint32_t status = 0;
    mpd_t output;

    if (context_words > input_words) input_words = context_words;
    (void)ensure_words(plugin, result, input_words);
    output = view_result(result);
    mpd_qset_string(&output, string, arithmetic_context(plugin), &status);
    commit_view(plugin, result, &output);
    check_status(plugin, status);
}

static void decimalExtract(decplugin *plugin, char *coefficient,
                           rxinteger *exponent, value *decimal) {
    mpd_t input = view_value(decimal);
    char *text;
    char *out = coefficient;
    const char *scan;
    const char *end;
    size_t digits = 0u;
    size_t leading = 0u;
    int negative;

    (void)plugin;
    *exponent = 0;
    if (mpd_isnan(&input)) {
        strcpy(coefficient, "nan");
        return;
    }
    if (mpd_isinfinite(&input)) {
        strcpy(coefficient, mpd_isnegative(&input) ? "-inf" : "inf");
        return;
    }
    if (mpd_iszero(&input)) {
        strcpy(coefficient, "0");
        return;
    }

    text = mpd_to_sci(&input, 0);
    if (!text) RX_PANIC_OOM("format libmpdec candidate coefficient", 0u,
                            "decimal value");
    negative = *text == '-';
    scan = text + (negative ? 1 : 0);
    end = strchr(scan, 'e');
    if (!end) end = strchr(scan, 'E');
    if (!end) end = scan + strlen(scan);

    if (negative) *out++ = '-';
    while (scan < end) {
        if (isdigit((unsigned char)*scan)) out[digits++] = *scan;
        scan++;
    }
    while (leading + 1u < digits && out[leading] == '0') leading++;
    if (leading) {
        memmove(out, out + leading, digits - leading);
        digits -= leading;
    }
    while (digits > 1u && out[digits - 1u] == '0') digits--;
    if (digits > 1u) {
        memmove(out + 2, out + 1, digits - 1u);
        out[1] = '.';
        out[digits + 1u] = '\0';
    }
    else {
        out[digits] = '\0';
    }
    *exponent = (rxinteger)mpd_adjexp(&input);
    mpd_free(text);
}

static void format_components(decplugin *plugin, const char *coefficient,
                              rxinteger exponent, mpd_ssize_t raw_exponent,
                              char *output) {
    static numeric_context defaults = {
        DEFAULT_NUMERIC_DIGITS, DEFAULT_NUMERIC_FUZZ, DEFAULT_NUMERIC_FORM,
        DEFAULT_NUMERIC_CASE, NUMERIC_STANDARD_COMMON
    };
    numeric_context *numeric = plugin->num_context ? plugin->num_context
                                                   : &defaults;
    const char *digits = coefficient;
    char plain[256];
    char *compact = plain;
    size_t coefficient_length = strlen(coefficient);
    size_t digit_count = 0u;
    size_t i;
    int negative = 0;
    int use_exponential;
    int upper = numeric->casetype == CASE_UPPER;
    rxinteger coefficient_exponent;

    if (!isdigit((unsigned char)coefficient[0]) && coefficient[0] != '-') {
        strcpy(output, coefficient);
        for (i = 0; output[i]; ++i) {
            output[i] = (char)(upper ? toupper((unsigned char)output[i])
                                    : tolower((unsigned char)output[i]));
        }
        return;
    }
    if (coefficient[0] == '-' &&
        !isdigit((unsigned char)coefficient[1])) {
        strcpy(output, coefficient);
        for (i = 1; output[i]; ++i) {
            output[i] = (char)(upper ? toupper((unsigned char)output[i])
                                    : tolower((unsigned char)output[i]));
        }
        return;
    }
    if (coefficient[0] == '-') {
        negative = 1;
        digits++;
    }
    if (coefficient_length + (size_t)(exponent < 0 ? -exponent : exponent) +
        8u > sizeof(plain)) {
        compact = (char *)malloc(coefficient_length + 1u);
        if (!compact) RX_PANIC_OOM("format libmpdec candidate digits",
                                   coefficient_length + 1u, coefficient);
    }
    for (i = 0; digits[i]; ++i) {
        if (digits[i] != '.') compact[digit_count++] = digits[i];
    }
    compact[digit_count] = '\0';

    /* Match mc_decimal's existing formatting boundary. After normalization,
     * decNumber makes this decision from its coefficient exponent rather
     * than the adjusted exponent returned by decimalExtract. */
    coefficient_exponent = exponent - (rxinteger)digit_count + 1;
    use_exponential = !(
        raw_exponent == 0 ||
        (coefficient_exponent >= -5 &&
         coefficient_exponent <= (rxinteger)numeric->digits - 1) ||
        (coefficient_exponent <= 0 && exponent + 1 >= -5));
    if (use_exponential && numeric->form == NUMERIC_FORM_SCIENTIFIC) {
        sprintf(output, "%s%s%c%+lld", negative ? "-" : "", coefficient +
                (negative ? 1 : 0), upper ? 'E' : 'e', (long long)exponent);
    }
    else if (use_exponential) {
        int remainder = (int)(exponent % 3);
        int shift = (remainder + 3) % 3;
        rxinteger engineering_exponent = exponent - shift;
        size_t integer_digits = (size_t)shift + 1u;
        char *cursor = output;
        if (negative) *cursor++ = '-';
        for (i = 0; i < integer_digits; ++i)
            *cursor++ = i < digit_count ? compact[i] : '0';
        if (digit_count > integer_digits) {
            *cursor++ = '.';
            memcpy(cursor, compact + integer_digits,
                   digit_count - integer_digits);
            cursor += digit_count - integer_digits;
        }
        sprintf(cursor, "%c%+lld", upper ? 'E' : 'e',
                (long long)engineering_exponent);
    }
    else {
        char *cursor = output;
        long long point = (long long)exponent + 1;
        if (negative) *cursor++ = '-';
        if (point <= 0) {
            *cursor++ = '0';
            *cursor++ = '.';
            for (i = 0; i < (size_t)(-point); ++i) *cursor++ = '0';
            memcpy(cursor, compact, digit_count);
            cursor += digit_count;
        }
        else if ((size_t)point >= digit_count) {
            memcpy(cursor, compact, digit_count);
            cursor += digit_count;
            for (i = digit_count; i < (size_t)point; ++i) *cursor++ = '0';
        }
        else {
            memcpy(cursor, compact, (size_t)point);
            cursor += point;
            *cursor++ = '.';
            memcpy(cursor, compact + point, digit_count - (size_t)point);
            cursor += digit_count - (size_t)point;
        }
        *cursor = '\0';
    }
    if (compact != plain) free(compact);
}

static void decimalToString(decplugin *plugin, const value *number,
                            char *string) {
    char *coefficient;
    rxinteger exponent;
    mpd_t input;

    clear_signal(plugin);
    if (!number->decimal_value || rxvm_value_decimal_length(number) == 0u) {
        strcpy(string, "nan");
        return;
    }
    input = view_value(number);
    if (mpd_iszero(&input)) {
        strcpy(string, mpd_isnegative(&input) ? "-0" : "0");
        return;
    }
    coefficient = (char *)malloc(getRequiredStringSize(plugin, number));
    if (!coefficient) RX_PANIC_OOM("format libmpdec candidate value",
                                   getRequiredStringSize(plugin, number),
                                   "decimal value");
    decimalExtract(plugin, coefficient, &exponent, (value *)number);
    format_components(plugin, coefficient, exponent, input.exp, string);
    free(coefficient);
    clear_signal(plugin);
}

static void decimalFromInt(decplugin *plugin, value *result, rxinteger input) {
    uint32_t status = 0;
    mpd_t output;
    (void)ensure_words(plugin, result, operation_words(plugin));
    output = view_result(result);
    mpd_qset_i64(&output, (int64_t)input, arithmetic_context(plugin), &status);
    commit_view(plugin, result, &output);
    check_status(plugin, status);
}

static void decimalToInt(decplugin *plugin, const value *number,
                         rxinteger *integer) {
    uint32_t status = 0;
    mpd_t input = view_value(number);
    *integer = (rxinteger)mpd_qget_i64(&input, &status);
    check_status(plugin, status);
}

static void decimalFromDouble(decplugin *plugin, value *result, double input) {
    char buffer[32];
    if (isnan(input)) strcpy(buffer, "nan");
    else if (isinf(input)) strcpy(buffer, signbit(input) ? "-inf" : "inf");
    else if (input == 0.0 && signbit(input)) strcpy(buffer, "-0");
    else snprintf(buffer, sizeof(buffer), "%.16g", input);
    decimalFromString(plugin, result, buffer);
}

static void decimalToDouble(decplugin *plugin, const value *number,
                            double *result) {
    mpd_t input = view_value(number);
    char *text;
    clear_signal(plugin);
    if (mpd_isnan(&input)) { *result = NAN; return; }
    if (mpd_isinfinite(&input)) {
        *result = mpd_isnegative(&input) ? -INFINITY : INFINITY;
        return;
    }
    if (mpd_iszero(&input)) {
        *result = mpd_isnegative(&input) ? -0.0 : 0.0;
        return;
    }
    text = mpd_to_sci(&input, 0);
    if (!text) RX_PANIC_OOM("convert libmpdec candidate to double", 0u,
                            "decimal value");
    *result = strtod(text, NULL);
    mpd_free(text);
}

typedef void (*binary_operation)(mpd_t *, const mpd_t *, const mpd_t *,
                                 const mpd_context_t *, uint32_t *);

static void apply_binary(decplugin *plugin, value *result, const value *op1,
                         const value *op2, binary_operation operation) {
    uint32_t status = 0;
    mpd_t left;
    mpd_t right;
    mpd_t output;
    (void)ensure_words(plugin, result, operation_words(plugin));
    left = view_value(op1);
    right = view_value(op2);
    output = view_result(result);
    operation(&output, &left, &right, arithmetic_context(plugin), &status);
    commit_view(plugin, result, &output);
    check_status(plugin, status);
}

static void decimalAdd(decplugin *plugin, value *result, const value *op1,
                       const value *op2) {
    apply_binary(plugin, result, op1, op2, mpd_qadd);
}

static void decimalSub(decplugin *plugin, value *result, const value *op1,
                       const value *op2) {
    apply_binary(plugin, result, op1, op2, mpd_qsub);
}

static void decimalMul(decplugin *plugin, value *result, const value *op1,
                       const value *op2) {
    apply_binary(plugin, result, op1, op2, mpd_qmul);
}

static void decimalDiv(decplugin *plugin, value *result, const value *op1,
                       const value *op2) {
    apply_binary(plugin, result, op1, op2, mpd_qdiv);
}

static void decimalPow(decplugin *plugin, value *result, const value *op1,
                       const value *op2) {
    apply_binary(plugin, result, op1, op2, mpd_qpow);
}

static void decimalNeg(decplugin *plugin, value *result, const value *op1) {
    uint32_t status = 0;
    mpd_t input;
    mpd_t output;
    (void)ensure_words(plugin, result, operation_words(plugin));
    input = view_value(op1);
    output = view_result(result);
    mpd_qminus(&output, &input, arithmetic_context(plugin), &status);
    commit_view(plugin, result, &output);
    check_status(plugin, status);
}

static size_t comparison_digits(decplugin *plugin) {
    numeric_context *numeric = plugin->num_context;
    size_t digits = getDigits(plugin);
    if (numeric && numeric->fuzz > 0 &&
        (size_t)numeric->fuzz < digits) digits -= (size_t)numeric->fuzz;
    if (digits == 0u) digits = 1u;
    return digits;
}

static int compare_views(decplugin *plugin, const mpd_t *left,
                         const mpd_t *right) {
    uint32_t status = 0;
    size_t digits = comparison_digits(plugin);
    mpd_context_t compare_context = *arithmetic_context(plugin);
    int result;

    if (digits == getDigits(plugin)) {
        result = mpd_qcmp(left, right, &status);
    }
    else {
        mpd_t *rounded_left = mpd_qnew_size((mpd_ssize_t)words_for_digits(digits));
        mpd_t *rounded_right = mpd_qnew_size((mpd_ssize_t)words_for_digits(digits));
        if (!rounded_left || !rounded_right) {
            if (rounded_left) mpd_del(rounded_left);
            if (rounded_right) mpd_del(rounded_right);
            RX_PANIC_OOM("allocate libmpdec comparison values", digits,
                         "decimal comparison");
        }
        compare_context.prec = (mpd_ssize_t)digits;
        mpd_qplus(rounded_left, left, &compare_context, &status);
        mpd_qplus(rounded_right, right, &compare_context, &status);
        result = mpd_qcmp(rounded_left, rounded_right, &status);
        mpd_del(rounded_left);
        mpd_del(rounded_right);
    }
    check_status(plugin, status);
    return result == INT_MAX ? 0 : result;
}

static int decimalCompare(decplugin *plugin, const value *op1,
                          const value *op2) {
    mpd_t left = view_value(op1);
    mpd_t right = view_value(op2);
    return compare_views(plugin, &left, &right);
}

static int decimalCompareString(decplugin *plugin, const value *op1,
                                const char *op2) {
    uint32_t status = 0;
    mpd_t left = view_value(op1);
    mpd_t *right = mpd_qnew_size((mpd_ssize_t)words_for_digits(strlen(op2) + 1u));
    int result;
    if (!right) RX_PANIC_OOM("allocate libmpdec string comparison",
                             strlen(op2) + 1u, op2);
    mpd_qset_string(right, op2, arithmetic_context(plugin), &status);
    if (status & (MPD_Errors | MPD_Overflow | MPD_Underflow)) {
        check_status(plugin, status);
        mpd_del(right);
        return 0;
    }
    result = compare_views(plugin, &left, right);
    mpd_del(right);
    return result;
}

static int decimalIsZero(decplugin *plugin, const value *number) {
    mpd_t input = view_value(number);
    (void)plugin;
    return mpd_iszero(&input);
}

typedef void (*unary_operation)(mpd_t *, const mpd_t *,
                                const mpd_context_t *, uint32_t *);

static void apply_unary(decplugin *plugin, value *result, const value *op1,
                        unary_operation operation) {
    uint32_t status = 0;
    mpd_t input;
    mpd_t output;
    (void)ensure_words(plugin, result, operation_words(plugin));
    input = view_value(op1);
    output = view_result(result);
    operation(&output, &input, arithmetic_context(plugin), &status);
    commit_view(plugin, result, &output);
    check_status(plugin, status);
}

static void decimalTruncate(decplugin *plugin, value *result,
                            const value *op1) {
    apply_unary(plugin, result, op1, mpd_qtrunc);
}

static void decimalRound(decplugin *plugin, value *result, const value *op1) {
    apply_unary(plugin, result, op1, mpd_qround_to_intx);
}

static void destroy_decplugin(rxvm_plugin *base) {
    free(base->private_context);
    free(base);
}

static rxvm_plugin *new_decplugin(void) {
    mpd_plugin_context *context =
        (mpd_plugin_context *)calloc(1u, sizeof(*context));
    decplugin *plugin = (decplugin *)calloc(1u, sizeof(*plugin));
    if (!context || !plugin) {
        free(context);
        free(plugin);
        RX_PANIC_OOM("allocate libmpdec candidate plugin", sizeof(*context) +
                     sizeof(*plugin), "decimal plugin");
    }
    mpd_defaultcontext(&context->arithmetic);
    context->arithmetic.traps = 0;

    plugin->base.type = RXVM_PLUGIN_DECIMAL;
    plugin->base.name = "mpddecimal-candidate";
    plugin->base.version = "Plugin:0.1 Library:4.0.1";
    plugin->base.description =
        "Non-default DECIMAL-01 candidate using libmpdec";
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
