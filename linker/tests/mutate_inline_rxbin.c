/* Focused RXBIN feature/operational-binding mutations for maintained controls. */
#include "rxbin.h"
#include "rxinline_recipe.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t get_u32(const unsigned char *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8u |
           (uint32_t)p[2] << 16u | (uint32_t)p[3] << 24u;
}

static void put_u32(unsigned char *p, uint32_t value) {
    unsigned int i;
    for (i = 0u; i < 4u; ++i) p[i] = (unsigned char)(value >> (i * 8u));
}

static int remove_feature(const char *input, const char *output) {
    unsigned char buffer[4096];
    FILE *source = fopen(input, "rb");
    FILE *target;
    size_t length;
    uint32_t flags;
    int ok;
    if (!source) return 0;
    length = fread(buffer, 1u, RXBIN007_HEADER_SIZE, source);
    if (length != RXBIN007_HEADER_SIZE || memcmp(buffer, RXBIN007_MAGIC, 8u)) {
        fclose(source); return 0;
    }
    flags = get_u32(buffer + 12u);
    if (!(flags & RXBIN007_FEATURE_INLINE_SCOPES)) { fclose(source); return 0; }
    put_u32(buffer + 12u, flags & ~RXBIN007_FEATURE_INLINE_SCOPES);
    target = fopen(output, "wb");
    if (!target) { fclose(source); return 0; }
    ok = fwrite(buffer, 1u, length, target) == length;
    while (ok && (length = fread(buffer, 1u, sizeof(buffer), source)) != 0u)
        ok = fwrite(buffer, 1u, length, target) == length;
    if (ferror(source)) ok = 0;
    if (fclose(source) != 0) ok = 0;
    if (fclose(target) != 0) ok = 0;
    return ok;
}

static int feature_rejected(const char *path) {
    FILE *source = fopen(path, "rb");
    rxbin_reader reader;
    module_file *module = NULL;
    const char *error;
    int rejected;
    if (!source) return 0;
    rxbin_reader_init_file(&reader, source);
    rejected = rxbin_reader_next_module(&reader, &module) != 0;
    error = rxbin_last_error();
    rejected = rejected && error && strstr(error, "requires feature flag 0x00000100");
    rxbin_reader_close(&reader); fclose(source); free_module(module);
    return rejected;
}

static uint32_t other_procedure(const module_file *module, uint32_t excluded) {
    int offset = module->header.proc_head;
    uint32_t ordinal = 0u;
    while (offset >= 0) {
        const proc_constant *proc = (const proc_constant *)(
            (const unsigned char *)module->constant + (size_t)offset);
        if (ordinal != excluded && proc->start != SIZE_MAX) return ordinal;
        offset = proc->next; ++ordinal;
    }
    return UINT32_MAX;
}

static int mutate(module_file **modules, size_t count, const char *mode) {
    size_t m;
    for (m = 0u; m < count; ++m) {
        module_file *module = modules[m];
        bin_code *code = (bin_code *)module->instructions;
        size_t at = 0u;
        while (at < module->header.instruction_size) {
            int opcode = code[at].instruction.opcode;
            OpFormat format = rxbin_opcode_format(opcode);
            size_t width = rxop_format_operand_count(format), j;
            if (!strcmp(mode, "recipe-mismatch") && opcode == OP_INLENTER_REG_FUNC_STRING_ID) {
                string_constant *binary = (string_constant *)(
                    (unsigned char *)module->constant + code[at + 3u].index);
                rxinl_recipe_view recipe;
                uint32_t replacement;
                if (!rxinl_recipe_decode(binary->string, binary->string_len, &recipe) ||
                    !(recipe.flags & RXINL_RECIPE_SEALED) || recipe.image_module >= count) return 0;
                replacement = other_procedure(modules[recipe.image_module], recipe.provider_procedure);
                return replacement != UINT32_MAX && rxinl_recipe_seal(
                    binary->string, binary->string_len, recipe.image_module, replacement);
            }
            for (j = 0u; j < width; ++j) {
                chameleon_constant *entry;
                string_constant *binary;
                rxinl_qualified_proc_view qualified;
                if (opcode == OP_INLENTER_REG_FUNC_STRING_ID ||
                    rxop_format_operand_type(format, j) != OP_FUNC) continue;
                entry = (chameleon_constant *)((unsigned char *)module->constant + code[at + j + 1u].index);
                if (entry->type != BINARY_CONST) continue;
                binary = (string_constant *)entry;
                if (!rxinl_qualified_proc_decode(binary->string, binary->string_len, &qualified) || !qualified.sealed) return 0;
                if (!strcmp(mode, "qproc-module")) {
                    put_u32((unsigned char *)binary->string + 4u, UINT32_MAX - 1u); return 1;
                }
                if (!strcmp(mode, "qproc-procedure")) {
                    put_u32((unsigned char *)binary->string + 8u, UINT32_MAX - 1u); return 1;
                }
            }
            at += width + 1u;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    module_file *modules[64] = { 0 };
    module_file *module = NULL;
    rxbin_reader reader;
    FILE *source, *target;
    size_t count = 0u, i;
    int ok = 0;
    if (argc != 4) return 2;
    if (!strcmp(argv[1], "feature"))
        return remove_feature(argv[2], argv[3]) && feature_rejected(argv[3]) ? 0 : 1;
    source = fopen(argv[2], "rb");
    if (!source) return 1;
    rxbin_reader_init_file(&reader, source);
    while (count < 64u && rxbin_reader_next_module(&reader, &module) == 0) modules[count++] = module;
    rxbin_reader_close(&reader); fclose(source);
    if (count && count < 64u && mutate(modules, count, argv[1])) {
        target = fopen(argv[3], "wb");
        if (target) {
            ok = write_modules(modules, count, NULL, target) == 0;
            if (fclose(target) != 0) ok = 0;
        }
    }
    if (!ok) fprintf(stderr, "inline mutation failed: %s\n", rxbin_last_error() ? rxbin_last_error() : argv[1]);
    for (i = 0u; i < count; ++i) free_module(modules[i]);
    return ok ? 0 : 1;
}
