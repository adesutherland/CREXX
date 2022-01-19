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

/* value factory - multiple/array of values created */
/*
static RX_INLINE void array_value_f(value **array, int num, void* parent) {
    int i;
    value* this = malloc(num * sizeof(value));
    for (i=0; i<num; i++) {
        this[i].owner = parent;
        this[i].string_value = 0;
        array[i] = &(this[i]);
    }
}
*/

/* value factories */
static RX_INLINE value* value_f(void* parent, value **free_list) {
    value* this;
    if (*free_list) {
        this = *free_list;
        *free_list = this->prev_free;
    }
    else {
        this = malloc(sizeof(value));
        this->string_value = 0;
    }
    this->owner = parent;
    return this;
}

/* value factories - int value */
static RX_INLINE value* value_int_f(void* parent, rxinteger initial_value,
                                    value **free_list) {
    value* this = value_f(parent, free_list);
    this->int_value = initial_value;
    return this;
}

static RX_INLINE value* value_float_f(void* parent, double initial_value,
                                      value **free_list) {
    value* this = value_f(parent, free_list);
    this->float_value = initial_value;
    return this;
}

/*
 * Returns required buffer size - the smallest power of two that's greater or
 * equal to a given value
 */
static RX_INLINE size_t buffer_size(size_t value) {
    size_t i;
    if (value <= SMALLEST_STRING_BUFFER_LENGTH)
        return SMALLEST_STRING_BUFFER_LENGTH;

    --value;
    for(i = 1; i < sizeof(size_t); i*=2)
        value |= value >> i;
    return value+1;
}

static RX_INLINE void prep_string_buffer(value *v, size_t length) {
    v->string_length = length;
    if (v->string_value) {
        if (v->string_length > v->string_buffer_length) {
            free(v->string_value);
            v->string_buffer_length = buffer_size(v->string_length);
            v->string_value = malloc(v->string_buffer_length);
        }
    }
    else {
        v->string_buffer_length = buffer_size(v->string_length);
        v->string_value = malloc(v->string_buffer_length);
    }
}

/* value factories - constant string value */
static RX_INLINE value* value_conststring_f(void* parent, string_constant *initial_value,
                                            value **free_list) {
    value* this = value_f(parent, free_list);
    this->string_length = initial_value->string_len;
    prep_string_buffer(this, this->string_length);
    memcpy(this->string_value, initial_value->string,  this->string_length);
    this->string_pos = 0;
#ifndef NUTF8
    this->string_char_pos = 0;
    this->string_chars = initial_value->string_chars;
#endif
    return this;
}

/* value factories - null string value */
static RX_INLINE value* value_nullstring_f(void* parent, char *initial_value,
                                           value **free_list) {
    value* this = value_f(parent, free_list);
    this->string_length = strlen(initial_value);
    prep_string_buffer(this, this->string_length);
    memcpy(this->string_value, initial_value,  this->string_length);
    this->string_pos = 0;
#ifndef NUTF8
    this->string_char_pos = 0;
    this->string_chars = utf8nlen(this->string_value, this->string_length);
#endif
    return this;
}

/* Clears a register */
static RX_INLINE void clear_reg(value* reg) {
    void* parent = reg->owner;
    if (reg->string_value) free(reg->string_value);
    memset(reg, 0, sizeof(value));
    reg->owner = parent;
    return;
}

static RX_INLINE void free_value(void* parent, value *v, value **free_list) {
    if (v && v->owner == parent) {
        v->prev_free = *free_list;
        *free_list = v;
    }
}

static RX_INLINE void remove_free_values(value **free_list) {
    value *v;
    while (*free_list) {
        v = *free_list;
        *free_list = v->prev_free;
        if (v->string_value) free(v->string_value);
        free(v);
    }
}

/* Int Flag */
static RX_INLINE void set_type_int(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_int = 1;
}

static RX_INLINE void add_type_int(value *v) {
    v->status.type_int = 1;
}

static RX_INLINE unsigned int get_type_int(value *v) {
    return v->status.type_int;
}

/* Float Flag */
static RX_INLINE void set_type_float(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_float = 1;
}

static RX_INLINE void add_type_float(value *v) {
    v->status.type_float = 1;
}

static RX_INLINE unsigned int get_type_float(value *v) {
    return v->status.type_float;
}

/* Decimal Flag */
static RX_INLINE void set_type_decimal(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_decimal = 1;
}

static RX_INLINE void add_type_decimal(value *v) {
    v->status.type_decimal = 1;
}

static RX_INLINE unsigned int get_type_decimal(value *v) {
    return v->status.type_decimal;
}

/* String Flag */
static RX_INLINE void set_type_string(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_string = 1;
}

