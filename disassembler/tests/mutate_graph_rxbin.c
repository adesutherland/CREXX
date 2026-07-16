/* cREXX License (MIT) */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RXBIN007_HEADER_SIZE 64u
#define RXBIN007_DIRECTORY_ENTRY_SIZE 40u
#define RXBIN007_GRAPH_FACTS_SECTION 5u

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
    uint64_t facts_offset;
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
    facts_offset = read_u64le(bytes + facts_row + 16u);
    if (facts_offset >= (uint64_t)length) {
        free(bytes);
        return 7;
    }
    bytes[(size_t)facts_offset] = (unsigned char)'X';
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
