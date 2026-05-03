#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "rxbin.h"
#include "rxdefs.h"

static int read_entire_file(const char *path, char **buffer, size_t *size) {
    FILE *fp;
    long length;

    *buffer = 0;
    *size = 0;

    fp = fopen(path, "rb");
    if (!fp) return 0;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }

    length = ftell(fp);
    if (length < 0) {
        fclose(fp);
        return 0;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    *size = (size_t)length;
    *buffer = malloc(*size);
    if (!*buffer) {
        fclose(fp);
        return 0;
    }

    if (fread(*buffer, 1, *size, fp) != *size) {
        free(*buffer);
        *buffer = 0;
        *size = 0;
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}

int main(void) {
    FILE *fp = 0;
    module_header raw_header;
    module_file *from_file = 0;
    module_file *from_memory = 0;
    char *buffer = 0;
    char *cursor = 0;
    size_t buffer_size = 0;
    bin_code *code = 0;

    fp = fopen("tests_compact_format.rxbin", "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open tests_compact_format.rxbin\n");
        return 1;
    }

    if (fread(&raw_header, sizeof(raw_header), 1, fp) != 1) {
        fprintf(stderr, "Failed to read compact-format module header\n");
        fclose(fp);
        return 1;
    }

    if ((raw_header.section_flags & RXBIN_SECTION_INST_PACKED) == 0) {
        fprintf(stderr, "Instruction section was not written in packed form\n");
        fclose(fp);
        return 1;
    }

    if ((raw_header.section_flags & RXBIN_SECTION_CONST_PACKED) == 0) {
        fprintf(stderr, "Constant pool was not written in compressed form\n");
        fclose(fp);
        return 1;
    }

    if (raw_header.instruction_stored_size >= raw_header.instruction_size * sizeof(bin_code)) {
        fprintf(stderr, "Packed instruction section is not smaller than raw slots\n");
        fclose(fp);
        return 1;
    }

    if (raw_header.constant_stored_size >= raw_header.constant_size) {
        fprintf(stderr, "Compressed constant pool is not smaller than raw bytes\n");
        fclose(fp);
        return 1;
    }

    rewind(fp);
    if (read_module(&from_file, fp) != 0) {
        fprintf(stderr, "Failed to decode tests_compact_format.rxbin from file\n");
        fclose(fp);
        return 1;
    }
    fclose(fp);

    code = (bin_code *)from_file->instructions;
    if (code[0].instruction.opcode != OP_LOAD_REG_INT || code[2].iconst != -1) {
        fprintf(stderr, "First signed integer literal did not decode correctly\n");
        free_module(from_file);
        return 1;
    }

    if (code[3].instruction.opcode != OP_LOAD_REG_INT || code[5].iconst != -2) {
        fprintf(stderr, "Second signed integer literal did not decode correctly\n");
        free_module(from_file);
        return 1;
    }

    if (!read_entire_file("tests_compact_format.rxbin", &buffer, &buffer_size)) {
        fprintf(stderr, "Failed to read tests_compact_format.rxbin into memory\n");
        free_module(from_file);
        return 1;
    }

    cursor = buffer;
    if (read_module_mem(&from_memory, &cursor, buffer + buffer_size) != 0) {
        fprintf(stderr, "Failed to decode tests_compact_format.rxbin from memory\n");
        free(buffer);
        free_module(from_file);
        return 1;
    }

    if (cursor != buffer + buffer_size) {
        fprintf(stderr, "Memory decode did not consume exactly one module image\n");
        free_module(from_memory);
        free(buffer);
        free_module(from_file);
        return 1;
    }

    if (from_file->header.instruction_size != from_memory->header.instruction_size ||
        from_file->header.constant_size != from_memory->header.constant_size ||
        from_file->header.section_flags != from_memory->header.section_flags) {
        fprintf(stderr, "Decoded module headers disagree between file and memory paths\n");
        free_module(from_memory);
        free(buffer);
        free_module(from_file);
        return 1;
    }

    if (memcmp(from_file->instructions, from_memory->instructions,
               from_file->header.instruction_size * sizeof(bin_code)) != 0) {
        fprintf(stderr, "Decoded instruction streams differ between file and memory paths\n");
        free_module(from_memory);
        free(buffer);
        free_module(from_file);
        return 1;
    }

    if (memcmp(from_file->constant, from_memory->constant, from_file->header.constant_size) != 0) {
        fprintf(stderr, "Decoded constant pools differ between file and memory paths\n");
        free_module(from_memory);
        free(buffer);
        free_module(from_file);
        return 1;
    }

    free_module(from_memory);
    free(buffer);
    free_module(from_file);
    return 0;
}