static RX_INLINE void add_type_string(value *v) {
    v->status.type_string = 1;
}

static RX_INLINE unsigned int get_type_string(value *v) {
    return v->status.type_string;
}

/* Object Flag */
static RX_INLINE void set_type_object(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_object = 1;
}

static RX_INLINE void add_type_object(value *v) {
    v->status.type_object = 1;
}

static RX_INLINE unsigned int get_type_object(value *v) {
    return v->status.type_object;
}

/* Unset Flag */
static RX_INLINE void unset_type(value *v) {
    v->status.all_type_flags = 0;
}

static RX_INLINE void set_int(value *v, rxinteger value) {
    v->int_value = value;
}
static RX_INLINE void set_float(value *v, double value) {
    v->float_value = value;
}

static RX_INLINE void extend_string_buffer(value *v, size_t length) {
    v->string_length = length;
    if (v->string_value) {
        if (v->string_length > v->string_buffer_length) {
            v->string_buffer_length = buffer_size(v->string_length);
            v->string_value = realloc(v->string_value, v->string_buffer_length);
        }
    }
    else {
        v->string_buffer_length = buffer_size(v->string_length);
        v->string_value = malloc(v->string_buffer_length);
    }
}

static RX_INLINE void set_string(value *v, char *value, size_t length) {
    prep_string_buffer(v,length);
    memcpy(v->string_value, value, v->string_length);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = utf8nlen(v->string_value, v->string_length); /* SLOW! */
#endif
}

static RX_INLINE void set_const_string(value *v, string_constant *from) {
    prep_string_buffer(v,from->string_len);
    memcpy(v->string_value, from->string, v->string_length);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = from->string_chars;
#endif
}

static RX_INLINE void set_value_string(value *v, value *from) {
    prep_string_buffer(v, from->string_length);
    memcpy(v->string_value, from->string_value, v->string_length);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = from->string_chars;
#endif
}

