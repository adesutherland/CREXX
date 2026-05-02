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

/* CREXX Module Loader */

#include <stdlib.h>
#include "rxvmintp.h"
#include "rxastree.h"
#include "rxvmvars.h"
#include "rxvmsock.h"
#include "rxpa.h"

// RXPA (Plugin Architecture) Support declarations  for this file

/* Context for RXPA Functions / callbacks */
void rxvm_addfunc(rxpa_libfunc func, char* name, char* option, char* type, char* args);
void rxvm_addclass(char* name, char* option, char* type);
void rxvm_addinterface(char* name, char* option, char* type);
void rxvm_addimplements(char* name, char* interface_name);
void rxvm_addmember(char* owner, char* kind, char* member, char* type, char* args);
char* rxvm_getstring(rxpa_attribute_value attributeValue);
void rxvm_setstring(rxpa_attribute_value attributeValue, char* string);
void rxvm_setint(rxpa_attribute_value attributeValue, rxinteger int_value);
rxinteger rxvm_getint(rxpa_attribute_value attributeValue);
void rxvm_setfloat(rxpa_attribute_value attributeValue, double double_value);
double rxvm_getfloat(rxpa_attribute_value attributeValue);
int rxvm_setnativepayload(rxpa_attribute_value attributeValue, const void *payload, size_t length,
                          const rxpa_native_payload_ops *ops, unsigned int flags);
void* rxvm_getnativepayload(rxpa_attribute_value attributeValue, size_t *out_length,
                            const rxpa_native_payload_ops **out_ops, unsigned int *out_flags);
rxinteger rxvm_getnumattrs(rxpa_attribute_value attributeValue);
void rxvm_setnumattrs(rxpa_attribute_value attributeValue, rxinteger numAttrs);
rxpa_attribute_value rxvm_getattr(rxpa_attribute_value attributeValue, rxinteger index);
rxpa_attribute_value rxvm_insertattr(rxpa_attribute_value attributeValue, rxinteger index);
void rxvm_removeattr(rxpa_attribute_value attributeValue, rxinteger index);
void rxvm_swapattrs(rxpa_attribute_value attributeValue, rxinteger index1, rxinteger index2);
void rxvm_setsayexit(say_exit_func sayExitFunc);
void rxvm_resetsayexit();

typedef  struct rxpa_context {
    rxvm_context *rxvm_context;
    module_file *plugin_being_loaded;
    size_t const_buffer_size;
    size_t const_buffer_top;
    int meta_tail;
} rxpa_context;

/* Statically Linked Plugin List */
struct static_linked_function {
    char *name;
    rxpa_libfunc func;
    struct static_linked_function *next;
};
static struct static_linked_function *static_linked_functions = 0;

struct static_linked_metadata {
    char *kind;
    char *symbol;
    char *option;
    char *type;
    char *interface_symbol;
    char *owner;
    char *member_kind;
    char *member;
    char *args;
    struct static_linked_metadata *next;
};
static struct static_linked_metadata *static_linked_metadata = 0;

// Local Global Context Variable
static rxpa_context *current_rxpa_context = 0;

// Create a new RXPA context for a module
static rxpa_context *rxpa_context_f(rxvm_context *rxvm_context);

// Free statically linked functions list
static void free_rxpa_context(rxpa_context *context);

static void free_interface_factory_registry(rxvm_context *context);
static void free_interface_method_registry(rxvm_context *context);
static void build_module_runtime_procedures(module *mod);
static proc_runtime *get_module_runtime_procedure(module *mod, size_t proc_offset);

// End of RXPA Declarations for this file

/* Initialise modules context */
void rxinimod(rxvm_context *context) {
    context->num_modules = 0;
    context->exposed_proc_tree = 0;
    context->exposed_reg_tree = 0;
    context->debug_mode = 0;
    context->location = 0;
    context->ext_proc = 0;
    context->ext_argc = 0;
    context->ext_args = 0;
    context->ext_ret = 0;
    context->interface_factories = 0;
    context->num_interface_factories = 0;
    context->interface_factory_capacity = 0;
    context->interface_methods = 0;
    context->num_interface_methods = 0;
    context->interface_method_capacity = 0;
    context->socket_registry = 0;
    context->link_dirty = 0;
    context->interface_method_registry_dirty = 0;
    context->interface_factory_registry_dirty = 0;

    /* Support 128 modules initially - this grows automatically */
    context->module_buffer_size = 128;
    context->modules = malloc(sizeof(module*) * context->module_buffer_size);
}

/* Free Module Context */
void rxfremod(rxvm_context *context) {
    int j, k;

    free_interface_factory_registry(context);
    free_interface_method_registry(context);
    rxvm_socket_free_registry(context);

    /* Free Symbol Search Trees */
    DEBUG("Free Symbol Search Trees\n");
    free_tree(&context->exposed_proc_tree);
    context->exposed_proc_tree = 0;
    free_tree(&context->exposed_reg_tree);
    context->exposed_reg_tree = 0;

    /* Free Program Modules */
    for (j=0; j<context->num_modules; j++) {
        size_t i;
        /* Drain procedure stack frame free lists */
        for (i = 0; i < context->modules[j]->procedure_count; i++) {
            proc_runtime *p_entry = &context->modules[j]->procedures[i];
            /* Only drain if this module owns the procedure (it's not imported) */
            if (p_entry->frame_free_list == &(p_entry->frame_free_list_head)) {
                stack_frame *f = *p_entry->frame_free_list;
                while (f) {
                    stack_frame *next = f->prev_free;
                    completely_free_frame(f);
                    f = next;
                }
                *p_entry->frame_free_list = 0;
            }
        }

        free_module(context->modules[j]->file);
        if (context->modules[j]->globals) {
            value **temp_globals = context->modules[j]->globals;
            char *temp_dont_free = context->modules[j]->globals_dont_free;
            int temp_count = context->modules[j]->segment.globals;
            context->modules[j]->globals = 0;
            context->modules[j]->globals_dont_free = 0;
            for (k = 0; k < temp_count; k++) {
                if (temp_globals[k] && (!temp_dont_free || !temp_dont_free[k])) {
                    clear_value(temp_globals[k]);
                    free(temp_globals[k]);
                }
            }
            free(temp_globals);
            if (temp_dont_free) free(temp_dont_free);
        }
        if (context->modules[j]->prepared_dispatch) free(context->modules[j]->prepared_dispatch);
        if (context->modules[j]->proc_runtime_lookup) free(context->modules[j]->proc_runtime_lookup);
        if (context->modules[j]->procedures) free(context->modules[j]->procedures);
        free(context->modules[j]);
    }
    free(context->modules);
    if (context->location) free(context->location);
}

