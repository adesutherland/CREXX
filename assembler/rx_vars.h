/* CREXX
 * RUNTIME Variable Support
 */

#ifndef CREXX_RX_VARS_H
#define CREXX_RX_VARS_H

//#include "rx_intrp.h"

#define SMALL_STRING_BUFFER 24
#define EXTEND_STRING_BUFFER 300

typedef struct value value;
typedef struct string_extend_buffer string_extend_buffer;

struct string_extend_buffer {
    char buffer[EXTEND_STRING_BUFFER];
    string_extend_buffer *next;
    string_extend_buffer *prev;
    value *owning_value;
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
    long long int int_value;
    double float_value;
    void *decimal_value; /* TODO */
    char string_value[SMALL_STRING_BUFFER];
    size_t string_length;
    string_extend_buffer *string_extend; /* TODO */
    void *object_value;
};

/* value factories */
static value* value_f() {
    value* this = calloc(1,sizeof(value)); /* Zeros data */
    return this;
}

/* value factories - int value */
static value* value_int_f(long long initial_value) {
    value* this = calloc(1,sizeof(value)); /* Zeros data */
    this->status.primed_int = 1;
    this->int_value = initial_value;
    return this;
}

/* value factories - constant string value */
static value* value_conststring_f(string_constant *initial_value) {
    /* TODO Handle strings > 24 characters! */
    value* this = calloc(1,sizeof(value)); /* Zeros data */
    this->status.primed_string = 1;
    this->string_length = initial_value->string_len;
    memcpy(this->string_value, initial_value->string,  this->string_length);
    return this;
}

/* value factories - null string value */
static value* value_nullstring_f(char *initial_value) {
    /* TODO Handle strings > 24 characters! */
    value* this = calloc(1,sizeof(value)); /* Zeros data */
    this->status.primed_string = 1;
    this->string_length = strlen(initial_value);
    memcpy(this->string_value, initial_value,  this->string_length);
    return this;
}

static void free_value(value *v) {
    /* TODO string extensions and other complexities */
    free(v);
}

static void set_int(value *v, long long value) {
    v->status.all_flags = 0;
    v->status.primed_int = 1;
    v->int_value = value;
}

static void set_conststring(value *v, string_constant *value) {
    v->status.all_flags = 0;
    v->status.primed_string = 1;
    v->string_length = value->string_len;
    memcpy(v->string_value, value->string,  v->string_length);
}

static void prime_string(value *v) {
    if (v->status.primed_string) return;
    if (v->status.primed_int) {
        /* TODO clear extended string data */
        v->string_length = snprintf(v->string_value,SMALL_STRING_BUFFER,"%lld",v->int_value);
        v->status.primed_string = 1;
    }
    else if (v->status.primed_float) {
        /* TODO clear extended string data */
        v->string_length = snprintf(v->string_value,SMALL_STRING_BUFFER,"%f",v->float_value);
        v->status.primed_string = 1;
    }
}

#endif //CREXX_RX_VARS_H
