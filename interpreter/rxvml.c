#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "rxvml.h"
#include "rxvmintp.h"
#include "rxvm.h"
#include "rxvmvars.h"
#include "rxastree.h"
#include "rxvmplugin.h"

typedef struct rxvml_registry_entry {
    value* obj;
    char class_name[256];
} rxvml_registry_entry;

typedef struct rxvml_address_callback_entry {
    char env_name[256];
    rxvml_address_callback callback;
    void* userdata;
} rxvml_address_callback_entry;

typedef struct rxvml_external_call_state {
    proc_runtime* ext_proc;
    int ext_argc;
    value** ext_args;
    value* ext_ret;
} rxvml_external_call_state;

enum {
    RXVML_ADDRESS_BINDING_EXTERNAL_ALIAS = 0,
    RXVML_ADDRESS_BINDING_FLAGS = 1,
    RXVML_ADDRESS_BINDING_INTERNAL_NAME = 2,
    RXVML_ADDRESS_BINDING_KIND = 3,
    RXVML_ADDRESS_BINDING_VALUE = 4,
    RXVML_ADDRESS_BINDING_VALUE_OBJECT = 5,
    RXVML_ADDRESS_BINDING_ATTR_COUNT = 6
};

enum {
    RXVML_ADDRESS_RESPONSE_UPDATED_BINDING_COUNT = 4,
    RXVML_ADDRESS_RESPONSE_UPDATED_BINDINGS = 5
};

enum {
    RXVML_ADDRESS_REQUEST_BINDING_COUNT = 0,
    RXVML_ADDRESS_REQUEST_BINDINGS = 1,
    RXVML_ADDRESS_REQUEST_COMMAND = 2,
    RXVML_ADDRESS_REQUEST_ENVIRONMENT_NAME = 3,
    RXVML_ADDRESS_REQUEST_FLAGS = 4,
    RXVML_ADDRESS_REQUEST_SANDBOX = 5,
    RXVML_ADDRESS_REQUEST_STDERR_ENDPOINT = 6,
    RXVML_ADDRESS_REQUEST_STDIN_ENDPOINT = 7,
    RXVML_ADDRESS_REQUEST_STDOUT_ENDPOINT = 8,
    RXVML_ADDRESS_REQUEST_ATTR_COUNT = 9
};

enum {
    RXVML_STANDARD_ADDRESS_SANDBOX_KEY_COUNT = 0,
    RXVML_STANDARD_ADDRESS_SANDBOX_KEYS = 1,
    RXVML_STANDARD_ADDRESS_SANDBOX_PRESENT = 2,
    RXVML_STANDARD_ADDRESS_SANDBOX_VALUES = 3,
    RXVML_STANDARD_ADDRESS_SANDBOX_ATTR_COUNT = 4
};

static const char RXVML_ADDRESS_BINDING_TYPE_NAME[] = "_rxsysb.addressbinding";
static const char RXVML_STANDARD_ADDRESS_SANDBOX_TYPE_NAME[] = "_rxsysb.standardaddresssandbox";
static const char RXVML_STANDARD_ADDRESS_STEM_TYPE_NAME[] = "_rxsysb.standardaddressstem";

struct rxvml_context {
    rxvm_context vm;
    const char* last_error;
    rxvml_registry_entry* registry;
    size_t registry_size;
    rxvml_address_callback_entry* address_callbacks;
    size_t address_callback_size;
};

static rxvml_context* rxvml_active_context = NULL;

/* rxvml_value is defined as an alias to value in the public header */

/* Internal helper to find a procedure by name */
static proc_runtime* find_procedure(rxvm_context* ctx, const char* name) {
    proc_runtime* p = NULL;
    if (src_node(ctx->exposed_proc_tree, (char*)name, (size_t*)&p)) {
        return p;
    }
    return NULL;
}

static void rxvml_save_external_call_state(
    rxvm_context* vm,
    rxvml_external_call_state* state) {

    state->ext_proc = vm->ext_proc;
    state->ext_argc = vm->ext_argc;
    state->ext_args = vm->ext_args;
    state->ext_ret = vm->ext_ret;
}

static void rxvml_restore_external_call_state(
    rxvm_context* vm,
    const rxvml_external_call_state* state) {

    vm->ext_proc = state->ext_proc;
    vm->ext_argc = state->ext_argc;
    vm->ext_args = state->ext_args;
    vm->ext_ret = state->ext_ret;
}

static int rxvml_invoke_external_proc(
    rxvml_context* ctx,
    proc_runtime* proc,
    size_t argc,
    rxvml_value** args,
    const char* dummy_argv0,
    rxvml_value** response_out) {

    rxvml_external_call_state saved_state;
    rxvml_context* previous_active_context;
    value** call_args = NULL;
    value* call_ret;
    char* dummy_argv[1];

    if (!ctx || !proc) return -1;
    if (argc > (size_t)INT_MAX) {
        ctx->last_error = "Too many rxvml call arguments";
        return -1;
    }
    if (argc > 0 && !args) {
        ctx->last_error = "Missing rxvml call argument vector";
        return -1;
    }

    if (argc > 0) {
        size_t i;
        call_args = malloc(sizeof(value*) * argc);
        if (!call_args) {
            ctx->last_error = "Failed to allocate rxvml call arguments";
            return -1;
        }
        for (i = 0; i < argc; i++) {
            if (!args[i]) {
                free(call_args);
                ctx->last_error = "Null rxvml call argument";
                return -1;
            }
            call_args[i] = (value*)args[i];
        }
    }

    call_ret = value_f();
    if (!call_ret) {
        if (call_args) free(call_args);
        ctx->last_error = "Failed to allocate rxvml return value";
        return -1;
    }

    /*
     * rxvml entry points may be called from a native callback while run() is
     * already active. Preserve the outer call trampoline so nested method calls
     * do not corrupt the callback's return path.
     */
    rxvml_save_external_call_state(&ctx->vm, &saved_state);
    ctx->vm.ext_proc = proc;
    ctx->vm.ext_argc = (int)argc;
    ctx->vm.ext_args = call_args;
    ctx->vm.ext_ret = call_ret;

    previous_active_context = rxvml_active_context;
    rxvml_active_context = ctx;
    rxvm_prepare(&ctx->vm);
    dummy_argv[0] = (char*)(dummy_argv0 ? dummy_argv0 : "rxvml_call");
    run(&ctx->vm, 0, dummy_argv);
    rxvml_active_context = previous_active_context;

    if (response_out) {
        *response_out = (rxvml_value*)call_ret;
    } else {
        rxvml_value_free((rxvml_value*)call_ret);
    }

    if (call_args) free(call_args);
    rxvml_restore_external_call_state(&ctx->vm, &saved_state);
    return 0;
}

