#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "rxbin.h"
#include "rxdefs.h"

static uint32_t read_u32le(const unsigned char *bytes) {
    return (uint32_t)bytes[0] |
           ((uint32_t)bytes[1] << 8u) |
           ((uint32_t)bytes[2] << 16u) |
           ((uint32_t)bytes[3] << 24u);
}

static uint64_t read_u64le(const unsigned char *bytes) {
    uint64_t value = 0u;
    unsigned int i;
    for (i = 0u; i < 8u; i++) value |= (uint64_t)bytes[i] << (i * 8u);
    return value;
}

static void write_u32le(unsigned char *bytes, uint32_t value) {
    bytes[0] = (unsigned char)value;
    bytes[1] = (unsigned char)(value >> 8u);
    bytes[2] = (unsigned char)(value >> 16u);
    bytes[3] = (unsigned char)(value >> 24u);
}

static void write_u64le(unsigned char *bytes, uint64_t value) {
    unsigned int i;
    for (i = 0u; i < 8u; i++) bytes[i] = (unsigned char)(value >> (i * 8u));
}

static int decompress_lzss(const unsigned char *input,
                           size_t input_size,
                           unsigned char *output,
                           size_t output_size) {
    size_t input_position = 0u;
    size_t output_position = 0u;

    while (output_position < output_size) {
        unsigned char control;
        unsigned int bit;

        if (input_position >= input_size) return 0;
        control = input[input_position++];
        for (bit = 0u; bit < 8u && output_position < output_size; bit++) {
            if (control & (unsigned char)(1u << bit)) {
                size_t length;
                size_t distance;
                size_t copied;

                if (input_size - input_position < 2u) return 0;
                length = (size_t)((input[input_position] >> 4u) + 3u);
                distance = (size_t)((((size_t)input[input_position] & 0x0fu) << 8u) |
                                    input[input_position + 1u]) + 1u;
                input_position += 2u;
                if (!distance || distance > output_position ||
                    length > output_size - output_position) return 0;
                for (copied = 0u; copied < length; copied++) {
                    output[output_position] = output[output_position - distance];
                    output_position++;
                }
            } else {
                if (input_position >= input_size) return 0;
                output[output_position++] = input[input_position++];
            }
        }
    }
    return input_position == input_size;
}

