//
// Created by Adrian Sutherland on 16/09/2024.
// Defined the value / register structure for the interpreter
//

#ifndef CREXX_VALUE_H
#define CREXX_VALUE_H

#include <stddef.h>

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
#define DEFAULT_NUMERIC_STANDARD NUMERIC_STANDARD_COMMON

/* Minimum value for digits */
#define DIGITS_MINIMUM 5

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

typedef union {
    /* todo - these flag definitions are not used and are not correct */
    struct {
        unsigned int type_object : 1;
        unsigned int type_string : 1;
        unsigned int type_decimal : 1;
        unsigned int type_float : 1;
        unsigned int type_int : 1;
    };
    unsigned int all_type_flags;
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
    size_t binary_buffer_length; // binary_value buffer length
    value **attributes;
    value **unlinked_attributes;
    value **attribute_buffers;
    size_t max_num_attributes;
    size_t num_attributes;
    size_t num_attribute_buffers;
    char small_string_buffer[SMALLEST_STRING_BUFFER_LENGTH];
};


#endif //CREXX_VALUE_H
