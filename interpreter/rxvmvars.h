/* CREXX
 * RUNTIME Variable Support
 */

#ifndef CREXX_RXVMVARS_H
#define CREXX_RXVMVARS_H

#ifndef NUTF8
#include "utf.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>

/* Forward declarations */
static void extract_double_decimal(numeric_context* num_context, value *coefficient, value *exponent, double value);
static void extract_integer_decimal(numeric_context* num_context, value *coefficient, value *exponent, rxinteger value);
static void RexxDecimalFormat(numeric_context* num_context, value *coefficient_value, value *exponent_value, value *formatted_output_value);

/* Zeros a register value */
RX_INLINE void value_zero(value *v) {
    v->status.all_type_flags = 0;
    v->int_value = 0;
    v->float_value = 0;
    v->string_length = 0; // Lazy Free String - just zero the used length
    v->string_pos = 0;
#ifndef NUTF8
    v->string_chars = 0;
    v->string_char_pos = 0;
#endif
    v->num_attributes = 0;

    /* Lazy Free Decimal - just zero the used length */
    v->decimal_value_length = 0;

    /* Lazy Free binary - just zero the used length */
    v->binary_length = 0;
}

/* Setup a new value structure */
RX_INLINE void value_init(value *v) {
    v->string_value = v->small_string_buffer;
    v->string_buffer_length = sizeof(v->small_string_buffer);
    v->attributes = 0;
    v->unlinked_attributes = 0;
    v->attribute_buffers = 0;
    v->num_attribute_buffers = 0;
    v->max_num_attributes = 0;
    v->binary_value = 0;
    v->binary_buffer_length = 0;
    v->decimal_value = 0;
    v->decimal_value_length = 0;
    v->decimal_buffer_length = 0;
    value_zero(v);
}

/* Value Factory mallocs and inits */
RX_INLINE value* value_f() {
    value* this;
    this = malloc(sizeof(value));
    value_init(this);
    return this;
}

/*
 * Returns required buffer size - the smallest power of two that's greater or
 * equal to a given value. A minimum size is enforced.
 */
RX_INLINE size_t power_of_two_size(size_t value) {
    if (value == 0) return 0; // Handle zero input
    size_t new_size = value > 8 ? value : 8; // Enforce a minimum size of 8
    new_size--;
    new_size |= new_size >> 1;
    new_size |= new_size >> 2;
    new_size |= new_size >> 4;
    new_size |= new_size >> 8;
    new_size |= new_size >> 16;
#if __SIZEOF_SIZE_T__ == 8
    new_size |= new_size >> 32;
#endif
    new_size++;
    return new_size;
}

/* Sets up the required number of attributes */
RX_INLINE void set_num_attributes(value* v, size_t num) {
    size_t i;
    value *a;

    if (num <= v->num_attributes) {
        /* Reducing the number of attributes is easy */
        v->num_attributes = num;
        return;
    }

    if (num <= v->max_num_attributes) {
        /* Just need to reset the recycled attributes */
        for (i = v->num_attributes; i < num; i++) {
            v->attributes[i] = v->unlinked_attributes[i]; /* Ensure Attribute is unlinked */
            value_zero(v->attributes[i]);
        }
        v->num_attributes = num;
        return;
    }

    /* Increasing the number of attributes, we need to allocate more space */

    /* We first need to recycle any unused attributes */
    for (i = v->num_attributes; i < v->max_num_attributes; i++) {
        v->attributes[i] = v->unlinked_attributes[i]; /* Ensure Attribute is unlinked */
        value_zero(v->attributes[i]);
    }

    /* Calculate the new maximum number of attributes using bit-twiddling */
    size_t new_max = power_of_two_size(num);

    /* Now we need to make the pointer arrays big enough */
    if (v->attributes) v->attributes = realloc(v->attributes, sizeof(value*) * new_max);
    else v->attributes = malloc(sizeof(value*) * new_max);

    if (v->unlinked_attributes) v->unlinked_attributes = realloc(v->unlinked_attributes, sizeof(value*) * new_max);
    else v->unlinked_attributes = malloc(sizeof(value*) * new_max);

    /* We create a buffer for the new attributes separate to the existing buffers. */
    size_t old_capacity = power_of_two_size(v->num_attribute_buffers);
    v->num_attribute_buffers++;
    size_t new_capacity = power_of_two_size(v->num_attribute_buffers);

    // Reallocate only when the required capacity has changed
    if (new_capacity > old_capacity) {
        if (v->attribute_buffers) {
            v->attribute_buffers = realloc(v->attribute_buffers, sizeof(value*) * new_capacity);
        } else {
            v->attribute_buffers = malloc(sizeof(value*) * new_capacity);
        }
    }

    /* Create a new buffer */
    v->attribute_buffers[v->num_attribute_buffers - 1] =
            malloc(sizeof(value) * (new_max - v->max_num_attributes));

    /* Initiate the new attributes */
    a = v->attribute_buffers[v->num_attribute_buffers - 1];
    for (i = v->max_num_attributes; i < new_max; i++, a++) {
        value_init(a);
        v->attributes[i] = v->unlinked_attributes[i] = a;
    }

    /* Set the new number of attributes */
    v->num_attributes = num;
    v->max_num_attributes = new_max;
}

/*
 * Returns required buffer size - the smallest power of two that's greater or
 * equal to a given value
 */
RX_INLINE size_t buffer_size(size_t value) {
    size_t i;
    if (value <= SMALLEST_STRING_BUFFER_LENGTH)
        return SMALLEST_STRING_BUFFER_LENGTH;

    return power_of_two_size(value);
}

RX_INLINE void prep_string_buffer(value *v, size_t length) {
    v->string_length = length;
    if (v->string_length > v->string_buffer_length) {
        if (v->string_value != v->small_string_buffer) free(v->string_value);
        v->string_buffer_length = buffer_size(v->string_length);
        v->string_value = malloc(v->string_buffer_length);
    }
}

RX_INLINE void extend_string_buffer(value *v, size_t length) {
    v->string_length = length;
    if (v->string_length > v->string_buffer_length) {
        v->string_buffer_length = buffer_size(v->string_length);
        if (v->string_value == v->small_string_buffer) {
            v->string_value = malloc(v->string_buffer_length);
            memcpy(v->string_value, v->small_string_buffer, sizeof(v->small_string_buffer));
        }
        else {
            v->string_value = realloc(v->string_value, v->string_buffer_length);
        }
    }
}