static int expand_sections(const char *input,
                           size_t input_size,
                           uint32_t expand_mask,
                           char **output,
                           size_t *output_size) {
    size_t next_offset;
    uint32_t section;

    *output = 0;
    *output_size = 0u;
    if (!input || input_size < RXBIN007_HEADER_SIZE +
                              RXBIN007_SECTION_COUNT * RXBIN007_DIRECTORY_ENTRY_SIZE) return 0;
    next_offset = RXBIN007_HEADER_SIZE +
                  RXBIN007_SECTION_COUNT * RXBIN007_DIRECTORY_ENTRY_SIZE;
    for (section = 0u; section < RXBIN007_SECTION_COUNT; section++) {
        const unsigned char *row;
        uint64_t section_size;

        row = (const unsigned char *)input + RXBIN007_HEADER_SIZE +
              (size_t)section * RXBIN007_DIRECTORY_ENTRY_SIZE;
        section_size = read_u64le(row +
                ((expand_mask & (1u << section)) ? 32u : 24u));
        if (next_offset > SIZE_MAX - 7u) return 0;
        next_offset = (next_offset + 7u) & ~(size_t)7u;
        if (section_size > SIZE_MAX || (size_t)section_size > SIZE_MAX - next_offset) return 0;
        next_offset += (size_t)section_size;
    }
    *output = (char *)calloc(1u, next_offset);
    if (!*output) return 0;
    *output_size = next_offset;
    memcpy(*output,
           input,
           RXBIN007_HEADER_SIZE +
           RXBIN007_SECTION_COUNT * RXBIN007_DIRECTORY_ENTRY_SIZE);
    write_u64le((unsigned char *)*output + 16u, next_offset);
    next_offset = RXBIN007_HEADER_SIZE +
                  RXBIN007_SECTION_COUNT * RXBIN007_DIRECTORY_ENTRY_SIZE;
    for (section = 0u; section < RXBIN007_SECTION_COUNT; section++) {
        const unsigned char *input_row;
        unsigned char *output_row;
        uint32_t flags;
        uint64_t input_offset;
        uint64_t stored_size;
        uint64_t expanded_size;
        int expand;

        input_row = (const unsigned char *)input + RXBIN007_HEADER_SIZE +
                    (size_t)section * RXBIN007_DIRECTORY_ENTRY_SIZE;
        output_row = (unsigned char *)*output + RXBIN007_HEADER_SIZE +
                     (size_t)section * RXBIN007_DIRECTORY_ENTRY_SIZE;
        flags = read_u32le(input_row + 4u);
        input_offset = read_u64le(input_row + 16u);
        stored_size = read_u64le(input_row + 24u);
        expanded_size = read_u64le(input_row + 32u);
        expand = flags == RXBIN007_SECTION_LZSS &&
                 (expand_mask & (1u << section));
        next_offset = (next_offset + 7u) & ~(size_t)7u;
        if (input_offset > input_size || stored_size > input_size - (size_t)input_offset ||
            (flags != 0u && flags != RXBIN007_SECTION_LZSS)) goto error;
        if (expand) {
            if (!decompress_lzss((const unsigned char *)input + (size_t)input_offset,
                                 (size_t)stored_size,
                                 (unsigned char *)*output + next_offset,
                                 (size_t)expanded_size)) goto error;
        } else {
            if (!flags && stored_size != expanded_size) goto error;
            memcpy(*output + next_offset,
                   input + (size_t)input_offset,
                   (size_t)stored_size);
        }
        write_u32le(output_row + 4u, expand ? 0u : flags);
        write_u64le(output_row + 16u, next_offset);
        write_u64le(output_row + 24u, expand ? expanded_size : stored_size);
        next_offset += (size_t)(expand ? expanded_size : stored_size);
    }
    return 1;

error:
    free(*output);
    *output = 0;
    *output_size = 0u;
    return 0;
}

static int image_matches(module_file *expected, char *image, size_t size,
                         int require_mixed_storage) {
    module_file *decoded = 0;
    module_file *extra = 0;
    char *cursor = image;
    rxbin_reader reader;
    uint32_t section;
    int compressed = 0;
    int plain = 0;
    int ok;

    for (section = 0u; section < RXBIN007_SECTION_COUNT; section++) {
        const unsigned char *row = (const unsigned char *)image +
                RXBIN007_HEADER_SIZE +
                (size_t)section * RXBIN007_DIRECTORY_ENTRY_SIZE;
        if (read_u32le(row + 4u) == RXBIN007_SECTION_LZSS) compressed = 1;
        else if (read_u32le(row + 4u) == 0u) plain = 1;
    }
    if (require_mixed_storage && (!compressed || !plain)) return 0;
    rxbin_reader_init_mem(&reader, &cursor, image + size);
    ok = rxbin_reader_next_module(&reader, &decoded) == 0 && decoded &&
         rxbin_reader_next_module(&reader, &extra) == 1 && !extra;
    rxbin_reader_close(&reader);
    if (!ok) {
        free_module(decoded);
        free_module(extra);
        return 0;
    }
    ok = cursor == image + size &&
         expected->header.instruction_size == decoded->header.instruction_size &&
         expected->header.constant_size == decoded->header.constant_size &&
         expected->header.section_flags == decoded->header.section_flags &&
         memcmp(expected->instructions, decoded->instructions,
                expected->header.instruction_size * sizeof(bin_code)) == 0 &&
         memcmp(expected->constant, decoded->constant,
                expected->header.constant_size) == 0;
    free_module(decoded);
    return ok;
}

