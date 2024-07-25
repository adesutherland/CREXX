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
#include <stdlib.h>

#define BIN_VERSION "001"

#define BIN_HEADER "cReXx" /* Do not change */

typedef struct bin_space bin_space;

/* cREXX Instruction Coding */
#pragma pack(push,4)
typedef struct instruction_coding {
    int opcode;
    int no_ops;
} instruction_coding;
#pragma pack(pop)

/* Single cREXX Binary Code Entry */
#pragma pack(push,4)
typedef union bin_code {
    instruction_coding instruction;
    void* impl_address;
    double fconst;
    rxinteger iconst;
    char cconst;
    size_t index;
} bin_code;
#pragma pack(pop)

/* cREXX Binary Program */
#pragma pack(push,4)
struct bin_space {
    int globals;
    size_t inst_size;
    size_t const_size;
    struct module *module;
    bin_code *binary;
    unsigned char *const_pool;
};
#pragma pack(pop)

enum const_pool_type {
    STRING_CONST, PROC_CONST, EXPOSE_REG_CONST, EXPOSE_PROC_CONST,
    META_SRC, META_FILE, META_FUNC, META_REG, META_CONST, META_CLEAR
};

/* cREXX chameleon entry in the constant pool
 * A poor C users abstract class!
 * */
typedef struct chameleon_constant {
    size_t size_in_pool; /* including any padding for alignment */
    enum const_pool_type type;
} chameleon_constant;

/* cREXX String entry in the constant pool */
typedef struct string_constant {
    chameleon_constant base;
    size_t string_len;
#ifndef NUTF8
    size_t string_chars;
#endif
    char string[1]; /* Must be last member */
} string_constant;

typedef struct stack_frame stack_frame;

/* cREXX Procedure entry in the constant pool */
typedef struct proc_constant {
    chameleon_constant base;
    int next;
    int locals;
    bin_space *binarySpace; // Pointer to the binary space in CREXX, for native functions this is set to NULL
    stack_frame **frame_free_list;
    stack_frame *frame_free_list_head;
    size_t start;
    size_t exposed;
    char name[1]; /* Must be last member */
} proc_constant;

/* cREXX Exposed Register entry in the constant pool */
typedef struct expose_reg_constant {
    chameleon_constant base;
    int next;
    int global_reg;
    char index[1]; /* Must be last member */
} expose_reg_constant;

/* cREXX Exposed Procedure entry in the constant pool */
typedef struct expose_proc_constant {
    chameleon_constant base;
    int next;
    size_t procedure;
    unsigned char imported : 1;
    char index[1]; /* Must be last member */
} expose_proc_constant;

/* cREXX Generic meta entry to hold prev/next offsets */
typedef struct meta_entry {
    chameleon_constant base;
    int prev;
    int next;
    size_t address;
} meta_entry;

/* cREXX Meta Source entry in the constant pool */
typedef struct meta_src_constant {
    meta_entry base;
    size_t line;
    size_t column;
    size_t source;
} meta_src_constant;

/* cREXX Meta File entry in the constant pool */
typedef struct meta_file_constant {
    meta_entry base;
    size_t file;
} meta_file_constant;

typedef struct meta_func_constant {
    meta_entry base;
    size_t symbol;
    size_t option;
    size_t type;
    size_t func;
    size_t args;
    size_t inliner;
} meta_func_constant;

typedef struct meta_reg_constant {
    meta_entry base;
    size_t symbol;
    size_t option;
    size_t type;
    size_t reg;
} meta_reg_constant;

typedef struct meta_const_constant {
    meta_entry base;
    size_t symbol;
    size_t option;
    size_t type;
    size_t constant;
} meta_const_constant;

typedef struct meta_clear_constant {
    meta_entry base;
    size_t symbol;
} meta_clear_constant;

