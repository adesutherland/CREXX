/* Focused RXBIN feature-flag mutation helper for compatibility tests. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    FILE *input;
    FILE *output;
    unsigned char *data;
    char *end;
    long length;
    unsigned long flags;
    size_t size;

    if (argc != 4) {
        fprintf(stderr, "usage: mutate_rxbin_feature input output flags\n");
        return 2;
    }
    flags = strtoul(argv[3], &end, 0);
    if (!argv[3][0] || *end || flags > 0xffffffffUL) {
        fprintf(stderr, "invalid feature flags: %s\n", argv[3]);
        return 2;
    }
    input = fopen(argv[1], "rb");
    if (!input) {
        fprintf(stderr, "cannot open input: %s\n", argv[1]);
        return 2;
    }
    if (fseek(input, 0, SEEK_END) != 0 || (length = ftell(input)) < 16 ||
        fseek(input, 0, SEEK_SET) != 0) {
        fclose(input);
        fprintf(stderr, "input is not an RXBIN 007 container\n");
        return 2;
    }
    size = (size_t)length;
    data = (unsigned char *)malloc(size);
    if (!data || fread(data, 1, size, input) != size) {
        free(data);
        fclose(input);
        fprintf(stderr, "cannot read input\n");
        return 2;
    }
    fclose(input);
    if (memcmp(data, "cReXx007", 8) != 0) {
        free(data);
        fprintf(stderr, "input is not an RXBIN 007 container\n");
        return 2;
    }
    data[12] = (unsigned char)(flags & 0xffUL);
    data[13] = (unsigned char)((flags >> 8) & 0xffUL);
    data[14] = (unsigned char)((flags >> 16) & 0xffUL);
    data[15] = (unsigned char)((flags >> 24) & 0xffUL);

    output = fopen(argv[2], "wb");
    if (!output) {
        free(data);
        fprintf(stderr, "cannot write output: %s\n", argv[2]);
        return 2;
    }
    if (fwrite(data, 1, size, output) != size) {
        fclose(output);
        free(data);
        fprintf(stderr, "cannot write output: %s\n", argv[2]);
        return 2;
    }
    if (fclose(output) != 0) {
        free(data);
        fprintf(stderr, "cannot close output: %s\n", argv[2]);
        return 2;
    }
    free(data);
    return 0;
}
