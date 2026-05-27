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
// In the style of a single file library with a few simple static functions
//

#ifndef CREXX_RXBIN_H
#define CREXX_RXBIN_H

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "rxdefs.h"

#define BIN_VERSION "004"

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
    STRING_CONST, BINARY_CONST, DECIMAL_CONST, FLOAT_CONST, PROC_CONST, EXPOSE_REG_CONST, EXPOSE_PROC_CONST,
    META_SRC, META_FILE, META_FUNC, META_REG, META_CONST, META_CLEAR,
    META_CLASS, META_ATTR, META_INTERFACE, META_IMPLEMENTS, META_MEMBER, META_INLINE, META_SOURCE_STEP
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
    rxbin_shared_constant_pool *shared_constant_pool; /* Shared pool backing, if any */
    unsigned char fromfile : 1; /* Marks if the module owns heap allocations that free_module() should release */
    unsigned char native : 1;    /* Marks if the module is a native module */
} module_file;

typedef struct rxbin_reader {
    FILE *file;
    char **buffer_cursor;
    const char *buffer_end;
    unsigned char from_memory : 1;
    rxbin_shared_constant_pool *active_shared_pool;
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

#define RXBIN_LZSS_WINDOW 4096u
#define RXBIN_LZSS_HASH_SIZE 8192u
#define RXBIN_LZSS_MIN_MATCH 3u
#define RXBIN_LZSS_MAX_MATCH 18u
#define RXBIN_LZSS_MAX_CHAIN 64u

/* Sets Header Version and initialises the header */
static void init_module(module_file *module) {
    memset(module, 0, sizeof(module_file)); /* Zero module file (valgrind complains otherwise) */
    memcpy(module->header.FILE_HEADER, BIN_HEADER, sizeof(BIN_HEADER));
    memcpy(module->header.FILE_VERSION, BIN_VERSION, sizeof(BIN_VERSION));
    module->header.record_type = RXBIN_RECORD_MODULE_LOCAL;
    module->fromfile = 0;
}

/* Check Header Version */
/* 0 - OK */
/* 1 - Missing Header */
/* 2 - Version Mismatch */
static int check_header_version(module_header *header) {
    if (memcmp(header->FILE_HEADER, BIN_HEADER, sizeof(BIN_HEADER)) != 0) return 1;
    if (memcmp(header->FILE_VERSION, BIN_VERSION, sizeof(BIN_VERSION)) != 0) return 2;
    if (header->section_flags & ~(RXBIN_SECTION_INST_PACKED | RXBIN_SECTION_CONST_PACKED)) return 2;
    if (header->record_type < RXBIN_RECORD_MODULE_LOCAL || header->record_type > RXBIN_RECORD_MODULE_SHARED)
        return 2;
    if (header->record_type == RXBIN_RECORD_POOL_SHARED) {
        if (header->instruction_size || header->instruction_stored_size) return 2;
        if (header->section_flags & RXBIN_SECTION_INST_PACKED) return 2;
    } else if (header->record_type == RXBIN_RECORD_MODULE_SHARED) {
        if (header->constant_size || header->constant_stored_size) return 2;
        if (header->section_flags & RXBIN_SECTION_CONST_PACKED) return 2;
    }
    return 0;
}

static int rxbin_get_operand_types(OpFormat format, OperandType *types) {
    switch (format) {
        case FMT_EMPTY: return 0;
        case FMT_B: types[0] = OP_BINARY; return 1;
        case FMT_C: types[0] = OP_CHAR; return 1;
        case FMT_F: types[0] = OP_FLOAT; return 1;
        case FMT_I: types[0] = OP_INT; return 1;
        case FMT_I_I: types[0] = OP_INT; types[1] = OP_INT; return 2;
        case FMT_I_I_I: types[0] = OP_INT; types[1] = OP_INT; types[2] = OP_INT; return 3;
        case FMT_I_I_R: types[0] = OP_INT; types[1] = OP_INT; types[2] = OP_REG; return 3;
        case FMT_I_R: types[0] = OP_INT; types[1] = OP_REG; return 2;
        case FMT_I_R_R: types[0] = OP_INT; types[1] = OP_REG; types[2] = OP_REG; return 3;
        case FMT_L: types[0] = OP_ID; return 1;
        case FMT_L_L_R: types[0] = OP_ID; types[1] = OP_ID; types[2] = OP_REG; return 3;
        case FMT_L_P_S: types[0] = OP_ID; types[1] = OP_FUNC; types[2] = OP_STRING; return 3;
        case FMT_L_R: types[0] = OP_ID; types[1] = OP_REG; return 2;
        case FMT_L_R_I: types[0] = OP_ID; types[1] = OP_REG; types[2] = OP_INT; return 3;
        case FMT_L_R_R: types[0] = OP_ID; types[1] = OP_REG; types[2] = OP_REG; return 3;
        case FMT_L_R_S: types[0] = OP_ID; types[1] = OP_REG; types[2] = OP_STRING; return 3;
        case FMT_L_S: types[0] = OP_ID; types[1] = OP_STRING; return 2;
        case FMT_P: types[0] = OP_FUNC; return 1;
        case FMT_P_S: types[0] = OP_FUNC; types[1] = OP_STRING; return 2;
        case FMT_R: types[0] = OP_REG; return 1;
        case FMT_R_B: types[0] = OP_REG; types[1] = OP_BINARY; return 2;
        case FMT_R_C: types[0] = OP_REG; types[1] = OP_CHAR; return 2;
        case FMT_R_D: types[0] = OP_REG; types[1] = OP_DECIMAL; return 2;
        case FMT_R_D_R: types[0] = OP_REG; types[1] = OP_DECIMAL; types[2] = OP_REG; return 3;
        case FMT_R_F: types[0] = OP_REG; types[1] = OP_FLOAT; return 2;
        case FMT_R_F_I: types[0] = OP_REG; types[1] = OP_FLOAT; types[2] = OP_INT; return 3;
        case FMT_R_F_R: types[0] = OP_REG; types[1] = OP_FLOAT; types[2] = OP_REG; return 3;
        case FMT_R_I: types[0] = OP_REG; types[1] = OP_INT; return 2;
        case FMT_R_I_I: types[0] = OP_REG; types[1] = OP_INT; types[2] = OP_INT; return 3;
        case FMT_R_I_R: types[0] = OP_REG; types[1] = OP_INT; types[2] = OP_REG; return 3;
        case FMT_R_P: types[0] = OP_REG; types[1] = OP_FUNC; return 2;
        case FMT_R_P_R: types[0] = OP_REG; types[1] = OP_FUNC; types[2] = OP_REG; return 3;
        case FMT_R_R: types[0] = OP_REG; types[1] = OP_REG; return 2;
        case FMT_R_R_D: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_DECIMAL; return 3;
        case FMT_R_R_F: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_FLOAT; return 3;
        case FMT_R_R_I: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_INT; return 3;
        case FMT_R_R_R: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_REG; return 3;
        case FMT_R_R_S: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_STRING; return 3;
        case FMT_R_S: types[0] = OP_REG; types[1] = OP_STRING; return 2;
        case FMT_R_S_I: types[0] = OP_REG; types[1] = OP_STRING; types[2] = OP_INT; return 3;
        case FMT_R_S_R: types[0] = OP_REG; types[1] = OP_STRING; types[2] = OP_REG; return 3;
        case FMT_R_S_S: types[0] = OP_REG; types[1] = OP_STRING; types[2] = OP_STRING; return 3;
        case FMT_S: types[0] = OP_STRING; return 1;
        case FMT_S_R: types[0] = OP_STRING; types[1] = OP_REG; return 2;
        case FMT_S_S: types[0] = OP_STRING; types[1] = OP_STRING; return 2;
        case FMT_S_S_R: types[0] = OP_STRING; types[1] = OP_STRING; types[2] = OP_REG; return 3;
        default: return 0;
    }
}

static OpFormat rxbin_opcode_format(int opcode) {
    static const OpFormat opcode_formats[OP_MAX_INSTRUCTIONS] = {
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) [OPCODE] = FMT,
#include "rxops.h"
#undef X
    };

    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return FMT_EMPTY;
    return opcode_formats[opcode];
}

