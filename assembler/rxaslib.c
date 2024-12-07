//
// Created by Adrian Sutherland on 09/08/2022.
//
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rxasgrmr.h"
#include "rxas.h"
#include "rxvminst.h"
#include "rxasassm.h"
#include "rxbin.h"

/* Main Assembler Function
 * Returns: 0 on success, -1 on an error
 * scanner should be clear with the following set:
 *   scanner->debug_mode
 *   scanner->traceFile
 *   scanner->optimise
 *   scanner->file_name
 *   scanner->output_file_name
 *   scanner->location
 */
int rxasmble(Assembler_Context *scanner) {
    FILE *outFile;
    int token_type;
    Assembler_Token *token;
    bin_space *pgm;
    module_file module;

    /* Opening and Assemble file */
    if (scanner->debug_mode) printf("Assembling %s\n", scanner->file_name);
    if (rxasinfl(scanner,0)) return -1;

    /* Parse & Process */
    rxaspars(scanner);
    if (scanner->debug_mode) printf("Assembler Complete\n");

    /* Print Errors */
    rxasperr(scanner);

    /* Output rxbin */
    if (rxasoutf(scanner)) return -1;

    /* That's it */
    if (scanner->debug_mode) printf("Assembling complete\n");

    rxasclrc(scanner);

    return 0;
}

/* Init Assembler Context from Buffer
 * Returns: 0 on success, -1 on an error
 * scanner should be clear with the following set:
 *   scanner->debug_mode
 *   scanner->traceFile
 *   scanner->optimise
 *   scanner->file_name (nominal)
 *   scanner->output_file_name
 *   scanner->location
 *   scanner->buff
 *   scanner->buff_end
 */
int rxasinbf(Assembler_Context *scanner) {
    /* Initialize scanner */
    scanner->top = scanner->buff;
    scanner->cursor = scanner->buff;
    scanner->linestart = scanner->buff;
    scanner->line = 1;
    scanner->token_head = 0;
    scanner->token_tail = 0;
    scanner->token_counter = 0;
    scanner->error_tail = 0;
    scanner->severity = 0;
    scanner->optimiser_queue = calloc(sizeof(instruction_queue),
                                      OPTIMISER_TARGET_MAX_QUEUE_SIZE +
                                      OPTIMISER_QUEUE_EXTRA_BUFFER_SIZE);
    scanner->optimiser_queue_items = 0;
    scanner->optimiser_counter = 0;

    scanner->binary.globals = 0;
    scanner->binary.const_size = 0;
    scanner->binary.inst_size = 0;

    scanner->inst_buffer_size = 1024;
    scanner->binary.binary = malloc(scanner->inst_buffer_size * sizeof(bin_code));
    memset(scanner->binary.binary,0,scanner->inst_buffer_size * sizeof(bin_code));

    scanner->const_buffer_size = sizeof(unsigned char) * 1024;
    scanner->binary.const_pool = malloc(scanner->const_buffer_size);
    memset(scanner->binary.const_pool,0,scanner->const_buffer_size);

    scanner->string_constants_tree = 0;
    scanner->proc_constants_tree = 0;
    scanner->label_constants_tree = 0;
    scanner->extern_constants_tree = 0;
    scanner->extern_regs = 0;
    scanner->proc_head = -1;
    scanner->proc_tail = -1;
    scanner->expose_head = -1;
    scanner->expose_tail = -1;
    scanner->meta_head = -1;
    scanner->meta_tail = -1;

    /* Create parser and set up tracing */
    scanner->parser = RxasmAlloc(malloc);
#ifndef NDEBUG
    if (scanner->debug_mode && scanner->traceFile) RxasmTrace(scanner->traceFile, "parser >> ");
#endif
    return 0;
}

/* Init Assembler Context from File
 * Returns: 0 on success, -1 on an error
 * scanner should be clear with the following set:
 *   scanner->debug_mode
 *   scanner->traceFile
 *   scanner->optimise
 *   scanner->file_name (nominal)
 *   scanner->output_file_name
 *   scanner->location
 *
 *  file_name_includes_type_extension should be set to true if the filename in the Assembler_Context
 *  already has the ".rxas" extension (otherwise it is added)
 */
