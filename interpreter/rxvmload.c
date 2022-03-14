/* CREXX Module Loader */

#include <stdlib.h>
#include "rxvmintp.h"
#include "rxastree.h"

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
    }
    free(context->modules);
}

/* Loads a new module
 * returns 0  - Success
 *         >0 - the number of unresolved references
 *         -1 - Error loading file */
int rxldmod(rxvm_context *context, char *file_name) {
    FILE *fp;
    size_t n;

    n = context->num_modules;

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
    context->modules[n].globals = calloc(context->modules[n].segment.globals, sizeof(value));

    fclose(fp);

    context->modules[n].name = file_name;
    context->modules[n].module_number = n + 1;
    context->num_modules = n + 1;

    return 0;
}


