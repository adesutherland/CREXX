/* CREXX Module Loader */

#include <stdlib.h>
#include "rxvmintp.h"
#include "rxastree.h"
#include "rxvmvars.h"
#include "rxpa.h"

// RXPA (Plugin Architecture) Support declarations  for this file

/* Context for RXPA Functions / callbacks */
typedef  struct rxpa_context {
    rxvm_context *rxvm_context;
    module_file *plugin_being_loaded;
    size_t const_buffer_size;
    size_t const_buffer_top;
} rxpa_context;

/* Statically Linked Plugin List */
struct static_linked_function {
    char *name;
    rxpa_libfunc func;
    struct static_linked_function *next;
};
static struct static_linked_function *static_linked_functions = 0;

// Local Global Context Variable
static rxpa_context *current_rxpa_context = 0;

// Create a new RXPA context for a module
static rxpa_context *rxpa_context_f(rxvm_context *rxvm_context);

// Free statically linked functions list
static void free_rxpa_context(rxpa_context *context);

// End of RXPA Declarations for this file

/* Initialise modules context */
void rxinimod(rxvm_context *context) {
    context->num_modules = 0;
    context->exposed_proc_tree = 0;
    context->exposed_reg_tree = 0;
    context->debug_mode = 0;
    context->location = 0;

    /* Support 128 modules initially - this grows automatically */
    context->module_buffer_size = 128;
    context->modules = malloc(sizeof(module*) * context->module_buffer_size);
}

/* Free Module Context */
void rxfremod(rxvm_context *context) {
    int j;

    /* Free Symbol Search Trees */
    DEBUG("Free Symbol Search Trees\n");
    free_tree(&context->exposed_proc_tree);
    context->exposed_proc_tree = 0;
    free_tree(&context->exposed_reg_tree);
    context->exposed_reg_tree = 0;

    /* Free Program Modules */
    for (j=0; j<context->num_modules; j++) {
        free_module(context->modules[j]->file);
        if (context->modules[j]->globals) free(context->modules[j]->globals);
        if (context->modules[j]->globals_dont_free) free(context->modules[j]->globals_dont_free);
        free(context->modules[j]);
    }
    free(context->modules);
}

