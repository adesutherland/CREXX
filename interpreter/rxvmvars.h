/* CREXX
 * RUNTIME Variable Support
 */

#ifndef CREXX_RXVMVARS_H
#define CREXX_RXVMVARS_H

//#include "rx_intrp.h"

//#define SMALL_STRING_BUFFER 24
#define SMALL_STRING_BUFFER 81
#define EXTEND_STRING_BUFFER 300

typedef struct value value;
typedef struct string_extend_buffer string_extend_buffer;

struct string_extend_buffer {
    size_t buffer_size;
    char buffer[EXTEND_STRING_BUFFER];
    /*
    char buffer[EXTEND_STRING_BUFFER];
    string_extend_buffer *next;
    string_extend_buffer *prev;
    value *owning_value;*/
};

typedef union {
    struct {
        unsigned int primed_int : 1;
        unsigned int primed_float : 1;
        unsigned int primed_decimal : 1;
        unsigned int primed_string : 1;
        unsigned int is_object : 1; /* Objects aren't primed as such */
    };
    unsigned int all_flags;;
} value_status;

struct value {
    /* bit field to store value status */
    value_status status;

    /* Value */
    rxinteger int_value;
    double float_value;
    void *decimal_value; /* TODO */
    char string_value[SMALL_STRING_BUFFER];
    size_t string_length;
    string_extend_buffer *string_extend; /* TODO */
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
    this->status.primed_int = 1;
    this->int_value = initial_value;
    return this;
}
static value* value_float_f(void* parent, double initial_value) {
    value* this = calloc(1,sizeof(value)); /* Zeros data */
    this->owner = parent;
    this->status.primed_float = 1;
    this->float_value = initial_value;
    return this;
}

/* value factories - constant string value */
static value* value_conststring_f(void* parent, string_constant *initial_value) {
    /* TODO Handle strings > 24 characters! */
    value* this = calloc(1,sizeof(value)); /* Zeros data */
    this->owner = parent;
    this->status.primed_string = 1;
    this->string_length = initial_value->string_len;
    memcpy(this->string_value, initial_value->string,  this->string_length);
    return this;
}

/* value factories - null string value */
static value* value_nullstring_f(void* parent, char *initial_value) {
    /* TODO Handle strings > 24 characters! */
    value* this = calloc(1,sizeof(value)); /* Zeros data */
    this->owner = parent;
    this->status.primed_string = 1;
    this->string_length = strlen(initial_value);
    memcpy(this->string_value, initial_value,  this->string_length);
    return this;
}

static void free_value(void* parent, value *v) {
    if (v && v->owner == parent) {
        /* TODO string extensions and other complexities */
        free(v);
    }
}

static void set_int(value *v, rxinteger value) {
    v->status.all_flags = 0;
    v->status.primed_int = 1;
    v->int_value = value;
}
static void set_float(value *v, double value) {
    v->status.all_flags = 0;
    v->status.primed_float = 1;
    v->float_value = value;
}

/*
 * TODO - Optimisation opportunity - keyhole optimiser can change to
 * more specific instructions where there is no overlap or to do
 * a prepend / append
 */
static void string_concat(value *v1, value *v2, value *v3) {
    size_t len = v2->string_length + v3->string_length;
    char *buffer = malloc(len);

    memcpy(buffer, v2->string_value, v2->string_length);
    memcpy(buffer + v2->string_length, v3->string_value, v3->string_length);

    /* TODO Need to fix for large strings */
    if (len > SMALL_STRING_BUFFER) len = SMALL_STRING_BUFFER;

    memcpy(v1->string_value, buffer, len);
    v1->string_length = len;
    free(buffer);
}

static void string_sconcat(value *v1, value *v2, value *v3) {
    size_t len = v2->string_length + v3->string_length + 1;
    char *buffer = malloc(len);

    memcpy(buffer, v2->string_value, v2->string_length);
    buffer[v2->string_length] = ' ';
    memcpy(buffer + v2->string_length + 1, v3->string_value, v3->string_length);

    /* TODO Need to fix for large strings */
    if (len > SMALL_STRING_BUFFER) len = SMALL_STRING_BUFFER;

    memcpy(v1->string_value, buffer, len);
    v1->string_length = len;
    free(buffer);
}

/* This resets the variable flag so that only the matser type flag is set.
 * This allow the value to have multiple fast updates without
 * having to consider the status. When the series of changes are done the flags
 * can be reset to allow another buffer to be primed (like string for printing)
 * (Deprecated)
 */
static void master_int(value *v) {
    v->status.all_flags = 0;
    v->status.primed_int = 1;
}

static void master_float(value *v) {
    v->status.all_flags = 0;
    v->status.primed_float = 1;
}

static void master_string(value *v) {
    v->status.all_flags = 0;
    v->status.primed_string = 1;
}

static void set_conststring(value *v, string_constant *value) {
    v->status.all_flags = 0;
    v->status.primed_string = 1;
    v->string_length = value->string_len;
    memcpy(v->string_value, value->string,  v->string_length);
}

/* Calculate the string value */
static void prime_string(value *v) {
    if (v->status.primed_string) return;
    if (v->status.primed_int) {
        /* TODO clear extended string data */
#ifdef __32BIT__
        v->string_length = snprintf(v->string_value,SMALL_STRING_BUFFER,"%ld",v->int_value);
#else
        v->string_length = snprintf(v->string_value,SMALL_STRING_BUFFER,"%lld",v->int_value);
#endif
        v->status.primed_string = 1;
    }
    else if (v->status.primed_float) {
        /* TODO clear extended string data */
        v->string_length = snprintf(v->string_value,SMALL_STRING_BUFFER,"%g",v->float_value);
        v->status.primed_string = 1;
    }
}

#endif //CREXX_RXVMVARS_H
