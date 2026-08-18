/*
 * cREXX License (MIT)
 *
 * Compact SHA-256 used for sealed semantic-graph and callable contracts.
 */

#include <stdint.h>
#include <string.h>
#include "rxsha256.h"

typedef struct rx_sha256_state {
    uint32_t h[8];
    uint64_t bytes;
    unsigned char block[64];
    size_t used;
} rx_sha256_state;

static uint32_t rx_sha_rotr(uint32_t value, unsigned int count) {
    return (value >> count) | (value << (32u - count));
}

static uint32_t rx_sha_load32(const unsigned char *source) {
    return ((uint32_t)source[0] << 24) |
           ((uint32_t)source[1] << 16) |
           ((uint32_t)source[2] << 8) |
           (uint32_t)source[3];
}

static void rx_sha_store32(unsigned char *target, uint32_t value) {
    target[0] = (unsigned char)(value >> 24);
    target[1] = (unsigned char)(value >> 16);
    target[2] = (unsigned char)(value >> 8);
    target[3] = (unsigned char)value;
}

static void rx_sha256_transform(rx_sha256_state *state,
                                const unsigned char block[64]) {
    static const uint32_t constants[64] = {
        UINT32_C(0x428a2f98), UINT32_C(0x71374491), UINT32_C(0xb5c0fbcf), UINT32_C(0xe9b5dba5),
        UINT32_C(0x3956c25b), UINT32_C(0x59f111f1), UINT32_C(0x923f82a4), UINT32_C(0xab1c5ed5),
        UINT32_C(0xd807aa98), UINT32_C(0x12835b01), UINT32_C(0x243185be), UINT32_C(0x550c7dc3),
        UINT32_C(0x72be5d74), UINT32_C(0x80deb1fe), UINT32_C(0x9bdc06a7), UINT32_C(0xc19bf174),
        UINT32_C(0xe49b69c1), UINT32_C(0xefbe4786), UINT32_C(0x0fc19dc6), UINT32_C(0x240ca1cc),
        UINT32_C(0x2de92c6f), UINT32_C(0x4a7484aa), UINT32_C(0x5cb0a9dc), UINT32_C(0x76f988da),
        UINT32_C(0x983e5152), UINT32_C(0xa831c66d), UINT32_C(0xb00327c8), UINT32_C(0xbf597fc7),
        UINT32_C(0xc6e00bf3), UINT32_C(0xd5a79147), UINT32_C(0x06ca6351), UINT32_C(0x14292967),
        UINT32_C(0x27b70a85), UINT32_C(0x2e1b2138), UINT32_C(0x4d2c6dfc), UINT32_C(0x53380d13),
        UINT32_C(0x650a7354), UINT32_C(0x766a0abb), UINT32_C(0x81c2c92e), UINT32_C(0x92722c85),
        UINT32_C(0xa2bfe8a1), UINT32_C(0xa81a664b), UINT32_C(0xc24b8b70), UINT32_C(0xc76c51a3),
        UINT32_C(0xd192e819), UINT32_C(0xd6990624), UINT32_C(0xf40e3585), UINT32_C(0x106aa070),
        UINT32_C(0x19a4c116), UINT32_C(0x1e376c08), UINT32_C(0x2748774c), UINT32_C(0x34b0bcb5),
        UINT32_C(0x391c0cb3), UINT32_C(0x4ed8aa4a), UINT32_C(0x5b9cca4f), UINT32_C(0x682e6ff3),
        UINT32_C(0x748f82ee), UINT32_C(0x78a5636f), UINT32_C(0x84c87814), UINT32_C(0x8cc70208),
        UINT32_C(0x90befffa), UINT32_C(0xa4506ceb), UINT32_C(0xbef9a3f7), UINT32_C(0xc67178f2)
    };
    uint32_t words[64];
    uint32_t a, b, c, d, e, f, g, h;
    size_t index;

    for (index = 0u; index < 16u; index++) {
        words[index] = rx_sha_load32(block + index * 4u);
    }
    for (; index < 64u; index++) {
        uint32_t s0 = rx_sha_rotr(words[index - 15u], 7u) ^
                      rx_sha_rotr(words[index - 15u], 18u) ^
                      (words[index - 15u] >> 3u);
        uint32_t s1 = rx_sha_rotr(words[index - 2u], 17u) ^
                      rx_sha_rotr(words[index - 2u], 19u) ^
                      (words[index - 2u] >> 10u);
        words[index] = words[index - 16u] + s0 +
                       words[index - 7u] + s1;
    }

    a = state->h[0]; b = state->h[1]; c = state->h[2]; d = state->h[3];
    e = state->h[4]; f = state->h[5]; g = state->h[6]; h = state->h[7];
    for (index = 0u; index < 64u; index++) {
        uint32_t sum1 = rx_sha_rotr(e, 6u) ^ rx_sha_rotr(e, 11u) ^ rx_sha_rotr(e, 25u);
        uint32_t choose = (e & f) ^ ((~e) & g);
        uint32_t temp1 = h + sum1 + choose + constants[index] + words[index];
        uint32_t sum0 = rx_sha_rotr(a, 2u) ^ rx_sha_rotr(a, 13u) ^ rx_sha_rotr(a, 22u);
        uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = sum0 + majority;
        h = g; g = f; f = e; e = d + temp1;
        d = c; c = b; b = a; a = temp1 + temp2;
    }
    state->h[0] += a; state->h[1] += b; state->h[2] += c; state->h[3] += d;
    state->h[4] += e; state->h[5] += f; state->h[6] += g; state->h[7] += h;
}

static void rx_sha256_update(rx_sha256_state *state,
                             const unsigned char *data,
                             size_t length) {
    state->bytes += (uint64_t)length;
    while (length) {
        size_t space = 64u - state->used;
        size_t take = length < space ? length : space;
        memcpy(state->block + state->used, data, take);
        state->used += take;
        data += take;
        length -= take;
        if (state->used == 64u) {
            rx_sha256_transform(state, state->block);
            state->used = 0u;
        }
    }
}

void rx_sha256(const void *data, size_t length, unsigned char digest[32]) {
    rx_sha256_state state;
    uint64_t bits;
    size_t index;

    state.h[0] = UINT32_C(0x6a09e667); state.h[1] = UINT32_C(0xbb67ae85);
    state.h[2] = UINT32_C(0x3c6ef372); state.h[3] = UINT32_C(0xa54ff53a);
    state.h[4] = UINT32_C(0x510e527f); state.h[5] = UINT32_C(0x9b05688c);
    state.h[6] = UINT32_C(0x1f83d9ab); state.h[7] = UINT32_C(0x5be0cd19);
    state.bytes = 0u;
    state.used = 0u;
    if (length) rx_sha256_update(&state, (const unsigned char *)data, length);
    bits = state.bytes * UINT64_C(8);
    state.block[state.used++] = 0x80u;
    if (state.used > 56u) {
        memset(state.block + state.used, 0, 64u - state.used);
        rx_sha256_transform(&state, state.block);
        state.used = 0u;
    }
    memset(state.block + state.used, 0, 56u - state.used);
    for (index = 0u; index < 8u; index++) {
        state.block[63u - index] = (unsigned char)(bits >> (index * 8u));
    }
    rx_sha256_transform(&state, state.block);
    for (index = 0u; index < 8u; index++) {
        rx_sha_store32(digest + index * 4u, state.h[index]);
    }
}
