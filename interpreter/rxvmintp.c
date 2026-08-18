/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
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

/*
 * Created by adrian on 29/03/2021.
 */

/*
 * ===================================================================
 * LEVEL C COMPATIBILITY & DESIGN NOTES (StrictAnsiArithmetic Flag)
 * ===================================================================
 * This section documents the arithmetic behaviour that must change when
 * the `StrictAnsiArithmetic` flag is enabled for Level C compatibility.
 *
 * -------------------------------------------------------------------
 * 1. Decimal Arithmetic (% and // operators on `.decimal` type)
 * -------------------------------------------------------------------
 * For Level C, the ANSI "Integer Magnitude-Precision Constraint"
 * must be enforced.
 *
 * CHECK: After calculating the intermediate division result, but
 * before finalizing it, the following check is required:
 *
 * IF LENGTH(TRUNC(intermediate_result)) > DIGITS() THEN
 * RAISE SYNTAX Error 26.11
 *
 * -------------------------------------------------------------------
 * 2. Float Arithmetic (% and // operators on `.float` and `.decimal` types)
 * -------------------------------------------------------------------
 * The current design implements a "Float-First" model:
 *
 * 1. Perform the division using native floating-point arithmetic.
 * 2. Truncate the final result to its integer part.
 *
 * This was chosen over an "Integer-First" model where operands
 * would be truncated to integers *before* the division.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxsignature.h"
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <fcntl.h>
#endif
#ifdef __APPLE__
#include <mach/thread_policy.h>
#include <dlfcn.h>
#endif
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include "platform.h"
#include "rxas.h"
#include "../binutils/include/rxdefs.h"
#include "../binutils/include/rxflags.h"
#include "../binutils/include/rxjtable.h"
#include "rxastree.h"
#include "rxvmintp.h"
#include "rxvmchannel.h"
/* #include <complex.h> */
#include <signal.h>


#include "rxvmvars.h"
#include "rxvmstem.h"
#include "rxvmplugin_framework.h"
#include "rxvmsock.h"

#ifdef _MSC_VER
typedef struct rxvm_timeval {
    time_t tv_sec;
    long tv_usec;
} rxvm_timeval;

static void rxvm_get_timeval(rxvm_timeval *time_value) {
    FILETIME file_time;
    ULARGE_INTEGER ticks;
    uint64_t unix_ticks;

    GetSystemTimePreciseAsFileTime(&file_time);
    ticks.LowPart = file_time.dwLowDateTime;
    ticks.HighPart = file_time.dwHighDateTime;
    unix_ticks = ticks.QuadPart - UINT64_C(116444736000000000);
    time_value->tv_sec = (time_t) (unix_ticks / UINT64_C(10000000));
    time_value->tv_usec = (long) ((unix_ticks % UINT64_C(10000000)) / UINT64_C(10));
}

static long rxvm_timezone_seconds(void) {
    long timezone_seconds = 0;
    _tzset();
    (void) _get_timezone(&timezone_seconds);
    return timezone_seconds;
}
#define RXVM_TZNAME _tzname
#else
typedef struct timeval rxvm_timeval;

static void rxvm_get_timeval(rxvm_timeval *time_value) {
    (void) gettimeofday(time_value, NULL);
}

static long rxvm_timezone_seconds(void) {
    tzset();
    return timezone;
}
#define RXVM_TZNAME tzname
#endif

typedef char rxvm_execution_slot_must_hold_pointer[
        sizeof(bin_code) >= sizeof(void *) ? 1 : -1];

#if defined(__has_attribute)
#  if __has_attribute(noinline)
#    define RXVM_LABEL_OWNER_NOINLINE __attribute__((noinline))
#  else
#    define RXVM_LABEL_OWNER_NOINLINE
#  endif
#  if __has_attribute(noclone)
#    define RXVM_LABEL_OWNER_NOCLONE __attribute__((noclone))
#  else
#    define RXVM_LABEL_OWNER_NOCLONE
#  endif
#elif defined(__GNUC__)
#  define RXVM_LABEL_OWNER_NOINLINE __attribute__((noinline))
#  define RXVM_LABEL_OWNER_NOCLONE __attribute__((noclone))
#else
#  define RXVM_LABEL_OWNER_NOINLINE
#  define RXVM_LABEL_OWNER_NOCLONE
#endif

#define RXVM_LABEL_OWNER RXVM_LABEL_OWNER_NOINLINE RXVM_LABEL_OWNER_NOCLONE

#if defined(_MSC_VER)
#  define RXVM_HELPER_NOINLINE __declspec(noinline)
#  define RXVM_HELPER_COLD
#else
#  define RXVM_HELPER_NOINLINE RXVM_LABEL_OWNER_NOINLINE
#  if defined(__has_attribute)
#    if __has_attribute(cold)
#      define RXVM_HELPER_COLD __attribute__((cold))
#    else
#      define RXVM_HELPER_COLD
#    endif
#  elif defined(__GNUC__)
#    define RXVM_HELPER_COLD __attribute__((cold))
#  else
#    define RXVM_HELPER_COLD
#  endif
#endif

#if CREXX_VM_HANDLER_PANEL == 0 || \
        (defined(__GNUC__) && !defined(__clang__) && !defined(_MSC_VER))
#define RXVM_HANDLER_USE_POINTER_FACADE 1
#else
#define RXVM_HANDLER_USE_POINTER_FACADE 0
#endif

#include "rxvmhandlerpolicy.h"

int rxvm_link(rxvm_context *ctx);

#define RXVM_FFORMAT_MAX_FIELD 1000

typedef struct rxvm_parse_span {
    size_t start;
    size_t length;
} rxvm_parse_span;

static void rxvm_parse_words3_spans(const value *source,
                                    rxvm_parse_span spans[3]) {
    const char *text = source->string_value;
    size_t length = source->string_length;
    size_t cursor = 0;
    size_t field;

    for (field = 0; field < 2; field++) {
        while (cursor < length && text[cursor] == ' ') cursor++;
        spans[field].start = cursor;
        while (cursor < length && text[cursor] != ' ') cursor++;
        spans[field].length = cursor - spans[field].start;
        if (cursor < length) cursor++;
    }
    spans[2].start = cursor;
    spans[2].length = length - cursor;
}

static void rxvm_parse_words3_drop_spans(const value *source,
                                         rxvm_parse_span spans[3]) {
    const char *text = source->string_value;
    size_t length = source->string_length;
    size_t cursor = 0;
    size_t field;

    for (field = 0; field < 3; field++) {
        while (cursor < length && text[cursor] == ' ') cursor++;
        spans[field].start = cursor;
        while (cursor < length && text[cursor] != ' ') cursor++;
        spans[field].length = cursor - spans[field].start;
        if (cursor < length) cursor++;
    }
}

static void rxvm_parse_set_span_bytes(value *target,
                                      const char *source,
                                      const rxvm_parse_span *span) {
    set_string(target, (char *)source + span->start, span->length);
}

static size_t rxvm_parse_byte_offset(const value *source, size_t character_offset) {
#ifndef NUTF8
    size_t byte_offset = 0;
    size_t current_character = 0;

    if (character_offset >= source->string_chars) return source->string_length;
#if ASCII_FAST_PATH
    if (source->string_chars == source->string_length) return character_offset;
#endif
    while (current_character < character_offset && byte_offset < source->string_length) {
        byte_offset += utf8codepointcalcsize(source->string_value + byte_offset);
        current_character++;
    }
    return byte_offset;
#else
    return character_offset < source->string_length ? character_offset : source->string_length;
#endif
}

static void rxvm_parse_position2_spans(const value *source,
                                       rxinteger split,
                                       rxvm_parse_span spans[2]) {
    const char *text = source->string_value;
    size_t length = source->string_length;
    size_t character_offset = split > 0 ? (size_t)split : 0;
    size_t cursor = rxvm_parse_byte_offset(source, character_offset);

    spans[0].start = 0;
    spans[0].length = cursor;
    while (cursor < length && text[cursor] == ' ') cursor++;
    spans[1].start = cursor;
    while (cursor < length && text[cursor] != ' ') cursor++;
    spans[1].length = cursor - spans[1].start;
}

static int rxvm_padded_string_compare(const char *left, size_t left_len, const char *right, size_t right_len) {
    size_t max_len = left_len > right_len ? left_len : right_len;
    size_t i;

    for (i = 0; i < max_len; i++) {
        unsigned char left_ch = i < left_len ? (unsigned char) left[i] : (unsigned char) ' ';
        unsigned char right_ch = i < right_len ? (unsigned char) right[i] : (unsigned char) ' ';
        if (left_ch != right_ch) return left_ch > right_ch ? 1 : -1;
    }

    return 0;
}

static int rxvm_trimmed_space_equal(const char *left, size_t left_len, const char *right, size_t right_len) {
    while (left_len > 0 && left[left_len - 1] == ' ') left_len--;
    while (right_len > 0 && right[right_len - 1] == ' ') right_len--;
    while (left_len > 0 && *left == ' ') {
        left++;
        left_len--;
    }
    while (right_len > 0 && *right == ' ') {
        right++;
        right_len--;
    }
    return left_len == right_len && memcmp(left, right, left_len) == 0;
}

/*
 * Reject only a leading byte that cannot begin a strtod subject in the active
 * locale. Numeric and uncertain spans retain the exact current converter.
 * Keep this helper out of line so the loose comparator remains inlineable.
 */
static RXVM_HELPER_NOINLINE int rxvm_loose_string2float(
        double *out, const char *text, size_t length) {
    unsigned char first;
    struct lconv *numeric_locale;

    if (length == 0) return 1;
    first = (unsigned char) text[0];
    if (first >= 0x80 || isspace(first) ||
        (first >= '0' && first <= '9') || first == '+' || first == '-' ||
        first == '.' || first == ',' || first == 'i' || first == 'I' ||
        first == 'n' || first == 'N') {
        return string2float(out, (char *) text, length);
    }

    numeric_locale = localeconv();
    if (numeric_locale && numeric_locale->decimal_point &&
        numeric_locale->decimal_point[0] != '\0' &&
        first == (unsigned char) numeric_locale->decimal_point[0]) {
        return string2float(out, (char *) text, length);
    }
    return 1;
}

static int rxvm_loose_compare_text(const char *left, size_t left_len, const char *right, size_t right_len) {
    double left_number;
    double right_number;
    int left_numeric = rxvm_loose_string2float(&left_number, left, left_len) == 0;
    int right_numeric = rxvm_loose_string2float(&right_number, right, right_len) == 0;

    if (left_numeric && right_numeric) {
        if (left_number > right_number) return 1;
        if (left_number < right_number) return -1;
        return 0;
    }

    return rxvm_padded_string_compare(left, left_len, right, right_len);
}

static int rxvm_loose_compare_values(value *left, value *right) {
    return rxvm_loose_compare_text(left->string_value, left->string_length,
                                   right->string_value, right->string_length);
}

static int rxvm_loose_compare_value_const(value *left, string_constant *right) {
    return rxvm_loose_compare_text(left->string_value, left->string_length,
                                   right->string, right->string_len);
}

static int rxvm_loose_compare_const_value(string_constant *left, value *right) {
    return rxvm_loose_compare_text(left->string, left->string_len,
                                   right->string_value, right->string_length);
}

static char *rxvm_format_buffer_at(char *buffer, size_t buffer_len, size_t used) {
    if (buffer_len == 0) return buffer;
    return buffer + (used < buffer_len ? used : buffer_len - 1);
}

static size_t rxvm_format_buffer_remaining(size_t buffer_len, size_t used) {
    return used < buffer_len ? buffer_len - used : 0;
}

static size_t rxvm_format_append_literal(char *buffer, size_t buffer_len, size_t used, const char *text, size_t text_len) {
    if (buffer_len > 0) {
        size_t offset = used < buffer_len ? used : buffer_len - 1;
        size_t writable = buffer_len - offset;
        if (writable > 0) {
            size_t copy_len;
            writable--;
            copy_len = text_len < writable ? text_len : writable;
            if (copy_len > 0) memcpy(buffer + offset, text, copy_len);
            buffer[offset + copy_len] = 0;
        }
    }
    return used + text_len;
}

static int rxvm_checked_size_add(size_t left, size_t right, size_t *result) {
    if (left > (size_t)-1 - right) return 0;
    *result = left + right;
    return 1;
}

static int rxvm_checked_size_mul(size_t left, size_t right, size_t *result) {
    if (left != 0 && right > (size_t)-1 / left) return 0;
    *result = left * right;
    return 1;
}

static int rxvm_memory_range(size_t length, rxinteger offset_value, size_t width, size_t *offset) {
    size_t local_offset;

    if (offset_value < 0) return 0;
    if ((uintmax_t)offset_value > (uintmax_t)SIZE_MAX) return 0;

    local_offset = (size_t)offset_value;
    if (local_offset > length) return 0;
    if (width > length - local_offset) return 0;

    if (offset) *offset = local_offset;
    return 1;
}

static int rxvm_binary_range(value *buffer, rxinteger offset_value, size_t width, size_t *offset) {
    return rxvm_memory_range(buffer->binary_length, offset_value, width, offset);
}

static int rxvm_rxinteger_to_size(rxinteger value, size_t *result) {
    if (value < 0) return 0;
    if ((uintmax_t)value > (uintmax_t)SIZE_MAX) return 0;
    if (result) *result = (size_t)value;
    return 1;
}

static uint16_t rxvm_bswap16(uint16_t value) {
    return (uint16_t)((value >> 8) | (value << 8));
}

static uint32_t rxvm_bswap32(uint32_t value) {
    return ((value & UINT32_C(0x000000ff)) << 24) |
           ((value & UINT32_C(0x0000ff00)) << 8) |
           ((value & UINT32_C(0x00ff0000)) >> 8) |
           ((value & UINT32_C(0xff000000)) >> 24);
}

static uint64_t rxvm_bswap64(uint64_t value) {
    return ((value & UINT64_C(0x00000000000000ff)) << 56) |
           ((value & UINT64_C(0x000000000000ff00)) << 40) |
           ((value & UINT64_C(0x0000000000ff0000)) << 24) |
           ((value & UINT64_C(0x00000000ff000000)) << 8) |
           ((value & UINT64_C(0x000000ff00000000)) >> 8) |
           ((value & UINT64_C(0x0000ff0000000000)) >> 24) |
           ((value & UINT64_C(0x00ff000000000000)) >> 40) |
           ((value & UINT64_C(0xff00000000000000)) >> 56);
}

