//
// Logic for importing functions
// Created by Adrian Sutherland on 04/07/2022.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl_tree.h"
#include "platform.h"
#include "rxcpmain.h"
#include "rxbin.h"
#include "rxas.h"
#include "rxpa.h"
#ifndef NUTF8
#include "utf.h"
#endif


/* Internal Tree node structure for functions */
struct tree_wrapper {
    imported_func *func;
    struct avl_tree_node index_node;
};

#define GET_INDEX(i) avl_tree_entry((i), struct tree_wrapper, index_node)->func->fqname

#define GET_VALUE(i) avl_tree_entry((i), struct tree_wrapper, index_node)->func

static int compare_node_node(const struct avl_tree_node *node1,
                             const struct avl_tree_node *node2)
{
    char* n1 = GET_INDEX(node1);
    char* n2 = GET_INDEX(node2);
    return strcmp(n1,n2);
}

static int compare_node_value(const void *value,
                              const struct avl_tree_node *nodeptr) {
    char* node = GET_INDEX(nodeptr);
    return strcmp((char*)value,node);
}

// Search for a function / variable from fqname in the master context
// Returns 1 if found and sets value
// Returns 0 if not found
static int src_func(Context *context, char* fqname, imported_func **func) {
    struct avl_tree_node *result;
    struct avl_tree_node *root = context->master_context->importable_function_tree;

    result = avl_tree_lookup(root, fqname, compare_node_value);

    if (result) {
        *func = GET_VALUE(result);
        return 1;
    }
    else return 0;
}

// Search for a function / variable from namespace and name in the master context
// Returns 1 if found and sets value
// Returns 0 if not found
static int src_nsfu(Context *context, char* namespace, char* name, imported_func **func) {
    struct avl_tree_node *result;
    struct avl_tree_node *root = context->master_context->importable_function_tree;
    char *fqname;
    fqname = malloc(strlen(namespace) + strlen(name) + 2);
    strcpy(fqname,namespace);
    strcat(fqname, ".");
    strcat(fqname, name);

    result = avl_tree_lookup(root, fqname, compare_node_value);

    if (result) {
        *func = GET_VALUE(result);
        free(fqname);
        return 1;
    }

    free(fqname);
    return 0;
}

// Search for a function / variable from name (checking for imported namespaces in the context)
// if only_namespace is set then the search only covers the file namespace not imported ones
// (this is for global variables)
// Returns 1 if found and sets value
// Returns 0 if not found
static int src_fqfu(Context *context, int only_namespace, char* name, imported_func **func) {
    char *namespace;
    size_t i;

    /* Check the module namespace */
    namespace = scp_chd(context->ast->scope, 0)->name;
    if (src_nsfu(context, namespace, name, func)) return 1;

    /* Check imported namespaces */
    if (!only_namespace) {
        for (i = 1; i < scp_noch(context->ast->scope); i++) {
            namespace = scp_chd(context->ast->scope, i)->name;
            if (src_nsfu(context, namespace, name, func)) return 1;
        }
    }

    return 0;
}

/* Add a [inconsistent] duplicate symbol to the end of the symbols duplicate list */
static void add_inconsistent_duplicate(imported_func *first, imported_func *duplicate) {
    /* Loop to the end of the list */
    while (first->duplicate) first=first->duplicate;

    /* Add the duplicate */
    first->duplicate = duplicate;
}

/* strcmp() that handles nulls */
static int safe_strcmp(const char *s1, const char* s2) {
    if (s1==0 && s2==0) return 0;
    if (s1==0) return -1;
    if (s2==0) return 1;
    return strcmp(s1,s2);
}

/* Adds a func / variable to the master context*/
/* Returns 0 on success, 1 on duplicate
 * If it is a duplicate this function either calls freimpfc(func) or stashes it in the duplicate list
 * (the caller does not need to worry (should not worry) about freeing it if it is a duplicate) */
static int add_func(Context *context, imported_func *func) {
    struct avl_tree_node **root = (struct avl_tree_node **)&(context->master_context->importable_function_tree);
    imported_func *existing_func;
    /* Does the function already exist? */
    if ( src_func(context, func->fqname, &existing_func) ) {
        /* Yes a duplicate - so check it is consistent */

        /* Both should be a variable or a function */
        if (func->is_variable != existing_func->is_variable) {
            add_inconsistent_duplicate(existing_func, func);
            return 1;
        }

        /* Both should have the same type */
        if (safe_strcmp(func->type, existing_func->type) != 0) {
            add_inconsistent_duplicate(existing_func, func);
            return 1;
        }

        /* If a function both should have the same arguments */
        if (func->is_variable!=0 && safe_strcmp(func->args, existing_func->args) != 0) {
            add_inconsistent_duplicate(existing_func, func);
            return 1;
        }

        /* Otherwise, it is just a consistent duplicate - not an error */
        /* We are responsible for freeing it */
        freimpfc(func);
        return 1;
    }

    struct tree_wrapper *i = malloc(sizeof(struct tree_wrapper));
    i->func = func;
    if (avl_tree_insert(root, &i->index_node, compare_node_node)) {
        /* Duplicate */
        fprintf(stderr, "Internal Error: Unexpected duplicate symbol\n");
        free(i);
        /* We are responsible for freeing it */
        freimpfc(func);
        return 1;
    }
    return 0;
}

