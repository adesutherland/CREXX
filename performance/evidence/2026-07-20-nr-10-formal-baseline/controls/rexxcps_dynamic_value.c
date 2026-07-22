/*
 * Evidence-only native C ceiling for NR-10.
 *
 * This is not RexxCPS and is not a cREXX product benchmark. It models a small
 * tagged dynamic value with integer, text, conversion and comparison paths.
 * Runtime input plus a volatile sink keeps the measured result observable.
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum { VALUE_INTEGER, VALUE_TEXT } value_tag;
typedef struct {
    value_tag tag;
    uint64_t integer;
    char text[32];
} dynamic_value;

static volatile uint64_t control_sink;

static double seconds_now(void) {
    struct timespec value;
    clock_gettime(CLOCK_MONOTONIC, &value);
    return (double)value.tv_sec + (double)value.tv_nsec / 1000000000.0;
}

__attribute__((noinline)) static void set_integer(dynamic_value *value, uint64_t integer) {
    value->tag = VALUE_INTEGER;
    value->integer = integer;
}

__attribute__((noinline)) static void to_text(dynamic_value *value) {
    if (value->tag == VALUE_INTEGER) {
        snprintf(value->text, sizeof(value->text), "%" PRIu64, value->integer);
        value->tag = VALUE_TEXT;
    }
}

__attribute__((noinline)) static uint64_t to_integer(dynamic_value *value) {
    if (value->tag == VALUE_TEXT) {
        value->integer = strtoull(value->text, NULL, 10);
        value->tag = VALUE_INTEGER;
    }
    return value->integer;
}

static uint64_t kernel(uint64_t seed, uint64_t outer) {
    dynamic_value left;
    dynamic_value right;
    set_integer(&left, seed);
    set_integer(&right, seed ^ UINT64_C(0x9e3779b97f4a7c15));
    for (uint64_t i = 0; i < outer; ++i) {
        for (uint64_t clause = 0; clause < 1000; ++clause) {
            switch (clause % 5U) {
                case 0:
                    set_integer(&left, to_integer(&left) + to_integer(&right) + clause);
                    break;
                case 1:
                    to_text(&left);
                    break;
                case 2:
                    set_integer(&right, to_integer(&left) ^ (i + clause));
                    break;
                case 3:
                    to_text(&right);
                    if (strcmp(left.text, right.text) == 0) right.text[0] = '1';
                    break;
                default:
                    set_integer(&left, to_integer(&left) * UINT64_C(33) + to_integer(&right));
                    break;
            }
        }
    }
    return to_integer(&left) ^ to_integer(&right);
}

int main(int argc, char **argv) {
    uint64_t seed = argc > 1 ? strtoull(argv[1], NULL, 10) : UINT64_C(123456789);
    uint64_t outer = 4;
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
    printf("CONTROL: tagged dynamic value; 1000 nominal operations per iteration\n");
    printf("DIGEST: %" PRIu64 "\n", digest);
    printf("Performance: %.0f dynamic-value C nominal operations per second\n", rate);
    printf("PASS: NR-10 dynamic-value native C ceiling\n");
    return 0;
}