static RX_INLINE void set_buffer_string(value *v, char *buffer, size_t length, size_t buffer_length
#ifndef NUTF8
  , size_t string_chars
#endif
) {
    if (v->string_value) free(v->string_value);
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
static RX_INLINE void copy_value(value *dest, value *source) {
    dest->status.all_type_flags = source->status.all_type_flags;
    dest->int_value = source->int_value;
    dest->float_value = source->float_value;
    dest->decimal_value = source->decimal_value;
    dest->string_pos = source->string_pos;
    dest->string_length = source->string_length;
#ifndef NUTF8
    dest->string_chars = source->string_chars;
    dest->string_char_pos = source->string_char_pos;
#endif
    dest->object_value = source->object_value; /* TODO */
    /* Copy String Data */
    if (dest->string_length) {
        prep_string_buffer(dest, dest->string_length);
        memcpy(dest->string_value, source->string_value, dest->string_length);
    }
    else dest->string_value = 0;
}

/* Copy string value */
static RX_INLINE void copy_string_value(value *dest, value *source) {
    dest->string_pos = source->string_pos;
    dest->string_length = source->string_length;
#ifndef NUTF8
    dest->string_chars = source->string_chars;
    dest->string_char_pos = source->string_char_pos;
#endif
    if (dest->string_length) {
        prep_string_buffer(dest, dest->string_length);
        memcpy(dest->string_value, source->string_value, dest->string_length);
    }
    else dest->string_value = 0;
}

/* Compares two strings. returns -1, 0, 1 as appropriate */
#define MIN(a,b) (((a)<(b))?(a):(b))
static RX_INLINE int string_cmp(char *value1, size_t length1, char *value2, size_t length2) {
    rxinteger idiff;

    if ((idiff = memcmp(value1, value2,MIN(length1, length2)) != 0))
        return (int)idiff;

    idiff = (rxinteger)length1 - (rxinteger)length2;
    return idiff>0 ? 1 : (idiff<0 ? -1 : 0);
}

static RX_INLINE int string_cmp_value(value *v1, value *v2) {
    return string_cmp(v1->string_value, v1->string_length,
                      v2->string_value, v2->string_length);
}

static RX_INLINE int string_cmp_const(value *v1, string_constant *v2) {
    return string_cmp(v1->string_value, v1->string_length,
                      v2->string, v2->string_len);
}

static RX_INLINE void string_append(value *v1, value *v2) {
    size_t start = v1->string_length;

    extend_string_buffer(v1, v1->string_length + v2->string_length);
    memcpy(v1->string_value + start, v2->string_value, v2->string_length);
    v1->string_pos = 0;
#ifndef NUTF8
    v1->string_char_pos = 0;
    v1->string_chars += v2->string_chars;
#endif
}

static RX_INLINE void string_sappend(value *v1, value *v2) {
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

static RX_INLINE void string_concat(value *v1, value *v2, value *v3) {
    size_t len = v2->string_length + v3->string_length ;
    size_t buffer_len = buffer_size(len);
    char *buffer = malloc(buffer_len);

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

static RX_INLINE void string_sconcat(value *v1, value *v2, value *v3) {
    size_t len = v2->string_length + v3->string_length + 1;
    size_t buffer_len = buffer_size(len);
    char *buffer = malloc(buffer_len);

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

static RX_INLINE void string_concat_var_const(value *v1, value *v2, string_constant *v3) {
    size_t len = v2->string_length + v3->string_len;
    size_t buffer_len = buffer_size(len);
    char *buffer = malloc(buffer_len);

    memcpy(buffer, v2->string_value, v2->string_length);
    memcpy(buffer + v2->string_length, v3->string, v3->string_len);
    v1->string_pos = 0;

#ifdef NUTF8
    set_buffer_string(v1, buffer, len, buffer_len);
#else
    set_buffer_string(v1, buffer, len, buffer_len, v2->string_chars + v3->string_chars);
    v1->string_char_pos = 0;
#endif
}

static RX_INLINE void string_sconcat_var_const(value *v1, value *v2, string_constant *v3) {
    size_t len = v2->string_length + v3->string_len + 1;
    size_t buffer_len = buffer_size(len);
    char *buffer = malloc(buffer_len);

    memcpy(buffer, v2->string_value, v2->string_length);
    buffer[v2->string_length] = ' ';
    memcpy(buffer + v2->string_length + 1, v3->string, v3->string_len);
    v1->string_pos = 0;

#ifdef NUTF8
    set_buffer_string(v1, buffer, len, buffer_len);
#else
    set_buffer_string(v1, buffer, len, buffer_len, v2->string_chars + v3->string_chars + 1);
    v1->string_char_pos = 0;
#endif
}

static RX_INLINE void string_concat_const_var(value *v1, string_constant *v2, value *v3) {
    size_t len = v2->string_len + v3->string_length;
    size_t buffer_len = buffer_size(len);
    char *buffer = malloc(buffer_len);

    memcpy(buffer, v2->string, v2->string_len);
    memcpy(buffer + v2->string_len, v3->string_value, v3->string_length);
    v1->string_pos = 0;

#ifdef NUTF8
    set_buffer_string(v1, buffer, len, buffer_len);
#else
    set_buffer_string(v1, buffer, len, buffer_len, v2->string_chars + v3->string_chars);
    v1->string_char_pos = 0;
#endif
}

static RX_INLINE void string_sconcat_const_var(value *v1, string_constant *v2, value *v3) {
    size_t len = v2->string_len + v3->string_length + 1;
    size_t buffer_len = buffer_size(len);
    char *buffer = malloc(buffer_len);

    memcpy(buffer, v2->string, v2->string_len);
    buffer[v2->string_len] = ' ';
    memcpy(buffer + v2->string_len + 1, v3->string_value, v3->string_length);
    v1->string_pos = 0;

#ifdef NUTF8
    set_buffer_string(v1, buffer, len, buffer_len);
#else
    set_buffer_string(v1, buffer, len, buffer_len, v2->string_chars + v3->string_chars + 1);
    v1->string_char_pos = 0;
#endif
}

#ifndef NUTF8
/* This sets v's string_pos (the byte index) and v's string_char_pos
 * (the utf8 codepoint index) based on a new string_char_pos */
static RX_INLINE void string_set_byte_pos(value *v, size_t new_string_char_pos) {
    assert (v->string_char_pos < v->string_chars);
    /* We need to walk through the UTF8 characters until we get to
     * the required string_char_pos and then we will know the corresponding
     * string_pos. But we need to work out the best starting point for the walk:
     * - The start of the string,
     * - The end of the string, or
     * - from the existing string_pos
     */
    size_t byte_from;
    size_t char_from;
    size_t gap;
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

static RX_INLINE void string_concat_char(value *v1, value *v2) {
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
static RX_INLINE void string_from_int(value *v) {
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
static RX_INLINE void string_from_float(value *v) {
    prep_string_buffer(v, SMALLEST_STRING_BUFFER_LENGTH); // Large enough for a float
    v->string_length = snprintf(v->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%g",v->float_value);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = v->string_length;
#endif
}

/* Calculate the integer value from float */
static RX_INLINE void int_from_float(value *v) {
    v->int_value = floor(v->float_value);
    if (v->float_value - (double)v->int_value > 0.5) v->int_value++;
}

/* Convert a string to an integer - returns 1 on error */
static RX_INLINE int string2integer(rxinteger *out, char *string, size_t length) {
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
static RX_INLINE int string2float(double *out, char *string, size_t length) {
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