static void rxbin_byte_buffer_init(rxbin_byte_buffer *buffer) {
    buffer->data = 0;
    buffer->size = 0;
    buffer->capacity = 0;
}

static void rxbin_byte_buffer_free(rxbin_byte_buffer *buffer) {
    if (buffer->data) free(buffer->data);
    buffer->data = 0;
    buffer->size = 0;
    buffer->capacity = 0;
}

static int rxbin_byte_buffer_reserve(rxbin_byte_buffer *buffer, size_t extra) {
    size_t required;
    size_t new_capacity;
    unsigned char *new_data;

    if (extra > SIZE_MAX - buffer->size) return 0;
    required = buffer->size + extra;
    if (required <= buffer->capacity) return 1;

    new_capacity = buffer->capacity ? buffer->capacity : 64;
    while (new_capacity < required) {
        if (new_capacity > SIZE_MAX / 2) {
            new_capacity = required;
            break;
        }
        new_capacity *= 2;
    }

    new_data = realloc(buffer->data, new_capacity);
    if (!new_data) return 0;

    buffer->data = new_data;
    buffer->capacity = new_capacity;
    return 1;
}

static int rxbin_byte_buffer_append_byte(rxbin_byte_buffer *buffer, unsigned char value) {
    if (!rxbin_byte_buffer_reserve(buffer, 1)) return 0;
    buffer->data[buffer->size++] = value;
    return 1;
}

static int rxbin_byte_buffer_append_bytes(rxbin_byte_buffer *buffer, const unsigned char *data, size_t size) {
    if (!size) return 1;
    if (!rxbin_byte_buffer_reserve(buffer, size)) return 0;
    memcpy(buffer->data + buffer->size, data, size);
    buffer->size += size;
    return 1;
}

