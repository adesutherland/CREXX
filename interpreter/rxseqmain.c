/* Offline analyser for dynamic RXSEQ instruction-window profiles. */

#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxbin.h"
#include "rxdefs.h"
#include "rxseqfile.h"

typedef struct rxseq_profile_module {
    size_t id;
    uint64_t hash;
    size_t instruction_size;
    char *name;
    module_file *loaded;
} rxseq_profile_module;

typedef struct rxseq_site {
    size_t module_id;
    size_t start;
    uint64_t count;
} rxseq_site;

typedef struct rxseq_profile {
    unsigned int length;
    int overflowed;
    rxseq_profile_module *modules;
    size_t module_count;
    rxseq_site *sites;
    size_t site_count;
} rxseq_profile;

typedef struct rxseq_loaded_modules {
    module_file **items;
    size_t count;
    size_t capacity;
} rxseq_loaded_modules;

typedef struct rxseq_symbol {
    OperandType type;
    uint64_t value;
    unsigned int ordinal;
    char kind;
} rxseq_symbol;

typedef struct rxseq_candidate {
    char *pattern;
    char *mapping;
    char *example_module;
    size_t example_start;
    unsigned int symbols;
    uint64_t count;
    uint64_t sites;
    uint64_t modules;
    unsigned char *seen_modules;
} rxseq_candidate;

static const OpInfo rxseq_ops[OP_MAX_INSTRUCTIONS] = {
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) \
    [OPCODE] = { #NAME, OPCODE, FMT, FLOW, FLAGS, DESC },
#include "rxops.h"
#undef X
};

static void rxseq_usage(FILE *out) {
    fprintf(out,
            "Usage: rxseq profile.rxseq module.rxbin [module2.rxbin ...] "
            "[--output candidates.csv]\n"
            "\n"
            "Reads a binary RXSEQ profile, verifies the exact modules, normalises register/constant "
            "reuse,\nclusters dynamic instruction sequences, and reports counts.\n");
}

static int rxseq_fail(const char *message, const char *detail) {
    fprintf(stderr, "ERROR: %s%s%s\n", message, detail ? ": " : "",
            detail ? detail : "");
    return 2;
}

static char *rxseq_strdup(const char *text) {
    size_t size = strlen(text) + 1;
    char *copy = malloc(size);
    if (copy) memcpy(copy, text, size);
    return copy;
}

