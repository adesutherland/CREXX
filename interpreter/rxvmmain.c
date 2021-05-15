#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rxvminst.h"
#include "rxvmintp.h"
#include "rxas.h"

static void help() {
    char* helpMessage =
            "cREXX VM/Interpreter\n"
            "Version : " rxversion "\n"
            "Usage   : rxvm [options] binary_file\n"
            "Options :\n"
            "  -h              Prints help message\n"
            "  -c              Prints Copyright & License Details\n"
#ifndef NDEBUG
            "  -d              Debug/Trace Mode\n"
#endif
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
    bin_space pgm;
    char *file_name;
    int debug_mode = 0;
    int i;

    /* Parse arguments  */
    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (strlen(argv[i]) > 2) {
            error_and_exit(2, "Invalid argument");
        }
        switch (toupper((argv[i][1]))) {
            case '-':
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
            case 'D': /* Debug */
                debug_mode = 1;
                break;
#endif

            default:
                error_and_exit(2, "Invalid argument");
        }
    }

    if (i == argc) {
        error_and_exit(2, "Missing input file");
    }

    file_name = argv[i++];

    fp = fopen(file_name, "rb");
    if (!fp) {
        fprintf(stderr, "ERROR opening file %s\n", file_name);
        exit(-1);
    }

    fread(&pgm.globals, 1, sizeof(int), fp);
    fread(&pgm.inst_size, 1, sizeof(size_t), fp);
    fread(&pgm.const_size, 1, sizeof(size_t), fp);

    pgm.binary     = calloc(pgm.inst_size, sizeof(bin_code));
    pgm.const_pool = calloc(pgm.const_size, 1);

    fread(pgm.binary, sizeof(bin_code), pgm.inst_size, fp);
    fread(pgm.const_pool, 1, pgm.const_size, fp);

    fclose(fp);

    init_ops(); /* TODO we need to remove this */

    /* Run the program */
    run(&pgm, argc - i, argv + i, debug_mode);
}
