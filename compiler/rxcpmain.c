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
#include "rxcp_source_ext.h"
#include "rxcp_source_tree.h"
#include "../binutils/include/rxdefs.h"
#include "rxcpdary.h"
#include "rxvmplugin_framework.h"
#include "rxvml.h"

// Static Linked Functions
struct static_linked_function *static_linked_functions;
struct static_linked_metadata *static_linked_metadata;

static RexxLevel parse_cli_level_name(const char *level_name) {
    if (!level_name) return UNKNOWN;

    if (strcmp(level_name, "a") == 0 || strcmp(level_name, "levela") == 0) return LEVELA;
    if (strcmp(level_name, "b") == 0 || strcmp(level_name, "levelb") == 0) return LEVELB;
    if (strcmp(level_name, "c") == 0 || strcmp(level_name, "levelc") == 0) return LEVELC;
    if (strcmp(level_name, "d") == 0 || strcmp(level_name, "leveld") == 0) return LEVELD;
    if (strcmp(level_name, "g") == 0 || strcmp(level_name, "levelg") == 0) return LEVELG;
    if (strcmp(level_name, "l") == 0 || strcmp(level_name, "levell") == 0) return LEVELL;

    return UNKNOWN;
}

static void append_cli_import_name(char ***imports, size_t *count, const char *name) {
    char **new_imports;

    if (!imports || !count || !name || !name[0]) return;

    new_imports = realloc(*imports, sizeof(char*) * (*count + 1));
    if (!new_imports) error_and_exit(255, "Out of memory appending --import");

    *imports = new_imports;
    (*imports)[*count] = strdup(name);
    if (!(*imports)[*count]) error_and_exit(255, "Out of memory copying --import");
    (*count)++;
}

static void append_location_arg(char **locations, const char *value) {
    size_t old_len;
    size_t value_len;
    char *combined;

    if (!locations || !value || !value[0]) return;
    if (!*locations) {
        *locations = strdup(value);
        if (!*locations) error_and_exit(255, "Out of memory copying location list");
        return;
    }

    old_len = strlen(*locations);
    value_len = strlen(value);
    combined = realloc(*locations, old_len + 1 + value_len + 1);
    if (!combined) error_and_exit(255, "Out of memory extending location list");
    combined[old_len] = ';';
    memcpy(combined + old_len + 1, value, value_len + 1);
    *locations = combined;
}

static char **split_location_list(const char *locations) {
    char *copy;
    char *cursor;
    char **result;
    size_t count;
    size_t index;

    if (!locations || !locations[0]) return 0;

    count = 1;
    for (cursor = (char *) locations; *cursor; cursor++) {
        if (*cursor == ';') count++;
    }

    result = calloc(count + 1, sizeof(char *));
    if (!result) error_and_exit(255, "Out of memory splitting location list");

    copy = strdup(locations);
    if (!copy) error_and_exit(255, "Out of memory copying location list");

    index = 0;
    result[index++] = copy;
    for (cursor = copy; *cursor; cursor++) {
        if (*cursor == ';') {
            *cursor = 0;
            result[index++] = cursor + 1;
        }
    }
    result[index] = 0;
    return result;
}

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
            "  -s source       Source import locations - \";\" delimited list\n"
            "  -i import       Binary import locations - \";\" delimited list\n"
            "  --level level   Default source level when OPTIONS omits one\n"
            "  --import ns     Inject a file-level IMPORT namespace (repeatable)\n"
            "  --import-rxas   Enable auto-import scanning of .rxas in binary roots\n"
            "  -o output_file  REXX Assembler Output File\n"
            "  -n              No Optimising\n"
            "  -x              Disable compiler exits\n"
#ifdef ENABLE_PARSER_MODE
            "  --syntaxhighlight  Run as a DSLSH syntax-highlighting server (stdio mode)\n"
            "  --port port     Run as a DSLSH syntax-highlighting server (socket mode on port)\n"
