//
// Logic for importing functions
// Created by Adrian Sutherland on 04/07/2022.
//

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "rxcpmain.h"
#include "rxcpdary.h"
#include "rxbin.h"
#include "rxas.h"
#ifndef NUTF8
#include "utf.h"
#endif

#include <string.h>
#include <stdlib.h>
#include "avl_tree.h"

/* Internal Tree node structure for functions */
struct tree_wrapper {
    char* index;
    imported_func *func;
    struct avl_tree_node index_node;
};

#define GET_INDEX(i) avl_tree_entry((i), struct tree_wrapper, index_node)->index

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

/* Adds a func */
/* Returns 0 on success, 1 on duplicate */
static int add_func(Context *context, imported_func *func) {
    struct avl_tree_node **root = (struct avl_tree_node **)&(context->importable_function_tree);

    struct tree_wrapper *i = malloc(sizeof(struct tree_wrapper));
    i->index = func->name;
    i->func = func;
    if (avl_tree_insert(root, &i->index_node, compare_node_node)) {
        /* Duplicate */
        free(i);
        return 1;
    }
    return 0;
}

// Search for a function / variable
// Returns 1 if found and sets value
// Returns 0 if not found
static int src_func(Context *context, char* index, imported_func **func) {
    struct avl_tree_node *result;
    struct avl_tree_node *root = context->importable_function_tree;

    result = avl_tree_lookup(root, index, compare_node_value);

    if (result) {
        *func = GET_VALUE(result);
        return 1;
    }
    else return 0;
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

/* TODO Move to Platform - need to make the output malloced */
static const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

/* TODO Move to Platform - need to make the output malloced */
static const char *get_filename(const char *path)
{
    size_t len = strlen(path);
    size_t i;
    if (!len) return "";

    for (i = len - 1; i; i--)
    {
        if ( path[i] == '\\' || path[i] == '/' )
        {
            path = path + i + 1;
            break;
        }
    }
    return path;
}

/* TODO Move to Platform */
/* Gets the directory of a filename in a malloced buffer */
/* returns null if there is no directory part */
static char *get_filename_directory(const char *path)
{
    size_t len = strlen(path);
    if (!len) return 0;
    char* result;

    for (len--; len; len--)
    {
        if ( path[len] == '\\' || path[len] == '/' )
        {
            result = malloc(len + 1);
            result[len] = 0;
            memcpy(result, path, len);
            return result;
        }
    }

    return 0;
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

    ASTNode *child1, *child2, *child3;

    Context *master_context = pl;
    imported_func *func;

    if (direction == out) {
        /* OUT - BOTTOM UP */
        child1 = node->child;
        if (!child1) return result_normal;

        child2 = child1->sibling;
        if (!child2) return result_normal;

        child3 = child2->sibling;
        if (!child3) return result_normal;

        if (node->node_type == PROCEDURE) {
            if (child3->node_type != NOP) {
                if  (error_in_node(child1)) return result_normal; /* If there is an error we must ignore this entry */
                if  (error_in_node(child2)) return result_normal; /* If there is an error we must ignore this entry */

                char *type = nodetype(child1);
                char *source = clnnode(child2);
                char *args = source;
                char *fqname = sym_frnm(node->symbolNode->symbol);

                func = rximpf_f(master_context, node->context->file_name, fqname, "b", type, args, 0, 0);

                if (func) {
                    add_func(master_context, func);
                    // TODO Remove dpa_add(master_context->importable_function_tree, func);
                }

                free(type);
                free(source);
                free(fqname);
            }
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

static void read_constant_pool_for_functions(Context *master_context, char *full_file_name, void* constant, size_t constant_size) {
    chameleon_constant *entry;
    size_t i;
    imported_func *func;
    size_t exposed_ix;
    expose_proc_constant *exposed;
    char* fqname;
    char* option = 0;
    char* type = 0;
    char* args = 0;
    char* inliner = 0;

    i = 0;
    while (i < constant_size) {
        entry = (chameleon_constant *) (constant + i);
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

                    func = rximpf_f(master_context, full_file_name, fqname, option, type, args, inliner, 0);

                    if (func) {
                        add_func(master_context, func);
                        // TODO Remove dpa_add(master_context->importable_function_tree, func);
                    }

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

        i += entry->size_in_pool;
    }
}

static void parseRxasFileForFunctions(Context *master_context, char* file_name, char* location) {
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
    scanner.debug_mode = master_context->debug_mode;
    scanner.traceFile = 0;
    scanner.optimise = 0;
    scanner.file_name = file_name;
    scanner.output_file_name = 0;
    scanner.location = location;

    if (master_context->debug_mode) printf("Importing Procedures - Reading RXAS file %s for possible procedure imports\n", file_name);

    /* Opening an Assembler file */
    if (rxasinfl(&scanner, 1)) {
        rxasclrc(&scanner);
        return;
    }

    /* Parse & Process */
    rxaspars(&scanner);

    read_constant_pool_for_functions(master_context, file_name, scanner.binary.const_pool,
                                     scanner.binary.const_size);

    rxasclrc(&scanner);
}

static void parseRxbinFileForFunctions(Context *master_context, char* file_name, char* location) {
    FILE *fp;
    module_file *file_module_section;
    size_t modules_processed = 0;
    int loaded_rc;
    char *full_file_name;

    if (master_context->debug_mode) printf("Importing Procedures - Reading RXBIN file %s for possible procedure imports\n", file_name);

    fp = openfile(file_name, "", location, "rb");
    if (!fp) {
        if (master_context->debug_mode) printf("Importing Procedures - Could not open file\n");
        return;
    }

    loaded_rc = 0;
    while (loaded_rc == 0) {
        file_module_section = 0;
        switch (loaded_rc = read_module(&file_module_section, fp)) {
            case 0: /* Success */

                full_file_name = mprintf("%s@%s", file_module_section->name, file_name);

                read_constant_pool_for_functions(master_context, full_file_name, file_module_section->constant,
                                                 file_module_section->header.constant_size);
                free(full_file_name);
                free_module(file_module_section);
                modules_processed++;
                break;

            case 1: /* eof */
                if (file_module_section) free_module(file_module_section);
                if (!modules_processed) {
                    if (master_context->debug_mode) printf("Importing Procedures - Empty file\n");
                    return;
                }
                break;

            default: /* error */
                if (file_module_section) free_module(file_module_section);
                if (master_context->debug_mode) printf("Importing Procedures - Error reading file\n");
                return;
        }
    }

    fclose(fp);
}

static void parseRexxFileForFunctions(Context *master_context, char* file_name, char* location) {
    size_t bytes;
    Context *context;
    char *buff_start;

    if (master_context->debug_mode) printf("Importing Procedures - Reading REXX file %s for possible procedure imports\n", file_name);

    /* Context for parsing */
    context = cntx_f();

    /* Open input file */
    context->file_pointer = openfile(file_name, "", location, "r");
    if (context->file_pointer == NULL) {
        fprintf(stderr, "Importing Procedures - Can't open input file: %s\n", file_name);
        exit(-1);
    }

    /* Propagate trace file */
#ifndef NDEBUG
    context->traceFile = master_context->traceFile;
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
    context->debug_mode = master_context->debug_mode;
    context->location = master_context->location;
    context->file_name = (char*)get_filename(file_name);

    /* Create Options parser to work out required language level */
    opt_pars(context);

    /* Deallocate memory and reset context */
    free_ast(context);
    free_tok(context);
    cntx_buf(context, buff_start, bytes);

    context->dont_import = 1; /* Don't try an import files to resolve procedures */

    /* Parse program for real */
    switch (context->level){
        case LEVELA:
        case LEVELC:
        case LEVELD:
            fprintf(stderr,"Importing Procedures - REXX Level A/C/D (cREXX Classic) - Not supported yet\n");
            break;

        case LEVELB:
        case LEVELG:
        case LEVELL:
            rexbpars(context);
            break;

        default:
            fprintf(stderr, "Importing Procedures - INTERNAL ERROR: Failed to determine REXX Level\n");
    }

    if (!context->ast) {
        fprintf(stderr,"Importing Procedures - INTERNAL ERROR: Compiler Exiting - Failure to create AST\n");
        goto finish;
    }

    /* Extract Function Definitions  */
    rxcp_val(context); /* Full validation to ensure procedure types are handled */
    ast_wlkr(context->ast, procedure_signature_walker, master_context);

    finish:

    /* Free context */
#ifndef NDEBUG
    context->traceFile =  0;
#endif
    fre_cntx(context);
}

/* Parse and rxcp_val a rexx program in a string and return the context */
Context *parseRexx(char *location, char* file_name, RexxLevel level, int debug_mode, char* rexx_source, size_t bytes) {
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
            context->dont_import = 1;

            rexbpars(context);
            break;

        default:
            fprintf(stderr, "INTERNAL ERROR: Importing Procedures - Failed to determine REXX Level\n");
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

/* Try and import an external function - return its symbol if successful */
Symbol *sym_imfn(Context *master_context, ASTNode *node) {
    size_t i, f;
    Symbol *symbol;
    ASTNode *func_node;
    Symbol *func_symbol;
    Symbol *found_symbol = 0;
    Scope *namespace;
    imported_func *func;

    char *name = (char*)malloc(node->node_string_length + 1);
    memcpy(name, node->node_string, node->node_string_length);
    name[node->node_string_length] = 0;

    /* Lowercase symbol name */
#ifdef NUTF8
    for (c = name ; *c; ++c) *c = (char)tolower(*c);
#else
    utf8lwr(name);
#endif

    if (master_context->debug_mode) printf("Importing Procedures - Looking for Procedure %s\n", name);

    /* Load files from the importable_file_list */
    if (!master_context->importable_file_list) master_context->importable_file_list = rxfl_lst(master_context);
    if (!master_context->importable_file_list) {
        /* No files to import */
        free(name);
        return 0;
    }

    i = 0;
    for (f = 0; master_context->importable_file_list[f] && !found_symbol; f++) {
        /* Already imported? */
        if (!master_context->importable_file_list[f]->imported) {
            master_context->importable_file_list[f]->imported = 1;

            /* Import File */
            switch (master_context->importable_file_list[f]->type) {
                case REXX_FILE:
                    parseRexxFileForFunctions(master_context,
                                              master_context->importable_file_list[f]->name,
                                              master_context->importable_file_list[f]->location);
                    break;
                case RXBIN_FILE:
                    parseRxbinFileForFunctions(master_context,
                                               master_context->importable_file_list[f]->name,
                                               master_context->importable_file_list[f]->location);
                    break;
                case RXAS_FILE:;
                    parseRxasFileForFunctions(master_context,
                                              master_context->importable_file_list[f]->name,
                                              master_context->importable_file_list[f]->location);
                    break;
            }
        }

        /* Check the added functions in the function list */

        if (src_func(master_context, name, &func) && !func->is_variable) {
            /* Found it! */
            if (func->already_loaded) {
                fprintf(stderr, "INTERNAL ERROR: Attempted to load function %s twice\n", func->name);
                return 0;
            }
            func->already_loaded = 1;

            if (master_context->debug_mode) printf("Importing Procedures - Found Procedure %s\n", func->fqname);

            /* Splice the ASTs together */
            add_ast(master_context->ast, func->context->ast->child);

            /* Splice in namespace symbol */
            symbol = sym_rslv(master_context->ast->scope, func->context->ast->child);
            if (!symbol) {
                symbol = sym_f(master_context->ast->scope, func->context->ast->child);
                symbol->symbol_type = NAMESPACE_SYMBOL;
                namespace = scp_f(master_context->ast->scope, node, symbol);
            } else {
                namespace = symbol->defines_scope;
            }
            sym_adnd(symbol, func->context->ast->child, 1, 1);
            if (func->context->namespace) sym_adnd(symbol, func->context->namespace, 0, 1);

            /* Splice new procedure symbol */
            /* Find the current procedure symbol */
            func_node = func->context->ast->child->child;
            if (func_node->node_type != PROCEDURE) func_node = func_node->sibling;

            if (func_node->node_type != PROCEDURE) {
                fprintf(stderr,
                        "INTERNAL ERROR: Importing Procedures - Could not find imported procedure in AST\n");
                dump_error_ast(func->context);
            } else {
                func_node->scope = namespace;
                func_symbol = func_node->symbolNode->symbol;

                /* Make a duplicate symbol in the master_context */
                found_symbol = sym_fn(namespace, func_symbol->name,
                                      strlen(func_symbol->name));
                if (!found_symbol) {
                    fprintf(stderr,
                            "INTERNAL ERROR: Importing Procedures - Could not duplicate imported procedure symbol\n");
                    dump_error_ast(func->context);
                } else {
                    /* Set symbol type */
                    found_symbol->symbol_type = func_symbol->symbol_type;

                    /* Splice by linking the new symbol to the function node */
                    sym_adnd(found_symbol, func_node, 0, 1);

                    /* remove old symbol */
                    scp_rmsy(func_symbol->scope, func_symbol); /* Remove from scope */
                    free_sym(func_symbol); /* Free symbol  */
                }
            }
        }
    }
    free(name);
    return found_symbol;
}

/* imported_func factory - returns null if the function is not in an applicable namespace */
imported_func *rximpf_f(Context*  master_context, char* file_name, char *fqname, char *options, char *type, char *args,
                        char *implementation, char is_variable)  {
    imported_func *func;
    char *buffer;
    size_t i;
    char found;
    char *name;

    if (!fqname) return 0;

    /* Get the namespace (string before the last ".") from the fqn */
    size_t len  = strlen(fqname);
    while (len) {
        len--;
        if (fqname[len] == '.') break;
    }
    if (len) {
        /* Check the namespace has been imported */
        found = 0;
        for (i = 0; i < scp_noch(master_context->ast->scope); i++) {
            if ( strncmp(fqname, scp_chd(master_context->ast->scope, i)->name, len) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) return 0;
    }
    else return 0; /* No namespace */

    name = fqname + len + 1;

    /* OK - Create func */
    func = malloc(sizeof(imported_func));
    func->context = 0;
    func->already_loaded = 0;
    func->is_variable = is_variable; /* Is a function or a Variable */

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
        if (master_context->debug_mode) printf("Importing Procedures - Analysing procedure %s\n", func->fqname);
        func->context = parseRexx(master_context->location, func->file_name, LEVELB, master_context->debug_mode, buffer,
                                  strlen(buffer));

        if (!func->context || error_in_node(func->context->ast)) {
            fprintf(stderr,
                    "Importing Procedures - ERROR: Error parsing imported procedure arguments for %s, skipping\n",
                    func->fqname);
            fprintf(stderr, "buffer is %s\n", buffer);
            dump_error_ast(func->context);
            freimpfc(func);
            return 0;
        }

        /* Make sure the AST seems sane */
        if (func->context->ast->child->node_type != PROGRAM_FILE) {
            fprintf(stderr,
                    "Importing Procedures - ERROR: Unexpected syntax error parsing imported procedure arguments for %s, skipping\n",
                    func->fqname);
            dump_error_ast(func->context);
            freimpfc(func);
            return 0;
        }

        /* Fixup the node type */
        func->context->ast->child->node_type = IMPORTED_FILE;
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
        default: return;
    }

    /* Read files in the directory */
    name = dirfstfl(directory, type_name, &dir_ptr);
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
    char* full_name;
    char* exe_dir;
    size_t d;

    list = malloc( sizeof(importable_file*) );
    list[0] = 0;

    full_name = mprintf("%s.rexx", context->file_name);
    exe_dir = exepath();

    /* Read REXX files in the current directory */
    list_files_in_dir(context->location, REXX_FILE, full_name, &list, &number);

    /* Read RXAS files in the current directory */
    list_files_in_dir(context->location, RXAS_FILE, 0, &list, &number);

    /* Read RXBIN files in the current directory */
    list_files_in_dir(context->location, RXBIN_FILE, 0, &list, &number);

    /* Look in the imported location */
    if (context->import_locations) {
        for (d = 0; context->import_locations[d]; d++) {
            /* Read in REXX files in the directory */
            list_files_in_dir(context->import_locations[d], REXX_FILE, 0, &list, &number);

            /* Read in RXAS files in the directory */
            list_files_in_dir(context->import_locations[d], RXAS_FILE, 0, &list, &number);

            /* Read in RXBIN files in the directory */
            list_files_in_dir(context->import_locations[d], RXBIN_FILE, 0, &list, &number);
        }
    }

    if (exe_dir) {
        /* Read in REXX files in the directory holding the compiler */
        list_files_in_dir(exe_dir, REXX_FILE, 0, &list, &number);

        /* Read in RXAS files in the directory holding the compiler */
        list_files_in_dir(exe_dir, RXAS_FILE, 0, &list, &number);

        /* Read in RXBIN files in the directory holding the compiler */
        list_files_in_dir(exe_dir, RXBIN_FILE, 0, &list, &number);

        free(exe_dir);
    }

    free(full_name);

    return list;
}

