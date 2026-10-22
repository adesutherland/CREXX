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

#ifndef CREXX_RXVML_H
#define CREXX_RXVML_H

#include <stddef.h>
#include "rxvalue.h"

#define RXVML_ABI_VERSION 8
#define RXVML_ADDRESS_ENVIRONMENT_INTERFACE "_rxsysb.addressenvironment"
#define RXVML_ADDRESS_ENVIRONMENT_FACTORY_PROC "_rxsysb._new_address_environment"

typedef struct rxvml_context rxvml_context;
typedef value   rxvml_value;

typedef struct rxvml_address_binding {
    const char* kind;
    const char* internal_name;
    const char* external_alias;
    const char* value;
    rxvml_value* value_object;
    const char* flags;
} rxvml_address_binding;

typedef struct rxvml_address_request {
    const char* environment_name;
    const char* command;
    rxvml_value* stdin_endpoint;
    rxvml_value* stdout_endpoint;
    rxvml_value* stderr_endpoint;
    size_t binding_count;
    const rxvml_address_binding* bindings;
    rxvml_value* sandbox;
} rxvml_address_request;

typedef struct rxvml_address_response {
    int rc;
    const char* condition_name;
    const char* diagnostic;
    size_t updated_binding_count;
    const rxvml_address_binding* updated_bindings;
} rxvml_address_response;

typedef int (*rxvml_address_callback)(
    rxvml_context* ctx,
    const rxvml_address_request* request,
    rxvml_address_response* response,
    void* userdata);

typedef struct rxvml_address_function_request {
    const char* environment_name;
    const char* function_name;
    size_t argc;
    const char** args;
    rxvml_value* sandbox;
    const char* flags;
} rxvml_address_function_request;

typedef struct rxvml_address_function_response {
    int rc;
    const char* result;
    const char* condition_name;
    const char* diagnostic;
} rxvml_address_function_response;

typedef int (*rxvml_address_function_callback)(
    rxvml_context* ctx,
    const rxvml_address_function_request* request,
    rxvml_address_function_response* response,
    void* userdata);

/* Additive length-bearing ADDRESS callbacks. Content is UTF-8, including NUL;
 * protocol names/flags remain NUL-free. Request snapshots are read-only and
 * live through the callback, including nested VM calls. Response buffers live
 * until release(release_userdata), called once after copying even on refusal.
 * A NULL release permits static/caller-owned response buffers. VM value handles
 * remain borrowed from this context and may not outlive the callback. Each response
 * copy admits at most this many logical payload/auxiliary bytes. */
#define RXVML_ADDRESS_SPAN_MAX_BYTES (64u * 1024u * 1024u)
typedef struct rxvml_text_span { const char *data; size_t length; } rxvml_text_span;
typedef struct rxvml_address_binding_spans {
    const char *kind, *internal_name, *external_alias;
    rxvml_text_span value;
    rxvml_value *value_object;
    const char *flags;
} rxvml_address_binding_spans;
typedef struct rxvml_address_request_spans {
    size_t struct_size;
    const char *environment_name;
    rxvml_text_span command;
    rxvml_value *stdin_endpoint, *stdout_endpoint, *stderr_endpoint;
    size_t binding_count;
    const rxvml_address_binding_spans *bindings;
    rxvml_value *sandbox;
} rxvml_address_request_spans;
typedef struct rxvml_address_response_spans {
    size_t struct_size;
    int rc;
    const char *condition_name;
    rxvml_text_span diagnostic;
    size_t updated_binding_count;
    const rxvml_address_binding_spans *updated_bindings;
    void (*release)(void *);
    void *release_userdata;
} rxvml_address_response_spans;
typedef struct rxvml_address_function_request_spans {
    size_t struct_size;
    const char *environment_name, *function_name;
    size_t argc;
    const rxvml_text_span *args;
    rxvml_value *sandbox;
    const char *flags;
} rxvml_address_function_request_spans;
typedef struct rxvml_address_function_response_spans {
    size_t struct_size;
    int rc, has_result;
    rxvml_text_span result;
    const char *condition_name;
    rxvml_text_span diagnostic;
    void (*release)(void *);
    void *release_userdata;
} rxvml_address_function_response_spans;
typedef int (*rxvml_address_callback_spans)(rxvml_context *,
    const rxvml_address_request_spans *, rxvml_address_response_spans *, void *);
typedef int (*rxvml_address_function_callback_spans)(rxvml_context *,
    const rxvml_address_function_request_spans *, rxvml_address_function_response_spans *, void *);

/* Context lifecycle */
rxvml_context* rxvml_create(const char* location, unsigned flags);
void           rxvml_destroy(rxvml_context* ctx);

/* Module management */
int rxvml_load_module_file(rxvml_context* ctx, const char* rxbin_path);
int rxvml_load_module_buffer(rxvml_context* ctx, const void* buf, size_t len);