static void free_interface_factory_registry(rxvm_context *context) {
    size_t i;

    if (!context || !context->interface_factories) {
        if (context) {
            context->num_interface_factories = 0;
            context->interface_factory_capacity = 0;
        }
        return;
    }

    for (i = 0; i < context->num_interface_factories; i++) {
        free(context->interface_factories[i].interface_name);
        free(context->interface_factories[i].factory_name);
        free(context->interface_factories[i].class_name);
    }

    free(context->interface_factories);
    context->interface_factories = 0;
    context->num_interface_factories = 0;
    context->interface_factory_capacity = 0;
}

static void free_interface_method_registry(rxvm_context *context) {
    size_t i;

    if (!context || !context->interface_methods) {
        if (context) {
            context->num_interface_methods = 0;
            context->interface_method_capacity = 0;
        }
        return;
    }

    for (i = 0; i < context->num_interface_methods; i++) {
        free(context->interface_methods[i].class_name);
        free(context->interface_methods[i].member_name);
    }

    free(context->interface_methods);
    context->interface_methods = 0;
    context->num_interface_methods = 0;
    context->interface_method_capacity = 0;
}

static void build_module_runtime_procedures(module *mod) {
    int i;
    size_t proc_index;
    unsigned char *seen;

    mod->procedures = 0;
    mod->procedure_count = 0;
    mod->proc_runtime_lookup = 0;
    mod->proc_runtime_lookup_size = (mod->segment.const_size + (size_t)7) >> 3;
    seen = 0;

    if (mod->proc_runtime_lookup_size) {
        seen = calloc(mod->proc_runtime_lookup_size, sizeof(unsigned char));
        if (!seen) {
            fprintf(stderr, "PANIC: Out of memory\n");
            exit(-1);
        }
    }

    i = mod->proc_head;
    while (i != -1) {
        proc_constant *definition = (proc_constant *)(mod->segment.const_pool + (size_t)i);
        if (definition->base.type != PROC_CONST) {
            fprintf(stderr, "PANIC: Invalid procedure chain in module %s\n", mod->name);
            exit(-1);
        }
        if (!seen[(size_t)i >> 3]) {
            seen[(size_t)i >> 3] = 1;
            mod->procedure_count++;
        }
        i = definition->next;
    }

    i = mod->expose_head;
    while (i != -1) {
        chameleon_constant *entry = (chameleon_constant *)(mod->segment.const_pool + (size_t)i);
        if (entry->type == EXPOSE_PROC_CONST) {
            expose_proc_constant *exposed = (expose_proc_constant *)entry;
            proc_constant *definition = (proc_constant *)(mod->segment.const_pool + exposed->procedure);
            if (definition->base.type != PROC_CONST) {
                fprintf(stderr, "PANIC: Invalid exposed procedure in module %s\n", mod->name);
                exit(-1);
            }
            if (!seen[exposed->procedure >> 3]) {
                seen[exposed->procedure >> 3] = 1;
                mod->procedure_count++;
            }
            i = exposed->next;
        } else if (entry->type == EXPOSE_REG_CONST) {
            i = ((expose_reg_constant *)entry)->next;
        } else {
            fprintf(stderr, "PANIC: Invalid expose chain in module %s\n", mod->name);
            exit(-1);
        }
    }

    if (mod->procedure_count) {
        mod->procedures = calloc(mod->procedure_count, sizeof(proc_runtime));
        if (!mod->procedures) {
            fprintf(stderr, "PANIC: Out of memory\n");
            exit(-1);
        }
    }

    if (mod->proc_runtime_lookup_size) {
        mod->proc_runtime_lookup = calloc(mod->proc_runtime_lookup_size, sizeof(proc_runtime *));
        if (!mod->proc_runtime_lookup) {
            fprintf(stderr, "PANIC: Out of memory\n");
            exit(-1);
        }
        memset(seen, 0, mod->proc_runtime_lookup_size);
    }

    i = mod->proc_head;
    proc_index = 0;
    while (i != -1) {
        proc_constant *definition = (proc_constant *)(mod->segment.const_pool + (size_t)i);
        if (!seen[(size_t)i >> 3]) {
            proc_runtime *runtime = &mod->procedures[proc_index++];

            runtime->definition = definition;
            runtime->locals = definition->locals;
            runtime->binarySpace = (!mod->native && definition->start != SIZE_MAX) ? &mod->segment : 0;
            runtime->frame_free_list_head = 0;
            runtime->frame_free_list = &runtime->frame_free_list_head;
            runtime->start = definition->start;
            runtime->name = definition->name;
            mod->proc_runtime_lookup[(size_t)i >> 3] = runtime;
            seen[(size_t)i >> 3] = 1;
        }
        i = definition->next;
    }

    i = mod->expose_head;
    while (i != -1) {
        chameleon_constant *entry = (chameleon_constant *)(mod->segment.const_pool + (size_t)i);
        if (entry->type == EXPOSE_PROC_CONST) {
            expose_proc_constant *exposed = (expose_proc_constant *)entry;
            if (!seen[exposed->procedure >> 3]) {
                proc_constant *definition = (proc_constant *)(mod->segment.const_pool + exposed->procedure);
                proc_runtime *runtime = &mod->procedures[proc_index++];

                runtime->definition = definition;
                runtime->locals = definition->locals;
                runtime->binarySpace = (!mod->native && definition->start != SIZE_MAX) ? &mod->segment : 0;
                runtime->frame_free_list_head = 0;
                runtime->frame_free_list = &runtime->frame_free_list_head;
                runtime->start = definition->start;
                runtime->name = definition->name;
                mod->proc_runtime_lookup[exposed->procedure >> 3] = runtime;
                seen[exposed->procedure >> 3] = 1;
            }
            i = exposed->next;
        } else {
            i = ((expose_reg_constant *)entry)->next;
        }
    }

    if (seen) free(seen);
}