#endif
            ;

    printf("%s",helpMessage);
}

static void license() {
    char *message = CREXX_LICENSE_TEXT;

    printf("%s",message);
}


/* Context Factory */
Context *cntx_f() {
    Context *context;
    context = calloc(1, sizeof(Context)); /* Zero Contents */

    context->level = UNKNOWN;
    context->cli_level_override = UNKNOWN;
    context->cli_default_level = UNKNOWN;
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

    if (context->import_locations) {
        if (context->import_locations_owns_buffer && context->import_locations[0]) free(context->import_locations[0]);
        free(context->import_locations);
    }

    if (context->source_import_locations) {
        if (context->source_import_locations_owns_buffer && context->source_import_locations[0]) free(context->source_import_locations[0]);
        free(context->source_import_locations);
    }

    source_tree_free(context);

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

    rxcp_free_exits(context);

    if (context->master_context && context == context->master_context) {
        if (context->loading_files) {
            for (i = 0; i < context->loading_files_count; i++) free(context->loading_files[i]);
            free(context->loading_files);
        }
    }

    if (context->cli_import_names) {
        for (i = 0; i < context->cli_import_count; i++) free(context->cli_import_names[i]);
        free(context->cli_import_names);
    }

    free(context->buff_start);

    if (context->extra_buffers) {
        for (i = 0; i < context->extra_buffers_count; i++) free(context->extra_buffers[i]);
        free(context->extra_buffers);
    }

    if (context->traceFile) fclose(context->traceFile);

    if (context->initial_source_extension) free(context->initial_source_extension);

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
    char *source_import_locations = 0;
    char *exe_path = 0;
    char *combined_import_locations = 0;
    char c;
    int do_optimise = 1;
    int disable_exits = 0;
    int auto_import_rxas = 0;
    char *file_directory = 0;
    char *input_source_extension = 0;
    const char* filename_extension;
    RexxLevel cli_default_level = UNKNOWN;
    char **cli_import_names = 0;
    size_t cli_import_count = 0;

#ifdef ENABLE_PARSER_MODE
    int parser_mode = 0;
    int parser_port = 0;
    int parser_stdio = 0;
#endif

    /* Parse arguments  */
    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') break; /* End of options */

        if (strcmp(argv[i], "--help") == 0) {
            help();
            exit(0);
        }

#ifdef ENABLE_PARSER_MODE
        if (strcmp(argv[i], "--syntaxhighlight") == 0 || strcmp(argv[i], "--parser") == 0) {
            parser_mode = 1;
            parser_stdio = 1;
            continue;
        }
        if (strcmp(argv[i], "--port") == 0) {
            i++;
            if (i >= argc) {
                error_and_exit(2, "Missing port number after --port");
            }
            parser_mode = 1;
            parser_stdio = 0;
            parser_port = atoi(argv[i]);
            continue;
        }
#endif

        if (strcmp(argv[i], "--level") == 0) {
            RexxLevel parsed_level;
            i++;
            if (i >= argc) {
                error_and_exit(2, "Missing level after --level");
            }
            parsed_level = parse_cli_level_name(argv[i]);
            if (parsed_level == UNKNOWN) {
                error_and_exit(2, "Invalid level name after --level");
            }
            cli_default_level = parsed_level;
            continue;
        }

        if (strcmp(argv[i], "--import") == 0) {
            i++;
            if (i >= argc) {
                error_and_exit(2, "Missing namespace after --import");
            }
            append_cli_import_name(&cli_import_names, &cli_import_count, argv[i]);
            continue;
        }

        if (strcmp(argv[i], "--source") == 0) {
            i++;
            if (i >= argc) {
                error_and_exit(2, "Missing source location list after --source");
            }
            append_location_arg(&source_import_locations, argv[i]);
            continue;
        }

        if (strcmp(argv[i], "--import-rxas") == 0) {
            auto_import_rxas = 1;
            continue;
        }

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
                append_location_arg(&import_locations, argv[i]);
                break;

            case 's': /* Source Import Locations */
                i++;
                if (i >= argc) {
                    error_and_exit(2, "Missing source location list -s");
                }
                append_location_arg(&source_import_locations, argv[i]);
                break;

            case 'n': /* No Optimisation */
                do_optimise = 0;
                break;

            case 'x': /* Disable Exits */
                disable_exits = 1;
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
#ifdef ENABLE_PARSER_MODE
        if (!parser_mode) error_and_exit(2, "Missing input source file");
        file_name = "dsl_buffer.rexx";
#else
        error_and_exit(2, "Missing input source file");
#endif
    } else {
        file_name = argv[i++];
    }