/* Value construction & setting */
rxvml_value* rxvml_value_new(rxvml_context* ctx);
void         rxvml_set_int(rxvml_value* v, rxinteger i);
int          rxvml_set_str(rxvml_value* v, const char* s, size_t len);
int          rxvml_set_native_payload(rxvml_value* v, const void* payload, size_t len,
                                      const rxvm_native_payload_ops* ops, unsigned int flags);
void*        rxvml_get_native_payload(rxvml_value* v, size_t* out_len,
                                      const rxvm_native_payload_ops** out_ops,
                                      unsigned int* out_flags);
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
int rxvml_call_procedure_descriptor(
    rxvml_context* ctx,
    const char* proc_descriptor,
    size_t argc,
    rxvml_value** args,
    rxvml_value** response_out);

int rxvml_run(
    rxvml_context* ctx,
    int argc,
    const char** argv,
    int* program_rc);

/* UTF-8 argument spans may contain embedded NUL; lengths are bytes. */
int rxvml_run_with_lengths(
    rxvml_context* ctx,
    int argc,
    const char** argv,
    const size_t* argv_lengths,
    int* program_rc);

int rxvml_call_factory_descriptor(
    rxvml_context* ctx,
    const char* class_name,
    const char* factory_descriptor,
    size_t argc,
    rxvml_value** args,
    rxvml_value** response_out);

/* ADDRESS convenience wrappers over the canonical Rexx/runtime path */
int rxvml_address_register_environment(
    rxvml_context* ctx,
    const char* env_name,
    rxvml_value* env_obj);

int rxvml_address_register_callback_environment(
    rxvml_context* ctx,
    const char* env_name,
    const char* instance_id,
    rxvml_address_callback callback,
    rxvml_address_function_callback function_callback,
    void* userdata);

int rxvml_address_register_callback_environment_spans(
    rxvml_context *ctx, const char *env_name, const char *instance_id,
    rxvml_address_callback_spans callback,
    rxvml_address_function_callback_spans function_callback, void *userdata);

/* Queue POSIX_INT in this context's existing active execution word. Only the
 * current execution owner may call, including its native/OS-signal callback.
 * Foreign/inactive contexts return -1 without changing any pending word.
 * The caller keeps ctx live; this is not a cross-thread entry or queue. */
int rxvml_request_interrupt(rxvml_context *ctx);

int rxvml_address_create_environment(
    rxvml_context* ctx,
    const char* env_name,
    rxvml_value** env_obj_out);

int rxvml_address_set_environment(
    rxvml_context* ctx,
    const char* env_name);

/* Read from the standard ADDRESS sandbox layout. */
int rxvml_address_sandbox_get(
    const rxvml_address_request* request,
    const char* name,
    char* out_value,
    size_t out_value_len);

/* Write to the request sandbox through the standard layout, or method fallback. */
int rxvml_address_sandbox_set(
    rxvml_context* ctx,
    const rxvml_address_request* request,
    const char* name,
    const char* value);

/* Emit text to the ADDRESS command's output or error redirect. If no redirect
 * was supplied, stdout/stderr is used. The helper finalizes the endpoint so
 * array/string redirects are readable when the native callback returns. */
int rxvml_address_emit_output(
    rxvml_context* ctx,
    const rxvml_address_request* request,
    const char* text);

int rxvml_address_emit_error(
    rxvml_context* ctx,
    const rxvml_address_request* request,
    const char* text);

/* Look up a scalar ADDRESS binding by internal name or external alias. */
int rxvml_address_binding_get(
    const rxvml_address_request* request,
    const char* name,
    char* out_value,
    size_t out_value_len);

/* ADDRESS stem helpers for exposed .string[] / addressstem bindings. */
int rxvml_address_stem_get(
    const rxvml_value* stem,
    const char* name,
    char* out_value,
    size_t out_value_len);

int rxvml_address_stem_set(
    rxvml_context* ctx,
    rxvml_value* stem,
    const char* name,
    const char* value);

int rxvml_address_binding_stem_get(
    const rxvml_address_binding* binding,
    const char* name,
    char* out_value,
    size_t out_value_len);

int rxvml_address_binding_stem_set(
    rxvml_context* ctx,
    const rxvml_address_binding* binding,
    const char* name,
    const char* value);


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

int rxvml_call_method_descriptor(
    rxvml_context* ctx,
    rxvml_value* obj,
    const char* class_name,
    const char* method_descriptor,
    size_t argc,
    rxvml_value** args,
    rxvml_value** response_out);

/* Say Exit */
/* Receives the complete SAY/SAYX byte span, including SAY's trailing LF.
 * The pointer remains valid only for the duration of the callback.
 * NULL restores default routing. */
typedef void (*rxvml_say_exit_bytes_func)(const char* message, size_t length);
void rxvml_set_say_exit_bytes(rxvml_say_exit_bytes_func say_exit);
void rxvml_set_context_say_exit_bytes(rxvml_context* ctx,
                                      rxvml_say_exit_bytes_func say_exit);

/* Error reporting */
int  rxvml_last_error(rxvml_context* ctx, const char** out_msg);

/* Introspection */
unsigned int rxvml_get_debug_mode(rxvml_context* ctx);

#endif /* CREXX_RXVML_H */