RX_INLINE void null_terminate_string_buffer(value *v) {

    if (v->string_length + 1 > v->string_buffer_length) {
        /* Make room for the null */
        extend_string_buffer(v, v->string_length + 1);

        /* extend_string_buffer() increments string_length so put it back */
        v->string_length--;
    }

    /* Add the null */
    v->string_value[v->string_length] = 0;
}

/* Clears a value of all its children and other malloced buffers */
RX_MOSTLYINLINE void clear_value(value* v) {
    int i;

    /* Clear attribute values */
    if (v->unlinked_attributes) {
        for (i = 0; i < v->max_num_attributes; i++) {
            if (v->unlinked_attributes[i]) {
                clear_value(v->unlinked_attributes[i]);
            }
        }
        free(v->unlinked_attributes);
        v->unlinked_attributes = 0;
    }

    /* Free attribute buffer */
    if (v->attribute_buffers) {
        for (i = 0; i < v->num_attribute_buffers; i++) {
            if (v->attribute_buffers[i]) free(v->attribute_buffers[i]);
        }
        free(v->attribute_buffers);
        v->attribute_buffers = 0;
        v->num_attribute_buffers = 0;
    }

    /* Free pointer arrays */
    if (v->attributes) {
        free(v->attributes);
        v->attributes = 0;
    }
    v->max_num_attributes = 0;
    v->num_attributes = 0;

    /* Free strings */
    if (v->string_value != v->small_string_buffer) {
        free(v->string_value);
        v->string_value = v->small_string_buffer;
        v->string_buffer_length = sizeof(v->small_string_buffer);
    }
    v->string_length = 0;
    v->string_pos = 0;
#ifndef NUTF8
    v->string_chars = 0;
    v->string_char_pos = 0;
#endif

    /* Free decimal */
    if (v->decimal_value) free(v->decimal_value);
    v->decimal_value = 0;
    v->decimal_value_length = 0;
    v->decimal_buffer_length = 0;

    /* Free binary */
    if (v->binary_value) free(v->binary_value);
    v->binary_value = 0;
    v->binary_length = 0;
    v->binary_buffer_length = 0;

    value_zero(v);
}

/* Int Flag */
RX_INLINE void set_type_int(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_int = 1;
}

RX_INLINE void add_type_int(value *v) {
    v->status.type_int = 1;
}

RX_INLINE unsigned int get_type_int(value *v) {
    return v->status.type_int;
}

/* Float Flag */
RX_INLINE void set_type_float(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_float = 1;
}

RX_INLINE void add_type_float(value *v) {
    v->status.type_float = 1;
}

RX_INLINE unsigned int get_type_float(value *v) {
    return v->status.type_float;
}

/* Decimal Flag */
RX_INLINE void set_type_decimal(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_decimal = 1;
}

RX_INLINE void add_type_decimal(value *v) {
    v->status.type_decimal = 1;
}

RX_INLINE unsigned int get_type_decimal(value *v) {
    return v->status.type_decimal;
}

/* String Flag */
RX_INLINE void set_type_string(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_string = 1;
}

RX_INLINE void add_type_string(value *v) {
    v->status.type_string = 1;
}

RX_INLINE unsigned int get_type_string(value *v) {
    return v->status.type_string;
}

/* Object Flag */
RX_INLINE void set_type_object(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_object = 1;
}

RX_INLINE void add_type_object(value *v) {
    v->status.type_object = 1;
}

RX_INLINE unsigned int get_type_object(value *v) {
    return v->status.type_object;
}

/* Unset Flag */
RX_INLINE void unset_type(value *v) {
    v->status.all_type_flags = 0;
}

RX_INLINE void set_int(value *v, rxinteger value) {
    v->int_value = value;
}
RX_INLINE void set_float(value *v, double value) {
    v->float_value = value;
}

RX_INLINE void set_string(value *v, char *value, size_t length) {
    prep_string_buffer(v,length);
    memcpy(v->string_value, value, v->string_length);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = utf8nlen(v->string_value, v->string_length); /* SLOW! */
#endif
}

/* set value string from null string value */
RX_INLINE void set_null_string(value *v, const char *from) {
    if (v->string_value == from) return;
    prep_string_buffer(v, strlen(from));
    memcpy(v->string_value, from, v->string_length);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = utf8nlen(v->string_value, v->string_length);
#endif
}

RX_INLINE void set_const_string(value *v, string_constant *from) {
    prep_string_buffer(v,from->string_len);
    memcpy(v->string_value, from->string, v->string_length);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = from->string_chars;
#endif
}

RX_INLINE void set_value_string(value *v, value *from) {
    prep_string_buffer(v, from->string_length);
    memcpy(v->string_value, from->string_value, v->string_length);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = from->string_chars;
#endif
}

RX_INLINE void set_buffer_string(
        value *v,
        char *buffer,
        size_t length,
        size_t buffer_length
#ifndef NUTF8
        , size_t string_chars
#endif
) {
    if (v->string_value != v->small_string_buffer) free(v->string_value);
    v->string_value = buffer;
    v->string_length = length;
    v->string_buffer_length = buffer_length;
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = string_chars;
#endif
}

/* Copy a value */
RX_MOSTLYINLINE void copy_value(value *dest, value *source) {
    size_t i;

    if (dest == source) return;

    dest->status.all_type_flags = source->status.all_type_flags;
    dest->int_value = source->int_value;
    dest->float_value = source->float_value;

    /* Copy Decimal Value */
    if (source->decimal_value_length) {
        dest->decimal_value_length = source->decimal_value_length;
        dest->decimal_buffer_length = dest->decimal_value_length;
        if (dest->decimal_value) dest->decimal_value = realloc(dest->decimal_value, dest->decimal_value_length);
        else dest->decimal_value = malloc(dest->decimal_value_length);
        memcpy(dest->decimal_value, source->decimal_value, dest->decimal_value_length);
    }
    else {
        dest->decimal_value_length = 0;
    }

    /* Copy Strings */
    if (source->string_length) {
        /* Copy String Data */
        prep_string_buffer(dest, source->string_length);
        dest->string_pos = source->string_pos;
#ifndef NUTF8
        dest->string_chars = source->string_chars;
        dest->string_char_pos = source->string_char_pos;
#endif
        memcpy(dest->string_value, source->string_value, dest->string_length);
    }
    else {
        dest->string_length = 0;
        dest->string_pos = 0;
#ifndef NUTF8
        dest->string_chars = 0;
        dest->string_char_pos = 0;;
#endif
    }

    /* Copy Binary */
    if (source->binary_length) {
        dest->binary_length = source->binary_length;
        dest->binary_buffer_length = dest->binary_length;
        if (dest->binary_value) dest->binary_value = realloc(dest->binary_value, dest->binary_length);
        else dest->binary_value = malloc(dest->binary_length);
        memcpy(dest->binary_value, source->binary_value, dest->binary_length);
    }
    else {
        dest->binary_length = 0;
    }

    /* Copy Attributes */
    set_num_attributes(dest,source->num_attributes);
    for (i = 0; i < dest->num_attributes; i++)
        copy_value(dest->attributes[i], source->attributes[i]);
}

