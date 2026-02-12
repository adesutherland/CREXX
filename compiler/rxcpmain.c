/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Compiler Entry Point and CLI Orchestration
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "rxcpmain.h"
#include "rxcp_plugin.h"
#include "../binutils/include/rxdefs.h"
#include "rxcpdary.h"
#include "rxvmplugin_framework.h"
#include "rxvml.h"

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
            "  -d[level]       Debug Mode (Optional Level)\n"
            "  -dp             Stop after parsing and validation\n"
            "  -l location     Working Location (directory)\n"
            "  -i import       Locations to import file - \";\" delimited list\n"
            "  -o output_file  REXX Assembler Output File\n"
            "  -n              No Optimising\n";

    printf("%s",helpMessage);
}

static void license() {
    char *message =
            "cREXX License (MIT)\n"
            "Copyright (c) 2020-2026 Adrian Sutherland\n\n"

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


/* Context Factory */
Context *cntx_f() {
    Context *context;
    context = calloc(1, sizeof(Context)); /* Zero Contents */

    context->level = UNKNOWN;
    context->lexer_stem_mode = 0;
    context->comments_hash = 1; /* This is the recommended & default line comment style */
    context->floats_decimal = 0; /* Force floats to be decimal */
    context->floats_binary = 0;  /* Force floats to be binary */
    context->numeric_common = 0;   /* Set if numeric Common Option Explicitly Specified */
    context->numeric_classic = 0;  /* Set if numeric Classic Option Explicitly Specified */
    context->numeric_standard = 0; /* Common Numeric Standard by default */
    context->comments_dash_specified = 0; /* Set if Dash comments option explicitly specified */
    context->comments_hash_specified = 0; /* Set if hash comments option explicitly specified */
    context->comments_slash_specified = 0; /* Set if Slash comments option explicitly specified */
    context->debug_mode = 0;
    context->optimise = 1; /* Optimise by default */
    context->decimal_plugin = 0; /* No decimal plugin by default */

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
    if (context->importable_class_tree) {
        fre_ctre(context);
        context->importable_class_tree  = 0;
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

    /* Deallocate importable_class_tree */
    if (context->importable_class_tree) fre_ctre(context);
    context->importable_class_tree  = 0;

    /* Deallocate importable_file_list */
    if (context->importable_file_list) {
        rxfl_fre(context->importable_file_list);
        context->importable_file_list = 0;
    }

    /* Deallocate Tokens */
    free_tok(context);

    /* Deallocate VM Bridge */
    if (context->rxvml_bridge) {
        rxvml_destroy((rxvml_context*)context->rxvml_bridge);
        context->rxvml_bridge = 0;
    }

    if (context->master_context && context == context->master_context) {
        if (context->loading_files) {
            for (i = 0; i < context->loading_files_count; i++) free(context->loading_files[i]);
            free(context->loading_files);
        }
    }

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
    int stop_after_parse = 0;
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
        if (strncmp(argv[i], "-d", 2) == 0) {
            if (argv[i][2] == 'p') {
                stop_after_parse = 1;
                if (debug_mode == 0) debug_mode = 1;
            } else if (argv[i][2] != 0) {
                debug_mode = atoi(argv[i] + 2);
            } else {
                debug_mode = 2;
            }
            continue;
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

    if (debug_mode >= 2) fprintf(stderr, "Input file is %s\n", file_name);

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
    context->master_context = context;
    context->loading_files_count = 1;
    context->loading_files = malloc(sizeof(char*));
    context->loading_files[0] = strdup(context->file_name);

    /* Open trace file */
#ifndef NDEBUG
    // Trace file logic removed to avoid redundant file creation.
    // Tracing is now redirected to stderr when debug_mode is active.
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
    context->stop_after_parse = stop_after_parse;
    context->optimise = do_optimise;
    if (file_directory) context->location = file_directory;
    else context->location = location;

    /* Load VM Plugins */
    // Manually initialize the plugins that are statically linked with manual initializers (hardcoded)
    CALL_PLUGIN_INITIALIZER(decnumber); // MC Decimal Plugin
    context->decimal_plugin = (decplugin*)get_rxvmplugin(RXVM_PLUGIN_DECIMAL);
    if (!context->decimal_plugin) {
        printf("PANIC - No default decimal plugin\n");
        exit(255); // Documented 255 is for missing decimal plugin
    }
    if (context->debug_mode >= 2) fprintf(stderr, "Using Decimal Plugin %s\n", ((decplugin*)context->decimal_plugin)->base.name);

    /* Create Options parser to work out the required language level */
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
            /* init_ops() removed - now using static opdata */

        case LEVELG:
        case LEVELL:
            if (debug_mode >= 2) fprintf(stderr, "REXX Level B/G/L (cREXX)\n");
            rexbpars(context); // Built AST
            if (context->debug_mode) {
                rxcp_debug_header("STAGE_RAW", -1);
                rxcp_print_ast_recursive(context->ast, 0);
            }
            break;

        default:
            fprintf(stderr, "INTERNAL ERROR: Failed to determine REXX Level\n");
    }


    if (!context->ast) {
        fprintf(stderr,"INTERNAL ERROR: Compiler Exiting - Failure to create AST\n");
        goto finish;
    }


    errors = prnterrs(context);
    if (errors) {
        fprintf(stderr,"%d error(s) in source file\n", errors);
        if (context->stop_after_parse) goto dp_stop;
        goto finish;
    }

    rxcp_val(context);
#ifndef __CMS__
    if (debug_mode >= 2) {
        // pdot_tree(context->ast, "astgraph1", context->file_name);
    }
#endif
    errors = prnterrs(context);
    if (errors) {
        fprintf(stderr,"%d error(s) in source file\n", errors);
        if (context->stop_after_parse) goto dp_stop;
        goto finish;
    }

    /* Optimise AST Tree */
    if (context->optimise) {
        if (debug_mode >= 2) fprintf(stderr, "Optimising AST Tree\n");
        optimise(context);
#ifndef __CMS__
        if (debug_mode >= 2) {
            // pdot_tree(context->ast, "astgraph2", context->file_name);
        }
#endif
    }

    errors = prnterrs(context);
    if (errors) {
        fprintf(stderr,"%d error(s) in source file\n", errors);
        if (context->stop_after_parse) goto dp_stop;
        goto finish;
    }

    if (context->debug_mode) {
        rxcp_debug_header("STAGE_FINAL", -1);
        rxcp_print_ast_recursive(context->ast, 0);
    }

    dp_stop:
    if (context->stop_after_parse) {
        errors = prnterrs(context);
        if (errors) {
            fprintf(stderr,"%d error(s) in source file\n", errors);
        }
        fflush(stderr);
        fflush(stdout);
        if (debug_mode) {
            fprintf(stderr, "[INFO] Stopping compilation after Parser/Fixup/Validation (-dp).\n");
        }
        goto finish;
    }

    /* Generate Assembler */
    outFile = openfile(output_file_name, "rxas", location, "w");
    if (outFile == NULL) {
        fprintf(stderr, "Can't open output file %s\n", output_file_name);
        exit(-1);
    }
    if (debug_mode >= 2) fprintf(stderr, "Generating Assembler file %s\n", output_file_name);
    emit(context, outFile);

#ifndef __CMS__
    if (debug_mode >= 2) {
        // pdot_tree(context->ast, "astgraph3", context->file_name);
    }
#endif
    if (debug_mode >= 2) fprintf(stderr, "Compiler Exiting - Success\n");

    finish:

    warnings = prntwars(context);
    if (warnings) {
        fprintf(stderr,"%d warning(s) in source file\n", warnings);
    }

    /* Close outfile */
    if (outFile) fclose(outFile);

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
