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
    if (node->child && error_in_node(node->child)) return 1;
    node = node->sibling;
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

                printf("   .meta \"%s\"=\"b\" \"%s\" %.*s() \"%s\" \"\"\n",
                       fqname, /* FQ Symbol Name */
                       type, /* Return Type */
                       (int) node->node_string_length, node->node_string, /* Function name */
                       args /* Args */);

                func = rximpfc_f(master_context, fqname, node->symbolNode->symbol->name, "b", type, args, 0);

                dpa_add(master_context->importable_function_array, func);

                free(type);
                free(source);
                free(args);
                free(fqname);
            }
        }
    }
    return result_normal;
}

void parseFileForFunctions(Context *master_context, char* file_name) {
    size_t bytes;
    Context *context;
    char *buff_start;

    if (master_context->debug_mode) printf("Reading file %s for possible procedure imports\n", file_name);

    context = cntx_f();

    /* Open input file */
    context->file_pointer = openfile(file_name, "", master_context->location, "r");
    if (context->file_pointer == NULL) {
        fprintf(stderr, "Can't open input file: %s\n", file_name);
        exit(-1);
    }

    /* Open trace file */
#ifndef NDEBUG
    if (master_context->debug_mode) {
        context->traceFile = openfile(file_name, "trace", master_context->location, "w");
        if (context->traceFile == NULL) {
            fprintf(stderr, "Can't open trace file\n");
            exit(-1);
        }
    }
#endif

    buff_start = file2buf(context->file_pointer, &bytes);
    /* Close file */
    fclose(context->file_pointer);
    context->file_pointer  = 0;

    if (buff_start == NULL) {
        fprintf(stderr, "Can't read input file\n");
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

#ifndef __CMS__
    if (master_context->debug_mode) {
        pdot_tree(context->ast, "astgraphf0.dot");
        /* Get dot from https://graphviz.org/download/ */
        system("dot astgraphf0.dot -Tpng -o astgraphf0.png");
    }
#endif

    if (master_context->debug_mode) printf("Validating AST Tree\n");
    validate(context);
#ifndef __CMS__
    if (master_context->debug_mode) {
        pdot_tree(context->ast, "astgraphf1.dot");
        system("dot astgraphf1.dot -Tpng -o astgraphf1.png");
    }
#endif

    /* Extract Function Definitions  */
    ast_wlkr(context->ast, procedure_signature_walker, master_context);

    if (master_context->debug_mode) printf("Importing Procedures - Compiler read exiting - Success\n");

    finish:

    /* Free context */
    fre_cntx(context);
}

/* Parse and validate a rexx program in a string and return the context */
Context *parseRexx(char *location, char* file_name, RexxLevel level, int debug_mode, char* rexx_source, size_t bytes) {
    Context *context;

    if (debug_mode) printf("Parsing Rexx: %.*s\n", (int)bytes, rexx_source);

    context = cntx_f();
    cntx_buf(context, rexx_source, bytes);

    /* Initialize context */
    context->location = location;
    context->file_name = file_name;
    context->level = level;

    rexbpars(context); /* TODO Handle Levels, this is level B only  */

    if (!context->ast) {
        if (debug_mode) fprintf(stderr,"Parsing Rexx - INTERNAL ERROR: Compiler Exiting - Failure to create AST\n");
        goto finish;
    }

    validate(context);

#ifndef __CMS__
    if (debug_mode) {
        pdot_tree(context->ast, "parserexx.dot");
        /* Get dot from https://graphviz.org/download/ */
        system("dot parserexx.dot -Tpng -o parserexx.png");
    }
#endif

    if (error_in_node(context->ast)) printf("Error in the Args\n");

    if (debug_mode) printf("Parsing REXX - Compiler read exiting - Success\n");

    finish:

    return context;
}

/* Try and import an external function - return its symbol if successful */
Symbol *sym_imfn(Context *context, ASTNode *node) {
    size_t i;
    dpa *func_array = context->importable_function_array;

    char *name = (char*)malloc(node->node_string_length + 1);
    memcpy(name, node->node_string, node->node_string_length);
    name[node->node_string_length] = 0;

    /* For each file */
    parseFileForFunctions(context, "dummy.rexx");
    for (i = 0; i < func_array->size; i++) {
        imported_func *func = (imported_func*)func_array->pointers[i];
        if ( strcmp(name,func->name) == 0 ) {
            /* Found it! */

            break;
        }
    }

    free(name);
    return 0;
}

/* imported_func factory  */
imported_func *rximpfc_f(Context*  master_context, char *fqname, char *name, char *options, char *type, char *args, char *implementation)  {
    imported_func *func = malloc(sizeof(imported_func));
    char *buffer;

    func->context = 0;

    if (fqname) {
        /* Store fqname */
        size_t len  = strlen(fqname);
        func->fqname = malloc(len + 1);
        strcpy(func->fqname,fqname);

        /* Get the namespace (string before the last ".") from the fqn */
        while (len) {
            len--;
            if (fqname[len] == '.') break;
        }
        if (len) {
            func->namespace = malloc(len + 1);
            memcpy(func->namespace, fqname, len);
            func->namespace[len] = 0;
        }
        else func->namespace = 0;
    }
    else {
        func->fqname = 0;
        func->namespace = 0;
    }

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
    buffer = mprintf("options levelb\n%s: procedure = %s\narg %s\n\0", func->name, func->type, func->args); // NOLINT

    parseRexx(master_context->location, func->namespace, LEVELB, master_context->debug_mode, buffer, strlen(buffer) + 1);

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

typedef struct func_payload  {
    imported_func *function;
} func_payload;
