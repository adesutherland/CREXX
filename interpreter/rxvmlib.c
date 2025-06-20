/* CREXX VM Library Main */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "rxvmintp.h"
#include "rxvmplugin_framework.h"

/* Program Buffer */
extern char rx__pg[];
extern size_t rx__pg_l;


static void help() {
    char* helpMessage =
            "cREXX Embedded VM/Interpreter\n"
            "Version : " rxversion "\n"
#ifdef NTHREADED
            "        : Bytecode Mode\n"
#else
            "        : Threaded Mode\n"
#endif
            "cREXX Wrapper Options :\n"
            "  --h              Prints this Help Message\n"
            "  --c              Prints Copyright & License Details\n"
#ifndef NDEBUG
            "  --d              Debug/Trace Mode\n"
#endif
            "  --l location     Working Location (directory)\n"
            "  --v              Prints cREXX Wrapper Version\n\n"
            "Wrapper options (if any) must be the first arguments and program arguments follow\n";

    printf("%s",helpMessage);
}

static void error_and_exit(char* message) {

    fprintf(stderr, "ERROR: %s - try \"rxvm -h\"\n", message);
    exit(2);
}

static void license() {
    char *message =
            "cREXX License (MIT)\n"
            "Copyright (c) 2020-2025 Adrian Sutherland\n\n"

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

int main(int argc, char *argv[]) {

    char *file_name;
    int i, j;
    int rc;
    rxvm_context context;

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

    /* Parse arguments  */
    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        // If an argument is not a wrapper one, then break out of the loop
        if (strlen(argv[i]) != 3) break;
        if (argv[i][1] != '-') break;

        switch (toupper((argv[i][2]))) {
            case 'L': /* Working Location / Directory */
                i++;
                if (i >= argc) {
                    error_and_exit("Missing location after -l");
                }
                context.location = argv[i];
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
                goto end_wrapper_args; /* Not a wrapper argument, so break out of the loop */
        }
    }

end_wrapper_args:

    /* Load Module */
    if (rxldmodm(&context, (char*)&rx__pg, rx__pg_l) == 0) {
        fprintf(stderr, "ERROR reading buffer\n");
        exit(-1);
    }

    /* Load plugins statically linked from linked buffer */
    if (rxldmodp(&context) == -1) {
        fprintf(stderr, "ERROR reading linked plugins\n");
        exit(-1);
    }

    /*
     * Load VM RXAS Plugin(s)
     * Note in general the last plugin loaded has priority.
     * In future versions this will be extebded to allow dynamic (via RXAS instructins) selection of the
     * RXAS plugin to use on a procedure-by-procedure basis
     *
     * CURRENTLY THIS EMBEDDED VM DOES NOT SUPPORT DYNAMIC LOADING OF RXAS PLUGINS
     */
    // First - the linker "magic" will take care of initializing static linked plugins with auto-initializers
    // Secondly - we manually initialize the plugins that are statically linked with manual initializers (hardcoded)
    CALL_PLUGIN_INITIALIZER(dbnumber);

    /* Run the program */
#ifndef NDEBUG
    if (context.debug_mode) printf("Starting Execution\n");
#endif

    rc = run(&context, argc - i, argv + i);

    /* Free Memory */
    rxfremod(&context);

    return rc;
}
