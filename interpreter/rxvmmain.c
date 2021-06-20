#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "platform.h"
#include "rxvminst.h"
#include "rxvmintp.h"
#include "rxas.h"

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
            "  -c              Prints Copyright & License Details\n"
#ifndef NDEBUG
            "  -d              Debug/Trace Mode\n"
#endif
            "  -l location     Working Location (directory)\n"
            "  -v              Prints Version\n";

    printf("%s",helpMessage);
}

static void error_and_exit(int rc, char* message) {

    fprintf(stderr, "ERROR: %s - try \"rxvm -h\"\n", message);
    exit(rc);
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

int main(int argc, char *argv[]) {

    FILE *fp;
    module *pgm;
    char *file_name;
    int debug_mode = 0;
    int i, j;
    int num_modules;
    char *location = 0;

    /* Parse arguments  */
    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (strlen(argv[i]) > 2) {
            error_and_exit(2, "Invalid argument");
        }
        switch (toupper((argv[i][1]))) {
            case '-':
                break;

            case 'L': /* Working Location / Directory */
                i++;
                if (i >= argc) {
                    error_and_exit(2, "Missing location after -l");
                }
                location = argv[i];
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
                debug_mode = 1;
                break;
#endif

            default:
                error_and_exit(2, "Invalid argument");
        }
    }

    num_modules = argc - i;
    if (!num_modules) {
        error_and_exit(2, "No input files");
    }
    pgm = malloc(sizeof(module) * num_modules);

    for (j=0; j<num_modules; j++) {

        file_name = argv[i++];
        if (file_name[0] == '-') {
            if (strlen(file_name) > 2) {
                error_and_exit(2, "Invalid argument, expecting \"-a\"");
            }
            if (toupper((file_name[1])) != 'A') {
                error_and_exit(2, "Invalid argument, expecting \"-a\"");
            }
            num_modules = j;
            if (!num_modules) {
                error_and_exit(2, "No input files before arguments");
            }
            break;
        }

        pgm[j].name = file_name;
        fp = openfile(file_name, "rxbin", location, "rb");
        if (!fp) {
            fprintf(stderr, "ERROR opening file %s\n", file_name);
            exit(-1);
        }

        fread(&pgm[j].segment.globals, 1, sizeof(int), fp);
        fread(&pgm[j].segment.inst_size, 1, sizeof(size_t), fp);
        fread(&pgm[j].segment.const_size, 1, sizeof(size_t), fp);

        pgm[j].segment.binary = calloc(pgm[j].segment.inst_size, sizeof(bin_code));
        pgm[j].segment.const_pool = calloc(pgm[j].segment.const_size, 1);

        fread(pgm[j].segment.binary, sizeof(bin_code), pgm[j].segment.inst_size, fp);
        fread(pgm[j].segment.const_pool, 1, pgm[j].segment.const_size, fp);

        pgm[j].segment.module_index = j;
        pgm[j].globals = calloc(pgm[j].segment.globals, sizeof(value));

        fclose(fp);
    }

    /* Run the program */
#ifndef NDEBUG
    if (debug_mode) printf("Starting Execution\n");
#endif

    run(num_modules, pgm, argc - i, argv + i, debug_mode);

    /* Free Memory */
    for (j=0; j<num_modules; j++) {
        free(pgm[j].segment.binary);
        free(pgm[j].segment.const_pool);
        free(pgm[j].globals);
    }
    free(pgm);
    return 0;
}