#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || \
    (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define RXVM_HOST_LITTLE_ENDIAN 1
#elif defined(__BIG_ENDIAN__) || \
      (defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define RXVM_HOST_BIG_ENDIAN 1
#endif

static uint64_t rxvm_binary_read_le_bytes(const unsigned char *bytes, size_t offset, size_t width) {
    const unsigned char *source = bytes + offset;
    uint64_t result = 0;
    size_t i;

    if (width == 1) return source[0];
#if defined(RXVM_HOST_LITTLE_ENDIAN)
    if (width == 2) {
        uint16_t value;
        memcpy(&value, source, sizeof(value));
        return value;
    }
    if (width == 4) {
        uint32_t value;
        memcpy(&value, source, sizeof(value));
        return value;
    }
    if (width == 8) {
        uint64_t value;
        memcpy(&value, source, sizeof(value));
        return value;
    }
#elif defined(RXVM_HOST_BIG_ENDIAN)
    if (width == 2) {
        uint16_t value;
        memcpy(&value, source, sizeof(value));
        return rxvm_bswap16(value);
    }
    if (width == 4) {
        uint32_t value;
        memcpy(&value, source, sizeof(value));
        return rxvm_bswap32(value);
    }
    if (width == 8) {
        uint64_t value;
        memcpy(&value, source, sizeof(value));
        return rxvm_bswap64(value);
    }
#endif

    for (i = 0; i < width; i++) {
        result |= ((uint64_t)source[i]) << (i * 8);
    }
    return result;
}

#define RXVM_PARSE_PLAN_MAGIC 0x50u
#define RXVM_PARSE_PLAN_VERSION_FROZEN 1u
#define RXVM_PARSE_PLAN_VERSION_DYNAMIC 2u
#define RXVM_PARSE_PLAN_HEADER_SIZE_FROZEN 8u
#define RXVM_PARSE_PLAN_HEADER_SIZE_DYNAMIC 12u
#define RXVM_PARSE_PLAN_FLAG_STORE 0x01u
#define RXVM_PARSE_PLAN_FLAG_SKIP 0x01u
#define RXVM_PARSE_PLAN_FLAG_CAPTURE 0x01u

#define RXVM_PARSE_PLAN_INVALID 0
#define RXVM_PARSE_PLAN_OK 1
#define RXVM_PARSE_PLAN_CONVERSION (-1)

typedef struct rxvm_parse_plan_item {
    unsigned char kind;
    unsigned char flags;
    size_t offset;
    size_t next_offset;
    const unsigned char *literal;
    size_t literal_bytes;
    size_t literal_chars;
    rxinteger movement;
    uint16_t operand_index;
} rxvm_parse_plan_item;

static int rxvm_parse_plan_header(const string_constant *plan,
                                  unsigned char *version,
                                  size_t *header_size,
                                  uint16_t *item_count,
                                  uint16_t *result_count,
                                  uint16_t *dynamic_count) {
    const unsigned char *bytes;
    uint16_t encoded_header_size;

    if (!plan || plan->string_len < RXVM_PARSE_PLAN_HEADER_SIZE_FROZEN) return 0;
    bytes = (const unsigned char *)plan->string;
    if (bytes[0] != RXVM_PARSE_PLAN_MAGIC) return 0;
    *version = bytes[1];
    encoded_header_size = (uint16_t)rxvm_binary_read_le_bytes(bytes, 2, 2);
    if (*version == RXVM_PARSE_PLAN_VERSION_FROZEN) {
        if (encoded_header_size != RXVM_PARSE_PLAN_HEADER_SIZE_FROZEN) return 0;
        *dynamic_count = 0u;
    } else if (*version == RXVM_PARSE_PLAN_VERSION_DYNAMIC) {
        if (encoded_header_size != RXVM_PARSE_PLAN_HEADER_SIZE_DYNAMIC ||
            plan->string_len < RXVM_PARSE_PLAN_HEADER_SIZE_DYNAMIC ||
            bytes[10] != 0u || bytes[11] != 0u) return 0;
        *dynamic_count = (uint16_t)rxvm_binary_read_le_bytes(bytes, 8, 2);
    } else {
        return 0;
    }
    *header_size = encoded_header_size;
    *item_count = (uint16_t)rxvm_binary_read_le_bytes(bytes, 4, 2);
    *result_count = (uint16_t)rxvm_binary_read_le_bytes(bytes, 6, 2);
    return *item_count > 0;
}

static int rxvm_parse_plan_read_item(const string_constant *plan,
                                     size_t offset,
                                     rxvm_parse_plan_item *item) {
    const unsigned char *bytes;
    uint64_t movement;

    if (!plan || !item || offset > plan->string_len ||
        plan->string_len - offset < 2u) return 0;
    bytes = (const unsigned char *)plan->string;
    memset(item, 0, sizeof(*item));
    item->kind = bytes[offset];
    item->flags = bytes[offset + 1u];
    item->offset = offset;
    offset += 2u;

    switch (item->kind) {
        case 1:
            if ((item->flags & ~RXVM_PARSE_PLAN_FLAG_STORE) != 0u) return 0;
            break;
        case 2:
            if (item->flags != 0u || plan->string_len - offset < 8u) return 0;
            item->literal_bytes = (size_t)rxvm_binary_read_le_bytes(bytes, offset, 4);
            item->literal_chars = (size_t)rxvm_binary_read_le_bytes(bytes, offset + 4u, 4);
            offset += 8u;
            if (item->literal_bytes > plan->string_len - offset ||
                item->literal_chars > item->literal_bytes) return 0;
            item->literal = bytes + offset;
            offset += item->literal_bytes;
            break;
        case 3:
        case 4:
        case 5:
            if ((item->flags & ~RXVM_PARSE_PLAN_FLAG_SKIP) != 0u ||
                plan->string_len - offset < 8u) return 0;
            movement = rxvm_binary_read_le_bytes(bytes, offset, 8);
            if (movement > (uint64_t)INT64_MAX) return 0;
            item->movement = (rxinteger)movement;
            offset += 8u;
            break;
        case 6:
            if (item->flags != 0u) return 0;
            break;
        case 7:
        case 8:
        case 9:
        case 10:
            if ((item->flags & ~RXVM_PARSE_PLAN_FLAG_CAPTURE) != 0u ||
                plan->string_len - offset < 2u) return 0;
            item->operand_index =
                    (uint16_t)rxvm_binary_read_le_bytes(bytes, offset, 2);
            offset += 2u;
            break;
        default:
            return 0;
    }
    item->next_offset = offset;
    return 1;
}

/* Resolve a version-2 dynamic item without allocating or mutating its source
 * value. External operands occupy temporary result-vector slots after the
 * public results; completed-target references address an earlier result slot. */
static int rxvm_parse_plan_resolve_item(rxvm_parse_plan_item *item,
                                        value *result,
                                        uint16_t result_count,
                                        uint16_t dynamic_count,
                                        unsigned char version) {
    value *operand;
    rxinteger movement;
    size_t index;

    if (item->kind < 7u || item->kind > 10u) return RXVM_PARSE_PLAN_OK;
    if (version != RXVM_PARSE_PLAN_VERSION_DYNAMIC) return RXVM_PARSE_PLAN_INVALID;
    if ((item->flags & RXVM_PARSE_PLAN_FLAG_CAPTURE) != 0u) {
        if (item->operand_index >= result_count) return RXVM_PARSE_PLAN_INVALID;
        index = item->operand_index;
    } else {
        if (item->operand_index >= dynamic_count) return RXVM_PARSE_PLAN_INVALID;
        index = (size_t)result_count + item->operand_index;
    }
    if (index >= result->num_attributes) return RXVM_PARSE_PLAN_INVALID;
    operand = result->attributes[index];

    if (item->kind == 7u) {
        item->kind = 2u;
        item->flags = 0u;
        item->literal = (const unsigned char *)operand->string_value;
        item->literal_bytes = operand->string_length;
#ifndef NUTF8
        item->literal_chars = operand->string_chars;
#else
        item->literal_chars = operand->string_length;
#endif
        return RXVM_PARSE_PLAN_OK;
    }

    if (string2integer(&movement, operand->string_value, operand->string_length) ||
        movement < 0) return RXVM_PARSE_PLAN_CONVERSION;
    item->kind = (unsigned char)(item->kind - 5u);
    item->flags = 0u;
    item->movement = movement;
    return RXVM_PARSE_PLAN_OK;
}

static size_t rxvm_parse_plan_source_chars(const value *source) {
#ifndef NUTF8
    return source->string_chars;
#else
    return source->string_length;
#endif
}

static void rxvm_parse_plan_set_empty(value *target) {
    set_string(target, "", 0);
}

static void rxvm_parse_plan_set_span(value *target,
                                     const value *source,
                                     size_t start,
                                     size_t character_length) {
    size_t byte_start;
    size_t byte_end;

    if (character_length == 0u) {
        rxvm_parse_plan_set_empty(target);
        return;
    }
    byte_start = rxvm_parse_byte_offset(source, start - 1u);
    byte_end = rxvm_parse_byte_offset(source, start - 1u + character_length);
    set_string(target, source->string_value + byte_start, byte_end - byte_start);
#ifndef NUTF8
    rxvm_value_set_string_chars_known(target, character_length);
    mark_utf8_valid_count(target);
#endif
}

static int rxvm_parse_plan_is_blank(const value *source, size_t position) {
    size_t byte_offset = rxvm_parse_byte_offset(source, position - 1u);
    return byte_offset < source->string_length &&
           source->string_value[byte_offset] == ' ';
}

static size_t rxvm_parse_plan_find_literal(const value *source,
                                           size_t start,
                                           const rxvm_parse_plan_item *literal) {
    size_t source_chars = rxvm_parse_plan_source_chars(source);
    size_t start_offset;
    const char *candidate;
    const char *last;

    if (literal->literal_bytes == 0u || start == 0u || start > source_chars) return 0u;
    start_offset = rxvm_parse_byte_offset(source, start - 1u);
    if (literal->literal_bytes > source->string_length - start_offset) return 0u;
    candidate = source->string_value + start_offset;
    last = source->string_value + source->string_length - literal->literal_bytes;
    while (candidate <= last) {
        candidate = (const char *)memchr(candidate, literal->literal[0],
                                        (size_t)(last - candidate + 1));
        if (!candidate) return 0u;
        if (memcmp(candidate, literal->literal, literal->literal_bytes) == 0) {
#ifndef NUTF8
            size_t character_offset = 0;
            if (utf8nvalid_count(source->string_value,
                                 (size_t)(candidate - source->string_value),
                                 &character_offset) != 0) return 0u;
            return character_offset + 1u;
#else
            return (size_t)(candidate - source->string_value) + 1u;
#endif
        }
        candidate++;
    }
    return 0u;
}

static int rxvm_parse_plan_equals_at(const value *source,
                                     size_t start,
                                     const rxvm_parse_plan_item *literal) {
    size_t source_chars = rxvm_parse_plan_source_chars(source);
    size_t byte_start;

    if (start == 0u || literal->literal_chars > source_chars - (start - 1u)) return 0;
    byte_start = rxvm_parse_byte_offset(source, start - 1u);
    if (literal->literal_bytes > source->string_length - byte_start) return 0;
    return literal->literal_bytes == 0u ||
           memcmp(source->string_value + byte_start,
                  literal->literal,
                  literal->literal_bytes) == 0;
}

static size_t rxvm_parse_plan_apply_cursor(const value *source,
                                           size_t cursor,
                                           const rxvm_parse_plan_item *item) {
    size_t source_chars = rxvm_parse_plan_source_chars(source);
    size_t position;
    size_t movement = (size_t)item->movement;
    size_t room;

    if (item->kind == 2) {
        if (item->literal_bytes == 0u) return cursor;
        position = rxvm_parse_plan_find_literal(source, cursor, item);
        return position == 0u ? source_chars + 1u : position;
    }
    if (item->kind == 3) {
        if (movement < 1u) return 1u;
        if (movement > source_chars + 1u) return source_chars + 1u;
        return movement;
    }
    if (item->kind == 4) {
        room = source_chars + 1u - cursor;
        if (movement > room) return source_chars + 1u;
        return cursor + movement;
    }
    if (item->kind == 5) {
        if (movement >= cursor) return 1u;
        return cursor - movement;
    }
    if (item->kind == 6) {
        position = cursor;
        while (position <= source_chars && rxvm_parse_plan_is_blank(source, position)) position++;
        return position;
    }
    return cursor;
}

static size_t rxvm_parse_plan_store_implicit(value *target,
                                             int store,
                                             const value *source,
                                             size_t capture_start) {
    size_t source_chars = rxvm_parse_plan_source_chars(source);
    size_t first = capture_start;
    size_t after;

    while (first <= source_chars && rxvm_parse_plan_is_blank(source, first)) first++;
    if (first > source_chars) {
        if (store) rxvm_parse_plan_set_empty(target);
        return source_chars + 1u;
    }
    after = first;
    while (after <= source_chars && !rxvm_parse_plan_is_blank(source, after)) after++;
    if (store) rxvm_parse_plan_set_span(target, source, first, after - first);
    if (after <= source_chars) after++;
    return after;
}

static void rxvm_parse_plan_store_word(value *target,
                                       int store,
                                       const value *source,
                                       size_t field_start,
                                       int raw) {
    size_t source_chars = rxvm_parse_plan_source_chars(source);
    size_t first = field_start;
    size_t word_start;
    size_t after;

    if (first > source_chars) {
        if (store) rxvm_parse_plan_set_empty(target);
        return;
    }
    word_start = first;
    while (word_start <= source_chars && rxvm_parse_plan_is_blank(source, word_start)) word_start++;
    if (word_start > source_chars) {
        if (store) rxvm_parse_plan_set_span(target, source, first, source_chars - first + 1u);
        return;
    }
    after = word_start;
    while (after <= source_chars && !rxvm_parse_plan_is_blank(source, after)) after++;
    if (raw) {
        while (after <= source_chars && rxvm_parse_plan_is_blank(source, after)) after++;
        if (store) rxvm_parse_plan_set_span(target, source, first, after - first);
    } else if (store) {
        rxvm_parse_plan_set_span(target, source, word_start, after - word_start);
    }
}

static size_t rxvm_parse_plan_store_literal(value *target,
                                            int store,
                                            const value *source,
                                            size_t capture_start,
                                            size_t cursor,
                                            const rxvm_parse_plan_item *literal,
                                            size_t *relative_anchor) {
    size_t source_chars = rxvm_parse_plan_source_chars(source);
    size_t position;

    if (literal->literal_bytes == 0u) {
        if (store) rxvm_parse_plan_set_empty(target);
        return cursor;
    }
    position = rxvm_parse_plan_find_literal(source, cursor, literal);
    if (position == 0u) {
        if (store) {
            if (capture_start > source_chars) rxvm_parse_plan_set_empty(target);
            else rxvm_parse_plan_set_span(target, source, capture_start,
                                          source_chars - capture_start + 1u);
        }
        return source_chars + 1u;
    }
    *relative_anchor = position;
    if (store) rxvm_parse_plan_set_span(target, source, capture_start,
                                        position - capture_start);
    return position + literal->literal_chars;
}

static size_t rxvm_parse_plan_store_numeric(value *target,
                                            int store,
                                            const value *source,
                                            size_t cursor,
                                            size_t capture_start,
                                            const rxvm_parse_plan_item *item,
                                            size_t *relative_anchor) {
    size_t source_chars = rxvm_parse_plan_source_chars(source);
    size_t movement = (size_t)item->movement;
    size_t position;
    size_t room;

    if ((item->kind == 4 || item->kind == 5) && *relative_anchor > 0u) {
        cursor = *relative_anchor;
        *relative_anchor = 0u;
    }

    if (item->kind == 3) {
        position = movement;
        if (position < 1u) position = 1u;
        if (position > source_chars + 1u) position = source_chars + 1u;
    } else if (item->kind == 4) {
        room = source_chars + 1u - cursor;
        position = movement > room ? source_chars + 1u : cursor + movement;
    } else {
        position = movement >= cursor ? 1u : cursor - movement;
    }

    if (position <= capture_start) {
        rxvm_parse_plan_store_word(target, store, source, capture_start,
                                   item->kind == 5);
        position = capture_start;
    } else if (store) {
        rxvm_parse_plan_set_span(target, source, capture_start,
                                 position - capture_start);
    }
    return position;
}

static int rxvm_parse_plan_apply_pre(const string_constant *plan,
                                     value *result,
                                     const value *source,
                                     uint16_t result_count,
                                     uint16_t dynamic_count,
                                     unsigned char version,
                                     size_t start_offset,
                                     size_t end_offset,
                                     unsigned char next_kind,
                                     const rxvm_parse_plan_item *shared_item,
                                     unsigned char shared_kind,
                                     size_t *cursor,
                                     size_t *relative_anchor) {
    rxvm_parse_plan_item item;
    rxvm_parse_plan_item next_item;
    size_t offset = start_offset;
    int pre_relative_anchor = 0;
    int status;

    if (shared_kind == 3u) {
        *cursor = rxvm_parse_plan_apply_cursor(source, *cursor, shared_item);
    } else if (shared_kind == 8u) {
        size_t position;
        if (shared_item->literal_chars >= *cursor) {
            *cursor = rxvm_parse_plan_source_chars(source) + 1u;
        } else {
            position = *cursor - shared_item->literal_chars;
            if (rxvm_parse_plan_equals_at(source, position, shared_item)) *cursor = position;
            else *cursor = rxvm_parse_plan_source_chars(source) + 1u;
        }
    }

    while (offset < end_offset) {
        size_t position;
        unsigned char following_kind = 0u;
        int anchor_at_start;

        if (!rxvm_parse_plan_read_item(plan, offset, &item) ||
            item.next_offset > end_offset) return RXVM_PARSE_PLAN_INVALID;
        status = rxvm_parse_plan_resolve_item(&item, result, result_count,
                                              dynamic_count, version);
        if (status != RXVM_PARSE_PLAN_OK) return status;
        if (item.flags & RXVM_PARSE_PLAN_FLAG_SKIP) {
            offset = item.next_offset;
            continue;
        }
        if (item.next_offset < end_offset) {
            if (!rxvm_parse_plan_read_item(plan, item.next_offset, &next_item) ||
                next_item.next_offset > end_offset) return RXVM_PARSE_PLAN_INVALID;
            status = rxvm_parse_plan_resolve_item(&next_item, result, result_count,
                                                  dynamic_count, version);
            if (status != RXVM_PARSE_PLAN_OK) return status;
            following_kind = next_item.kind;
        }
        if (item.kind == 2) {
            if (following_kind == 4u || following_kind == 5u) {
                pre_relative_anchor = 1;
            }
            position = item.literal_bytes == 0u ? *cursor :
                       rxvm_parse_plan_find_literal(source, *cursor, &item);
            if (item.literal_bytes != 0u && position == 0u) {
                *cursor = rxvm_parse_plan_source_chars(source) + 1u;
            } else {
                anchor_at_start = following_kind == 4u || following_kind == 5u;
                if (!anchor_at_start &&
                    (item.literal_bytes == 0u || item.literal[0] != ' ') &&
                    next_kind == 4u) anchor_at_start = 1;
                *relative_anchor = anchor_at_start ? position : 0u;
                *cursor = anchor_at_start ? position : position + item.literal_chars;
            }
        } else {
            *cursor = rxvm_parse_plan_apply_cursor(source, *cursor, &item);
        }
        offset = item.next_offset;
    }
    if (offset != end_offset) return RXVM_PARSE_PLAN_INVALID;
    if (pre_relative_anchor) *relative_anchor = 0u;
    return RXVM_PARSE_PLAN_OK;
}

static int rxvm_parse_plan_execute(value *result,
                                   value *source,
                                   const string_constant *plan) {
    unsigned char version;
    size_t header_size;
    uint16_t item_count;
    uint16_t result_count;
    uint16_t dynamic_count;
    rxvm_parse_plan_item current;
    rxvm_parse_plan_item next;
    rxvm_parse_plan_item after;
    rxvm_parse_plan_item shared_item;
    size_t offset;
    size_t pre_offset;
    size_t cursor = 1u;
    size_t capture_start = 0u;
    size_t relative_anchor = 0u;
    size_t output_index = 0u;
    size_t pending_output = 0u;
    unsigned int index;
    int pending = 0;
    int pending_store = 0;
    int status;
    unsigned char shared_kind = 0u;

    if (!rxvm_parse_plan_header(plan, &version, &header_size, &item_count,
                                &result_count, &dynamic_count)) {
        return RXVM_PARSE_PLAN_INVALID;
    }
    offset = header_size;
    pre_offset = header_size;
    if (dynamic_count > 0u) {
        size_t required = (size_t)result_count + dynamic_count;
        if (result->num_attributes < required) return RXVM_PARSE_PLAN_INVALID;
    } else {
        set_num_attributes(result, result_count);
    }

    for (index = 0; index < item_count; index++) {
        int next_exists;
        int after_exists;

        if (!rxvm_parse_plan_read_item(plan, offset, &current)) {
            return RXVM_PARSE_PLAN_INVALID;
        }
        status = rxvm_parse_plan_resolve_item(&current, result, result_count,
                                              dynamic_count, version);
        if (status != RXVM_PARSE_PLAN_OK) return status;
        next_exists = index + 1u < item_count;
        after_exists = index + 2u < item_count;
        if (next_exists) {
            if (!rxvm_parse_plan_read_item(plan, current.next_offset, &next)) {
                return RXVM_PARSE_PLAN_INVALID;
            }
        }
        if (after_exists) {
            if (!rxvm_parse_plan_read_item(plan, next.next_offset, &after)) {
                return RXVM_PARSE_PLAN_INVALID;
            }
        }

        if (current.kind == 1u) {
            if (pending) {
                cursor = rxvm_parse_plan_store_implicit(
                        pending_store ? result->attributes[pending_output] : 0,
                        pending_store, source, capture_start);
                pending = 0;
            }
            if (next_exists) {
                status = rxvm_parse_plan_resolve_item(&next, result, result_count,
                                                      dynamic_count, version);
                if (status != RXVM_PARSE_PLAN_OK) return status;
            }
            status = rxvm_parse_plan_apply_pre(plan, result, source,
                                               result_count, dynamic_count, version,
                                               pre_offset, current.offset,
                                               next_exists ? next.kind : 0u,
                                               &shared_item, shared_kind,
                                               &cursor, &relative_anchor);
            if (status != RXVM_PARSE_PLAN_OK) return status;
            shared_kind = 0u;
            pending_store = (current.flags & RXVM_PARSE_PLAN_FLAG_STORE) != 0u;
            if (pending_store) {
                if (output_index >= result_count) return 0;
                pending_output = output_index++;
            }
            capture_start = cursor;
            pending = 1;
            pre_offset = current.next_offset;
        } else if (!pending) {
            /* Controls before a target are applied together when that target arrives. */
        } else {
            value *target = pending_store ? result->attributes[pending_output] : 0;
            if (current.kind == 2u) {
                cursor = rxvm_parse_plan_store_literal(target, pending_store, source,
                                                       capture_start, cursor, &current,
                                                       &relative_anchor);
                if (next_exists && after_exists && next.kind == 1u) {
                    status = rxvm_parse_plan_resolve_item(&after, result, result_count,
                                                          dynamic_count, version);
                    if (status != RXVM_PARSE_PLAN_OK) return status;
                    if (after.kind == 4u) {
                        shared_item = current;
                        shared_kind = 8u;
                    }
                }
            } else if (current.kind == 3u || current.kind == 4u || current.kind == 5u) {
                cursor = rxvm_parse_plan_store_numeric(target, pending_store, source,
                                                       cursor, capture_start, &current,
                                                       &relative_anchor);
                relative_anchor = 0u;
                if (current.kind == 3u && next_exists && next.kind == 1u) {
                    shared_item = current;
                    shared_kind = 3u;
                }
            } else {
                cursor = rxvm_parse_plan_store_implicit(target, pending_store,
                                                        source, capture_start);
            }
            pending = 0;
            capture_start = 0u;
            pre_offset = current.next_offset;
        }
        offset = current.next_offset;
    }

    if (offset != plan->string_len) return RXVM_PARSE_PLAN_INVALID;
    if (pending) {
        size_t source_chars = rxvm_parse_plan_source_chars(source);
        if (pending_store) {
            if (capture_start > source_chars) rxvm_parse_plan_set_empty(result->attributes[pending_output]);
            else rxvm_parse_plan_set_span(result->attributes[pending_output], source,
                                          capture_start, source_chars - capture_start + 1u);
        }
    }
    if (output_index != result_count) return RXVM_PARSE_PLAN_INVALID;
    if (result->num_attributes != result_count) {
        set_num_attributes(result, result_count);
    }
    return RXVM_PARSE_PLAN_OK;
}

static uint64_t rxvm_binary_read_le(const value *buffer, size_t offset, size_t width) {
    return rxvm_binary_read_le_bytes((const unsigned char *)buffer->binary_value, offset, width);
}

static void rxvm_binary_write_le(value *buffer, size_t offset, size_t width, uint64_t data) {
    unsigned char *bytes = (unsigned char *)buffer->binary_value + offset;
    size_t i;

    if (width == 1) {
        bytes[0] = (unsigned char)(data & 0xffu);
        clear_vm_private_flags(buffer);
        return;
    }
#if defined(RXVM_HOST_LITTLE_ENDIAN)
    if (width == 2) {
        uint16_t value = (uint16_t)data;
        memcpy(bytes, &value, sizeof(value));
        clear_vm_private_flags(buffer);
        return;
    }
    if (width == 4) {
        uint32_t value = (uint32_t)data;
        memcpy(bytes, &value, sizeof(value));
        clear_vm_private_flags(buffer);
        return;
    }
    if (width == 8) {
        uint64_t value = data;
        memcpy(bytes, &value, sizeof(value));
        clear_vm_private_flags(buffer);
        return;
    }
#elif defined(RXVM_HOST_BIG_ENDIAN)
    if (width == 2) {
        uint16_t value = rxvm_bswap16((uint16_t)data);
        memcpy(bytes, &value, sizeof(value));
        clear_vm_private_flags(buffer);
        return;
    }
    if (width == 4) {
        uint32_t value = rxvm_bswap32((uint32_t)data);
        memcpy(bytes, &value, sizeof(value));
        clear_vm_private_flags(buffer);
        return;
    }
    if (width == 8) {
        uint64_t value = rxvm_bswap64(data);
        memcpy(bytes, &value, sizeof(value));
        clear_vm_private_flags(buffer);
        return;
    }
#endif

    for (i = 0; i < width; i++) {
        bytes[i] = (unsigned char)((data >> (i * 8)) & 0xffu);
    }
    clear_vm_private_flags(buffer);
}

static rxinteger rxvm_sign_extend_le_value(uint64_t value, unsigned int bits) {
    uint64_t sign_bit = UINT64_C(1) << (bits - 1);
    uint64_t mask = (UINT64_C(1) << bits) - 1;

    value &= mask;
    if ((value & sign_bit) == 0) return (rxinteger)value;
    return -(rxinteger)((~value & mask) + 1);
}

static uintmax_t rxvm_rxinteger_positive_max(void) {
    size_t bits = sizeof(rxinteger) * CHAR_BIT;
    size_t uintmax_bits = sizeof(uintmax_t) * CHAR_BIT;

    if (bits >= uintmax_bits) return UINTMAX_MAX >> 1;
    return (((uintmax_t)1) << (bits - 1)) - 1;
}

static int rxvm_uint32_fits_rxinteger(uint32_t value) {
    return (uintmax_t)value <= rxvm_rxinteger_positive_max();
}

static int rxvm_i64_raw_to_rxinteger(uint64_t raw, rxinteger *result) {
    uintmax_t positive_max = rxvm_rxinteger_positive_max();

    if ((raw & UINT64_C(0x8000000000000000)) == 0) {
        if ((uintmax_t)raw > positive_max) return 0;
        *result = (rxinteger)raw;
        return 1;
    }

    {
        uint64_t magnitude = (~raw) + UINT64_C(1);
        uintmax_t negative_max = positive_max + 1;

        if ((uintmax_t)magnitude > negative_max) return 0;
        if ((uintmax_t)magnitude == negative_max) {
            *result = -((rxinteger)(magnitude - 1)) - 1;
        }
        else {
            *result = -(rxinteger)magnitude;
        }
        return 1;
    }
}

static double rxvm_binary_read_f32_le_bytes(const unsigned char *bytes, size_t offset) {
    uint32_t bits = (uint32_t)rxvm_binary_read_le_bytes(bytes, offset, 4);
    float result;

    memcpy(&result, &bits, sizeof(result));
    return (double)result;
}

static int rxvm_binary_write_f32_le(value *buffer, size_t offset, double value) {
    uint32_t bits;
    float narrowed;

    if (sizeof(narrowed) != sizeof(bits)) return -1;
    narrowed = (float)value;
    memcpy(&bits, &narrowed, sizeof(bits));
    rxvm_binary_write_le(buffer, offset, 4, bits);
    return 0;
}

static double rxvm_binary_read_f64_le(const value *buffer, size_t offset) {
    uint64_t bits = rxvm_binary_read_le(buffer, offset, 8);
    double result;

    memcpy(&result, &bits, sizeof(result));
    return result;
}

static int rxvm_binary_write_f64_le(value *buffer, size_t offset, double value) {
    uint64_t bits;

    if (sizeof(value) != sizeof(bits)) return -1;
    memcpy(&bits, &value, sizeof(bits));
    rxvm_binary_write_le(buffer, offset, 8, bits);
    return 0;
}

static double rxvm_binary_read_f64_le_bytes(const unsigned char *bytes, size_t offset) {
    uint64_t bits = rxvm_binary_read_le_bytes(bytes, offset, 8);
    double result;

    memcpy(&result, &bits, sizeof(result));
    return result;
}

static int rxvm_find_nul_field(const unsigned char *bytes, size_t length, rxinteger offset_value,
                               size_t *offset, size_t *field_length) {
    size_t local_offset;
    size_t i;

    if (!rxvm_memory_range(length, offset_value, 0, &local_offset)) return 0;
    for (i = local_offset; i < length; i++) {
        if (bytes[i] == 0) {
            if (offset) *offset = local_offset;
            if (field_length) *field_length = i - local_offset;
            return 1;
        }
    }
    return 0;
}

static int rxvm_compare_bytes(const unsigned char *left, size_t left_len,
                              const unsigned char *right, size_t right_len) {
    size_t common = left_len < right_len ? left_len : right_len;
    int cmp = common ? memcmp(left, right, common) : 0;

    if (cmp < 0) return -1;
    if (cmp > 0) return 1;
    if (left_len < right_len) return -1;
    if (left_len > right_len) return 1;
    return 0;
}

static int rxvm_jtable_read_header(string_constant *table, unsigned char *algorithm,
                                   uint32_t *key_length, uint32_t *case_count,
                                   uint16_t *header_size) {
    const unsigned char *bytes;

    if (!table || table->string_len < RX_JTABLE_HEADER_SIZE) return 0;
    bytes = (const unsigned char *)table->string;
    *algorithm = bytes[0];
    if (*algorithm < RX_JTABLE_ALG_LINEAR || *algorithm > RX_JTABLE_ALG_ACPH) return 0;
    *header_size = (uint16_t)rxvm_binary_read_le_bytes(bytes, 2, 2);
    if (*header_size < RX_JTABLE_HEADER_SIZE || *header_size > table->string_len) return 0;
    *key_length = (uint32_t)rxvm_binary_read_le_bytes(bytes, 4, 4);
    *case_count = (uint32_t)rxvm_binary_read_le_bytes(bytes, 8, 4);
    if (*case_count == 0) return 0;
    return 1;
}

static int rxvm_jtable_lookup_linear(string_constant *table, const unsigned char *key,
                                     size_t key_length, uint32_t fixed_key_length,
                                     uint32_t case_count, uint16_t header_size,
                                     size_t *target_out) {
    const unsigned char *bytes;
    size_t entries_length;
    size_t entry_offset;
    uint32_t i;

    if (header_size != RX_JTABLE_HEADER_SIZE) return -1;
    if ((size_t)case_count > SIZE_MAX / RX_JTABLE_LINEAR_ENTRY_SIZE) return -1;
    entries_length = (size_t)case_count * RX_JTABLE_LINEAR_ENTRY_SIZE;
    if ((size_t)header_size > SIZE_MAX - entries_length ||
        (size_t)header_size + entries_length > table->string_len) return -1;
    if (fixed_key_length != 0 && key_length != fixed_key_length) return 0;

    bytes = (const unsigned char *)table->string;
    entry_offset = header_size;
    for (i = 0; i < case_count; i++) {
        uint32_t key_offset;
        uint32_t entry_key_length;
        uint32_t target;

        key_offset = (uint32_t)rxvm_binary_read_le_bytes(bytes, entry_offset, 4);
        entry_key_length = (uint32_t)rxvm_binary_read_le_bytes(bytes, entry_offset + 4, 4);
        target = (uint32_t)rxvm_binary_read_le_bytes(bytes, entry_offset + 8, 4);
        if ((size_t)key_offset > table->string_len ||
            (size_t)entry_key_length > table->string_len - (size_t)key_offset) {
            return -1;
        }
        if ((size_t)entry_key_length == key_length &&
            (key_length == 0 || memcmp(bytes + key_offset, key, key_length) == 0)) {
            *target_out = (size_t)target;
            return 1;
        }
        entry_offset += RX_JTABLE_LINEAR_ENTRY_SIZE;
    }
    return 0;
}

static int rxvm_jtable_lookup_openhash(string_constant *table, const unsigned char *key,
                                       size_t key_length, uint32_t fixed_key_length,
                                       uint32_t case_count, uint16_t header_size,
                                       size_t *target_out) {
    const unsigned char *bytes = (const unsigned char *)table->string;
    uint32_t slot_count;
    uint32_t hash;
    size_t slots_length;
    size_t slot;
    size_t probes;

    if (header_size != RX_JTABLE_OPEN_HEADER_SIZE) return -1;
    slot_count = (uint32_t)rxvm_binary_read_le_bytes(bytes, 12, 4);
    if (slot_count < 2u || (slot_count & (slot_count - 1u)) != 0 ||
        case_count > slot_count / 2u) return -1;
    if ((size_t)slot_count > SIZE_MAX / RX_JTABLE_OPEN_SLOT_SIZE) return -1;
    slots_length = (size_t)slot_count * RX_JTABLE_OPEN_SLOT_SIZE;
    if ((size_t)header_size > SIZE_MAX - slots_length ||
        (size_t)header_size + slots_length > table->string_len) return -1;
    if (fixed_key_length != 0 && key_length != fixed_key_length) return 0;

    hash = rx_jtable_hash_bytes(key, key_length);
    slot = hash & (slot_count - 1u);
    for (probes = 0; probes < slot_count; probes++) {
        size_t slot_offset = header_size + slot * RX_JTABLE_OPEN_SLOT_SIZE;
        uint32_t stored_hash = (uint32_t)rxvm_binary_read_le_bytes(bytes, slot_offset, 4);
        uint32_t key_offset = (uint32_t)rxvm_binary_read_le_bytes(bytes, slot_offset + 4, 4);
        uint32_t stored_key_length;

        if (key_offset == RX_JTABLE_OPEN_EMPTY) return 0;
        stored_key_length = (uint32_t)rxvm_binary_read_le_bytes(bytes, slot_offset + 8, 4);
        if ((size_t)key_offset > table->string_len ||
            (size_t)stored_key_length > table->string_len - (size_t)key_offset) return -1;
        if (stored_hash == hash && (size_t)stored_key_length == key_length &&
            (key_length == 0 || memcmp(bytes + key_offset, key, key_length) == 0)) {
            *target_out = (size_t)rxvm_binary_read_le_bytes(bytes, slot_offset + 12, 4);
            return 1;
        }
        slot = (slot + 1u) & (slot_count - 1u);
    }
    return -1;
}

static int rxvm_jtable_lookup_acph(string_constant *table, const unsigned char *key,
                                   size_t key_length, uint32_t fixed_key_length,
                                   uint32_t case_count, uint16_t header_size,
                                   size_t *target_out) {
    const unsigned char *bytes = (const unsigned char *)table->string;
    uint32_t node_offset;
    uint32_t depth;

    if (header_size != RX_JTABLE_ACPH_HEADER_SIZE) return -1;
    if (fixed_key_length != 0 && key_length != fixed_key_length) return 0;
    node_offset = (uint32_t)rxvm_binary_read_le_bytes(bytes, 12, 4);

    for (depth = 0; depth < case_count; depth++) {
        uint32_t column;
        uint16_t slot_count;
        unsigned char prime;
        uint16_t symbol;
        size_t node_length;
        size_t slot_index;
        size_t slot_offset;
        uint16_t stored_symbol;
        unsigned char kind;
        uint32_t value_offset;

        if ((size_t)node_offset > table->string_len ||
            RX_JTABLE_ACPH_NODE_SIZE > table->string_len - (size_t)node_offset) return -1;
        column = (uint32_t)rxvm_binary_read_le_bytes(bytes, node_offset, 4);
        slot_count = (uint16_t)rxvm_binary_read_le_bytes(bytes, node_offset + 4, 2);
        prime = bytes[node_offset + 6];
        if (slot_count == 0 || slot_count > RX_JTABLE_ACPH_SYMBOL_COUNT ||
            (slot_count != RX_JTABLE_ACPH_SYMBOL_COUNT && prime == 0)) return -1;
        node_length = RX_JTABLE_ACPH_NODE_SIZE + (size_t)slot_count * RX_JTABLE_ACPH_SLOT_SIZE;
        if (node_length > table->string_len - (size_t)node_offset) return -1;

        symbol = (size_t)column < key_length ? key[column] : RX_JTABLE_ACPH_END_SYMBOL;
        slot_index = rx_jtable_acph_hash(symbol, prime, slot_count);
        slot_offset = (size_t)node_offset + RX_JTABLE_ACPH_NODE_SIZE +
                      slot_index * RX_JTABLE_ACPH_SLOT_SIZE;
        stored_symbol = (uint16_t)rxvm_binary_read_le_bytes(bytes, slot_offset, 2);
        kind = bytes[slot_offset + 2];
        value_offset = (uint32_t)rxvm_binary_read_le_bytes(bytes, slot_offset + 4, 4);
        if (kind == RX_JTABLE_ACPH_SLOT_EMPTY) return 0;
        if (stored_symbol > RX_JTABLE_ACPH_END_SYMBOL) return -1;
        if (stored_symbol != symbol) return 0;

        if (kind == RX_JTABLE_ACPH_SLOT_LEAF) {
            uint32_t key_offset;
            uint32_t stored_key_length;

            if ((size_t)value_offset > table->string_len ||
                RX_JTABLE_ACPH_LEAF_SIZE > table->string_len - (size_t)value_offset) return -1;
            key_offset = (uint32_t)rxvm_binary_read_le_bytes(bytes, value_offset, 4);
            stored_key_length = (uint32_t)rxvm_binary_read_le_bytes(bytes, value_offset + 4, 4);
            if ((size_t)key_offset > table->string_len ||
                (size_t)stored_key_length > table->string_len - (size_t)key_offset) return -1;
            if ((size_t)stored_key_length != key_length ||
                (key_length != 0 && memcmp(bytes + key_offset, key, key_length) != 0)) return 0;
            *target_out = (size_t)rxvm_binary_read_le_bytes(bytes, value_offset + 8, 4);
            return 1;
        }
        if (kind != RX_JTABLE_ACPH_SLOT_CHILD) return -1;
        node_offset = value_offset;
    }
    return -1;
}

static int rxvm_jtable_lookup_parsed(string_constant *table, const unsigned char *key,
                                     size_t key_length, size_t *target_out,
                                     unsigned char algorithm, uint32_t fixed_key_length,
                                     uint32_t case_count, uint16_t header_size) {
    switch (algorithm) {
        case RX_JTABLE_ALG_LINEAR:
            return rxvm_jtable_lookup_linear(table, key, key_length, fixed_key_length,
                                             case_count, header_size, target_out);
        case RX_JTABLE_ALG_OPENHASH:
            return rxvm_jtable_lookup_openhash(table, key, key_length, fixed_key_length,
                                               case_count, header_size, target_out);
        case RX_JTABLE_ALG_ACPH:
            return rxvm_jtable_lookup_acph(table, key, key_length, fixed_key_length,
                                           case_count, header_size, target_out);
        default:
            return -1;
    }
}

static int rxvm_jtable_lookup(string_constant *table, const unsigned char *key,
                              size_t key_length, size_t *target_out) {
    unsigned char algorithm;
    uint32_t fixed_key_length;
    uint32_t case_count;
    uint16_t header_size;

    if (!rxvm_jtable_read_header(table, &algorithm, &fixed_key_length, &case_count, &header_size)) return -1;
    return rxvm_jtable_lookup_parsed(table, key, key_length, target_out, algorithm,
                                     fixed_key_length, case_count, header_size);
}

static int rxvm_binary_field_is_valid_utf8(const void *bytes, size_t length) {
#ifndef NUTF8
    return validate_utf8_bytes(bytes, length, 0) == 0;
#else
    (void)bytes;
    (void)length;
    return 1;
#endif
}

static int rxvm_utf8_is_boundary(const char *bytes, size_t length, size_t offset) {
#ifndef NUTF8
    if (offset > length) return 0;
    if (offset == length) return 1;
    return (((unsigned char)bytes[offset] & 0xc0u) != 0x80u);
#else
    (void)bytes;
    return offset <= length;
#endif
}

static int rxvm_string_const_slice(string_constant *source, rxinteger offset_value, size_t requested_chars,
                                  size_t *offset, size_t *byte_length) {
    size_t local_offset;

    if (!rxvm_memory_range(source->string_len, offset_value, 0, &local_offset)) return 0;

#ifndef NUTF8
    if (!rxvm_utf8_is_boundary(source->string, source->string_len, local_offset)) return -1;

    {
        size_t pos = local_offset;
        size_t i;

        for (i = 0; i < requested_chars; i++) {
            size_t step;
            if (pos >= source->string_len) return 0;
            step = utf8codepointcalcsize(source->string + pos);
            if (step > source->string_len - pos) return -1;
            pos += step;
        }

        if (offset) *offset = local_offset;
        if (byte_length) *byte_length = pos - local_offset;
        return 1;
    }
#else
    if (requested_chars > source->string_len - local_offset) return 0;
    if (offset) *offset = local_offset;
    if (byte_length) *byte_length = requested_chars;
    return 1;
#endif
}

static int rxvm_format_parse_field_number(const char **cursor, const char *end, int *present, int *value) {
    const char *p = *cursor;
    int parsed_value = 0;

    *present = 0;
    while (p < end && *p >= '0' && *p <= '9') {
        int digit = *p - '0';
        *present = 1;
        if (parsed_value <= (RXVM_FFORMAT_MAX_FIELD - digit) / 10) {
            parsed_value = parsed_value * 10 + digit;
        }
        else {
            parsed_value = RXVM_FFORMAT_MAX_FIELD;
        }
        p++;
    }
    *value = parsed_value;
    *cursor = p;
    return 1;
}

static int rxvm_format_is_float_conversion(char ch) {
    return ch == 'a' || ch == 'A' ||
           ch == 'e' || ch == 'E' ||
           ch == 'f' || ch == 'F' ||
           ch == 'g' || ch == 'G';
}

static int rxvm_format_parse_float_conversion(const char *format,
                                              size_t format_len,
                                              size_t percent_index,
                                              size_t *next_index,
                                              int *width_present,
                                              int *width,
                                              int *precision_present,
                                              int *precision,
                                              char *conversion) {
    const char *p = format + percent_index + 1;
    const char *end = format + format_len;

    if (p >= end) return 0;
    if (*p == '*') return 0;

    rxvm_format_parse_field_number(&p, end, width_present, width);

    if (p < end && *p == '.') {
        p++;
        *precision_present = 1;
        *precision = 0;
        if (p < end && *p == '*') return 0;
        rxvm_format_parse_field_number(&p, end, precision_present, precision);
        *precision_present = 1;
    }
    else {
        *precision_present = 0;
        *precision = 0;
    }

    if (p >= end || !rxvm_format_is_float_conversion(*p)) return 0;
    *conversion = *p++;
    *next_index = (size_t)(p - format);
    return 1;
}

static size_t rxvm_format_append_double(char *buffer,
                                        size_t buffer_len,
                                        size_t used,
                                        int width_present,
                                        int width,
                                        int precision_present,
                                        int precision,
                                        char conversion,
                                        double value) {
    char *out = rxvm_format_buffer_at(buffer, buffer_len, used);
    size_t remaining = rxvm_format_buffer_remaining(buffer_len, used);
    int written = -1;

#define RXVM_APPEND_DOUBLE_CASE(ch, no_width, with_width, with_precision, with_width_precision) \
    case ch: \
        if (width_present && precision_present) written = snprintf(out, remaining, with_width_precision, width, precision, value); \
        else if (width_present) written = snprintf(out, remaining, with_width, width, value); \
        else if (precision_present) written = snprintf(out, remaining, with_precision, precision, value); \
        else written = snprintf(out, remaining, no_width, value); \
        break

    switch (conversion) {
        RXVM_APPEND_DOUBLE_CASE('a', "%a", "%*a", "%.*a", "%*.*a");
        RXVM_APPEND_DOUBLE_CASE('A', "%A", "%*A", "%.*A", "%*.*A");
        RXVM_APPEND_DOUBLE_CASE('e', "%e", "%*e", "%.*e", "%*.*e");
        RXVM_APPEND_DOUBLE_CASE('E', "%E", "%*E", "%.*E", "%*.*E");
        RXVM_APPEND_DOUBLE_CASE('f', "%f", "%*f", "%.*f", "%*.*f");
        RXVM_APPEND_DOUBLE_CASE('F', "%F", "%*F", "%.*F", "%*.*F");
        RXVM_APPEND_DOUBLE_CASE('g', "%g", "%*g", "%.*g", "%*.*g");
        RXVM_APPEND_DOUBLE_CASE('G', "%G", "%*G", "%.*G", "%*.*G");
        default:
            break;
    }

#undef RXVM_APPEND_DOUBLE_CASE

    return written < 0 ? used : used + (size_t)written;
}

static size_t rxvm_format_float_with_checked_format(char *buffer,
                                                    size_t buffer_len,
                                                    const char *format,
                                                    double value) {
    size_t format_len;
    size_t i = 0;
    size_t used = 0;
    int converted = 0;

    if (buffer_len > 0) buffer[0] = 0;
    if (!format) return 0;

    format_len = strlen(format);
    while (i < format_len) {
        if (format[i] != '%') {
            used = rxvm_format_append_literal(buffer, buffer_len, used, format + i, 1);
            i++;
            continue;
        }

        if (i + 1 < format_len && format[i + 1] == '%') {
            used = rxvm_format_append_literal(buffer, buffer_len, used, "%", 1);
            i += 2;
            continue;
        }

        if (!converted) {
            size_t next_index;
            int width_present;
            int width;
            int precision_present;
            int precision;
            char conversion;

            if (rxvm_format_parse_float_conversion(format,
                                                   format_len,
                                                   i,
                                                   &next_index,
                                                   &width_present,
                                                   &width,
                                                   &precision_present,
                                                   &precision,
                                                   &conversion)) {
                used = rxvm_format_append_double(buffer,
                                                 buffer_len,
                                                 used,
                                                 width_present,
                                                 width,
                                                 precision_present,
                                                 precision,
                                                 conversion,
                                                 value);
                converted = 1;
                i = next_index;
                continue;
            }
        }

        used = rxvm_format_append_literal(buffer, buffer_len, used, format + i, format_len - i);
        break;
    }

    return used;
}

static int rxvm_insert_attributes_checked(value *array, rxinteger index, rxinteger count) {
    size_t insert_index;
    size_t insert_count;

    if (!array || index < 0 || count < 0) return -1;

    insert_index = (size_t)index;
    insert_count = (size_t)count;
    if (insert_index > array->num_attributes) return -1;
    if (insert_count > SIZE_MAX - array->num_attributes) return -1;

    insert_attributes(array, insert_index, insert_count);
    return 0;
}

static int rxvm_insert_attributes1_checked(value *array, rxinteger index, rxinteger count) {
    if (index <= 0) return -1;
    return rxvm_insert_attributes_checked(array, index - 1, count);
}

static int rxvm_delete_attributes_checked(value *array, rxinteger index, rxinteger count) {
    size_t delete_index;
    size_t delete_count;

    if (!array || index < 0 || count < 0) return -1;

    delete_index = (size_t)index;
    delete_count = (size_t)count;
    if (delete_count == 0) {
        if (delete_index > array->num_attributes) return -1;
        return 0;
    }
    if (delete_index >= array->num_attributes) return -1;
    if (delete_count > array->num_attributes - delete_index) return -1;

    delete_attributes(array, delete_index, delete_count);
    return 0;
}

static int rxvm_delete_attributes1_checked(value *array, rxinteger index, rxinteger count) {
    if (index <= 0) return -1;
    return rxvm_delete_attributes_checked(array, index - 1, count);
}

static int rxvm_reference_storage_in_value_tree(value *root, value *storage) {
    size_t i;

    if (!root || !storage) return 0;
    if (root == storage) return 1;

    if (root->unlinked_attributes) {
        for (i = 0; i < rxvm_value_max_attributes(root); i++) {
            if (rxvm_reference_storage_in_value_tree(root->unlinked_attributes[i], storage)) return 1;
        }
    }

    return 0;
}

static stack_frame *rxvm_reference_lifetime_owner_frame(stack_frame *frame,
                                                        value *storage) {
    size_t i;

    while (frame) {
        if (frame->procedure) {
            size_t locals = (size_t)frame->procedure->locals;
            size_t globals = frame->procedure->binarySpace
                             ? (size_t)frame->procedure->binarySpace->globals
                             : 0;
            size_t a0_index = locals + globals;

            if (locals > frame->number_locals) locals = frame->number_locals;
            for (i = 0; i < locals; i++) {
                if (rxvm_reference_storage_in_value_tree(frame->baselocals[i], storage)) return frame;
            }

            if (a0_index < frame->number_locals &&
                rxvm_reference_storage_in_value_tree(frame->baselocals[a0_index], storage)) {
                return frame;
            }
        }

        frame = frame->parent;
    }

    return 0;
}

static void rxvm_mark_reference_lifetime_owner(stack_frame *current_frame,
                                               value *storage) {
    stack_frame *owner_frame = rxvm_reference_lifetime_owner_frame(current_frame,
                                                                   storage);
    if (owner_frame) owner_frame->has_reference_lifetimes = 1;
}

static rxvm_ref_owner_kind rxvm_reference_owner_kind_for_storage(stack_frame *frame,
                                                                 size_t register_index,
                                                                 value *storage,
                                                                 void **owner) {
    size_t i;
    size_t locals;
    size_t globals;
    int found_base_storage = 0;

    if (owner) *owner = frame;
    if (!frame || !storage || !frame->procedure) return RXVM_REF_OWNER_NONE;

    locals = (size_t)frame->procedure->locals;
    globals = frame->procedure->binarySpace ? (size_t)frame->procedure->binarySpace->globals : 0;

    for (i = 0; i < frame->number_locals; i++) {
        if (frame->baselocals[i] == storage) {
            register_index = i;
            found_base_storage = 1;
            break;
        }
    }

    if (!found_base_storage) {
        if (owner) *owner = 0;
        return RXVM_REF_ATTRIBUTE;
    }

    if (register_index < locals) return RXVM_REF_LOCAL;
    if (register_index < locals + globals) {
        if (owner) *owner = frame->procedure->binarySpace ? frame->procedure->binarySpace->module : 0;
        return RXVM_REF_GLOBAL;
    }
    if (register_index < frame->number_locals) return RXVM_REF_ARGUMENT;

    return RXVM_REF_ATTRIBUTE;
}

static rxvm_reference_cell *rxvm_reference_payload_cell(value *reference_value) {
    if (!reference_value) return 0;
    return reference_value->reference_payload;
}

static void rxvm_release_frame_reference_lifetimes(stack_frame *frame) {
    size_t i;
    size_t globals;
    size_t a0_index;

    if (!frame || !frame->procedure || !frame->has_reference_lifetimes) return;

    for (i = 0; i < (size_t)frame->procedure->locals; i++) {
        release_value_reference_lifetime(frame->baselocals[i]);
    }

    globals = frame->procedure->binarySpace ? (size_t)frame->procedure->binarySpace->globals : 0;
    a0_index = (size_t)frame->procedure->locals + globals;
    if (a0_index < frame->number_locals) {
        release_value_reference_lifetime(frame->baselocals[a0_index]);
    }

    frame->has_reference_lifetimes = 0;
}

/* This defines the expected max number of args - if a call has more args than
 * this then an oversized block will be malloced
 * In terms of memory usage / waste each one is only 2 x pointer size */
#define NOMINAL_NUM_ARGS 20

/* Define this to use a safe stack frame recycling mechanism - zeros registers in the stack frame */
/*#define SAFE_RECYCLED_STACKFRAMES*/
#undef SAFE_RECYCLED_STACKFRAMES

/* Misc. Utilities here */
static string_constant *get_runtime_string_constant(module *mod, size_t offset);

static void *rxvm_internal_alloc(size_t size) {
    return rxvm_memory_alloc_bytes(0, size);
}

static void rxvm_internal_free(void *pointer) {
    (void)rxvm_memory_release(pointer);
}

static value *decimal_literal_value(decplugin *decimal, const char *literal) {
    value *literal_value = value_f();
    decimal->decimalFromString(decimal, literal_value, literal);
    return literal_value;
}

static void free_decimal_literal_value(value *literal_value) {
    value_free(literal_value);
}

static char *build_runtime_member_name(const char *class_name, size_t class_name_length,
                                       const char *member_name, size_t member_name_length) {
    char *proc_name;

    proc_name = rxvm_internal_alloc(
            class_name_length + member_name_length + 2);
    if (!proc_name) return 0;

    memcpy(proc_name, class_name, class_name_length);
    proc_name[class_name_length] = '.';
    memcpy(proc_name + class_name_length + 1, member_name, member_name_length);
    proc_name[class_name_length + member_name_length + 1] = 0;

    return proc_name;
}

static int runtime_name_equals_ignore_case(const char *left, size_t left_length,
                                           const char *right, size_t right_length) {
    size_t i;

    if (!left || !right || left_length != right_length) return 0;
    for (i = 0; i < left_length; i++) {
        if (tolower((unsigned char)left[i]) != tolower((unsigned char)right[i])) return 0;
    }
    return 1;
}

static proc_runtime *resolve_runtime_procedure(rxvm_context *context, const char *proc_name, size_t proc_name_length) {
    size_t mod_index;

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod = context->modules[mod_index];
        int meta_ix = mod->meta_head;

        while (meta_ix != -1) {
            meta_entry *meta = (meta_entry *) (mod->segment.const_pool + meta_ix);

            if (meta->base.type == META_FUNC) {
                meta_func_constant *meta_func = (meta_func_constant *) meta;
                string_constant *symbol_name =
                        (string_constant *) (mod->segment.const_pool + meta_func->symbol);

                if (symbol_name->base.type == STRING_CONST &&
                    runtime_name_equals_ignore_case(symbol_name->string, symbol_name->string_len,
                                                    proc_name, proc_name_length)) {
                    return rxvm_get_module_runtime_procedure(mod, meta_func->func);
                }
            }

            meta_ix = meta->next;
        }
    }

    return 0;
}

static int compare_runtime_name(const char *left, size_t left_length,
                                const char *right, size_t right_length) {
    size_t min_length;
    int cmp;

    if (!left && !right) return 0;
    if (!left) return -1;
    if (!right) return 1;

    min_length = left_length < right_length ? left_length : right_length;
    cmp = memcmp(left, right, min_length);
    if (cmp) return cmp;
    if (left_length < right_length) return -1;
    if (left_length > right_length) return 1;
    return 0;
}

static char *dup_runtime_name(const char *name, size_t name_length) {
    char *copy;

    copy = rxvm_internal_alloc(name_length + 1);
    if (!copy) return 0;

    memcpy(copy, name, name_length);
    copy[name_length] = 0;
    return copy;
}

static char *build_interface_factory_error(const char *prefix,
                                           const char *interface_name,
                                           size_t interface_name_length) {
    size_t prefix_length;
    char *buffer;

    prefix_length = strlen(prefix);
    buffer = rxvm_internal_alloc(prefix_length + interface_name_length + 1);
    if (!buffer) return 0;

    memcpy(buffer, prefix, prefix_length);
    memcpy(buffer + prefix_length, interface_name, interface_name_length);
    buffer[prefix_length + interface_name_length] = 0;
    return buffer;
}

static int runtime_type_name_is_builtin(const char *type_name, size_t type_name_length) {
    static const char *builtins[] = {
            ".boolean", ".int", ".float", ".decimal", ".string",
            ".binary", ".object", ".void", ".unknown", 0
    };
    size_t i;

    if (!type_name) return 0;

    for (i = 0; builtins[i]; i++) {
        if (strlen(builtins[i]) == type_name_length &&
            memcmp(builtins[i], type_name, type_name_length) == 0) {
            return 1;
        }
    }

    return 0;
}

static int runtime_type_name_is_object_contract(const char *type_name, size_t type_name_length) {
    static const char object_name[] = ".object";
    size_t i;

    if (!type_name || type_name_length != sizeof(object_name) - 1) return 0;
    for (i = 0; i < sizeof(object_name) - 1; i++) {
        if (tolower((unsigned char)type_name[i]) != object_name[i]) return 0;
    }
    return 1;
}

static char *runtime_normalize_type_name(const char *type_name, size_t type_name_length) {
    size_t i;
    size_t out_length;
    char *normalized;
    size_t out_index;
    size_t start;

    if (!type_name) return 0;
    if (runtime_type_name_is_object_contract(type_name, type_name_length)) {
        return dup_runtime_name(".object", sizeof(".object") - 1u);
    }
    if (runtime_type_name_is_builtin(type_name, type_name_length)) {
        return dup_runtime_name(type_name, type_name_length);
    }

    start = (type_name_length > 0 && type_name[0] == '.') ? 1 : 0;
    normalized = rxvm_internal_alloc(type_name_length + 1);
    if (!normalized) return 0;

    out_index = 0;
    for (i = start; i < type_name_length; i++) {
        if (i + 1 < type_name_length &&
            ((type_name[i] == ':' && type_name[i + 1] == ':') ||
             (type_name[i] == '.' && type_name[i + 1] == '.'))) {
            normalized[out_index++] = '.';
            i++;
        } else {
            normalized[out_index++] = type_name[i];
        }
    }

    out_length = out_index;
    normalized[out_length] = 0;
    return normalized;
}

static char *runtime_internal_type_to_source_name(const char *type_name, size_t type_name_length) {
    size_t dots = 0;
    size_t i;
    char *source_name;
    size_t out_index;

    if (!type_name) return 0;
    if (runtime_type_name_is_builtin(type_name, type_name_length)) {
        return dup_runtime_name(type_name, type_name_length);
    }

    for (i = 0; i < type_name_length; i++) {
        if (type_name[i] == '.') dots++;
    }

    source_name = rxvm_internal_alloc(type_name_length + dots + 2);
    if (!source_name) return 0;

    out_index = 0;
    source_name[out_index++] = '.';
    for (i = 0; i < type_name_length; i++) {
        if (type_name[i] == '.') {
            source_name[out_index++] = '.';
            source_name[out_index++] = '.';
        } else {
            source_name[out_index++] = type_name[i];
        }
    }
    source_name[out_index] = 0;

    return source_name;
}

static const char *runtime_value_type_name(const value *object_value,
                                           size_t *type_name_length) {
    if (type_name_length) *type_name_length = 0u;
    if (!object_value || !object_value->object_type) return 0;
    if (type_name_length) {
        *type_name_length = object_value->object_type->name_length;
    }
    return object_value->object_type->name;
}

typedef enum runtime_contract_kind {
    RUNTIME_CONTRACT_UNKNOWN = 0,
    RUNTIME_CONTRACT_CLASS,
    RUNTIME_CONTRACT_INTERFACE
} runtime_contract_kind;

static runtime_contract_kind runtime_lookup_contract_kind(rxvm_context *context,
                                                          const char *type_name,
                                                          size_t type_name_length) {
    size_t mod_index;

    if (!context || !type_name || !type_name_length) return RUNTIME_CONTRACT_UNKNOWN;

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod = context->modules[mod_index];
        int meta_ix = mod->meta_head;

        while (meta_ix != -1) {
            meta_entry *meta = (meta_entry *) (mod->segment.const_pool + meta_ix);

            if (meta->base.type == META_CLASS || meta->base.type == META_INTERFACE) {
                size_t symbol_index = 0;
                string_constant *symbol_name;

                if (meta->base.type == META_CLASS) {
                    symbol_index = ((meta_class_constant *) meta)->symbol;
                } else {
                    symbol_index = ((meta_interface_constant *) meta)->symbol;
                }

                symbol_name = get_runtime_string_constant(mod, symbol_index);
                if (symbol_name &&
                    symbol_name->string_len == type_name_length &&
                    memcmp(symbol_name->string, type_name, type_name_length) == 0) {
                    return meta->base.type == META_CLASS ? RUNTIME_CONTRACT_CLASS : RUNTIME_CONTRACT_INTERFACE;
                }
            }

            meta_ix = meta->next;
        }
    }

    return RUNTIME_CONTRACT_UNKNOWN;
}