static int rxbin_append_varuint_direct(rxbin_byte_buffer *buffer, uint64_t value) {
    unsigned char bytes[9];
    size_t count = 0;

    if (value <= UINT64_C(0x7F)) {
        bytes[count++] = (unsigned char)value;
    } else if (value <= UINT64_C(0x1FFF)) {
        bytes[count++] = (unsigned char)(0xC0u | ((value >> 8) & 0x1Fu));
        bytes[count++] = (unsigned char)(value & 0xFFu);
    } else if (value <= UINT64_C(0xFFFFF)) {
        bytes[count++] = (unsigned char)(0xE0u | ((value >> 16) & 0x0Fu));
        bytes[count++] = (unsigned char)((value >> 8) & 0xFFu);
        bytes[count++] = (unsigned char)(value & 0xFFu);
    } else if (value <= UINT64_C(0x7FFFFFF)) {
        bytes[count++] = (unsigned char)(0xF0u | ((value >> 24) & 0x07u));
        bytes[count++] = (unsigned char)((value >> 16) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 8) & 0xFFu);
        bytes[count++] = (unsigned char)(value & 0xFFu);
    } else if (value <= UINT64_C(0x3FFFFFFFF)) {
        bytes[count++] = (unsigned char)(0xF8u | ((value >> 32) & 0x03u));
        bytes[count++] = (unsigned char)((value >> 24) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 16) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 8) & 0xFFu);
        bytes[count++] = (unsigned char)(value & 0xFFu);
    } else if (value <= UINT64_C(0x1FFFFFFFFFF)) {
        bytes[count++] = (unsigned char)(0xFCu | ((value >> 40) & 0x01u));
        bytes[count++] = (unsigned char)((value >> 32) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 24) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 16) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 8) & 0xFFu);
        bytes[count++] = (unsigned char)(value & 0xFFu);
    } else if (value <= UINT64_C(0xFFFFFFFFFFFF)) {
        bytes[count++] = 0xFEu;
        bytes[count++] = (unsigned char)((value >> 40) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 32) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 24) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 16) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 8) & 0xFFu);
        bytes[count++] = (unsigned char)(value & 0xFFu);
    } else {
        bytes[count++] = 0xFFu;
        bytes[count++] = (unsigned char)((value >> 56) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 48) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 40) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 32) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 24) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 16) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 8) & 0xFFu);
        bytes[count++] = (unsigned char)(value & 0xFFu);
    }

    return rxbin_byte_buffer_append_bytes(buffer, bytes, count);
}

static void rxbin_var_writer_init(rxbin_var_writer *writer, rxbin_byte_buffer *buffer) {
    writer->buffer = buffer;
    writer->pending_tiny = 0;
    writer->have_pending_tiny = 0;
}

static int rxbin_var_writer_write(rxbin_var_writer *writer, uint64_t value) {
    if (value <= UINT64_C(7)) {
        if (writer->have_pending_tiny) {
            unsigned char pair_byte = (unsigned char)(0x80u | (writer->pending_tiny << 3) | (unsigned char)value);
            writer->have_pending_tiny = 0;
            return rxbin_byte_buffer_append_byte(writer->buffer, pair_byte);
        }
        writer->pending_tiny = (unsigned char)value;
        writer->have_pending_tiny = 1;
        return 1;
    }

    if (writer->have_pending_tiny) {
        if (!rxbin_append_varuint_direct(writer->buffer, writer->pending_tiny)) return 0;
        writer->have_pending_tiny = 0;
    }

    return rxbin_append_varuint_direct(writer->buffer, value);
}

static int rxbin_var_writer_flush(rxbin_var_writer *writer) {
    if (!writer->have_pending_tiny) return 1;
    writer->have_pending_tiny = 0;
    return rxbin_append_varuint_direct(writer->buffer, writer->pending_tiny);
}

static void rxbin_var_reader_init(rxbin_var_reader *reader, const unsigned char *data, size_t size) {
    reader->cursor = data;
    reader->end = data + size;
    reader->queued_value = 0;
    reader->have_queued_value = 0;
}

static int rxbin_var_reader_read(rxbin_var_reader *reader, uint64_t *value) {
    unsigned char first;

    if (reader->have_queued_value) {
        reader->have_queued_value = 0;
        *value = reader->queued_value;
        return 1;
    }

    if (reader->cursor >= reader->end) return 0;
    first = *(reader->cursor++);

    if ((first & 0x80u) == 0) {
        *value = first;
        return 1;
    }

    if ((first & 0xC0u) == 0x80u) {
        *value = (uint64_t)((first >> 3) & 0x07u);
        reader->queued_value = (uint64_t)(first & 0x07u);
        reader->have_queued_value = 1;
        return 1;
    }

    if ((first & 0xE0u) == 0xC0u) {
        if (reader->end - reader->cursor < 1) return 0;
        *value = ((uint64_t)(first & 0x1Fu) << 8) |
                 (uint64_t)*(reader->cursor++);
        return 1;
    }

    if ((first & 0xF0u) == 0xE0u) {
        if (reader->end - reader->cursor < 2) return 0;
        *value = ((uint64_t)(first & 0x0Fu) << 16) |
                 ((uint64_t)reader->cursor[0] << 8) |
                 (uint64_t)reader->cursor[1];
        reader->cursor += 2;
        return 1;
    }

    if ((first & 0xF8u) == 0xF0u) {
        if (reader->end - reader->cursor < 3) return 0;
        *value = ((uint64_t)(first & 0x07u) << 24) |
                 ((uint64_t)reader->cursor[0] << 16) |
                 ((uint64_t)reader->cursor[1] << 8) |
                 (uint64_t)reader->cursor[2];
        reader->cursor += 3;
        return 1;
    }

    if ((first & 0xFCu) == 0xF8u) {
        if (reader->end - reader->cursor < 4) return 0;
        *value = ((uint64_t)(first & 0x03u) << 32) |
                 ((uint64_t)reader->cursor[0] << 24) |
                 ((uint64_t)reader->cursor[1] << 16) |
                 ((uint64_t)reader->cursor[2] << 8) |
                 (uint64_t)reader->cursor[3];
        reader->cursor += 4;
        return 1;
    }

    if ((first & 0xFEu) == 0xFCu) {
        if (reader->end - reader->cursor < 5) return 0;
        *value = ((uint64_t)(first & 0x01u) << 40) |
                 ((uint64_t)reader->cursor[0] << 32) |
                 ((uint64_t)reader->cursor[1] << 24) |
                 ((uint64_t)reader->cursor[2] << 16) |
                 ((uint64_t)reader->cursor[3] << 8) |
                 (uint64_t)reader->cursor[4];
        reader->cursor += 5;
        return 1;
    }

    if (first == 0xFEu) {
        if (reader->end - reader->cursor < 6) return 0;
        *value = ((uint64_t)reader->cursor[0] << 40) |
                 ((uint64_t)reader->cursor[1] << 32) |
                 ((uint64_t)reader->cursor[2] << 24) |
                 ((uint64_t)reader->cursor[3] << 16) |
                 ((uint64_t)reader->cursor[4] << 8) |
                 (uint64_t)reader->cursor[5];
        reader->cursor += 6;
        return 1;
    }

    if (reader->end - reader->cursor < 8) return 0;
    *value = ((uint64_t)reader->cursor[0] << 56) |
             ((uint64_t)reader->cursor[1] << 48) |
             ((uint64_t)reader->cursor[2] << 40) |
             ((uint64_t)reader->cursor[3] << 32) |
             ((uint64_t)reader->cursor[4] << 24) |
             ((uint64_t)reader->cursor[5] << 16) |
             ((uint64_t)reader->cursor[6] << 8) |
             (uint64_t)reader->cursor[7];
    reader->cursor += 8;
    return 1;
}

