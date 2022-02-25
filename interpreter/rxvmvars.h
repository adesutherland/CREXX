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
    v->object_value = 0;
}

/* Setup a new value structure */
RX_INLINE void value_init(value *v) {
    v->string_value = v->small_string_buffer;
    v->string_buffer_length = sizeof(v->small_string_buffer);
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

/* Clears a value of all its children and other malloced buffers */
RX_INLINE void clear_value(value* v) {
    if (v->string_value != v->small_string_buffer) free(v->string_value);
    return;
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
RX_INLINE void copy_value(value *dest, value *source) {
    dest->status.all_type_flags = source->status.all_type_flags;
    dest->int_value = source->int_value;
    dest->float_value = source->float_value;
    dest->decimal_value = source->decimal_value;
    dest->object_value = source->object_value; /* TODO */

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

/* Move a value */
RX_INLINE void move_value(value *dest, value *source) {
    if (dest == source) return;

    dest->status.all_type_flags = source->status.all_type_flags;
    dest->int_value = source->int_value;
    dest->float_value = source->float_value;
    dest->decimal_value = source->decimal_value;
    dest->object_value = source->object_value; /* TODO */

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
            if (dest->string_value != dest->small_string_buffer)
                free(dest->string_value);
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

    /* Reset source */
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
    rxinteger idiff;

    if ((idiff = memcmp(value1, value2,MIN(length1, length2)) != 0))
        return (int)idiff;

    idiff = (rxinteger)length1 - (rxinteger)length2;
    return idiff>0 ? 1 : (idiff<0 ? -1 : 0);
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
    v->string_length = snprintf(v->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%g",v->float_value);
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
