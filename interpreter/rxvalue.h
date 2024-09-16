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
typedef long rxinteger;
#else
typedef long long rxinteger;
#endif
#endif
#endif //RXINTEGER_T

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
    void *decimal_value;
    size_t decimal_value_length;
    char *string_value;
    size_t string_length;
    size_t string_buffer_length;
    size_t string_pos;
#ifndef NUTF8
    size_t string_chars;
    size_t string_char_pos;
#endif
    char *binary_value;
    size_t binary_length;
    value **attributes;
    value **unlinked_attributes;
    value **attribute_buffers;
    size_t max_num_attributes;
    size_t num_attributes;
    size_t num_attribute_buffers;
    char small_string_buffer[SMALLEST_STRING_BUFFER_LENGTH];
};


#endif //CREXX_VALUE_H
