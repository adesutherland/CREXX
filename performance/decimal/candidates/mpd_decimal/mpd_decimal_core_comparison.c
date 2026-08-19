/*
 * DECIMAL-01 attribution diagnostic: call the two arithmetic libraries
 * directly, without the cREXX value or plugin adapter.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decNumber.h"
#include "mpdecimal.h"

static int parse_count(const char *text, size_t *result) {
    char *end = NULL;
    unsigned long parsed;
    errno = 0;
    parsed = strtoul(text, &end, 10);
    if (errno || end == text || *end || parsed == 0u) return 1;
    *result = (size_t)parsed;
    return (unsigned long)*result != parsed;
}

static int run_decnumber(int digits, int common, size_t iterations) {
    decContext context;
    decNumber left;
    decNumber right;
    decNumber result;
    char checksum[128];
    size_t i;

    decContextDefault(&context, DEC_INIT_BASE);
    context.traps = 0;
    context.digits = digits;
    context.clamp = common ? 1 : 0;
    context.round = common ? DEC_ROUND_HALF_EVEN : DEC_ROUND_HALF_UP;
    decNumberFromString(&left, "12345.6789", &context);
    decNumberFromString(&right, "3.125", &context);
    for (i = 0; i < iterations; ++i) {
        decNumberAdd(&result, &left, &right, &context);
        decNumberSubtract(&result, &left, &right, &context);
        decNumberMultiply(&result, &left, &right, &context);
        decNumberDivide(&result, &left, &right, &context);
    }
    decNumberToString(&result, checksum);
    printf("DECIMAL-CORE provider=decnumber digits=%d iterations=%zu "
           "checksum=%s\n", digits, iterations, checksum);
    return context.status & DEC_Errors ? 1 : 0;
}

static void init_mpd(mpd_t *number, mpd_uint_t *data, mpd_ssize_t words) {
    memset(data, 0, (size_t)words * sizeof(*data));
    number->flags = MPD_STATIC | MPD_STATIC_DATA;
    number->exp = 0;
    number->digits = 1;
    number->len = 1;
    number->alloc = words;
    number->data = data;
}

static int run_libmpdec(int digits, int common, size_t iterations) {
    enum { MAX_WORDS = 8 };
    mpd_context_t context;
    mpd_uint_t left_data[MAX_WORDS];
    mpd_uint_t right_data[MAX_WORDS];
    mpd_uint_t result_data[MAX_WORDS];
    mpd_t left;
    mpd_t right;
    mpd_t result;
    mpd_ssize_t words = (mpd_ssize_t)
        (((size_t)digits * 2u + 2u + (size_t)MPD_RDIGITS - 1u) /
         (size_t)MPD_RDIGITS);
    uint32_t status = 0;
    char *checksum;
    size_t i;

    if (words < MPD_MINALLOC) words = MPD_MINALLOC;
    if (words > MAX_WORDS) return 2;
    mpd_defaultcontext(&context);
    context.prec = digits;
    context.emax = 999999999;
    context.emin = -999999999;
    context.traps = 0;
    context.clamp = common ? MPD_CLAMP_IEEE_754 : MPD_CLAMP_DEFAULT;
    context.round = common ? MPD_ROUND_HALF_EVEN : MPD_ROUND_HALF_UP;
    init_mpd(&left, left_data, words);
    init_mpd(&right, right_data, words);
    init_mpd(&result, result_data, words);
    mpd_qset_string(&left, "12345.6789", &context, &status);
    mpd_qset_string(&right, "3.125", &context, &status);
    for (i = 0; i < iterations; ++i) {
        mpd_qadd(&result, &left, &right, &context, &status);
        mpd_qsub(&result, &left, &right, &context, &status);
        mpd_qmul(&result, &left, &right, &context, &status);
        mpd_qdiv(&result, &left, &right, &context, &status);
    }
    checksum = mpd_to_sci(&result, 0);
    if (!checksum) return 1;
    printf("DECIMAL-CORE provider=libmpdec digits=%d iterations=%zu "
           "checksum=%s\n", digits, iterations, checksum);
    mpd_free(checksum);
    return status & (MPD_Errors | MPD_Overflow | MPD_Underflow) ? 1 : 0;
}

int main(int argc, char **argv) {
    int digits;
    int common;
    size_t iterations;

    if (argc != 5 || parse_count(argv[3], &iterations) ||
        (strcmp(argv[1], "decnumber") != 0 &&
         strcmp(argv[1], "libmpdec") != 0) ||
        (strcmp(argv[2], "common") != 0 &&
         strcmp(argv[2], "classic") != 0)) {
        fprintf(stderr, "FAIL: use decnumber|libmpdec common|classic "
                        "ITERATIONS DIGITS\n");
        return 2;
    }
    digits = atoi(argv[4]);
    common = strcmp(argv[2], "common") == 0;
    if (digits <= 0) return 2;
    if (strcmp(argv[1], "decnumber") == 0)
        return run_decnumber(digits, common, iterations);
    return run_libmpdec(digits, common, iterations);
}