static int find_record(const unsigned char *section,
                       size_t section_size,
                       uint32_t wanted_type,
                       uint32_t *id,
                       size_t *payload_offset) {
    uint32_t record_count;
    uint32_t record_index;
    size_t offset;

    if (!section || section_size < 16u || !id || !payload_offset) return 0;
    record_count = read_u32le(section + 12u);
    offset = 16u;
    for (record_index = 0u; record_index < record_count; record_index++) {
        uint32_t record_id;
        uint32_t type;
        uint64_t payload_size;
        size_t next;

        if (offset > section_size || section_size - offset < 24u) return 0;
        record_id = read_u32le(section + offset + 4u);
        type = read_u32le(section + offset + 8u);
        payload_size = read_u64le(section + offset + 16u);
        if (payload_size > SIZE_MAX ||
            (size_t)payload_size > section_size - offset - 24u) return 0;
        if (type == wanted_type) {
            *id = record_id;
            *payload_offset = offset + 24u;
            return 1;
        }
        next = offset + 24u + (size_t)payload_size;
        offset = (next + 7u) & ~(size_t)7u;
    }
    return 0;
}

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

static int mutation_is_rejected(const char *buffer,
                                size_t buffer_size,
                                size_t offset,
                                unsigned char replacement,
                                int expected_rc,
                                const char *description) {
    char *copy;
    char *cursor;
    module_file *module;
    int rc;

    if (offset >= buffer_size) return 0;
    copy = (char *)malloc(buffer_size);
    if (!copy) return 0;
    memcpy(copy, buffer, buffer_size);
    copy[offset] = (char)replacement;
    cursor = copy;
    module = 0;
    rc = read_module_mem(&module, &cursor, copy + buffer_size);
    if (module) free_module(module);
    free(copy);
    if (rc != expected_rc) {
        fprintf(stderr, "%s mutation returned %d, expected %d\n",
                description, rc, expected_rc);
        return 0;
    }
    return 1;
}

static int mutation_u32_is_rejected(const char *buffer,
                                    size_t buffer_size,
                                    size_t offset,
                                    uint32_t replacement,
                                    const char *description) {
    char *copy;
    char *cursor;
    module_file *module;
    int rc;

    if (offset > buffer_size || buffer_size - offset < 4u) return 0;
    copy = (char *)malloc(buffer_size);
    if (!copy) return 0;
    memcpy(copy, buffer, buffer_size);
    write_u32le((unsigned char *)copy + offset, replacement);
    cursor = copy;
    module = 0;
    rc = read_module_mem(&module, &cursor, copy + buffer_size);
    if (module) free_module(module);
    free(copy);
    if (rc != -1) {
        fprintf(stderr, "%s mutation returned %d, expected -1\n",
                description, rc);
        return 0;
    }
    return 1;
}

static int mutation_u64_is_rejected(const char *buffer,
                                    size_t buffer_size,
                                    size_t offset,
                                    uint64_t replacement,
                                    const char *description) {
    char *copy;
    char *cursor;
    module_file *module;
    int rc;

    if (offset > buffer_size || buffer_size - offset < 8u) return 0;
    copy = (char *)malloc(buffer_size);
    if (!copy) return 0;
    memcpy(copy, buffer, buffer_size);
    write_u64le((unsigned char *)copy + offset, replacement);
    cursor = copy;
    module = 0;
    rc = read_module_mem(&module, &cursor, copy + buffer_size);
    if (module) free_module(module);
    free(copy);
    if (rc != -1) {
        fprintf(stderr, "%s mutation returned %d, expected -1\n",
                description, rc);
        return 0;
    }
    return 1;
}

/* Rewriting a loaded import must always emit u64 all-ones, including when
 * this test executable uses 32-bit size_t. */
static int imported_start_writer_is_canonical(module_file *module) {
    FILE *file = tmpfile();
    char *bytes = 0, *expanded = 0;
    long length;
    size_t expanded_size = 0, section, payload;
    uint32_t id;
    int ok = 0;
    if (!file) return 0;
    if (write_module(module, file) || fflush(file) || fseek(file, 0, SEEK_END)) goto done;
    length = ftell(file);
    if (length <= 0 || fseek(file, 0, SEEK_SET)) goto done;
    bytes = (char *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length ||
        !expand_sections(bytes, (size_t)length, UINT32_MAX,
                         &expanded, &expanded_size)) goto done;
    section = (size_t)read_u64le((unsigned char *)expanded + RXBIN007_HEADER_SIZE +
                                2u * RXBIN007_DIRECTORY_ENTRY_SIZE + 16u);
    if (!find_record((unsigned char *)expanded + section, expanded_size - section,
                     PROC_CONST, &id, &payload)) goto done;
    ok = read_u32le((unsigned char *)expanded + section + payload + 4u) == UINT32_MAX &&
         read_u64le((unsigned char *)expanded + section + payload + 8u) == UINT64_MAX;
done:
    free(bytes); free(expanded); fclose(file);
    if (!ok) fprintf(stderr, "Imported procedure writer sentinel is not canonical\n");
    return ok;
}

