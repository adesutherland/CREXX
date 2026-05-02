/* CREXX VM Main file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "platform.h"
#include "rxvmintp.h"
#include "rxvmplugin_framework.h"
#include "rxvm.h"

/* Library Buffer */
#ifdef LINK_CREXX_LIB
extern char rx__pg[];
extern size_t rx__pg_l;
#endif

static void help() {
    char* helpMessage =
            "cREXX VM/Interpreter\n"
            "Version : " rxversion "\n"
#ifdef NTHREADED
            "        : Bytecode Mode\n"
#else
            "        : Threaded Mode\n"
#endif
            "Usage   : rxvm [options] binary_file [binary_file_2 ...] -a args ... \n"
            "Options :\n"
            "  -h              Prints help message\n"
            "  -p plugin       Load VM Plugin*\n"
            "  -c              Prints Copyright & License Details\n"
#ifndef NDEBUG
            "  -d              Debug/Trace Mode\n"
#endif
            "  -l location     Working Location (directory)\n"
            "  -v              Prints Version\n"
            "\n*   VM Extension Plugin are specified by the full file name without the extension\n"
            "    Multiple plugins can be loaded by specifying multiple -p options\n"
            "    E.g. -p myplugin1 -p dir/myplugin2\n"
            "    *** Defining plugin location to be improved in release version ***\n"; // todo

    printf("%s",helpMessage);
}

static void error_and_exit(char* message) {

    fprintf(stderr, "ERROR: %s - try \"rxvm -h\"\n", message);
    exit(2);
}

static void license() {
    char *message = CREXX_LICENSE_TEXT;

    printf("%s",message);
}

int main(int argc, char *argv[]) {
    char *file_name;
    char *combined_location = 0;
    char *exe_path = 0;
    int i, j;
    int rc;
    rxvm_context context;
    size_t num_modules;

    platform_install_signal_handlers();

#ifdef _WIN32
    /* Enable UTF-8 Processes */
    SetConsoleOutputCP(CP_UTF8);

    /* Enable ANSI virtual terminal sequences */
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif

    /* Init Context */
    rxinimod(&context);

    /*
     * Load VM RXAS Plugin(s)
     * Note in general the last plugin loaded has priority.
     * In future versions this will be extended to allow dynamic (via RXAS instructions) selection of the
     * RXAS plugin to use on a procedure-by-procedure basis
     */

    // First - the linker "magic" will take care of initializing static linked plugins with auto-initializers

    // Secondly - we manually initialize the plugins that are statically linked with manual initializers (hardcoded)
    CALL_PLUGIN_INITIALIZER(decnumber);

    // Finally - we manually load the dynamic plugins as we process the command line arguments (-p)

    /* Parse arguments  */
    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (strlen(argv[i]) > 2) {
            error_and_exit("Invalid argument");
        }
        switch (toupper((argv[i][1]))) {
            case '-':
                break;

            case 'L': /* Working Location / Directory */
                i++;
                if (i >= argc) {
                    error_and_exit("Missing location after -l");
                }
                context.location = argv[i];
                break;

            case 'P': /* Load Plugin */
                i++;
                if (i >= argc) {
                    error_and_exit("Missing plugin after -p");
                }
                if (load_rxvmplugin(0, argv[i]) != 0) {
                    fprintf(stderr, "ERROR loading plugin %s\n", argv[i]);
                    exit(-1);
                }
                break;

            case 'V': /* Version */
#ifdef NTHREADED
                printf("%s (Bytecode Mode)\n", rxversion);
#else
                printf("%s (Threaded Mode)\n", rxversion);
#endif
                exit(0);

            case 'H': /* Help */
            case '?':
                help();
                exit(0);

            case 'C': /* License */
                license();
                exit(0);

#ifndef NDEBUG
            case 'D': /* Debug */
                context.debug_mode = 1;
                break;
#endif

            default:
                error_and_exit("Invalid argument");
        }
    }
    num_modules = argc - i;
    if (!num_modules) {
        error_and_exit("No input files");
    }

    /* Add current and executable path to location */
    exe_path = exepath();
    if (context.location) {
        combined_location = malloc(strlen(context.location) + strlen(exe_path) + 5);
        sprintf(combined_location, ".;%s;%s", context.location, exe_path);
        context.location = combined_location;
    } else {
        combined_location = malloc(strlen(exe_path) + 5);
        sprintf(combined_location, ".;%s", exe_path);
        context.location = combined_location;
    }
    free(exe_path);

    for (j=0; j<num_modules; j++) {

        file_name = argv[i++];

        /* Check for -a - start of program arguments */
        if (file_name[0] == '-') {
            if (strlen(file_name) > 2) {
                error_and_exit("Invalid argument, expecting \"-a\"");
            }
            if (toupper((file_name[1])) != 'A') {
                error_and_exit("Invalid argument, expecting \"-a\"");
            }
            num_modules = j;
            if (!num_modules) {
                error_and_exit("No input files before arguments");
            }
            break;
        }

        /* Load Module */
        if (rxldmod(&context, file_name) <= 0) {
            fprintf(stderr, "ERROR reading module file %s\n", file_name);
            exit(-1);
        }
    }

#ifdef LINK_CREXX_LIB
    /* Load CREXX Library from linked buffer */
    if (rxldmodm(&context, (char*)&rx__pg, rx__pg_l) == 0) {
        fprintf(stderr, "ERROR reading linked library buffer\n");
        exit(-1);
    }
#endif

    /* Load plugins statically linked from linked buffer */
    if (rxldmodp(&context) == -1) {
        fprintf(stderr, "ERROR reading linked plugins\n");
        exit(-1);
    }

    /* Run the program */
#ifndef NDEBUG
    if (context.debug_mode) printf("Starting Execution\n");
#endif

    rc = rxvm_run(&context, argc - i, argv + i);

    /* Free Memory */
    rxfremod(&context);
    clear_rxvmplugin_factories();

    return rc;
}