int rxasinfl(Assembler_Context *scanner, int file_name_includes_type_extension) {
    FILE *fp;
    size_t bytes;

    /* Open input file */
    if (file_name_includes_type_extension) fp = openfile(scanner->file_name, "", scanner->location, "r");
    else fp = openfile(scanner->file_name, "rxas", scanner->location, "r");
    if (fp == NULL) {
        fprintf(stderr, "Can't open input file %s\n", scanner->file_name);
        return -1;
    }

    scanner->buff = file2buf(fp, &bytes);
    if (scanner->buff == NULL) {
        fprintf(stderr, "Can't read input file %s\n", scanner->file_name);
        return -1;
    }
    fclose(fp);
    /* Pointer to the end of the buffer */
    scanner->buff_end = (char*) (((char*)scanner->buff) + bytes);

    return rxasinbf(scanner);
}

/* Parse & Process Assembler Function
 * Returns: scanner->severity
 * scanner has to have been initiated
 */
int rxaspars(Assembler_Context *scanner) {
    int token_type;
    Assembler_Token *token;

    /* Parse & Process */
    while((token_type = rx_scan(scanner, scanner->buff_end))) {
        // Skip Scanner Errors
        if (token_type < 0) continue;
        // EOS Special Processing
        if(token_type == EOS) {
            // Send a NEWLINE
            token = rxast_f(scanner, NEWLINE);
            Rxasm(scanner->parser, NEWLINE, token, scanner);

            // Send EOS
            token = rxast_f(scanner, token_type);
            Rxasm(scanner->parser, token_type, token, scanner);

            // Send a null
            Rxasm(scanner->parser, 0, NULL, scanner);
            break;
        }
        // Setup and parse token
        token = rxast_f(scanner, token_type);
        Rxasm(scanner->parser, token_type, token, scanner);
    }

    /* Flush the Keyhole Optimiser Queue */
    flushopt(scanner);

    /* Backpatch and check references */
    backptch(scanner);

    return scanner->severity;
}

/* Output rxbin Function
 * Returns: 0 on success, otherwise an error
 * scanner has to have been initiated and parsed and scanner->output_file_name set
 */
int rxasoutf(Assembler_Context *scanner) {
    FILE *outFile;
    bin_space *pgm;
    module_file module;

    if (scanner->severity == 0) {
        /* Output File */
        if (scanner->output_file_name == 0) scanner->output_file_name = scanner->file_name;

        if (scanner->debug_mode) printf("Writing to %s\n", scanner->output_file_name);

        outFile = openfile(scanner->output_file_name, "rxbin", scanner->location, "wb");
        if (outFile == NULL) {
            fprintf(stderr, "Can't open output file: %s\n", scanner->output_file_name);
            return -1;
        }

        init_module(&module);
        pgm = &(scanner->binary);
        module.header.name_size = strlen(scanner->file_name) + 1;
        module.header.description_size = strlen(scanner->file_name) + 1;
        module.header.instruction_size = pgm->inst_size;
        module.header.constant_size = pgm->const_size;
        module.header.globals = pgm->globals;
        module.header.meta_head  = scanner->meta_head;
        module.header.proc_head  = scanner->proc_head;
        module.header.expose_head  = scanner->expose_head;
        module.name = scanner->file_name;
        module.description = scanner->file_name;
        module.instructions = pgm->binary;
        module.constant = pgm->const_pool;
        write_module(&module,outFile);
        fclose(outFile);
    }
    else {
        fprintf(stderr, "Errors in assembler can't generate output file: %s\n", scanner->output_file_name);
        return -1;
    }

    return 0;
}

/* Clear and Free Assembler Context */
void rxasclrc(Assembler_Context *scanner) {
    /* Deallocate Binary */
    if (scanner->binary.binary) free(scanner->binary.binary);
    if (scanner->binary.const_pool) free(scanner->binary.const_pool);

    /* Free Optimiser Queue */
    if (scanner->optimiser_queue) free(scanner->optimiser_queue);

    /* Free Assembler Work Data */
    freeasbl(scanner);

    /* Deallocate parser */
    if (scanner->parser) RxasmFree(scanner->parser, free);

    /* Deallocate Tokens */
    rxasf_t(scanner);

    /* Deallocate Error */
    rxasfrer(scanner);

    /* Free Binary Buffer */
    if (scanner->buff) free(scanner->buff);

    /* Zero Context */
    memset(scanner, 0, sizeof(Assembler_Context));
}
