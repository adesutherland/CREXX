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

static void dump_error_ast(Context *context) {
#ifndef __CMS__
    if (context->debug_mode) {
        pdot_tree(context->ast, "error.dot");
        /* Get dot from https://graphviz.org/download/ */
        system("dot error.dot -Tpng -o error.png");
    }
#endif
}

static const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

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

static char error_in_node(ASTNode* node) {  // NOLINT
    if (node->node_type == ERROR) return 1;
    node = node->child;
    while (node) {
        if (error_in_node(node)) return 1;
        node = node->sibling;
    }
    return 0;
}

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
                if  (error_in_node(child1)) result_normal; /* If there is an error we must ignore this entry */
                if  (error_in_node(child2)) result_normal; /* If there is an error we must ignore this entry */

                char *type = nodetype(child1);
                char *source = clnnode(child2);
                char *args = encdstrg(source, strlen(source));
                char *fqname = sym_frnm(node->symbolNode->symbol);

                func = rximpfc_f(master_context, fqname, node->symbolNode->symbol->name, "b", type, args, 0);

                if (func) dpa_add(master_context->importable_function_array, func);

                free(type);
                free(source);
                free(args);
                free(fqname);
            }
        }
    }
    return result_normal;
}

void parseRexxFileForFunctions(Context *master_context, char* file_name) {
    size_t bytes;
    Context *context;
    char *buff_start;

    if (master_context->debug_mode) printf("Importing Procedures - Reading file %s for possible procedure imports\n", file_name);

    /* Context for parsing */
    context = cntx_f();

    /* Open input file */
    context->file_pointer = openfile(file_name, "", master_context->location, "r");
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
            if (master_context->debug_mode) printf("Importing Procedures - REXX Level B/G/L (cREXX)\n");
            rexbpars(context);
            break;

        default:
            fprintf(stderr, "Importing Procedures - INTERNAL ERROR: Failed to determine REXX Level\n");
    }

    if (!context->ast) {
        fprintf(stderr,"Importing Procedures - INTERNAL ERROR: Compiler Exiting - Failure to create AST\n");
        goto finish;
    }

    if (master_context->debug_mode) printf("Importing Procedures - Validating AST Tree\n");
    validate(context);

    /* Extract Function Definitions  */
    ast_wlkr(context->ast, procedure_signature_walker, master_context);

    if (master_context->debug_mode) printf("Importing Procedures - Compiler read exiting - Success\n");

    finish:

    /* Free context */
#ifndef NDEBUG
    context->traceFile =  0;
#endif
    fre_cntx(context);
}

/* Parse and validate a rexx program in a string and return the context */
Context *parseRexx(char *location, char* file_name, RexxLevel level, int debug_mode, char* rexx_source, size_t bytes) {
    Context *context;

    if (debug_mode) printf("Importing Procedures - Parsing REXX for file %s\n", file_name);

    context = cntx_f();
    cntx_buf(context, rexx_source, bytes);

    /* Initialize context */
    context->location = location;
    context->file_name = file_name;
    context->level = level;
    context->debug_mode = debug_mode;
    context->dont_import = 1;

    switch (context->level){
        case LEVELA:
        case LEVELC:
        case LEVELD:
            fprintf(stderr,"INTERNAL ERROR: Importing Procedures - REXX Level A/C/D (cREXX Classic) - Not supported yet\n");
            break;

        case LEVELB:
        case LEVELG:
        case LEVELL:
            if (context->debug_mode) printf("Importing Procedures - Is Level B/G/L (cREXX)\n");
            rexbpars(context);
            break;

        default:
            fprintf(stderr, "INTERNAL ERROR: Importing Procedures - Failed to determine REXX Level\n");
    }

    rexbpars(context); /* TODO Handle Levels, this is level B only  */

    if (!context->ast) {
        if (debug_mode) fprintf(stderr,"INTERNAL ERROR: Importing Procedures - Compiler Exiting - Failure to create AST\n");
        goto finish;
    }

    validate(context);

#ifndef __CMS__
    if (debug_mode) {
//        pdot_tree(context->ast, "parserexx.dot");
        /* Get dot from https://graphviz.org/download/ */
//        system("dot parserexx.dot -Tpng -o parserexx.png");
    }
#endif

    if (debug_mode) printf("Importing Procedures - Parsing REXX - Success\n");

    finish:

    return context;
}

