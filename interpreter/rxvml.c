#include <stdlib.h>
#include <string.h>
#include "rxvml.h"
#include "rxvmintp.h"
#include "rxvm.h"
#include "rxvmvars.h"
#include "rxastree.h"
#include "rxvmplugin.h"

struct rxvml_context {
    rxvm_context vm;
    const char* last_error;
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
    return ctx;
}

void rxvml_destroy(rxvml_context* ctx) {
    if (!ctx) return;
    rxfremod(&ctx->vm);
    free(ctx);
}

int rxvml_load_module_file(rxvml_context* ctx, const char* rxbin_path) {
    return rxldmod(&ctx->vm, (char*)rxbin_path);
}

int rxvml_load_module_buffer(rxvml_context* ctx, const void* buf, size_t len) {
    return rxldmodm(&ctx->vm, (char*)buf, len);
}

rxvml_value* rxvml_array_new(rxvml_context* ctx, size_t length) {
    value* v = value_f();
    set_num_attributes(v, length);
    v->status.type_object = 1;
    return (rxvml_value*)v;
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

rxvml_value* rxvml_make_token(rxvml_context* ctx, const rxvml_token_desc* d) {
    value* v = value_f();
    /* Attributes: 1..12 per reasoning note */
    set_num_attributes(v, 15);
    v->status.type_object = 1;

    /* 1. type */
    set_int(v->attributes[0], d->type);
    /* 2. subtype */
    set_int(v->attributes[1], d->subtype);
    /* 3. text */
    if (d->text) set_string(v->attributes[2], (char*)d->text, d->text_len);
    else set_null_string(v->attributes[2], "");
    /* 4. line */
    set_int(v->attributes[3], d->line);
    /* 5. column */
    set_int(v->attributes[4], d->column);
    /* 6. length */
    set_int(v->attributes[5], d->length);
    /* 7. file */
    if (d->file) set_null_string(v->attributes[6], d->file);
    else set_null_string(v->attributes[6], "");
    /* 8. nodeType */
    set_int(v->attributes[7], d->node_type);
    /* 9. ordinalLow */
    set_int(v->attributes[8], d->ord_low);
    /* 10. ordinalHigh */
    set_int(v->attributes[9], d->ord_high);
    /* 11. symbolName */
    if (d->sym_name) set_null_string(v->attributes[10], d->sym_name);
    else set_null_string(v->attributes[10], "");
    /* 12. symbolType */
    set_int(v->attributes[11], d->sym_type);
    /* 13. isUpdated (negotiation) */
    set_int(v->attributes[12], 0);
    /* 14. requestedTokenType */
    set_int(v->attributes[13], 0);
    /* 15. requestedSymbolType */
    set_int(v->attributes[14], 0);

    return (rxvml_value*)v;
}

int rxvml_call_plugin(
    rxvml_context* ctx,
    const char* proc_name,
    rxvml_value* token_array,
    rxvml_value** response_out) {

    if (ctx->vm.num_modules > 0) {
        rxvm_link(&ctx->vm);
    }
    proc_constant* p = find_procedure(&ctx->vm, proc_name);
    if (!p) {
        if (ctx->vm.debug_mode) fprintf(stderr, "DEBUG_EXIT: Procedure %s not found in bridge VM (%zu modules)\n", proc_name, ctx->vm.num_modules);
        ctx->last_error = "Procedure not found";
        return -1;
    }

    /* Set up the external call context */
    ctx->vm.ext_proc = p;
    ctx->vm.ext_argc = 1;
    /* Allocate ext_args on the heap so rxvm_call can free it */
    ctx->vm.ext_args = malloc(sizeof(value*));
    ctx->vm.ext_args[0] = (value*)token_array;

    ctx->vm.ext_ret = value_f();

    /* Run the VM */
    {
        char* dummy_argv[] = {"rxc_plugin"};
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

const char* rxvml_get_replacement_code(rxvml_context* ctx, rxvml_value* response) {
    value* v = (value*)response;
    if (!v) return NULL;
    /* If it's a string, return it directly */
    if (v->status.type_string || (v->string_length > 0 && v->string_value && v->status.all_type_flags == 0)) {
        null_terminate_string_buffer(v);
        return v->string_value;
    }
    /* Assume rxc.response has code at attribute 1 */
    if (v->num_attributes >= 1 && (v->attributes[0]->status.type_string || (v->attributes[0]->string_length > 0 && v->attributes[0]->string_value))) {
        null_terminate_string_buffer(v->attributes[0]);
        return v->attributes[0]->string_value;
    }
    return NULL;
}

const char* rxvml_get_error_message(rxvml_context* ctx, rxvml_value* response) {
    value* v = (value*)response;
    /* Assume rxc.response has error at attribute 2 */
    if (v && v->num_attributes >= 2 && (v->attributes[1]->status.type_string || (v->attributes[1]->string_length > 0 && v->attributes[1]->string_value))) {
        null_terminate_string_buffer(v->attributes[1]);
        return v->attributes[1]->string_value;
    }
    return NULL;
}

int rxvml_get_token_negotiation(rxvml_context* ctx, rxvml_value* token, int* out_new_type, int* out_is_updated) {
    value* v = (value*)token;
    if (!v || v->num_attributes < 15) return -1;
    if (out_new_type) *out_new_type = (int)v->attributes[13]->int_value;
    if (out_is_updated) *out_is_updated = (int)v->attributes[12]->int_value;
    return 0;
}

int rxvml_to_int(rxvml_context* ctx, const rxvml_value* v, rxinteger* out_v) {
    value* val = (value*)v;
    if (val && val->status.type_int) {
        if (out_v) *out_v = val->int_value;
        return 0;
    }
    return -1;
}

int rxvml_to_str(rxvml_context* ctx, const rxvml_value* v, const char** out_s, size_t* out_len) {
    value* val = (value*)v;
    if (val && val->status.type_string) {
        null_terminate_string_buffer(val);
        if (out_s) *out_s = val->string_value;
        if (out_len) *out_len = val->string_length;
        return 0;
    }
    return -1;
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