int rxvm_link(rxvm_context* ctx);
void rxvm_addfunc(rxpa_libfunc func, char* name, char* option, char* type, char* args);

static const char* rxvml_value_cstr(value* v) {
    if (!v) return "";
    null_terminate_string_buffer(v);
    return v->string_value ? v->string_value : "";
}

static void rxvml_set_value_cstr(value* v, const char* s) {
    set_null_string(v, s ? s : "");
}

static void rxvml_populate_address_binding_value(value* binding_value, const rxvml_address_binding* binding) {
    clear_value(binding_value);
    value_init(binding_value);
    binding_value->object_type_name = RXVML_ADDRESS_BINDING_TYPE_NAME;
    binding_value->object_type_name_length = sizeof(RXVML_ADDRESS_BINDING_TYPE_NAME) - 1;
    set_num_attributes(binding_value, RXVML_ADDRESS_BINDING_ATTR_COUNT);

    rxvml_set_value_cstr(binding_value->attributes[RXVML_ADDRESS_BINDING_EXTERNAL_ALIAS], binding ? binding->external_alias : "");
    rxvml_set_value_cstr(binding_value->attributes[RXVML_ADDRESS_BINDING_FLAGS], binding ? binding->flags : "");
    rxvml_set_value_cstr(binding_value->attributes[RXVML_ADDRESS_BINDING_INTERNAL_NAME], binding ? binding->internal_name : "");
    rxvml_set_value_cstr(binding_value->attributes[RXVML_ADDRESS_BINDING_KIND], binding ? binding->kind : "var");
    rxvml_set_value_cstr(binding_value->attributes[RXVML_ADDRESS_BINDING_VALUE], binding ? binding->value : "");
}

static int rxvml_copy_address_response_updates(value* response_value, const rxvml_address_response* response) {
    value* updated_bindings;
    size_t i;

    if (!response_value || !response) return -1;
    if (response_value->num_attributes <= RXVML_ADDRESS_RESPONSE_UPDATED_BINDINGS) return -1;

    updated_bindings = response_value->attributes[RXVML_ADDRESS_RESPONSE_UPDATED_BINDINGS];
    if (!updated_bindings) return -1;

    if (response->updated_binding_count > 0 && !response->updated_bindings) return -1;

    set_int(response_value->attributes[RXVML_ADDRESS_RESPONSE_UPDATED_BINDING_COUNT],
            (rxinteger)response->updated_binding_count);
    set_num_attributes(updated_bindings, response->updated_binding_count);

    for (i = 0; i < response->updated_binding_count; i++) {
        rxvml_populate_address_binding_value(updated_bindings->attributes[i], &response->updated_bindings[i]);
    }

    return 0;
}

static void rxvml_normalize_address_name(const char* env_name, char* out, size_t out_size) {
    const unsigned char* start;
    const unsigned char* end;
    size_t i = 0;

    if (!out || out_size == 0) return;
    out[0] = 0;
    if (!env_name) return;

    start = (const unsigned char*)env_name;
    while (*start && isspace(*start)) start++;

    end = start + strlen((const char*)start);
    while (end > start && isspace(*(end - 1))) end--;

    while (start < end && i + 1 < out_size) {
        out[i++] = (char)toupper(*start++);
    }
    out[i] = 0;
}

static int rxvml_is_standard_address_sandbox(value* sandbox) {
    if (!sandbox || !sandbox->object_type_name) return 0;
    return sandbox->object_type_name_length == sizeof(RXVML_STANDARD_ADDRESS_SANDBOX_TYPE_NAME) - 1 &&
           memcmp(sandbox->object_type_name,
                  RXVML_STANDARD_ADDRESS_SANDBOX_TYPE_NAME,
                  sandbox->object_type_name_length) == 0;
}

static int rxvml_is_standard_address_stem(value* stem) {
    if (!stem || !stem->object_type_name) return 0;
    return stem->object_type_name_length == sizeof(RXVML_STANDARD_ADDRESS_STEM_TYPE_NAME) - 1 &&
           memcmp(stem->object_type_name,
                  RXVML_STANDARD_ADDRESS_STEM_TYPE_NAME,
                  stem->object_type_name_length) == 0;
}

static void rxvml_normalize_sandbox_key(const char* name, char* out, size_t out_size) {
    rxvml_normalize_address_name(name, out, out_size);
}

static int rxvml_standard_string_map_find(value* sandbox, const char* name) {
    char key[256];
    value* key_count_value;
    value* keys;
    value* present;
    rxinteger key_count;
    rxinteger i;

    if (!rxvml_is_standard_address_sandbox(sandbox) && !rxvml_is_standard_address_stem(sandbox)) return 0;
    if (sandbox->num_attributes < RXVML_STANDARD_ADDRESS_SANDBOX_ATTR_COUNT) return 0;

    key_count_value = sandbox->attributes[RXVML_STANDARD_ADDRESS_SANDBOX_KEY_COUNT];
    keys = sandbox->attributes[RXVML_STANDARD_ADDRESS_SANDBOX_KEYS];
    present = sandbox->attributes[RXVML_STANDARD_ADDRESS_SANDBOX_PRESENT];
    if (!key_count_value || !keys || !present) return 0;

    rxvml_normalize_sandbox_key(name, key, sizeof(key));
    if (key[0] == 0) return 0;

    key_count = key_count_value->int_value;
    if (key_count < 0) return 0;
    if ((size_t)key_count > keys->num_attributes) key_count = (rxinteger)keys->num_attributes;
    if ((size_t)key_count > present->num_attributes) key_count = (rxinteger)present->num_attributes;

    for (i = 1; i <= key_count; i++) {
        value* present_value = present->attributes[i - 1];
        value* key_value = keys->attributes[i - 1];
        if (!present_value || present_value->int_value != 1 || !key_value) continue;
        if (strcmp(rxvml_value_cstr(key_value), key) == 0) return (int)i;
    }

    return 0;
}

