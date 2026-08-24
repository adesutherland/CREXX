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

static void rxhash_digest_hex(const unsigned char digest[RX_SHA256_DIGEST_SIZE],
                              char hex[RX_SHA256_DIGEST_SIZE * 2u + 1u])
{
    static const char digits[] = "0123456789abcdef";
    size_t i;

    for (i = 0u; i < RX_SHA256_DIGEST_SIZE; ++i) {
        hex[i * 2u] = digits[digest[i] >> 4u];
        hex[i * 2u + 1u] = digits[digest[i] & 0x0fu];
    }
    hex[RX_SHA256_DIGEST_SIZE * 2u] = '\0';
}

static const char *rxhash_state_error(rx_sha256_state_status status)
{
    switch (status) {
        case RX_SHA256_STATE_INVALID_LENGTH:
            return "RXHASH SHA-256 state has invalid length";
        case RX_SHA256_STATE_INVALID_MAGIC:
            return "RXHASH SHA-256 state has invalid magic";
        case RX_SHA256_STATE_UNSUPPORTED_VERSION:
            return "RXHASH SHA-256 state version is unsupported";
        case RX_SHA256_STATE_INVALID_FORMAT:
            return "RXHASH SHA-256 state has invalid format";
        case RX_SHA256_STATE_INCONSISTENT:
            return "RXHASH SHA-256 state is internally inconsistent";
        case RX_SHA256_STATE_INTEGRITY_FAILURE:
            return "RXHASH SHA-256 state integrity check failed";
        default:
            return "RXHASH SHA-256 state is invalid";
    }
}

static const char *rxhash_file_error(rx_sha256_file_status status)
{
    switch (status) {
        case RX_SHA256_FILE_OPEN_FAILURE:
            return "RXHASH SHA-256 file open failed";
        case RX_SHA256_FILE_READ_FAILURE:
            return "RXHASH SHA-256 file read failed";
        case RX_SHA256_FILE_CLOSE_FAILURE:
            return "RXHASH SHA-256 file close failed";
        default:
            return "RXHASH SHA-256 file hashing failed";
    }
}

PROCEDURE(sha256)
{
    const unsigned char *data;
    size_t length;
    unsigned char digest[RX_SHA256_DIGEST_SIZE];

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    if (!rxhash_payload(ARG0, &data, &length))
        RETURNSIGNAL(SIGNAL_FAILURE, "binary payload is unavailable")

    rx_sha256(data, length, digest);
    if (SETNATIVEPAYLOAD(RETURN, digest, sizeof(digest), NULL, 0u) != 0)
        RETURNSIGNAL(SIGNAL_FAILURE, "SHA-256 digest allocation failed")
    RESETSIGNAL
}

PROCEDURE(sha256hex)
{
    const unsigned char *data;
    size_t length;
    unsigned char digest[RX_SHA256_DIGEST_SIZE];
    char hex[RX_SHA256_DIGEST_SIZE * 2u + 1u];

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    if (!rxhash_payload(ARG0, &data, &length))
        RETURNSIGNAL(SIGNAL_FAILURE, "binary payload is unavailable")

    rx_sha256(data, length, digest);
    rxhash_digest_hex(digest, hex);
    RETURNSTR(hex);
    RESETSIGNAL
}

PROCEDURE(sha256init)
{
    rx_sha256_context context;
    unsigned char state[RX_SHA256_SERIALIZED_STATE_SIZE];

    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "no arguments expected")
    rx_sha256_init(&context);
    if (rx_sha256_export_state(&context, state) != RX_SHA256_STATE_OK)
        RETURNSIGNAL(SIGNAL_FAILURE, "SHA-256 state initialization failed")
    if (SETNATIVEPAYLOAD(RETURN, state, sizeof(state), NULL, 0u) != 0)
        RETURNSIGNAL(SIGNAL_FAILURE, "SHA-256 state allocation failed")
    RESETSIGNAL
}

PROCEDURE(sha256update)
{
    const unsigned char *state_data;
    const unsigned char *data;
    size_t state_length;
    size_t length;
    rx_sha256_context context;
    rx_sha256_state_status status;
    unsigned char state[RX_SHA256_SERIALIZED_STATE_SIZE];

    if (NUM_ARGS != 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    if (!rxhash_payload(ARG0, &state_data, &state_length) ||
        !rxhash_payload(ARG1, &data, &length))
        RETURNSIGNAL(SIGNAL_FAILURE, "binary payload is unavailable")
    status = rx_sha256_import_state(&context, state_data, state_length);
    if (status != RX_SHA256_STATE_OK)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, rxhash_state_error(status))
    if (!rx_sha256_update(&context, data, length))
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXHASH.SHA256UPDATE exceeds the SHA-256 length limit")
    status = rx_sha256_export_state(&context, state);
    if (status != RX_SHA256_STATE_OK)
        RETURNSIGNAL(SIGNAL_FAILURE, "SHA-256 state serialization failed")
    if (SETNATIVEPAYLOAD(RETURN, state, sizeof(state), NULL, 0u) != 0)
        RETURNSIGNAL(SIGNAL_FAILURE, "SHA-256 state allocation failed")
    RESETSIGNAL
}