static int rxseq_read_profile(const char *path, rxseq_profile *profile) {
    FILE *file = fopen(path, "rb");
    unsigned char magic[8];
    uint32_t version, header_size, length, result, flags, reserved;
    uint64_t module_count, site_count;
    size_t i;
    if (!file) return rxseq_fail("cannot open RXSEQ profile", path);
    memset(profile, 0, sizeof(*profile));
    if (!rxseq_read_bytes(file, magic, sizeof(magic))) {
        fclose(file);
        return rxseq_fail("truncated RXSEQ header", path);
    }
    if (memcmp(magic, rxseq_file_magic, sizeof(magic)) != 0) {
        fclose(file);
        return rxseq_fail("invalid RXSEQ binary magic", path);
    }
    if (!rxseq_read_u32(file, &version) ||
            !rxseq_read_u32(file, &header_size) ||
            !rxseq_read_u32(file, &length) ||
            !rxseq_read_u32(file, &result) ||
            !rxseq_read_u32(file, &flags) ||
            !rxseq_read_u32(file, &reserved) ||
            !rxseq_read_u64(file, &module_count) ||
            !rxseq_read_u64(file, &site_count)) {
        fclose(file);
        return rxseq_fail("truncated RXSEQ header", path);
    }
    (void)result;
    if (version != RXSEQ_FORMAT_VERSION) {
        fclose(file);
        return rxseq_fail("unsupported RXSEQ binary version", path);
    }
    if (header_size != RXSEQ_HEADER_SIZE || reserved ||
            (flags & ~RXSEQ_FLAG_OVERFLOW) || length < 2 || length > 4) {
        fclose(file);
        return rxseq_fail("invalid RXSEQ binary header", path);
    }
    if (module_count > SIZE_MAX / sizeof(*profile->modules) ||
            site_count > SIZE_MAX / sizeof(*profile->sites)) {
        fclose(file);
        return rxseq_fail("RXSEQ record count is too large", path);
    }
    profile->length = length;
    profile->overflowed = (flags & RXSEQ_FLAG_OVERFLOW) != 0;
    profile->module_count = (size_t)module_count;
    profile->site_count = (size_t)site_count;
    profile->modules = calloc(profile->module_count, sizeof(*profile->modules));
    profile->sites = calloc(profile->site_count, sizeof(*profile->sites));
    if ((!profile->modules && profile->module_count) ||
            (!profile->sites && profile->site_count)) {
        fclose(file);
        return rxseq_fail("out of memory reading RXSEQ records", 0);
    }
    for (i = 0; i < profile->module_count; i++) {
        uint64_t id, hash, instruction_size, name_size;
        rxseq_profile_module *module = &profile->modules[i];
        if (!rxseq_read_varuint(file, &id) ||
                !rxseq_read_u64(file, &hash) ||
                !rxseq_read_varuint(file, &instruction_size) ||
                !rxseq_read_varuint(file, &name_size)) {
            fclose(file);
            return rxseq_fail("truncated RXSEQ module record", path);
        }
        if (id != (uint64_t)i + 1 || instruction_size > SIZE_MAX ||
                name_size > SIZE_MAX - 1 || name_size > UINT64_C(16777216)) {
            fclose(file);
            return rxseq_fail("invalid RXSEQ module record", path);
        }
        module->name = malloc((size_t)name_size + 1);
        if (!module->name) {
            fclose(file);
            return rxseq_fail("out of memory reading RXSEQ module name", 0);
        }
        if (!rxseq_read_bytes(file, module->name, (size_t)name_size)) {
            fclose(file);
            return rxseq_fail("truncated RXSEQ module name", path);
        }
        if (memchr(module->name, 0, (size_t)name_size)) {
            fclose(file);
            return rxseq_fail("invalid RXSEQ module name", path);
        }
        module->name[(size_t)name_size] = 0;
        module->id = (size_t)id;
        module->hash = hash;
        module->instruction_size = (size_t)instruction_size;
    }
    for (i = 0; i < profile->site_count; i++) {
        uint64_t module_id, start, count;
        rxseq_site *site = &profile->sites[i];
        if (!rxseq_read_varuint(file, &module_id) ||
                !rxseq_read_varuint(file, &start) ||
                !rxseq_read_varuint(file, &count)) {
            fclose(file);
            return rxseq_fail("truncated RXSEQ site record", path);
        }
        if (!module_id || module_id > module_count || start > SIZE_MAX ||
                !count ||
                start >= profile->modules[module_id - 1].instruction_size) {
            fclose(file);
            return rxseq_fail("invalid RXSEQ site record", path);
        }
        if (i && (module_id < profile->sites[i - 1].module_id ||
                (module_id == profile->sites[i - 1].module_id &&
                 start <= profile->sites[i - 1].start))) {
            fclose(file);
            return rxseq_fail("unordered or duplicate RXSEQ site record", path);
        }
        site->module_id = (size_t)module_id;
        site->start = (size_t)start;
        site->count = count;
    }
    if (fgetc(file) != EOF || ferror(file)) {
        fclose(file);
        return rxseq_fail("trailing or unreadable RXSEQ data", path);
    }
    if (fclose(file) != 0) {
        return rxseq_fail("failed while closing RXSEQ profile", path);
    }
    return 0;
}

static int rxseq_add_loaded(rxseq_loaded_modules *loaded, module_file *module) {
    if (loaded->count == loaded->capacity) {
        size_t capacity = loaded->capacity ? loaded->capacity * 2 : 16;
        module_file **items = realloc(loaded->items, capacity * sizeof(*items));
        if (!items) return 0;
        loaded->items = items;
        loaded->capacity = capacity;
    }
    loaded->items[loaded->count++] = module;
    return 1;
}