static int rxvml_standard_string_map_set(value* map, const char* name, const char* value_text) {
    char key[256];
    value* key_count_value;
    value* keys;
    value* present;
    value* values;
    rxinteger key_count;
    int index;

    if (!rxvml_is_standard_address_sandbox(map) && !rxvml_is_standard_address_stem(map)) return -2;
    if (map->num_attributes < RXVML_STANDARD_ADDRESS_SANDBOX_ATTR_COUNT) return -2;

    key_count_value = map->attributes[RXVML_STANDARD_ADDRESS_SANDBOX_KEY_COUNT];
    keys = map->attributes[RXVML_STANDARD_ADDRESS_SANDBOX_KEYS];
    present = map->attributes[RXVML_STANDARD_ADDRESS_SANDBOX_PRESENT];
    values = map->attributes[RXVML_STANDARD_ADDRESS_SANDBOX_VALUES];
    if (!key_count_value || !keys || !present || !values) return -2;

    rxvml_normalize_sandbox_key(name, key, sizeof(key));
    if (key[0] == 0) return 0;

    index = rxvml_standard_string_map_find(map, key);
    if (index <= 0) {
        key_count = key_count_value->int_value;
        if (key_count < 0) key_count = 0;
        key_count++;
        index = (int)key_count;
        set_int(key_count_value, key_count);
        set_num_attributes(keys, key_count);
        set_num_attributes(present, key_count);
        set_num_attributes(values, key_count);
        rxvml_set_value_cstr(keys->attributes[index - 1], key);
    } else {
        if ((size_t)index > keys->num_attributes) set_num_attributes(keys, (size_t)index);
        if ((size_t)index > present->num_attributes) set_num_attributes(present, (size_t)index);
        if ((size_t)index > values->num_attributes) set_num_attributes(values, (size_t)index);
    }

    set_int(present->attributes[index - 1], 1);
    rxvml_set_value_cstr(values->attributes[index - 1], value_text ? value_text : "");
    return 0;
}

int rxvml_address_sandbox_get(
    const rxvml_address_request* request,
    const char* name,
    char* out_value,
    size_t out_value_len) {

    value* sandbox;
    value* values;
    int index;
    const char* text;

    if (out_value && out_value_len > 0) out_value[0] = 0;
    if (!request || !request->sandbox || !out_value || out_value_len == 0) return -1;

    sandbox = (value*)request->sandbox;
    if (!rxvml_is_standard_address_sandbox(sandbox)) return -2;
    if (sandbox->num_attributes < RXVML_STANDARD_ADDRESS_SANDBOX_ATTR_COUNT) return -2;

    values = sandbox->attributes[RXVML_STANDARD_ADDRESS_SANDBOX_VALUES];
    if (!values) return -2;

    index = rxvml_standard_string_map_find(sandbox, name);
    if (index <= 0 || (size_t)index > values->num_attributes) return 1;

    text = rxvml_value_cstr(values->attributes[index - 1]);
    if (!text) text = "";
    strncpy(out_value, text, out_value_len - 1);
    out_value[out_value_len - 1] = 0;
    return 0;
}

int rxvml_address_sandbox_set(
    rxvml_context* ctx,
    const rxvml_address_request* request,
    const char* name,
    const char* value_text) {

    value* sandbox;
    char class_name[512];
    rxvml_value* name_arg;
    rxvml_value* value_arg;
    rxvml_value* args[2];
    rxvml_value* method_result = NULL;
    int rc;

    if (!ctx || !request || !request->sandbox || !name) {
        if (ctx) ctx->last_error = "Invalid ADDRESS sandbox set arguments";
        return -1;
    }

    sandbox = (value*)request->sandbox;
    if (rxvml_is_standard_address_sandbox(sandbox)) {
        return rxvml_standard_string_map_set(sandbox, name, value_text);
    }

    if (!sandbox->object_type_name || sandbox->object_type_name_length == 0 ||
        sandbox->object_type_name_length >= sizeof(class_name)) {
        ctx->last_error = "ADDRESS sandbox object has no callable class name";
        return -2;
    }

    memcpy(class_name, sandbox->object_type_name, sandbox->object_type_name_length);
    class_name[sandbox->object_type_name_length] = 0;

    name_arg = rxvml_value_new(ctx);
    value_arg = rxvml_value_new(ctx);
    if (!name_arg || !value_arg) {
        if (name_arg) rxvml_value_free(name_arg);
        if (value_arg) rxvml_value_free(value_arg);
        ctx->last_error = "Failed to allocate ADDRESS sandbox set arguments";
        return -3;
    }

    rxvml_set_str(name_arg, name, strlen(name));
    rxvml_set_str(value_arg, value_text ? value_text : "", strlen(value_text ? value_text : ""));

    args[0] = name_arg;
    args[1] = value_arg;
    rc = rxvml_call_method(ctx, (rxvml_value*)sandbox, class_name, "set", 2, args, &method_result);

    if (method_result) rxvml_value_free(method_result);
    rxvml_value_free(name_arg);
    rxvml_value_free(value_arg);
    return rc;
}

int rxvml_address_stem_get(
    const rxvml_value* stem_value,
    const char* name,
    char* out_value,
    size_t out_value_len) {

    value* stem = (value*)stem_value;
    value* values;
    int index;
    const char* text;

    if (out_value && out_value_len > 0) out_value[0] = 0;
    if (!stem || !out_value || out_value_len == 0) return -1;

    if (!rxvml_is_standard_address_stem(stem)) return -2;
    if (stem->num_attributes < RXVML_STANDARD_ADDRESS_SANDBOX_ATTR_COUNT) return -2;

    values = stem->attributes[RXVML_STANDARD_ADDRESS_SANDBOX_VALUES];
    if (!values) return -2;

    index = rxvml_standard_string_map_find(stem, name);
    if (index <= 0 || (size_t)index > values->num_attributes) return 1;

    text = rxvml_value_cstr(values->attributes[index - 1]);
    if (!text) text = "";
    strncpy(out_value, text, out_value_len - 1);
    out_value[out_value_len - 1] = 0;
    return 0;
}

int rxvml_address_stem_set(
    rxvml_context* ctx,
    rxvml_value* stem_value,
    const char* name,
    const char* value_text) {

    value* stem = (value*)stem_value;
    char class_name[512];
    rxvml_value* name_arg;
    rxvml_value* value_arg;
    rxvml_value* args[2];
    rxvml_value* method_result = NULL;
    int rc;

    if (!ctx || !stem || !name) {
        if (ctx) ctx->last_error = "Invalid ADDRESS stem set arguments";
        return -1;
    }

    if (rxvml_is_standard_address_stem(stem)) {
        return rxvml_standard_string_map_set(stem, name, value_text);
    }

    if (!stem->object_type_name || stem->object_type_name_length == 0 ||
        stem->object_type_name_length >= sizeof(class_name)) {
        ctx->last_error = "ADDRESS stem object has no callable class name";
        return -2;
    }

    memcpy(class_name, stem->object_type_name, stem->object_type_name_length);
    class_name[stem->object_type_name_length] = 0;

    name_arg = rxvml_value_new(ctx);
    value_arg = rxvml_value_new(ctx);
    if (!name_arg || !value_arg) {
        if (name_arg) rxvml_value_free(name_arg);
        if (value_arg) rxvml_value_free(value_arg);
        ctx->last_error = "Failed to allocate ADDRESS stem set arguments";
        return -3;
    }

    rxvml_set_str(name_arg, name, strlen(name));
    rxvml_set_str(value_arg, value_text ? value_text : "", strlen(value_text ? value_text : ""));

    args[0] = name_arg;
    args[1] = value_arg;
    rc = rxvml_call_method(ctx, (rxvml_value*)stem, class_name, "set", 2, args, &method_result);

    if (method_result) rxvml_value_free(method_result);
    rxvml_value_free(name_arg);
    rxvml_value_free(value_arg);

    return rc;
}