PROCEDURE(sha256final)
{
    const unsigned char *state_data;
    size_t state_length;
    rx_sha256_context context;
    rx_sha256_state_status status;
    unsigned char digest[RX_SHA256_DIGEST_SIZE];

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    if (!rxhash_payload(ARG0, &state_data, &state_length))
        RETURNSIGNAL(SIGNAL_FAILURE, "binary payload is unavailable")
    status = rx_sha256_import_state(&context, state_data, state_length);
    if (status != RX_SHA256_STATE_OK)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, rxhash_state_error(status))
    rx_sha256_final(&context, digest);
    if (SETNATIVEPAYLOAD(RETURN, digest, sizeof(digest), NULL, 0u) != 0)
        RETURNSIGNAL(SIGNAL_FAILURE, "SHA-256 digest allocation failed")
    RESETSIGNAL
}

PROCEDURE(sha256finalhex)
{
    const unsigned char *state_data;
    size_t state_length;
    rx_sha256_context context;
    rx_sha256_state_status status;
    unsigned char digest[RX_SHA256_DIGEST_SIZE];
    char hex[RX_SHA256_DIGEST_SIZE * 2u + 1u];

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    if (!rxhash_payload(ARG0, &state_data, &state_length))
        RETURNSIGNAL(SIGNAL_FAILURE, "binary payload is unavailable")
    status = rx_sha256_import_state(&context, state_data, state_length);
    if (status != RX_SHA256_STATE_OK)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, rxhash_state_error(status))
    rx_sha256_final(&context, digest);
    rxhash_digest_hex(digest, hex);
    RETURNSTR(hex);
    RESETSIGNAL
}

PROCEDURE(sha256file)
{
    const char *path;
    rx_sha256_file_status status;
    unsigned char digest[RX_SHA256_DIGEST_SIZE];

    if (NUM_ARGS != 1 || !(path = GETSTRING(ARG0)) || path[0] == '\0')
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXHASH.SHA256FILE expects one non-empty path")
    status = rx_sha256_file(path, digest);
    if (status == RX_SHA256_FILE_LENGTH_OVERFLOW)
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXHASH SHA-256 file exceeds the SHA-256 length limit")
    if (status != RX_SHA256_FILE_OK)
        RETURNSIGNAL(SIGNAL_NOTREADY, rxhash_file_error(status))
    if (SETNATIVEPAYLOAD(RETURN, digest, sizeof(digest), NULL, 0u) != 0)
        RETURNSIGNAL(SIGNAL_FAILURE, "SHA-256 digest allocation failed")
    RESETSIGNAL
}

PROCEDURE(sha256filehex)
{
    const char *path;
    rx_sha256_file_status status;
    unsigned char digest[RX_SHA256_DIGEST_SIZE];
    char hex[RX_SHA256_DIGEST_SIZE * 2u + 1u];

    if (NUM_ARGS != 1 || !(path = GETSTRING(ARG0)) || path[0] == '\0')
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXHASH.SHA256FILEHEX expects one non-empty path")
    status = rx_sha256_file(path, digest);
    if (status == RX_SHA256_FILE_LENGTH_OVERFLOW)
        RETURNSIGNAL(SIGNAL_OVERFLOW_UNDERFLOW,
                     "RXHASH SHA-256 file exceeds the SHA-256 length limit")
    if (status != RX_SHA256_FILE_OK)
        RETURNSIGNAL(SIGNAL_NOTREADY, rxhash_file_error(status))
    rxhash_digest_hex(digest, hex);
    RETURNSTR(hex);
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
ADDPROC(sha256hex, "rxhash.sha256hex", "b", ".string", "data = .binary");
ADDPROC(sha256init, "rxhash.sha256init", "b", ".binary", "");
ADDPROC(sha256update, "rxhash.sha256update", "b", ".binary", "state = .binary,data = .binary");
ADDPROC(sha256final, "rxhash.sha256final", "b", ".binary", "state = .binary");
ADDPROC(sha256finalhex, "rxhash.sha256finalhex", "b", ".string", "state = .binary");
ADDPROC(sha256file, "rxhash.sha256file", "b", ".binary", "path = .string");
ADDPROC(sha256filehex, "rxhash.sha256filehex", "b", ".string", "path = .string");
ADDPROC(djb2, "rxhash.djb2", "b", ".int", "data = .binary");
ADDPROC(murmur3, "rxhash.murmur3", "b", ".int", "data = .binary,seed = .int");
ADDPROC(fnv1a, "rxhash.fnv1a", "b", ".int", "data = .binary");
ADDPROC(crc32, "rxhash.crc32", "b", ".int", "data = .binary");
ENDLOADFUNCS