static int rxseq_load_file(const char *path, rxseq_loaded_modules *loaded) {
    FILE *file = fopen(path, "rb");
    int records = 0;
    if (!file) return rxseq_fail("cannot open RXBIN module", path);
    for (;;) {
        module_file *module = 0;
        int result = read_module(&module, file);
        if (result == 1) break;
        if (result != 0 || !module) {
            if (module) free_module(module);
            fclose(file);
            return rxseq_fail("cannot read RXBIN module", path);
        }
        if (!rxseq_add_loaded(loaded, module)) {
            free_module(module);
            fclose(file);
            return rxseq_fail("out of memory loading RXBIN modules", 0);
        }
        records++;
    }
    fclose(file);
    return records ? 0 : rxseq_fail("RXBIN file contains no modules", path);
}

static int rxseq_match_modules(rxseq_profile *profile,
                               const rxseq_loaded_modules *loaded) {
    size_t i, j;
    if (loaded->count != profile->module_count)
        return rxseq_fail("module count differs from profiled run", 0);
    for (i = 0; i < profile->module_count; i++) {
        rxseq_profile_module *expected = &profile->modules[i];
        int name_seen = 0;
        for (j = 0; j < loaded->count; j++) {
            module_file *actual = loaded->items[j];
            uint64_t hash;
            if (strcmp(expected->name, actual->name ? actual->name : "") != 0)
                continue;
            name_seen = 1;
            hash = rxseq_hash_module_file(actual);
            if (hash == expected->hash &&
                    actual->header.instruction_size == expected->instruction_size) {
                expected->loaded = actual;
                break;
            }
        }
        if (!expected->loaded)
            return rxseq_fail(name_seen ? "module content hash mismatch" :
                                      "profiled module is missing",
                              expected->name);
    }
    return 0;
}

static uint64_t rxseq_operand_value(const bin_code *operand, OperandType type) {
    switch (type) {
        case OP_INT: return (uint64_t)operand->iconst;
        case OP_CHAR: return (uint64_t)(unsigned char)operand->cconst;
        default: return (uint64_t)operand->index;
    }
}

static const char *rxseq_operand_type_name(OperandType type) {
    switch (type) {
        case OP_ID: return "label";
        case OP_FUNC: return "func";
        case OP_INT: return "int";
        case OP_FLOAT: return "float";
        case OP_CHAR: return "char";
        case OP_STRING: return "string";
        case OP_DECIMAL: return "decimal";
        case OP_BINARY: return "binary";
        default: return "operand";
    }
}

static unsigned int rxseq_symbol_for(rxseq_symbol *symbols,
                                     unsigned int *symbol_count,
                                     unsigned int *register_count,
                                     unsigned int *constant_count,
                                     OperandType type,
                                     uint64_t value) {
    unsigned int i;
    char kind = type == OP_REG ? 'r' : 'c';
    for (i = 0; i < *symbol_count; i++) {
        if (symbols[i].type == type && symbols[i].value == value)
            return i;
    }
    i = (*symbol_count)++;
    symbols[i].type = type;
    symbols[i].value = value;
    symbols[i].kind = kind;
    symbols[i].ordinal = kind == 'r' ? ++(*register_count) : ++(*constant_count);
    return i;
}

static int rxseq_append(char *buffer, size_t size, size_t *used,
                        const char *format, ...) {
    int written;
    va_list args;
    if (*used >= size) return 0;
    va_start(args, format);
    written = vsnprintf(buffer + *used, size - *used, format, args);
    va_end(args);
    if (written < 0 || (size_t)written >= size - *used) return 0;
    *used += (size_t)written;
    return 1;
}

