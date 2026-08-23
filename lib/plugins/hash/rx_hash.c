/*
 * cREXX License (MIT)
 *
 * Binary digest, checksum, and named non-cryptographic hash functions supplied
 * by the standard rx_hash provider.
 */

#include <stddef.h>
#include <stdint.h>

#include "crexxpa.h"
#include "rxsha256.h"

RXPA_PLUGIN_PROCESS_REENTRANT

static int rxhash_payload(rxpa_attribute_value value,
                          const unsigned char **data, size_t *length)
{
    const void *payload;

    *length = 0u;
    payload = GETNATIVEPAYLOAD(value, length, NULL, NULL);
    if (!payload && *length != 0u) return 0;
    *data = (const unsigned char *)payload;
    return 1;
}

PROCEDURE(sha256)
{
    const unsigned char *data;
    size_t length;
    unsigned char digest[32];

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    if (!rxhash_payload(ARG0, &data, &length))
        RETURNSIGNAL(SIGNAL_FAILURE, "binary payload is unavailable")

    rx_sha256(data, length, digest);
    if (SETNATIVEPAYLOAD(RETURN, digest, sizeof(digest), NULL, 0u) != 0)
        RETURNSIGNAL(SIGNAL_FAILURE, "SHA-256 digest allocation failed")
    RESETSIGNAL
}

PROCEDURE(djb2)
{
    const unsigned char *data;
    size_t length;
    size_t i;
    uint32_t hash = UINT32_C(5381);

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    if (!rxhash_payload(ARG0, &data, &length))
        RETURNSIGNAL(SIGNAL_FAILURE, "binary payload is unavailable")
    for (i = 0; i < length; ++i) hash = hash * UINT32_C(33) + data[i];
    RETURNINT((rxinteger)hash);
    RESETSIGNAL
}

static uint32_t rxhash_rotl32(uint32_t value, unsigned int bits)
{
    return (value << bits) | (value >> (32u - bits));
}

PROCEDURE(murmur3)
{
    const unsigned char *data;
    size_t length;
    size_t blocks;
    size_t i;
    uint32_t hash;
    uint32_t tail = 0u;

    if (NUM_ARGS != 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    if (!rxhash_payload(ARG0, &data, &length))
        RETURNSIGNAL(SIGNAL_FAILURE, "binary payload is unavailable")

    hash = (uint32_t)GETINT(ARG1);
    blocks = length / 4u;
    for (i = 0; i < blocks; ++i) {
        const unsigned char *block = data + i * 4u;
        uint32_t value = (uint32_t)block[0] |
                         ((uint32_t)block[1] << 8) |
                         ((uint32_t)block[2] << 16) |
                         ((uint32_t)block[3] << 24);
        value *= UINT32_C(0xcc9e2d51);
        value = rxhash_rotl32(value, 15u);
        value *= UINT32_C(0x1b873593);
        hash ^= value;
        hash = rxhash_rotl32(hash, 13u);
        hash = hash * UINT32_C(5) + UINT32_C(0xe6546b64);
    }

    data += blocks * 4u;
    switch (length & 3u) {
        case 3u: tail ^= (uint32_t)data[2] << 16; /* fall through */
        case 2u: tail ^= (uint32_t)data[1] << 8;  /* fall through */
        case 1u:
            tail ^= data[0];
            tail *= UINT32_C(0xcc9e2d51);
            tail = rxhash_rotl32(tail, 15u);
            tail *= UINT32_C(0x1b873593);
            hash ^= tail;
            break;
        default:
            break;
    }

    hash ^= (uint32_t)length;
    hash ^= hash >> 16;
    hash *= UINT32_C(0x85ebca6b);
    hash ^= hash >> 13;
    hash *= UINT32_C(0xc2b2ae35);
    hash ^= hash >> 16;
    RETURNINT((rxinteger)hash);
    RESETSIGNAL
}

PROCEDURE(fnv1a)
{
    const unsigned char *data;
    size_t length;
    size_t i;
    uint32_t hash = UINT32_C(2166136261);

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    if (!rxhash_payload(ARG0, &data, &length))
        RETURNSIGNAL(SIGNAL_FAILURE, "binary payload is unavailable")
    for (i = 0; i < length; ++i) {
        hash ^= data[i];
        hash *= UINT32_C(16777619);
    }
    RETURNINT((rxinteger)hash);
    RESETSIGNAL
}

PROCEDURE(crc32)
{
    const unsigned char *data;
    size_t length;
    size_t i;
    uint32_t hash = UINT32_C(0xffffffff);

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    if (!rxhash_payload(ARG0, &data, &length))
        RETURNSIGNAL(SIGNAL_FAILURE, "binary payload is unavailable")
    for (i = 0; i < length; ++i) {
        unsigned int bit;
        hash ^= data[i];
        for (bit = 0u; bit < 8u; ++bit) {
            uint32_t mask = (uint32_t)-(int32_t)(hash & 1u);
            hash = (hash >> 1) ^ (UINT32_C(0xedb88320) & mask);
        }
    }
    RETURNINT((rxinteger)(~hash));
    RESETSIGNAL
}

LOADFUNCS
ADDPROC(sha256, "rxhash.sha256", "b", ".binary", "data = .binary");
ADDPROC(djb2, "rxhash.djb2", "b", ".int", "data = .binary");
ADDPROC(murmur3, "rxhash.murmur3", "b", ".int", "data = .binary,seed = .int");
ADDPROC(fnv1a, "rxhash.fnv1a", "b", ".int", "data = .binary");
ADDPROC(crc32, "rxhash.crc32", "b", ".int", "data = .binary");
ENDLOADFUNCS