static proc_runtime *get_module_runtime_procedure(module *mod, size_t proc_offset) {
    if (!mod || proc_offset >= mod->segment.const_size || !mod->proc_runtime_lookup) return 0;
    return mod->proc_runtime_lookup[proc_offset >> 3];
}

/* Link a loaded module */
void rxvm_link_module(rxvm_context *context, size_t module_number_to_link) {
    int i;
    size_t mod_index;
    chameleon_constant *c_entry;
    proc_runtime *p_entry, *p_entry_linked;
    value *g_reg;

    DEBUG("Add Module Symbols\n");
    i = context->modules[module_number_to_link]->expose_head;
    mod_index = module_number_to_link;
    while (i != -1) {
        c_entry = (chameleon_constant *)(context->modules[mod_index]->segment.const_pool + (size_t)i);
        switch (c_entry->type) {

            case EXPOSE_REG_CONST:
                /* Exposed Register */
                if (src_node(context->exposed_reg_tree,
                             ((expose_reg_constant *) c_entry)->index,
                             (size_t *) &g_reg)) {
                    /* Register already exposed / initialised */
                    context->modules[mod_index]
                            ->globals[((expose_reg_constant *) c_entry)
                            ->global_reg] =
                            g_reg;
                    context->modules[mod_index]
                            ->globals_dont_free[((expose_reg_constant *) c_entry)
                            ->global_reg] = 1;
                } else {
                    /* Need to initialise a register and expose it in the search tree */
                    context->modules[mod_index]
                            ->globals[((expose_reg_constant *) c_entry)
                            ->global_reg] = value_f();
                    add_node(&context->exposed_reg_tree, ((expose_reg_constant *)c_entry)->index,
                             (size_t)(context->modules[mod_index]
                                     ->globals[((expose_reg_constant *)c_entry)
                                     ->global_reg]));
                }
                break;

            case EXPOSE_PROC_CONST:
                /* Exposed Procedure */
                p_entry = get_module_runtime_procedure(
                        context->modules[mod_index],
                        ((expose_proc_constant *) c_entry)->procedure);

                if (((expose_proc_constant *) c_entry)->imported) {
                    /* Imported - Add to the unresolved symbols count for later */
                    context->modules[mod_index]->unresolved_symbols++;
                }
                else {
                    /* Exported - check duplicate */
                    if (add_node(&context->exposed_proc_tree,
                                 ((expose_proc_constant *) c_entry)->index,
                                 (size_t) p_entry)) {
                        DEBUG("WARNING: Duplicate exposed symbol: %s\n",
                              ((expose_proc_constant *) c_entry)->index);
                        context->modules[mod_index]->duplicated_symbols++;
                    }
                }
                break;

            default:;
        }

        i = ((expose_reg_constant *)c_entry)->next;
    }

    DEBUG("Resolve Symbols\n");
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        /* Skip modules without any unresolved symbols */
        if (!context->modules[mod_index]->unresolved_symbols) continue;
        i = context->modules[mod_index]->expose_head;
        while (i != -1) {
            c_entry = (chameleon_constant *)(context->modules[mod_index]->segment.const_pool + (size_t)i);
            switch (c_entry->type) {
                case EXPOSE_PROC_CONST:
                    if (((expose_proc_constant *) c_entry)->imported) {
                        if (src_node(context->exposed_proc_tree,
                                     ((expose_proc_constant *) c_entry)->index,
                                     (size_t *) &p_entry_linked)) {

                            /* Patch the procedure entry with the linked one */
                            p_entry = get_module_runtime_procedure(
                                    context->modules[mod_index],
                                    ((expose_proc_constant *) c_entry)->procedure);
                            if (p_entry->start == SIZE_MAX ) { /* If not already linked up */
                                p_entry->locals = p_entry_linked->locals;
                                p_entry->start = p_entry_linked->start;
                                p_entry->binarySpace =
                                        p_entry_linked->binarySpace;
                                p_entry->frame_free_list =
                                        p_entry_linked->frame_free_list;

                                /* Reduce the number of unresolved symbols */
                                context->modules[mod_index]
                                        ->unresolved_symbols--;
                            }
                        }
                    }
                    break;

                default:;
            }

            i = ((expose_proc_constant *)c_entry)->next;
        }
    }

    /* Allocate Module Globals that have not already been allocated during linking */
    DEBUG("Allocate Globals\n");
    mod_index = module_number_to_link;
    for (i = 0; i < context->modules[mod_index]->segment.globals; i++) {
        if (!context->modules[mod_index]->globals[i]) {
            context->modules[mod_index]->globals[i] = value_f();
        }
    }

}