/* Link a loaded module */
static void link_module(rxvm_context *context, size_t module_number_to_link) {
    size_t i, mod_index;
    chameleon_constant *c_entry;
    proc_constant *p_entry, *p_entry_linked;
    value *g_reg;

    DEBUG("Add Module Symbols\n");
    i = 0;
    mod_index = module_number_to_link;
    while (i < context->modules[mod_index]->segment.const_size) {
        c_entry =
                (chameleon_constant *) (
                        context->modules[mod_index]->segment.const_pool + i);
        switch (c_entry->type) {

            case PROC_CONST:
                if (((proc_constant *) c_entry)->start != SIZE_MAX) {
                    if (context->modules[mod_index]->native) {
                        ((proc_constant *) c_entry)->binarySpace = 0;
                    }
                    else {
                        /* Mark the owning module segment address */
                        ((proc_constant *) c_entry)->binarySpace =
                                &context->modules[mod_index]->segment;
                    }
                    /* Stack Frame Free List */
                    ((proc_constant *) c_entry)->frame_free_list =
                            &(((proc_constant *) c_entry)
                                    ->frame_free_list_head);
                    *(((proc_constant *) c_entry)->frame_free_list) = 0;
                }
                break;

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
                p_entry =
                        (proc_constant *) (
                                context->modules[mod_index]->segment.const_pool
                                + ((expose_proc_constant *) c_entry)
                                        ->procedure);

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

        i += c_entry->size_in_pool;
    }

    DEBUG("Resolve Symbols\n");
    for (mod_index = 0; mod_index < context->num_modules; mod_index++) {
        /* Skip modules without any unresolved symbols */
        if (!context->modules[mod_index]->unresolved_symbols) continue;
        i = 0;
        while (i < context->modules[mod_index]->segment.const_size) {
            c_entry =
                    (chameleon_constant *) (
                            context->modules[mod_index]->segment.const_pool + i);
            switch (c_entry->type) {
                case EXPOSE_PROC_CONST:
                    if (((expose_proc_constant *) c_entry)->imported) {
                        if (src_node(context->exposed_proc_tree,
                                     ((expose_proc_constant *) c_entry)->index,
                                     (size_t *) &p_entry_linked)) {

                            /* Patch the procedure entry with the linked one */
                            p_entry =
                                    (proc_constant *) (
                                            context->modules[mod_index]->segment.const_pool
                                            + ((expose_proc_constant *) c_entry)
                                                    ->procedure);
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

            i += c_entry->size_in_pool;
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

    context->num_modules = context->modules[n]->module_number = n + 1;

    link_module(context, n);
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
    if (fileexists(file_name, "rxbin", context->location)) {
        DEBUG("CREXX Module file\n");
        fp = openfile(file_name, "rxbin", context->location, "rb");
        if (!fp) return 0;

        loaded_rc = 0;
        while (loaded_rc == 0) {
            file_module_section = 0;
            switch (loaded_rc = read_module(&file_module_section, fp)) {
                case 0: /* Success */
                    n = prep_and_link_module(context, file_module_section);
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

    // Check if the file is a crexx native plugin
    else if (fileexists(file_name, "rxplugin", context->location)) {
        DEBUG("CREXX Native Plugin file\n");

        // Create the rxpa_initctxptr context
        struct rxpa_initctxptr rxpa_functions;
        rxpa_functions.addfunc = rxpa_addfunc;
        rxpa_functions.getstring = rxpa_getstring;
        rxpa_functions.setstring = rxpa_setstring;
        rxpa_functions.setint = rxpa_setint;
        rxpa_functions.getint = rxpa_getint;
        rxpa_functions.setfloat = rxpa_setfloat;
        rxpa_functions.getfloat = rxpa_getfloat;
        rxpa_functions.getnumattrs = rxpa_getnumattrs;
        rxpa_functions.setnumattrs = rxpa_setnumattrs;
        rxpa_functions.getattr = rxpa_getattr;
        rxpa_functions.insertattr = rxpa_insertattr;
        rxpa_functions.removeattr = rxpa_removeattr;
        rxpa_functions.swapattrs = rxpa_swapattrs;

        // Exit Function Management
        rxpa_functions.setsayexit = rxpa_setsayexit;
        rxpa_functions.resetsayexit = rxpa_resetsayexit;

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
        int rc = load_plugin(&rxpa_functions, context->location, full_file_name);
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
  //  if (context->const_buffer) free(context->const_buffer);
    free(context);
}

// Create a new RXPA context for a module
static rxpa_context *rxpa_context_f(rxvm_context *rxvm_context) {
    rxpa_context *new_rxpa_context = malloc(sizeof(rxpa_context));
    new_rxpa_context->rxvm_context = rxvm_context;
    new_rxpa_context->const_buffer_size = 0;
    new_rxpa_context->const_buffer_top = 0;
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
}

// RXPA Add Function Implementation
// This is the callback function for rxldmod() when the plugin adds functions,
// or is called during initialising a statically linked plugin
void rxpa_addfunc(rxpa_libfunc func, char* name, __attribute__((unused)) char* option, __attribute__((unused)) char* type, __attribute__((unused)) char* args) {

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

/* Loads statically loaded plugins
 * returns -1  - Error
 *         >=0 - Last Module Number loaded (1 based) (more than one (or none) might have been loaded ...)  */
int rxldmodp(rxvm_context *context) {
    size_t n;

    if (static_linked_functions == 0) return 0;

    DEBUG("Loading Statically linked Plugins\n");

    // Create the rxpa_initctxptr context
    struct rxpa_initctxptr rxpa_functions;
    rxpa_functions.addfunc = rxpa_addfunc;
    rxpa_functions.getstring = rxpa_getstring;
    rxpa_functions.setstring = rxpa_setstring;
    rxpa_functions.setint = rxpa_setint;
    rxpa_functions.getint = rxpa_getint;
    rxpa_functions.setfloat = rxpa_setfloat;
    rxpa_functions.getfloat = rxpa_getfloat;
    rxpa_functions.getnumattrs = rxpa_getnumattrs;
    rxpa_functions.setnumattrs = rxpa_setnumattrs;
    rxpa_functions.getattr = rxpa_getattr;
    rxpa_functions.insertattr = rxpa_insertattr;
    rxpa_functions.removeattr = rxpa_removeattr;
    rxpa_functions.swapattrs = rxpa_swapattrs;

    // Exit Function Management
    rxpa_functions.setsayexit = rxpa_setsayexit;
    rxpa_functions.resetsayexit = rxpa_resetsayexit;

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
        rxpa_addfunc(static_func->func, static_func->name, 0, 0, 0) ;

        // Loop to next static function
        static_func = static_func->next;
    }

    // Link as a plugin
    n = prep_and_link_module(context, current_rxpa_context->plugin_being_loaded);

    // Free the rxpa_context
    free_rxpa_context(current_rxpa_context);
    current_rxpa_context = 0;

    // Delete Static Function List
    while (static_linked_functions) {
        struct static_linked_function *next = static_linked_functions->next;
        free(static_linked_functions);
        static_linked_functions = next;
    }

    return (int)(n); /* Module Number */
}
