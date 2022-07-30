/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "rxcpmain.h"
#include "rxvminst.h"
#include "rxcpdary.h"

static void help() {
    char* helpMessage =
            "cREXX Compiler\n"
            "Version : " rxversion "\n"
            "Usage   : rxc [options] source_file\n"
            "Options :\n"
            "  -h              Prints help message\n"
            "  -c              Prints Copyright & License Details\n"
            "  -v              Prints Version\n"
#ifndef NDEBUG
            "  -d              Debug/Verbose Mode\n"
            "                    AST diagrams require \"dot\" from\n"
            "                    https://graphviz.org/download/\n"
#endif
            "  -l location     Working Location (directory)\n"
            "  -o output_file  REXX Assembler Output File\n"
            "  -n              No Optimising\n";

    printf("%s",helpMessage);
}

static void license() {
    char *message =
            "cREXX License (MIT)\n"
            "Copyright (c) 2020-2021 Adrian Sutherland\n\n"

            "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
            "of this software and associated documentation files (the \"Software\"), to deal\n"
            "in the Software without restriction, including without limitation the rights\n"
            "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
            "copies of the Software, and to permit persons to whom the Software is\n"
            "furnished to do so, subject to the following conditions:\n\n"

            "The above copyright notice and this permission notice shall be included in all\n"
            "copies or substantial portions of the Software.\n\n"

            "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
            "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
            "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
            "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
            "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
            "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
            "SOFTWARE.\n\n"
            "See https://github.com/adesutherland/CREXX for project details\n";

    printf("%s",message);
}

static void error_and_exit(int rc, char* message) {

    fprintf(stderr, "ERROR: %s - try \"rxc -h\"\n", message);
    exit(rc);
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

/* Context Factory */
Context *cntx_f() {
    Context *context;
    context = calloc(1, sizeof(Context)); /* Zero Contents */

    context->level = UNKNOWN;
    context->hashcomments = 1; /* This is the recommended & default line comment style */

    return context;
}

/* Set Context Buffer */
void cntx_buf(Context *context, char* buff_start, size_t bytes) {
    size_t i;
    context->buff_start = buff_start;
    context->buff_end = context->buff_start + bytes;
    context->top = context->buff_start;
    context->cursor = context->buff_start;
    context->linestart = context->buff_start;
    context->prev_linestart = context->buff_start;
    context->line = 0;
    context->namespace = 0;
    context->current_scope = 0;
    if (context->importable_function_array) {
        /* Deallocate importable_function_array */
        for (i = 0; i < ((dpa*)(context->importable_function_array))->size; i++ ) {
            freimpfc(((dpa *) (context->importable_function_array))->pointers[i]);
        }
        free_dpa(context->importable_function_array);
        context->importable_function_array  = 0;
    }
    context->importable_function_array = dpa_f();

    /* Reset importable_file_list */
    if (context->importable_file_list) {
        rxfl_fre(context->importable_file_list);
        context->importable_file_list = 0;
    }
}

/* Free Context */
void fre_cntx(Context *context)  {
    size_t i;
    if (context->file_pointer) fclose(context->file_pointer);

    /* Deallocate Scope and Symbols */
    if (context->ast &&  context->ast->scope) scp_free(context->ast->scope);

    /* Deallocate AST */
    free_ast(context);

    /* Deallocate importable_function_array */
    for (i = 0; i < ((dpa*)(context->importable_function_array))->size; i++ ) {
        freimpfc(((dpa *) (context->importable_function_array))->pointers[i]);
    }
    free_dpa(context->importable_function_array);
    context->importable_function_array  = 0;

    /* Deallocate importable_file_list */
    if (context->importable_file_list) {
        rxfl_fre(context->importable_file_list);
        context->importable_file_list = 0;
    }

    /* Deallocate Tokens */
    free_tok(context);

    free(context->buff_start);

    if (context->traceFile) fclose(context->traceFile);

    free(context);
}

int main(int argc, char *argv[]) {

    FILE *outFile = 0;
    size_t bytes;
    char* buff_start;
    Context *context;
    int errors = 0;
    int i;
    char *output_file_name = 0;
    int debug_mode = 0;
    char *file_name;
    char *location = 0;
    int do_optimise = 1;

    /* Parse arguments  */
    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (strlen(argv[i]) > 2) {
            error_and_exit(2, "Invalid argument");
        }
        switch (tolower((argv[i][1]))) {
            case '-':
                break;

            case 'o': /* Output File */
                i++;
                if (i >= argc) {
                    error_and_exit(2, "Missing REXX Assembler output file after -o");
                }
                output_file_name = argv[i];
                break;

            case 'l': /* Working Location / Directory */
                i++;
                if (i >= argc) {
                    error_and_exit(2, "Missing location after -l");
                }
                location = argv[i];
                break;
            case 'n': /* No Optimisation */
                do_optimise = 0;
                break;

            case 'v': /* Version */
                printf("%s\n", rxversion);
                exit(0);

            case 'h': /* Help */
            case '?':
                help();
                exit(0);

            case 'c': /* License */
                license();
                exit(0);

#ifndef NDEBUG
            case 'd': /* Debug Mode */
                debug_mode = 1;
                break;
#endif

            default:
                error_and_exit(2, "Invalid argument");
        }
    }

    if (i == argc) {
        error_and_exit(2, "Missing input source file");
    }

    file_name = argv[i++];

    if (i < argc) {
        error_and_exit(2, "Unexpected Arguments");
    }

    if (!output_file_name) output_file_name = file_name;

    /* Context Structure */
    context = cntx_f();

    /* Open input file */
    const char* filename_extension = get_filename_ext(file_name);
    if (filename_extension[0] == 0)
      {
        context->file_pointer = openfile(file_name,"rexx", location, "r");
      }
    else {
        context->file_pointer = openfile(file_name,"", location, "r");
    }
    if (context->file_pointer == NULL) {
        fprintf(stderr, "Can't open input file: %s\n", file_name);
        exit(-1);
    }

    /* Open trace file */
#ifndef NDEBUG
    if (debug_mode) {
        context->traceFile = openfile(file_name, "trace", location, "w");
        if (context->traceFile == NULL) {
            fprintf(stderr, "Can't open trace file\n");
            exit(-1);
        }
    }
#endif

    buff_start = file2buf(context->file_pointer, &bytes);
    /* Close file */
    fclose(context->file_pointer);
    context->file_pointer = 0;

    if(buff_start == NULL) {
        fprintf(stderr, "Can't read input file\n");
        exit(-1);
    }

    /* Initialize context */
    cntx_buf(context, buff_start, bytes);
    context->debug_mode = debug_mode;
    context->optimise = do_optimise;
    context->location = location;
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
            fprintf(stderr,"REXX Level A/C/D (cREXX Classic) - Not supported yet\n");
            break;

        case LEVELB:
            /* We need the assembler db for ASSEMBLE */
            init_ops();

        case LEVELG:
        case LEVELL:
            if (debug_mode) printf("REXX Level B/G/L (cREXX)\n");
            rexbpars(context); // Built AST
            free_ops(); // Free Instruction Database
            break;

        default:
            fprintf(stderr, "INTERNAL ERROR: Failed to determine REXX Level\n");
    }


    if (!context->ast) {
        fprintf(stderr,"INTERNAL ERROR: Compiler Exiting - Failure to create AST\n");
        goto finish;
    }

