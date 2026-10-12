/* cREXX License (MIT), Copyright (c) 2026 cREXX contributors */
#ifndef CREXX_RXFLOATFORMAT_H
#define CREXX_RXFLOATFORMAT_H

#include <stdint.h>
#include <string.h>
#include <math.h>

/* inf is an infinity. nan without a payload names the canonical quiet NaN.
 * nan(IEEE-bits) preserves every NaN bit without changing opcode families. */
static inline int rx_double_special_literal(double number, char literal[22]) {
    static const char hex[] = "0123456789abcdef";
    uint64_t bits;
    size_t i;
    memcpy(&bits, &number, sizeof(bits));
    if (isfinite(number)) return 0;
    if (isinf(number)) {
        strcpy(literal, signbit(number) ? "-inf" : "inf");
        return 1;
    }
    if (bits == UINT64_C(0x7ff8000000000000)) {
        strcpy(literal, "nan");
        return 1;
    }
    memcpy(literal, "nan(", 4);
    for (i = 0; i < 16; ++i)
        literal[4 + i] = hex[(bits >> (60u - i * 4u)) & 15u];
    literal[20] = ')';
    literal[21] = 0;
    return 1;
}

/* Return 1 for a special literal, 0 for other text, -1 for invalid NaN bits.
 * A leading NaN sign is admitted for existing C-style inline metadata; RXAS
 * emits the explicit bit form whenever that sign is significant. */
static inline int rx_double_from_special_literal(const char *text, size_t length, double *number) {
    uint64_t bits = 0;
    size_t i;
    int negative = 0;
    if (!text || !number) return 0;
    if (length && (*text == '-' || *text == '+')) {
        negative = *text == '-';
        ++text; --length;
    }
    if (length == 3 && (text[0] == 'i' || text[0] == 'I') &&
        (text[1] == 'n' || text[1] == 'N') && (text[2] == 'f' || text[2] == 'F')) {
        bits = UINT64_C(0x7ff0000000000000);
        if (negative) bits |= UINT64_C(0x8000000000000000);
    }
    else if (length >= 3 && (text[0] == 'n' || text[0] == 'N') &&
             (text[1] == 'a' || text[1] == 'A') && (text[2] == 'n' || text[2] == 'N')) {
        if (length == 3) {
            bits = UINT64_C(0x7ff8000000000000);
            if (negative) bits |= UINT64_C(0x8000000000000000);
        }
        else {
            if (length != 21 || text[3] != '(' || text[20] != ')') return -1;
            for (i = 4; i < 20; ++i) {
                unsigned int digit;
                char c = text[i];
                if (c >= '0' && c <= '9') digit = (unsigned int)(c - '0');
                else if (c >= 'a' && c <= 'f') digit = (unsigned int)(c - 'a' + 10);
                else if (c >= 'A' && c <= 'F') digit = (unsigned int)(c - 'A' + 10);
                else return -1;
                bits = (bits << 4u) | digit;
            }
            if ((bits & UINT64_C(0x7ff0000000000000)) != UINT64_C(0x7ff0000000000000) ||
                !(bits & UINT64_C(0x000fffffffffffff))) return -1;
        }
    }
    else return 0;
    memcpy(number, &bits, sizeof(bits));
    return 1;
}

static inline int rx_decimal_special_literal(const char *text) {
    char lower[9];
    size_t i;
    if (!text) return 0;
    if (*text == '-' || *text == '+') ++text;
    for (i = 0; text[i] && i < sizeof(lower) - 1; ++i)
        lower[i] = text[i] >= 'A' && text[i] <= 'Z' ? text[i] + ('a' - 'A') : text[i];
    lower[i] = 0;
    return !text[i] && (!strcmp(lower, "nan") || !strcmp(lower, "inf") || !strcmp(lower, "infinity"));
}

#endif
