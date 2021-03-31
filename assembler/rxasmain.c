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
#include "rx_intrp.h"

int main(int argc, char *argv[]) {

    FILE *fp, *traceFile;
    char *buff, *buff_end;
    size_t bytes;
    int token_type;
    Token *token;
    Assembler_Context scanner;
    void *parser;
    char* file_name;

    printf("REXX Assembler Testbed Version: PoC 2\n\n");

    if (argc !=2) {
        printf("Invalid Arguments\nFormat: rxas file_name\n");
        exit (-1);
    }
    if (strcmp(argv[1],"-v") == 0) {
        printf("Version: PoC 2 Build 1\n");
        exit (0);
    }
    file_name = argv[1];

    /* Print - Architecture */
    printf("OS Architecture Details\n");
    printf("Type sizes: int=%d, char=%d, void*=%d, double=%d, long=%d, long long=%d size_t=%d\n",
           sizeof(int), sizeof(char), sizeof(void*), sizeof(double), sizeof(long), sizeof(long long), sizeof(size_t));

    // Load and Print Instruction Database
    init_ops();
    print_ops();

    /* Opening and Assemble file */
    printf("Assembling %s\n", file_name);

    /* Open input file */
    if (argc==2)   fp = fopen(argv[1], "r");
    else fp = fopen(file_name, "r");
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

        // Setup and parse token
        token = token_f(&scanner, token_type);
//prnt_tok(token); /* printf("\n"); */
        Parse(parser, token_type, token, &scanner);

        // Execute Parse for the last time
        if(token_type == EOS) {
            Parse(parser, 0, NULL, &scanner);
            break;
        }
    }

    /* Backpatch and check references */
    backpatch(&scanner);

    /* Print Errors */
    prnt_err(&scanner);

    /* Free Assembler Work Data */
    free_assembler(&scanner);

    printf("Assembler Complete\n\n");

    /* Disassemble */
    printf("Running Disassembler\n\n");
    disassemble(&scanner, stdout);

    /* Run */
    printf("\nRunning Program\n");
    run(&(scanner.binary));

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