int rxvml_address_binding_stem_get(
    const rxvml_address_binding* binding,
    const char* name,
    char* out_value,
    size_t out_value_len) {

    if (!binding || !binding->value_object) {
        if (out_value && out_value_len > 0) out_value[0] = 0;
        return -1;
    }
    return rxvml_address_stem_get(binding->value_object, name, out_value, out_value_len);
}

int rxvml_address_binding_stem_set(
    rxvml_context* ctx,
    const rxvml_address_binding* binding,
    const char* name,
    const char* value_text) {

    if (!binding || !binding->value_object) {
        if (ctx) ctx->last_error = "ADDRESS binding has no stem object";
        return -1;
    }
    return rxvml_address_stem_set(ctx, binding->value_object, name, value_text);
}

static void rxvml_reset_native_signal(rxpa_attribute_value signal) {
    value* signal_value = (value*)signal;
    if (!signal_value) return;
    set_int(signal_value, SIGNAL_NONE);
    set_null_string(signal_value, "");
}

static void rxvml_set_native_failure(rxpa_attribute_value signal, int code, const char* message) {
    value* signal_value = (value*)signal;
    if (!signal_value) return;
    set_int(signal_value, code);
    set_null_string(signal_value, message ? message : "");
}

static rxvml_address_callback_entry* rxvml_get_address_callback(
    rxvml_context* ctx,
    int handle) {

    if (!ctx || handle <= 0 || handle > (int)ctx->address_callback_size) return NULL;
    if (!ctx->address_callbacks[handle - 1].callback) return NULL;
    return &ctx->address_callbacks[handle - 1];
}

static rxvml_address_callback_entry* rxvml_get_address_callback_by_name(
    rxvml_context* ctx,
    const char* env_name,
    int* out_handle) {

    char normalized[256];
    size_t i;

    if (out_handle) *out_handle = 0;
    if (!ctx || !env_name) return NULL;

    rxvml_normalize_address_name(env_name, normalized, sizeof(normalized));
    if (normalized[0] == 0) return NULL;

    for (i = 0; i < ctx->address_callback_size; i++) {
        if (ctx->address_callbacks[i].callback &&
            strcmp(ctx->address_callbacks[i].env_name, normalized) == 0) {
            if (out_handle) *out_handle = (int)i + 1;
            return &ctx->address_callbacks[i];
        }
    }

    return NULL;
}

static void rxvml_free_address_callback_handle(rxvml_context* ctx, int handle) {
    rxvml_address_callback_entry* entry = rxvml_get_address_callback(ctx, handle);
    if (!entry) return;
    entry->env_name[0] = 0;
    entry->callback = NULL;
    entry->userdata = NULL;
}

static int rxvml_alloc_address_callback_handle(
    rxvml_context* ctx,
    const char* env_name,
    rxvml_address_callback callback,
    void* userdata) {

    char normalized[256];
    int existing_handle = 0;
    size_t i;

    if (!ctx || !env_name || !callback) return 0;

    rxvml_normalize_address_name(env_name, normalized, sizeof(normalized));
    if (normalized[0] == 0) return 0;

    if (rxvml_get_address_callback_by_name(ctx, normalized, &existing_handle)) {
        ctx->address_callbacks[existing_handle - 1].callback = callback;
        ctx->address_callbacks[existing_handle - 1].userdata = userdata;
        return existing_handle;
    }

    for (i = 0; i < ctx->address_callback_size; i++) {
        if (!ctx->address_callbacks[i].callback) {
            strncpy(ctx->address_callbacks[i].env_name, normalized, sizeof(ctx->address_callbacks[i].env_name) - 1);
            ctx->address_callbacks[i].env_name[sizeof(ctx->address_callbacks[i].env_name) - 1] = 0;
            ctx->address_callbacks[i].callback = callback;
            ctx->address_callbacks[i].userdata = userdata;
            return (int)i + 1;
        }
    }

    {
        size_t old_size = ctx->address_callback_size;
        size_t new_size = old_size == 0 ? 8 : old_size * 2;
        rxvml_address_callback_entry* new_callbacks =
            realloc(ctx->address_callbacks, sizeof(rxvml_address_callback_entry) * new_size);
        if (!new_callbacks) return 0;

        ctx->address_callbacks = new_callbacks;
        for (i = old_size; i < new_size; i++) {
            ctx->address_callbacks[i].env_name[0] = 0;
            ctx->address_callbacks[i].callback = NULL;
            ctx->address_callbacks[i].userdata = NULL;
        }

        strncpy(ctx->address_callbacks[old_size].env_name, normalized, sizeof(ctx->address_callbacks[old_size].env_name) - 1);
        ctx->address_callbacks[old_size].env_name[sizeof(ctx->address_callbacks[old_size].env_name) - 1] = 0;
        ctx->address_callbacks[old_size].callback = callback;
        ctx->address_callbacks[old_size].userdata = userdata;
        ctx->address_callback_size = new_size;
        return (int)old_size + 1;
    }
}