static int runtime_class_implements_interface(rxvm_context *context,
                                              const char *class_name,
                                              size_t class_name_length,
                                              const char *interface_name,
                                              size_t interface_name_length) {
    size_t mod_index;

    if (!context || !class_name || !interface_name) return 0;

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod = context->modules[mod_index];
        int meta_ix = mod->meta_head;

        while (meta_ix != -1) {
            meta_entry *meta = (meta_entry *) (mod->segment.const_pool + meta_ix);

            if (meta->base.type == META_IMPLEMENTS) {
                meta_implements_constant *impl_meta = (meta_implements_constant *) meta;
                string_constant *class_symbol = get_runtime_string_constant(mod, impl_meta->symbol);
                string_constant *interface_symbol = get_runtime_string_constant(mod, impl_meta->interface_symbol);

                if (class_symbol && interface_symbol &&
                    class_symbol->string_len == class_name_length &&
                    interface_symbol->string_len == interface_name_length &&
                    memcmp(class_symbol->string, class_name, class_name_length) == 0 &&
                    memcmp(interface_symbol->string, interface_name, interface_name_length) == 0) {
                    return 1;
                }
            }

            meta_ix = meta->next;
        }
    }

    return 0;
}

static int runtime_value_matches_object_type(rxvm_context *context,
                                             value *object_value,
                                             const char *type_name,
                                             size_t type_name_length) {
    char *normalized_type_name;
    const char *object_type_name;
    size_t object_type_name_length;
    runtime_contract_kind kind;
    int matches;

    if (!object_value || !type_name || !type_name_length) return 0;
    if (context->link_dirty || context->interface_method_registry_dirty || context->interface_factory_registry_dirty) {
        rxvm_link(context);
    }

    normalized_type_name = runtime_normalize_type_name(type_name, type_name_length);
    if (!normalized_type_name) return 0;

    if (runtime_type_name_is_object_contract(normalized_type_name, strlen(normalized_type_name))) {
        rxvm_internal_free(normalized_type_name);
        return object_value->object_type != 0;
    }

    if (runtime_type_name_is_builtin(normalized_type_name, strlen(normalized_type_name))) {
        rxvm_internal_free(normalized_type_name);
        return 0;
    }

    matches = 0;
    object_type_name = runtime_value_type_name(object_value,
                                               &object_type_name_length);
    if (object_type_name && object_type_name_length > 0) {
        kind = runtime_lookup_contract_kind(context, normalized_type_name, strlen(normalized_type_name));
        if (kind == RUNTIME_CONTRACT_INTERFACE) {
            matches = runtime_class_implements_interface(context,
                                                         object_type_name,
                                                         object_type_name_length,
                                                         normalized_type_name,
                                                         strlen(normalized_type_name));
        } else {
            matches = (object_type_name_length == strlen(normalized_type_name) &&
                       memcmp(object_type_name, normalized_type_name,
                              object_type_name_length) == 0);
        }
    }

    rxvm_internal_free(normalized_type_name);
    return matches;
}

static char *build_runtime_cast_error(value *object_value,
                                      const char *target_type_name,
                                      size_t target_type_name_length) {
    char *target_source_name;
    char *source_type_name;
    char *buffer;
    size_t buffer_length;
    const char *object_type_name;
    size_t object_type_name_length;

    target_source_name = runtime_internal_type_to_source_name(target_type_name, target_type_name_length);
    if (!target_source_name) return 0;

    object_type_name = runtime_value_type_name(object_value,
                                               &object_type_name_length);
    if (object_type_name && object_type_name_length > 0) {
        source_type_name = runtime_internal_type_to_source_name(object_type_name,
                                                                object_type_name_length);
    } else {
        source_type_name = dup_runtime_name(
                ".object", sizeof(".object") - 1u);
    }

    if (!source_type_name) {
        rxvm_internal_free(target_source_name);
        return 0;
    }

    buffer_length = strlen("Cannot cast ") + strlen(source_type_name) + strlen(" to ") + strlen(target_source_name) + 1;
    buffer = rxvm_internal_alloc(buffer_length);
    if (buffer) {
        snprintf(buffer, buffer_length, "Cannot cast %s to %s", source_type_name, target_source_name);
    }
    rxvm_internal_free(source_type_name);
    rxvm_internal_free(target_source_name);
    return buffer;
}

static char *build_runtime_uninitialized_object_error(value *object_value) {
    char *source_type_name;
    char *buffer;
    size_t buffer_length;
    const char *object_type_name;
    size_t object_type_name_length;

    object_type_name = runtime_value_type_name(object_value,
                                               &object_type_name_length);
    if (object_type_name && object_type_name_length > 0) {
        source_type_name = runtime_internal_type_to_source_name(object_type_name,
                                                                object_type_name_length);
    } else {
        source_type_name = dup_runtime_name(
                ".object", sizeof(".object") - 1u);
    }
    if (!source_type_name) return 0;

    buffer_length = strlen("Object ") + strlen(source_type_name) + strlen(" is not initialized") + 1;
    buffer = rxvm_internal_alloc(buffer_length);
    if (buffer) {
        snprintf(buffer, buffer_length, "Object %s is not initialized", source_type_name);
    }
    rxvm_internal_free(source_type_name);
    return buffer;
}

static void clear_runtime_interface_factories(rxvm_context *context) {
    size_t i;

    if (!context || !context->interface_factories) {
        if (context) {
            context->num_interface_factories = 0;
            context->interface_factory_capacity = 0;
        }
        return;
    }

    for (i = 0; i < context->num_interface_factories; i++) {
        rxvm_internal_free(context->interface_factories[i].interface_name);
        rxvm_internal_free(context->interface_factories[i].factory_name);
        rxvm_internal_free(context->interface_factories[i].descriptor);
        rx_sig_free(&context->interface_factories[i].signature);
        rxvm_internal_free(context->interface_factories[i].class_name);
    }

    (void)rxvm_memory_release(context->interface_factories);
    context->interface_factories = 0;
    context->num_interface_factories = 0;
    context->interface_factory_capacity = 0;
}

static void clear_runtime_interface_methods(rxvm_context *context) {
    size_t i;

    if (!context || !context->interface_methods) {
        if (context) {
            context->num_interface_methods = 0;
            context->interface_method_capacity = 0;
        }
        return;
    }

    for (i = 0; i < context->num_interface_methods; i++) {
        rxvm_internal_free(context->interface_methods[i].class_name);
        rxvm_internal_free(context->interface_methods[i].descriptor);
    }

    (void)rxvm_memory_release(context->interface_methods);
    context->interface_methods = 0;
    context->num_interface_methods = 0;
    context->interface_method_capacity = 0;
}

static int compare_runtime_interface_method_entries(const void *left_value,
                                                    const void *right_value) {
    const rxvm_interface_method_entry *left = left_value;
    const rxvm_interface_method_entry *right = right_value;
    int result;

    result = compare_runtime_name(left->class_name, left->class_name_length,
                                  right->class_name, right->class_name_length);
    if (result) return result;
    return compare_runtime_name(left->descriptor, left->descriptor_length,
                                right->descriptor, right->descriptor_length);
}

static int compare_runtime_interface_method_key(const char *class_name,
                                                size_t class_name_length,
                                                const char *descriptor,
                                                size_t descriptor_length,
                                                const rxvm_interface_method_entry *entry) {
    int result;

    result = compare_runtime_name(class_name, class_name_length,
                                  entry->class_name, entry->class_name_length);
    if (result) return result;
    return compare_runtime_name(descriptor, descriptor_length,
                                entry->descriptor, entry->descriptor_length);
}

static int compare_runtime_interface_factory_entries(const void *left_value,
                                                     const void *right_value) {
    const rxvm_interface_factory_entry *left = left_value;
    const rxvm_interface_factory_entry *right = right_value;
    int result;

    result = compare_runtime_name(left->interface_name, left->interface_name_length,
                                  right->interface_name, right->interface_name_length);
    if (result) return result;
    result = compare_runtime_name(left->factory_name, left->factory_name_length,
                                  right->factory_name, right->factory_name_length);
    if (result) return result;
    result = compare_runtime_name(left->class_name, left->class_name_length,
                                  right->class_name, right->class_name_length);
    if (result) return result;
    return compare_runtime_name(left->descriptor, left->descriptor_length,
                                right->descriptor, right->descriptor_length);
}

static int compare_runtime_interface_factory_key(const char *interface_name,
                                                 size_t interface_name_length,
                                                 const char *factory_name,
                                                 size_t factory_name_length,
                                                 const rxvm_interface_factory_entry *entry) {
    int result;

    result = compare_runtime_name(interface_name, interface_name_length,
                                  entry->interface_name, entry->interface_name_length);
    if (result) return result;
    return compare_runtime_name(factory_name, factory_name_length,
                                entry->factory_name, entry->factory_name_length);
}

static int runtime_member_kind_is_method(const string_constant *kind_symbol) {
    if (!kind_symbol || kind_symbol->string_len < 6) return 0;
    return memcmp(kind_symbol->string, "method", 6) == 0;
}

static int runtime_member_kind_is_final(const string_constant *kind_symbol) {
    static const char final_flag[] = "final";
    size_t i;

    if (!runtime_member_kind_is_method(kind_symbol)) return 0;
    if (kind_symbol->string_len < sizeof(final_flag) - 1) return 0;

    for (i = 0; i + (sizeof(final_flag) - 1) <= kind_symbol->string_len; i++) {
        if (memcmp(kind_symbol->string + i, final_flag, sizeof(final_flag) - 1) == 0) {
            return 1;
        }
    }

    return 0;
}

static string_constant *get_runtime_string_constant(module *mod, size_t offset) {
    string_constant *entry;

    if (!mod || offset >= mod->segment.const_size) return 0;
    entry = (string_constant *) (mod->segment.const_pool + offset);
    if (entry->base.type != STRING_CONST) return 0;
    return entry;
}

static char *runtime_metadata_type_to_contract_name(const char *type_name) {
    size_t in_index;
    size_t out_index;
    size_t length;
    char *normalized;

    if (!type_name || !*type_name || type_name[0] != '.') return 0;

    length = strlen(type_name);
    normalized = rxvm_internal_alloc(length + 1);
    if (!normalized) return 0;

    out_index = 0;
    for (in_index = 1; in_index < length; in_index++) {
        if (type_name[in_index] == '.' &&
            in_index + 1 < length &&
            type_name[in_index + 1] == '.') {
            normalized[out_index++] = '.';
            in_index++;
        } else {
            normalized[out_index++] = type_name[in_index];
        }
    }
    normalized[out_index] = 0;

    return normalized;
}

static int runtime_signature_type_assignable(void *userdata,
                                             const char *actual_type,
                                             const char *expected_type) {
    rxvm_context *context;
    char *actual_contract;
    char *expected_contract;
    int result;

    context = (rxvm_context *)userdata;
    if (!context || !actual_type || !expected_type) return 0;
    if (actual_type[0] == '.' && strcmp(actual_type, expected_type) == 0) return 1;

    actual_contract = runtime_metadata_type_to_contract_name(actual_type);
    expected_contract = runtime_metadata_type_to_contract_name(expected_type);
    if (!actual_contract || !expected_contract) {
        if (actual_contract) rxvm_internal_free(actual_contract);
        if (expected_contract) rxvm_internal_free(expected_contract);
        return 0;
    }

    if (strcmp(actual_contract, expected_contract) == 0) {
        result = 1;
    } else {
        result = runtime_class_implements_interface(context,
                                                    actual_contract,
                                                    strlen(actual_contract),
                                                    expected_contract,
                                                    strlen(expected_contract));
    }
    rxvm_internal_free(actual_contract);
    rxvm_internal_free(expected_contract);
    return result;
}

static int runtime_type_matches_contract_name(const char *type_name,
                                              const char *contract_name,
                                              size_t contract_name_length) {
    char *normalized;
    const char *short_name;
    size_t normalized_length;
    size_t short_name_length;
    size_t i;
    int matches;

    if (!type_name || !contract_name || !contract_name_length) return 0;

    normalized = runtime_metadata_type_to_contract_name(type_name);
    if (!normalized) return 0;

    normalized_length = strlen(normalized);
    matches = normalized_length == contract_name_length &&
              memcmp(normalized, contract_name, contract_name_length) == 0;
    if (!matches) {
        short_name = 0;
        for (i = contract_name_length; i > 0; i--) {
            if (contract_name[i - 1] == '.') {
                short_name = contract_name + i;
                break;
            }
        }
        if (short_name && *short_name) {
            short_name_length = (size_t) ((contract_name + contract_name_length) - short_name);
            matches = normalized_length == short_name_length &&
                      memcmp(normalized, short_name, short_name_length) == 0;
        }
    }

    rxvm_internal_free(normalized);
    return matches;
}

static int runtime_proc_matches_signature(rxvm_context *context,
                                          proc_runtime *proc,
                                          const rx_callable_signature *expected) {
    size_t mod_index;
    rx_callable_compare_options options;

    if (!context || !proc || !expected) return 0;

    memset(&options, 0, sizeof(options));
    options.allow_return_covariance = 1;
    options.type_assignable = runtime_signature_type_assignable;
    options.userdata = context;

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod;
        int meta_ix;

        mod = context->modules[mod_index];
        meta_ix = mod->meta_head;
        while (meta_ix != -1) {
            meta_entry *meta;

            meta = (meta_entry *) (mod->segment.const_pool + meta_ix);
            if (meta->base.type == META_FUNC) {
                meta_func_constant *func_meta;
                proc_runtime *meta_proc;

                func_meta = (meta_func_constant *) meta;
                meta_proc = rxvm_get_module_runtime_procedure(mod, func_meta->func);
                if (meta_proc == proc) {
                    string_constant *type_symbol;
                    string_constant *args_symbol;
                    rx_callable_signature actual;
                    int matches;

                    type_symbol = get_runtime_string_constant(mod, func_meta->type);
                    args_symbol = get_runtime_string_constant(mod, func_meta->args);
                    if (!type_symbol || !args_symbol) return 0;

                    if (!rx_sig_init_from_parts(&actual,
                                                expected->name ? expected->name : "",
                                                type_symbol->string,
                                                args_symbol->string)) {
                        return 0;
                    }
                    matches = rx_sig_matches_contract(expected, &actual, &options);
                    rx_sig_free(&actual);
                    return matches;
                }
            }
            meta_ix = meta->next;
        }
    }

    return 0;
}

static char *build_runtime_member_descriptor(const string_constant *member_symbol,
                                             const string_constant *type_symbol,
                                             const string_constant *args_symbol) {
    if (!member_symbol || !type_symbol || !args_symbol) return 0;
    return rx_sig_build_descriptor(member_symbol->string,
                                   type_symbol->string,
                                   args_symbol->string);
}

static char *build_runtime_factory_selector_name(const string_constant *interface_symbol,
                                                 const string_constant *factory_symbol) {
    if (!interface_symbol || !factory_symbol) return 0;
    if (factory_symbol->string_len == 1 && factory_symbol->string[0] == '*') {
        return dup_runtime_name(interface_symbol->string, interface_symbol->string_len);
    }
    {
        char *selector;
        selector = rxvm_internal_alloc(
                interface_symbol->string_len + 2 +
                factory_symbol->string_len + 1);
        if (!selector) return 0;
        memcpy(selector, interface_symbol->string, interface_symbol->string_len);
        selector[interface_symbol->string_len] = '.';
        selector[interface_symbol->string_len + 1] = '.';
        memcpy(selector + interface_symbol->string_len + 2,
               factory_symbol->string,
               factory_symbol->string_len);
        selector[interface_symbol->string_len + 2 + factory_symbol->string_len] = 0;
        return selector;
    }
}

static char *build_runtime_factory_descriptor(const string_constant *interface_symbol,
                                              const string_constant *factory_symbol,
                                              const string_constant *type_symbol,
                                              const string_constant *args_symbol) {
    char *selector;
    char *descriptor;

    if (!interface_symbol || !factory_symbol || !type_symbol || !args_symbol) return 0;

    selector = build_runtime_factory_selector_name(interface_symbol, factory_symbol);
    if (!selector) return 0;
    descriptor = rx_sig_build_descriptor(selector, type_symbol->string, args_symbol->string);
    rxvm_internal_free(selector);
    return descriptor;
}

/*
 * R2a keeps the canonical LINKATTR1/COPY/UNLINK RXBIN stream immutable and
 * selects a process-private execution handler only for this exact shape:
 *
 *   linkattr1 temporary, object, immediate_attribute
 *   copy      destination, temporary
 *   unlink    temporary
 *
 * The private handler still checks the runtime payload before taking the
 * reference-descriptor path.  This recognizer is deliberately structural and
 * makes no source-name, procedure-name or benchmark assumption.
 */
enum {
    RXVM_PRIVATE_R2_COPYATTR1_REG_REG_INT = OP_MAX_INSTRUCTIONS,
    RXVM_PRIVATE_R1_RELINK_REG_REG
};

static int rxvm_private_r2_copyattr1_candidate(const module *mod,
                                               size_t instruction_index) {
    const bin_code *code;
    size_t temporary;
    size_t object;
    size_t destination;

    if (!mod || !mod->segment.binary ||
            instruction_index > mod->segment.inst_size ||
            mod->segment.inst_size - instruction_index < 9u)
        return 0;

    code = mod->segment.binary + instruction_index;
    if (code[0].instruction.opcode != OP_LINKATTR1_REG_REG_INT ||
            code[0].instruction.no_ops != 3 ||
            code[4].instruction.opcode != OP_COPY_REG_REG ||
            code[4].instruction.no_ops != 2 ||
            code[7].instruction.opcode != OP_UNLINK_REG ||
            code[7].instruction.no_ops != 1)
        return 0;

    temporary = code[1].index;
    object = code[2].index;
    destination = code[5].index;
    return code[6].index == temporary && code[8].index == temporary &&
           temporary != object && temporary != destination &&
           object != destination;
}

/*
 * R1a recognizes only the exact canonical local-relink shape:
 *
 *   unlink  destination
 *   linkref destination, source_reference
 *
 * The source must be distinct so unlinking the destination cannot change the
 * value subsequently validated by LINKREF.  The serialized instruction stream
 * remains canonical; only the process-local execution image is specialized.
 */
static int rxvm_private_r1_relink_candidate(const module *mod,
                                             size_t instruction_index) {
    const bin_code *code;
    size_t destination;

    if (!mod || !mod->segment.binary ||
            instruction_index > mod->segment.inst_size ||
            mod->segment.inst_size - instruction_index < 5u)
        return 0;

    code = mod->segment.binary + instruction_index;
    if (code[0].instruction.opcode != OP_UNLINK_REG ||
            code[0].instruction.no_ops != 1 ||
            code[2].instruction.opcode != OP_LINKREF_REG_REG ||
            code[2].instruction.no_ops != 2)
        return 0;

    destination = code[1].index;
    return code[3].index == destination && code[4].index != destination;
}

typedef struct rxvm_source_context {
    string_constant *file;
    string_constant *source;
    size_t line;
    size_t column;
    size_t active_end_column;
    uint32_t step_id;
    uint32_t clause_id;
    uint32_t flags;
} rxvm_source_context;

static void resolve_runtime_source_context(module *mod, size_t address, rxvm_source_context *source_context) {
    int meta_ix;

    source_context->file = 0;
    source_context->source = 0;
    source_context->line = 0;
    source_context->column = 0;
    source_context->active_end_column = 0;
    source_context->step_id = 0;
    source_context->clause_id = 0;
    source_context->flags = 0;

    if (!mod) return;

    meta_ix = mod->meta_head;
    while (meta_ix != -1) {
        meta_entry *meta = (meta_entry *) (mod->segment.const_pool + meta_ix);
        if (meta->address > address) break;

        if (meta->base.type == META_SOURCE_STEP) {
            meta_source_step_constant *step_meta = (meta_source_step_constant *) meta;
            source_context->file = get_runtime_string_constant(mod, step_meta->file);
            source_context->source = get_runtime_string_constant(mod, step_meta->source_line);
            source_context->line = step_meta->line;
            source_context->column = step_meta->active_start_column;
            source_context->active_end_column = step_meta->active_end_column;
            source_context->step_id = step_meta->step_id;
            source_context->clause_id = step_meta->clause_id;
            source_context->flags = step_meta->flags;
        }

        meta_ix = meta->next;
    }
}

static void print_runtime_panic_location(rxvm_context *context, rxinteger module_number, rxinteger address) {
    module *mod;
    rxvm_source_context source_context;
    size_t module_index;
    size_t instruction_address;

    if (!context || module_number <= 0 || address < 0) return;

    module_index = (size_t) module_number - 1;
    if (module_index >= context->num_modules) return;

    mod = context->modules[module_index];
    instruction_address = (size_t) address;

    fprintf(stderr, "  at module %zu", (size_t) module_number);
    if (mod && mod->name) fprintf(stderr, " (%s)", mod->name);
    fprintf(stderr, " address %zu (0x%zx)\n", instruction_address, instruction_address);

    resolve_runtime_source_context(mod, instruction_address, &source_context);
    if (source_context.source) {
        fprintf(stderr, "  source: ");
        if (source_context.file) {
            fprintf(stderr, "%.*s:", (int) source_context.file->string_len, source_context.file->string);
        }
        fprintf(stderr, "%zu:%zu: %.*s\n",
                source_context.line,
                source_context.column,
                (int) source_context.source->string_len,
                source_context.source->string);
    }
}

static char *build_runtime_factory_proc_name(const char *class_name,
                                             size_t class_name_length,
                                             const char *factory_name,
                                             size_t factory_name_length) {
    static const char default_factory_name[] = "\xc2\xa7" "factory";
    static const char factory_prefix[] = "\xc2\xa7" "factory.";
    size_t prefix_length;
    char *proc_name;

    if (!class_name || !class_name_length || !factory_name || !factory_name_length) return 0;

    if (factory_name_length == 1 && factory_name[0] == '*') {
        return build_runtime_member_name(class_name, class_name_length,
                                         default_factory_name, sizeof(default_factory_name) - 1);
    }

    prefix_length = sizeof(factory_prefix) - 1;
    proc_name = rxvm_internal_alloc(
            class_name_length + 1 + prefix_length + factory_name_length + 1);
    if (!proc_name) return 0;

    memcpy(proc_name, class_name, class_name_length);
    proc_name[class_name_length] = '.';
    memcpy(proc_name + class_name_length + 1, factory_prefix, prefix_length);
    memcpy(proc_name + class_name_length + 1 + prefix_length, factory_name, factory_name_length);
    proc_name[class_name_length + 1 + prefix_length + factory_name_length] = 0;

    return proc_name;
}

static char *build_runtime_match_proc_name(const char *class_name,
                                           size_t class_name_length,
                                           const char *factory_name,
                                           size_t factory_name_length) {
    static const char default_match_name[] = "\xc2\xa7" "match";
    static const char match_prefix[] = "\xc2\xa7" "match.";
    size_t prefix_length;
    char *proc_name;

    if (!class_name || !class_name_length || !factory_name || !factory_name_length) return 0;

    if (factory_name_length == 1 && factory_name[0] == '*') {
        return build_runtime_member_name(class_name, class_name_length,
                                         default_match_name, sizeof(default_match_name) - 1);
    }

    prefix_length = sizeof(match_prefix) - 1;
    proc_name = rxvm_internal_alloc(
            class_name_length + 1 + prefix_length + factory_name_length + 1);
    if (!proc_name) return 0;

    memcpy(proc_name, class_name, class_name_length);
    proc_name[class_name_length] = '.';
    memcpy(proc_name + class_name_length + 1, match_prefix, prefix_length);
    memcpy(proc_name + class_name_length + 1 + prefix_length, factory_name, factory_name_length);
    proc_name[class_name_length + 1 + prefix_length + factory_name_length] = 0;

    return proc_name;
}

static int invoke_runtime_factory_match(rxvm_context *context,
                                        proc_runtime *match_proc,
                                        rxinteger argc,
                                        value **args,
                                        rxinteger *score_out) {
    proc_runtime *saved_ext_proc;
    int saved_ext_argc;
    value **saved_ext_args;
    value *saved_ext_ret;
    value *match_ret;
    char *dummy_argv[] = {"rxvm_factory_match"};

    if (score_out) *score_out = 1;
    if (!context || !match_proc) return 1;

    saved_ext_proc = context->ext_proc;
    saved_ext_argc = context->ext_argc;
    saved_ext_args = context->ext_args;
    saved_ext_ret = context->ext_ret;

    context->ext_proc = match_proc;
    context->ext_argc = (int) argc;
    context->ext_args = args;
    context->ext_ret = value_f_in(context->worker.memory_worker);
    match_ret = context->ext_ret;
    if (!match_ret) {
        context->ext_proc = saved_ext_proc;
        context->ext_argc = saved_ext_argc;
        context->ext_args = saved_ext_args;
        context->ext_ret = saved_ext_ret;
        return 0;
    }

    run(context, 0, dummy_argv);
    if (score_out) *score_out = match_ret->int_value;

    value_free(match_ret);

    context->ext_proc = saved_ext_proc;
    context->ext_argc = saved_ext_argc;
    context->ext_args = saved_ext_args;
    context->ext_ret = saved_ext_ret;

    return 1;
}

