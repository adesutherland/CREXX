/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//
// RXPA (CREXX Plugin Architecture) support functions
//
#include <stdlib.h>
#include <string.h>

#include "crexxpa.h"
#include "rxvmintp.h"
#include "rxvmvars.h"

/* Transient memory pool for RXPA Copy-Out */
typedef struct rxpa_pool_node {
    void* ptr;
    struct rxpa_pool_node* next;
} rxpa_pool_node;

static rxpa_pool_node* current_pool_head = NULL;

typedef struct rxpa_value_visit_set {
    value** items;
    size_t count;
    size_t capacity;
} rxpa_value_visit_set;

typedef enum rxpa_utf8_validation_result {
    RXPA_UTF8_OK = 0,
    RXPA_UTF8_INVALID = 1,
    RXPA_UTF8_NO_MEMORY = 2
} rxpa_utf8_validation_result;

static int rxpa_utf8_checks_disabled(void) {
    const char* env = getenv("CREXX_RXPA_DISABLE_UTF8_CHECKS");
    if (!env || !*env) return 0;
    if (strcmp(env, "0") == 0 ||
        strcmp(env, "false") == 0 ||
        strcmp(env, "FALSE") == 0 ||
        strcmp(env, "off") == 0 ||
        strcmp(env, "OFF") == 0 ||
        strcmp(env, "no") == 0 ||
        strcmp(env, "NO") == 0) {
        return 0;
    }
    return 1;
}

static int rxpa_visit_value(rxpa_value_visit_set* visited, value* v) {
    value** items;
    size_t i;
    size_t new_capacity;

    if (!visited || !v) return 0;
    for (i = 0; i < visited->count; i++) {
        if (visited->items[i] == v) return 0;
    }

    if (visited->count == visited->capacity) {
        new_capacity = visited->capacity ? visited->capacity * 2 : 16;
        items = (value**)realloc(visited->items, new_capacity * sizeof(value*));
        if (!items) return -1;
        visited->items = items;
        visited->capacity = new_capacity;
    }

    visited->items[visited->count++] = v;
    return 1;
}

static rxpa_utf8_validation_result rxpa_validate_value_tree(
    value* v,
    rxpa_value_visit_set* visited) {

    int visit_rc;
    size_t i;

    if (!v) return RXPA_UTF8_OK;

    visit_rc = rxpa_visit_value(visited, v);
    if (visit_rc < 0) return RXPA_UTF8_NO_MEMORY;
    if (visit_rc == 0) return RXPA_UTF8_OK;

#ifndef NUTF8
    {
        size_t chars = 0;
        if (validate_utf8_bytes(v->string_value, v->string_length, &chars) != 0) {
            return RXPA_UTF8_INVALID;
        }
        v->string_chars = chars;
        v->string_char_pos = 0;
        mark_utf8_valid_count(v);
    }
#endif

    for (i = 0; i < v->num_attributes; i++) {
        rxpa_utf8_validation_result rc;
        rc = rxpa_validate_value_tree(v->attributes ? v->attributes[i] : NULL, visited);
        if (rc != RXPA_UTF8_OK) return rc;
    }

    return RXPA_UTF8_OK;
}

static void rxpa_set_signal(value* signal, rxsignal code, const char* message) {
    if (!signal) return;
    if (signal->num_attributes) set_num_attributes(signal, 0);
    set_int(signal, (rxinteger)code);
    set_null_string(signal, message ? message : "");
}

static void rxpa_validate_native_outputs(
    int args,
    value** argv,
    value* ret,
    value* signal) {

    rxpa_value_visit_set visited;
    rxpa_utf8_validation_result rc;
    int i;

    if (rxpa_utf8_checks_disabled()) return;

    visited.items = NULL;
    visited.count = 0;
    visited.capacity = 0;

    rc = rxpa_validate_value_tree(ret, &visited);
    if (rc == RXPA_UTF8_OK && args > 0 && argv) {
        for (i = 0; i < args; i++) {
            rc = rxpa_validate_value_tree(argv[i], &visited);
            if (rc != RXPA_UTF8_OK) break;
        }
    }
    if (rc == RXPA_UTF8_OK) {
        rc = rxpa_validate_value_tree(signal, &visited);
    }

    free(visited.items);

    if (rc == RXPA_UTF8_INVALID) {
        rxpa_set_signal(signal, SIGNAL_UNICODE_ERROR, "Invalid UTF-8 returned by native RXPA function");
    } else if (rc == RXPA_UTF8_NO_MEMORY) {
        rxpa_set_signal(signal, SIGNAL_FAILURE, "Unable to validate native RXPA UTF-8 output");
    }
}