static void rxvml_native_address_execute(
    rxinteger argc,
    rxpa_attribute_value* args,
    rxpa_attribute_value ret,
    rxpa_attribute_value signal) {

    rxvml_context* ctx = rxvml_active_context;
    rxvml_address_callback_entry* entry;
    rxvml_address_request request;
    rxvml_address_response response;
    rxvml_address_binding* bindings = NULL;
    value* request_value;
    value* response_value;
    value* binding_array;
    int handle;
    int callback_rc;
    size_t binding_count;
    size_t i;

    rxvml_reset_native_signal(signal);
    if (ret) set_int((value*)ret, -1);

    if (argc != 3 || !args) {
        rxvml_set_native_failure(signal, SIGNAL_INVALID_ARGUMENTS, "native ADDRESS bridge expected handle, request, and response");
        return;
    }

    if (!ctx) {
        rxvml_set_native_failure(signal, SIGNAL_FAILURE, "native ADDRESS bridge has no active rxvml context");
        return;
    }

    handle = (int)((value*)args[0])->int_value;
    entry = rxvml_get_address_callback(ctx, handle);
    if (!entry) {
        rxvml_set_native_failure(signal, SIGNAL_FAILURE, "native ADDRESS callback handle is not registered");
        return;
    }

    request_value = (value*)args[1];
    response_value = (value*)args[2];
    if (!request_value || request_value->num_attributes < RXVML_ADDRESS_REQUEST_ATTR_COUNT) {
        rxvml_set_native_failure(signal, SIGNAL_INVALID_ARGUMENTS, "native ADDRESS request object is malformed");
        return;
    }

    if (request_value->attributes[RXVML_ADDRESS_REQUEST_BINDING_COUNT]->int_value < 0) {
        rxvml_set_native_failure(signal, SIGNAL_INVALID_ARGUMENTS, "native ADDRESS request binding count is negative");
        return;
    }

    /*
     * Level B object attributes are emitted in metadata order, not source
     * declaration order. These indexes match _rxsysb.addressrequest and
     * _rxsysb.addressbinding metadata from lib/rxfnsb/rexx/_address.rexx.
     */
    binding_count = (size_t)request_value->attributes[RXVML_ADDRESS_REQUEST_BINDING_COUNT]->int_value;
    binding_array = request_value->attributes[RXVML_ADDRESS_REQUEST_BINDINGS];
    if (!binding_array && binding_count > 0) {
        rxvml_set_native_failure(signal, SIGNAL_INVALID_ARGUMENTS, "native ADDRESS request bindings are malformed");
        return;
    }
    if (binding_array && binding_count > binding_array->num_attributes) {
        binding_count = binding_array->num_attributes;
    }

    if (binding_count > 0) {
        bindings = calloc(binding_count, sizeof(rxvml_address_binding));
        if (!bindings) {
            rxvml_set_native_failure(signal, SIGNAL_FAILURE, "failed to allocate native ADDRESS binding view");
            return;
        }

        for (i = 0; i < binding_count; i++) {
            value* binding = binding_array->attributes[i];
            if (binding && binding->num_attributes >= 5) {
                bindings[i].external_alias = rxvml_value_cstr(binding->attributes[RXVML_ADDRESS_BINDING_EXTERNAL_ALIAS]);
                bindings[i].flags = rxvml_value_cstr(binding->attributes[RXVML_ADDRESS_BINDING_FLAGS]);
                bindings[i].internal_name = rxvml_value_cstr(binding->attributes[RXVML_ADDRESS_BINDING_INTERNAL_NAME]);
                bindings[i].kind = rxvml_value_cstr(binding->attributes[RXVML_ADDRESS_BINDING_KIND]);
                bindings[i].value = rxvml_value_cstr(binding->attributes[RXVML_ADDRESS_BINDING_VALUE]);
                if (binding->num_attributes > RXVML_ADDRESS_BINDING_VALUE_OBJECT) {
                    bindings[i].value_object = (rxvml_value*)binding->attributes[RXVML_ADDRESS_BINDING_VALUE_OBJECT];
                }
            }
        }
    }

    request.command = rxvml_value_cstr(request_value->attributes[RXVML_ADDRESS_REQUEST_COMMAND]);
    request.environment_name = rxvml_value_cstr(request_value->attributes[RXVML_ADDRESS_REQUEST_ENVIRONMENT_NAME]);
    request.binding_count = binding_count;
    request.bindings = bindings;
    request.sandbox = (rxvml_value*)request_value->attributes[RXVML_ADDRESS_REQUEST_SANDBOX];

    response.rc = 0;
    response.condition_name = NULL;
    response.diagnostic = NULL;
    response.updated_binding_count = 0;
    response.updated_bindings = NULL;

    callback_rc = entry->callback(ctx, &request, &response, entry->userdata);
    if (callback_rc != 0 && response.rc == 0) response.rc = callback_rc;

    if (rxvml_copy_address_response_updates(response_value, &response) != 0) {
        rxvml_set_native_failure(signal, SIGNAL_FAILURE, "failed to copy native ADDRESS response updates");
        if (bindings) free(bindings);
        return;
    }

    if (ret) set_int((value*)ret, (rxinteger)response.rc);
    if (bindings) free(bindings);
}

static void rxvml_native_address_match(
    rxinteger argc,
    rxpa_attribute_value* args,
    rxpa_attribute_value ret,
    rxpa_attribute_value signal) {

    rxvml_context* ctx = rxvml_active_context;
    const char* env_name;

    rxvml_reset_native_signal(signal);
    if (ret) set_int((value*)ret, 0);

    if (argc != 1 || !args) return;
    if (!ctx) return;

    env_name = rxvml_value_cstr((value*)args[0]);
    if (rxvml_get_address_callback_by_name(ctx, env_name, NULL)) {
        if (ret) set_int((value*)ret, 1000);
    }
}

static void rxvml_native_address_handle(
    rxinteger argc,
    rxpa_attribute_value* args,
    rxpa_attribute_value ret,
    rxpa_attribute_value signal) {

    rxvml_context* ctx = rxvml_active_context;
    const char* env_name;
    int handle = 0;

    rxvml_reset_native_signal(signal);
    if (ret) set_int((value*)ret, 0);

    if (argc != 1 || !args) return;
    if (!ctx) return;

    env_name = rxvml_value_cstr((value*)args[0]);
    rxvml_get_address_callback_by_name(ctx, env_name, &handle);
    if (ret) set_int((value*)ret, handle);
}

rxvml_context* rxvml_create(const char* location, unsigned flags) {
    rxvml_context* ctx;

    /* Initialize mandatory plugins */
    CALL_PLUGIN_INITIALIZER(decnumber);

    ctx = malloc(sizeof(rxvml_context));
    if (!ctx) return NULL;
    rxinimod(&ctx->vm);
    if (location) ctx->vm.location = strdup(location);
    ctx->vm.debug_mode = flags;
    ctx->last_error = NULL;
    ctx->registry = NULL;
    ctx->registry_size = 0;
    ctx->address_callbacks = NULL;
    ctx->address_callback_size = 0;

    rxvm_addfunc(rxvml_native_address_execute, "_rxsysb._native_address_execute", 0, 0, 0);
    rxvm_addfunc(rxvml_native_address_match, "_rxsysb._native_address_match", 0, 0, 0);
    rxvm_addfunc(rxvml_native_address_handle, "_rxsysb._native_address_handle", 0, 0, 0);
    if (rxldmodp(&ctx->vm) == -1) {
        rxfremod(&ctx->vm);
        free(ctx);
        return NULL;
    }

    return ctx;
}