/* Free Func Tree and functions */
void fre_ftre(Context *context) {
    struct tree_wrapper *i;
    struct avl_tree_node **root = (struct avl_tree_node **)&(context->importable_function_tree);

    /* This walks the tree in post order which allows each node be freed */
    avl_tree_for_each_in_postorder(i, *root, struct tree_wrapper, index_node) {
        freimpfc(i->func);
        free(i);
    }
    *root = 0;
}

static void dump_error_ast(Context *context) {
#ifndef __CMS__
    if (context->debug_mode) {
        pdot_tree(context->ast, "error", context->file_name);
    }
#endif
}

static char error_in_node(ASTNode* node) {  // NOLINT
    if (node->node_type == ERROR) return 1;
    node = node->child;
    while (node) {
        if (error_in_node(node)) return 1;
        node = node->sibling;
    }
    return 0;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"
static walker_result procedure_signature_walker(walker_direction direction,
                                                ASTNode* node,
                                                void *pl) {

    Context *context = pl;
    ASTNode *type_node;
    ASTNode *args_node;
    ASTNode *impl_node;
    imported_func *func;
    char *type;
    char *args;
//    char *impl;
    char *fqname;

    if (direction == out) {
        /* OUT - BOTTOM UP */
        if (node->node_type == PROCEDURE) {
            type_node = ast_chld(node, CLASS, VOID);
            args_node = ast_chld(node, ARGS, 0);
            impl_node = ast_chld(node, INSTRUCTIONS, NOP);
            fqname = sym_frnm(node->symbolNode->symbol);

            if (impl_node && impl_node->node_type != NOP && node->symbolNode->symbol->exposed) {

                if ( !type_node || !args_node || error_in_node(type_node) || error_in_node(args_node) ) {
                    /* Error in the syntax of the important function */
                    func = rximpf_f(context, node->file_name, fqname, "b", 0, 0, 0, 0);
                    if (func) func->error_state = "SYNTAX_ERROR_IN_IMPORT_DECL";
                }

                else {
                    /* Add the function to the "database" of functions */
                    type = ast_n2tp(type_node);
                    args = meta_narg(args_node);

                    func = rximpf_f(context, node->file_name, fqname, "b", type, args, 0, 0);

                    /* Check Type <> unknown */
                    if ( !type || strcmp(type, ".unknown") == 0 ) {
                        if (func) func->error_state = "SYNTAX_ERROR_IN_IMPORT_DECL";
                    }

                    free(type);
                    free(args);
                }
            }
            free(fqname);
        }

    }
    return result_normal;
}
#pragma clang diagnostic pop

/* Get the constant string in a malloced buffer */
static char* get_const_string(void* constpool, size_t ix) {
    char *result;
    char *c;
    size_t sz;

    if (ix == -1) return 0;

    c = ((string_constant *)(constpool + ix))->string;
    sz = ((string_constant *)(constpool + ix))->string_len;

    result = malloc(sz + 1);
    if (result) {
        memcpy(result, c, sz);
        result[sz] = 0;
    }
    return result;
}

/* Get the global variable type by reading metadata */
/* name is null terminated fqname of the variable */
/* returns found meta_reg_constant entry - or 0 if not found */
static meta_reg_constant* get_variable_type(char* name, void* constant, int meta_head) {
    meta_entry *entry;
    meta_reg_constant *mentry;
    size_t i;
    char* fqname;

    i = meta_head;
    while (i != -1) {
        entry = (meta_entry *) (constant + i);

        if (entry->base.type == META_REG) {
            mentry = (meta_reg_constant *) entry;
            fqname = get_const_string(constant, mentry->symbol);
            if (fqname) {
                if (strcmp(fqname, name) == 0) {
                    free(fqname);
                    return mentry;
                }
                free(fqname);
                fqname = 0;
            }
        }

        i = entry->next;
    }
    return 0;
}

static void read_constant_pool_for_functions(Context *context, char *full_file_name, void* constant, size_t constant_size, int meta_head) {
    chameleon_constant *entry;
    size_t i;
    size_t exposed_ix;
    expose_proc_constant *exposed;
    char* fqname;
    char* option = 0;
    char* type = 0;
    char* args = 0;
    char* inliner = 0;

    /* Loop through all the constant entries */
    i = 0;
    while (i < constant_size) {
        entry = (chameleon_constant *) (constant + i);

        /* A function definition */
        if (entry->type == META_FUNC) {
            meta_func_constant *mentry = (meta_func_constant *) entry;

            /* Check it is exposed */
            exposed_ix = ((proc_constant *) (constant + mentry->func))->exposed;
            if ((int) exposed_ix != -1) {
                exposed = (expose_proc_constant *) (constant + exposed_ix);

                /* Exported */
                if (!exposed->imported) {
                    fqname = exposed->index;
                    option = get_const_string(constant, mentry->option);
                    type = get_const_string(constant, mentry->type);
                    args = get_const_string(constant, mentry->args);
                    inliner = get_const_string(constant, mentry->inliner);

                    rximpf_f(context, full_file_name, fqname, option, type, args, inliner, 0);

                    if (option) {
                        free(option);
                        option = 0;
                    }
                    if (type) {
                        free(type);
                        type = 0;
                    }
                    if (args) {
                        free(args);
                        args = 0;
                    }
                    if (inliner) {
                        free(inliner);
                        inliner = 0;
                    }
                }
            }
        }

        /* A exposed global variable */
        else if (entry->type == EXPOSE_REG_CONST) {
            fqname = ((expose_reg_constant *)entry)->index;
            meta_reg_constant *mentry = get_variable_type(fqname, constant, meta_head);
            if (mentry) {
                option = get_const_string(constant, mentry->option);
                type = get_const_string(constant, mentry->type);

                rximpf_f(context, full_file_name, fqname, option, type, "", "", 1);

                if (option) {
                    free(option);
                    option = 0;
                }
                if (type) {
                    free(type);
                    type = 0;
                }
            }
        }

        i += entry->size_in_pool;
    }
}

// RXPA Disabler Function
static char* plugin_being_loaded = "statically linked";
static Context* plugin_being_loaded_context = 0;

static void disablerFunction(char* fname) {
    fprintf(stderr, "RXC Panic: When loading %s plugin, it called %s() illegally\n", plugin_being_loaded, fname);
    exit(EXIT_FAILURE);
}

// RXPA Helper functions required to be provided - dummy functions
char* rxpa_getstring(rxpa_attribute_value attributeValue)  /* Get a string from an attribute value */
    { disablerFunction("rxpa_getstring"); return NULL; }

void rxpa_setstring(rxpa_attribute_value attributeValue, char* string)  /* Set a string in an attribute value */
    { disablerFunction("rxpa_setstring"); }

void rxpa_setint(rxpa_attribute_value attributeValue, rxinteger value)  /* Set an integer in an attribute value */
    { disablerFunction("rxpa_setint"); }

rxinteger rxpa_getint(rxpa_attribute_value attributeValue)  /* Get an integer from an attribute value */
    { disablerFunction("rxpa_getint"); return 0; }

void rxpa_setfloat(rxpa_attribute_value attributeValue, double value)  /* Set a float in an attribute value */
    { disablerFunction("rxpa_setfloat"); }

double rxpa_getfloat(rxpa_attribute_value attributeValue)  /* Get a float from an attribute value */
    { disablerFunction("rxpa_getfloat"); return 0.0; }

rxinteger rxpa_getnumattrs(rxpa_attribute_value attributeValue)  /* Get the number of child attributes */
    { disablerFunction("rxpa_getnumattrs"); return 0; }

void rxpa_setnumattrs(rxpa_attribute_value attributeValue, rxinteger numAttrs)  /* Set the number of child attributes */
    { disablerFunction("rxpa_setnumattrs"); }

rxpa_attribute_value rxpa_getattr(rxpa_attribute_value attributeValue, rxinteger index)  /* Get the nth child attribute */
    { disablerFunction("rxpa_getattr"); return NULL; }

rxpa_attribute_value rxpa_insertattr(rxpa_attribute_value attributeValue, rxinteger index)  /* Insert a child attribute before the nth position */
    { disablerFunction("rxpa_insertattr"); return NULL; }

void rxpa_removeattr(rxpa_attribute_value attributeValue, rxinteger index)  /* Remove the nth child attribute */
    { disablerFunction("rxpa_removeattr"); }

void rxpa_swapattrs(rxpa_attribute_value attributeValue, rxinteger index1, rxinteger index2)  /* Swap the nth child attribute with the mth child attribute */
    { disablerFunction("rxpa_swapattrs"); }

// Exit Function Management
void rxpa_setsayexit(say_exit_func sayExitFunc)  /* Set Say exit function */
    { disablerFunction("rxpa_setsayexit"); }

void rxpa_resetsayexit()  /* Reset Say exit function */
    { disablerFunction("rxpa_resetsayexit"); }

// RXPA Add Function Implementation
// This is the callback function for loadPluginFileForFunctions() when the plugin adds functions,
// oir is called during initialising a statically linked plugin
void rxpa_addfunc(rxpa_libfunc func, char* name, char* option, char* type, char* args) {
    if (plugin_being_loaded_context) {
        if (plugin_being_loaded_context->debug_mode) printf("Importing Procedures - Loading %s\n", name);
        rximpf_f(plugin_being_loaded_context, plugin_being_loaded, name, option, type, args, 0, 0);
    }
    else {
        // Add to the list of statically linked functions
        struct static_linked_function *new_static_func = malloc(sizeof(struct static_linked_function));
        new_static_func->name = name;
        new_static_func->option = option;
        new_static_func->type = type;
        new_static_func->args = args;
        new_static_func->next = static_linked_functions;
        static_linked_functions = new_static_func;
    }
}

static void loadPluginFileForFunctions(Context *context, char* file_name, char* location) {

    /* Update context */
    plugin_being_loaded = file_name;
    plugin_being_loaded_context = context;

    // Create the rxpa_initctxptr context
    struct rxpa_initctxptr rxpa_context;
    rxpa_context.addfunc = rxpa_addfunc;
    rxpa_context.getstring = rxpa_getstring;
    rxpa_context.setstring = rxpa_setstring;
    rxpa_context.setint = rxpa_setint;
    rxpa_context.getint = rxpa_getint;
    rxpa_context.setfloat = rxpa_setfloat;
    rxpa_context.getfloat = rxpa_getfloat;
    rxpa_context.getnumattrs = rxpa_getnumattrs;
    rxpa_context.setnumattrs = rxpa_setnumattrs;
    rxpa_context.getattr = rxpa_getattr;
    rxpa_context.insertattr = rxpa_insertattr;
    rxpa_context.removeattr = rxpa_removeattr;
    rxpa_context.swapattrs = rxpa_swapattrs;
    rxpa_context.setsayexit = rxpa_setsayexit;
    rxpa_context.resetsayexit = rxpa_resetsayexit;

    if (context->debug_mode) printf("Importing Procedures - Reading CREXX Plugin file %s for possible procedure imports\n", file_name);

    // Load the plugin - and run the plugin initialization function
    int rc = load_plugin(&rxpa_context, location, file_name);
    if (!rc) {
        if (context->debug_mode) printf("Importing Procedures - CREXX Plugin %s loaded successfully\n", file_name);
    }
    else {
        fprintf(stderr, "Importing Procedures - Failed to load plugin %s\n", file_name);
    }
}

static void parseRxasFileForFunctions(Context *context, char* file_name, char* location) {
    FILE *outFile;
    int token_type;
    Assembler_Token *token;
    bin_space *pgm;
    module_file module;

    Assembler_Context scanner;
    int i;

    /* Zero Context */
    memset(&scanner, 0, sizeof(Assembler_Context));

    /* scanner context parameter */
    scanner.debug_mode = context->debug_mode;
    scanner.traceFile = 0;
    scanner.optimise = 0;
    scanner.file_name = file_name;
    scanner.output_file_name = 0;
    scanner.location = location;

    if (context->debug_mode) printf("Importing Procedures - Reading RXAS file %s for possible procedure imports\n", file_name);

    /* Opening an Assembler file */
    if (rxasinfl(&scanner, 1)) {
        rxasclrc(&scanner);
        return;
    }

    /* Parse & Process */
    rxaspars(&scanner);
    read_constant_pool_for_functions(context, file_name, scanner.binary.const_pool,
                                     scanner.binary.const_size, scanner.meta_head);

    rxasclrc(&scanner);
}

static void parseRxbinFileForFunctions(Context *context, char* file_name, char* location) {
    FILE *fp;
    module_file *file_module_section;
    size_t modules_processed = 0;
    int loaded_rc;
    char *full_file_name;

    if (context->debug_mode) printf("Importing Procedures - Reading RXBIN file %s for possible procedure imports\n", file_name);

    fp = openfile(file_name, "", location, "rb");
    if (!fp) {
        if (context->debug_mode) printf("Importing Procedures - Could not open file\n");
        return;
    }

    loaded_rc = 0;
    while (loaded_rc == 0) {
        file_module_section = 0;
        switch (loaded_rc = read_module(&file_module_section, fp)) {
            case 0: /* Success */

                full_file_name = mprintf("%s@%s", file_module_section->name, file_name);

                read_constant_pool_for_functions(context, full_file_name, file_module_section->constant,
                                                 file_module_section->header.constant_size, file_module_section->header.meta_head);
                free(full_file_name);
                free_module(file_module_section);
                modules_processed++;
                break;

            case 1: /* eof */
                if (file_module_section) free_module(file_module_section);
                if (!modules_processed) {
                    if (context->debug_mode) printf("Importing Procedures - Empty file\n");
                    return;
                }
                break;

            default: /* error */
                if (file_module_section) free_module(file_module_section);
                if (context->debug_mode) printf("Importing Procedures - Error reading file\n");
                return;
        }
    }

    fclose(fp);
}

static void parseRexxFileForFunctions(Context *parent_context, char* file_name, char* location) {
    size_t bytes;
    Context *context;
    char *buff_start;
    imported_func *global;

    if (parent_context->debug_mode) printf("Importing Procedures - Reading REXX file %s for possible procedure imports\n", file_name);

    /* Context for parsing */
    context = cntx_f();

    /* Open input file */
    context->file_pointer = openfile(file_name, "", location, "r");
    if (context->file_pointer == NULL) {
        fprintf(stderr, "Panic Importing Procedures - Can't open input file: %s\n", file_name);
        exit(-1);
    }

    /* Propagate trace file */
#ifndef NDEBUG
    context->traceFile = parent_context->traceFile;
#endif

    buff_start = file2buf(context->file_pointer, &bytes);
    /* Close file */
    fclose(context->file_pointer);
    context->file_pointer  = 0;

    if (buff_start == NULL) {
        fprintf(stderr, "Importing Procedures - Can't read input file\n");
        exit(-1);
    }

    /* Initialize context */
    cntx_buf(context, buff_start, bytes);
    context->debug_mode = parent_context->debug_mode;
    context->location = parent_context->location;
    context->file_name = (char*) filename(file_name);

    /* Propagate the master_context */
    context->master_context = parent_context->master_context;

    /* Create Options parser to work out required language level */
    opt_pars(context);

    /* Deallocate memory and reset context */
    free_ast(context);
    free_tok(context);
    cntx_buf(context, buff_start, bytes);

    /* Parse program for real */
    switch (context->level){
        case LEVELA:
        case LEVELC:
        case LEVELD:
            if (parent_context->debug_mode) fprintf(stderr,"Importing Procedures - REXX Level A/C/D (cREXX Classic) - Not supported yet\n");
            break;

        case LEVELB:
        case LEVELG:
        case LEVELL:
            rexbpars(context);
            break;

        default:
            if (parent_context->debug_mode) fprintf(stderr, "Importing Procedures - Failed to determine REXX Level of imported rexx file\n");
    }

    if (!context->ast) {
        if (parent_context->debug_mode) fprintf(stderr,"Importing Procedures - Failure to create AST of imported rexx file\n");
        goto finish;
    }

    /* Extract Function Definitions  */
    rxcp_val(context); /* Full validation to ensure procedure types are handled */

#ifndef __CMS__
    if (parent_context->debug_mode) {
        pdot_tree(context->ast, "astimport", context->file_name);
    }
#endif

    ast_wlkr(context->ast, procedure_signature_walker, context);

    /* Extract Global Variables */
    /* Returns all the symbols in the scope of the PROGRAM_FILE node */
    Symbol **symbols = scp_syms(context->ast->child->scope);
    int i;
    for (i=0; symbols[i]; i++) {
        if (symbols[i]->symbol_type == VARIABLE_SYMBOL && symbols[i]->exposed) {
            /* import symbol */
            char* fqname = sym_frnm(symbols[i]);
            rximpf_f(parent_context, file_name, fqname, 0, type_nm(symbols[i]->type), 0, 0, 1);
            free(fqname);
        }
    }
    free(symbols);

    finish:

    /* Free context */
#ifndef NDEBUG
    context->traceFile =  0;
#endif
    fre_cntx(context);
}

/* Parse and rxcp_val a rexx program in a string and return the context */
static Context *parseRexx(Context* parent_context, char *location, char* file_name, RexxLevel level, int debug_mode, char* rexx_source, size_t bytes) {
    Context *context;

    switch (level){
        case LEVELA:
        case LEVELC:
        case LEVELD:
            fprintf(stderr,"INTERNAL ERROR: Importing Procedures - REXX Level A/C/D (cREXX Classic) - Not supported yet\n");
            return 0;

        case LEVELB:
        case LEVELG:
        case LEVELL:
            context = cntx_f();
            cntx_buf(context, rexx_source, bytes);

            /* Initialize context */
            context->location = location;
            context->file_name = file_name;
            context->level = level;
            context->debug_mode = debug_mode;
            context->master_context = parent_context->master_context;

            rexbpars(context);
            break;

        default:
            fprintf(stderr, "INTERNAL ERROR: Importing Procedures - Failed to determine REXX Level\n");
            return 0;
    }

    if (!context->ast) {
        if (debug_mode) fprintf(stderr,"INTERNAL ERROR: Importing Procedures - Compiler Exiting - Failure to create AST\n");
        goto finish;
    }

    rxcp_bvl(context);

#ifndef __CMS__
//    if (debug_mode) {
//        pdot_tree(context->ast, "parserexx.dot");
//        /* Get dot from https://graphviz.org/download/ */
//        system("dot parserexx.dot -Tpng -o parserexx.png");
//    }
#endif

    finish:

    return context;
}

/* Load the next importable file */
/* return 1 if a file was loaded, or 0 if no more files are available */
static int load_another_file(Context *context) {
    size_t f;
    Context *master_context = context->master_context;

    /* Load files from the importable_file_list */
    if (!master_context->importable_file_list) master_context->importable_file_list = rxfl_lst(master_context);
    if (!master_context->importable_file_list) {
        /* No files to import */
        return 0;
    }

    for (f = 0; master_context->importable_file_list[f]; f++) {
        /* Already imported? */
        if (!master_context->importable_file_list[f]->imported) {
            master_context->importable_file_list[f]->imported = 1;

            /* Import File */
            switch (master_context->importable_file_list[f]->type) {
                case REXX_FILE:
                    parseRexxFileForFunctions(context,
                                              master_context->importable_file_list[f]->name,
                                              master_context->importable_file_list[f]->location);
                    break;
                case RXBIN_FILE:
                    parseRxbinFileForFunctions(context,
                                               master_context->importable_file_list[f]->name,
                                               master_context->importable_file_list[f]->location);
                    break;
                case RXAS_FILE:
                    parseRxasFileForFunctions(context,
                                              master_context->importable_file_list[f]->name,
                                              master_context->importable_file_list[f]->location);
                    break;
                case NATIVE_FILE:
                    loadPluginFileForFunctions(context,
                                              master_context->importable_file_list[f]->name,
                                              master_context->importable_file_list[f]->location);
                    break;
            }
            return 1; /* File loaded - job done */
        }
    }

    // Process any statically linked functions
    if (static_linked_functions) {
        struct static_linked_function *static_func = static_linked_functions;
        while (static_func) {
            rximpf_f(context, "statically-linked",
                     static_func->name, static_func->option, static_func->type,
                     static_func->args, 0, 0);
            static_func = static_func->next;
        }

        // Free the list of statically linked functions -  we only load these once
        free_static_linked_functions();
        static_linked_functions = 0;
        return 1;
    }

    return 0; /* No file was loaded */
}

static ValueType type_from_string(char* type) {
    if (!type) return TP_UNKNOWN;
    if (strcmp(type, ".int") == 0) return TP_INTEGER;
    if (strcmp(type, ".float") == 0) return TP_FLOAT;
    if (strcmp(type, ".decimal") == 0) return TP_DECIMAL;
    if (strcmp(type, ".string") == 0) return TP_STRING;
    if (strcmp(type, ".binary") == 0) return TP_BINARY;
    if (strcmp(type, ".boolean") == 0) return TP_BOOLEAN;
    if (strcmp(type, ".void") == 0) return TP_VOID;
    if (strcmp(type, ".unknown") == 0) return TP_UNKNOWN;
    return TP_OBJECT;
}

/* Try and import an external function - return its symbol if successful */
Symbol *sym_imfn(Context *context, ASTNode *node) {
    Symbol *symbol;
    ASTNode *func_node;
    Symbol *func_symbol;
    Symbol *found_symbol = 0;
    Scope *namespace;
    imported_func *func;
    imported_func *found_func = 0;
    imported_func *inconsistent_func;
    ValueType tp;
    char error = 0;
    char* defining_file = context->file_name;

    char *name = (char*)malloc(node->node_string_length + 1);
    memcpy(name, node->node_string, node->node_string_length);
    name[node->node_string_length] = 0;

    /* Lowercase symbol name */
#ifdef NUTF8
    char *c;
    for (c = name; *c; ++c) *c = (char)tolower(*c);
#else
    utf8lwr(name);
#endif

    if (context->debug_mode) printf("Importing Procedures for file %s Looking for Procedure %s\n", defining_file, name);

    /* Process all the unread files - but we just are interested in the first found variable - duplicates done next */
    do {
        /* Check if the function has been loaded */
        if (src_fqfu(context, 0, name, &func)) {
            if (context->debug_mode)
                printf("Importing Procedures - Found Procedure %s in file %s\n", func->fqname, func->file_name);
            found_func = func;
            break;
        }
    } while (load_another_file(context));

    if (context->debug_mode) printf("Finished Importing files needed for file %s when Looking for Procedure %s\n", defining_file, name);

    if (found_func) {
        /* Compare found variable with the type defined in the master file being compiled */
        tp = type_from_string(found_func->type);
        if (found_func->is_variable) {
            mknd_err(node,
                     "PROC_VAR_MISMATCH, \"%s\", \"%s\", \"%s\"",
                     name,found_func->file_name, defining_file);
            error = 1;
        }

        /* Has the func got an error_state? */
        if (found_func->error_state) {
            mknd_err(node,
                     "%s, \"%s\", \"%s\"",
                     found_func->error_state, name, found_func->file_name);
            error = 1;
        }

        /* Produce Errors for all import inconsistencies */
        inconsistent_func = found_func->duplicate;
        while (inconsistent_func) {
            if (inconsistent_func->is_variable) {
                mknd_err(node,
                         "PROC_VAR_MISMATCH, \"%s\", \"%s\", \"%s\"",
                         name,inconsistent_func->file_name, defining_file);
                error = 1;
            }

            if (safe_strcmp(found_func->type, inconsistent_func->type)) {
                mknd_err(node,
                         "TYPE_MISMATCH, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
                         name,found_func->type, found_func->file_name, inconsistent_func->type,
                         inconsistent_func->file_name);
                error = 1;
            }

            if (safe_strcmp(found_func->args, inconsistent_func->args)) {
                mknd_err(node,
                         "ARGS_MISMATCH, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
                         name,found_func->args, found_func->file_name, inconsistent_func->args,
                         inconsistent_func->file_name);
                error = 1;
            }

            inconsistent_func = inconsistent_func->duplicate;
        }
        if (!error) {

            if (context->debug_mode)
                printf("Importing Procedures - Found Procedure %s\n", found_func->fqname);

            /* Splice the ASTs together */
            add_dast(context->ast, func->context->ast->child);
            found_symbol = sym_rfqn(context->ast, found_func->fqname);
            found_symbol->exposed = 1; /* Exposed by definition! */
            found_symbol->is_arg = 0; /* Can't expose args */
            found_symbol->is_opt_arg = 0;
            found_symbol->is_ref_arg = 0;
            found_symbol->is_const_arg = 0;
        }
    }

    free(name);
    return found_symbol;
}

/* Set the type of a symbol from imported modules */
void sym_imva(Context *context, Symbol *symbol) {
    imported_func *var;
    imported_func *found_var = 0;
    imported_func *inconsistent_var;
    ValueType tp;
    char error = 0;
    char* defining_file = 0;

    if (context->debug_mode) printf("Importing Globals - Looking for Global %s\n", symbol->name);

    defining_file = context->file_name;

    /* Process all the unread files - but we just are interested in the first found variable - duplicated done next */
    do {
        if (!found_var) {
            /* Check if the function has been loaded */
            if (src_fqfu(context, 1, symbol->name, &var)) {
                if (context->debug_mode) printf("Importing Globals - Found Global Symbol %s\n", var->fqname);
                found_var = var;
            }
        }
    } while (load_another_file(context));

    if (found_var) {
        /* Compare found variable with the type defined in the master file being compiled */
        tp = type_from_string(found_var->type);
        if (!found_var->is_variable) {
            mknd_err(sym_trnd(symbol, 0)->node, "PROC_VAR_MISMATCH, \"%s\", \"%s\", \"%s\"",
                     symbol->name, defining_file, found_var->file_name);
            error = 1;
        }
        else if (symbol->type != TP_UNKNOWN) {
            if ((tp != TP_UNKNOWN) && (tp != symbol->type)) {
                mknd_err(sym_trnd(symbol, 0)->node, "TYPE_MISMATCH, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
                         symbol->name,type_nm(symbol->type), defining_file, found_var->type, found_var->file_name);
                error = 1;
            }
        }

        /* Produce Errors for all import inconsistencies */
        inconsistent_var = found_var->duplicate;
        while (inconsistent_var) {
            if (!inconsistent_var->is_variable) {
                mknd_err(sym_trnd(symbol, 0)->node, "PROC_VAR_MISMATCH, \"%s\", \"%s\", \"%s\"",
                         inconsistent_var->name, defining_file, inconsistent_var->file_name);
                error = 1;
            }

            if (safe_strcmp(found_var->type, inconsistent_var->type)) {
                mknd_err(sym_trnd(symbol, 0)->node,
                         "TYPE_MISMATCH, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
                         found_var->name, found_var->type, found_var->file_name, inconsistent_var->type, inconsistent_var->file_name);
                error = 1;
            }
            inconsistent_var = inconsistent_var->duplicate;
        }
        if (!error) {
            symbol->type = tp;
        }
    }
}

/* imported_func factory - returns null if the function is not in an applicable namespace or is a duplicate */
imported_func *rximpf_f(Context* context, char* file_name, char *fqname, char *options, char *type, char *args,
                        char *implementation, char is_variable)  {
    imported_func *func;
    char *buffer;
    char *name;

    if (!fqname) return 0;

    /* Get the namespace (string before the last ".") from the fqn */
    size_t len  = strlen(fqname);
    while (len) {
        len--;
        if (fqname[len] == '.') break;
    }

    name = fqname + len + 1;

    /* OK - Create func */
    func = malloc(sizeof(imported_func));
    func->context = 0;
    func->is_variable = is_variable; /* Is a function or a Variable */
    func->duplicate = 0;
    func->error_state = 0;

    /* Store the namespace */
    func->namespace = malloc(len + 1);
    memcpy(func->namespace, fqname, len);
    func->namespace[len] = 0;

    /* Store the file name */
    func->file_name = malloc(strlen(file_name) + 1);
    strcpy(func->file_name,file_name);

    /* Store fqname */
    func->fqname = malloc(strlen(fqname) + 1);
    strcpy(func->fqname,fqname);

    /* Store the rest of the fields */
    if (name) {
        func->name = malloc(strlen(name) + 1);
        strcpy(func->name,name);
    }
    else func->name = 0;

    if (options) {
        func->options = malloc(strlen(options) + 1);
        strcpy(func->options,options);
    }
    else func->options = 0;

    if (type) {
        func->type = malloc(strlen(type) + 1);
        strcpy(func->type,type);
    }
    else func->type = 0;

    if (args) {
        func->args = malloc(strlen(args) + 1);
        strcpy(func->args,args);
    }
    else func->args = 0;

    if (implementation) {
        func->implementation = malloc(strlen(implementation) + 1);
        strcpy(func->implementation,implementation);
    }
    else func->implementation = 0;

    if (func->is_variable == 0) {
        /* Generate Function Declaration AST - only if we are a function not a variable */
        // TODO This only does Level B
        buffer = mprintf("options levelb\nnamespace %s\n%s: procedure = %s\narg %s\n", func->namespace, func->name,
                         func->type, func->args);
        if (context->debug_mode) printf("Importing Procedures - Analysing procedure %s\n", func->fqname);
        func->context = parseRexx(context, context->location, func->file_name, LEVELB, context->debug_mode, buffer,
                                  strlen(buffer));

        if (!func->context || error_in_node(func->context->ast)) {
            func->error_state = "INTERNAL_ERROR_PARSING_IMPORT_AST";
        }

        /* Make sure the AST seems sane */
        else if (func->context->ast->child->node_type != PROGRAM_FILE) {
            func->error_state = "INTERNAL_ERROR_PARSING_IMPORT_AST";
        }

        /* Fixup the node type */
        func->context->ast->child->node_type = IMPORTED_FILE;
    }

    if (add_func(context, func)) {
        /* Duplicate */
        func = 0;
    }

    return func;
}

/* Free an imported_func */
void freimpfc(imported_func *func) {
    if (func->namespace) free(func->namespace);
    if (func->file_name) free(func->file_name);
    if (func->fqname) free(func->fqname);
    if (func->name) free(func->name);
    if (func->options) free(func->options);
    if (func->type) free(func->type);
    if (func->args) free(func->args);
    if (func->implementation) free(func->implementation);
    if (func->context) fre_cntx(func->context);
    if (func->duplicate) freimpfc(func->duplicate);
    free(func);
}

/* free the list of importable files */
void rxfl_fre(importable_file **file_list) {
    importable_file **file;
    for (file = file_list; *file; file++) {
        free((*file)->name);
        if ((*file)->location) free((*file)->location);
        free(*file);
    }
    free(file_list);
}

static void add_file_to_list(importable_file *file, size_t *number, importable_file ***list) {
    (*number)++;
    *list = (importable_file**)realloc(*list, ((*number) + 1) * sizeof(importable_file*));
    (*list)[(*number)-1] = file;
    (*list)[*number] = 0;
}

static importable_file* importable_file_f(char* name, file_type type, char *location) {
    importable_file *file;
    file = malloc(sizeof(importable_file));
    file->name = malloc(strlen(name) + 1);
    strcpy(file->name, name);
    if (location) {
        file->location = malloc(strlen(location) + 1);
        strcpy(file->location, location);
    }
    else file->location = 0;
    file->imported = 0;
    file->type = type;
    return file;
}

/* Get a list of files if a type in a directory (can be null), skipping skip_name (can be null) */
static void list_files_in_dir(char *directory, file_type type, char* skip_name, importable_file ***list, size_t *number) {

    void *dir_ptr;
    char* name;
    importable_file *file;
    char* type_name;
    char *file_prefix = 0;

    switch (type) {
        case REXX_FILE:
            type_name = "rexx";
            break;
        case RXAS_FILE:
            type_name = "rxas";
            break;
        case RXBIN_FILE:
            type_name = "rxbin";
            break;
        case NATIVE_FILE:
            type_name = "rxplugin";
            file_prefix = "rx";
    }

    /* Read files in the directory */
    name = dirfstfl(directory,  file_prefix, type_name, &dir_ptr);
    if (name) {
        if (!skip_name || strcmp(name, skip_name) != 0 ) { // Skip if the same name as the file
            file = importable_file_f(name, type, directory);
            add_file_to_list(file, number, list);
        }
        do {
            name = dirnxtfl(&dir_ptr);
            if (name) {
                if (!skip_name || strcmp(name, skip_name) != 0 ) { // Skip if the same name as the file
                    file = importable_file_f(name, type, directory);
                    add_file_to_list(file, number, list);
                }
            }
        } while (name);
    }
    dirclose(&dir_ptr);

}

/* Get the list of importable files as a null terminated malloced array */
importable_file **rxfl_lst(Context *context) {
    size_t number = 0;
    importable_file **list = 0;
    char* exe_dir;
    size_t d;

    list = malloc( sizeof(importable_file*) );
    list[0] = 0;
    exe_dir = exepath();

    /* Read REXX files in the current directory */
    list_files_in_dir(context->location, REXX_FILE, context->file_name, &list, &number);

    /* Read RXAS files in the current directory */
    list_files_in_dir(context->location, RXAS_FILE, 0, &list, &number);

    /* Read RXBIN files in the current directory */
    list_files_in_dir(context->location, RXBIN_FILE, 0, &list, &number);

    /* Read in native plugins  in the current directory */
    list_files_in_dir(context->location, NATIVE_FILE, 0, &list, &number);

    /* Look in the imported location */
    if (context->import_locations) {
        for (d = 0; context->import_locations[d]; d++) {
            /* Read in REXX files in the directory */
            list_files_in_dir(context->import_locations[d], REXX_FILE, 0, &list, &number);

            /* Read in RXAS files in the directory */
            list_files_in_dir(context->import_locations[d], RXAS_FILE, 0, &list, &number);

            /* Read in RXBIN files in the directory */
            list_files_in_dir(context->import_locations[d], RXBIN_FILE, 0, &list, &number);

            /* Read in native plugins in the directory */
            list_files_in_dir(context->import_locations[d], NATIVE_FILE, 0, &list, &number);
        }
    }

    if (exe_dir) {
        /* Read in REXX files in the directory holding the compiler */
        list_files_in_dir(exe_dir, REXX_FILE, 0, &list, &number);

        /* Read in RXAS files in the directory holding the compiler */
        list_files_in_dir(exe_dir, RXAS_FILE, 0, &list, &number);

        /* Read in RXBIN files in the directory holding the compiler */
        list_files_in_dir(exe_dir, RXBIN_FILE, 0, &list, &number);

        /* Read in native plugins in the directory holding the compiler */
        list_files_in_dir(exe_dir, NATIVE_FILE, 0, &list, &number);

        free(exe_dir);
    }

    return list;
}

// Free statically linked functions list
void free_static_linked_functions()
{
    while (static_linked_functions) {
        struct static_linked_function *next = static_linked_functions->next;
        free(static_linked_functions);
        static_linked_functions = next;
    }
}
