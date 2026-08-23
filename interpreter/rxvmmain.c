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
#include "rxvmprocessworker.h"
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
            "  --provider-path directories\n"
            "                  Trusted ';'-separated RXPA provider directories\n"
            "  -c              Prints Copyright & License Details\n"
#ifndef NDEBUG
            "  -d              Debug/Trace Mode\n"
#endif
#ifdef CREXX_VM_PROFILING
            "  --profile       Print VM instruction/transition timing profile\n"
            "  --profile=counts\n"
            "                  Print deterministic counts with timing fields zero\n"
            "  --profile-output file\n"
            "                  Write profile to file (.csv selects CSV format)\n"
            "  --sequence-count N\n"
            "                  Extract executed sequential windows (N is 2, 3, or 4)\n"
            "  --sequence-output file.rxseq\n"
            "                  Write the instruction-sequence execution profile\n"
#endif
            "  -l location     Working Location (directory)\n"
            "  -v              Prints Version\n"
            "\n*   VM Extension Plugin are specified by the full file name without the extension\n"
            "    Multiple plugins can be loaded by specifying multiple -p options\n"
            "    E.g. -p myplugin1 -p dir/myplugin2\n"
            "    *** Defining plugin location to be improved in release version ***\n"; // todo

    printf("%s",helpMessage);
}

static char *provider_application_directory(const char *module_path) {
    const char *slash;
    size_t prefix_length;
    char *result;

    if (!module_path || !*module_path) return 0;
    slash = strrchr(module_path, '/');
#ifdef _WIN32
    {
        const char *backslash = strrchr(module_path, '\\');
        if (!slash || (backslash && backslash > slash)) slash = backslash;
    }
#endif
    prefix_length = slash ? (size_t)(slash - module_path + 1) : 0u;
    result = malloc(prefix_length + strlen("providers") + 1u);
    if (!result) return 0;
    if (prefix_length) memcpy(result, module_path, prefix_length);
    strcpy(result + prefix_length, "providers");
    return result;
}