void rxvml_destroy(rxvml_context* ctx) {
    size_t i;
    if (!ctx) return;
    if (ctx->registry) {
        for (i = 0; i < ctx->registry_size; i++) {
            if (ctx->registry[i].obj) {
                clear_value(ctx->registry[i].obj);
                free(ctx->registry[i].obj);
            }
        }
        free(ctx->registry);
    }
    if (ctx->address_callbacks) free(ctx->address_callbacks);
    rxfremod(&ctx->vm);
    free(ctx);
}

int rxvml_load_module_file(rxvml_context* ctx, const char* rxbin_path) {
    return rxldmod(&ctx->vm, (char*)rxbin_path);
}

int rxvml_load_module_buffer(rxvml_context* ctx, const void* buf, size_t len) {
    return rxldmodm(&ctx->vm, (char*)buf, len);
}

rxvml_value* rxvml_value_new(rxvml_context* ctx) {
    return (rxvml_value*)value_f();
}

void rxvml_set_int(rxvml_value* v, rxinteger i) {
    set_int((value*)v, i);
}

void rxvml_set_str(rxvml_value* v, const char* s, size_t len) {
    if (s) set_string((value*)v, (char*)s, len);
    else set_null_string((value*)v, "");
}

int rxvml_set_native_payload(rxvml_value* v, const void* payload, size_t len,
                             const rxvm_native_payload_ops* ops, unsigned int flags) {
    return set_native_payload((value*)v, payload, len, ops, flags);
}

void* rxvml_get_native_payload(rxvml_value* v, size_t* out_len,
                               const rxvm_native_payload_ops** out_ops,
                               unsigned int* out_flags) {
    return get_native_payload((value*)v, out_len, out_ops, out_flags);
}

rxvml_value* rxvml_object_new(rxvml_context* ctx, size_t num_attrs) {
    value* v = value_f();
    set_num_attributes(v, num_attrs);
    return (rxvml_value*)v;
}

int rxvml_set_attribute(rxvml_context* ctx, rxvml_value* obj, size_t index1, rxvml_value* val) {
    value* v_obj = (value*)obj;
    value* v_val = (value*)val;
    if (index1 == 0 || index1 > v_obj->num_attributes) return -1;
    copy_value(v_obj->attributes[index1 - 1], v_val);
    return 0;
}

rxvml_value* rxvml_get_attribute(rxvml_context* ctx, rxvml_value* obj, size_t index1) {
    value* v_obj = (value*)obj;
    if (index1 == 0 || index1 > v_obj->num_attributes) return NULL;
    value* v_ret = value_f();
    copy_value(v_ret, v_obj->attributes[index1 - 1]);
    return (rxvml_value*)v_ret;
}

rxvml_value* rxvml_array_new(rxvml_context* ctx, size_t length) {
    return rxvml_object_new(ctx, length);
}

int rxvml_array_set(rxvml_context* ctx, rxvml_value* arr, size_t index1, rxvml_value* elem) {
    value* v_arr = (value*)arr;
    value* v_elem = (value*)elem;
    if (index1 == 0 || index1 > v_arr->num_attributes) return -1;
    /* In cREXX, attributes are 0-indexed in C, 1-indexed in Rexx */
    /* link_attribute might be better, but for now we just copy the value pointer or copy the value */
    /* Actually, attributes[i] is a pointer to a value */
    /* Copy into the pre-allocated attribute slot to avoid pointer aliasing */
    copy_value(v_arr->attributes[index1 - 1], v_elem);
    return 0;
}

void rxvml_value_free(rxvml_value* v) {
    if (v) {
        clear_value((value*)v);
        free(v);
    }
}


int rxvml_reg_alloc(rxvml_context* ctx, rxvml_value* v, const char* class_name) {
    size_t i;
    if (!v) return -1;
    for (i = 0; i < ctx->registry_size; i++) {
        if (ctx->registry[i].obj == NULL) {
            ctx->registry[i].obj = (value*)v;
            if (class_name) strncpy(ctx->registry[i].class_name, class_name, 255);
            else ctx->registry[i].class_name[0] = 0;
            ctx->registry[i].class_name[255] = 0;
            return (int)i;
        }
    }
    {
        size_t new_size = ctx->registry_size == 0 ? 8 : ctx->registry_size * 2;
        int idx;
        rxvml_registry_entry* new_registry = realloc(ctx->registry, sizeof(rxvml_registry_entry) * new_size);
        if (!new_registry) return -1;
        ctx->registry = new_registry;
        for (i = ctx->registry_size; i < new_size; i++) {
            ctx->registry[i].obj = NULL;
            ctx->registry[i].class_name[0] = 0;
        }
        idx = (int)ctx->registry_size;
        ctx->registry[idx].obj = (value*)v;
        if (class_name) strncpy(ctx->registry[idx].class_name, class_name, 255);
        else ctx->registry[idx].class_name[0] = 0;
        ctx->registry[idx].class_name[255] = 0;
        ctx->registry_size = new_size;
        return idx;
    }
}

void rxvml_reg_free(rxvml_context* ctx, int reg_idx) {
    if (reg_idx >= 0 && reg_idx < (int)ctx->registry_size) {
        if (ctx->registry[reg_idx].obj) {
            clear_value(ctx->registry[reg_idx].obj);
            free(ctx->registry[reg_idx].obj);
            ctx->registry[reg_idx].obj = NULL;
            ctx->registry[reg_idx].class_name[0] = 0;
        }
    }
}

rxvml_value* rxvml_reg_get(rxvml_context* ctx, int reg_idx, char* out_class_name) {
    if (reg_idx >= 0 && reg_idx < (int)ctx->registry_size) {
        if (out_class_name) strcpy(out_class_name, ctx->registry[reg_idx].class_name);
        return (rxvml_value*)ctx->registry[reg_idx].obj;
    }
    return NULL;
}

