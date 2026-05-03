#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "platform.h"
#include "rxbin.h"
#include "rxdefs.h"

static uint64_t bits_of(double value) {
    uint64_t bits = 0;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

int main(void) {
    FILE *fp = fopen("tests_float_pool.rxbin", "rb");
    module_file *module = 0;
    chameleon_constant *entry;
    bin_code *code;
    size_t i;
    size_t float_count = 0;
    size_t first_index;
    size_t second_index;
    size_t third_index;

    if (!fp) {
        fprintf(stderr, "Failed to open tests_float_pool.rxbin\n");
        return 1;
    }

    if (read_module(&module, fp) != 0) {
        fclose(fp);
        fprintf(stderr, "Failed to read tests_float_pool.rxbin\n");
        return 1;
    }
    fclose(fp);

    code = (bin_code *)module->instructions;
    if (code[0].instruction.opcode != OP_LOAD_REG_FLOAT ||
        code[3].instruction.opcode != OP_LOAD_REG_FLOAT ||
        code[6].instruction.opcode != OP_LOAD_REG_FLOAT) {
        fprintf(stderr, "Unexpected instruction layout in float pool fixture\n");
        free_module(module);
        return 1;
    }

    first_index = code[2].index;
    second_index = code[5].index;
    third_index = code[8].index;

    if (first_index != second_index) {
        fprintf(stderr, "Repeated float literals were not deduplicated\n");
        free_module(module);
        return 1;
    }

    if (first_index == third_index) {
        fprintf(stderr, "Distinct float literals were incorrectly merged\n");
        free_module(module);
        return 1;
    }

    if (FLOAT_CONST_AT(module->constant, first_index)->base.type != FLOAT_CONST ||
        FLOAT_CONST_AT(module->constant, third_index)->base.type != FLOAT_CONST) {
        fprintf(stderr, "Float operands do not point at FLOAT_CONST entries\n");
        free_module(module);
        return 1;
    }

    if (bits_of(FLOAT_CONST_VALUE(module->constant, first_index)) != bits_of(1.5)) {
        fprintf(stderr, "Stored 1.5 bits do not match the expected IEEE-64 payload\n");
        free_module(module);
        return 1;
    }

    if (bits_of(FLOAT_CONST_VALUE(module->constant, third_index)) != UINT64_C(0x8000000000000000)) {
        fprintf(stderr, "Stored -0.0 bits were not preserved exactly\n");
        free_module(module);
        return 1;
    }

    i = 0;
    while (i < module->header.constant_size) {
        entry = (chameleon_constant *)(module->constant + i);
        if (entry->type == FLOAT_CONST) {
            float_count++;
        }
        i += entry->size_in_pool;
    }

    if (float_count != 2) {
        fprintf(stderr, "Expected 2 FLOAT_CONST entries, found %lu\n", (unsigned long)float_count);
        free_module(module);
        return 1;
    }

    free_module(module);
    return 0;
}