static int add_runtime_interface_factory_entry(rxvm_context *context,
                                               const char *interface_name,
                                               size_t interface_name_length,
                                               const char *factory_name,
                                               size_t factory_name_length,
                                               const char *descriptor,
                                               size_t descriptor_length,
                                               const char *class_name,
                                               size_t class_name_length,
                                               proc_runtime *match_proc,
                                               proc_runtime *factory_proc) {
    rxvm_interface_factory_entry *entry;

    if (!context || !interface_name || !factory_name || !descriptor || !class_name || !factory_proc) return 0;

    if (context->num_interface_factories >= context->interface_factory_capacity) {
        size_t new_capacity;
        rxvm_interface_factory_entry *new_entries;

        new_capacity = context->interface_factory_capacity ? context->interface_factory_capacity * 2 : 16;
        new_entries = rxvm_memory_resize_bytes(
                context->worker.memory_worker, context->interface_factories,
                sizeof(rxvm_interface_factory_entry) *
                    context->num_interface_factories,
                sizeof(rxvm_interface_factory_entry) * new_capacity);
        if (!new_entries) return 0;

        context->interface_factories = new_entries;
        context->interface_factory_capacity = new_capacity;
    }

    entry = &context->interface_factories[context->num_interface_factories];
    memset(entry, 0, sizeof(*entry));

    entry->interface_name = dup_runtime_name(interface_name, interface_name_length);
    entry->factory_name = dup_runtime_name(factory_name, factory_name_length);
    entry->descriptor = dup_runtime_name(descriptor, descriptor_length);
    entry->class_name = dup_runtime_name(class_name, class_name_length);
    if (!entry->interface_name || !entry->factory_name || !entry->descriptor || !entry->class_name ||
        !rx_sig_parse_descriptor(entry->descriptor, &entry->signature)) {
        rxvm_internal_free(entry->interface_name);
        rxvm_internal_free(entry->factory_name);
        rxvm_internal_free(entry->descriptor);
        rx_sig_free(&entry->signature);
        rxvm_internal_free(entry->class_name);
        memset(entry, 0, sizeof(*entry));
        return 0;
    }

    entry->interface_name_length = interface_name_length;
    entry->factory_name_length = factory_name_length;
    entry->descriptor_length = descriptor_length;
    entry->class_name_length = class_name_length;
    entry->match_proc = match_proc;
    entry->factory_proc = factory_proc;
    context->num_interface_factories++;

    return 1;
}

static int add_runtime_interface_method_entry(rxvm_context *context,
                                              const char *class_name,
                                              size_t class_name_length,
                                              const char *descriptor,
                                              size_t descriptor_length,
                                              proc_runtime *method_proc) {
    rxvm_interface_method_entry *entry;
    size_t i;

    if (!context || !class_name || !descriptor || !method_proc) return 0;

    for (i = 0; i < context->num_interface_methods; i++) {
        entry = &context->interface_methods[i];
        if (entry->class_name_length == class_name_length &&
            entry->descriptor_length == descriptor_length &&
            memcmp(entry->class_name, class_name, class_name_length) == 0 &&
            memcmp(entry->descriptor, descriptor, descriptor_length) == 0) {
            return entry->method_proc == method_proc;
        }
    }

    if (context->num_interface_methods >= context->interface_method_capacity) {
        size_t new_capacity;
        rxvm_interface_method_entry *new_entries;

        new_capacity = context->interface_method_capacity ? context->interface_method_capacity * 2 : 32;
        new_entries = rxvm_memory_resize_bytes(
                context->worker.memory_worker, context->interface_methods,
                sizeof(rxvm_interface_method_entry) *
                    context->num_interface_methods,
                sizeof(rxvm_interface_method_entry) * new_capacity);
        if (!new_entries) return 0;

        context->interface_methods = new_entries;
        context->interface_method_capacity = new_capacity;
    }

    entry = &context->interface_methods[context->num_interface_methods];
    memset(entry, 0, sizeof(*entry));

    entry->class_name = dup_runtime_name(class_name, class_name_length);
    entry->descriptor = dup_runtime_name(descriptor, descriptor_length);
    if (!entry->class_name || !entry->descriptor) {
        rxvm_internal_free(entry->class_name);
        rxvm_internal_free(entry->descriptor);
        memset(entry, 0, sizeof(*entry));
        return 0;
    }

    entry->class_name_length = class_name_length;
    entry->descriptor_length = descriptor_length;
    entry->method_proc = method_proc;
    context->num_interface_methods++;

    return 1;
}

void rxvm_rebuild_interface_method_registry(rxvm_context *context) {
    size_t mod_index;

    if (!context) return;

    clear_runtime_interface_methods(context);

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod;
        int meta_ix;

        mod = context->modules[mod_index];
        meta_ix = mod->meta_head;

        while (meta_ix != -1) {
            meta_entry *meta;

            meta = (meta_entry *) (mod->segment.const_pool + meta_ix);
            if (meta->base.type == META_IMPLEMENTS) {
                meta_implements_constant *impl_meta;
                string_constant *class_symbol;
                string_constant *interface_symbol;
                size_t iface_mod_index;

                impl_meta = (meta_implements_constant *) meta;
                class_symbol = get_runtime_string_constant(mod, impl_meta->symbol);
                interface_symbol = get_runtime_string_constant(mod, impl_meta->interface_symbol);

                if (!class_symbol || !interface_symbol) {
                    meta_ix = meta->next;
                    continue;
                }

                for (iface_mod_index = 0; iface_mod_index < context->num_modules; iface_mod_index++) {
                    module *iface_mod;
                    int iface_meta_ix;

                    iface_mod = context->modules[iface_mod_index];
                    iface_meta_ix = iface_mod->meta_head;

                    while (iface_meta_ix != -1) {
                        meta_entry *iface_meta;

                        iface_meta = (meta_entry *) (iface_mod->segment.const_pool + iface_meta_ix);
                        if (iface_meta->base.type == META_MEMBER) {
                            meta_member_constant *member_meta;
                            string_constant *owner_symbol;
                            string_constant *kind_symbol;
                            string_constant *member_symbol;
                            string_constant *type_symbol;
                            string_constant *args_symbol;

                            member_meta = (meta_member_constant *) iface_meta;
                            owner_symbol = get_runtime_string_constant(iface_mod, member_meta->owner);
                            kind_symbol = get_runtime_string_constant(iface_mod, member_meta->kind);
                            member_symbol = get_runtime_string_constant(iface_mod, member_meta->member);
                            type_symbol = get_runtime_string_constant(iface_mod, member_meta->type);
                            args_symbol = get_runtime_string_constant(iface_mod, member_meta->args);

                            if (owner_symbol && kind_symbol && member_symbol && type_symbol && args_symbol &&
                                runtime_member_kind_is_method(kind_symbol) &&
                                owner_symbol->string_len == interface_symbol->string_len &&
                                memcmp(owner_symbol->string, interface_symbol->string, interface_symbol->string_len) == 0) {
                                char *class_proc_name;
                                char *interface_proc_name;
                                char *descriptor;
                                proc_runtime *class_proc;
                                proc_runtime *interface_proc;
                                proc_runtime *effective_proc;
                                rx_callable_signature expected_signature;

                                class_proc_name = build_runtime_member_name(class_symbol->string,
                                                                            class_symbol->string_len,
                                                                            member_symbol->string,
                                                                            member_symbol->string_len);
                                interface_proc_name = build_runtime_member_name(interface_symbol->string,
                                                                                interface_symbol->string_len,
                                                                                member_symbol->string,
                                                                                member_symbol->string_len);
                                if (!class_proc_name || !interface_proc_name) {
                                    if (class_proc_name)
                                        rxvm_internal_free(class_proc_name);
                                    if (interface_proc_name)
                                        rxvm_internal_free(interface_proc_name);
                                    iface_meta_ix = iface_meta->next;
                                    continue;
                                }

                                descriptor = build_runtime_member_descriptor(member_symbol, type_symbol, args_symbol);
                                if (!descriptor ||
                                    !rx_sig_init_from_parts(&expected_signature,
                                                            member_symbol->string,
                                                            type_symbol->string,
                                                            args_symbol->string)) {
                                    if (class_proc_name)
                                        rxvm_internal_free(class_proc_name);
                                    if (interface_proc_name)
                                        rxvm_internal_free(interface_proc_name);
                                    if (descriptor) free(descriptor);
                                    iface_meta_ix = iface_meta->next;
                                    continue;
                                }

                                class_proc = resolve_runtime_procedure(context, class_proc_name, strlen(class_proc_name));
                                interface_proc = resolve_runtime_procedure(context, interface_proc_name, strlen(interface_proc_name));
                                rxvm_internal_free(class_proc_name);
                                rxvm_internal_free(interface_proc_name);

                                effective_proc = class_proc;
                                if (!effective_proc && runtime_member_kind_is_final(kind_symbol)) {
                                    effective_proc = interface_proc;
                                }

                                if (effective_proc &&
                                    runtime_proc_matches_signature(context, effective_proc, &expected_signature)) {
                                    add_runtime_interface_method_entry(context,
                                                                       class_symbol->string,
                                                                       class_symbol->string_len,
                                                                       descriptor,
                                                                       strlen(descriptor),
                                                                       effective_proc);
                                }
                                rx_sig_free(&expected_signature);
                                free(descriptor);
                            }
                        }

                        iface_meta_ix = iface_meta->next;
                    }
                }
            }

            meta_ix = meta->next;
        }
    }

    if (context->num_interface_methods > 1) {
        qsort(context->interface_methods,
              context->num_interface_methods,
              sizeof(*context->interface_methods),
              compare_runtime_interface_method_entries);
    }
}

static proc_runtime *resolve_runtime_method(rxvm_context *context,
                                            const char *class_name,
                                            size_t class_name_length,
                                            const char *descriptor,
                                            size_t descriptor_length) {
    size_t lower;
    size_t upper;
    char *proc_name;
    proc_runtime *called_function;
    char *descriptor_text;
    rx_callable_signature expected_signature;

    if (!context || !class_name || !class_name_length || !descriptor || !descriptor_length) return 0;

    if (context->link_dirty || context->interface_method_registry_dirty || context->interface_factory_registry_dirty) {
        rxvm_link(context);
    }

    lower = 0;
    upper = context->num_interface_methods;
    while (lower < upper) {
        size_t middle = lower + (upper - lower) / 2;
        rxvm_interface_method_entry *entry = &context->interface_methods[middle];
        int comparison = compare_runtime_interface_method_key(
                class_name, class_name_length, descriptor, descriptor_length, entry);

        if (comparison < 0) upper = middle;
        else if (comparison > 0) lower = middle + 1;
        else {
            return entry->method_proc;
        }
    }

    descriptor_text = dup_runtime_name(descriptor, descriptor_length);
    if (!descriptor_text) return 0;
    if (!rx_sig_parse_descriptor(descriptor_text, &expected_signature)) {
        rxvm_internal_free(descriptor_text);
        return 0;
    }

    proc_name = build_runtime_member_name(class_name,
                                          class_name_length,
                                          expected_signature.name,
                                          strlen(expected_signature.name));
    if (!proc_name) {
        rx_sig_free(&expected_signature);
        rxvm_internal_free(descriptor_text);
        return 0;
    }
    called_function = resolve_runtime_procedure(context, proc_name, strlen(proc_name));
    rxvm_internal_free(proc_name);
    if (called_function &&
        !runtime_proc_matches_signature(context, called_function, &expected_signature)) {
        called_function = 0;
    }
    rx_sig_free(&expected_signature);
    rxvm_internal_free(descriptor_text);
    return called_function;
}

void rxvm_rebuild_interface_factory_registry(rxvm_context *context) {
    size_t mod_index;

    if (!context) return;

    clear_runtime_interface_factories(context);

    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        module *mod;
        int meta_ix;

        mod = context->modules[mod_index];
        meta_ix = mod->meta_head;

        while (meta_ix != -1) {
            meta_entry *meta;

            meta = (meta_entry *) (mod->segment.const_pool + meta_ix);
            if (meta->base.type == META_IMPLEMENTS) {
                meta_implements_constant *impl_meta;
                string_constant *class_symbol;
                string_constant *interface_symbol;
                size_t iface_mod_index;

                impl_meta = (meta_implements_constant *) meta;
                class_symbol = get_runtime_string_constant(mod, impl_meta->symbol);
                interface_symbol = get_runtime_string_constant(mod, impl_meta->interface_symbol);

                if (!class_symbol || !interface_symbol) {
                    meta_ix = meta->next;
                    continue;
                }

                for (iface_mod_index = 0; iface_mod_index < context->num_modules; iface_mod_index++) {
                    module *iface_mod;
                    int iface_meta_ix;

                    iface_mod = context->modules[iface_mod_index];
                    iface_meta_ix = iface_mod->meta_head;

                    while (iface_meta_ix != -1) {
                        meta_entry *iface_meta;

                        iface_meta = (meta_entry *) (iface_mod->segment.const_pool + iface_meta_ix);
                        if (iface_meta->base.type == META_MEMBER) {
                            meta_member_constant *member_meta;
                            string_constant *owner_symbol;
                            string_constant *kind_symbol;
                            string_constant *member_symbol;
                            string_constant *type_symbol;
                            string_constant *args_symbol;

                            member_meta = (meta_member_constant *) iface_meta;
                            owner_symbol = get_runtime_string_constant(iface_mod, member_meta->owner);
                            kind_symbol = get_runtime_string_constant(iface_mod, member_meta->kind);
                            member_symbol = get_runtime_string_constant(iface_mod, member_meta->member);
                            type_symbol = get_runtime_string_constant(iface_mod, member_meta->type);
                            args_symbol = get_runtime_string_constant(iface_mod, member_meta->args);

                            if (owner_symbol && kind_symbol && member_symbol && type_symbol && args_symbol &&
                                kind_symbol->string_len == 7 &&
                                memcmp(kind_symbol->string, "factory", 7) == 0 &&
                                owner_symbol->string_len == interface_symbol->string_len &&
                                memcmp(owner_symbol->string, interface_symbol->string, interface_symbol->string_len) == 0) {
                                char *factory_proc_name;
                                char *match_proc_name;
                                char *selector_name;
                                char *descriptor;
                                proc_runtime *factory_proc;
                                proc_runtime *match_proc;
                                rx_callable_signature expected_factory;
                                rx_callable_signature expected_match;
                                int factory_signature_ready;
                                int match_signature_ready;

                                factory_proc_name = build_runtime_factory_proc_name(class_symbol->string,
                                                                                    class_symbol->string_len,
                                                                                    member_symbol->string,
                                                                                    member_symbol->string_len);
                                match_proc_name = build_runtime_match_proc_name(class_symbol->string,
                                                                               class_symbol->string_len,
                                                                               member_symbol->string,
                                                                               member_symbol->string_len);
                                if (!factory_proc_name) {
                                    if (match_proc_name)
                                        rxvm_internal_free(match_proc_name);
                                    iface_meta_ix = iface_meta->next;
                                    continue;
                                }

                                descriptor = build_runtime_factory_descriptor(interface_symbol,
                                                                              member_symbol,
                                                                              type_symbol,
                                                                              args_symbol);
                                selector_name = build_runtime_factory_selector_name(interface_symbol,
                                                                                   member_symbol);
                                rx_sig_init_empty(&expected_factory);
                                rx_sig_init_empty(&expected_match);
                                factory_signature_ready = descriptor && selector_name &&
                                                          rx_sig_init_from_parts(&expected_factory,
                                                                                 selector_name,
                                                                                 type_symbol->string,
                                                                                 args_symbol->string);
                                match_signature_ready = factory_signature_ready &&
                                                        rx_sig_init_from_parts(&expected_match,
                                                                               "",
                                                                               ".int",
                                                                               args_symbol->string);
                                if (!descriptor || !factory_signature_ready || !match_signature_ready) {
                                    rx_sig_free(&expected_factory);
                                    rx_sig_free(&expected_match);
                                    if (descriptor) free(descriptor);
                                    if (selector_name)
                                        rxvm_internal_free(selector_name);
                                    rxvm_internal_free(factory_proc_name);
                                    if (match_proc_name)
                                        rxvm_internal_free(match_proc_name);
                                    iface_meta_ix = iface_meta->next;
                                    continue;
                                }

                                factory_proc = resolve_runtime_procedure(context, factory_proc_name, strlen(factory_proc_name));
                                match_proc = match_proc_name ?
                                             resolve_runtime_procedure(context, match_proc_name, strlen(match_proc_name)) :
                                             0;
                                rxvm_internal_free(factory_proc_name);
                                if (match_proc_name)
                                    rxvm_internal_free(match_proc_name);

                                if (factory_proc &&
                                    runtime_proc_matches_signature(context, factory_proc, &expected_factory) &&
                                    (!match_proc || runtime_proc_matches_signature(context, match_proc, &expected_match))) {
                                    add_runtime_interface_factory_entry(context,
                                                                        interface_symbol->string,
                                                                        interface_symbol->string_len,
                                                                        member_symbol->string,
                                                                        member_symbol->string_len,
                                                                        descriptor,
                                                                        strlen(descriptor),
                                                                        class_symbol->string,
                                                                        class_symbol->string_len,
                                                                        match_proc,
                                                                        factory_proc);
                                }
                                rx_sig_free(&expected_factory);
                                rx_sig_free(&expected_match);
                                rxvm_internal_free(selector_name);
                                free(descriptor);
                            }
                        }

                        iface_meta_ix = iface_meta->next;
                    }
                }
            }

            meta_ix = meta->next;
        }
    }

    if (context->num_interface_factories > 1) {
        qsort(context->interface_factories,
              context->num_interface_factories,
              sizeof(*context->interface_factories),
              compare_runtime_interface_factory_entries);
    }
}

static void parse_runtime_factory_selector(const char *selector,
                                           size_t selector_length,
                                           const char **interface_name,
                                           size_t *interface_name_length,
                                           const char **factory_name,
                                           size_t *factory_name_length) {
    const char *selector_start;
    const char *selector_end;
    const char *sep;

    if (interface_name) *interface_name = selector;
    if (interface_name_length) *interface_name_length = selector_length;
    if (factory_name) *factory_name = "*";
    if (factory_name_length) *factory_name_length = 1;

    if (!selector || !selector_length) return;

    selector_start = selector;
    selector_end = selector_start + selector_length;
    sep = 0;
    while (!sep && selector + 1 < selector_end) {
        if ((selector[0] == ':' && selector[1] == ':') ||
            (selector[0] == '.' && selector[1] == '.')) {
            sep = selector;
            break;
        }
        selector++;
    }
    if (!sep) return;

    if (interface_name) *interface_name = selector_start;
    if (interface_name_length) *interface_name_length = (size_t) (sep - selector_start);
    if (factory_name) *factory_name = sep + 2;
    if (factory_name_length) *factory_name_length = selector_length - ((size_t) (sep - selector_start) + 2);
}

static int runtime_factory_descriptor_matches(rxvm_context *context,
                                              const rx_callable_signature *expected_signature,
                                              const rx_callable_signature *registered_signature,
                                              const char *interface_name,
                                              size_t interface_name_length) {
    rx_callable_compare_options options;
    int matches;

    if (!context || !expected_signature || !registered_signature ||
        !interface_name || !interface_name_length) {
        return 0;
    }

    memset(&options, 0, sizeof(options));
    options.allow_return_covariance = 1;
    options.type_assignable = runtime_signature_type_assignable;
    options.userdata = context;

    matches = rx_sig_matches_contract(expected_signature, registered_signature, &options);
    if (!matches &&
        expected_signature->name && registered_signature->name &&
        strcmp(expected_signature->name, registered_signature->name) == 0 &&
        rx_sig_args_match(expected_signature, registered_signature) &&
        runtime_type_matches_contract_name(expected_signature->return_type,
                                           interface_name,
                                           interface_name_length) &&
        runtime_type_matches_contract_name(registered_signature->return_type,
                                           interface_name,
                                           interface_name_length)) {
        matches = 1;
    }

    return matches;
}

static int resolve_runtime_factory(rxvm_context *context,
                                   const char *descriptor,
                                   size_t descriptor_length,
                                   rxinteger argc,
                                   value **args,
                                   proc_runtime **factory_out,
                                   char **error_out) {
    char *descriptor_text;
    rx_callable_signature expected_signature;
    const char *selector;
    size_t selector_length;
    const char *interface_name;
    size_t interface_name_length;
    const char *factory_name;
    size_t factory_name_length;
    size_t entry_index;
    proc_runtime *best_factory;
    rxinteger best_score;
    char *best_class_name;
    size_t best_class_name_length;
    int saw_candidate;
    int saw_positive_score;
    size_t bucket_start;
    size_t bucket_end;

    if (factory_out) *factory_out = 0;
    if (error_out) *error_out = 0;
    if (!context || !descriptor || !descriptor_length || !factory_out) return 0;

    if (context->link_dirty || context->interface_factory_registry_dirty) {
        rxvm_link(context);
    }

    descriptor_text = dup_runtime_name(descriptor, descriptor_length);
    if (!descriptor_text) return 0;
    if (!rx_sig_parse_descriptor(descriptor_text, &expected_signature)) {
        if (error_out) *error_out = build_interface_factory_error("Invalid interface factory descriptor ", descriptor, descriptor_length);
        rxvm_internal_free(descriptor_text);
        return 0;
    }

    selector = expected_signature.name;
    selector_length = strlen(selector);
    parse_runtime_factory_selector(selector, selector_length,
                                   &interface_name, &interface_name_length,
                                   &factory_name, &factory_name_length);
    if (!interface_name_length || !factory_name_length) {
        if (error_out) *error_out = build_interface_factory_error("No interface factory providers for ", selector, selector_length);
        rx_sig_free(&expected_signature);
        rxvm_internal_free(descriptor_text);
        return 0;
    }

    bucket_start = 0;
    bucket_end = context->num_interface_factories;
    while (bucket_start < bucket_end) {
        size_t middle = bucket_start + (bucket_end - bucket_start) / 2;
        int comparison = compare_runtime_interface_factory_key(
                interface_name, interface_name_length,
                factory_name, factory_name_length,
                &context->interface_factories[middle]);

        if (comparison > 0) bucket_start = middle + 1;
        else bucket_end = middle;
    }

    best_factory = 0;
    best_score = 0;
    best_class_name = 0;
    best_class_name_length = 0;
    saw_candidate = 0;
    saw_positive_score = 0;

    for (entry_index = bucket_start;
         entry_index < context->num_interface_factories;
         entry_index++) {
        rxvm_interface_factory_entry *entry;
        rxinteger score;

        entry = &context->interface_factories[entry_index];
        if (compare_runtime_interface_factory_key(
                interface_name, interface_name_length,
                factory_name, factory_name_length, entry) != 0) break;

        saw_candidate = 1;
        if (!runtime_factory_descriptor_matches(context,
                                                &expected_signature,
                                                &entry->signature,
                                                interface_name,
                                                interface_name_length)) {
            continue;
        }

        score = 1;
        if (entry->match_proc &&
            !invoke_runtime_factory_match(context, entry->match_proc, argc, args, &score)) {
            if (error_out) *error_out = build_interface_factory_error("Failed to evaluate interface factory match for ", selector, selector_length);
            if (best_class_name) rxvm_internal_free(best_class_name);
            rx_sig_free(&expected_signature);
            rxvm_internal_free(descriptor_text);
            return 0;
        }
        if (score > 0) {
            saw_positive_score = 1;
            if (!best_factory ||
                score > best_score ||
                (score == best_score &&
                 compare_runtime_name(entry->class_name, entry->class_name_length,
                                      best_class_name, best_class_name_length) < 0)) {
                char *new_best_name;

                new_best_name = dup_runtime_name(entry->class_name, entry->class_name_length);
                if (!new_best_name) {
                    if (error_out) *error_out = build_interface_factory_error("Failed to resolve factory provider for ", selector, selector_length);
                    if (best_class_name) rxvm_internal_free(best_class_name);
                    rx_sig_free(&expected_signature);
                    rxvm_internal_free(descriptor_text);
                    return 0;
                }

                if (best_class_name) rxvm_internal_free(best_class_name);
                best_class_name = new_best_name;
                best_class_name_length = entry->class_name_length;
                best_score = score;
                best_factory = entry->factory_proc;
            }
        }
    }

    if (best_class_name) rxvm_internal_free(best_class_name);

    if (!saw_candidate) {
        if (error_out) *error_out = build_interface_factory_error("No interface factory providers for ", selector, selector_length);
        rx_sig_free(&expected_signature);
        rxvm_internal_free(descriptor_text);
        return 0;
    }

    if (!saw_positive_score || !best_factory) {
        if (error_out) *error_out = build_interface_factory_error("No matching interface factory provider for ", selector, selector_length);
        rx_sig_free(&expected_signature);
        rxvm_internal_free(descriptor_text);
        return 0;
    }

    *factory_out = best_factory;
    rx_sig_free(&expected_signature);
    rxvm_internal_free(descriptor_text);
    return 1;
}

static RxGraph *runtime_module_graph(module *mod) {
    return mod && mod->file ? mod->file->semantic_graph : 0;
}

static proc_runtime *runtime_graph_procedure_unbound(rxvm_context *context,
                                                     const RxGraph *graph,
                                                     RxCallableId callable) {
    RxGraphCallableView view;
    size_t module_index;

    if (!context || !graph || callable == RX_GRAPH_NONE ||
        !rx_graph_callable(graph, callable, &view)) return 0;
    for (module_index = 0u; module_index < context->num_modules; module_index++) {
        module *mod;
        size_t proc_index;

        mod = context->modules[module_index];
        if (!mod || !mod->file || mod->file->semantic_graph != graph ||
            mod->file->semantic_module_index != view.procedure.module_index) continue;
        for (proc_index = 0u; proc_index < mod->procedure_count; proc_index++) {
            if (mod->procedures[proc_index].start == view.procedure.procedure_offset) {
                return &mod->procedures[proc_index];
            }
        }
        return 0;
    }
    return 0;
}

static int runtime_graph_factory_entry_matches(
        rxvm_context *context,
        const rx_callable_signature *expected_signature,
        const char *interface_name,
        size_t interface_name_length,
        const rxvm_interface_factory_entry *entry) {
    if (!context || !expected_signature || !interface_name ||
        !interface_name_length || !entry ||
        !rx_sig_args_match(expected_signature, &entry->signature)) return 0;
    if (expected_signature->return_type && entry->signature.return_type &&
        strcmp(expected_signature->return_type,
               entry->signature.return_type) == 0) return 1;
    if (runtime_signature_type_assignable(context,
                                          entry->signature.return_type,
                                          expected_signature->return_type)) return 1;
    return runtime_type_matches_contract_name(expected_signature->return_type,
                                              interface_name,
                                              interface_name_length) &&
           runtime_type_matches_contract_name(entry->signature.return_type,
                                              interface_name,
                                              interface_name_length);
}

static size_t rxvm_bind_graph_factory(
        rxvm_context *context,
        rxvm_graph_binding *binding,
        RxFactoryId factory,
        rxvm_graph_provider_binding *providers,
        size_t provider_capacity) {
    RxGraphFactoryView factory_view;
    RxGraphMemberView member_view;
    rxvm_graph_factory_binding *factory_binding;
    rx_callable_signature expected_signature;
    const char *interface_name;
    size_t interface_name_length;
    size_t bucket_start;
    size_t bucket_end;
    size_t entry_index;
    size_t provider_count;

    if (!context || !binding || factory >= binding->factory_count ||
        !binding->factory_bindings ||
        !rx_graph_factory(binding->graph, factory, &factory_view) ||
        !rx_graph_member(binding->graph, factory_view.member, &member_view)) {
        return 0u;
    }
    factory_binding = &binding->factory_bindings[factory];
    interface_name = rx_graph_type_name(binding->graph,
                                        factory_view.interface_type);
    if (!interface_name || !member_view.name || !member_view.descriptor) return 0u;
    interface_name_length = strlen(interface_name);
    factory_binding->interface_name = interface_name;
    factory_binding->member_name = member_view.name;

    if (!rx_sig_parse_descriptor(member_view.descriptor,
                                 &expected_signature)) return 0u;
    bucket_start = 0u;
    bucket_end = context->num_interface_factories;
    while (bucket_start < bucket_end) {
        size_t middle;
        int comparison;

        middle = bucket_start + (bucket_end - bucket_start) / 2u;
        comparison = compare_runtime_interface_factory_key(
            interface_name, interface_name_length,
            member_view.name, strlen(member_view.name),
            &context->interface_factories[middle]);
        if (comparison > 0) bucket_start = middle + 1u;
        else bucket_end = middle;
    }

    provider_count = 0u;
    for (entry_index = bucket_start;
         entry_index < context->num_interface_factories;
         entry_index++) {
        const rxvm_interface_factory_entry *entry;
        rxvm_graph_provider_binding *provider;

        entry = &context->interface_factories[entry_index];
        if (compare_runtime_interface_factory_key(
                interface_name, interface_name_length,
                member_view.name, strlen(member_view.name), entry) != 0) break;
        if (!runtime_graph_factory_entry_matches(context,
                                                 &expected_signature,
                                                 interface_name,
                                                 interface_name_length,
                                                 entry)) continue;
        if (providers && provider_count < provider_capacity) {
            provider = &providers[provider_count];
            provider->class_name = entry->class_name;
            provider->factory_target = entry->factory_proc;
            provider->match_target = entry->match_proc;
            provider->requires_match = entry->match_proc != 0;
        }
        provider_count++;
    }
    rx_sig_free(&expected_signature);

    if (providers && provider_count <= provider_capacity) {
        factory_binding->providers = provider_count ? providers : 0;
        factory_binding->provider_count = provider_count;
        if (provider_count == 1u && !providers[0].requires_match) {
            factory_binding->direct_target = providers[0].factory_target;
        }
    }
    return provider_count;
}

void rxvm_free_graph_bindings(rxvm_context *context) {
    size_t binding_index;
    size_t module_index;

    if (!context) return;
    for (module_index = 0u; module_index < context->num_modules; module_index++) {
        if (context->modules[module_index]) {
            context->modules[module_index]->graph_binding = 0;
        }
    }
    for (binding_index = 0u;
         binding_index < context->graph_binding_count;
         binding_index++) {
        rxvm_graph_binding *binding;
        binding = context->graph_bindings[binding_index];
        if (!binding) continue;
        (void)rxvm_memory_release(binding->callable_targets);
        (void)rxvm_memory_release(binding->factory_bindings);
        (void)rxvm_memory_release(binding->provider_bindings);
        (void)rxvm_memory_release(binding);
    }
    (void)rxvm_memory_release(context->graph_bindings);
    context->graph_bindings = 0;
    context->graph_binding_count = 0u;
    context->graph_binding_capacity = 0u;
}

void rxvm_rebuild_graph_bindings(rxvm_context *context) {
    size_t module_index;
    size_t binding_index;

    if (!context) return;
    rxvm_free_graph_bindings(context);
    for (module_index = 0u; module_index < context->num_modules; module_index++) {
        module *mod;
        RxGraph *graph;
        rxvm_graph_binding *binding;

        mod = context->modules[module_index];
        graph = runtime_module_graph(mod);
        if (!graph) continue;
        binding = 0;
        for (binding_index = 0u;
             binding_index < context->graph_binding_count;
             binding_index++) {
            if (context->graph_bindings[binding_index]->graph == graph) {
                binding = context->graph_bindings[binding_index];
                break;
            }
        }
        if (!binding) {
            size_t callable_count;
            size_t factory_count;
            if (context->graph_binding_count == context->graph_binding_capacity) {
                size_t new_capacity;
                rxvm_graph_binding **new_bindings;
                new_capacity = context->graph_binding_capacity
                    ? context->graph_binding_capacity * 2u : 4u;
                if (new_capacity < context->graph_binding_count ||
                    new_capacity > SIZE_MAX / sizeof(*new_bindings)) {
                    RX_PANIC_OOM("size rxvm graph bindings", (size_t)-1, mod->name);
                }
                new_bindings = (rxvm_graph_binding **)rxvm_memory_resize_bytes(
                    context->worker.memory_worker, context->graph_bindings,
                    context->graph_binding_count * sizeof(*new_bindings),
                    new_capacity * sizeof(*new_bindings));
                if (!new_bindings) {
                    RX_PANIC_OOM("realloc rxvm graph bindings",
                                 new_capacity * sizeof(*new_bindings),
                                 mod->name);
                }
                context->graph_bindings = new_bindings;
                context->graph_binding_capacity = new_capacity;
            }
            binding = (rxvm_graph_binding *)rxvm_memory_calloc_bytes(
                    context->worker.memory_worker, 1u, sizeof(*binding));
            if (!binding) {
                RX_PANIC_OOM("calloc rxvm graph binding", sizeof(*binding), mod->name);
            }
            callable_count = rx_graph_callable_count(graph);
            factory_count = rx_graph_factory_count(graph);
            binding->graph = graph;
            binding->callable_count = callable_count;
            binding->factory_count = factory_count;
            if (callable_count) {
                if (callable_count > SIZE_MAX / sizeof(*binding->callable_targets)) {
                    RX_PANIC_OOM("size rxvm callable bindings", (size_t)-1, mod->name);
                }
                binding->callable_targets = (proc_runtime **)
                    rxvm_memory_calloc_bytes(
                        context->worker.memory_worker, callable_count,
                        sizeof(*binding->callable_targets));
                if (!binding->callable_targets) {
                    RX_PANIC_OOM("calloc rxvm callable bindings",
                                 callable_count * sizeof(*binding->callable_targets),
                                 mod->name);
                }
            }
            if (factory_count) {
                if (factory_count > SIZE_MAX / sizeof(*binding->factory_bindings)) {
                    RX_PANIC_OOM("size rxvm factory bindings", (size_t)-1, mod->name);
                }
                binding->factory_bindings = (rxvm_graph_factory_binding *)
                    rxvm_memory_calloc_bytes(
                        context->worker.memory_worker, factory_count,
                        sizeof(*binding->factory_bindings));
                if (!binding->factory_bindings) {
                    RX_PANIC_OOM("calloc rxvm factory bindings",
                                 factory_count * sizeof(*binding->factory_bindings),
                                 mod->name);
                }
            }
            context->graph_bindings[context->graph_binding_count++] = binding;
        }
        mod->graph_binding = binding;
    }
    for (binding_index = 0u;
         binding_index < context->graph_binding_count;
         binding_index++) {
        rxvm_graph_binding *binding;
        RxCallableId callable;
        binding = context->graph_bindings[binding_index];
        for (callable = 0u; callable < binding->callable_count; callable++) {
            binding->callable_targets[callable] =
                runtime_graph_procedure_unbound(context,
                                                binding->graph,
                                                callable);
        }
        if (binding->factory_count) {
            size_t provider_offset;
            RxFactoryId factory;

            binding->provider_count = 0u;
            for (factory = 0u; factory < binding->factory_count; factory++) {
                size_t count;
                count = rxvm_bind_graph_factory(context,
                                                binding,
                                                factory,
                                                0,
                                                0u);
                if (count > SIZE_MAX - binding->provider_count) {
                    RX_PANIC_OOM("size rxvm provider bindings", (size_t)-1, 0);
                }
                binding->provider_count += count;
            }
            if (binding->provider_count) {
                if (binding->provider_count >
                    SIZE_MAX / sizeof(*binding->provider_bindings)) {
                    RX_PANIC_OOM("size rxvm provider bindings", (size_t)-1, 0);
                }
                binding->provider_bindings =
                    (rxvm_graph_provider_binding *)rxvm_memory_calloc_bytes(
                        context->worker.memory_worker, binding->provider_count,
                        sizeof(*binding->provider_bindings));
                if (!binding->provider_bindings) {
                    RX_PANIC_OOM("calloc rxvm provider bindings",
                                 binding->provider_count *
                                     sizeof(*binding->provider_bindings),
                                 0);
                }
            }
            provider_offset = 0u;
            for (factory = 0u; factory < binding->factory_count; factory++) {
                size_t count;
                if (provider_offset > binding->provider_count) {
                    RX_PANIC_OOM("validate rxvm provider bindings", (size_t)-1, 0);
                }
                count = rxvm_bind_graph_factory(
                    context,
                    binding,
                    factory,
                    binding->provider_bindings
                        ? binding->provider_bindings + provider_offset : 0,
                    binding->provider_count - provider_offset);
                if (count > binding->provider_count - provider_offset) {
                    RX_PANIC_OOM("validate rxvm provider bindings", (size_t)-1, 0);
                }
                provider_offset += count;
            }
        }
    }
    context->semantic_generation = context->semantic_generation == UINT64_MAX
        ? 1u : context->semantic_generation + 1u;
}