/* Function to call a native RXPA (CREXX Plugin Architecture) function */
void rxvm_callfunc(void* function, int args, value** argv, value* ret, value* signal) {
    rxpa_libfunc native_function = (rxpa_libfunc)function;
    rxpa_attribute_value* arg_values = (rxpa_attribute_value*)argv;
    rxpa_attribute_value return_value = (rxpa_attribute_value)ret;
    rxpa_attribute_value signal_value = (rxpa_attribute_value)signal;

    /* Save Context */
    rxpa_pool_node* saved_head = current_pool_head;
    /* Reset Context */
    current_pool_head = NULL;

    /* Call */
    native_function(args, arg_values, return_value, signal_value);

    rxpa_validate_native_outputs(args, argv, ret, signal);

    /* Cleanup */
    while (current_pool_head) {
        rxpa_pool_node* next = current_pool_head->next;
        free(current_pool_head->ptr);
        free(current_pool_head);
        current_pool_head = next;
    }

    /* Restore Context */
    current_pool_head = saved_head;
}

/* Function to get signal text from a signal code  */
char* rxvm_getsignaltext(rxsignal signal) {
     switch (signal) {
         case SIGNAL_NONE:
             return "OK";
         case SIGNAL_ERROR:
             return "ERROR";
         case SIGNAL_OVERFLOW_UNDERFLOW:
             return "OVERFLOW_UNDERFLOW";
         case SIGNAL_CONVERSION_ERROR:
             return "CONVERSION_ERROR";
         case SIGNAL_UNKNOWN_INSTRUCTION:
             return "UNKNOWN_INSTRUCTION";
         case SIGNAL_FUNCTION_NOT_FOUND:
             return "FUNCTION_NOT_FOUND";
         case SIGNAL_OUT_OF_RANGE:
             return "OUT_OF_RANGE";
         case SIGNAL_REFERENCE_INVALID:
             return "REFERENCE_INVALID";
         case SIGNAL_OBJECT_NOT_INITIALIZED:
             return "OBJECT_NOT_INITIALIZED";
         case SIGNAL_FAILURE:
             return "FAILURE";
         case SIGNAL_HALT:
             return "HALT";
         case SIGNAL_NOTREADY:
             return "NOTREADY";
         case SIGNAL_INVALID_ARGUMENTS:
             return "INVALID_ARGUMENTS";
         case SIGNAL_OTHER:
             return "OTHER";
         default:;
     }
     return "UNKNOWN";
 }

 /* Function to get a signal code from a signal text */
 rxsignal rxvm_getsignalcode(char* signalText) {
        if (strcmp(signalText, "OK") == 0) {
            return SIGNAL_NONE;
        } else if (strcmp(signalText, "ERROR") == 0) {
            return SIGNAL_ERROR;
        } else if (strcmp(signalText, "OVERFLOW_UNDERFLOW") == 0) {
            return SIGNAL_OVERFLOW_UNDERFLOW;
        } else if (strcmp(signalText, "CONVERSION_ERROR") == 0) {
            return SIGNAL_CONVERSION_ERROR;
        } else if (strcmp(signalText, "UNKNOWN_INSTRUCTION") == 0) {
            return SIGNAL_UNKNOWN_INSTRUCTION;
        } else if (strcmp(signalText, "FUNCTION_NOT_FOUND") == 0) {
            return SIGNAL_FUNCTION_NOT_FOUND;
        } else if (strcmp(signalText, "OUT_OF_RANGE") == 0) {
            return SIGNAL_OUT_OF_RANGE;
        } else if (strcmp(signalText, "REFERENCE_INVALID") == 0) {
            return SIGNAL_REFERENCE_INVALID;
        } else if (strcmp(signalText, "FAILURE") == 0) {
            return SIGNAL_FAILURE;
        } else if (strcmp(signalText, "HALT") == 0) {
            return SIGNAL_HALT;
        } else if (strcmp(signalText, "NOTREADY") == 0) {
            return SIGNAL_NOTREADY;
        } else if (strcmp(signalText, "INVALID_ARGUMENTS") == 0) {
            return SIGNAL_INVALID_ARGUMENTS;
        } else if (strcmp(signalText, "OTHER") == 0) {
            return SIGNAL_OTHER;
        }
        return SIGNAL_OTHER;
 }