static int rxseq_describe_site(const rxseq_profile *profile,
                               const rxseq_site *site,
                               char *pattern,
                               size_t pattern_size,
                               char *mapping,
                               size_t mapping_size,
                               unsigned int *symbol_count_out) {
    const rxseq_profile_module *profile_module = &profile->modules[site->module_id - 1];
    const module_file *module = profile_module->loaded;
    const bin_code *binary = (const bin_code *)module->instructions;
    size_t index = site->start;
    size_t pattern_used = 0, mapping_used = 0;
    rxseq_symbol symbols[12];
    unsigned int symbol_count = 0, register_count = 0, constant_count = 0;
    unsigned int instruction_number;
    for (instruction_number = 0; instruction_number < profile->length;
            instruction_number++) {
        int opcode;
        int operand_count;
        OperandType types[3] = {OP_NONE, OP_NONE, OP_NONE};
        int operand;
        const OpInfo *info;
        if (index >= module->header.instruction_size) return 0;
        opcode = binary[index].instruction.opcode;
        operand_count = binary[index].instruction.no_ops;
        if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS ||
                !rxseq_ops[opcode].mnemonic || operand_count < 0 ||
                operand_count > 3 || index + (size_t)operand_count >=
                    module->header.instruction_size)
            return 0;
        info = &rxseq_ops[opcode];
        if (rxbin_get_operand_types(info->format, types) != operand_count)
            return 0;
        if (instruction_number &&
                !rxseq_append(pattern, pattern_size, &pattern_used, " | "))
            return 0;
        if (!rxseq_append(pattern, pattern_size, &pattern_used, "%s(", info->mnemonic))
            return 0;
        for (operand = 0; operand < operand_count; operand++) {
            unsigned int symbol = rxseq_symbol_for(symbols, &symbol_count,
                    &register_count, &constant_count, types[operand],
                    rxseq_operand_value(&binary[index + (size_t)operand + 1],
                                        types[operand]));
            if (operand && !rxseq_append(pattern, pattern_size, &pattern_used, ","))
                return 0;
            if (!rxseq_append(pattern, pattern_size, &pattern_used, "%c%u",
                              symbols[symbol].kind, symbols[symbol].ordinal))
                return 0;
        }
        if (!rxseq_append(pattern, pattern_size, &pattern_used, ")")) return 0;
        index += (size_t)operand_count + 1;
    }
    {
        unsigned int i;
        for (i = 0; i < symbol_count; i++) {
            if (i && !rxseq_append(mapping, mapping_size, &mapping_used, ";"))
                return 0;
            if (symbols[i].kind == 'r') {
                if (!rxseq_append(mapping, mapping_size, &mapping_used,
                        "r%u=R%" PRIu64, symbols[i].ordinal, symbols[i].value))
                    return 0;
            } else {
                if (symbols[i].type == OP_INT) {
                    if (!rxseq_append(mapping, mapping_size, &mapping_used,
                            "c%u=int:%" PRId64, symbols[i].ordinal,
                            (int64_t)symbols[i].value))
                        return 0;
                } else if (!rxseq_append(mapping, mapping_size, &mapping_used,
                        "c%u=%s@%" PRIu64, symbols[i].ordinal,
                        rxseq_operand_type_name(symbols[i].type),
                        symbols[i].value)) {
                    return 0;
                }
            }
        }
    }
    *symbol_count_out = symbol_count;
    return 1;
}

