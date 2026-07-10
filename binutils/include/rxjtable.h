/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXJTABLE_H
#define CREXX_RXJTABLE_H

#include <stddef.h>
#include <stdint.h>

#define RX_JTABLE_ALG_LINEAR 1u
#define RX_JTABLE_ALG_OPENHASH 2u
#define RX_JTABLE_ALG_ACPH 3u

#define RX_JTABLE_HEADER_SIZE 12u

#define RX_JTABLE_LINEAR_ENTRY_SIZE 12u

#define RX_JTABLE_OPEN_HEADER_SIZE 16u
#define RX_JTABLE_OPEN_SLOT_SIZE 16u
#define RX_JTABLE_OPEN_EMPTY UINT32_MAX

#define RX_JTABLE_ACPH_HEADER_SIZE 16u
#define RX_JTABLE_ACPH_NODE_SIZE 8u
#define RX_JTABLE_ACPH_SLOT_SIZE 8u
#define RX_JTABLE_ACPH_LEAF_SIZE 12u
#define RX_JTABLE_ACPH_END_SYMBOL 256u
#define RX_JTABLE_ACPH_SYMBOL_COUNT 257u
#define RX_JTABLE_ACPH_SLOT_EMPTY 0u
#define RX_JTABLE_ACPH_SLOT_LEAF 1u
#define RX_JTABLE_ACPH_SLOT_CHILD 2u

static inline uint32_t rx_jtable_hash_bytes(const unsigned char *bytes, size_t length) {
    uint32_t hash = UINT32_C(2166136261);
    size_t i;

    for (i = 0; i < length; i++) {
        hash ^= bytes[i];
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static inline size_t rx_jtable_acph_hash(uint16_t symbol, uint8_t prime, uint16_t slot_count) {
    if (slot_count == RX_JTABLE_ACPH_SYMBOL_COUNT) return symbol;
    return ((((size_t)prime - 1u) ^ (size_t)symbol) * (size_t)prime) % slot_count;
}

static inline unsigned char rx_jtable_select_auto(size_t case_count, size_t total_key_length) {
    size_t whole_average;
    size_t remainder;

    if (case_count <= 1u) return RX_JTABLE_ALG_LINEAR;
    whole_average = total_key_length / case_count;
    remainder = total_key_length % case_count;
    if (whole_average < 2u ||
        (whole_average == 2u && remainder == 0u)) {
        return RX_JTABLE_ALG_OPENHASH;
    }
    if (case_count >= 256u &&
        (whole_average < 4u || (whole_average == 4u && remainder == 0u))) {
        return RX_JTABLE_ALG_OPENHASH;
    }
    return RX_JTABLE_ALG_ACPH;
}

#endif
