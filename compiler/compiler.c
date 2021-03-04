/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "rexxgrmr.h"
#include "compiler.h"


int main(int argc, char *argv[]) {

    FILE *fp, *traceFile;
    char *buff, *buff_end;
    size_t bytes;
    int token_type;
    Token *token;
    Scanner scanner;
    void *parser;

    /* Open input file */
    if (argc==2)   fp = fopen(argv[1], "r");
    else fp = fopen("test.rexx", "r");
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
    scanner.line = 0;
    scanner.token_head = 0;
    scanner.token_tail = 0;
    scanner.token_counter = 0;
    scanner.ast = 0;
    scanner.last_node = 0;

    /* Pointer to the end of the buffer */
    buff_end = (char*) (((char*)buff) + bytes);

    /* Create parser and set up tracing */
    parser = ParseAlloc(malloc);
#ifndef NDEBUG
    ParseTrace(traceFile, "parser >> ");
#endif
    while((token_type = scan(&scanner, buff_end))) {
        // Skip Scanner Errors - TODO
        if (token_type < 0) continue;

        // Setup and parse token
        token = token_f(&scanner, token_type);
        Parse(parser, token_type, token, &scanner);

        // Execute Parse for the last time
        if(token_type == SY_EOS) {
            Parse(parser, 0, NULL, &scanner);
            break;
        }
    }

    /* Deallocate parser */
    ParseFree(parser, free);

    if (scanner.ast) {
        prnt_ast(scanner.ast);
        printf("\n");
    }

    if (scanner.ast) {
        int counter = 0;
        printf("digraph REXX {\n");
        pdot_ast(scanner.ast, -1, &counter);
        printf("\n}\n");
    }

    /* Deallocate AST */
    if (scanner.ast) free_ast(scanner.ast);

    /* Deallocate Tokens */
    free_tok(&scanner);

    /* Close files and deallocate */
    fclose(fp);
    fclose(traceFile);
    free(buff);
    return(0);
}