static uint64_t rxbin_zigzag_encode(rxinteger value) {
    return (((uint64_t)value) << 1) ^
           (uint64_t)(value >> ((sizeof(rxinteger) * CHAR_BIT) - 1));
}

static rxinteger rxbin_zigzag_decode(uint64_t value) {
    return (rxinteger)((value >> 1) ^ (uint64_t)(-(int64_t)(value & 1u)));
}

static int rxbin_token_from_operand(const bin_code *operand, OperandType type, uint64_t *token) {
    switch (type) {
        case OP_INT:
            *token = rxbin_zigzag_encode(operand->iconst);
            return 1;
        case OP_CHAR:
            *token = (uint64_t)(unsigned char)operand->cconst;
            return 1;
        case OP_ID:
        case OP_REG:
        case OP_FUNC:
        case OP_FLOAT:
        case OP_STRING:
        case OP_DECIMAL:
        case OP_BINARY:
            *token = (uint64_t)operand->index;
            return 1;
        default:
            return 0;
    }
}

static int rxbin_operand_from_token(bin_code *operand, OperandType type, uint64_t token) {
    switch (type) {
        case OP_INT:
            operand->iconst = rxbin_zigzag_decode(token);
            return 1;
        case OP_CHAR:
            if (token > UCHAR_MAX) return 0;
            operand->cconst = (char)(unsigned char)token;
            return 1;
        case OP_ID:
        case OP_REG:
        case OP_FUNC:
        case OP_FLOAT:
        case OP_STRING:
        case OP_DECIMAL:
        case OP_BINARY:
            if (token > (uint64_t)SIZE_MAX) return 0;
            operand->index = (size_t)token;
            return 1;
        default:
            return 0;
    }
}

static int rxbin_encode_instruction_stream(const bin_code *instructions, size_t instruction_size, rxbin_byte_buffer *buffer) {
    rxbin_var_writer writer;
    size_t index = 0;

    rxbin_var_writer_init(&writer, buffer);

    while (index < instruction_size) {
        OperandType types[3];
        OpFormat format;
        int operand_count;
        int opcode = instructions[index].instruction.opcode;
        int operand_index;

        format = rxbin_opcode_format(opcode);
        if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return 0;

        operand_count = rxbin_get_operand_types(format, types);
        if (instructions[index].instruction.no_ops != operand_count) return 0;
        if (index + (size_t)operand_count >= instruction_size) return 0;

        if (!rxbin_var_writer_write(&writer, (uint64_t)opcode)) return 0;

        for (operand_index = 0; operand_index < operand_count; operand_index++) {
            uint64_t token = 0;

            if (!rxbin_token_from_operand(&instructions[index + (size_t)operand_index + 1], types[operand_index], &token))
                return 0;
            if (!rxbin_var_writer_write(&writer, token)) return 0;
        }

        index += (size_t)operand_count + 1;
    }

    return rxbin_var_writer_flush(&writer);
}

static int rxbin_decode_instruction_stream(const unsigned char *encoded, size_t encoded_size, bin_code *instructions,
                                           size_t instruction_size) {
    rxbin_var_reader reader;
    size_t index = 0;

    rxbin_var_reader_init(&reader, encoded, encoded_size);

    while (index < instruction_size) {
        OperandType types[3];
        OpFormat format;
        uint64_t opcode_token = 0;
        int operand_count;
        int operand_index;

        if (!rxbin_var_reader_read(&reader, &opcode_token)) return 0;
        if (opcode_token >= (uint64_t)OP_MAX_INSTRUCTIONS) return 0;

        format = rxbin_opcode_format((int)opcode_token);
        operand_count = rxbin_get_operand_types(format, types);
        if (index + (size_t)operand_count >= instruction_size) return 0;

        instructions[index].instruction.opcode = (int)opcode_token;
        instructions[index].instruction.no_ops = operand_count;

        for (operand_index = 0; operand_index < operand_count; operand_index++) {
            uint64_t token = 0;
            if (!rxbin_var_reader_read(&reader, &token)) return 0;
            if (!rxbin_operand_from_token(&instructions[index + (size_t)operand_index + 1], types[operand_index], token))
                return 0;
        }

        index += (size_t)operand_count + 1;
    }

    return reader.cursor == reader.end && !reader.have_queued_value;
}

static unsigned int rxbin_lzss_hash(const unsigned char *input) {
    return (unsigned int)(((unsigned int)input[0] * 251u +
                           (unsigned int)input[1] * 11u +
                           (unsigned int)input[2]) & (RXBIN_LZSS_HASH_SIZE - 1u));
}

