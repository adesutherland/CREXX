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
        uint64_t expanded_size;

        row = (const unsigned char *)input + RXBIN007_HEADER_SIZE +
              (size_t)section * RXBIN007_DIRECTORY_ENTRY_SIZE;
        expanded_size = read_u64le(row + 32u);
        next_offset = (next_offset + 7u) & ~(size_t)7u;
        if (expanded_size > SIZE_MAX || (size_t)expanded_size > SIZE_MAX - next_offset) return 0;
        next_offset += (size_t)expanded_size;
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

        input_row = (const unsigned char *)input + RXBIN007_HEADER_SIZE +
                    (size_t)section * RXBIN007_DIRECTORY_ENTRY_SIZE;
        output_row = (unsigned char *)*output + RXBIN007_HEADER_SIZE +
                     (size_t)section * RXBIN007_DIRECTORY_ENTRY_SIZE;
        flags = read_u32le(input_row + 4u);
        input_offset = read_u64le(input_row + 16u);
        stored_size = read_u64le(input_row + 24u);
        expanded_size = read_u64le(input_row + 32u);
        next_offset = (next_offset + 7u) & ~(size_t)7u;
        if (input_offset > input_size || stored_size > input_size - (size_t)input_offset ||
            (flags != 0u && flags != RXBIN007_SECTION_LZSS)) goto error;
        if (flags == RXBIN007_SECTION_LZSS) {
            if (!decompress_lzss((const unsigned char *)input + (size_t)input_offset,
                                 (size_t)stored_size,
                                 (unsigned char *)*output + next_offset,
                                 (size_t)expanded_size)) goto error;
        } else {
            if (stored_size != expanded_size) goto error;
            memcpy(*output + next_offset,
                   input + (size_t)input_offset,
                   (size_t)stored_size);
        }
        write_u32le(output_row + 4u, 0u);
        write_u64le(output_row + 16u, next_offset);
        write_u64le(output_row + 24u, expanded_size);
        next_offset += (size_t)expanded_size;
    }
    return 1;

error:
    free(*output);
    *output = 0;
    *output_size = 0u;
    return 0;
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

int main(void) {
    FILE *fp = 0;
    unsigned char raw_header[RXBIN007_HEADER_SIZE];
    unsigned char directory[RXBIN007_SECTION_COUNT * RXBIN007_DIRECTORY_ENTRY_SIZE];
    module_file *from_file = 0;
    module_file *from_memory = 0;
    char *buffer = 0;
    char *expanded_buffer = 0;
    char *cursor = 0;
    size_t buffer_size = 0;
    size_t expanded_buffer_size = 0u;
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
        if (!saw_compressed || !saw_uncompressed) {
            fprintf(stderr, "RXBIN 007 fixture did not exercise mixed section storage\n");
            fclose(fp);
            return 1;
        }
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

    if (!expand_sections(buffer,
                         buffer_size,
                         &expanded_buffer,
                         &expanded_buffer_size)) {
        fprintf(stderr, "Could not expand RXBIN 007 sections for corruption tests\n");
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
