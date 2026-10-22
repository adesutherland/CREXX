/* cREXX License (MIT). Build-time SDK view of the owning native metadata. */
#include "rxdefs.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifndef LL_RXAS_CATALOG_FINGERPRINT
#error LL_RXAS_CATALOG_FINGERPRINT must identify the owning metadata inputs
#endif

static void quoted(FILE *out, const char *text) {
    fputc('\'', out);
    for (; *text; ++text) {
        fputc(*text, out);
        if (*text == '\'') fputc('\'', out);
    }
    fputc('\'', out);
}

static void signature(FILE *out, const RxOpEffects *effect, size_t count,
                      int which) {
    size_t i;
    if (!count) { fputc('-', out); return; }
    for (i = 0; i < count; ++i) {
        int bit = which == 0 ? rxop_effect_reads_operand(effect, i) :
                  which == 1 ? rxop_effect_writes_operand(effect, i) :
                  which == 2 ? rxop_effect_kills_operand(effect, i) :
                               rxop_effect_branch_target_operand(effect, i);
        fputc(bit ? '1' : '0', out);
    }
}

static void components(FILE *out, int opcode, size_t count, int writes) {
    size_t i;
    if (!count) { fputc('-', out); return; }
    for (i = 0; i < count; ++i) {
        if (i) fputc(',', out);
        fprintf(out, "%u", writes ? rxop_component_writes(opcode, i) :
                                   rxop_component_reads(opcode, i));
    }
}

static const char *nonempty(const char *value) {
    return value && *value ? value : "-";
}

typedef struct { char key[256]; int opcode; } Lookup;
typedef struct { const char *name; int field; int numeric; } Query;

static const Query queries[] = {
    {"mnemonic", 1, 0}, {"signature", 2, 0}, {"flow", 3, 1},
    {"classification", 4, 1}, {"implicit", 5, 1}, {"semantics", 6, 1},
    {"barrier", 7, 1}, {"sourcevisible", 8, 1},
    {"signalstate", 15, 1}, {"signalphase", 16, 1},
    {"signalsource", 17, 1}, {"signalsourceoperand", 18, 1},
    {"signalnames", 19, 0}, {"failurecomponents", 21, 1},
    {"failurecontext", 22, 1}, {"signaldependencies", 23, 1},
    {"continuations", 24, 1}, {"signalproperties", 25, 1},
    {"policyeffect", 26, 1}, {"policysource", 27, 1},
    {"policysourceoperand", 28, 1}, {"policyname", 29, 0}
};

static int compare_lookup(const void *a, const void *b) {
    return strcmp(((const Lookup *)a)->key, ((const Lookup *)b)->key);
}

static void hex_file(FILE *out, FILE *input) {
    int byte;
    rewind(input);
    while ((byte = fgetc(input)) != EOF) fprintf(out, "%02X", (unsigned int)byte);
}

static void hex_u32(FILE *out, uint32_t value) {
    unsigned int i;
    for (i = 0; i < 4; ++i) fprintf(out, "%02X", (value >> (i * 8)) & 255u);
}

