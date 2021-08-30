/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "rxcpmain.h"

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

int main(int argc, char *argv[]) {

    FILE *fp, *traceFile = 0, *outFile = 0;
    char *buff;
    size_t bytes;
    Context context;
    int errors;
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
        switch (toupper((argv[i][1]))) {
            case '-':
                break;

            case 'O': /* Output File */
                i++;
                if (i >= argc) {
                    error_and_exit(2, "Missing REXX Assembler output file after -o");
                }
                output_file_name = argv[i];
                break;

            case 'L': /* Working Location / Directory */
                i++;
                if (i >= argc) {
                    error_and_exit(2, "Missing location after -l");
                }
                location = argv[i];
                break;
            case 'N': /* No Optimisation */
                do_optimise = 0;
                break;

            case 'V': /* Version */
                printf("%s\n", rxversion);
                exit(0);

            case 'H': /* Help */
            case '?':
                help();
                exit(0);

            case 'C': /* License */
                license();
                exit(0);

#ifndef NDEBUG
            case 'D': /* Debug Mode */
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

    /* Open input file */
    fp = openfile(file_name, "rexx", location, "r");
    if (fp == NULL) {
        fprintf(stderr, "Can't open input file: %s\n", file_name);
        exit(-1);
    }

    /* Open trace file */
#ifndef NDEBUG
    if (debug_mode) {
        traceFile = openfile(file_name, "trace", location, "w");
        if (traceFile == NULL) {
            fprintf(stderr, "Can't open trace file\n");
            exit(-1);
        }
    }
#endif

    buff = file2buf(fp);
    /* Close file */
    fclose(fp);

    if(buff == NULL) {
        fprintf(stderr, "Can't read input file\n");
        exit(-1);
    }
    bytes = strlen(buff); // TODO Remove the need for this

    /* Initialize context */
    context.file_name = file_name;
    context.traceFile = traceFile;
    context.top = buff;
    context.cursor = buff;
    context.linestart = buff;
    context.line = 0;
    context.token_head = 0;
    context.token_tail = 0;
    context.token_counter = 0;
    context.ast = 0;
    context.free_list = 0;
    context.level = UNKNOWN;
    context.optimise = do_optimise;
    context.buff_end = (char*) (((char*)buff) + bytes);
    context.buff_start = buff;

    /* Create Options parser to work out required language level */
    opt_pars(&context);

    /* Deallocate memory and reset context */
    free_tok(&context);
    context.top = buff;
    context.cursor = buff;
    context.linestart = buff;
    context.line = 0;
    context.token_head = 0;
    context.token_tail = 0;
    context.token_counter = 0;
    context.ast = 0;
    context.free_list = 0;

    /* Parse program for real */
    switch (context.level){
        case LEVELA:
        case LEVELC:
        case LEVELD:
            fprintf(stderr,"REXX Level A/C/D (cREXX Classic) - Not supported yet\n");
            break;

        case LEVELB:
        case LEVELG:
        case LEVELL:
            if (debug_mode) printf("REXX Level B/G/L (cREXX)\n");
            rexbpars(&context);
            break;

        default:
            fprintf(stderr, "Internal Error - Failed to determine REXX Level\n");
    }


    if (!context.ast) {
        fprintf(stderr,"ERROR: Compiler Exiting - Failure\n");
        goto finish;
    }

#ifndef __CMS__
    if (debug_mode) {
        pdot_tree(context.ast, "astgraph0.dot");
        /* Get dot from https://graphviz.org/download/ */
        system("dot astgraph0.dot -Tpng -o astgraph0.png");
    }
#endif
    if (debug_mode)
        printf("Validating AST Tree\n");
    validate(&context);
#ifndef __CMS__
    if (debug_mode) {
        pdot_tree(context.ast, "astgraph1.dot");
        system("dot astgraph1.dot -Tpng -o astgraph1.png");
    }
#endif
    errors = prnterrs(&context);
    if (errors) {
        fprintf(stderr,"%d error(s) in source file\n", errors);
        goto finish;
    }

    /* Optimise AST Tree */
    if (context.optimise) {
        if (debug_mode)
            printf("Optimising AST Tree\n");
        optimise(&context);
#ifndef __CMS__
        if (debug_mode) {
            pdot_tree(context.ast, "astgraph2.dot");
            system("dot astgraph2.dot -Tpng -o astgraph2.png");
        }
#endif
    }

    errors = prnterrs(&context);
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
    if (debug_mode)
        printf("Generating Assembler file %s\n", output_file_name);
    emit(&context, outFile);

#ifndef __CMS__
    if (debug_mode) {
        pdot_tree(context.ast, "astgraph3.dot");
        system("dot astgraph3.dot -Tpng -o astgraph3.png");
    }
#endif
    if (debug_mode) printf("Compiler Exiting - Success\n");

    finish:

    /* Deallocate AST */
    free_ast(&context);

    /* Deallocate Tokens */
    free_tok(&context);

    /* Close files and deallocate */
    if (outFile) fclose(outFile);
#ifndef NDEBUG
    if (traceFile) fclose(traceFile);
#endif
    free(buff);

    return(0);
}
