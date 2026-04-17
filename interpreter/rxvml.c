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

struct rxvml_context {
    rxvm_context vm;
    const char* last_error;
    rxvml_registry_entry* registry;
    size_t registry_size;
};

/* rxvml_value is defined as an alias to value in the public header */

/* Internal helper to find a procedure by name */
static proc_constant* find_procedure(rxvm_context* ctx, const char* name) {
    proc_constant* p = NULL;
    if (src_node(ctx->exposed_proc_tree, (char*)name, (size_t*)&p)) {
        return p;
    }
    return NULL;
}

int rxvm_link(rxvm_context* ctx);

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
    proc_constant* p = find_procedure(&ctx->vm, proc_name);
    if (!p) {
        ctx->last_error = "Procedure not found";
        return -1;
    }

    /* Set up the external call context */
    ctx->vm.ext_proc = p;
    ctx->vm.ext_argc = argc;
    if (argc > 0) {
        ctx->vm.ext_args = malloc(sizeof(value*) * argc);
        for (size_t i = 0; i < argc; i++) {
            ctx->vm.ext_args[i] = (value*)args[i];
        }
    } else {
        ctx->vm.ext_args = NULL;
    }

    ctx->vm.ext_ret = value_f();

    /* Run the VM */
    {
        char* dummy_argv[] = {"rxc_bridge_proc"};
        rxvm_prepare(&ctx->vm);
        run(&ctx->vm, 0, dummy_argv);
    }

    if (response_out) {
        *response_out = (rxvml_value*)ctx->vm.ext_ret;
    } else {
        rxvml_value_free((rxvml_value*)ctx->vm.ext_ret);
    }

    /* Clear the ext call fields */
    ctx->vm.ext_proc = 0;
    ctx->vm.ext_argc = 0;
    if (ctx->vm.ext_args) free(ctx->vm.ext_args);
    ctx->vm.ext_args = 0;
    ctx->vm.ext_ret = 0;

    return 0;
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
    proc_constant* p = find_procedure(&ctx->vm, full_method_name);
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

    /* Set up the external call context */
    ctx->vm.ext_proc = p;
    ctx->vm.ext_argc = argc + 1;
    ctx->vm.ext_args = malloc(sizeof(value*) * (argc + 1));
    ctx->vm.ext_args[0] = (value*)obj;
    for (size_t i = 0; i < argc; i++) {
        ctx->vm.ext_args[i + 1] = (value*)args[i];
    }

    ctx->vm.ext_ret = value_f();

    /* Run the VM */
    {
        char* dummy_argv[] = {"rxc_plugin_method"};
        rxvm_prepare(&ctx->vm);
        run(&ctx->vm, 0, dummy_argv);
    }

    if (response_out) {
        *response_out = (rxvml_value*)ctx->vm.ext_ret;
    } else {
        rxvml_value_free((rxvml_value*)ctx->vm.ext_ret);
    }

    /* Clear the ext call fields */
    ctx->vm.ext_proc = 0;
    ctx->vm.ext_argc = 0;
    if (ctx->vm.ext_args) free(ctx->vm.ext_args);
    ctx->vm.ext_args = 0;
    ctx->vm.ext_ret = 0;

    return 0;
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