int rxvml_discover_classes(rxvml_context* ctx, const char* ns, rxvml_class_info** out_classes, size_t* out_count) {
    size_t count = 0;
    size_t capacity = 8;
    rxvml_class_info* classes = malloc(sizeof(rxvml_class_info) * capacity);
    size_t ns_len = strlen(ns);
    size_t m;

    for (m = 0; m < ctx->vm.num_modules; m++) {
        module* mod = ctx->vm.modules[m];
        if (mod->meta_head == -1) continue;

        {
            int meta_off = mod->meta_head;
            while (meta_off != -1) {
                chameleon_constant* c = (chameleon_constant*)(mod->segment.const_pool + meta_off);
                if (c->type == META_CLASS) {
                    meta_class_constant* mc = (meta_class_constant*)c;
                    string_constant* sc = (string_constant*)(mod->segment.const_pool + mc->symbol);
                    
                    if (strncmp(sc->string, ns, ns_len) == 0 && sc->string[ns_len] == '.') {
                        if (count >= capacity) {
                            capacity *= 2;
                            classes = realloc(classes, sizeof(rxvml_class_info) * capacity);
                        }
                        strncpy(classes[count].class_name, sc->string, 255);
                        classes[count].class_name[255] = 0;
                        
                        /* Factory name pattern: namespace.classname.§factory */
                        snprintf(classes[count].factory_proc, 511, "%s.§factory", sc->string);
                        classes[count].factory_proc[511] = 0;
                        
                        count++;
                    }
                }
                meta_off = ((meta_entry*)c)->next;
            }
        }
    }

    *out_classes = classes;
    *out_count = count;
    return 0;
}

int rxvml_call_procedure(
    rxvml_context* ctx,
    const char* proc_name,
    size_t argc,
    rxvml_value** args,
    rxvml_value** response_out) {

    if (ctx->vm.num_modules > 0) {
        rxvm_link(&ctx->vm);
    }
    proc_runtime* p = find_procedure(&ctx->vm, proc_name);
    if (!p) {
        ctx->last_error = "Procedure not found";
        return -1;
    }

    return rxvml_invoke_external_proc(ctx, p, argc, args, "rxc_bridge_proc", response_out);
}

static proc_runtime* rxvml_find_last_module_procedure(rxvm_context* vm, const char* name) {
    size_t mod_index;

    if (!vm || !name) return NULL;
    for (mod_index = vm->num_modules; mod_index > 0; mod_index--) {
        module* mod = vm->modules[mod_index - 1];
        int i = mod ? mod->proc_head : -1;
        while (i != -1) {
            proc_constant* definition = (proc_constant*)(mod->segment.const_pool + i);
            if (definition->base.type == PROC_CONST && strcmp(definition->name, name) == 0) {
                return mod->proc_runtime_lookup[(size_t)i >> 3];
            }
            i = definition->next;
        }
    }
    return NULL;
}

int rxvml_run(
    rxvml_context* ctx,
    int argc,
    const char** argv,
    int* program_rc) {

    proc_runtime* main_proc;
    rxvml_value* arg_array = NULL;
    rxvml_value* arg_values[1];
    rxvml_value* result = NULL;
    rxinteger int_result = 0;
    int i;
    int rc = -1;

    if (program_rc) *program_rc = 0;
    if (!ctx || argc < 0) return -1;
    if (argc > 0 && !argv) {
        ctx->last_error = "Missing rxvml run argument vector";
        return -1;
    }

    arg_array = rxvml_array_new(ctx, (size_t)argc);
    if (!arg_array) {
        ctx->last_error = "Failed to allocate rxvml run argument array";
        goto cleanup;
    }
    for (i = 0; i < argc; i++) {
        rxvml_value* arg_value = rxvml_value_new(ctx);
        if (!arg_value) {
            ctx->last_error = "Failed to allocate rxvml run argument";
            goto cleanup;
        }
        rxvml_set_str(arg_value, argv[i] ? argv[i] : "", strlen(argv[i] ? argv[i] : ""));
        if (rxvml_array_set(ctx, arg_array, (size_t)i + 1, arg_value) != 0) {
            rxvml_value_free(arg_value);
            ctx->last_error = "Failed to set rxvml run argument";
            goto cleanup;
        }
        rxvml_value_free(arg_value);
    }

    if (ctx->vm.num_modules > 0) {
        rxvm_link(&ctx->vm);
    }
    main_proc = rxvml_find_last_module_procedure(&ctx->vm, "main");
    if (!main_proc) {
        ctx->last_error = "Program main procedure not found";
        goto cleanup;
    }

    arg_values[0] = arg_array;
    if (rxvml_invoke_external_proc(ctx, main_proc, 1, arg_values, "rxvml_run", &result) != 0) {
        goto cleanup;
    }

    if (result && rxvml_to_int(ctx, result, &int_result) == 0 && program_rc) {
        *program_rc = (int)int_result;
    }
    rc = 0;

cleanup:
    if (result) rxvml_value_free(result);
    if (arg_array) rxvml_value_free(arg_array);
    return rc;
}

int rxvml_call_method(
    rxvml_context* ctx,
    rxvml_value* obj,
    const char* class_name,
    const char* method_name,
    size_t argc,
    rxvml_value** args,
    rxvml_value** response_out) {

    char full_method_name[1024];
    snprintf(full_method_name, sizeof(full_method_name), "§%s.%s", class_name, method_name);

    if (ctx->vm.num_modules > 0) {
        rxvm_link(&ctx->vm);
    }
    proc_runtime* p = find_procedure(&ctx->vm, full_method_name);
    if (!p) {
        snprintf(full_method_name, sizeof(full_method_name), "%s.%s", class_name, method_name);
        p = find_procedure(&ctx->vm, full_method_name);
    }
    if (!p) {
        snprintf(full_method_name, sizeof(full_method_name), "%s.§%s", class_name, method_name);
        p = find_procedure(&ctx->vm, full_method_name);
    }
    if (!p) {
        snprintf(full_method_name, sizeof(full_method_name), "§%s.§%s", class_name, method_name);
        p = find_procedure(&ctx->vm, full_method_name);
    }
    if (!p) {
        ctx->last_error = "Method not found";
        return -1;
    }

    {
        rxvml_value** method_args = malloc(sizeof(rxvml_value*) * (argc + 1));
        int rc;
        size_t i;

        if (!method_args) {
            ctx->last_error = "Failed to allocate rxvml method arguments";
            return -1;
        }

        method_args[0] = obj;
        for (i = 0; i < argc; i++) method_args[i + 1] = args[i];

        rc = rxvml_invoke_external_proc(
            ctx,
            p,
            argc + 1,
            method_args,
            "rxc_plugin_method",
            response_out);

        free(method_args);
        return rc;
    }
}

int rxvml_call_factory(
    rxvml_context* ctx,
    const char* class_name,
    size_t argc,
    rxvml_value** args,
    rxvml_value** response_out) {

    char factory_proc[1024];
    snprintf(factory_proc, sizeof(factory_proc), "§%s.§factory", class_name);

    int rc = rxvml_call_procedure(ctx, factory_proc, argc, args, response_out);
    if (rc != 0) {
        snprintf(factory_proc, sizeof(factory_proc), "%s.§factory", class_name);
        rc = rxvml_call_procedure(ctx, factory_proc, argc, args, response_out);
    }
    return rc;
}