#ifndef __CMS__
    if (debug_mode) {
        pdot_tree(context->ast, "astgraph0.dot");
        /* Get dot from https://graphviz.org/download/ */
        system("dot astgraph0.dot -Tpng -o astgraph0.png");
    }
#endif

    errors = prnterrs(context);
    if (errors) {
        fprintf(stderr,"%d error(s) in source file\n", errors);
        goto finish;
    }

    if (debug_mode) printf("Validating AST Tree\n");
    validate(context);
#ifndef __CMS__
    if (debug_mode) {
        pdot_tree(context->ast, "astgraph1.dot");
        system("dot astgraph1.dot -Tpng -o astgraph1.png");
    }
#endif
    errors = prnterrs(context);
    if (errors) {
        fprintf(stderr,"%d error(s) in source file\n", errors);
        goto finish;
    }

    /* Optimise AST Tree */
    if (context->optimise) {
        if (debug_mode) printf("Optimising AST Tree\n");
        optimise(context);
#ifndef __CMS__
        if (debug_mode) {
            pdot_tree(context->ast, "astgraph2.dot");
            system("dot astgraph2.dot -Tpng -o astgraph2.png");
        }
#endif
    }

    errors = prnterrs(context);
    if (errors) {
        fprintf(stderr,"%d error(s) in source file\n", errors);
        goto finish;
    }

    /* Generate Assembler */
    outFile = openfile(output_file_name, "rxas", location, "w");
    if (outFile == NULL) {
        fprintf(stderr, "Can't open output file %s\n", output_file_name);
        exit(-1);
    }
    if (debug_mode) printf("Generating Assembler file %s\n", output_file_name);
    emit(context, outFile);

#ifndef __CMS__
    if (debug_mode) {
        pdot_tree(context->ast, "astgraph3.dot");
        system("dot astgraph3.dot -Tpng -o astgraph3.png");
    }
#endif
    if (debug_mode) printf("Compiler Exiting - Success\n");

    finish:

    /* Close outfile */
    if (outFile) fclose(outFile);

    /* Free context */
    fre_cntx(context);

    if (errors) return(1);
    else return(0);
}
