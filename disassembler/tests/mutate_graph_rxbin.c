/* cREXX License (MIT) */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RXBIN007_HEADER_SIZE 64u
#define RXBIN007_DIRECTORY_ENTRY_SIZE 40u
#define RXBIN007_GRAPH_FACTS_SECTION 5u
#define RXBIN007_SECTION_LZSS 1u

static uint32_t read_u32le(const unsigned char *bytes) {
    uint32_t value;
    unsigned int i;

    value = 0u;
    for (i = 0u; i < 4u; i++) value |= (uint32_t)bytes[i] << (i * 8u);
    return value;
}

static uint64_t read_u64le(const unsigned char *bytes) {
    uint64_t value;
    unsigned int i;

    value = 0u;
    for (i = 0u; i < 8u; i++) value |= (uint64_t)bytes[i] << (i * 8u);
    return value;
}

int main(int argc, char **argv) {
    FILE *input;
    FILE *output;
    unsigned char *bytes;
    long length;
    size_t facts_row;
    size_t magic_offset;
    uint32_t facts_flags;
    uint64_t facts_offset;
    uint64_t facts_stored_size;
    int result;

    if (argc != 3) return 2;
    input = fopen(argv[1], "rb");
    if (!input) return 3;
    if (fseek(input, 0, SEEK_END) != 0 || (length = ftell(input)) < 0 ||
        fseek(input, 0, SEEK_SET) != 0) {
        fclose(input);
        return 4;
    }
    bytes = (unsigned char *)malloc((size_t)length);
    if (!bytes) {
        fclose(input);
        return 5;
    }
    result = fread(bytes, 1u, (size_t)length, input) == (size_t)length;
    fclose(input);
    facts_row = RXBIN007_HEADER_SIZE +
        (RXBIN007_GRAPH_FACTS_SECTION - 1u) * RXBIN007_DIRECTORY_ENTRY_SIZE;
    if (!result || (size_t)length < facts_row + RXBIN007_DIRECTORY_ENTRY_SIZE) {
        free(bytes);
        return 6;
    }
    facts_flags = read_u32le(bytes + facts_row + 4u);
    facts_offset = read_u64le(bytes + facts_row + 16u);
    facts_stored_size = read_u64le(bytes + facts_row + 24u);
    if ((facts_flags & ~RXBIN007_SECTION_LZSS) ||
        facts_offset > (uint64_t)length ||
        facts_stored_size > (uint64_t)length - facts_offset ||
        facts_stored_size == 0u) {
        free(bytes);
        return 7;
    }
    magic_offset = (size_t)facts_offset;
    if (facts_flags & RXBIN007_SECTION_LZSS) {
        /* The first decoded byte must be a literal following the control byte. */
        if (facts_stored_size < 2u || (bytes[magic_offset] & 1u)) {
            free(bytes);
            return 7;
        }
        magic_offset++;
    }
    if (bytes[magic_offset] != (unsigned char)'R') {
        free(bytes);
        return 7;
    }
    bytes[magic_offset] = (unsigned char)'X';
    output = fopen(argv[2], "wb");
    if (!output) {
        free(bytes);
        return 8;
    }
    result = fwrite(bytes, 1u, (size_t)length, output) == (size_t)length &&
             fclose(output) == 0;
    free(bytes);
    return result ? 0 : 9;
}
