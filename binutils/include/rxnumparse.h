/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXNUMPARSE_H
#define CREXX_RXNUMPARSE_H

#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define RX_NUMERIC_KEY_SIZE 8u
#define RX_NUMERIC_NAN_BITS UINT64_C(0x7ff8000000000000)

static inline int rx_string_to_double(double *out, const char *string, size_t length) {
    char local[128];
    char *buffer = local;
    char *end;
    double value;
    int rc = 1;

    if (!out || !string || length == SIZE_MAX) return 1;
    if (length + 1u > sizeof(local)) {
        buffer = malloc(length + 1u);
        if (!buffer) return 1;
    }
    memcpy(buffer, string, length);
    buffer[length] = 0;

    errno = 0;
    end = buffer;
    value = strtod(buffer, &end);
    if (errno != ERANGE && end != buffer) {
        while (*end != 0 && isspace((unsigned char)*end)) {
            end++;
        }
        if (*end == 0) {
            *out = value;
            rc = 0;
        }
    }

    if (buffer != local) free(buffer);
    return rc;
}

static inline void rx_double_to_numeric_key(double value, unsigned char key[RX_NUMERIC_KEY_SIZE]) {
    uint64_t bits;
    size_t i;

    if (isnan(value)) bits = RX_NUMERIC_NAN_BITS;
    else {
        if (value == 0.0) value = 0.0;
        memcpy(&bits, &value, sizeof(bits));
    }
    for (i = 0; i < RX_NUMERIC_KEY_SIZE; i++) {
        key[i] = (unsigned char)((bits >> (i * 8u)) & UINT64_C(0xff));
    }
}

static inline int rx_numeric_key_from_text(unsigned char key[RX_NUMERIC_KEY_SIZE],
                                           const char *string,
                                           size_t length,
                                           int *is_nan_out) {
    double value;

    if (rx_string_to_double(&value, string, length)) return 0;
    if (is_nan_out) *is_nan_out = isnan(value) != 0;
    rx_double_to_numeric_key(value, key);
    return 1;
}

#endif