/* Move a value */
RX_INLINE void move_value(value *dest, value *source) {
    if (dest == source) return;

    /* Clear out destination - including string / attributes */
    clear_value(dest);
    value_init(dest);

    /* Copy basic values */
    dest->status.all_type_flags = source->status.all_type_flags;
    dest->int_value = source->int_value;
    dest->float_value = source->float_value;

    /* Move Decimal Value */
    if (source->decimal_value_length) {
        dest->decimal_value_length = source->decimal_value_length;
        dest->decimal_value = source->decimal_value;
        dest->decimal_buffer_length = source->decimal_buffer_length;
        source->decimal_value = 0;
        source->decimal_value_length = 0;
        source->decimal_buffer_length = 0;
    }

    /* Move String */
    if (source->string_length) {
        if (source->string_value == source->small_string_buffer) {
            /* Copy String Data */
            prep_string_buffer(dest, source->string_length);
            dest->string_pos = source->string_pos;
#ifndef NUTF8
            dest->string_chars = source->string_chars;
            dest->string_char_pos = source->string_char_pos;
#endif
            memcpy(dest->string_value, source->string_value,
                   dest->string_length);
        }
        else {
            /* Move String Data */
            dest->string_value = source->string_value;
            dest->string_length = source->string_length;
            dest->string_pos = source->string_pos;
#ifndef NUTF8
            dest->string_chars = source->string_chars;
            dest->string_char_pos = source->string_char_pos;
#endif
        }
    }
    else {
        dest->string_length = 0;
        dest->string_pos = 0;
#ifndef NUTF8
        dest->string_chars = 0;
        dest->string_char_pos = 0;;
#endif
    }

    /* Move Binary */
    if (source->binary_value) {
        dest->binary_length = source->binary_length;
        dest->binary_value = source->binary_value;
        dest->binary_buffer_length = source->binary_buffer_length;
        source->binary_value = 0;
        source->binary_length = 0;
        source->binary_buffer_length = 0;
    }

    /* Move Attributes */
    dest->attributes = source->attributes;
    dest->unlinked_attributes = source->unlinked_attributes;
    dest->attribute_buffers = source->attribute_buffers;
    dest->max_num_attributes = source->max_num_attributes;
    dest->num_attributes = source->num_attributes;
    dest->num_attribute_buffers = source->num_attribute_buffers;

    /* Reset / fixup source */
    value_init(source);
}

/* Copy string value */
RX_INLINE void copy_string_value(value *dest, value *source) {
    if (dest == source) return;
    if (source->string_length) {
        /* Copy String Data */
        prep_string_buffer(dest, source->string_length);
        dest->string_pos = source->string_pos;
#ifndef NUTF8
        dest->string_chars = source->string_chars;
        dest->string_char_pos = source->string_char_pos;
#endif
        memcpy(dest->string_value, source->string_value, source->string_length);
    }
    else {
        dest->string_length = 0;
        dest->string_pos = 0;
#ifndef NUTF8
        dest->string_chars = 0;
        dest->string_char_pos = 0;
#endif
    }
}

/* Compares two strings. returns -1, 0, 1 as appropriate */
#define MIN(a,b) (((a)<(b))?(a):(b))
RX_INLINE int string_cmp(char *value1, size_t length1, char *value2, size_t length2) {
    int ret;

    ret = memcmp(value1, value2, MIN(length1, length2));
    if (!ret) ret = length1 - length2;
    ret = ret > 0 ? 1 : (ret < 0 ? -1 : 0);

    return ret;
}

RX_INLINE int string_cmp_value(value *v1, value *v2) {
    return string_cmp(v1->string_value, v1->string_length,
                      v2->string_value, v2->string_length);
}

RX_INLINE int string_cmp_const(value *v1, string_constant *v2) {
    return string_cmp(v1->string_value, v1->string_length,
                      v2->string, v2->string_len);
}

RX_INLINE void string_append(value *v1, value *v2) {
    size_t start = v1->string_length;

    extend_string_buffer(v1, v1->string_length + v2->string_length);
    memcpy(v1->string_value + start, v2->string_value, v2->string_length);
    v1->string_pos = 0;
#ifndef NUTF8
    v1->string_char_pos = 0;
    v1->string_chars += v2->string_chars;
#endif
}

RX_INLINE void string_append_chars(value *v1, char *value, size_t length) {
    size_t start = v1->string_length;

    extend_string_buffer(v1, v1->string_length + length);
    memcpy(v1->string_value + start, value, length);

    v1->string_pos = 0;
#ifndef NUTF8
    v1->string_char_pos = 0;
    v1->string_chars += utf8nlen(value, length); /* SLOW! */
#endif
}

RX_INLINE void string_sappend(value *v1, value *v2) {
    size_t start = v1->string_length;

    extend_string_buffer(v1, v1->string_length + v2->string_length + 1);
    v1->string_value[start++] = ' ';
    memcpy(v1->string_value + start, v2->string_value, v2->string_length);
    v1->string_pos = 0;
#ifndef NUTF8
    v1->string_char_pos = 0;
    v1->string_chars += v2->string_chars + 1;
#endif
}

RX_INLINE void string_concat(value *v1, value *v2, value *v3) {
    size_t len = v2->string_length + v3->string_length ;
    size_t buffer_len = buffer_size(len);
    char *buffer;
    if (v1 == v2 || v1 == v3) {
        /* Need to use a buffer */
        buffer = malloc(buffer_len);

        memcpy(buffer, v2->string_value, v2->string_length);
        memcpy(buffer + v2->string_length, v3->string_value, v3->string_length);
        v1->string_pos = 0;

#ifdef NUTF8
        set_buffer_string(v1, buffer, len, buffer_len);
#else
        set_buffer_string(v1, buffer, len, buffer_len, v2->string_chars + v3->string_chars);
        v1->string_char_pos = 0;
#endif
    }
    else {
        /* Can write into v1 directly */
        prep_string_buffer(v1, len);
        memcpy(v1->string_value, v2->string_value, v2->string_length);
        memcpy(v1->string_value + v2->string_length, v3->string_value, v3->string_length);
        v1->string_pos = 0;
#ifndef NUTF8
        v1->string_chars = v2->string_chars + v3->string_chars;
        v1->string_char_pos = 0;
#endif
    }
}