static int rxseq_add_candidate(rxseq_candidate **items,
                               size_t *count,
                               size_t *capacity,
                               const rxseq_profile *profile,
                               const rxseq_site *site,
                               const char *pattern,
                               const char *mapping,
                               unsigned int symbols) {
    size_t i;
    rxseq_candidate *candidate = 0;
    for (i = 0; i < *count; i++) {
        if (strcmp((*items)[i].pattern, pattern) == 0) {
            candidate = &(*items)[i];
            break;
        }
    }
    if (!candidate) {
        if (*count == *capacity) {
            size_t next_capacity = *capacity ? *capacity * 2 : 64;
            rxseq_candidate *next = realloc(*items,
                    next_capacity * sizeof(*next));
            if (!next) return 0;
            *items = next;
            *capacity = next_capacity;
        }
        candidate = &(*items)[(*count)++];
        memset(candidate, 0, sizeof(*candidate));
        candidate->pattern = rxseq_strdup(pattern);
        candidate->mapping = rxseq_strdup(mapping);
        candidate->example_module = rxseq_strdup(
                profile->modules[site->module_id - 1].name);
        candidate->seen_modules = calloc(profile->module_count, 1);
        candidate->example_start = site->start;
        candidate->symbols = symbols;
        if (!candidate->pattern || !candidate->mapping ||
                !candidate->example_module || !candidate->seen_modules)
            return 0;
    }
    if (UINT64_MAX - candidate->count < site->count)
        candidate->count = UINT64_MAX;
    else candidate->count += site->count;
    candidate->sites++;
    if (!candidate->seen_modules[site->module_id - 1]) {
        candidate->seen_modules[site->module_id - 1] = 1;
        candidate->modules++;
    }
    return 1;
}

static int rxseq_candidate_compare(const void *left, const void *right) {
    const rxseq_candidate *a = left;
    const rxseq_candidate *b = right;
    if (a->count < b->count) return 1;
    if (a->count > b->count) return -1;
    return strcmp(a->pattern, b->pattern);
}

static int rxseq_csv_path(const char *path) {
    size_t length;
    if (!path) return 0;
    length = strlen(path);
    return length >= 4 && path[length - 4] == '.' &&
            tolower((unsigned char)path[length - 3]) == 'c' &&
            tolower((unsigned char)path[length - 2]) == 's' &&
            tolower((unsigned char)path[length - 1]) == 'v';
}

static void rxseq_csv_text(FILE *out, const char *text) {
    const char *p;
    fputc('"', out);
    for (p = text; *p; p++) {
        if (*p == '"') fputc('"', out);
        fputc(*p, out);
    }
    fputc('"', out);
}

static void rxseq_write_report(FILE *out,
                               const rxseq_profile *profile,
                               const rxseq_candidate *candidates,
                               size_t count,
                               int csv) {
    size_t i;
    if (csv) {
        fprintf(out, "rank,count,sites,modules,symbols,status,pattern,mapping,example_module,example_start\n");
        for (i = 0; i < count; i++) {
            const rxseq_candidate *candidate = &candidates[i];
            fprintf(out, "%zu,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%u,%s,",
                    i + 1, candidate->count, candidate->sites,
                    candidate->modules, candidate->symbols,
                    candidate->symbols <= 3 ? "candidate" : "over_3_symbols");
            rxseq_csv_text(out, candidate->pattern);
            fputc(',', out);
            rxseq_csv_text(out, candidate->mapping);
            fputc(',', out);
            rxseq_csv_text(out, candidate->example_module);
            fprintf(out, ",%zu\n", candidate->example_start);
        }
    } else {
        fprintf(out, "RXSEQ CANDIDATES length=%u clusters=%zu sites=%zu overflow=%s\n",
                profile->length, count, profile->site_count,
                profile->overflowed ? "yes" : "no");
        fprintf(out, "Count       Sites Modules Args Status          Pattern\n");
        for (i = 0; i < count; i++) {
            const rxseq_candidate *candidate = &candidates[i];
            fprintf(out, "%-11" PRIu64 " %-5" PRIu64 " %-7" PRIu64
                    " %-4u %-15s %s\n",
                    candidate->count, candidate->sites, candidate->modules,
                    candidate->symbols,
                    candidate->symbols <= 3 ? "candidate" : "over_3_symbols",
                    candidate->pattern);
            fprintf(out, "  mapping: %s; example: %s@%zu\n",
                    candidate->mapping, candidate->example_module,
                    candidate->example_start);
        }
    }
}

