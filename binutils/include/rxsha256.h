/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXSHA256_H
#define CREXX_RXSHA256_H

#include <stddef.h>

void rx_sha256(const void *data, size_t length, unsigned char digest[32]);

#endif
