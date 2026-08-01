#define _DARWIN_C_SOURCE 1

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <xlocale.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "rxinteger.h"

static uint64_t allocation_calls;
static uint64_t allocation_bytes;
static uint64_t copied_bytes;

static void *tracked_malloc(size_t size) {
    allocation_calls++;
    allocation_bytes += size;
    return malloc(size);
}

static void tracked_free(void *pointer) {
    free(pointer);
}

#define malloc tracked_malloc
#define free tracked_free
#include "rxnumparse.h"
#undef malloc
#undef free

extern int bounded_from_chars_double(double *out,
                                     const char *text,
                                     size_t length);

typedef int (*integer_parser)(rxinteger *, const char *, size_t);
typedef int (*float_parser)(double *, const char *, size_t);

struct input_case {
    const char *label;
    const char *text;
    size_t length;
};

static int ascii_space(unsigned char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
           c == '\f' || c == '\v';
}

static uint64_t double_bits(double value) {
    uint64_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static uint64_t elapsed_ns(struct timespec start, struct timespec finish) {
    return (uint64_t)(finish.tv_sec - start.tv_sec) * UINT64_C(1000000000) +
           (uint64_t)(finish.tv_nsec - start.tv_nsec);
}

static void reset_counters(void) {
    allocation_calls = 0;
    allocation_bytes = 0;
    copied_bytes = 0;
}

static int current_integer(rxinteger *out, const char *string, size_t length) {
    char *buffer = tracked_malloc(length + 1u);
    char *end = buffer;
    int rc = 0;
    rxinteger parsed;

    if (buffer == NULL) return 1;
    copied_bytes += length;
    buffer[length] = 0;
    memcpy(buffer, string, length);
    if (rxinteger_parse(buffer, &end, &parsed)) {
        rc = 1;
        goto done;
    }
    while (*end != 0) {
        if (!isspace((unsigned char)*end)) {
            rc = 1;
            goto done;
        }
        end++;
    }
    *out = parsed;

done:
    tracked_free(buffer);
    return rc;
}

static int bounded_integer(rxinteger *out, const char *string, size_t length) {
    size_t i = 0;
    int negative = 0;
    uintmax_t value = 0;
    uintmax_t limit;
    int digits = 0;

    if (out == NULL || string == NULL) return 1;
    while (i < length && ascii_space((unsigned char)string[i])) i++;
    if (i < length && (string[i] == '+' || string[i] == '-')) {
        negative = string[i] == '-';
        i++;
    }
    limit = negative ? (uintmax_t)RXINTEGER_MAX + UINTMAX_C(1)
                     : (uintmax_t)RXINTEGER_MAX;
    while (i < length && string[i] >= '0' && string[i] <= '9') {
        unsigned digit = (unsigned)(string[i] - '0');
        if (value > (limit - digit) / UINTMAX_C(10)) return 1;
        value = value * UINTMAX_C(10) + digit;
        digits = 1;
        i++;
    }
    if (!digits) return 1;
    while (i < length && ascii_space((unsigned char)string[i])) i++;
    if (i != length) return 1;
    if (negative) {
        if (value == (uintmax_t)RXINTEGER_MAX + UINTMAX_C(1)) {
            *out = RXINTEGER_MIN;
        } else {
            *out = -(rxinteger)value;
        }
    } else {
        *out = (rxinteger)value;
    }
    return 0;
}

static int current_float(double *out, const char *string, size_t length) {
    copied_bytes += length;
    return rx_string_to_double(out, string, length);
}

static locale_t c_numeric_locale(void) {
    static locale_t locale;
    if (locale == (locale_t)0) {
        locale = newlocale(LC_NUMERIC_MASK | LC_CTYPE_MASK, "C", NULL);
    }
    return locale;
}

static int copied_c_locale_float(double *out,
                                 const char *string,
                                 size_t length) {
    char local[128];
    char *buffer = local;
    char *end;
    double value;
    int rc = 1;
    locale_t locale = c_numeric_locale();

    if (out == NULL || string == NULL || length == SIZE_MAX ||
        locale == (locale_t)0) return 1;
    if (length + 1u > sizeof(local)) {
        buffer = tracked_malloc(length + 1u);
        if (buffer == NULL) return 1;
    }
    copied_bytes += length;
    memcpy(buffer, string, length);
    buffer[length] = 0;
    errno = 0;
    end = buffer;
    value = strtod_l(buffer, &end, locale);
    if (errno != ERANGE && end != buffer) {
        while (*end != 0 && isspace_l((unsigned char)*end, locale)) end++;
        if (*end == 0) {
            *out = value;
            rc = 0;
        }
    }
    if (buffer != local) tracked_free(buffer);
    return rc;
}

static void print_integer_case(const struct input_case *input,
                               const char *parser_name,
                               integer_parser parser) {
    rxinteger output = (rxinteger)INT64_C(0x1122334455667788);
    int rc;
    reset_counters();
    rc = parser(&output, input->text, input->length);
    printf("contract\tint\t%s\t%s\t%zu\t%d\t%" RXINTEGER_PRI
           "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\n",
           parser_name, input->label, input->length, rc, output,
           allocation_calls, allocation_bytes, copied_bytes);
}

static void print_float_case(const struct input_case *input,
                             const char *parser_name,
                             float_parser parser) {
    double output;
    int rc;
    uint64_t sentinel = UINT64_C(0x3ff3c0ca428c59dd);
    memcpy(&output, &sentinel, sizeof(output));
    reset_counters();
    rc = parser(&output, input->text, input->length);
    printf("contract\tfloat\t%s\t%s\t%zu\t%d\t%016" PRIx64
           "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\n",
           parser_name, input->label, input->length, rc, double_bits(output),
           allocation_calls, allocation_bytes, copied_bytes);
}

static void benchmark_integer(const char *name,
                              integer_parser parser,
                              const struct input_case *inputs,
                              size_t input_count,
                              uint64_t loops) {
    struct timespec start;
    struct timespec finish;
    volatile int64_t checksum = 0;
    uint64_t successes = 0;
    uint64_t i;
    reset_counters();
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < loops; i++) {
        const struct input_case *input = &inputs[i % input_count];
        rxinteger output = 0;
        if (parser(&output, input->text, input->length) == 0) {
            checksum ^= (int64_t)output;
            successes++;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    printf("timing\tint\t%s\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64
           "\t%" PRIu64 "\t%" PRIu64 "\t%" PRId64 "\n",
           name, loops, elapsed_ns(start, finish), allocation_calls,
           allocation_bytes, copied_bytes, (int64_t)checksum + successes);
}

static void benchmark_float(const char *name,
                            float_parser parser,
                            const struct input_case *inputs,
                            size_t input_count,
                            uint64_t loops) {
    struct timespec start;
    struct timespec finish;
    volatile uint64_t checksum = 0;
    uint64_t successes = 0;
    uint64_t i;
    reset_counters();
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < loops; i++) {
        const struct input_case *input = &inputs[i % input_count];
        double output = 0.0;
        if (parser(&output, input->text, input->length) == 0) {
            checksum ^= double_bits(output);
            successes++;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    printf("timing\tfloat\t%s\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64
           "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\n",
           name, loops, elapsed_ns(start, finish), allocation_calls,
           allocation_bytes, copied_bytes, checksum + successes);
}

int main(int argc, char **argv) {
    static const char embedded_int[] = {'4', '2', 0, 'x'};
    static const char embedded_float[] = {'1', '.', '5', 0, 'x'};
    static const struct input_case integer_cases[] = {
        {"empty", "", 0},
        {"space", " ", 1},
        {"zero", "0", 1},
        {"negative_zero", "-0", 2},
        {"plus", "+42", 3},
        {"whitespace", " \t-42\r\n", 7},
        {"leading_zero", "0012", 4},
        {"hex_like", "0x10", 4},
        {"trailing_junk", "42x", 3},
        {"max", "9223372036854775807", 19},
        {"min", "-9223372036854775808", 20},
        {"overflow", "9223372036854775808", 19},
        {"underflow", "-9223372036854775809", 20},
        {"embedded_nul", embedded_int, sizeof(embedded_int)}
    };
    static const struct input_case float_cases[] = {
        {"empty", "", 0},
        {"space", " ", 1},
        {"zero", "0", 1},
        {"negative_zero", "-0", 2},
        {"plus", "+1.5", 4},
        {"whitespace", " \t-1.5\r\n", 8},
        {"decimal_point", "1.5", 3},
        {"decimal_comma", "1,5", 3},
        {"hex_float", "0x1p2", 5},
        {"infinity", "inf", 3},
        {"negative_infinity", "-infinity", 9},
        {"nan", "nan", 3},
        {"nan_payload", "nan(123)", 8},
        {"min_normal", "2.2250738585072014e-308", 23},
        {"min_subnormal", "4.9406564584124654e-324", 23},
        {"underflow_zero", "1e-324", 6},
        {"max_finite", "1.7976931348623157e308", 22},
        {"overflow", "1.7976931348623159e308", 22},
        {"halfway_even", "1.00000000000000011102230246251565404236316680908203125", 55},
        {"trailing_junk", "1.5x", 4},
        {"embedded_nul", embedded_float, sizeof(embedded_float)}
    };
    static const struct input_case integer_bench[] = {
        {"small", "42", 2},
        {"signed", "-123456789", 10},
        {"spaced", "  +7654321 ", 12},
        {"max", "9223372036854775807", 19}
    };
    static const struct input_case float_bench[] = {
        {"half", "0.5", 3},
        {"signed", "-12345.6789", 11},
        {"scientific", "6.02214076e23", 13},
        {"max", "1.7976931348623157e308", 22}
    };
    struct input_case long_float;
    char long_text[192];
    const char *locale_name = argc > 1 ? argv[1] : "C";
    uint64_t loops = argc > 2 ? strtoull(argv[2], NULL, 10) : UINT64_C(2000000);
    size_t i;

    if (setlocale(LC_ALL, locale_name) == NULL) {
        fprintf(stderr, "cannot set locale: %s\n", locale_name);
        return 2;
    }
    printf("meta\tlocale\t%s\n", setlocale(LC_ALL, NULL));
    printf("meta\tinteger_bits\t%zu\n", sizeof(rxinteger) * 8u);
    for (i = 0; i < sizeof(integer_cases) / sizeof(integer_cases[0]); i++) {
        print_integer_case(&integer_cases[i], "current", current_integer);
        print_integer_case(&integer_cases[i], "bounded", bounded_integer);
    }
    for (i = 0; i < sizeof(float_cases) / sizeof(float_cases[0]); i++) {
        print_float_case(&float_cases[i], "current", current_float);
        print_float_case(&float_cases[i], "c_locale_copy", copied_c_locale_float);
        print_float_case(&float_cases[i], "from_chars", bounded_from_chars_double);
    }

    memset(long_text, '0', sizeof(long_text));
    long_text[0] = '0';
    long_text[1] = '.';
    long_text[2] = '5';
    long_text[sizeof(long_text) - 1] = '1';
    long_float.label = "long_fraction";
    long_float.text = long_text;
    long_float.length = sizeof(long_text);
    print_float_case(&long_float, "current", current_float);
    print_float_case(&long_float, "c_locale_copy", copied_c_locale_float);
    print_float_case(&long_float, "from_chars", bounded_from_chars_double);

    benchmark_integer("current", current_integer, integer_bench,
                      sizeof(integer_bench) / sizeof(integer_bench[0]), loops);
    benchmark_integer("bounded", bounded_integer, integer_bench,
                      sizeof(integer_bench) / sizeof(integer_bench[0]), loops);
    benchmark_float("current_short", current_float, float_bench,
                    sizeof(float_bench) / sizeof(float_bench[0]), loops);
    benchmark_float("c_locale_copy_short", copied_c_locale_float, float_bench,
                    sizeof(float_bench) / sizeof(float_bench[0]), loops);
    benchmark_float("from_chars_short", bounded_from_chars_double, float_bench,
                    sizeof(float_bench) / sizeof(float_bench[0]), loops);
    benchmark_float("current_long", current_float, &long_float, 1, loops / 4u);
    benchmark_float("c_locale_copy_long", copied_c_locale_float, &long_float, 1,
                    loops / 4u);
    benchmark_float("from_chars_long", bounded_from_chars_double, &long_float, 1,
                    loops / 4u);
    return 0;
}