static proc_runtime *resolve_runtime_graph_method(
                                                  const rxvm_graph_binding *binding,
                                                  value *object_value,
                                                  RxMemberId member) {
    RxCallableId callable;

    if (!object_value || !object_value->object_type ||
        !object_value->object_type->graph) return 0;
    if (!binding || binding->graph != object_value->object_type->graph) return 0;
    callable = rx_graph_type_ref_dispatch(object_value->object_type, member);
    return rxvm_bound_graph_callable(binding, callable);
}

RX_INLINE proc_runtime *resolve_runtime_graph_method_cached(
        rxvm_context *context,
        const rxvm_graph_binding *binding,
        rxvm_dynamic_site_cache *cache,
        value *object_value,
        RxMemberId member) {
    proc_runtime *target;
    uint32_t way;

    if (!cache) return resolve_runtime_graph_method(binding,
                                                    object_value,
                                                    member);
    if (cache->generation != context->semantic_generation) {
        memset(&cache->value, 0, sizeof(cache->value));
        cache->generation = context->semantic_generation;
    }
    for (way = 0u; way < RXVM_METHOD_CACHE_WAYS; way++) {
        if (cache->value.method.types[way] == object_value->object_type) {
            return cache->value.method.targets[way];
        }
    }
    target = resolve_runtime_graph_method(binding, object_value, member);
    if (target) {
        way = cache->value.method.next_way++ % RXVM_METHOD_CACHE_WAYS;
        cache->value.method.types[way] = object_value->object_type;
        cache->value.method.targets[way] = target;
    }
    return target;
}

static char *build_runtime_graph_factory_error(
        const char *prefix,
        const rxvm_graph_factory_binding *binding) {
    const char *member_name;
    size_t interface_length;
    size_t member_length;
    char *selector;
    char *message;

    if (!prefix || !binding || !binding->interface_name) return 0;
    member_name = binding->member_name;
    interface_length = strlen(binding->interface_name);
    if (!member_name || strcmp(member_name, "*") == 0) {
        return build_interface_factory_error(prefix,
                                             binding->interface_name,
                                             interface_length);
    }
    member_length = strlen(member_name);
    if (interface_length > SIZE_MAX - member_length - 3u) return 0;
    selector = (char *)rxvm_internal_alloc(
            interface_length + member_length + 3u);
    if (!selector) return 0;
    memcpy(selector, binding->interface_name, interface_length);
    selector[interface_length] = '.';
    selector[interface_length + 1u] = '.';
    memcpy(selector + interface_length + 2u, member_name, member_length + 1u);
    message = build_interface_factory_error(prefix,
                                            selector,
                                            interface_length + member_length + 2u);
    rxvm_internal_free(selector);
    return message;
}

static int resolve_runtime_graph_factory(rxvm_context *context,
                                         const rxvm_graph_factory_binding *binding,
                                         rxinteger argc,
                                         value **args,
                                         proc_runtime **factory_out,
                                         char **error_out) {
    size_t position;
    proc_runtime *best_factory;
    rxinteger best_score;
    const char *best_class_name;
    int saw_positive_score;

    if (factory_out) *factory_out = 0;
    if (error_out) *error_out = 0;
    if (!context || !binding || !factory_out ||
        !binding->provider_count) return -1;
    best_factory = 0;
    best_score = 0;
    best_class_name = 0;
    saw_positive_score = 0;
    for (position = 0u; position < binding->provider_count; position++) {
        const rxvm_graph_provider_binding *provider;
        rxinteger score;

        provider = &binding->providers[position];
        if (!provider->factory_target) continue;
        score = 1;
        if (provider->requires_match &&
            (!provider->match_target ||
             !invoke_runtime_factory_match(context,
                                           provider->match_target,
                                           argc,
                                           args,
                                           &score))) {
            if (error_out) {
                *error_out = build_runtime_graph_factory_error(
                    "Failed to evaluate interface factory match for ", binding);
            }
            return 0;
        }
        if (score <= 0) continue;
        saw_positive_score = 1;
        if (!best_factory || score > best_score ||
            (score == best_score && provider->class_name && best_class_name &&
             strcmp(provider->class_name, best_class_name) < 0)) {
            best_factory = provider->factory_target;
            best_score = score;
            best_class_name = provider->class_name;
        }
    }
    if (!saw_positive_score || !best_factory) {
        if (error_out) {
            *error_out = build_runtime_graph_factory_error(
                "No matching interface factory provider for ", binding);
        }
        return 0;
    }
    *factory_out = best_factory;
    return 1;
}

static int runtime_value_matches_graph_type(rxvm_context *context,
                                            value *object_value,
                                            const RxGraph *target_graph,
                                            RxGraphId target_type) {
    const char *target_name;
    const RxGraphTypeRef *target_type_ref;

    if (!object_value || !target_graph) return 0;
    target_type_ref = rx_graph_type_ref(target_graph, target_type);
    if (!target_type_ref) return 0;
    target_name = target_type_ref->name;
    if (runtime_type_name_is_object_contract(target_name, strlen(target_name))) {
        return object_value->object_type != 0;
    }
    if (object_value->object_type &&
        object_value->object_type->graph == target_graph) {
        return rx_graph_type_ref_supports(object_value->object_type,
                                          target_type_ref);
    }
    return runtime_value_matches_object_type(context,
                                             object_value,
                                             target_name,
                                             strlen(target_name));
}

/* Convert the compiler's "Mmm dd yyyy" date to yyyymmdd. */
static void rxvm_format_compile_date(char compile_date[9]) {
    const char *source_date = __DATE__;

    compile_date[0] = source_date[7];
    compile_date[1] = source_date[8];
    compile_date[2] = source_date[9];
    compile_date[3] = source_date[10];
    compile_date[4] =
            (source_date[0] == 'O' || source_date[0] == 'N' || source_date[0] == 'D')
            ? '1' : '0';
    compile_date[5] =
            source_date[0] == 'J' ? (source_date[1] == 'a' ? '1' :
                                     (source_date[2] == 'n' ? '6' : '7')) :
            source_date[0] == 'F' ? '2' :
            source_date[0] == 'M' ? (source_date[2] == 'r' ? '3' : '5') :
            source_date[0] == 'A' ? (source_date[1] == 'p' ? '4' : '8') :
            source_date[0] == 'S' ? '9' :
            source_date[0] == 'O' ? '0' :
            source_date[0] == 'N' ? '1' :
            source_date[0] == 'D' ? '2' : '0';
    compile_date[6] = source_date[4] == ' ' ? '0' : source_date[4];
    compile_date[7] = source_date[5];
    compile_date[8] = '\0';
}

/* Function to convert an interrupt to a string: interrupt_entry -> Code Description Massage */
const char *interrupt_to_string(unsigned char interrupt) {
    switch (interrupt) {
        case RXSIGNAL_KILL:
            return "KILL";
        case RXSIGNAL_ERROR:
            return "ERROR";
        case RXSIGNAL_OVERFLOW_UNDERFLOW:
            return"OVERFLOW_UNDERFLOW";
        case RXSIGNAL_CONVERSION_ERROR:
            return "CONVERSION_ERROR";
        case RXSIGNAL_UNKNOWN_INSTRUCTION:
            return "UNKNOWN_INSTRUCTION";
        case RXSIGNAL_FUNCTION_NOT_FOUND:
            return "FUNCTION_NOT_FOUND";
        case RXSIGNAL_NOT_IMPLEMENTED:
            return "NOT_IMPLEMENTED";
        case RXSIGNAL_INVALID_SIGNAL_CODE:
            return "INVALID_SIGNAL_CODE";
        case RXSIGNAL_REFERENCE_INVALID:
            return "REFERENCE_INVALID";
        case RXSIGNAL_OBJECT_NOT_INITIALIZED:
            return "OBJECT_NOT_INITIALIZED";
        case RXSIGNAL_RXBIN_CORRUPTION:
            return "RXBIN_CORRUPTION";
        case RXSIGNAL_CANCEL:
            return "CANCEL";
        case RXSIGNAL_OUT_OF_RANGE:
            return "OUT_OF_RANGE";
        case RXSIGNAL_FAILURE:
            return "FAILURE";
        case RXSIGNAL_QUIT:
            return "QUIT";
        case RXSIGNAL_TERM:
            return "TERM";
        case RXSIGNAL_NOTREADY:
            return "NOTREADY";
        case RXSIGNAL_INVALID_ARGUMENTS:
            return "INVALID_ARGUMENTS";
        case RXSIGNAL_DIVISION_BY_ZERO:
            return "DIVISION_BY_ZERO";
        case RXSIGNAL_UNICODE_ERROR:
            return "UNICODE_ERROR";
        case RXSIGNAL_POSIX_HUP:
            return "POSIX_HUP";
        case RXSIGNAL_POSIX_INT:
            return "POSIX_INT";
        case RXSIGNAL_POSIX_USR1:
            return "POSIX_USR1";
        case RXSIGNAL_POSIX_USR2:
            return "POSIX_USR2";;
        case RXSIGNAL_POSIX_CHLD:
            return "POSIX_CHLD";
        case RXSIGNAL_CHANNEL_ERROR:
            return "CHANNEL_ERROR";
        case RXSIGNAL_TASK_FAILURE:
            return "TASK_FAILURE";
        case RXSIGNAL_BREAKPOINT:
            return "BREAKPOINT";
        case RXSIGNAL_OTHER:
            return "OTHER";
        default:
            return 0; // Invalid Signal Code
    }
}

/* Function to convert a string to an interrupt: Code Description Massage -> interrupt_entry */
unsigned char string_to_interrupt(const char *interrupt) {
    if (strcmp(interrupt, "KILL") == 0) return RXSIGNAL_KILL;
    if (strcmp(interrupt, "ERROR") == 0) return RXSIGNAL_ERROR;
    if (strcmp(interrupt, "OVERFLOW_UNDERFLOW") == 0) return RXSIGNAL_OVERFLOW_UNDERFLOW;
    if (strcmp(interrupt, "CONVERSION_ERROR") == 0) return RXSIGNAL_CONVERSION_ERROR;
    if (strcmp(interrupt, "UNKNOWN_INSTRUCTION") == 0) return RXSIGNAL_UNKNOWN_INSTRUCTION;
    if (strcmp(interrupt, "FUNCTION_NOT_FOUND") == 0) return RXSIGNAL_FUNCTION_NOT_FOUND;
    if (strcmp(interrupt, "NOT_IMPLEMENTED") == 0) return RXSIGNAL_NOT_IMPLEMENTED;
    if (strcmp(interrupt, "INVALID_SIGNAL_CODE") == 0) return RXSIGNAL_INVALID_SIGNAL_CODE;
    if (strcmp(interrupt, "REFERENCE_INVALID") == 0) return RXSIGNAL_REFERENCE_INVALID;
    if (strcmp(interrupt, "OBJECT_NOT_INITIALIZED") == 0) return RXSIGNAL_OBJECT_NOT_INITIALIZED;
    if (strcmp(interrupt, "RXBIN_CORRUPTION") == 0) return RXSIGNAL_RXBIN_CORRUPTION;
    if (strcmp(interrupt, "CANCEL") == 0) return RXSIGNAL_CANCEL;
    if (strcmp(interrupt, "OUT_OF_RANGE") == 0) return RXSIGNAL_OUT_OF_RANGE;
    if (strcmp(interrupt, "FAILURE") == 0) return RXSIGNAL_FAILURE;
    if (strcmp(interrupt, "QUIT") == 0) return RXSIGNAL_QUIT;
    if (strcmp(interrupt, "TERM") == 0) return RXSIGNAL_TERM;
    if (strcmp(interrupt, "NOTREADY") == 0) return RXSIGNAL_NOTREADY;
    if (strcmp(interrupt, "INVALID_ARGUMENTS") == 0) return RXSIGNAL_INVALID_ARGUMENTS;
    if (strcmp(interrupt, "DIVISION_BY_ZERO") == 0) return RXSIGNAL_DIVISION_BY_ZERO;
    if (strcmp(interrupt, "UNICODE_ERROR") == 0) return RXSIGNAL_UNICODE_ERROR;
    if (strcmp(interrupt, "POSIX_HUP") == 0) return RXSIGNAL_POSIX_HUP;
    if (strcmp(interrupt, "POSIX_INT") == 0) return RXSIGNAL_POSIX_INT;
    if (strcmp(interrupt, "POSIX_USR1") == 0) return RXSIGNAL_POSIX_USR1;
    if (strcmp(interrupt, "POSIX_USR2") == 0) return RXSIGNAL_POSIX_USR2;
    if (strcmp(interrupt, "POSIX_CHLD") == 0) return RXSIGNAL_POSIX_CHLD;
    if (strcmp(interrupt, "CHANNEL_ERROR") == 0) return RXSIGNAL_CHANNEL_ERROR;
    if (strcmp(interrupt, "TASK_FAILURE") == 0) return RXSIGNAL_TASK_FAILURE;
    if (strcmp(interrupt, "BREAKPOINT") == 0) return RXSIGNAL_BREAKPOINT;
    if (strcmp(interrupt, "OTHER") == 0) return RXSIGNAL_OTHER;
    return RXSIGNAL_MAX; // Invalid Signal Code
}

typedef enum rxsignal_handler_action {
    RXSIGNAL_HANDLER_ACTION_NONE = 0,
    RXSIGNAL_HANDLER_ACTION_SKIP,
    RXSIGNAL_HANDLER_ACTION_FAIL
} rxsignal_handler_action;

static rxsignal_handler_action rxsignal_read_handler_action(value *action) {
    if (!action || action->string_length <= 0) return RXSIGNAL_HANDLER_ACTION_NONE;

    null_terminate_string_buffer(action);
    if (strcmp(action->string_value, "__rxsignal_skip") == 0) return RXSIGNAL_HANDLER_ACTION_SKIP;
    if (strcmp(action->string_value, "__rxsignal_fail") == 0) return RXSIGNAL_HANDLER_ACTION_FAIL;
    return RXSIGNAL_HANDLER_ACTION_NONE;
}

static value *rxsignal_handler_payload(stack_frame *handler_frame) {
    size_t arg_index;
    value *arg;

    if (!handler_frame || !handler_frame->procedure || !handler_frame->procedure->binarySpace) return NULL;
    arg_index = handler_frame->procedure->binarySpace->globals + handler_frame->procedure->locals + 1;
    if (arg_index >= handler_frame->number_locals) return NULL;
    arg = handler_frame->baselocals[arg_index];
    if (!arg || arg->num_attributes < 5 || !arg->attributes) return NULL;
    return arg->attributes[4];
}

static void rxsignal_apply_native_interrupt_mode(unsigned char sig, interrupt_entry *entry) {
    if (!entry || sig == 0 || sig >= RXSIGNAL_MAX) return;
    if (entry->response == RXSIGNAL_RESPONSE_IGNORE) {
        if (sig != RXSIGNAL_KILL) ignore_interrupt((int)sig);
    } else {
        enable_interrupt((int)sig);
    }
}

static RXVM_LABEL_OWNER_NOINLINE int rxsignal_ensure_private_interrupt_table(stack_frame *frame) {
    interrupt_entry *private_table;

    if (!frame || !frame->interrupt_table) return 0;
    if (frame->interrupt_table_owned) return 1;

    private_table = rxvm_memory_alloc_bytes(
            0, sizeof(interrupt_entry) * RXSIGNAL_MAX);
    if (!private_table) return 0;
    memcpy(private_table, frame->interrupt_table,
           sizeof(interrupt_entry) * RXSIGNAL_MAX);
    frame->interrupt_table = private_table;
    frame->interrupt_table_owned = 1;
    return 1;
}

static void rxsignal_release_private_interrupt_table(stack_frame *frame) {
    if (!frame) return;
    if (frame->interrupt_table_owned)
        (void)rxvm_memory_release(frame->interrupt_table);
    frame->interrupt_table = 0;
    frame->interrupt_table_owned = 0;
}

static void rxsignal_push_handler(stack_frame *frame, unsigned char sig) {
    interrupt_saved_entry *saved;

    if (!frame || sig == 0 || sig >= RXSIGNAL_MAX) return;

    saved = rxvm_memory_alloc_bytes(0, sizeof(interrupt_saved_entry));
    if (!saved) return;
    saved->signal = sig;
    saved->entry = frame->interrupt_table[sig - 1];
    saved->next = frame->interrupt_stack;
    frame->interrupt_stack = saved;
}

typedef enum rxsignal_pop_result {
    RXSIGNAL_POP_NOT_FOUND = 0,
    RXSIGNAL_POP_RESTORED = 1,
    RXSIGNAL_POP_NO_MEMORY = -1
} rxsignal_pop_result;

static rxsignal_pop_result rxsignal_pop_handler(stack_frame *frame, unsigned char sig) {
    interrupt_saved_entry *saved;
    interrupt_saved_entry *previous;

    if (!frame || sig == 0 || sig >= RXSIGNAL_MAX) return RXSIGNAL_POP_NOT_FOUND;

    previous = 0;
    saved = frame->interrupt_stack;
    while (saved) {
        if (saved->signal == sig) {
            if (!rxsignal_ensure_private_interrupt_table(frame)) return RXSIGNAL_POP_NO_MEMORY;
            frame->interrupt_table[sig - 1] = saved->entry;
            rxsignal_apply_native_interrupt_mode(sig, &frame->interrupt_table[sig - 1]);
            if (previous) previous->next = saved->next;
            else frame->interrupt_stack = saved->next;
            (void)rxvm_memory_release(saved);
            return RXSIGNAL_POP_RESTORED;
        }
        previous = saved;
        saved = saved->next;
    }

    return RXSIGNAL_POP_NOT_FOUND;
}

/* Restore handler registrations installed after the branch target's protected
 * block was entered.  The target block's own saved entries remain in place;
 * generated handler entry code pops those after control reaches the label. */
static int rxsignal_unwind_handler_stack_to(
        stack_frame *frame, interrupt_saved_entry *marker) {
    interrupt_saved_entry *saved;

    if (!frame) return marker == 0;
    while (frame->interrupt_stack != marker) {
        saved = frame->interrupt_stack;
        if (!saved) return 0;
        if (!rxsignal_ensure_private_interrupt_table(frame)) return 0;
        frame->interrupt_stack = saved->next;
        frame->interrupt_table[saved->signal - 1] = saved->entry;
        rxsignal_apply_native_interrupt_mode(
                saved->signal, &frame->interrupt_table[saved->signal - 1]);
        (void)rxvm_memory_release(saved);
    }
    return 1;
}

static void rxsignal_clear_handler_stack(stack_frame *frame) {
    interrupt_saved_entry *saved;

    if (!frame) return;
    while (frame->interrupt_stack) {
        saved = frame->interrupt_stack;
        frame->interrupt_stack = saved->next;
        rxsignal_apply_native_interrupt_mode(saved->signal, &saved->entry);
        (void)rxvm_memory_release(saved);
    }
}

/* Stack Frame Factory */
RX_INLINE stack_frame *frame_f(
                    proc_runtime *procedure,
                    int no_args,
                    stack_frame *parent,
                    bin_code *return_pc,
                    value *return_reg) {
    stack_frame *this;
    size_t num_locals;
    size_t nominal_num_locals;
    size_t local_count;
    size_t global_count = 0;
    size_t arg_count;
    size_t pointer_count;
    size_t pointer_bytes;
    size_t value_count;
    size_t value_bytes;
    int i, j;
    size_t frame_size;
    value *value_buffer;
    int reused_frame = 0;
#ifdef CREXX_VM_PROFILING
    uint64_t profile_phase_start;
    int profile_reused_frame = 0;
#endif

    if (!procedure || procedure->locals < 0 || no_args < 0) return 0;
    if (procedure->binarySpace && procedure->binarySpace->globals < 0) return 0;

    local_count = (size_t)procedure->locals;
    arg_count = (size_t)no_args;
    if (procedure->binarySpace) global_count = (size_t)procedure->binarySpace->globals;

    if (!rxvm_checked_size_add(local_count, global_count, &num_locals) ||
        !rxvm_checked_size_add(num_locals, arg_count, &num_locals) ||
        !rxvm_checked_size_add(num_locals, 1, &num_locals) ||
        !rxvm_checked_size_add(local_count, global_count, &nominal_num_locals) ||
        !rxvm_checked_size_add(nominal_num_locals, NOMINAL_NUM_ARGS, &nominal_num_locals) ||
        !rxvm_checked_size_add(nominal_num_locals, 1, &nominal_num_locals)) {
        return 0;
    }

    if (num_locals > (size_t)INT_MAX || nominal_num_locals > (size_t)INT_MAX) return 0;

    /* Do we need an oversized block */
    if (num_locals > nominal_num_locals) nominal_num_locals = num_locals;

    if (*procedure->frame_free_list &&
        (*procedure->frame_free_list)->nominal_number_locals >= num_locals) {

        /* We can reuse this stack frame */
        this = *procedure->frame_free_list;
        *procedure->frame_free_list = this->prev_free;
        this->prev_free = 0;
        reused_frame = 1;
#ifdef CREXX_VM_PROFILING
        rxvm_profile_record_frame_activation(1, 0, 0);
        profile_reused_frame = 1;
#endif

        /* Reset Local Registers */
#ifdef CREXX_VM_PROFILING
        profile_phase_start = rxvm_profile_frame_phase_begin();
#endif
        for (i = 0; i < procedure->locals; i++) {
            this->locals[i] = this->baselocals[i];
#ifdef SAFE_RECYCLED_STACKFRAMES
            value_zero(this->locals[i]);
#endif
        }
#ifdef CREXX_VM_PROFILING
        rxvm_profile_record_frame_phase(
                RXVM_PROFILE_FRAME_LOCAL_RELINK, 1, profile_phase_start,
                local_count);
#endif
        /* Make sure global registers are linked correctly */
#ifdef CREXX_VM_PROFILING
        profile_phase_start = rxvm_profile_frame_phase_begin();
#endif
        if (procedure->binarySpace) {
            for (j = 0; j < procedure->binarySpace->globals; i++, j++) {
                this->locals[i] = this->baselocals[i];
            }
        }
#ifdef CREXX_VM_PROFILING
        rxvm_profile_record_frame_phase(
                RXVM_PROFILE_FRAME_GLOBAL_RELINK, 1, profile_phase_start,
                global_count);
#endif
        /* Reset register a0 - number of arguments */
#ifdef CREXX_VM_PROFILING
        profile_phase_start = rxvm_profile_frame_phase_begin();
#endif
        this->locals[i] = this->baselocals[i];
        value_zero(this->locals[i]);
        this->locals[i]->int_value = no_args;
#ifdef CREXX_VM_PROFILING
        rxvm_profile_record_frame_phase(
                RXVM_PROFILE_FRAME_ARGUMENT_COUNT_RESET, 1,
                profile_phase_start, 1);
#endif
    }
    else {
        /* Need a new stack frame - allocate all the memory in one go */
        if (!rxvm_checked_size_mul(nominal_num_locals, 2, &pointer_count) ||
            !rxvm_checked_size_mul(sizeof(value*), pointer_count, &pointer_bytes) ||
            !rxvm_checked_size_add(local_count, 1, &value_count) ||
            !rxvm_checked_size_mul(sizeof(value), value_count, &value_bytes) ||
            !rxvm_checked_size_add(sizeof(stack_frame), pointer_bytes, &frame_size) ||
            !rxvm_checked_size_add(frame_size, value_bytes, &frame_size)) {
            return 0;
        }

        this = (stack_frame *)rxvm_memory_alloc_bytes(0, frame_size);
        if (!this) return 0;
#ifdef CREXX_VM_PROFILING
        rxvm_profile_record_frame_activation(0, frame_size, value_count);
#endif
        this->prev_free = 0;

        this->baselocals = (value**)(this + 1);
        this->locals = this->baselocals + nominal_num_locals;
        value_buffer = (value*)(this->locals + nominal_num_locals);

        /* Link Locals */
#ifdef CREXX_VM_PROFILING
        profile_phase_start = rxvm_profile_frame_phase_begin();
#endif
        for (i = 0; i < procedure->locals; i++, value_buffer++) {
            value_init(value_buffer);
#ifdef CREXX_VM_PROFILING
            rxvm_profile_mark_value_origin(
                    value_buffer, RXVM_PROFILE_VALUE_ORIGIN_FRAME);
#endif
            this->locals[i] = value_buffer;
            this->baselocals[i] = value_buffer;
        }
#ifdef CREXX_VM_PROFILING
        rxvm_profile_record_frame_phase(
                RXVM_PROFILE_FRAME_LOCAL_RELINK, 0, profile_phase_start,
                local_count);
#endif

        /* Link Globals */
#ifdef CREXX_VM_PROFILING
        profile_phase_start = rxvm_profile_frame_phase_begin();
#endif
        if (procedure->binarySpace) {
            for (j = 0; j < procedure->binarySpace->globals; i++, j++) {
                this->baselocals[i] =  procedure->binarySpace->module->globals[j];
                this->locals[i] = procedure->binarySpace->module->globals[j];
            }
        }
#ifdef CREXX_VM_PROFILING
        rxvm_profile_record_frame_phase(
                RXVM_PROFILE_FRAME_GLOBAL_RELINK, 0, profile_phase_start,
                global_count);
#endif

        /* Link a0 */
#ifdef CREXX_VM_PROFILING
        profile_phase_start = rxvm_profile_frame_phase_begin();
#endif
        value_init(value_buffer);
#ifdef CREXX_VM_PROFILING
        rxvm_profile_mark_value_origin(
                value_buffer, RXVM_PROFILE_VALUE_ORIGIN_FRAME);
#endif
        this->locals[i] = value_buffer;
        this->baselocals[i] = value_buffer;
        this->locals[i]->int_value = no_args;
#ifdef CREXX_VM_PROFILING
        rxvm_profile_record_frame_phase(
                RXVM_PROFILE_FRAME_ARGUMENT_COUNT_RESET, 0,
                profile_phase_start, 1);
#endif

        this->nominal_number_locals = nominal_num_locals;
    }
    this->parent = parent;
    this->interrupt_table = 0;
    this->interrupt_table_owned = 0;
    if (parent) {
#ifdef CREXX_VM_PROFILING
        profile_phase_start = rxvm_profile_frame_phase_begin();
#endif
        /* Inherit the parent's immutable policy.  A frame-local signal
         * instruction takes a private copy before its first mutation. */
        this->interrupt_table = parent->interrupt_table;
        this->is_interrupt = parent->is_interrupt;

        /* VM Plugins */
        this->unicode = parent->unicode;
        this->decimal = parent->decimal;

        /* Copy the numeric context */
        this->num_context = parent->num_context;

        // Set the numeric context of the decimal plugin
        if (this->decimal) {
            this->decimal->num_context = &this->num_context;
            this->decimal->syncNumericContext(this->decimal);
        }
#ifdef CREXX_VM_PROFILING
        rxvm_profile_record_frame_phase(
                RXVM_PROFILE_FRAME_INHERITED_CONTEXT,
                profile_reused_frame,
                profile_phase_start, 1);
#endif
    }
    else {
#ifdef CREXX_VM_PROFILING
        profile_phase_start = rxvm_profile_frame_phase_begin();
#endif
        this->interrupt_table = rxvm_memory_alloc_bytes(
                0, sizeof(interrupt_entry) * RXSIGNAL_MAX);
        if (!this->interrupt_table) {
#ifdef CREXX_VM_PROFILING
            rxvm_profile_record_frame_release();
#endif
            if (reused_frame) {
                this->prev_free = *procedure->frame_free_list;
                *procedure->frame_free_list = this;
            }
            else (void)rxvm_memory_release(this);
            return 0;
        }
        this->interrupt_table_owned = 1;
        for (i = 0; i < RXSIGNAL_MAX; i++) {
            this->interrupt_table[i].response = RXSIGNAL_RESPONSE_IGNORE;
            this->interrupt_table[i].function = 0;
            this->interrupt_table[i].jump = 0;
            this->interrupt_table[i].frame = 0;
            this->interrupt_table[i].value_register = 0;
            this->interrupt_table[i].stack_marker = 0;
        }

        /* Set up the default interrupt mask */
        this->interrupt_table[RXSIGNAL_KILL-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_ERROR-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_OVERFLOW_UNDERFLOW-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_CONVERSION_ERROR-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_UNKNOWN_INSTRUCTION-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_NOT_IMPLEMENTED-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_FUNCTION_NOT_FOUND-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_REFERENCE_INVALID-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_OBJECT_NOT_INITIALIZED-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_RXBIN_CORRUPTION-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_CANCEL-1].response = RXSIGNAL_RESPONSE_SILENT_HALT;
        this->interrupt_table[RXSIGNAL_OUT_OF_RANGE-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_FAILURE-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_QUIT-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_TERM-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_NOTREADY-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_INVALID_ARGUMENTS-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_DIVISION_BY_ZERO-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_UNICODE_ERROR-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_POSIX_HUP-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_POSIX_INT-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_POSIX_USR1-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_POSIX_USR2-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_POSIX_CHLD-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_CHANNEL_ERROR-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_TASK_FAILURE-1].response = RXSIGNAL_RESPONSE_HALT;
        this->interrupt_table[RXSIGNAL_BREAKPOINT-1].response = RXSIGNAL_RESPONSE_IGNORE;
        this->interrupt_table[RXSIGNAL_OTHER-1].response = RXSIGNAL_RESPONSE_HALT;
        this->is_interrupt = 0; // No signals pending

        /* VM Plugins */
        this->unicode = 0;
        this->decimal = 0;

        /* Default numeric context */
        this->num_context.digits = DIGITS_STRIKE_POINT;
        this->num_context.fuzz = 0;
        this->num_context.form = NUMERIC_FORM_SCIENTIFIC;
        this->num_context.casetype = CASE_LOWER;
        this->num_context.standard = NUMERIC_STANDARD_COMMON;
#ifdef CREXX_VM_PROFILING
        rxvm_profile_record_frame_phase(
                RXVM_PROFILE_FRAME_ROOT_CONTEXT,
                profile_reused_frame,
                profile_phase_start, 1);
#endif
    }
#ifdef CREXX_VM_PROFILING
    profile_phase_start = rxvm_profile_frame_phase_begin();
#endif
    this->interrupt_stack = 0;
    this->decimal_loaded_here = 0;
    this->unicode_loaded_here = 0;
    this->return_pc = return_pc;
    this->number_locals = num_locals;
    this->number_args = no_args;
    this->return_reg = return_reg;
    this->procedure = procedure;
    this->has_reference_lifetimes = 0;
    this->is_interrupt_action = 0;
    this->caller_arg_base = UINT32_MAX;
#ifdef CREXX_VM_PROFILING
    rxvm_profile_record_frame_phase(
            RXVM_PROFILE_FRAME_FINALIZE,
            profile_reused_frame,
            profile_phase_start, 1);
#endif

    return this;
}

/* Clear Stack Frame - deallocating register contents and plugins */
RX_INLINE void clear_frame(stack_frame *frame) {
    int i, offset;
    rxsignal_clear_handler_stack(frame);
    rxsignal_release_private_interrupt_table(frame);
    /* Reset Local Registers and a0 */
    for (i = 0; i < frame->procedure->locals; i++) {
        clear_value(frame->baselocals[i]);
    }
    offset = frame->procedure->locals;
    if (frame->procedure->binarySpace) offset += frame->procedure->binarySpace->globals;
    clear_value(frame->baselocals[offset]);
    if (frame->decimal_loaded_here) {
        frame->decimal->base.free((rxvm_plugin*)frame->decimal);
        frame->decimal_loaded_here = 0;
    }
    if (frame->unicode_loaded_here) {
        frame->unicode->base.free((rxvm_plugin*)frame->unicode);
        frame->unicode_loaded_here = 0;
    }
}

/* Free Stack Frame */
RX_INLINE void free_frame(stack_frame *frame) {
#ifdef CREXX_VM_PROFILING
    rxvm_profile_record_frame_release();
#endif
    rxsignal_clear_handler_stack(frame);
    rxsignal_release_private_interrupt_table(frame);
    rxvm_release_frame_reference_lifetimes(frame);
    /* Add to free list */
    frame->prev_free = *(frame->procedure->frame_free_list);
    *(frame->procedure->frame_free_list) = frame;
}

