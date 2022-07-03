#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "platform.h"
#include "rxdadism.h"
#include "rxvminst.h"
#include "rxbin.h"
//#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#endif
//#include <io.h>
//#include <fcntl.h>

static void help() {
    char* helpMessage =
            "cREXX Disassembler\n"
            "Version : " rxversion "\n"
            "Usage   : rxdas [options] binary_file\n"
            "Options :\n"
            "  -h              Help message\n"
            "  -c              Copyright & license details\n"
            "  -v              Version\n"
            "  -p              all constant Pool\n"
            "  -l location     working Location (directory)\n"
            "  -o output_file  Output file (default is stdout)\n";

    printf("%s",helpMessage);
}

static void error_and_exit(int rc, char* message) {

    fprintf(stderr, "ERROR: %s - try \"rxdas -h\"\n", message);
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
    bin_space pgm;
    char *file_name;
    char *location = 0;
    char *output_file_name = 0;
    FILE *output = stdout;
    int i;
    int print_all_constant_pool = 0;
    module_file *module;
    size_t modules_processed = 0;

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

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
                    error_and_exit(2, "Missing output file after -o");
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

            case 'p': /* constant Pool */
                print_all_constant_pool = 1;
                break;

            default:
                error_and_exit(2, "Invalid argument");
        }
    }

    if (i == argc) {
        error_and_exit(2, "Missing input file");
    }

    file_name = argv[i++];

    if (i < argc) {
        error_and_exit(2, "Unexpected Arguments");
    }

    fp = openfile(file_name, "rxbin", location, "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: opening file %s\n", file_name);
        exit (-1);
    }

    if (output_file_name) {
        output = openfile(output_file_name, "rxas", location, "w");
        if (!output) {
            fprintf(stderr, "ERROR: opening output file %s\n", output_file_name);
            exit(-1);
        }
    }

    init_ops();

    i = 0;
    while (i == 0) {
        module = 0;
        switch (i = read_module(&module, fp)) {
            case 0: /* Success */
                pgm.globals = module->header.globals;
                pgm.inst_size = module->header.instruction_size;
                pgm.const_size = module->header.constant_size;
                pgm.binary = module->instructions;
                pgm.const_pool = module->constant;
                disassemble(&pgm, module, output, print_all_constant_pool);
                free_module(module);
                module = 0;
                modules_processed++;
                break;

            case 1: /* eof */
                if (module) free_module(module);
                if (!modules_processed) {
                    fprintf(stderr, "ERROR: empty file %s\n", file_name);
                    exit(-1);
                }
                break;

            default: /* error */
                if (module) free_module(module);
                fprintf(stderr, "ERROR: reading file %s\n", file_name);
                exit(-1);
        }
    }

    fclose(fp);

    if (output_file_name) {
        fclose(output);
    }

    free_ops();

    return 0;
}