RX_INLINE void string_sconcat(value *v1, value *v2, value *v3) {
    size_t len = v2->string_length + v3->string_length + 1;
    size_t buffer_len = buffer_size(len);
    char *buffer;
    if (v1 == v2 || v1 == v3) {
        /* Need to use a buffer */
        buffer = malloc(buffer_len);

        memcpy(buffer, v2->string_value, v2->string_length);
        buffer[v2->string_length] = ' ';
        memcpy(buffer + v2->string_length + 1, v3->string_value, v3->string_length);
        v1->string_pos = 0;
#ifdef NUTF8
        set_buffer_string(v1, buffer, len, buffer_len);
#else
        set_buffer_string(v1, buffer, len, buffer_len, v2->string_chars + v3->string_chars + 1);
        v1->string_char_pos = 0;
#endif
    }
    else {
        /* Can write into v1 directly */
        prep_string_buffer(v1, len);
        memcpy(v1->string_value, v2->string_value, v2->string_length);
        v1->string_value[v2->string_length] = ' ';
        memcpy(v1->string_value + v2->string_length + 1, v3->string_value, v3->string_length);
        v1->string_pos = 0;
#ifndef NUTF8
        v1->string_chars = v2->string_chars + v3->string_chars + 1;
        v1->string_char_pos = 0;
#endif
    }
}

RX_INLINE void string_concat_var_const(value *v1, value *v2, string_constant *v3) {
    size_t len = v2->string_length + v3->string_len;
    size_t buffer_len = buffer_size(len);
    char *buffer;
    if (v1 == v2) {
        /* Need to use a buffer */
        buffer = malloc(buffer_len);

        memcpy(buffer, v2->string_value, v2->string_length);
        memcpy(buffer + v2->string_length, v3->string, v3->string_len);
        v1->string_pos = 0;

#ifdef NUTF8
        set_buffer_string(v1, buffer, len, buffer_len);
#else
        set_buffer_string(v1, buffer, len, buffer_len,
                          v2->string_chars + v3->string_chars);
        v1->string_char_pos = 0;
#endif
    }
    else {
        /* Can write into v1 directly */
        prep_string_buffer(v1, len);
        memcpy(v1->string_value, v2->string_value, v2->string_length);
        memcpy(v1->string_value + v2->string_length, v3->string, v3->string_len);
        v1->string_pos = 0;
#ifndef NUTF8
        v1->string_chars = v2->string_chars + v3->string_chars;
        v1->string_char_pos = 0;
#endif
    }
}

RX_INLINE void string_sconcat_var_const(value *v1, value *v2, string_constant *v3) {
    size_t len = v2->string_length + v3->string_len + 1;
    size_t buffer_len = buffer_size(len);
    char *buffer;
    if (v1 == v2) {
        /* Need to use a buffer */
        buffer = malloc(buffer_len);

        memcpy(buffer, v2->string_value, v2->string_length);
        buffer[v2->string_length] = ' ';
        memcpy(buffer + v2->string_length + 1, v3->string, v3->string_len);
        v1->string_pos = 0;

#ifdef NUTF8
        set_buffer_string(v1, buffer, len, buffer_len);
#else
        set_buffer_string(v1, buffer, len, buffer_len,
                          v2->string_chars + v3->string_chars + 1);
        v1->string_char_pos = 0;
#endif
    }
    else {
        /* Can write into v1 directly */
        prep_string_buffer(v1, len);
        memcpy(v1->string_value, v2->string_value, v2->string_length);
        v1->string_value[v2->string_length] = ' ';
        memcpy(v1->string_value + v2->string_length + 1, v3->string, v3->string_len);
        v1->string_pos = 0;
#ifndef NUTF8
        v1->string_chars = v2->string_chars + v3->string_chars + 1;
        v1->string_char_pos = 0;
#endif
    }
}

RX_INLINE void string_concat_const_var(value *v1, string_constant *v2, value *v3) {
    size_t len = v2->string_len + v3->string_length;
    size_t buffer_len = buffer_size(len);
    char *buffer;
    if (v1 == v3) {
        /* Need to use a buffer */
        buffer = malloc(buffer_len);

        memcpy(buffer, v2->string, v2->string_len);
        memcpy(buffer + v2->string_len, v3->string_value, v3->string_length);
        v1->string_pos = 0;

#ifdef NUTF8
        set_buffer_string(v1, buffer, len, buffer_len);
#else
        set_buffer_string(v1, buffer, len, buffer_len,
                          v2->string_chars + v3->string_chars);
        v1->string_char_pos = 0;
#endif
    }
    else {
        /* Can write into v1 directly */
        prep_string_buffer(v1, len);

        memcpy(v1->string_value, v2->string, v2->string_len);
        memcpy(v1->string_value + v2->string_len, v3->string_value, v3->string_length);
        v1->string_pos = 0;
#ifndef NUTF8
        v1->string_chars = v2->string_chars + v3->string_chars;
        v1->string_char_pos = 0;
#endif
    }
}

RX_INLINE void string_sconcat_const_var(value *v1, string_constant *v2, value *v3) {
    size_t len = v2->string_len + v3->string_length + 1;
    size_t buffer_len = buffer_size(len);
    char *buffer;
    if (v1 == v3) {
        /* Need to use a buffer */
        buffer = malloc(buffer_len);

        memcpy(buffer, v2->string, v2->string_len);
        buffer[v2->string_len] = ' ';
        memcpy(buffer + v2->string_len + 1, v3->string_value,
               v3->string_length);
        v1->string_pos = 0;

#ifdef NUTF8
        set_buffer_string(v1, buffer, len, buffer_len);
#else
        set_buffer_string(v1, buffer, len, buffer_len,
                          v2->string_chars + v3->string_chars + 1);
        v1->string_char_pos = 0;
#endif
    }
    else {
        /* Can write into v1 directly */
        prep_string_buffer(v1, len);
        memcpy(v1->string_value, v2->string, v2->string_len);
        v1->string_value[v2->string_len] = ' ';
        memcpy(v1->string_value + v2->string_len + 1, v3->string_value,
               v3->string_length);
        v1->string_pos = 0;
#ifndef NUTF8
        v1->string_chars = v2->string_chars + v3->string_chars + 1;
        v1->string_char_pos = 0;
#endif
    }
}

#ifndef NUTF8
/* This sets v's string_pos (the byte index) and v's string_char_pos
 * (the utf8 codepoint index) based on a new string_char_pos */
