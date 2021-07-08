/* CREXX
 * RUNTIME Variable Support
 */

#ifndef CREXX_RXVMVARS_H
#define CREXX_RXVMVARS_H

#ifndef NUTF8
#include "utf.h"
#endif

#define SMALLEST_STRING_BUFFER_LENGTH 32

typedef struct value value;

typedef union {
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
    void *decimal_value; /* TODO */
    char *string_value;
    size_t string_length;
    size_t string_buffer_length;
    size_t string_pos;
#ifndef NUTF8
    size_t string_chars;
    size_t string_char_pos;
#endif
    void *object_value;

    /*
     * Each value can either be owned by a stack frame or a variable pool
     * The owner is responsible for freeing the value, but because value can
     * be linked other stack frames and variable pools it is important that
     * none of these free its memory when they are being freed themselves.
     * The owner member is only used by parents to see if they are the real
     * parent - if you like a paternity test!
     * This also allows a value to be adopted by another parent (e.g. for a
     * returned register from a procedure)
     */
    void *owner;
};

/* value factories */
static value* value_f(void* parent) {
    value* this = calloc(1,sizeof(value)); /* Zeros data */
    this->owner = parent;
    return this;
}

/* value factories - int value */
static value* value_int_f(void* parent, rxinteger initial_value) {
    value* this = calloc(1,sizeof(value)); /* Zeros data */
    this->owner = parent;
    this->int_value = initial_value;
    return this;
}

static value* value_float_f(void* parent, double initial_value) {
    value* this = calloc(1,sizeof(value)); /* Zeros data */
    this->owner = parent;
    this->float_value = initial_value;
    return this;
}

/*
 * Returns required buffer size - the smallest power of two that's greater or
 * equal to a given value
 */
static size_t buffer_size(size_t value) {
    size_t i;
    if (value <= SMALLEST_STRING_BUFFER_LENGTH)
        return SMALLEST_STRING_BUFFER_LENGTH;

    --value;
    for(i = 1; i < sizeof(size_t); i*=2)
        value |= value >> i;
    return value+1;
}

/* value factories - constant string value */
static value* value_conststring_f(void* parent, string_constant *initial_value) {
    value* this = calloc(1,sizeof(value)); /* Zeros data */
    this->owner = parent;
    this->string_length = initial_value->string_len;
    this->string_buffer_length = buffer_size(this->string_length);
    this->string_value = malloc(this->string_buffer_length);
    memcpy(this->string_value, initial_value->string,  this->string_length);
    this->string_pos = 0;
#ifndef NUTF8
    this->string_char_pos = 0;
    this->string_chars = utf8nlen(this->string_value, this->string_length);
#endif
    return this;
}

/* value factories - null string value */
static value* value_nullstring_f(void* parent, char *initial_value) {
    value* this = calloc(1,sizeof(value)); /* Zeros data */
    this->owner = parent;
    this->string_length = strlen(initial_value);
    this->string_buffer_length = buffer_size(this->string_length);
    this->string_value = malloc(this->string_buffer_length);
    memcpy(this->string_value, initial_value,  this->string_length);
    this->string_pos = 0;
#ifndef NUTF8
    this->string_char_pos = 0;
    this->string_chars = utf8nlen(this->string_value, this->string_length);
#endif
    return this;
}

static void free_value(void* parent, value *v) {
    if (v && v->owner == parent) {
        if (v->string_value) free(v->string_value);
        free(v);
    }
}

/* Int Flag */
static void set_type_int(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_int = 1;
}

static void add_type_int(value *v) {
    v->status.type_int = 1;
}

static unsigned int get_type_int(value *v) {
    return v->status.type_int;
}

/* Float Flag */
static void set_type_float(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_float = 1;
}

static void add_type_float(value *v) {
    v->status.type_float = 1;
}

static unsigned int get_type_float(value *v) {
    return v->status.type_float;
}

/* Decimal Flag */
static void set_type_decimal(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_decimal = 1;
}

static void add_type_decimal(value *v) {
    v->status.type_decimal = 1;
}

static unsigned int get_type_decimal(value *v) {
    return v->status.type_decimal;
}

/* String Flag */
static void set_type_string(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_string = 1;
}

static void add_type_string(value *v) {
    v->status.type_string = 1;
}

static unsigned int get_type_string(value *v) {
    return v->status.type_string;
}

/* Object Flag */
static void set_type_object(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_object = 1;
}

static void add_type_object(value *v) {
    v->status.type_object = 1;
}

static unsigned int get_type_object(value *v) {
    return v->status.type_object;
}

/* Unset Flag */
static void unset_type(value *v) {
    v->status.all_type_flags = 0;
}

static void set_int(value *v, rxinteger value) {
    v->int_value = value;
}
static void set_float(value *v, double value) {
    v->float_value = value;
}

static void prep_string_buffer(value *v, size_t length) {
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

static void extend_string_buffer(value *v, size_t length) {
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

static void set_string(value *v, char *value, size_t length) {
    prep_string_buffer(v,length);
    memcpy(v->string_value, value, v->string_length);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = utf8nlen(v->string_value, v->string_length);
#endif
}

static void set_const_string(value *v, string_constant *from) {
    set_string(v, from->string,  from->string_len);
}

static void set_value_string(value *v, value *from) {
    set_string(v, from->string_value,  from->string_length);
}

static void set_buffer_string(value *v, char *buffer, size_t length, size_t buffer_length
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
static void copy_value(value *dest, value *source) {
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

static int string_cmp(char *value1, size_t length1, char *value2, size_t length2) {
    if (length1 > length2) return 1;
    if (length1 < length2) return -1;
    return strncmp(value1, value2, length1);
}

static int string_cmp_value(value *v1, value *v2) {
    return string_cmp(v1->string_value, v1->string_length,
                      v2->string_value, v2->string_length);
}

static int string_cmp_const(value *v1, string_constant *v2) {
    return string_cmp(v1->string_value, v1->string_length,
                      v2->string, v2->string_len);
}

/*
 * TODO - Optimisation opportunity - keyhole optimiser can change to
 * more specific instructions where there is no overlap or to do
 * a prepend / append
 */
static void string_concat(value *v1, value *v2, value *v3) {
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

static void string_sconcat(value *v1, value *v2, value *v3) {
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

static void string_concat_char(value *v1, value *v2) {
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
static void string_from_int(value *v) {
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
static void string_from_float(value *v) {
    prep_string_buffer(v, SMALLEST_STRING_BUFFER_LENGTH); // Large enough for a float
    v->string_length = snprintf(v->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%g",v->float_value);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = v->string_length;
#endif
}

#endif //CREXX_RXVMVARS_H