/* Get a string from an attribute value */
char* rxvm_getstring(rxpa_attribute_value attributeValue) {
    value* val = (value*)attributeValue;
    if (val) {
        char* ret;
        rxpa_pool_node* node;
        null_terminate_string_buffer(val);
#if pluginDEBUG>0
        printf("Argument String '%s'\n",val->string_value);
#endif
        /* Copy-Out */
        ret = malloc(val->string_length + 1);
        if (ret) {
            memcpy(ret, val->string_value, val->string_length);
            ret[val->string_length] = '\0';
            /* Track in pool */
            node = malloc(sizeof(rxpa_pool_node));
            if (node) {
                node->ptr = ret;
                node->next = current_pool_head;
                current_pool_head = node;
            } else {
                /* If we can't track it, we're in trouble, but let's at least not leak it now if we can help it */
                /* In a real system we'd signal an error */
            }
        }
        return ret;
    }
#if pluginDEBUG>0
    printf("Argument String ''\n");
#endif
    return "";
}

/* Set a string in an attribute value */
void rxvm_setstring(rxpa_attribute_value attributeValue, char* string){
    value* val = (value*)attributeValue;
    if (val) set_null_string(val, string);
}

/* Set an integer in an attribute value */
void rxvm_setint(rxpa_attribute_value attributeValue, rxinteger int_value) {
    value* val = (value*)attributeValue;
    if (val) set_int(val, int_value);
}

/* Get an integer from an attribute value */
rxinteger rxvm_getint(rxpa_attribute_value attributeValue) {
    value* val = (value*)attributeValue;
#if pluginDEBUG>0
    printf("Argument INT '%d'\n",val->int_value);
#endif
    if (val) return val->int_value;
    else return 0;
}

/* Set a float in an attribute value */
void rxvm_setfloat(rxpa_attribute_value attributeValue, double double_value) {
    value* val = (value*)attributeValue;
    if (val) set_float(val, double_value);
}

/* Get a float from an attribute value */
double rxvm_getfloat(rxpa_attribute_value attributeValue) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument FLOAT '%g'\n",val->float_value);
#endif
    if (val) return val->float_value;
    else return 0.0;
}

/* Set a native binary payload in an attribute value */
int rxvm_setnativepayload(rxpa_attribute_value attributeValue,
                          const void *payload,
                          size_t length,
                          const rxpa_native_payload_ops *ops,
                          unsigned int flags) {
    value* val = (value*)attributeValue;
    if (!val) return -1;
    return set_native_payload(val, payload, length, ops, flags);
}

/* Get a native binary payload from an attribute value */
void* rxvm_getnativepayload(rxpa_attribute_value attributeValue,
                            size_t *out_length,
                            const rxpa_native_payload_ops **out_ops,
                            unsigned int *out_flags) {
    return get_native_payload((value*)attributeValue, out_length, out_ops, out_flags);
}

/* Get the number of child attributes */
rxinteger rxvm_getnumattrs(rxpa_attribute_value attributeValue) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument NUMATTRS '%d'\n",(int)val->num_attributes);
#endif
    if (val) return (rxinteger)val->num_attributes;
    else return 0;
}

/* Set the number of child attributes */
void rxvm_setnumattrs(rxpa_attribute_value attributeValue, rxinteger numAttrs) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument SETNUMATTRS '%d'\n",(int)numAttrs);
#endif
    if (val) set_num_attributes(val, numAttrs);
}

/* Get the nth child attribute */
rxpa_attribute_value rxvm_getattr(rxpa_attribute_value attributeValue, rxinteger index) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument GETATTR '%d'\n",(int)index);
#endif
    if (val && index >= 0 && index < val->num_attributes) {
        return (rxpa_attribute_value)val->attributes[index];
    }
    return NULL;
}

/* Insert a child attribute before the nth position */
rxpa_attribute_value rxvm_insertattr(rxpa_attribute_value attributeValue, rxinteger index) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument INSERTATTR '%d'\n",(int)index);
#endif
    if (val && index >= 0 && index <= val->num_attributes) {
        insert_attributes(val, (size_t)index, 1);
        return (rxpa_attribute_value)val->attributes[index];
    }
    return NULL;
}

/* Remove the nth child attribute */
void rxvm_removeattr(rxpa_attribute_value attributeValue, rxinteger index) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument REMOVEATTR '%d'\n",(int)index);
#endif
    if (val && index >= 0 && index < val->num_attributes) {
        delete_attributes(val, (size_t)index, 1);
    }
}

/* Swap the nth child attribute with the mth child attribute */
void rxvm_swapattrs(rxpa_attribute_value attributeValue, rxinteger index1, rxinteger index2) {
    value* val = (value*)attributeValue;
#ifdef pluginDEBUG
    printf("Argument SWAPATTRS '%d' '%d'\n",(int)index1,(int)index2);
#endif
    if (val && index1 >= 0 && index1 < val->num_attributes && index2 >= 0 && index2 < val->num_attributes) {
        value* temp = val->attributes[index1];
        val->attributes[index1] = val->attributes[index2];
        val->attributes[index2] = temp;
    }
}