RX_INLINE void string_set_byte_pos(value *v, size_t new_string_char_pos) {
    assert (v->string_char_pos <= v->string_chars);

    // Boundary Check: If the requested position is beyond the last character,
    // clamp it to the end of the string.
    if (new_string_char_pos >= v->string_chars) {
        if (v->string_chars == 0) {
            v->string_pos = 0;
            v->string_char_pos = 0;
        } else {
            // Position at the very end of the string.
            // string_char_pos can be equal to string_chars, indicating a position
            // after the last character, which is useful for appending.
            // Here we will just set it to the last valid character index for seeking.
            v->string_pos = v->string_length;
            v->string_char_pos = v->string_chars;
        }
        return;
    }

    int diff = (int)new_string_char_pos - (int)v->string_char_pos;

    if (diff == 0) {
        return; // Nothing to do
    }

    // Optimised for stepping one character forward or backward
    if (diff == 1) {
        v->string_pos += utf8codepointcalcsize(v->string_value + v->string_pos);
        v->string_char_pos++;
        return;
    }
    if (diff == -1) {
        v->string_pos -= utf8rcodepointcalcsize(v->string_value + v->string_pos);
        v->string_char_pos--;
        return;
    }

    // For larger jumps, determine the most efficient starting point.
    // We compare the cost of seeking from the start, the current position, or the end.
    size_t cost_from_start = new_string_char_pos;
    size_t cost_from_current = (diff > 0) ? diff : -diff;
    size_t cost_from_end = v->string_chars - new_string_char_pos;

    if (cost_from_start <= cost_from_current && cost_from_start <= cost_from_end) {
        // Seek from the beginning
        v->string_char_pos = 0;
        v->string_pos = 0;
        while (v->string_char_pos < new_string_char_pos) {
            v->string_pos += utf8codepointcalcsize(v->string_value + v->string_pos);
            v->string_char_pos++;
        }
    } else if (cost_from_end < cost_from_current) {
        // Seek from the end (backwards)
        v->string_char_pos = v->string_chars;
        v->string_pos = v->string_length;
        while (v->string_char_pos > new_string_char_pos) {
            v->string_pos -= utf8rcodepointcalcsize(v->string_value + v->string_pos);
            v->string_char_pos--;
        }
    } else {
        // Seek from the current position
        if (diff > 0) { // Forward
            while (v->string_char_pos < new_string_char_pos) {
                v->string_pos += utf8codepointcalcsize(v->string_value + v->string_pos);
                v->string_char_pos++;
            }
        } else { // Backward
            while (v->string_char_pos > new_string_char_pos) {
                v->string_pos -= utf8rcodepointcalcsize(v->string_value + v->string_pos);
                v->string_char_pos--;
            }
        }
    }
}
#endif

RX_INLINE void string_concat_char(value *v1, value *v2) {
    int char_size;
    char *insert_at;

    v1->string_pos = v1->string_length;
#ifdef NUTF8
    char_size = 1;
#else
    v1->string_char_pos = v1->string_chars;
    char_size = utf8codepointsize(v2->int_value);
#endif

    extend_string_buffer(v1,v1->string_length + char_size);
    insert_at = v1->string_value + v1->string_pos;

#ifdef NUTF8
    *insert_at = (unsigned char)v2->int_value;
#else
    v1->string_chars += 1;
    utf8catcodepoint(insert_at, v2->int_value, char_size);
#endif
}

/* ****************************************************************************/
/* Funnctions to support operators for both the interpreter and the optimizer */
/* ****************************************************************************/