static void rxseq_free_all(rxseq_profile *profile,
                           rxseq_loaded_modules *loaded,
                           rxseq_candidate *candidates,
                           size_t candidate_count) {
    size_t i;
    for (i = 0; i < candidate_count; i++) {
        free(candidates[i].pattern);
        free(candidates[i].mapping);
        free(candidates[i].example_module);
        free(candidates[i].seen_modules);
    }
    free(candidates);
    for (i = 0; i < loaded->count; i++) free_module(loaded->items[i]);
    free(loaded->items);
    for (i = 0; i < profile->module_count; i++) free(profile->modules[i].name);
    free(profile->modules);
    free(profile->sites);
}

int main(int argc, char **argv) {
    const char *profile_path = 0;
    const char *output_path = 0;
    const char **module_paths;
    size_t module_path_count = 0;
    rxseq_profile profile;
    rxseq_loaded_modules loaded = {0};
    rxseq_candidate *candidates = 0;
    size_t candidate_count = 0, candidate_capacity = 0;
    FILE *out = stdout;
    int rc = 0;
    int i;
    memset(&profile, 0, sizeof(profile));
    module_paths = calloc((size_t)argc, sizeof(*module_paths));
    if (!module_paths) return rxseq_fail("out of memory parsing arguments", 0);
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            rxseq_usage(stdout);
            free(module_paths);
            return 0;
        }
        if (strcmp(argv[i], "--output") == 0) {
            if (++i >= argc || !argv[i][0]) {
                free(module_paths);
                return rxseq_fail("missing filename after --output", 0);
            }
            output_path = argv[i];
        } else if (strncmp(argv[i], "--output=", 9) == 0) {
            if (!argv[i][9]) {
                free(module_paths);
                return rxseq_fail("missing filename after --output=", 0);
            }
            output_path = argv[i] + 9;
        } else if (argv[i][0] == '-') {
            free(module_paths);
            return rxseq_fail("unknown option", argv[i]);
        } else if (!profile_path) {
            profile_path = argv[i];
        } else {
            module_paths[module_path_count++] = argv[i];
        }
    }
    if (!profile_path || !module_path_count) {
        rxseq_usage(stderr);
        free(module_paths);
        return 2;
    }
    rc = rxseq_read_profile(profile_path, &profile);
    for (i = 0; !rc && (size_t)i < module_path_count; i++)
        rc = rxseq_load_file(module_paths[i], &loaded);
    if (!rc) rc = rxseq_match_modules(&profile, &loaded);
    if (!rc) {
        size_t site_index;
        for (site_index = 0; site_index < profile.site_count; site_index++) {
            char pattern[2048] = {0};
            char mapping[2048] = {0};
            unsigned int symbols = 0;
            const rxseq_site *site = &profile.sites[site_index];
            if (!rxseq_describe_site(&profile, site, pattern, sizeof(pattern),
                                     mapping, sizeof(mapping), &symbols)) {
                rc = rxseq_fail("profile site does not decode as a sequential window",
                                profile.modules[site->module_id - 1].name);
                break;
            }
            if (!rxseq_add_candidate(&candidates, &candidate_count,
                    &candidate_capacity, &profile, site, pattern, mapping,
                    symbols)) {
                rc = rxseq_fail("out of memory clustering RXSEQ sites", 0);
                break;
            }
        }
    }
    if (!rc) {
        qsort(candidates, candidate_count, sizeof(*candidates),
              rxseq_candidate_compare);
        if (output_path) {
            out = fopen(output_path, "w");
            if (!out) rc = rxseq_fail("cannot open candidate output", output_path);
        }
        if (!rc) {
            rxseq_write_report(out, &profile, candidates, candidate_count,
                               rxseq_csv_path(output_path));
            if (out != stdout && fclose(out) != 0)
                rc = rxseq_fail("failed while closing candidate output", output_path);
        }
    }
    rxseq_free_all(&profile, &loaded, candidates, candidate_count);
    free(module_paths);
    return rc;
}
