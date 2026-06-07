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

//
// Created by Adrian Sutherland on 16/09/2024.
// Defined the value / register structure for the interpreter
//

#ifndef CREXX_VALUE_H
#define CREXX_VALUE_H

#include <stddef.h>
#include <stdint.h>

#define SMALLEST_STRING_BUFFER_LENGTH 32

// Define rxinteger type
#ifndef RXINTEGER_T
#define RXINTEGER_T
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L /* C99 */
#include <stdint.h>
typedef intmax_t rxinteger;
#else
#ifdef __32BIT__
typedef long rxinteger;    // Legacy C90
#else
#include <stdint.h>
typedef int64_t rxinteger; // C99+
#endif
#endif
#define IS_RXINTEGER_32BIT (sizeof(rxinteger) == 4)
#endif //RXINTEGER_T

/* The default number of digits. This constant is also the point at which integers and floating numbers are
 * truncated. If it is lower than 18, then integers are floats are rounded, if 18 or higher they are not rounded (for performance).
 * However, when displaying numbers, the digit number is always honored */
#define DIGITS_STRIKE_POINT 18

/* Default numeric context values */
#define DEFAULT_NUMERIC_DIGITS DIGITS_STRIKE_POINT
#define DEFAULT_NUMERIC_FUZZ 0
#define DEFAULT_NUMERIC_FORM NUMERIC_FORM_SCIENTIFIC
#define DEFAULT_NUMERIC_CASE CASE_LOWER

/* Minimum value for digits */
#define DIGITS_MINIMUM 1

/* Enumeration of numeric display format "form" types - scientific or engineering */
typedef enum {
    NUMERIC_FORM_INHERIT = 0,
    NUMERIC_FORM_SCIENTIFIC,
    NUMERIC_FORM_ENGINEERING
} numeric_form;

/* Enumeration of case types for numeric display */
typedef enum {
    CASE_INHERIT = 0,
    CASE_LOWER,
    CASE_UPPER
} case_type;

/* Enumeration of the numeric standards */
typedef enum {
    NUMERIC_STANDARD_INHERIT = 0,
    NUMERIC_STANDARD_COMMON,
    NUMERIC_STANDARD_CLASSIC
} numeric_standard;


/* Structure to hold the numeric context (digits, fuzz, form, etc.) */
typedef struct numeric_context {
    int digits;             /* Significant digits, -1 = inherited */
    int fuzz;               /* Fuzz factor, -1 = inherited */
    numeric_form form;      /* NUMERIC_FORM_INHERIT, NUMERIC_FORM_SCIENTIFIC, NUMERIC_FORM_ENGINEERING */
    case_type casetype;     /* CASE_INHERIT, CASE_LOWER, CASE_UPPER */
    numeric_standard standard; /* NUMERIC_STANDARD_INHERIT, NUMERIC_STANDARD_COMMON, NUMERIC_STANDARD_CLASSIC */
} numeric_context;

typedef struct value value;
typedef struct rxvm_reference_cell rxvm_reference_cell;

typedef enum rxvm_ref_state {
    RXVM_REF_INVALID = 0,
    RXVM_REF_VALID = 1
} rxvm_ref_state;

typedef enum rxvm_ref_owner_kind {
    RXVM_REF_OWNER_NONE = 0,
    RXVM_REF_LOCAL,
    RXVM_REF_ARGUMENT,
    RXVM_REF_GLOBAL,
    RXVM_REF_ATTRIBUTE
} rxvm_ref_owner_kind;

struct rxvm_reference_cell {
    uint64_t id;
    unsigned int retain_count;
    rxvm_ref_state state;
    rxvm_ref_owner_kind owner_kind;
    value *target;
    void *owner;
    uint64_t owner_generation;
    const char *debug_name;
};

#ifndef RXVM_NATIVE_PAYLOAD_OPS_DEFINED
#define RXVM_NATIVE_PAYLOAD_OPS_DEFINED
#define RXVM_NATIVE_PAYLOAD_FLAG_BITCOPY_SAFE 0x00000001u

typedef struct rxvm_native_payload_ops {
    const char *type_name;
    void (*copy)(void *dest_value, void *source_value);
    void (*finalize)(void *value);
} rxvm_native_payload_ops;
#endif

typedef union {
    /* Register/value status flags. Masks are centralized in binutils/include/rxflags.h. */
    uint32_t all_type_flags;
} value_type;

struct value {
    /* bit field to store value status - these are explicitly set (not automatic at all) */
    value_type status;

    /* Value */
    rxinteger int_value;
    double float_value;
    void *decimal_value; // Must be malloced
    size_t decimal_value_length; // decimal_value length
    size_t decimal_buffer_length; // decimal_value buffer length
    char *string_value;
    size_t string_length;
    size_t string_buffer_length;
    size_t string_pos;
#ifndef NUTF8
    size_t string_chars;
    size_t string_char_pos;
#endif
    char *binary_value; // Must be malloced
    size_t binary_length; // binary_value length
    size_t binary_pos; // byte cursor for binary operations
    size_t binary_buffer_length; // binary_value buffer length
    const rxvm_native_payload_ops *native_payload_ops; // Shared native payload operations, or NULL
    unsigned int native_payload_flags;
    rxvm_reference_cell *reference_identity; // This value is a reference target
    rxvm_reference_cell *reference_payload; // This value is itself a reference
    const char *object_type_name; // Runtime concrete class name, may point into a module constant pool
    size_t object_type_name_length;
    value **attributes;
    value **unlinked_attributes;
    value **attribute_buffers;
    size_t max_num_attributes;
    size_t num_attributes;
    size_t num_attribute_buffers;
    char small_string_buffer[SMALLEST_STRING_BUFFER_LENGTH];
};


#endif //CREXX_VALUE_H