/* Exercise LP64 and historical ILP32 import sentinels independently of the
 * writer's host size. The same test runs in a 32-bit reader build. */
static int imported_start_is_normalized(const char *image, size_t size,
                                        size_t payload, uint64_t start) {
    char *copy;
    char *cursor;
    rxbin_reader reader;
    module_file *module = 0;
    size_t offset;
    unsigned imports = 0u;
    int ok = 1;

    if (payload > size || size - payload < 16u) return 0;
    copy = (char *)malloc(size);
    if (!copy) return 0;
    memcpy(copy, image, size);
    write_u32le((unsigned char *)copy + payload + 4u, UINT32_MAX);
    write_u64le((unsigned char *)copy + payload + 8u, start);
    cursor = copy;
    rxbin_reader_init_mem(&reader, &cursor, copy + size);
    if (rxbin_reader_next_module(&reader, &module) != 0 || !module) ok = 0;
    if (ok) {
        for (offset = 0u; offset < module->header.constant_size;) {
            chameleon_constant *constant =
                (chameleon_constant *)(module->constant + offset);
            if (!constant->size_in_pool) { ok = 0; break; }
            if (constant->type == PROC_CONST) {
                proc_constant *procedure = (proc_constant *)constant;
                if (procedure->locals == -1) {
                    imports++;
                    if (procedure->start != SIZE_MAX) ok = 0;
                }
            }
            offset += constant->size_in_pool;
        }
        if (imports != 1u || !imported_start_writer_is_canonical(module)) ok = 0;
    }
    free_module(module);
    rxbin_reader_close(&reader);
    free(copy);
    if (!ok) fprintf(stderr, "Imported procedure start was not normalized\n");
    return ok;
}

/* A later module may fail after an earlier module retained the decoded pool.
 * Exercise both one shared pool and two independent pools, without keeping a
 * malformed binary in the repository. The reader must publish no partial
 * module and release construction/module ownership exactly once (SAN-015). */