static int rxbin_lzss_match_length(const unsigned char *input, size_t input_size, size_t left, size_t right) {
    size_t max_length = input_size - right;
    size_t length = 0;

    if (max_length > RXBIN_LZSS_MAX_MATCH) max_length = RXBIN_LZSS_MAX_MATCH;
    while (length < max_length && input[left + length] == input[right + length]) {
        length++;
    }
    return (int)length;
}

static void rxbin_lzss_index_position(const unsigned char *input, size_t input_size, size_t position,
                                      size_t *last_positions, size_t *prev_positions) {
    unsigned int hash;

    if (position + RXBIN_LZSS_MIN_MATCH > input_size) {
        prev_positions[position] = SIZE_MAX;
        return;
    }

    hash = rxbin_lzss_hash(input + position);
    prev_positions[position] = last_positions[hash];
    last_positions[hash] = position;
}

static int rxbin_compress_constant_pool(const unsigned char *input, size_t input_size, rxbin_byte_buffer *output) {
    size_t *prev_positions = 0;
    size_t last_positions[RXBIN_LZSS_HASH_SIZE];
    size_t position = 0;
    size_t control_index = 0;
    unsigned char control = 0;
    unsigned int control_bit = 0;
    int group_open = 0;
    unsigned int i;

    if (!input_size) return 1;

    prev_positions = malloc(sizeof(size_t) * input_size);
    if (!prev_positions) return 0;

    for (i = 0; i < RXBIN_LZSS_HASH_SIZE; i++) last_positions[i] = SIZE_MAX;

    while (position < input_size) {
        size_t best_length = 0;
        size_t best_distance = 0;

        if (!group_open) {
            if (!rxbin_byte_buffer_append_byte(output, 0)) {
                free(prev_positions);
                return 0;
            }
            control_index = output->size - 1;
            control = 0;
            control_bit = 0;
            group_open = 1;
        }

        if (position + RXBIN_LZSS_MIN_MATCH <= input_size) {
            unsigned int hash = rxbin_lzss_hash(input + position);
            size_t candidate = last_positions[hash];
            unsigned int chain = 0;

            while (candidate != SIZE_MAX && position > candidate && position - candidate <= RXBIN_LZSS_WINDOW &&
                   chain < RXBIN_LZSS_MAX_CHAIN) {
                size_t candidate_length = (size_t)rxbin_lzss_match_length(input, input_size, candidate, position);
                if (candidate_length >= RXBIN_LZSS_MIN_MATCH && candidate_length > best_length) {
                    best_length = candidate_length;
                    best_distance = position - candidate;
                    if (best_length == RXBIN_LZSS_MAX_MATCH) break;
                }
                candidate = prev_positions[candidate];
                chain++;
            }
        }

        if (best_length >= RXBIN_LZSS_MIN_MATCH) {
            unsigned char token_a = (unsigned char)(((best_length - RXBIN_LZSS_MIN_MATCH) << 4) |
                                                    (((best_distance - 1) >> 8) & 0x0Fu));
            unsigned char token_b = (unsigned char)((best_distance - 1) & 0xFFu);
            size_t offset;

            control |= (unsigned char)(1u << control_bit);
            if (!rxbin_byte_buffer_append_byte(output, token_a) ||
                !rxbin_byte_buffer_append_byte(output, token_b)) {
                free(prev_positions);
                return 0;
            }

            for (offset = 0; offset < best_length; offset++) {
                rxbin_lzss_index_position(input, input_size, position + offset, last_positions, prev_positions);
            }
            position += best_length;
        } else {
            if (!rxbin_byte_buffer_append_byte(output, input[position])) {
                free(prev_positions);
                return 0;
            }
            rxbin_lzss_index_position(input, input_size, position, last_positions, prev_positions);
            position++;
        }

        control_bit++;
        if (control_bit == 8) {
            output->data[control_index] = control;
            group_open = 0;
        }
    }

    if (group_open) output->data[control_index] = control;

    free(prev_positions);
    return 1;
}

static int rxbin_decompress_constant_pool(const unsigned char *input, size_t input_size, unsigned char *output,
                                          size_t output_size) {
    size_t in_pos = 0;
    size_t out_pos = 0;

    while (out_pos < output_size) {
        unsigned char control;
        unsigned int bit;

        if (in_pos >= input_size) return 0;
        control = input[in_pos++];

        for (bit = 0; bit < 8 && out_pos < output_size; bit++) {
            if (control & (unsigned char)(1u << bit)) {
                size_t length;
                size_t distance;
                size_t copied;

                if (input_size - in_pos < 2) return 0;
                length = (size_t)((input[in_pos] >> 4) + RXBIN_LZSS_MIN_MATCH);
                distance = (size_t)((((size_t)input[in_pos] & 0x0Fu) << 8) | input[in_pos + 1]) + 1;
                in_pos += 2;

                if (!distance || distance > out_pos) return 0;
                if (length > output_size - out_pos) return 0;

                for (copied = 0; copied < length; copied++) {
                    output[out_pos] = output[out_pos - distance];
                    out_pos++;
                }
            } else {
                if (in_pos >= input_size) return 0;
                output[out_pos++] = input[in_pos++];
            }
        }
    }

    return in_pos == input_size;
}

static int rxbin_duplicate_block(void **out, const void *input, size_t size) {
    void *copy;

    *out = 0;
    if (!size) return 1;

    copy = malloc(size);
    if (!copy) return 0;
    memcpy(copy, input, size);
    *out = copy;
    return 1;
}