static int replace_output(const char *temporary, const char *output) {
#ifdef _WIN32
    if (MoveFileExA(temporary, output, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) return 0;
    fprintf(stderr, "catalog replacement failed (Windows error %lu)\n", (unsigned long)GetLastError());
    return -1;
#else
    return rename(temporary, output);
#endif
}

int main(int argc, char **argv) {
    FILE *out, *rows, *keys;
    char *temporary;
    size_t path_size;
    int opcode, write_failed;
    Lookup lookup[OP_MAX_INSTRUCTIONS];
    uint32_t row_start[OP_MAX_INSTRUCTIONS], row_size[OP_MAX_INSTRUCTIONS];
    uint32_t key_start[OP_MAX_INSTRUCTIONS], key_size[OP_MAX_INSTRUCTIONS];
    size_t lookup_count = 0, entry;
    if (argc != 2) {
        fprintf(stderr, "usage: %s OUTPUT\n", argv[0]);
        return 2;
    }
    path_size = strlen(argv[1]) + sizeof(".tmp");
    temporary = malloc(path_size);
    if (!temporary) { fputs("catalog output path allocation failed\n", stderr); return 1; }
    snprintf(temporary, path_size, "%s.tmp", argv[1]);
    out = fopen(temporary, "wb");
    if (!out) { perror(temporary); free(temporary); return 1; }
    rows = tmpfile(); keys = tmpfile();
    if (!rows || !keys) {
        perror("catalog temporary data");
        if (rows) fclose(rows);
        if (keys) fclose(keys);
        fclose(out); remove(temporary); free(temporary); return 1;
    }
    fputs("/* cREXX License (MIT). Generated; native rxop metadata is the owner. */\n"
          "options levelb\nnamespace llrxascatalog expose catalog\nimport rxfnsb\n\n"
          "catalog: class\n  _rows = .binary\n  _rowindex = .binary\n"
          "  _keys = .binary\n  _keyindex = .binary\n  *: factory\n", out);
    for (opcode = 0; opcode < OP_MAX_INSTRUCTIONS; ++opcode) {
        const OpInfo *op = &op_table[opcode];
        RxOpEffects effect = rxop_effects(opcode);
        RxOpSignalContract signal = rxop_signal_contract(opcode);
        size_t count = rxop_format_operand_count(op->format), i;
        char name[128];
        for (i = 0; op->mnemonic[i] && op->mnemonic[i] != '_' &&
                    i + 1 < sizeof(name); ++i)
            name[i] = (char)tolower((unsigned char)op->mnemonic[i]);
        name[i] = 0;
        if (rxop_is_source_mnemonic(op->mnemonic)) {
            snprintf(lookup[lookup_count].key, sizeof(lookup[lookup_count].key),
                     "%s|%s", name, op->format);
            lookup[lookup_count++].opcode = opcode;
        }
        row_start[opcode] = (uint32_t)ftell(rows);
        fprintf(rows, "%s %s %d %d %d %u %d %d ",
                name, nonempty(op->format), (int)effect.flow,
                (int)effect.state, (int)effect.implicit, effect.semantics,
                effect.optimizer_barrier, rxop_is_source_mnemonic(op->mnemonic));
        for (i = 0; i < 4; ++i) { signature(rows, &effect, count, (int)i); fputc(' ', rows); }
        components(rows, opcode, count, 0); fputc(' ', rows);
        components(rows, opcode, count, 1);
        fprintf(rows, " %d %d %d %lld %s ", (int)signal.state,
                (int)signal.phase, (int)signal.source,
                signal.source_operand == SIZE_MAX ? -1LL : (long long)signal.source_operand,
                nonempty(signal.static_names));
        if (!count) fputc('-', rows);
        for (i = 0; i < count; ++i)
            fputc(rxop_signal_failure_writes_operand(&signal, i) ? '1' : '0', rows);
        fprintf(rows, " %u %u %u %u %u %d %d %lld %s %d\n",
                signal.failure_component_writes, signal.failure_context_writes,
                signal.dependencies, signal.continuations, signal.properties,
                (int)signal.policy_effect, (int)signal.policy_source,
                signal.policy_source_operand == SIZE_MAX ? -1LL : (long long)signal.policy_source_operand,
                nonempty(signal.policy_static_name),
                (effect.semantics & (RXOP_SEM_INDIRECT_WRITE | RXOP_SEM_INDIRECT_BRANCH)) != 0 ||
                (signal.properties & RXOP_SIGNAL_PROP_ASYNC_ENTRY) != 0);
        row_size[opcode] = (uint32_t)ftell(rows) - row_start[opcode];
    }
    qsort(lookup, lookup_count, sizeof(*lookup), compare_lookup);
    for (entry = 0; entry < lookup_count; ++entry) {
        key_start[entry] = (uint32_t)ftell(keys);
        fputs(lookup[entry].key, keys);
        key_size[entry] = (uint32_t)ftell(keys) - key_start[entry];
    }
    write_failed = ferror(rows) || ferror(keys);
    fputs("    _rows = '", out); hex_file(out, rows);
    fputs("'x\n    _rowindex = '", out);
    for (opcode = 0; opcode < OP_MAX_INSTRUCTIONS; ++opcode) {
        hex_u32(out, row_start[opcode]); hex_u32(out, row_size[opcode]);
    }
    fputs("'x\n    _keys = '", out); hex_file(out, keys);
    fputs("'x\n    _keyindex = '", out);
    for (entry = 0; entry < lookup_count; ++entry) {
        hex_u32(out, key_start[entry]); hex_u32(out, key_size[entry]);
        hex_u32(out, (uint32_t)lookup[entry].opcode);
    }
    fputs("'x\n", out);
    fprintf(out, "    return\n  count: method = .int\n    return %d\n", OP_MAX_INSTRUCTIONS);
    fputs("  fingerprint: method = .string\n    return ", out); quoted(out, LL_RXAS_CATALOG_FINGERPRINT);
    fputs("\n  version: method = .string\n    return 'llrxas.catalog.v1'\n"
          "  field: method = .string\n    arg opcode = .int, column = .int\n"
          "    if opcode < 0 | opcode >= count() then return ''\n"
          "    start = <at..u32>(opcode * 8) _rowindex\n"
          "    size = <at..u32>(opcode * 8 + 4) _rowindex\n"
          "    row = binsubstr(_rows,start + 1,size) as .string\n    return word(row,column)\n"
          "  find: method = .int\n    arg mnemonic = .string, signature = .string\n"
          "    key = lower(mnemonic) || '|' || signature\n"
          "    low = 0; high = <blen>(_keyindex) <idiv> 12 - 1\n    do while low <= high\n"
          "      middle = low + (high - low) <idiv> 2\n"
          "      start = <at..u32>(middle * 12) _keyindex\n"
          "      size = <at..u32>(middle * 12 + 4) _keyindex\n"
          "      candidate = binsubstr(_keys,start + 1,size) as .string\n"
          "      if candidate = key then return <at..u32>(middle * 12 + 8) _keyindex\n"
          "      if candidate < key then low = middle + 1\n"
          "      else high = middle - 1\n    end\n    return -1\n"
          "  bit: method = .int\n    arg opcode = .int, column = .int, operand = .int\n"
          "    return substr(field(opcode,column),operand,1) = '1'\n"
          "  reads: method = .int\n    arg opcode = .int, operand = .int\n    return bit(opcode,9,operand)\n"
          "  writes: method = .int\n    arg opcode = .int, operand = .int\n    return bit(opcode,10,operand)\n"
          "  kills: method = .int\n    arg opcode = .int, operand = .int\n    return bit(opcode,11,operand)\n"
          "  branch: method = .int\n    arg opcode = .int, operand = .int\n    return bit(opcode,12,operand)\n"
          "  failurewrites: method = .int\n    arg opcode = .int, operand = .int\n    return bit(opcode,20,operand)\n"
          "  unbounded: method = .int\n    arg opcode = .int\n    return field(opcode,30) = '1'\n"
          "  readcomponents: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,13),' ',','),operand) as .int\n"
          "  writecomponents: method = .int\n    arg opcode = .int, operand = .int\n"
          "    return word(translate(field(opcode,14),' ',','),operand) as .int\n", out);
    for (entry = 0; entry < sizeof(queries) / sizeof(*queries); ++entry) {
        const Query *query = &queries[entry];
        fprintf(out, "  %s: method = .%s\n    arg opcode = .int\n", query->name,
                query->numeric ? "int" : "string");
        if (query->numeric) fprintf(out, "    return field(opcode,%d) as .int\n", query->field);
        else fprintf(out, "    value = field(opcode,%d)\n    if value = '-' then return ''\n"
                          "    return value\n", query->field);
    }
    write_failed = write_failed || ferror(out) || ferror(rows) || ferror(keys);
    if (fclose(rows)) write_failed = 1;
    if (fclose(keys)) write_failed = 1;
    if (fclose(out) || write_failed) {
        perror(temporary); remove(temporary); free(temporary); return 1;
    }
    if (replace_output(temporary, argv[1])) {
        perror(argv[1]); remove(temporary); free(temporary); return 1;
    }
    free(temporary);
    return 0;
}