/* Common Functionality to prep and link a module */
/* Returns the new number of modules */
static size_t prep_and_link_module(rxvm_context *context, module_file *file_module_section) {
    size_t n = context->num_modules;
    void *new_buffer;

    DEBUG("Loading Module %s\n", file_module_section->name);

    /* Grow the module buffer if need be */
    while (n + 1 > context->module_buffer_size) {
        context->module_buffer_size *= 2;
        new_buffer = realloc(context->modules, sizeof(module*) * context->module_buffer_size);
        if (!new_buffer) {
            fprintf(stderr, "PANIC: Out of memory\n");
            exit(-1);
        }
        context->modules = new_buffer;
    }
    context->modules[n] = malloc(sizeof(module));
    context->modules[n]->segment.globals = file_module_section->header.globals;
    context->modules[n]->segment.inst_size = file_module_section->header.instruction_size;
    context->modules[n]->segment.const_size = file_module_section->header.constant_size;
    context->modules[n]->segment.binary = file_module_section->instructions;
    context->modules[n]->segment.const_pool = file_module_section->constant;
    context->modules[n]->segment.module = context->modules[n];
    context->modules[n]->name = file_module_section->name;
    context->modules[n]->description = file_module_section->description;
    context->modules[n]->proc_head = file_module_section->header.proc_head;
    context->modules[n]->expose_head = file_module_section->header.expose_head;
    context->modules[n]->meta_head = file_module_section->header.meta_head;
    context->modules[n]->globals = calloc(context->modules[n]->segment.globals, sizeof(value*));
    context->modules[n]->globals_dont_free = calloc(context->modules[n]->segment.globals, sizeof(char));
    context->modules[n]->unresolved_symbols = 0;
    context->modules[n]->duplicated_symbols = 0;
    context->modules[n]->file = file_module_section;
    context->modules[n]->native = file_module_section->native;
    context->modules[n]->state = RXVM_MOD_LOADED;
    context->modules[n]->procedures = 0;
    context->modules[n]->procedure_count = 0;
    context->modules[n]->proc_runtime_lookup = 0;
    context->modules[n]->proc_runtime_lookup_size = 0;
    context->modules[n]->prepared_dispatch = 0;
    build_module_runtime_procedures(context->modules[n]);
    context->link_dirty = 1;
    context->interface_method_registry_dirty = 1;
    context->interface_factory_registry_dirty = 1;

    context->num_modules = context->modules[n]->module_number = n + 1;

    return context->num_modules ;
}

/* Loads a module from a file
 * returns <= 0  - Error
 *         >0 - Last Module Number loaded (1 based) (more than one might have been loaded ...)  */