static int rxbin_read_file_block(FILE *file, void **out, size_t size) {
    void *buffer;

    *out = 0;
    if (!size) return 1;

    buffer = malloc(size);
    if (!buffer) return 0;
    if (fread(buffer, 1, size, file) != size) {
        free(buffer);
        return 0;
    }

    *out = buffer;
    return 1;
}

static void rxbin_shared_pool_retain(rxbin_shared_constant_pool *pool) {
    if (!pool) return;
    pool->refcount++;
}

static void rxbin_shared_pool_release(rxbin_shared_constant_pool **pool_ref) {
    rxbin_shared_constant_pool *pool;

    if (!pool_ref || !*pool_ref) return;
    pool = *pool_ref;
    if (pool->refcount) pool->refcount--;
    if (!pool->refcount) {
        if (pool->data) free(pool->data);
        free(pool);
    }
    *pool_ref = 0;
}

static int rxbin_decode_instruction_section(module_file *module, const unsigned char *stored_data) {
    size_t expanded_size = module->header.instruction_size * sizeof(bin_code);

    module->instructions = 0;

    if (!module->header.instruction_size) return 1;

    module->instructions = malloc(expanded_size);
    if (!module->instructions) return 0;

    if (module->header.section_flags & RXBIN_SECTION_INST_PACKED) {
        if (!rxbin_decode_instruction_stream(stored_data, module->header.instruction_stored_size,
                                             module->instructions, module->header.instruction_size))
            return 0;
        return 1;
    }

    if (module->header.instruction_stored_size != expanded_size) return 0;
    memcpy(module->instructions, stored_data, expanded_size);
    return 1;
}

static int rxbin_decode_constant_section(module_file *module, const unsigned char *stored_data) {
    module->constant = 0;

    if (!module->header.constant_size) return 1;

    module->constant = malloc(module->header.constant_size);
    if (!module->constant) return 0;

    if (module->header.section_flags & RXBIN_SECTION_CONST_PACKED) {
        return rxbin_decompress_constant_pool(stored_data, module->header.constant_stored_size,
                                              module->constant, module->header.constant_size);
    }

    if (module->header.constant_stored_size != module->header.constant_size) return 0;
    memcpy(module->constant, stored_data, module->header.constant_size);
    return 1;
}

static int rxbin_decode_shared_constant_pool(const module_header *header, const unsigned char *stored_data,
                                             rxbin_shared_constant_pool **pool_out) {
    rxbin_shared_constant_pool *pool;

    *pool_out = 0;

    pool = calloc(1, sizeof(rxbin_shared_constant_pool));
    if (!pool) return 0;

    pool->size = header->constant_size;
    pool->stored_size = header->constant_stored_size;
    pool->section_flags = header->section_flags & RXBIN_SECTION_CONST_PACKED;
    pool->refcount = 1;

    if (header->constant_size) {
        pool->data = malloc(header->constant_size);
        if (!pool->data) {
            free(pool);
            return 0;
        }

        if (header->section_flags & RXBIN_SECTION_CONST_PACKED) {
            if (!rxbin_decompress_constant_pool(stored_data, header->constant_stored_size,
                                                pool->data, header->constant_size)) {
                free(pool->data);
                free(pool);
                return 0;
            }
        } else {
            if (header->constant_stored_size != header->constant_size) {
                free(pool->data);
                free(pool);
                return 0;
            }
            memcpy(pool->data, stored_data, header->constant_size);
        }
    }

    *pool_out = pool;
    return 1;
}

static int rxbin_prepare_header_for_write(module_file *module, rxbin_byte_buffer *instruction_section,
                                          rxbin_byte_buffer *constant_section) {
    rxbin_byte_buffer packed_instructions;
    rxbin_byte_buffer packed_constants;
    size_t raw_instruction_bytes = module->header.instruction_size * sizeof(bin_code);
    unsigned int section_flags = 0;

    rxbin_byte_buffer_init(&packed_instructions);
    rxbin_byte_buffer_init(&packed_constants);

    if (module->header.record_type != RXBIN_RECORD_POOL_SHARED &&
        module->header.instruction_size &&
        rxbin_encode_instruction_stream((const bin_code *)module->instructions, module->header.instruction_size,
                                        &packed_instructions) &&
        packed_instructions.size < raw_instruction_bytes) {
        section_flags |= RXBIN_SECTION_INST_PACKED;
        *instruction_section = packed_instructions;
    } else {
        rxbin_byte_buffer_free(&packed_instructions);
        if (module->header.record_type != RXBIN_RECORD_POOL_SHARED &&
            !rxbin_byte_buffer_append_bytes(instruction_section, module->instructions, raw_instruction_bytes)) {
            rxbin_byte_buffer_free(&packed_constants);
            return 0;
        }
    }

    if (module->header.record_type != RXBIN_RECORD_MODULE_SHARED &&
        module->header.constant_size &&
        rxbin_compress_constant_pool((const unsigned char *)module->constant, module->header.constant_size,
                                     &packed_constants) &&
        packed_constants.size < module->header.constant_size) {
        section_flags |= RXBIN_SECTION_CONST_PACKED;
        *constant_section = packed_constants;
    } else {
        rxbin_byte_buffer_free(&packed_constants);
        if (module->header.record_type != RXBIN_RECORD_MODULE_SHARED &&
            !rxbin_byte_buffer_append_bytes(constant_section, module->constant, module->header.constant_size)) {
            return 0;
        }
    }

    module->header.section_flags = section_flags;
    module->header.instruction_stored_size = instruction_section->size;
    module->header.constant_stored_size = constant_section->size;
    if (module->header.record_type == RXBIN_RECORD_POOL_SHARED) {
        module->header.instruction_size = 0;
        module->header.instruction_stored_size = 0;
    }
    if (module->header.record_type == RXBIN_RECORD_MODULE_SHARED) {
        module->header.constant_stored_size = 0;
    }
    return 1;
}