/* Try and import an external function - return its symbol if successful */
Symbol *sym_imfn(Context *master_context, ASTNode *node) {
    size_t i, f;
    dpa *func_array = master_context->importable_function_array;
    Symbol *symbol;
    ASTNode *func_node;
    Symbol *func_symbol;
    Symbol *found_symbol = 0;

    char *name = (char*)malloc(node->node_string_length + 1);
    memcpy(name, node->node_string, node->node_string_length);
    name[node->node_string_length] = 0;

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
        if (master_context->importable_file_list[f]->imported) continue;
        master_context->importable_file_list[f]->imported = 1;

        /* Import File */
        switch (master_context->importable_file_list[f]->type) {
            case REXX_FILE:
                parseRexxFileForFunctions(master_context, master_context->importable_file_list[f]->name);
                break;
            case RXBIN_FILE:
            case RXAS_FILE:
                ;
        }

        /* Check the added functions in the function list */
        for ( ; i < func_array->size; i++) {
            found_symbol = 0;
            imported_func *func = (imported_func *) func_array->pointers[i];

            if (strcmp(name, func->name) == 0) {
                /* Found it! */

                /* Splice the ASTs together */
                add_ast(master_context->ast, func->context->ast->child);

                /* Splice in namespace symbol */
                symbol = sym_rslv(master_context->ast->scope, func->context->ast->child);
                if (!symbol) {
                    symbol = sym_f(master_context->ast->scope, func->context->ast->child);
                    symbol->symbol_type = NAMESPACE_SYMBOL;
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
                }
                else {
                    func_symbol = func_node->symbolNode->symbol;

                    /* Make a duplicate symbol in the master_context */
                    found_symbol = sym_fn(master_context->ast->child->scope, func_symbol->name,
                                          strlen(func_symbol->name));
                    if (!found_symbol) {
                        fprintf(stderr, "INTERNAL ERROR: Importing Procedures - Could not duplicate imported procedure symbol\n");
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

                break;
            }
        }
    }

    free(name);
    return found_symbol;
}

/* imported_func factory - returns null if the function is not in an applicable namespace */
imported_func *rximpfc_f(Context*  master_context, char *fqname, char *name, char *options, char *type, char *args, char *implementation)  {
    imported_func *func;
    char *buffer;
    Scope **namespace;
    size_t i;
    char found;

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

    /* OK - Create func */
    func = malloc(sizeof(imported_func));
    func->context = 0;

    /* Store the namespace */
    func->namespace = malloc(len + 1);
    memcpy(func->namespace, fqname, len);
    func->namespace[len] = 0;

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

    /* Generate Function Declaration AST */
    buffer = mprintf("options levelb\n%s: procedure = %s\narg %s\n", func->name, func->type, func->args);
// TODO Level detection from options
    if (master_context->debug_mode) printf("Importing Procedures - Generate AST for args for procedure %s\n", func->fqname);
    func->context = parseRexx(master_context->location, func->namespace, LEVELB, master_context->debug_mode, buffer, strlen(buffer));

    if (error_in_node(func->context->ast)) {
        fprintf(stderr, "Importing Procedures - ERROR: Error parsing imported procedure arguments for %s, skipping\n", func->fqname);
        fprintf(stderr, "buffer is %s\n", buffer);
        dump_error_ast(func->context);
        freimpfc(func);
        return 0;
    }

    /* Make sure the AST seems sane */
    if (func->context->ast->child->node_type != PROGRAM_FILE) {
        fprintf(stderr, "Importing Procedures - ERROR: Unexpected syntax error parsing imported procedure arguments for %s, skipping\n", func->fqname);
        dump_error_ast(func->context);
        freimpfc(func);
        return 0;
    }

    /* Fixup the node type */
    func->context->ast->child->node_type = IMPORTED_FILE;

    return func;
}

/* Free an imported_func */
void freimpfc(imported_func *func) {
    if (func->namespace) free(func->namespace);
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

static importable_file* importable_file_f(char* name, file_type type) {
    importable_file *file;
    file = malloc(sizeof(importable_file));
    file->name = malloc(strlen(name) + 1);
    strcpy(file->name, name);
    file->imported = 0;
    file->type = type;
    return file;
}

/* Get the list of importable files as a null terminated malloced array */
importable_file **rxfl_lst(Context *context) {
    size_t number = 0;
    importable_file **list = 0;
    importable_file *file;
    void *dir_ptr;
    char* name;
    char* full_name;
    char* exe_dir;

    list = malloc( sizeof(importable_file*) );
    list[0] = 0;

    full_name = mprintf("%s.rexx", context->file_name);

    /* Read REXX files in the current directory */
    name = dirfstfl(context->location, "rexx", &dir_ptr);
    if (name) {
        if ( strcmp(name, full_name) != 0 ) { // Skip if the same name as the main rexx file
            file = importable_file_f(name, REXX_FILE);
            add_file_to_list(file, &number, &list);
        }
        do {
            name = dirnxtfl(&dir_ptr);
            if (name) {
                if ( strcmp(name, full_name) != 0 ) { // Skip if the same name as the main rexx file
                    file = importable_file_f(name, REXX_FILE);
                    add_file_to_list(file, &number, &list);
                }
            }
        } while (name);
    }
    dirclose(&dir_ptr);

    /* Read in REXX files in the directory holding the compiler */
    exe_dir = exepath();
    if (exe_dir) {
        name = dirfstfl(exe_dir, "rexx", &dir_ptr);
        if (name) {
            file = importable_file_f(name, REXX_FILE);
            add_file_to_list(file, &number, &list);
            do {
                name = dirnxtfl(&dir_ptr);
                if (name) {
                    file = importable_file_f(name, REXX_FILE);
                    add_file_to_list(file, &number, &list);
                }
            } while (name);
        }
        dirclose(&dir_ptr);

        free(exe_dir);
    }

    free(full_name);

    return list;
}

