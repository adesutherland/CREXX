#include <ctype.h>
#include <inttypes.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxnumparse.h"

static uint64_t double_bits(double value) {
    uint64_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static int locale_aware_prefilter(double *out, const char *text, size_t length) {
    unsigned char first;
    struct lconv *numeric_locale;

    if (length == 0) return 1;
    first = (unsigned char)text[0];
    if (first >= 0x80 || isspace(first) ||
        (first >= '0' && first <= '9') || first == '+' || first == '-' ||
        first == '.' || first == ',' || first == 'i' || first == 'I' ||
        first == 'n' || first == 'N') {
        return rx_string_to_double(out, text, length);
    }

    numeric_locale = localeconv();
    if (numeric_locale && numeric_locale->decimal_point &&
        numeric_locale->decimal_point[0] != '\0' &&
        first == (unsigned char)numeric_locale->decimal_point[0]) {
        return rx_string_to_double(out, text, length);
    }
    return 1;
}

static int check_one(const char *locale_name,
                     const char *label,
                     const char *text,
                     size_t length) {
    uint64_t sentinel = UINT64_C(0x3ff3c0ca428c59dd);
    double current;
    double candidate;
    int current_rc;
    int candidate_rc;

    memcpy(&current, &sentinel, sizeof(current));
    memcpy(&candidate, &sentinel, sizeof(candidate));
    current_rc = rx_string_to_double(&current, text, length);
    candidate_rc = locale_aware_prefilter(&candidate, text, length);
    if (current_rc != candidate_rc || double_bits(current) != double_bits(candidate)) {
        printf("MISMATCH\t%s\t%s\t%zu\t%d\t%016" PRIx64
               "\t%d\t%016" PRIx64 "\n",
               locale_name, label, length, current_rc, double_bits(current),
               candidate_rc, double_bits(candidate));
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    static const char *corpus[] = {
        "", " ", "0", "-0", "+1.5", "1.5", "1,5", "0x1p2",
        "inf", "-infinity", "nan", "nan(123)", "1e-324",
        "1.7976931348623157e308", "1.5x", "A", "/", "_", "\t-3"
    };
    unsigned char bytes[2];
    uint64_t cases = 0;
    uint64_t mismatches = 0;
    int i;
    int b;

    if (argc != 2) return 2;
    if (setlocale(LC_ALL, argv[1]) == NULL) {
        printf("LOCALE_ERROR\t%s\n", argv[1]);
        return 3;
    }
    for (i = 0; i < (int)(sizeof(corpus) / sizeof(corpus[0])); i++) {
        mismatches += (uint64_t)check_one(argv[1], "corpus", corpus[i], strlen(corpus[i]));
        cases++;
    }
    for (b = 0; b <= 255; b++) {
        bytes[0] = (unsigned char)b;
        bytes[1] = (unsigned char)'1';
        mismatches += (uint64_t)check_one(argv[1], "byte1", (const char *)bytes, 1);
        mismatches += (uint64_t)check_one(argv[1], "byte2", (const char *)bytes, 2);
        cases += 2;
    }
    printf("SUMMARY\t%s\t%" PRIu64 "\t%" PRIu64 "\n",
           argv[1], cases, mismatches);
    return mismatches == 0 ? 0 : 1;
}
