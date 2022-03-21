/* CREXX Module Loader */

#include <stdlib.h>
#include "rxvmintp.h"
#include "rxastree.h"
#include "rxvmvars.h"

/* Initialise modules context */
void rxinimod(rxvm_context *context) {
    context->num_modules = 0;
    context->exposed_proc_tree = 0;
    context->exposed_reg_tree = 0;
    context->debug_mode = 0;
    context->location = 0;

    /* Support 64 modules initially - this grows automatically */
    context->module_buffer_size = 64;
    context->modules = malloc(sizeof(module) * context->module_buffer_size);
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
        free(context->modules[j].segment.binary);
        free(context->modules[j].segment.const_pool);
        free(context->modules[j].globals);
        free(context->modules[j].globals_dont_free);
    }
    free(context->modules);
}

/* Loads a new module
 * returns 0  - Success
 *         >0 - the number of unresolved references
 *         -1 - Error loading file */
int rxldmod(rxvm_context *context, char *file_name) {
    FILE *fp;
    size_t n, i;
    value *g_reg;
    size_t mod_index;
    chameleon_constant *c_entry;
    proc_constant *p_entry, *p_entry_linked;

    n = context->num_modules;

    DEBUG("Loading Module %s\n", file_name);

    /* Grow the module buffer if need be */
    while (n + 1 > context->module_buffer_size) {
        context->module_buffer_size *= 2;
        context->modules = realloc(context->modules, sizeof(module) * context->module_buffer_size);
    }

    fp = openfile(file_name, "rxbin", context->location, "rb");
    if (!fp) return -1;

    fread(&context->modules[n].segment.globals, 1, sizeof(int), fp);
    fread(&context->modules[n].segment.inst_size, 1, sizeof(size_t), fp);
    fread(&context->modules[n].segment.const_size, 1, sizeof(size_t), fp);

    context->modules[n].segment.binary = calloc(context->modules[n].segment.inst_size, sizeof(bin_code));
    context->modules[n].segment.const_pool = calloc(context->modules[n].segment.const_size, 1);

    fread(context->modules[n].segment.binary, sizeof(bin_code), context->modules[n].segment.inst_size, fp);
    fread(context->modules[n].segment.const_pool, 1, context->modules[n].segment.const_size, fp);

    context->modules[n].segment.module = &(context->modules[n]);
    context->modules[n].globals = calloc(context->modules[n].segment.globals, sizeof(value*));
    context->modules[n].globals_dont_free = calloc(context->modules[n].segment.globals, sizeof(char));

    context->modules[n].name = file_name;
    context->modules[n].unresolved_symbols = 0;
    context->modules[n].duplicated_symbols = 0;

    context->modules[n].module_number = n + 1;
    context->num_modules = n + 1;
    fclose(fp);

    /* Link Module */
    DEBUG("Add Module Symbols\n");
    mod_index = n;
    i = 0;
    while (i < context->modules[mod_index].segment.const_size) {
        c_entry =
                (chameleon_constant *) (
                        context->modules[mod_index].segment.const_pool + i);
        switch (c_entry->type) {

            case PROC_CONST:
                if (((proc_constant *) c_entry)->start != SIZE_MAX) {
                    /* Mark the owning module segment address */
                    ((proc_constant *) c_entry)->binarySpace =
                            &context->modules[mod_index].segment;
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
                            .globals[((expose_reg_constant *) c_entry)
                            ->global_reg] =
                            g_reg;
                    context->modules[mod_index]
                            .globals_dont_free[((expose_reg_constant *) c_entry)
                            ->global_reg] = 1;
                } else {
                    /* Need to initialise a register and expose it in the search tree */
                    context->modules[mod_index]
                            .globals[((expose_reg_constant *) c_entry)
                            ->global_reg] = value_f();
                    add_node(&context->exposed_reg_tree, ((expose_reg_constant *)c_entry)->index,
                             (size_t)(context->modules[mod_index]
                                         .globals[((expose_reg_constant *)c_entry)
                                         ->global_reg]));
                }
                break;

            case EXPOSE_PROC_CONST:
                /* Exposed Procedure */
                p_entry =
                        (proc_constant *) (
                                context->modules[mod_index].segment.const_pool
                                + ((expose_proc_constant *) c_entry)
                                        ->procedure);

                if (((expose_proc_constant *) c_entry)->imported) {
                    /* Imported - Add to the unresolved symbols count for later */
                    context->modules[mod_index].unresolved_symbols++;
                }
                else {
                    /* Exported - check duplicate */
                    if (add_node(&context->exposed_proc_tree,
                                 ((expose_proc_constant *) c_entry)->index,
                                 (size_t) p_entry)) {
                        DEBUG("WARNING: Duplicate exposed symbol: %s\n",
                                ((expose_proc_constant *) c_entry)->index);
                        context->modules[mod_index].duplicated_symbols++;
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
        if (!context->modules[mod_index].unresolved_symbols) continue;

        i = 0;
        while (i < context->modules[mod_index].segment.const_size) {
            c_entry =
                    (chameleon_constant *) (
                            context->modules[mod_index].segment.const_pool + i);
            switch (c_entry->type) {
                case EXPOSE_PROC_CONST:
                    if (((expose_proc_constant *) c_entry)->imported) {
                        if (src_node(context->exposed_proc_tree,
                                      ((expose_proc_constant *) c_entry)->index,
                                      (size_t *) &p_entry_linked)) {

                            /* Patch the procedure entry with the linked one */
                            p_entry =
                                    (proc_constant *) (
                                            context->modules[mod_index].segment.const_pool
                                            + ((expose_proc_constant *) c_entry)
                                                    ->procedure);
                            p_entry->locals = p_entry_linked->locals;
                            p_entry->start = p_entry_linked->start;
                            p_entry->binarySpace = p_entry_linked->binarySpace;
                            p_entry->frame_free_list =
                                    p_entry_linked->frame_free_list;

                            /* Reduce the number of unresolved symbols */
                            context->modules[mod_index].unresolved_symbols--;
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
    mod_index = n;
    for (i = 0; i < context->modules[mod_index].segment.globals; i++) {
        if (!context->modules[mod_index].globals[i]) {
            context->modules[mod_index].globals[i] = value_f();
        }
    }

    return 0;
}


