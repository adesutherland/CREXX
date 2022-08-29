// REXX Assembler
// Main Program
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rxvminst.h"
#include "rxasassm.h"

static void help() {
    char* helpMessage =
        "cREXX Assembler\n"
        "Version : " rxversion "\n"
        "Usage   : rxas [options] source_file\n"
        "Options :\n"
        "  -h              Help Message\n"
        "  -c              Copyright & License Details\n"
        "  -v              Version\n"
        "  -a              Architecture Details\n"
        "  -i              Print Instructions\n"
        "  -d              Debug/Verbose Mode\n"
        "  -l location     Working Location (directory)\n"
        "  -o output_file  Binary Output File\n"
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

    fprintf(stderr, "ERROR: %s - try \"rxas -h\"\n", message);
    exit(rc);
}

int main(int argc, char *argv[]) {
    Assembler_Context scanner;
    int i;

    /* Load Instruction Database */
    init_ops();

    /* scanner context parameter */
    scanner.debug_mode = 0;
    scanner.traceFile = 0;
    scanner.optimise = 1;
    scanner.file_name = 0;
    scanner.output_file_name = 0;
    scanner.location = 0;

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
                    error_and_exit(2, "Missing output file after -o");
                }
                scanner.output_file_name = argv[i];
                break;

            case 'L': /* Working Location / Directory */
                i++;
                if (i >= argc) {
                    error_and_exit(2, "Missing location after -l");
                }
                scanner.location = argv[i];
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

            case 'A': /* Architecture */
                printf("OS Architecture Details\n");
                printf("Type sizes: int=%d, char=%d, void*=%d, double=%d, long double=%d, long=%d, rxinteger(aka long long)=%d size_t=%d\n",
                       (int) sizeof(int),  (int) sizeof(char),        (int) sizeof(void*),     (int) sizeof(double),
                       (int) sizeof(long double), (int) sizeof(long), (int) sizeof(rxinteger), (int) sizeof(size_t));
                exit(0);

            case 'I': /* Instructions */
                prt_ops();
                exit(0);

            case 'N': /* No optimisation */
                scanner.optimise = 0;
                break;

            case 'D': /* Debug Mode */
                scanner.debug_mode = 1;
                break;

            default:
                error_and_exit(2, "Invalid argument");
        }
    }

    if (i == argc) {
        error_and_exit(2, "Missing input source file");
    }

    scanner.file_name = argv[i++];

    if (i < argc) {
        error_and_exit(2, "Unexpected Arguments");
    }

    /* Open trace file */
    if (scanner.debug_mode) {
        scanner.traceFile = openfile("trace", "out", scanner.location, "w");
        if (scanner.traceFile == NULL) {
            fprintf(stderr, "Can't open trace file\n");
            exit(-1);
        }
    }

    /* Assemble */
    if (rxasmble(&scanner)) exit(-1);

    // Free Instruction Database
    free_ops();

    if (scanner.traceFile) fclose(scanner.traceFile);

    return(scanner.severity);
}