static char *provider_search_path(const char *application,
                                  const char *configured,
                                  const char *environment,
                                  const char *executable) {
    const char *parts[4];
    size_t index;
    size_t length = 1u;
    char *result;

    parts[0] = application;
    parts[1] = configured;
    parts[2] = environment;
    parts[3] = executable;
    for (index = 0u; index < 4u; index++) {
        if (parts[index] && *parts[index]) length += strlen(parts[index]) + 1u;
    }
    result = malloc(length);
    if (!result) return 0;
    result[0] = 0;
    for (index = 0u; index < 4u; index++) {
        if (!parts[index] || !*parts[index]) continue;
        if (*result) strcat(result, ";");
        strcat(result, parts[index]);
    }
    return result;
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
    char *application_provider_path = 0;
    char *installed_provider_path = 0;
    const char *configured_provider_path = 0;
    int i, j;
    int rc;
    rxvm_context context;
    size_t num_modules;

    platform_install_signal_handlers();

    /* Private, rebuild-together process-provider worker mode. It is kept out
     * of public help and executes only the versioned framed task protocol. */
    if (argc == 3 && strcmp(argv[1], "--rxvm-process-worker") == 0) {
        return rxvm_process_worker_main(argv[2]);
    }

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
    if (rxvm_signal_bind_process_main(&context) != 0) {
        fprintf(stderr, "ERROR: process-main VM interrupt target is already bound\n");
        rxfremod(&context);
        return 2;
    }

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
        if (strcmp(argv[i], "--provider-path") == 0) {
            i++;
            if (i >= argc || !argv[i][0])
                error_and_exit("Missing directories after --provider-path");
            configured_provider_path = argv[i];
            continue;
        }
        if (strncmp(argv[i], "--provider-path=", 16) == 0) {
            if (!argv[i][16])
                error_and_exit("Missing directories after --provider-path=");
            configured_provider_path = argv[i] + 16;
            continue;
        }
#ifdef CREXX_VM_PROFILING
        if (strcmp(argv[i], "--sequence-count") == 0) {
            char *end = 0;
            unsigned long value;
            i++;
            if (i >= argc || !argv[i][0])
                error_and_exit("Missing value after --sequence-count");
            value = strtoul(argv[i], &end, 10);
            if (*end || value < 2 || value > 4)
                error_and_exit("Invalid sequence count (expected 2, 3, or 4)");
            context.sequence_count = (unsigned int)value;
            continue;
        }
        if (strncmp(argv[i], "--sequence-count=", 17) == 0) {
            char *end = 0;
            unsigned long value = strtoul(argv[i] + 17, &end, 10);
            if (!argv[i][17] || *end || value < 2 || value > 4)
                error_and_exit("Invalid sequence count (expected 2, 3, or 4)");
            context.sequence_count = (unsigned int)value;
            continue;
        }
        if (strcmp(argv[i], "--sequence-output") == 0) {
            i++;
            if (i >= argc || !argv[i][0])
                error_and_exit("Missing filename after --sequence-output");
            context.sequence_output = argv[i];
            continue;
        }
        if (strncmp(argv[i], "--sequence-output=", 18) == 0) {
            if (!argv[i][18])
                error_and_exit("Missing filename after --sequence-output=");
            context.sequence_output = argv[i] + 18;
            continue;
        }
        if (strcmp(argv[i], "--profile") == 0 ||
                strcmp(argv[i], "--profile=timing") == 0) {
            context.profile_mode = 1;
            continue;
        }
        if (strcmp(argv[i], "--profile=counts") == 0) {
            context.profile_mode = 2;
            continue;
        }
        if (strcmp(argv[i], "--profile-output") == 0) {
            i++;
            if (i >= argc || !argv[i][0]) {
                error_and_exit("Missing filename after --profile-output");
            }
            if (!context.profile_mode) context.profile_mode = 1;
            context.profile_output = argv[i];
            continue;
        }
        if (strncmp(argv[i], "--profile-output=", 17) == 0) {
            if (!argv[i][17]) {
                error_and_exit("Missing filename after --profile-output=");
            }
            if (!context.profile_mode) context.profile_mode = 1;
            context.profile_output = argv[i] + 17;
            continue;
        }
        if (strncmp(argv[i], "--profile=", 10) == 0) {
            error_and_exit("Invalid profile mode (expected timing or counts)");
        }
#endif
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
#ifdef CREXX_VM_PROFILING
    if (context.sequence_count && !context.sequence_output)
        error_and_exit("--sequence-count requires --sequence-output");
    if (context.sequence_output && !context.sequence_count)
        error_and_exit("--sequence-output requires --sequence-count");
    if (context.profile_mode && context.sequence_count)
        error_and_exit("--profile and --sequence-count are separate run modes");
#endif
    num_modules = argc - i;
    if (!num_modules) {
        error_and_exit("No input files");
    }

    /* Configure trusted provider discovery separately from the legacy module
     * path.  In particular, the implicit current-directory module lookup is
     * never promoted into an unrestricted native-library search. */
    exe_path = exepath();
    application_provider_path = provider_application_directory(argv[i]);
    if (exe_path && *exe_path) {
        installed_provider_path = malloc(strlen(exe_path) +
                                         strlen("/providers") + 1u);
        if (installed_provider_path)
            sprintf(installed_provider_path, "%s/providers", exe_path);
    }
    if (!application_provider_path ||
        ((exe_path && *exe_path) && !installed_provider_path)) {
        error_and_exit("Unable to allocate provider search path");
    }
    context.provider_location = provider_search_path(
            application_provider_path, configured_provider_path,
            getenv("CREXX_PROVIDER_PATH"), installed_provider_path);
    free(application_provider_path);
    free(installed_provider_path);
    if (!context.provider_location)
        error_and_exit("Unable to allocate provider search path");

    /* Add current and executable path to legacy module location. */
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
    {
        const rxvm_program_generation *generation = 0;
        rxvm_program_result generation_result =
                rxvm_program_generation_seal(&context, &generation);

        /* Existing native-bearing images remain valid controller programs,
         * but cannot seed attached bytecode workers.  Preserve their normal
         * startup and let a later chanopen report provider unavailability. */
        if (generation_result != RXVM_PROGRAM_OK &&
            generation_result != RXVM_PROGRAM_NATIVE_EXCLUDED) {
            fprintf(stderr, "ERROR sealing executable program generation\n");
            rxfremod(&context);
            clear_rxvmplugin_factories();
            return -1;
        }
    }

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
