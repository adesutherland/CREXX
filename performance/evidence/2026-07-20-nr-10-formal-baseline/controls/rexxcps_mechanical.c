/*
 * Evidence-only native C ceiling for NR-10.
 *
 * This is not RexxCPS and is not a cREXX product benchmark. It measures a
 * mechanically scalar, branch-heavy loop with 1,000 nominal operations per
 * outer iteration. Runtime input and a volatile sink prevent deletion.
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static volatile uint64_t control_sink;

static double seconds_now(void) {
    struct timespec value;
    clock_gettime(CLOCK_MONOTONIC, &value);
    return (double)value.tv_sec + (double)value.tv_nsec / 1000000000.0;
}

static uint64_t kernel(uint64_t state, uint64_t outer) {
    for (uint64_t i = 0; i < outer; ++i) {
        for (uint64_t clause = 0; clause < 1000; ++clause) {
            state = state * UINT64_C(6364136223846793005) +
                    UINT64_C(1442695040888963407) + clause;
            if ((state & 7U) < 3U) {
                state ^= state >> 13;
            } else if ((state & 7U) == 7U) {
                state += (state << 7) ^ i;
            } else {
                state = (state << 9) | (state >> 55);
            }
        }
    }
    return state;
}

int main(int argc, char **argv) {
    uint64_t seed = argc > 1 ? strtoull(argv[1], NULL, 10) : UINT64_C(123456789);
    uint64_t outer = 64;
    double elapsed = 0.0;
    do {
        double started = seconds_now();
        control_sink = kernel(seed, outer);
        elapsed = seconds_now() - started;
        if (elapsed < 0.20) outer *= 2;
    } while (elapsed < 0.20);

    uint64_t measured_outer = outer * 5;
    double started = seconds_now();
    uint64_t digest = kernel(seed ^ control_sink, measured_outer);
    elapsed = seconds_now() - started;
    control_sink = digest;
    if (elapsed <= 0.0 || digest == 0) return 1;

    double rate = ((double)measured_outer * 1000.0) / elapsed;
    printf("CONTROL: mechanical scalar flow; 1000 nominal operations per iteration\n");
    printf("DIGEST: %" PRIu64 "\n", digest);
    printf("Performance: %.0f mechanical C nominal operations per second\n", rate);
    printf("PASS: NR-10 mechanical native C ceiling\n");
    return 0;
}
