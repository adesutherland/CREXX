/* Focused SHA-256 engine, serialized-state, and streaming-I/O coverage. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxsha256.h"

typedef struct test_stream {
    const unsigned char *data;
    size_t length;
    size_t offset;
    size_t maximum_chunk;
    int fail_read_at_end;
    int read_error;
    int close_result;
    int close_calls;
} test_stream;

static int failures = 0;

static void fail(const char *label) {
    fprintf(stderr, "FAIL: %s\n", label);
    ++failures;
}

static void digest_hex(const unsigned char digest[RX_SHA256_DIGEST_SIZE],
                       char hex[RX_SHA256_DIGEST_SIZE * 2u + 1u]) {
    static const char digits[] = "0123456789abcdef";
    size_t index;
    for (index = 0u; index < RX_SHA256_DIGEST_SIZE; ++index) {
        hex[index * 2u] = digits[digest[index] >> 4u];
        hex[index * 2u + 1u] = digits[digest[index] & 15u];
    }
    hex[RX_SHA256_DIGEST_SIZE * 2u] = '\0';
}

static void check_digest(const char *label, const void *data, size_t length,
                         const char *expected) {
    unsigned char digest[RX_SHA256_DIGEST_SIZE];
    char actual[RX_SHA256_DIGEST_SIZE * 2u + 1u];
    rx_sha256(data, length, digest);
    digest_hex(digest, actual);
    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "FAIL: %s expected=%s actual=%s\n",
                label, expected, actual);
        ++failures;
    }
}

static void make_generated(unsigned char *data, size_t length) {
    size_t index;
    for (index = 0u; index < length; ++index) {
        data[index] = (unsigned char)((index * 31u + 7u) & 255u);
    }
}

static size_t test_stream_read(void *opaque, unsigned char *buffer,
                               size_t capacity) {
    test_stream *stream = (test_stream *)opaque;
    size_t remaining;
    size_t take;

    if (stream->offset == stream->length) {
        if (stream->fail_read_at_end) stream->read_error = 1;
        return 0u;
    }
    remaining = stream->length - stream->offset;
    take = remaining < capacity ? remaining : capacity;
    if (stream->maximum_chunk != 0u && take > stream->maximum_chunk) {
        take = stream->maximum_chunk;
    }
    memcpy(buffer, stream->data + stream->offset, take);
    stream->offset += take;
    return take;
}

static int test_stream_error(void *opaque) {
    return ((test_stream *)opaque)->read_error;
}

static int test_stream_close(void *opaque) {
    test_stream *stream = (test_stream *)opaque;
    ++stream->close_calls;
    return stream->close_result;
}

static void check_vectors(void) {
    static const unsigned char abc[] = {'a', 'b', 'c'};
    static const unsigned char binary[] = {
        0x00u, 0xc3u, 0x28u, 0xffu, 0x80u, 0x61u, 0x00u, 0xfeu
    };
    unsigned char generated[4096];

    check_digest("empty", NULL, 0u,
                 "e3b0c44298fc1c149afbf4c8996fb924"
                 "27ae41e4649b934ca495991b7852b855");
    check_digest("abc", abc, sizeof(abc),
                 "ba7816bf8f01cfea414140de5dae2223"
                 "b00361a396177a9cb410ff61f20015ad");
    check_digest("embedded NUL and invalid UTF-8", binary, sizeof(binary),
                 "21a525288802c54dcd15095a8670accc"
                 "60c9033be5b4f7b21aea3793d7388911");

    make_generated(generated, sizeof(generated));
    check_digest("55-byte boundary", generated, 55u,
                 "8aa994584139d128848eeebc4e815639"
                 "ba5ab6e6e39574195a63ac4f14f7c43b");
    check_digest("56-byte boundary", generated, 56u,
                 "ad574708f75c044c9b85de64cb568ee7"
                 "711ff4f36448c6242f053ba8f6cc2b63");
    check_digest("63-byte boundary", generated, 63u,
                 "280ed3e8ff1df845b2e7dfe6ac6cee81"
                 "7bef20e783cc65abc41b818b4d2fe076");
    check_digest("64-byte boundary", generated, 64u,
                 "c6ab9724ade5b6a7a1edfffb12f3aa918"
                 "1351355af8fd08c919952ad211339dd");
    check_digest("65-byte boundary", generated, 65u,
                 "788367c73c7ddf4c53f65e68cc0d943e"
                 "6227ab55b0e78ba63ace822b1c6301c0");
}

static void check_incremental(void) {
    unsigned char generated[4096];
    unsigned char expected[RX_SHA256_DIGEST_SIZE];
    unsigned char actual[RX_SHA256_DIGEST_SIZE];
    rx_sha256_context context;
    size_t offset;

    make_generated(generated, sizeof(generated));
    rx_sha256(generated, sizeof(generated), expected);

    rx_sha256_init(&context);
    if (!rx_sha256_update(&context, generated, sizeof(generated))) {
        fail("one complete update rejected");
        return;
    }
    rx_sha256_final(&context, actual);
    if (memcmp(actual, expected, sizeof(actual)) != 0) {
        fail("one-shot versus one complete update");
    }
    rx_sha256_final(&context, actual);
    if (memcmp(actual, expected, sizeof(actual)) != 0) {
        fail("repeated finalization");
    }

    rx_sha256_init(&context);
    for (offset = 0u; offset < 257u; ++offset) {
        if (!rx_sha256_update(&context, generated + offset, 1u)) {
            fail("byte-at-a-time update rejected");
            return;
        }
    }
    rx_sha256(generated, 257u, expected);
    rx_sha256_final(&context, actual);
    if (memcmp(actual, expected, sizeof(actual)) != 0) {
        fail("byte-at-a-time equivalence");
    }

    rx_sha256_init(&context);
    if (!rx_sha256_update(&context, NULL, 0u)) fail("empty update rejected");
    offset = 0u;
    while (offset < sizeof(generated)) {
        size_t take = ((offset * 17u + 3u) % 131u) + 1u;
        if (take > sizeof(generated) - offset) take = sizeof(generated) - offset;
        if (!rx_sha256_update(&context, generated + offset, take) ||
            !rx_sha256_update(&context, NULL, 0u)) {
            fail("irregular or empty update rejected");
            return;
        }
        offset += take;
    }
    rx_sha256(generated, sizeof(generated), expected);
    rx_sha256_final(&context, actual);
    if (memcmp(actual, expected, sizeof(actual)) != 0) {
        fail("irregular block-crossing equivalence");
    }
}

static void check_serialized_state(void) {
    static const unsigned char prefix[] = {'a', 'b'};
    static const unsigned char suffix[] = {'c'};
    unsigned char state[RX_SHA256_SERIALIZED_STATE_SIZE];
    unsigned char changed[RX_SHA256_SERIALIZED_STATE_SIZE];
    unsigned char digest[RX_SHA256_DIGEST_SIZE];
    rx_sha256_context context;
    rx_sha256_context restored;
    rx_sha256_state_status status;

    rx_sha256_init(&context);
    status = rx_sha256_export_state(&context, state);
    if (status != RX_SHA256_STATE_OK) {
        fail("initial state export");
        return;
    }
    if (memcmp(state, "RXSHA256", 8u) != 0 || state[8] != 1u ||
        state[9] != 0u || state[10] != 0u || state[11] != 56u ||
        state[52] != 0u) {
        fail("canonical state header");
    }
    status = rx_sha256_import_state(&restored, state, sizeof(state));
    if (status != RX_SHA256_STATE_OK) fail("initial state import");

    if (!rx_sha256_update(&context, prefix, sizeof(prefix)) ||
        rx_sha256_export_state(&context, state) != RX_SHA256_STATE_OK ||
        rx_sha256_import_state(&restored, state, sizeof(state)) != RX_SHA256_STATE_OK ||
        !rx_sha256_update(&restored, suffix, sizeof(suffix))) {
        fail("state round trip and resumed update");
        return;
    }
    rx_sha256_final(&restored, digest);
    check_digest("state round-trip control", "abc", 3u,
                 "ba7816bf8f01cfea414140de5dae2223"
                 "b00361a396177a9cb410ff61f20015ad");
    {
        unsigned char expected[RX_SHA256_DIGEST_SIZE];
        rx_sha256("abc", 3u, expected);
        if (memcmp(digest, expected, sizeof(digest)) != 0) {
            fail("state round-trip digest");
        }
    }

    if (rx_sha256_import_state(&restored, state, sizeof(state) - 1u) !=
        RX_SHA256_STATE_INVALID_LENGTH) fail("truncated state rejection");

    memcpy(changed, state, sizeof(state));
    changed[0] ^= 1u;
    if (rx_sha256_import_state(&restored, changed, sizeof(changed)) !=
        RX_SHA256_STATE_INVALID_MAGIC) fail("malformed magic rejection");

    memcpy(changed, state, sizeof(state));
    changed[8] = 2u;
    if (rx_sha256_import_state(&restored, changed, sizeof(changed)) !=
        RX_SHA256_STATE_UNSUPPORTED_VERSION) fail("wrong version rejection");

    memcpy(changed, state, sizeof(state));
    changed[9] = 1u;
    if (rx_sha256_import_state(&restored, changed, sizeof(changed)) !=
        RX_SHA256_STATE_INVALID_FORMAT) fail("nonzero flags rejection");

    memcpy(changed, state, sizeof(state));
    changed[52] = 3u;
    if (rx_sha256_import_state(&restored, changed, sizeof(changed)) !=
        RX_SHA256_STATE_INCONSISTENT) fail("pending count inconsistency rejection");

    memcpy(changed, state, sizeof(state));
    changed[120] ^= 1u;
    if (rx_sha256_import_state(&restored, changed, sizeof(changed)) !=
        RX_SHA256_STATE_INTEGRITY_FAILURE) fail("state integrity rejection");
}

static void check_streaming_file(void) {
    static const size_t length = 3u * 1024u * 1024u + 137u;
    unsigned char *data = (unsigned char *)malloc(length);
    unsigned char digest[RX_SHA256_DIGEST_SIZE];
    char actual[RX_SHA256_DIGEST_SIZE * 2u + 1u];
    test_stream test;
    rx_sha256_stream stream;
    rx_sha256_file_status status;

    if (!data) {
        fail("multi-megabyte fixture allocation");
        return;
    }
    memset(data, 'a', length);
    memset(&test, 0, sizeof(test));
    test.data = data;
    test.length = length;
    test.maximum_chunk = 65521u;
    stream.context = &test;
    stream.read = test_stream_read;
    stream.error = test_stream_error;
    stream.close = test_stream_close;
    status = rx_sha256_stream_digest(&stream, digest);
    digest_hex(digest, actual);
    if (status != RX_SHA256_FILE_OK ||
        strcmp(actual, "ec20e65865de23ccd97d25891db3bf75"
                       "a62a405194f8a9de9ba9cb870544d4d0") != 0 ||
        test.close_calls != 1) {
        fail("multi-megabyte bounded streaming digest");
    }

    memset(&test, 0, sizeof(test));
    test.data = data;
    test.length = 17u;
    test.fail_read_at_end = 1;
    stream.context = &test;
    status = rx_sha256_stream_digest(&stream, digest);
    if (status != RX_SHA256_FILE_READ_FAILURE || test.close_calls != 1) {
        fail("deterministic stream read failure");
    }

    memset(&test, 0, sizeof(test));
    test.data = data;
    test.length = 17u;
    test.close_result = -1;
    stream.context = &test;
    status = rx_sha256_stream_digest(&stream, digest);
    if (status != RX_SHA256_FILE_CLOSE_FAILURE || test.close_calls != 1) {
        fail("deterministic stream close failure");
    }

    if (rx_sha256_file("crexx-rxsha256-definitely-missing-file", digest) !=
        RX_SHA256_FILE_OPEN_FAILURE) {
        fail("deterministic file open failure");
    }
    free(data);
}

int main(void) {
    check_vectors();
    check_incremental();
    check_serialized_state();
    check_streaming_file();
    if (failures != 0) {
        fprintf(stderr, "rxsha256 unit failures=%d\n", failures);
        return 1;
    }
    puts("PASS: rxsha256 incremental state and streaming file engine");
    return 0;
}
