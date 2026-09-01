/*
 * DECIMAL-01 D3 attribution diagnostic: direct decNumber/decQuad arithmetic.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decNumber.h"
#include "decQuad.h"

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

static int round_quad(decQuad *number, int digits, decContext *context) {
    decQuad quantum;
    decQuad rounded;
    char quantum_text[32];
    int32_t adjusted;
    int32_t quantum_exponent;

    if (!decQuadIsFinite(number) || decQuadIsZero(number) ||
        digits >= DECQUAD_Pmax || (int)decQuadDigits(number) <= digits)
        return 0;
    adjusted = decQuadGetExponent(number) + (int32_t)decQuadDigits(number) - 1;
    quantum_exponent = adjusted - digits + 1;
    (void)snprintf(quantum_text, sizeof(quantum_text), "1E%+d",
                   quantum_exponent);
    decQuadFromString(&quantum, quantum_text, context);
    decQuadQuantize(&rounded, number, &quantum, context);
    decQuadCopy(number, &rounded);
    return context->status & DEC_Errors ? 1 : 0;
}

static int run_decquad(int digits, int common, size_t iterations) {
    decContext context;
    decQuad left;
    decQuad right;
    decQuad result;
    char checksum[DECQUAD_String];
    size_t i;

    decContextDefault(&context, DEC_INIT_DECQUAD);
    context.traps = 0;
    context.digits = DECQUAD_Pmax;
    context.clamp = common ? 1 : 0;
    context.round = common ? DEC_ROUND_HALF_EVEN : DEC_ROUND_HALF_UP;
    decQuadFromString(&left, "12345.6789", &context);
    decQuadFromString(&right, "3.125", &context);
    (void)round_quad(&left, digits, &context);
    (void)round_quad(&right, digits, &context);
    for (i = 0; i < iterations; ++i) {
        decQuadAdd(&result, &left, &right, &context);
        (void)round_quad(&result, digits, &context);
        decQuadSubtract(&result, &left, &right, &context);
        (void)round_quad(&result, digits, &context);
        decQuadMultiply(&result, &left, &right, &context);
        (void)round_quad(&result, digits, &context);
        decQuadDivide(&result, &left, &right, &context);
        (void)round_quad(&result, digits, &context);
    }
    decQuadToString(&result, checksum);
    printf("DECIMAL-CORE provider=decquad digits=%d iterations=%zu "
           "checksum=%s\n", digits, iterations, checksum);
    return context.status & DEC_Errors ? 1 : 0;
}

int main(int argc, char **argv) {
    int digits;
    int common;
    size_t iterations;

    if (argc != 5 || parse_count(argv[3], &iterations) ||
        (strcmp(argv[1], "decnumber") != 0 &&
         strcmp(argv[1], "decquad") != 0) ||
        (strcmp(argv[2], "common") != 0 &&
         strcmp(argv[2], "classic") != 0)) {
        fprintf(stderr, "FAIL: use decnumber|decquad common|classic "
                        "ITERATIONS DIGITS\n");
        return 2;
    }
    digits = atoi(argv[4]);
    common = strcmp(argv[2], "common") == 0;
    if (digits <= 0 || digits > DECQUAD_Pmax) return 2;
    if (strcmp(argv[1], "decnumber") == 0)
        return run_decnumber(digits, common, iterations);
    return run_decquad(digits, common, iterations);
}
