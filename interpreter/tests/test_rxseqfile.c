/* Unit tests for the portable RXSEQ integer encoding. */

#include <stdint.h>
#include <stdio.h>

#include "rxseqfile.h"

static int round_trip(uint64_t value) {
    FILE *file = tmpfile();
    uint64_t actual = 0;
    int ok;
    if (!file) return 0;
    ok = rxseq_write_varuint(file, value) &&
            fflush(file) == 0 &&
            fseek(file, 0, SEEK_SET) == 0 &&
            rxseq_read_varuint(file, &actual) &&
            actual == value &&
            fgetc(file) == EOF;
    fclose(file);
    return ok;
}

static int rejects(const unsigned char *bytes, size_t size) {
    FILE *file = tmpfile();
    uint64_t value;
    int rejected;
    if (!file) return 0;
    if (fwrite(bytes, 1, size, file) != size ||
            fflush(file) != 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    rejected = !rxseq_read_varuint(file, &value);
    fclose(file);
    return rejected;
}

int main(void) {
    static const uint64_t values[] = {
            UINT64_C(0), UINT64_C(1), UINT64_C(127), UINT64_C(128),
            UINT64_C(16383), UINT64_C(16384), UINT32_MAX, UINT64_MAX
    };
    static const unsigned char noncanonical[] = {0x80, 0x00};
    static const unsigned char truncated[] = {0x80};
    static const unsigned char overflow[] = {
            0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0x02
    };
    size_t i;
    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        if (!round_trip(values[i])) {
            fprintf(stderr, "RXSEQ varuint round trip failed at index %zu\n", i);
            return 1;
        }
    }
    if (!rejects(noncanonical, sizeof(noncanonical)) ||
            !rejects(truncated, sizeof(truncated)) ||
            !rejects(overflow, sizeof(overflow))) {
        fprintf(stderr, "RXSEQ invalid varuint was accepted\n");
        return 1;
    }
    return 0;
}
