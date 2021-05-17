// REXX Assembler
// Main Program
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rxasgrmr.h"
#include "rxas.h"
#include "rxvminst.h"
#include "rxasassm.h"

static void help() {
    char* helpMessage =
        "cREXX Assembler\n"
        "Version : " rxversion "\n"
        "Usage   : rxas [options] source_file\n"
        "Options :\n"
        "  -h              Prints help message\n"
        "  -c              Prints Copyright & License Details\n"
        "  -v              Prints Version\n"
        "  -a              Print Architecture Details\n"
        "  -i              Print Instructions\n"
        "  -d              Debug/Verbose Mode\n"
        "  -o output_file  Binary Output File\n";

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

    FILE *fp = 0, *traceFile = 0, *outFile = 0;
    char *buff, *buff_end;
    size_t bytes;
    int token_type;
    Token *token;
    Assembler_Context scanner;
    void *parser;
    char* file_name = 0;
    char *output_file_name = 0;
    char *extention;
    int debug_mode = 0;
    int i;
    bin_space *pgm;

    // Load Instruction Database
    init_ops();

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
                printf("Type sizes: int=%d, char=%d, void*=%d, double=%d, long=%d, rxinteger(aka long long)=%d size_t=%d\n",
                       (int) sizeof(int),  (int) sizeof(char),      (int) sizeof(void*), (int) sizeof(double),
                       (int) sizeof(long), (int) sizeof(rxinteger), (int) sizeof(size_t));
                exit(0);

            case 'I': /* Instructions */
                print_ops();
                exit(0);

            case 'D': /* Debug Mode */
                debug_mode = 1;
                break;

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

    /* Opening and Assemble file */
    if (debug_mode) printf("Assembling %s\n", file_name);

    /* Open input file */
    fp = fopen(file_name, "r");

    if(fp == NULL) {
        fprintf(stderr, "Can't open input file\n");
        exit(-1);
    }

    /* Open trace file */
    if (debug_mode) {
        traceFile = fopen("trace.out", "w");
        if (traceFile == NULL) {
            fprintf(stderr, "Can't open trace file\n");
            exit(-1);
        }
    }

    /* Get file size */
    fseek(fp, 0, SEEK_END);
    bytes = ftell(fp);
    rewind(fp);

    /* Allocate buffer and read */
    buff = (char*) malloc((bytes + 1) * sizeof(char) );
    bytes = fread(buff, 1, bytes, fp);
    if (!bytes) {
        fprintf(stderr, "Error reading input file\n");
        exit(-1);
    }
    buff[bytes] = 0;

    /* Initialize scanner */
    scanner.top = buff;
    scanner.cursor = buff;
    scanner.linestart = buff;
    scanner.line = 1;
    scanner.token_head = 0;
    scanner.token_tail = 0;
    scanner.token_counter = 0;
    scanner.error_tail = 0;
    scanner.severity = 0;

    scanner.binary.globals = 0;
    scanner.binary.const_size = 0;
    scanner.binary.inst_size = 0;
    scanner.binary.binary = malloc(sizeof(bin_code) * 5000); /* todo */
    scanner.binary.const_pool = malloc(sizeof(unsigned char) * 5000); /* TODO */

    scanner.string_constants_tree = 0;
    scanner.proc_constants_tree = 0;
    scanner.label_constants_tree = 0;

    /* Pointer to the end of the buffer */
    buff_end = (char*) (((char*)buff) + bytes);

    /* Create parser and set up tracing */
    parser = ParseAlloc(malloc);
#ifndef NDEBUG
    if (debug_mode) ParseTrace(traceFile, "parser >> ");
#endif
    while((token_type = scan(&scanner, buff_end))) {

        // Skip Scanner Errors
        if (token_type < 0) continue;

        // EOS Special Processing
        if(token_type == EOS) {
            // Send a NEWLINE
            token = token_f(&scanner, NEWLINE);
            Parse(parser, NEWLINE, token, &scanner);

            // Send EOS
            token = token_f(&scanner, token_type);
            Parse(parser, token_type, token, &scanner);

            // Send a null
            Parse(parser, 0, NULL, &scanner);
            break;
        }

        // Setup and parse token
        token = token_f(&scanner, token_type);
        Parse(parser, token_type, token, &scanner);
    }

    /* Backpatch and check references */
    backpatch(&scanner);

    /* Print Errors */
    prnt_err(&scanner);

    if (debug_mode) printf("Assembler Complete\n");

    if (scanner.severity == 0) {

        if (output_file_name == 0) {
            extention = strrchr(file_name, '.');
            if (extention) {
                output_file_name = malloc(extention - file_name + 7);
                memcpy(output_file_name, file_name, extention - file_name);
                strcpy(output_file_name + (extention - file_name), ".rxbin");
            } else {
                output_file_name = malloc(strlen(file_name) + 7);
                strcpy(output_file_name, file_name);
                strcat(output_file_name, ".rxbin");
            }

            if (debug_mode) printf("Writing to %s\n", output_file_name);
            outFile = fopen(output_file_name, "wb");
            free(output_file_name);
            if (outFile == NULL) {
                fprintf(stderr, "Can't open output file: %s\n",
                        output_file_name);
                exit(-1);
            }
        } else {
            if (debug_mode) printf("Writing to %s\n", output_file_name);
            outFile = fopen(output_file_name, "wb");
            if (outFile == NULL) {
                fprintf(stderr, "Can't open output file: %s\n",
                        output_file_name);
                exit(-1);
            }
        }

        pgm = &scanner.binary;
        fwrite(&pgm->globals, sizeof(pgm->globals), 1, outFile);
        fwrite(&pgm->inst_size, sizeof(pgm->inst_size), 1, outFile);
        fwrite(&pgm->const_size, sizeof(pgm->const_size), 1, outFile);

        fwrite(pgm->binary, sizeof(bin_code), pgm->inst_size, outFile);
        fwrite(pgm->const_pool, pgm->const_size, 1, outFile);

        fclose(outFile);
    }

    /* That's it */
    if (debug_mode) printf("Shutting Down\n");

    /* Deallocate Binary */
    free(scanner.binary.binary);
    free(scanner.binary.const_pool);

    /* Free Assembler Work Data */
    free_assembler(&scanner);

    /* Deallocate parser */
    ParseFree(parser, free);

    /* Deallocate Tokens */
    free_tok(&scanner);

    /* Deallocate Error */
    free_err(&scanner);

    // Free Instruction Database
    free_ops();

    /* Close files and deallocate */
    fclose(fp);
    if (traceFile) fclose(traceFile);

    /* Free Binary Buffer */
    if (buff) free(buff);

    return(0);
}
