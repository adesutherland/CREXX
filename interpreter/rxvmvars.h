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

/* Zeros a register value */
RX_INLINE void value_zero(value *v) {
    v->status.all_type_flags = 0;
    v->int_value = 0;
    v->float_value = 0;
    v->decimal_value = 0;
    v->string_length = 0;
    v->string_pos = 0;
#ifndef NUTF8
    v->string_chars = 0;
    v->string_char_pos = 0;
#endif
    v->num_attributes = 0;

    /* Free binary */
    if (v->binary_value) free(v->binary_value);
    v->binary_value = 0;
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
    v->binary_length = 0;
    value_zero(v);
}

/* Value Factory mallocs and inits */
RX_INLINE value* value_f() {
    value* this;
    this = malloc(sizeof(value));
    value_init(this);
    return this;
}

/* Sets up the required number of attributes */
RX_INLINE void set_num_attributes(value* v, size_t num) {
    size_t i;
    value *a;

    if (num <= v->num_attributes) {
        /* Reducing the number of attributes is easy */
        v->num_attributes = num;
    }

    else if (num <= v->max_num_attributes) {
        /* Just need to reset the recycled attributes */
        for (i = v->num_attributes; i < num; i++) {
            v->attributes[i] = v->unlinked_attributes[i]; /* Ensure Attribute is unlinked */
            value_zero(v->attributes[i]);
        }
        v->num_attributes = num;
    }

    else {
        /* We first need to recycle any unused attributes */
        for (i = v->num_attributes; i < v->max_num_attributes; i++) {
            v->attributes[i] = v->unlinked_attributes[i]; /* Ensure Attribute is unlinked */
            value_zero(v->attributes[i]);
        }

        /* Now we need to make the pointer arrays big enough */
        if (v->attributes) v->attributes = realloc(v->attributes, sizeof(value*) * num);
        else v->attributes = malloc(sizeof(value*) * num);

        if (v->unlinked_attributes) v->unlinked_attributes = realloc(v->unlinked_attributes, sizeof(value*) * num);
        else v->unlinked_attributes = malloc(sizeof(value*) * num);

        v->num_attribute_buffers++;
        if (v->attribute_buffers) v->attribute_buffers = realloc(v->attribute_buffers, sizeof(value*) * v->num_attribute_buffers);
        else v->attribute_buffers = malloc(sizeof(value*) * v->num_attribute_buffers);

        /* Create new buffer */
        v->attribute_buffers[v->num_attribute_buffers - 1] =
                malloc(sizeof(value) * (num - v->max_num_attributes));

        /* Initiate the new attributes */
        a = v->attribute_buffers[v->num_attribute_buffers - 1];
        for (i = v->max_num_attributes; i < num; i++, a++) {
            value_init(a);
            v->attributes[i] = a;
            v->unlinked_attributes[i] = a;
        }

        /* Set the new number of attributes */
        v->num_attributes = v->max_num_attributes = num;
    }
}

/*
 * Returns required buffer size - the smallest power of two that's greater or
 * equal to a given value
 */
