#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "platform.h"
#include "rxdadism.h"
#include "rxvminst.h"

static void help() {
    char* helpMessage =
            "cREXX Disassembler\n"
            "Version : " rxversion "\n"
            "Usage   : rxdas [options] binary_file\n"
            "Options :\n"
            "  -h              Help message\n"
            "  -c              Copyright & License Details\n"
            "  -v              Version\n"
            "  -l location     Working Location (directory)\n"
            "  -o output_file  Output File (default is stdout)\n";

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
                output_file_name = argv[i];
                break;

            case 'L': /* Working Location / Directory */
                i++;
                if (i >= argc) {
                    error_and_exit(2, "Missing location after -l");
                }
                location = argv[i];
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

    fread(&pgm.globals, 1, sizeof(int), fp);
    fread(&pgm.inst_size, 1, sizeof(size_t), fp);
    fread(&pgm.const_size, 1, sizeof(size_t), fp);

    pgm.binary     = calloc(pgm.inst_size, sizeof(bin_code));
    pgm.const_pool = calloc(pgm.const_size, 1);

    fread(pgm.binary, sizeof(bin_code), pgm.inst_size, fp);
    fread(pgm.const_pool, 1, pgm.const_size, fp);

    fclose(fp);

    init_ops();

    if (output_file_name) {
        output = openfile(output_file_name, "rxas", location, "w");
        if (!output) {
            fprintf(stderr, "ERROR: opening output file %s\n", output_file_name);
            exit(-1);
        }
    }

    disassemble(&pgm, output);

    if (output_file_name) {
        fclose(output);
    }

    return 0;
}
