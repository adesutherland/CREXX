/* Direct-C ceiling for the CREXXRAG-SHA256 A-versus-D gate. */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "rxsha256.h"

static uint64_t now_ns(void) {
#ifdef _WIN32
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;
    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&frequency);
    return (uint64_t)(counter.QuadPart / frequency.QuadPart) * UINT64_C(1000000000) +
           (uint64_t)(counter.QuadPart % frequency.QuadPart) * UINT64_C(1000000000) /
               (uint64_t)frequency.QuadPart;
#else
    struct timespec value;
    if (clock_gettime(CLOCK_MONOTONIC, &value) != 0) return 0u;
    return (uint64_t)value.tv_sec * UINT64_C(1000000000) +
           (uint64_t)value.tv_nsec;
#endif
}
static void digest_hex(const unsigned char digest[32], char result[65]) {
    static const char digits[] = "0123456789ABCDEF";
    size_t index;

    for (index = 0u; index < 32u; index++) {
        result[index * 2u] = digits[digest[index] >> 4u];
        result[index * 2u + 1u] = digits[digest[index] & 15u];
    }
    result[64] = '\0';
}

static int parse_positive(const char *text, size_t *value) {
    char *end;
    uintmax_t parsed;

    if (!text || !*text || text[0] == '-') return -1;
    end = NULL;
    parsed = strtoumax(text, &end, 10);
    if (!end || *end != '\0' || parsed == 0u || parsed > SIZE_MAX) return -1;
    *value = (size_t)parsed;
    return 0;
}

int main(int argc, char **argv) {
    unsigned char *data;
    unsigned char digest[32];
    char hex[65];
    size_t bytes;
    size_t iterations;
    size_t index;
    size_t iteration;
    uint64_t started;
    uint64_t elapsed;

    if (argc != 3 || parse_positive(argv[1], &bytes) != 0 ||
        parse_positive(argv[2], &iterations) != 0) {
        fprintf(stderr, "usage: %s BYTES ITERATIONS\n", argv[0]);
        return 2;
    }

    data = (unsigned char *)malloc(bytes);
    if (!data) {
        fprintf(stderr, "FAIL: direct-C input allocation\n");
        return 1;
    }
    for (index = 0u; index < bytes; index++)
        data[index] = (unsigned char)((index * 31u + 7u) % 256u);

    started = now_ns();
    for (iteration = 0u; iteration < iterations; iteration++)
        rx_sha256(data, bytes, digest);
    elapsed = now_ns() - started;

    digest_hex(digest, hex);
    printf("benchmark=crexxrag_sha256 variant=direct_c payload_bytes=%zu "
           "iterations=%zu elapsed_us=%" PRIu64 " digest=%s\n",
           bytes, iterations, elapsed / UINT64_C(1000), hex);
    printf("PASS: CREXXRAG-SHA256 direct-C ceiling\n");
    free(data);
    return 0;
}