RX_INLINE size_t buffer_size(size_t value) {
    size_t i;
    if (value <= SMALLEST_STRING_BUFFER_LENGTH)
        return SMALLEST_STRING_BUFFER_LENGTH;

    --value;
    for(i = 1; i < sizeof(size_t); i*=2)
        value |= value >> i;
    return value+1;
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
    for (i = 0; i < v->max_num_attributes; i++) clear_value(v->unlinked_attributes[i]);

    /* Free attribute buffer */
    for (i = 0; i < v->num_attribute_buffers; i++) free(v->attribute_buffers[i]);

    /* Free pointer arrays */
    if (v->attributes) free(v->attributes);
    if (v->unlinked_attributes) free(v->unlinked_attributes);
    if (v->attribute_buffers) free(v->attribute_buffers);

    /* Free strings */
    if (v->string_value != v->small_string_buffer) free(v->string_value);

    /* Free binary */
    if (v->binary_value) free(v->binary_value);
    v->binary_value = 0;
    v->binary_length = 0;
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
RX_INLINE void set_null_string(value *v, char *from) {
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
    dest->decimal_value = source->decimal_value;

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
    if (dest->binary_value) free(dest->binary_value);
    dest->binary_value = 0;
    dest->binary_length = 0;
    if (source->binary_value) {
        dest->binary_length = source->binary_length;
        dest->binary_value = malloc(dest->binary_length);
        memcpy(dest->binary_value, source->binary_value, dest->binary_length);
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
    dest->decimal_value = source->decimal_value;

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
    if (dest->binary_value) free(dest->binary_value);
    dest->binary_value = 0;
    dest->binary_length = 0;
    if (source->binary_value) {
        dest->binary_length = source->binary_length;
        dest->binary_value = source->binary_value;
        source->binary_value = 0;
        source->binary_length = 0;
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
    assert (v->string_char_pos < v->string_chars);
    /* We need to walk through the UTF8 characters until we get to
     * the required string_char_pos and then we will know the corresponding
     * string_pos. But we need to work out the best starting point for the walk:
     * - The start of the string,
     * - The end of the string, or
     * - from the existing string_pos
     */
    int diff;

    diff = (int)(new_string_char_pos - v->string_char_pos);

    if (diff == 1) {
        /* Optimised for the scenario where the user program is walking the string */
        v->string_pos += utf8codepointcalcsize(v->string_value + v->string_pos);
        v->string_char_pos++;
        return;
    }

    if (diff == -1) {
        /* Might be looping through the string backwards */
        v->string_pos -= utf8rcodepointcalcsize(v->string_value + v->string_pos);
        v->string_char_pos--;
        return;
    }

    if (diff > 1) {
        /* So the new position is bigger than the current position ... we need
         * to check if it is quicker to search from the end of the string */
        if (v->string_chars - 1 - new_string_char_pos < diff) {
            /* loop from the end */
            v->string_pos = v->string_length;
            v->string_char_pos = v->string_chars - 1;
            v->string_pos -= utf8rcodepointcalcsize(v->string_value + v->string_pos);
            while (v->string_char_pos != new_string_char_pos) {
                v->string_pos -= utf8rcodepointcalcsize(v->string_value + v->string_pos);
                v->string_char_pos--;
            }
            return;
        }
        /* loop from the current position */
        while (v->string_char_pos != new_string_char_pos) {
            v->string_pos += utf8codepointcalcsize(v->string_value + v->string_pos);
            v->string_char_pos++;
        }
        return;
    }

    if (diff < 1) {
        /* So the new position is smaller than the current position ... we need
         * to check if it is quicker to search from the beginning of the string */
        if (new_string_char_pos <= -diff) {
            /* loop from the beginning */
            v->string_pos = 0;
            v->string_char_pos = 0;
            while (v->string_char_pos != new_string_char_pos) {
                v->string_pos += utf8codepointcalcsize(v->string_value + v->string_pos);
                v->string_char_pos++;
            }
            return;
        }
        /* loop from the current position */
        while (v->string_char_pos != new_string_char_pos) {
            v->string_pos -= utf8rcodepointcalcsize(v->string_value + v->string_pos);
            v->string_char_pos--;
        }
        return;
    }

    /* If we reach here it means the current position was not changed - fine */
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

/* Calculate the string value */
RX_INLINE void string_from_int(value *v) {
    prep_string_buffer(v, SMALLEST_STRING_BUFFER_LENGTH); // Large enough for an int
#ifdef __32BIT__
    v->string_length = snprintf(v->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%ld",v->int_value);
#else
    v->string_length = snprintf(v->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%lld",v->int_value);
#endif
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = v->string_length;
#endif
}

/* Calculate the string value */
RX_INLINE void string_from_float(value *v) {
    prep_string_buffer(v, SMALLEST_STRING_BUFFER_LENGTH); // Large enough for a float
    v->string_length = snprintf(v->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%.15g",v->float_value);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = v->string_length;
#endif
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

#endif //CREXX_RXVMVARS_H
