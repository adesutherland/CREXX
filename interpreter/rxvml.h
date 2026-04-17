#ifndef CREXX_RXVML_H
#define CREXX_RXVML_H

#include <stddef.h>
#include "rxvalue.h"

#define RXVML_ABI_VERSION 1

typedef struct rxvml_context rxvml_context;
typedef value   rxvml_value;

/* Context lifecycle */
rxvml_context* rxvml_create(const char* location, unsigned flags);
void           rxvml_destroy(rxvml_context* ctx);

/* Module management */
int rxvml_load_module_file(rxvml_context* ctx, const char* rxbin_path);
int rxvml_load_module_buffer(rxvml_context* ctx, const void* buf, size_t len);

/* Value construction & setting */
rxvml_value* rxvml_value_new(rxvml_context* ctx);
void         rxvml_set_int(rxvml_value* v, rxinteger i);
void         rxvml_set_str(rxvml_value* v, const char* s, size_t len);
void         rxvml_value_free(rxvml_value* v);

/* Object & Array construction */
rxvml_value* rxvml_object_new(rxvml_context* ctx, size_t num_attrs);
int          rxvml_set_attribute(rxvml_context* ctx, rxvml_value* obj, size_t index1, rxvml_value* val);
rxvml_value* rxvml_get_attribute(rxvml_context* ctx, rxvml_value* obj, size_t index1);

rxvml_value* rxvml_array_new(rxvml_context* ctx, size_t length);
int          rxvml_array_set(rxvml_context* ctx, rxvml_value* arr, size_t index1, rxvml_value* elem);

/* Introspection / extraction */
int      rxvml_to_int      (rxvml_context* ctx, const rxvml_value* v, rxinteger* out_v);
int      rxvml_to_str      (rxvml_context* ctx, const rxvml_value* v, const char** out_s, size_t* out_len);
size_t   rxvml_num_attributes(const rxvml_value* v);

/* Invocation */
int rxvml_call_procedure(
    rxvml_context* ctx,
    const char* proc_name,
    size_t argc,
    rxvml_value** args,
    rxvml_value** response_out);

int rxvml_call_factory(
    rxvml_context* ctx,
    const char* class_name,
    size_t argc,
    rxvml_value** args,
    rxvml_value** response_out);

/* Persistent Registry for Stateful Exits */
int          rxvml_reg_alloc(rxvml_context* ctx, rxvml_value* v, const char* class_name);
void         rxvml_reg_free(rxvml_context* ctx, int reg_idx);
rxvml_value* rxvml_reg_get(rxvml_context* ctx, int reg_idx, char* out_class_name);

/* Discovery & Advanced Invocation */
typedef struct rxvml_class_info {
    char class_name[256];
    char factory_proc[512];
} rxvml_class_info;

int rxvml_discover_classes(rxvml_context* ctx, const char* ns, rxvml_class_info** out_classes, size_t* out_count);

int rxvml_call_method(
    rxvml_context* ctx,
    rxvml_value* obj,
    const char* class_name,
    const char* method_name,
    size_t argc,
    rxvml_value** args,
    rxvml_value** response_out);

/* Say Exit */
typedef void (*rxvml_say_exit_func)(char* message);
void rxvml_set_say_exit(rxvml_say_exit_func say_exit);

/* Error reporting */
int  rxvml_last_error(rxvml_context* ctx, const char** out_msg);

/* Introspection */
unsigned int rxvml_get_debug_mode(rxvml_context* ctx);

#endif /* CREXX_RXVML_H */