static void free_external_entry_arguments(stack_frame *frame) {
    size_t i;
    size_t j;

    if (!frame || frame->parent || !frame->procedure || !frame->procedure->binarySpace) return;

    j = (size_t)frame->procedure->binarySpace->globals + (size_t)frame->procedure->locals + 1;
    for (i = 0; i < frame->number_args; i++, j++) {
        value *arg = frame->baselocals[j];
        if (!arg) continue;
        value_free(arg);
        frame->baselocals[j] = 0;
        frame->locals[j] = 0;
    }
}

/* Restore the active pointer permutation created by compiler call-window
 * swaps.  Call-window registers have unique base storage, so locating each
 * displaced base pointer identifies the source register for the inverse swap.
 * Values are not copied or reset: arg expose and reference mutations survive. */
static int rxvm_restore_call_argument_mapping(
        stack_frame *caller, size_t arg_base, size_t arg_count
#ifdef CREXX_VM_PROFILING
        , size_t *restored_slots
#endif
        ) {
    size_t arg;
    size_t source;

#ifdef CREXX_VM_PROFILING
    if (restored_slots) *restored_slots = 0;
#endif
    if (!caller || !arg_count) return 1;
    if (arg_base > caller->number_locals ||
        arg_count > caller->number_locals - arg_base) return 0;

    for (arg = 0; arg < arg_count; arg++) {
        size_t slot = arg_base + arg;
        value *slot_base = caller->baselocals[slot];

        if (!slot_base) return 0;
        if (caller->locals[slot] == slot_base) continue;

        for (source = 0; source < caller->number_locals; source++) {
            if (caller->locals[source] == slot_base) break;
        }
        if (source == caller->number_locals) return 0;

        {
            value *mapped = caller->locals[slot];
            caller->locals[slot] = caller->locals[source];
            caller->locals[source] = mapped;
#ifdef CREXX_VM_PROFILING
            if (restored_slots) (*restored_slots)++;
#endif
        }
    }

    return 1;
}

static int rxvm_restore_caller_call_argument_mapping(stack_frame *callee
#ifdef CREXX_VM_PROFILING
        , size_t *restored_slots
#endif
        ) {
#ifdef CREXX_VM_PROFILING
    if (restored_slots) *restored_slots = 0;
#endif
    if (!callee || !callee->parent || callee->caller_arg_base == UINT32_MAX) return 1;
    return rxvm_restore_call_argument_mapping(callee->parent,
                                              (size_t)callee->caller_arg_base,
                                              callee->number_args
#ifdef CREXX_VM_PROFILING
                                              ,
                                              restored_slots);
#else
                                              );
#endif
}

/* A native call has no child frame to carry caller_arg_base.  If a branch
 * handler remains in the interrupted frame, recover the call window from the
 * canonical CALL/DCALL or fused-call count operand on that cold signal path. */
static int rxsignal_restore_interrupted_call_argument_mapping(
        stack_frame *frame, rxinteger interrupted_module,
        rxinteger interrupted_address
#ifdef CREXX_VM_PROFILING
        , int *window_observed, size_t *restored_slots
#endif
        ) {
    bin_space *space;
    size_t address;
    size_t count_reg;
    rxinteger count;
    int opcode;

#ifdef CREXX_VM_PROFILING
    if (window_observed) *window_observed = 0;
    if (restored_slots) *restored_slots = 0;
#endif
    if (!frame || !frame->procedure || !frame->procedure->binarySpace) return 1;
    space = frame->procedure->binarySpace;
    if (!space->module || interrupted_module != (rxinteger)space->module->module_number) return 1;
    if (interrupted_address < 0) return 1;

    address = (size_t)interrupted_address;
    if (address >= space->inst_size) return 0;
    opcode = space->binary[address].instruction.opcode;
    if (opcode != OP_CALL_REG_FUNC_REG &&
        opcode != OP_DCALL_REG_REG_REG &&
        opcode != OP_SWAPCALL_REG_FUNC_REG_REG_REG &&
        opcode != OP_SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG &&
        opcode != OP_SETTPCALL_REG_FUNC_REG_REG_INT) return 1;
#ifdef CREXX_VM_PROFILING
    if (window_observed) *window_observed = 1;
#endif
    if (space->inst_size - address < 4) return 0;

    count_reg = space->binary[address + 3].index;
    if (count_reg >= frame->number_locals || !frame->locals[count_reg]) return 0;
    count = frame->locals[count_reg]->int_value;
    if (count < 0 || (uintmax_t)count > (uintmax_t)SIZE_MAX) return 0;

    return rxvm_restore_call_argument_mapping(frame, count_reg + 1,
                                               (size_t)count
#ifdef CREXX_VM_PROFILING
                                               , restored_slots
#endif
                                               );
}

static void rxsignal_restore_branch_call_argument_mapping(
        stack_frame *current, const interrupt_entry *handler,
        rxinteger interrupted_module, rxinteger interrupted_address
#ifdef CREXX_VM_PROFILING
        , rxvm_profile_state *profile
#endif
        ) {
    int mapping_restored;
#ifdef CREXX_VM_PROFILING
    int window_observed = 0;
    size_t restored_slots = 0;
#endif

    if (!current || !handler || current != handler->frame) return;
    mapping_restored = rxsignal_restore_interrupted_call_argument_mapping(
            current, interrupted_module, interrupted_address
#ifdef CREXX_VM_PROFILING
            ,
            &window_observed, &restored_slots);
#else
            );
#endif
#ifdef CREXX_VM_PROFILING
    if (profile && profile->enabled)
        rxvm_profile_record_signal_native_restore_at(
                profile, window_observed, (uint64_t)restored_slots,
                !mapping_restored);
#endif
    assert(mapping_restored);
    (void)mapping_restored;
}

static stack_frame *rxsignal_unwind_to_frame(
        stack_frame *current, stack_frame *target
#ifdef CREXX_VM_PROFILING
        , rxvm_profile_state *profile
#endif
        ) {
    stack_frame *discard;
#ifdef CREXX_VM_PROFILING
    uint64_t frames_discarded = 0;
    uint64_t windows_restored = 0;
    uint64_t slots_restored = 0;
    int restoration_failed = 0;
    uint64_t unwind_time_ns = 0;
#endif

    if (!target) return current;

    while (current && current != target) {
        int mapping_restored;
#ifdef CREXX_VM_PROFILING
        size_t restored_slots = 0;
#endif
        discard = current;
        current = current->parent;
        mapping_restored = rxvm_restore_caller_call_argument_mapping(
                discard
#ifdef CREXX_VM_PROFILING
                , &restored_slots
#endif
                );
#ifdef CREXX_VM_PROFILING
        frames_discarded++;
        if (discard->caller_arg_base != UINT32_MAX) windows_restored++;
        slots_restored += restored_slots;
        if (!mapping_restored) restoration_failed = 1;
#endif
        assert(mapping_restored);
        (void)mapping_restored;
#ifdef CREXX_VM_PROFILING
        if (profile && profile->enabled) {
            if (!unwind_time_ns) unwind_time_ns = rxvm_profile_now_ns();
            rxvm_profile_frame_unwind_at(profile, discard, unwind_time_ns);
        }
#endif
        free_frame(discard);
    }

#ifdef CREXX_VM_PROFILING
    if (profile && profile->enabled)
        rxvm_profile_record_signal_unwind_at(
                profile, frames_discarded, windows_restored, slots_restored,
                restoration_failed);
#endif

    return current ? current : target;
}

#ifdef CREXX_VM_PROFILING
#define RXVM_PROFILE_UNWIND_STATE , &vm_profile
#else
#define RXVM_PROFILE_UNWIND_STATE
#endif

static void rxsignal_populate_raw_interrupt(value *raw,
                                            unsigned char interrupt,
                                            rxinteger module,
                                            rxinteger address,
                                            value *payload) {
    value_zero(raw);
    set_num_attributes(raw, 5);
    raw->attributes[0]->int_value = (rxinteger)interrupt;
    raw->attributes[1]->int_value = module;
    raw->attributes[2]->int_value = address;
    set_null_string(raw->attributes[3], interrupt_to_string(interrupt));
    move_value(raw->attributes[4], payload);
}

static void rxsignal_populate_runtime_signal(value *dest, value *raw) {
    static const RxGraphTypeRef runtime_signal_type = {
        .name = "rxfnsb.runtime_signal",
        .name_length = sizeof("rxfnsb.runtime_signal") - 1u,
        .id = RX_GRAPH_NONE
    };

    value_zero(dest);
    set_num_attributes(dest, 6);
    dest->object_type = &runtime_signal_type;
    dest->attributes[0]->int_value = 1;
    copy_value(dest->attributes[4], raw);
}

void completely_free_frame(stack_frame *frame) {
    clear_frame(frame);
    (void)rxvm_memory_release(frame);
}

// Function to set an interrupt
void raise_signal(unsigned char signal) {
    rxvm_context *context = rxvm_active_context_current();
    if (context) {
        if (context->active.pending_interrupts)
            rxvm_signal_pending_or(context->active.pending_interrupts,
                                   rxsignal_mask(signal));
        return;
    }
    rxvm_signal_raise_process_main(signal);
}

// Function to clear an interrupt
void clear_signal(unsigned char signal) {
    rxvm_context *context = rxvm_active_context_current();
    if (context) {
        if (context->active.pending_interrupts)
            rxvm_signal_pending_and(context->active.pending_interrupts,
                                    ~rxsignal_mask(signal));
        return;
    }
    rxvm_signal_clear_process_main(signal);
}

static RXVM_LABEL_OWNER_NOINLINE void rxsignal_raise_private_table_oom(
        volatile sig_atomic_t *pending_interrupts,
        stack_frame *frame,
        bin_code **interrupted_pc,
        bin_code *pc,
        value **interrupt_object) {
    if (!frame->is_interrupt) *interrupted_pc = pc;
    rxvm_signal_pending_or(pending_interrupts,
                           rxsignal_mask(RXSIGNAL_FAILURE));
    value_zero(interrupt_object[RXSIGNAL_FAILURE]);
    set_null_string(interrupt_object[RXSIGNAL_FAILURE],
                    "Unable to allocate private interrupt table");
}

// Macro to detect and throw a signal if a RXVM plugin-raised error is present
#define RXSIGNAL_IF_RXVM_PLUGIN_ERROR(signal) \
if ((signal)->base.signal_number > RXSIGNAL_NONE && (signal)->base.signal_number < RXSIGNAL_MAX) { \
if (!current_frame->is_interrupt) interrupted_pc = pc; \
rxvm_signal_pending_or(&pending_interrupts, \
        rxsignal_mask((signal)->base.signal_number)); \
value_zero(interrupt_object[(signal)->base.signal_number]); \
set_null_string(interrupt_object[(signal)->base.signal_number], (signal)->base.signal_string); \
}

// Macro to throw a signal
#define SET_SIGNAL(signal) \
{if (!current_frame->is_interrupt) interrupted_pc = pc; \
rxvm_signal_pending_or(&pending_interrupts, rxsignal_mask(signal)); \
value_zero(interrupt_object[(signal)]);}

// Macro to throw a signal with a message
#define SET_SIGNAL_MSG(signal, message) \
{if (!current_frame->is_interrupt) interrupted_pc = pc; \
rxvm_signal_pending_or(&pending_interrupts, rxsignal_mask(signal)); \
value_zero(interrupt_object[(signal)]); \
set_null_string(interrupt_object[(signal)], (message));}

// Macro to throw a signal with a payload
#define SET_SIGNAL_PAYLOAD(signal, payload) \
{if (!current_frame->is_interrupt) interrupted_pc = pc; \
rxvm_signal_pending_or(&pending_interrupts, rxsignal_mask(signal)); \
copy_value(interrupt_object[(signal)], (payload));}

#define SET_SIGNAL_FROM_NAME(name) \
{ unsigned char signal__ = string_to_interrupt((name)); \
if (signal__ == RXSIGNAL_MAX) { SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE); } \
else { SET_SIGNAL(signal__); } }

#define SET_SIGNAL_MSG_FROM_NAME(name, message) \
{ unsigned char signal__ = string_to_interrupt((name)); \
if (signal__ == RXSIGNAL_MAX) { SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE); } \
else { SET_SIGNAL_MSG(signal__, (message)); } }

#define SET_SIGNAL_PAYLOAD_FROM_NAME(name, payload) \
{ unsigned char signal__ = string_to_interrupt((name)); \
if (signal__ == RXSIGNAL_MAX) { SET_SIGNAL(RXSIGNAL_INVALID_SIGNAL_CODE); } \
else { SET_SIGNAL_PAYLOAD(signal__, (payload)); } }

#ifndef NUTF8
#define REQUIRE_VALID_UTF8_REGISTER(reg) \
do { \
    if (!has_utf8_valid_count_or_empty((reg))) refresh_utf8_flags((reg)); \
    if (!has_utf8_valid_count_or_empty((reg))) { \
        SET_SIGNAL_MSG(RXSIGNAL_UNICODE_ERROR, "Invalid UTF-8 string operand"); \
        DISPATCH; \
    } \
} while (0)
#else
#define REQUIRE_VALID_UTF8_REGISTER(reg) do { } while (0)
#endif

// Macro and function to detect and throw a signal if a RXPA plugin-raised error is present
#define INTERRUPT_FROM_RXPA_SIGNAL(signal) if ((signal)->int_value || (signal)->string_length) { if (!current_frame->is_interrupt) interrupted_pc = pc; interrupt_from_rxpa_signal(signal,interrupt_object); }

void interrupt_from_rxpa_signal(value *signal, value* interrupt_object[RXSIGNAL_MAX]) {
    size_t int_num;

    if (signal->int_value < 1 || signal->int_value >= RXSIGNAL_MAX) {
        null_terminate_string_buffer(signal);
        int_num = string_to_interrupt(signal->string_value);
        if (int_num == RXSIGNAL_MAX) {
            int_num = RXSIGNAL_OTHER;
            value_zero(interrupt_object[int_num]);
            set_null_string(interrupt_object[int_num], signal->string_value);
        }
        else {
            value_zero(interrupt_object[int_num]);
        }
    } else {
        int_num = signal->int_value;
        value_zero(interrupt_object[int_num]);
    }

    // Set the interrupt
    {
        rxvm_context *context = rxvm_active_context_current();
        if (context) {
            if (context->active.pending_interrupts)
                rxvm_signal_pending_or(context->active.pending_interrupts,
                                       rxsignal_mask((int)int_num));
        } else {
            rxvm_signal_raise_process_main((unsigned char)int_num);
        }
    }
}

#define HANDLE_INTERRUPT_ACTION_RETURN() \
if (is_interrupt && temp_frame->is_interrupt_action) { \
    rxsignal_handler_action action__ = rxsignal_read_handler_action(interrupt_action_value); \
    if (action__ != RXSIGNAL_HANDLER_ACTION_SKIP) { \
        value *payload__ = rxsignal_handler_payload(temp_frame); \
        if (payload__ && payload__->string_length) { \
            fprintf(stderr, "PANIC: %.*s (SIGNAL %s)\n", (int)(payload__->string_length), payload__->string_value, interrupt_to_string(is_interrupt)); \
        } else { \
            fprintf(stderr, "PANIC: (SIGNAL %s)\n", interrupt_to_string(is_interrupt)); \
        } \
        print_runtime_panic_location(context, last_interrupted_module[is_interrupt], last_interrupted_address[is_interrupt]); \
        value_zero(interrupt_action_value); \
        rc = (int)is_interrupt; \
        free_frame(temp_frame); \
        RXVM_HANDLER_FINISH(); \
    } \
    value_zero(interrupt_action_value); \
}

#define IS_UNICODE_WHITESPACE(cp) ( \
    (cp) == 0x0009 || /* Tab */       \
    (cp) == 0x000A || /* Line Feed */ \
    (cp) == 0x000B || /* Vertical Tab */ \
    (cp) == 0x000C || /* Form Feed */ \
    (cp) == 0x000D || /* Carriage Return */ \
    (cp) == 0x0020 || /* Space */     \
    (cp) == 0x0085 || /* Next Line */ \
    (cp) == 0x00A0 || /* No-Break Space */ \
    (cp) == 0x1680 || \
    ((cp) >= 0x2000 && (cp) <= 0x200A) || \
    (cp) == 0x2028 || \
    (cp) == 0x2029 || \
    (cp) == 0x202F || \
    (cp) == 0x205F || \
    (cp) == 0x3000 )

/* -------------------------------------------------------------------------
 * Inline helper: forward ASCII non-blank scan.
 * Returns index (0-based) of first non-blank, or -len if none found.
 * -------------------------------------------------------------------------
 */
#define ASCII_FAST_PATH 1    // 1. activate ASCII fast path, 0: run normal mode
#if  ASCII_FAST_PATH
RX_INLINE rxinteger ascii_fwd_nonblank(const unsigned char *s, rxinteger start, rxinteger len) {
    rxinteger i;
    int ch;
    for (i = start; i < len; i++) {
        ch = (unsigned char)s[i];
       if (!IS_UNICODE_WHITESPACE(ch)) return i;
    }
    return -1;  /* Not found in forward scan */
}
RX_INLINE rxinteger ascii_back_nonblank( unsigned char *s, rxinteger start, rxinteger len) {
    rxinteger i;
    int ch;
    if (len <= 0) return -1;
    for (i = start; i >= 0; --i) {
        ch = (unsigned char)s[i];
        if (!IS_UNICODE_WHITESPACE(ch)) return i;
    }
    return -1;  /* Not found in reverse scan */
}

RX_INLINE rxinteger ascii_fwd_blank(const unsigned char *s, rxinteger start, rxinteger len) {
    rxinteger i;
    int ch;
    for (i = start; i < len; i++) {
        ch = (unsigned char)s[i];
        if (IS_UNICODE_WHITESPACE(ch)) return i;
    }
    return -1;  /* Not found in forward scan */
}
RX_INLINE rxinteger ascii_back_blank( unsigned char *s, rxinteger start, rxinteger len) {
    rxinteger i;
    int ch;
    if (len <= 0) return -1;
    for (i = start; i >= 0; --i) {
        ch = (unsigned char)s[i];
        if (IS_UNICODE_WHITESPACE(ch)) return i;
    }
    return -1;  /* Not found in reverse scan */
}
#endif

static const Instruction rxvm_instruction_meta_map[OP_MAX_INSTRUCTIONS] = {
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) \
    { OPCODE, #NAME, DESC, sizeof(FMT) - 1u, FMT },
#include "../binutils/include/rxops.h"
#undef X
};

#define meta_map rxvm_instruction_meta_map

/* Both engines use the same execution-image preparation helper. The threaded
 * owner supplies its local label table and private-label addresses; the switch
 * engine stores numeric private opcodes instead. */
static void rxvm_prepare_execution_image(
        module *vm_module__, const void *const *handler_address_map__,
        void *private_r2_handler__, void *private_r1_handler__) {
    size_t vm_bytes__;
    size_t vm_i__ = 0;
    int vm_new_image__ = vm_module__->execution_image == 0;

    if (!vm_module__->segment.inst_size) return;
    if (vm_new_image__) {
        if (!rxvm_checked_size_mul(sizeof(bin_code),
                                   vm_module__->segment.inst_size,
                                   &vm_bytes__)) {
            RX_PANIC_OOM("size rxvm execution image", (size_t)-1,
                         vm_module__->name);
        }
        vm_module__->execution_image = rxvm_memory_alloc_bytes(
                vm_module__->memory_worker, vm_bytes__);
        if (!vm_module__->execution_image) {
            RX_PANIC_OOM("malloc rxvm execution image", vm_bytes__,
                         vm_module__->name);
        }
        memcpy(vm_module__->execution_image, vm_module__->segment.binary,
               vm_bytes__);
    }
    while (vm_i__ < vm_module__->segment.inst_size) {
        size_t vm_instruction__ = vm_i__;
        size_t vm_operand_count__ =
                vm_module__->segment.binary[vm_instruction__].instruction.no_ops;
        unsigned int vm_opcode__ =
                vm_module__->segment.binary[vm_instruction__].instruction.opcode;
        size_t vm_operand__;
        vm_i__ += vm_operand_count__ + 1;
        if (!vm_new_image__ && vm_operand_count__) {
            memcpy(vm_module__->execution_image + vm_instruction__ + 1,
                   vm_module__->segment.binary + vm_instruction__ + 1,
                   sizeof(bin_code) * vm_operand_count__);
        }
        if (vm_opcode__ < OP_MAX_INSTRUCTIONS) {
            for (vm_operand__ = 0; vm_operand__ < vm_operand_count__;
                 vm_operand__++) {
                if (rxop_format_operand_type(meta_map[vm_opcode__].format,
                                             vm_operand__) == OP_FUNC) {
                    size_t vm_offset__ = vm_module__->segment.binary[
                            vm_instruction__ + vm_operand__ + 1].index;
                    vm_module__->execution_image[
                            vm_instruction__ + vm_operand__ + 1].handler =
                            (void *)rxvm_get_module_runtime_procedure(
                                    vm_module__, vm_offset__);
                }
            }
        }
#ifdef NTHREADED
        (void)handler_address_map__;
        (void)private_r2_handler__;
        (void)private_r1_handler__;
#else
        vm_module__->execution_image[vm_instruction__].handler =
                vm_opcode__ < OP_MAX_INSTRUCTIONS
                    ? (void *)handler_address_map__[vm_opcode__]
                    : (void *)handler_address_map__[OP_IUNKNOWN];
#endif
        if (rxvm_private_r2_copyattr1_candidate(vm_module__, vm_instruction__)) {
#ifdef NTHREADED
            vm_module__->execution_image[vm_instruction__].instruction.opcode =
                    RXVM_PRIVATE_R2_COPYATTR1_REG_REG_INT;
#else
            vm_module__->execution_image[vm_instruction__].handler =
                    private_r2_handler__;
#endif
        }
        if (rxvm_private_r1_relink_candidate(vm_module__, vm_instruction__)) {
#ifdef NTHREADED
            vm_module__->execution_image[vm_instruction__].instruction.opcode =
                    RXVM_PRIVATE_R1_RELINK_REG_REG;
#else
            vm_module__->execution_image[vm_instruction__].handler =
                    private_r1_handler__;
#endif
        }
    }
}

typedef int instructions;

#define VM_PREPARE_EXECUTION_IMAGE(module_)                                    \
    rxvm_prepare_execution_image((module_), address_map,                       \
                                 private_r2_handler, private_r1_handler)

#define RXVM_SWAP_PAIR(first_, second_)                                        \
    do {                                                                       \
        value *swap_value__ = REG_OP(first_);                                  \
        REG_OP(first_) = REG_OP(second_);                                      \
        REG_OP(second_) = swap_value__;                                        \
        RXVM_INSTRUMENTATION_SWAP(current_frame, REG_IDX(first_),               \
                                  REG_IDX(second_));                            \
    } while (0)

typedef enum rxvm_handler_result {
    RXVM_HANDLER_RESULT_DISPATCH = 0,
    RXVM_HANDLER_RESULT_INTERRUPT,
    RXVM_HANDLER_RESULT_RESUME,
    RXVM_HANDLER_RESULT_INTERRUPT_TABLE_OOM,
    RXVM_HANDLER_RESULT_FINISHED
} rxvm_handler_result;

#if RXVM_HANDLER_USE_POINTER_FACADE
/* Preserve the R2 all-inline source shape exactly. Although this facade is
 * optimized away when every handler expands in the owner, deleting it changes
 * threaded compiler heuristics and generated layout measurably. */
typedef struct rxvm_handler_state {
    rxvm_context *context;
    int *rc;
    unsigned int *initSeed;
    char *hasSeed;
    bin_code **pc;
    bin_code **next_pc;
    bin_code **interrupted_pc;
    int *mod_index;
    value **signal_value;
    value **interrupt_action_value;
    value **arguments_array;
    value **interrupt_object;
    rxinteger *last_interrupted_address;
    rxinteger *last_interrupted_module;
    stack_frame **current_frame;
    stack_frame **temp_frame;
    bin_space **current_binary_space;
    bin_code **current_execution_base;
    bin_code **current_canonical_base;
    unsigned char **current_const_pool;
    value ***current_locals;
    value **work1;
    module **current_module;
    volatile sig_atomic_t *pending_interrupts;
    const void *const *address_map;
    void *private_r2_handler;
    void *private_r1_handler;
#ifndef NTHREADED
    void **next_inst;
#endif
#ifdef CREXX_VM_PROFILING
    rxvm_profile_state *profile;
    rxvm_sequence_state *sequence;
#endif
#ifdef CREXX_VM_INSTRUMENTATION_TEST
    rxvm_test_instrumentation_state *test_instrumentation;
#endif
} rxvm_handler_state;
#else
/* Snapshot used only on an outlined-handler edge. Keeping scalar values in the
 * snapshot avoids making the hot run() locals addressable merely because one
 * or more cold instruction handlers are outlined. */
typedef struct rxvm_handler_state {
    rxvm_context *context;
    int rc;
    unsigned int initSeed;
    char hasSeed;
    bin_code *pc;
    bin_code *next_pc;
    bin_code *interrupted_pc;
    int mod_index;
    value *signal_value;
    value *interrupt_action_value;
    value *arguments_array;
    value **interrupt_object;
    rxinteger *last_interrupted_address;
    rxinteger *last_interrupted_module;
    stack_frame *current_frame;
    stack_frame *temp_frame;
    bin_space *current_binary_space;
    bin_code *current_execution_base;
    bin_code *current_canonical_base;
    unsigned char *current_const_pool;
    value **current_locals;
    value *work1;
    module *current_module;
    volatile sig_atomic_t *pending_interrupts;
    const void *const *address_map;
    void *private_r2_handler;
    void *private_r1_handler;
#ifndef NTHREADED
    void *next_inst;
#endif
#ifdef CREXX_VM_PROFILING
    rxvm_profile_state *profile;
    rxvm_sequence_state *sequence;
#endif
#ifdef CREXX_VM_INSTRUMENTATION_TEST
    rxvm_test_instrumentation_state *test_instrumentation;
#endif
} rxvm_handler_state;

#ifndef NTHREADED
#define RXVM_HANDLER_STATE_SNAPSHOT_THREADED(state_) \
    (state_).next_inst = next_inst
#define RXVM_HANDLER_STATE_COMMIT_THREADED(state_) \
    next_inst = (state_).next_inst
#else
#define RXVM_HANDLER_STATE_SNAPSHOT_THREADED(state_) ((void)0)
#define RXVM_HANDLER_STATE_COMMIT_THREADED(state_) ((void)0)
#endif

#ifdef CREXX_VM_PROFILING
#define RXVM_HANDLER_STATE_SNAPSHOT_PROFILE(state_) \
    do {                                             \
        (state_).profile = &vm_profile;              \
        (state_).sequence = &vm_sequence;            \
    } while (0)
#else
#define RXVM_HANDLER_STATE_SNAPSHOT_PROFILE(state_) ((void)0)
#endif

#ifdef CREXX_VM_INSTRUMENTATION_TEST
#define RXVM_HANDLER_STATE_SNAPSHOT_INSTRUMENTATION(state_) \
    (state_).test_instrumentation = &vm_instrumentation
#else
#define RXVM_HANDLER_STATE_SNAPSHOT_INSTRUMENTATION(state_) ((void)0)
#endif

#define RXVM_HANDLER_STATE_SNAPSHOT(state_)                                 \
    do {                                                                    \
        (state_).context = context;                                         \
        (state_).rc = rc;                                                   \
        (state_).initSeed = initSeed;                                       \
        (state_).hasSeed = hasSeed;                                         \
        (state_).pc = pc;                                                   \
        (state_).next_pc = next_pc;                                         \
        (state_).interrupted_pc = interrupted_pc;                           \
        (state_).mod_index = mod_index;                                     \
        (state_).signal_value = signal_value;                               \
        (state_).interrupt_action_value = interrupt_action_value;           \
        (state_).arguments_array = arguments_array;                         \
        (state_).interrupt_object = interrupt_object;                       \
        (state_).last_interrupted_address = last_interrupted_address;       \
        (state_).last_interrupted_module = last_interrupted_module;         \
        (state_).current_frame = current_frame;                             \
        (state_).temp_frame = temp_frame;                                   \
        (state_).current_binary_space = current_binary_space;               \
        (state_).current_execution_base = current_execution_base;           \
        (state_).current_canonical_base = current_canonical_base;           \
        (state_).current_const_pool = current_const_pool;                   \
        (state_).current_locals = current_locals;                           \
        (state_).work1 = work1;                                             \
        (state_).current_module = current_module;                           \
        (state_).pending_interrupts = &pending_interrupts;                  \
        (state_).address_map = address_map;                                 \
        (state_).private_r2_handler = private_r2_handler;                   \
        (state_).private_r1_handler = private_r1_handler;                   \
        RXVM_HANDLER_STATE_SNAPSHOT_THREADED(state_);                       \
        RXVM_HANDLER_STATE_SNAPSHOT_PROFILE(state_);                        \
        RXVM_HANDLER_STATE_SNAPSHOT_INSTRUMENTATION(state_);                \
    } while (0)

#define RXVM_HANDLER_STATE_COMMIT(state_)                                   \
    do {                                                                    \
        rc = (state_).rc;                                                   \
        initSeed = (state_).initSeed;                                       \
        hasSeed = (state_).hasSeed;                                         \
        pc = (state_).pc;                                                   \
        next_pc = (state_).next_pc;                                         \
        interrupted_pc = (state_).interrupted_pc;                           \
        mod_index = (state_).mod_index;                                     \
        signal_value = (state_).signal_value;                               \
        interrupt_action_value = (state_).interrupt_action_value;           \
        arguments_array = (state_).arguments_array;                         \
        current_frame = (state_).current_frame;                             \
        temp_frame = (state_).temp_frame;                                   \
        current_binary_space = (state_).current_binary_space;               \
        current_execution_base = (state_).current_execution_base;           \
        current_canonical_base = (state_).current_canonical_base;           \
        current_const_pool = (state_).current_const_pool;                   \
        current_locals = (state_).current_locals;                           \
        work1 = (state_).work1;                                             \
        current_module = (state_).current_module;                           \
        RXVM_HANDLER_STATE_COMMIT_THREADED(state_);                         \
    } while (0)

typedef rxvm_handler_result (*rxvm_handler_function)(rxvm_handler_state *);

static RXVM_HELPER_NOINLINE RXVM_HELPER_COLD rxvm_handler_result
rxvm_invoke_outlined_handler(rxvm_handler_function function,
                             rxvm_handler_state *state) {
    return function(state);
}
#endif

#define context (rxvm_state->context)
#if RXVM_HANDLER_USE_POINTER_FACADE
#define rc (*rxvm_state->rc)
#define initSeed (*rxvm_state->initSeed)
#define hasSeed (*rxvm_state->hasSeed)
#define pc (*rxvm_state->pc)
#define next_pc (*rxvm_state->next_pc)
#define interrupted_pc (*rxvm_state->interrupted_pc)
#define mod_index (*rxvm_state->mod_index)
#define signal_value (*rxvm_state->signal_value)
#define interrupt_action_value (*rxvm_state->interrupt_action_value)
#define arguments_array (*rxvm_state->arguments_array)
#define interrupt_object (rxvm_state->interrupt_object)
#define last_interrupted_address (rxvm_state->last_interrupted_address)
#define last_interrupted_module (rxvm_state->last_interrupted_module)
#define current_frame (*rxvm_state->current_frame)
#define temp_frame (*rxvm_state->temp_frame)
#define current_binary_space (*rxvm_state->current_binary_space)
#define current_execution_base (*rxvm_state->current_execution_base)
#define current_canonical_base (*rxvm_state->current_canonical_base)
#define current_const_pool (*rxvm_state->current_const_pool)
#define current_locals (*rxvm_state->current_locals)
#define work1 (*rxvm_state->work1)
#define current_module (*rxvm_state->current_module)
#ifndef NTHREADED
#define next_inst (*rxvm_state->next_inst)
#endif
#else
#define rc (rxvm_state->rc)
#define initSeed (rxvm_state->initSeed)
#define hasSeed (rxvm_state->hasSeed)
#define pc (rxvm_state->pc)
#define next_pc (rxvm_state->next_pc)
#define interrupted_pc (rxvm_state->interrupted_pc)
#define mod_index (rxvm_state->mod_index)
#define signal_value (rxvm_state->signal_value)
#define interrupt_action_value (rxvm_state->interrupt_action_value)
#define arguments_array (rxvm_state->arguments_array)
#define interrupt_object (rxvm_state->interrupt_object)
#define last_interrupted_address (rxvm_state->last_interrupted_address)
#define last_interrupted_module (rxvm_state->last_interrupted_module)
#define current_frame (rxvm_state->current_frame)
#define temp_frame (rxvm_state->temp_frame)
#define current_binary_space (rxvm_state->current_binary_space)
#define current_execution_base (rxvm_state->current_execution_base)
#define current_canonical_base (rxvm_state->current_canonical_base)
#define current_const_pool (rxvm_state->current_const_pool)
#define current_locals (rxvm_state->current_locals)
#define work1 (rxvm_state->work1)
#define current_module (rxvm_state->current_module)
#ifndef NTHREADED
#define next_inst (rxvm_state->next_inst)
#endif
#endif
#define pending_interrupts (*rxvm_state->pending_interrupts)
#define address_map (rxvm_state->address_map)
#define private_r2_handler (rxvm_state->private_r2_handler)
#define private_r1_handler (rxvm_state->private_r1_handler)
#ifdef CREXX_VM_PROFILING
#define vm_profile (*rxvm_state->profile)
#define vm_sequence (*rxvm_state->sequence)
#endif
#ifdef CREXX_VM_INSTRUMENTATION_TEST
#define vm_instrumentation (*rxvm_state->test_instrumentation)
#endif

/* Handler bodies are passed through RXVM_HANDLER as macro arguments. Keep all
 * build selection outside those arguments: preprocessing directives inside a
 * function-like macro invocation are undefined and MSVC leaves them in the
 * expanded C source. */
#ifdef NUTF8
#define RXVM_UTF8_ONLY(...)
#define RXVM_BYTE_ONLY(...) __VA_ARGS__
#else
#define RXVM_UTF8_ONLY(...) __VA_ARGS__
#define RXVM_BYTE_ONLY(...)
#endif

