/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//
// CREXX Binary File Structure and IO
// Created by Adrian Sutherland on 30/05/2022.
//
// Public in-memory model and API for the dedicated rxbin format library.
//

#ifndef CREXX_RXBIN_H
#define CREXX_RXBIN_H

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxinteger.h"
#include "rxdefs.h"
#include "rxgraph.h"

#define BIN_VERSION "007"

#define BIN_HEADER "cReXx" /* Do not change */
#define RXBIN007_MAGIC "cReXx007"
#define RXBIN007_HEADER_SIZE 64u
#define RXBIN007_DIRECTORY_ENTRY_SIZE 40u
#define RXBIN007_SECTION_COUNT 6u
#define RXBIN007_NONE UINT32_MAX

enum rxbin007_section_kind {
    RXBIN007_SECTION_MODULES = 1,
    RXBIN007_SECTION_INSTRUCTIONS = 2,
    RXBIN007_SECTION_CONSTANTS = 3,
    RXBIN007_SECTION_METADATA = 4,
    RXBIN007_SECTION_GRAPH_FACTS = 5,
    RXBIN007_SECTION_GRAPH_INDEXES = 6
};

enum rxbin007_section_flags {
    RXBIN007_SECTION_LZSS = 1u << 0
};

enum rxbin007_feature_flags {
    RXBIN007_FEATURE_FIXED_CALLS = 1u << 0,
    RXBIN007_FEATURE_FROZEN_PARSE = 1u << 1,
    RXBIN007_FEATURE_NATIVE_STEM = 1u << 2,
    RXBIN007_SUPPORTED_FEATURES = RXBIN007_FEATURE_FIXED_CALLS |
                                   RXBIN007_FEATURE_FROZEN_PARSE |
                                   RXBIN007_FEATURE_NATIVE_STEM
};

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
    void *handler; /* Process-local execution images only; never serialized. */
    double fconst;
    rxinteger iconst;
    char cconst;
    size_t index;
} bin_code;
#pragma pack(pop)

/* Preserve the eight-byte VM/RXBIN cell while operand counts become unbounded. */
typedef char rxbin_code_entry_must_remain_eight_bytes[
        sizeof(bin_code) == 8 ? 1 : -1];

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
    STRING_CONST, BINARY_CONST, DECIMAL_CONST, FLOAT_CONST, PROC_CONST, EXPOSE_REG_CONST, EXPOSE_PROC_CONST,
    META_FUNC, META_REG, META_CONST, META_CLEAR,
    META_CLASS, META_ATTR, META_INTERFACE, META_IMPLEMENTS, META_MEMBER, META_INLINE, META_SOURCE_STEP,
    META_TRACE_EVENT
};

/* cREXX chameleon entry in the constant pool
 * A poor C users abstract class!
 * */
typedef struct chameleon_constant {
    size_t size_in_pool; /* including any padding for alignment */
    enum const_pool_type type;
} chameleon_constant;

/* cREXX String entry in the constant pool - this is for STRING_CONST, BINARY_CONST or DECIMAL_CONST */
typedef struct string_constant {
    chameleon_constant base;
    size_t string_len;
#ifndef NUTF8
    size_t string_chars;
#endif
    char string[1]; /* Must be the last member */
} string_constant;

/* cREXX Float entry in the constant pool - this is for FLOAT_CONST */
typedef struct float_constant {
    chameleon_constant base;
    double double_value;
} float_constant;

#define FLOAT_CONST_AT(pool, index) ((float_constant *)((pool) + (index)))
#define FLOAT_CONST_VALUE(pool, index) (FLOAT_CONST_AT((pool), (index))->double_value)

typedef struct stack_frame stack_frame;

