/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include <stdio.h>
#include "rxjtable.h"

static int expect_algorithm(size_t cases, size_t bytes, unsigned char expected) {
    unsigned char actual = rx_jtable_select_auto(cases, bytes);
    if (actual == expected) return 0;
    fprintf(stderr, "auto(%lu cases, %lu bytes) returned %u, expected %u\n",
            (unsigned long)cases, (unsigned long)bytes,
            (unsigned int)actual, (unsigned int)expected);
    return 1;
}

int main(void) {
    int failures = 0;

    failures += expect_algorithm(1, 0, RX_JTABLE_ALG_LINEAR);
    failures += expect_algorithm(1, 1024, RX_JTABLE_ALG_LINEAR);
    failures += expect_algorithm(2, 0, RX_JTABLE_ALG_OPENHASH);
    failures += expect_algorithm(2, 4, RX_JTABLE_ALG_OPENHASH);
    failures += expect_algorithm(2, 5, RX_JTABLE_ALG_ACPH);
    failures += expect_algorithm(3, 6, RX_JTABLE_ALG_OPENHASH);
    failures += expect_algorithm(3, 7, RX_JTABLE_ALG_ACPH);
    failures += expect_algorithm(255, 1020, RX_JTABLE_ALG_ACPH);
    failures += expect_algorithm(256, 768, RX_JTABLE_ALG_OPENHASH);
    failures += expect_algorithm(256, 1024, RX_JTABLE_ALG_OPENHASH);
    failures += expect_algorithm(256, 1025, RX_JTABLE_ALG_ACPH);
    failures += expect_algorithm(512, 4096, RX_JTABLE_ALG_ACPH);
    return failures != 0;
}