#if ASCII_FAST_PATH
#define RXVM_ASCII_FAST_ONLY(...) __VA_ARGS__
#else
#define RXVM_ASCII_FAST_ONLY(...)
#endif

#ifdef _WIN32
#define RXVM_WINDOWS_ONLY(...) __VA_ARGS__
#define RXVM_NONWINDOWS_ONLY(...)
#else
#define RXVM_WINDOWS_ONLY(...)
#define RXVM_NONWINDOWS_ONLY(...) __VA_ARGS__
#endif

#if defined(__linux__)
#define RXVM_PLATFORM_NAME "linux"
#elif defined(_WIN32)
#define RXVM_PLATFORM_NAME "windows"
#elif defined(__APPLE__)
#define RXVM_PLATFORM_NAME "macOS"
#elif defined(__CMS__)
#define RXVM_PLATFORM_NAME "cms"
#else
#define RXVM_PLATFORM_NAME "unknown"
#endif

#undef DISPATCH
#define DISPATCH                                                               \
    do {                                                                       \
        RXVM_DISPATCH_PREPARE();                                               \
        if (pending_interrupts && !current_frame->is_interrupt)                \
            return RXVM_HANDLER_RESULT_INTERRUPT;                              \
        return RXVM_HANDLER_RESULT_DISPATCH;                                   \
    } while (0)
#undef VM_RESUME_INTERRUPTED
#define VM_RESUME_INTERRUPTED(signal_)                                         \
    do {                                                                       \
        RXVM_RESUME_INTERRUPTED_PREPARE(signal_);                              \
        return RXVM_HANDLER_RESULT_RESUME;                                     \
    } while (0)
#undef RXVM_HANDLER_FINISH
#define RXVM_HANDLER_FINISH() return RXVM_HANDLER_RESULT_FINISHED
#undef RXVM_HANDLER_INTERRUPT_TABLE_OOM
#define RXVM_HANDLER_INTERRUPT_TABLE_OOM()                                    \
    return RXVM_HANDLER_RESULT_INTERRUPT_TABLE_OOM
#define RXVM_HANDLER(name_, ...)                                               \
    static RXVM_HELPER_NOINLINE rxvm_handler_result                            \
            rxvm_handler_##name_(rxvm_handler_state *rxvm_state) {             \
        __VA_ARGS__                                                            \
    }
#define RXVM_PRIVATE_HANDLER(name_, dispatch_opcode_, profile_opcode_, ...)     \
    RXVM_HANDLER(name_, __VA_ARGS__)

#include "rxvmhandlers_core.inc"
#include "rxvmhandlers_control.inc"
#include "rxvmhandlers_numeric.inc"
#include "rxvmhandlers_string.inc"
#include "rxvmhandlers_system.inc"
#include "rxvmhandlers_channel.inc"

#undef RXVM_PRIVATE_HANDLER
#undef RXVM_HANDLER

#if !RXVM_HANDLER_USE_POINTER_FACADE
#define RXVM_HANDLER_FUNCTION_INLINE(name_) 0
#define RXVM_HANDLER_FUNCTION_OUTLINE(name_) rxvm_handler_ ## name_
#define RXVM_HANDLER_FUNCTION_SELECT_INNER(policy_) \
    RXVM_HANDLER_FUNCTION_ ## policy_
#define RXVM_HANDLER_FUNCTION_SELECT(policy_) \
    RXVM_HANDLER_FUNCTION_SELECT_INNER(policy_)
#define RXVM_HANDLER_FUNCTION(name_) \
    RXVM_HANDLER_FUNCTION_SELECT(RXVM_HANDLER_POLICY(name_))(name_)