int rxldmod(rxvm_context *context, char *file_name) {
    FILE *fp;
    module_file *file_module_section;
    size_t modules_processed = 0;
    int loaded_rc;
    size_t n = 0;

    DEBUG("Loading Module(s) from file %s\n", file_name);

    // Check if the file is a crexx module
    // if context->location is not set we need to first check the file_name as an absolute path
    // then if not found try as a relative path from the current working directory ('.')
    char *location = context->location;
    char found_location[MAXFILEPATH];
    int file_exists = 0;

    found_location[0] = 0;

    /* Determine if provided file_name already contains an extension */
    int has_ext = 0;
    if (file_name) {
        const char *last_slash = strrchr(file_name, '/');
#ifdef _WIN32
        const char *last_bsl = strrchr(file_name, '\\');
        if (!last_slash || (last_bsl && last_bsl > last_slash)) last_slash = last_bsl;
#endif
        const char *fname = last_slash ? last_slash + 1 : file_name;
        if (strchr(fname, '.') != NULL) has_ext = 1;
    }
    const char *type_bin = has_ext ? "" : "rxbin";

    // Check if the file exists as an absolute path
    if (fileexists(file_name, (char*)type_bin, 0)) {
        if (context->debug_mode) fprintf(stderr, "DEBUG_EXIT: Found module %s (as absolute path)\n", file_name);
        file_exists = 1;
    } else if (location) {
        char *loc_copy = strdup(location);
        char *token = strtok(loc_copy, ";");
        while (token) {
            if (context->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Checking for module %s in location %s\n", file_name, token);
            if (fileexists(file_name, (char*)type_bin, token)) {
                strncpy(found_location, token, MAXFILEPATH - 1);
                found_location[MAXFILEPATH - 1] = 0;
                file_exists = 1;
                break;
            }
            token = strtok(NULL, ";");
        }
        free(loc_copy);
    } else {
        // Try as a relative path from the current working directory
        if (context->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Checking for module %s in current directory\n", file_name);
        if (fileexists(file_name, (char*)type_bin, ".")) {
            strncpy(found_location, ".", MAXFILEPATH - 1);
            found_location[MAXFILEPATH - 1] = 0;
            file_exists = 1;
        }
    }

    if (file_exists) {
        DEBUG("CREXX Module file\n");
        fp = openfile(file_name, (char*)type_bin, found_location[0] ? found_location : 0, "rb");
        if (!fp) return 0;

        loaded_rc = 0;
        while (loaded_rc == 0) {
            file_module_section = 0;
            switch (loaded_rc = read_module(&file_module_section, fp)) {
                case 0: /* Success */
                    DEBUG("Module Read\n");
                    n = prep_and_link_module(context, file_module_section);
                    DEBUG("Module Prep and linked\n");
                    modules_processed++;
                    break;

                case 1: /* eof */
                    if (file_module_section) free_module(file_module_section);
                    if (!modules_processed) {
                        DEBUG("ERROR: empty file %s\n", file_name);
                        fclose(fp);
                        return 0;
                    }
                    break;

                default: /* error */
                    if (file_module_section) free_module(file_module_section);
                    DEBUG("ERROR: reading file %s\n", file_name);
                    fclose(fp);
                    return(-1);
            }
        }
        fclose(fp);
    }

    else {
        // Check if the file is a native plugin
        // if context->location is not set we need to first check the file_name as an absolute path
        // then if not found try as a relative path from the current working directory ('.')
        location = context->location;
        found_location[0] = 0;
        file_exists = 0;
        /* For plugins, also avoid appending extension if one is already provided */
        const char *type_plugin = has_ext ? "" : "rxplugin";

        // Check if the file exists as an absolute path
        if (fileexists(file_name, (char*)type_plugin, 0)) {
            if (context->debug_mode) fprintf(stderr, "DEBUG_EXIT: Found plugin %s (as absolute path)\n", file_name);
            file_exists = 1;
        } else if (location) {
            char *loc_copy = strdup(location);
            char *token = strtok(loc_copy, ";");
            while (token) {
                if (context->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Checking for plugin %s in location %s\n", file_name, token);
                if (fileexists(file_name, (char*)type_plugin, token)) {
                    strncpy(found_location, token, MAXFILEPATH - 1);
                    found_location[MAXFILEPATH - 1] = 0;
                    file_exists = 1;
                    break;
                }
                token = strtok(NULL, ";");
            }
            free(loc_copy);
        } else {
            // Try as a relative path from the current working directory
            if (context->debug_mode >= 2) fprintf(stderr, "DEBUG_EXIT: Checking for plugin %s in current directory\n", file_name);
            if (fileexists(file_name, (char*)type_plugin, ".")) {
                strncpy(found_location, ".", MAXFILEPATH - 1);
                found_location[MAXFILEPATH - 1] = 0;
                file_exists = 1;
            }
        }

        if (file_exists) {
            DEBUG("CREXX Native Plugin file\n");
            // Check if the file exists
            fp = openfile(file_name, (char*)type_plugin, found_location[0] ? found_location : 0, "rb");

            // Create the rxpa_initctxptr context
            struct rxpa_initctxptr rxpa_functions;
            rxpa_functions.addfunc = rxvm_addfunc;
            rxpa_functions.addclass = rxvm_addclass;
            rxpa_functions.addinterface = rxvm_addinterface;
            rxpa_functions.addimplements = rxvm_addimplements;
            rxpa_functions.addmember = rxvm_addmember;
            rxpa_functions.getstring = rxvm_getstring;
            rxpa_functions.setstring = rxvm_setstring;
            rxpa_functions.setint = rxvm_setint;
            rxpa_functions.getint = rxvm_getint;
            rxpa_functions.setfloat = rxvm_setfloat;
            rxpa_functions.getfloat = rxvm_getfloat;
            rxpa_functions.setnativepayload = rxvm_setnativepayload;
            rxpa_functions.getnativepayload = rxvm_getnativepayload;
            rxpa_functions.getnumattrs = rxvm_getnumattrs;
            rxpa_functions.setnumattrs = rxvm_setnumattrs;
            rxpa_functions.getattr = rxvm_getattr;
            rxpa_functions.insertattr = rxvm_insertattr;
            rxpa_functions.removeattr = rxvm_removeattr;
            rxpa_functions.swapattrs = rxvm_swapattrs;

            // Exit Function Management
            rxpa_functions.setsayexit = rxvm_setsayexit;
            rxpa_functions.resetsayexit = rxvm_resetsayexit;

            // Load the plugin - and run the plugin initialization function
            // Create the filename by appending ".rxplugin" to the file name
            char *full_file_name = malloc(strlen(file_name) + strlen(".rxplugin") + 1);
            sprintf(full_file_name, "%s.rxplugin", file_name);

            // Create rxpa_context and module for the addfunc callback
            current_rxpa_context = rxpa_context_f(context);
            current_rxpa_context->plugin_being_loaded = malloc(sizeof(module_file));
            init_module(current_rxpa_context->plugin_being_loaded);
            current_rxpa_context->plugin_being_loaded->header.name_size = strlen(full_file_name) + 1;
            current_rxpa_context->plugin_being_loaded->name = malloc(
                    current_rxpa_context->plugin_being_loaded->header.name_size);
            strcpy(current_rxpa_context->plugin_being_loaded->name, full_file_name);
            current_rxpa_context->plugin_being_loaded->fromfile = 0;
            current_rxpa_context->plugin_being_loaded->native = 1;
            current_rxpa_context->plugin_being_loaded->header.expose_head = -1;
            current_rxpa_context->plugin_being_loaded->header.proc_head = -1;
            current_rxpa_context->plugin_being_loaded->header.meta_head = -1;

            // Load the plugin
            int rc = load_plugin(&rxpa_functions, found_location[0] ? found_location : 0, full_file_name);
            free(full_file_name);

            // Check Result
            if (!rc) {
                DEBUG("CREXX Plugin %s loaded successfully\n", file_name);
                n = prep_and_link_module(context, current_rxpa_context->plugin_being_loaded);
                current_rxpa_context->plugin_being_loaded = 0; // We are done with it! It will be freed eventually
            } else {
                DEBUG("Failed to load plugin %s (rc=%d)\n", file_name, rc);
                free_rxpa_context(current_rxpa_context);
                return(-1);
            }

            // Free the rxpa_context
            free_rxpa_context(current_rxpa_context);
            current_rxpa_context = 0;
        }
        // Else an unrecognised file
        else {
            DEBUG("Unrecognised file type %s\n", file_name);
            return(-1);
        }
    }
    return (int)(n); /* Module Number */
}

/* Loads a module from a memory buffer
 * returns 0  - Error
 *         >0 - Last Module Number loaded (1 based) (more than one might have been loaded ...)  */
int rxldmodm(rxvm_context *context, char *buffer_start, size_t buffer_length) {
    module_file *file_module_section;
    size_t modules_processed = 0;
    int loaded_rc;
    size_t n;
    char *buffer_end = buffer_start + buffer_length;

    DEBUG("Loading Module(s) from memory\n");

    loaded_rc = 0;
    while (loaded_rc == 0) {
        file_module_section = 0;
        switch (loaded_rc = read_module_mem(&file_module_section, &buffer_start, buffer_end)) {
            case 0: /* Success */
                n = prep_and_link_module(context, file_module_section);
                modules_processed++;
                break;

            case 1: /* eof */
                if (file_module_section) free_module(file_module_section);
                if (!modules_processed) {
                    DEBUG("ERROR: empty buffer\n");
                    return 0;
                }
                break;

            default: /* error */
                if (file_module_section) free_module(file_module_section);
                fprintf(stderr, "ERROR: reading buffer\n");
                exit(-1);
        }
    }

    return (int)(n); /* Module Number */
}

// RXPA (Plugin Architecture) Support Functions

// Free statically linked functions list
static void free_rxpa_context(rxpa_context *context)
{
    if (context->plugin_being_loaded) free_module(context->plugin_being_loaded);
    free(context);
}

// Create a new RXPA context for a module
static rxpa_context *rxpa_context_f(rxvm_context *rxvm_context) {
    rxpa_context *new_rxpa_context = malloc(sizeof(rxpa_context));
    new_rxpa_context->rxvm_context = rxvm_context;
    new_rxpa_context->const_buffer_size = 0;
    new_rxpa_context->const_buffer_top = 0;
    new_rxpa_context->meta_tail = -1;
    return new_rxpa_context;
}

/* Reserves space in the constant pool for an entry and returns its index;
 * This is used to create a fake  constant pool for native functions with the function name and pointer
 * the caller can then populate the entry.
 * NOTE - THIS CALL MIGHT MOVE THE CONSTANT POOL - CHANGING ENTRY ADDRESSES (USE OFFSETS!)
 * The 'size' parameter is the size of the payload including
 * space for chameleon_constant etc.
 * Returns the index to the entry (from binary.const_pool)
 */
static size_t reserve_in_const_pool(rxpa_context *context, size_t size, enum const_pool_type type) {
    size_t index, new_size;
    chameleon_constant * entry;
    void* new_buffer;

    /* Create buffer if needed */
    if (!context->plugin_being_loaded->constant) {
        context->const_buffer_size = 1024;
        context->plugin_being_loaded->constant = malloc(context->const_buffer_size);
        context->plugin_being_loaded->header.constant_size = 0;
    }

    /* Extend the buffer if we need to */
    while (size + 8 > context->const_buffer_size - context->plugin_being_loaded->header.constant_size) { // +8 for the 8 bit alignment
        new_size = context->const_buffer_size * 2;
        new_buffer = realloc(context->plugin_being_loaded->constant, new_size);
        if (!new_buffer) {
            fprintf(stderr, "PANIC: Out of memory\n");
            exit(-1);
        }
        context->plugin_being_loaded->constant = new_buffer;
        memset(context->plugin_being_loaded->constant + context->const_buffer_size, 0, context->const_buffer_size);
        context->const_buffer_size = new_size;
    }

    /* We are putting the entry at the top of the pool */
    index = context->const_buffer_top;
    entry = (chameleon_constant*)(context->plugin_being_loaded->constant + index);

    entry->type = type;

    /* Round up the size for alignment */
    size = (size + (size_t)7) & ~ (size_t)0x07; /* 8 byte alignment */

    /* Store the size */
    entry->size_in_pool = size;

    /* Move up the const_size "pointer" */
    context->plugin_being_loaded->header.constant_size += size;
    context->const_buffer_top += size;

    return index;
}

static size_t add_native_string_to_pool(rxpa_context *context, const char *value) {
    string_constant *sentry;
    size_t entry_index;
    size_t entry_size;
    size_t string_len;

    if (!value) value = "";
    string_len = strlen(value);
    entry_size = sizeof(string_constant) + string_len;
    entry_index = reserve_in_const_pool(context, entry_size, STRING_CONST);
    sentry = (string_constant *) (context->plugin_being_loaded->constant + entry_index);
    sentry->string_len = string_len;
#ifndef NUTF8
    sentry->string_chars = string_len;
#endif
    memcpy(sentry->string, value, string_len + 1);
    return entry_index;
}

static size_t add_native_meta_entry(rxpa_context *context, size_t entry_size, enum const_pool_type type) {
    meta_entry *entry;
    size_t entry_index;

    entry_index = reserve_in_const_pool(context, entry_size, type);
    entry = (meta_entry*)(context->plugin_being_loaded->constant + entry_index);

    if (context->plugin_being_loaded->header.meta_head != -1) {
        ((meta_entry*)(context->plugin_being_loaded->constant + context->meta_tail))->next = (int)entry_index;
        entry->prev = context->meta_tail;
        context->meta_tail = (int)entry_index;
        entry->next = -1;
    }
    else {
        context->plugin_being_loaded->header.meta_head = (int)entry_index;
        context->meta_tail = (int)entry_index;
        entry->next = -1;
        entry->prev = -1;
    }

    entry->address = 0;
    return entry_index;
}

static void add_class_meta_to_module(rxpa_context *context, char *name, char *option, char *type) {
    size_t s_name = add_native_string_to_pool(context, name);
    size_t s_option = add_native_string_to_pool(context, option);
    size_t s_type = add_native_string_to_pool(context, type);
    size_t entry = add_native_meta_entry(context, sizeof(meta_class_constant), META_CLASS);
    meta_class_constant *meta;

    meta = (meta_class_constant*)(context->plugin_being_loaded->constant + entry);
    meta->symbol = s_name;
    meta->option = s_option;
    meta->type = s_type;
}

static void add_interface_meta_to_module(rxpa_context *context, char *name, char *option, char *type) {
    size_t s_name = add_native_string_to_pool(context, name);
    size_t s_option = add_native_string_to_pool(context, option);
    size_t s_type = add_native_string_to_pool(context, type);
    size_t entry = add_native_meta_entry(context, sizeof(meta_interface_constant), META_INTERFACE);
    meta_interface_constant *meta;

    meta = (meta_interface_constant*)(context->plugin_being_loaded->constant + entry);
    meta->symbol = s_name;
    meta->option = s_option;
    meta->type = s_type;
}

static void add_implements_meta_to_module(rxpa_context *context, char *name, char *interface_name) {
    size_t s_name = add_native_string_to_pool(context, name);
    size_t s_interface = add_native_string_to_pool(context, interface_name);
    size_t entry = add_native_meta_entry(context, sizeof(meta_implements_constant), META_IMPLEMENTS);
    meta_implements_constant *meta;

    meta = (meta_implements_constant*)(context->plugin_being_loaded->constant + entry);
    meta->symbol = s_name;
    meta->interface_symbol = s_interface;
}

static void add_member_meta_to_module(rxpa_context *context, char *owner, char *kind, char *member, char *type, char *args) {
    size_t s_owner = add_native_string_to_pool(context, owner);
    size_t s_kind = add_native_string_to_pool(context, kind);
    size_t s_member = add_native_string_to_pool(context, member);
    size_t s_type = add_native_string_to_pool(context, type);
    size_t s_args = add_native_string_to_pool(context, args);
    size_t entry = add_native_meta_entry(context, sizeof(meta_member_constant), META_MEMBER);
    meta_member_constant *meta;

    meta = (meta_member_constant*)(context->plugin_being_loaded->constant + entry);
    meta->owner = s_owner;
    meta->kind = s_kind;
    meta->member = s_member;
    meta->type = s_type;
    meta->args = s_args;
}

// Add a statically linked function to the constant pool being created
void add_proc_to_module(rxpa_context* context , char* index, rxpa_libfunc func) {
    size_t entry_size, proc_index, exposed_proc_index;
    proc_constant *proc;
    expose_proc_constant *exposed_proc;

    /* Create Procedure Entry */
    entry_size = sizeof(proc_constant) + strlen(index);
    proc_index = reserve_in_const_pool(context, entry_size, PROC_CONST);
    proc = (proc_constant *) (context->plugin_being_loaded->constant + proc_index);

    /* Set structure data */
    memcpy(proc->name, index, strlen(index) + 1);
    proc->next = -1;
    proc->locals = 0;
    proc->start = (size_t)func;

    /* Create Exposed Procedure Entry */
    entry_size = sizeof(expose_proc_constant) + strlen(index);
    exposed_proc_index = reserve_in_const_pool(context, entry_size,EXPOSE_PROC_CONST);
    exposed_proc = (expose_proc_constant *) (context->plugin_being_loaded->constant + exposed_proc_index);

    /* Set structure data */
    memcpy(exposed_proc->index, index, strlen(index) + 1);
    exposed_proc->procedure = proc_index;
    exposed_proc->imported = 0;

    /* Chain the exposed constant entries */
    exposed_proc->next = context->plugin_being_loaded->header.expose_head;
    context->plugin_being_loaded->header.expose_head = (int)exposed_proc_index;

    proc = (proc_constant *) (context->plugin_being_loaded->constant + proc_index);
    proc->exposed = exposed_proc_index;
}

// RXPA Add Function Implementation
// This is the callback function for rxldmod() when the plugin adds functions,
// or is called during initialising a statically linked plugin
void rxvm_addfunc(rxpa_libfunc func, char* name, __attribute__((unused)) char* option, __attribute__((unused)) char* type, __attribute__((unused)) char* args) {

    if (current_rxpa_context) {
        rxvm_context *context = current_rxpa_context->rxvm_context;
        DEBUG("Loading Procedure %s from plugin %s\n", name, context->modules[context->num_modules - 1]->name);

        // Add the procedure to the module
        add_proc_to_module(current_rxpa_context, name, func);
    }
    else {
        // Add to the list of statically linked functions
        struct static_linked_function *new_static_func = malloc(sizeof(struct static_linked_function));
        new_static_func->name = name;
        new_static_func->func = func;
        new_static_func->next = static_linked_functions;
        static_linked_functions = new_static_func;
    }
}

static void append_static_metadata(char *kind, char *symbol, char *option, char *type,
                                   char *interface_symbol, char *owner, char *member_kind,
                                   char *member, char *args) {
    struct static_linked_metadata *new_meta = malloc(sizeof(struct static_linked_metadata));

    new_meta->kind = kind;
    new_meta->symbol = symbol;
    new_meta->option = option;
    new_meta->type = type;
    new_meta->interface_symbol = interface_symbol;
    new_meta->owner = owner;
    new_meta->member_kind = member_kind;
    new_meta->member = member;
    new_meta->args = args;
    new_meta->next = static_linked_metadata;
    static_linked_metadata = new_meta;
}

void rxvm_addclass(char* name, char* option, char* type) {
    if (current_rxpa_context) {
        add_class_meta_to_module(current_rxpa_context, name, option, type);
    }
    else {
        append_static_metadata("class", name, option, type, 0, 0, 0, 0, 0);
    }
}

void rxvm_addinterface(char* name, char* option, char* type) {
    if (current_rxpa_context) {
        add_interface_meta_to_module(current_rxpa_context, name, option, type);
    }
    else {
        append_static_metadata("interface", name, option, type, 0, 0, 0, 0, 0);
    }
}

void rxvm_addimplements(char* name, char* interface_name) {
    if (current_rxpa_context) {
        add_implements_meta_to_module(current_rxpa_context, name, interface_name);
    }
    else {
        append_static_metadata("implements", name, 0, 0, interface_name, 0, 0, 0, 0);
    }
}

void rxvm_addmember(char* owner, char* kind, char* member, char* type, char* args) {
    if (current_rxpa_context) {
        add_member_meta_to_module(current_rxpa_context, owner, kind, member, type, args);
    }
    else {
        append_static_metadata("member", 0, 0, type, 0, owner, kind, member, args);
    }
}

/* Loads statically loaded plugins
 * returns -1  - Error
 *         >=0 - Last Module Number loaded (1 based) (more than one (or none) might have been loaded ...)  */
int rxldmodp(rxvm_context *context) {
    size_t n;

    if (static_linked_functions == 0 && static_linked_metadata == 0) return 0;

    DEBUG("Loading Statically linked Plugins\n");

    // Create the rxpa_initctxptr context
    struct rxpa_initctxptr rxpa_functions;
    rxpa_functions.addfunc = rxvm_addfunc;
    rxpa_functions.addclass = rxvm_addclass;
    rxpa_functions.addinterface = rxvm_addinterface;
    rxpa_functions.addimplements = rxvm_addimplements;
    rxpa_functions.addmember = rxvm_addmember;
    rxpa_functions.getstring = rxvm_getstring;
    rxpa_functions.setstring = rxvm_setstring;
    rxpa_functions.setint = rxvm_setint;
    rxpa_functions.getint = rxvm_getint;
    rxpa_functions.setfloat = rxvm_setfloat;
    rxpa_functions.getfloat = rxvm_getfloat;
    rxpa_functions.setnativepayload = rxvm_setnativepayload;
    rxpa_functions.getnativepayload = rxvm_getnativepayload;
    rxpa_functions.getnumattrs = rxvm_getnumattrs;
    rxpa_functions.setnumattrs = rxvm_setnumattrs;
    rxpa_functions.getattr = rxvm_getattr;
    rxpa_functions.insertattr = rxvm_insertattr;
    rxpa_functions.removeattr = rxvm_removeattr;
    rxpa_functions.swapattrs = rxvm_swapattrs;

    // Exit Function Management
    rxpa_functions.setsayexit = rxvm_setsayexit;
    rxpa_functions.resetsayexit = rxvm_resetsayexit;

    // Create rxpa_context and module
    char* dummy_file_name = "statically_linked_plugins";
    current_rxpa_context = rxpa_context_f(context);
    current_rxpa_context->plugin_being_loaded = malloc(sizeof(module_file));
    init_module(current_rxpa_context->plugin_being_loaded);
    current_rxpa_context->plugin_being_loaded->header.name_size = strlen(dummy_file_name) + 1;
    current_rxpa_context->plugin_being_loaded->name = malloc(
            current_rxpa_context->plugin_being_loaded->header.name_size);
    strcpy(current_rxpa_context->plugin_being_loaded->name, dummy_file_name);
    current_rxpa_context->plugin_being_loaded->fromfile = 0;
    current_rxpa_context->plugin_being_loaded->native = 1;
    current_rxpa_context->plugin_being_loaded->header.expose_head = -1;
    current_rxpa_context->plugin_being_loaded->header.proc_head = -1;
    current_rxpa_context->plugin_being_loaded->header.meta_head = -1;

    // Process static plugin functions
    struct static_linked_function *static_func = static_linked_functions;
    while (static_func) {
        DEBUG("CREXX Statically Linked Native Plugin %s\n", static_func->name);
        rxvm_addfunc(static_func->func, static_func->name, 0, 0, 0) ;

        // Loop to next static function
        static_func = static_func->next;
    }

    struct static_linked_metadata *static_meta = static_linked_metadata;
    while (static_meta) {
        if (strcmp(static_meta->kind, "class") == 0) {
            rxvm_addclass(static_meta->symbol, static_meta->option, static_meta->type);
        }
        else if (strcmp(static_meta->kind, "interface") == 0) {
            rxvm_addinterface(static_meta->symbol, static_meta->option, static_meta->type);
        }
        else if (strcmp(static_meta->kind, "implements") == 0) {
            rxvm_addimplements(static_meta->symbol, static_meta->interface_symbol);
        }
        else if (strcmp(static_meta->kind, "member") == 0) {
            rxvm_addmember(static_meta->owner, static_meta->member_kind, static_meta->member,
                           static_meta->type, static_meta->args);
        }
        static_meta = static_meta->next;
    }

    // Link as a plugin
    n = prep_and_link_module(context, current_rxpa_context->plugin_being_loaded);

    // Free the rxpa_context
    current_rxpa_context->plugin_being_loaded = 0; // It's now owned by the module list
    free_rxpa_context(current_rxpa_context);
    current_rxpa_context = 0;

    // Delete Static Function List
    while (static_linked_functions) {
        struct static_linked_function *next = static_linked_functions->next;
        free(static_linked_functions);
        static_linked_functions = next;
    }
    while (static_linked_metadata) {
        struct static_linked_metadata *next = static_linked_metadata->next;
        free(static_linked_metadata);
        static_linked_metadata = next;
    }

    return (int)(n); /* Module Number */
}
