// REXX Assembler
// Main Program
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rxasgrmr.h"
#include "rxas.h"
#include "operands.h"
#include "rxasassm.h"

int main(int argc, char *argv[]) {

    FILE *fp, *traceFile, *outFile;
    char *buff, *buff_end;
    size_t bytes;
    int token_type;
    Token *token;
    Assembler_Context scanner;
    void *parser;
    char* fileName;

    printf("REXX Assembler Testbed Version: PoC 2\n\n");

    if (argc !=2) {
        printf("Invalid Arguments\nFormat: rxas fileName\n");
        exit (-1);
    }
    if (strcmp(argv[1],"-v") == 0) {
        printf("Version: PoC 2 Build 2\n");
        exit (0);
    }
    fileName = argv[1];

    /* Print - Architecture */
    printf("OS Architecture Details\n");
    printf("Type sizes: int=%d, char=%d, void*=%d, double=%d, long=%d, long long=%d size_t=%d\n",
           (int) sizeof(int),  (int) sizeof(char),      (int) sizeof(void*), (int) sizeof(double),
           (int) sizeof(long), (int) sizeof(long long), (int) sizeof(size_t));

    // Load and Print Instruction Database
    init_ops();
    print_ops();

    /* Opening and Assemble file */
    printf("Assembling %s\n", fileName);

    /* Open input file */
    fp = fopen(fileName, "r");

    if(fp == NULL) {
        fprintf(stderr, "Can't open input file\n");
        exit(-1);
    }

    /* Open trace file */
    traceFile = fopen("trace.out", "w");
    if(traceFile == NULL) {
        fprintf(stderr, "Can't open trace file\n");
        exit(-1);
    }

    /* Get file size */
    fseek(fp, 0, SEEK_END);
    bytes = ftell(fp);
    rewind(fp);

    /* Allocate buffer and read */
    buff = (char*) malloc(bytes * sizeof(char));
    bytes = fread(buff, 1, bytes, fp);
    if (!bytes) {
        fprintf(stderr, "Error reading input file\n");
        exit(-1);
    }

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
    scanner.binary.binary = malloc(sizeof(bin_code) * 5000);
    scanner.binary.const_pool = malloc(sizeof(unsigned char) * 5000);

    scanner.string_constants_tree = 0;
    scanner.proc_constants_tree = 0;
    scanner.label_constants_tree = 0;

    /* Pointer to the end of the buffer */
    buff_end = (char*) (((char*)buff) + bytes);

    /* Create parser and set up tracing */
    parser = ParseAlloc(malloc);
#ifndef NDEBUG
    ParseTrace(traceFile, "parser >> ");
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

    /* Free Assembler Work Data */
    free_assembler(&scanner);

    printf("Assembler Complete\n\n");

    /* TODO: temp. writing to disk, must be made stable */
    strcat(fileName, ".out");

    {
        bin_space *pgm;

        pgm = &scanner.binary;

        outFile = fopen(fileName, "wb");

        fwrite(&pgm->globals,    sizeof(pgm->globals),    1, outFile);
        fwrite(&pgm->inst_size,  sizeof(pgm->inst_size),  1, outFile);
        fwrite(&pgm->const_size, sizeof(pgm->const_size), 1, outFile);

        fwrite(pgm->binary, sizeof(bin_code), pgm->inst_size, outFile );
        fwrite(pgm->const_pool, pgm->const_size, 1, outFile);

        fclose(outFile);
    }

    /* That's it */
    printf("\nShutting Down\n");

    /* Deallocate Binary */
    free(scanner.binary.binary);
    free(scanner.binary.const_pool);

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
    fclose(traceFile);
    free(buff);
    return(0);
}