static int linked_error_cleanup(module_file *prototype) {
    int separate;
    for (separate = 0; separate < 2; separate++) {
        module_file second = *prototype;
        module_file *modules[2];
        module_file *first_read = 0;
        module_file *second_read = 0;
        module_file *unexpected = 0;
        FILE *file = tmpfile();
        char *bytes = 0;
        char *expanded = 0;
        char *cursor;
        char *second_pool = 0;
        size_t expanded_size = 0;
        size_t module_offset;
        size_t instruction_row;
        size_t instruction_size;
        long length;
        int ok = 0;
        int reader_open = 0;
        int attempt;
        rxbin_reader reader;

        if (!file) return 0;
        second.name = "compact_cleanup_second";
        /* Keep the private instruction carrier without redeclaring module
         * one's exported callable identity from the shared metadata pool. */
        second.header.meta_head = -1;
        second.header.expose_head = -1;
        if (separate) {
            second_pool = (char *)malloc(prototype->header.constant_size);
            if (!second_pool) goto done;
            memcpy(second_pool, prototype->constant,
                   prototype->header.constant_size);
            second.constant = (unsigned char *)second_pool;
        }
        modules[0] = prototype;
        modules[1] = &second;
        if (write_modules(modules, 2u, 0, file) || fflush(file) ||
            fseek(file, 0, SEEK_END)) goto done;
        length = ftell(file);
        if (length <= 0 || fseek(file, 0, SEEK_SET)) goto done;
        bytes = (char *)malloc((size_t)length);
        if (!bytes || fread(bytes, 1u, (size_t)length, file) != (size_t)length ||
            !expand_sections(bytes, (size_t)length, UINT32_MAX,
                             &expanded, &expanded_size)) goto done;

        cursor = expanded;
        rxbin_reader_init_mem(&reader, &cursor, expanded + expanded_size);
        reader_open = 1;
        if (rxbin_reader_next_module(&reader, &first_read) ||
            rxbin_reader_next_module(&reader, &second_read) ||
            !first_read || !second_read ||
            ((first_read->shared_constant_pool == second_read->shared_constant_pool)
                != !separate) ||
            rxbin_reader_next_module(&reader, &unexpected) != 1 || unexpected)
            goto done;
        rxbin_reader_close(&reader);
        reader_open = 0;
        free_module(first_read); first_read = 0;
        free_module(second_read); second_read = 0;

        module_offset = (size_t)read_u64le((unsigned char *)expanded +
                RXBIN007_HEADER_SIZE + 16u);
        instruction_row = RXBIN007_HEADER_SIZE + RXBIN007_DIRECTORY_ENTRY_SIZE;
        instruction_size = (size_t)read_u64le((unsigned char *)expanded +
                instruction_row + 32u);
        if (read_u32le((unsigned char *)expanded + 28u) != 2u ||
            module_offset > expanded_size ||
            expanded_size - module_offset < 16u + 2u * 88u ||
            read_u64le((unsigned char *)expanded + module_offset +
                       16u + 88u + 40u) == 0u) goto done;
        /* Module one is valid. Module two's nonempty instruction range now
         * starts at EOF, forcing partial-module cleanup after pool retention. */
        write_u64le((unsigned char *)expanded + module_offset +
                    16u + 88u + 32u, instruction_size);
        for (attempt = 0; attempt < 3; attempt++) {
            int rc;
            const char *detail;
            cursor = expanded;
            rxbin_reader_init_mem(&reader, &cursor, expanded + expanded_size);
            reader_open = 1;
            rc = rxbin_reader_next_module(&reader, &unexpected);
            detail = rxbin_last_error();
            if (rc != -1 || unexpected || !detail ||
                !strstr(detail, "module directory or instructions are invalid"))
                goto done;
            rxbin_reader_close(&reader);
            reader_open = 0;
        }
        ok = 1;
done:
        if (reader_open) rxbin_reader_close(&reader);
        free_module(first_read);
        free_module(second_read);
        free_module(unexpected);
        free(second_pool);
        free(expanded);
        free(bytes);
        fclose(file);
        if (!ok) {
            fprintf(stderr, "Linked RXBIN partial-module cleanup failed (separate pools=%d): %s\n",
                    separate, rxbin_last_error() ? rxbin_last_error() : "no detail");
            return 0;
        }
    }
    return 1;
}

