/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include <errno.h>
#include <inttypes.h>

#include "rxinteger.h"

int rxinteger_parse(const char *text, char **end, rxinteger *result) {
    char *parsed_end;
    intmax_t parsed;

    if (end) *end = (char *)text;
    if (!text || !result) return 1;

    errno = 0;
    parsed_end = (char *)text;
    parsed = strtoimax(text, &parsed_end, 10);
    if (end) *end = parsed_end;
    if (parsed_end == text || errno == ERANGE ||
        parsed < (intmax_t)RXINTEGER_MIN ||
        parsed > (intmax_t)RXINTEGER_MAX) {
        return 1;
    }

    *result = (rxinteger)parsed;
    return 0;
}