/* cREXX Procedure entry in the constant pool */
typedef struct proc_constant {
    chameleon_constant base;
    int next;
    int locals;
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

#define RXBIN_SOURCE_AUTHORED   0x00000001u
#define RXBIN_SOURCE_GENERATED  0x00000002u
#define RXBIN_SOURCE_SYNTHETIC  0x00000004u
#define RXBIN_SOURCE_INLINED    0x00000008u
#define RXBIN_SOURCE_EXACT      0x00000010u
#define RXBIN_SOURCE_INHERITED  0x00000020u
#define RXBIN_SOURCE_COMPOSITE  0x00000040u

typedef struct meta_source_step_constant {
    meta_entry base;
    size_t file;
    size_t source_line;
    uint32_t step_id;
    uint32_t clause_id;
    uint32_t line;
    uint32_t active_start_column;
    uint32_t active_end_column;
    uint32_t flags;
} meta_source_step_constant;

#define RXBIN_TRACE_REF_NONE ((size_t)-1)

#define RXBIN_TRACE_KIND_SOURCE      'S'
#define RXBIN_TRACE_KIND_VARIABLE    'V'
#define RXBIN_TRACE_KIND_ASSIGNMENT  'A'
#define RXBIN_TRACE_KIND_COMPOUND    'C'
#define RXBIN_TRACE_KIND_LITERAL     'L'
#define RXBIN_TRACE_KIND_BINARY_OP   'O'
#define RXBIN_TRACE_KIND_PREFIX_OP   'P'
#define RXBIN_TRACE_KIND_FUNCTION    'F'
#define RXBIN_TRACE_KIND_RESULT      'R'
#define RXBIN_TRACE_KIND_MESSAGE     'M'

#define RXBIN_TRACE_VALUE_NONE       'N'
#define RXBIN_TRACE_VALUE_REGISTER   'R'
#define RXBIN_TRACE_VALUE_CONSTANT   'K'

#define RXBIN_TRACE_MODE_A           0x00000001u
#define RXBIN_TRACE_MODE_R           0x00000002u
#define RXBIN_TRACE_MODE_I           0x00000004u
#define RXBIN_TRACE_MODE_C           0x00000008u
#define RXBIN_TRACE_MODE_E           0x00000010u
#define RXBIN_TRACE_MODE_F           0x00000020u
#define RXBIN_TRACE_MODE_L           0x00000040u
#define RXBIN_TRACE_MODE_ASM         0x00000080u
#define RXBIN_TRACE_MODE_LLM         0x00000100u

typedef struct meta_trace_event_constant {
    meta_entry base;
    uint8_t kind;
    uint8_t value_source;
    uint8_t value_type;
    uint8_t register_type;
    uint32_t mode_mask;
    uint32_t flags;
    size_t value_ref;
    size_t symbol;
    size_t resolved_name;
    uint32_t source_step_id;
    uint32_t clause_id;
} meta_trace_event_constant;

typedef struct meta_func_constant {
    meta_entry base;
    size_t symbol;
    size_t option;
    size_t type;
    size_t func;
    size_t args;
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

typedef struct meta_class_constant {
    meta_entry base;
    size_t symbol;
    size_t option;
    size_t type;
} meta_class_constant;

typedef struct meta_attr_constant {
    meta_entry base;
    size_t symbol;
    size_t option;
    size_t type;
    size_t reg;
} meta_attr_constant;

typedef struct meta_interface_constant {
    meta_entry base;
    size_t symbol;
    size_t option;
    size_t type;
} meta_interface_constant;

typedef struct meta_implements_constant {
    meta_entry base;
    size_t symbol;
    size_t interface_symbol;
} meta_implements_constant;

typedef struct meta_member_constant {
    meta_entry base;
    size_t owner;
    size_t kind;
    size_t member;
    size_t type;
    size_t args;
} meta_member_constant;

typedef struct meta_inline_constant {
    meta_entry base;
    size_t symbol;
    size_t payload;
} meta_inline_constant;

enum rxbin_section_flags {
    RXBIN_SECTION_INST_PACKED = 1u << 0,
    RXBIN_SECTION_CONST_PACKED = 1u << 1
};

enum rxbin_record_type {
    RXBIN_RECORD_MODULE_LOCAL = 1,
    RXBIN_RECORD_POOL_SHARED = 2,
    RXBIN_RECORD_MODULE_SHARED = 3
};

typedef struct module_header {
    char FILE_HEADER[sizeof(BIN_HEADER)];
    char FILE_VERSION[sizeof(BIN_VERSION)];
    unsigned int record_type;
    size_t name_size;  /* number of byte/chars including null terminator */
    size_t description_size;  /* number of byte/chars including null terminator */
    size_t instruction_size;  /* number of expanded 64 bit instructions */
    size_t instruction_stored_size; /* number of bytes stored on disk */
    size_t constant_size; /* number of bytes after decompression */
    size_t constant_stored_size; /* number of bytes stored on disk */
    unsigned int section_flags;
    int globals;
    int proc_head;
    int expose_head;
    int meta_head;
} module_header;

typedef struct rxbin_shared_constant_pool {
    unsigned char *data;
    size_t size;
    size_t stored_size;
    unsigned int section_flags;
    size_t refcount;
} rxbin_shared_constant_pool;

typedef struct module_file {
    module_header header;
    char* name; /* Null Terminated */
    char* description; /* Null Terminated */
    void* instructions; /* Expanded instruction stream in memory */
    void* constant; /* Expanded constant pool in memory */
    RxGraph *semantic_graph; /* Shared immutable semantic graph for this image. */
    uint32_t semantic_module_index; /* Module ordinal used by graph procedure references. */
    rxbin_shared_constant_pool *shared_constant_pool; /* Shared pool backing, if any */
    unsigned char fromfile : 1; /* Marks if the module owns heap allocations that free_module() should release */
    unsigned char native : 1;    /* Marks if the module is a native module */
    unsigned char graph_operands : 1; /* Graph-bearing string operands contain RXBIN 007 graph IDs. */
} module_file;

typedef struct rxbin_reader {
    FILE *file;
    char **buffer_cursor;
    const char *buffer_end;
    unsigned char from_memory : 1;
    rxbin_shared_constant_pool *active_shared_pool;
    void *image_state;
    size_t next_module;
} rxbin_reader;

typedef struct rxbin_byte_buffer {
    unsigned char *data;
    size_t size;
    size_t capacity;
} rxbin_byte_buffer;

typedef struct rxbin_var_writer {
    rxbin_byte_buffer *buffer;
    unsigned char pending_tiny;
    unsigned char have_pending_tiny;
} rxbin_var_writer;

typedef struct rxbin_var_reader {
    const unsigned char *cursor;
    const unsigned char *end;
    uint64_t queued_value;
    unsigned char have_queued_value;
} rxbin_var_reader;

void init_module(module_file *module);
OpFormat rxbin_opcode_format(int opcode);
void rxbin_byte_buffer_init(rxbin_byte_buffer *buffer);
void rxbin_byte_buffer_free(rxbin_byte_buffer *buffer);
int rxbin_byte_buffer_reserve(rxbin_byte_buffer *buffer, size_t extra);
int rxbin_byte_buffer_append_bytes(rxbin_byte_buffer *buffer,
                                   const unsigned char *data,
                                   size_t size);
void rxbin_var_writer_init(rxbin_var_writer *writer, rxbin_byte_buffer *buffer);
int rxbin_var_writer_write(rxbin_var_writer *writer, uint64_t value);
int rxbin_var_writer_flush(rxbin_var_writer *writer);
void rxbin_var_reader_init(rxbin_var_reader *reader,
                           const unsigned char *data,
                           size_t size);
int rxbin_var_reader_read(rxbin_var_reader *reader, uint64_t *value);
int write_module(module_file *module, FILE *outFile);
int write_modules(module_file *const *modules,
                  size_t module_count,
                  RxGraph *semantic_graph,
                  FILE *outFile);
const char *rxbin_last_error(void);
void rxbin_reader_init_file(rxbin_reader *reader, FILE *inFile);
void rxbin_reader_init_mem(rxbin_reader *reader,
                           char **in_buffer,
                           const char *end_of_buffer);
void rxbin_reader_close(rxbin_reader *reader);
int rxbin_reader_next_module(rxbin_reader *reader, module_file **module);
int read_module(module_file **module, FILE *inFile);
int read_module_mem(module_file **module,
                    char **in_buffer,
                    const char *end_of_buffer);
void free_module(module_file *module);

#endif //CREXX_RXBIN_H