static int rxvml_call_int_procedure(
    rxvml_context* ctx,
    const char* proc_name,
    size_t argc,
    rxvml_value** args,
    int* out_rc) {

    rxvml_value* response = NULL;
    rxinteger rc_value = 0;

    if (!ctx || !proc_name) return -1;

    if (rxvml_call_procedure(ctx, proc_name, argc, args, &response) != 0) {
        return -1;
    }

    if (!response) {
        ctx->last_error = "Procedure returned NULL";
        return -1;
    }

    if (rxvml_to_int(ctx, response, &rc_value) != 0) {
        ctx->last_error = "Procedure did not return an integer";
        rxvml_value_free(response);
        return -1;
    }

    if (out_rc) *out_rc = (int)rc_value;
    rxvml_value_free(response);
    return 0;
}

int rxvml_address_register_environment(
    rxvml_context* ctx,
    const char* env_name,
    rxvml_value* env_obj) {

    rxvml_value* name_arg;
    rxvml_value* args[2];
    int rc_value = 0;

    if (!ctx || !env_name || !env_obj) {
        if (ctx) ctx->last_error = "Invalid address registration arguments";
        return -1;
    }

    name_arg = rxvml_value_new(ctx);
    if (!name_arg) {
        ctx->last_error = "Failed to allocate address registration argument";
        return -1;
    }

    rxvml_set_str(name_arg, env_name, strlen(env_name));
    args[0] = name_arg;
    args[1] = env_obj;

    if (rxvml_call_int_procedure(ctx, "_rxsysb._register_address_environment", 2, args, &rc_value) != 0) {
        rxvml_value_free(name_arg);
        return -1;
    }

    rxvml_value_free(name_arg);
    return rc_value;
}

int rxvml_address_register_callback_environment(
    rxvml_context* ctx,
    const char* env_name,
    rxvml_address_callback callback,
    void* userdata) {

    rxvml_value* env_obj = NULL;
    int handle;
    int enable_rc = 0;
    int register_rc;

    if (!ctx || !env_name || !callback) {
        if (ctx) ctx->last_error = "Invalid native address callback registration arguments";
        return -1;
    }

    handle = rxvml_alloc_address_callback_handle(ctx, env_name, callback, userdata);
    if (handle == 0) {
        ctx->last_error = "Failed to allocate native address callback handle";
        return -1;
    }

    if (rxvml_call_int_procedure(ctx, "_rxsysb._enable_native_address_environment", 0, NULL, &enable_rc) != 0 ||
        enable_rc != 0) {
        rxvml_free_address_callback_handle(ctx, handle);
        if (!ctx->last_error) ctx->last_error = "Failed to enable native address environment provider";
        return -1;
    }

    if (rxvml_address_create_environment(ctx, env_name, &env_obj) != 0 || !env_obj) {
        rxvml_free_address_callback_handle(ctx, handle);
        if (!ctx->last_error) ctx->last_error = "Failed to materialize native address environment provider";
        return -1;
    }

    register_rc = rxvml_address_register_environment(ctx, env_name, env_obj);
    rxvml_value_free(env_obj);

    if (register_rc != 0) {
        rxvml_free_address_callback_handle(ctx, handle);
    }

    return register_rc;
}

int rxvml_address_create_environment(
    rxvml_context* ctx,
    const char* env_name,
    rxvml_value** env_obj_out) {

    rxvml_value* name_arg;
    rxvml_value* args[1];

    if (env_obj_out) *env_obj_out = NULL;

    if (!ctx || !env_name || !env_obj_out) {
        if (ctx) ctx->last_error = "Invalid address environment factory arguments";
        return -1;
    }

    name_arg = rxvml_value_new(ctx);
    if (!name_arg) {
        ctx->last_error = "Failed to allocate address environment factory argument";
        return -1;
    }

    rxvml_set_str(name_arg, env_name, strlen(env_name));
    args[0] = name_arg;

    if (rxvml_call_procedure(ctx, RXVML_ADDRESS_ENVIRONMENT_FACTORY_PROC, 1, args, env_obj_out) != 0 || !*env_obj_out) {
        rxvml_value_free(name_arg);
        if (!ctx->last_error) ctx->last_error = "Failed to create address environment via interface factory";
        return -1;
    }

    rxvml_value_free(name_arg);
    return 0;
}

int rxvml_address_set_environment(
    rxvml_context* ctx,
    const char* env_name) {

    rxvml_value* name_arg;
    rxvml_value* args[1];
    int rc_value = 0;

    if (!ctx || !env_name) {
        if (ctx) ctx->last_error = "Invalid address environment name";
        return -1;
    }

    name_arg = rxvml_value_new(ctx);
    if (!name_arg) {
        ctx->last_error = "Failed to allocate address environment argument";
        return -1;
    }

    rxvml_set_str(name_arg, env_name, strlen(env_name));
    args[0] = name_arg;

    if (rxvml_call_int_procedure(ctx, "_rxsysb._set_address_environment", 1, args, &rc_value) != 0) {
        rxvml_value_free(name_arg);
        return -1;
    }

    rxvml_value_free(name_arg);
    return rc_value;
}


int rxvml_to_int(rxvml_context* ctx, const rxvml_value* v, rxinteger* out_v) {
    value* val = (value*)v;
    if (val) {
        if (out_v) *out_v = val->int_value;
        return 0;
    }
    return -1;
}

int rxvml_to_str(rxvml_context* ctx, const rxvml_value* v, const char** out_s, size_t* out_len) {
    value* val = (value*)v;
    if (val && val->string_value) {
        null_terminate_string_buffer(val);
        if (out_s) *out_s = val->string_value;
        if (out_len) *out_len = val->string_length;
        return 0;
    }
    return -1;
}

size_t rxvml_num_attributes(const rxvml_value* v) {
    const value* val = (const value*)v;
    if (!val) return 0;
    return val->num_attributes;
}

int rxvml_last_error(rxvml_context* ctx, const char** out_msg) {
    if (out_msg) *out_msg = ctx->last_error;
    return ctx->last_error ? -1 : 0;
}

unsigned int rxvml_get_debug_mode(rxvml_context* ctx) {
    if (!ctx) return 0;
    return ctx->vm.debug_mode;
}

void rxvml_set_say_exit(rxvml_say_exit_func say_exit) {
    rxvm_setsayexit((say_exit_func)say_exit);
}