/* Write out the module */
/* 0 on success, 1 on error (use perror) */
static int write_module(module_file *module, FILE *outFile) {
    rxbin_byte_buffer instruction_section;
    rxbin_byte_buffer constant_section;
    int rc = 1;

    rxbin_byte_buffer_init(&instruction_section);
    rxbin_byte_buffer_init(&constant_section);

    if (!rxbin_prepare_header_for_write(module, &instruction_section, &constant_section)) goto done;

    if (fwrite(&(module->header), sizeof(module->header), 1, outFile) != 1) goto done;

    if (fwrite(module->name, 1, module->header.name_size, outFile) != module->header.name_size) goto done;

    if (fwrite(module->description, 1, module->header.description_size, outFile) != module->header.description_size)
        goto done;

    if (instruction_section.size &&
        fwrite(instruction_section.data, 1, instruction_section.size, outFile) != instruction_section.size)
        goto done;

    if (constant_section.size &&
        fwrite(constant_section.data, 1, constant_section.size, outFile) != constant_section.size)
        goto done;

    rc = 0;

done:
    rxbin_byte_buffer_free(&instruction_section);
    rxbin_byte_buffer_free(&constant_section);
    return rc;
}

static void free_module(module_file *module);

static void rxbin_reader_init_file(rxbin_reader *reader, FILE *inFile) {
    memset(reader, 0, sizeof(rxbin_reader));
    reader->file = inFile;
}

static void rxbin_reader_init_mem(rxbin_reader *reader, char **in_buffer, const char *end_of_buffer) {
    memset(reader, 0, sizeof(rxbin_reader));
    reader->from_memory = 1;
    reader->buffer_cursor = in_buffer;
    reader->buffer_end = end_of_buffer;
}

static void rxbin_reader_close(rxbin_reader *reader) {
    if (!reader) return;
    if (reader->active_shared_pool) rxbin_shared_pool_release(&reader->active_shared_pool);
    reader->file = 0;
    reader->buffer_cursor = 0;
    reader->buffer_end = 0;
    reader->from_memory = 0;
}

static int rxbin_reader_read_header(rxbin_reader *reader, module_header *header) {
    if (reader->from_memory) {
        if (*reader->buffer_cursor >= reader->buffer_end) return 1;
        if ((size_t)(reader->buffer_end - *reader->buffer_cursor) < sizeof(module_header)) return -1;
        memcpy(header, *reader->buffer_cursor, sizeof(module_header));
        *reader->buffer_cursor += sizeof(module_header);
        return 0;
    }

    if (fread(header, sizeof(module_header), 1, reader->file) != 1) {
        if (feof(reader->file)) return 1;
        return -1;
    }
    return 0;
}

static int rxbin_reader_take_block(rxbin_reader *reader, void **out, size_t size) {
    if (reader->from_memory) {
        *out = 0;
        if (!size) return 1;
        if ((size_t)(reader->buffer_end - *reader->buffer_cursor) < size) return 0;
        if (!rxbin_duplicate_block(out, *reader->buffer_cursor, size)) return 0;
        *reader->buffer_cursor += size;
        return 1;
    }
    return rxbin_read_file_block(reader->file, out, size);
}

static int rxbin_reader_view_block(rxbin_reader *reader, const unsigned char **out, size_t size) {
    *out = 0;
    if (!size) return 1;

    if (reader->from_memory) {
        if ((size_t)(reader->buffer_end - *reader->buffer_cursor) < size) return 0;
        *out = (const unsigned char *)*reader->buffer_cursor;
        *reader->buffer_cursor += size;
        return 1;
    }

    return rxbin_read_file_block(reader->file, (void **)out, size);
}

