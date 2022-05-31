//
// CREXX Binary File Structure and IO
// Created by Adrian Sutherland on 30/05/2022.
//
// In the style of a single file library with a few simple static functions
//

#ifndef CREXX_RXBIN_H
#define CREXX_RXBIN_H

#include <string.h>
#include <stdio.h>
#include <rxas.h>

#define BIN_VERSION "001"

#define BIN_HEADER "cReXx"

typedef struct module_header {
    char FILE_HEADER[sizeof(BIN_HEADER)];
    char FILE_VERSION[sizeof(BIN_VERSION)];
    size_t name_size;  /* number of byte/chars including null terminator */
    size_t description_size;  /* number of byte/chars including null terminator */
    size_t instruction_size;  /* number of 64 bit instructions */
    size_t constant_size; /* Number of bytes */
    int globals;
} module_header;

typedef struct module_file {
    module_header header;
    char* name; /* Null Terminated */
    char* description; /* Null Terminated */
    void* instructions;
    void* constant;
} module_file;

/* Sets Header Version */
static void set_header_version(module_header *header) {
    memcpy(header->FILE_HEADER, BIN_HEADER, sizeof(BIN_HEADER));
    memcpy(header->FILE_VERSION, BIN_VERSION, sizeof(BIN_VERSION));
}

/* Check Header Version */
/* 0 - OK */
/* 1 - Missing Header */
/* 2 - Version Mismatch */
static int check_header_version(module_header *header) {
    if (memcmp(header->FILE_HEADER, BIN_HEADER, sizeof(BIN_HEADER)) != 0) return 1;
    if (memcmp(header->FILE_VERSION, BIN_VERSION, sizeof(BIN_VERSION)) != 0) return 2;
    return 0;
}

/* Write out the module */
/* 0 on success, 1 on error (use perror) */
static int write_module(module_file *module, FILE *outFile) {

    set_header_version( &(module->header) );

    if (fwrite(&(module->header), sizeof(module->header), 1, outFile) != 1)
        return 1;

    if (fwrite(module->name, 1, module->header.name_size, outFile) != module->header.name_size)
        return 1;

    if (fwrite(module->description, 1, module->header.description_size, outFile) != module->header.description_size)
        return 1;

    if (fwrite(module->instructions, sizeof(bin_code), module->header.instruction_size, outFile) != module->header.instruction_size)
        return 1;

    if (fwrite(module->constant, 1, module->header.constant_size, outFile) != module->header.constant_size)
        return 1;

    return 0;
}

/* Read in the module */
/* The module is malloced (with more than one malloc call) - it must be freed with free_module() */
/* 0 on success,
 * 1 on eof
 * 2 on file version mismatch
 * -1 on error
 * (use perror), on an error you can/should use free_module() */
static int read_module(module_file **module, FILE *inFile) {
    *module = malloc(sizeof(module_file));
    if (*module == 0) return -1;

    /* Zero these so free_module() will not crash after read_module() error */
    (*module)->name = 0;
    (*module)->description = 0;
    (*module)->instructions = 0;
    (*module)->constant = 0;

    if (fread(*module, sizeof(module_header), 1, inFile) != 1) {
       if (feof(inFile)) return 1; /* No module to read - useful for calling in a loop from one file */
       else return -1; /* A real error */
    }

    /* Check Header */
    switch (check_header_version(&(*module)->header)) {
        case 1: return -1; /* Unknown error */
        case 2: return 2; /* Version mismatch */
        default:;
    }

    (*module)->name = malloc((*module)->header.name_size);
    if ( (*module)->name == 0 ) return -1;

    (*module)->description = malloc((*module)->header.description_size);
    if ( (*module)->description == 0 ) return -1;

    (*module)->instructions = malloc((*module)->header.instruction_size * sizeof(bin_code));
    if ( (*module)->instructions == 0 ) return -1;

    (*module)->constant = malloc((*module)->header.constant_size);
    if ( (*module)->constant == 0 ) return -1;

    if (fread( (*module)->name, 1, (*module)->header.name_size, inFile) !=
            (*module)->header.name_size)
        return -1;

    if (fread( (*module)->description, 1, (*module)->header.description_size, inFile) !=
            (*module)->header.description_size)
        return -1;

    if (fread( (*module)->instructions, sizeof(bin_code), (*module)->header.instruction_size, inFile) !=
            (*module)->header.instruction_size)
        return -1;

    if (fread( (*module)->constant, 1, (*module)->header.constant_size, inFile) !=
            (*module)->header.constant_size)
        return -1;

    return 0;
}

/* Free the module */
/* Free's the module returned by read_module() */
static void free_module(module_file *module) {
    if (module->name) free(module->name);
    if (module->description) free(module->description);
    if (module->instructions) free(module->instructions);
    if (module->constant) free(module->constant);
    free(module);
}

#endif //CREXX_RXBIN_H