static rxvm_handler_function const
rxvm_outlined_handler_functions[RXVM_PRIVATE_R1_RELINK_REG_REG + 1] = {
#define RXVM_HANDLER(name_, ...) \
    [OP_ ## name_] = RXVM_HANDLER_FUNCTION(name_),
#define RXVM_PRIVATE_HANDLER(name_, dispatch_opcode_, profile_opcode_, ...) \
    [dispatch_opcode_] = RXVM_HANDLER_FUNCTION(name_),
#include "rxvmhandlers_core.inc"
#include "rxvmhandlers_control.inc"
#include "rxvmhandlers_numeric.inc"
#include "rxvmhandlers_string.inc"
#include "rxvmhandlers_system.inc"
#include "rxvmhandlers_channel.inc"
#undef RXVM_PRIVATE_HANDLER
#undef RXVM_HANDLER
};

#ifdef CREXX_VM_PROFILING
static const unsigned char
rxvm_handler_inline_placements[RXVM_PRIVATE_R1_RELINK_REG_REG + 1] = {
#define RXVM_HANDLER(name_, ...) \
    [OP_ ## name_] = RXVM_HANDLER_IS_INLINE(name_),
#define RXVM_PRIVATE_HANDLER(name_, dispatch_opcode_, profile_opcode_, ...) \
    [dispatch_opcode_] = RXVM_HANDLER_IS_INLINE(name_),
#include "rxvmhandlers_core.inc"
#include "rxvmhandlers_control.inc"
#include "rxvmhandlers_numeric.inc"
#include "rxvmhandlers_string.inc"
#include "rxvmhandlers_system.inc"
#include "rxvmhandlers_channel.inc"
#undef RXVM_PRIVATE_HANDLER
#undef RXVM_HANDLER
};
#define RXVM_HANDLER_EFFECTIVE_INLINE(dispatch_opcode_)                       \
    ((dispatch_opcode_) <                                                    \
                    sizeof(rxvm_handler_inline_placements) /                  \
                    sizeof(rxvm_handler_inline_placements[0])                 \
            ? rxvm_handler_inline_placements[(dispatch_opcode_)]             \
            : RXVM_HANDLER_IS_INLINE(IUNKNOWN))
#else
#define RXVM_HANDLER_EFFECTIVE_INLINE(dispatch_opcode_) 0
#endif

#undef RXVM_HANDLER_FUNCTION
#undef RXVM_HANDLER_FUNCTION_SELECT
#undef RXVM_HANDLER_FUNCTION_SELECT_INNER
#undef RXVM_HANDLER_FUNCTION_OUTLINE
#undef RXVM_HANDLER_FUNCTION_INLINE
#endif

#undef RXVM_HANDLER_INTERRUPT_TABLE_OOM
#define RXVM_HANDLER_INTERRUPT_TABLE_OOM() goto interrupt_table_oom
#undef RXVM_HANDLER_FINISH
#define RXVM_HANDLER_FINISH() goto interprt_finished
#undef VM_RESUME_INTERRUPTED
#define VM_RESUME_INTERRUPTED(signal_) RXVM_OWNER_RESUME_INTERRUPTED(signal_)
#undef DISPATCH
#define DISPATCH RXVM_OWNER_DISPATCH()

#ifdef CREXX_VM_INSTRUMENTATION_TEST
#undef vm_instrumentation
#endif
#ifdef CREXX_VM_PROFILING
#undef vm_sequence
#undef vm_profile
#endif
#ifndef NTHREADED
#undef next_inst
#endif
#undef pending_interrupts
#undef private_r1_handler
#undef private_r2_handler
#undef address_map
#undef current_module
#undef work1
#undef current_locals
#undef current_const_pool
#undef current_canonical_base
#undef current_execution_base
#undef current_binary_space
#undef temp_frame
#undef current_frame
#undef last_interrupted_module
#undef last_interrupted_address
#undef interrupt_object
#undef arguments_array
#undef interrupt_action_value
#undef signal_value
#undef mod_index
#undef interrupted_pc
#undef next_pc
#undef pc
#undef hasSeed
#undef initSeed
#undef rc
#undef context

RX_INLINE sig_atomic_t rxvm_compatibility_pending_load(
        volatile sig_atomic_t *pending) {
#if defined(_WIN32) && defined(_MSC_VER)
    typedef char rxvm_compatibility_pending_must_match_long[
            sizeof(sig_atomic_t) == sizeof(LONG) ? 1 : -1];
    (void)sizeof(rxvm_compatibility_pending_must_match_long);
    return (sig_atomic_t)ReadAcquire((LONG const volatile *)pending);
#elif defined(__GNUC__) || defined(__clang__)
    return __atomic_load_n(pending, __ATOMIC_ACQUIRE);
#else
    return *pending;
#endif
}

typedef enum rxvm_sparse_safepoint_kind {
    RXVM_SPARSE_SAFEPOINT_NONE = 0,
    RXVM_SPARSE_SAFEPOINT_BACKEDGE,
    RXVM_SPARSE_SAFEPOINT_BOUNDARY
} rxvm_sparse_safepoint_kind;

/* This table is consulted only after an outlined sparse-owner handler. Inline
 * handlers use the compile-time VM_SELECT_* hook below. Keep the cases aligned
 * with the semantic audit in PERF3-13-E5-NATIVE-DOORBELL-DESIGN.md. */
static const unsigned char
rxvm_sparse_safepoint_kinds[RXVM_PRIVATE_R1_RELINK_REG_REG + 1] = {
#define RXVM_SPARSE_BACKEDGE(op_) [OP_ ## op_] = RXVM_SPARSE_SAFEPOINT_BACKEDGE
#define RXVM_SPARSE_BOUNDARY(op_) [OP_ ## op_] = RXVM_SPARSE_SAFEPOINT_BOUNDARY
    RXVM_SPARSE_BACKEDGE(BR_ID),
    RXVM_SPARSE_BACKEDGE(BRT_ID_REG),
    RXVM_SPARSE_BACKEDGE(BRF_ID_REG),
    RXVM_SPARSE_BACKEDGE(BRTF_ID_ID_REG),
    RXVM_SPARSE_BACKEDGE(BEQ_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(BEQ_ID_REG_INT),
    RXVM_SPARSE_BACKEDGE(BNE_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(BNE_ID_REG_INT),
    RXVM_SPARSE_BACKEDGE(UNLINKBR_REG_ID),
    RXVM_SPARSE_BACKEDGE(FGTBR_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(FLTBR_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(IGTBR_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(ILTBR_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(BCT_ID_REG),
    RXVM_SPARSE_BACKEDGE(BCT_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(BCTNM_ID_REG),
    RXVM_SPARSE_BACKEDGE(BCTNM_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(BCTP_ID_REG),
    RXVM_SPARSE_BACKEDGE(BCF_ID_REG),
    RXVM_SPARSE_BACKEDGE(BCF_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(BGT_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(BGT_ID_REG_INT),
    RXVM_SPARSE_BACKEDGE(BGE_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(BGE_ID_REG_INT),
    RXVM_SPARSE_BACKEDGE(BLT_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(BLT_ID_REG_INT),
    RXVM_SPARSE_BACKEDGE(BLE_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(BLE_ID_REG_INT),
    RXVM_SPARSE_BACKEDGE(DGTBR_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(DLTBR_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(DEQBR_ID_REG_REG),
    RXVM_SPARSE_BACKEDGE(BRTPT_ID_REG),
    RXVM_SPARSE_BACKEDGE(BRTPANDT_ID_REG_INT),
    RXVM_SPARSE_BACKEDGE(JUMPS_REG_BINARY),
    RXVM_SPARSE_BACKEDGE(JUMPB_REG_BINARY),
    RXVM_SPARSE_BACKEDGE(JUMPBS_REG_REG_BINARY),
    RXVM_SPARSE_BACKEDGE(JUMPI_REG_BINARY),
    RXVM_SPARSE_BACKEDGE(JUMPR_REG_BINARY),
    RXVM_SPARSE_BACKEDGE(JUMPN_REG_BINARY),
    RXVM_SPARSE_BOUNDARY(CALL_FUNC),
    RXVM_SPARSE_BOUNDARY(CALL_REG_FUNC),
    RXVM_SPARSE_BOUNDARY(CALL_REG_FUNC_REG),
    RXVM_SPARSE_BOUNDARY(DCALL_REG_REG_REG),
    RXVM_SPARSE_BOUNDARY(SWAPCALL_REG_FUNC_REG_REG_REG),
    RXVM_SPARSE_BOUNDARY(SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG),
    RXVM_SPARSE_BOUNDARY(SETTPCALL_REG_FUNC_REG_REG_INT),
    RXVM_SPARSE_BOUNDARY(CALL1_REG_FUNC_REG),
    RXVM_SPARSE_BOUNDARY(CALL2_REG_FUNC_REG_REG),
    RXVM_SPARSE_BOUNDARY(CALL3_REG_FUNC_REG_REG_REG),
    RXVM_SPARSE_BOUNDARY(CALL4_REG_FUNC_REG_REG_REG_REG),
    RXVM_SPARSE_BOUNDARY(RET),
    RXVM_SPARSE_BOUNDARY(RET_REG),
    RXVM_SPARSE_BOUNDARY(RET_INT),
    RXVM_SPARSE_BOUNDARY(RET_FLOAT),
    RXVM_SPARSE_BOUNDARY(RET_STRING)
#undef RXVM_SPARSE_BOUNDARY
#undef RXVM_SPARSE_BACKEDGE
};

/* INTERRUPT is a dispatch pseudo-op owned by run(), not an RXVM_HANDLER body. */
#define RXVM_HANDLER_LABEL_INLINE(name_) &&name_
#define RXVM_HANDLER_LABEL_OUTLINE(name_) &&rxvm_handler_call
#define RXVM_HANDLER_LABEL_SELECT_INNER(policy_) RXVM_HANDLER_LABEL_ ## policy_
#define RXVM_HANDLER_LABEL_SELECT(policy_) \
    RXVM_HANDLER_LABEL_SELECT_INNER(policy_)
#define RXVM_HANDLER_LABEL(name_) \
    RXVM_HANDLER_LABEL_SELECT(RXVM_HANDLER_POLICY(name_))(name_)
#define RXVM_PRIVATE_HANDLER_LABEL_INLINE(name_) &&name_
#define RXVM_PRIVATE_HANDLER_LABEL_OUTLINE(name_) \
    &&rxvm_handler_ ## name_ ## _call
#define RXVM_PRIVATE_HANDLER_LABEL_SELECT_INNER(policy_) \
    RXVM_PRIVATE_HANDLER_LABEL_ ## policy_
#define RXVM_PRIVATE_HANDLER_LABEL_SELECT(policy_) \
    RXVM_PRIVATE_HANDLER_LABEL_SELECT_INNER(policy_)
#define RXVM_PRIVATE_HANDLER_LABEL(name_) \
    RXVM_PRIVATE_HANDLER_LABEL_SELECT(RXVM_HANDLER_POLICY(name_))(name_)

/* Sparse compatibility observation is emitted only in the separately selected
 * targetable owner. The accepted E4 owner never expands this macro. */
#define RXVM_SPARSE_OBSERVE()                                                  \
    do {                                                                       \
        sig_atomic_t rxvm_sparse_pending__ =                                  \
                context->active.external_mailbox_claim                        \
                ? context->active.external_mailbox_claim(                     \
                        context->active.external_mailbox_owner)                \
                : (rxvm_compatibility_pending_load(                           \
                        compatibility_interrupts) &                            \
                   rxsignal_mask(RXSIGNAL_CANCEL));                            \
        pending_interrupts |= rxvm_sparse_pending__;                          \
    } while (0)

#ifdef NTHREADED
#define RXVM_SPARSE_DISPATCH_TARGET() goto rxvm_sparse_CASE_START
#else
#define RXVM_SPARSE_DISPATCH_TARGET() VM_DISPATCH_TARGET()
#endif

#define RXVM_SPARSE_OWNER_DISPATCH()                                           \
    do {                                                                       \
        RXVM_DISPATCH_PREPARE();                                               \
        if (pending_interrupts && !current_frame->is_interrupt)                \
            goto INTERRUPT;                                                    \
        RXVM_SPARSE_DISPATCH_TARGET();                                         \
    } while (0)

/* This selector is used only at execution entry and after taking the existing
 * cold interrupt route. Ordinary/native handler dispatch still expands
 * RXVM_OWNER_DISPATCH() directly and has no owner-mode branch. */
#define RXVM_SELECTED_DISPATCH()                                               \
    do {                                                                       \
        RXVM_DISPATCH_PREPARE();                                               \
        if (compatibility_interrupts) {                                        \
            RXVM_SPARSE_OBSERVE();                                             \
            if (pending_interrupts && !current_frame->is_interrupt)            \
                goto INTERRUPT;                                                \
            RXVM_SPARSE_DISPATCH_TARGET();                                     \
        }                                                                      \
        VM_DISPATCH_TARGET();                                                  \
    } while (0)
#define RXVM_SELECTED_TARGET()                                                 \
    do {                                                                       \
        if (compatibility_interrupts) {                                        \
            RXVM_SPARSE_OBSERVE();                                             \
            if (pending_interrupts && !current_frame->is_interrupt)            \
                goto INTERRUPT;                                                \
            RXVM_SPARSE_DISPATCH_TARGET();                                     \
        }                                                                      \
        VM_DISPATCH_TARGET();                                                  \
    } while (0)

/* Interpreter */
static RXVM_LABEL_OWNER RX_FLATTEN int rxvm_run_owned_core(
        rxvm_context *context, int argc, char *argv[]) {
    proc_runtime *procedure;
    proc_runtime *step_handler = 0;
    int rc = 0;
    unsigned int initSeed = 0;   /* keep last seed for Random function within REXX run */
    char hasSeed = 0; /* no seed set */
    bin_code *pc = 0, *next_pc = 0;
    bin_code *interrupted_pc = 0;
#if RXVM_HANDLER_USE_POINTER_FACADE
    int mod_index;
#else
    int mod_index = 0;
#endif
    value *interrupt_arg;
    value *signal_value;
    value *interrupt_action_value;
    unsigned char signal_code = 0;
    value *arguments_array = 0;                /* note that the needs mallocing / freeing */
    unsigned char last_interrupt = 0; /* Interrupt being handled */
    /* Array of objects attached to raised interrupts */
    value *interrupt_object[RXSIGNAL_MAX];
    /* Array of addresses that were last interrupted by interrupt number */
    rxinteger last_interrupted_address[RXSIGNAL_MAX] = {0};
    /* Array of modules that were last interrupted by interrupt number */
    rxinteger last_interrupted_module[RXSIGNAL_MAX] = {0};
#if RXVM_HANDLER_USE_POINTER_FACADE
    stack_frame *current_frame = 0, *temp_frame;
#else
    stack_frame *current_frame = 0, *temp_frame = 0;
#endif
    bin_space *current_binary_space = 0;
    bin_code *current_execution_base = 0;
    bin_code *current_canonical_base = 0;
    unsigned char *current_const_pool = 0;
    value **current_locals = 0;
    /* 3 Work Registers */
    value *work1;
    value *work2;
    value *work3;
    module *current_module = 0;
    rxvm_memory_worker *previous_memory_worker;
    volatile sig_atomic_t pending_interrupts = 0;
    volatile sig_atomic_t *previous_pending_interrupts = 0;
    volatile sig_atomic_t *compatibility_interrupts =
            context->active.compatibility_interrupts;
    rxvm_handler_state handler_state;
    unsigned int sparse_dispatch_opcode = OP_IUNKNOWN;
    size_t sparse_source_index = 0;
    int sparse_source_module = 0;
#if !RXVM_HANDLER_USE_POINTER_FACADE
    rxvm_handler_function handler_function = 0;
#endif
    rxvm_handler_result handler_result = RXVM_HANDLER_RESULT_DISPATCH;
#ifdef NTHREADED
    void *next_inst = 0;
    const void **address_map = 0;
    void *private_r2_handler = 0;
    void *private_r1_handler = 0;
#else
#if RXVM_HANDLER_USE_POINTER_FACADE
    void *next_inst = compatibility_interrupts
            ? &&rxvm_sparse_IUNKNOWN : &&IUNKNOWN;
    const void *regular_address_map[OP_MAX_INSTRUCTIONS] = {
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) &&NAME,
#include "../binutils/include/rxops.h"
#undef X
    };
    const void *sparse_address_map[OP_MAX_INSTRUCTIONS] = {
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) &&rxvm_sparse_ ## NAME,
#include "../binutils/include/rxops.h"
#undef X
    };
#else
    void *next_inst = compatibility_interrupts
            ? &&rxvm_sparse_IUNKNOWN
            : RXVM_HANDLER_LABEL(IUNKNOWN);
    static const void *regular_address_map[OP_MAX_INSTRUCTIONS] = {
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) RXVM_HANDLER_LABEL(NAME),
#include "../binutils/include/rxops.h"
#undef X
    };
    static const void *sparse_address_map[OP_MAX_INSTRUCTIONS] = {
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) &&rxvm_sparse_ ## NAME,
#include "../binutils/include/rxops.h"
#undef X
    };
#endif
    const void *const *address_map = compatibility_interrupts
            ? sparse_address_map : regular_address_map;
#if RXVM_HANDLER_USE_POINTER_FACADE
    void *private_r2_handler = compatibility_interrupts
            ? &&rxvm_sparse_PRIVATE_R2_COPYATTR1 : &&PRIVATE_R2_COPYATTR1;
    void *private_r1_handler = compatibility_interrupts
            ? &&rxvm_sparse_PRIVATE_R1_RELINK : &&PRIVATE_R1_RELINK;
#else
    void *private_r2_handler = compatibility_interrupts
            ? &&rxvm_sparse_PRIVATE_R2_COPYATTR1
            : RXVM_PRIVATE_HANDLER_LABEL(PRIVATE_R2_COPYATTR1);
    void *private_r1_handler = compatibility_interrupts
            ? &&rxvm_sparse_PRIVATE_R1_RELINK
            : RXVM_PRIVATE_HANDLER_LABEL(PRIVATE_R1_RELINK);
#endif
#endif
    if (rxvm_signal_enter_execution(
            context, &pending_interrupts,
            &previous_pending_interrupts) != 0) {
        abort();
    }
    previous_memory_worker =
            rxvm_memory_enter(context->worker.memory_worker);
    RXVM_INSTRUMENTATION_STATE();
#if RXVM_HANDLER_USE_POINTER_FACADE
    handler_state.context = context;
    handler_state.rc = &rc;
    handler_state.initSeed = &initSeed;
    handler_state.hasSeed = &hasSeed;
    handler_state.pc = &pc;
    handler_state.next_pc = &next_pc;
    handler_state.interrupted_pc = &interrupted_pc;
    handler_state.mod_index = &mod_index;
    handler_state.signal_value = &signal_value;
    handler_state.interrupt_action_value = &interrupt_action_value;
    handler_state.arguments_array = &arguments_array;
    handler_state.interrupt_object = interrupt_object;
    handler_state.last_interrupted_address = last_interrupted_address;
    handler_state.last_interrupted_module = last_interrupted_module;
    handler_state.current_frame = &current_frame;
    handler_state.temp_frame = &temp_frame;
    handler_state.current_binary_space = &current_binary_space;
    handler_state.current_execution_base = &current_execution_base;
    handler_state.current_canonical_base = &current_canonical_base;
    handler_state.current_const_pool = &current_const_pool;
    handler_state.current_locals = &current_locals;
    handler_state.work1 = &work1;
    handler_state.current_module = &current_module;
    handler_state.pending_interrupts = &pending_interrupts;
    handler_state.address_map = address_map;
    handler_state.private_r2_handler = private_r2_handler;
    handler_state.private_r1_handler = private_r1_handler;
#ifndef NTHREADED
    handler_state.next_inst = &next_inst;
#endif
#ifdef CREXX_VM_PROFILING
    handler_state.profile = &vm_profile;
    handler_state.sequence = &vm_sequence;
#endif
#ifdef CREXX_VM_INSTRUMENTATION_TEST
    handler_state.test_instrumentation = &vm_instrumentation;
#endif
#endif
    RXVM_INSTRUMENTATION_VM_BEGIN(context);

    signal_value = value_f();
    interrupt_action_value = value_f();
    work1 = value_f();
    work2 = value_f();
    work3 = value_f();
#ifdef CREXX_VM_PROFILING
    rxvm_profile_mark_value_origin(
            signal_value, RXVM_PROFILE_VALUE_ORIGIN_SCRATCH);
    rxvm_profile_mark_value_origin(
            interrupt_action_value, RXVM_PROFILE_VALUE_ORIGIN_SCRATCH);
    rxvm_profile_mark_value_origin(work1, RXVM_PROFILE_VALUE_ORIGIN_SCRATCH);
    rxvm_profile_mark_value_origin(work2, RXVM_PROFILE_VALUE_ORIGIN_SCRATCH);
    rxvm_profile_mark_value_origin(work3, RXVM_PROFILE_VALUE_ORIGIN_SCRATCH);
#endif

    /* Set up the interrupt object array */
    {
        size_t i;
        for (i = 0; i < RXSIGNAL_MAX; i++) {
            interrupt_object[i] = value_f();
#ifdef CREXX_VM_PROFILING
            rxvm_profile_mark_value_origin(
                    interrupt_object[i], RXVM_PROFILE_VALUE_ORIGIN_SCRATCH);
#endif
        }
    }
    /* Initialize the native signal handler system */
    initialize_vm_signals();

    /*
     * Instruction database - loaded from a generated header file
     */
    /* Allocate Interrupt Arg */
    interrupt_arg = value_f();
#ifdef CREXX_VM_PROFILING
    rxvm_profile_mark_value_origin(
            interrupt_arg, RXVM_PROFILE_VALUE_ORIGIN_SCRATCH);
#endif

    /* Thread code - we need to do it here because address_map is only valid
     * in this run() function */
    DEBUG("Threading/Preparing\n");
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        /* Idempotent check */
        if (context->modules[mod_index]->state >= RXVM_MOD_THREADED) continue;

        VM_PREPARE_EXECUTION_IMAGE(context->modules[mod_index]);
        context->modules[mod_index]->state = RXVM_MOD_THREADED;
    }

    if (context->prepare_only) {
        /* We are only here to thread, return success */
        rc = 0;
        goto interprt_finished;
    }

    /* Find the program's entry point */
    DEBUG("Find program entry point\n");
    if (context->ext_proc) {
        procedure = context->ext_proc;
    } else {
        for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
            int i = context->modules[mod_index]->proc_head;
            while (i != -1) {
                proc_constant *definition =
                        (proc_constant *) (context->modules[mod_index]->segment.const_pool +
                                           i);
                if (definition->base.type == PROC_CONST &&
                    strcmp(definition->name, "main") == 0) {
                    procedure = rxvm_get_module_runtime_procedure(context->modules[mod_index], (size_t)i);
                    break;
                }
                i = definition->next;
                procedure = 0;
            }
            if (procedure) break;
        }
    }

    if (!procedure) {
        DEBUG("main() not found\n");
        goto interprt_finished;
    }

    DEBUG("Create first Stack Frame\n");
    if (context->ext_proc) {
        temp_frame = frame_f(procedure, context->ext_argc, 0, 0, context->ext_ret);
        if (!temp_frame) {
            fprintf(stderr, "PANIC - Unable to allocate stack frame\n");
            rc = RXSIGNAL_FAILURE;
            goto interprt_finished;
        }
        RXVM_INSTRUMENTATION_CALL(
                RXVM_PROFILE_CALL_EXTERNAL_ROOT, procedure, context->ext_argc,
                RXVM_PROFILE_FRAME_LAST_ACTIVATION, RXVM_PROFILE_CALL_SUCCESS,
                0, procedure->binarySpace->module->module_number,
                procedure->start, 0, 0);
        VM_ACTIVATE_FRAME(temp_frame, RXVM_TRANSITION_EXTERNAL_ENTRY);
        /* Arguments (passed as individual objects) */
        {
            int i;
            int a1 = procedure->binarySpace->globals + procedure->locals + 1;
            for (i = 0; i < context->ext_argc; i++) {
                current_frame->baselocals[a1 + i] = value_f();
#ifdef CREXX_VM_PROFILING
                rxvm_profile_mark_value_origin(
                        current_frame->baselocals[a1 + i],
                        RXVM_PROFILE_VALUE_ORIGIN_SCRATCH);
#endif
                current_locals[a1 + i] = current_frame->baselocals[a1 + i];
                copy_value(current_frame->baselocals[a1 + i], context->ext_args[i]);
            }
        }
    } else {
        temp_frame = frame_f(procedure, 1, 0, 0, 0);
        if (!temp_frame) {
            fprintf(stderr, "PANIC - Unable to allocate stack frame\n");
            rc = RXSIGNAL_FAILURE;
            goto interprt_finished;
        }
        RXVM_INSTRUMENTATION_CALL(
                RXVM_PROFILE_CALL_EXTERNAL_ROOT, procedure, 1,
                RXVM_PROFILE_FRAME_LAST_ACTIVATION, RXVM_PROFILE_CALL_SUCCESS,
                0, procedure->binarySpace->module->module_number,
                procedure->start, 0, 0);
        VM_ACTIVATE_FRAME(temp_frame, RXVM_TRANSITION_EXTERNAL_ENTRY);
        /* Arguments (passed in an array) */
        /* a0 is already set by frame_f() */
        /* a1 is the array  */
        {
            int i;
            int a1 = procedure->binarySpace->globals + procedure->locals + 1;
            arguments_array = value_f();
#ifdef CREXX_VM_PROFILING
            rxvm_profile_mark_value_origin(
                    arguments_array, RXVM_PROFILE_VALUE_ORIGIN_SCRATCH);
#endif
            current_frame->baselocals[a1] = arguments_array;
            current_locals[a1] = current_frame->baselocals[a1];
            set_num_attributes(current_frame->baselocals[a1], argc);

            for (i = 0; i < argc; i++) {
                set_null_string(current_frame->baselocals[a1]->attributes[i], argv[i]);
            }
        }
    }

    /* Load VM Plugins */
    DEBUG("Load VM Plugins\n");
    current_frame->decimal = (decplugin*)rxvmplugin_instance_set_get(
            &context->plugin_instances, RXVM_PLUGIN_DECIMAL);
    if (!current_frame->decimal) {
        printf("PANIC - No default decimal plugin\n");
        exit(255); // Documented 255 is for missing decimal plugin
    }
    current_frame->decimal_loaded_here = 0;

    // Set the numeric context of the decimal plugin
    current_frame->decimal->num_context = &current_frame->num_context;
    current_frame->decimal->syncNumericContext(current_frame->decimal);

    /* Start */
    DEBUG("Starting inst# %s-0x%x\n", procedure->binarySpace->module->name, (int) procedure->start);
    VM_SELECT_INDEX(procedure->start, RXVM_TRANSITION_EXTERNAL_ENTRY);
    RXVM_SELECTED_DISPATCH();

    /* Instruction implementations */
    /* ----------------------------------------------------------------------------
     * The following shortcut macros are used in the instruction implementation
     *      op1R   address the first register operand
     *      op2R   address the second register operand
     *      op3R   address the third  register operand
     *      op4R   address the fourth register operand
     *
     *      op1RI  integer of first register operand
     *      op2RI  integer of second register operand
     *      op3RI  integer of third register operand
     *      op4RI  integer of fourth register operand
     *
     *      op1RF  float of first register operand
     *      op2RF  float of second register operand
     *      op3RF  float of third register operand
     *
     *      op1I   integer value of first operand (non-register value)
     *      op2I   integer value of second operand (non-register value)
     *      op3I   integer value of third  operand (non-register value)
     *
     *      op1F   float value of first operand (non-register value)
     *      op2F   float value of second operand (non-register value)
     *      op3F   float value of third  operand (non-register value)
     *
     *      CONV2INT(integer-result-variable,value-to-be-converted)
     *      CONV2FLOAT(float-result-variable,value-to-be-converted)
     * ----------------------------------------------------------------------------
     */

    /* Signal Interrupt Support - this is only used/called when interrupts are pending */
    START_INTERRUPT;
    if (context->active.external_mailbox_claim) {
        pending_interrupts |= context->active.external_mailbox_claim(
                context->active.external_mailbox_owner);
    }
    DEBUG("TRACE - SIGNAL FIRED - CHECK HANDLER\n");

    /* Also clear any pending signals that are ignored and also find the first signal which */
    /* is masked and pending - the first one is the highest priority */
    last_interrupt = 0;
    for (signal_code = 0; signal_code < RXSIGNAL_MAX - 1; signal_code++) {
        sig_atomic_t signal_mask = rxsignal_mask(signal_code + 1);
        if (pending_interrupts & signal_mask) {
            bin_code *signal_pc = (interrupted_pc && signal_code + 1 != RXSIGNAL_BREAKPOINT) ? interrupted_pc : pc;
            last_interrupted_module[signal_code + 1] = (rxinteger) current_module->module_number;
            last_interrupted_address[signal_code + 1] =
                    (rxinteger) VM_CANONICAL_INDEX(signal_pc);
            if (current_frame->interrupt_table[signal_code].response == RXSIGNAL_RESPONSE_IGNORE) {
                DEBUG("TRACE - INTR IGNORE %s\n", interrupt_to_string(signal_code + 1));
                rxvm_signal_pending_and(&pending_interrupts, ~signal_mask);
            } else {
                last_interrupt = signal_code + 1;
                break;
            }
        }
    }
    interrupted_pc = 0;

    if (!last_interrupt || last_interrupt >= RXSIGNAL_MAX) {
        /* No un-ignored interrupts pending */
        RXVM_INSTRUMENTATION_INTERRUPT_RESUME(0, current_module->module_number,
                                              VM_CANONICAL_INDEX(pc));
        RXVM_SELECTED_TARGET();
    }

    // Clear the interrupt
    if (last_interrupt != RXSIGNAL_BREAKPOINT) {
        // Breakpoints are not cleared
        rxvm_signal_pending_and(&pending_interrupts,
                                ~rxsignal_mask(last_interrupt));
    }

    // Handle the interrupt
    RXVM_INSTRUMENTATION_INTERRUPT_SELECT(last_interrupt,
                                          current_module->module_number,
                                          VM_CANONICAL_INDEX(pc));
    RXVM_INSTRUMENTATION_INTERRUPT_ENTRY(last_interrupt,
                                         current_module->module_number,
                                         VM_CANONICAL_INDEX(pc));
    interrupt_entry signal_handler = current_frame->interrupt_table[last_interrupt - 1];
    switch (signal_handler.response) {

        case RXSIGNAL_RESPONSE_HALT:
            /* Halt */
            DEBUG("TRACE - INTR HANDLER -> HALT %s\n", interrupt_to_string(last_interrupt));
            /* Print error message to stderr */
            if (interrupt_object[last_interrupt]->string_length) {
                fprintf(stderr, "PANIC: %.*s (SIGNAL %s)\n", (int)(interrupt_object[last_interrupt]->string_length), interrupt_object[last_interrupt]->string_value, interrupt_to_string(last_interrupt));
            } else {
                fprintf(stderr, "PANIC: (SIGNAL %s)\n", interrupt_to_string(last_interrupt));
                print_runtime_panic_location(context,
                                             last_interrupted_module[last_interrupt],
                                             last_interrupted_address[last_interrupt]);
            }
            rc = (int)last_interrupt;
            RXVM_INSTRUMENTATION_INTERRUPT_TERMINAL(last_interrupt,
                                                    current_module->module_number,
                                                    VM_CANONICAL_INDEX(pc));
            goto interprt_finished;

        case RXSIGNAL_RESPONSE_SILENT_HALT:
            /* Silent Halt */
            DEBUG("TRACE - INTR HANDLER -> SILENT HALT %s\n", interrupt_to_string(last_interrupt));
            rc = 0;
            RXVM_INSTRUMENTATION_INTERRUPT_TERMINAL(last_interrupt,
                                                    current_module->module_number,
                                                    VM_CANONICAL_INDEX(pc));
            goto interprt_finished;

        case RXSIGNAL_RESPONSE_CALL_BRANCH:
            DEBUG("TRACE - INTR HANDLER -> SET BRANCH FOR CALL RETURN ");
            rxsignal_restore_branch_call_argument_mapping(
                    current_frame, &signal_handler,
                    last_interrupted_module[last_interrupt],
                    last_interrupted_address[last_interrupt]
                    RXVM_PROFILE_UNWIND_STATE);
            VM_ACTIVATE_FRAME(rxsignal_unwind_to_frame(current_frame, signal_handler.frame
                                                       RXVM_PROFILE_UNWIND_STATE),
                              RXVM_TRANSITION_INTERRUPT_ENTRY);
            {
                int stack_restored = rxsignal_unwind_handler_stack_to(
                        current_frame, signal_handler.stack_marker);
                assert(stack_restored);
                (void)stack_restored;
            }
            VM_SELECT_INDEX(signal_handler.jump, RXVM_TRANSITION_INTERRUPT_ENTRY);
            pc = next_pc;
            // Fall through to CALL

        case RXSIGNAL_RESPONSE_CALL_ACTION:
        case RXSIGNAL_RESPONSE_CALL: {
            /* Call */
            proc_runtime *intr_function = signal_handler.function;
            char action_aware = signal_handler.response == RXSIGNAL_RESPONSE_CALL_ACTION;
            DEBUG("TRACE - INTR HANDLER -> CALL %s->%s()\n", interrupt_to_string(last_interrupt), intr_function->name);

            if (intr_function->start == SIZE_MAX) {
                RXVM_INSTRUMENTATION_CALL(
                        intr_function->binarySpace
                            ? RXVM_PROFILE_CALL_SIGNAL_BYTECODE
                            : RXVM_PROFILE_CALL_SIGNAL_NATIVE,
                        intr_function, 1, RXVM_PROFILE_FRAME_NONE_FAILED,
                        RXVM_PROFILE_CALL_UNRESOLVED, current_frame,
                        current_module->module_number, VM_CANONICAL_INDEX(pc),
                        0, 0);
                SET_SIGNAL_MSG(RXSIGNAL_FUNCTION_NOT_FOUND, "Exception handler not exposed/linked")
                RXVM_SELECTED_DISPATCH();
            }

            /* Populate the interrupt argument object */
            rxsignal_populate_raw_interrupt(interrupt_arg,
                                            last_interrupt,
                                            last_interrupted_module[last_interrupt],
                                            last_interrupted_address[last_interrupt],
                                            interrupt_object[last_interrupt]);

            if (intr_function->binarySpace == 0) {
                /* This is a native plugin function */
                RXVM_INSTRUMENTATION_CALL(
                        RXVM_PROFILE_CALL_SIGNAL_NATIVE, intr_function, 1,
                        RXVM_PROFILE_FRAME_NO_CHILD_NATIVE,
                        RXVM_PROFILE_CALL_SUCCESS, current_frame,
                        current_module->module_number, VM_CANONICAL_INDEX(pc),
                        0, 0);
                RXVM_INSTRUMENTATION_NATIVE_BEGIN(intr_function);
                rxvm_call_native_procedure(intr_function, 1, &interrupt_arg,
                                           0, signal_value);
                RXVM_INSTRUMENTATION_NATIVE_END();
                if (signal_value->int_value > RXSIGNAL_NONE && signal_value->int_value < RXSIGNAL_MAX) {
                    if (signal_value->string_length) {
                        SET_SIGNAL_MSG(signal_value->int_value, signal_value->string_value)
                    } else {
                        SET_SIGNAL(signal_value->int_value)
                    }
                }
                RXVM_SELECTED_DISPATCH();
            } else {
                /* A CREXX Procedure */
                if (action_aware) value_zero(interrupt_action_value);
                temp_frame = frame_f(intr_function, 1, current_frame, pc, action_aware ? interrupt_action_value : 0);
                if (!temp_frame) {
                    RXVM_INSTRUMENTATION_CALL(
                            RXVM_PROFILE_CALL_SIGNAL_BYTECODE, intr_function, 1,
                            RXVM_PROFILE_FRAME_NONE_FAILED,
                            RXVM_PROFILE_CALL_FRAME_FAILED, current_frame,
                            current_module->module_number,
                            VM_CANONICAL_INDEX(pc), 0, 0);
                    SET_SIGNAL_MSG(RXSIGNAL_FAILURE, "Unable to allocate stack frame")
                    RXVM_SELECTED_DISPATCH();
                }
                RXVM_INSTRUMENTATION_CALL(
                        RXVM_PROFILE_CALL_SIGNAL_BYTECODE, intr_function,
                        temp_frame->number_args,
                        RXVM_PROFILE_FRAME_LAST_ACTIVATION,
                        RXVM_PROFILE_CALL_SUCCESS, current_frame,
                        current_module->module_number, VM_CANONICAL_INDEX(pc),
                        0, 0);
                /* Prepare dispatch to procedure as early as possible */
                VM_ACTIVATE_FRAME(temp_frame, RXVM_TRANSITION_INTERRUPT_ENTRY);
                VM_SELECT_INDEX(intr_function->start, RXVM_TRANSITION_INTERRUPT_ENTRY);


                /* Interrupt being handled */
                current_frame->is_interrupt = last_interrupt;
                current_frame->is_interrupt_action = action_aware;

                /* Argument */
                size_t arg_index = intr_function->binarySpace->globals + intr_function->locals + 1;
                current_frame->baselocals[arg_index] = current_locals[arg_index] = interrupt_arg;

                /* DISPATCH goes the interrupt handler */
                RXVM_SELECTED_DISPATCH();
            }
        }

        case RXSIGNAL_RESPONSE_BRANCH:
            DEBUG("TRACE - INTR HANDLER -> BRANCH %s\n", interrupt_to_string(last_interrupt));
            rxsignal_restore_branch_call_argument_mapping(
                    current_frame, &signal_handler,
                    last_interrupted_module[last_interrupt],
                    last_interrupted_address[last_interrupt]
                    RXVM_PROFILE_UNWIND_STATE);
            VM_ACTIVATE_FRAME(rxsignal_unwind_to_frame(current_frame, signal_handler.frame
                                                       RXVM_PROFILE_UNWIND_STATE),
                              RXVM_TRANSITION_INTERRUPT_ENTRY);
            {
                int stack_restored = rxsignal_unwind_handler_stack_to(
                        current_frame, signal_handler.stack_marker);
                assert(stack_restored);
                (void)stack_restored;
            }
            VM_SELECT_INDEX(signal_handler.jump, RXVM_TRANSITION_INTERRUPT_ENTRY);
            RXVM_SELECTED_DISPATCH();

        case RXSIGNAL_RESPONSE_BRANCH_VALUE:
            DEBUG("TRACE - INTR HANDLER -> BRANCH VALUE %s\n", interrupt_to_string(last_interrupt));
            rxsignal_restore_branch_call_argument_mapping(
                    current_frame, &signal_handler,
                    last_interrupted_module[last_interrupt],
                    last_interrupted_address[last_interrupt]
                    RXVM_PROFILE_UNWIND_STATE);
            VM_ACTIVATE_FRAME(rxsignal_unwind_to_frame(current_frame, signal_handler.frame
                                                       RXVM_PROFILE_UNWIND_STATE),
                              RXVM_TRANSITION_INTERRUPT_ENTRY);
            {
                int stack_restored = rxsignal_unwind_handler_stack_to(
                        current_frame, signal_handler.stack_marker);
                assert(stack_restored);
                (void)stack_restored;
            }
            rxsignal_populate_raw_interrupt(interrupt_arg,
                                            last_interrupt,
                                            last_interrupted_module[last_interrupt],
                                            last_interrupted_address[last_interrupt],
                                            interrupt_object[last_interrupt]);
            rxsignal_populate_runtime_signal(current_locals[signal_handler.value_register], interrupt_arg);
            VM_SELECT_INDEX(signal_handler.jump, RXVM_TRANSITION_INTERRUPT_ENTRY);
            RXVM_SELECTED_DISPATCH();

        case RXSIGNAL_RESPONSE_RETURN:
            DEBUG("TRACE - INTR HANDLER -> RET %s\n", interrupt_to_string(last_interrupt));
            {
                /* Where we return to */
                next_pc = current_frame->return_pc;
                // Note that current_frame->is_interrupt cannot be set as a signal triggers us
                /* back to the parent's stack frame */
                temp_frame = current_frame;
                VM_ACTIVATE_FRAME_OR_NULL(current_frame->parent, RXVM_TRANSITION_RETURN);
                if (!current_frame) {
                    DEBUG("TRACE - INTR RETURNING FROM MAIN()\n");
                    /* Free Argument Values a1... */
                    int i, j;
                    /* a0 is the number of args */
                    int num_args = (int)temp_frame->baselocals[temp_frame->procedure->binarySpace->globals +
                                                               temp_frame->procedure->locals]->int_value;
                    for (i = 0, j = temp_frame->procedure->binarySpace->globals + temp_frame->procedure->locals + 1;
                         i < num_args;
                         i++, j++) {
                        value_free(temp_frame->baselocals[j]);
                         }
                    rc = 0;
                    free_frame(temp_frame);
                    arguments_array = 0; /* We have freed it in the loop above */
                    RXVM_INSTRUMENTATION_INTERRUPT_TERMINAL(
                            last_interrupt,
                            current_module ? current_module->module_number : 0,
                            (current_execution_base && pc)
                                ? VM_CANONICAL_INDEX(pc) : 0);
                    goto interprt_finished;
                }
                free_frame(temp_frame);
                VM_SELECT_POINTER(next_pc, RXVM_TRANSITION_RETURN);
                RXVM_SELECTED_DISPATCH();
            }

        case RXSIGNAL_RESPONSE_IGNORE:
            /* Ignore - Should never get here */
            DEBUG("*ERROR* TRACE INTR HANDLER -> IGNORE (SHOULD NOT GET HERE) %s\n", interrupt_to_string(last_interrupt));
            RXVM_SELECTED_TARGET();
    }

    /* Should never get here */
    RXVM_SELECTED_TARGET();

START_OF_INSTRUCTIONS

#ifdef NTHREADED
#undef END_OF_INSTRUCTIONS
#if RXVM_HANDLER_USE_POINTER_FACADE
#define END_OF_INSTRUCTIONS \
    default: SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION); DISPATCH; }
#else
#define END_OF_INSTRUCTIONS default: goto rxvm_handler_call; }
#endif
#endif

#if RXVM_HANDLER_USE_POINTER_FACADE
#define RXVM_HANDLER(name_, ...)                                               \
        START_INSTRUCTION(name_) RXVM_EMIT_HANDLER(name_, __VA_ARGS__);
#ifdef NTHREADED
#define RXVM_PRIVATE_HANDLER(name_, dispatch_opcode_, profile_opcode_, ...)     \
        case dispatch_opcode_:                                                 \
            RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(                            \
                    current_module->module_number, VM_CANONICAL_INDEX(pc),      \
                    profile_opcode_, RXVM_HANDLER_IS_INLINE(name_));            \
            RXVM_EMIT_HANDLER(name_, __VA_ARGS__);
#else
#define RXVM_PRIVATE_HANDLER(name_, dispatch_opcode_, profile_opcode_, ...)     \
        name_:                                                                 \
            RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(                            \
                    current_module->module_number, VM_CANONICAL_INDEX(pc),      \
                    profile_opcode_, RXVM_HANDLER_IS_INLINE(name_));            \
            RXVM_EMIT_HANDLER(name_, __VA_ARGS__);
#endif
#else
#define RXVM_OWNER_HANDLER_INLINE(name_, ...)                                 \
        START_INSTRUCTION(name_) RXVM_HANDLER_EMIT_INLINE(name_, __VA_ARGS__);
#define RXVM_OWNER_HANDLER_OUTLINE(name_, ...)
#define RXVM_OWNER_HANDLER_SELECT_INNER(policy_) \
        RXVM_OWNER_HANDLER_ ## policy_
#define RXVM_OWNER_HANDLER_SELECT(policy_) \
        RXVM_OWNER_HANDLER_SELECT_INNER(policy_)
#define RXVM_OWNER_HANDLER(name_, ...)                                        \
        RXVM_OWNER_HANDLER_SELECT(RXVM_HANDLER_POLICY(name_))(name_, __VA_ARGS__)

#define RXVM_HANDLER(name_, ...)                                               \
        RXVM_OWNER_HANDLER(name_, __VA_ARGS__)
#ifdef NTHREADED
#define RXVM_OWNER_PRIVATE_INLINE(name_, dispatch_opcode_, profile_opcode_, ...) \
        case dispatch_opcode_:                                                 \
            RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(                            \
                    current_module->module_number, VM_CANONICAL_INDEX(pc),      \
                    profile_opcode_, RXVM_HANDLER_IS_INLINE(name_));            \
            RXVM_HANDLER_EMIT_INLINE(name_, __VA_ARGS__);
#else
#define RXVM_OWNER_PRIVATE_INLINE(name_, dispatch_opcode_, profile_opcode_, ...) \
        name_:                                                                 \
            RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(                            \
                    current_module->module_number, VM_CANONICAL_INDEX(pc),      \
                    profile_opcode_, RXVM_HANDLER_IS_INLINE(name_));            \
            RXVM_HANDLER_EMIT_INLINE(name_, __VA_ARGS__);
#endif
#define RXVM_OWNER_PRIVATE_OUTLINE(name_, dispatch_opcode_, profile_opcode_, ...)
#define RXVM_OWNER_PRIVATE_SELECT_INNER(policy_) \
        RXVM_OWNER_PRIVATE_ ## policy_
#define RXVM_OWNER_PRIVATE_SELECT(policy_) \
        RXVM_OWNER_PRIVATE_SELECT_INNER(policy_)
#define RXVM_PRIVATE_HANDLER(name_, dispatch_opcode_, profile_opcode_, ...)     \
        RXVM_OWNER_PRIVATE_SELECT(RXVM_HANDLER_POLICY(name_))(                  \
                name_, dispatch_opcode_, profile_opcode_, __VA_ARGS__)
#endif

#include "rxvmhandlers_core.inc"
#include "rxvmhandlers_control.inc"
#include "rxvmhandlers_numeric.inc"
#include "rxvmhandlers_string.inc"
#include "rxvmhandlers_system.inc"
#include "rxvmhandlers_channel.inc"

#undef RXVM_PRIVATE_HANDLER
#undef RXVM_HANDLER
#if !RXVM_HANDLER_USE_POINTER_FACADE
#undef RXVM_OWNER_PRIVATE_SELECT
#undef RXVM_OWNER_PRIVATE_SELECT_INNER
#undef RXVM_OWNER_PRIVATE_OUTLINE
#undef RXVM_OWNER_PRIVATE_INLINE
#undef RXVM_OWNER_HANDLER
#undef RXVM_OWNER_HANDLER_SELECT
#undef RXVM_OWNER_HANDLER_SELECT_INNER
#undef RXVM_OWNER_HANDLER_OUTLINE
#undef RXVM_OWNER_HANDLER_INLINE
#endif

    END_OF_INSTRUCTIONS

    /* The compatibility owner is selected once before rxvm_prepare(). It has
     * the same inline/outline placement as the accepted E4 owner, but owns a
     * distinct direct-threaded label map (or switch). Its normal DISPATCH does
     * not read the external flag. Observation is injected only at semantic
     * progress points: calls, returns, native/plugin returns, and taken
     * backward/self branches. No executing worker can jump between owners. */
#undef RXVM_CONTROL_FLOW_SAFEPOINT
#define RXVM_CONTROL_FLOW_SAFEPOINT(target_, reason_)                           \
    do {                                                                        \
        int rxvm_sparse_reason__ = (int)(reason_);                              \
        if (rxvm_sparse_reason__ == RXVM_TRANSITION_CALL ||                     \
                rxvm_sparse_reason__ == RXVM_TRANSITION_RETURN) {               \
            RXVM_SPARSE_OBSERVE();                                              \
        } else if (rxvm_sparse_reason__ == RXVM_TRANSITION_BRANCH) {            \
            bin_code *rxvm_sparse_target__ = (target_);                         \
            if (VM_CANONICAL_INDEX(rxvm_sparse_target__) <=                     \
                    VM_CANONICAL_INDEX(pc)) {                                   \
                RXVM_SPARSE_OBSERVE();                                          \
            }                                                                   \
        }                                                                       \
    } while (0)
#undef RXVM_EXTERNAL_SAFEPOINT
#define RXVM_EXTERNAL_SAFEPOINT() RXVM_SPARSE_OBSERVE()
#undef DISPATCH
#define DISPATCH RXVM_SPARSE_OWNER_DISPATCH()

#define RXVM_SPARSE_CAPTURE_SOURCE(dispatch_opcode_)                            \
    do {                                                                        \
        sparse_dispatch_opcode = (unsigned int)(dispatch_opcode_);              \
        sparse_source_index = VM_CANONICAL_INDEX(pc);                           \
        sparse_source_module = current_module->module_number;                   \
    } while (0)

#define RXVM_SPARSE_EMIT_INLINE(name_, dispatch_opcode_, ...)                   \
    RXVM_HANDLER_EMIT_INLINE(name_, __VA_ARGS__)
#if RXVM_HANDLER_USE_POINTER_FACADE
#define RXVM_SPARSE_EMIT_OUTLINE(name_, dispatch_opcode_, ...)                  \
    do {                                                                        \
        RXVM_SPARSE_CAPTURE_SOURCE(dispatch_opcode_);                           \
        handler_result = rxvm_handler_ ## name_(&handler_state);                \
        goto rxvm_sparse_handler_result;                                        \
    } while (0)
#else
#define RXVM_SPARSE_EMIT_OUTLINE(name_, dispatch_opcode_, ...)                  \
    do {                                                                        \
        RXVM_SPARSE_CAPTURE_SOURCE(dispatch_opcode_);                           \
        handler_function = rxvm_handler_ ## name_;                             \
        goto rxvm_sparse_handler_state_call;                                    \
    } while (0)
#endif
#define RXVM_SPARSE_EMIT_SELECT_INNER(policy_) RXVM_SPARSE_EMIT_ ## policy_
#define RXVM_SPARSE_EMIT_SELECT(policy_) RXVM_SPARSE_EMIT_SELECT_INNER(policy_)
#define RXVM_SPARSE_EMIT(name_, dispatch_opcode_, ...)                          \
    RXVM_SPARSE_EMIT_SELECT(RXVM_HANDLER_POLICY(name_))(                        \
            name_, dispatch_opcode_, __VA_ARGS__)

#ifdef NTHREADED
    rxvm_sparse_CASE_START:
    switch ((instructions)(pc->instruction.opcode)) {
#define RXVM_SPARSE_START_PUBLIC(name_, profile_opcode_)                        \
        case OP_ ## name_:                                                      \
            RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(                            \
                    current_module->module_number, VM_CANONICAL_INDEX(pc),      \
                    profile_opcode_, RXVM_HANDLER_IS_INLINE(name_));
#define RXVM_SPARSE_START_PRIVATE(name_, dispatch_opcode_, profile_opcode_)     \
        case dispatch_opcode_:                                                  \
            RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(                            \
                    current_module->module_number, VM_CANONICAL_INDEX(pc),      \
                    profile_opcode_, RXVM_HANDLER_IS_INLINE(name_));
#else
#define RXVM_SPARSE_START_PUBLIC(name_, profile_opcode_)                        \
        rxvm_sparse_ ## name_:                                                  \
            RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(                            \
                    current_module->module_number, VM_CANONICAL_INDEX(pc),      \
                    profile_opcode_, RXVM_HANDLER_IS_INLINE(name_));
#define RXVM_SPARSE_START_PRIVATE(name_, dispatch_opcode_, profile_opcode_)     \
        rxvm_sparse_ ## name_:                                                  \
            RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(                            \
                    current_module->module_number, VM_CANONICAL_INDEX(pc),      \
                    profile_opcode_, RXVM_HANDLER_IS_INLINE(name_));
#endif

#define RXVM_HANDLER(name_, ...)                                                \
        RXVM_SPARSE_START_PUBLIC(name_, OP_ ## name_)                           \
        RXVM_SPARSE_EMIT(name_, OP_ ## name_, __VA_ARGS__);
#define RXVM_PRIVATE_HANDLER(name_, dispatch_opcode_, profile_opcode_, ...)     \
        RXVM_SPARSE_START_PRIVATE(name_, dispatch_opcode_, profile_opcode_)     \
        RXVM_SPARSE_EMIT(name_, dispatch_opcode_, __VA_ARGS__);

#ifndef NTHREADED
    rxvm_sparse_INTERRUPT:
        goto INTERRUPT;
#endif

#include "rxvmhandlers_core.inc"
#include "rxvmhandlers_control.inc"
#include "rxvmhandlers_numeric.inc"
#include "rxvmhandlers_string.inc"
#include "rxvmhandlers_system.inc"
#include "rxvmhandlers_channel.inc"

#undef RXVM_PRIVATE_HANDLER
#undef RXVM_HANDLER
#undef RXVM_SPARSE_START_PRIVATE
#undef RXVM_SPARSE_START_PUBLIC
#ifdef NTHREADED
        default:
            SET_SIGNAL(RXSIGNAL_UNKNOWN_INSTRUCTION);
            DISPATCH;
    }
#endif
#undef RXVM_SPARSE_EMIT
#undef RXVM_SPARSE_EMIT_SELECT
#undef RXVM_SPARSE_EMIT_SELECT_INNER
#undef RXVM_SPARSE_EMIT_OUTLINE
#undef RXVM_SPARSE_EMIT_INLINE
#undef RXVM_SPARSE_CAPTURE_SOURCE

#if !RXVM_HANDLER_USE_POINTER_FACADE
    rxvm_sparse_handler_state_call:
        RXVM_HANDLER_STATE_SNAPSHOT(handler_state);
        handler_result =
                rxvm_invoke_outlined_handler(handler_function, &handler_state);
        RXVM_HANDLER_STATE_COMMIT(handler_state);
#endif

    rxvm_sparse_handler_result:
        switch (handler_result) {
            case RXVM_HANDLER_RESULT_DISPATCH:
            case RXVM_HANDLER_RESULT_RESUME:
                {
                    rxvm_sparse_safepoint_kind safepoint =
                            sparse_dispatch_opcode <
                                    sizeof(rxvm_sparse_safepoint_kinds) /
                                    sizeof(rxvm_sparse_safepoint_kinds[0])
                            ? (rxvm_sparse_safepoint_kind)
                                    rxvm_sparse_safepoint_kinds[
                                            sparse_dispatch_opcode]
                            : RXVM_SPARSE_SAFEPOINT_NONE;
                    if (safepoint == RXVM_SPARSE_SAFEPOINT_BOUNDARY) {
                        RXVM_SPARSE_OBSERVE();
                    } else if (safepoint == RXVM_SPARSE_SAFEPOINT_BACKEDGE &&
                            current_module &&
                            current_module->module_number ==
                                    sparse_source_module &&
                            VM_CANONICAL_INDEX(pc) <= sparse_source_index) {
                        RXVM_SPARSE_OBSERVE();
                    }
                }
                if (pending_interrupts && !current_frame->is_interrupt)
                    goto INTERRUPT;
                RXVM_SPARSE_DISPATCH_TARGET();
            case RXVM_HANDLER_RESULT_INTERRUPT:
                goto INTERRUPT;
            case RXVM_HANDLER_RESULT_INTERRUPT_TABLE_OOM:
                goto interrupt_table_oom;
            case RXVM_HANDLER_RESULT_FINISHED:
                goto interprt_finished;
        }
        abort();

#undef DISPATCH
#define DISPATCH RXVM_OWNER_DISPATCH()
#undef RXVM_EXTERNAL_SAFEPOINT
#define RXVM_EXTERNAL_SAFEPOINT() do { } while (0)
#undef RXVM_CONTROL_FLOW_SAFEPOINT
#define RXVM_CONTROL_FLOW_SAFEPOINT(target_, reason_)                           \
    do {                                                                        \
        (void)(target_);                                                        \
        (void)(reason_);                                                        \
    } while (0)

#if !RXVM_HANDLER_USE_POINTER_FACADE
    rxvm_handler_call:
        {
#ifdef NTHREADED
            unsigned int dispatch_opcode = pc->instruction.opcode;
#else
            unsigned int dispatch_opcode =
                    VM_CANONICAL_POINTER(VM_CANONICAL_INDEX(pc))
                            ->instruction.opcode;
#endif
            unsigned int profile_opcode = dispatch_opcode;
            if (dispatch_opcode == RXVM_PRIVATE_R2_COPYATTR1_REG_REG_INT) {
                profile_opcode = OP_LINKATTR1_REG_REG_INT;
            } else if (dispatch_opcode == RXVM_PRIVATE_R1_RELINK_REG_REG) {
                profile_opcode = OP_UNLINK_REG;
            }
            if (dispatch_opcode <
                    sizeof(rxvm_outlined_handler_functions) /
                    sizeof(rxvm_outlined_handler_functions[0]) &&
                    rxvm_outlined_handler_functions[dispatch_opcode]) {
                handler_function =
                        rxvm_outlined_handler_functions[dispatch_opcode];
            } else {
                handler_function = rxvm_outlined_handler_functions[OP_IUNKNOWN];
                profile_opcode = OP_IUNKNOWN;
            }
            RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(
                    current_module->module_number, VM_CANONICAL_INDEX(pc),
                    profile_opcode,
                    RXVM_HANDLER_EFFECTIVE_INLINE(dispatch_opcode));
        }
        goto rxvm_handler_state_call;

#ifndef NTHREADED
    rxvm_handler_PRIVATE_R2_COPYATTR1_call:
        handler_function = rxvm_outlined_handler_functions[
                RXVM_PRIVATE_R2_COPYATTR1_REG_REG_INT];
        RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(
                current_module->module_number, VM_CANONICAL_INDEX(pc),
                OP_LINKATTR1_REG_REG_INT,
                RXVM_HANDLER_IS_INLINE(PRIVATE_R2_COPYATTR1));
        goto rxvm_handler_state_call;

    rxvm_handler_PRIVATE_R1_RELINK_call:
        handler_function = rxvm_outlined_handler_functions[
                RXVM_PRIVATE_R1_RELINK_REG_REG];
        RXVM_INSTRUMENTATION_INSTRUCTION_BEGIN(
                current_module->module_number, VM_CANONICAL_INDEX(pc),
                OP_UNLINK_REG, RXVM_HANDLER_IS_INLINE(PRIVATE_R1_RELINK));
#endif

    rxvm_handler_state_call:
        RXVM_HANDLER_STATE_SNAPSHOT(handler_state);
        handler_result =
                rxvm_invoke_outlined_handler(handler_function, &handler_state);
        RXVM_HANDLER_STATE_COMMIT(handler_state);
#endif

    rxvm_normal_handler_result:
        switch (handler_result) {
            case RXVM_HANDLER_RESULT_DISPATCH:
                VM_DISPATCH_TARGET();
            case RXVM_HANDLER_RESULT_INTERRUPT:
                goto INTERRUPT;
            case RXVM_HANDLER_RESULT_RESUME:
                VM_DISPATCH_TARGET();
            case RXVM_HANDLER_RESULT_INTERRUPT_TABLE_OOM:
                goto interrupt_table_oom;
            case RXVM_HANDLER_RESULT_FINISHED:
                goto interprt_finished;
        }
        abort();

    interrupt_table_oom:
        rxsignal_raise_private_table_oom(&pending_interrupts, current_frame, &interrupted_pc, pc,
                                         interrupt_object);
        RXVM_SELECTED_DISPATCH();

    interprt_finished:

    if (current_module && pc) {
        RXVM_INSTRUMENTATION_INSTRUCTION_TERMINAL(current_module->module_number,
                                                  VM_CANONICAL_INDEX(pc),
                                                  RXVM_TRANSITION_TERMINAL);
    }

    /* Cleanup / Remove OS Interrupt handlers */
    cleanup_vm_signals();

    /* Unwind any stack frames */
    while (current_frame) {
        temp_frame = current_frame->parent;
        if (context->ext_proc && !current_frame->parent) {
            free_external_entry_arguments(current_frame);
        }
        free_frame(current_frame);
        current_frame = temp_frame;
    }

    /* Deallocate Frames */
    /* We need to loop through each procedure in each module */
    DEBUG("Deallocating Frames and Registers\n");
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        size_t i;
        for (i = 0; i < context->modules[mod_index]->procedure_count; i++) {
            proc_runtime *runtime_proc = &context->modules[mod_index]->procedures[i];
            if (runtime_proc->frame_free_list == &runtime_proc->frame_free_list_head) {
                /* Free frames in the procedures free list */
                while (*(runtime_proc->frame_free_list)) {
                    temp_frame = *(runtime_proc->frame_free_list);
                    *(runtime_proc->frame_free_list) = temp_frame->prev_free;
                    clear_frame(temp_frame);
                    (void)rxvm_memory_release(temp_frame);
                }
            }
        }
    }

    /* Free signal value */
    value_free(signal_value);

    /* Free interrupt action value */
    value_free(interrupt_action_value);

    /* Free work registers */
    value_free(work1);
    value_free(work2);
    value_free(work3);

    /* Free interrupt argument */
    value_free(interrupt_arg);

    /* Free array of interrupt objects - interrupt_object[] */
    {
        size_t i;
        for (i = 0; i < RXSIGNAL_MAX; i++) {
            value_free(interrupt_object[i]);
        }
    }

    /* Free arguments array */
    if (arguments_array) {
        value_free(arguments_array);
    }

#ifndef NDEBUG
    if (context->debug_mode) rxvm_mprintf("Interpreter Finished with rc=%d\n", rc);
#endif

    RXVM_INSTRUMENTATION_VM_END(context, rc);

    rxvm_memory_leave(previous_memory_worker);

    if (rxvm_signal_leave_execution(
            context, &pending_interrupts,
            previous_pending_interrupts) != 0) {
        abort();
    }

    return rc;
}

#undef RXVM_SELECTED_TARGET
#undef RXVM_SELECTED_DISPATCH

#ifdef CREXX_VM_INSTRUMENTATION_TEST
#undef vm_instrumentation
#endif
#ifdef CREXX_VM_PROFILING
#undef vm_sequence
#undef vm_profile
#endif
#ifndef NTHREADED
#undef next_inst
#endif
#undef pending_interrupts
#undef current_module
#undef work1
#undef current_locals
#undef current_const_pool
#undef current_canonical_base
#undef current_execution_base
#undef current_binary_space
#undef temp_frame
#undef current_frame
#undef last_interrupted_module
#undef last_interrupted_address
#undef interrupt_object
#undef arguments_array
#undef interrupt_action_value
#undef signal_value
#undef mod_index
#undef interrupted_pc
#undef next_pc
#undef pc
#undef hasSeed
#undef initSeed
#undef rc
#undef context

int run(rxvm_context *context, int argc, char *argv[]) {
    rxvm_worker_transition_result transition;
    rxvm_context *previous_active_context;
    int rc;

    transition = rxvm_worker_begin_execution(&context->worker);

    if (transition != RXVM_WORKER_TRANSITION_OK) {
        fprintf(stderr,
                "RXVM worker execution rejected: %s (%s)\n",
                transition == RXVM_WORKER_TRANSITION_WRONG_THREAD
                    ? "wrong owner thread" : "invalid lifecycle state",
                rxvm_worker_state_name(
                        rxvm_worker_get_state(&context->worker)));
        return 1;
    }
    rxpa_compatibility_execution_enter(&context->rxpa_compatibility);

    if (rxvmplugin_instance_set_prepare(
            &context->plugin_instances, RXVM_PLUGIN_DECIMAL) != 0) {
        fprintf(stderr, "PANIC - No default decimal plugin\n");
        rxpa_compatibility_execution_leave(&context->rxpa_compatibility);
        if (rxvm_worker_end_execution(&context->worker) !=
                RXVM_WORKER_TRANSITION_OK) {
            abort();
        }
        return 255;
    }

    previous_active_context = rxvm_active_context_enter(context);
    rc = rxvm_run_owned_core(context, argc, argv);
    rxvm_active_context_leave(previous_active_context);
    rxpa_compatibility_execution_leave(&context->rxpa_compatibility);
    if (rxvm_worker_end_execution(&context->worker) !=
            RXVM_WORKER_TRANSITION_OK) {
        abort();
    }
    return rc;
}