static int rxbin_reader_next_module(rxbin_reader *reader, module_file **module) {
    module_header header;
    const unsigned char *stored_instructions;
    const unsigned char *stored_constants;
    void *owned_name;
    void *owned_description;
    void *owned_stored_instructions;
    void *owned_stored_constants;
    rxbin_shared_constant_pool *shared_pool;
    int rc;

    *module = 0;

    while (1) {
        stored_instructions = 0;
        stored_constants = 0;
        owned_name = 0;
        owned_description = 0;
        owned_stored_instructions = 0;
        owned_stored_constants = 0;
        shared_pool = 0;

        rc = rxbin_reader_read_header(reader, &header);
        if (rc != 0) return rc;

        switch (check_header_version(&header)) {
            case 1:
                return -1;
            case 2:
                return 2;
            default:;
        }

        if (!rxbin_reader_take_block(reader, &owned_name, header.name_size)) goto error;
        if (!rxbin_reader_take_block(reader, &owned_description, header.description_size)) goto error;
        if (!rxbin_reader_view_block(reader, &stored_instructions, header.instruction_stored_size)) goto error;
        if (!reader->from_memory) owned_stored_instructions = (void *)stored_instructions;
        if (!rxbin_reader_view_block(reader, &stored_constants, header.constant_stored_size)) goto error;
        if (!reader->from_memory) owned_stored_constants = (void *)stored_constants;

        if (header.record_type == RXBIN_RECORD_POOL_SHARED) {
            if (!rxbin_decode_shared_constant_pool(&header, stored_constants, &shared_pool)) goto error;
            if (owned_name) free(owned_name);
            if (owned_description) free(owned_description);
            if (owned_stored_instructions) free(owned_stored_instructions);
            if (owned_stored_constants) free(owned_stored_constants);
            if (reader->active_shared_pool) rxbin_shared_pool_release(&reader->active_shared_pool);
            reader->active_shared_pool = shared_pool;
            continue;
        }

        *module = malloc(sizeof(module_file));
        if (!*module) goto error;

        init_module(*module);
        (*module)->header = header;
        (*module)->fromfile = 1;
        (*module)->native = 0;
        (*module)->name = owned_name;
        (*module)->description = owned_description;
        (*module)->instructions = 0;
        (*module)->constant = 0;
        (*module)->shared_constant_pool = 0;
        owned_name = 0;
        owned_description = 0;

        if (!rxbin_decode_instruction_section(*module, stored_instructions)) goto error;

        if (header.record_type == RXBIN_RECORD_MODULE_LOCAL) {
            if (!rxbin_decode_constant_section(*module, stored_constants)) goto error;
        } else {
            if (!reader->active_shared_pool) goto error;
            rxbin_shared_pool_retain(reader->active_shared_pool);
            (*module)->shared_constant_pool = reader->active_shared_pool;
            (*module)->constant = reader->active_shared_pool->data;
            (*module)->header.constant_size = reader->active_shared_pool->size;
            (*module)->header.constant_stored_size = reader->active_shared_pool->stored_size;
        }

        if (owned_stored_instructions) free(owned_stored_instructions);
        if (owned_stored_constants) free(owned_stored_constants);
        return 0;

error:
        if (owned_name) free(owned_name);
        if (owned_description) free(owned_description);
        if (owned_stored_instructions) free(owned_stored_instructions);
        if (owned_stored_constants) free(owned_stored_constants);
        if (shared_pool) rxbin_shared_pool_release(&shared_pool);
        if (*module) {
            free_module(*module);
            *module = 0;
        }
        return -1;
    }
}

typedef struct rxbin_file_reader_state {
    FILE *file;
    rxbin_reader reader;
    struct rxbin_file_reader_state *next;
} rxbin_file_reader_state;

typedef struct rxbin_mem_reader_state {
    char **cursor_ref;
    rxbin_reader reader;
    struct rxbin_mem_reader_state *next;
} rxbin_mem_reader_state;

static rxbin_file_reader_state *rxbin_file_reader_states = 0;
static rxbin_mem_reader_state *rxbin_mem_reader_states = 0;

static void rxbin_close_file_reader(FILE *inFile) {
    rxbin_file_reader_state **node = &rxbin_file_reader_states;

    while (*node) {
        if ((*node)->file == inFile) {
            rxbin_file_reader_state *current = *node;
            *node = current->next;
            rxbin_reader_close(&current->reader);
            free(current);
            return;
        }
        node = &(*node)->next;
    }
}

static void rxbin_close_mem_reader(char **in_buffer) {
    rxbin_mem_reader_state **node = &rxbin_mem_reader_states;

    while (*node) {
        if ((*node)->cursor_ref == in_buffer) {
            rxbin_mem_reader_state *current = *node;
            *node = current->next;
            rxbin_reader_close(&current->reader);
            free(current);
            return;
        }
        node = &(*node)->next;
    }
}

/* Read in the module */
/* The module is heap backed and must be freed with free_module() */
/* 0 on success,
 * 1 on eof
 * 2 on file version mismatch
 * -1 on error
 * (use perror), on an error you can/should use free_module() */
static int read_module(module_file **module, FILE *inFile) {
    rxbin_file_reader_state *node = rxbin_file_reader_states;
    int rc;

    while (node && node->file != inFile) node = node->next;
    if (!node) {
        node = calloc(1, sizeof(rxbin_file_reader_state));
        if (!node) return -1;
        node->file = inFile;
        rxbin_reader_init_file(&node->reader, inFile);
        node->next = rxbin_file_reader_states;
        rxbin_file_reader_states = node;
    }

    rc = rxbin_reader_next_module(&node->reader, module);
    if (rc != 0) rxbin_close_file_reader(inFile);
    return rc;
}

/* "Read" in the module from a memory buffer */
/* The module is heap backed and can be freed with free_module() */
/* end_of_buffer is the first byte AFTER the buffer - i.e. not part of the buffer */
/* 0 on success,
 * 1 on eof
 * 2 on file version mismatch
 * -1 on error
 * (use perror), on an error you can/should use free_module() */
static int read_module_mem(module_file **module, char **in_buffer, const char *end_of_buffer) {
    rxbin_mem_reader_state *node = rxbin_mem_reader_states;
    int rc;

    while (node && node->cursor_ref != in_buffer) node = node->next;
    if (!node) {
        node = calloc(1, sizeof(rxbin_mem_reader_state));
        if (!node) return -1;
        node->cursor_ref = in_buffer;
        rxbin_reader_init_mem(&node->reader, in_buffer, end_of_buffer);
        node->next = rxbin_mem_reader_states;
        rxbin_mem_reader_states = node;
    }

    rc = rxbin_reader_next_module(&node->reader, module);
    if (rc != 0) rxbin_close_mem_reader(in_buffer);
    return rc;
}

/* Free the module */
/* Free's the module returned by read_module() or read_module_mem() */
static void free_module(module_file *module) {
    if (!module) return;
    if (module->shared_constant_pool) {
        rxbin_shared_pool_release(&module->shared_constant_pool);
        module->constant = 0;
    }
    if (module->fromfile || module->native) {
        if (module->name) free(module->name);
        if (module->description) free(module->description);
        if (module->instructions) free(module->instructions);
        if (module->constant) free(module->constant);
    }
    free(module);
}

#endif //CREXX_RXBIN_H
