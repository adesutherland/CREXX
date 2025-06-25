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

// Static Linked Functions
struct static_linked_function *static_linked_functions;

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
            "  -i import       Locations to import file - \";\" delimited list\n"
            "  -o output_file  REXX Assembler Output File\n"
            "  -n              No Optimising\n";

    printf("%s",helpMessage);
}

static void license() {
    char *message =
            "cREXX License (MIT)\n"
            "Copyright (c) 2020-2024 Adrian Sutherland\n\n"

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
static char *get_filename_directory(const char *file_name)
{
    size_t len = strlen(file_name);
    if (!len) return 0;
    char* result;

    for (len--; len; len--)
    {
        if (file_name[len] == '\\' || file_name[len] == '/' )
        {
            result = malloc(len + 1);
            result[len] = 0;
            memcpy(result, file_name, len);
            return result;
        }
    }

    return 0;
}

/* Context Factory */
Context *cntx_f() {
    Context *context;
    context = calloc(1, sizeof(Context)); /* Zero Contents */

    context->level = UNKNOWN;
    context->lexer_stem_mode = 0;
    context->hashcomments = 1; /* This is the recommended & default line comment style */
    context->decimal = 0; /* Use binary decimal by default */

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
    if (context->importable_function_tree) {
        fre_ftre(context);
        context->importable_function_tree  = 0;
    }

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

    if (context->import_locations) free(context->import_locations);

    /* Deallocate Scope and Symbols */
    if (context->ast &&  context->ast->scope) scp_free(context->ast->scope);

    /* Deallocate AST */
    free_ast(context);

    /* Deallocate importable_function_tree */
    fre_ftre(context);
    context->importable_function_tree  = 0;

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

int rxcmain(int argc, char *argv[]) {

    FILE *outFile = 0;
    size_t bytes;
    char* buff_start;
    Context *context;
    int errors = 0, warnings = 0;
    int i;
    char *output_file_name = 0;
    int debug_mode = 0;
    char *file_name;
    char *location = 0;
    char *import_locations = 0;
    int num_import_locations;
    size_t ix;
    char c;
    int do_optimise = 1;
    char *file_directory = 0;

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

            case 'i': /* Import Locations */
                i++;
                if (i >= argc) {
                    error_and_exit(2, "Missing import location list -i");
                }
                import_locations = argv[i];
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

    /* I am the main context */
    context->master_context = context;

    /* Derive the location from the file name */
    if (!location) {
        file_directory = file_dir(file_name);
    }

    if (debug_mode) printf("Input file is %s\n", file_name);

    /* Import location list */
    if (import_locations) {
        /* How many import locations */
        num_import_locations = 1;
        for (ix = 0; import_locations[ix]; ix++) if (import_locations[ix] == ';') num_import_locations++;

        /* Malloc array */
        context->import_locations = malloc((num_import_locations + 1) * sizeof(char*));

        /* Copy Pointers */
        num_import_locations = 0;
        context->import_locations[num_import_locations] = import_locations;
        for (ix = 0; import_locations[ix]; ix++) if (import_locations[ix] == ';') {
            num_import_locations++;
            import_locations[ix] = 0;
            context->import_locations[num_import_locations] = import_locations + ix + 1;
        }
        context->import_locations[++num_import_locations] = 0;
    }

    /* Open input file */
    const char* filename_extension = filenext(file_name);
    if (filename_extension[0] == 0)
      {
        context->file_pointer = openfile(file_name,"rexx", location, "r");
        context->file_name = mprintf("%s.rexx", filename(file_name));
      }
    else {
        context->file_pointer = openfile(file_name,"", location, "r");
        context->file_name = mprintf("%s", filename(file_name));
    }
    if (context->file_pointer == NULL) {
        fprintf(stderr, "Can't open input file: %s\n", file_name);
        exit(-1);
    }

    /* Open trace file */
#ifndef NDEBUG
    if (debug_mode) {
        context->traceFile = openfile((char*) filename(file_name), "trace", location, "w");
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
    if (file_directory) context->location = file_directory;
    else context->location = location;

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
        pdot_tree(context->ast, "astgraph0", context->file_name);
    }
#endif

    errors = prnterrs(context);
    if (errors) {
        fprintf(stderr,"%d error(s) in source file\n", errors);
        goto finish;
    }

    if (debug_mode) printf("Validating AST Tree\n");
    rxcp_val(context);
#ifndef __CMS__
    if (debug_mode) {
        pdot_tree(context->ast, "astgraph1", context->file_name);
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
            pdot_tree(context->ast, "astgraph2", context->file_name);
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
        pdot_tree(context->ast, "astgraph3", context->file_name);
    }
#endif
    if (debug_mode) printf("Compiler Exiting - Success\n");

    finish:

    warnings = prntwars(context);
    if (warnings) {
        fprintf(stderr,"%d warning(s) in source file\n", warnings);
    }

    /* Close outfile */
    if (outFile) fclose(outFile);

    if (context->level == LEVELB) free_ops(); // Free Instruction Database

    /* Free context */
    free(context->file_name);
    fre_cntx(context);

    if (file_directory) free(file_directory);

    /* Free statically linked functions */
   free_static_linked_functions();

    if (errors) return(2);
    if (warnings) return(1);
    return(0);
}
