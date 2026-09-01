/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxbin.h"

static uint32_t read_u32_le(const unsigned char *bytes) {
    return (uint32_t)bytes[0] |
           ((uint32_t)bytes[1] << 8u) |
           ((uint32_t)bytes[2] << 16u) |
           ((uint32_t)bytes[3] << 24u);
}

static uint64_t read_u64_le(const unsigned char *bytes) {
    uint64_t value = 0u;
    unsigned int index;

    for (index = 0u; index < 8u; index++) {
        value |= (uint64_t)bytes[index] << (index * 8u);
    }
    return value;
}

static int patch_serialized_opcode(FILE *output, long surrogate, long opcode) {
    unsigned char header[RXBIN007_HEADER_SIZE];
    unsigned char directory_entry[RXBIN007_DIRECTORY_ENTRY_SIZE];
    unsigned char instruction_prefix[10];
    unsigned char expected[2];
    unsigned char replacement[2];
    uint64_t directory_offset;
    uint64_t instruction_offset;

    if (surrogate < 128 || surrogate > 8191 || opcode < 128 || opcode > 8191 ||
        fflush(output) != 0 || fseek(output, 0, SEEK_SET) != 0 ||
        fread(header, 1u, sizeof(header), output) != sizeof(header) ||
        memcmp(header, RXBIN007_MAGIC, 8u) != 0) return 0;
    directory_offset = read_u64_le(header + 32u);
    if (directory_offset > LONG_MAX ||
        fseek(output,
              (long)directory_offset + RXBIN007_DIRECTORY_ENTRY_SIZE,
              SEEK_SET) != 0 ||
        fread(directory_entry, 1u, sizeof(directory_entry), output) !=
                sizeof(directory_entry) ||
        read_u32_le(directory_entry) != RXBIN007_SECTION_INSTRUCTIONS ||
        read_u32_le(directory_entry + 4u) != 0u) return 0;
    instruction_offset = read_u64_le(directory_entry + 16u);
    if (instruction_offset > LONG_MAX ||
        fseek(output, (long)instruction_offset, SEEK_SET) != 0 ||
        fread(instruction_prefix, 1u, sizeof(instruction_prefix), output) !=
                sizeof(instruction_prefix) ||
        memcmp(instruction_prefix, "RXQ7\1\0\0\0", 8u) != 0) return 0;

    expected[0] = (unsigned char)(0xc0u | (((unsigned long)surrogate >> 8u) & 0x1fu));
    expected[1] = (unsigned char)((unsigned long)surrogate & 0xffu);
    replacement[0] = (unsigned char)(0xc0u | (((unsigned long)opcode >> 8u) & 0x1fu));
    replacement[1] = (unsigned char)((unsigned long)opcode & 0xffu);
    if (memcmp(instruction_prefix + 8u, expected, sizeof(expected)) != 0 ||
        fseek(output, (long)instruction_offset + 8L, SEEK_SET) != 0 ||
        fwrite(replacement, 1u, sizeof(replacement), output) != sizeof(replacement) ||
        fflush(output) != 0) return 0;
    return 1;
}

int main(int argc, char **argv) {
    FILE *input;
    FILE *output;
    module_file *module = NULL;
    bin_code *instructions;
    long opcode;
    int operand_count;
    int surrogate;
    char *end = NULL;
    int rc = 1;

    if (argc != 4) {
        fprintf(stderr, "usage: %s input.rxbin output.rxbin opcode\n", argv[0]);
        return 2;
    }
    opcode = strtol(argv[3], &end, 10);
    if (!end || *end || opcode < 466 || opcode > 471) {
        fprintf(stderr, "retired opcode must be in 466..471\n");
        return 2;
    }
    input = fopen(argv[1], "rb");
    if (!input) {
        perror(argv[1]);
        return 1;
    }
    if (read_module(&module, input) != 0) {
        fprintf(stderr, "cannot read RXBIN module\n");
        fclose(input);
        return 1;
    }
    fclose(input);

    instructions = module ? (bin_code *)module->instructions : NULL;
    if (!instructions || module->header.instruction_size == 0 ||
        instructions[0].instruction.no_ops != 3) {
        fprintf(stderr, "retired-opcode fixture has an unexpected instruction shape\n");
        goto done;
    }
    operand_count = opcode == 466 ? 3 : (opcode == 471 ? 1 : 2);
    surrogate = operand_count == 3
            ? OP_FADD_REG_REG_REG
            : (operand_count == 2 ? OP_APPENDCHAR_REG_REG : OP_STOBIN_REG);
    instructions[0].instruction.opcode = surrogate;
    instructions[0].instruction.no_ops = operand_count;
    instructions[operand_count + 1] = instructions[4];
    module->header.instruction_size = (size_t)operand_count + 2u;

    output = fopen(argv[2], "w+b");
    if (!output) {
        perror(argv[2]);
        goto done;
    }
    if (write_module(module, output) != 0 ||
        !patch_serialized_opcode(output, surrogate, opcode)) {
        fprintf(stderr, "cannot write retired-opcode RXBIN module\n");
        fclose(output);
        goto done;
    }
    if (fclose(output) != 0) goto done;
    rc = 0;

done:
    free_module(module);
    return rc;
}
