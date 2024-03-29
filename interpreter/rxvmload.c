/* CREXX Module Loader */

#include <stdlib.h>
#include "rxvmintp.h"
#include "rxastree.h"
#include "rxvmvars.h"
#include "rxbin.h"

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
        free(context->modules[j]->globals);
        free(context->modules[j]->globals_dont_free);
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
                    /* Mark the owning module segment address */
                    ((proc_constant *) c_entry)->binarySpace =
                            &context->modules[mod_index]->segment;
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
static size_t prep_and_link_module(rxvm_context *context, module_file *file_module_section) {
    size_t n = context->num_modules;

    DEBUG("Loading Module %s\n", file_module_section->name);

    /* Grow the module buffer if need be */
    while (n + 1 > context->module_buffer_size) {
        context->module_buffer_size *= 2;
        context->modules = realloc(context->modules, sizeof(module*) * context->module_buffer_size);
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

    context->num_modules = context->modules[n]->module_number = n + 1;

    link_module(context, n);
    return n;
}

/* Loads a module from a file
 * returns 0  - Error
 *         >0 - Last Module Number loaded (1 based) (more than one might have been loaded ...)  */
int rxldmod(rxvm_context *context, char *file_name) {
    FILE *fp;
    module_file *file_module_section;
    size_t modules_processed = 0;
    int loaded_rc;
    size_t n;

    DEBUG("Loading Module(s) from file %s\n", file_name);

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
                    return 0;
                }
                break;

            default: /* error */
                if (file_module_section) free_module(file_module_section);
                fprintf(stderr, "ERROR: reading file %s\n", file_name);
                exit(-1);
        }
    }

    fclose(fp);

    return (int)(n+1); /* Module Number */
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

    return (int)(n+1); /* Module Number */
}