/* Calculate the string value */
RX_INLINE void int_to_string(numeric_context *cnt, value *temp, value *v) {
    if (cnt->digits >= DIGITS_STRIKE_POINT) {
        // Fast path for a large number of digits - just convert the integer to string and set exponent to 0
        prep_string_buffer(v, SMALLEST_STRING_BUFFER_LENGTH); // Large enough for an int
#ifdef __32BIT__
        v->string_length = snprintf(v->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%ld",(long)v->int_value);
#else
        v->string_length = snprintf(v->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%lld",(long long)v->int_value);
#endif
        v->string_pos = 0;
#ifndef NUTF8
        v->string_char_pos = 0;
        v->string_chars = v->string_length;
#endif
        return;
    }

    extract_integer_decimal(cnt,temp, temp, v->int_value);
    RexxDecimalFormat(cnt, temp, temp, v);
}

/* Calculate the string value of v from its float value
 * cnt - numeric context is needed to ensure the format is correct
 * temp - a work buffer for the conversion
 * v - the value to convert (float value -> string value)
 */
RX_INLINE void float_to_string(numeric_context *cnt, value *temp, value *v) {
    extract_double_decimal(cnt,temp, temp, v->float_value);
    RexxDecimalFormat(cnt, temp, temp, v);
}

/* Calculate the integer value from float */
RX_INLINE void int_from_float(value *v) {
    v->int_value = floor(v->float_value);
    if (v->float_value - (double)v->int_value > 0.5) v->int_value++;
}

/* Make a malloced null terminated string from a register - needs to be free()d */
RX_INLINE char* reg2nullstring(value* reg) {
    char *buffer = malloc(reg->string_length + 1);

    /* Null terminated buffer */
    buffer[reg->string_length] = 0;
    memcpy(buffer, reg->string_value, reg->string_length);

    return buffer;
}

/* Convert a string to an integer - returns 1 on error */
RX_INLINE int string2integer(rxinteger *out, char *string, size_t length) {
    char *buffer = malloc(length + 1);
    char *end = buffer;
    int rc = 0;
    errno = 0;

    /* Null terminated buffer */
    buffer[length] = 0;
    memcpy(buffer, string, length);

    /* Convert */
#ifdef __32BIT__
    rxinteger l = strtol(buffer, &end, 10);
#else
    rxinteger l = strtoll(buffer, &end, 10);
#endif

    /* Convert error */
    if (errno == ERANGE || end == buffer) {
        rc = 1;
        goto end_string2integer;
    }

    /* Check only trailing spaces */
    while (*end != 0) {
        if (!isspace(*end)) {
            rc = 1;
            goto end_string2integer;
        }
        end++;
    }

    /* All good */
    *out = l;

    end_string2integer:
    free(buffer);
    return rc;
}

/* Convert a string to a float - returns 1 on error */
RX_INLINE int string2float(double *out, char *string, size_t length) {
    char *buffer = malloc(length + 1);
    char *end = buffer;
    int rc = 0;
    errno = 0;

    /* Null terminated buffer */
    buffer[length] = 0;
    memcpy(buffer, string, length);

    /* Convert */
    double l = strtod(buffer, &end);

    /* Convert error */
    if (errno == ERANGE || end == buffer) {
        rc = 1;
        goto end_string2float;
    }

    /* Check only trailing spaces */
    while (*end != 0) {
        if (!isspace(*end)) {
            rc = 1;
            goto end_string2float;
        }
        end++;
    }

    /* All good */
    *out = l;

    end_string2float:
    free(buffer);
    return rc;
}

/* Convert a string to a decimal - returns 1 on error
 * Validated the string is a valid decimal number
 * and returns the decimal as a malloced string in out.
 * The out must be freed by the caller
 * Returns 0 on success, 1 on error.
 */
RX_INLINE int stringtodecimal(char **out, char *string, size_t length) {
    // Note that decimal can have a large number of digits
    // First validate the string is a valid decimal number by checking each character
    // Skip leading spaces
    *out = NULL; // Set output to NULL (used on an error)
    int i;

    if (length == 0) {
        // Empty string is not a valid decimal
        return 1; // Error
    }
    // Skip leading spaces
    for (i = 0; i < length; i++) {
        if (!isspace(string[i])) break;
    }
    int start = i;
    // Check for a sign
    if (string[i] == '+') {
        i++;
        start = i;
    }
    else if (string[i] == '-') {
        i++;
    }
    // Check for digits before the decimal point
    int has_digits = 0;
    while (i < length && isdigit(string[i])) {
        has_digits = 1;
        i++;
    }
    // Check for a decimal point
    if (i < length && string[i] == '.') {
        i++;
        // Check for digits after the decimal point
        while (i < length && isdigit(string[i])) {
            has_digits = 1;
            i++;
        }
    }
    // Check for a trailing exponent
    if (i < length && (string[i] == 'e' || string[i] == 'E')) {
        i++;
        // Check for an optional sign
        if (i < length && (string[i] == '+' || string[i] == '-')) {
            i++;
        }
        // Check for digits in the exponent
        if (i < length && isdigit(string[i])) {
            while (i < length && isdigit(string[i])) {
                i++;
            }
        }
        else {
            // No digits in exponent
            return 1; // Error
        }
    }
    int end = i; // Mark the end of the valid decimal part

    // Check for a trailing 'd' which we use as a marker for a decimal literal
    if (i < length && string[i] == 'd') {
        i++;
    }

    // Skip trailing spaces
    while (i < length && isspace(string[i])) {
        i++;
    }

    // If we reached the end of the string and we have digits, we have a valid decimal
    if (i == length && has_digits) {
        // Allocate memory for the decimal string
        *out = malloc(end - start + 1);
        if (*out == NULL) {
            // Error allocating memory - PANIC and exit with error
            fprintf(stderr, "Memory allocation error in stringtodecimal\n");
            exit(EXIT_FAILURE);
        }
        // Copy the valid decimal part to the output
        memcpy(*out, string + start, end - start);
        (*out)[end - start] = '\0'; // Null-terminate the string
        return 0; // Success
    }
    // If we reach here, the string is not a valid decimal
    return 1; // Error
}

// Static function to trim trailing zeros from a number format and including possibly the decimal point
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

// Function to extract decimal components from a double
// - coefficient (string) will be set to the coefficient string (or nan, inf, -inf)
// - exponent (integer) will be set to the exponent
static void extract_double_decimal(numeric_context* num_context, value *coefficient, value *exponent, double value) {

    size_t digits = num_context->digits;
    if (digits < DIGITS_MINIMUM) digits = DIGITS_MINIMUM;
    else if (digits > DBL_DIG) digits = DBL_DIG;

    // Set Buffer Size in Coefficient Value
    prep_string_buffer(coefficient, digits + 5); // +5 for sign, decimal point, possible rounding digit and null terminator
    exponent->int_value = 0;

    // Handle special cases
    if (isnan(value)) {
        if (num_context->casetype == CASE_UPPER)
            strcpy(coefficient->string_value, "NAN");
        else
            strcpy(coefficient->string_value, "nan");
        coefficient->string_length = 3;
        coefficient->string_pos = 0;
#ifndef NUTF8
        coefficient->string_chars = coefficient->string_length;
        coefficient->string_char_pos = 0;
#endif
        return;
    }
    if (isinf(value)) {
        if (num_context->casetype == CASE_UPPER)
            strcpy(coefficient->string_value, signbit(value) ? "-INF" : "INF");
        else
            strcpy(coefficient->string_value, signbit(value) ? "-inf" : "inf");
        coefficient->string_length = signbit(value) ? 4 : 3;
        coefficient->string_pos = 0;
#ifndef NUTF8
        coefficient->string_chars = coefficient->string_length;
        coefficient->string_char_pos = 0;
#endif
        return;
    }

    if (value == 0.0) {
        // Handle zero
        strcpy(coefficient->string_value, "0");
        coefficient->string_length = 1;
        coefficient->string_pos = 0;
#ifndef NUTF8
        coefficient->string_chars = coefficient->string_length;
        coefficient->string_char_pos = 0;
#endif
        return;
    }

    // Determine if negative
    int is_negative = value < 0.0;
    double abs_value = fabs(value);

    // Calculate decimal exponent
    int64_t exp = (int64_t)floor(log10(abs_value));

    // Normalize the coefficient to [1.0, 10.0)
    double coeff = abs_value / pow(10.0, (double)exp);

    // Adjust if coeff is exactly 10.0 due to floating-point inaccuracies
    if (coeff >= 10.0) {
        coeff /= 10.0;
        exp += 1;
    }

    // Adjust if coeff is smaller than 1 due to floating-point inaccuracies
    if (coeff < 1.0) {
        coeff *= 10.0;
        exp -= 1;
    }

    exponent->int_value = exp;

    // Format the coefficient string with precision up to DBL_DIG-1 fractional digits
    snprintf(coefficient->string_value, digits + 5, is_negative ? "-%.*lf" : "%.*lf", (int)(digits - 1), coeff);

    // Logic to [re-]check for edge case where rounding the coefficient could change the exponent - we look at the string
    char* abs_start = is_negative ? coefficient->string_value + 1 : coefficient->string_value;

    // If the coefficient starts with "10." it means rounding has caused it to become >= 10.0 so we need to adjust
    if (strncmp(abs_start, "10.", 3) == 0) {
        // Adjust coefficient and exponent but moving the decimal point left
        abs_start[2] = abs_start[1]; // Move the '0' to replace the '.'
        abs_start[1] = '.';          // Put the decimal point after the '1'
        (exponent->int_value)++;     // Increment the exponent
    }
    else if (strncmp(abs_start, "0.", 2) == 0) {
        // If the coefficient starts with "0." it means rounding has caused it to become < 1.0 so we need to adjust
        if (abs_start[2] != '\0') { // Just in case of a malformed string
            // Adjust coefficient and exponent by moving the decimal point right
            abs_start[0] = abs_start[2]; // Move the first digit after the '.' to the front
            // Shift the rest of the string left
            memmove(abs_start + 2, abs_start + 3, strlen(abs_start + 3) + 1);
            (exponent->int_value)--;                 // Decrement the exponent
        }
    }

    trim_numeric_trailing_zeros(coefficient->string_value);
    coefficient->string_length = strlen(coefficient->string_value);
    coefficient->string_pos = 0;
#ifndef NUTF8
    coefficient->string_chars = coefficient->string_length;
    coefficient->string_char_pos = 0;
#endif
}

/* Calculate the number of digits in an integer, including the sign if negative */
static size_t number_of_digits(rxinteger n) {
    if (n == 0) {
        return 1;
    }
    size_t digits = 0;

    // By using an unsigned type, we can safely represent the absolute value
    unsigned long long num;

    if (n < 0) {
        num = -(unsigned long long)n;
        digits = 1; // For the negative sign
    } else {
        num = n;
    }

    while (num > 0) {
        num /= 10;
        digits++;
    }

    return digits;
}

/*
 * Converts a numeric value represented by a coefficient and exponent into a formatted string.
 * The formatting can be either scientific or engineering, and the case of the exponent can be upper
 * or lower.
 * - the coefficient is a string representing the normalized number (e.g., "-1.2345").
 * - the exponent is an integer representing the power of ten.
 *
 * In the num_conntext:
 * - the digits parameter specifies the total number of significant digits to consider.
 * - the form parameter specifies whether to use scientific or engineering notation.
 * - the casetype parameter specifies whether the exponent should be in upper or lower case.
 *
 * The formatted output is written to the formatted_output buffer.
 *
 * The function handles special cases like zero, NaN, and infinity.
 */
static void RexxDecimalFormat(numeric_context* num_context, value *coefficient_value, value *exponent_value, value *formatted_output_value) {

    const char *coef_start;
    size_t digits_in_coef;
    int use_exponential;
    int is_engineering;
    size_t i;
    const char *scientific_format;
    const char *engineering_format;
    char *coefficient = coefficient_value->string_value;
    coefficient[coefficient_value->string_length] = 0; // Null-terminate - just in case
    rxinteger exponent = exponent_value->int_value;

    /* Prepare the output buffer */
    // Calculate the output buffer size which is based on the number of digits and the exponent size from the arguments
    size_t output_buffer_size = coefficient_value->string_length + number_of_digits(exponent) + 5; // +5 for sign, decimal point, 'e' and null terminator
    prep_string_buffer(formatted_output_value, output_buffer_size);
    formatted_output_value->string_length = 0;
    formatted_output_value->string_value[0] = 0; // Null-terminate - just in case
    formatted_output_value->string_pos = 0;
#ifndef NUTF8
    formatted_output_value->string_chars = formatted_output_value->string_length;
    formatted_output_value->string_char_pos = 0;
#endif
    char *formatted_output = formatted_output_value->string_value;

    /* Case specific Formats */
    if (num_context->casetype == CASE_UPPER) {
        scientific_format = "%sE%+lld";
        engineering_format = "E%+lld";
    }
    else {
        scientific_format = "%se%+lld";
        engineering_format = "e%+lld";
    }

    // If exponent is 0, we can use the simple format directly
    if (exponent == 0) {
        strcpy(formatted_output, coefficient); // This also handles 0, nan, inf
        // convert case if the first character isn't a digit or '-'

        /* Detecting a number - funny logic, but we have "-inf" and so on to handle, and it is a normalised x.xxx */
        /* So, is it a one-digit number, if not check the second character for a decimal point */
        if (formatted_output[1] != 0 && formatted_output[1] != '.' ) {
            /* If not a number (e.g. nan, inf, -inf), convert case */
            if (num_context->casetype == CASE_UPPER) {
                for (i = 0; formatted_output[i] != 0; i++)
                    formatted_output[i] = (char)toupper((unsigned char)formatted_output[i]);
            }
            else {
                for (i = 0; formatted_output[i] != 0; i++)
                    formatted_output[i] = (char)tolower((unsigned char)formatted_output[i]);
            }
        }
        formatted_output_value->string_length = strlen(formatted_output_value->string_value);
        formatted_output_value->string_pos = 0;
#ifndef NUTF8
        formatted_output_value->string_chars = formatted_output_value->string_length;
        formatted_output_value->string_char_pos = 0;
#endif
        return;
    }

    // Apply the REXX rule to decide on simple vs. exponential format
    // See ANSI REXX standard, 7.4.10, Floating() routine
    use_exponential = (exponent + 1 > (rxinteger)(num_context->digits)) || (exponent < -6);
    if (use_exponential) {
        is_engineering = (num_context->form == NUMERIC_FORM_ENGINEERING);
        if (!is_engineering) {
            // --- SCIENTIFIC NOTATION ---
            // The number has already been formatted in scientific notation by decimalExtract
            sprintf(formatted_output, scientific_format, coefficient, exponent);
            formatted_output_value->string_length = strlen(formatted_output_value->string_value);
            formatted_output_value->string_pos = 0;
#ifndef NUTF8
            formatted_output_value->string_chars = formatted_output_value->string_length;
            formatted_output_value->string_char_pos = 0;
#endif
            return;
        }
    }

    // We will need to process the coefficient for simple format or engineering format

    // Remove the negative and decimal point from the coefficient, it has been normalised to [-]x[.xxxxx]
    // Add the negative to the string output
    if (coefficient[0] == '-') {
        coef_start = coefficient + 1;
        *formatted_output++ = '-';
    } else {
        coef_start = coefficient;
    }
    digits_in_coef = strlen(coef_start);
    if (digits_in_coef > 1) {
        // More than one digit - remove the decimal point
        digits_in_coef--;
    }

    if (!use_exponential) {
        // --- SIMPLE FORMAT ---
        if (exponent > 0) { // Note: exponent == 0 is handled above
            // Positive exponent
            if (exponent < digits_in_coef) {
                // Insert a decimal point within the coefficient
                // Copy up to the position of the decimal point
                // First digit
                *formatted_output++ = coef_start[0];
                // Remaining digits before the new decimal point
                if (coef_start[1] != 0) {
                    strncpy(formatted_output, coef_start + 2, exponent);
                    formatted_output += (exponent);
                }
                // Insert the decimal point - if we have more digits
                if (coef_start[exponent + 2] != 0) {
                    *formatted_output++ = '.';
                    // Copy the rest of the digits after the decimal point
                    strcpy(formatted_output, coef_start + exponent + 2);
                }
                else {
                    formatted_output[0] = 0; // Null-terminate
                }
            } else {
                // Append zeros to the end
                // Copy the coefficient
                // First digit
                formatted_output[0] = coef_start[0];
                // Remaining digits after the decimal point (if any)
                if (coef_start[1] != 0) {
                    strcpy(formatted_output + 1, coef_start + 2);
                }
                // Append zeros
                for (i = 0; i < exponent - digits_in_coef + 1; i++) {
                    formatted_output[i + digits_in_coef] = '0';
                }
                formatted_output[i + digits_in_coef] = 0; // Null-terminate
            }
        }
        else {
            // Negative exponent
            strcpy(formatted_output, "0.");
            for (i = 0; i < -exponent - 1; i++) {
                formatted_output[i + 2] = '0';
            }
            // Copy the coefficient after the leading zeros - first digit
            formatted_output[-exponent + 1] = coef_start[0];
            // Remaining digits after the decimal point (if any)
            for (i = 1; i < digits_in_coef; i++) {
                formatted_output[-exponent + 1 + i] = coef_start[i + 1];
            }
            formatted_output[-exponent + 1 + digits_in_coef] = 0;
        }
        formatted_output_value->string_length = strlen(formatted_output_value->string_value);
        formatted_output_value->string_pos = 0;
#ifndef NUTF8
            formatted_output_value->string_chars = formatted_output_value->string_length;
            formatted_output_value->string_char_pos = 0;
#endif
        return;
    }

    // --- EXPONENTIAL FORMAT ---
    // ENGINEERING form: exponent must be a multiple of 3; 1..3 digits before the decimal point.
    // Adjust the exponent to a multiple of 3 using a non-negative remainde
    int rem = (int)(exponent % 3);
    int k = (rem + 3) % 3;          // how many places to shift the decimal point to the RIGHT
    rxinteger eng_exp = exponent - k;

    // Build the engineering mantissa by inserting the decimal point after (k+1) digits.
    size_t need_int = (size_t)k + 1;

    if (digits_in_coef <= need_int) {

        // Not enough digits for a fractional part: pad with zeros up to need_int
        // Copy first digit
        formatted_output[0] = coef_start[0];
        // Copy remaining digits in the coefficient (if any)
        if (coef_start[1] != 0) {
            strncpy(formatted_output + 1, coef_start + 2, digits_in_coef - 1);
        }
        // Pad with zeros
        for (i = digits_in_coef; i < need_int; ++i) {
            formatted_output[i] = '0';
        }
        // Move output pointed to the end of the integer part
        formatted_output += need_int;

    } else {

        // We have more than (k+1) digits: insert decimal point
        // Copy the first digit
        formatted_output[0] = coef_start[0];
        // Copy the next (need_int - 1) digits (skipping the old decimal point)
        memcpy(formatted_output + 1, coef_start + 2, need_int - 1);
        formatted_output[need_int] = '.';
        // Copy the rest of the digits after the decimal point
        for (i = 0; i < digits_in_coef - need_int; ++i) {
            formatted_output[need_int + 1 + i] = coef_start[need_int + 1 + i];
        }
        // Move output pointed to the end of the integer part
        formatted_output += 1 + digits_in_coef;

    }

    // Add the exponent
    sprintf(formatted_output, engineering_format, eng_exp);
    formatted_output_value->string_length = strlen(formatted_output_value->string_value);
    formatted_output_value->string_pos = 0;
#ifndef NUTF8
    formatted_output_value->string_chars = formatted_output_value->string_length;
    formatted_output_value->string_char_pos = 0;
#endif
}

// Function to extract decimal components from an integer
// - coefficient (string) will be set to the coefficient string (or nan, inf, -inf)
// - exponent (integer) will be set to the exponent
static void extract_integer_decimal(numeric_context* num_context, value *coefficient, value *exponent, rxinteger value) {

    // Handle special case of zero
    if (value == 0) {
        strcpy(coefficient->string_value, "0");
        coefficient->string_length = 1;
        coefficient->string_pos = 0;
#ifndef NUTF8
        coefficient->string_char_pos = 0;
        coefficient->string_chars = 1;
#endif
        exponent->int_value = 0;
        return;
    }

    prep_string_buffer(coefficient, SMALLEST_STRING_BUFFER_LENGTH); // Large enough for an int
#ifdef __32BIT__
    coefficient->string_length = snprintf(coefficient->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%ld",(long)value);
#else
    coefficient->string_length = snprintf(coefficient->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%lld",(long long)value);
#endif
    coefficient->string_pos = 0;

    // We are converting to coefficient, as an example, from 123456 to 1.23456 (i.e. normalised scientific notation)
    if (value > 0) {
        // Positive number logic

        // Calculate the exponent based on the number of digits in the integer
        // The exponent is the number of digits - 1
        exponent->int_value = (rxinteger)(coefficient->string_length - 1);

        // Insert the decimal point after the first digit if there are more than 1 digit
        if (coefficient->string_length > 1) {
            // Shift the string to the right to make space for the decimal point
            memmove(coefficient->string_value + 2, coefficient->string_value + 1, coefficient->string_length);
            coefficient->string_value[1] = '.';
            coefficient->string_length++;
            coefficient->string_value[coefficient->string_length] = 0; // Null-terminate
        }
    }
    else {
        // Negative number logic

        // Calculate the exponent based on the number of digits in the integer
        // The exponent is the number of digits - 2 (to account for the '-' sign)
        exponent->int_value = (rxinteger)(coefficient->string_length - 2);

        // Insert the decimal point after the first digit following the '-' sign if there are more than 2 characters
        if (coefficient->string_length > 2) {
            // Shift the string to the right to make space for the decimal point
            memmove(coefficient->string_value + 3, coefficient->string_value + 2, coefficient->string_length - 1);
            coefficient->string_value[2] = '.';
            coefficient->string_length++;
            coefficient->string_value[coefficient->string_length] = 0; // Null-terminate
        }
    }

    // Set the utf8 values
#ifndef NUTF8
    coefficient->string_char_pos = 0;
    coefficient->string_chars = coefficient->string_length;
#endif
}

#endif //CREXX_RXVMVARS_H