#ifdef ENABLE_PARSER_MODE
    if (parser_mode) {
        /* Call the DSL initialization from rxcp_dsl.c */
        extern int rxc_parser_mode_main(int stdio_mode, int port, const char *file_name, int debug_mode);
        return rxc_parser_mode_main(parser_stdio, parser_port, file_name, debug_mode);
    }
#endif

    if (i < argc) {
        error_and_exit(2, "Unexpected Arguments");
    }

    /* Add the executable-path binary import root. */
    exe_path = exepath();
    if (import_locations) {
        combined_import_locations = malloc(strlen(import_locations) + strlen(exe_path) + 2);
        sprintf(combined_import_locations, "%s;%s", import_locations, exe_path);
        import_locations = combined_import_locations;
    } else {
        combined_import_locations = malloc(strlen(exe_path) + 1);
        sprintf(combined_import_locations, "%s", exe_path);
        import_locations = combined_import_locations;
    }
    free(exe_path);
    if (source_import_locations && debug_mode >= 2) fprintf(stderr, "Combined source import roots: %s\n", source_import_locations);
    if (import_locations && debug_mode >= 2) fprintf(stderr, "Combined binary import roots: %s\n", import_locations);

    filename_extension = filenext(file_name);
    input_source_extension = rxcp_source_extension_copy(file_name);
    if (!input_source_extension) error_and_exit(255, "Out of memory copying source extension");
    if (!input_source_extension[0]) {
        free(input_source_extension);
        input_source_extension = strdup("rexx");
        if (!input_source_extension) error_and_exit(255, "Out of memory copying source extension");
    }

    char *allocated_output_file_name = 0;
    if (!output_file_name) {
        allocated_output_file_name = strip_rightmost_extension_if(file_name, input_source_extension);
        output_file_name = allocated_output_file_name;
    }

    /* Context Structure */
    context = cntx_f();
    context->initial_source_extension = input_source_extension;
    context->cli_level_override = cli_default_level;
    context->cli_default_level = cli_default_level != UNKNOWN ?
                                 cli_default_level :
                                 rxcp_source_default_level_for_extension(context->initial_source_extension);
    context->cli_import_names = cli_import_names;
    context->cli_import_count = cli_import_count;

    /* I am the main context */
    context->master_context = context;

    /* Derive the location from the file name */
    if (!location) {
        file_directory = file_dir(file_name);
    }

    if (debug_mode >= 2) fprintf(stderr, "Input file is %s\n", file_name);

    /* Open input file */
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
    context->disable_exits = disable_exits || (getenv("RXCP_DISABLE_EXIT") != NULL);
    context->auto_import_rxas = (char) auto_import_rxas;
    if (file_directory) context->location = strdup(file_directory);
    else context->location = location ? strdup(location) : 0;

    if (import_locations) {
        context->import_locations = split_location_list(import_locations);
        context->import_locations_owns_buffer = 1;
        if (debug_mode >= 2) {
            int di;
            for (di = 0; context->import_locations[di]; di++) {
                fprintf(stderr, "Binary import location %d: %s\n", di, context->import_locations[di]);
            }
        }
        free(import_locations);
        import_locations = 0;
        combined_import_locations = 0;
    }

    if (source_import_locations) {
        context->source_import_locations = split_location_list(source_import_locations);
        context->source_import_locations_owns_buffer = 1;
        if (debug_mode >= 2) {
            int di;
            for (di = 0; context->source_import_locations[di]; di++) {
                fprintf(stderr, "Source import location %d: %s\n", di, context->source_import_locations[di]);
            }
        }
        free(source_import_locations);
        source_import_locations = 0;
    }

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
        case LEVELC:
            fprintf(stderr,"REXX Level C (Classic REXX) is currently supported only for DSLSH syntax highlighting. Compilation is not supported yet\n");
            break;

        case LEVELA:
        case LEVELD:
            fprintf(stderr,"REXX Level A/D (cREXX Classic) - Not supported yet\n");
            break;

        case LEVELB:
            /* We need the assembler db for ASSEMBLE */
            /* init_ops() removed - now using static opdata */

        case LEVELG:
        case LEVELL:
            if (debug_mode >= 2) fprintf(stderr, "REXX Level B/G/L (cREXX)\n");
            rxcp_init_exits(context);
            rexbpars(context); // Build AST
            if (context->debug_mode) {
                rxcp_debug_header("STAGE_RAW", -1);
                rxcp_print_ast_recursive(context->ast, 0);
            }
            break;

        default:
            fprintf(stderr, "INTERNAL ERROR: Failed to determine REXX Level\n");
    }


    if (!context->ast) {
        /* Last-resort structural diagnosis path.
         * Keep grammar recovery for cases that preserve a usable AST.
         * Use fallback diagnostics only when parsing has already failed to
         * produce any AST at all. */
        errors = rxcp_run_fallback_diagnostics(context);
        if (errors) {
            errors = prnterrs(context);
            if (errors) fprintf(stderr,"%d error(s) in source file\n", errors);
            goto finish;
        }
        fprintf(stderr,"INTERNAL ERROR: Compiler Exiting - Failure to create AST\n");
        goto finish;
    }


    rxcp_prepare_source_ast(context);
    source_tree_sync_diagnostics(context);

    errors = prnterrs(context);
    if (errors) {
        fprintf(stderr,"%d error(s) in source file\n", errors);
        if (context->stop_after_parse) goto dp_stop;
        goto finish;
    }

    rxcp_val(context);
    rxcp_exit_rewrite_diagnostics(context);
    source_tree_sync_diagnostics(context);
    source_tree_sync_semantics(context);

    /* Print all warnings and errors after the fixed loop but before optimising/emitting */
    warnings = prntwars(context);
    if (warnings) {
        fprintf(stderr,"%d warning(s) in source file\n", warnings);
    }

    errors = prnterrs(context);
    if (errors) {
        fprintf(stderr,"%d error(s) in source file\n", errors);
        if (context->stop_after_parse) goto dp_stop;
        goto finish;
    }

    /* Remove all warnings and errors from the tree so they don't break the emitter */
    rxcp_collect_and_prune_diagnostics(context);

    /* Apply the same semantic copy-elision rule in no-opt and opt builds:
     * only read-only by-value formals may share the incoming argument
     * register. This must not drift back into a type-based heuristic. Later
     * optimisation passes may re-run it after rewriting the tree. */
    mark_const_args(context);

#ifndef __CMS__
    if (debug_mode >= 2) {
        // pdot_tree(context->ast, "astgraph1", context->file_name);
    }
#endif

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
        rxcp_print_symbol_table(context->ast->scope, 0);
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

    /* Close outfile */
    if (outFile) fclose(outFile);

    /* Free context */
    if (context->file_name) free(context->file_name);
    if (context->location) free(context->location);
    fre_cntx(context);

    if (file_directory) free(file_directory);

    /* Free statically linked functions */
   free_static_linked_functions();

    if (combined_import_locations) free(combined_import_locations);

    if (allocated_output_file_name) free(allocated_output_file_name);

    if (errors) return(2);
    return(0);
}
