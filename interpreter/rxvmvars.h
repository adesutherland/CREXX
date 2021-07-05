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
}

static void set_const_string(value *v, string_constant *from) {
    set_string(v, from->string,  from->string_len);
}

static void set_value_string(value *v, value *from) {
    set_string(v, from->string_value,  from->string_length);
}

static void set_buffer_string(value *v, char *buffer, size_t length, size_t buffer_length) {
    if (v->string_value) free(v->string_value);
    v->string_value = buffer;
    v->string_length = length;
    v->string_buffer_length = buffer_length;
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
    size_t len = v2->string_length + v3->string_length;
    size_t buffer_len = buffer_size(len);
    char *buffer = malloc(buffer_len);

    memcpy(buffer, v2->string_value, v2->string_length);
    memcpy(buffer + v2->string_length, v3->string_value, v3->string_length);

    set_buffer_string(v1, buffer, len, buffer_len);
}

static void string_sconcat(value *v1, value *v2, value *v3) {
    size_t len = v2->string_length + v3->string_length + 1;
    size_t buffer_len = buffer_size(len);
    char *buffer = malloc(buffer_len);

    memcpy(buffer, v2->string_value, v2->string_length);
    buffer[v2->string_length] = ' ';
    memcpy(buffer + v2->string_length + 1, v3->string_value, v3->string_length);

    set_buffer_string(v1, buffer, len, buffer_len);
}

static void string_concat_char(value *v1, value *v2) {
    int char_size;
    size_t len;
    char *insert_at;
#ifdef NUTF8
    char_size = 1;
#else
    char_size = utf8codepointsize(v2->int_value);
#endif
    insert_at = v1->string_value + v1->string_length;
    len = v1->string_length + char_size;
    extend_string_buffer(v1,len);
#ifdef NUTF8
    *insert_at = (unsigned char)v2->int_value;
#else
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
}

/* Calculate the string value */
static void string_from_float(value *v) {
    prep_string_buffer(v, SMALLEST_STRING_BUFFER_LENGTH); // Large enough for a float
    v->string_length = snprintf(v->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%g",v->float_value);
    v->status.type_string = 1;
}

#endif //CREXX_RXVMVARS_H