int main(void) {
    FILE *fp = 0;
    unsigned char raw_header[RXBIN007_HEADER_SIZE];
    unsigned char directory[RXBIN007_SECTION_COUNT * RXBIN007_DIRECTORY_ENTRY_SIZE];
    module_file *from_file = 0;
    module_file *from_memory = 0;
    char *buffer = 0;
    char *expanded_buffer = 0;
    char *mixed_buffer = 0;
    char *cursor = 0;
    size_t buffer_size = 0;
    size_t expanded_buffer_size = 0u;
    size_t mixed_buffer_size = 0u;
    uint32_t mixed_expand_mask = 0u;
    size_t compressed_row = SIZE_MAX;
    bin_code *code = 0;
    uint64_t instruction_bytes = 0u;

    fp = fopen("tests_compact_format.rxbin", "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open tests_compact_format.rxbin\n");
        return 1;
    }

    if (fread(raw_header, 1u, sizeof(raw_header), fp) != sizeof(raw_header) ||
        fread(directory, 1u, sizeof(directory), fp) != sizeof(directory)) {
        fprintf(stderr, "Failed to read RXBIN 007 header and directory\n");
        fclose(fp);
        return 1;
    }

    if (memcmp(raw_header, RXBIN007_MAGIC, 8u) != 0 ||
        read_u32le(raw_header + 8u) != RXBIN007_HEADER_SIZE ||
        read_u32le(raw_header + 12u) != 0u ||
        read_u32le(raw_header + 24u) != RXBIN007_SECTION_COUNT ||
        read_u32le(raw_header + 28u) != 1u) {
        fprintf(stderr, "Fixture does not use the canonical RXBIN 007 container\n");
        fclose(fp);
        return 1;
    }

    {
        uint32_t section;
        int saw_compressed = 0;
        int saw_uncompressed = 0;
        for (section = 0u; section < RXBIN007_SECTION_COUNT; section++) {
            const unsigned char *row = directory +
                (size_t)section * RXBIN007_DIRECTORY_ENTRY_SIZE;
            uint32_t flags = read_u32le(row + 4u);
            uint64_t stored_size = read_u64le(row + 24u);
            uint64_t expanded_size = read_u64le(row + 32u);
            if (read_u32le(row) != section + 1u ||
                (flags != 0u && flags != RXBIN007_SECTION_LZSS) ||
                read_u32le(row + 8u) != 8u ||
                read_u32le(row + 12u) != 0u ||
                (flags == RXBIN007_SECTION_LZSS
                    ? stored_size >= expanded_size
                    : stored_size != expanded_size)) {
                fprintf(stderr, "RXBIN 007 base section directory is not canonical\n");
                fclose(fp);
                return 1;
            }
            if (flags == RXBIN007_SECTION_LZSS) {
                saw_compressed = 1;
                if (compressed_row == SIZE_MAX) {
                    compressed_row = RXBIN007_HEADER_SIZE +
                                     (size_t)section * RXBIN007_DIRECTORY_ENTRY_SIZE;
                }
            } else {
                saw_uncompressed = 1;
            }
        }
        if (!saw_compressed) {
            fprintf(stderr, "RXBIN 007 fixture did not exercise compressed section storage\n");
            fclose(fp);
            return 1;
        }
        /* Compression selection is the writer's choice. If every section
         * compresses, expand one explicitly and retain the other compressed
         * sections rather than depending on a one-byte compression margin. */
        if (!saw_uncompressed) mixed_expand_mask = 1u;
        instruction_bytes = read_u64le(
            directory + RXBIN007_DIRECTORY_ENTRY_SIZE + 24u);
    }

    rewind(fp);
    if (read_module(&from_file, fp) != 0) {
        fprintf(stderr, "Failed to decode tests_compact_format.rxbin from file\n");
        fclose(fp);
        return 1;
    }
    fclose(fp);

    if (!linked_error_cleanup(from_file)) {
        free_module(from_file);
        return 1;
    }

    if (instruction_bytes >=
        8u + from_file->header.instruction_size * sizeof(bin_code)) {
        fprintf(stderr, "Canonical variable-integer instruction stream is not compact\n");
        free_module(from_file);
        return 1;
    }

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

    if (!expand_sections(buffer, buffer_size, mixed_expand_mask,
                         &mixed_buffer, &mixed_buffer_size) ||
        !image_matches(from_file, mixed_buffer, mixed_buffer_size, 1)) {
        fprintf(stderr, "Mixed RXBIN 007 section storage did not decode equivalently\n");
        free(mixed_buffer);
        free_module(from_memory);
        free(buffer);
        free_module(from_file);
        return 1;
    }
    free(mixed_buffer);

    if (!expand_sections(buffer,
                         buffer_size,
                         UINT32_MAX,
                         &expanded_buffer,
                         &expanded_buffer_size) ||
        !image_matches(from_file, expanded_buffer, expanded_buffer_size, 0)) {
        fprintf(stderr, "Could not expand RXBIN 007 sections for corruption tests\n");
        free(expanded_buffer);
        free_module(from_memory);
        free(buffer);
        free_module(from_file);
        return 1;
    }

    {
        size_t facts_row = RXBIN007_HEADER_SIZE +
            (RXBIN007_SECTION_GRAPH_FACTS - 1u) * RXBIN007_DIRECTORY_ENTRY_SIZE;
        size_t constants_row = RXBIN007_HEADER_SIZE +
            (RXBIN007_SECTION_CONSTANTS - 1u) * RXBIN007_DIRECTORY_ENTRY_SIZE;
        size_t metadata_row = RXBIN007_HEADER_SIZE +
            (RXBIN007_SECTION_METADATA - 1u) * RXBIN007_DIRECTORY_ENTRY_SIZE;
        size_t facts_offset = (size_t)read_u64le((const unsigned char *)buffer +
                                                 facts_row + 16u);
        size_t constants_offset = (size_t)read_u64le((const unsigned char *)expanded_buffer +
                                                     constants_row + 16u);
        size_t constants_size = (size_t)read_u64le((const unsigned char *)expanded_buffer +
                                                   constants_row + 24u);
        size_t metadata_offset = (size_t)read_u64le((const unsigned char *)expanded_buffer +
                                                    metadata_row + 16u);
        size_t metadata_size = (size_t)read_u64le((const unsigned char *)expanded_buffer +
                                                  metadata_row + 24u);
        uint32_t procedure_id;
        uint32_t ignored_id;
        size_t ignored_payload;
        size_t function_payload;
        int found_wrong_kind_fixture;

        found_wrong_kind_fixture =
            constants_offset <= expanded_buffer_size &&
            constants_size <= expanded_buffer_size - constants_offset &&
            metadata_offset <= expanded_buffer_size &&
            metadata_size <= expanded_buffer_size - metadata_offset &&
            find_record((const unsigned char *)expanded_buffer + constants_offset,
                        constants_size,
                        PROC_CONST,
                        &procedure_id,
                        &ignored_payload) &&
            find_record((const unsigned char *)expanded_buffer + metadata_offset,
                        metadata_size,
                        META_FUNC,
                        &ignored_id,
                        &function_payload);
        if (!found_wrong_kind_fixture) {
            fprintf(stderr, "Could not locate PROC/META_FUNC wrong-kind fixture records\n");
        }
        if (!mutation_is_rejected(buffer,
                                  buffer_size,
                                  7u,
                                  (unsigned char)'6',
                                  2,
                                  "legacy-version") ||
            !mutation_is_rejected(buffer,
                                  buffer_size,
                                  40u,
                                  1u,
                                  -1,
                                  "reserved-header") ||
            !mutation_is_rejected(buffer,
                                  buffer_size,
                                  RXBIN007_HEADER_SIZE + 12u,
                                  1u,
                                  -1,
                                  "reserved-directory") ||
            !mutation_u32_is_rejected(buffer,
                                      buffer_size,
                                      RXBIN007_HEADER_SIZE + 4u,
                                      2u,
                                      "unknown-section-compression") ||
            !mutation_u64_is_rejected(
                buffer,
                buffer_size,
                compressed_row + 32u,
                read_u64le((const unsigned char *)buffer + compressed_row + 24u),
                "compressed-section-without-size-reduction") ||
            !mutation_is_rejected(buffer,
                                  buffer_size,
                                  facts_offset,
                                  (unsigned char)'X',
                                  -1,
                                  "graph-facts") ||
            !mutation_u64_is_rejected(buffer,
                                      buffer_size,
                                      16u,
                                      32u,
                                      "undersized-declared-file") ||
            !found_wrong_kind_fixture ||
            (sizeof(size_t) < 8u && !mutation_u64_is_rejected(expanded_buffer, expanded_buffer_size,
                                      constants_offset + ignored_payload + 8u,
                                      UINT64_MAX - 1u, "overflowing-real-procedure-address")) ||
            !imported_start_is_normalized(expanded_buffer,
                                           expanded_buffer_size,
                                           constants_offset + ignored_payload,
                                           UINT64_MAX) ||
            !imported_start_is_normalized(expanded_buffer,
                                           expanded_buffer_size,
                                           constants_offset + ignored_payload,
                                           UINT32_MAX) ||
            !mutation_u32_is_rejected(expanded_buffer,
                                      expanded_buffer_size,
                                      metadata_offset + function_payload + 16u,
                                      procedure_id,
                                      "metadata-wrong-record-kind")) {
            free(expanded_buffer);
            free_module(from_memory);
            free(buffer);
            free_module(from_file);
            return 1;
        }
    }

    free(expanded_buffer);
    free_module(from_memory);
    free(buffer);
    free_module(from_file);
    return 0;
}