typedef struct module_header {
    char FILE_HEADER[sizeof(BIN_HEADER)];
    char FILE_VERSION[sizeof(BIN_VERSION)];
    size_t name_size;  /* number of byte/chars including null terminator */
    size_t description_size;  /* number of byte/chars including null terminator */
    size_t instruction_size;  /* number of 64 bit instructions */
    size_t constant_size; /* Number of bytes */
    int globals;
    int proc_head;
    int expose_head;
    int meta_head;
} module_header;

typedef struct module_file {
    module_header header;
    char* name; /* Null Terminated */
    char* description; /* Null Terminated */
    void* instructions; /* Note - for a native module this is a dynamic library handle/pointer */
    void* constant;
    unsigned char fromfile : 1; /* Marks if the module file was loaded from a file (rather than statically linked) */
    unsigned char native : 1;    /* Marks if the module is a native module */
} module_file;

/* Sets Header Version and initialises the header */
static void init_module(module_file *module) {
    memset(module,0,sizeof(module_file)); /* Zero module file (valgrind complains otherwise) */
    memcpy(module->header.FILE_HEADER, BIN_HEADER, sizeof(BIN_HEADER));
    memcpy(module->header.FILE_VERSION, BIN_VERSION, sizeof(BIN_VERSION));
    module->fromfile = 0;
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
/* The module is fromfile (with more than one malloc call) - it must be freed with free_module() */
/* 0 on success,
 * 1 on eof
 * 2 on file version mismatch
 * -1 on error
 * (use perror), on an error you can/should use free_module() */
static int read_module(module_file **module, FILE *inFile) {
    *module = malloc(sizeof(module_file));
    if (*module == 0) return -1;

    /* Zero these so free_module() will not crash after read_module() error */
    (*module)->fromfile = 1;
    (*module)->native = 0;
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

/* "Read" in the module from a memory buffer */
/* The module is not fromfile however it "should" or at least "can" be freed with free_module() */
/* end_of_buffer is the first byte AFTER the buffer - i.e. not part of the buffer */
/* 0 on success,
 * 1 on eof
 * 2 on file version mismatch
 * -1 on error
 * (use perror), on an error you can/should use free_module() */
static int read_module_mem(module_file **module, char** in_buffer, const char *end_of_buffer) {
    *module = 0;
    if ( *in_buffer >= end_of_buffer) return 1; /* "eof" */

    *module = malloc(sizeof(module_file));
    if (*module == 0) return -1;

    /* Zero these so free_module() will not crash after read_module() error */
    (*module)->fromfile = 0;
    (*module)->native = 0;
    (*module)->name = 0;
    (*module)->description = 0;
    (*module)->instructions = 0;
    (*module)->constant = 0;

    if (*in_buffer + sizeof(module_header) > end_of_buffer) {
        *module = 0;
        return -1; /* "error" */
    }

    memcpy(*module, *in_buffer, sizeof(module_header));
    *in_buffer += sizeof(module_header);

    /* Check Header */
    switch (check_header_version(&(*module)->header)) {
        case 1: return -1; /* Unknown error */
        case 2: return 2; /* Version mismatch */
        default:;
    }

    (*module)->name = *in_buffer;
    *in_buffer += (*module)->header.name_size;
    if (*in_buffer > end_of_buffer) return -1; /* "error" */

    (*module)->description = *in_buffer;
    *in_buffer += (*module)->header.description_size;
    if (*in_buffer > end_of_buffer) return -1; /* "error" */

    (*module)->instructions = *in_buffer;
    *in_buffer += (*module)->header.instruction_size * sizeof(bin_code);
    if (*in_buffer > end_of_buffer) return -1; /* "error" */

    (*module)->constant = *in_buffer;
    *in_buffer += (*module)->header.constant_size;
    if (*in_buffer > end_of_buffer) return -1; /* "error" */

    return 0;
}

/* Free the module */
/* Free's the module returned by read_module() or read_module_mem() */
static void free_module(module_file *module) {
    if (module->fromfile || module->native) {
        if (module->name) free(module->name);
        if (module->description) free(module->description);
        if (module->instructions) free(module->instructions);
        if (module->constant) free(module->constant);
    }
    free(module);
}

#endif //CREXX_RXBIN_H
